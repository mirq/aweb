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

/* winhis.c - AWeb window history object */

#include "aweb.h"
#include "winhis.h"
#include "url.h"
#include "whisprivate.h"
#include <proto/utility.h>

WinhisList_t winhis;
struct SignalSemaphore whissema;

static ULONG whisnr=0;

/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/

/* Dispose a Framehis */
static void Disposeframehis(struct Framehis *fhis)
{  if(fhis)
   {  if(fhis->id) FREE(fhis->id);
      if(fhis->fragment) FREE(fhis->fragment);
      FREE(fhis);
   }
}

/* Add an empty framehis */
static struct Framehis *Addframehis(struct Winhis *whis,UBYTE *id)
{  struct Framehis *fhis;
   if(fhis=ALLOCSTRUCT(Framehis,1,MEMF_CLEAR))
   {  fhis->id=Dupstr(id?id:NULLSTRING,-1);
      ADDTAIL(&whis->frames,fhis);
   }
   return fhis;
}

/* Find a framehis for this id or create an empty one */
static struct Framehis *Findframehis(struct Winhis *whis,UBYTE *id)
{  struct Framehis *fhis;
   for(fhis=whis->frames.first;fhis->next;fhis=fhis->next)
   {  if(STREQUAL(fhis->id,id)) return fhis;
   }
   return Addframehis(whis,id);
}

/*------------------------------------------------------------------------*/

/* Copy a winhis, leaving out this frame id and children. Copy everything if
 * id==~0 or the URL for this frame is unchanged. */
static void Copywinhis(struct Winhis *to,struct Winhis *from,UBYTE *id,void *url)
{  struct Framehis *fhis,*fnew;
   long idlen=0;
   BOOL sameurl=FALSE;
   if(id!=(UBYTE *)~0) idlen=strlen(id);
   to->key=from->key;
   to->windownr=from->windownr;
   for(fhis=from->frames.first;fhis->next;fhis=fhis->next)
   {  if(id!=(UBYTE *)~0 && STREQUAL(fhis->id,id) && fhis->url==url) sameurl=TRUE;
      if(id==(UBYTE *)~0 || !STRNEQUAL(fhis->id,id,idlen) || sameurl)
      {  if(fnew=Addframehis(to,fhis->id))
         {  fnew->url=fhis->url;
            if(fhis->fragment) fnew->fragment=Dupstr(fhis->fragment,-1);
            fnew->left=fhis->left;
            fnew->top=fhis->top;
         }
      }
   }
}

/* Clear all frames below this one */
static void Clearbelow(struct Winhis *whis,struct Framehis *fhis)
{  struct Framehis *fh,*nextfh;
   long idlen=strlen(fhis->id);
   for(fh=whis->frames.first;fh->next;fh=nextfh)
   {  nextfh=fh->next;
      if(fh!=fhis && STRNEQUAL(fh->id,fhis->id,idlen))
      {  REMOVE(fh);
         Disposeframehis(fh);
      }
   }
}

/* Get next winhis for the same window, through the mainline */
static struct Winhis *Nextwinhis(struct Winhis *whis)
{  ULONG key;
   UWORD skipf=WINHF_SKIP;
   if(whis)
   {  key=whis->key;
      if(whis->wflags&WINHF_SKIP) skipf=0;
      for(whis=whis->object.next;whis->object.next;whis=whis->object.next)
      {  if(whis->key==key && !(whis->wflags&skipf)) return whis;
      }
   }
   return NULL;
}

/* Get previous winhis for the same window, through the mainline */
static struct Winhis *Prevwinhis(struct Winhis *whis)
{  ULONG key;
   UWORD skipf=WINHF_SKIP;
   if(whis)
   {  key=whis->key;
      if(whis->wflags&WINHF_SKIP) skipf=0;
      for(whis=whis->object.prev;whis->object.prev;whis=whis->object.prev)
      {  if(whis->key==key && !(whis->wflags&skipf)) return whis;
      }
   }
   return NULL;
}

