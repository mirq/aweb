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

/* author.c - AWeb Basic HTTP authorization */

#include "aweb.h"
#include "url.h"
#include "fetch.h"
#include "application.h"
#include "task.h"
#include "fetchdriver.h"
#include "versions.h"

#include <intuition/intuition.h>

#undef NO_INLINE_STDARG
#include <reaction/reaction.h>
#define NO_INLINE_STDARG

#include <proto/intuition.h>
#include <proto/utility.h>

#include "proto/awebauthorize.h"

#define AUTHSAVENAME "AWebPath:AWeb.auth"

/*-----------------------------------------------------------------------*/
/* wrappers for library functions */

/*-----------------------------------------------------------------------*/

static LIST(Authnode) auths;
static struct SignalSemaphore authsema;
static BOOL changed;

static struct Authedit *authedit;

/*-----------------------------------------------------------------------*/

/* Signal edit task that list has changed */
static void Modifiedlist(void)
{  if(authedit && authedit->task)
   {  Asetattrsasync(authedit->task,AOAEW_Modified,TRUE,TAG_END);
   }
}

/* Signal edit task to show the passwords or not */
static void Sendshowpass(BOOL showpass)
{  if(authedit && authedit->task)
   {  Asetattrsasync(authedit->task,AOAEW_Showpass,showpass,TAG_END);
   }
}

static struct Authnode *Findauth(UBYTE *server,UBYTE *realm)
{  struct Authnode *an,*anf=NULL;
   ObtainSemaphore(&authsema);
   for(an=auths.first;an->next;an=an->next)
   {  if(STRIEQUAL(an->auth.server,server) && STRIEQUAL(an->auth.realm,realm))
      {  anf=an;
         break;
      }
   }
   ReleaseSemaphore(&authsema);
   return anf;
}

/* Remember info from this auth. Add to head so Guessauthorize() will find
 * the most recent. */
static void Rememberauth(struct Authorize *auth)
{  struct Authnode *an;
   ObtainSemaphore(&authsema);
   if(!(an=Findauth(auth->server,auth->realm)))
   {  if(an=ALLOCSTRUCT(Authnode,1,MEMF_CLEAR))
      {  ADDHEAD(&auths,an);
         if(!(an->auth.server=Dupstr(auth->server,-1))
         || !(an->auth.realm=Dupstr(auth->realm,-1)))
         {  if(an->auth.server) FREE(an->auth.server);
            if(an->auth.realm) FREE(an->auth.realm);
            REMOVE(an);
            FREE(an);
            an=NULL;
         }
      }
   }
   if(an && auth->cookie)
   {  if(an->auth.cookie) FREE(an->auth.cookie);
      an->auth.cookie=Dupstr(auth->cookie,-1);
   }
   changed=TRUE;
   ReleaseSemaphore(&authsema);
   Modifiedlist();
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

/* Decodes cookie from Authorize. Creates dynamic string "userid:password"*/
static UBYTE *Decodecookie(struct Authorize *auth)
{  UBYTE *temp,*src,*dest;
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
   }
   return temp;
}

static void Checkpassword(struct Authorize *auth)
{
   struct Authnode *an;
   UBYTE *pw, *masterpw;
   BOOL result=FALSE;

   ObtainSemaphore(&authsema);
   if(!(an=Findauth(auth->server,auth->realm)))
   {  /* if not exists add the password to the list */
      if(an=ALLOCSTRUCT(Authnode,1,MEMF_CLEAR))
      {  ADDHEAD(&auths,an);
         if(!(an->auth.server=Dupstr(auth->server,-1))
         || !(an->auth.realm=Dupstr(auth->realm,-1)))
         {  if(an->auth.server) FREE(an->auth.server);
            if(an->auth.realm) FREE(an->auth.realm);
            REMOVE(an);
            FREE(an);
            an=NULL;
         }
      }
      if(an)
      {
         if(an->auth.cookie) FREE(an->auth.cookie);
         an->auth.cookie=Dupstr(auth->cookie,-1);
         Modifiedlist();
         changed=TRUE;
      }
   }
   else
   {  /* check if the Passwords are equal */
      if(an && auth->cookie)
      {
         pw= Decodecookie(auth);
         masterpw= Decodecookie(&an->auth);
         if(strlen(pw)==strlen(masterpw) && strncmp(pw,masterpw,strlen(pw))==0)
         {
            result = TRUE;
         }
         if(pw) FREE(pw);
         if(masterpw) FREE(masterpw);
      }
   }
   ReleaseSemaphore(&authsema);
   Sendshowpass(result);
}

