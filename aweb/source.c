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

/* source.c - AWeb source interpreter object */

#include "aweb.h"
#include "source.h"
#include "sourcedriver.h"
#include "copy.h"
#include "copydriver.h"
#include "url.h"
#include "file.h"
#include "filereq.h"
#include "application.h"
#include "plugin.h"
#include "fetch.h"
#include "info.h"

#include <proto/utility.h>
#include <proto/awebplugin.h>

#if defined(__amigaos4__)
#define IAwebPlugin      ((struct AwebPluginIFace *)src->pluginbase)
#else
#undef AWEBPLUGIN_BASE_NAME
#define AWEBPLUGIN_BASE_NAME ((struct Library *)src->pluginbase)
#endif

#ifdef DEVELOPER
extern BOOL charsetdebug;
#endif
/*------------------------------------------------------------------------*/

struct Source
{  struct Aobject object;
   void *url;                 /* Url pointed to */
   LIST(Copyref) copies;      /* Copy objects from this source */
   void *driver;              /* Source driver */
   long length;
   UBYTE contenttype[32];     /* Content type */
   UBYTE defaulttype[32];     /* Default content type */
   UBYTE charset[32];         /* Character set */
   UBYTE etag[64];            /* Etag gvb etag */
   ULONG sdtype,cdtype;       /* Driver types */
   UWORD flags;
   long dispcount;
   void *savereq;             /* Save source requester */
   long memory;               /* Memory occupied by driver */
   void *pluginbase;          /* Library base of open plugin or Interface for os4 must be open with Openaweblib()*/
   void *plugindata;          /* Plugin private data */
   UBYTE *savename;           /* Name to save under */
   UBYTE *filename;           /* Filename suggested by Content-Disposition */
   ULONG lastmodified;        /* Date */
   void *serverpush;          /* FETCH object to cancel if no longer displayed. */
   BOOL  clientpull;          /* This source has a client pull associated with it */
   UBYTE *cipher;             /* Cipher method used */
   UBYTE *ssllibrary;         /* SSL library used */
   LIST(Header) headers;      /* HTTP headers */
};

#define SRCF_SAVEAS     0x0001   /* Only create SAVEAS driver */
#define SRCF_ERROR      0x0002   /* Source is in error */
#define SRCF_SAVEAPPEND 0x0004   /* Create SAVEAS driver in append mode */
#define SRCF_EOF        0x0008   /* EOF was reached on data */
#define SRCF_CACHE      0x0010   /* Read from cache (info) */
#define SRCF_NOICON     0x0020   /* No icons when saving */
#define SRCF_NOFLUSH    0x0040   /* Never flush this source */
#define SRCF_DOCEXT     0x0080   /* Only create DOCEXT driver */
#define SRCF_FILTER     0x0100   /* Data should go through filter */
#define SRCF_FOREIGN    0x0200   /* Data uses foreign character set */

struct Copyref
{  NODE(Copyref);
   struct Aobject *object;
   UWORD flags;
};

#define COPF_DRIVER     0x0001   /* Copy has a driver */

struct Header
{  NODE(Header);
   UBYTE *text;
};

static LIST(Source) sources;

static long totalmemory=0;


/*------------------------------------------------------------------------*/

/* Send this message to all children */
static void Broadcast(struct Source *src,struct Amessage *amsg)
{  struct Copyref *cop,*prev;
   /* If for some reason (defective server) the copies are set a new source
    * that is the same, they will remove and add themselves to this list.
    * In order to prevent the endless loop this would lead to, the list is
    * scanned backwards. */
   for(cop=src->copies.last;cop->prev;cop=prev)
   {  prev=cop->prev;
      AmethodA(cop->object,amsg);
   }
}

/* Set these attributes to all children */
VARARGS68K_DECLARE(static void Setchildren(struct Source *src,...))
{
   struct Amset ams={{0}};
   VA_LIST ap;
   VA_STARTLIN(ap,src);

   ams.amsg.method=AOM_SET;
   ams.tags=(struct TagItem*)VA_GETLIN(ap, struct TagItem *);
   Broadcast(src,(struct Amessage *)&ams);
}

/* Create a new copydriver */
static void *Newcopydriver(struct Source *src,void *cop)
{  void *cd=NULL;
   if(src->driver && src->cdtype)
   {  cd=Anewobject(src->cdtype,
         AOCDV_Copy,(Tag)cop,
         AOCDV_Sourcedriver,(Tag)src->driver,
         TAG_END);
   }
   return cd;
}

/* Create a copydriver for all copies that don't have a driver yet. */
static void Usedriver(struct Source *src)
{  struct Copyref *cop;
   void *cd;
   if(src->driver && src->cdtype)
   {  for(cop=src->copies.first;cop->next;cop=cop->next)
      {  if(!(cop->flags&COPF_DRIVER))
         {  if(cd=Newcopydriver(src,cop->object))
            {  Asetattrs(cop->object,AOCPY_Driver,(Tag)cd,TAG_END);
               cop->flags|=COPF_DRIVER;
               Aaddchild(src->driver,cop->object,AOREL_SRC_COPY);
            }
         }
      }
   }
}

