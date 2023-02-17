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

/* hotlisttask.o - AWeb hotlist AwebLib module */

#include "aweblib.h"
#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"
#include "hotlist.h"

#include <exec/resident.h>
#include <proto/exec.h>

#include "task.h"

struct Library *AwebSupportBase;
struct ExecBase *SysBase;
struct DosLibrary    *DOSBase;
struct GfxBase       *GfxBase;
struct IntuitionBase *IntuitionBase;
struct Library *WindowBase,*LayoutBase,*ButtonBase,*ListBrowserBase,
   *StringBase,*ChooserBase,*CheckBoxBase,*SpaceBase,*LabelBase,*GlyphBase,
   *DrawListBase;

#ifdef __MORPHOS__
struct Library       *UtilityBase;
#else
struct UtilityBase       *UtilityBase;
#endif

#ifdef __MORPHOS__
ULONG __abox__=1;
#endif

#if defined(__amigaos4__)
struct AwebSupportIFace *IAwebSupport;
struct DOSIFace *IDOS;
struct GraphicsIFace *IGraphics;
struct IntuitionIFace *IIntuition;
struct UtilityIFace *IUtility;
struct WindowIFace *IWindow;
struct LayoutIFace *ILayout;
struct ButtonIFace *IButton;
struct ListBrowserIFace *IListBrowser;
struct ChooserIFace *IChooser;
struct LabelIFace *ILabel;
struct StringIFace *IString;
struct CheckBoxIFace *ICheckBox;
struct SpaceIFace *ISpace;
struct GlyphIFace *IGlyph;
struct DrawListIFace *IDrawList;


#endif

#define HOTLIST_VERSION 36
#define HOTLIST_REVISION 1
#define HOTLIST_VERSTRING "36.1"



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

static __saveds struct SegList * Real_Closelib( LIBMAN_TYPE LIBMAN_NAME);
LIBFUNC_P0
(
 __saveds struct SegList * , Closelib,
 LIBMAN_TYPE, LIBMAN_NAME
);

static __saveds struct SegList * Real_Expungelib( LIBMAN_TYPE LIBMAN_NAME);
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
    static __saveds Subtaskfunction *, GetTaskFunc,
    ULONG, id, D0,
    LIBMAN_TYPE, LIBMAN_NAME
);

__saveds void Hotviewtask(struct Hotwindow *how);
__saveds void Hotmgrtask(struct Hotwindow *how);


/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct Library *HotlistBase;

static APTR libseglist;

static char __aligned libname[]="hotlist.aweblib";
static char __aligned libid[]="$VER: hotlist.aweblib " HOTLIST_VERSTRING  " (" __AMIGADATE__ ") " CPU;

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
        (void *)GetTaskFunc,
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
    HOTLIST_VERSION,
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
   GetTaskFunc,
   (APTR)-1,
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
   HOTLIST_VERSION,
   NT_LIBRARY,
   0,
   libname,
   libid,
   inittab
};

#endif // endof OS3 lib section


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
    libBase->lib_Version      = HOTLIST_VERSION;
    libBase->lib_Revision     = HOTLIST_REVISION;
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
   HotlistBase=libbase;
   libbase->lib_Revision=HOTLIST_REVISION;
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
__saveds struct Library * , Openlib,
LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT

#if defined(__amigaos4__)
   struct Library *Hotlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Hotlibbase = (struct Library *)LIBMAN_NAME;
#endif

   Hotlibbase->lib_OpenCnt++;
   Hotlibbase->lib_Flags&=~LIBF_DELEXP;
   if(Hotlibbase->lib_OpenCnt==1)
   {
      if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return (struct Library *) Real_Closelib(LIBMAN_NAME);
#if defined (__amigaos4__)
      if(!( IAwebSupport = (struct AwebSupportIface *)GetInterface(AwebSupportBase,"main",1,0))) return Real_Closelib(LIBMAN_NAME);
#endif
   }
   return Hotlibbase;

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

