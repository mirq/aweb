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

/* soundsource.c - AWeb sound source interpreter object */

#include "aweb.h"
#include "source.h"
#include "sourcedriver.h"
#include "url.h"
#include "file.h"
#include "soundprivate.h"
#include <proto/utility.h>

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/* Create a new file object */
static void *Newfile(struct Sndsource *sns)
{  void *url=(void *)Agetattr(sns->source,AOSRC_Url);
   UBYTE *urlname,*ext=NULL;
   void *file;
   if(urlname=(UBYTE *)Agetattr(url,AOURL_Url))
   {  ext=Urlfileext(urlname);
   }
   file=Anewobject(AOTP_FILE,
      AOFIL_Extension,(Tag)ext,
      TAG_END);
   if(ext) FREE(ext);
   return file;
}

/*------------------------------------------------------------------------*/

static long Setsoundsource(struct Sndsource *sns,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=Amethodas(AOTP_OBJECT,sns,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            sns->source=(void *)tag->ti_Data;
            break;
         case AOSDV_Savesource:
            if(sns->filename)
            {  Asetattrs((void *)tag->ti_Data,
                  AOFIL_Copyfile,(Tag)sns->filename,
                  TAG_END);
            }
            break;
      }
   }
   return result;
}

static struct Sndsource *Newsoundsource(struct Amset *ams)
{  struct Sndsource *sns;
   if(sns=Allocobject(AOTP_SOUNDSOURCE,sizeof(struct Sndsource),ams))
   {  Setsoundsource(sns,ams);
   }
   return sns;
}

static long Getsoundsource(struct Sndsource *sns,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=AmethodasA(AOTP_OBJECT,(struct Aobject *)sns,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            PUTATTR(tag,sns->source);
            break;
         case AOSDV_Saveable:
            PUTATTR(tag,BOOLVAL(sns->filename));
            break;
      }
   }
   return result;
}

static long Srcupdatesoundsource(struct Sndsource *sns,struct Amsrcupdate *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   long length=0;
   UBYTE *data=NULL;
   BOOL eof=FALSE;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOURL_Data:
            data=(UBYTE *)tag->ti_Data;
            break;
         case AOURL_Datalength:
            length=tag->ti_Data;
            break;
         case AOURL_Reload:
            sns->flags&=~SNSF_EOF;
            if(sns->file)
            {  Adisposeobject(sns->file);
               sns->file=NULL;
               sns->filename=NULL;
            }
            break;
         case AOURL_Eof:
            if(tag->ti_Data) eof=TRUE;
            break;
      }
   }
   if(data)
   {  if(!sns->file) sns->file=Newfile(sns);
      if(sns->file)
      {  Asetattrs(sns->file,
            AOFIL_Data,(Tag)data,
            AOFIL_Datalength,length,
            TAG_END);
      }
   }
   if(eof && sns->file)
   {  Asetattrs(sns->file,AOFIL_Eof,TRUE,TAG_END);
      sns->filename=(UBYTE *)Agetattr(sns->file,AOFIL_Name);
      sns->flags|=SNSF_EOF;
      Anotifyset(sns->source,AOSNP_Srcupdate,TRUE,TAG_END);
   }
   return 0;
}

/* A new child was added; if we have data then send it a notify */
static long Addchildsoundsource(struct Sndsource *sns,struct Amadd *ama)
{  if(sns->filename) Asetattrs(ama->child,AOSNP_Srcupdate,TRUE,TAG_END);
   return 0;
}

static void Disposesoundsource(struct Sndsource *sns)
{  if(sns->file) Adisposeobject(sns->file);
   Amethodas(AOTP_OBJECT,sns,AOM_DISPOSE);
}

static void Deinstallsoundsource(void)
{
}

USRFUNC_H2
(
static long  , Soundsource_Dispatcher,
struct Sndsource *,sns,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newsoundsource((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setsoundsource(sns,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getsoundsource(sns,(struct Amset *)amsg);
         break;
      case AOM_SRCUPDATE:
         result=Srcupdatesoundsource(sns,(struct Amsrcupdate *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchildsoundsource(sns,(struct Amadd *)amsg);
         break;
      case AOM_DISPOSE:
         Disposesoundsource(sns);
         break;
      case AOM_DEINSTALL:
         Deinstallsoundsource();
         break;
      default:
         result=AmethodasA(AOTP_OBJECT,(struct Aobject *)sns,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installsoundsource(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_SOUNDSOURCE,(Tag)Soundsource_Dispatcher)) return FALSE;
   return TRUE;
}