/* Open this plugin and determine how it is to be used */
static void Openplugin(struct Source *src,UBYTE *name)
{
  if(src->pluginbase=Openaweblib(name,0))
   {  struct Plugininfo pi={0};
      if(Initplugin(&pi))
      {  src->sdtype=pi.sourcedriver;
         src->cdtype=pi.copydriver;
         if(!src->sdtype)
         {
/* If we are not SD then we may be a filter. We call Queryplugin to find out   */
/* If we are running os3 then we check the libNegsize to ensure Query exists   */
/* From now on it is bad practice not to include Queryplugin() in a plugin but */
/* we might be loading an old m68k plugin without it.                          */
/* For OS4 Query plugin **MUST** exist!.                                       */

#if defined(__amigaos4__)
           {
            struct Pluginquery pq = {0};
            pq.structsize = sizeof(pq);
            Queryplugin(&pq);
               if(pq.filter)
               {  src->sdtype=AOTP_DOCSOURCE;
                  src->cdtype=AOTP_DOCUMENT;
                  src->flags|=SRCF_FILTER;
               }
           }

#else
           if(((struct Library *)src->pluginbase)->lib_NegSize>=36)
            {  struct Pluginquery pq={0};
               pq.structsize=sizeof(pq);
               Queryplugin(&pq);
               if(pq.filter)
               {  src->sdtype=AOTP_DOCSOURCE;
                  src->cdtype=AOTP_DOCUMENT;
                  src->flags|=SRCF_FILTER;
               }
            }
#endif
         }
      }
      else
      {
         Remaweblib(src->pluginbase);
         Closeaweblib(src->pluginbase);
         src->pluginbase=NULL;
      }
   }
}

