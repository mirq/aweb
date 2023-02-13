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

/* mail.c - AWeb mailer */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "aweblib.h"
#include "tcperr.h"
#include "fetchdriver.h"
#include "task.h"
#include "application.h"
#include "awebtcp.h"
#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"

#include <exec/resident.h>
#include <libraries/asl.h>
#include <graphics/displayinfo.h>
#include <proto/asl.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <reaction/reaction.h>


#ifdef __MORPHOS__
ULONG __abox__=1;
#endif

#ifndef LOCALONLY

struct DosLibrary    *DOSBase;
struct GfxBase       *GfxBase;
struct Library *UtilityBase,*AslBase,*AwebTcpBase;
struct Library *AwebSupportBase;

#if defined(__amigaos4__)
struct DOSIFace *IDOS;
struct GraphicsIFace *IGraphics;
struct UtilityIFace *IUtility;
struct AslIFace *IAsl;
struct AwebTcpIFace *IAwebTcp;
struct AwebSupportIFace *IAwebSupport;
#endif

#define MAIL_VERSION 37
#define MAIL_REVISION 0
#define MAIL_VERSTRING "37.0 " CPU


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
    __saveds static Subtaskfunction *, GetTaskFunc,
    ULONG, id, D0,
    FTP_TYPE, FTP_NAME
);

__saveds static void Fetchdrivertask(struct Fetchdriver *fd);

/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct Library *MailBase;

static APTR libseglist;

struct ExecBase *SysBase;


LIBSTART_DUMMY

static char __aligned libname[]="mail.aweblib";
static char __aligned libid[]="mail.aweblib " MAIL_VERSTRING " " __AMIGADATE__;

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
    MAIL_VERSION,
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
   MAIL_VERSION,
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
    libBase->lib_Version      = MAIL_VERSION;
    libBase->lib_Revision     = MAIL_REVISION;
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
   MailBase=libbase;
   libbase->lib_Revision=MAIL_REVISION;
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
   struct Library *Maillibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Maillibbase = (struct Library *)LIBMAN_NAME;
#endif

   Maillibbase->lib_OpenCnt++;
   Maillibbase->lib_Flags&=~LIBF_DELEXP;
   if(Maillibbase->lib_OpenCnt==1)
   {
      if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return Real_Closelib(LIBMAN_NAME);
#if defined (__amigaos4__)
      if(!( IAwebSupport = (struct AwebSupportIface *)GetInterface(AwebSupportBase,"main",1,0))) return Real_Closelib(LIBMAN_NAME);
#endif
   }
   return Maillibbase;

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
   struct Library *Maillibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Maillibbase = (struct Library *)LIBMAN_NAME;
