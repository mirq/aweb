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

/* ftp.c - AWeb ftp protocol client */

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "aweblib.h"
#include "tcperr.h"
#include "fetchdriver.h"
#include "task.h"
#include "application.h"
#include "awebtcp.h"
#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"
#include "ftpparse.h"


#include <exec/resident.h>
#undef NO_INLINE_STDARG
#include <reaction/reaction.h>
#define NO_INLINE_STDARG
#include <proto/intuition.h>
#include <proto/exec.h>

#ifdef __MORPHOS__
ULONG __abox__=1;
#endif

#define FTP_VERSION 37
#define FTP_REVISION 1
#define FTP_VERSTRING "37.1 " CPU

#ifndef LOCALONLY

struct Ftpaddr
{  UBYTE *buf;
   UBYTE *hostname;
   long port;
   UBYTE *login;
   UBYTE *password;
   UBYTE *abspath;
   UBYTE type;
   UBYTE *fullname;  /* host/path with userid:passwd without trailing / or ;type=X */
};

struct Ftpresponse
{  UBYTE *buffer;
   long length;      /* nr of characters in buffer */
   long linelen;     /* nr of characters in first line */
};

enum FTPGADGET_IDS
{  AGID_OK=1,AGID_CANCEL,
};

struct DosLibrary    *DOSBase;
struct IntuitionBase *IntuitionBase;
struct UtilityBase *UtilityBase;
struct Library *AwebTcpBase;
struct Library *WindowBase,*LayoutBase,*ButtonBase,*StringBase,*LabelBase,*SpaceBase;
struct Library *AwebSupportBase;

#if defined(__amigaos4__)
struct AwebSupportIFace *IAwebSupport;
struct AwebTcpIFace *IAwebTcp;
struct DOSIFace *IDOS;
struct IntuitionIFace *IIntuition;
struct UtilityIFace *IUtility;
struct WindowIFace* IWindow;
struct LayoutIFace *ILayout;
struct ButtonIFace *IButton;
struct StringIFace *IString;
struct LabelIFace *ILabel;
struct SpaceIFace *ISpace;
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

static __saveds struct SegList *  Real_Closelib(LIBMAN_TYPE LIBMAN_NAME);
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
    FTP_TYPE, FTP_NAME
);

__saveds static void Fetchdrivertask(struct Fetchdriver *fd);

/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct Library *FtpBase;

static APTR libseglist;

struct ExecBase *SysBase;


LIBSTART_DUMMY

static char __aligned libname[]="ftp.aweblib";
static char __aligned libid[]="ftp.aweblib " FTP_VERSTRING " " __AMIGADATE__;

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
    FTP_VERSION,
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
   FTP_VERSION,
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
    libBase->lib_Version      = FTP_VERSION;
    libBase->lib_Revision     = FTP_REVISION;
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
   FtpBase=libbase;
   libbase->lib_Revision=FTP_REVISION;
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
   struct Library *Ftplibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Ftplibbase = (struct Library *)LIBMAN_NAME;
#endif

   Ftplibbase->lib_OpenCnt++;
   Ftplibbase->lib_Flags&=~LIBF_DELEXP;
   if(Ftplibbase->lib_OpenCnt==1)
   {
      if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return Real_Closelib(LIBMAN_NAME);
#if defined (__amigaos4__)
      if(!( IAwebSupport = (struct AwebSupportIface *)GetInterface(AwebSupportBase,"main",1,0))) return Real_Closelib(LIBMAN_NAME);
#endif
   }
   return Ftplibbase;

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