/* User skipped from wha to whb. Set skip flag for whisses in between */
static void Skipwinhis(struct Winhis *whb,struct Winhis *wha)
{  ULONG key;
   BOOL ok=FALSE;
   if(whb)
   {  if(wha)
      {  /* If wha is still NEW, the skip is ok but wha's NEW flag
          * should be cleared. It's a "dead" winhis (e.g. a download)
          * followed by a "live" one. */
         if(wha->wflags&WINHF_NEW)
         {  wha->wflags&=~WINHF_NEW;
            ok=TRUE;
         }
         else if(!(wha->wflags&WINHF_SKIP))
         {  ok=TRUE;
         }
         if(wha->whisnr<whb->whisnr
         && ok
         && !(whb->wflags&WINHF_HISTORY)
         && (whb->wflags&WINHF_NEW))
         {  whb->wflags&=~(WINHF_SKIP|WINHF_NEW);
            key=wha->key;
            for(wha=wha->object.next;wha->object.next && wha!=whb;wha=wha->object.next)
            {  if(wha->key==key)
               {  wha->wflags|=WINHF_SKIP;
               }
            }
            Changewhiswindow(wha->windownr);
         }
      }
      else
      {  /* First ever winhis */
         whb->wflags&=~(WINHF_SKIP|WINHF_NEW);
         Changewhiswindow(whb->windownr);
      }
   }
}

static struct Winhis *Nextinframe(struct Winhis *whis,UBYTE *frameid)
{  UBYTE *id;
   while(whis=Nextwinhis(whis))
   {  id=whis->frameid?whis->frameid:NULLSTRING;
      if(STREQUAL(id,frameid))
      {  /* Same frame */
         break;
      }
      if(STRNEQUAL(id,frameid,strlen(id)))
      {  /* Parent frame changed */
         whis=NULL;
         break;
      }
   }
   return whis;
}

static struct Winhis *Previnframe(struct Winhis *whis,UBYTE *frameid)
{  UBYTE *id;
   id=whis->frameid?whis->frameid:NULLSTRING;
   if(STREQUAL(id,frameid))
   {  /* Not yet at the beginning of history for this frame */
      while(whis=Prevwinhis(whis))
      {  id=whis->frameid?whis->frameid:NULLSTRING;
         if(STRNEQUAL(id,frameid,strlen(id)))
         {  /* Same frame or a parent */
            break;
         }
      }
   }
   else
   {  /* Started at the beginning of history for this frame */
      whis=NULL;
   }
   return whis;
}

static struct Winhis *Leadingwinhis(struct Winhis *whis,UBYTE *frameid)
{  UBYTE *id;
   while(whis)
   {  id=whis->frameid?whis->frameid:NULLSTRING;
      if(STRNEQUAL(id,frameid,strlen(id))) break;
      whis=Prevwinhis(whis);
   }
   return whis;
}

/* See if any url for a frame in this winhis is inputting.
 * Then look for a new winhis for the same window and do the same. */
static BOOL Inputwinhis(struct Winhis *whis)
{  struct Framehis *fhis;
   ULONG key=whis->key;
   for(fhis=whis->frames.first;fhis->next;fhis=fhis->next)
   {  if(Agetattr(fhis->url,AOURL_Input)) return TRUE;
   }
   for(whis=whis->object.next;whis->object.next;whis=whis->object.next)
   {  if(whis->key==key && (whis->wflags&WINHF_NEW))
      {  for(fhis=whis->frames.first;fhis->next;fhis=fhis->next)
         {  if(Agetattr(fhis->url,AOURL_Input)) return TRUE;
         }
      }
   }
   return FALSE;
}

/* Cancel all input of urls for frames in this winhis.
 * Then look for a new winhis for the same window and do the same. */
static void *Cancelwinhis(struct Winhis *whis)
{  struct Framehis *fhis;
   ULONG key=whis->key;
   for(fhis=whis->frames.first;fhis->next;fhis=fhis->next)
   {  Auspecial(fhis->url,AUMST_CANCELFETCH);
   }
   for(whis=whis->object.next;whis->object.next;whis=whis->object.next)
   {  if(whis->key==key && (whis->wflags&WINHF_NEW))
      {  for(fhis=whis->frames.first;fhis->next;fhis=fhis->next)
         {  Auspecial(fhis->url,AUMST_CANCELFETCH);
         }
      }
   }
   return FALSE;
}

/*------------------------------------------------------------------------*/

