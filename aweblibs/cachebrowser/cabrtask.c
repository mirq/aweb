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

/* cabrtask.c - AWeb cabrowse task */

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include "aweblib.h"
#include "cache.h"
#include "url.h"
#include "task.h"
#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"


#include <exec/resident.h>
#include <intuition/intuition.h>
#include <libraries/locale.h>
#include <reaction/reaction.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#include "caprivate.h"

#ifdef __MORPHOS__
ULONG __abox__=1;
#endif

#define CACHEBROWSER_VERSION 38
#define CACHEBROWSER_REVISION 1
#define CACHEBROWSER_VERSTRING "38.1"

struct Cabrinfo   /* UserData */
{  void *url;
   ULONG cachedate;
   UBYTE urlbuf[128];
   UBYTE datebuf[32];
   UBYTE sizebuf[16];
   UBYTE typebuf[32];
   UBYTE charset[32];
   UBYTE etag[64];
   UBYTE filebuf[40];
};

enum CABROWSE_GADGETIDS
{  CGID_LIST=1,CGID_FIND,CGID_PAT,CGID_FSTRING,CGID_NEXT,CGID_PSTRING,
   CGID_SORT,CGID_OPEN,CGID_SAVE,CGID_DELETE,
};

enum CABROWSE_PAGECODES
{  CPGC_STATUS,CPGC_FIND,CPGC_PATTERN,
};

static struct ColumnInfo columninfo[]=
{ { 50,NULL,0 },
  { 10,NULL,0 },
  { 10,NULL,0 },
  { 15,NULL,0 },
  { 15,NULL,0 },
  { 15,NULL,0 },
  { 15,NULL,0 },
  { -1,NULL,0 }
};

static UBYTE *sortlabels[4];

enum CABR_SORTMODES
{  CABR_URL=0,CABR_DATE,CABR_TYPE,
};

struct Library    *AwebSupportBase;
struct DosLibrary        *DOSBase;
struct IntuitionBase     *IntuitionBase;
struct Library *WindowBase,*LayoutBase,*ButtonBase,*ListBrowserBase,
   *ChooserBase,*FuelGaugeBase,*LabelBase,*StringBase;

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
struct FuelGaugeIFace *IFuelGauge;
struct LabelIFace *ILabel;
struct StringIFace *IString;

#endif

static BOOL shift;
static struct Hook idcmphook;


/*-----------------------------------------------------------------------*/
/* AWebLib module startup */

/* pre declare struct SegList */

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
    __saveds static Subtaskfunction *, GetTaskFunc,
    ULONG, id, D0,
    CACHEBROWSER_TYPE, CACHEBROWSER_NAME
);

__saveds static void Cabrtask(struct Cabrwindow *cbw);


/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct Library *CabrowseBase;

static APTR libseglist;

struct ExecBase *SysBase;

LIBSTART_DUMMY

static char __aligned libname[]="cachebrowser.aweblib";
static char __aligned libid[]="$VER: cachebrowser.aweblib " CACHEBROWSER_VERSTRING " (" __AMIGADATE__ ") " CPU;

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
static struct Resident lib_res __attribute__((used)) =
{
    RTC_MATCHWORD,
    &lib_res,
    &lib_res+1,
    RTF_NATIVE|RTF_AUTOINIT,
    CACHEBROWSER_VERSION,
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
   CACHEBROWSER_VERSION,
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
    libBase->lib_Version      = CACHEBROWSER_VERSION;
    libBase->lib_Revision     = CACHEBROWSER_REVISION;
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
   CabrowseBase=libbase;
   libbase->lib_Revision=CACHEBROWSER_REVISION;
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
   struct Library *Cabrlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Cabrlibbase = (struct Library *)LIBMAN_NAME;
#endif

   Cabrlibbase->lib_OpenCnt++;
   Cabrlibbase->lib_Flags&=~LIBF_DELEXP;
   if(Cabrlibbase->lib_OpenCnt==1)
   {
      if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return (struct Library *) Real_Closelib(LIBMAN_NAME);
#if defined (__amigaos4__)
      if(!( IAwebSupport = (struct AwebSupportIface *)GetInterface(AwebSupportBase,"main",1,0))) return Real_Closelib(LIBMAN_NAME);
#endif
   }
   return Cabrlibbase;

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
   struct Library *Cabrlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Cabrlibbase = (struct Library *)LIBMAN_NAME;
#endif


