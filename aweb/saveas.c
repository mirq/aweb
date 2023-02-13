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

/* saveas.c - AWeb save as local file source driver object */

#include "aweb.h"
#include "source.h"
#include "sourcedriver.h"
#include "url.h"
#include "file.h"
#include "filereq.h"
#include "fetch.h"
#include "application.h"
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

struct Saveas
{  struct Aobject object;
   void *source;              /* The source for this driver */
   void *fetch;               /* The fetch object to kill on cancel */
   void *req;                 /* Save requester */
   void *file;                /* File to save in */
   void *temp;                /* Temp file to buffer */
   UWORD flags;
   UBYTE *unktype;            /* Unknown MIME type or NULL */
};

#define SASF_EOF        0x0001   /* Eof was reached before filename known. */
#define SASF_CANCEL     0x0002   /* Load was cancelled */
#define SASF_TEXT       0x0004   /* Text icon instead of data */
#define SASF_ERROR      0x0008   /* Errored, don't save */
#define SASF_NOICON     0x0010   /* Don't create icons */

/*------------------------------------------------------------------------*/

/* Reset so new data will be saved again. */
static void Resetsaveas(struct Saveas *sas)
{  if(sas->file)
   {  Adisposeobject(sas->file);
      sas->file=NULL;
   }
   if(sas->temp)
   {  Adisposeobject(sas->temp);
      sas->temp=NULL;
   }
   if(sas->req)
   {  Adisposeobject(sas->req);
      sas->req=NULL;
   }
   sas->flags&=~(SASF_EOF|SASF_CANCEL);
}

static UWORD Icontype(struct Saveas *sas)
{  UWORD icon=FILEICON_NONE;
   if(!(sas->flags&SASF_NOICON))
   {  if(sas->flags&SASF_TEXT) icon=FILEICON_TEXT;
      else icon=FILEICON_DATA;
   }
   return icon;
}

/*------------------------------------------------------------------------*/

static long Setsaveas(struct Saveas *sas,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=Amethodas(AOTP_OBJECT,sas,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            sas->source=(void *)tag->ti_Data;
            break;
         case AOURL_Contenttype:
            if(tag->ti_Data)
            {  if(STRNIEQUAL((UBYTE *)tag->ti_Data,"text/",5)) sas->flags|=SASF_TEXT;
               else sas->flags&=~SASF_TEXT;
            }
            break;
         case AOSDV_Savesource:
            if(!sas->file)
            {  sas->file=(void *)tag->ti_Data;
            }
            else
            {  Adisposeobject((void *)tag->ti_Data);
            }
            break;
         case AOSDV_Noicon:
            SETFLAG(sas->flags,SASF_NOICON,tag->ti_Data);
            break;
         case AOSDV_Unknowntype:
            if(sas->unktype) FREE(sas->unktype);
            sas->unktype=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
      }
   }
   return result;
}

static struct Saveas *Newsaveas(struct Amset *ams)
{  struct Saveas *sas;
   if(sas=Allocobject(AOTP_SAVEAS,sizeof(struct Saveas),ams))
   {  Setsaveas(sas,ams);
      Asetattrs((void *)Agetattr(sas->source,AOSRC_Url),AOURL_Cachefetch,FALSE,TAG_END);
   }
   return sas;
}

static long Getsaveas(struct Saveas *sas,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=AmethodasA(AOTP_OBJECT,(struct Aobject *)sas,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            PUTATTR(tag,sas->source);
            break;
         case AOSDV_Volatile:
            PUTATTR(tag,TRUE);
            break;
      }
   }
   return result;
}

static UBYTE* Savepathsrc(void *src)
{
    UBYTE *path;
    UBYTE *fname;
    UBYTE *name = NULL;
    long len;
    if((fname = (UBYTE *)Agetattr(src,AOSRC_Filename)))
    {
        if((path = (UBYTE *)Agetattr(Aweb(), AOAPP_Savepath)))
        {
            /* only use the file part of a suggested filename */
            fname = FilePart(fname);
            len = strlen(path) + strlen(fname);
            if((name = ALLOCTYPE(UBYTE,len + 4,0)))
            {
                strcpy(name,path);
                AddPart(name,fname,len+3);
            }
        }
    }
    return name;
}

