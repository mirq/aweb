/**********************************************************************
 *
 * This file is part of the AWeb-II distribution
 *
 * Copyright (C) 2002 Yvon Rozijn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the AWeb Public License as included in this
 * distribution.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * AWeb Public License for more details.
 *
 **********************************************************************/

/* startup.c
 *
 * General startup file for AWeb plugin modules
 *
 * Loosely based on the "demo library in pure C" code
 * by Andreas R. Kleinert.
 *
 * Use this file unchanged as startup file in your own
 * AWeb plugin module project.
 *
 */

#include "pluginlib.h"
#include <libraries/awebsupport.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/resident.h>
#include <proto/exec.h>

#include "proto/librarymanager.h"

struct SegList;

#if defined(__amigaos4__)

USRFUNC_P3
(
__saveds struct Library * , Initlib,
struct Library *,libbase,D0,
struct SegList *,seglist,A0,
struct ExecIFace *,exec,A6

);

#else

USRFUNC_P3
(
__saveds struct Library * , Initlib,
struct Library *,libbase,D0,
struct SegList *,seglist,A0,
struct ExecBase *,sysbase,A6

);

#endif
LIBFUNC_P0
(
__saveds struct Library * , Openlib,
LIBMAN_TYPE, LIBMAN_NAME
);

LIBFUNC_P0
(
 __saveds struct SegList * , Closelib,
 LIBMAN_TYPE, LIBMAN_NAME
);

LIBFUNC_P0
(
__saveds struct SegList * , Expungelib,
LIBMAN_TYPE, LIBMAN_NAME
);

LIBFUNC_P0
(
__saveds ULONG  , Extfunclib,
LIBMAN_TYPE, LIBMAN_NAME
);


LIBFUNC_P1
(
extern  __saveds ULONG  , Initplugin,
struct Plugininfo *,pi,A0,
PLUGIN_TYPE, PLUGIN_NAME
);

LIBFUNC_P1
(
extern  __saveds void  , Queryplugin,
struct Pluginquery *,pq,A0,
PLUGIN_TYPE, PLUGIN_NAME

);

LIBFUNC_P1
(
extern  __saveds void  , Commandplugin,
struct Plugincommand *,pc,A0,
PLUGIN_TYPE, PLUGIN_NAME

);

LIBFUNC_P1
(
extern  __saveds void  , Filterplugin,
struct Pluginfilter *,pf,A0,
PLUGIN_TYPE, PLUGIN_NAME

);

/* Function declarations for project dependent hook functions */
extern ULONG Initpluginlib(struct Library *libbase);
extern void Expungepluginlib(struct Library *libbase);



struct Library *AwebPluginBase;

static APTR libseglist;

struct ExecBase *SysBase;


LIBSTART_DUMMY

static char __aligned libname[]=PLUGIN_LIBNAME;
static char __aligned libid[]=PLUGIN_LIBID;

/*----------------------------------------------------------------------*/
#if defined(__amigaos4__)


/* ------------------- Manager Interface ------------------------ */
static LONG _manager_Obtain(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount++;
}

static ULONG _manager_Release(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount--;
}

/* Manager interface vectors */
static void *lib_manager_vectors[] =
{
    (void *)_manager_Obtain,
    (void *)_manager_Release,
    (void *)0,
    (void *)0,
    (void *)Openlib,
    (void *)Closelib,
    (void *)Expungelib,
    (void *)0,
    (void *)-1,
};

/* "__library" interface tag list */
static struct TagItem lib_managerTags[] =
{
    {MIT_Name,             (ULONG)"__library"},
    {MIT_VectorTable,      (ULONG)lib_manager_vectors},
    {MIT_Version,          1},
    {TAG_DONE,             0}
};

/* ------------------- Library Interface(s) ------------------------ */

/* ------------------- Main Interface ------------------------------ */

ULONG _Main_Obtain(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount++;

}

static ULONG _Main_Release(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount--;
}

static void *main_vectors[] = {
        (void *)_Main_Obtain,
        (void *)_Main_Release,
        (void *)NULL,
        (void *)NULL,
        (void *)Initplugin,
        (void *)Queryplugin,
#ifdef PLUGIN_COMMANDPLUGIN
        (void *)Commandplugin,
#else
                NULL,
#endif
#ifdef PLUGIN_FILTERPLUGIN
        (void *)Filterplugin,
#else
                NULL,
#endif

        (void *)-1
};



/* extern ULONG VecTable68K;*/

static struct TagItem mainTags[] =
{
    {MIT_Name,              (uint32)"main"},
    {MIT_VectorTable,       (uint32)main_vectors},
    {MIT_Version,           1},
    {TAG_DONE,              0}
};

static uint32 libInterfaces[] =
{
    (uint32)lib_managerTags,
    (uint32)mainTags,
    (uint32)0
};

