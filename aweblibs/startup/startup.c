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

/* startup.c - AWeb startup window AWebLib module */

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include "aweblib.h"
#include "application.h"

#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"

#include <reaction/reaction.h>
#include <exec/resident.h>
#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <proto/intuition.h>

#include <proto/awebstartup.h>

#define STARTUP_VERSION 36
#define STARTUP_REVISION 0
#define STARTUP_VERSTRING "36.0 " CPU

#ifdef __MORPHOS__
/*
 * To tell the loader that this is a new abox elf and not
 * one for the ppc.library.
 * ** IMPORTANT **
 */
ULONG   __abox__=1;

#endif


struct SegList;

struct Library  *AwebSupportBase;
struct DosLibrary *DOSBase;
struct IntuitionBase *IntuitionBase;
void *UtilityBase;
struct Library *WindowBase,*LayoutBase,*ButtonBase,
   *LabelBase,*PenMapBase,*FuelGaugeBase;

#if defined(__amigaos4__)

struct AwebSupportIFace *IAwebSupport;
struct DOSIFace *IDOS;
struct IntuitionIFace *IIntuition;
struct UtilityIFace *IUtility;
struct WindowIFace *IWindow;
struct LayoutIFace *ILayout;
struct ButtonIFace *IButton;
struct PenMapIFace *IPenMap;
struct FuelGaugeIFace *IFuelGauge;
struct LabelIFace *ILabel;

#endif


/*-----------------------------------------------------------------------*/
/* AWebLib module startup */

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

static __saveds struct SegList *Real_Closelib(LIBMAN_TYPE LIBMAN_NAME);
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

LIBFUNC_P4
(
__saveds void  , doStartupopen,
struct Screen *,screen,A0,
UBYTE *,version,A1,
ULONG *,imagepalette,A2,
UBYTE *,imagedata,A3,
STARTUP_TYPE, STARTUP_NAME
);

LIBFUNC_P1
(
__saveds void  , doStartupstate,
ULONG ,state,D0,
STARTUP_TYPE,STARTUP_NAME
);

LIBFUNC_P2
(
__saveds void  , doStartuplevel,
long ,ready,D0,
long ,total,D1,
STARTUP_TYPE,STARTUP_NAME
);

LIBFUNC_P0
(
__saveds void  , doStartupclose,
STARTUP_TYPE,STARTUP_NAME
);

/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct Library *StartupBase;

static APTR libseglist;

struct ExecBase *SysBase;

LIBSTART_DUMMY

static char __aligned libname[]="startup.aweblib";
static char __aligned libid[]="startup.aweblib " STARTUP_VERSTRING " " __AMIGADATE__;

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

ULONG _Cabr_Obtain(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount++;

}

static ULONG _Cabr_Release(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount--;
}

static void *main_vectors[] = {
        (void *)_Cabr_Obtain,
        (void *)_Cabr_Release,
        (void *)NULL,
        (void *)NULL,
        (void *)doStartupopen,
        (void *)doStartupstate,
        (void *)doStartuplevel,
        (void *)doStartupclose,
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
    STARTUP_VERSION,
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
   doStartupopen,
   doStartupstate,
   doStartuplevel,
   doStartupclose,
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
   STARTUP_VERSION,
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
    libBase->lib_Version      = STARTUP_VERSION;
    libBase->lib_Revision     = STARTUP_REVISION;
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
   StartupBase=libbase;
   libbase->lib_Revision=STARTUP_REVISION;
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
   struct Library *Startuplibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Startuplibbase = (struct Library *)LIBMAN_NAME;
#endif

   Startuplibbase->lib_OpenCnt++;
   Startuplibbase->lib_Flags&=~LIBF_DELEXP;
   if(Startuplibbase->lib_OpenCnt==1)
   {
      if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return Real_Closelib(LIBMAN_NAME);
#if defined (__amigaos4__)
      if(!( IAwebSupport = (struct AwebSupportIface *)GetInterface(AwebSupportBase,"main",1,0))) return Real_Closelib(LIBMAN_NAME);
#endif
   }
   if(!Fullversion())
   {
      Real_Closelib(Startuplibbase);
      return NULL;
   }
   return Startuplibbase;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds struct SegList * , Closelib,
LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT
                return Real_Closelib(LIBMAN_NAME);
                LIBFUNC_EXIT
}