static long Srcupdatesaveas(struct Saveas *sas,struct Amsrcupdate *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   long length=0;
   UBYTE *data=NULL,*name;
   UBYTE *unkurl=0,*unkext=0;
   void *url;
   BOOL eof=FALSE,error=FALSE;
   if(!sas->fetch && !(sas->flags&SASF_EOF)) sas->fetch=ams->fetch;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOURL_Contenttype:
            if(tag->ti_Data)
            {  if(STRNIEQUAL((UBYTE *)tag->ti_Data,"text/",5)) sas->flags|=SASF_TEXT;
               else sas->flags&=~SASF_TEXT;
            }
            break;
         case AOURL_Data:
            data=(UBYTE *)tag->ti_Data;
            break;
         case AOURL_Datalength:
            length=tag->ti_Data;
            break;
         case AOURL_Reload:
            Resetsaveas(sas);
            break;
         case AOURL_Eof:
            eof=TRUE;
            sas->fetch=NULL;  /* Don't attempt to kill nonexisting fetch */
            break;
         case AOURL_Error:
            if(tag->ti_Data)
            {  sas->flags|=SASF_ERROR;
               if(sas->file)
               {  Adisposeobject(sas->file);
                  sas->file=NULL;
               }
               if(sas->temp)
               {  Adisposeobject(sas->temp);
                  sas->temp=NULL;
               }
            }
            break;
      }
   }
   /* At the very first data or eof, ask for the file name and create a buffer file. */
   if(!(sas->flags&(SASF_CANCEL|SASF_ERROR)))
   {  if((data || eof) && !sas->file && !sas->temp)
      {  if(!sas->req)
         {
            if(!(name=Savepathsrc(sas->source)))
            {
                name=Savepath((void *)Agetattr(sas->source,AOSRC_Url));
            }
            if(sas->unktype)
            {  url=(void *)Agetattr(sas->source,AOSRC_Url);
               unkurl=(UBYTE *)Agetattr(url,AOURL_Url);
               unkext=Urlfileext(unkurl);
            }
            sas->req=Anewobject(AOTP_FILEREQ,
               AOFRQ_Title,(Tag)AWEBSTR(MSG_FILE_SAVETITLE),
               AOFRQ_Filename,(Tag)name,
               AOFRQ_Savemode,TRUE,
               AOFRQ_Savecheck,TRUE,
               sas->unktype?AOFRQ_Unkurl:TAG_IGNORE,(Tag)unkurl,
               sas->unktype?AOFRQ_Unktype:TAG_IGNORE,(Tag)sas->unktype,
               sas->unktype?AOFRQ_Unkext:TAG_IGNORE,unkext?(Tag)unkext:(Tag)"(none)",
               AOBJ_Target,(Tag)sas,
               TAG_END);
            if(name) FREE(name);
            if(sas->unktype && unkext) FREE(unkext);
         }
         sas->temp=Anewobject(AOTP_FILE,TAG_END);
      }
      if(data)
      {  if(sas->file)
         {  Asetattrs(sas->file,
               AOFIL_Data,(Tag)data,
               AOFIL_Datalength,length,
               TAG_END);
            error=Agetattr(sas->file,AOFIL_Error);
         }
         else if(sas->temp)
         {  Asetattrs(sas->temp,
               AOFIL_Data,(Tag)data,
               AOFIL_Datalength,length,
               TAG_END);
            error=Agetattr(sas->temp,AOFIL_Error);
         }
      }
      if(eof)
      {  if(sas->file)
         {  Asetattrs(sas->file,
               AOFIL_Icontype,Icontype(sas),
               AOFIL_Eof,TRUE,
               AOFIL_Delete,FALSE,
               AOFIL_Comment,Agetattr((void *)Agetattr(sas->source,AOSRC_Url),AOURL_Url),
               TAG_END);
         }
         else
         {  sas->flags|=SASF_EOF;
         }
      }
      if(error)
      {  if(sas->fetch) Asetattrs(sas->fetch,AOFCH_Cancel,TRUE,TAG_END);
         Resetsaveas(sas);
         sas->flags|=SASF_CANCEL;
      }
   }
   return 0;
}