  Cabrlibbase->lib_OpenCnt--;
   if(Cabrlibbase->lib_OpenCnt==0)
   {  if(AwebSupportBase)
      {
#if defined(__amigaos4__)
    if(IAwebSupport) DropInterface((struct Interface *)IAwebSupport);
    IAwebSupport = NULL;
#endif

         CloseLibrary(AwebSupportBase);
         AwebSupportBase=NULL;
      }
      if(Cabrlibbase->lib_Flags&LIBF_DELEXP)
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
   struct Library *Cabrlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Cabrlibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Cabrlibbase->lib_OpenCnt==0)
   {  ULONG size=Cabrlibbase->lib_NegSize+Cabrlibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Cabrlibbase-Cabrlibbase->lib_NegSize;
      Remove((struct Node *)Cabrlibbase);
      Expungeaweblib(Cabrlibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Cabrlibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Cabrlibbase->lib_Flags|=LIBF_DELEXP;
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
#ifdef LOCALONLY
   return FALSE;
#else
   if(!AwebModuleInit()) return FALSE;

   if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library",39))) return FALSE;
   if(!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",39))) return FALSE;
   if(!(UtilityBase=(struct UtilityBase *) OpenLibrary("utility.library",39))) return FALSE;
   if(!(WindowBase=OpenLibrary("window.class",OSNEED(0,44)))) return FALSE;
   if(!(LayoutBase=OpenLibrary("gadgets/layout.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ButtonBase=OpenLibrary("gadgets/button.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ListBrowserBase=OpenLibrary("gadgets/listbrowser.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ChooserBase=OpenLibrary("gadgets/chooser.gadget",OSNEED(0,44)))) return FALSE;
   if(!(FuelGaugeBase=OpenLibrary("gadgets/fuelgauge.gadget",OSNEED(0,44)))) return FALSE;
   if(!(StringBase=OpenLibrary("gadgets/string.gadget",OSNEED(0,44)))) return FALSE;
   if(!(LabelBase=OpenLibrary("images/label.image",OSNEED(0,44)))) return FALSE;

#if defined(__amigaos4__)
   if(!(IDOS       = (struct DOSIFace *)        GetInterface(DOSBase,"main",1,0))) return FALSE;
   if(!(IIntuition = (struct IntuitionIFacee *) GetInterface(IntuitionBase,"main",1,0))) return FALSE;
   if(!(IUtility   = (struct UtilityIFace *)    GetInterface(UtilityBase,"main",1,0))) return FALSE;
   if(!(IWindow = (struct WindowIFace *)GetInterface(WindowBase,"main",1,0))) return FALSE;
   if(!(ILayout = (struct LayoutIFace *)GetInterface(LayoutBase,"main",1,0))) return FALSE;
   if(!(IButton = (struct ButtonIFace *)GetInterface(ButtonBase,"main",1,0))) return FALSE;
   if(!(IListBrowser = (struct ListBrowserIFace *)GetInterface(ListBrowserBase,"main",1,0))) return FALSE;
   if(!(IChooser = (struct ChooserIFace *)GetInterface(ChooserBase,"main",1,0))) return FALSE;
   if(!(IFuelGauge = (struct FuelGaugeIFace *)GetInterface(FuelGaugeBase,"main",1,0))) return FALSE;
   if(!(IString = (struct StribngIFace *)GetInterface(StringBase,"main",1,0))) return FALSE;
   if(!(ILabel = (struct LabrelIFace *)GetInterface(LabelBase,"main",1,0))) return FALSE;

#endif
   return TRUE;
#endif
}

static void Expungeaweblib(struct Library *libbase)
{
#ifndef LOCALONLY

#if defined(__amigaos4__)

   if(ILabel)DropInterface((struct Interface *)ILabel);
   if(IString)DropInterface((struct Interface *)IString);
   if(IFuelGauge)DropInterface((struct Interface *)IFuelGauge);
   if(IChooser)DropInterface((struct Interface *)IChooser);
   if(IListBrowser)DropInterface((struct Interface *)IListBrowser);
   if(IButton)DropInterface((struct Interface *)IButton);
   if(ILayout)DropInterface((struct Interface *)ILayout);
   if(IWindow)DropInterface((struct Interface *)IWindow);
   if(IUtility)DropInterface((struct Interface *)IUtility);
   if(IIntuition)DropInterface((struct Interface *)IIntuition);
   if(IDOS)DropInterface((struct Interface *)IDOS);

#endif

   if(LabelBase) CloseLibrary(LabelBase);
   if(StringBase) CloseLibrary(StringBase);
   if(FuelGaugeBase) CloseLibrary(FuelGaugeBase);
   if(ChooserBase) CloseLibrary(ChooserBase);
   if(ListBrowserBase) CloseLibrary(ListBrowserBase);
   if(ButtonBase) CloseLibrary(ButtonBase);
   if(LayoutBase) CloseLibrary(LayoutBase);
   if(WindowBase) CloseLibrary(WindowBase);
   if(UtilityBase) CloseLibrary((struct Library *) UtilityBase);
   if(IntuitionBase) CloseLibrary((struct Library *) IntuitionBase);
   if(DOSBase) CloseLibrary((struct Library *) DOSBase);

   AwebModuleExit();
#endif
}

/*-----------------------------------------------------------------------*/

#ifndef LOCALONLY

VARARGS68K_DECLARE(static void Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...))
{
   VA_LIST va;
   struct TagItem *tags;
   VA_STARTLIN(va,req);
   tags = (struct TagItem *)VA_GETLIN(va,struct TagItem *);
   if(SetGadgetAttrsA(gad,win,req,tags) && win) RefreshGList(gad,win,req,1);
   VA_END(va);
}

VARARGS68K_DECLARE(static void Setpagegadgetattrs(struct Gadget *gad,void *page,struct Window *win,
   struct Requester *req,...))
{
   VA_LIST va;
   struct TagItem *tags;
   VA_STARTLIN(va,req);
   tags = (struct TagItem *)VA_GETLIN(va, struct TagItem *);
   if(SetPageGadgetAttrsA(gad,page,win,req,tags) && win) RefreshGList(gad,win,req,1);
   VA_END(va);
}



VARARGS68K_DECLARE(static void Editlbnode(struct Gadget *gad,struct Window *win,struct Requester *req,
   struct Node *node,...))
{
   VA_LIST va;
   struct TagItem *tags;
   VA_STARTLIN(va,node);
   tags = (struct TagItem *)VA_GETLIN(va, struct TagItem *);

   if(DoGadgetMethod(gad,win,req,LBM_EDITNODE,NULL,node,tags))
   {  // RefreshGList(gad,win,req,1);
   }
   VA_END(va);
}

static UBYTE *Strstri(UBYTE *str,UBYTE *sub)
{  long l=strlen(sub);
   UBYTE *end=str+strlen(str)-l;
   for(;str<=end;str++)
   {  if(STRNIEQUAL(str,sub,l)) return str;
   }
   return NULL;
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
   struct Cabrinfo *ci;
   while(node=RemHead(list))
   {  ci=NULL;
      GetListBrowserNodeAttrs(node,LBNA_UserData,&ci,TAG_END);
      if(ci) FREE(ci);
      FreeListBrowserNode(node);
   }
}

static struct Node *Allocbrnode(struct Cabrwindow *cbw,struct Cache *cac)
{  struct Node *node=NULL;
   struct Cabrinfo *ci;
   struct DateStamp ds={0};
   int i;
   if((ci=ALLOCSTRUCT(Cabrinfo,1,MEMF_CLEAR)))
   {  ci->url=cac->url;
      ci->cachedate=cac->cachedate;
      strncpy(ci->urlbuf,(UBYTE *)Agetattr(cac->url,AOURL_Url),127);
      ds.ds_Days=cac->cachedate/86400;
      Lprintdate(ci->datebuf,Locale()->loc_ShortDateFormat,&ds);
      sprintf(ci->sizebuf,"%ld",cac->disksize);
      for(i=0;cac->mimetype[i];i++) ci->typebuf[i]=tolower(cac->mimetype[i]);
      for(i=0;cac->charset[i];i++) ci->charset[i]=tolower(cac->charset[i]);
      for(i=0;cac->etag[i];i++) ci->etag[i]=cac->etag[i];
      strncpy(ci->filebuf,cbw->cfnameshort(cac->name),sizeof(ci->filebuf)-1);
      if(!(node=AllocListBrowserNode(7,
         LBNA_UserData,ci,
         LBNA_Column,0,
            LBNCA_Text,ci->urlbuf,
         LBNA_Column,1,
            LBNCA_Text,ci->datebuf,
            LBNCA_Justification,LCJ_RIGHT,
         LBNA_Column,2,
            LBNCA_Text,ci->sizebuf,
            LBNCA_Justification,LCJ_RIGHT,
         LBNA_Column,3,
            LBNCA_Text,ci->typebuf,
         LBNA_Column,4,
            LBNCA_Text,ci->charset,
         LBNA_Column,5,
            LBNCA_Text,ci->etag,
         LBNA_Column,6,
            LBNCA_Text,ci->filebuf,
         TAG_END)))
         FREE(ci);
   }
   return node;
}

static void Allocnodes(struct Cabrwindow *cbw,struct List *list)
{  struct Node *node;
   struct Cache *cac;
   ObtainSemaphore(cbw->cachesema);
   for(cac=cbw->cache->first;cac->object.next;cac=cac->object.next)
   {  if(cac->name)  /* We have a file */
      {  if(node=Allocbrnode(cbw,cac))
         {  AddTail(list,node);
            cbw->nrfiles++;
            cac->brnode=node;
         }
      }
   }
   ReleaseSemaphore(cbw->cachesema);
}

/* set all Cache->brnode to NULL to prevent REMOBJECT messages with invalid nodes */
static void Clearbrnodes(struct Cabrwindow *cbw)
{  struct Cache *cac;
   ObtainSemaphore(cbw->cachesema);
   for(cac=cbw->cache->first;cac->object.next;cac=cac->object.next) cac->brnode=NULL;
   ReleaseSemaphore(cbw->cachesema);
}

static void Makesortlist(struct Cabrwindow *cbw)
{  struct Node *node;
   long i;
   for(i=0;sortlabels[i];i++)
   {  if(node=AllocChooserNode(
         CNA_Text,sortlabels[i],
         TAG_END))
         AddTail(&cbw->sortlist,node);
   }
}

typedef int cabrsortf(const void *, const void *);

static int Cabrsorturl(struct Node **na,struct Node **nb)
{  UBYTE *urla,*urlb;
   GetListBrowserNodeAttrs(*na,LBNA_Column,0,LBNCA_Text,&urla,TAG_END);
   GetListBrowserNodeAttrs(*nb,LBNA_Column,0,LBNCA_Text,&urlb,TAG_END);
   return stricmp(urla,urlb);
}

static int Cabrsortdate(struct Node **na,struct Node **nb)
{  struct Cabrinfo *cia,*cib;
   GetListBrowserNodeAttrs(*na,LBNA_UserData,&cia,TAG_END);
   GetListBrowserNodeAttrs(*nb,LBNA_UserData,&cib,TAG_END);
   return (int)cib->cachedate-(int)cia->cachedate;  /* most recent first */
}

static int Cabrsorttype(struct Node **na,struct Node **nb)
{  UBYTE *typea,*typeb;
   int c;
   GetListBrowserNodeAttrs(*na,LBNA_Column,3,LBNCA_Text,&typea,TAG_END);
   GetListBrowserNodeAttrs(*nb,LBNA_Column,3,LBNCA_Text,&typeb,TAG_END);
   if(!typea) typea="";
   if(!typeb) typeb="";
   c=stricmp(typea,typeb);
   if(!c) c=Cabrsorturl(na,nb);
   return c;
}

static void Sortnodes(struct Cabrwindow *cbw,UWORD sort)
{  struct Node **nodes;
   struct Node *node;
   long i;
   cabrsortf *f;
   if(nodes=ALLOCSTRUCT(Node *,cbw->nrfiles,MEMF_CLEAR))
   {  for(i=0;i<cbw->nrfiles;i++)
      {  if(node=RemHead(&cbw->gadlist)) nodes[i]=node;
         else break;
      }
      cbw->nrfiles=i;
      switch(sort)
      {  case CABR_URL:
            f=(cabrsortf *)Cabrsorturl;
            break;
         case CABR_DATE:
            f=(cabrsortf *)Cabrsortdate;
            break;
         case CABR_TYPE:
            f=(cabrsortf *)Cabrsorttype;
            break;
         default:
            f=NULL;
      }
      if(f) qsort(nodes,cbw->nrfiles,sizeof(struct Node *),f);
      for(i=0;i<cbw->nrfiles;i++)
      {  AddTail(&cbw->gadlist,nodes[i]);
      }
      FREE(nodes);
      cbw->sort=sort;
   }
}

static void Setcabrstatus(struct Cabrwindow *cbw)
{  long varargs[2];
   varargs[0]=cbw->nrfiles;
   varargs[1]=cbw->currentsize/1024;
   Setpagegadgetattrs(cbw->statusgad,cbw->pagegad,cbw->window,NULL,
      GA_Text,AWEBSTR(MSG_CABR_STATUS),
      BUTTON_VarArgs,varargs,
      TAG_END);
   Setpagegadgetattrs(cbw->fuelgad,cbw->pagegad,cbw->window,NULL,
      FUELGAUGE_Level,cbw->currentsize,TAG_END);
}

static void Setgads(struct Cabrwindow *cbw)
{
   long numsel=Getvalue(cbw->listgad,LISTBROWSER_NumSelected);
   Setgadgetattrs(cbw->opengad,cbw->window,NULL,GA_Disabled,numsel!=1,TAG_END);
   Setgadgetattrs(cbw->savegad,cbw->window,NULL,GA_Disabled,numsel!=1,TAG_END);
   Setgadgetattrs(cbw->deletegad,cbw->window,NULL,GA_Disabled,numsel<1,TAG_END);
}

static void Moveselected(struct Cabrwindow *cbw,long n,BOOL shift)
{  BOOL update=FALSE;
   ULONG sel;
   struct Node *node,*selnode;
   selnode=Getnode(&cbw->gadlist,cbw->lastselect);
   if(selnode && (!selnode->ln_Succ || !selnode->ln_Pred)) selnode=NULL;
   if(n<0)
   {  if(selnode && selnode->ln_Pred->ln_Pred)
      {  selnode=selnode->ln_Pred;
         cbw->lastselect--;
         update=TRUE;
      }
      else if(cbw->gadlist.lh_TailPred->ln_Pred)
      {  selnode=cbw->gadlist.lh_TailPred;
         cbw->lastselect=cbw->nrfiles-1;
         update=TRUE;
      }
   }
   else if(n>0)
   {  if(selnode && selnode->ln_Succ->ln_Succ)
      {  selnode=selnode->ln_Succ;
         cbw->lastselect++;
         update=TRUE;
      }
      else if(cbw->gadlist.lh_Head->ln_Succ)
      {  selnode=cbw->gadlist.lh_Head;
         cbw->lastselect=0;
         update=TRUE;
      }
   }
   if(update)
   {  for(node=cbw->gadlist.lh_Head;node->ln_Succ;node=node->ln_Succ)
      {  GetListBrowserNodeAttrs(node,LBNA_Selected,&sel,TAG_END);
         if((!shift && sel && node!=selnode)
         || (shift && sel && node==selnode))
         {  Editlbnode(cbw->listgad,cbw->window,NULL,node,LBNA_Selected,FALSE,TAG_END);
         }
         else if(node==selnode && !sel)
         {  Editlbnode(cbw->listgad,cbw->window,NULL,node,LBNA_Selected,TRUE,TAG_END);
         }
      }
      Setgadgetattrs(cbw->listgad,cbw->window,NULL,
         LISTBROWSER_MakeVisible,cbw->lastselect,
         TAG_END);
      Setgads(cbw);
   }
   else
   {  cbw->lastselect=-1;
   }
   cbw->flags&=~CBRF_WASFIND;
}

static void Sortcache(struct Cabrwindow *cbw)
{  UWORD sort=Getvalue(cbw->sortgad,CHOOSER_Active);
   Setgadgetattrs(cbw->listgad,cbw->window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   Sortnodes(cbw,sort);
   Setgadgetattrs(cbw->listgad,cbw->window,NULL,
      LISTBROWSER_Labels,&cbw->gadlist,
      LISTBROWSER_Top,0,
      TAG_END);
   Setgads(cbw);
}

static void Addbrnode(struct Cabrwindow *cbw,struct Cache *cac)
{  struct Node *newnode,*n;
   cabrsortf *f;
   Setgadgetattrs(cbw->listgad,cbw->window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   if(newnode=Allocbrnode(cbw,cac))
   {  switch(cbw->sort)
      {  case CABR_URL:
            f=(cabrsortf *)Cabrsorturl;
            break;
         case CABR_DATE:
            f=(cabrsortf *)Cabrsortdate;
            break;
         case CABR_TYPE:
            f=(cabrsortf *)Cabrsorttype;
            break;
         default:
            f=NULL;
      }
      if(f)
      {  for(n=cbw->gadlist.lh_Head;n->ln_Succ;n=n->ln_Succ)
         {  if(f(&newnode,&n)<0) break;
         }
         Insert(&cbw->gadlist,newnode,n->ln_Pred);
      }
      else AddTail(&cbw->gadlist,newnode);
      cbw->nrfiles++;
      cac->brnode=newnode;
   }
}

static void Rembrnode(struct Cabrwindow *cbw,struct Node *brnode)
{  struct Cabrinfo *ci=NULL;
   Setgadgetattrs(cbw->listgad,cbw->window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   Remove(brnode);
   GetListBrowserNodeAttrs(brnode,LBNA_UserData,&ci,TAG_END);
   if(ci) FREE(ci);
   FreeListBrowserNode(brnode);
   cbw->nrfiles--;
}

static void Updatedlist(struct Cabrwindow *cbw)
{  Setgadgetattrs(cbw->listgad,cbw->window,NULL,
      LISTBROWSER_Labels,&cbw->gadlist,
      LISTBROWSER_AutoFit,TRUE,
      TAG_END);
   Setgads(cbw);
   Setcabrstatus(cbw);
}

static void Docurrent(struct Cabrwindow *cbw,ULONG tag)
{  struct Cabrinfo *ci=NULL;
   struct Node *node;
   ULONG sel;
   if(tag==AOCBR_Delete || Getvalue(cbw->listgad,LISTBROWSER_NumSelected)==1)
   {  for(node=cbw->gadlist.lh_Head;node->ln_Succ;node=node->ln_Succ)
      {  GetListBrowserNodeAttrs(node,LBNA_Selected,&sel,LBNA_UserData,&ci,TAG_END);
         if(sel && ci && ci->url)
         {  Updatetaskattrs(tag,(Tag)ci->url,TAG_END);
         }
      }
   }
}

/* Find the first or next entry where the url contains the find string,
 * unselect all others. */
static void Findstring(struct Cabrwindow *cbw)
{  UBYTE *string=(UBYTE *)Getvalue(cbw->fstrgad,STRINGA_TextVal);
   struct Node *node=NULL;
   UBYTE *url;
   ULONG sel;
   long nsel=0;
   Setgadgetattrs(cbw->listgad,cbw->window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   if(string && *string)
   {  if(cbw->flags&CBRF_WASFIND)
      {  node=Getnode(&cbw->gadlist,cbw->lastselect);
         if(node)
         {  SetListBrowserNodeAttrs(node,LBNA_Selected,FALSE,TAG_END);
            node=node->ln_Succ;
            nsel=cbw->lastselect+1;
         }
      }
      if(!node)
      {  node=cbw->gadlist.lh_Head;
         nsel=0;
      }
      cbw->flags&=~CBRF_WASFIND;
      for(;node->ln_Succ;node=node->ln_Succ,nsel++)
      {  GetListBrowserNodeAttrs(node,
            LBNA_Selected,&sel,
            LBNA_Column,0,LBNCA_Text,&url,
            TAG_END);
         if(sel) SetListBrowserNodeAttrs(node,LBNA_Selected,FALSE,TAG_END);
         if(!(cbw->flags&CBRF_WASFIND) && url && Strstri(url,string))
         {  SetListBrowserNodeAttrs(node,LBNA_Selected,TRUE,TAG_END);
            cbw->lastselect=nsel;
            cbw->flags|=CBRF_WASFIND;
         }
      }
   }
   else cbw->flags&=~CBRF_WASFIND;
   Setgadgetattrs(cbw->listgad,cbw->window,NULL,
      LISTBROWSER_Labels,&cbw->gadlist,
      LISTBROWSER_Top,MAX(cbw->lastselect,0),
      TAG_END);
   Setgads(cbw);
}

/* Select all entries for the pattern, unselect all others */
static void Selectpattern(struct Cabrwindow *cbw)
{  UBYTE *pattern=(UBYTE *)Getvalue(cbw->pstrgad,STRINGA_TextVal);
   UBYTE parsepat[256];
   struct Node *node;
   long nsel;
   UBYTE *url,*p;
   ULONG sel;
   BOOL match,matched=FALSE,scheme;
   LONG wild;
   scheme=BOOLVAL(strstr(pattern,"://"));
   Setgadgetattrs(cbw->listgad,cbw->window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   if(pattern && *pattern
   && (wild=ParsePatternNoCase(pattern,parsepat,sizeof(parsepat)))>=0)
   {  for(node=cbw->gadlist.lh_Head,nsel=0;node->ln_Succ;node=node->ln_Succ,nsel++)
      {  GetListBrowserNodeAttrs(node,
            LBNA_Selected,&sel,
            LBNA_Column,0,LBNCA_Text,&url,
            TAG_END);
         if(wild)
         {  if(!scheme)
            {  p=strstr(url,"://");
               if(p) p+=3;
               else p=url;
            }
            else p=url;
            match=MatchPatternNoCase(parsepat,p);
         }
         else
         {  match=BOOLVAL(Strstri(url,pattern));
         }
         if(sel && !match)
         {  SetListBrowserNodeAttrs(node,LBNA_Selected,FALSE,TAG_END);
         }
         else if(!sel && match)
         {  SetListBrowserNodeAttrs(node,LBNA_Selected,TRUE,TAG_END);
            if(!matched)
            {  cbw->lastselect=nsel;
            }
         }
         matched|=match;
      }
   }
   cbw->flags&=~CBRF_WASFIND;
   Setgadgetattrs(cbw->listgad,cbw->window,NULL,
      LISTBROWSER_Labels,&cbw->gadlist,
      LISTBROWSER_Top,MAX(cbw->lastselect,0),
      TAG_END);
   Setgads(cbw);
}

/*------------------------------------------------------------------------*/

DECLARE_HOOK
(
static long __saveds , Idcmphook,
struct Hook *,hook,A0,
APTR, dummy, A2,
struct IntuiMessage *,msg,A1
)
{
    USRFUNC_INIT
  struct Cabrwindow *cbw=hook->h_Data;
   switch(msg->Class)
   {  case IDCMP_RAWKEY:
         if(msg->Qualifier&(IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) shift=TRUE;
         else shift=FALSE;
         break;
      case IDCMP_CHANGEWINDOW:
         Updatetaskattrs(AOCBR_Dimx,cbw->window->LeftEdge,
            AOCBR_Dimy,cbw->window->TopEdge,
            AOCBR_Dimw,cbw->window->Width-cbw->window->BorderLeft-cbw->window->BorderRight,
            AOCBR_Dimh,cbw->window->Height-cbw->window->BorderTop-cbw->window->BorderBottom,
            TAG_END);
         break;
   }
   return 0;

    USRFUNC_EXIT
}

static void Buildcabrowsewindow(struct Cabrwindow *cbw)
{

   NewList(&cbw->gadlist);
   NewList(&cbw->sortlist);
   columninfo[0].ci_Title=AWEBSTR(MSG_CABR_URL);
   columninfo[1].ci_Title=AWEBSTR(MSG_CABR_DATE);
   columninfo[2].ci_Title=AWEBSTR(MSG_CABR_SIZE);
   columninfo[3].ci_Title=AWEBSTR(MSG_CABR_TYPE);
   columninfo[4].ci_Title=AWEBSTR(MSG_CABR_CHARSET);
   columninfo[5].ci_Title=AWEBSTR(MSG_CABR_ETAG);
   columninfo[6].ci_Title=AWEBSTR(MSG_CABR_FILE);

   sortlabels[0]=AWEBSTR(MSG_CABR_URL);
   sortlabels[1]=AWEBSTR(MSG_CABR_DATE);
   sortlabels[2]=AWEBSTR(MSG_CABR_TYPE);
   cbw->lastselect=-1;
   idcmphook.h_Entry=(HOOKFUNC)GIVEME_HOOKENTRY(Idcmphook);
   idcmphook.h_Data=cbw;


   if(cbw->screen=LockPubScreen(cbw->screenname))
   {  ObtainSemaphore(cbw->cachesema);


      Allocnodes(cbw,&cbw->gadlist);
      Sortnodes(cbw,cbw->sort);

      ReleaseSemaphore(cbw->cachesema);

      Makesortlist(cbw);
      if(!cbw->w)
      {  cbw->x=cbw->screen->Width/4;
         cbw->y=cbw->screen->Height/4;
         cbw->w=cbw->screen->Width/2;
         cbw->h=cbw->screen->Height/2;
      }
      cbw->winobj=WindowObject,
         WA_Title,AWEBSTR(MSG_CABR_TITLE),
         WA_Left,cbw->x,
         WA_Top,cbw->y,
         WA_InnerWidth,cbw->w,
         WA_InnerHeight,cbw->h,
         WA_SizeGadget,TRUE,
         WA_DepthGadget,TRUE,
         WA_DragBar,TRUE,
         WA_CloseGadget,TRUE,
         WA_Activate,TRUE,
         WA_AutoAdjust,TRUE,
         WA_SimpleRefresh,TRUE,
         WA_PubScreen,cbw->screen,
         WA_IDCMP,IDCMP_RAWKEY,
         WINDOW_IDCMPHook,&idcmphook,
         WINDOW_IDCMPHookBits,IDCMP_RAWKEY|IDCMP_CHANGEWINDOW,
         WINDOW_Layout,cbw->toplayout=VLayoutObject,
            LAYOUT_SpaceOuter,TRUE,
            LAYOUT_DeferLayout,TRUE,
            StartMember,cbw->listgad=ListBrowserObject,
               GA_ID,CGID_LIST,
               GA_RelVerify,TRUE,
               LISTBROWSER_MultiSelect,TRUE,
               LISTBROWSER_ShowSelected,TRUE,
               LISTBROWSER_ColumnInfo,columninfo,
               LISTBROWSER_Labels,&cbw->gadlist,
               LISTBROWSER_ColumnTitles,TRUE,
               LISTBROWSER_AutoFit,TRUE,
               LISTBROWSER_HorizontalProp,TRUE,
            EndMember,
            StartMember,cbw->pagelayout=HLayoutObject,
               StartMember,HLayoutObject,
                  LAYOUT_InnerSpacing,FALSE,
                  LAYOUT_EvenSize,TRUE,
                  StartMember,cbw->findgad=ButtonObject,
                     GA_ID,CGID_FIND,
                     GA_Immediate,TRUE,
                     GA_RelVerify,TRUE,
                     GA_Text,AWEBSTR(MSG_CABR_FIND),
                     BUTTON_PushButton,TRUE,
                  EndMember,
                  StartMember,cbw->patgad=ButtonObject,
                     GA_ID,CGID_PAT,
                     GA_Immediate,TRUE,
                     GA_RelVerify,TRUE,
                     GA_Text,AWEBSTR(MSG_CABR_PATTERN),
                     BUTTON_PushButton,TRUE,
                  EndMember,
               EndMember,
               CHILD_WeightedWidth,0,
               CHILD_WeightedHeight,0,
               StartMember,cbw->pagegad=PageObject,
                  PAGE_Add,HLayoutObject,
                     StartMember,cbw->fuelgad=FuelGaugeObject,
                        FUELGAUGE_Min,0,
                        FUELGAUGE_Max,cbw->disksize*1024,
                        FUELGAUGE_Level,cbw->currentsize,
                        FUELGAUGE_Percent,TRUE,
                        FUELGAUGE_Ticks,21,
                        FUELGAUGE_ShortTicks,TRUE,
                     EndMember,
                     StartMember,cbw->statusgad=ButtonObject,
                        GA_ReadOnly,TRUE,
                        GA_Text," ",
                        BUTTON_Justification,BCJ_LEFT,
                     EndMember,
                     CHILD_WeightedHeight,0,
                  EndMember,
                  PAGE_Add,HLayoutObject,
                     StartMember,cbw->fstrgad=StringObject,
                        GA_ID,CGID_FSTRING,
                        GA_RelVerify,TRUE,
                        STRINGA_TextVal,"",
                        STRINGA_MaxChars,127,
                     EndMember,
                     CHILD_WeightedHeight,0,
                     StartMember,ButtonObject,
                        GA_ID,CGID_NEXT,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_CABR_NEXT),
                     EndMember,
                     CHILD_WeightedHeight,0,
                     CHILD_WeightedWidth,0,
                  EndMember,
                  PAGE_Add,HLayoutObject,
                     StartMember,cbw->pstrgad=StringObject,
                        GA_ID,CGID_PSTRING,
                        GA_RelVerify,TRUE,
                        STRINGA_TextVal,"",
                        STRINGA_MaxChars,127,
                     EndMember,
                     CHILD_WeightedHeight,0,
                  EndMember,
               EndMember,
            EndMember,
            CHILD_WeightedHeight,0,
            StartMember,HLayoutObject,
               StartMember,cbw->sortgad=ChooserObject,
                  GA_ID,CGID_SORT,
                  GA_RelVerify,TRUE,
                  CHOOSER_PopUp,TRUE,
                  CHOOSER_Labels,&cbw->sortlist,
                  CHOOSER_Active,cbw->sort,
               EndMember,
               MemberLabel(AWEBSTR(MSG_CABR_SORTBY)),
               StartMember,cbw->opengad=ButtonObject,
                  GA_ID,CGID_OPEN,
                  GA_Text,AWEBSTR(MSG_CABR_OPEN),
                  GA_RelVerify,TRUE,
                  GA_Disabled,TRUE,
               EndMember,
               StartMember,cbw->savegad=ButtonObject,
                  GA_ID,CGID_SAVE,
                  GA_Text,AWEBSTR(MSG_CABR_SAVE),
                  GA_RelVerify,TRUE,
                  GA_Disabled,TRUE,
               EndMember,
               StartMember,cbw->deletegad=ButtonObject,
                  GA_ID,CGID_DELETE,
                  GA_Text,AWEBSTR(MSG_CABR_DELETE),
                  GA_RelVerify,TRUE,
                  GA_Disabled,TRUE,
               EndMember,
            EndMember,
            CHILD_WeightedHeight,0,
         EndMember,
      EndWindow;
      if(cbw->winobj)
      {
      if(cbw->window=(struct Window *)RA_OpenWindow(cbw->winobj))
         {  GetAttr(WINDOW_SigMask,cbw->winobj,&cbw->winsigmask);
            Setcabrstatus(cbw);
         }
      }
   }
}

static BOOL Handlecabrowsewindow(struct Cabrwindow *cbw)
{  ULONG result,relevent;
   BOOL done=FALSE;
   UWORD click;
   while((result=RA_HandleInput(cbw->winobj,&click))!=WMHI_LASTMSG)
   {  switch(result&WMHI_CLASSMASK)
      {  case WMHI_CLOSEWINDOW:
            done=TRUE;
            break;
         case WMHI_GADGETUP:
            switch(result&WMHI_GADGETMASK)
            {  case CGID_LIST:
                  relevent=Getvalue(cbw->listgad,LISTBROWSER_RelEvent);
                  if(relevent&LBRE_DOUBLECLICK)
                  {  if(click==cbw->lastclick) Docurrent(cbw,AOCBR_Open);
                  }
                  Setgads(cbw);
                  cbw->lastclick=click;
                  cbw->lastselect=click;
                  cbw->flags&=~CBRF_WASFIND;
                  break;
               case CGID_FIND:
                  if(cbw->findgad->Flags&GFLG_SELECTED)
                  {  SetGadgetAttrs(cbw->patgad,cbw->window,NULL,
                        GA_Selected,FALSE,TAG_END);
                     SetGadgetAttrs(cbw->pagegad,cbw->window,NULL,
                        PAGE_Current,CPGC_FIND,TAG_END);
                  }
                  else
                  {  SetGadgetAttrs(cbw->pagegad,cbw->window,NULL,
                        PAGE_Current,CPGC_STATUS,TAG_END);
                  }
                  RethinkLayout(cbw->pagelayout,cbw->window,NULL,TRUE);
                  if(cbw->findgad->Flags&GFLG_SELECTED)
                  {  ActivateLayoutGadget(cbw->toplayout,cbw->window,NULL,(ULONG) cbw->fstrgad);
                  }
                  break;
               case CGID_PAT:
                  if(cbw->patgad->Flags&GFLG_SELECTED)
                  {  SetGadgetAttrs(cbw->findgad,cbw->window,NULL,
                        GA_Selected,FALSE,TAG_END);
                     SetGadgetAttrs(cbw->pagegad,cbw->window,NULL,
                        PAGE_Current,CPGC_PATTERN,TAG_END);
                  }
                  else
                  {  SetGadgetAttrs(cbw->pagegad,cbw->window,NULL,
                        PAGE_Current,CPGC_STATUS,TAG_END);
                  }
                  RethinkLayout(cbw->pagelayout,cbw->window,NULL,TRUE);
                  if(cbw->patgad->Flags&GFLG_SELECTED)
                  {  ActivateLayoutGadget(cbw->toplayout,cbw->window,NULL,(ULONG) cbw->pstrgad);
                  }
                  break;
               case CGID_FSTRING:
               case CGID_NEXT:
                  Findstring(cbw);
                  break;
               case CGID_PSTRING:
                  Selectpattern(cbw);
                  break;
               case CGID_SORT:
                  Sortcache(cbw);
                  break;
               case CGID_OPEN:
                  Docurrent(cbw,AOCBR_Open);
                  break;
               case CGID_SAVE:
                  Docurrent(cbw,AOCBR_Save);
                  break;
               case CGID_DELETE:
                  Docurrent(cbw,AOCBR_Delete);
                  break;
            }
            break;
         case WMHI_RAWKEY:
            switch(result&WMHI_GADGETMASK)
            {  case 0x45:  /* esc */
                  done=TRUE;
                  break;
               case 0x4c:  /* up */
                  Moveselected(cbw,-1,shift);
                  break;
               case 0x4d:  /* down */
                  Moveselected(cbw,1,shift);
                  break;
               case 0x43:  /* num enter */
               case 0x44:  /* enter */
                  Docurrent(cbw,AOCBR_Open);
                  break;
               case 0x42:  /* tab */
                  switch(Getvalue(cbw->pagegad,PAGE_Current))
                  {  case CPGC_FIND:
                        ActivateLayoutGadget(cbw->toplayout,cbw->window,NULL,(ULONG) cbw->fstrgad);
                        break;
                     case CPGC_PATTERN:
                        ActivateLayoutGadget(cbw->toplayout,cbw->window,NULL,(ULONG) cbw->pstrgad);
                        break;
                  }
                  break;
            }
            break;
/*
         case WMHI_CHANGEWINDOW:
            Updatetaskattrs(AOCBR_Dimx,cbw->window->LeftEdge,
               AOCBR_Dimy,cbw->window->TopEdge,
               AOCBR_Dimw,cbw->window->Width-cbw->window->BorderLeft-cbw->window->BorderRight,
               AOCBR_Dimh,cbw->window->Height-cbw->window->BorderTop-cbw->window->BorderBottom,
               TAG_END);
            break;
*/
      }
   }
   return done;
}

static void Closecabrowsewindow(struct Cabrwindow *cbw)
{  if(cbw->winobj)
   {  if(cbw->window)
      {  Updatetaskattrs(AOCBR_Dimx,cbw->window->LeftEdge,
            AOCBR_Dimy,cbw->window->TopEdge,
            AOCBR_Dimw,cbw->window->Width-cbw->window->BorderLeft-cbw->window->BorderRight,
            AOCBR_Dimh,cbw->window->Height-cbw->window->BorderTop-cbw->window->BorderBottom,
            TAG_END);
      }
      DisposeObject(cbw->winobj);
   }
   Freelbnodes(&cbw->gadlist);
   Freechnodes(&cbw->sortlist);
   if(cbw->screen) UnlockPubScreen(NULL,cbw->screen);cbw->screen=NULL;
}

#endif /* !LOCALONLY */


LIBFUNC_H1
(
    __saveds static Subtaskfunction *, GetTaskFunc,
    ULONG, id, D0,
    CACHEBROWSER_TYPE, CACHEBROWSER_NAME
)
{
    LIBFUNC_INIT

    return (Subtaskfunction *)&Cabrtask;

    LIBFUNC_EXIT
}

__saveds static void Cabrtask(struct Cabrwindow *cbw)
{
#ifndef LOCALONLY
   struct Taskmsg *cm;
   BOOL done=FALSE,updlist=FALSE;
   ULONG getmask;
   struct TagItem *tag,*tstate;

   Buildcabrowsewindow(cbw);

   if(cbw->window)
   {  while(!done)
      {  getmask=Waittask(cbw->winsigmask);
         while(!done && (cm=Gettaskmsg()))
         {  if(cm->amsg && cm->amsg->method==AOM_SET)
            {  tstate=((struct Amset *)cm->amsg)->tags;
               while(tag=NextTagItem(&tstate))
               {  switch(tag->ti_Tag)
                  {  case AOTSK_Stop:
                        if(tag->ti_Data) done=TRUE;
                        break;
                     case AOCBR_Tofront:
                        if(tag->ti_Data)
                        {  WindowToFront(cbw->window);
                           ActivateWindow(cbw->window);
                        }
                        break;
                     case AOCBR_Addobject:
                        Addbrnode(cbw,(struct Cache *)tag->ti_Data);
                        updlist=TRUE;
                        break;
                     case AOCBR_Remobject:
                        Rembrnode(cbw,(struct Node *)tag->ti_Data);
                        updlist=TRUE;
                        break;
                     case AOCBR_Disksize:
                        cbw->currentsize=tag->ti_Data;
                        break;
                  }
               }
            }
            Replytaskmsg(cm);
         }
         if(updlist)
         {  Updatedlist(cbw);
            Delay(20);
            updlist=FALSE;
         }
         if(!done && (getmask&cbw->winsigmask))
         {  done=Handlecabrowsewindow(cbw);
         }
      }
      Clearbrnodes(cbw);
      Closecabrowsewindow(cbw);
   }
#endif
   Updatetaskattrs(AOTSK_Async,TRUE,
      AOCBR_Close,TRUE,
      TAG_END);
}