static __saveds struct SegList *Real_Closelib(LIBMAN_TYPE LIBMAN_NAME)
{
#if defined(__amigaos4__)
   struct Library *Startuplibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Startuplibbase = (struct Library *)LIBMAN_NAME;
#endif


  Startuplibbase->lib_OpenCnt--;
   if(Startuplibbase->lib_OpenCnt==0)
   {  if(AwebSupportBase)
      {
#if defined(__amigaos4__)
    if(IAwebSupport) DropInterface((struct Interface *)IAwebSupport);
    IAwebSupport = NULL;
#endif

         CloseLibrary(AwebSupportBase);
         AwebSupportBase=NULL;
      }
      if(Startuplibbase->lib_Flags&LIBF_DELEXP)
      {  return Real_Expungelib(LIBMAN_NAME);
      }
   }
   return NULL;
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
   struct Library *Startuplibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Startuplibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Startuplibbase->lib_OpenCnt==0)
   {  ULONG size=Startuplibbase->lib_NegSize+Startuplibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Startuplibbase-Startuplibbase->lib_NegSize;
      Remove((struct Node *)Startuplibbase);
      Expungeaweblib(Startuplibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Startuplibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Startuplibbase->lib_Flags|=LIBF_DELEXP;
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

   if(!(IntuitionBase=(struct IntuitionBase *) OpenLibrary("intuition.library",39))) return FALSE;
   if(!(UtilityBase=OpenLibrary("utility.library",39))) return FALSE;
   if(!(WindowBase=OpenLibrary("window.class",OSNEED(0,44)))) return FALSE;
   if(!(LayoutBase=OpenLibrary("gadgets/layout.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ButtonBase=OpenLibrary("gadgets/button.gadget",OSNEED(0,44)))) return FALSE;
   if(!(FuelGaugeBase=OpenLibrary("gadgets/fuelgauge.gadget",OSNEED(0,44)))) return FALSE;
   if(!(PenMapBase=OpenLibrary("images/penmap.image",OSNEED(0,44)))) return FALSE;
   if(!(LabelBase=OpenLibrary("images/label.image",OSNEED(0,44)))) return FALSE;

#if defined(__amigaos4__)
   if(!(IIntuition = (struct IntuitionIFacee *) GetInterface(IntuitionBase,"main",1,0))) return FALSE;
   if(!(IUtility   = (struct UtilityIFace *)    GetInterface(UtilityBase,"main",1,0))) return FALSE;
   if(!(IWindow = (struct WindowIFace *)GetInterface(WindowBase,"main",1,0))) return FALSE;
   if(!(ILayout = (struct LayoutIFace *)GetInterface(LayoutBase,"main",1,0))) return FALSE;
   if(!(IButton = (struct ButtonIFace *)GetInterface(ButtonBase,"main",1,0))) return FALSE;
   if(!(IPenMap = (struct PenMapIFace *)GetInterface(PenMapBase,"main",1,0))) return FALSE;
   if(!(IFuelGauge = (struct FuelGaugeIFace *)GetInterface(FuelGaugeBase,"main",1,0))) return FALSE;
   if(!(ILabel = (struct LabrelIFace *)GetInterface(LabelBase,"main",1,0))) return FALSE;

#endif

   return TRUE;
}

static void Expungeaweblib(struct Library *libbase)
{
#if defined(__amigaos4__)

   if(ILabel)DropInterface((struct Interface *)ILabel);
   if(IPenMap)DropInterface((struct Interface *)IPenMap);
   if(IFuelGauge)DropInterface((struct Interface *)IFuelGauge);
   if(IButton)DropInterface((struct Interface *)IButton);
   if(ILayout)DropInterface((struct Interface *)ILayout);
   if(IWindow)DropInterface((struct Interface *)IWindow);
   if(IUtility)DropInterface((struct Interface *)IUtility);
   if(IIntuition)DropInterface((struct Interface *)IIntuition);

#endif

   if(LabelBase) CloseLibrary(LabelBase);
   if(PenMapBase) CloseLibrary(PenMapBase);
   if(FuelGaugeBase) CloseLibrary(FuelGaugeBase);
   if(ButtonBase) CloseLibrary(ButtonBase);
   if(LayoutBase) CloseLibrary(LayoutBase);
   if(WindowBase) CloseLibrary(WindowBase);
   if(UtilityBase) CloseLibrary(UtilityBase);
   if(IntuitionBase) CloseLibrary(IntuitionBase);
   AwebModuleExit();
}

/*-----------------------------------------------------------------------*/




static void *loadreqobj;
static struct Window *loadreqwin;
static struct Gadget *loadreqmsg,*loadreqfg;
static long loadreqmin,loadreqmax;
static struct TextAttr loadreqfont={0};

/*-----------------------------------------------------------------------*/


VARARGS68K_DECLARE(static void  Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...))
{
   VA_LIST va;
   struct TagItem *tags;
   VA_STARTLIN(va,req);
   tags = (struct TagItem *)VA_GETLIN(va,struct TagItem *);
   if(SetGadgetAttrsA(gad,win,req,tags) && win) RefreshGList(gad,win,req,1);
   VA_END(va);
}


DECLARE_HOOK
(
__saveds void  , Nobackfillhook,
struct Hook *,unused1,A0,
struct Node *,unused2,A2,
Msg *,msg,A1
)
{
    USRFUNC_INIT


    USRFUNC_EXIT
}

LIBFUNC_H4
(
__saveds void  , doStartupopen,
struct Screen *,screen,A0,
UBYTE *,version,A1,
ULONG *,imagepalette,A2,
UBYTE *,imagedata,A3,
STARTUP_TYPE,STARTUP_NAME
)
{
    LIBFUNC_INIT
  struct Hook nobfhook;
   nobfhook.h_Entry=(HOOKFUNC) GIVEME_HOOKENTRY(Nobackfillhook);
   if(screen)
   {  loadreqfont.ta_Name=screen->RastPort.Font->tf_Message.mn_Node.ln_Name;
      loadreqfont.ta_YSize=screen->RastPort.Font->tf_YSize;
   }
      loadreqobj=WindowObject,
         WA_PubScreen,screen,
         WINDOW_Position,WPOS_CENTERSCREEN,
         WINDOW_Layout,VLayoutObject,
            LAYOUT_SpaceOuter,TRUE,
            LAYOUT_LeftSpacing,4,
            LAYOUT_RightSpacing,4,
            LAYOUT_BottomSpacing,4,
            LAYOUT_TopSpacing,4,                        /* added */
            LAYOUT_HorizAlignment,LALIGN_CENTER,       /* added */
            LAYOUT_BackFill,&nobfhook,
            StartMember,VLayoutObject,           /* was H */
               LAYOUT_VertAlignment,LALIGN_CENTER,
               LAYOUT_HorizAlignment,LALIGN_CENTER,   /* was LEFT */
               LAYOUT_SpaceInner, TRUE,               /* added */
               LAYOUT_InnerSpacing,4,
               StartMember,HLayoutObject,
                  LAYOUT_BevelStyle,BVS_GROUP,
                  StartImage,PenMapObject,
                     IA_Data,imagedata,
                     PENMAP_Palette,imagepalette,
                     PENMAP_Screen,screen,
                  EndImage,
                  CHILD_MinWidth,(ULONG)((UWORD *)imagedata)[0],
                  CHILD_MaxWidth,(ULONG)((UWORD *)imagedata)[0],
                  CHILD_MinHeight,(ULONG)((UWORD *)imagedata)[1],
               EndMember,
               StartImage,LabelObject,
                  IA_Font,&loadreqfont,
                  LABEL_Justification,LJ_CENTER,
                  LABEL_Text,"AWeb APL Lite\n",
                  LABEL_Text,version,
               EndImage,
            EndMember,
            CHILD_WeightedWidth,0,
            StartMember,loadreqmsg=ButtonObject,
               GA_ReadOnly,TRUE,
               GA_Text,"",
               BUTTON_BevelStyle,BVS_NONE,
               BUTTON_DomainString,"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
            EndMember,
            StartMember,loadreqfg=FuelGaugeObject,
               FUELGAUGE_Min,0,
               FUELGAUGE_Max,100,
               FUELGAUGE_Level,0,
               FUELGAUGE_Ticks,0,
               FUELGAUGE_Percent,FALSE,
            EndMember,
         End,
      End;
      if(loadreqobj)
      {  loadreqwin=RA_OpenWindow(loadreqobj);
      }


    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds void  , doStartupclose,
STARTUP_TYPE,STARTUP_NAME

)
{
    LIBFUNC_INIT
  if(loadreqobj) DisposeObject(loadreqobj);
   loadreqobj=NULL;
   loadreqwin=NULL;

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
__saveds void  , doStartupstate,
ULONG ,state,D0,
STARTUP_TYPE,STARTUP_NAME

)
{
    LIBFUNC_INIT
  UBYTE *msg=NULL;
   if(loadreqwin)
   {  switch(state)
      {  case LRQ_IMAGES:
            msg=AWEBSTR(MSG_STARTUP_IMAGES);
            loadreqmin=0;
            loadreqmax=25;
            break;
         case LRQ_FONTS:
            msg=AWEBSTR(MSG_STARTUP_FONTS);
            loadreqmin=25;
            loadreqmax=50;
            break;
         case LRQ_CACHE:
            msg=AWEBSTR(MSG_STARTUP_CACHE);
            loadreqmin=50;
            loadreqmax=100;
            break;
      }
      if(msg)
      {  Setgadgetattrs(loadreqmsg,loadreqwin,NULL,GA_Text,msg,TAG_END);
         Setgadgetattrs(loadreqfg,loadreqwin,NULL,FUELGAUGE_Level,loadreqmin,TAG_END);
      }
   }

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
__saveds void  , doStartuplevel,
long ,ready,D0,
long ,total,D1,
STARTUP_TYPE,STARTUP_NAME

)
{
    LIBFUNC_INIT
  long level;
   if(loadreqwin)
   {  level=ready*(loadreqmax-loadreqmin)/total+loadreqmin;
      Setgadgetattrs(loadreqfg,loadreqwin,NULL,FUELGAUGE_Level,level,TAG_END);
   }

    LIBFUNC_EXIT
}
