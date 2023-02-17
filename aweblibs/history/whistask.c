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

/* whistask.c - AWeb window history AWebLib module */

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include "aweblib.h"
#include "task.h"
#include "whisprivate.h"
#include "url.h"
#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"

#include <exec/resident.h>
#include <intuition/intuition.h>
#include <reaction/reaction.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#ifdef __MORPHOS__
ULONG __abox__=1;
#endif

#define HISTORY_VERSION 36
#define HISTORY_REVISION 1
#define HISTORY_VERSTRING "36.1"

enum WINHIS_GADGETIDS
{  WGID_LIST=1,WGID_FILTER,WGID_WIN,WGID_ORDER,WGID_DISPLAY,
};

static struct Image *mainimg;

static struct ColumnInfo columninfo[]=
{ {  8,NULL,0 },
  {  4,NULL,0 },
  {  4,NULL,0 },
  { 84,NULL,0 },
  { -1,NULL,0 }
};

static UBYTE *orderlabels[6];

enum WHIS_ORDER
{  WHIS_NATURAL=0,WHIS_MAINLINE,WHIS_RETRIEVED,WHIS_TITLE,WHIS_URL
};

static struct Hook idcmphook;

struct Library *AwebSupportBase;
struct DosLibrary * DOSBase;
struct IntuitionBase *IntuitionBase;
struct Library *WindowBase,*LayoutBase,*ButtonBase,*ListBrowserBase,
   *ChooserBase,*CheckBoxBase,*IntegerBase,*SpaceBase,*LabelBase,*GlyphBase;

#ifdef __MORPHOS__
struct Library       *UtilityBase;
#else
struct UtilityBase       *UtilityBase;
#endif

#if defined(__amigaos4__)
struct AwebSupportIFace *IAwebSupport;
struct DOSIFace *IDOS;
struct IntuitionIFace *IIntuition;
struct UtilityIFace *IUtility;
struct WindowIFace *IWindow;
struct LayoutIFace *ILayout;
struct ButtonIFace *IButton;
struct ListBrowserIFace *IListBrowser;
struct ChooserIFace *IChooser;
struct LabelIFace *ILabel;
struct IntegerIFace *IInteger;
struct CheckBoxIFace *ICheckBox;
struct SpaceIFace *ISpace;
struct GlyphIFace *IGlyph;


#endif

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

static __saveds struct SegList * Real_Closelib(LIBMAN_TYPE LIBMAN_NAME);
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
    __saveds Subtaskfunction *, AwebHistoryGetTaskFunc,
    ULONG, id, D0,
    HISTORY_TYPE, HISTORY_NAME
);

static __saveds void Winhistask(struct Whiswindow *whw);

/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct Library *WinhisBase;

static APTR libseglist;

struct ExecBase *SysBase;

LIBSTART_DUMMY

static char __aligned libname[]="history.aweblib";
static char __aligned libid[]="$VER: history.aweblib " HISTORY_VERSTRING  " (" __AMIGADATE__ ") " CPU;

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
        (void *)AwebHistoryGetTaskFunc,
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
    HISTORY_VERSION,
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
   AwebHistoryGetTaskFunc,
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
   HISTORY_VERSION,
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
    libBase->lib_Version      = HISTORY_VERSION;
    libBase->lib_Revision     = HISTORY_REVISION;
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
   WinhisBase=libbase;
   libbase->lib_Revision=HISTORY_REVISION;
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
   struct Library *Histlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Histlibbase = (struct Library *)LIBMAN_NAME;
#endif

   Histlibbase->lib_OpenCnt++;
   Histlibbase->lib_Flags&=~LIBF_DELEXP;
   if(Histlibbase->lib_OpenCnt==1)
   {
      if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return (struct Library *) Real_Closelib(LIBMAN_NAME);
#if defined (__amigaos4__)
      if(!( IAwebSupport = (struct AwebSupportIface *)GetInterface(AwebSupportBase,"main",1,0))) return Real_Closelib(LIBMAN_NAME);
#endif
   }
   return Histlibbase;
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

