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

/* gopher.c - AWeb gopher client */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "aweblib.h"
#include "tcperr.h"
#include "fetchdriver.h"
#include "task.h"
#include "awebtcp.h"
#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"
#include <proto/exec.h>
#include <exec/resident.h>

#define GOPHER_VERSION 36
#define GOPHER_REVISION 1
#define GOPHER_VERSTRING "36.1"

#ifdef __MORPHOS__
ULONG __abox__ =1;
#endif


#ifndef LOCALONLY

struct Gopheraddr
{  UBYTE *buf;
   UBYTE *hostname;
   long port;
   UBYTE *selector;
   UBYTE type;
   BOOL query;
};

struct Library *AwebTcpBase;
struct Library *AwebSupportBase;

#if defined (__amigaos4__)

struct AwebTcpIFace *IAwebTcp;
struct AwebSupportIFace *IAwebSupport;

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
    __saveds static Subtaskfunction *, GetTaskFunc,
    ULONG, id, D0,
    FTP_TYPE, FTP_NAME
);

__saveds static void Fetchdrivertask(struct Fetchdriver *fd);

/* Function declarations for project dependent hook functions */
static ULONG Initaweblib(struct Library *libbase);
static void Expungeaweblib(struct Library *libbase);

struct Library *GopherBase;

static APTR libseglist;

struct ExecBase *SysBase;


LIBSTART_DUMMY

static char __aligned libname[]="gopher.aweblib";
static char __aligned libid[]="$VER: gopher.aweblib " GOPHER_VERSTRING " (" __AMIGADATE__ ") " CPU;

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
    GOPHER_VERSION,
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
   GOPHER_VERSION,
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
    libBase->lib_Version      = GOPHER_VERSION;
    libBase->lib_Revision     = GOPHER_REVISION;
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
   GopherBase=libbase;
   libbase->lib_Revision=GOPHER_REVISION;
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
   struct Library *Gopherlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Gopherlibbase = (struct Library *)LIBMAN_NAME;
#endif

   Gopherlibbase->lib_OpenCnt++;
   Gopherlibbase->lib_Flags&=~LIBF_DELEXP;
   if(Gopherlibbase->lib_OpenCnt==1)
   {
      if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return (struct Library *) Real_Closelib(LIBMAN_NAME);
#if defined (__amigaos4__)
      if(!( IAwebSupport = (struct AwebSupportIface *)GetInterface(AwebSupportBase,"main",1,0))) return Real_Closelib(LIBMAN_NAME);
#endif
   }
   return Gopherlibbase;

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
   struct Library *Gopherlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Gopherlibbase = (struct Library *)LIBMAN_NAME;
