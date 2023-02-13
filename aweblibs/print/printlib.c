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

/* printlib.c - AWeb print magic and debug aweblib module */

#include "aweblib.h"
#include "print.h"
#include "libraries/awebmodule.h"
#include <exec/resident.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>

struct Library *AwebSupportBase;
struct DosLibrary *DOSBase;
struct Library *PrintBase;

#ifdef __MORPHOS__
struct Library *UtilityBase;
#else
struct UtilityBase *UtilityBase;
#endif

#if defined(__amigaos4__)
struct AwebSupportIFace *IAwebSupport;
struct DOSIFace * IDOS;
struct UtilityIFace *IUtility;
#endif


#ifdef __MORPHOS__
ULONG __abox__=1;
#endif


#define PRINT_VERSION 37
#define PRINT_REVISION 0
#define PRINT_VERSTRING "37.0 " CPU


/*-----------------------------------------------------------------------*/
/* AWebLib module startup */

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

__saveds struct Seglist *Real_Closelib(LIBMAN_TYPE LIBMAN_NAME);

LIBFUNC_P0
(
 __saveds struct SegList * , Closelib,
 LIBMAN_TYPE, LIBMAN_NAME
);

static __saveds struct SegList * Real_Expungelib(LIBMAN_TYPE LIBMAN_NAME);
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
__saveds void  , doPrintdebugprefs,
struct Print *,prt,A0,
PRINT_TYPE, PRINT_NAME
);

LIBFUNC_P1
(
__saveds void  , doPrintfinddimensions,
struct Print *,prt,A0,
PRINT_TYPE, PRINT_NAME

);

LIBFUNC_P1
(
__saveds void  , doPrintprintsection,
struct Print *,prt,A0,
PRINT_TYPE, PRINT_NAME
);

LIBFUNC_P1
(
__saveds void  , doPrintclosedebug,
struct Print *,prt,A0,
PRINT_TYPE, PRINT_NAME
);


/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct Library *AwebStartupBase;

static APTR libseglist;

struct ExecBase *SysBase;

static char __aligned libname[]="print.aweblib";
static char __aligned libid[]="print.aweblib " PRINT_VERSTRING " " __AMIGADATE__;


LIBSTART_DUMMY

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

ULONG _Print_Obtain(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount++;

}

static ULONG _Print_Release(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount--;
}

static void *main_vectors[] = {
        (void *)_Print_Obtain,
        (void *)_Print_Release,
        (void *)NULL,
        (void *)NULL,
        (void *)doPrintdebugprefs,
        (void *)doPrintfinddimensions,
        (void *)doPrintprintsection,
        (void *)doPrintclosedebug,
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
    {CLT_DataSize,         (uint32)(sizeof(struct Library))},
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
    PRINT_VERSION,
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
   (APTR)FUNCARRAY_32BIT_NATIVE,
#endif
         Openlib,
   Closelib,
   Expungelib,
   Extfunclib,
   doPrintdebugprefs,
   doPrintfinddimensions,
   doPrintprintsection,
   doPrintclosedebug,
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
   PRINT_VERSION,
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


    libBase->lib_Node.ln_Type = NT_LIBRARY;
    libBase->lib_Node.ln_Pri  = 0;
    libBase->lib_Node.ln_Name = libname;
    libBase->lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
    libBase->lib_Version      = PRINT_VERSION;
    libBase->lib_Revision     = PRINT_REVISION;
    libBase->lib_IdString     = libid;

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
__saveds struct Library * , Initlib,
struct Library *,libbase,D0,
struct SegList *,seglist,A0,
struct ExecBase *,sysbase,A6
)
{
    USRFUNC_INIT
  SysBase=sysbase;
   PrintBase=libbase;
   libbase->lib_Revision=PRINT_REVISION;
   libseglist=seglist;
   if(!Initaweblib(libbase))
   {  Expungeaweblib(libbase);
      libbase=NULL;
   }
   return libbase;

    USRFUNC_EXIT
}

#endif

/*----------------------------------------------------------------------*/

LIBFUNC_H0
(
__saveds struct Library * , Openlib,
LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT

#if defined(__amigaos4__)
   struct Library *Printlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Printlibbase = (struct Library *)LIBMAN_NAME;
#endif

   Printlibbase->lib_OpenCnt++;
   Printlibbase->lib_Flags&=~LIBF_DELEXP;
   if(Printlibbase->lib_OpenCnt==1)
   {
      if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return Real_Closelib(LIBMAN_NAME);
#if defined (__amigaos4__)
      if(!( IAwebSupport = (struct AwebSupportIface *)GetInterface(AwebSupportBase,"main",1,0))) return Real_Closelib(LIBMAN_NAME);
#endif
   }
   return Printlibbase;

    LIBFUNC_EXIT
}





