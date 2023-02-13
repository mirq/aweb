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

/* jslib.c - AWeb js awebplugin library module */

#define NO_INLINE_STDARG

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include "platform_specific.h"

#include "awebjs.h"
#include "jprotos.h"
#include "keyfile.h"
#include "libraries/awebmodule.h"

#include "libraries/awebclib.h"

#include <libraries/locale.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <exec/resident.h>
#include <graphics/gfxbase.h>
#include <reaction/reaction.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#ifdef __MORPHOS__
/*
 * To tell the loader that this is a new abox elf and not
 * one for the ppc.library.
 * ** IMPORTANT **
 */
const ULONG   __abox__=1;

#endif

#if defined(__amigaos4__)

/* stop inlines and redefinitions  */
#undef __USE_INLINE__                  /* stops os4 inlines*/
#define LIBRARIES_AWEBJS_H
#include "proto/awebjs.h"

#endif


#include <time.h>
#include <stdio.h>

#define JAVASCRIPT_VERSION 38
#define JAVASCRIPT_REVISION 0
#define JAVASCRIPT_VERSTRING "38.0 " CPU


#define PUDDLESIZE         16*1024
#define TRESHSIZE          4*1024

struct ExecBase *SysBase;
struct DosLibrary *DOSBase;

#if defined(__amigaos4__) || defined(__MORPHOS__)
struct Library *LocaleBase;
#else
struct LocaleBase *LocaleBase;
#endif

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *UtilityBase,*AslBase;

#if defined(__amigaos4__)
struct ExecIFace *IExec;
struct DOSIFace *IDOS;
struct LocaleIFace *ILocale;
struct IntuitionIFace *IIntuition;
struct GraphicsIFace *IGraphics;
struct UtilityIFace *IUtility;
struct AslIFace *IAsl;
#endif

struct Locale *locale;
struct Hook idcmphook;


/* Runtime error values */
#define JERRORS_CONTINUE   -1 /* Don't show errors and try to continue script */
#define JERRORS_OFF        0  /* Don't show errors and stop script */
#define JERRORS_ON         1  /* Show errors and stop script */

/*-----------------------------------------------------------------------*/
/* AWebLib module startup */

struct SegList;

#if defined (__amigaos4__)

USRFUNC_P3
(
 __saveds struct Library *, Initlib,
 struct Library *, libbase, D0,
 APTR, seglist, A0,
 struct ExecIFace *, exec, A6
);

#else

USRFUNC_P3
(
 __saveds struct Library *, Initlib,
 struct Library *, libbase, D0,
 struct SegList *, seglist, A0,
 struct ExecBase *, sysbase, A6
);

#endif

LIBFUNC_P0
(
 __saveds struct Library *, Openlib,
  LIBMAN_TYPE, LIBMAN_NAME
);

static __saveds struct SegList * Real_Closelib(LIBMAN_TYPE LIBMAN_NAME);
LIBFUNC_P0
(
 __saveds struct SegList *, Closelib,
  LIBMAN_TYPE, LIBMAN_NAME

);

static __saveds struct SegList * Real_Expungelib(LIBMAN_TYPE LIBMAN_NAME);
LIBFUNC_P0
(
 __saveds struct SegList *, Expungelib,
  LIBMAN_TYPE, LIBMAN_NAME

);