/* Bind a driver to this source, and copydrivers to our copies. */
static void Adddriver(struct Source *src,UBYTE *data,long length)
{  long drt;
   UBYTE *name=NULL,*args=NULL,*type=NULL,*charset=NULL,*etag=NULL;
   ULONG extprogtag=TAG_IGNORE;
   ULONG noicontag=TAG_IGNORE;
   ULONG unknowntag=TAG_IGNORE;
   BOOL cancel = FALSE;;
   if(src->driver)
   {  Setchildren(src,AOCPY_Driver,NULL,TAG_END);
      Adisposeobject(src->driver);
      src->driver=NULL;
      if(src->pluginbase)
      {
         Remaweblib(src->pluginbase);
         Closeaweblib(src->pluginbase);
         src->pluginbase=NULL;
      }
      totalmemory-=src->memory;
      src->memory=0;
   }

   /* If server has reported a MIME type, use it, else use the one from cache */
   if(*src->contenttype)
   {
      type=src->contenttype;
      charset=src->charset;
   }
   else
   {
      type=(UBYTE *)Agetattr(src->url,AOURL_Contenttype);
      charset=(UBYTE *)Agetattr(src->url,AOURL_Charset);
   }
   etag=(UBYTE *)Agetattr(src->url,AOURL_Etag);

   /* See if it is reasonable */
   if(type && !Checkmimetype(data,length,type))
   {
      type=NULL;
      charset=NULL;
   }

   if(!type)
   {  /* No reasonable type, find the one from MIME settings */
      type=Mimetypefromext((UBYTE *)Agetattr(src->url,AOURL_Url));
      /* And see if that is reasonable */
      if(type && !Checkmimetype(data,length,type)) type=NULL;
      if(type) strcpy(src->contenttype,type);
   }

   if(!type)
   {  /* Still no reasonable type, try to infer from data */
      type=Mimetypefromdata(data,length,src->defaulttype);
      if(type) strcpy(src->contenttype,type);
   }

   if(charset)
   {
#ifdef DEVELOPER
      if (charsetdebug) printf("source.c/Adddriver(): charset for the document: %s\n",charset);
#endif
      strcpy(src->charset,charset);
   }
#ifdef DEVELOPER
   else
      if (charsetdebug) printf("source.c/Adddriver(): charset for the document not specified\n");
#endif

   if(src->flags&SRCF_SAVEAS)
   {  src->sdtype=AOTP_SAVEAS;
      src->cdtype=0;
      if(src->flags&SRCF_NOICON) noicontag=AOSDV_Noicon;
   }
   else if(src->flags&SRCF_DOCEXT)
   {  src->sdtype=AOTP_DOCEXT;
      src->cdtype=0;
   }
   else
   {  drt=Getmimedriver(type,(UBYTE *)Agetattr(src->url,AOURL_Url),&name,&args);
      if(*src->defaulttype && drt==MIMEDRV_DOCUMENT
      && !STRNIEQUAL(src->defaulttype,"TEXT/",5))
      {  /* NEVER attempt to use a document as an embedded object */
         drt=0;
      }
      if(*prefs.program.imgvcmd
      && STRNIEQUAL(src->defaulttype,"TEXT/",5)
      && type && STRNIEQUAL(type,"IMAGE/",6))
      {  /* View image using separate image viewer */
         drt=MIMEDRV_EXTPROG;
         name=prefs.program.imgvcmd;
         args=prefs.program.imgvargs;
         Setchildren(src,AOCPY_Nodisplay,TRUE,TAG_END);
      }
      src->sdtype=src->cdtype=0;
      switch(drt)
      {  case MIMEDRV_DOCUMENT:
            src->sdtype=AOTP_DOCSOURCE;
            src->cdtype=AOTP_DOCUMENT;
            break;
         case MIMEDRV_IMAGE:
            src->sdtype=AOTP_IMGSOURCE;
            src->cdtype=AOTP_IMGCOPY;
            break;
         case MIMEDRV_SOUND:
            src->sdtype=AOTP_SOUNDSOURCE;
            src->cdtype=AOTP_SOUNDCOPY;
            break;
         case MIMEDRV_PLUGIN:
            Openplugin(src,name);
            if(!src->sdtype)
            {  src->sdtype=AOTP_SAVEAS;
               src->cdtype=0;
            }
            break;
         case MIMEDRV_EXTPROG:
            src->sdtype=AOTP_EXTPROG;
            src->cdtype=0;
            break;
         case MIMEDRV_EXTPROGPIPE:
            src->sdtype=AOTP_EXTPROG;
            src->cdtype=0;
            extprogtag=AOSDV_Pipe;
            break;
         case MIMEDRV_EXTPROGNOFTCH:
            src->sdtype=AOTP_EXTPROG;
            src->cdtype=0;
            extprogtag=AOSDV_Nofetch;
            break;
         case MIMEDRV_CANCEL:
            cancel = TRUE;
            src->sdtype = 0;;
            src->cdtype = 0;
            break;
         default:
            if(!(src->flags&SRCF_ERROR))
            {  src->sdtype=AOTP_SAVEAS;
               src->cdtype=0;
               if(drt==MIMEDRV_NONE)
               {  unknowntag=AOSDV_Unknowntype;
               }
            }
      }
   }
   if(src->sdtype)
   {  src->driver=Anewobject(src->sdtype,
         AOSDV_Source,(Tag)src,
         AOURL_Contenttype,(Tag)type,
         AOURL_Contentlength,src->length,
         AOSDV_Name,(Tag)name,
         AOSDV_Arguments,(Tag)args,
         extprogtag,TRUE,
         noicontag,TRUE,
         unknowntag,(Tag)type,
         AOSDV_Displayed,src->dispcount>0,
         TAG_END);
      if(src->driver && src->cdtype)
      {  Usedriver(src);
      }
      if(src->driver && (src->flags&SRCF_SAVEAS) && (src->filename || src->savename))
      {  void *file=Anewobject(AOTP_FILE,
            AOFIL_Name,src->savename?(Tag)src->savename:(Tag)src->filename,
            AOFIL_Delete,FALSE,
            AOFIL_Append,BOOLVAL(src->flags&SRCF_SAVEAPPEND),
            AOFIL_Comment,Agetattr(src->url,AOURL_Url),
            TAG_END);
         if(file)
         {  Asetattrs(src->driver,AOSDV_Savesource,(Tag)file,TAG_END);
         }
      }
   }
   if(cancel)
   {
       if(src->url)
       {
           Auspecial(src->url,AUMST_CANCELFETCH);
       }
   }
}

/* Save the source. */
static void Savesource(struct Source *src,UBYTE *name,BOOL append)
{  void *file;
   short icontype=FILEICON_NONE;
   if(Agetattr(src->driver,AOSDV_Saveable))
   {  ObtainSemaphore(&prefssema);
      if(prefs.program.saveicons && !(src->flags&SRCF_NOICON))
      {  if(STRNIEQUAL(src->contenttype,"TEXT/",5)) icontype=FILEICON_TEXT;
         else icontype=FILEICON_DATA;
      }
      ReleaseSemaphore(&prefssema);
      if(file=Anewobject(AOTP_FILE,
         AOFIL_Name,(Tag)name,
         AOFIL_Append,append,
         AOFIL_Delete,FALSE,
         AOFIL_Icontype,icontype,
         AOFIL_Comment,Agetattr(src->url,AOURL_Url),
         TAG_END))
      {  Asetattrs(src->driver,AOSDV_Savesource,(Tag)file,TAG_END);
         Asetattrs(file,AOFIL_Eof,TRUE,TAG_END);
         Adisposeobject(file);
      }
      else
      {  name=Savepath(src->url);
         src->savereq=Anewobject(AOTP_FILEREQ,
            AOFRQ_Title,(Tag)AWEBSTR(MSG_FILE_SAVETITLE),
            AOFRQ_Filename,(Tag)name,
            AOFRQ_Savemode,TRUE,
            AOFRQ_Savecheck,TRUE,
            AOBJ_Target,(Tag)src,
            TAG_END);
         if(name) FREE(name);
      }
   }
}

