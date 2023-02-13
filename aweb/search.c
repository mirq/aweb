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

/* search.c - AWeb search function window object */

#include "aweb.h"
#include "search.h"
#include "frame.h"
#include "url.h"
#include "window.h"
#include "application.h"
#include <reaction/reaction.h>
#include <proto/utility.h>
#include <proto/intuition.h>

/*------------------------------------------------------------------------*/

struct Search
{  struct Aobject object;
   void *frame;
   void *winobj;                 /* The CA window object */
   struct Window *window;
   struct Gadget *layout,*stringgad,*casegad;
   struct Buffer *buffer;        /* Current buffer to search in */
   long pos;                     /* Current search position, or -1 to start at frame top */
   long length;                  /* Current highlight length, or 0 if no highlight */
   long startpos;                /* First valid buffer position */
   void *nfwinobj;               /* The CA "Not found" window object */
   struct Window *nfwindow;
   struct Image *nflabel;
   UBYTE *title;
};

static LIST(Search) searchreqs;

enum SEARCH_GADGET_IDS
{  SGID_SEARCH=1,SGID_FROMTOP,SGID_BACKWARDS,SGID_CANCEL,
   SGID_NFOK,
};

static UBYTE *laststring;

/*------------------------------------------------------------------------*/

/* Remember the last string searched */
static void Setlaststring(UBYTE *string)
{  if(laststring) FREE(laststring);
   laststring=Dupstr(string,-1);
}

/* Close the "Not found" window */
static void Closenfwindow(struct Search *sr)
{  if(sr->nfwinobj)
   {  DisposeObject(sr->nfwinobj);
      sr->nfwinobj=NULL;
      sr->nfwindow=NULL;
   }
   if(sr->nflabel)
   {  DisposeObject(sr->nflabel);
      sr->nflabel=NULL;
   }
}

/* Open the "Not found" window */
static void Opennfwindow(struct Search *sr)
{  struct Screen *screen;
   struct MsgPort *port;
   sr->nflabel=LabelObject,
      LABEL_DrawInfo,Agetattr(Aweb(),AOAPP_Drawinfo),
      LABEL_Text,AWEBSTR(MSG_SEARCH_NOTFOUND),
   End;
   if(!sr->nflabel) goto err;
   Agetattrs(Aweb(),
      AOAPP_Screen,(Tag)&screen,
      AOAPP_Reactionport,(Tag)&port,
      TAG_END);
   sr->nfwinobj=WindowObject,
      WA_Title,AWEBSTR(MSG_REQUEST_TITLE),
      WA_AutoAdjust,TRUE,
      WA_DragBar,TRUE,
      WA_DepthGadget,TRUE,
      WA_Activate,TRUE,
      WA_SimpleRefresh,TRUE,
      WA_PubScreen,screen,
      WA_Left,sr->window->LeftEdge+10,
      WA_Top,sr->window->TopEdge+20,
      WA_BackFill,&requestbackfillhook,
      WINDOW_SharedPort,port,
      WINDOW_UserData,sr,
      WINDOW_Layout,sr->layout=VLayoutObject,
         LAYOUT_SpaceOuter,TRUE,
         LAYOUT_HorizAlignment,LALIGN_CENTER,
         StartMember,ButtonObject,
            GA_Image,sr->nflabel,
            GA_ReadOnly,TRUE,
         EndMember,
         CHILD_MinWidth,sr->nflabel->Width+32,
         CHILD_MinHeight,sr->nflabel->Height+16,
         StartMember,ButtonObject,
            GA_ID,SGID_NFOK,
            GA_RelVerify,TRUE,
            GA_Text,AWEBSTR(MSG_SEARCH_OK),
         EndMember,
         CHILD_WeightedWidth,0,
         CHILD_NominalSize,TRUE,
      End,
   EndWindow;
   if(!sr->nfwinobj) goto err;
   if(!(sr->nfwindow=(struct Window *)RA_OpenWindow(sr->nfwinobj))) goto err;
   return;

err:
   Closenfwindow(sr);
}