static long Updatesaveas(struct Saveas *sas,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   UBYTE *name=NULL;
   BOOL append=FALSE,cancel=FALSE;;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOFRQ_Filename:
            name=(UBYTE *)tag->ti_Data;
            if(!name) cancel=TRUE;
            sas->req=NULL; /* FILEREQ objects dispose themselves. */
            break;
         case AOFRQ_Append:
            append=BOOLVAL(tag->ti_Data);
            break;
      }
   }
   if(sas->flags&SASF_ERROR)
   {  name=NULL;
   }
   if(name)
   {  /* Create the file, close the temp file and copy the buffered data. */
      sas->file=Anewobject(AOTP_FILE,
         AOFIL_Name,(Tag)name,
         AOFIL_Append,append,
         TAG_END);
      if(sas->file)
      {  if(sas->temp)
         {  Asetattrs(sas->temp,AOFIL_Eof,TRUE,TAG_END);
            Asetattrs(sas->file,
               AOFIL_Copyfile,Agetattr(sas->temp,AOFIL_Name),
               TAG_END);
            Adisposeobject(sas->temp);
            sas->temp=NULL;
         }
         if(sas->flags&SASF_EOF)
         {  Asetattrs(sas->file,
               AOFIL_Icontype,Icontype(sas),
               AOFIL_Eof,TRUE,
               AOFIL_Delete,FALSE,
               AOFIL_Comment,Agetattr((void *)Agetattr(sas->source,AOSRC_Url),AOURL_Url),
               TAG_END);
         }
         Asetattrs(Aweb(),AOAPP_Savepath,(Tag)name,TAG_END);
      }
      else
      {  /* Couldn't open file, try again */
         if(!(name=Savepathsrc(sas->source)))
         {
             name=Savepath((void *)Agetattr(sas->source,AOSRC_Url));
         }
         sas->req=Anewobject(AOTP_FILEREQ,
            AOFRQ_Title,(Tag)AWEBSTR(MSG_FILE_SAVETITLE),
            AOFRQ_Filename,(Tag)name,
            AOFRQ_Savemode,TRUE,
            AOFRQ_Savecheck,TRUE,
            AOBJ_Target,(Tag)sas,
            TAG_END);
         if(name) FREE(name);
      }
   }
   if(cancel)
   {  if(sas->fetch) Asetattrs(sas->fetch,AOFCH_Cancel,TRUE,TAG_END);
      Resetsaveas(sas);
      sas->flags|=SASF_CANCEL;
   }
   return 0;
}

static void Disposesaveas(struct Saveas *sas)
{  /* Requester must be disposed first. When it updates the others are disposed of. */
   if(sas->req) Adisposeobject(sas->req);
   if(sas->temp) Adisposeobject(sas->temp);
   if(sas->file) Adisposeobject(sas->file);
   if(sas->unktype) FREE(sas->unktype);
   Amethodas(AOTP_OBJECT,sas,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Saveas_Dispatcher,
struct Saveas *,sas,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newsaveas((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setsaveas(sas,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getsaveas(sas,(struct Amset *)amsg);
         break;
      case AOM_UPDATE:
         result=Updatesaveas(sas,(struct Amset *)amsg);
         break;
      case AOM_SRCUPDATE:
         result=Srcupdatesaveas(sas,(struct Amsrcupdate *)amsg);
         break;
      case AOM_DISPOSE:
         Disposesaveas(sas);
         break;
      case AOM_DEINSTALL:
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installsaveas(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_SAVEAS,(Tag)Saveas_Dispatcher)) return FALSE;
   return TRUE;
}
