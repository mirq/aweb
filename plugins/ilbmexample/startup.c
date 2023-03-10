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
#include <clib/exec_protos.h>
#include <proto/exec.h>

/* Function declarations for the library entry points */
struct SegList;

USRFUNC_P3
(
__saveds struct Library * , Initlib,
struct ExecBase *,sysbase,A6,
struct SegList *,seglist,A0,
struct Library *,libbase,D0
);

USRFUNC_P1
(
__saveds struct Library * , Openlib,
struct Library *,libbase,A6
);

USRFUNC_P1
(
__saveds struct SegList * , Closelib,
struct Library *,libbase,A6
);

USRFUNC_P1
(
__saveds struct SegList * , Expungelib,
struct Library *,libbase,A6
);

USRFUNC_P0
(
__saveds ULONG   , Extfunclib
);

USRFUNC_P1
(
extern   __saveds ULONG   , Initplugin,
struct Plugininfo *,pi,A0
);

USRFUNC_P1
(
extern   __saveds void  , Queryplugin,
struct Pluginquery *,pq,A0
);

USRFUNC_P1
(
extern   __saveds void  , Commandplugin,
struct Plugincommand *,pc,A0
);

USRFUNC_P1
(
extern   __saveds void  , Filterplugin,
struct Pluginfilter *,pf,A0
);

/* Function declarations for project dependent hook functions */
extern ULONG Initpluginlib(struct Library *libbase);
extern void Expungepluginlib(struct Library *libbase);

/* Our library base. Probably not really a struct Library but a
 * derived structure. */
struct Library *AwebPluginBase;

/* Our seglist, to be returned when expunged. */
static APTR libseglist;

struct ExecBase *SysBase;

/* Return error when run as a program */
USRFUNC_H0
(
LONG __saveds   , Libstart
)
{
    USRFUNC_INIT
  return -1;

    USRFUNC_EXIT
}

/* Library function table. The library vectors will be
 * generated from these. */
static APTR functable[]=
{  Openlib,
   Closelib,
   Expungelib,
   Extfunclib,
   Initplugin,
   Queryplugin,
#ifdef PLUGIN_COMMANDPLUGIN
   Commandplugin,
#else
   NULL,
#endif
#ifdef PLUGIN_FILTERPLUGIN
   Filterplugin,
#else
   NULL,
#endif
   (APTR)-1
};

/* Init table used in library initialization. */
static ULONG inittab[]=
{  PLUGIN_SIZE,    /* Size of library base structure */
   (ULONG) functable,   /* Library function table */
   0,         /* Library base structure initialization */
   (ULONG) Initlib   /* Initial entry point */
};

static char __aligned libname[]=PLUGIN_LIBNAME;
static char __aligned libid[]=PLUGIN_LIBID;

/* The ROM tag */
struct Resident __aligned romtag=
{  RTC_MATCHWORD,
   &romtag,
   &romtag+1,
   RTF_AUTOINIT,
   PLUGIN_VERSION,
   NT_LIBRARY,
   0,
   libname,
   libid,
   inittab
};

/* Initialization of the library after loading. Most of the library
 * node is already initialized from the data in the ROM tag, except
 * for the revision.
 * Store vital pointers, and call the Initpluginlib hook function. */
USRFUNC_H3
(
__saveds struct Library * , Initlib,
struct ExecBase *,sysbase,A6,
struct SegList *,seglist,A0,
struct Library *,libbase,D0
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

USRFUNC_H1
(
__saveds struct Library * , Openlib,
struct Library *,libbase,A6
)
{
    USRFUNC_INIT
  libbase->lib_OpenCnt++;
   libbase->lib_Flags&=~LIBF_DELEXP;
   return libbase;

    USRFUNC_EXIT
}

USRFUNC_H1
(
__saveds struct SegList * , Closelib,
struct Library *,libbase,A6
)
{
    USRFUNC_INIT
  libbase->lib_OpenCnt--;
   if(libbase->lib_OpenCnt==0 && (libbase->lib_Flags&LIBF_DELEXP))
   {  return Expungelib(libbase);
   }
   return NULL;

    USRFUNC_EXIT
}

USRFUNC_H1
(
__saveds struct SegList * , Expungelib,
struct Library *,libbase,A6
)
{
    USRFUNC_INIT
  if(libbase->lib_OpenCnt==0)
   {  ULONG size=libbase->lib_NegSize+libbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)libbase-libbase->lib_NegSize;
      Remove((struct Node *)libbase);
      Expungepluginlib(libbase);
      FreeMem(ptr,size);
      return libseglist;
   }
   libbase->lib_Flags|=LIBF_DELEXP;
   return NULL;

    USRFUNC_EXIT
}

USRFUNC_H0
(
__saveds ULONG   , Extfunclib
)
{
    USRFUNC_INIT
  return 0;

    USRFUNC_EXIT
}