/* Close the search requester window */
static void Closesearchreq(struct Search *sr)
{  if(sr->winobj)
   {  if(sr->stringgad)
      {  Setlaststring((UBYTE *)Getvalue(sr->stringgad,STRINGA_TextVal));
      }
      if(sr->winobj) DisposeObject(sr->winobj);
      sr->winobj=NULL;
   }
}

/* Set the requester title */
static void Settitle(struct Search *sr)
{  UBYTE *title=NULL,*name=NULL,*newtitle;
   void *url=NULL,*win=NULL;
   Agetattrs(sr->frame,
      AOFRM_Url,(Tag)&url,
      AOFRM_Name,(Tag)&name,
      AOBJ_Window,(Tag)&win,
      AOFRM_Title,(Tag)&title,
      TAG_END);
   if(!title) title=(UBYTE *)Agetattr(url,AOURL_Url);
   if(win && title)
   {  if(newtitle=Windowtitle(win,name,title))
      {  if(sr->window) SetWindowTitles(sr->window,newtitle,(UBYTE *)~0);
         if(sr->title) FREE(sr->title);
         sr->title=newtitle;
      }
   }
}

/* Open the search requester window */
static BOOL Opensearchreq(struct Search *sr)
{  struct Screen *screen;
   struct MsgPort *port;
   Settitle(sr);
   Agetattrs(Aweb(),
      AOAPP_Screen,(Tag)&screen,
      AOAPP_Reactionport,(Tag)&port,
      TAG_END);
   sr->winobj=WindowObject,
      WA_Title,sr->title,
      WA_AutoAdjust,TRUE,
      WA_DragBar,TRUE,
      WA_CloseGadget,TRUE,
      WA_DepthGadget,TRUE,
      WA_SizeGadget,TRUE,
      WA_Activate,TRUE,
      WA_SimpleRefresh,TRUE,
      WA_PubScreen,screen,
      WINDOW_SharedPort,port,
      WINDOW_UserData,sr,
      WINDOW_Position,WPOS_CENTERSCREEN,
      WINDOW_LockHeight,TRUE,
      WINDOW_Layout,sr->layout=VLayoutObject,
         LAYOUT_SpaceOuter,TRUE,
         StartMember,sr->stringgad=StringObject,
            STRINGA_TextVal,laststring?laststring:NULLSTRING,
            STRINGA_MaxChars,128,
         EndMember,
         MemberLabel(AWEBSTR(MSG_SEARCH_STRING)),
         StartMember,sr->casegad=CheckBoxObject,
            GA_Text,AWEBSTR(MSG_SEARCH_IGNORECASE),
            GA_Selected,TRUE,
         EndMember,
         StartMember,HLayoutObject,
            StartMember,ButtonObject,
               GA_ID,SGID_SEARCH,
               GA_RelVerify,TRUE,
               GA_Text,AWEBSTR(MSG_SEARCH_SEARCH),
            EndMember,
            StartMember,ButtonObject,
               GA_ID,SGID_FROMTOP,
               GA_RelVerify,TRUE,
               GA_Text,AWEBSTR(MSG_SEARCH_FROMTOP),
            EndMember,
            StartMember,ButtonObject,
               GA_ID,SGID_BACKWARDS,
               GA_RelVerify,TRUE,
               GA_Text,AWEBSTR(MSG_SEARCH_BACKWARDS),
            EndMember,
            StartMember,ButtonObject,
               GA_ID,SGID_CANCEL,
               GA_RelVerify,TRUE,
               GA_Text,AWEBSTR(MSG_SEARCH_CANCEL),
            EndMember,
         EndMember,
      End,
   EndWindow;
   if(!sr->winobj) return FALSE;
   if(!(sr->window=(struct Window *)RA_OpenWindow(sr->winobj))) return FALSE;
   ActivateLayoutGadget(sr->layout,sr->window,NULL,(ULONG) sr->stringgad);
   return TRUE;
}

/*------------------------------------------------------------------------*/

/* Remove the previous highlight */
static void Unhighlight(struct Search *sr)
{  struct Amsearch ams={{0}};
   if(sr->length)
   {  ams.amsg.method=AOM_SEARCHSET;
      ams.pos=sr->pos;
      ams.length=sr->length;
      ams.flags=AMSF_UNHIGHLIGHT;
      ams.text=sr->buffer;
      AmethodA(sr->frame,(struct Amessage *)&ams);
      sr->length=0;
   }
}