/* Save the source in a temp file and spawn the viewer. */
static void Viewsource(struct Source *src)
{  void *temp;
   long viewable=FALSE,saveable=FALSE;
   UBYTE *filename;
   Agetattrs(src->driver,
      AOSDV_Saveable,(Tag)&saveable,
      AOSDV_Viewable,(Tag)&viewable,
      TAG_END);
   if(prefs.program.viewcmd && prefs.program.viewargs && saveable && viewable)
   {  if(temp=Anewobject(AOTP_FILE,
         AOFIL_Delete,FALSE,
         TAG_END))
      {  Asetattrs(src->driver,AOSDV_Savesource,(Tag)temp,TAG_END);
         Asetattrs(temp,AOFIL_Eof,TRUE,TAG_END);
         if(filename=Fullname((UBYTE *)Agetattr(temp,AOFIL_Name)))
         {  if(!Spawn(TRUE,prefs.program.viewcmd,prefs.program.viewargs,"fn",
               filename,Agetattr(Aweb(),AOAPP_Screenname)))
            {  Asetattrs(temp,AOFIL_Delete,TRUE,TAG_END);
            }
            FREE(filename);
         }
         Adisposeobject(temp);
      }
   }
}

/* Flush this source's driver and all copy drivers */
static void Flushsource(struct Source *src)
{  struct Copyref *cop,*next;
   for(cop=src->copies.first;cop->next;cop=next)
   {  next=cop->next;
      Asetattrs(cop->object,AOCPY_Driver,0,TAG_END);
      cop->flags&=~COPF_DRIVER;
   }
   if(src->driver)
   {  Adisposeobject(src->driver);
      src->driver=NULL;
      if(src->pluginbase)
      {
         Remaweblib(src->pluginbase);
         Closeaweblib(src->pluginbase);
         src->pluginbase=NULL;
      }
      totalmemory-=src->memory;
      src->memory=0;
   }
}

/* Flush all sources matching flush type */
static void Flushsourcetype(UWORD type)
{  struct Source *src;
   BOOL flush;
   for(src=sources.first;src->object.next;src=src->object.next)
   {  if(src->driver && (src->flags&SRCF_EOF) && !(src->flags&SRCF_NOFLUSH))
      {  flush=FALSE;
         switch(type)
         {  case SRCFT_NDIMAGES:
               flush=(src->sdtype!=AOTP_DOCSOURCE && !src->dispcount
                   /* && src->sdtype!=AOTP_SAVEAS */ );
               break;
            case SRCFT_ALLIMAGES:
               flush=(src->sdtype!=AOTP_DOCSOURCE /* && src->sdtype!=AOTP_SAVEAS */);
               break;
            case SRCFT_NDDOCUMENTS:
               flush=(src->sdtype==AOTP_DOCSOURCE && !src->dispcount);
               break;
         }
         if(flush) Flushsource(src);
      }
   }
   Doupdateframes();
}

/* Flush all sources beyond memory limit. First flush all nondisplayed objects,
 * then flush (displayed) nondocuments if keep-minimum free isn't met. */
static void Flushexcess(void)
{  struct Source *src;
   long max=prefs.network.camemsize*1024;
   long minfast=prefs.network.minfreefast*1024;
   long minchip=prefs.network.minfreechip*1024;
   if(AvailMem(MEMF_CHIP)<minchip || AvailMem(MEMF_FAST)<minfast)
   {  void *p=AllocVec(AvailMem(MEMF_TOTAL),0);
      if(p) FreeVec(p);
   }
   for(src=sources.first;src->object.next &&
      (totalmemory>max || AvailMem(MEMF_CHIP)<minchip || AvailMem(MEMF_FAST)<minfast);
      src=src->object.next)
   {  if(src->memory && !src->dispcount && (src->flags&SRCF_EOF) && !(src->flags&SRCF_NOFLUSH))
      {  Flushsource(src);
      }
   }
   for(src=sources.first;src->object.next &&
      (AvailMem(MEMF_CHIP)<minchip || AvailMem(MEMF_FAST)<minfast);
      src=src->object.next)
   {  if(src->memory && src->sdtype!=AOTP_DOCSOURCE && !(src->flags&SRCF_NOFLUSH))
      {  Flushsource(src);
      }
   }
}