static long Setwinhis(struct Winhis *whis,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   struct Framehis *fhis;
   BOOL chgwhw=FALSE;
   fhis=Findframehis(whis,NULLSTRING);
   ObtainSemaphore(&whissema);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOWHS_Key:
            whis->key=tag->ti_Data;
            break;
         case AOWHS_Windownr:
            whis->windownr=tag->ti_Data;
            break;
         case AOWHS_Frameid:
            if(tag->ti_Data!=~0)
            {  fhis=Findframehis(whis,tag->ti_Data?(UBYTE *)tag->ti_Data:NULLSTRING);
            }
            break;
         case AOWHS_Url:
            if(fhis)
            {  fhis->url=(void *)tag->ti_Data;
               if(STREQUAL(fhis->id,whis->frameid))
               {  whis->titleurl=fhis->url;
                  if(ams->amsg.method!=AOM_NEW) chgwhw=TRUE;
               }
            }
            break;
         case AOWHS_Fragment:
            if(fhis)
            {  if(fhis->fragment) FREE(fhis->fragment);
               if(tag->ti_Data) fhis->fragment=Dupstr((UBYTE *)tag->ti_Data,-1);
               else fhis->fragment=NULL;
            }
            break;
         case AOWHS_Leftpos:
            if(fhis)
            {  fhis->left=tag->ti_Data;
            }
            break;
         case AOWHS_Toppos:
            if(fhis)
            {  fhis->top=tag->ti_Data;
            }
            break;
         case AOWHS_Skipfrom:
            Skipwinhis(whis,(struct Winhis *)tag->ti_Data);
            break;
         case AOWHS_Title:
            if(fhis && STREQUAL(fhis->id,whis->frameid))
            {  if(whis->title) FREE(whis->title);
               whis->title=Dupstr((UBYTE *)tag->ti_Data,-1);
               Changewhiswindow(whis->windownr);
            }
            break;
         case AOWHS_Loadnr:
            if(fhis && STREQUAL(fhis->id,whis->frameid))
            {  whis->loadnr=tag->ti_Data;
            }
            break;
         case AOWHS_History:
            if(tag->ti_Data) whis->wflags|=WINHF_HISTORY;
            else whis->wflags&=~WINHF_HISTORY;
            break;
         case AOWHS_Clearbelow:
            if(tag->ti_Data)
            {  Clearbelow(whis,fhis);
            }
            break;
         case AOWHS_Commands:
            SETFLAG(whis->wflags,WINHF_COMMANDS,tag->ti_Data);
            break;
         case AOWHS_Cancel:
            if(tag->ti_Data)
            {  Cancelwinhis(whis);
            }
            break;
      }
   }
   ReleaseSemaphore(&whissema);
   if(chgwhw) Changewhiswindow(whis->windownr);
   return 0;
}

static struct Winhis *Newwinhis(struct Amset *ams)
{  struct Winhis *whis,*copy;
   struct Framehis *fhis;
   if(whis=Allocobject(AOTP_WINHIS,sizeof(struct Winhis),ams))
   {  NEWLIST(&whis->frames);
      whis->whisnr=++whisnr;
      whis->wflags=WINHF_SKIP|WINHF_NEW;
      whis->frameid=(UBYTE *)GetTagData(AOWHS_Frameid,~0,ams->tags);
      if(copy=(struct Winhis *)GetTagData(AOWHS_Copyfrom,0,ams->tags))
      {  Copywinhis(whis,copy,whis->frameid?whis->frameid:NULLSTRING,
            (void *)GetTagData(AOWHS_Url,0,ams->tags));
      }
      if(!whis->frameid || whis->frameid==(UBYTE *)~0) whis->frameid="";
      Setwinhis(whis,ams);
      if(fhis=Findframehis(whis,whis->frameid?whis->frameid:NULLSTRING))
      {  whis->frameid=fhis->id;
      }
      ObtainSemaphore(&whissema);
      ADDTAIL(&winhis,whis);
      ReleaseSemaphore(&whissema);
      Changewhiswindow(whis->windownr);
   }
   return whis;
}