static void Loadauthor(void)
{  struct Authorize auth={0};
   long fh;
   UBYTE *buffer=NULL;
   UBYTE *p,*end=NULL;
   __aligned struct FileInfoBlock fib={0};
   if(fh=Open(AUTHSAVENAME,MODE_OLDFILE))
   {  if(ExamineFH(fh,&fib))
      {  if(fib.fib_Size)
         {  if(buffer=ALLOCTYPE(UBYTE,fib.fib_Size,0))
            {  Read(fh,buffer,fib.fib_Size);
               end=buffer+fib.fib_Size;
            }
         }
      }
      Close(fh);
   }
   if(buffer)
   {  p=buffer+strlen(buffer)+1;   /* warning line */
      while(p<end)
      {  auth.server=p;
         auth.realm=auth.server+strlen(auth.server)+1;
         if(auth.realm>=end) break;
         auth.cookie=auth.realm+strlen(auth.realm)+1;
         if(auth.cookie>=end) break;
         p=auth.cookie+strlen(auth.cookie)+1;
         if(p>end) break;
         Rememberauth(&auth);
      }
      FREE(buffer);
   }
   changed=FALSE;
}

/*-----------------------------------------------------------------------*/

void Authorize(struct Fetchdriver *fd, struct Authorize *auth, BOOL proxy)
{
    struct Authorreq areq = {0};
    void *libbase;

    areq.auth         = auth;
    areq.name         = fd->name;
    areq.proxy        = proxy;
    areq.Rememberauth = Rememberauth;

    if((libbase = Openaweblib(AWEBLIBPATH AUTHORIZE_AWEBLIB, AUTHORIZE_VERSION)))
    {
        __Authorreq_WB(libbase, &areq, FALSE);

        Closeaweblib(libbase);
    }
}

static void Domasterpw(struct Authedit *aew)
{
   struct Authorize *auth;
   struct Authorreq areq = {0};
   long result = 1;
   void *libbase;

   if(!Findauth("Master","Master"))
   {
      result=Syncrequest(AWEBSTR(MSG_REQUEST_TITLE),
                         AWEBSTR(MSG_AUTHEDIT_SHOWPWREQ),
                         AWEBSTR(MSG_AUTHEDIT_SHOWPWREQ_BUTTONS),
                         0);
   }
   if(result)
   {
      if(auth=Newauthorize("Master","Master"))
      {
         areq.auth=auth;
         areq.name="";
         areq.proxy=FALSE;
         areq.onlypw=TRUE;
         areq.Rememberauth = Checkpassword;

         if((libbase = Openaweblib(AWEBLIBPATH AUTHORIZE_AWEBLIB, AUTHORIZE_VERSION)))
         {
            __Authorreq_WB(libbase, &areq, TRUE);
            Closeaweblib(libbase);
         }
      }
   }
   Setgadgetattrs(aew->toplayout,aew->window,NULL,
      GA_Disabled,FALSE,
      TAG_END);
   Setgadgetattrs(aew->showpassgad,aew->window,NULL,
      GA_Selected,FALSE,
      TAG_END);
}

/*-----------------------------------------------------------------------*/

/* Open aweblib and start edit task */
static BOOL Starttask(struct Authedit *aew)
{
    Agetattrs
    (
        Aweb(),
        AOAPP_Screenname,(Tag)&aew->screenname,
        AOAPP_Pwfont,    (Tag)&aew->pwfont,
        TAG_END
    );

    if ((aew->libbase = Openaweblib(AWEBLIBPATH AUTHORIZE_AWEBLIB, AUTHORIZE_VERSION)))
    {
         aew->task = Anewobject
         (
             AOTP_TASK,
             AOTSK_Entry,    (Tag)__AuthGetTaskFunc_WB(aew->libbase, 0),
             AOTSK_Name,     (Tag)"AWeb authorization edit",
             AOTSK_Userdata, (Tag)aew,
             AOBJ_Target,    (Tag)aew,
             TAG_END
         );

         if(aew->task)
             Asetattrs(aew->task, AOTSK_Start, TRUE, TAG_END);
         else
         {
             Closeaweblib(aew->libbase);
             aew->libbase = NULL;
         }
    }

    return BOOLVAL(aew->task);
}

/* Dispose task and close aweblib */
static void Stoptask(struct Authedit *aew)
{  if(aew->libbase)
   {  if(aew->task)
      {  Adisposeobject(aew->task);
         aew->task=NULL;
      }
      Closeaweblib(aew->libbase);
      aew->libbase=NULL;
   }
}

/*------------------------------------------------------------------------*/

static void Finddeletenode(struct Authnode *andel)
{  struct Authnode *an;
   ObtainSemaphore(&authsema);
   for(an=auths.first;an->next;an=an->next)
   {  if(an==andel)
      {  REMOVE(an);
         if(an->auth.server) FREE(an->auth.server);
         if(an->auth.realm) FREE(an->auth.realm);
         if(an->auth.cookie) FREE(an->auth.cookie);
         FREE(an);
         changed=TRUE;
         break;
      }
   }
   ReleaseSemaphore(&authsema);
}

