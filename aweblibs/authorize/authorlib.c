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

/* authorlib.c - AWeb authorize AwebLib module */

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include "aweblib.h"
#include "application.h"
#include "task.h"
#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"

#include <exec/resident.h>
#include <intuition/intuition.h>
#include <reaction/reaction.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#include "libraries/awebauthorize.h"

#ifdef __MORPHOS__
ULONG __abox__=1;
#endif

#define AUTHORIZE_VERSION 36
#define AUTHORIZE_REVISION 3
#define AUTHORIZE_VERSTRING "36.3"

enum AUTHGADGET_IDS
{  AGID_OK=1,AGID_CANCEL,
};

struct Aewinfo    /* UserData */
{  struct Authnode *authnode;
   UBYTE *server;
   UBYTE *userid;       /* Also buffer */
   UBYTE *password;     /* points into userid buffer */
   struct Node *node;   /* LB node */
};

enum AUTHEDIT_GADGETIDS
{  EGID_LIST=1,EGID_DEL,EGID_USERID,EGID_PASSWORD,EGID_SHOWPASS
};

static struct ColumnInfo columninfo[]=
{  { 34,NULL, CIF_SORTABLE },
   { 33,NULL, CIF_SORTABLE },
   { -1,NULL, CIF_SORTABLE },
   { -1,NULL,0 }
};

static UWORD column=3,order=1;
static UBYTE *title[3];
static UBYTE buf[128];
static BOOL hasstring45;

static struct Hook idcmphook;

struct Library *AwebSupportBase;
struct ExecBase *SysBase;
struct DosLibrary    *DOSBase;
struct IntuitionBase *IntuitionBase;
struct Library *WindowBase,*LayoutBase,*ButtonBase,*ListBrowserBase,
   *StringBase,*ChooserBase,*CheckBoxBase,*SpaceBase,*LabelBase,*GlyphBase;

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
struct CheckBoxIFace *ICheckBox;
struct SpaceIFace *ISpace;
struct LabelIFace *ILabel;
struct StringIFace *IString;
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

static inline __saveds struct SegList *Real_Closelib(LIBMAN_TYPE LIBMAN_NAME);
LIBFUNC_P0
(
 __saveds struct SegList * , Closelib,
 LIBMAN_TYPE, LIBMAN_NAME
);

static inline __saveds struct SegList *Real_Expungelib(LIBMAN_TYPE LIBMAN_NAME);
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

LIBFUNC_P2
(
__saveds void  , Authorreq,
struct Authorreq *,areq,A0,
BOOL, onlypw, D0,
AUTHORIZE_TYPE, AUTHORIZE_NAME
);

LIBFUNC_P1
(
    __saveds Subtaskfunction *, AuthGetTaskFunc,
    ULONG, id, D0,
    AUTHORIZE_TYPE, AUTHORIZE_NAME
);

__saveds static void Authedittask(struct Authedit *aew);

LIBFUNC_P3
(
__saveds void  , Authorset,
struct Authorize *,auth,A0,
UBYTE *,userid,A1,
UBYTE *,passwd,A2,
AUTHORIZE_TYPE, AUTHORIZE_NAME

);




/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct Library *AwebAuthorizeBase;

static APTR libseglist;

struct ExecBase *SysBase;


LIBSTART_DUMMY

static char __aligned libname[]="authorize.aweblib";
static char __aligned libid[]="$VER: authorize.aweblib " AUTHORIZE_VERSTRING " (" __AMIGADATE__ ") " CPU;

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
        (void *)AuthGetTaskFunc,
        (void *)Authorreq,
        (void *)Authorset,
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
static struct Resident lib_res __attribute((used))  =
{
    RTC_MATCHWORD,
    &lib_res,
    &lib_res+1,
    RTF_NATIVE|RTF_AUTOINIT,
    AUTHORIZE_VERSION,
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
   AuthGetTaskFunc,
   Authorreq,
   Authorset,
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
                RTF_AUTOINIT | RTF_PPC,
#else
   RTF_AUTOINIT,
#endif
   AUTHORIZE_VERSION,
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

    USRFUNC_INIT

    struct ExecIFace *IExec
#ifdef __GNUC__
        __attribute__((unused))
#endif
        = (struct ExecIFace *)exec;


    libBase->lib_Node.ln_Type = NT_LIBRARY;
    libBase->lib_Node.ln_Pri  = 0;
    libBase->lib_Node.ln_Name = libname;
    libBase->lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
    libBase->lib_Version      = AUTHORIZE_VERSION;
    libBase->lib_Revision     = AUTHORIZE_REVISION;
    libBase->lib_IdString     = libid;

    libseglist = seglist;

    if(!Initaweblib((struct Library *)libBase))
    {
        Expungeaweblib((struct Library *)libBase);
        libBase = NULL;
    }

       return (struct Library *)libBase;

       USRFUNC_EXIT
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
   AwebAuthorizeBase=libbase;
   libbase->lib_Revision=AUTHORIZE_REVISION;
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
   struct Library *Authorizelibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Authorizelibbase = (struct Library *)LIBMAN_NAME;
#endif

   Authorizelibbase->lib_OpenCnt++;
   Authorizelibbase->lib_Flags&=~LIBF_DELEXP;
   if(Authorizelibbase->lib_OpenCnt==1)
   {
      if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return (struct Library *) Real_Closelib(LIBMAN_NAME);
#if defined (__amigaos4__)
      if(!( IAwebSupport = (struct AwebSupportIface *)GetInterface(AwebSupportBase,"main",1,0))) return Real_Closelib(LIBMAN_NAME);
#endif
   }
   return Authorizelibbase;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds struct SegList * , Closelib,
LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT
                return Real_Closelib( LIBMAN_NAME);
                LIBFUNC_EXIT
}