static __saveds struct SegList *  Real_Closelib(LIBMAN_TYPE LIBMAN_NAME)
{
#if defined(__amigaos4__)
   struct Library *Ftplibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Ftplibbase = (struct Library *)LIBMAN_NAME;
#endif


  Ftplibbase->lib_OpenCnt--;
   if(Ftplibbase->lib_OpenCnt==0)
   {  if(AwebSupportBase)
      {
#if defined(__amigaos4__)
    if(IAwebSupport) DropInterface((struct Interface *)IAwebSupport);
    IAwebSupport = NULL;
#endif

         CloseLibrary(AwebSupportBase);
         AwebSupportBase=NULL;
      }
      if(Ftplibbase->lib_Flags&LIBF_DELEXP)
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
   struct Library *Ftplibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Ftplibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Ftplibbase->lib_OpenCnt==0)
   {  ULONG size=Ftplibbase->lib_NegSize+Ftplibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Ftplibbase-Ftplibbase->lib_NegSize;
      Remove((struct Node *)Ftplibbase);
      Expungeaweblib(Ftplibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Ftplibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Ftplibbase->lib_Flags|=LIBF_DELEXP;
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

/*-----------------------------------------------------------------------*/

/* Get password. Returns dynamic string or NULL */
static UBYTE *Getpassword(struct Ftpaddr *fa)
{  void *winobj;
   void *pwgad,*toplayout;
   struct Window *window;
   UBYTE *password=NULL;
   UBYTE *screenname;
   struct TextFont *pwfont;
   struct Screen *screen;
   Agetattrs(Aweb(),
      AOAPP_Screenname,&screenname,
      AOAPP_Pwfont,&pwfont,
      TAG_END);
   if(screen=LockPubScreen(screenname))
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
                     LABEL_Text,AWEBSTR(MSG_AUTH_PROMPT_FTP),
                     LABEL_Text,"\n",
                     LABEL_Text,fa->hostname,
                     LABEL_Justification,LJ_CENTRE,
                  EndImage,
               EndMember,
               StartMember,ButtonObject,
                  GA_Text,fa->login,
                  GA_ReadOnly,TRUE,
                  BUTTON_Justification,BCJ_LEFT,
               EndMember,
               CHILD_Label,LabelObject,
                  LABEL_Text,AWEBSTR(MSG_AUTH_USERID_FTP),
               EndMember,
               StartMember,pwgad=StringObject,
                  STRINGA_TextVal,"",
                  STRINGA_MaxChars,127,
                  STRINGA_Font,pwfont,
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
            ActivateLayoutGadget(toplayout,window,NULL,(ULONG) pwgad);
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
                              GetAttr(STRINGA_TextVal,pwgad,
                                 (ULONG *)&password);
                              if(password) password=Dupstr(password,-1);
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
   if(screen) UnlockPubScreen(NULL,screen);
   return password;
}

/*-----------------------------------------------------------------------*/

/* Decode %AA url encoding */
static void Urldecode(UBYTE *string)
{  UBYTE *p,*q,*end;
   UBYTE c;
   short i;
   p=q=string;
   end=string+strlen(string);
   while(p<end)
   {  if(*p=='%' && p<end-3)
      {  c=0;
         for(i=0;i<2;i++)
         {  c<<=4;
            p++;
            if(*p>='0' && *p<='9')
            {  c+=*p-'0';
            }
            else if(*p>='A' && *p<='F')
            {  c+=*p-'A'+10;
            }
            else if(*p>='a' && *p<='f')
            {  c+=*p-'a'+10;
            }
         }
         *q=c;
      }
      else if(q!=p)
      {  *q=*p;
      }
      p++;
      q++;
   }
   *q='\0';
}

static STRPTR deflogin = "anonymous";
static STRPTR defpass = "";

/* Parse [login[:password]@]host[:port][/path[/][;type=X]] */
static BOOL Makeftpaddr(struct Ftpaddr *fa,UBYTE *name)
{  UBYTE *p,*q,*r;
   if(!(fa->buf=ALLOCTYPE(UBYTE,2*strlen(name)+8,0))) return FALSE;

   p=name;
   for(q=p;*q && *q!='@' && *q!='/';q++);
   if(*q=='@')
   {  /* Copy login and optional password */
      fa->login=q=fa->buf;
      while(*p && *p!=':' && *p!='@') *q++=*p++;
      *q++='\0';
      if(*p==':')
      {  p++;
         fa->password=q;
         while(*p && *p!='@') *q++=*p++;
         *q++='\0';
      }
      else
      {  if(STRIEQUAL(fa->login,"anonymous")) fa->password=defpass;
         else fa->password=defpass;
      }
      p++;
   }
   else
   {  fa->login=deflogin;
      fa->password=defpass;
      q=fa->buf;
   }
   /* Copy hostname upto port or path */
   fa->hostname=q;
   while(*p && *p!='/' && *p!=':') *q++=*p++;
   *q++='\0';
   /* Get port number or default */
   if(*p==':')
   {  fa->port=0;
      p++;
      while(isdigit(*p))
      {  fa->port=10*fa->port+(*p-'0');
         p++;
      }
      while(*p && *p!='/') p++;
   }
   else
   {  fa->port=21;
   }
   if(*p) p++;    /* Skip separator '/' */
   /* Get abs path */
   fa->abspath=q;
   if(*p)
   {  while(*p && *p!=';') *q++=*p++;
      r=p;
      if(*(q-1)=='/')
      {  q--;        /* Remove trailing slash */
         r--;
      }
      *q++='\0';
      if(STRNIEQUAL(p,";type=",6) && p[6])
      {  fa->type=toupper(p[6]);
         if(!strchr("AID",fa->type)) fa->type='I';
      }
      else fa->type='I';
   }
   else
   {  *q++='\0';
      fa->type='D';
      r=p;
   }
   fa->fullname=q;
   memmove(fa->fullname,name,r-name);
   Urldecode(fa->hostname);
   if(fa->login != deflogin) Urldecode(fa->login);
   if(fa->password != defpass) Urldecode(fa->password);
   Urldecode(fa->abspath);
   return TRUE;
}

/* Get the e-mail address or create dummy password */
static UBYTE *Defaultpassword(struct Fetchdriver *fd)
{  UBYTE *result;
   ObtainSemaphore(fd->prefssema);
   if(fd->prefs->network.ftpemailaddr && *fd->prefs->network.emailaddr)
   {  result=Dupstr(fd->prefs->network.emailaddr,-1);
   }
   else
   {  result=Dupstr("anonymous@anonymous.nn",-1);
   }
   ReleaseSemaphore(fd->prefssema);
   return result;
}

/* Return the next line of response, or NULL in case of error.
 * If no response in buffer, read it. */
static UBYTE *Nextline(long sock,struct Ftpresponse *fr,struct Library *SocketBase)
{
   #define BUFSIZE 1024
   UBYTE *p,*end;
   long r;
   if(!fr->buffer
   && !(fr->buffer=ALLOCTYPE(UBYTE,BUFSIZE,MEMF_CLEAR))) return NULL;
   /* Delete current first line */
   if(fr->linelen)
   {  memmove(fr->buffer,fr->buffer+fr->linelen,fr->length-fr->linelen);
      fr->length-=fr->linelen;
      fr->linelen=0;
   }
   /* Find end of first line. If no EOL yet, read more data. */
   for(;;)
   {  end=fr->buffer+fr->length;
      for(p=fr->buffer;p<end && *p!='\n';p++);
      if(p<end)
      {  *p='\0';
         fr->linelen=p-fr->buffer+1;
         if(fr->linelen>0 && fr->buffer[fr->linelen-1]=='\r')
         {  fr->buffer[fr->linelen-1]='\0';
         }
         break;
      }
      r=a_recv(sock,fr->buffer+fr->length,BUFSIZE-fr->length-1,0,SocketBase);
      if(r<=0) return NULL;
      fr->length+=r;
   }
   return fr->buffer;
}

/* Repeat reading lines until a noncontinued line is read. Return the
 * numeric status, or 999 in case of error. */
static long Getresponse(long sock,struct Ftpresponse *fr,struct Library *SocketBase)
{  long r;
   UBYTE *p;
   if(!(p=Nextline(sock,fr,SocketBase))) return 999;
   r=atoi(p);
   if(strlen(p)>3 && p[3]=='-')
   {  do
      {  if(!(p=Nextline(sock,fr,SocketBase))) return 999;
      } while(!(strlen(p)>3 && p[3]==' ' && (atoi(p)==r)));
   }
   return r;
}

/* Build an FTP command and send it. Return the numeric status. */
static long Ftpcommand(long sock,struct Ftpresponse *fr,struct Library *SocketBase,
   UBYTE *fmt,...)
{  va_list args;
   UBYTE cmd[256];
   long len;
   va_start(args,fmt);
   vsprintf(cmd,fmt,args);
   va_end(args);
   strcat(cmd,"\r\n");
   len=strlen(cmd);
   if(a_send(sock,cmd,len,0,SocketBase)!=len) return 999;
   return Getresponse(sock,fr,SocketBase);
}

static BOOL Login(long sock,struct Fetchdriver *fd,struct Ftpresponse *fr,struct Ftpaddr *fa,
   struct Library *SocketBase)
{  long result;
   UBYTE *pwbuf;
   Updatetaskattrs(AOURL_Netstatus,NWS_LOGIN,TAG_END);
   Tcpmessage(fd,TCPMSG_LOGIN,fa->hostname);
   result=Ftpcommand(sock,fr,SocketBase,"USER %s",fa->login);
   if(result>=400) return FALSE;
   if(result==331)
   {  if(*fa->password)
      {  result=Ftpcommand(sock,fr,SocketBase,"PASS %s",fa->password);
      }
      else
      {  if(STREQUAL(fa->login,"anonymous"))
         {  pwbuf=Defaultpassword(fd);
         }
         else
         {  pwbuf=Getpassword(fa);
         }
         if(pwbuf)
         {  result=Ftpcommand(sock,fr,SocketBase,"PASS %s",pwbuf);
            FREE(pwbuf);
         }
         else result=999;
      }


      if(result>=400) return FALSE;
   }
   return TRUE;
}

/* If passive, ask the server for an address+port and connect to it.
 * Else create a new socket and listen to it. */
static BOOL Initdatasock(long ctrlsock,struct Fetchdriver *fd,struct Ftpresponse *fr,
   int addrtype,struct Library *SocketBase,long *plsock)
{  long lsock;
   struct sockaddr_in lsad={0};
   int len=sizeof(lsad);
   UBYTE *a,*p;


   if(a_getsockname(ctrlsock,(struct sockaddr *)&lsad,&len,SocketBase)<0) return FALSE;

   if((lsock=a_socket(addrtype,SOCK_STREAM,0,SocketBase))<0) return FALSE;

   *plsock=lsock;
   if(fd->prefs->network.passiveftp)
   {  UBYTE ap[6];
      UBYTE *q;
      short i;

      if(Ftpcommand(ctrlsock,fr,SocketBase,"PASV")!=227) return FALSE;
      if(q=strchr(fr->buffer,'('))
      {  for(i=0;i<6;i++)
         {  q++;
            ap[i]=atoi(q);
            if(i<5)
            {  q=strchr(q,',');
               if(!q) return FALSE;
            }
         }
      }
      else return FALSE;
      if(a_connect2(lsock,addrtype,*(u_long *)ap,*(short *)(ap+4),SocketBase)) return FALSE;
   }
   else
   {

      lsad.sin_port=0;
      len=sizeof(lsad);

      if(a_bind(lsock,(struct sockaddr *)&lsad,sizeof(lsad),SocketBase)<0) return FALSE;

      if(a_getsockname(lsock,(struct sockaddr *)&lsad,&len,SocketBase)<0) return FALSE;

      if(a_listen(lsock,1,SocketBase)<0) return -1;

      a=(UBYTE *)&lsad.sin_addr;
      p=(UBYTE *)&lsad.sin_port;
      if(Ftpcommand(ctrlsock,fr,SocketBase,"PORT %d,%d,%d,%d,%d,%d",
         a[0],a[1],a[2],a[3],p[0],p[1])!=200) return FALSE;

   }
   return TRUE;
}

/* If passive, just use the existing lsock. Else accept a new connection. */
static BOOL Opendatasock(struct Fetchdriver *fd,long *plsock,long *pdsock,
   struct Library *SocketBase)
{  struct sockaddr_in dsad={0};
   int len=sizeof(dsad);
   if(fd->prefs->network.passiveftp)
   {  *pdsock=*plsock;
      *plsock=-1;
   }
   else
   {  if((*pdsock=a_accept(*plsock,(struct sockaddr *)&dsad,&len,SocketBase))<0) return FALSE;
      a_close(*plsock,SocketBase);
      *plsock=-1;
   }
   return TRUE;
}

/*-----------------------------------------------------------------------*/

/* Copy string, but escape HTML characters */
static long Htmlmove(UBYTE *to,UBYTE *from,long len)
{  long n=0;
   for(;len;from++,len--)
   {  switch(*from)
      {  case '<':
            strcpy(to,"&lt;");
            to+=4;
            n+=4;
            break;
         case '>':
            strcpy(to,"&gt;");
            to+=4;
            n+=4;
            break;
         case '&':
            strcpy(to,"&amp;");
            to+=5;
            n+=5;
            break;
         default:
            *to++=*from;
            n++;
      }
   }
   return n;
}

/* Read and parse directory data. Build HTML document */
static void Readdir(long sock,struct Fetchdriver *fd,struct Ftpaddr *fa,
   struct Library *SocketBase)
{  struct Ftpresponse fr={0};
   UBYTE *line,*p,*q,*name,*link;
   long len,plen;

   len=sprintf(fd->block,
      "<HTML>\n<HEAD>\n<TITLE>ftp://%s</TITLE>\n</HEAD>\n"
      "<BODY>\n<H1>ftp://%s</H1>\n<b><font size=+1>%s</font></b><p>\n",
      fa->fullname,fa->hostname,fa->fullname);
   if(*fa->abspath)
   {  /* Find parent dir */
      for(p=fa->fullname,q=NULL;*p;p++)
      {  if(*p=='/') q=p;
      }
      if(q)
      {  plen=q-fa->fullname;
      }
      else
      {  plen=strlen(fa->fullname);
      }
      len+=sprintf(fd->block+len,
         "Up to parent directory: <A HREF=\"ftp://%*.*s\">ftp://%*.*s</A>\n<P>\n",
         plen,plen,fa->fullname,plen,plen,fa->fullname);
   }
   len+=sprintf(fd->block+len,"<PRE>\n");
   Updatetaskattrs(
      AOURL_Data,fd->block,
      AOURL_Datalength,len,
      TAG_END);
   while(line=Nextline(sock,&fr,SocketBase))
   {
      if(STRNIEQUAL(line,"total ",6))
      {  strcpy(fd->block,line);
         len=strlen(fd->block);
      }
      else
      {  /* Find the last word. If the previous word is "->" is't a link and include
          * it in the anchor. */
          struct ftpparse fp;
          name = NULL;
          if (ftpparse(&fp,line,strlen(line)))
          {
             p = q = line+strlen(line)-1;
             while(p>=line && isspace(*p)) p--;
             /* End of last word */
             p[1]='\0';
             link = fp.name;
             name = Dupstr(fp.name,-1);
             p = name -1;
             if((p = strstr(link,"->")))
             {
                while((--p <=q) && isspace(*p));
                p[1] = '\0';
             }
          }
         len=link-line;
         if(len>=0)
         {  len=Htmlmove(fd->block,line,len);
         }
         len+=sprintf(fd->block+len,"<A HREF=\"ftp://%s/%s\">",fa->fullname,link);
         len+=Htmlmove(fd->block+len,name,strlen(name));
         len+=sprintf(fd->block+len,"</A>\n");
         if(name) FREE(name);
      }
      Updatetaskattrs(
         AOURL_Data,fd->block,
         AOURL_Datalength,len,
         TAG_END);
   }
   if(fr.buffer) FREE(fr.buffer);
}

/*-----------------------------------------------------------------------*/

LIBFUNC_H1
(
    __saveds static Subtaskfunction *, GetTaskFunc,
    ULONG, id, D0,
    FTP_TYPE, FTP_NAME
)
{
    LIBFUNC_INIT

    return (Subtaskfunction *)&Fetchdrivertask;

    LIBFUNC_EXIT
}

__saveds static void Fetchdrivertask(struct Fetchdriver *fd)
{
   struct Library *SocketBase;
   struct Ftpaddr fa={0};
   struct hostent *hent;
   long ctrlsock=-1,datasock=-1,listsock=-1;
   struct Ftpresponse fr={0};
   long len;
   BOOL file = FALSE;
   BOOL error = FALSE;

   AwebTcpBase=Opentcp(&SocketBase,fd,TRUE);
#if defined(__amigaos4__)
    IAwebTcp = (struct AwebTcpIFace *)GetInterface(AwebTcpBase,"main",1,0);
#endif
   if(SocketBase)
   {  if(Makeftpaddr(&fa,fd->name))
      {  Updatetaskattrs(AOURL_Netstatus,NWS_LOOKUP,TAG_END);
         Tcpmessage(fd,TCPMSG_LOOKUP,fa.hostname);
         if(hent=Lookup(fa.hostname,SocketBase))
         {  if((ctrlsock=a_socket(hent->h_addrtype,SOCK_STREAM,0,SocketBase))>=0)
            {  Updatetaskattrs(AOURL_Netstatus,NWS_CONNECT,TAG_END);
               Tcpmessage(fd,TCPMSG_CONNECT,"FTP",hent->h_name);
               if(!a_connect(ctrlsock,hent,fa.port,SocketBase)
               && Getresponse(ctrlsock,&fr,SocketBase)<300)
               {  if(Login(ctrlsock,fd,&fr,&fa,SocketBase))
                  {  if(Initdatasock(ctrlsock,fd,&fr,hent->h_addrtype,SocketBase,&listsock))
                     {

                        if(fa.type!='D'
                        && Ftpcommand(ctrlsock,&fr,SocketBase,"TYPE %c",fa.type)
                        && Ftpcommand(ctrlsock,&fr,SocketBase,"RETR %s",fa.abspath)==150)
                        {  file=TRUE;
                        }
                        else if(
                           (!*fa.abspath ||
                              Ftpcommand(ctrlsock,&fr,SocketBase,"CWD %s",fa.abspath)==250)
                        && Ftpcommand(ctrlsock,&fr,SocketBase,"TYPE A")
                        && Ftpcommand(ctrlsock,&fr,SocketBase,"LIST -l")<200)
                        {  file=FALSE;
                        }
                        else
                        {  Tcperror(fd,TCPERR_NOFILE,fa.abspath);
                           error=TRUE;
                        }
                        if(!error)
                        {  Updatetaskattrs(AOURL_Netstatus,NWS_WAIT,TAG_END);
                           Tcpmessage(fd,TCPMSG_WAITING,"FTP");
                           if(Opendatasock(fd,&listsock,&datasock,SocketBase))
                           {  if(file)
                              {  for(;;)
                                 {  len=a_recv(datasock,fd->block,fd->blocksize,0,SocketBase);
                                    if(len<0 || Checktaskbreak())
                                    {  error=TRUE;
                                       break;
                                    }
                                    Updatetaskattrs(
                                       AOURL_Data,fd->block,
                                       AOURL_Datalength,len,
                                       TAG_END);
                                    if(len==0) break;
                                 }
                              }
                              else
                              {  Readdir(datasock,fd,&fa,SocketBase);

                              }
                              a_shutdown(datasock,2,SocketBase);
                              a_close(datasock,SocketBase);
                           }
                           else error=TRUE;
                        }
                     }
                     else error=TRUE;
                     if(listsock>=0) a_close(listsock,SocketBase);
                     Ftpcommand(ctrlsock,&fr,SocketBase,"QUIT");
                  }
                  else
                  {  Tcperror(fd,TCPERR_NOLOGIN,fa.hostname,fa.login);
                  }
               }
               else
               {  Tcperror(fd,TCPERR_NOCONNECT,hent->h_name);
               }
               a_close(ctrlsock,SocketBase);
            }
            else error=TRUE;
         }
         else
         {  Tcperror(fd,TCPERR_NOHOST,fa.hostname);
         }
         FREE(fa.buf);
      }
      a_cleanup(SocketBase);

#if defined(__amigaos4__)
    DropInterface((struct Interface *)IAwebTcp);
    /* Under OS4 our socketbase is really the socketiface */
    {
        struct Library *sbase = (struct Library *)((struct Interface *)SocketBase)->Data.LibBase;
        DropInterface((struct Interface *)SocketBase);
        if(sbase)CloseLibrary(sbase);
    }

#else
      CloseLibrary(SocketBase);
#endif

   }
   else
   {  Tcperror(fd,TCPERR_NOLIB);
   }
   if(fr.buffer) FREE(fr.buffer);
   Updatetaskattrs(AOTSK_Async,TRUE,
      AOURL_Error,error,
      AOURL_Eof,TRUE,
      AOURL_Terminate,TRUE,
      TAG_END);
}

/*-----------------------------------------------------------------------*/

static ULONG Initaweblib(struct Library *libbase)
{
   if(!AwebModuleInit()) return FALSE;
   if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library",39))) return FALSE;
   if(!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",39))) return FALSE;
   if(!(UtilityBase = (struct UtilityBase *) OpenLibrary("utility.library",0))) return FALSE;
   if(!(WindowBase=(struct ClassLibrary *)OpenLibrary("window.class",OSNEED(0,44)))) return FALSE;
   if(!(LayoutBase=(struct ClassLibrary *)OpenLibrary("gadgets/layout.gadget",OSNEED(0,44)))) return FALSE;
   if(!(ButtonBase=(struct ClassLibrary *)OpenLibrary("gadgets/button.gadget",OSNEED(0,44)))) return FALSE;
   if(!(StringBase=(struct ClassLibrary *)OpenLibrary("gadgets/string.gadget",OSNEED(0,44)))) return FALSE;
   if(!(SpaceBase=(struct ClassLibrary *)OpenLibrary("gadgets/space.gadget",OSNEED(0,44)))) return FALSE;
   if(!(LabelBase=(struct ClassLibrary *)OpenLibrary("images/label.image",OSNEED(0,44)))) return FALSE;

#if defined(__amigaos4__)
   if(!(IDOS = (struct DOSIFace *)GetInterface(DOSBase,"main",1,0))) return FALSE;
   if(!(IIntuition = (struct IntuitionIFace *) GetInterface(IntuitionBase,"main",1,0))) return FALSE;
   if(!(IUtility   = (struct UtilityIFace *)    GetInterface(UtilityBase,"main",1,0))) return FALSE;
   if(!(IWindow = (struct WindowIFace *)GetInterface(WindowBase,"main",1,0))) return FALSE;
   if(!(ILayout = (struct LayoutIFace *)GetInterface(LayoutBase,"main",1,0))) return FALSE;
   if(!(IButton = (struct ButtonIFace *)GetInterface(ButtonBase,"main",1,0))) return FALSE;
   if(!(IString = (struct StringFace *)GetInterface(StringBase,"main",1,0))) return FALSE;
   if(!(ISpace = (struct SpaceIFace *)GetInterface(SpaceBase,"main",1,0))) return FALSE;
   if(!(ILabel = (struct LabrelIFace *)GetInterface(LabelBase,"main",1,0))) return FALSE;

#endif

   return TRUE;
}

static void Expungeaweblib(struct Library *libbase)
{
#if defined(__amigaos4__)

   if(ILabel)DropInterface((struct Interface *)ILabel);
   if(IString)DropInterface((struct Interface *)IString);
   if(ISpace)DropInterface((struct Interface *)ISpace);
   if(IButton)DropInterface((struct Interface *)IButton);
   if(ILayout)DropInterface((struct Interface *)ILayout);
   if(IWindow)DropInterface((struct Interface *)IWindow);
   if(IUtility)DropInterface((struct Interface *)IUtility);
   if(IIntuition)DropInterface((struct Interface *)IIntuition);
   if(IDOS)DropInterface((struct Interface *)IDOS);
#endif

   if(LabelBase) CloseLibrary(LabelBase);
   if(SpaceBase) CloseLibrary(SpaceBase);
   if(StringBase) CloseLibrary(StringBase);
   if(ButtonBase) CloseLibrary(ButtonBase);
   if(LayoutBase) CloseLibrary(LayoutBase);
   if(WindowBase) CloseLibrary(WindowBase);
   if(IntuitionBase) CloseLibrary(IntuitionBase);
   if(UtilityBase) CloseLibrary(UtilityBase);
   if(DOSBase) CloseLibrary(DOSBase);
   AwebModuleExit();
}

#endif /* !LOCALONLY */