static void Findchangenode(struct Authnode *anchg,struct Authorize *auth)
{  struct Authnode *an;
   ObtainSemaphore(&authsema);
   for(an=auths.first;an->next;an=an->next)
   {  if(an==anchg)
      {  if(auth->cookie)
         {  if(an->auth.cookie) FREE(an->auth.cookie);
            an->auth.cookie=Dupstr(auth->cookie,-1);
            changed=TRUE;
         }
         break;
      }
   }
   ReleaseSemaphore(&authsema);
}

static long Updateauthedit(struct Authedit *aew,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   BOOL close=FALSE;
   struct Authnode *an=NULL;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOAEW_Close:
            if(tag->ti_Data) close=TRUE;
            break;
         case AOAEW_Dimx:
            prefs.window.autx=aew->x=tag->ti_Data;
            break;
         case AOAEW_Dimy:
            prefs.window.auty=aew->y=tag->ti_Data;
            break;
         case AOAEW_Dimw:
            prefs.window.autw=aew->w=tag->ti_Data;
            break;
         case AOAEW_Dimh:
            prefs.window.auth=aew->h=tag->ti_Data;
            break;
         case AOAEW_Authnode:
            an=(struct Authnode *)tag->ti_Data;
            break;
         case AOAEW_Delete:
            if(tag->ti_Data)
            {  Finddeletenode(an);
            }
            break;
         case AOAEW_Change:
            if(tag->ti_Data)
            {  Findchangenode(an,(struct Authorize *)tag->ti_Data);
            }
            break;
         case AOAEW_Domasterpw:
            if(tag->ti_Data)
            {  Domasterpw(aew);
            }
            break;
      }
   }
   if(close && !(aew->flags&AEWF_BREAKING))
   {  Stoptask(aew);
   }
   return 0;
}

static long Setauthedit(struct Authedit *aew,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOAPP_Screenvalid:
            if(!tag->ti_Data)
            {  if(aew->task)
               {  aew->flags|=AEWF_BREAKING|AEWF_WASOPEN;
                  Stoptask(aew);
                  aew->flags&=~AEWF_BREAKING;
               }
            }
            else
            {  if((aew->flags&AEWF_WASOPEN) && !aew->task)
               {  Starttask(aew);
               }
            }
            break;
      }
   }
   return 0;
}

static struct Authedit *Newauthedit(struct Amset *ams)
{  struct Authedit *aew;
   if(aew=Allocobject(AOTP_AUTHEDIT,sizeof(struct Authedit),ams))
   {  Aaddchild(Aweb(),(struct Aobject *)aew,(ULONG)AOREL_APP_USE_SCREEN);
      if(Agetattr(Aweb(),AOAPP_Screenvalid))
      {  aew->authsema=&authsema;
         aew->auths=(APTR)&auths;
         aew->x=prefs.window.autx;
         aew->y=prefs.window.auty;
         aew->w=prefs.window.autw;
         aew->h=prefs.window.auth;
         if(!Starttask(aew))
         {  Adisposeobject((struct Aobject *)aew);
            aew=NULL;
         }
      }
   }
   return aew;
}

static void Disposeauthedit(struct Authedit *aew)
{  if(aew->task)
   {  aew->flags|=AEWF_BREAKING;
      Stoptask(aew);
   }
   Aremchild(Aweb(),(struct Aobject *)aew,(ULONG)AOREL_APP_USE_SCREEN);
   Amethodas(AOTP_OBJECT,aew,AOM_DISPOSE);
}

static void Deinstallauthedit(void)
{  if(authedit) Adisposeobject((struct Aobject *)authedit);
}