/* Create nice info window lines */
static void Makeinfo(struct Source *src,void *inf)
{  struct Header *hdr;
   UBYTE *str,*buf;
   Asetattrs(inf,AOINF_Url,(Tag)src->url,TAG_END);
   if(src->flags&SRCF_CACHE)
   {  Asetattrs(inf,
         AOINF_Text,(Tag)AWEBSTR(MSG_INFO_FROMCACHE),
         TAG_END);
   }
   if(src->ssllibrary)
   {  str=AWEBSTR(MSG_INFO_SSLLIBRARY);
      if(buf=ALLOCTYPE(UBYTE,strlen(str)+strlen(src->ssllibrary)+4,0))
      {  sprintf(buf,str,src->ssllibrary);
         Asetattrs(inf,
            AOINF_Text,(Tag)buf,
            TAG_END);
         FREE(buf);
      }
   }
   if(src->cipher)
   {  str=AWEBSTR(MSG_INFO_CIPHER);
      if(buf=ALLOCTYPE(UBYTE,strlen(str)+strlen(src->cipher)+4,0))
      {  sprintf(buf,str,src->cipher);
         Asetattrs(inf,
            AOINF_Text,(Tag)buf,
            TAG_END);
         FREE(buf);
      }
   }
   if(!ISEMPTY(&src->headers))
   {  Asetattrs(inf,
         AOINF_Text,(Tag)AWEBSTR(MSG_INFO_XFER),
         AOINF_Header,TRUE,
         TAG_END);
      for(hdr=src->headers.first;hdr->next;hdr=hdr->next)
      {  Asetattrs(inf,AOINF_Text,(Tag)hdr->text,TAG_END);
      }
   }
}

/* Pass data through filter, then to sourcedriver. */
static void Filterdata(struct Source *src,void *fetch,UBYTE *data,long length,BOOL eof)
{
   struct Sourcefilter sf={{0}};
   struct Pluginfilter pf={0};
   pf.structsize=sizeof(pf);
   pf.handle=&sf;
   pf.data=data;
   pf.length=length;
   pf.eof=eof;
   pf.contenttype=src->contenttype;
   pf.url=(STRPTR)Agetattr(src->url,AOURL_Url);
   pf.userdata = src->plugindata;
   pf.encoding =  (STRPTR)Agetattr(src->url,AOURL_Charset);
   Filterplugin(&pf);
   src->plugindata=pf.userdata;
   if(*sf.contenttype)
   {  strncpy(src->contenttype,sf.contenttype,sizeof(src->contenttype)-1);
   }
   Asrcupdatetags(src->driver,fetch,
      (*sf.contenttype)?AOURL_Contenttype:TAG_IGNORE,(Tag)src->contenttype,
      sf.buf.length?AOURL_Data:TAG_IGNORE,(Tag)sf.buf.buffer,
      sf.buf.length?AOURL_Datalength:TAG_IGNORE,sf.buf.length,
      eof?AOURL_Eof:TAG_IGNORE,eof,
      TAG_END);
   Freebuffer(&sf.buf);
}

/*------------------------------------------------------------------------*/