static inline __saveds struct SegList *Real_Closelib(LIBMAN_TYPE LIBMAN_NAME)
{
#if defined(__amigaos4__)
   struct Library *Authorizelibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Authorizelibbase = (struct Library *)LIBMAN_NAME;
#endif


  Authorizelibbase->lib_OpenCnt--;
   if(Authorizelibbase->lib_OpenCnt==0)
   {  if(AwebSupportBase)
      {
#if defined(__amigaos4__)
    if(IAwebSupport) DropInterface((struct Interface *)IAwebSupport);
    IAwebSupport = NULL;
#endif

         CloseLibrary(AwebSupportBase);
         AwebSupportBase=NULL;
      }
      if(Authorizelibbase->lib_Flags&LIBF_DELEXP)
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

static inline __saveds struct SegList *Real_Expungelib(LIBMAN_TYPE LIBMAN_NAME)
{
#if defined(__amigaos4__)
   struct Library *Authorizelibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Authorizelibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Authorizelibbase->lib_OpenCnt==0)
   {  ULONG size=Authorizelibbase->lib_NegSize+Authorizelibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Authorizelibbase-Authorizelibbase->lib_NegSize;
      Remove((struct Node *)Authorizelibbase);
      Expungeaweblib(Authorizelibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Authorizelibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Authorizelibbase->lib_Flags|=LIBF_DELEXP;
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
   if(!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",39))) return FALSE;
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

#if defined(__amigaos4__)
   if(!(IDOS       = (struct DOSIFace *)        GetInterface(DOSBase,"main",1,0))) return FALSE;
   if(!(IIntuition = (struct IntuitionIFacee *) GetInterface(IntuitionBase,"main",1,0))) return FALSE;
   if(!(IUtility   = (struct UtilityIFace *)    GetInterface(UtilityBase,"main",1,0))) return FALSE;
   if(!(IWindow = (struct WindowIFace *)GetInterface(WindowBase,"main",1,0))) return FALSE;
   if(!(ILayout = (struct LayoutIFace *)GetInterface(LayoutBase,"main",1,0))) return FALSE;
   if(!(IButton = (struct ButtonIFace *)GetInterface(ButtonBase,"main",1,0))) return FALSE;
   if(!(IListBrowser = (struct ListBrowserIFace *)GetInterface(ListBrowserBase,"main",1,0))) return FALSE;
   if(!(IString = (struct StribngIFace *)GetInterface(StringBase,"main",1,0))) return FALSE;
   if(!(IChooser = (struct ChooserIFace *)GetInterface(ChooserBase,"main",1,0))) return FALSE;
   if(!(ICheckBox = (struct CheckBoxIFace *)GetInterface(CheckBoxBase,"main",1,0))) return FALSE;
   if(!(ISpace = (struct SpaceIFace *)GetInterface(SpaceBase,"main",1,0))) return FALSE;
   if(!(ILabel = (struct LabelIFace *)GetInterface(LabelBase,"main",1,0))) return FALSE;
   if(!(IGlyph = (struct LabrelIFace *)GetInterface(GlyphBase,"main",1,0))) return FALSE;

#endif

   // we test here if string.gadget is at least Version 45
   // because up to Version 44.2 it doesn't work correct.
   if(StringBase->lib_Version > 44)
   { hasstring45 = TRUE;
   }
   else hasstring45 = FALSE;

   return TRUE;
}

static void Expungeaweblib(struct Library *libbase)
{
#if defined(__amigaos4__)

   if(IGlyph)DropInterface((struct Interface *)IGlyph);
   if(ILabel)DropInterface((struct Interface *)ILabel);
   if(ISpace)DropInterface((struct Interface *)ISpace);
   if(IChooser)DropInterface((struct Interface *)IChooser);
   if(ICheckBox)DropInterface((struct Interface *)ICheckBox);
   if(IString)DropInterface((struct Interface *)IString);
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
   if(ChooserBase) CloseLibrary(ChooserBase);
   if(CheckBoxBase) CloseLibrary(CheckBoxBase);
   if(StringBase) CloseLibrary(StringBase);
   if(ListBrowserBase) CloseLibrary(ListBrowserBase);
   if(ButtonBase) CloseLibrary(ButtonBase);
   if(LayoutBase) CloseLibrary(LayoutBase);
   if(WindowBase) CloseLibrary(WindowBase);
   if(UtilityBase) CloseLibrary((struct Library *) UtilityBase);
   if(IntuitionBase) CloseLibrary((struct Library *) IntuitionBase);
   if(DOSBase) CloseLibrary((struct Library *) DOSBase);

   AwebModuleExit();
}

/*-----------------------------------------------------------------------*/

VARARGS68K_DECLARE(static void Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...))
{
   VA_LIST va;
   struct TagItem *tags;
   VA_STARTLIN(va,req);
   tags = (struct TagItem *)VA_GETLIN(va,struct TagItem *);
   if(SetGadgetAttrsA(gad,win,req,tags)) RefreshGList(gad,win,req,1);
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

static UBYTE base64[]=
   "ABCDEFGHIJKLMNOP"
   "QRSTUVWXYZabcdef"
   "ghijklmnopqrstuv"
   "wxyz0123456789+/";

/* Encodes userid:password into Authorize structure */
static void Bakecookie(struct Authorize *auth,UBYTE *userid,UBYTE *password)
{  UBYTE *cookie,*temp;
   long length1=strlen(userid)+1+strlen(password);
   long length2=length1*4/3+5;
   UBYTE *src,*dest,c=0;
   short t;
   if(temp=ALLOCTYPE(UBYTE,length1+1,0))
   {  strcpy(temp,userid);
      strcat(temp,":");
      strcat(temp,password);
      if(cookie=ALLOCTYPE(UBYTE,length2,MEMF_PUBLIC))
      {  src=temp;
         dest=cookie;
         t=0;
         while(*src)
         {  switch(t)
            {  case 0:  /* xxxxxx.. */
                  c=(src[0]&0xfc)>>2;
                  break;
               case 1:  /* ......xx xxxx.... */
                  c=((src[0]&0x03)<<4) | ((src[1]&0xf0)>>4);
                  src++;
                  break;
               case 2:  /* ....xxxx xx...... */
                  c=((src[0]&0x0f)<<2) | ((src[1]&0xc0)>>6);
                  src++;
                  break;
               case 3:  /* ..xxxxxx */
                  c=(src[0]&0x3f);
                  src++;
                  break;
            }
            *dest++=base64[c];
            if(++t>3) t=0;
         }
         if(t)
         {  while(t++<4) *dest++='=';
         }
         *dest='\0';
         if(auth->cookie) FREE(auth->cookie);
         auth->cookie=cookie;
      }
      FREE(temp);
   }
}

/* Returns 6bits data, or 0xff if invalid. */
static UBYTE Unbase64(UBYTE c)
{  if(c>='A' && c<='Z') c-=0x41;
   else if(c>='a' && c<='z') c-=0x47;
   else if(c>='0' && c<='9') c+=4;
   else if(c=='+') c=0x3e;
   else if(c=='/') c=0x3f;
   else c=0xff;
   return c;
}

/* Decodes cookie from Authorize. Creates dynamic string "userid\0password"*/
/* If replace == FALSE then the ":" isn't replaced by a "\0" */
static UBYTE *Solvecookie(struct Authorize *auth, BOOL replace)
{  UBYTE *temp,*src,*dest,*p;
   UBYTE c;
   short t;
   long len=strlen(auth->cookie)  /* *3/4+5; */   *2;
   if(temp=ALLOCTYPE(UBYTE,len,MEMF_CLEAR))
   {  src=auth->cookie;
      dest=temp;
      t=0;
      while(*src)
      {  c=Unbase64(*src);
         if(c==0xff) break;
         switch(t)
         {  case 0:  /* xxxxxx.. */
               *dest=c<<2;
               break;
            case 1:  /* ......xx xxxx.... */
               *dest|=(c>>4)&0x03;
               dest++;
               *dest=c<<4;
               break;
            case 2:  /* ....xxxx xx...... */
               *dest|=(c>>2)&0x0f;
               dest++;
               *dest=c<<6;
               break;
            case 3:  /* ..xxxxxx */
               *dest|=c&0x3f;
               dest++;
               break;
         }
         if(++t>3) t=0;
         src++;
      }
      if(replace)
      {
         if(p=strchr(temp,':'))
         {  *p='\0';
         }
      }
   }
   return temp;
}

static void Freeauthinfo(struct Aewinfo *ai)
{  if(ai)
   {  if(ai->userid) FREE(ai->userid);
      if(ai->server) FREE(ai->server);
      FREE(ai);
   }
}

static struct Aewinfo *Newauthinfo(struct Authnode *an)
{  struct Aewinfo *ai=NULL;
   struct Authorize *auth=&an->auth;
   long len;
   if(auth->server && auth->realm && auth->cookie)
   {  if(ai=ALLOCSTRUCT(Aewinfo,1,MEMF_CLEAR))
      {  ai->authnode=an;
         len=strlen(auth->server)+strlen(auth->realm)+4;
         if(ai->server=ALLOCTYPE(UBYTE,len,0))
         {  sprintf(ai->server,"%s (%s)",auth->server,auth->realm);
         }
         if(ai->userid=Solvecookie(auth, TRUE))
         {  ai->password=ai->userid+strlen(ai->userid)+1;
         }
         if(!ai->server || !ai->userid)
         {  Freeauthinfo(ai);
            ai=NULL;
         }
      }
   }
   return ai;
}

static void Freelbnodes(struct List *list)
{  struct Node *node;
   struct Aewinfo *ai;
   while(node=RemHead(list))
   {  ai=NULL;
      GetListBrowserNodeAttrs(node,LBNA_UserData,&ai,TAG_END);
      Freeauthinfo(ai);
      FreeListBrowserNode(node);
   }
}

static struct Node *Allocaewnode(struct Authedit *aew,struct Authnode *an)
{  struct Node *node=NULL;
   struct Aewinfo *ai;
   if(ai=Newauthinfo(an))
   {  if(node=AllocListBrowserNode(4,
         LBNA_UserData,ai,
         LBNA_Column,0,
            LBNCA_Text,ai->server,
         LBNA_Column,1,
            LBNCA_Text,ai->userid,
         LBNA_Column,2,
            LBNCA_Text,ai->password,
         TAG_END))
      {  ai->node=node;
      }
      else
      {  Freeauthinfo(ai);
      }
   }
   return node;
}

/* returns number of selected node */
static long Allocnodes(struct Authedit *aew,struct List *list,struct Authnode *select)
{  struct Node *node;
   struct Authnode *an;
   long selected=-1,n;
   ObtainSemaphore(aew->authsema);
   for(an=aew->auths->first,n=0;an->next;an=an->next,n++)
   {  if(an->auth.cookie)
      {  if(node=Allocaewnode(aew,an))
         {  AddTail(list,node);
            if(an==select) selected=n;
         }
      }
   }
   ReleaseSemaphore(aew->authsema);
   return selected;
}

static struct Aewinfo *Selectedinfo(struct Authedit *aew)
{  long selected=Getvalue(aew->listgad,LISTBROWSER_Selected);
   struct Node *node=Getnode(&aew->gadlist,selected);
   struct Aewinfo *ai=NULL;
   if(node) GetListBrowserNodeAttrs(node,LBNA_UserData,&ai,TAG_END);
   return ai;
}

static void Setgads(struct Authedit *aew)
{  struct Aewinfo *ai=Selectedinfo(aew);
   BOOL disableui=!ai,disablepw=!ai;

   if(ai && !strncmp(ai->server,"Master",5))
   {  disableui=TRUE;
      if(aew->showpass) disablepw=FALSE;
      else disablepw=TRUE;
   }
   Setgadgetattrs(aew->servergad,aew->window,NULL,
      GA_Text,ai?ai->server:NULLSTRING,
      TAG_END);
   Setgadgetattrs(aew->delgad,aew->window,NULL,
      GA_Disabled,disablepw,
      TAG_END);
   Setgadgetattrs(aew->useridgad,aew->window,NULL,
      STRINGA_TextVal,ai?ai->userid:NULLSTRING,
      STRINGA_DispPos,0,
      GA_Disabled,disableui,
      TAG_END);
   Setgadgetattrs(aew->passwordgad,aew->window,NULL,
      STRINGA_TextVal,ai?ai->password:NULLSTRING,
      STRINGA_DispPos,0,
      GA_Disabled,disablepw,
      TAG_END);
}

/* List has changed. Build new gadgetlist, but remember selection */
static void Modifiedlist(struct Authedit *aew)
{  long selected;
   struct Aewinfo *ai=Selectedinfo(aew);
   struct Authnode *anselect=NULL;
   if(ai) anselect=ai->authnode;
   Setgadgetattrs(aew->listgad,aew->window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   Freelbnodes(&aew->gadlist);
   selected=Allocnodes(aew,&aew->gadlist,anselect);
   Setgadgetattrs(aew->listgad,aew->window,NULL,
      LISTBROWSER_Labels,&aew->gadlist,
      LISTBROWSER_Selected,selected,
      LISTBROWSER_MakeVisible,selected,
      TAG_END);
   Setgads(aew);
}

static void Delentry(struct Authedit *aew)
{  struct Aewinfo *ai=Selectedinfo(aew);
   if(ai)
   {  Updatetaskattrs(
         AOAEW_Authnode,(Tag)ai->authnode,
         AOAEW_Delete,TRUE,
         TAG_END);
      Setgadgetattrs(aew->listgad,aew->window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      Freelbnodes(&aew->gadlist);
      Allocnodes(aew,&aew->gadlist,NULL);
      Setgadgetattrs(aew->listgad,aew->window,NULL,
         LISTBROWSER_Labels,&aew->gadlist,
         LISTBROWSER_AutoFit,TRUE,
         LISTBROWSER_Selected,-1,
         TAG_END);
      Setgads(aew);
   }
}

static void Changeentry(struct Authedit *aew)
{  struct Aewinfo *ai=Selectedinfo(aew);
   UBYTE *userid,*password;
   struct Authorize auth={0};
   struct Authnode *anselect;
   long selected;
   if(ai)
   {  userid=(UBYTE *)Getvalue(aew->useridgad,STRINGA_TextVal);
      password=(UBYTE *)Getvalue(aew->passwordgad,STRINGA_TextVal);
      Bakecookie(&auth,userid,password);
      Updatetaskattrs(
         AOAEW_Authnode,(Tag)ai->authnode,
         AOAEW_Change,(Tag)&auth,
         TAG_END);
      anselect=ai->authnode;
      Setgadgetattrs(aew->listgad,aew->window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      Freelbnodes(&aew->gadlist);
      selected=Allocnodes(aew,&aew->gadlist,anselect);
      Setgadgetattrs(aew->listgad,aew->window,NULL,
         LISTBROWSER_Labels,&aew->gadlist,
         LISTBROWSER_Selected,selected,
         LISTBROWSER_MakeVisible,selected,
         LISTBROWSER_AutoFit,TRUE,
         TAG_END);
   }
}

static void Moveselected(struct Authedit *aew,short n)
{  long selected;
   selected=Getvalue(aew->listgad,LISTBROWSER_Selected);
   if(n<0)
   {  if(selected>0)
      {  selected--;
      }
      else
      {  selected=Getvalue(aew->listgad,LISTBROWSER_TotalNodes)-1;
      }
   }
   else if(n>0)
   {  struct Node *node=Getnode(&aew->gadlist,selected);
      if(node && node->ln_Succ->ln_Succ)
      {  selected++;
      }
      else
      {  selected=0;
      }
   }
   Setgadgetattrs(aew->listgad,aew->window,NULL,
      LISTBROWSER_Selected,selected,
      LISTBROWSER_MakeVisible,selected,
      TAG_END);
   Setgads(aew);
}

/* Sort the attached list and remember selection if rem == TRUE */
static void Sortlist(struct Authedit *aew, BOOL rem)
{
   ULONG selected=0;

   if(rem) GetAttr(LISTBROWSER_SelectedNode, aew->listgad, &selected);
   DoGadgetMethod(aew->listgad, aew->window, NULL, LBM_SORT, NULL, column, order, 0);

   if(selected && rem)
   {
      Setgadgetattrs(aew->listgad,aew->window,NULL,
         LISTBROWSER_SelectedNode,selected,
         TAG_END);
   }
}

static void Setshowpass(struct Authedit *aew)
{
   void * newpwgad;
   struct TextFont *screenfont;
   struct Aewinfo *ai=Selectedinfo(aew);

   Setgadgetattrs(aew->listgad,aew->window,NULL,
      LISTBROWSER_Labels,~0,
      TAG_END);
   columninfo[2].ci_Width=(aew->showpass)? 33: -1;
   Setgadgetattrs(aew->listgad,aew->window,NULL,
      LISTBROWSER_Labels,&aew->gadlist,
      TAG_END);

   if(hasstring45)
   {  // Since string.gadget doesn't support to change the STRINGA_HookType but
      // we want to display the passwords in the stringadget too  we create a new
      // StringObject and replace the old StringObject with the new StringObject.
      // The old StringObject will be disposed by string.gadget itself.
      newpwgad=StringObject,
         GA_ID,EGID_PASSWORD,
         GA_RelVerify,TRUE,
         GA_TabCycle,TRUE,
         STRINGA_TextVal,(ai)?ai->password:NULLSTRING,
         STRINGA_MaxChars,127,
         (aew->showpass)? TAG_IGNORE: STRINGA_HookType,SHK_PASSWORD,
         GA_Disabled,(ai)?FALSE:TRUE,
         End;
      if(newpwgad)
      {  // use intuition.library SetGadgetAttrs() here!
         if(SetGadgetAttrs(aew->vlayout,aew->window,NULL,
            LAYOUT_ModifyChild, aew->passwordgad,
            CHILD_ReplaceObject,newpwgad,
            TAG_END))
         {
            RethinkLayout(aew->vlayout,aew->window,NULL,TRUE);
         }
         aew->passwordgad=newpwgad;
      }
   }
   else
   {
      Agetattrs(Aweb(), AOAPP_Screenfont,(Tag)&screenfont, TAG_END);
      Setgadgetattrs(aew->passwordgad,aew->window,NULL,
         STRINGA_Font,(aew->showpass)?screenfont:aew->pwfont,
         GA_Disabled,(ai)?FALSE:TRUE,
         TAG_END);
   }
   Setgadgetattrs(aew->showpassgad,aew->window,NULL,
      GA_Selected,aew->showpass,
      TAG_END);
   Setgadgetattrs(aew->delgad,aew->window,NULL,
      GA_Disabled,(ai)?FALSE:TRUE,
      TAG_END);
}

DECLARE_HOOK
(
static long __saveds , Idcmphook,
struct Hook *,hook,A0,
APTR, Dummy, A2,
struct IntuiMessage *,msg,A1
)
{
    USRFUNC_INIT
  struct Authedit *aew=hook->h_Data;
   switch(msg->Class)
   {  case IDCMP_CHANGEWINDOW:
         Updatetaskattrs(AOAEW_Dimx,aew->window->LeftEdge,
            AOAEW_Dimy,aew->window->TopEdge,
            AOAEW_Dimw,aew->window->Width-aew->window->BorderLeft-aew->window->BorderRight,
            AOAEW_Dimh,aew->window->Height-aew->window->BorderTop-aew->window->BorderBottom,
            TAG_END);
         break;
   }
   return 0;

    USRFUNC_EXIT
}

/*-----------------------------------------------------------------------*/

static void Buildautheditwindow(struct Authedit *aew)
{  NewList(&aew->gadlist);
   title[0]=AWEBSTR(MSG_AUTHEDIT_TITLE_SERVER);
   title[1]=AWEBSTR(MSG_AUTHEDIT_TITLE_USERID);
   title[2]=AWEBSTR(MSG_AUTHEDIT_TITLE_PASSWORD);
   columninfo[0].ci_Title=title[0];
   columninfo[1].ci_Title=title[1];
   columninfo[2].ci_Title=title[2];
   columninfo[2].ci_Width=-1;
   idcmphook.h_Entry=(HOOKFUNC) GIVEME_HOOKENTRY(Idcmphook);
   idcmphook.h_Data=aew;
   if(aew->screen=LockPubScreen(aew->screenname))
   {  ObtainSemaphore(aew->authsema);
      Allocnodes(aew,&aew->gadlist,NULL);
      ReleaseSemaphore(aew->authsema);
      if(!aew->w)
      {  aew->x=aew->screen->Width/4;
         aew->y=aew->screen->Height/4;
         aew->w=aew->screen->Width/2;
         aew->h=aew->screen->Height/2;
      }
      aew->winobj=WindowObject,
         WA_Title,AWEBSTR(MSG_AUTHEDIT_TITLE),
         WA_Left,aew->x,
         WA_Top,aew->y,
         WA_InnerWidth,aew->w,
         WA_InnerHeight,aew->h,
         WA_SizeGadget,TRUE,
         WA_DepthGadget,TRUE,
         WA_DragBar,TRUE,
         WA_CloseGadget,TRUE,
         WA_Activate,TRUE,
         WA_AutoAdjust,TRUE,
         WA_SimpleRefresh,TRUE,
         WA_PubScreen,aew->screen,
         WA_IDCMP,IDCMP_RAWKEY,
         WINDOW_IDCMPHook,&idcmphook,
         WINDOW_IDCMPHookBits,IDCMP_CHANGEWINDOW,
         WINDOW_Layout,aew->toplayout=VLayoutObject,
            LAYOUT_SpaceOuter,TRUE,
            LAYOUT_DeferLayout,TRUE,
            StartMember,aew->listgad=ListBrowserObject,
               GA_ID,EGID_LIST,
               GA_RelVerify,TRUE,
               LISTBROWSER_ShowSelected,TRUE,
               LISTBROWSER_ColumnInfo,columninfo,
               LISTBROWSER_Labels,&aew->gadlist,
               LISTBROWSER_ColumnTitles,TRUE,
               LISTBROWSER_AutoFit,TRUE,
               LISTBROWSER_HorizontalProp,TRUE,
               LISTBROWSER_TitleClickable,TRUE,
            EndMember,
            StartMember,HLayoutObject,
               StartMember,aew->vlayout=VLayoutObject,
                  StartMember,aew->servergad=ButtonObject,
                     GA_ReadOnly,TRUE,
                     GA_Text,"",
                     BUTTON_Justification,BCJ_LEFT,
                     GA_Underscore,0,
                  EndMember,
                  MemberLabel(AWEBSTR(MSG_AUTHEDIT_SERVER)),
                  StartMember,aew->useridgad=StringObject,
                     GA_ID,EGID_USERID,
                     GA_RelVerify,TRUE,
                     GA_TabCycle,TRUE,
                     STRINGA_TextVal,"",
                     STRINGA_MaxChars,127,
                     GA_Disabled,TRUE,
                  EndMember,
                  MemberLabel(AWEBSTR(MSG_AUTHEDIT_USERID)),
                  StartMember,aew->passwordgad=StringObject,
                     GA_ID,EGID_PASSWORD,
                     GA_RelVerify,TRUE,
                     GA_TabCycle,TRUE,
                     STRINGA_TextVal,"",
                     STRINGA_MaxChars,127,
                     (hasstring45)?  TAG_IGNORE: STRINGA_Font,aew->pwfont,
                     (!hasstring45)? TAG_IGNORE: STRINGA_HookType,SHK_PASSWORD,
                     GA_Disabled,TRUE,
                  EndMember,
                  MemberLabel(AWEBSTR(MSG_AUTHEDIT_PASSWORD)),
                  StartMember,aew->showpassgad=CheckBoxObject,
                     GA_ID,EGID_SHOWPASS,
                     GA_RelVerify,TRUE,
                     GA_TabCycle,TRUE,
                  EndMember,
                  MemberLabel(AWEBSTR(MSG_AUTHEDIT_SHOWPASS)),
               EndMember,
               StartMember,VLayoutObject,
                  StartMember,aew->delgad=ButtonObject,
                     GA_ID,EGID_DEL,
                     GA_RelVerify,TRUE,
                     GA_Text,AWEBSTR(MSG_AUTHEDIT_DEL),
                     GA_Disabled,TRUE,
                  EndMember,
                  CHILD_WeightedHeight,0,
               EndMember,
               CHILD_WeightedWidth,0,
            EndMember,
            CHILD_WeightedHeight,0,
         EndMember,
      EndWindow;
      if(aew->winobj)
      {  if(aew->window=(struct Window *)RA_OpenWindow(aew->winobj))
         {  GetAttr(WINDOW_SigMask,aew->winobj,&aew->winsigmask);
         }
      }
   }
}

static BOOL Handleautheditwindow(struct Authedit *aew)
{  ULONG result;
   BOOL done=FALSE;
   UWORD click;
   UWORD newcolumn;
   UBYTE *ptr=NULL;

   while((result=RA_HandleInput(aew->winobj,&click))!=WMHI_LASTMSG)
   {  switch(result&WMHI_CLASSMASK)
      {  case WMHI_CLOSEWINDOW:
            done=TRUE;
            break;
         case WMHI_GADGETUP:
            switch(result&WMHI_GADGETMASK)
            {  case EGID_LIST:
                  Setgads(aew);
                  if(Getvalue(aew->listgad,LISTBROWSER_RelEvent) == LBRE_TITLECLICK)
                  {
                     newcolumn = Getvalue(aew->listgad,LISTBROWSER_RelColumn);
                     Setgadgetattrs(aew->listgad,aew->window,NULL,LISTBROWSER_Labels,~0,TAG_END);
                     /* restore the content of the old column */
                     if(column != newcolumn)
                     {
                        columninfo[column].ci_Title=title[column];
                        order = 1;
                     }
                     /* copy the original title to buf and add the order signs */
                     strcpy(buf,columninfo[newcolumn].ci_Title);
                     ptr = strrchr(buf,' ');
                     if(ptr) *ptr='\0';

                     if(order == 1)
                     {
                        order = 0;
                        strcat(buf," \\/");
                     }
                     else
                     {
                        order = 1;
                        strcat(buf," /\\");

                     }
                     columninfo[newcolumn].ci_Title=buf;
                     Setgadgetattrs(aew->listgad,aew->window,NULL,
                           LISTBROWSER_Labels,&aew->gadlist,
                           TAG_END);
                     column = newcolumn;
                     Sortlist(aew, TRUE);
                  }
                  break;
               case EGID_DEL:
                  Delentry(aew);
                  Sortlist(aew, FALSE);
                  break;
               case EGID_USERID:
               case EGID_PASSWORD:
                  Changeentry(aew);
                  Sortlist(aew, TRUE);
                  break;
               case EGID_SHOWPASS:
                  if(aew->showpass == TRUE)
                  {
                     aew->showpass = FALSE;
                     Setshowpass(aew);
                  }
                  else
                  {  /* disable the authedit window while the requester is open */
                     Setgadgetattrs(aew->toplayout,aew->window,NULL,
                           GA_Disabled,TRUE,
                           TAG_END);
                     Updatetaskattrs(AOAEW_Domasterpw,TRUE, TAG_END);
                  }
                  Setgads(aew);
                  break;
            }
            break;
         case WMHI_RAWKEY:
            switch(result&WMHI_GADGETMASK)
            {  case 0x45:  /* esc */
                  done=TRUE;
                  break;
               case 0x4c:  /* up */
                  Moveselected(aew,-1);
                  break;
               case 0x4d:  /* down */
                  Moveselected(aew,1);
                  break;
            }
            break;
/*
         case WMHI_CHANGEWINDOW:
            Updatetaskattrs(AOAEW_Dimx,aew->window->LeftEdge,
               AOAEW_Dimy,aew->window->TopEdge,
               AOAEW_Dimw,aew->window->Width-aew->window->BorderLeft-aew->window->BorderRight,
               AOAEW_Dimh,aew->window->Height-aew->window->BorderTop-aew->window->BorderBottom,
               TAG_END);
            break;
*/
      }
   }
   return done;
}

static void Closeautheditwindow(struct Authedit *aew)
{  if(aew->winobj)
   {  if(aew->window)
      {  Updatetaskattrs(AOAEW_Dimx,aew->window->LeftEdge,
            AOAEW_Dimy,aew->window->TopEdge,
            AOAEW_Dimw,aew->window->Width-aew->window->BorderLeft-aew->window->BorderRight,
            AOAEW_Dimh,aew->window->Height-aew->window->BorderTop-aew->window->BorderBottom,
            TAG_END);
      }
      DisposeObject(aew->winobj);
   }
   if(aew->screen) UnlockPubScreen(NULL,aew->screen);aew->screen=NULL;
   Freelbnodes(&aew->gadlist);
}

LIBFUNC_H1
(
    __saveds Subtaskfunction *, AuthGetTaskFunc,
    ULONG, id, D0,
    AUTHORIZE_TYPE, AUTHORIZE_NAME
)
{
    LIBFUNC_INIT

    return (Subtaskfunction *)&Authedittask;

    LIBFUNC_EXIT
}

__saveds static void Authedittask(struct Authedit *aew)
{
   struct Taskmsg *em;
   BOOL done=FALSE;
   ULONG getmask;
   struct TagItem *tag,*tstate;
   Buildautheditwindow(aew);
   if(aew->window)
   {  while(!done)
      {  getmask=Waittask(aew->winsigmask);
         while(!done && (em=Gettaskmsg()))
         {  if(em->amsg && em->amsg->method==AOM_SET)
            {  tstate=((struct Amset *)em->amsg)->tags;
               while(tag=NextTagItem(&tstate))
               {  switch(tag->ti_Tag)
                  {  case AOTSK_Stop:
                        if(tag->ti_Data) done=TRUE;
                        break;
                     case AOAEW_Tofront:
                        if(tag->ti_Data)
                        {  WindowToFront(aew->window);
                           ActivateWindow(aew->window);
                        }
                        break;
                     case AOAEW_Modified:
                        if(tag->ti_Data)
                        {
                           Modifiedlist(aew);
                           /* sort the list only if they was sorted before */
                           if(column < 3) Sortlist(aew, TRUE);
                        }
                        break;
                     case AOAEW_Showpass:
                        Modifiedlist(aew);
                        /* sort the list only if they was sorted before */
                        if(column < 3) Sortlist(aew, TRUE);
                        aew->showpass=tag->ti_Data;
                        Setshowpass(aew);
                        break;
                  }
               }
            }
            Replytaskmsg(em);
         }
         if(!done && (getmask&aew->winsigmask))
         {  done=Handleautheditwindow(aew);
         }
      }
      Closeautheditwindow(aew);
   }
   Updatetaskattrs(AOTSK_Async,TRUE,
      AOAEW_Close,TRUE,
      TAG_END);
}

/*-----------------------------------------------------------------------*/

LIBFUNC_H2
(
__saveds void  , Authorreq,
struct Authorreq *,areq,A0,
BOOL, onlypw, D0,
AUTHORIZE_TYPE, AUTHORIZE_NAME
)
{
    LIBFUNC_INIT
   void *useridgad=NULL,*passwordgad,*toplayout,*useridlab=NULL;
   UBYTE *userid,*password;
   Object *winobj;
   struct Window *window;
   UBYTE *screenname;
   struct TextFont *pwfont;
   struct Screen *screen;
   struct Buffer namebuf={0};
   UBYTE *p;

   Agetattrs(Aweb(),
      AOAPP_Screenname,(Tag)&screenname,
      AOAPP_Pwfont,(Tag)&pwfont,
      TAG_END);
   screen=LockPubScreen(screenname);
   p=areq->name;
   while(strlen(p)>70)
   {  Addtobuffer(&namebuf,p,70);
      Addtobuffer(&namebuf,"\n",1);
      p+=70;
   }
   if(!onlypw)
   {
      useridgad=StringObject,
                  STRINGA_TextVal,"",
                  STRINGA_MaxChars,127,
                  GA_TabCycle,TRUE,
                End;
      useridlab=LabelObject,
                  LABEL_Text,AWEBSTR(MSG_AUTH_USERID),
                EndMember;
   }
   Addtobuffer(&namebuf,p,-1);
   Addtobuffer(&namebuf,"",1);
   if(screen)
   {  winobj=WindowObject,
         WA_Title,AWEBSTR(MSG_AUTH_TITLE),
         WA_Left,40,
         WA_Top,60,
         WA_AutoAdjust,TRUE,
         WA_CloseGadget,TRUE,
         WA_DragBar,TRUE,
         WA_DepthGadget,TRUE,
         WA_SizeGadget,TRUE,
         WA_Activate,Awebactive(),
         WA_PubScreen,screen,
         WINDOW_Position,WPOS_CENTERSCREEN,
         WINDOW_Layout,toplayout=VLayoutObject,
            LAYOUT_VertAlignment,LALIGN_TOP,
            LAYOUT_FixedVert,FALSE,
            StartMember,VLayoutObject,
               LAYOUT_SpaceOuter,TRUE,
               StartMember,HLayoutObject,
                  LAYOUT_HorizAlignment,LALIGN_CENTER,
                  StartImage,LabelObject,
                     (!onlypw)? LABEL_Text: TAG_IGNORE, AWEBSTR(areq->proxy?MSG_AUTH_PROMPT_PROXY:MSG_AUTH_PROMPT),
                     (onlypw)?  LABEL_Text: TAG_IGNORE, AWEBSTR(MSG_AUTH_PROMPT_MASTER),
                     LABEL_Text,"\n",
                     LABEL_Text,namebuf.buffer,
                     (onlypw)? TAG_IGNORE: LABEL_Text,"\n(",
                     (onlypw)? TAG_IGNORE: LABEL_Text,areq->auth->realm,
                     (onlypw)? TAG_IGNORE: LABEL_Text,")",
                     LABEL_Justification,LJ_CENTRE,
                     LABEL_Underscore,0,
                  EndImage,
               EndMember,
               (!onlypw)? LAYOUT_AddChild: TAG_IGNORE, useridgad,
               (!onlypw)? CHILD_Label: TAG_IGNORE, useridlab,
               StartMember,passwordgad=StringObject,
                  STRINGA_TextVal,"",
                  STRINGA_MaxChars,127,
                  (hasstring45)?  TAG_IGNORE: STRINGA_Font,pwfont,
                  (!hasstring45)? TAG_IGNORE: STRINGA_HookType,SHK_PASSWORD,
                  GA_TabCycle,TRUE,
               EndMember,
               CHILD_Label,LabelObject,
                  LABEL_Text,AWEBSTR(MSG_AUTH_PASSWORD),
               EndMember,
            EndMember,
            StartMember,HLayoutObject,
               LAYOUT_SpaceOuter,TRUE,
               LAYOUT_BevelStyle,BVS_SBAR_VERT,
               StartMember,ButtonObject,
                  GA_Text,AWEBSTR(MSG_AUTH_OK),
                  GA_ID,AGID_OK,
                  GA_RelVerify,TRUE,
               EndMember,
               StartMember,SpaceObject,
               EndMember,
               StartMember,ButtonObject,
                  GA_Text,AWEBSTR(MSG_AUTH_CANCEL),
                  GA_ID,AGID_CANCEL,
                  GA_RelVerify,TRUE,
               EndMember,
            EndMember,
         End,
      EndWindow;
      if(winobj)
      {  if(window=RA_OpenWindow(winobj))
         {  ULONG sigmask=0,getmask;
            ULONG result;
            BOOL done=FALSE;
            GetAttr(WINDOW_SigMask,winobj,&sigmask);
            ActivateLayoutGadget(toplayout,window,NULL, (onlypw)? (ULONG)passwordgad: (ULONG)useridgad);
            while(!done)
            {  getmask=Wait(sigmask|SIGBREAKF_CTRL_C);
               if(getmask&SIGBREAKF_CTRL_C) break;
               while((result=RA_HandleInput(winobj,NULL))!=WMHI_LASTMSG)
               {  switch(result&WMHI_CLASSMASK)
                  {  case WMHI_CLOSEWINDOW:
                        done=TRUE;
                        break;
                     case WMHI_GADGETUP:
                        switch(result&WMHI_GADGETMASK)
                        {  case AGID_OK:
                              if(!onlypw)
                              {
                                 GetAttr(STRINGA_TextVal,useridgad,
                                    (ULONG *)&userid);
                              }
                              else
                              {
                                 userid="Master";
                              }
                              GetAttr(STRINGA_TextVal,passwordgad,
                                 (ULONG *)&password);
                              Bakecookie(areq->auth,userid,password);
                              areq->Rememberauth(areq->auth);
                              done=TRUE;
                              break;
                           case AGID_CANCEL:
                              done=TRUE;
                              break;
                        }
                        break;
                     case WMHI_RAWKEY:
                        if((result&WMHI_GADGETMASK)==0x45) done=TRUE;
                        break;
                  }
               }
            }
         }
         DisposeObject(winobj);
      }
   }
   Freebuffer(&namebuf);
   if(screen) UnlockPubScreen(NULL,screen);

    LIBFUNC_EXIT
}

/*-----------------------------------------------------------------------*/

LIBFUNC_H3
(
__saveds void  , Authorset,
struct Authorize *,auth,A0,
UBYTE *,userid,A1,
UBYTE *,passwd,A2,
AUTHORIZE_TYPE, AUTHORIZE_NAME
)
{
    LIBFUNC_INIT
  Bakecookie(auth,userid,passwd);

    LIBFUNC_EXIT
}