LIBFUNC_P0
(
 __saveds ULONG , Extfunclib
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds void *, Newjcontext,
 UBYTE *, screenname, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds void , Freejcontext,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P7
(
 __saveds BOOL , Runjprogram,
 struct Jcontext *, jc, A0,
 struct Jobject *, fscope, A1,
 UBYTE *, source, A2,
 struct Jobject *, jthis, A3,
 struct Jobject **, gwtab, A4,
 ULONG , protkey, D0,
 ULONG , userdata, D1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds void *, Newjobject,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds void , Disposejobject,
 struct Jobject *, jo, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P5
(
 __saveds struct Jobject *, AddjfunctionA,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2,
 APTR, code , A3,
 UBYTE **, args, A4
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

#if defined (__amigaos4__)
LIBFUNC_P5
(
 __saveds VARARGS68K struct Jobject *, Addjfunction,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2,
 void (*code)(void *), , A3,
 , ... ,
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME

);
#endif

LIBFUNC_P2
(
 __saveds struct Variable *, Jfargument,
 struct Jcontext *, jc, A0,
 long , n, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds UBYTE *, Jtostring,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P3
(
 __saveds void , Jasgstring,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1,
 UBYTE *, string, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P3
(
 __saveds void , Jasgobject,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1,
 struct Jobject *, jo, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P4
(
 __saveds void , Setjobject,
 struct Jobject *, jo, A0,
 Objhookfunc *, hook, A1,
 void *, internal, A2,
 Objdisposehookfunc *, dispose , A3
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P3
(
 __saveds struct Variable *, Jproperty,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P3
(
 __saveds void , Setjproperty,
 struct Variable *, jv, A0,
 Varhookfunc *, hook, A1,
 void *, hookdata, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds struct Jobject *, Jthis,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds void *, Jointernal,
 struct Jobject *, jo, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P3
(
 __saveds void , Jasgboolean,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1,
 BOOL , bvalue, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds BOOL , Jtoboolean,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds struct Jobject *, Newjarray,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds struct Variable *, Jnewarrayelt,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds struct Jobject *, Jtoobject,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds long , Jtonumber,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P3
(
 __saveds void , Jasgnumber,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1,
 long , nvalue, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds BOOL , Jisarray,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P3
(
 __saveds struct Jobject *, Jfindarray,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P3
(
 __saveds void , Jsetprototype,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 struct Jobject *, proto, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds ULONG , Jgetuserdata,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds BOOL , Jisnumber,
 struct Variable *, jv, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Clearjobject,
 struct Jobject *, jo, A0,
 UBYTE **, except, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds void , Freejobject,
 struct Jobject *, jo, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds void , Jdumpobjects,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds struct Variable *, Jgetreturnvalue,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Jpprotect,
 struct Variable *, var, A0,
 ULONG , protkey, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Jcprotect,
 struct Jcontext *, jc, A0,
 ULONG , protkey, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds UBYTE *, Jpname,
 struct Variable *, var, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds Objdisposehookfunc *, Jdisposehook,
 struct Jobject *, jo, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Jsetfeedback,
 struct Jcontext *, jc, A0,
 Jfeedback *, jf, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Jdebug,
 struct Jcontext *, jc, A0,
 BOOL , debugon, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P4
(
 __saveds void , Jerrors,
 struct Jcontext *, jc, A0,
 BOOL , comperrors, D0,
 long , runerrors, D1,
 BOOL , watch, D2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Jkeepobject,
 struct Jobject *, jo, A0,
 BOOL , used, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds void , Jgarbagecollect,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Jsetlinenumber,
 struct Jcontext *, jc, A0,
 long , linenr, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Jsetobjasfunc,
 struct Jobject *, jo, A0,
 BOOL , asfunc, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Jsetscreen,
 struct Jcontext *, jc, A0,
 UBYTE *, screenname, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P4
(
 __saveds void , Jaddeventhandler,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2,
 UBYTE *, source, A3
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P3
(
 __saveds struct Variable *, Jaddproperty,
 struct Jcontext  *,jc, A0,
 struct Jobject   *,jo, A1,
 UBYTE *, name, A2,
 JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);


LIBFUNC_P1
(
 __saveds struct Jobject *, Newjscope,
 struct Jcontext *, jc, A0,
 JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P1
(
 __saveds void, Disposejscope,
 struct Jobject *, jo, A0,
 JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

LIBFUNC_P2
(
 __saveds void , Jallowgc,
 struct Jcontext *, jc, A0,
 BOOL, allow, D0,
 JAVASCRIPT_TYPE, JAVASCRIPT_NAME
);

/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct AWebJSBase
{
    struct Library libNode;

};


struct AWebJSBase *AWebJSBase;

APTR libseglist;

/* the dummy function in case this lib gets run */

LIBSTART_DUMMY

static char __aligned libname[]="javascript.aweblib";
static char __aligned libid[]="javascript.aweblib " JAVASCRIPT_VERSTRING " " __AMIGADATE__;


#if defined(__amigaos4__)

/* OS4.0 specific function tables and library manager interface defintions */

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

ULONG _AWebJS_Obtain(struct AWebJSIFace *Self)
{
    return Self->Data.RefCount++;

}

static ULONG _AWebJS_Release(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount--;
}

static void *main_vectors[] = {
        (void *)_AWebJS_Obtain,
        (void *)_AWebJS_Release,
        (void *)NULL,
        (void *)NULL,
        (void *)Newjcontext,
        (void *)Freejcontext,
        (void *)Runjprogram,
        (void *)Newjobject,
        (void *)Disposejobject,
        (void *)AddjfunctionA,
        (void *)Addjfunction,
        (void *)Jfargument,
        (void *)Jtostring,
        (void *)Jasgstring,
        (void *)Jasgobject,
        (void *)Setjobject,
        (void *)Jproperty,
        (void *)Setjproperty,
        (void *)Jthis,
        (void *)Jointernal,
        (void *)Jasgboolean,
        (void *)Jtoboolean,
        (void *)Newjarray,
        (void *)Jnewarrayelt,
        (void *)Jtoobject,
        (void *)Jtonumber,
        (void *)Jasgnumber,
        (void *)Jisarray,
        (void *)Jfindarray,
        (void *)Jsetprototype,
        (void *)Jgetuserdata,
        (void *)Jisnumber,
        (void *)Clearjobject,
        (void *)Freejobject,
        (void *)Jdumpobjects,
        (void *)Jgetreturnvalue,
        (void *)Jpprotect,
        (void *)Jcprotect,
        (void *)Jpname,
        (void *)Jdisposehook,
        (void *)Jsetfeedback,
        (void *)Jdebug,
        (void *)Jerrors,
        (void *)Jkeepobject,
        (void *)Jgarbagecollect,
        (void *)Jsetlinenumber,
        (void *)Jsetobjasfunc,
        (void *)Jsetscreen,
        (void *)Jaddeventhandler,
        (void *)Jaddproperty,
        (void *)Newjscope,
        (void *)Disposejscope,
        (void *)Jallowgc,
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
    {CLT_DataSize,         (uint32)(sizeof(struct AWebJSBase))},
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
    JAVASCRIPT_VERSION,
    NT_LIBRARY,
    0,
    libname,
    libid,
    libCreateTags
};





#else

/* os3.x style function tables etc */

static APTR functable[]=
{
#ifdef __MORPHOS__
   (APTR)FUNCARRAY_32BIT_NATIVE,
#endif
         Openlib,
   Closelib,
   Expungelib,
   Extfunclib,
   Newjcontext,
   Freejcontext,
   Runjprogram,
   Newjobject,
   Disposejobject,
   AddjfunctionA,
   Jfargument,
   Jtostring,
   Jasgstring,
   Jasgobject,
   Setjobject,
   Jproperty,
   Setjproperty,
   Jthis,
   Jointernal,
   Jasgboolean,
   Jtoboolean,
   Newjarray,
   Jnewarrayelt,
   Jtoobject,
   Jtonumber,
   Jasgnumber,
   Jisarray,
   Jfindarray,
   Jsetprototype,
   Jgetuserdata,
   Jisnumber,
   Clearjobject,
   Freejobject,
   Jdumpobjects,
   Jgetreturnvalue,
   Jpprotect,
   Jcprotect,
   Jpname,
   Jdisposehook,
   Jsetfeedback,
   Jdebug,
   Jerrors,
   Jkeepobject,
   Jgarbagecollect,
   Jsetlinenumber,
   Jsetobjasfunc,
   Jsetscreen,
   Jaddeventhandler,
   Jaddproperty,
   Newjscope,
   Disposejscope,
   Jallowgc,
   (APTR)-1
};

/* Init table used in library initialization. */
static ULONG inittab[]=
{  sizeof(struct Library),
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
   JAVASCRIPT_VERSION,
   NT_LIBRARY,
   0,
   libname,
   libid,
   inittab
};

//#else
//#error "Unsupported OS!"
#endif


/* Just to get the ball rolling use a bulk ifdef to choose */
/* os3 or os4 Initlib functions */
#if defined(__amigaos4__)

USRFUNC_H3
(
 __saveds struct Library *, Initlib,
 struct Library *, libbase, D0,
 APTR, seglist, A0,
 struct ExecIFace *, exec, A6
)

// struct Library *Initlib(struct Library *LibraryBase, APTR seglist, struct Interface *exec)
{
    struct AWebJSBase *libBase = (struct AWebJSBase *)libbase;
    struct ExecIFace *IExec
#ifdef __GNUC__
        __attribute__((unused))
#endif
        = (struct ExecIFace *)exec;


    libBase->libNode.lib_Node.ln_Type = NT_LIBRARY;
    libBase->libNode.lib_Node.ln_Pri  = 0;
    libBase->libNode.lib_Node.ln_Name = libname;
    libBase->libNode.lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
    libBase->libNode.lib_Version      = JAVASCRIPT_VERSION;
    libBase->libNode.lib_Revision     = JAVASCRIPT_REVISION;
    libBase->libNode.lib_IdString     = libid;

    libseglist = seglist;

    if(!Initaweblib((struct Library *)libBase))
    {
        Expungeaweblib((struct Library *)libBase);
        libBase = NULL;
    }

       return (struct Library *)libBase;
}


#else

USRFUNC_H3
(
 __saveds struct Library *, Initlib,
 struct Library *, libbase, D0,
 struct SegList *, seglist, A0,
 struct ExecBase *, sysbase, A6
)
{
     USRFUNC_INIT
  SysBase=sysbase;
   AWebJSBase=libbase;
   libbase->lib_Revision=JAVASCRIPT_REVISION;
   libseglist=seglist;
   if(!Initaweblib(libbase))
   {  Expungeaweblib(libbase);
      libbase=NULL;
   }
   return libbase;
     USRFUNC_EXIT

}
#endif

LIBFUNC_H0
(
 __saveds struct Library *, Openlib,
  LIBMAN_TYPE, LIBMAN_NAME
)
{
     LIBFUNC_INIT

#if defined(__amigaos4__)
  struct AWebJSBase *AlibBase = (struct AWebJSBase *)LIBMAN_NAME->Data.LibBase;
#else
  struct AWebJSBase *AlibBase = (struct AWebJSBase *)LIBMAN_NAME;
#endif

    AlibBase->libNode.lib_OpenCnt++;
    AlibBase->libNode.lib_Flags&=~LIBF_DELEXP;
   return (struct Library *)AlibBase;

     LIBFUNC_EXIT

}

LIBFUNC_H0
(
 __saveds struct SegList *, Closelib,
  LIBMAN_TYPE, LIBMAN_NAME
)
{
     LIBFUNC_INIT

#if defined(__amigaos4__)
  struct AWebJSBase *AlibBase = (struct AWebJSBase *)LIBMAN_NAME->Data.LibBase;
#else
  struct AWebJSBase *AlibBase = (struct AWebJSBase *)LIBMAN_NAME;
#endif

   AlibBase->libNode.lib_OpenCnt--;
   if(AlibBase->libNode.lib_OpenCnt==0)
   {  if(AlibBase->libNode.lib_Flags&LIBF_DELEXP)
      {
#if defined(__amigaos4__)
       return Expungelib(LIBMAN_NAME);
#elif defined(__amigaos__)
       return Real_Expungelib(AlibBase);
#endif
      }
   }
   return NULL;
     LIBFUNC_EXIT

}

LIBFUNC_H0
(
 __saveds struct SegList *, Expungelib,
  LIBMAN_TYPE, LIBMAN_NAME
)
{
     LIBFUNC_INIT
     return   Real_Expungelib( LIBMAN_NAME);
     LIBFUNC_EXIT
}
static __saveds struct SegList * Real_Expungelib(LIBMAN_TYPE LIBMAN_NAME)
{
#if defined(__amigaos4__)
  struct AWebJSBase *AlibBase = (struct AWebJSBase *)LIBMAN_NAME->Data.LibBase;
#else
  struct AWebJSBase *AlibBase = (struct AWebJSBase *)LIBMAN_NAME;
#endif



   if(AlibBase->libNode.lib_OpenCnt==0)
   {  ULONG size=AlibBase->libNode.lib_NegSize+AlibBase->libNode.lib_PosSize;
      UBYTE *ptr=(UBYTE *)AlibBase-AlibBase->libNode.lib_NegSize;
      Remove((struct Node *)AlibBase);
      Expungeaweblib((struct Library *)AlibBase);

#if defined (__amigaos4__)
      DeleteLibrary(AlibBase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   AlibBase->libNode.lib_Flags|=LIBF_DELEXP;
   return NULL;
}

LIBFUNC_H0
(
 __saveds ULONG , Extfunclib
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  return 0;
     LIBFUNC_EXIT

}


/*-----------------------------------------------------------------------*/



static ULONG Initaweblib(struct Library *libbase)
{
   if(!AwebModuleInit()) return FALSE;

   if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library",39))) return FALSE;
   if(!(LocaleBase = (struct Library *) OpenLibrary("locale.library",0))) return FALSE;
   if(!(UtilityBase=OpenLibrary("utility.library",39))) return FALSE;
   if(!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",39))) return FALSE;
   if(!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library",39))) return FALSE;
   if(!(AslBase=OpenLibrary("asl.library",OSNEED(39,44)))) return FALSE;
#if defined(__amigaos4__)

    if(!(IDOS         = (struct DOSIFace *)GetInterface((struct Library *)DOSBase,"main",1,0))) return FALSE;
    if(!(ILocale      = (struct LocaleIFace *)GetInterface((struct Library *)LocaleBase,"main",1,0))) return FALSE;
    if(!(IUtility     = (struct UtilityIFace *)GetInterface((struct Library *)UtilityBase,"main",1,0))) return FALSE;
    if(!(IIntuition   = (struct IntuitionIFace *)GetInterface((struct Library *)IntuitionBase,"main",1,0))) return FALSE;
    if(!(IGraphics    = (struct GraphicsIFace *)GetInterface((struct Library *)GfxBase,"main",1,0))) return FALSE;
    if(!(IAsl         = (struct AslIFace *)GetInterface((struct Library *)AslBase,"main",1,0))) return FALSE;
#endif

   if(!(locale=OpenLocale(NULL))) return FALSE;

   return TRUE;
}

static void Expungeaweblib(struct Library *libbase)
{
   if(locale) CloseLocale(locale);

#if defined(__amigaos4__)
    if(IAsl) DropInterface((struct Interface *)IAsl);
    if(IGraphics) DropInterface((struct Interface *)IGraphics);
    if(IIntuition) DropInterface((struct Interface *)IIntuition);
    if(IUtility) DropInterface((struct Interface *)IUtility);
    if(ILocale) DropInterface((struct Interface *)ILocale);
    if(IDOS) DropInterface((struct Interface *)IDOS);
#endif

   if(AslBase) CloseLibrary((struct Library *)AslBase);
   if(GfxBase) CloseLibrary((struct Library *)GfxBase);
   if(IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
   if(UtilityBase) CloseLibrary((struct Library *)UtilityBase);
   if(LocaleBase) CloseLibrary((struct Library *)LocaleBase);
   if(DOSBase) CloseLibrary((struct Library *)DOSBase);

   AwebModuleExit();
}

/*-----------------------------------------------------------------------*/

UBYTE *Jdupstr(UBYTE *str,long len,void *pool)
{  UBYTE *dup=NULL;
   if(str)
   {  if(len<0) len=strlen(str);
      if(dup=ALLOCTYPE(UBYTE,len+1,0,pool))
      {  strncpy(dup,str,len);
      }
   }
   return dup;
}

struct Jbuffer *Newjbuffer(void *pool)
{  struct Jbuffer *jb=ALLOCSTRUCT(Jbuffer,1,0,pool);
   if(jb)
   {  jb->pool=pool;
      if((jb->buffer=ALLOCTYPE(UBYTE,1024,0,jb->pool)))
      {
        jb->size = 1024;
        *jb->buffer = (UBYTE)'\0';
      }
   }
   return jb;
}

void Freejbuffer(struct Jbuffer *jb)
{  if(jb)
   {  if(jb->buffer) FREE(jb->buffer);
      FREE(jb);
   }
}

void Addtojbuffer(struct Jbuffer *jb,UBYTE *text,long length)
{  if(length<0) length=strlen(text);
   if(jb->length+length+1>jb->size)
   {  long newsize=((jb->length+length+1024)/1024)*1024;
      UBYTE *newbuf=ALLOCTYPE(UBYTE,newsize,0,jb->pool);
      if(!newbuf) return;
      if(jb->size)
      {  memmove(newbuf,jb->buffer,jb->size);
         FREE(jb->buffer);
      }
      jb->buffer=newbuf;
      jb->size=newsize;
   }
   if(length)
   {  memmove(jb->buffer+jb->length,text,length);
      jb->length+=length;
   }
}

BOOL Calleractive(void)
{  struct IntuitionBase *ibase=(struct IntuitionBase *)IntuitionBase;
   ULONG lock=LockIBase(0);
   BOOL active=FALSE;
   if(ibase->ActiveWindow && ibase->ActiveWindow->UserPort
   && ibase->ActiveWindow->UserPort->mp_SigTask==FindTask(NULL))
   {  active=TRUE;
   }
   UnlockIBase(lock);
   return active;
}

#if defined(__amigaos4__)
#include <exec/emulation.h>
#else
#define IsNative(x) 1
#endif

USRFUNC_H3
(
static long __saveds , Idcmphook,
 struct Hook *, hook, A0,
 APTR, unused, A2,
 struct IntuiMessage *, msg, A1
)
{
     USRFUNC_INIT
  struct Jcontext *jc=hook->h_Data;
   if(msg->Class==IDCMP_CHANGEWINDOW)
   {
       if(IsNative(jc->feedback))
       {
           jc->feedback(jc);
       }
       else
       {
#if defined(__amigaos4__)

         static uint16 stub[] = {0x2f08,  // uint32 stub(REG(a0,void *arg), REG(a1,uint32 (* f)(void *arg)))
                                 0x4e91,  // {
                                 0x588f,  //    return f(arg);
                                 0x4e75}; // }

         EmulateTags(stub,
                     ET_SaveRegs, TRUE,
                     ET_RegisterA0, jc,
                     ET_RegisterA1, jc->feedback,
                     TAG_DONE);

#endif
       }
   }
   return 0;
     USRFUNC_EXIT

}

/* lnr<0 gives general requester. pos<0 gives runtime, pos>=0 gives parsing requester.
 * Returns TRUE when to ignore, FALSE when to stop.
 * But: lnr<0 && pos>0 gives loop warning requester;
 * returns TRUE when to stop, FALSE when to continue. */
BOOL Errorrequester
(
    struct Jcontext *jc, long lnr, UBYTE *line, long pos, UBYTE *msg, VA_LIST args
)
{
     struct Library *WindowBase=NULL,*LayoutBase=NULL,*ButtonBase=NULL,*LabelBase=NULL;
#if defined(__amigaos4__)
     struct LabelIFace *ILabel = NULL;
     struct WindowIFace *IWindow = NULL;
     struct LayoutIFace *ILayout = NULL;
     struct ButtonIFace *IButton = NULL;
#endif
     BOOL ignore=FALSE;
     void *winobj,*buttonrow;
     ULONG sigmask,result;
     UBYTE lnrbuf[32],src[128],buf[128];
     UBYTE *p,*q;
     long len;
     BOOL done=FALSE;
     struct TextAttr ta={ 0 };
     struct Screen *screen;

     if(lnr>=0)
     {
         sprintf(lnrbuf,"Line %d\n",lnr);

       if(pos>=0)
      {  if(line)
         {  for(p=line+pos-1;p>=line && *p!='\n' && *p!='\r';p--);
            pos-=(p+1-line);
            line=p+1;
            for(p=line;*p && *p!='\n' && *p!='\r';p++);
            len=p-line;
            if(pos<=30)
            {  q=line;
               if(len>60) len=60;
            }
            else
            {  if(len>60)
               {  if(len>pos+30)
                  {  q=line+pos-30;
                     pos=30;
                     len=60;
                  }
                  else
                  {  q=line+len-60;
                     pos-=len-60;
                     len=60;
                  }
               }
               else
               {  q=line;
               }
            }
            strncpy(src,q,len);
            p=src+len;
         }
         else
         {  p=src;
         }
         *p++='\n';
         for(;pos;pos--) *p++=' ';
         *p++='^';
         *p++='\0';
      }
      else if(line)
      {  strncpy(src,line,60);
      }
      else
      {  *src='\0';
      }
   }
   else
   {  *lnrbuf='\0';
      *src='\0';
   }
   vsprintf(buf, msg, args);
   ta.ta_Name=((struct GfxBase *)GfxBase)->DefaultFont->tf_Message.mn_Node.ln_Name;
   ta.ta_YSize=((struct GfxBase *)GfxBase)->DefaultFont->tf_YSize;
   if(!(screen=LockPubScreen(jc->screenname)))
   {  screen=LockPubScreen(NULL);
   }
   if(screen
   && (WindowBase=OpenLibrary("window.class",OSNEED(0,44)))
   && (LayoutBase=OpenLibrary("gadgets/layout.gadget",OSNEED(0,44)))
   && (ButtonBase=OpenLibrary("gadgets/button.gadget",OSNEED(0,44)))
   && (LabelBase=OpenLibrary("images/label.image",OSNEED(0,44)))
#if defined(__amigaos4__)
   && (IWindow = (struct WindowIFace *)GetInterface(WindowBase,"main",1,0))
   && (ILayout = (struct LayoutIFace *)GetInterface(LayoutBase,"main",1,0))
   && (IButton = (struct LayoutIFace *)GetInterface(ButtonBase,"main",1,0))
   && (ILabel  = (struct LabelIFace *)GetInterface(LabelBase,"main",1,0))
#endif
   )
   {  winobj=WindowObject,
         WA_Title,(lnr<0 && pos>0)?"JavaScript warning":"JavaScript error",
         WA_DepthGadget,TRUE,
         WA_DragBar,TRUE,
         WA_Activate,Calleractive(),
         WA_AutoAdjust,TRUE,
         WA_SimpleRefresh,TRUE,
         WA_PubScreen,screen,
         WINDOW_Position,WPOS_CENTERSCREEN,
         WINDOW_IDCMPHook,&idcmphook,
         WINDOW_IDCMPHookBits,IDCMP_CHANGEWINDOW,
         WINDOW_Layout,VLayoutObject,
            LAYOUT_DeferLayout,TRUE,
            StartMember,VLayoutObject,
               LAYOUT_SpaceOuter,TRUE,
               StartImage,LabelObject,
                  LABEL_Text,lnrbuf,
               EndImage,
               StartImage,LabelObject,
                  IA_Font,&ta,
                  LABEL_Text,src,
                  LABEL_Underscore,0,
               EndImage,
               StartImage,LabelObject,
                  LABEL_Text,buf,
                  LABEL_Underscore,0,
               EndImage,
            EndMember,
            StartMember,buttonrow=HLayoutObject,
               LAYOUT_BevelStyle,BVS_SBAR_VERT,
               LAYOUT_SpaceOuter,TRUE,
               LAYOUT_EvenSize,TRUE,
               StartMember,ButtonObject,
                  GA_ID,1,
                  GA_RelVerify,TRUE,
                  GA_Text,"_Ok",
               EndMember,
               CHILD_WeightedWidth,0,
               CHILD_NominalSize,TRUE,
            EndMember,
         End,
      EndWindow;
      if(winobj && lnr>=0 && !(jc->dflags&DEBF_DOPEN))
      {  SetAttrs(buttonrow,
            StartMember,ButtonObject,
               GA_ID,2,
               GA_RelVerify,TRUE,
               GA_Text,pos<0?"_Debug":"Ignore _All",
            EndMember,
            CHILD_WeightedWidth,0,
            CHILD_NominalSize,TRUE,
            TAG_END);
      }
      if(winobj && lnr<0 && pos>0)
      {  SetAttrs(buttonrow,
            StartMember,ButtonObject,
               GA_ID,2,
               GA_RelVerify,TRUE,
               GA_Text,"_Stop",
            EndMember,
            CHILD_WeightedWidth,0,
            CHILD_NominalSize,TRUE,
            TAG_END);
      }
      if(winobj && RA_OpenWindow(winobj))
      {  GetAttr(WINDOW_SigMask,winobj,&sigmask);
         while(!done)
         {  Wait(sigmask);
            while(!done && (result=RA_HandleInput(winobj,NULL))!=WMHI_LASTMSG)
            {  switch(result&WMHI_CLASSMASK)
               {  case WMHI_GADGETUP:
                     if(result&WMHI_GADGETMASK)
                     {  done=TRUE;
                        ignore=((result&WMHI_GADGETMASK)==2);
                     }
                     break;
               }
            }
         }
      }
      if(winobj) DisposeObject(winobj);
      /* Allow window refreshing after requester closes */
      jc->feedback(jc);
   }
   if(screen) UnlockPubScreen(NULL,screen);

#if defined(__amigaos4__)
   if(ILabel)DropInterface((struct Interface *)ILabel);
   if(IWindow)DropInterface((struct Interface *)IWindow);
   if(IButton)DropInterface((struct Interface *)IButton);
   if(ILayout)DropInterface((struct Interface *)ILayout);
#endif

   if(LabelBase) CloseLibrary((struct Library *)LabelBase);
   if(ButtonBase) CloseLibrary((struct Library *)ButtonBase);
   if(LayoutBase) CloseLibrary((struct Library *)LayoutBase);
   if(WindowBase) CloseLibrary((struct Library *)WindowBase);
   return ignore;
}

#if !defined(__MORPHOS__)
void timer(long clock[2]);
#endif


BOOL Feedback(struct Jcontext *jc)
{  static long counter=0;
   BOOL run=TRUE;
   if(jc->flags&EXF_STOP) run=FALSE;
   /* Don't get the time etc with _every_ call to avoid excessive overhead */
   if(run && ++counter>60)
   {
      unsigned int clock[2]={ 0,0 };
      counter=0;
      timer((long *)clock);
      if(jc->dflags & DEBF_DEBUG)
      {
          jc->warntime = clock[0] + 60;
      }
      if(run && jc->warntime && clock[0]>jc->warntime)
      {  run=!Errorrequester(jc,-1,NULL,1,"JavaScript runs for one minute",NULL);
         if(run)
         {  timer((long *)clock);
            jc->warntime=clock[0]+60;
         }
      }
      if(run && jc->warnmem && AvailMem(0)<jc->warnmem)
      {  run=!Errorrequester(jc,-1,NULL,1,"JavaScript seems to be eating up all memory",NULL);
         if(run)
         {  jc->warnmem=AvailMem(0)/4;
         }
      }
      if(run && jc->feedback && clock[0]>jc->fbtime)
      {  run=jc->feedback(jc);
         jc->fbtime=clock[0]+1;
      }
   }
   return run;
}

/*-----------------------------------------------------------------------*/

LIBFUNC_H1
(
 __saveds void *, Newjcontext,
 UBYTE *, screenname, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Jcontext *jc=NULL;
   void *pool;
   /* New Jcontext is created in its own pool */
   if(pool=CreatePool(MEMF_ANY|MEMF_CLEAR,PUDDLESIZE,TRESHSIZE))
   {  if(jc=ALLOCSTRUCT(Jcontext,1,0,pool))
      {
         jc->pool=pool;
         jc->objpool=CreatePool(MEMF_ANY|MEMF_CLEAR,PUDDLESIZE,TRESHSIZE);
         jc->varpool=CreatePool(MEMF_ANY|MEMF_CLEAR,PUDDLESIZE,TRESHSIZE);
         NewList((struct List *)&jc->objects);
         Newexecute(jc);
         jc->screenname=screenname;
      }
   }
   if(!jc)
   {
       if(pool) DeletePool(pool);
   }

   return jc;

     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds void , Freejcontext,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc)
   {  if(jc->pool)
      {  Freeexecute(jc);
         if(jc->objpool) DeletePool(jc->objpool);
         if(jc->varpool) DeletePool(jc->varpool);
         DeletePool(jc->pool);
         /* This deletes jc itself too because it was allocated in the pool */
      }
   }

     LIBFUNC_EXIT

}

LIBFUNC_H7
(
 __saveds BOOL , Runjprogram,
 struct Jcontext *, jc, A0,
 struct Jobject *, fscope, A1,
 UBYTE *, source, A2,
 struct Jobject *, jthis, A3,
 struct Jobject **, gwtab, A4,
 ULONG , protkey, D0,
 ULONG , userdata, D1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  BOOL result=TRUE;
   ULONG olduserdata,oldprotkey,olddflags;
   long oldwarnmem;
   struct Value val={0};
   struct Jobject *jo,*jn;
   struct Jobject *oldfscope;
   unsigned int clock[2]={ 0,0 };

   if(jc && source)
   {  idcmphook.h_Entry=(HOOKFUNC)Idcmphook;
      idcmphook.h_Data=jc;
      jc->flags&=~(JCF_ERROR|JCF_IGNORE|EXF_STOP);
      jc->generation++;
      oldfscope=jc->fscope;
      jc->fscope=fscope;
      jc->warntime=0;
      jc->warnmem=0;
      Jcompile(jc,source);
      jc->linenr=0;
      if(!(jc->flags&JCF_ERROR))
      {
         olduserdata=jc->userdata;
         jc->userdata=userdata;
         oldprotkey=jc->protkey;
         jc->protkey=protkey;
         olddflags=jc->dflags;
         jc->dflags&=~DEBF_DOPEN;
         oldwarnmem=jc->warnmem;
         if((jc->dflags&DEBF_DEBUG) && (jc->dflags&DEBF_DBREAK))
         {  Startdebugger(jc);
         }
         if(jc->flags&EXF_WARNINGS)
         {  timer((long *)clock);
            jc->warntime=clock[0]+60;
            jc->warnmem=AvailMem(0)/4;
         }
         Jexecute(jc,jthis,gwtab);
         if(jc->dflags&DEBF_DOPEN)
         {  Stopdebugger(jc);
         }
         jc->dflags=olddflags;
         Asgvalue(&val,jc->val);
         if(val.type==VTP_UNDEFINED) result=TRUE;
         else if(val.type==VTP_OBJECT && !val.value.obj.ovalue) result=TRUE;
         else
         {  Toboolean(&val,jc);
            result=val.value.bvalue;
         }
         Clearvalue(&val);
         jc->warnmem=oldwarnmem;
         jc->userdata=olduserdata;
         jc->protkey=oldprotkey;

         /* "Unkeep" any temporary objects created during the program */
         for(jo=jc->objects.first;jo->next;jo=jn)
         {  jn=jo->next;
            if(jo->flags&OBJF_TEMP)
            {
              Keepobject(jo,FALSE);
            }
         }

      }
      jc->fscope=oldfscope;

   }
   return result;
     LIBFUNC_EXIT

}

void __stdargs _XCEXIT(long x)
{
}

LIBFUNC_H1
(
 __saveds void *, Newjobject,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
   struct Jobject *jo;
   if(jo=Newobject(jc))
   {
       Initconstruct(jc,jo,NULL,NULL);
   }
   return jo;
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds void , Disposejobject,
 struct Jobject *, jo, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Variable *var;
   /* In order to cope with reference loops, all references are cleared before
    * the object is unused.
    * Also, all possible references to external objects are cleared. */
   if(jo)
   {  for(var=jo->properties.first;var->next;var=var->next)
      {  if(var->val.type==VTP_OBJECT)
         {  Clearvalue(&var->val);
         }
         var->hookdata=NULL;
      }
      jo->internal=NULL;
      jo->dispose=NULL;
      jo->keepnr=0;
   }
     LIBFUNC_EXIT

}

LIBFUNC_H5
(
 __saveds struct Jobject *, AddjfunctionA,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2,
 APTR, code, A3,
 UBYTE **, args, A4
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Jobject *f=NULL;
   struct Variable *prop;
   if(name)
   {  if((prop=Getownproperty(jo,name))
      || (prop=Addproperty(jo,name)))
      {  if(f=InternalfunctionA(jc,name,code,args))
         {  if(f->function)
            {  f->function->fscope=jo;
            }
            Asgfunction(&prop->val,f,jo);
         }
      }
   }
   return f;
     LIBFUNC_EXIT

}

#if defined(__amigaos4__)
LIBFUNC_H5
(
 __saveds VARARGS68K struct Jobject *, Addjfunction,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2,
 void (*code)(void *), , A3,
 , ... ,
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
    VA_LIST ap;
    UBYTE **tags;

    VA_STARTLIN(ap, code);
    tags = (UBYTE **)VA_GETLIN(ap, UBYTE *);

    return Self->AddjfunctionA(
        jc,
        jo,
        name,
        code,
        (UBYTE **)tags);

}
#endif

LIBFUNC_H2
(
 __saveds struct Variable *, Jfargument,
 struct Jcontext *, jc, A0,
 long , n, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Jobject *jo;
   struct Variable *var;
   jo=jc->functions.first->arguments;
   var=Arrayelt(jo,n);
   return var;
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds UBYTE *, Jtostring,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  Tostring(&jv->val,jc);
   return jv->val.value.svalue;
     LIBFUNC_EXIT

}

LIBFUNC_H3
(
 __saveds void , Jasgstring,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1,
 UBYTE *, string, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  Asgstring(jv?&jv->val:&jc->functions.first->retval,string?string:(UBYTE *)"",jc->pool);
     LIBFUNC_EXIT

}

LIBFUNC_H3
(
 __saveds void , Jasgobject,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1,
 struct Jobject *, jo, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
     if(jv && jo)
     {
        jo->var = jv;
     }
  Asgobject(jv?&jv->val:&jc->functions.first->retval,jo);
     LIBFUNC_EXIT

}

LIBFUNC_H4
(
 __saveds void , Setjobject,
 struct Jobject *, jo, A0,
 Objhookfunc *, hook, A1,
 void *, internal, A2,
 Objdisposehookfunc *, dispose , A3
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  jo->hook=hook;
   jo->internal=internal;
   jo->dispose=dispose;
     LIBFUNC_EXIT

}

LIBFUNC_H3
(
 __saveds struct Variable *, Jproperty,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Variable *var=NULL;
   if(!name) name="";
   if(!(var=Getownproperty(jo,name)))
   {  var=Addproperty(jo,name);
   }
   return var;
     LIBFUNC_EXIT

}

LIBFUNC_H3
(
 __saveds void , Setjproperty,
 struct Variable *, jv, A0,
 Varhookfunc *, hook, A1,
 void *, hookdata, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(hook==(Varhookfunc *)-1)
   {  jv->hook=Constantvhook;
   }
   else if(hook==(Varhookfunc *)-2)
   {  jv->flags|=VARF_SYNONYM;
      jv->hookdata=hookdata;
   }
   else
   {  jv->hook=hook;
      jv->hookdata=hookdata;
   }
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds struct Jobject *, Jthis,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  return jc?jc->jthis:NULL;
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds void *, Jointernal,
 struct Jobject *, jo, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  return jo?jo->internal:NULL;
     LIBFUNC_EXIT

}

LIBFUNC_H3
(
 __saveds void , Jasgboolean,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1,
 BOOL , bvalue, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  Asgboolean(jv?&jv->val:&jc->functions.first->retval,bvalue);
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds BOOL , Jtoboolean,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  Toboolean(&jv->val,jc);
   return jv->val.value.bvalue;
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds struct Jobject *, Newjarray,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  return oldNewarray(jc);
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds struct Variable *, Jnewarrayelt,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  return Addarrayelt(jc,jo);
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds struct Jobject *, Jtoobject,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  Toobject(&jv->val,jc);
   return jv->val.value.obj.ovalue;
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds long , Jtonumber,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  long n;
   Tonumber(&jv->val,jc);
   if(jv->val.attr==VNA_VALID) n=(long)jv->val.value.nvalue;
   else n=0;
   return n;
     LIBFUNC_EXIT

}

LIBFUNC_H3
(
 __saveds void , Jasgnumber,
 struct Jcontext *, jc, A0,
 struct Variable *, jv, A1,
 long , nvalue, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  Asgnumber(jv?&jv->val:&jc->functions.first->retval,VNA_VALID,(double)nvalue);
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds BOOL , Jisarray,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  return Isarray(jo);
     LIBFUNC_EXIT

}

LIBFUNC_H3
(
 __saveds struct Jobject *, Jfindarray,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Variable *var;
   struct Jobject *ja=NULL;
   if(var=Getownproperty(jo,name))
   {  if(var->val.type==VTP_OBJECT && var->val.value.obj.ovalue && Isarray(var->val.value.obj.ovalue))
      {  ja=var->val.value.obj.ovalue;
      }
   }
   return ja;
     LIBFUNC_EXIT

}

LIBFUNC_H3
(
 __saveds void , Jsetprototype,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 struct Jobject *, proto, A2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Variable *var;
   if((var=Getownproperty(jo,"prototype"))
   || (var=Addproperty(jo,"prototype")))
   {  if(!proto->constructor) proto->constructor=jo;
      proto->hook=Prototypeohook;
      Asgobject(&var->val,proto);
   }
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds ULONG , Jgetuserdata,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc)
   {  return jc->userdata;
   }
   else
   {  return 0;
   }
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds BOOL , Jisnumber,
 struct Variable *, jv, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  return (BOOL)(jv && jv->val.type==VTP_NUMBER && jv->val.attr==VNA_VALID);
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Clearjobject,
 struct Jobject *, jo, A0,
 UBYTE **, except, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jo)
   {  Clearobject(jo,except);
   }
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds void , Freejobject,
 struct Jobject *, jo, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT

     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds void , Jdumpobjects,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc)
   {  Dumpobjects(jc);
   }
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds struct Variable *, Jgetreturnvalue,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Variable *var;
   if(jc && jc->result->val.type) var=jc->result;
   else var=NULL;
   return var;
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Jpprotect,
 struct Variable *, var, A0,
 ULONG , protkey, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(var)
   {  var->protkey=protkey;
   }
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Jcprotect,
 struct Jcontext *, jc, A0,
 ULONG , protkey, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc)
   {  jc->protkey=protkey;
   }
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds UBYTE *, Jpname,
 struct Variable *, var, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(var) return var->name;
   else return NULL;
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds Objdisposehookfunc*, Jdisposehook,
 struct Jobject *, jo, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  void *hook=NULL;
   if(jo) hook=(void *)jo->dispose;
   return hook;
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Jsetfeedback,
 struct Jcontext *, jc, A0,
 Jfeedback *, jf, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc)
   {  jc->feedback=jf;
   }
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Jdebug,
 struct Jcontext *, jc, A0,
 BOOL , debugon, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc)
   {  if(debugon)
      {  jc->dflags|=DEBF_DEBUG|DEBF_DBREAK;
      }
      else
      {  jc->dflags&=~DEBF_DEBUG|DEBF_DBREAK;
      }
   }
     LIBFUNC_EXIT

}

LIBFUNC_H4
(
 __saveds void , Jerrors,
 struct Jcontext *, jc, A0,
 BOOL , comperrors, D0,
 long , runerrors, D1,
 BOOL , watch, D2
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc)
   {  if(comperrors)
      {  jc->flags|=JCF_ERRORS;
      }
      else
      {  jc->flags&=~JCF_ERRORS;
      }
      jc->flags&=~(EXF_ERRORS|EXF_DONTSTOP);
      switch(runerrors)
      {  case JERRORS_CONTINUE:
            jc->flags|=EXF_DONTSTOP;
            break;
         case JERRORS_OFF:
            break;
         default:
            jc->flags|=EXF_ERRORS;
            break;
      }
      if(watch)
      {  jc->flags|=EXF_WARNINGS;
      }
      else
      {  jc->flags&=~EXF_WARNINGS;
      }
   }
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Jkeepobject,
 struct Jobject *, jo, A0,
 BOOL , used, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jo) Keepobject(jo,used);
     LIBFUNC_EXIT

}

LIBFUNC_H1
(
 __saveds void , Jgarbagecollect,
 struct Jcontext *, jc, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc) Garbagecollect(jc);
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Jsetlinenumber,
 struct Jcontext *, jc, A0,
 long , linenr, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc) jc->linenr=linenr;
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Jsetobjasfunc,
 struct Jobject *, jo, A0,
 BOOL , asfunc, D0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jo)
   {  if(asfunc)
      {  jo->flags|=OBJF_ASFUNCTION;
      }
      else
      {  jo->flags&=~OBJF_ASFUNCTION;
      }
   }
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Jsetscreen,
 struct Jcontext *, jc, A0,
 UBYTE *, screenname, A1
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  if(jc)
   {  jc->screenname=screenname;
   }
     LIBFUNC_EXIT

}

LIBFUNC_H4
(
 __saveds void , Jaddeventhandler,
 struct Jcontext *, jc, A0,
 struct Jobject *, jo, A1,
 UBYTE *, name, A2,
 UBYTE *, source, A3
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Variable *var;
   struct Jobject *jeventh;
   if(jc && jo && !Getownproperty(jo,name))
   {  if(var=Addproperty(jo,name))
      {  if(source)
         {  jc->generation++;
            if(jeventh=Jcompiletofunction(jc,source,name))
            {  Asgfunction(&var->val,jeventh,jo);
            }
            else
            {  Asgstring(&var->val,source,jc->pool);
            }
         }
         else
         {  Asgobject(&var->val,NULL);
         }
      }
   }
     LIBFUNC_EXIT

}

LIBFUNC_H3
(
 __saveds struct Variable *, Jaddproperty,
 struct Jcontext  *,jc, A0,
 struct Jobject   *,jo, A1,
 UBYTE *, name, A2,
 JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
    LIBFUNC_INIT
    struct Variable * var = NULL;
    if (!name) name = "";
    var = Addproperty(jo,name);
    return var;
    LIBFUNC_EXIT
}

LIBFUNC_H1
(
 __saveds struct Jobject *, Newjscope,
 struct Jcontext *,jc, A0,
 JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
    LIBFUNC_INIT
    return Newscope(jc);
    LIBFUNC_EXIT
}

LIBFUNC_H1
(
 __saveds void , Disposejscope,
 struct Jobject *, jo, A0
 , JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
     LIBFUNC_INIT
  struct Variable *var;
   /* In order to cope with reference loops, all references are cleared before
    * the object is unused.
    * Also, all possible references to external objects are cleared. */
   if(jo)
   {  for(var=jo->properties.first;var->next;var=var->next)
      {  if(var->val.type==VTP_OBJECT)
         {  Clearvalue(&var->val);
         }
         var->hookdata=NULL;
      }
      jo->internal=NULL;
      jo->dispose=NULL;
      jo->keepnr=0;
   }
     LIBFUNC_EXIT

}

LIBFUNC_H2
(
 __saveds void , Jallowgc,
 struct Jcontext *, jc, A0,
 BOOL, allow, D0,
 JAVASCRIPT_TYPE, JAVASCRIPT_NAME
)
{
    LIBFUNC_INIT
    if(jc)
    {
        if(allow)
        {
            jc->nogc--;
            //adebug("allowing gc\n");
        }
        else
        {
            jc->nogc++;
            //adebug("forbiding gc\n");

        }
    }
    LIBFUNC_EXIT
}