static long Setsource(struct Source *src,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   UBYTE *name;
   BOOL flushexcess=FALSE,savesource=FALSE,saveappend=FALSE;
   UBYTE *savename=NULL;
   void *srca;
   ULONG urltag;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSRC_Url:
            if(!src->url) src->url=(void *)tag->ti_Data;
            break;
         case AOSRC_Usedriver:
            Usedriver(src);
            break;
         case AOSRC_Movetourl:
            src->serverpush=NULL;
            if(src->flags&SRCF_SAVEAS) urltag=AOURL_Saveassource;
            else if(src->flags&SRCF_DOCEXT) urltag=AOURL_Docextsource;
            else urltag=AOURL_Source;
            srca=(void *)Agetattr((void *)tag->ti_Data,urltag);
            Asetattrs(srca,
               AOSRC_Defaulttype,(Tag)src->defaulttype,
               AOSRC_Saveas,src->flags&SRCF_SAVEAS,
               AOSRC_Savename,(Tag)src->savename,
               AOSRC_Saveappend,src->flags&SRCF_SAVEAPPEND,
               AOSRC_Noicon,src->flags&SRCF_NOICON,
               AOSRC_Docext,src->flags&SRCF_DOCEXT,
               TAG_END);
            Setchildren(src,AOCPY_Source,srca,TAG_END);
            break;
         case AOSRC_Saveas:
            if(tag->ti_Data) src->flags|=SRCF_SAVEAS;
            else src->flags&=~SRCF_SAVEAS;
            break;
         case AOSRC_Savesource:
            savesource=BOOLVAL(tag->ti_Data);
            break;
         case AOSRC_Savename:
            savename=(UBYTE *)tag->ti_Data;
            break;
         case AOSRC_Saveappend:
            saveappend=BOOLVAL(tag->ti_Data);
            break;
         case AOSRC_Docext:
            SETFLAG(src->flags,SRCF_DOCEXT,tag->ti_Data);
            break;
         case AOSRC_Viewsource:
            if(tag->ti_Data) Viewsource(src);
            break;
         case AOSRC_Editsource:
            if(tag->ti_Data) Asetattrs(src->driver,AOSDV_Editsource,TRUE,TAG_END);
            break;
         case AOSRC_Displayed:
            if(tag->ti_Data)
            {  src->dispcount++;
               REMOVE(src);   /* Move last displayed to tail */
               ADDTAIL(&sources,src);
               if(src->driver && src->dispcount==1)
               {  Asetattrs(src->driver,AOSDV_Displayed,TRUE,TAG_END);
               }
            }
            else if(src->dispcount)
            {  src->dispcount--;
               if(src->driver && src->dispcount==0)
               {  Asetattrs(src->driver,AOSDV_Displayed,FALSE,TAG_END);
                  if(src->serverpush)
                  {  Asetattrs(src->serverpush,AOFCH_Cancel,TRUE,TAG_END);
                  }
               }
            }
            break;
         case AOSRC_Flush:
            if(tag->ti_Data)
            {  Flushsource(src);
            }
            break;
         case AOSRC_Memory:
            flushexcess=(tag->ti_Data>src->memory);
            totalmemory-=src->memory;
            src->memory=tag->ti_Data;
            totalmemory+=src->memory;
            if(flushexcess) Deferflushmem();
            break;
         case AOSRC_Defaulttype:
            if(tag->ti_Data)
            {  strncpy(src->defaulttype,(UBYTE *)tag->ti_Data,31);
            }
            break;
         case AOSRC_Lastmodified:
            src->lastmodified=tag->ti_Data;
            break;
         case AOSRC_Noicon:
            SETFLAG(src->flags,SRCF_NOICON,tag->ti_Data);
            break;
         case AOSRC_Noflush:
            SETFLAG(src->flags,SRCF_NOFLUSH,tag->ti_Data);
            break;
         case AOINF_Inquire:
            Makeinfo(src,(void *)tag->ti_Data);
            break;
      }
   }
   if(savesource && Agetattr(src->driver,AOSDV_Saveable))
   {  if(savename)
      {  Savesource(src,savename,saveappend);
      }
      else if(!src->savereq)
      {  name=Savepath(src->url);
         src->savereq=Anewobject(AOTP_FILEREQ,
            AOFRQ_Title,(Tag)AWEBSTR(MSG_FILE_SAVETITLE),
            AOFRQ_Filename,(Tag)name,
            AOFRQ_Savemode,TRUE,
            AOFRQ_Savecheck,TRUE,
            AOBJ_Target,(Tag)src,
            TAG_END);
         if(name) FREE(name);
      }
   }
   else if(savename && (src->flags&SRCF_SAVEAS))
   {  src->savename=Dupstr(savename,-1);
      if(saveappend) src->flags|=SRCF_SAVEAPPEND;
   }
   return 0;
}

static struct Source *Newsource(struct Amset *ams)
{  struct Source *src;
   if(src=Allocobject(AOTP_SOURCE,sizeof(struct Source),ams))
   {  ADDTAIL(&sources,src);
      NEWLIST(&src->copies);
      NEWLIST(&src->headers);
      Setsource(src,ams);
   }
   return src;
}

static long Getsource(struct Source *src,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSRC_Url:
            PUTATTR(tag,src->url);
            break;
         case AOSRC_Driver:
            PUTATTR(tag,Agetattr(src->driver,AOSDV_Volatile)?NULL:src->driver);
            break;
         case AOSRC_Displayed:
            PUTATTR(tag,BOOLVAL(src->dispcount));
            break;
         case AOSRC_Getsource:
            if(Agetattr(src->driver,AOSDV_Getable))
            {  PUTATTR(tag,Agetattr(src->driver,AOSDV_Getsource));
            }
            break;
         case AOSRC_Lastmodified:
            PUTATTR(tag,src->lastmodified);
            break;
         case AOSRC_Defaulttype:
            PUTATTR(tag,*src->defaulttype?src->defaulttype:NULL);
            break;
         case AOURL_Contenttype:
            PUTATTR(tag,*src->contenttype?src->contenttype:NULL);
            break;
         case AOBJ_Secure:
            PUTATTR(tag,BOOLVAL(src->cipher));
            break;
         case AOSRC_Foreign:
            PUTATTR(tag,BOOLVAL(src->flags&SRCF_FOREIGN));
            break;
         case AOSRC_Charset:
            PUTATTR(tag,src->charset);
            break;
         case AOSRC_Etag:
            PUTATTR(tag,src->etag);
            break;
         case AOSRC_Filename:
            PUTATTR(tag,src->filename);
            break;
      }
   }
   return 0;
}