static long Getwinhis(struct Winhis *whis,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   struct Framehis *fhis;
   fhis=Findframehis(whis,NULLSTRING);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOWHS_Key:
            PUTATTR(tag,whis->key);
            break;
         case AOWHS_Windownr:
            PUTATTR(tag,whis->windownr);
            break;
         case AOWHS_Frameid:
            fhis=Findframehis(whis,tag->ti_Data?(UBYTE *)tag->ti_Data:NULLSTRING);
            break;
         case AOWHS_Url:
            PUTATTR(tag,fhis?fhis->url:NULL);
            break;
         case AOWHS_Fragment:
            PUTATTR(tag,fhis?fhis->fragment:NULL);
            break;
         case AOWHS_Leftpos:
            PUTATTR(tag,fhis?fhis->left:0);
            break;
         case AOWHS_Toppos:
            PUTATTR(tag,fhis?fhis->top:0);
            break;
         case AOWHS_Previous:
            PUTATTR(tag,Prevwinhis(whis));
            break;
         case AOWHS_Next:
            PUTATTR(tag,Nextwinhis(whis));
            break;
         case AOWHS_History:
            PUTATTR(tag,BOOLVAL(whis->wflags&WINHF_HISTORY));
            break;
         case AOWHS_Isleading:
            PUTATTR(tag,STREQUAL(fhis->id,whis->frameid));
            break;
         case AOWHS_Commands:
            PUTATTR(tag,BOOLVAL(whis->wflags&WINHF_COMMANDS));
            break;
         case AOWHS_Previnframe:
            PUTATTR(tag,Previnframe(whis,fhis->id));
            break;
         case AOWHS_Nextinframe:
            PUTATTR(tag,Nextinframe(whis,fhis->id));
            break;
         case AOWHS_Leadingwinhis:
            PUTATTR(tag,Leadingwinhis(whis,fhis->id));
            break;
         case AOWHS_Input:
            PUTATTR(tag,Inputwinhis(whis));
            break;
      }
   }
   return 0;
}

static void Disposewinhis(struct Winhis *whis)
{  void *p;
   REMOVE(whis);
   while(p=REMHEAD(&whis->frames)) Disposeframehis(p);
   if(whis->title) FREE(whis->title);
   Amethodas(AOTP_OBJECT,whis,AOM_DISPOSE);
}

static void Deinstallwinhis(void)
{  ObtainSemaphore(&whissema);
   while(winhis.first->object.next) Adisposeobject((struct Aobject *)winhis.first);
   ReleaseSemaphore(&whissema);
}

USRFUNC_H2
(
    static long, Winhis_Dispatcher,
    struct Winhis *, whis, A0,
    struct Amessage *, amsg, A1
)
{

   USRFUNC_INIT

   long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newwinhis((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setwinhis(whis,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getwinhis(whis,(struct Amset *)amsg);
         break;
      case AOM_DISPOSE:
         Disposewinhis(whis);
         break;
      case AOM_DEINSTALL:
         Deinstallwinhis();
         break;
   }
   return result;

   USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installwinhis(void)
{  NEWLIST(&winhis);
   InitSemaphore(&whissema);
   if(!Amethod(NULL,AOM_INSTALL,AOTP_WINHIS,(Tag)Winhis_Dispatcher)) return FALSE;
   return TRUE;
}

void Historyarexx(struct Arexxcmd *ac,long *windownr,BOOL mainline,UBYTE *stem)
{  struct Winhis *whis;
   long index=0;
   UBYTE buf[16];
   ObtainSemaphore(&whissema);
   for(whis=winhis.first;whis->object.next;whis=whis->object.next)
   {  if(!windownr || whis->windownr==*windownr)
      {  if(!(mainline && (whis->wflags&WINHF_SKIP)))
         {  index++;
            sprintf(buf,"%ld",whis->windownr);
            Setstemvar(ac,stem,index,"WINDOW",buf);
            Setstemvar(ac,stem,index,"URL",
               whis->titleurl?(UBYTE *)Agetattr(whis->titleurl,AOURL_Url):NULLSTRING);
            Setstemvar(ac,stem,index,"TITLE",whis->title?whis->title:NULLSTRING);
         }
      }
   }
   sprintf(buf,"%ld",index);
   Setstemvar(ac,stem,0,NULL,buf);
   ReleaseSemaphore(&whissema);
}

#ifdef DEVELOPER

static UBYTE *Frameid(UBYTE *id)
{  static UBYTE buf[64];
   UBYTE *p,*q;
   if(id)
   {  q=buf;
      for(p=id;*p;p++)
      {  if(q==buf) *q++='#';
         else *q++='.';
         q+=sprintf(q,"%d",*p);
      }
      *q='\0';
   }
   else strcpy(buf,"_top");
   return buf;
}

void __stdargs Dumpwinhis(struct Winhis *whis)
{  struct Framehis *fhis;
   if(whis)
   {  printf("WINHIS %08lx%s%s%s for %s\n",(long)whis,
         (whis->wflags&WINHF_SKIP)?" skip":"",
         (whis->wflags&WINHF_HISTORY)?" history":"",
         (whis->wflags&WINHF_NEW)?" new":"",
         Frameid(whis->frameid));
      for(fhis=whis->frames.first;fhis->next;fhis=fhis->next)
      {  printf("  ID %s URL %s\n",Frameid(fhis->id),(UBYTE *)Agetattr(fhis->url,AOURL_Url));
      }
   }
}
#endif