USRFUNC_H2
(
static long  , Authedit_Dispatcher,
struct Authedit *,aew,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newauthedit((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setauthedit(aew,(struct Amset *)amsg);
         break;
      case AOM_UPDATE:
         result=Updateauthedit(aew,(struct Amset *)amsg);
         break;
      case AOM_DISPOSE:
         Disposeauthedit(aew);
         break;
      case AOM_DEINSTALL:
         Deinstallauthedit();
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installauthedit(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_AUTHEDIT,(Tag)Authedit_Dispatcher)) return FALSE;
   return TRUE;
}

void Openauthedit(void)
{  if(authedit && authedit->task)
   {  Asetattrsasync(authedit->task,AOAEW_Tofront,TRUE,TAG_END);
   }
   else
   {  if(authedit) Adisposeobject((struct Aobject *)authedit);
      authedit=Anewobject(AOTP_AUTHEDIT,TAG_END);
   }
}

void Closeauthedit(void)
{  if(authedit)
   {  Adisposeobject((struct Aobject *)authedit);
      authedit=NULL;
   }
}

BOOL Isopenauthedit(void)
{  return (BOOL)(authedit && authedit->task);
}

/*-----------------------------------------------------------------------*/

BOOL Initauthor(void)
{  NEWLIST(&auths);
   InitSemaphore(&authsema);
   Loadauthor();
   return TRUE;
}

void Freeauthor(void)
{  if(auths.first)
   {  if(changed) Saveauthor();
      Flushauthor();
   }
}

struct Authorize *Newauthorize(UBYTE *server,UBYTE *realm)
{  struct Authnode *an;
   struct Authorize *auth=ALLOCSTRUCT(Authorize,1,MEMF_CLEAR);
   if(auth)
   {  auth->server=Dupstr(server,-1);
      auth->realm=Dupstr(realm,-1);
      ObtainSemaphore(&authsema);
      an=Findauth(server,realm);
      if(an && an->auth.cookie)
      { auth->cookie=Dupstr(an->auth.cookie,-1);
      }
      ReleaseSemaphore(&authsema);
   }
   return auth;
}

void Freeauthorize(struct Authorize *auth)
{  if(auth->server) FREE(auth->server);
   if(auth->realm) FREE(auth->realm);
   if(auth->cookie) FREE(auth->cookie);
   FREE(auth);
}

struct Authorize *Guessauthorize(UBYTE *server)
{  struct Authnode *an;
   struct Authorize *auth=NULL;
   ObtainSemaphore(&authsema);
   for(an=auths.first;an->next;an=an->next)
   {  if(STRIEQUAL(an->auth.server,server))
      {  auth=ALLOCSTRUCT(Authorize,1,MEMF_CLEAR);
         if(auth)
         {  auth->server=Dupstr(an->auth.server,-1);
            auth->realm=Dupstr(an->auth.realm,-1);
            auth->cookie=Dupstr(an->auth.cookie,-1);
         }
         break;
      }
   }
   ReleaseSemaphore(&authsema);
   return auth;
}

struct Authorize *Dupauthorize(struct Authorize *auth)
{  struct Authorize *newauth=ALLOCSTRUCT(Authorize,1,MEMF_CLEAR);
   if(newauth)
   {  if((!auth->server || (newauth->server=Dupstr(auth->server,-1)))
      && (!auth->realm || (newauth->realm=Dupstr(auth->realm,-1)))
      && (!auth->cookie || (newauth->cookie=Dupstr(auth->cookie,-1)))
      )  return newauth;
      Freeauthorize(newauth);
   }
   return NULL;
}

void Flushauthor(void)
{  struct Authnode *an;
   ObtainSemaphore(&authsema);
   while(an=(struct Authnode *)REMHEAD(&auths))
   {  if(an->auth.server) FREE(an->auth.server);
      if(an->auth.realm)  FREE(an->auth.realm);
      if(an->auth.cookie) FREE(an->auth.cookie);
      FREE(an);
   }
   ReleaseSemaphore(&authsema);
   Modifiedlist();
}

void Saveauthor(void)
{  long fh;
   struct Authnode *an;
   UBYTE *warning=" AWeb private file - DO NOT MODIFY";
   if(fh=Open(AUTHSAVENAME,MODE_NEWFILE))
   {  Write(fh,warning,strlen(warning)+1);
      ObtainSemaphore(&authsema);
      for(an=auths.first;an->next;an=an->next)
      {  if(an->auth.cookie)
         {  Write(fh,an->auth.server,strlen(an->auth.server)+1);
            Write(fh,an->auth.realm,strlen(an->auth.realm)+1);
            Write(fh,an->auth.cookie,strlen(an->auth.cookie)+1);
         }
      }
      changed=FALSE;
      ReleaseSemaphore(&authsema);
      Close(fh);
   }
}

void Forgetauthorize(struct Authorize *auth)
{  struct Authnode *an;
   ObtainSemaphore(&authsema);
   for(an=auths.first;an->next;an=an->next)
   {  if(STRIEQUAL(an->auth.server,auth->server) && STRIEQUAL(an->auth.realm,auth->realm))
      {  REMOVE(an);
         if(an->auth.server) FREE(an->auth.server);
         if(an->auth.realm) FREE(an->auth.realm);
         if(an->auth.cookie) FREE(an->auth.cookie);
         FREE(an);
         break;
      }
   }
   changed=TRUE;
   ReleaseSemaphore(&authsema);
   Modifiedlist();
}

void Setauthorize(struct Authorize *auth, UBYTE *userid, UBYTE *passwd)
{
    void *libbase;

    if((libbase = Openaweblib(AWEBLIBPATH AUTHORIZE_AWEBLIB, AUTHORIZE_VERSION)))
    {
         __Authorset_WB(libbase, auth, userid, passwd);

         Closeaweblib(libbase);
    }
}