static long Updatesource(struct Source *src,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   UBYTE *name=NULL;
   BOOL append=FALSE;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOFRQ_Filename:
            name=(UBYTE *)tag->ti_Data;
            src->savereq=NULL;
            break;
         case AOFRQ_Append:
            append=BOOLVAL(tag->ti_Data);
            break;
      }
   }
   if(name)
   {  Savesource(src,name,append);
      Asetattrs(Aweb(),AOAPP_Savepath,(Tag)name,TAG_DONE);
   }
   return 0;
}

static long Srcupdatesource(struct Source *src,struct Amsrcupdate *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   long length=0;
   UBYTE *data=NULL,*clientpull=NULL;
   BOOL eof=FALSE,error=FALSE;
   struct Header *hdr;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOURL_Contenttype:
            if(tag->ti_Data)
            {  strncpy(src->contenttype,(UBYTE *)tag->ti_Data,31);
            }
            break;
         case AOURL_Contentlength:
            src->length=tag->ti_Data;
            break;
         case AOURL_Data:
            data=(UBYTE *)tag->ti_Data;
            break;
         case AOURL_Datalength:
            length=tag->ti_Data;
            break;
         case AOURL_Eof:
            eof=BOOLVAL(tag->ti_Data);
            break;
         case AOURL_Error:
            if(tag->ti_Data)
            {  src->flags|=SRCF_ERROR;
               error=TRUE;
            }
            break;
         case AOURL_Lastmodified:
            src->lastmodified=tag->ti_Data;
            break;
         case AOURL_Terminate:
            src->serverpush=NULL;
            break;
         case AOURL_Serverpush:
            src->serverpush=(void *)tag->ti_Data;
            break;
         case AOURL_Clientpull:
            src->clientpull = tag->ti_Data?TRUE:FALSE;
            clientpull=(UBYTE *)tag->ti_Data;
            break;
         case AOURL_Fromcache:
            SETFLAG(src->flags,SRCF_CACHE,tag->ti_Data);
            break;
         case AOURL_Header:
            if(hdr=ALLOCSTRUCT(Header,1,0))
            {  if(hdr->text=Dupstr((UBYTE *)tag->ti_Data,-1))
               {  ADDTAIL(&src->headers,hdr);
               }
               else FREE(hdr);
            }
            break;
         case AOURL_Cipher:
            if(src->cipher) FREE(src->cipher);
            src->cipher=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOURL_Foreign:
            SETFLAG(src->flags,SRCF_FOREIGN,tag->ti_Data);
            break;
         case AOURL_Ssllibrary:
            if(src->ssllibrary) FREE(src->ssllibrary);
            src->ssllibrary=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOURL_Charset:
            if(tag->ti_Data)
            {
               strncpy(src->charset,(UBYTE *)tag->ti_Data,31);
#ifdef DEVELOPER
               if (charsetdebug) printf ("source.c/Srcupdatesource(): charset %s set for the document\n",src->charset);
#endif
            }
            break;

         case AOURL_Etag:
            if(tag->ti_Data)
            {
               strncpy(src->etag,(UBYTE *)tag->ti_Data,63);
            }
            break;
         case AOURL_Filename:
            if(src->filename)FREE(src->filename);
            if(tag->ti_Data)
            {
                src->filename = Dupstr((UBYTE *)tag->ti_Data,-1);
            }
            else
            {
                src->filename=NULL;
            }
      }
   }
   if(error) Setchildren(src,AOCPY_Error,TRUE,TAG_END);
   if(data && !src->driver)
   {
       Adddriver(src,data,length);
   }
   if(data || error || eof)
   {  /* If there is no driver yet for DOCEXT source, make one now or else
       * document parsing will never be signalled to continue. */
      if(!src->driver && eof && src->clientpull)
      {
       Adddriver(src,"",0);
      }
      if(!src->driver && (src->flags&SRCF_DOCEXT))
      {  Adddriver(src,"",0);
      }
      if(src->driver)
      {  if(src->flags&SRCF_FILTER)
         {
             Filterdata(src,ams->fetch,data,length,error||eof);
         }
         else
         {  AmethodA(src->driver,(struct Amessage *)ams);
         }
      }
   }
   if(clientpull)
   {
       Setchildren(src,AOURL_Clientpull,clientpull,TAG_END);
   }
   if(eof)
   {
      src->flags|=SRCF_EOF;
      Setchildren(src,AOURL_Eof,TRUE,TAG_END);
   }
   if(src->serverpush && !src->dispcount)
   {  Asetattrs(src->serverpush,AOFCH_Cancel,TRUE,TAG_END);
   }
   return 0;
}