/* String comparizon functions ignoring different forms of whitespace */
static int Strncmpsp(UBYTE *a,UBYTE *b,int n)
{  while(*a && *b && --n)
   {  if(*a==*b || (Isspace(*a)&&Isspace(*b))) { a++;b++; }
      else break;
   }
   if(Isspace(*a)&&Isspace(*b)) return 0;
   else return *b-*a;
}

static int Strnicmpsp(UBYTE *a,UBYTE *b,int n)
{  while(*a && *b && --n)
   {  if(toupper(*a)==toupper(*b) || (Isspace(*a)&&Isspace(*b))) { a++;b++; }
      else break;
   }
   if(Isspace(*a)&&Isspace(*b)) return 0;
   return toupper(*b)-toupper(*a);
}

/* Find the next(previous) occurrence of string in buffer */
static long Searchbuffer(struct Buffer *buf,long startpos,long pos,UBYTE *string,
   BOOL ignore,short d)
{  UBYTE *p,*end;
   long len=strlen(string);
   if(buf && buf->buffer)
   {  if(pos<0)
      {  if(d>0) pos=startpos;
         else pos=buf->length-len;
      }
      p=buf->buffer+pos;
      if(d>0) end=buf->buffer+buf->length-len;
      else end=buf->buffer+startpos-1;
      p+=d;
      while(p!=end)
      {  if(ignore)
         {  if(!Strnicmpsp(p,string,len)) return p-buf->buffer;
         }
         else
         {  if(!Strncmpsp(p,string,len)) return p-buf->buffer;
         }
         p+=d;
      }
   }
   return -1;
}

/* Do the search. */
static void Dosearch(struct Search *sr,short how)
{  struct Amsearch ams={{0}};
   short d=0;
   long newpos;
   UBYTE *string;
   BOOL ignore;
   Closenfwindow(sr);
   switch(how)
   {  case SGID_SEARCH:
         d=1;
         ams.flags|=AMSF_CURRENTPOS;
         break;
      case SGID_FROMTOP:
         d=1;
         sr->pos=-1;
         break;
      case SGID_BACKWARDS:
         d=-1;
         ams.flags|=AMSF_CURRENTPOS;
         break;
   }
   if(sr->pos<0)
   {  ams.amsg.method=AOM_SEARCHPOS;
      ams.pos=-1;
      AmethodA(sr->frame,(struct Amessage *)&ams);
      sr->buffer=ams.text;
      sr->startpos=ams.startpos;
      if(ams.flags&AMSF_CURRENTPOS) sr->pos=ams.pos;
      else sr->pos=ams.startpos;
   }
   string=(UBYTE *)Getvalue(sr->stringgad,STRINGA_TextVal);
   Setlaststring(string);
   ignore=Getselected(sr->casegad);
   newpos=Searchbuffer(sr->buffer,sr->startpos,sr->pos,string,ignore,d);
   if(newpos>=0)
   {  ams.amsg.method=AOM_SEARCHSET;
      ams.pos=newpos;
      ams.length=strlen(string);
      ams.flags=AMSF_HIGHLIGHT;
      ams.text=sr->buffer;
      ams.top=-1;
      AmethodA(sr->frame,(struct Amessage *)&ams);  /* Clears our pos when frame scrolls */
      sr->pos=newpos;
      sr->length=strlen(string);
   }
   else
   {  Opennfwindow(sr);
   }
}

/*------------------------------------------------------------------------*/

