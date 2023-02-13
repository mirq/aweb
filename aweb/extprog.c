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

/* extprog.c - AWeb spawn external program source driver object */

#include "aweb.h"
#include "source.h"
#include "sourcedriver.h"
#include "url.h"
#include "file.h"
#include "application.h"
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

struct Extprog
{  struct Aobject object;
   void *source;              /* The source for this driver */
   void *file;                /* File to save in */
   UBYTE *name;               /* Command to start */
   UBYTE *args;               /* Command arguments */
   UBYTE mimetype[32];        /* MIME type */
   UWORD flags;
};

#define EXPF_PIPE    0x0001   /* Should use a pipe */
#define EXPF_NOFETCH 0x0002   /* Should cancel fetch */
#define EXPF_SPAWNED 0x0004   /* External program spawned */


/*------------------------------------------------------------------------*/

static long Setextprog(struct Extprog *exp,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=Amethodas(AOTP_OBJECT,exp,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            exp->source=(void *)tag->ti_Data;
            break;
         case AOSDV_Name:
            exp->name=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOSDV_Arguments:
            exp->args=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOSDV_Pipe:
            SETFLAG(exp->flags,EXPF_PIPE,tag->ti_Data);
            break;
         case AOURL_Contenttype:
            strncpy(exp->mimetype,(UBYTE *)tag->ti_Data,31);
            break;
         case AOSDV_Nofetch:
            SETFLAG(exp->flags,EXPF_NOFETCH,tag->ti_Data);
            break;
      }
   }
   return result;
}

static struct Extprog *Newextprog(struct Amset *ams)
{  struct Extprog *exp;
   if(exp=Allocobject(AOTP_EXTPROG,sizeof(struct Extprog),ams))
   {  Setextprog(exp,ams);
   }
   return exp;
}

static long Getextprog(struct Extprog *exp,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=AmethodasA(AOTP_OBJECT,(struct Aobject *)exp,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            PUTATTR(tag,exp->source);
            break;
         case AOSDV_Volatile:
            PUTATTR(tag,TRUE);
            break;
      }
   }
   return result;
}

static long Srcupdateextprog(struct Extprog *exp,struct Amsrcupdate *ams)
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
            if(exp->file)
            {  Adisposeobject(exp->file);
               exp->file=NULL;
            }
            break;
         case AOURL_Eof:
            eof=TRUE;
            break;
      }
   }
   if((data || eof) && exp->flags & EXPF_NOFETCH)
   {
       /* spawn out external then cancel the fetch */
       if(exp->name && exp->args && !(exp->flags & EXPF_SPAWNED))
       {
           Spawn(FALSE,exp->name,exp->args,"num",
               Agetattr(Aweb(),AOAPP_Screenname),
               Agetattr((void *)Agetattr(exp->source,AOSRC_Url),AOURL_Url),
               exp->mimetype);
           exp->flags |= EXPF_SPAWNED;
       }
       Auspecial((void *)Agetattr((struct Aobject *)exp->source,AOSRC_Url),AUMST_CANCELFETCH);
   }
   else
   if((data || eof) && !exp->file)
   {  void *url=(void *)Agetattr(exp->source,AOSRC_Url);
      UBYTE *urlname=(UBYTE *)Agetattr(url,AOURL_Url);
      UBYTE *ext=urlname?Urlfileext(urlname):NULLSTRING;
      if((exp->flags&EXPF_PIPE) && exp->name && exp->args)
      {  if(exp->file=Anewobject(AOTP_FILE,
            AOFIL_Pipe,TRUE,
            AOFIL_Extension,(Tag)ext,
            TAG_END))
         {  Spawn(FALSE,exp->name,exp->args,"fnum",
               Agetattr(exp->file,AOFIL_Name),
               Agetattr(Aweb(),AOAPP_Screenname),
               Agetattr((void *)Agetattr(exp->source,AOSRC_Url),AOURL_Url),
               exp->mimetype);
         }
      }
      else
      {  exp->file=Anewobject(AOTP_FILE,
            AOFIL_Extension,(Tag)ext,
            TAG_END);
      }
      if(ext) FREE(ext);
   }
   if(data)
   {  if(exp->file)
      {  Asetattrs(exp->file,
            AOFIL_Data,(Tag)data,
            AOFIL_Datalength,length,
            TAG_END);
      }
   }
   if(eof)
   {  if(exp->file)
      {  Asetattrs(exp->file,AOFIL_Eof,TRUE,TAG_END);
         if(!(exp->flags&EXPF_PIPE)
         && exp->name && exp->args
         && Spawn(TRUE,exp->name,exp->args,"fnum",
               Agetattr(exp->file,AOFIL_Name),
               Agetattr(Aweb(),AOAPP_Screenname),
               Agetattr((void *)Agetattr(exp->source,AOSRC_Url),AOURL_Url),
               exp->mimetype))
         {  Asetattrs(exp->file,AOFIL_Delete,FALSE,TAG_DONE);
         }
         Adisposeobject(exp->file);
         exp->file=NULL;
      }
   }
   return 0;
}

static void Disposeextprog(struct Extprog *exp)
{  if(exp->name) FREE(exp->name);
   if(exp->args) FREE(exp->args);
   if(exp->file) Adisposeobject(exp->file);
   Amethodas(AOTP_OBJECT,exp,AOM_DISPOSE);
}

USRFUNC_H2
(
    static long, Extprog_Dispatcher,
    struct Extprog *, exp, A0,
    struct Amessage *, amsg, A1
)
{

    USRFUNC_INIT

   long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newextprog((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setextprog(exp,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getextprog(exp,(struct Amset *)amsg);
         break;
      case AOM_SRCUPDATE:
         result=Srcupdateextprog(exp,(struct Amsrcupdate *)amsg);
         break;
      case AOM_DISPOSE:
         Disposeextprog(exp);
         break;
      case AOM_DEINSTALL:
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installextprog(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_EXTPROG,(Tag)Extprog_Dispatcher)) return FALSE;
   return TRUE;
}
