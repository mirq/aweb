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

/* docext.c - AWeb HTML document extension (script, style) object */

#include "aweb.h"
#include "source.h"
#include "sourcedriver.h"
#include "copy.h"
#include "url.h"
#include "docprivate.h"
#include <proto/utility.h>

static LIST(Docext) docexts;

/*------------------------------------------------------------------------*/

/* Reference to a waiting document */
struct Docref
{  NODE(Docref);
   struct Document *doc;
   void *url;
};

static LIST(Docref) docrefs;

/* Signal all waiting documents */
static void Signaldocs(struct Docext *dox)
{  struct Docref *dr,*drnext;
   void *url,*durl;
   durl=(void *)Agetattr(dox->url,AOURL_Finalurlptr);
/*
printf("Signal for url %08x=%s\n"
       "             ->%08x=%s\n",
       dox->url,Agetattr(dox->url,AOURL_Url),
       durl,Agetattr(durl,AOURL_Url));
*/
   for(dr=docrefs.first;dr->next;dr=drnext)
   {  drnext=dr->next;
      url=(void *)Agetattr(dr->url,AOURL_Finalurlptr);
/*
printf("       ref url %08x=%s\n"
       "             ->%08x=%s\n",
       dr->url,Agetattr(dr->url,AOURL_Url),
       url,Agetattr(url,AOURL_Url));
*/
      if(url==durl)
      {  REMOVE(dr);
         Asetattrs((struct Aobject *)dr->doc,AODOC_Docextready,(Tag)dr->url,TAG_END);
         FREE(dr);
      }
   }
}

static void Addwaitingdoc(struct Document *doc,void *url)
{  struct Docref *dr;
   if(dr=ALLOCSTRUCT(Docref,1,0))
   {  dr->doc=doc;
      dr->url=url;
      ADDTAIL(&docrefs,dr);
   }
}

void Remwaitingdoc(struct Document *doc)
{  struct Docref *dr,*drnext;
   for(dr=docrefs.first;dr->next;dr=drnext)
   {  drnext=dr->next;
      if(dr->doc==doc)
      {  REMOVE(dr);
         FREE(dr);
      }
   }
}

/*------------------------------------------------------------------------*/

static long Setdocext(struct Docext *dox,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            dox->source=(void *)tag->ti_Data;
            break;
      }
   }
   return 0;
}

static struct Docext *Newdocext(struct Amset *ams)
{  struct Docext *dox;
   if(dox=Allocobject(AOTP_DOCEXT,sizeof(struct Docext),ams))
   {  Setdocext(dox,ams);
      dox->url=(void *)Agetattr(dox->source,AOSRC_Url);
      ADDTAIL(&docexts,dox);
   }
   return dox;
}

static long Getdocext(struct Docext *dox,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            PUTATTR(tag,dox->source);
            break;
      }
   }
   return 0;
}

static long Srcupdatedocext(struct Docext *dox,struct Amsrcupdate *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   long length=0;
   UBYTE *data=NULL;
   BOOL eof=FALSE;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOURL_Contentlength:
            Expandbuffer(&dox->buf,tag->ti_Data-dox->buf.length);
            break;
         case AOURL_Data:
            data=(UBYTE *)tag->ti_Data;
            break;
         case AOURL_Datalength:
            length=tag->ti_Data;
            break;
         case AOURL_Reload:
            Freebuffer(&dox->buf);
            dox->flags&=~DOXF_EOF;
            break;
         case AOURL_Eof:
            if(tag->ti_Data)
            {  dox->flags|=DOXF_EOF;
               eof=TRUE;
            }
            break;
         case AOURL_Error:
            if(tag->ti_Data)
            {  dox->flags|=DOXF_EOF|DOXF_ERROR;
               eof=TRUE;
            }
            break;
      }
   }
   if(data)
   {  Addtobuffer(&dox->buf,data,length);
      Asetattrs(dox->source,AOSRC_Memory,dox->buf.size,TAG_END);
   }
   if(eof)
   {  Addtobuffer(&dox->buf,"",1);
      Signaldocs(dox);
   }
   return 0;
}

/* A new child was added; send it an initial update msg */
static long Addchilddocext(struct Docext *dox,struct Amadd *ama)
{  Asetattrs(ama->child,AODOC_Srcupdate,TRUE,TAG_END);
   return 0;
}

static void Disposedocext(struct Docext *dox)
{  REMOVE(dox);
   Freebuffer(&dox->buf);
   Asetattrs(dox->source,AOSRC_Memory,0,TAG_END);
   Amethodas(AOTP_OBJECT,dox,AOM_DISPOSE);
}

static void Deinstalldocext(void)
{  struct Docref *p;
   while(p=(struct Docref *)REMHEAD(&docrefs)) FREE(p);
}

USRFUNC_H2
(
static long  , Docext_Dispatcher,
struct Docext *,dox,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newdocext((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setdocext(dox,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getdocext(dox,(struct Amset *)amsg);
         break;
      case AOM_SRCUPDATE:
         result=Srcupdatedocext(dox,(struct Amsrcupdate *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchilddocext(dox,(struct Amadd *)amsg);
         break;
      case AOM_DISPOSE:
         Disposedocext(dox);
         break;
      case AOM_DEINSTALL:
         Deinstalldocext();
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installdocext(void)
{  NEWLIST(&docexts);
   NEWLIST(&docrefs);
   if(!Amethod(NULL,AOM_INSTALL,AOTP_DOCEXT,(Tag)Docext_Dispatcher)) return FALSE;
   return TRUE;
}

/* Return the source for this document extension. If NULL return, a
 * load for that file was started and the document was added to the
 * wait list.
 * If (UBYTE *)~0 return, the extension is in error. */
UBYTE *Finddocext(struct Document *doc,void *url,BOOL reload)
{  struct Docext *dox;
   ULONG loadflags=AUMLF_DOCEXT;
   void *durl;
   void *furl=(void *)Agetattr(url,AOURL_Finalurlptr);
/*
printf("Search for url %08x=%s\n"
       "             ->%08x=%s\n",
       url,Agetattr(url,AOURL_Url),
       furl,Agetattr(furl,AOURL_Url));
*/
   if(reload)
   {  loadflags|=AUMLF_RELOAD;
   }
   else
   {  for(dox=docexts.first;dox->object.next;dox=dox->object.next)
      {  durl=(void *)Agetattr(dox->url,AOURL_Finalurlptr);
/*
printf("       dox url %08x=%s\n"
       "             ->%08x=%s\n",
       dox->url,Agetattr(dox->url,AOURL_Url),
       durl,Agetattr(durl,AOURL_Url));
*/
         if(durl==furl)
         {  if(dox->flags&DOXF_ERROR) return (UBYTE *)~0;
            else return dox->buf.buffer;
         }
      }
   }
   Addwaitingdoc(doc,url);
   Auload(url,loadflags,NULL,NULL,NULL);
   return NULL;
}