struct TagItem libCreateTags[] =
{
    {CLT_DataSize,         (uint32)PLUGIN_SIZE},
    {CLT_InitFunc,         (uint32)Initlib},
    {CLT_Interfaces,       (uint32)libInterfaces},
 /*   {CLT_Vector68K,        (uint32)&VecTable68K}, */
    {TAG_DONE,             0}
};


/* ------------------- ROM Tag ------------------------ */
static struct Resident lib_res __attribute((used)) =
{
    RTC_MATCHWORD,
    &lib_res,
    &lib_res+1,
    RTF_NATIVE|RTF_AUTOINIT,
    PLUGIN_VERSION,
    NT_LIBRARY,
    0,
    libname,
    libid,
    libCreateTags
};







#else // !(os4 but os3)
/*----------------------------------------------------------------------*/
/* OS3.x Library */



static APTR functable[]=
{
#ifdef __MORPHOS__
   (APTR) FUNCARRAY_32BIT_NATIVE,
#endif
   Openlib,
   Closelib,
   Expungelib,
   Extfunclib,
   Initplugin,
   (APTR)-1
};

/* Init table used in library initialization. */
static ULONG inittab[]=
{  PLUGIN_SIZE,
   (ULONG) functable,
   0,
   (ULONG) Initlib
};


/* The ROM tag */
struct Resident __aligned romtag=
{  RTC_MATCHWORD,
   &romtag,
   &romtag+1,
#ifdef __MORPHOS__
   RTF_PPC | RTF_AUTOINIT,
#else
   RTF_AUTOINIT,
#endif
   PLUGIN_VERSION,
   NT_LIBRARY,
   0,
   libname,
   libid,
   inittab
};

#endif

#if defined(__amigaos4__)

USRFUNC_H3
(
 __saveds struct Library *, Initlib,
 struct Library *, libBase, D0,
 struct SegList *, seglist, A0,
 struct ExecIFace *, exec, A6
)

// struct Library *Initlib(struct Library *LibraryBase, APTR seglist, struct Interface *exec)
{
    struct ExecIFace *IExec
#ifdef __GNUC__
        __attribute__((unused))
#endif
        = (struct ExecIFace *)exec;

    AwebPluginBase = libBase;

    libBase->lib_Node.ln_Type = NT_LIBRARY;
    libBase->lib_Node.ln_Pri  = 0;
    libBase->lib_Node.ln_Name = libname;
    libBase->lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
    libBase->lib_Version      = PLUGIN_VERSION;
    libBase->lib_Revision     = PLUGIN_REVISION;
    libBase->lib_IdString     = libid;

    libseglist = seglist;

    if(!Initpluginlib((struct Library *)libBase))
    {
        Expungepluginlib((struct Library *)libBase);
        libBase = NULL;
    }

       return (struct Library *)libBase;
}


#else
USRFUNC_H3
(
__saveds struct Library * , Initlib,
struct Library *,libbase,D0,
struct SegList *,seglist,A0,
struct ExecBase *,sysbase,A6
)
{
    USRFUNC_INIT
  SysBase=sysbase;
   AwebPluginBase=libbase;
   libbase->lib_Revision=PLUGIN_REVISION;
   libseglist=seglist;
   if(!Initpluginlib(libbase))
   {  Expungepluginlib(libbase);
      libbase=NULL;
   }
   return libbase;

    USRFUNC_EXIT
}

#endif

LIBFUNC_H0
(
__saveds struct Library * , Openlib,
LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT

#if defined(__amigaos4__)
   struct Library *Pluginlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Pluginlibbase = (struct Library *)LIBMAN_NAME;
#endif

   Pluginlibbase->lib_OpenCnt++;
   Pluginlibbase->lib_Flags&=~LIBF_DELEXP;
   return Pluginlibbase;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds struct SegList * , Closelib,
LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT

#if defined(__amigaos4__)
   struct Library *Pluginlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Pluginlibbase = (struct Library *)LIBMAN_NAME;
#endif


  Pluginlibbase->lib_OpenCnt--;
   if(Pluginlibbase->lib_OpenCnt==0)
   {
      if(Pluginlibbase->lib_Flags&LIBF_DELEXP)
      {  return (struct SegList *)__LibExpunge_WB(LIBMAN_NAME);
      }
   }
   return NULL;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds struct SegList * , Expungelib,
LIBMAN_TYPE,LIBMAN_NAME
)
{
    LIBFUNC_INIT

#if defined(__amigaos4__)
   struct Library *Pluginlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Pluginlibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Pluginlibbase->lib_OpenCnt==0)
   {  ULONG size=Pluginlibbase->lib_NegSize+Pluginlibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Pluginlibbase-Pluginlibbase->lib_NegSize;
      Remove((struct Node *)Pluginlibbase);
      Expungepluginlib(Pluginlibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Pluginlibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Pluginlibbase->lib_Flags|=LIBF_DELEXP;
   return NULL;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds ULONG  , Extfunclib,
LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT
  return 0;

    LIBFUNC_EXIT
}