static __saveds struct SegList * Real_Closelib( LIBMAN_TYPE LIBMAN_NAME)
{

#if defined(__amigaos4__)
   struct Library *Hotlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Hotlibbase = (struct Library *)LIBMAN_NAME;
#endif


  Hotlibbase->lib_OpenCnt--;
   if(Hotlibbase->lib_OpenCnt==0)
   {  if(AwebSupportBase)
      {
#if defined(__amigaos4__)
    if(IAwebSupport) DropInterface((struct Interface *)IAwebSupport);
    IAwebSupport = NULL;
#endif

         CloseLibrary(AwebSupportBase);
         AwebSupportBase=NULL;
      }
      if(Hotlibbase->lib_Flags&LIBF_DELEXP)
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
                        return Real_Expungelib(LIBMAN_NAME)     ;
        LIBFUNC_EXIT
}

static __saveds struct SegList * Real_Expungelib(LIBMAN_TYPE LIBMAN_NAME)
{
#if defined(__amigaos4__)
   struct Library *Hotlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Hotlibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Hotlibbase->lib_OpenCnt==0)
   {  ULONG size=Hotlibbase->lib_NegSize+Hotlibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Hotlibbase-Hotlibbase->lib_NegSize;
      Remove((struct Node *)Hotlibbase);
      Expungeaweblib(Hotlibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Hotlibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Hotlibbase->lib_Flags|=LIBF_DELEXP;
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

LIBFUNC_H1
(
    __saveds static Subtaskfunction *, GetTaskFunc,
    ULONG, id, D0,
    LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT

    if (id == 0)
        return (Subtaskfunction *)&Hotviewtask;
    else
        return (Subtaskfunction *)&Hotmgrtask;

    LIBFUNC_EXIT
}

/*-----------------------------------------------------------------------*/

static ULONG Initaweblib(struct Library *libbase)
{
   if(!AwebModuleInit()) return FALSE;
   if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library",39))) return FALSE;
   if(!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",39))) return FALSE;
   if(!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library",39))) return FALSE;
   if(!(UtilityBase= (struct UtilityBase *) OpenLibrary("utility.library",39))) return FALSE;
   if(!(WindowBase=OpenLibrary("window.class",OSNEED(0,44)))) return FALSE;
   if(!(LayoutBase=OpenLibrary("gadgets/layout.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ButtonBase=OpenLibrary("gadgets/button.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ListBrowserBase=OpenLibrary("gadgets/listbrowser.gadget",OSNEED(0,44)))) return FALSE;
   if(!(StringBase=OpenLibrary("gadgets/string.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ChooserBase=OpenLibrary("gadgets/chooser.gadget",OSNEED(0,44)))) return FALSE;
   if(!(CheckBoxBase=OpenLibrary("gadgets/checkbox.gadget",OSNEED(0,44)))) return FALSE;
   if(!(SpaceBase=OpenLibrary("gadgets/space.gadget",OSNEED(0,44)))) return FALSE;
   if(!(LabelBase=OpenLibrary("images/label.image",OSNEED(0,44)))) return FALSE;
   if(!(GlyphBase=OpenLibrary("images/glyph.image",OSNEED(0,44)))) return FALSE;
   if(!(DrawListBase=OpenLibrary("images/drawlist.image",OSNEED(0,44)))) return FALSE;

#if defined(__amigaos4__)
   if(!(IDOS       = (struct DOSIFace *)        GetInterface(DOSBase,"main",1,0))) return FALSE;
   if(!(IIntuition = (struct IntuitionIFacee *) GetInterface(IntuitionBase,"main",1,0))) return FALSE;
   if(!(IUtility   = (struct UtilityIFace *)    GetInterface(UtilityBase,"main",1,0))) return FALSE;
   if(!(IGraphics  =(struct GraphicsIFace *)    GetInterface(GfxBase,"main",1,0))) return FALSE;
   if(!(IWindow = (struct WindowIFace *)GetInterface(WindowBase,"main",1,0))) return FALSE;
   if(!(ILayout = (struct LayoutIFace *)GetInterface(LayoutBase,"main",1,0))) return FALSE;
   if(!(IButton = (struct ButtonIFace *)GetInterface(ButtonBase,"main",1,0))) return FALSE;
   if(!(IListBrowser = (struct ListBrowserIFace *)GetInterface(ListBrowserBase,"main",1,0))) return FALSE;
   if(!(IChooser = (struct ChooserIFace *)GetInterface(ChooserBase,"main",1,0))) return FALSE;
   if(!(IString = (struct StringIFace *)GetInterface(StringBase,"main",1,0))) return FALSE;
   if(!(ILabel = (struct LabelIFace *)GetInterface(LabelBase,"main",1,0))) return FALSE;
   if(!(IGlyph = (struct GlyphIFace *)GetInterface(GlyphBase,"main",1,0))) return FALSE;
   if(!(IDrawList = (struct DrawListIFace *)GetInterface(DrawListBase,"main",1,0))) return FALSE;
   if(!(ISpace = (struct SpaceIFace *)GetInterface(SpaceBase,"main",1,0))) return FALSE;
   if(!(ICheckBox = (struct CheckBoxIFace *)GetInterface(CheckBoxBase,"main",1,0))) return FALSE;

#endif

   return TRUE;
}

static void Expungeaweblib(struct Library *libbase)
{
#if defined(__amigaos4__)

   if(IGlyph)DropInterface((struct Interface *)IGlyph);
   if(IDrawList)DropInterface((struct Interface *)IDrawList);
   if(ISpace)DropInterface((struct Interface *)ISpace);
   if(ICheckBox)DropInterface((struct Interface *)ICheckBox);
   if(ILabel)DropInterface((struct Interface *)ILabel);
   if(IString)DropInterface((struct Interface *)IString);
   if(IChooser)DropInterface((struct Interface *)IChooser);
   if(IListBrowser)DropInterface((struct Interface *)IListBrowser);
   if(IButton)DropInterface((struct Interface *)IButton);
   if(ILayout)DropInterface((struct Interface *)ILayout);
   if(IWindow)DropInterface((struct Interface *)IWindow);
   if(IUtility)DropInterface((struct Interface *)IUtility);
   if(IGraphics)DropInterface((struct Interface *)IGraphics);
   if(IIntuition)DropInterface((struct Interface *)IIntuition);
   if(IDOS)DropInterface((struct Interface *)IDOS);

#endif

   if(LabelBase) CloseLibrary(LabelBase);
   if(DrawListBase) CloseLibrary(DrawListBase);
   if(GlyphBase) CloseLibrary(GlyphBase);
   if(SpaceBase) CloseLibrary(SpaceBase);
   if(CheckBoxBase) CloseLibrary(CheckBoxBase);
   if(ChooserBase) CloseLibrary(ChooserBase);
   if(StringBase) CloseLibrary(StringBase);
   if(ListBrowserBase) CloseLibrary(ListBrowserBase);
   if(ButtonBase) CloseLibrary(ButtonBase);
   if(LayoutBase) CloseLibrary(LayoutBase);
   if(WindowBase) CloseLibrary(WindowBase);
   if(UtilityBase) CloseLibrary((struct Library *) UtilityBase);
   if(GfxBase) CloseLibrary((struct Library *) GfxBase);
   if(IntuitionBase) CloseLibrary((struct Library *) IntuitionBase);
   if(DOSBase) CloseLibrary((struct Library *)DOSBase);

   AwebModuleExit();
}