/* Process IntuiMessages */
static void Processsearch(void)
{  struct Search *sr,*nextsr;
   ULONG result;
   BOOL done;
   for(sr=searchreqs.first;sr->object.next;sr=nextsr)
   {  nextsr=sr->object.next;
      done=FALSE;
      if(sr->nfwinobj)
      {  while(!done && (result=RA_HandleInput(sr->nfwinobj,NULL))!=WMHI_LASTMSG)
         {  switch(result&WMHI_CLASSMASK)
            {  case WMHI_GADGETUP:
                  if((result&WMHI_GADGETMASK)==SGID_NFOK) done=TRUE;
                  break;
               case WMHI_RAWKEY:
                  if((result&WMHI_GADGETMASK)==0x45) done=TRUE; /* esc */
                  break;
            }
         }
         if(done) Closenfwindow(sr);
      }
      done=FALSE;
      while(!done && (result=RA_HandleInput(sr->winobj,NULL))!=WMHI_LASTMSG)
      {  switch(result&WMHI_CLASSMASK)
         {  case WMHI_CLOSEWINDOW:
               done=TRUE;
               break;
            case WMHI_GADGETUP:
               switch(result&WMHI_GADGETMASK)
               {  case SGID_SEARCH:
                  case SGID_FROMTOP:
                  case SGID_BACKWARDS:
                     Unhighlight(sr);
                     Dosearch(sr,result&WMHI_GADGETMASK);
                     break;
                  case SGID_CANCEL:
                     done=TRUE;
                     break;
               }
               break;
            case WMHI_RAWKEY:
               switch(result&WMHI_GADGETMASK)
               {  case 0x45:     /* esc */
                     done=TRUE;
                     break;
                  case 0x43:     /* num enter */
                  case 0x44:     /* enter */
                     Unhighlight(sr);
                     Dosearch(sr,SGID_SEARCH);
                     break;
               }
               break;
         }
      }
      if(done)
      {  Asetattrs(sr->frame,AOFRM_Search,FALSE,TAG_END);
      }
   }
}

/*------------------------------------------------------------------------*/

static long Setsearch(struct Search *sr,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Frame:
            sr->frame=(void *)tag->ti_Data;
            break;
         case AOSRH_Reset:
            sr->buffer=NULL;
            sr->pos=-1;
            Settitle(sr);
            break;
         case AOSRH_Scrolled:
            Unhighlight(sr);
            sr->pos=-1;
            break;
         case AOSRH_Activate:
            Closenfwindow(sr);
            if(sr->window)
            {  WindowToFront(sr->window);
               ActivateWindow(sr->window);
            }
            break;
         case AOAPP_Screenvalid:
            if(tag->ti_Data)
            {  Opensearchreq(sr);
            }
            else
            {  Closesearchreq(sr);
               Closenfwindow(sr);
            }
            break;
      }
   }
   return 0;
}

static void Disposesearch(struct Search *sr)
{  REMOVE(sr);
   Aremchild(Aweb(),(struct Aobject *)sr,AOREL_APP_USE_SCREEN);
   Unhighlight(sr);
   Closesearchreq(sr);
   Closenfwindow(sr);
   if(sr->title) FREE(sr->title);
   Amethodas(AOTP_OBJECT,sr,AOM_DISPOSE);
}

static struct Search *Newsearch(struct Amset *ams)
{  struct Search *sr;
   if(sr=Allocobject(AOTP_SEARCH,sizeof(struct Search),ams))
   {  ADDTAIL(&searchreqs,sr);
      sr->pos=-1;
      Setsearch(sr,ams);
      Aaddchild(Aweb(),(struct Aobject *)sr,AOREL_APP_USE_SCREEN);
      Asetattrs(Aweb(),
         AOAPP_Processtype,AOTP_SEARCH,
         AOAPP_Processfun,(Tag)Processsearch,
         TAG_END);
      if(!Agetattr(Aweb(),AOAPP_Screenvalid) || !Opensearchreq(sr))
      {  Disposesearch(sr);
         sr=NULL;
      }
   }
   return sr;
}

static void Deinstallsearch(void)
{  struct Search *sr;
   while((sr=searchreqs.first) && sr->object.next)
   {  Asetattrs(sr->frame,AOFRM_Search,FALSE,TAG_END);
   }
   if(laststring) FREE(laststring);
}

USRFUNC_H2
(
static long  , Search_Dispatcher,
struct Search *,sr,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newsearch((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setsearch(sr,(struct Amset *)amsg);
         break;
         break;
      case AOM_DISPOSE:
         Disposesearch(sr);
         break;
      case AOM_DEINSTALL:
         Deinstallsearch();
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installsearch(void)
{  NEWLIST(&searchreqs);
   if(!Amethod(NULL,AOM_INSTALL,AOTP_SEARCH,(Tag)Search_Dispatcher)) return FALSE;
   return TRUE;
}