static long Addchildsource(struct Source *src,struct Amadd *ama)
{  struct Copyref *cop=ALLOCSTRUCT(Copyref,1,MEMF_CLEAR);
   if(cop)
   {  cop->object=ama->child;
      ADDTAIL(&src->copies,cop);
      if(Agetattr(cop->object,AOCPY_Driver)) cop->flags|=COPF_DRIVER;
      if(src->flags&SRCF_EOF)
      {  Asetattrs(cop->object,AOURL_Eof,TRUE,TAG_END);
      }
      if(src->flags&SRCF_ERROR)
      {  Asetattrs(cop->object,AOCPY_Error,TRUE,TAG_END);
      }
      if((cop->flags&COPF_DRIVER) && src->driver)
      {  Aaddchild(src->driver,ama->child,AOREL_SRC_COPY);
      }
   }
   return 0;
}

static long Remchildsource(struct Source *src,struct Amadd *ama)
{  struct Copyref *cop;
   for(cop=src->copies.first;cop->next;cop=cop->next)
   {  if(cop->object==ama->child)
      {  if((cop->flags&COPF_DRIVER) && src->driver)
         {  Aremchild(src->driver,cop->object,0);
         }
         REMOVE(cop);
         FREE(cop);
         break;
      }
   }
   return 0;
}

static long Notifysource(struct Source *src,struct Amnotify *amn)
{  if(amn->nmsg->method==AOM_GETREXX)
   {  struct Amgetrexx *amg=(struct Amgetrexx *)amn->nmsg;
      if(amg->info=AMGRI_INFO)
      {  struct Header *hdr;
         if(src->flags&SRCF_CACHE)
         {  amg->index++;
            Setstemvar(amg->ac,amg->stem,amg->index,"TYPE","CACHE");
            Setstemvar(amg->ac,amg->stem,amg->index,"VALUE","1");
         }
         for(hdr=src->headers.first;hdr->next;hdr=hdr->next)
         {  amg->index++;
            Setstemvar(amg->ac,amg->stem,amg->index,"TYPE","HTTP");
            Setstemvar(amg->ac,amg->stem,amg->index,"VALUE",hdr->text);
         }
      }
   }
   else Broadcast(src,amn->nmsg);
   return 0;
}

static void Disposesource(struct Source *src)
{  struct Copyref *cop;
   struct Header *hdr;
   REMOVE(src);
   while(cop=(struct Copyref *)REMHEAD(&src->copies))
   {  Asetattrs(cop->object,AOCPY_Source,0,TAG_END);
      FREE(cop);
   }
   while(hdr=(struct Header *)REMHEAD(&src->headers))
   {  if(hdr->text) FREE(hdr->text);
      FREE(hdr);
   }
   if(src->driver) Adisposeobject(src->driver);
   if(src->savereq) Adisposeobject(src->savereq);
   totalmemory-=src->memory;
   if(src->pluginbase)
   {  Remaweblib(src->pluginbase);
      Closeaweblib(src->pluginbase);
   }
   if(src->savename) FREE(src->savename);
   if(src->filename) FREE(src->filename);
   if(src->cipher) FREE(src->cipher);
   if(src->ssllibrary) FREE(src->ssllibrary);
   Amethodas(AOTP_OBJECT,src,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Source_Dispatcher,
struct Source *,src,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newsource((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setsource(src,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getsource(src,(struct Amset *)amsg);
         break;
      case AOM_UPDATE:
         result=Updatesource(src,(struct Amset *)amsg);
         break;
      case AOM_SRCUPDATE:
         result=Srcupdatesource(src,(struct Amsrcupdate *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchildsource(src,(struct Amadd *)amsg);
         break;
      case AOM_REMCHILD:
         result=Remchildsource(src,(struct Amadd *)amsg);
         break;
      case AOM_NOTIFY:
         result=Notifysource(src,(struct Amnotify *)amsg);
         break;
      case AOM_DISPOSE:
         Disposesource(src);
         break;
      case AOM_DEINSTALL:
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installsource(void)
{  NEWLIST(&sources);
   if(!Amethod(NULL,AOM_INSTALL,AOTP_SOURCE,(Tag)Source_Dispatcher)) return FALSE;
   return TRUE;
}

void Flushsources(UWORD type)
{  switch(type)
   {  case SRCFT_NDIMAGES:
      case SRCFT_ALLIMAGES:
      case SRCFT_NDDOCUMENTS:
         Busypointer(TRUE);
         Flushsourcetype(type);
         Busypointer(FALSE);
         break;
      case SRCFT_EXCESS:
         Flushexcess();
         break;
   }
}

void Srcsetfiltertype(struct Sourcefilter *sf,UBYTE *type)
{  strncpy(sf->contenttype,type,sizeof(sf->contenttype)-1);
}

void Srcwritefilter(struct Sourcefilter *sf,UBYTE *data,long length)
{  if(length)
   {  Addtobuffer(&sf->buf,data,length);
   }
}