static __saveds struct SegList * Real_Closelib(LIBMAN_TYPE LIBMAN_NAME)
{
#if defined(__amigaos4__)
   struct Library *Histlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Histlibbase = (struct Library *)LIBMAN_NAME;
#endif


  Histlibbase->lib_OpenCnt--;
   if(Histlibbase->lib_OpenCnt==0)
   {  if(AwebSupportBase)
      {
#if defined(__amigaos4__)
    if(IAwebSupport) DropInterface((struct Interface *)IAwebSupport);
    IAwebSupport = NULL;
#endif

         CloseLibrary(AwebSupportBase);
         AwebSupportBase=NULL;
      }
      if(Histlibbase->lib_Flags&LIBF_DELEXP)
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
};

static __saveds struct SegList * Real_Expungelib(LIBMAN_TYPE LIBMAN_NAME)
{

#if defined(__amigaos4__)
   struct Library *Histlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Histlibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Histlibbase->lib_OpenCnt==0)
   {  ULONG size=Histlibbase->lib_NegSize+Histlibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Histlibbase-Histlibbase->lib_NegSize;
      Remove((struct Node *)Histlibbase);
      Expungeaweblib(Histlibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Histlibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Histlibbase->lib_Flags|=LIBF_DELEXP;
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

   if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library",0))) return FALSE;
   if(!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",39))) return FALSE;
   if(!(UtilityBase= (struct UtilityBase *) OpenLibrary("utility.library",39))) return FALSE;
   if(!(WindowBase=OpenLibrary("window.class",OSNEED(0,44)))) return FALSE;
   if(!(LayoutBase=OpenLibrary("gadgets/layout.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ButtonBase=OpenLibrary("gadgets/button.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ChooserBase=OpenLibrary("gadgets/chooser.gadget",OSNEED(0,44)))) return FALSE;
   if(!(CheckBoxBase=OpenLibrary("gadgets/checkbox.gadget",OSNEED(0,44)))) return FALSE;
   if(!(IntegerBase=OpenLibrary("gadgets/integer.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ListBrowserBase=OpenLibrary("gadgets/listbrowser.gadget",OSNEED(0,44)))) return FALSE;
   if(!(SpaceBase=OpenLibrary("gadgets/space.gadget",OSNEED(0,44)))) return FALSE;
   if(!(LabelBase=OpenLibrary("images/label.image",OSNEED(0,44)))) return FALSE;
   if(!(GlyphBase=OpenLibrary("images/glyph.image",OSNEED(0,44)))) return FALSE;

#if defined(__amigaos4__)
   if(!(IDOS       = (struct DOSIFace *)        GetInterface(DOSBase,"main",1,0))) return FALSE;
   if(!(IIntuition = (struct IntuitionIFacee *) GetInterface(IntuitionBase,"main",1,0))) return FALSE;
   if(!(IUtility   = (struct UtilityIFace *)    GetInterface(UtilityBase,"main",1,0))) return FALSE;
   if(!(IWindow = (struct WindowIFace *)GetInterface(WindowBase,"main",1,0))) return FALSE;
   if(!(ILayout = (struct LayoutIFace *)GetInterface(LayoutBase,"main",1,0))) return FALSE;
   if(!(IButton = (struct ButtonIFace *)GetInterface(ButtonBase,"main",1,0))) return FALSE;
   if(!(IListBrowser = (struct ListBrowserIFace *)GetInterface(ListBrowserBase,"main",1,0))) return FALSE;
   if(!(IChooser = (struct ChooserIFace *)GetInterface(ChooserBase,"main",1,0))) return FALSE;
   if(!(IInteger = (struct IntegerIFace *)GetInterface(IntegerBase,"main",1,0))) return FALSE;
   if(!(ILabel = (struct LabelIFace *)GetInterface(LabelBase,"main",1,0))) return FALSE;
   if(!(IGlyph = (struct GlyphIFace *)GetInterface(GlyphBase,"main",1,0))) return FALSE;
   if(!(ISpace = (struct SpaceIFace *)GetInterface(SpaceBase,"main",1,0))) return FALSE;
   if(!(ICheckBox = (struct CheckBoxIFace *)GetInterface(CheckBoxBase,"main",1,0))) return FALSE;

#endif

   if(!(mainimg=NewObject(GLYPH_GetClass(),NULL,
      IA_Width,8,
      IA_Height,8,
      GLYPH_Glyph,GLYPH_DOWNARROW,
      TAG_END))) return FALSE;

   return TRUE;
}

static void Expungeaweblib(struct Library *libbase)
{

   if(mainimg) DisposeObject(mainimg);

#if defined(__amigaos4__)

   if(IGlyph)DropInterface((struct Interface *)IGlyph);
   if(ISpace)DropInterface((struct Interface *)ISpace);
   if(ICheckBox)DropInterface((struct Interface *)ICheckBox);
   if(ILabel)DropInterface((struct Interface *)ILabel);
   if(IInteger)DropInterface((struct Interface *)IInteger);
   if(IChooser)DropInterface((struct Interface *)IChooser);
   if(IListBrowser)DropInterface((struct Interface *)IListBrowser);
   if(IButton)DropInterface((struct Interface *)IButton);
   if(ILayout)DropInterface((struct Interface *)ILayout);
   if(IWindow)DropInterface((struct Interface *)IWindow);
   if(IUtility)DropInterface((struct Interface *)IUtility);
   if(IIntuition)DropInterface((struct Interface *)IIntuition);
   if(IDOS)DropInterface((struct Interface *)IDOS);

#endif


   if(GlyphBase) CloseLibrary(GlyphBase);
   if(LabelBase) CloseLibrary(LabelBase);
   if(SpaceBase) CloseLibrary(SpaceBase);
   if(ListBrowserBase) CloseLibrary(ListBrowserBase);
   if(IntegerBase) CloseLibrary(IntegerBase);
   if(CheckBoxBase) CloseLibrary(CheckBoxBase);
   if(ChooserBase) CloseLibrary(ChooserBase);
   if(ButtonBase) CloseLibrary(ButtonBase);
   if(LayoutBase) CloseLibrary(LayoutBase);
   if(WindowBase) CloseLibrary(WindowBase);
   if(UtilityBase) CloseLibrary((struct Library *) UtilityBase);
   if(IntuitionBase) CloseLibrary((struct Library *) IntuitionBase);

    AwebModuleExit();
}

/*-----------------------------------------------------------------------*/

VARARGS68K_DECLARE(static void Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...))
{
   VA_LIST va;
   struct TagItem *tags;
   VA_STARTLIN(va,req);
   tags = (struct TagItem *)VA_GETLIN(va,struct TagItem *);
   if(SetGadgetAttrsA(gad,win,req,tags) && win) RefreshGList(gad,win,req,1);
   VA_END(va);
}

static long Getvalue(void *gad,ULONG tag)
{  long value=0;
   GetAttr(tag,gad,(ULONG *)&value);
   return value;
}

static struct Node *Getnode(struct List *list,long n)
{  struct Node *node=list->lh_Head;
   while(node->ln_Succ && n)
   {  node=node->ln_Succ;
      n--;
   }
   if(node->ln_Succ) return node;
   else return NULL;
}

static void Freechnodes(struct List *list)
{  struct Node *node;
   while(node=RemHead(list)) FreeChooserNode(node);
}

static void Freelbnodes(struct List *list)
{  struct Node *node;
   while(node=RemHead(list)) FreeListBrowserNode(node);
}

static void Makeorderlist(struct Whiswindow *whw)
{  struct Node *node;
   short i;
   for(i=0;orderlabels[i];i++)
   {  if(node=AllocChooserNode(
         CNA_Text,orderlabels[i],
         TAG_END))
         AddTail(&whw->orderlist,node);
   }
}

/* Find the leading frame history */
static struct Framehis *Findfhis(struct Winhis *whis)
{  struct Framehis *fhis;
   for(fhis=whis->frames.first;fhis->next;fhis=fhis->next)
   {  if(STREQUAL(fhis->id,whis->frameid)) return fhis;
   }
   return NULL;
}

/* Return the Urlname to be used as title */
static UBYTE *Urltitle(struct Winhis *whis)
{  if(whis->titleurl) return (UBYTE *)Agetattr(whis->titleurl,AOURL_Url);
   return "";
}

static UBYTE *Buildurlfrag(struct Whiswindow *whw,struct Framehis *fhis)
{  long l;
   if(fhis)
   {  strncpy(whw->urlbuf,
         fhis->url?(UBYTE *)Agetattr(fhis->url,AOURL_Url):NULLSTRING,
         URLBUFSIZE-1);
      l=strlen(whw->urlbuf);
      if(fhis->fragment && l<URLBUFSIZE-1)
      {  strcat(whw->urlbuf,"#");
         l++;
         strncat(whw->urlbuf,fhis->fragment,URLBUFSIZE-l-1);
      }
   }
   else
   {  *whw->urlbuf='\0';
   }
   return whw->urlbuf;
}

static struct Node *Allocnode(struct Whiswindow *whw,struct Winhis *whis)
{  struct Node *node;
   UBYTE buffer[8];
   BOOL skip;
   UBYTE *name;
   struct Framehis *fhis=Findfhis(whis);
   sprintf(buffer,"%ld",whis->windownr);
   skip=whw->order>WHIS_MAINLINE || (whis->wflags&WINHF_SKIP)!=0;
   if(whw->order==WHIS_URL) name=Urltitle(whis);
   else if(whis->title) name=whis->title;
   else if(fhis) name=Buildurlfrag(whw,fhis);
   else name="";
   node=AllocListBrowserNode(4,
      LBNA_UserData,whis,
      LBNA_Column,0,
         LBNCA_CopyText,TRUE,
         LBNCA_Text,buffer,
         LBNCA_Justification,LCJ_RIGHT,
      LBNA_Column,1,
         LBNCA_Text,(*whis->frameid)?"+":"",
      LBNA_Column,2,
         skip?TAG_IGNORE:LBNCA_Image,mainimg,
         skip?LBNCA_Text:TAG_IGNORE,"",
      LBNA_Column,3,
         LBNCA_CopyText,TRUE,
         LBNCA_Text,name,
      TAG_END);
   return node;
}

typedef int whissortf(const void *, const void *);

static int Whissortretrieved(struct Winhis **wa,struct Winhis **wb)
{  int c=0;
   if((*wa)->loadnr && (*wb)->loadnr) c=(*wa)->loadnr-(*wb)->loadnr;
   else c=(int)(*wa)->whisnr-(int)(*wb)->whisnr;
   if(c==0) c=(*wa)->windownr-(*wb)->windownr;
   return c;
}

static int Whissorttitle(struct Winhis **wa,struct Winhis **wb)
{  UBYTE *pa,*pb;
   int c=0;
   pa=(*wa)->title?(*wa)->title:Urltitle(*wa);
   pb=(*wb)->title?(*wb)->title:Urltitle(*wb);
   c=stricmp(pa,pb);
   if(c==0) c=(*wa)->windownr-(*wb)->windownr;
   if(c==0) c=(int)(*wa)->whisnr-(int)(*wb)->whisnr;
   return c;
}

static int Whissorturl(struct Winhis **wa,struct Winhis **wb)
{  int c=0;
   c=stricmp(Urltitle(*wa),Urltitle(*wb));
   if(c==0) c=(*wa)->windownr-(*wb)->windownr;
   if(c==0) c=(int)(*wa)->whisnr-(int)(*wb)->whisnr;
   return c;
}

static void Allocnodes(struct Whiswindow *whw,struct List *list)
{  struct Node *node;
   struct Winhis *whis;
   long windownr=0;
   long nnodes=0;
   if(whw->filter)
   {  if(whw->wingad) windownr=Getvalue(whw->wingad,INTEGER_Number);
      else windownr=whw->windownr;
   }
   if(whw->order<=WHIS_MAINLINE)  /* NATURAL or MAINLINE */
   {  whw->currentnode=-1;
      for(whis=whw->winhislist->first;whis->object.next;whis=whis->object.next)
      {  if(whis->key)
         {  if(!whw->filter || whis->windownr==windownr)
            {  if(whw->order==WHIS_NATURAL || !(whis->wflags&WINHF_SKIP))
               {  if(node=Allocnode(whw,whis))
                  {  AddTail(list,node);
                     if(whis==whw->current) whw->currentnode=nnodes;
                     nnodes++;
                  }
               }
            }
         }
      }
   }
   else
   {  struct Winhis **whwab;
      long i,n,lastnr=0;
      whissortf *f;
      if(whw->winhislist->last->object.prev) n=whw->winhislist->last->whisnr;
      else return;
      if(whwab=ALLOCTYPE(struct Winhis *,n,0))
      {  for(whis=whw->winhislist->first,i=0;whis->object.next;whis=whis->object.next)
         {  if(whis->key)
            {  if(!whw->filter || whis->windownr==windownr) whwab[i++]=whis;
            }
         }
         n=i;
         switch(whw->order)
         {  case WHIS_RETRIEVED:
               f=(whissortf *)Whissortretrieved;
               break;
            case WHIS_TITLE:
               f=(whissortf *)Whissorttitle;
               break;
            case WHIS_URL:
               f=(whissortf *)Whissorturl;
               break;
            default:
               f=NULL;
         }
         if(f) qsort(whwab,n,sizeof(struct Winhis *),f);
         for(i=0;i<n;i++)
         {  if(!whwab[i]->loadnr || whwab[i]->loadnr!=lastnr)
            {  if(node=Allocnode(whw,whwab[i])) AddTail(list,node);
               if(whwab[i]->loadnr) lastnr=whwab[i]->loadnr;
            }
         }
         FREE(whwab);
      }
   }
}

static void Setgads(struct Whiswindow *whw)
{  ULONG selected=Getvalue(whw->listgad,LISTBROWSER_Selected);
   struct Node *node=Getnode(&whw->gadlist,selected);
   struct Winhis *whis;
   struct Framehis *fhis;
   UBYTE *title="",*url="";
   if(node)
   {  GetListBrowserNodeAttrs(node,LBNA_UserData,&whis,TAG_END);
      if(fhis=Findfhis(whis))
      {  if(whis->title) title=whis->title;
         url=Buildurlfrag(whw,fhis);
      }
   }
   Setgadgetattrs(whw->titlegad,whw->window,NULL,
      GA_Text,title,
      TAG_END);
   Setgadgetattrs(whw->urlgad,whw->window,NULL,
      GA_Text,url,
      TAG_END);
}

static void Rebuild(struct Whiswindow *whw,BOOL keepselected)
{  long selected=keepselected?Getvalue(whw->listgad,LISTBROWSER_Selected):~0;
   Setgadgetattrs(whw->listgad,whw->window,NULL,
      LISTBROWSER_Labels,~0,TAG_END);
   Freelbnodes(&whw->gadlist);
   Allocnodes(whw,&whw->gadlist);
   if(selected<0 && whw->order<=WHIS_MAINLINE) selected=whw->currentnode;
   Setgadgetattrs(whw->listgad,whw->window,NULL,
      LISTBROWSER_Labels,&whw->gadlist,
      LISTBROWSER_Selected,selected,
      TAG_END);
   if(selected>=0)
   {  Setgadgetattrs(whw->listgad,whw->window,NULL,
         LISTBROWSER_MakeVisible,selected,
         TAG_END);
   }
   Setgads(whw);
}

static void Dofilter(struct Whiswindow *whw)
{  whw->filter=Getvalue(whw->filtergad,GA_Selected);
   Setgadgetattrs(whw->wingad,whw->window,NULL,
      GA_Disabled,!whw->filter,
      TAG_END);
   Rebuild(whw,FALSE);
}

static void Doorder(struct Whiswindow *whw)
{  whw->order=Getvalue(whw->ordergad,CHOOSER_Active);
   Rebuild(whw,FALSE);
}

static void Setcurrentnode(struct Whiswindow *whw)
{  struct Node *node;
   long nr=0;
   struct Winhis *whis;
   whw->currentnode=-1;
   for(node=whw->gadlist.lh_Head;node->ln_Succ;node=node->ln_Succ,nr++)
   {  GetListBrowserNodeAttrs(node,LBNA_UserData,&whis,TAG_END);
      if(whis==whw->current)
      {  whw->currentnode=nr;
         break;
      }
   }
   Setgadgetattrs(whw->listgad,whw->window,NULL,
      LISTBROWSER_Selected,whw->currentnode,
      LISTBROWSER_MakeVisible,whw->currentnode,
      TAG_END);
}

static BOOL Dodisplay(struct Whiswindow *whw)
{  long selected=Getvalue(whw->listgad,LISTBROWSER_Selected);
   struct Node *node=Getnode(&whw->gadlist,selected);
   struct Winhis *whis;
   BOOL done=FALSE;
   if(node)
   {  GetListBrowserNodeAttrs(node,LBNA_UserData,&whis,TAG_END);
      ReleaseSemaphore(whw->whissema);  /* avoid deadlock, main task needs it */
      Updatetaskattrs(AOWHW_Display,(Tag)whis,TAG_END);
      ObtainSemaphore(whw->whissema);
      done=whw->autoclose;
   }
   return done;
}

static void Moveselected(struct Whiswindow *whw,short n)
{  long selected=Getvalue(whw->listgad,LISTBROWSER_Selected);
   BOOL update=FALSE;
   if(n<0)
   {  if(selected>0)
      {  selected--;
         update=TRUE;
      }
      else if(selected<0)
      {  selected=0;
         update=TRUE;
      }
   }
   else if(n>0)
   {  struct Node *node=Getnode(&whw->gadlist,selected);
      if(node && node->ln_Succ->ln_Succ)
      {  selected++;
         update=TRUE;
      }
      else if(!node)
      {  selected=0;
         update=TRUE;
      }
   }
   if(update)
   {  Setgadgetattrs(whw->listgad,whw->window,NULL,
         LISTBROWSER_Selected,selected,
         LISTBROWSER_MakeVisible,selected,
         TAG_END);
      Setgads(whw);
   }
}

DECLARE_HOOK
(
static long __saveds   , Idcmphook,
struct Hook *,hook,A0,
APTR, dummy, A2,
struct IntuiMessage *,msg,A1
)
{
    USRFUNC_INIT
  struct Whiswindow *whw=hook->h_Data;
   switch(msg->Class)
   {  case IDCMP_CHANGEWINDOW:
         Updatetaskattrs(AOWHW_Dimx,whw->window->LeftEdge,
            AOWHW_Dimy,whw->window->TopEdge,
            AOWHW_Dimw,whw->window->Width-whw->window->BorderLeft-whw->window->BorderRight,
            AOWHW_Dimh,whw->window->Height-whw->window->BorderTop-whw->window->BorderBottom,
            TAG_END);
         break;
   }
   return 0;

    USRFUNC_EXIT
}

/*-----------------------------------------------------------------------*/

static void Buildwinhiswindow(struct Whiswindow *whw)
{  NewList(&whw->gadlist);
   NewList(&whw->orderlist);
   orderlabels[0]=AWEBSTR(MSG_WHIS_ORDER_NATURAL);
   orderlabels[1]=AWEBSTR(MSG_WHIS_ORDER_MAINLINE);
   orderlabels[2]=AWEBSTR(MSG_WHIS_ORDER_RETRIEVED);
   orderlabels[3]=AWEBSTR(MSG_WHIS_ORDER_TITLE);
   orderlabels[4]=AWEBSTR(MSG_WHIS_ORDER_URL);
   idcmphook.h_Entry=(HOOKFUNC) GIVEME_HOOKENTRY(Idcmphook);
   idcmphook.h_Data=whw;
   if(whw->screen=LockPubScreen(whw->screenname))
   {  whw->filter=TRUE;
      whw->order=WHIS_NATURAL;
      ObtainSemaphore(whw->whissema);
      Allocnodes(whw,&whw->gadlist);
      ReleaseSemaphore(whw->whissema);
      Makeorderlist(whw);
      if(!whw->w)
      {  whw->x=whw->screen->Width/4;
         whw->y=whw->screen->Height/4;
         whw->w=whw->screen->Width/2;
         whw->h=whw->screen->Height/2;
      }
      whw->winobj=WindowObject,
         WA_Title,AWEBSTR(MSG_WHIS_TITLE),
         WA_Left,whw->x,
         WA_Top,whw->y,
         WA_InnerWidth,whw->w,
         WA_InnerHeight,whw->h,
         WA_SizeGadget,TRUE,
         WA_DepthGadget,TRUE,
         WA_DragBar,TRUE,
         WA_CloseGadget,TRUE,
         WA_Activate,TRUE,
         WA_AutoAdjust,TRUE,
         WA_SimpleRefresh,TRUE,
         WA_PubScreen,whw->screen,
         WA_IDCMP,IDCMP_RAWKEY,
         WINDOW_IDCMPHook,&idcmphook,
         WINDOW_IDCMPHookBits,IDCMP_CHANGEWINDOW,
         WINDOW_Layout,VLayoutObject,
            LAYOUT_SpaceOuter,TRUE,
            LAYOUT_DeferLayout,TRUE,
            StartMember,whw->listgad=ListBrowserObject,
               GA_ID,WGID_LIST,
               GA_RelVerify,TRUE,
               LISTBROWSER_ShowSelected,TRUE,
               LISTBROWSER_ColumnInfo,columninfo,
               LISTBROWSER_Labels,&whw->gadlist,
               LISTBROWSER_Separators,FALSE,
               LISTBROWSER_Selected,whw->currentnode,
            EndMember,
            StartMember,HLayoutObject,
               StartMember,whw->titlegad=ButtonObject,
                  GA_Text," ",
                  GA_ReadOnly,TRUE,
                  BUTTON_Justification,BCJ_LEFT,
                  GA_Underscore,0,
               EndMember,
               StartMember,ButtonObject,
                  GA_ID,WGID_DISPLAY,
                  GA_RelVerify,TRUE,
                  GA_Text,AWEBSTR(MSG_WHIS_DISPLAY),
               EndMember,
               CHILD_WeightedWidth,0,
            EndMember,
            CHILD_WeightedHeight,0,
            StartMember,whw->urlgad=ButtonObject,
               GA_Text," ",
               GA_ReadOnly,TRUE,
               BUTTON_Justification,BCJ_LEFT,
               GA_Underscore,0,
            EndMember,
            CHILD_WeightedHeight,0,
            StartMember,HLayoutObject,
               LAYOUT_HorizAlignment,LALIGN_LEFT,
               StartMember,whw->filtergad=CheckBoxObject,
                  GA_ID,WGID_FILTER,
                  GA_RelVerify,TRUE,
                  GA_Text,AWEBSTR(MSG_WHIS_FILTER),
                  GA_Selected,TRUE,
               EndMember,
               CHILD_WeightedWidth,0,
               StartMember,whw->wingad=IntegerObject,
                  GA_ID,WGID_WIN,
                  GA_RelVerify,TRUE,
                  INTEGER_Minimum,1,
                  INTEGER_Maximum,99999,
                  INTEGER_Number,whw->windownr,
               EndMember,
               MemberLabel(AWEBSTR(MSG_WHIS_WINDOW)),
               CHILD_MaxWidth,100,
               StartMember,SpaceObject,
               EndMember,
               StartMember,whw->ordergad=ChooserObject,
                  GA_ID,WGID_ORDER,
                  GA_RelVerify,TRUE,
                  CHOOSER_PopUp,TRUE,
                  CHOOSER_Labels,&whw->orderlist,
                  CHOOSER_Active,WHIS_NATURAL,
               EndMember,
               MemberLabel(AWEBSTR(MSG_WHIS_ORDER)),
               CHILD_WeightedWidth,0,
            EndMember,
            CHILD_WeightedHeight,0,
         EndMember,
      EndWindow;
      if(whw->winobj)
      {  if(whw->window=(struct Window *)RA_OpenWindow(whw->winobj))
         {  GetAttr(WINDOW_SigMask,whw->winobj,&whw->winsigmask);
            if(whw->currentnode>=0)
            {  Setgadgetattrs(whw->listgad,whw->window,NULL,
                  LISTBROWSER_MakeVisible,whw->currentnode,
                  TAG_END);
            }
            Setgads(whw);
         }
      }
   }
}

static BOOL Handlewinhiswindow(struct Whiswindow *whw)
{  ULONG result,relevent;
   BOOL done=FALSE;
   UWORD click;
   while((result=RA_HandleInput(whw->winobj,&click))!=WMHI_LASTMSG)
   {  ObtainSemaphore(whw->whissema);
      switch(result&WMHI_CLASSMASK)
      {  case WMHI_CLOSEWINDOW:
            done=TRUE;
            break;
         case WMHI_GADGETUP:
            switch(result&WMHI_GADGETMASK)
            {  case WGID_LIST:
                  relevent=Getvalue(whw->listgad,LISTBROWSER_RelEvent);
                  if(relevent&LBRE_DOUBLECLICK)
                  {  if(click==whw->lastclick) done=Dodisplay(whw);
                  }
                  Setgads(whw);
                  whw->lastclick=click;
                  break;
               case WGID_FILTER:
                  Dofilter(whw);
                  break;
               case WGID_WIN:
                  if(whw->filter)
                  {  whw->windownr=Getvalue(whw->wingad,INTEGER_Number);
                     Updatetaskattrs(AOWHW_Getcurrent,TRUE,TAG_END);
                     Rebuild(whw,FALSE);
                  }
                  break;
               case WGID_ORDER:
                  Doorder(whw);
                  break;
               case WGID_DISPLAY:
                  done=Dodisplay(whw);
                  break;
            }
            break;
         case WMHI_RAWKEY:
            switch(result&WMHI_GADGETMASK)
            {  case 0x45:  /* esc */
                  done=TRUE;
                  break;
               case 0x43:  /* num enter */
               case 0x44:  /* enter */
                  done=Dodisplay(whw);
                  break;
               case 0x4c:  /* up */
                  Moveselected(whw,-1);
                  break;
               case 0x4d:  /* down */
                  Moveselected(whw,1);
                  break;
            }
            break;
/*
         case WMHI_CHANGEWINDOW:
            Updatetaskattrs(AOWHW_Dimx,whw->window->LeftEdge,
               AOWHW_Dimy,whw->window->TopEdge,
               AOWHW_Dimw,whw->window->Width-whw->window->BorderLeft-whw->window->BorderRight,
               AOWHW_Dimh,whw->window->Height-whw->window->BorderTop-whw->window->BorderBottom,
               TAG_END);
            break;
*/
      }
      ReleaseSemaphore(whw->whissema);
   }
   return done;
}

static void Closewinhiswindow(struct Whiswindow *whw)
{  if(whw->winobj)
   {  if(whw->window)
      {  Updatetaskattrs(AOWHW_Dimx,whw->window->LeftEdge,
            AOWHW_Dimy,whw->window->TopEdge,
            AOWHW_Dimw,whw->window->Width-whw->window->BorderLeft-whw->window->BorderRight,
            AOWHW_Dimh,whw->window->Height-whw->window->BorderTop-whw->window->BorderBottom,
            TAG_END);
      }
      DisposeObject(whw->winobj);
   }
   Freelbnodes(&whw->gadlist);
   Freechnodes(&whw->orderlist);
   if(whw->screen) UnlockPubScreen(NULL,whw->screen);whw->screen=NULL;
   whw->wingad=NULL;
}

static __saveds void Winhistask(struct Whiswindow *whw)
{
   struct Taskmsg *wm;
   ULONG done=FALSE;
   ULONG getmask;
   struct TagItem *tag,*tstate;
   struct Winhis *whis;
   Buildwinhiswindow(whw);
   if(whw->window)
   {  while(!done)
      {  getmask=Waittask(whw->winsigmask);
         while(!done && (wm=Gettaskmsg()))
         {  if(wm->amsg && wm->amsg->method==AOM_SET)
            {  tstate=((struct Amset *)wm->amsg)->tags;
               while(tag=NextTagItem(&tstate))
               {  switch(tag->ti_Tag)
                  {  case AOTSK_Stop:
                        if(tag->ti_Data) done=TRUE;
                        break;
                     case AOWHW_Tofront:
                        if(tag->ti_Data)
                        {  WindowToFront(whw->window);
                           ActivateWindow(whw->window);
                        }
                        if(Getvalue(whw->wingad,INTEGER_Number)!=tag->ti_Data)
                        {  Setgadgetattrs(whw->wingad,whw->window,NULL,
                              INTEGER_Number,tag->ti_Data,
                              TAG_END);
                           whw->windownr=tag->ti_Data;
                           Updatetaskattrs(AOWHW_Getcurrent,TRUE,TAG_END);
                           ObtainSemaphore(whw->whissema);
                           Rebuild(whw,FALSE);
                           ReleaseSemaphore(whw->whissema);
                        }
                        break;
                     case AOWHW_Changed:
                        if(!whw->filter || (long)tag->ti_Data<0
                        || tag->ti_Data==Getvalue(whw->wingad,INTEGER_Number))
                        {  ObtainSemaphore(whw->whissema);
                           Rebuild(whw,TRUE);
                           ReleaseSemaphore(whw->whissema);
                        }
                        break;
                     case AOWHW_Current:
                        whis=(struct Winhis *)tag->ti_Data;
                        if(!whw->filter
                        || (whis && (whis->windownr==Getvalue(whw->wingad,INTEGER_Number))))
                        {  ObtainSemaphore(whw->whissema);
                           whw->current=whis;
                           if(whw->order<=WHIS_MAINLINE) Setcurrentnode(whw);
                           ReleaseSemaphore(whw->whissema);
                        }
                        break;
                  }
               }
            }
            Replytaskmsg(wm);
         }
         if(!done && (getmask&whw->winsigmask))
         {  done=Handlewinhiswindow(whw);
         }
      }
      Closewinhiswindow(whw);
   }
   Updatetaskattrs(AOTSK_Async,TRUE,
      AOWHW_Close,TRUE,
      TAG_END);
}


LIBFUNC_H1
(
    __saveds Subtaskfunction *, AwebHistoryGetTaskFunc,
    ULONG, id, D0,
    HISTORY_TYPE, HISTORY_NAME
)
{
     LIBFUNC_INIT

     return (Subtaskfunction *)&Winhistask;

     LIBFUNC_EXIT
}

/*-----------------------------------------------------------------------*/