#endif


  Maillibbase->lib_OpenCnt--;
   if(Maillibbase->lib_OpenCnt==0)
   {  if(AwebSupportBase)
      {
#if defined(__amigaos4__)
    if(IAwebSupport) DropInterface((struct Interface *)IAwebSupport);
    IAwebSupport = NULL;
#endif

         CloseLibrary(AwebSupportBase);
         AwebSupportBase=NULL;
      }
      if(Maillibbase->lib_Flags&LIBF_DELEXP)
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
   struct Library *Maillibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Maillibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Maillibbase->lib_OpenCnt==0)
   {  ULONG size=Maillibbase->lib_NegSize+Maillibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Maillibbase-Maillibbase->lib_NegSize;
      Remove((struct Node *)Maillibbase);
      Expungeaweblib(Maillibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Maillibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Maillibbase->lib_Flags|=LIBF_DELEXP;
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

/* Convert hexadecimal */
static UBYTE Hextodec(UBYTE c)
{  UBYTE ch=0;
   if(c>='0' && c<='9') ch=c-'0';
   else
   {  c=toupper(c);
      if(c>='A' && c<='F') ch=c+10-'A';
   }
   return ch;
}

/* Remove URL escape from block. Return new length (always <= old length) */
static long Unescape(UBYTE *block,long len)
{  UBYTE *start,*p,*end;
   long newlen=len;
   start=block;
   end=block+len;
   while(start<end)
   {  for(p=start;p<end && *p!='%';p++)
      {  if(*p=='+') *p=' ';
      }
      if(p<end)
      {  /* Now *p=='%' */
         if(p<end-2)
         {  *p=(Hextodec(p[1])<<4)+Hextodec(p[2]);
            memmove(p+1,p+3,end-p-3);
            newlen-=2;
         }
      }
      start=p+1;
   }
   return newlen;
}

/* Append this block with dangerous characters HTML escaped. */
static void Addblock(struct Buffer *buf,UBYTE *block,long len)
{  UBYTE *start,*p,*end,*esc;
   start=block;
   end=block+len;
   while(start<end)
   {  for(p=start;p<end && *p!='<' && *p!='&' && *p!='\'';p++);
      Addtobuffer(buf,start,p-start);
      if(p<end)
      {  switch(*p)
         {  case '<':esc="&lt;";break;
            case '&':esc="&amp;";break;
            case '\'':esc="&#39;";break;
            default: esc="";break;
         }
         Addtobuffer(buf,esc,strlen(esc));
         p++;  /* Skip over dangerous character */
      }
      start=p;
   }
}

/* Build mail form in HTML format */
static void Buildmailform(struct Fetchdriver *fd)
{  UBYTE *to,*p,*q,*subject=NULL;
   UBYTE *params[2];
   ULONG file;
   struct Buffer buf={0};
   long len,subjlen;
   short i;
   BOOL first,formmail;
   Updatetaskattrs(
      AOURL_Contenttype,"text/html",
      TAG_END);
   to=fd->name+7; /* skip mailto: */
   if(p=strchr(to,'?'))
   {  *p++=0;
      params[0]=p;
   }
   else params[0]=NULL;
   params[1]=fd->postmsg;
   for(i=0;i<2;i++)
   {  if(params[i])
      {  p=params[i];
         for(;;)
         {  if(STRNIEQUAL(p,"SUBJECT=",8))
            {  subject=p+8;
               if(q=strchr(subject,'&')) subjlen=q-subject;
               else
               {  subjlen=strlen(subject);
                  if(p==params[i]) params[i]=NULL; /* subject is only form parameter */
               }
               break;
            }
            if(q=strchr(p,'&')) p=q+1;
            else break;
         }
      }
   }
   formmail=(params[0] || params[1]);
   p=fd->block;
   p+=sprintf(p,"<HTML>\n<HEAD>\n<TITLE>");
   p+=sprintf(p,AWEBSTR(MSG_MAIL_MAILTO_TITLE),to);
   p+=sprintf(p,"</TITLE>\n</HEAD>\n<BODY>\n<H1>");
   p+=sprintf(p,AWEBSTR(MSG_MAIL_MAILTO_HEADER),to);
   p+=sprintf(p,"</H1>\n<FORM ACTION='x-aweb:mail/1/mail' METHOD=POST>\n");
   p+=sprintf(p,"<TABLE>\n<TR>\n<TD ALIGN='right'>%s\n"
      "<TD><INPUT NAME='to' VALUE='%s' SIZE=60>\n",AWEBSTR(MSG_MAIL_TO),to);
   p+=sprintf(p,"<TR>\n<TD ALIGN='right'>%s\n"
      "<TD><INPUT NAME='subject' VALUE='",AWEBSTR(MSG_MAIL_SUBJECT));
   if(subject)
   {  Addtobuffer(&buf,fd->block,p-fd->block);
      subjlen=Unescape(subject,subjlen);
      Addblock(&buf,subject,subjlen);
      p=fd->block;
   }
   else
   {  p+=sprintf(p,"mailto:%s",to);
   }
   p+=sprintf(p,"' SIZE=60>\n");
   p+=sprintf(p,"</TABLE>\n%s"
      " (<INPUT TYPE=CHECKBOX NAME='extra' VALUE='Y'%s> %s)"
      "<BR>\n<TEXTAREA NAME='data' COLS=70 ROWS=20>\n",
      AWEBSTR(MSG_MAIL_MESSAGE_BODY),
      formmail?" CHECKED":"",
      AWEBSTR(MSG_MAIL_EXTRA_HEADERS));
   if(formmail)
   {  p+=sprintf(p,"Content-type: application/x-www-form-urlencoded\n\n");
      Addtobuffer(&buf,fd->block,p-fd->block);
      first=TRUE;
      for(i=0;i<2;i++)
      {  while(params[i] && *params[i])
         {  if(!(q=strchr(params[i],'&'))) q=params[i]+strlen(params[i]);
            if(!STRNIEQUAL(params[i],"SUBJECT=",8))
            {  if(!first) Addblock(&buf,"&",1);
               first=FALSE;
               Addblock(&buf,params[i],q-params[i]);
            }
            params[i]=q;
            if(*params[i]) params[i]++;
         }
         p=fd->block;
      }
   }
   else
   {  ObtainSemaphore(fd->prefssema);
      if(fd->prefs->network.sigfile)
      {  p+=sprintf(p,"&#10;\n--\n");
         Addtobuffer(&buf,fd->block,p-fd->block);
         if(file=Open(fd->prefs->network.sigfile,MODE_OLDFILE))
         {  first=TRUE;
            while(len=Read(file,fd->block,fd->blocksize))
            {  if(first && len>3 && STRNEQUAL(fd->block,"--\n",3))
               {  Addblock(&buf,fd->block+3,len-3);
               }
               else
               {  Addblock(&buf,fd->block,len);
               }
               first=FALSE;
            }
            Close(file);
         }
         p=fd->block;
      }
      ReleaseSemaphore(fd->prefssema);
   }
   p+=sprintf(p,"</TEXTAREA>\n<BR><BR>\n<INPUT TYPE=SUBMIT VALUE='%s'> "
      "<INPUT TYPE=RESET VALUE='%s'> "
      "<INPUT TYPE=SUBMIT NAME='Return' VALUE='%s'>\n",
      AWEBSTR(MSG_MAIL_SEND),AWEBSTR(MSG_MAIL_RESET),AWEBSTR(MSG_MAIL_RETURN));
   p+=sprintf(p,"</FORM>\n</BODY>\n</HTML>\n");
   Addtobuffer(&buf,fd->block,p-fd->block);
   Updatetaskattrs(
      AOURL_Data,buf.buffer,
      AOURL_Datalength,buf.length,
      TAG_END);
   Freebuffer(&buf);
}

/*-----------------------------------------------------------------------*/

struct Mailinfo
{  struct Buffer buf;
   UBYTE *to;
};

/* See if mail form was submitted from RETURN button */
static BOOL Findreturn(struct Fetchdriver *fd)
{  UBYTE *p;
   p=fd->postmsg;
   while(p=strchr(p,'&'))
   {  if(STRNIEQUAL(p,"&RETURN=",8)) return TRUE;
      p++;
   }
   return FALSE;
}

/* Build the mail message. Expects form fields in order:
 * To, Subject, [Inreplyto], [extra=y], Data. */
static void Buildmessage(struct Fetchdriver *fd,struct Mailinfo *mi)
{  UBYTE *p,*q,*start;
   long len;
   p=fd->postmsg;
   if(!STRNIEQUAL(p,"TO=",3)) return;
   p+=3;
   if(!(q=strchr(p,'&'))) return;
   *q++='\0';
   len=Unescape(p,q-p);
   mi->to=Dupstr(p,len);
   ObtainSemaphore(fd->prefssema);
   len=sprintf(fd->block,"From: %s (%s)\r\n",fd->prefs->network.emailaddr,fd->prefs->network.fullname);
   Addtobuffer(&mi->buf,fd->block,len);
   len=sprintf(fd->block,"To: %s\r\n",mi->to);
   Addtobuffer(&mi->buf,fd->block,len);
   p=q;
   if(!STRNIEQUAL(p,"SUBJECT=",8))
   {  ReleaseSemaphore(fd->prefssema);
      return;
   }
   p+=8;
   if(!(q=strchr(p,'&')))
   {  ReleaseSemaphore(fd->prefssema);
      return;
   }
   *q++='\0';
   Unescape(p,q-p);
   len=sprintf(fd->block,"Subject: %s\r\n",p);
   Addtobuffer(&mi->buf,fd->block,len);
   if(*fd->prefs->network.replyaddr)
   {  len=sprintf(fd->block,"Reply-to: %s\r\n",fd->prefs->network.replyaddr);
      Addtobuffer(&mi->buf,fd->block,len);
   }
   ReleaseSemaphore(fd->prefssema);
   p=q;
   if(STRNIEQUAL(p,"INREPLYTO=",10))
   {  p+=10;
      if(!(q=strchr(p,'&'))) return;
      *q++='\0';
      Unescape(p,q-p);
      len=sprintf(fd->block,"In-Reply-To: %s\r\n",p);
      Addtobuffer(&mi->buf,fd->block,len);
      p=q;
   }
   len=sprintf(fd->block,"X-Mailer: Amiga-AWeb/%s\r\n",Awebversion());
   Addtobuffer(&mi->buf,fd->block,len);
   if(STRNIEQUAL(p,"EXTRA=Y&",8))
   {  p+=8;
   }
   else
   {  Addtobuffer(&mi->buf,"\r\n",2);
   }
   if(!STRNIEQUAL(p,"DATA=",5)) return;
   p+=5;
   len=Unescape(p,strlen(p));
   p[len]='\0';
   start=p;
   for(;;)
   {  if(*p=='.')
      {  Addtobuffer(&mi->buf,start,p-start);
         Addtobuffer(&mi->buf,".",1);
         start=p;
      }
      if(!(q=strchr(p,'\r'))) break;
      q+=2; /* Skip over \r\n */
      p=q;
   }
   Addtobuffer(&mi->buf,start,strlen(start));
   p=mi->buf.buffer+mi->buf.length-2;
   if(!(p[0]=='\r' && p[1]=='\n'))
   {  Addtobuffer(&mi->buf,"\r\n",2);
   }
}

/* Get a response code */
static long Getresponse(struct Fetchdriver *fd,long sock,struct Library *SocketBase)
{  long len,result;
   len=a_recv(sock,fd->block,fd->blocksize,0,SocketBase);
   if(len<0) return 999;
   fd->block[MIN(len,fd->blocksize-1)]='\0';
   result=atoi(fd->block);
   return result;
}

/* Send a command, return the response. */
static long Sendcommand(struct Fetchdriver *fd,long sock,struct Library *SocketBase,
   UBYTE *fmt,...)
{  va_list args;
   long len;
   va_start(args,fmt);
   vsprintf(fd->block,fmt,args);
   va_end(args);
   strcat(fd->block,"\r\n");
   len=strlen(fd->block);
   if(a_send(sock,fd->block,len,0,SocketBase)!=len) return 999;
   return Getresponse(fd,sock,SocketBase);
}

enum SENDMESSAGE_RETURNS
{  SMR_FAIL,SMR_NOCONNECT,SMR_OK,
};

/* Show mail error requester. Return TRUE if retry requested. */
static BOOL Mailretry(struct Fetchdriver *fd,struct Mailinfo *mi,short err)
{  struct FileRequester *afr;
   struct Screen *scr;
   long result,file,height;
   struct DimensionInfo dim={0};
   if(err==SMR_NOCONNECT)
   {  ObtainSemaphore(fd->prefssema);
      if(*fd->prefs->network.smtphost)
      {  sprintf(fd->block,AWEBSTR(MSG_MAIL_NOCONNECT),fd->prefs->network.smtphost);
      }
      else
      {  strcpy(fd->block,AWEBSTR(MSG_MAIL_NOSMTPHOST));
      }
      ReleaseSemaphore(fd->prefssema);
   }
   else
   {  strcpy(fd->block,AWEBSTR(MSG_MAIL_MAILFAILED));
   }
   result=Syncrequest(AWEBSTR(MSG_MAIL_TITLE),fd->block,AWEBSTR(MSG_MAIL_BUTTONS));
   if(result==2) /* Save */
   {  if(scr=(struct Screen *)Agetattr(Aweb(),AOAPP_Screen))
      {  if(GetDisplayInfoData(NULL,(UBYTE *)&dim,sizeof(dim),DTAG_DIMS,
            GetVPModeID(&scr->ViewPort)))
         {  height=(dim.Nominal.MaxY-dim.Nominal.MinY+1)*4/5;
         }
         else
         {  height=scr->Height*4/5;
         }
         if(afr=AllocAslRequestTags(ASL_FileRequest,
            ASLFR_Screen,scr,
            ASLFR_TitleText,AWEBSTR(MSG_MAIL_SAVETITLE),
            ASLFR_InitialHeight,height,
            ASLFR_InitialDrawer,"RAM:",
            ASLFR_InitialFile,"MailMessage",
            ASLFR_RejectIcons,TRUE,
            ASLFR_DoSaveMode,TRUE,
            TAG_END))
         {  if(AslRequest(afr,NULL))
            {  strcpy(fd->block,afr->fr_Drawer);
               AddPart(fd->block,afr->fr_File,fd->blocksize);
               if(file=Open(fd->block,MODE_OLDFILE))
               {  Seek(file,0,OFFSET_END);
               }
               else
               {  file=Open(fd->block,MODE_NEWFILE);
               }
               if(file)
               {  Write(file,mi->buf.buffer,mi->buf.length);
                  Close(file);
               }
            }
            FreeAslRequest(afr);
         }
      }
      result=0;
   }
   return (BOOL)(result==1);
}

/* Connect and transfer the message */
static short Sendmessage(struct Fetchdriver *fd,struct Mailinfo *mi)
{  struct Library *SocketBase;
   struct hostent *hent;
   long sock;
   UBYTE name[128],*to,*p,*q;
   BOOL error=FALSE;
   short retval=SMR_FAIL;

   AwebTcpBase=Opentcp(&SocketBase,fd,TRUE);
#if defined(__amigaos4__)
IAwebTcp = GetInterface(AwebTcpBase,"main",1,0);
#endif

   if(SocketBase)
   {  Updatetaskattrs(AOURL_Netstatus,NWS_LOOKUP,TAG_END);
      Tcpmessage(fd,TCPMSG_LOOKUP,fd->prefs->network.smtphost);
      ObtainSemaphore(fd->prefssema);
      hent=Lookup(fd->prefs->network.smtphost,SocketBase);
      ReleaseSemaphore(fd->prefssema);
      if(hent)
      {  if((sock=a_socket(hent->h_addrtype,SOCK_STREAM,0,SocketBase))>=0)
         {  Updatetaskattrs(AOURL_Netstatus,NWS_CONNECT,TAG_END);
            Tcpmessage(fd,TCPMSG_CONNECT,"SMTP",hent->h_name);
            if(!a_connect(sock,hent,25,SocketBase)
            && Getresponse(fd,sock,SocketBase)<400)
            {  a_gethostname(name,sizeof(name),SocketBase);
               if(Sendcommand(fd,sock,SocketBase,"HELO %s",name)<400)
               {  ObtainSemaphore(fd->prefssema);
                  strncpy(name,fd->prefs->network.emailaddr,sizeof(name));
                  ReleaseSemaphore(fd->prefssema);
                  Updatetaskattrs(AOURL_Netstatus,NWS_WAIT,TAG_END);
                  Tcpmessage(fd,TCPMSG_MAILSEND);
                  if(Sendcommand(fd,sock,SocketBase,"MAIL FROM:<%s>",name)<400)
                  {  to=Dupstr(mi->to,-1);
                     p=to;
                     while(p)
                     {  if(q=strchr(p,',')) *q++='\0';
                        if(Sendcommand(fd,sock,SocketBase,"RCPT TO:<%s>",p)>=400)
                        {  error=TRUE;
                           break;
                        }
                        p=q;
                     }
                     if(to) Freemem(to);
                     if(!error)
                     {  if(Sendcommand(fd,sock,SocketBase,"DATA")<400)
                        {  a_send(sock,mi->buf.buffer,mi->buf.length,0,SocketBase);
                           a_send(sock,".\r\n",3,0,SocketBase);
                           if(Getresponse(fd,sock,SocketBase)<400)
                           {  retval=SMR_OK;
                           }
                        }
                     }
                  }
               }
            }
            else retval=SMR_NOCONNECT;
            a_close(sock,SocketBase);
         }
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
   else retval=SMR_NOCONNECT;
   return retval;
}

/* Build result page */
static void Buildresult(struct Fetchdriver *fd,struct Mailinfo *mi)
{  long len;
   Freebuffer(&mi->buf);
   len=sprintf(fd->block,"<HTML>\n<HEAD>\n<TITLE>");
   Addtobuffer(&mi->buf,fd->block,len);
   len=sprintf(fd->block,AWEBSTR(MSG_MAIL_MAIL_SENT_TITLE),mi->to);
   Addtobuffer(&mi->buf,fd->block,len);
   len=sprintf(fd->block,"</TITLE>\n</HEAD>\n<BODY>\n");
   Addtobuffer(&mi->buf,fd->block,len);
   len=sprintf(fd->block,AWEBSTR(MSG_MAIL_MAIL_SENT),mi->to);
   Addtobuffer(&mi->buf,fd->block,len);
   len=sprintf(fd->block,"<p>\n<FORM ACTION='x-aweb:mail/2'>\n<INPUT TYPE=SUBMIT VALUE='%s'>\n</FORM>\n",
      AWEBSTR(MSG_MAIL_RETURN));
   Addtobuffer(&mi->buf,fd->block,len);
   len=sprintf(fd->block,"</BODY>\n</HTML>\n");
   Addtobuffer(&mi->buf,fd->block,len);
   Updatetaskattrs(
      AOURL_Contenttype,"text/html",
      AOURL_Data,mi->buf.buffer,
      AOURL_Datalength,mi->buf.length,
      TAG_END);
}

/* Send mail */
static void Sendmail(struct Fetchdriver *fd)
{  struct Mailinfo mi={0};
   short r;
   if(Findreturn(fd))
   {  Updatetaskattrs(
         AOURL_Gohistory,-1,
         TAG_END);
   }
   else
   {  Buildmessage(fd,&mi);
      for(;;)
      {  r=Sendmessage(fd,&mi);
         if(r==SMR_OK) break;
         if(!Mailretry(fd,&mi,r)) break;
      }
      if(r==SMR_OK)
      {  Buildresult(fd,&mi);
      }
      Freebuffer(&mi.buf);
      if(mi.to) FREE(mi.to);
   }
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
   if(STRNIEQUAL(fd->name,"mailto:",7))
   {  Buildmailform(fd);
   }
   else if(STRNIEQUAL(fd->name,"x-aweb:mail/1/mail",18))
   {  if(fd->referer
      && (STRNIEQUAL(fd->referer,"mailto:",7)
         || STRNIEQUAL(fd->referer,"x-aweb:news/reply?",18)))
      {  Sendmail(fd);
      }
   }
   else if(STRNIEQUAL(fd->name,"x-aweb:mail/2",13))
   {  Updatetaskattrs(
         AOURL_Gohistory,-2,
         TAG_END);
   }
   Updatetaskattrs(AOTSK_Async,TRUE,
      AOURL_Eof,TRUE,
      AOURL_Terminate,TRUE,
      TAG_END);
}

/*-----------------------------------------------------------------------*/

static ULONG Initaweblib(struct Library *libbase)
{
   if(!AwebModuleInit()) return FALSE;
   if(!(DOSBase = (struct DosLibrary *) OpenLibrary("dos.library",39))) return FALSE;
   if(!(AslBase=OpenLibrary("asl.library",OSNEED(39,44)))) return FALSE;
   if(!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library",39))) return FALSE;
   if(!(UtilityBase=OpenLibrary("utility.library",39))) return FALSE;

#if defined(__amigaos4__)
   if(!(IDOS = (struct DOSIFace *)GetInterface(DOSBase,"main",1,0))) return FALSE;
   if(!(IAsl = (struct AslIFace *)GetInterface(AslBase,"main",1,0))) return FALSE;
   if(!(IGraphics = (struct GraphicsIFace *)GetInterface(GfxBase,"main",1,0))) return FALSE;
   if(!(IUtility  = (struct UtilityIFace *)GetInterface(UtilityBase,"main",1,0))) return FALSE;
#endif

   return TRUE;
}

static void Expungeaweblib(struct Library *libbase)
{

#if defined(__amigaos4__)
if(IDOS)DropInterface((struct Interface *)IDOS);
if(IUtility)DropInterface((struct Interface *)IUtility);
if(IAsl)DropInterface((struct Interface *)IAsl);
if(IGraphics)DropInterface((struct Interface *)IGraphics);
#endif


   if(UtilityBase) CloseLibrary(UtilityBase);
   if(GfxBase) CloseLibrary(GfxBase);
   if(AslBase) CloseLibrary(AslBase);
   if(DOSBase) CloseLibrary(DOSBase);

   AwebModuleExit();
}

#endif /* !LOCALONLY */
