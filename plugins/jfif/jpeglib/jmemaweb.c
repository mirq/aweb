/*
 * jmemaweb.c
 *
 * Copyright (C) 1992-1994, Thomas G. Lane, 1998 Yvon Rozijn
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides an Amiga-AWeb implementation of the system-dependent
 * portion of the JPEG memory manager.   This implementation assumes that
 * you must explicitly construct a name for each temp file.
 * Also, the problem of determining the amount of memory available
 * is shoved onto the user.
 */

#define __USE_INLINE__
#define __USE_BASETYPE__

#include <exec/memory.h>
#include <exec/semaphores.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#undef GLOBAL
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"            /* import the system-dependent declarations */

extern struct ExecBase *SysBase;
extern struct DosLibrary  *DOSBase;

/*
 * Selection of a file name for a temporary file.
 * This is system-dependent!
 *
 * The code as given is suitable for most Unix systems, and it is easily
 * modified for most non-Unix systems.   Some notes:
 *   1.   The temp file is created in the directory named by TEMP_DIRECTORY.
 *         The default value is /usr/tmp, which is the conventional place for
 *         creating large temp files on Unix.   On other systems you'll probably
 *         want to change the file location.   You can do this by editing the
 *         #define, or (preferred) by defining TEMP_DIRECTORY in jconfig.h.
 *
 *   2.   If you need to change the file name as well as its location,
 *         you can override the TEMP_FILE_NAME macro.   (Note that this is
 *         actually a printf format string; it must contain %s and %d.)
 *         Few people should need to do this.
 *
 *   3.   mktemp() is used to ensure that multiple processes running
 *         simultaneously won't select the same file names.   If your system
 *         doesn't have mktemp(), define NO_MKTEMP to do it the hard way.
 *         (If you don't have <errno.h>, also define NO_ERRNO_H.)
 *
 *   4.   You probably want to define NEED_SIGNAL_CATCHER so that cjpeg.c/djpeg.c
 *         will cause the temp files to be removed if you stop the program early.
 */

#ifndef TEMP_DIRECTORY
#define TEMP_DIRECTORY "JPEGTMP:"
#endif

static struct SignalSemaphore memsema;
static boolean initialized=FALSE;
static int next_file_num;       /* to distinguish among several temp files */

#ifdef TEMP_FILE_NAME
#undef TEMP_FILE_NAME
#endif
#define TEMP_FILE_NAME   "%sAWEBJFIF%03d.TMP"

LOCAL(void)
select_file_name (char * fname)
{
   long tlock;

   /* Keep generating file names till we find one that's not in use */
   for (;;) {
      next_file_num++;          /* advance counter */
      sprintf(fname, TEMP_FILE_NAME, TEMP_DIRECTORY, next_file_num);
      if ((tlock = Lock(fname,SHARED_LOCK)) == NULL)
      {  break;
      }
      UnLock(tlock);            /* oops, it's there; close tfile & try again */
   }
}

/*
 * Memory allocation and freeing are controlled by the regular Amiga library
 * routines AllocVec() and FreeVec()
 */

GLOBAL(void *)
jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
   return (void *) AllocVec(sizeofobject,MEMF_PUBLIC);
}

GLOBAL(void)
jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject)
{
   FreeVec(object);
}


/*
 * "Large" objects are treated the same as "small" ones.
 * NB: although we include FAR keywords in the routine declarations,
 * this file won't actually work in 80x86 small/medium model; at least,
 * you probably won't be able to process useful-size images in only 64KB.
 */

GLOBAL(void FAR *)
jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{
   return (void FAR *) AllocVec(sizeofobject,MEMF_PUBLIC);
}

GLOBAL(void)
jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject)
{
   FreeVec(object);
}


/*
 * This routine computes the total memory space available for allocation.
 * It's impossible to do this in a portable way; our current solution is
 * to make the user tell us (with a default value set at compile time).
 * If you can actually get the available space, it's a good idea to subtract
 * a slop factor of 5% or so.
 */

#ifndef DEFAULT_MAX_MEM         /* so can override from makefile */
#define DEFAULT_MAX_MEM         1000000L /* default: one megabyte */
#endif

GLOBAL(long)
jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
                      long max_bytes_needed, long already_allocated)
{
   return cinfo->mem->max_memory_to_use - already_allocated;
}


/*
 * Backing store (temporary file) management.
 * Backing store objects are only used when the value returned by
 * jpeg_mem_available is less than the total space needed.   You can dispense
 * with these routines if you have plenty of virtual memory; see jmemnobs.c.
 */


METHODDEF(void)
read_backing_store (j_common_ptr cinfo, backing_store_ptr info,
                      void FAR * buffer_address,
                      long file_offset, long byte_count)
{
   Seek((long)info->temp_file, file_offset, OFFSET_BEGINNING);
   if (Read((long)info->temp_file, buffer_address, byte_count)
         != byte_count)
      ERREXIT(cinfo, JERR_TFILE_READ);
}


METHODDEF(void)
write_backing_store (j_common_ptr cinfo, backing_store_ptr info,
                       void FAR * buffer_address,
                       long file_offset, long byte_count)
{
   Seek((long)info->temp_file, file_offset, OFFSET_BEGINNING);
   if (Write((long)info->temp_file, buffer_address, byte_count)
         != byte_count)
      ERREXIT(cinfo, JERR_TFILE_WRITE);
}


METHODDEF(void)
close_backing_store (j_common_ptr cinfo, backing_store_ptr info)
{
   Close((long)info->temp_file);        /* close the file */
   DeleteFile(info->temp_name); /* delete the file */
   TRACEMSS(cinfo, 1, JTRC_TFILE_CLOSE, info->temp_name);
}


/*
 * Initial opening of a backing-store object.
 */

GLOBAL(void)
jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
                         long total_bytes_needed)
{
   ObtainSemaphore(&memsema);
   select_file_name(info->temp_name);
   if ((info->temp_file = (FILE *)Open(info->temp_name, MODE_NEWFILE)) == NULL)
   {  ReleaseSemaphore(&memsema);
      ERREXITS(cinfo, JERR_TFILE_CREATE, info->temp_name);
   }
   ReleaseSemaphore(&memsema);

   info->read_backing_store = read_backing_store;
   info->write_backing_store = write_backing_store;
   info->close_backing_store = close_backing_store;
   TRACEMSS(cinfo, 1, JTRC_TFILE_OPEN, info->temp_name);
}


/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.
 */

GLOBAL(long)
jpeg_mem_init (j_common_ptr cinfo)
{
   Forbid();
   if(!initialized)
   {  InitSemaphore(&memsema);
      next_file_num = 0;                /* initialize temp file name generator */
      initialized=TRUE;
   }
   Permit();

   return DEFAULT_MAX_MEM;      /* default for max_memory_to_use */
}

GLOBAL(void)
jpeg_mem_term (j_common_ptr cinfo)
{
   /* no work */
}