#endif


  Gopherlibbase->lib_OpenCnt--;
   if(Gopherlibbase->lib_OpenCnt==0)
   {  if(AwebSupportBase)
      {
#if defined(__amigaos4__)
    if(IAwebSupport) DropInterface((struct Interface *)IAwebSupport);
    IAwebSupport = NULL;
#endif

         CloseLibrary(AwebSupportBase);
         AwebSupportBase=NULL;
      }
      if(Gopherlibbase->lib_Flags&LIBF_DELEXP)
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
   struct Library *Gopherlibbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
   struct Library *Gopherlibbase = (struct Library *)LIBMAN_NAME;
#endif


  if(Gopherlibbase->lib_OpenCnt==0)
   {  ULONG size=Gopherlibbase->lib_NegSize+Gopherlibbase->lib_PosSize;
      UBYTE *ptr=(UBYTE *)Gopherlibbase-Gopherlibbase->lib_NegSize;
      Remove((struct Node *)Gopherlibbase);
      Expungeaweblib(Gopherlibbase);
#if defined(__amigaos4__)
    DeleteLibrary(Gopherlibbase);
#else
      FreeMem(ptr,size);
#endif

      return libseglist;
   }
   Gopherlibbase->lib_Flags|=LIBF_DELEXP;
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

static BOOL Makegopheraddr(struct Gopheraddr *ha,UBYTE *name)
{  long len=strlen(name);
   UBYTE *p,*q;
   ha->buf=ALLOCTYPE(UBYTE,len+2,0);
   if(!ha->buf) return FALSE;
   p=name;
   ha->hostname=q=ha->buf;
   while(*p && *p!='/' && *p!=':') *q++=*p++;
   *q++='\0';
   if(*p==':')
   {  ha->port=0;
      p++;
      while(isdigit(*p))
      {  ha->port=10*ha->port+(*p-'0');
         p++;
      }
   }
   else ha->port=70;
   while(*p && *p!='/') p++;
   if(*p) p++; /* skip / */
   if(*p)
   {  ha->type=*p++;
      strcpy(q,p);
   }
   else
   {  ha->type='1';
      *q='\0';
   }
   ha->selector=q;
   if(ha->type=='7' && (p=strchr(ha->selector,'?')))
   {  ha->query=TRUE;
      *p='\t';
   }
   Urldecode(ha->hostname);
   Urldecode(ha->selector);
   return TRUE;
}

/*-----------------------------------------------------------------------*/

struct GResponse
{  struct Buffer buf;
   BOOL headerdone;
};

static UBYTE *Findtab(UBYTE *p,UBYTE *end)
{  while(p<end && *p!='\t' && *p!='\r' && *p!='\n') p++;
   if(p<end) return p;
   else return NULL;
}

static UBYTE *Findeol(UBYTE *p,UBYTE *end)
{  while(p<end && *p!='\r' && *p!='\n') p++;
   if(p<end) return p;
   else return NULL;
}

static void Builddir(struct Fetchdriver *fd,struct GResponse *resp,long read)
{  UBYTE *p,*end,*descr,*selector,*host,*hport,*plus;
   UBYTE type;
   UBYTE *icon;
   long length=0;
   if(!Addtobuffer(&resp->buf,fd->block,read)) return;
   if(!resp->headerdone)
   {  sprintf(fd->block,"<html><h1>%s</h1>",AWEBSTR(MSG_AWEB_GOPHERMENU));
      length=strlen(fd->block);
      resp->headerdone=TRUE;
   }
   for(;;)
   {  p=resp->buf.buffer;
      end=p+resp->buf.length;
      while(p<end && (*p=='\n' || *p=='\r')) p++;
      if(p>=end) break;
      type=*p;
      descr=++p;
      if(!(p=Findtab(p,end))) break;
      *p='\0';
      selector=++p;
      if(!(p=Findtab(p,end))) break;
      *p='\0';
      host=++p;
      if(!(p=Findtab(p,end))) break;
      *p='\0';
      hport=++p;
/* check for gopher+ */
      if(!(p=Findtab(p,end))) break;
      *p='\0';
      plus=++p;

      if(!(p=Findtab(p,end))) break;
      if(*p=='\r' || *p=='\n') *p='\0';
      else
      {  *p='\0';
         if(!(p=Findeol(p,end))) break;
      }
      switch(type)
      {  case '0':icon="&text.document;";break;
         case '1':icon="&folder;";break;

         case '2':icon="&telephone;";break;
         case '3':icon=NULL;break;              /* error*/

         case '4':icon="&binhex.document;";break;
         case '5':icon="&archive;";break;
         case '6':icon="&uuencoded.document;";break;
         case '7':icon="&index;";break;
                 case '8':icon="&telnet;";break;
         case '9':icon="&binary.document;";break;

         case '+':icon="&redunt;";break;
         case 'T':icon="&tn3270;";break;
                 case 'g':                                              /* gif */
         case 'I':                                              /* image */
         case ':':icon="&image;";break; /* gopher+ image */

         case 'h':icon="&html;";break;  /* html */
         case 'i':icon=NULL;break;              /* info */
         case 'w':icon="&www;";break;   /* www link */

         case 's':
                 case '<':icon="&audio;";break; /* sound */
         case ';':icon="&film;";break; /* gopher+ movie */
         default: icon=NULL;
      }
      if(icon)
                switch(type)
                {
                        case '0':
                        case '1':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '9':
                        case '+':
                        case 'g':
                        case 'h':
                        case 'I':
                        case 's':
                        {
                            length+=sprintf(fd->block+length,"<BR>%s <A HREF=\"gopher://%s:%s/%c%s\">%s</A>",icon,host,hport,type,selector,descr);
                                if(length>INPUTBLOCKSIZE-1000)
                        {  Updatetaskattrs(
                       AOURL_Data,(Tag)fd->block,
                   AOURL_Datalength,length,
                   TAG_END);
                   length=0;
                                }
                                break;

/* telnet type */
                        case '8':
                        {
                                length+=sprintf(fd->block+length,"<BR>%s <A HREF=\"telnet://%s:%s%s\">%s</A>",icon,host,hport,selector,descr);
                                if(length>INPUTBLOCKSIZE-1000)
                        {  Updatetaskattrs(
                       AOURL_Data,(Tag)fd->block,
                   AOURL_Datalength,length,
                   TAG_END);
                   length=0;
                                }
                                break;
                        }
/* www link type */
                        case 'w':
                        {
                                length+=sprintf(fd->block+length,"<BR>%s <A HREF=\"http://%s:%s%s\">%s</A>",icon,host,hport,selector,descr);
                                if(length>INPUTBLOCKSIZE-1000)
                        {  Updatetaskattrs(
                       AOURL_Data,(Tag)fd->block,
                   AOURL_Datalength,length,
                   TAG_END);
                   length=0;
                                }
                                break;
                        }
/* info link type */
                        case 'i':
                        {
                                length+=sprintf(fd->block+length,"<BR>%s",descr);
                                if(length>INPUTBLOCKSIZE-1000)
                        {  Updatetaskattrs(
                       AOURL_Data,(Tag)fd->block,
                   AOURL_Datalength,length,
                   TAG_END);
                   length=0;
                                }
                                break;
                        }
                        default:
                        break;
                }

/* show any infomations that doesnt have a knowen type */
      } else {
         length+=sprintf(fd->block+length,"<BR>%s",descr);
         if(length>INPUTBLOCKSIZE-1000)
         {  Updatetaskattrs(
               AOURL_Data,(Tag)fd->block,
               AOURL_Datalength,length,
               TAG_END);
            length=0;
         }
          }
      p++;
      Deleteinbuffer(&resp->buf,0,p-resp->buf.buffer);
   }
   if(length)
   {  Updatetaskattrs(
         AOURL_Data,(Tag)fd->block,
         AOURL_Datalength,length,
         TAG_END);
   }
}

static void Deleteperiods(struct Fetchdriver *fd,struct GResponse *resp,long read)
{  UBYTE *p,*end,*begin;
   long length=0;
   if(!Addtobuffer(&resp->buf,fd->block,read)) return;
   for(;;)
   {  p=resp->buf.buffer;
      end=p+resp->buf.length;
      if(p>=end) break;
      if(*p=='.') p++;
      begin=p;
      if(!(p=Findeol(p,end))) break;
      p++;
      memmove(fd->block+length,begin,p-begin);
      length+=p-begin;
      if(length>INPUTBLOCKSIZE-1000)
      {  Updatetaskattrs(
            AOURL_Data,(Tag)fd->block,
            AOURL_Datalength,length,
            TAG_END);
         length=0;
      }
      Deleteinbuffer(&resp->buf,0,p-resp->buf.buffer);
   }
   if(length)
   {  Updatetaskattrs(
         AOURL_Data,(Tag)fd->block,
         AOURL_Datalength,length,
         TAG_END);
   }
}

static void Makeindex(struct Fetchdriver *fd)
{  long length;
/* we ought to do differen msg for cso and normal indexs */
   sprintf(fd->block,"<html><h1>%s</h1><isindex>",AWEBSTR(MSG_AWEB_GOPHERINDEX));
   length=strlen(fd->block);
   Updatetaskattrs(
      AOURL_Data,(Tag)fd->block,
      AOURL_Datalength,length,
      TAG_END);
}

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
   struct Gopheraddr ha={0};
   struct hostent *hent;
   struct GResponse resp={{0}};
   BOOL error=FALSE;
   long sock;
   long result,length;
   AwebTcpBase=Opentcp(&SocketBase,fd,!fd->validate);
#if defined(__amigaos4__)
    IAwebTcp = (struct AwebTcpIFace *)GetInterface(AwebTcpBase,"main",1,0);
#endif
   if(SocketBase)
   {  if(Makegopheraddr(&ha,fd->name))
/* both index and CSO are index fields */
      {  if ((ha.type=='7' && !ha.query) || (ha.type=='2' && !ha.query) )
         {  Makeindex(fd);
                 }
         else
         {  Updatetaskattrs(AOURL_Netstatus,NWS_LOOKUP,TAG_END);
            Tcpmessage(fd,TCPMSG_LOOKUP,ha.hostname);
            if(hent=Lookup(ha.hostname,SocketBase))
            {  if((sock=a_socket(hent->h_addrtype,SOCK_STREAM,0,SocketBase))>=0)
               {  Updatetaskattrs(AOURL_Netstatus,NWS_CONNECT,TAG_END);
                  Tcpmessage(fd,TCPMSG_CONNECT,"Gopher",hent->h_name);
                  if(!a_connect(sock,hent,ha.port,SocketBase))
                  {  length=sprintf(fd->block,"%s\r\n",ha.selector);
                     result=(a_send(sock,fd->block,length,0,SocketBase)==length);
                     if(result)
                     {  Updatetaskattrs(AOURL_Netstatus,NWS_WAIT,TAG_END);
                        Tcpmessage(fd,TCPMSG_WAITING,"Gopher");
                        for(;;)
                        {  length=a_recv(sock,fd->block,INPUTBLOCKSIZE,0,SocketBase);
                           if(length<0 || Checktaskbreak())
                           {  error=TRUE;
                              break;
                           }
                           if(ha.type=='1' || ha.type=='7')
                           {  Builddir(fd,&resp,length);
                           }
                           else if(ha.type=='0')
                           {  Deleteperiods(fd,&resp,length);
                           }
                           else
                           {  Updatetaskattrs(
                                 AOURL_Data,(Tag)fd->block,
                                 AOURL_Datalength,length,
                                 TAG_END);
                           }
                           if(length==0) break;
                        }
                        a_shutdown(sock,2,SocketBase);
                     }
                     else error=TRUE;
                  }
                  else
                  {  Tcperror(fd,TCPERR_NOCONNECT,hent->h_name);
                  }
                  a_close(sock,SocketBase);
               }
               else error=TRUE;
            }
            else
            {
               Tcperror(fd,TCPERR_NOHOST,ha.hostname);
            }
         }
         FREE(ha.buf);
      }
      else error=TRUE;
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
   Freebuffer(&resp.buf);
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

    return TRUE;
}

static void Expungeaweblib(struct Library *libbase)
{

   AwebModuleExit();
}

#endif /* LOCALONLY */