LIBFUNC_H0
(
__saveds struct SegList * , Closelib,
LIBMAN_TYPE, LIBMAN_NAME
)
{
    return Real_Closelib(LIBMAN_NAME);
}

__saveds struct Seglist *Real_Closelib(LIBMAN_TYPE LIBMAN_NAME)
{
    LIBFUNC_INIT

#if defined(__amigaos4__)
   struct Library *Printlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Printlibbase = (struct Library *)LIBMAN_NAME;
#endif


  Printlibbase->lib_OpenCnt--;
   if(Printlibbase->lib_OpenCnt==0)
   {  if(AwebSupportBase)
      {
#if defined(__amigaos4__)
    if(IAwebSupport) DropInterface((struct Interface *)IAwebSupport);
    IAwebSupport = NULL;
#endif

         CloseLibrary(AwebSupportBase);
         AwebSupportBase=NULL;
      }
      if(Printlibbase->lib_Flags&LIBF_DELEXP)
      {  return Real_Expungelib(LIBMAN_NAME);
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
        return Real_Expungelib(LIBMAN_NAME);
        LIBFUNC_EXIT
}

static __saveds struct SegList * Real_Expungelib(LIBMAN_TYPE LIBMAN_NAME)
{
#if defined(__amigaos4__)
   struct Library *Printlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Printlibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Printlibbase->lib_OpenCnt==0)
   {  ULONG size=Printlibbase->lib_NegSize+Printlibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Printlibbase-Printlibbase->lib_NegSize;
      Remove((struct Node *)Printlibbase);
      Expungeaweblib(Printlibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Printlibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Printlibbase->lib_Flags|=LIBF_DELEXP;
   return NULL;

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


/*-----------------------------------------------------------------------*/



static ULONG Initaweblib(struct Library *libbase)
{
   if(!AwebModuleInit()) return FALSE;
   if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library",39))) return FALSE;
   if(!(UtilityBase=OpenLibrary("utility.library",39))) return FALSE;

#if defined(__amigaos4__)
   if(!(IDOS = (struct DOSIFace *)GetInterface(DOSBase,"main",1,0))) return FALSE;
   if(!(IUtility = (struct IUtilityBase *)GetInterface(UtilityBase,"main",1,0))) return FALSE;
#endif

   return TRUE;
}

static void Expungeaweblib(struct Library *libbase)
{
#if defined(__amigaos4__)
    if(IDOS)DropInterface((struct Interface *)IDOS);
    if(IUtility)DropInterface((struct Interface *)IUtility);
#endif

   if(UtilityBase) CloseLibrary(UtilityBase);
   if(DOSBase) CloseLibrary(DOSBase);
   AwebModuleExit();
}

/*-----------------------------------------------------------------------*/

VARARGS68K_DECLARE(static void Prtdebug(long fh,char *str,...))
{
    VA_LIST va;
    long *args;
    VA_STARTLIN(va,str);
    args = (long *)VA_GETLIN(va, long *);
    VFPrintf(fh,str,(long *)args);
    VA_END(va);
}

LIBFUNC_H1
(
__saveds void  , doPrintdebugprefs,
struct Print *,prt,A0,
PRINT_TYPE, PRINT_NAME
)
{
    LIBFUNC_INIT
  if(prt->flags&PRTF_DEBUG)
   {  prt->debugfile=Open("T:AWebPrintDebug.txt",MODE_NEWFILE);
      if(prt->debugfile)
      {  long f=prt->debugfile;
         struct PrinterData *pd=(struct PrinterData *)prt->ioreq->io_Device;
         struct PrinterExtendedData *ped=(struct PrinterExtendedData *)
            &pd->pd_SegmentData->ps_PED;
         struct Preferences *pr=&pd->pd_Preferences;
         Prtdebug(f,"\n--- Start\n");
         Prtdebug(f,"\nPrinter:\n");
         Prtdebug(f,"Device=%s %ld.%ld (%s)\n",pd->pd_Device.dd_Device.lib_Node.ln_Name,
            pd->pd_Device.dd_Device.lib_Version,pd->pd_Device.dd_Device.lib_Revision,pd->pd_Device.dd_Device.lib_IdString);
         Prtdebug(f,"Driver=%s\n",pd->pd_DriverName);
         Prtdebug(f,"Printer=%s\n",ped->ped_PrinterName);
         Prtdebug(f,"NumRows=%ld\n",ped->ped_NumRows);
         Prtdebug(f,"MaxXDots=%ld\n",ped->ped_MaxXDots);
         Prtdebug(f,"MaxYDots=%ld\n",ped->ped_MaxYDots);
         Prtdebug(f,"XDotsInch=%ld\n",ped->ped_XDotsInch);
         Prtdebug(f,"YDotsInch=%ld\n",ped->ped_XDotsInch);
         Prtdebug(f,"\nPreferences:\n");
         Prtdebug(f,"PaperSize=0x%04lX\n",pr->PaperSize);
         Prtdebug(f,"PaperLength=%ld\n",pr->PaperLength);
         Prtdebug(f,"PaperType=0x%04lX\n",pr->PaperType);
         Prtdebug(f,"PrintFlags=0x%04lX\n",pr->PrintFlags);
         Prtdebug(f,"PrintMaxWidth=%ld\n",pr->PrintMaxWidth);
         Prtdebug(f,"PrintMaxHeight=%ld\n",pr->PrintMaxHeight);
         Prtdebug(f,"PrintDensity=%ld\n",pr->PrintDensity);
         Prtdebug(f,"PrintXOffset=%ld\n",pr->PrintXOffset);
      }
   }

    LIBFUNC_EXIT
}

/* Find rastport height so that printer rows is muptiple of printhead's rows.
 * Find largest height that match this condition.
 * Also, if printer has MaxYDots, use the height that causes the smallest number
 * of rows unprinted. If more heights result in equal remainder, use largest.
 */

static long Getdimensions(struct Print *prt,long start,long max)
{  long f=prt->debugfile;
   struct PrinterData *pd=(struct PrinterData *)prt->ioreq->io_Device;
   struct PrinterExtendedData *ped=(struct PrinterExtendedData *)&pd->pd_SegmentData->ps_PED;
   ULONG numrows=ped->ped_NumRows;
   long height,goodheight=0,minremainder,maxydots,rem;
   /* maxydots is the number of dots corresponding to the preferred page height */
   if(pd->pd_Preferences.PrintFlags&(BOUNDED_DIMENSIONS|ABSOLUTE_DIMENSIONS))
   {  maxydots=pd->pd_Preferences.PrintMaxHeight*ped->ped_YDotsInch/10;
      if(f) Prtdebug(f,"use dimensions; maxydots=%ld * %ld / 10 = %ld\n",
         pd->pd_Preferences.PrintMaxHeight,ped->ped_YDotsInch,maxydots);
   }
   else
   {  maxydots=ped->ped_MaxYDots;
      if(f) Prtdebug(f,"use default size; maxydots=%ld\n",maxydots);
   }
   if(!numrows) numrows=1;
   if(f) Prtdebug(f,"Use numrows=%ld\n",numrows);
   minremainder=maxydots;
   prt->ioreq->io_Command=PRD_DUMPRPORT;
   prt->ioreq->io_RastPort=prt->rp;
   prt->ioreq->io_ColorMap=prt->cmap;
   prt->ioreq->io_Modes=prt->screenmode;
   prt->ioreq->io_SrcX=0;
   prt->ioreq->io_SrcY=1;
   prt->ioreq->io_SrcWidth=prt->printwidth;
   for(height=max;height>=start;height--)
   {  prt->ioreq->io_SrcHeight=height-2;
      prt->ioreq->io_DestCols=(ULONG)0xffffffff/100*prt->scale;
      prt->ioreq->io_DestRows=(ULONG)0xffffffff/100*prt->scale;
      prt->ioreq->io_Special=SPECIAL_FRACCOLS|SPECIAL_FRACROWS|SPECIAL_ASPECT|SPECIAL_NOPRINT;
      DoIO(prt->ioreq);
      if(f) Prtdebug(f,"height=%ld -> DestRows=%ld\n",height,prt->ioreq->io_DestRows);
      if(maxydots && prt->ioreq->io_DestRows>maxydots) break;
      /* DestRows must be multiple of numrows, and not greater than what can maximally
       * be dumped to the printer. */
      if(prt->ioreq->io_DestRows>0 && prt->ioreq->io_DestRows%numrows==0
      && (!ped->ped_MaxYDots || prt->ioreq->io_DestRows<=ped->ped_MaxYDots))
      {  if(maxydots)
         {  /* DestRows must fit in MaxYDots with minimum remainder */
            rem=maxydots%prt->ioreq->io_DestRows;
            if(rem<minremainder)
            {  goodheight=height;
               minremainder=rem;
               prt->numstrips=maxydots/prt->ioreq->io_DestRows;
               if(f) Prtdebug(f,"  good height! remainder=%ld numstrips=%ld\n",
                  rem,prt->numstrips);
            }
            else if(f) Prtdebug(f,"  height fits but larger remainder %ld\n",rem);
         }
         else
         {  /* Use maximum height */
            goodheight=height;
            if(f) Prtdebug(f,"  good height for endless paper\n");
            break;
         }
      }
   }
   return goodheight;
}


LIBFUNC_H1
(
__saveds void  , doPrintfinddimensions,
struct Print *,prt,A0,
PRINT_TYPE, PRINT_NAME
)
{
    LIBFUNC_INIT
  long f=prt->debugfile;
   short d;
   if(prt->scale<=50) d=1;
   else d=-1;
   prt->printheight=prt->numstrips=0;
   if(f) Prtdebug(f,"\n--- Calculate\n");
   while(prt->scale>0 && prt->scale<=100)
   {  if(f) Prtdebug(f,"\nScale=%ld\n",prt->scale);
      prt->printheight=Getdimensions(prt,8,PRINTWINH);
      if(prt->printheight) break;
      prt->scale+=d;
   }
   if(f)
   {  Prtdebug(f,"\nFound printheight=%ld\n",prt->printheight);
      Prtdebug(f,"\n--- Print\n");
   }

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
__saveds void  , doPrintprintsection,
struct Print *,prt,A0,
PRINT_TYPE, PRINT_NAME

)
{
    LIBFUNC_INIT
  long f=prt->debugfile;
   BOOL last=(prt->top+prt->printheight-2>=prt->totalheight);
   if(f) Prtdebug(f,"Section from line %ld\n",prt->top);
   prt->ioreq->io_Command=PRD_DUMPRPORT;
   prt->ioreq->io_RastPort=prt->rp;
   prt->ioreq->io_ColorMap=prt->cmap;
   prt->ioreq->io_Modes=prt->screenmode;
   prt->ioreq->io_SrcX=0;
   prt->ioreq->io_SrcY=1;
   prt->ioreq->io_SrcWidth=prt->printwidth;
   prt->ioreq->io_SrcHeight=last?(prt->totalheight-prt->top):prt->printheight-2;
   prt->ioreq->io_DestCols=(ULONG)0xffffffff/100*prt->scale;
   prt->ioreq->io_DestRows=(ULONG)0xffffffff/100*prt->scale;
   prt->ioreq->io_Special=SPECIAL_FRACCOLS|SPECIAL_FRACROWS|SPECIAL_ASPECT|SPECIAL_TRUSTME;
   if(prt->flags&PRTF_CENTER) prt->ioreq->io_Special|=SPECIAL_CENTER;
   if(last)
   {  if(!(prt->flags&PRTF_FORMFEED)) prt->ioreq->io_Special|=SPECIAL_NOFORMFEED;
      if(f) Prtdebug(f,"  Last section");
   }
   else
   {  if(f && prt->numstrips)
      {  Prtdebug(f,"  printing %ld of %ld per page",prt->numprinted+1,prt->numstrips);
      }
      if(!prt->numstrips || ++prt->numprinted<prt->numstrips)
      {  prt->ioreq->io_Special|=SPECIAL_NOFORMFEED;
      }
      else prt->numprinted=0;
   }
   if(f)
   {  if(prt->ioreq->io_Special&SPECIAL_NOFORMFEED) Prtdebug(f,"\n");
      else Prtdebug(f," FORM FEED\n");
   }
   SendIO(prt->ioreq);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
__saveds void  , doPrintclosedebug,
struct Print *,prt,A0,
PRINT_TYPE, PRINT_NAME

)
{
    LIBFUNC_INIT
  if(prt->debugfile)
   {  Prtdebug(prt->debugfile,"\n--- End\n");
      Close(prt->debugfile);
      prt->debugfile=NULL;
   }

    LIBFUNC_EXIT
}
