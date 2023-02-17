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

/* request.c - AWeb requesters */

#include "aweb.h"
#include "application.h"
#include "window.h"
#include "fetch.h"
#include "splashimages.h"
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <dos/dostags.h>
#include <reaction/reaction.h>

#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>

struct Interface;

struct Classbases
{
    struct Library *Windowbase,*Layoutbase,*Buttonbase,*Labelbase,*Fuelgaugebase;
    struct Interface *IWindow, *ILayout, *IButton, *ILabel, *IFuelgauge;
};

struct Awebrequest
{  NODE(Awebrequest);
   UWORD type;
   struct Image *label;
   void *winobj;
   struct Window *window;
   struct Gadget *firstbutton;
   void *buffer,*buffer2;
   struct Classbases *cb;
   requestfunc *reqfun;
   void *data;
   UBYTE *url;
};

enum AR_TYPES
{  ARTYPE_ABOUT=1,ARTYPE_QUIT,ARTYPE_UNREG,
};

static LIST(Awebrequest) requests;
static struct SignalSemaphore reqsema;
static BOOL inited;

static struct MsgPort *reqport;

static UBYTE *aboutbuttons[]={ "_Ok",NULL };
static UBYTE *quitbuttons[]={ "_Ok","_Cancel",NULL };
static UBYTE *unregbuttons[]={ "_Ok",NULL };

struct Hook requestbackfillhook;
struct Hook nobackfillhook;

struct Progressreq
{  struct Image *label;
   struct Gadget *fuelgad;
   void *winobj;
   struct Window *window;
   struct Screen *screen;
   struct Classbases *cb;
};

#define GID_COPYURLTOCLIP  1001
#define GID_URLCLICKED     2001
#define GID_APLCLICKED     2002

/*-----------------------------------------------------------------------*/

static void Closeclasses(struct Classbases *cb)
{
   if(cb->Labelbase) Closelib((struct Library **)&cb->Labelbase, (struct Interface **)&cb->ILabel);
   if(cb->Buttonbase) Closelib((struct Library **)&cb->Buttonbase, &cb->IButton);
   if(cb->Fuelgaugebase) Closelib((struct Library **)&cb->Fuelgaugebase, &cb->IFuelgauge);
   if(cb->Layoutbase) Closelib((struct Library **)&cb->Layoutbase, &cb->ILayout);
   if(cb->Windowbase) Closelib((struct Library **)&cb->Windowbase, &cb->IWindow);
}

static struct Classbases *Openclasses(void)
{  struct Classbases *cb=ALLOCSTRUCT(Classbases,1,MEMF_CLEAR);
   BOOL allopen = FALSE;
   if(cb)
   {
      if((WindowBase=cb->Windowbase=Openclass("window.class",OSNEED(0,44),&cb->Windowbase,&cb->IWindow))
      && (LayoutBase=cb->Layoutbase=Openclass("gadgets/layout.gadget",OSNEED(0,44),&cb->Layoutbase,&cb->ILayout))
      && (ButtonBase=cb->Buttonbase=Openclass("gadgets/button.gadget",OSNEED(0,44),&cb->Buttonbase,&cb->IButton))
      && (FuelGaugeBase=cb->Fuelgaugebase=Openclass("gadgets/fuelgauge.gadget",OSNEED(0,44),&cb->Fuelgaugebase,&cb->IFuelgauge))
      && (LabelBase=cb->Labelbase=Openclass("images/label.image",OSNEED(0,44),&cb->Labelbase,&cb->ILabel))) allopen = TRUE;
#if defined(__amigaos4__)
      IWindow = (struct WindowIFace *)cb->IWindow;
      ILayout = (struct LayoutIFace *)cb->ILayout;
      IButton = (struct ButtonIFace *)cb->IButton;
      IFuelGauge = (struct FuelGaugeIFace *)cb->IFuelgauge;
      ILabel = (struct LabelIFace *)cb->ILabel;
#endif
      if (allopen == TRUE)return cb;
      Closeclasses(cb);
      FREE(cb);
   }
   return NULL;
}

static void Freeawebrequest(struct Awebrequest *ar)
{  if(ar->winobj) DisposeObject(ar->winobj);
   if(ar->label) DisposeObject(ar->label);
   if(ar->buffer) FREE(ar->buffer);
   if(ar->buffer2) FREE(ar->buffer2);
   if(ar->url) FREE(ar->url);
   if(ar->cb)
   {  Closeclasses(ar->cb);
      FREE(ar->cb);
   }
   FREE(ar);
}

DECLARE_HOOK
(
    static void __saveds, Backfillhook,
    struct Hook *,         hook, A0,
    struct RastPort *,     lrp,  A2,
    struct Layermessage *, msg,  A1
)
{
   USRFUNC_INIT

   static UWORD pattern[]={ 0xAAAA,0x5555 };
   struct RastPort rp=*lrp;
   rp.Layer=NULL;
   SetABPenDrMd(&rp,(long)(hook->h_Data),0,JAM1);
   SetAfPt(&rp,pattern,1);
   RectFill(&rp,msg->rect.MinX,msg->rect.MinY,msg->rect.MaxX,msg->rect.MaxY);

   USRFUNC_EXIT
}

/* if (string) is given, a string gadget is added */
static struct Awebrequest *Makeawebrequest2(struct Screen *screen,
   struct Image *label,UBYTE **buttonlabels,UBYTE *title,void *buffer,void *buffer2,
   struct Classbases *cb,UWORD type,BOOL synch,requestfunc *reqfun,void *data,
   UBYTE *string,UBYTE *url)
{  struct Awebrequest *ar=ALLOCSTRUCT(Awebrequest,1,MEMF_CLEAR);
   struct Gadget *toplayout,*stringrow,*stringgad=0,*buttonrow,*button;
   ULONG labelw,labelh;
   short i;
   if(ar)
   {  ar->type=type;
      ar->label=label;
      ar->buffer=buffer;
      ar->buffer2=buffer2;
      ar->cb=cb;
      ar->reqfun=reqfun;
      ar->data=data;
      ar->url=Dupstr(url,-1);
      GetAttr(IA_Width,label,&labelw);
      GetAttr(IA_Height,label,&labelh);
      ar->winobj=WindowObject,
         WA_Title,title,
         WA_AutoAdjust,TRUE,
         WA_DragBar,TRUE,
         WA_DepthGadget,TRUE,
         WA_SizeGadget,BOOLVAL(string),
         WA_Activate,Awebactive(),
         WA_SimpleRefresh,TRUE,
         WA_PubScreen,screen,
         WA_IDCMP,IDCMP_RAWKEY,
         WA_BackFill,&requestbackfillhook,
         (synch?TAG_IGNORE:WINDOW_SharedPort),reqport,
         (type==ARTYPE_ABOUT || prefs.program.centerreq)?WINDOW_Position:TAG_IGNORE,WPOS_CENTERSCREEN,
         WINDOW_Layout,toplayout=VLayoutObject,
            LAYOUT_SpaceOuter,TRUE,
            LAYOUT_FixedVert,FALSE,
            LAYOUT_VertAlignment,LALIGN_TOP,
            StartMember,ButtonObject,
               GA_Image,label,
               GA_ReadOnly,TRUE,
            EndMember,
            CHILD_MinWidth,labelw+32,
            CHILD_MinHeight,labelh+16,
            StartMember,stringrow=HLayoutObject,
            EndMember,
            CHILD_WeightedHeight,0,
            StartMember,buttonrow=HLayoutObject,
               LAYOUT_EvenSize,TRUE,
            EndMember,
         End,
      EndWindow;
      if(ar->winobj)
      {  if(string)
         {  SetAttrs(stringrow,
               StartMember,stringgad=StringObject,
                  GA_ID,1,
                  GA_RelVerify,TRUE,
                  STRINGA_Buffer,string,
                  STRINGA_MaxChars,127,
                  STRINGA_TextVal,string,
               EndMember,
               TAG_END);
         }
         if(ar->url)
         {  SetAttrs(stringrow,
               StartMember,ButtonObject,
                  GA_ID,GID_COPYURLTOCLIP,
                  GA_RelVerify,TRUE,
                  GA_Text,AWEBSTR(MSG_REQUEST_COPYURL),
               EndMember,
               TAG_END);
         }
         for(i=0;buttonlabels[i];i++)
         {  SetAttrs(buttonrow,
               StartMember,button=ButtonObject,
                  GA_Text,buttonlabels[i],
                  GA_ID,(buttonlabels[i+1])?(i+1):0,
                  GA_RelVerify,TRUE,
               EndMember,
               CHILD_WeightedWidth,0,
               CHILD_NominalSize,TRUE,
               TAG_END);
            if(i==0) ar->firstbutton=button;
         }
         if(i<2) SetAttrs(buttonrow,
               LAYOUT_HorizAlignment,LALIGN_CENTER,
               TAG_END);
         ar->window=RA_OpenWindow(ar->winobj);
         if(ar->window && stringgad)
            ActivateLayoutGadget(toplayout,ar->window,NULL, (ULONG) stringgad);
      }
      if(!ar->window)
      {  Freeawebrequest(ar);
         ar=NULL;
      }
      else if(!synch) ADDTAIL(&requests,ar);
   }
   else
   {  if(label) DisposeObject(label);
      if(buffer) FREE(buffer);
      if(cb)
      {  Closeclasses(cb);
         FREE(cb);
      }
   }
   return ar;
}


static struct Awebrequest *Makeawebrequest3(struct Screen *screen,
   struct Gadget *gadget,UBYTE **buttonlabels,UBYTE *title,void *buffer,void *buffer2,
   struct Classbases *cb,UWORD type,BOOL synch,requestfunc *reqfun,void *data,
   UBYTE *string,UBYTE *url)
{  struct Awebrequest *ar=ALLOCSTRUCT(Awebrequest,1,MEMF_CLEAR);
   struct Gadget *toplayout,*stringrow,*stringgad=0,*buttonrow,*button;
   short i;
   if(ar)
   {  ar->type=type;
      ar->label=NULL; /*  we got a gadget so we mustn't dispose of the label */
      ar->buffer=buffer;
      ar->buffer2=buffer2;
      ar->cb=cb;
      ar->reqfun=reqfun;
      ar->data=data;
      ar->url=Dupstr(url,-1);
      ar->winobj=WindowObject,
      /*   WA_Title,title, */
         WA_AutoAdjust,TRUE,
      /*   WA_DragBar,TRUE,    */
      /*   WA_DepthGadget,TRUE, */
      /*   WA_SizeGadget,BOOLVAL(string),*/
         WA_Activate,Awebactive(),
         WA_SimpleRefresh,TRUE,
         WA_PubScreen,screen,
         WA_IDCMP,IDCMP_RAWKEY,
         WA_BackFill,&nobackfillhook,
         (synch?TAG_IGNORE:WINDOW_SharedPort),reqport,
         (type==ARTYPE_ABOUT || prefs.program.centerreq)?WINDOW_Position:TAG_IGNORE,WPOS_CENTERSCREEN,
         WINDOW_Layout,toplayout=VLayoutObject,
            LAYOUT_SpaceOuter,TRUE,
            LAYOUT_LeftSpacing,4,
            LAYOUT_RightSpacing,4,
            LAYOUT_BottomSpacing,4,
            LAYOUT_TopSpacing,4,
            LAYOUT_HorizAlignment,LALIGN_CENTER,
            StartMember,gadget,                   /* */
            StartMember,stringrow=HLayoutObject,
            EndMember,
            CHILD_WeightedHeight,0,
            StartMember,buttonrow=HLayoutObject,
               LAYOUT_EvenSize,TRUE,
            EndMember,
         End,
      EndWindow;
      if(ar->winobj)
      {  if(string)
         {  SetAttrs(stringrow,
               StartMember,stringgad=StringObject,
                  GA_ID,1,
                  GA_RelVerify,TRUE,
                  STRINGA_Buffer,string,
                  STRINGA_MaxChars,127,
                  STRINGA_TextVal,string,
               EndMember,
               TAG_END);
         }
         if(ar->url)
         {  SetAttrs(stringrow,
               StartMember,ButtonObject,
                  GA_ID,GID_COPYURLTOCLIP,
                  GA_RelVerify,TRUE,
                  GA_Text,AWEBSTR(MSG_REQUEST_COPYURL),
               EndMember,
               TAG_END);
         }
         for(i=0;buttonlabels[i];i++)
         {  SetAttrs(buttonrow,
               StartMember,button=ButtonObject,
                  GA_Text,buttonlabels[i],
                  GA_ID,(buttonlabels[i+1])?(i+1):0,
                  GA_RelVerify,TRUE,
               EndMember,
               CHILD_WeightedWidth,0,
               CHILD_NominalSize,TRUE,
               TAG_END);
            if(i==0) ar->firstbutton=button;
         }
         if(i<2) SetAttrs(buttonrow,
               LAYOUT_HorizAlignment,LALIGN_CENTER,
               TAG_END);
         ar->window=RA_OpenWindow(ar->winobj);
         if(ar->window && stringgad)
            ActivateLayoutGadget(toplayout,ar->window,NULL, (ULONG) stringgad);
      }
      if(!ar->window)
      {  Freeawebrequest(ar);
         ar=NULL;
      }
      else if(!synch) ADDTAIL(&requests,ar);
   }
   else
   {  if(gadget) DisposeObject(gadget);
      if(buffer) FREE(buffer);
      if(cb)
      {  Closeclasses(cb);
         FREE(cb);
      }
   }
   return ar;
}



static struct Awebrequest *Makeawebrequest(struct Screen *screen,
   struct Image *label,UBYTE **buttonlabels,UBYTE *title,void *buffer,void *buffer2,
   struct Classbases *cb,UWORD type,BOOL synch,requestfunc *reqfun,void *data)
{  return Makeawebrequest2(screen,label,buttonlabels,title,buffer,buffer2,
      cb,type,synch,reqfun,data,NULL,NULL);
}

static struct Awebrequest *Findrequest(UWORD type)
{  struct Awebrequest *ar;
   for(ar=requests.first;ar->next;ar=ar->next)
   {  if(ar->type==type)
      {  WindowToFront(ar->window);
         ActivateWindow(ar->window);
         return ar;
      }
   }
   return NULL;
}

static void Processrequest(void)
{  ULONG result;
   BOOL done;
   short code;
   struct Awebrequest *ar,*nextar;
   requestfunc *reqfun;
   void *data;
   for(ar=requests.first;ar->next;ar=nextar)
   {  nextar=ar->next;
      done=FALSE;
      code=0;
      while((result=RA_HandleInput(ar->winobj,NULL))!=WMHI_LASTMSG)
      {  switch(result&WMHI_CLASSMASK)
         {  case WMHI_CLOSEWINDOW:
               done=TRUE;
               break;
            case WMHI_GADGETUP:
               code=result&WMHI_GADGETMASK;
               if(code==GID_COPYURLTOCLIP)
               {  Clipcopy(ar->url,strlen(ar->url));
               }
               else done=TRUE;
               break;
            case WMHI_RAWKEY:
               if((result&WMHI_GADGETMASK)==0x45) done=TRUE; /* esc */
               break;
         }
      }
      if(done)
      {  reqfun=ar->reqfun;
         data=ar->data;
         REMOVE(ar);
         Freeawebrequest(ar);
         if(reqfun) reqfun(code,data);
      }
   }
}

/* Create a button array (dynamic) */
static UBYTE **Makebuttonarray(UBYTE *labels)
{  short i,n;
   UBYTE *p;
   UBYTE **blab=NULL;
   for(p=labels,n=0;p;n++)
   {  if(p=strchr(p,'|')) *p++='\0';
   }
   if(blab=ALLOCTYPE(UBYTE *,n+1,MEMF_CLEAR))
   {  for(p=labels,i=0;i<n;i++)
      {  blab[i]=p;
         p=p+strlen(p)+1;
      }
   }
   return blab;
}

/*-----------------------------------------------------------------------*/

DECLARE_HOOK
(
        __saveds void  , NoBackFillHook,
        struct Hook *,         hook, A0,
        ULONG, dummy,  A2,
        ULONG, dummy2,  A1
)
{
    USRFUNC_INIT


    USRFUNC_EXIT
}





static struct Gadget *Aboutlabel(UBYTE *text, UBYTE *url, UBYTE *text2, UBYTE *aplurl,struct Screen *screen)
{  struct Gadget *gadget;
   struct ColorMap *cmap=NULL;
   struct Drawinfo *dri=NULL;
   struct Image *image = NULL;
   ULONG imageheight;
   ULONG imagewidth;

/* fixme: change this so that gadget size is compared to screen size */
/* rather than assuming 270 is big enough */


if (screen->Height > 270)
{
    image = PenMapObject,
        IA_Data,imagedata,
        PENMAP_Palette,imagepalette,
        PENMAP_Screen,screen,
    EndImage;
    ourpens  = FALSE;
    imagewidth  = (ULONG)((UWORD *)imagedata)[0];
    imageheight = (ULONG)((UWORD *)imagedata)[1];
}
else
{
   Agetattrs(Aweb(),
      AOAPP_Colormap,(ULONG)&cmap,
      AOAPP_Drawinfo,(ULONG)&dri,
      TAG_END);
   iconpens[0]=ObtainBestPen(cmap,0xffffffff,0xffffffff,0xffffffff,TAG_END);
   iconpens[1]=ObtainBestPen(cmap,0x00000000,0x00000000,0x00000000,TAG_END);
   iconpens[2]=ObtainBestPen(cmap,0xffffffff,0x00000000,0x00000000,TAG_END);
   iconpens[3]=ObtainBestPen(cmap,0x00000000,0x00000000,0xffffffff,TAG_END);
   ourpens=TRUE;
   image=LabelObject,
      LABEL_DrawInfo,dri,
      LABEL_Mapping,iconpens,
      LABEL_Image,&iconimage,
   End;
   imagewidth=(ULONG)iconimage.Width;
   imageheight=(ULONG)iconimage.Height;
}

    gadget=VLayoutObject,
       LAYOUT_VertAlignment,LALIGN_CENTER,
       LAYOUT_SpaceInner, TRUE,
       LAYOUT_InnerSpacing,4,
       LAYOUT_HorizAlignment,LALIGN_CENTER,
           StartMember,HLayoutObject,
              LAYOUT_BevelStyle,BVS_GROUP,
              StartImage,image,
              CHILD_MinWidth,imagewidth,
              CHILD_MinHeight,imageheight,
           EndMember,
           CHILD_WeightedWidth, 0,
       StartImage,LabelObject,
          LABEL_Justification,LJ_CENTER,
          LABEL_Text,text,
       EndImage,
       StartMember, ButtonObject,
           GA_Text,aplurl,
           GA_RelVerify, TRUE,
           GA_ID,GID_APLCLICKED,
           BUTTON_BevelStyle,BVS_NONE,
           BUTTON_TextPen,3,
       EndMember,
       StartImage,LabelObject,
          LABEL_Justification,LJ_CENTER,
          LABEL_Text,text2,
       EndImage,
       StartMember, ButtonObject,
           GA_Text,url,
           GA_RelVerify, TRUE,
           GA_ID,GID_URLCLICKED,
           BUTTON_BevelStyle,BVS_NONE,
           BUTTON_TextPen,3,
       EndMember,

    End;




   return gadget;
}

static struct Image *Quitlabel(void)
{  return LabelObject,
      LABEL_DrawInfo,Agetattr(Aweb(),AOAPP_Drawinfo),
      (Transferring() && !haiku)?LABEL_Text:TAG_IGNORE,AWEBSTR(MSG_QUIT_WARNING),
      LABEL_Text,haiku?HAIKU2:AWEBSTR(MSG_QUIT_TEXT),
      LABEL_Underscore,0,
   End;
}

static struct Image *Unreglabel(void)
{  return LabelObject,
      LABEL_DrawInfo,Agetattr(Aweb(),AOAPP_Drawinfo),
      LABEL_Text,AWEBSTR(MSG_DEMO_TEXT),
      LABEL_Underscore,0,
   End;
}

static void Aboutreqfun(short code,void *data)
{
   int port = (int) data;


   if(code == GID_URLCLICKED)
   {
       Supportarexxcmd(port,"OPEN " WWWADDRESS , NULL,0);

   }
/* temporary define for test should be moved to keyfile.h oe other more sensible place*/
#define APLNAME "AWeb Public License"

   if(code == GID_APLCLICKED)
   {
       Supportarexxcmd(port,"OPEN " APLADDRESS, NULL, 0);
   }

   if(ourpens == TRUE)
   {
   short i;
   struct ColorMap *cmap=(struct ColorMap *)Agetattr(Aweb(),AOAPP_Colormap);
   for(i=0;i<4;i++) ReleasePen(cmap,iconpens[i]);
   }
}

static void Quitreqfun(short code,void *data)
{  if(code==1) Quit(TRUE);
}

/* Wrap text if it is too long. Returns dynamic string. */
static UBYTE *Wraptext(UBYTE *text)
{  struct Buffer buf={0};
   UBYTE *p;
   long line,lastsp;
   line=0;
   lastsp=-1;
   for(p=text;*p;p++)
   {  if(isspace(*p)) lastsp=buf.length;
      Addtobuffer(&buf,p,1);
      if(*p=='\n')
      {  line=buf.length;
         lastsp=-1;
      }
      else if(buf.length-line>70)
      {  if(lastsp>=0)
         {  buf.buffer[lastsp]='\n';
            line=lastsp+1;
            lastsp=-1;
         }
         else
         {  Addtobuffer(&buf,"\n",1);
            line=buf.length;
            lastsp=-1;
         }
      }
   }
   p=Dupstr(buf.buffer,buf.length);
   Freebuffer(&buf);
   return p;
}

/*-----------------------------------------------------------------------*/

BOOL Initrequest(void)
{  NEWLIST(&requests);
   InitSemaphore(&reqsema);
   inited=TRUE;
   if(!(reqport=CreateMsgPort())) return FALSE;
   requestbackfillhook.h_Entry=(HOOKFUNC)GIVEME_HOOKENTRY(Backfillhook);
   requestbackfillhook.h_Data=(void *)2; /* (void *)(Drawinfoptr()->dri_Pens[SHINEPEN]); */
   nobackfillhook.h_Entry=(HOOKFUNC)GIVEME_HOOKENTRY(NoBackFillHook);
   aboutbuttons[0]=AWEBSTR(MSG_ABOUT_OK);
   quitbuttons[0]=AWEBSTR(MSG_QUIT_OK);
   quitbuttons[1]=AWEBSTR(MSG_QUIT_CANCEL);
   unregbuttons[0]=AWEBSTR(MSG_DEMO_OK);
   Setprocessfun(reqport->mp_SigBit,Processrequest);
   return TRUE;
}

void Freerequest(void)
{  if(inited) Closerequests();
   if(reqport)
   {  Setprocessfun(reqport->mp_SigBit,NULL);
      DeleteMsgPort(reqport);
   }
}

void Closerequests(void)
{  struct Awebrequest *ar;
   while(ar=(struct Awebrequest *)REMHEAD(&requests)) Freeawebrequest(ar);
}

void Aboutreq(UBYTE *portname)
{  struct Screen *screen;
   UBYTE *abstr=NULL;
   struct Classbases *cb;
   UBYTE *buf;
   UBYTE *buf2;
   UBYTE portbuf[10];
   UBYTE *portnumstr;
   ULONG portnum;

#ifdef LIMITED
   static UBYTE limited[128]="\0\0DEMO:";
#endif

   /* get the port number of the screen that asked for the about req */

   portnumstr = portname + SplitName(portname,'.',portbuf,0,sizeof(portbuf));
   (void)StrToLong(portnumstr,&portnum);

   Asetattrs(Aweb(),AOAPP_Iconify,FALSE,TAG_END);
   if(!Findrequest(ARTYPE_ABOUT))
   {  if(screen=LockPubScreen((UBYTE *)Agetattr(Aweb(),AOAPP_Screenname)))
      {  if((buf=ALLOCTYPE(UBYTE,512,0)) && (buf2=ALLOCTYPE(UBYTE,512,0)))
         {  strcpy(buf,aboutversion);
            strcat(buf,"\n");
            strcat(buf," \n� 2002 Yvon Rozijn\n");
            strcat(buf,"� 2002-2008 AWeb Development Team\n");
            strcat(buf," \n");
            strcat(buf,"This program is distributed under the\n");
            strcat(buf2," \n");
            strcat(buf2,AWEBSTR(MSG_ABOUT_TRANSLATOR));
            if(prefs.program.screentype==SCRTYPE_OWN)
            {  strcat(buf2,AWEBSTR(MSG_ABOUT_SCREENNAME));
               strcat(buf2,": AWeb\n");
            }
            if(portname)
            {  strcat(buf2,AWEBSTR(MSG_ABOUT_PORTNAME));
               strcat(buf2,": ");
               strcat(buf2,portname);
               strcat(buf2,"\n");
            }
            strcat(buf2," \n");
            strcat(buf2,AWEBSTR(MSG_ABOUT_WWW));
            strcat(buf2,": ");
            if(abstr) FreeVec(abstr);
            if(haiku) strcpy(buf,HAIKU1);
            if(cb=Openclasses())
            {  Makeawebrequest3(screen,Aboutlabel(buf,WWWADDRESS,buf2,APLNAME,screen),
                  aboutbuttons,AWEBSTR(MSG_ABOUT_TITLE),buf,NULL,cb,
                  ARTYPE_ABOUT,FALSE,Aboutreqfun,(void *)portnum,NULL,NULL);
            }
            else
            {  struct EasyStruct es={0};
               es.es_StructSize=sizeof(es);
               es.es_Title=AWEBSTR(MSG_ABOUT_TITLE);
               es.es_TextFormat=buf;
               es.es_GadgetFormat="Ok";
               EasyRequest((struct Window *)Agetattr(Firstwindow(),AOWIN_Window),&es,NULL);
               FREE(buf);
               FREE(buf2);
            }
         }
         UnlockPubScreen(NULL,screen);
      }
   }
}

void Closeabout(void)
{  struct Awebrequest *ar;
   for(ar=requests.first;ar->next;ar=ar->next)
   {  if(ar->type==ARTYPE_ABOUT)
      {  REMOVE(ar);
         Freeawebrequest(ar);
         break;
      }
   }
}

BOOL Isopenabout(void)
{  struct Awebrequest *ar;
   for(ar=requests.first;ar->next;ar=ar->next)
   {  if(ar->type==ARTYPE_ABOUT) return TRUE;
   }
   return FALSE;
}

BOOL Quitreq(void)
{  struct Screen *screen;
   struct Classbases *cb;
   BOOL result=FALSE;
   Asetattrs(Aweb(),AOAPP_Iconify,FALSE,TAG_END);
   if(Findrequest(ARTYPE_QUIT))
   {  result=TRUE;
   }
   else
   {  if(screen=LockPubScreen((UBYTE *)Agetattr(Aweb(),AOAPP_Screenname)))
      {  if(cb=Openclasses())
         {  if(Makeawebrequest(screen,Quitlabel(),
               quitbuttons,AWEBSTR(MSG_REQUEST_TITLE),NULL,NULL,cb,
               ARTYPE_QUIT,FALSE,Quitreqfun,NULL))
               result=TRUE;
         }
         UnlockPubScreen(NULL,screen);
      }
   }
   return result;
}

void Unregreq(void)
{  struct Screen *screen;
   struct Classbases *cb;
   Asetattrs(Aweb(),AOAPP_Iconify,FALSE,TAG_END);
   if(!Findrequest(ARTYPE_UNREG))
   {  if(screen=LockPubScreen((UBYTE *)Agetattr(Aweb(),AOAPP_Screenname)))
      {  if(cb=Openclasses())
         {  Makeawebrequest(screen,Unreglabel(),
               unregbuttons,AWEBSTR(MSG_REQUEST_TITLE),NULL,NULL,cb,
               ARTYPE_UNREG,FALSE,NULL,NULL);
         }
         UnlockPubScreen(NULL,screen);
      }
   }
}

BOOL Asyncrequestcc(UBYTE *title,UBYTE *text,UBYTE *labels,requestfunc *f,
   void *data,UBYTE *url)
{  struct Screen *screen;
   struct Image *txtlab;
   UBYTE *blabels=Dupstr(labels,-1);
   UBYTE *labtext=NULL;
   UBYTE **blab;
   struct Classbases *cb;
   BOOL ok=FALSE;
   Asetattrs(Aweb(),AOAPP_Iconify,FALSE,TAG_END);
   if(blabels && (screen=LockPubScreen((UBYTE *)Agetattr(Aweb(),AOAPP_Screenname)))
   && (blab=Makebuttonarray(blabels)))
   {  if(cb=Openclasses())
      {  labtext=Wraptext(text);
         txtlab=LabelObject,
            LABEL_DrawInfo,Agetattr(Aweb(),AOAPP_Drawinfo),
            LABEL_Text,labtext?labtext:text,
            LABEL_Underscore,0,
         End;
         if(txtlab)
         {  if(Makeawebrequest2(screen,txtlab,blab,title,
               labtext?labtext:text,blabels,cb,0,FALSE,f,data,NULL,url))
            {  ok=TRUE;
               /* labtext will be freed if requester closes, but caller expects
                * us to free text in case of succesful return. */
               if(labtext) FREE(text);
            }
         }
         else
         {  Closeclasses(cb);
            FREE(cb);
            if(labtext) FREE(labtext);
         }
      }
      UnlockPubScreen(NULL,screen);
      FREE(blab);
   }
   if(blabels && !ok) FREE(blabels);
   return ok;
}

BOOL Asyncrequest(UBYTE *title,UBYTE *text,UBYTE *labels,requestfunc *f,void *data)
{  return Asyncrequestcc(title,text,labels,f,data,NULL);
}

BOOL Asyncpromptrequest(UBYTE *title,UBYTE *text,UBYTE *labels,requestfunc *f,
   void *data,UBYTE *string)
{  struct Screen *screen;
   struct Image *txtlab;
   UBYTE *blabels=Dupstr(labels,-1);
   UBYTE *labtext=NULL;
   UBYTE **blab;
   struct Classbases *cb;
   BOOL ok=FALSE;
   Asetattrs(Aweb(),AOAPP_Iconify,FALSE,TAG_END);
   if(blabels && (screen=LockPubScreen((UBYTE *)Agetattr(Aweb(),AOAPP_Screenname)))
   && (blab=Makebuttonarray(blabels)))
   {  if(cb=Openclasses())
      {  labtext=Wraptext(text);
         txtlab=LabelObject,
            LABEL_DrawInfo,Agetattr(Aweb(),AOAPP_Drawinfo),
            LABEL_Text,labtext?labtext:text,
            LABEL_Underscore,0,
         End;
         if(txtlab)
         {  if(Makeawebrequest2(screen,txtlab,blab,title,
               labtext,blabels,cb,0,FALSE,f,data,string,NULL))
            {  ok=TRUE;
            }
         }
         else
         {  Closeclasses(cb);
            FREE(cb);
            if(labtext) FREE(labtext);
         }
      }
      UnlockPubScreen(NULL,screen);
      FREE(blab);
   }
   if(blabels && !ok) FREE(blabels);
   return ok;
}

static long Syncrequesta(UBYTE *title,UBYTE *text,UBYTE *labels,long delay,UBYTE *url)
{  struct Screen *screen=LockPubScreen((UBYTE *)Agetattr(Aweb(),AOAPP_Screenname));
   void *drawinfo=NULL; /* must be done here bcz tcp-end requester */
   UBYTE **blab;
   UBYTE *blabels=Dupstr(labels,-1);
   UBYTE *labtext;
   struct Image *txtlab;
   struct Awebrequest *ar;
   long id=0;
   struct Classbases *cb;
   if(blabels && screen && (drawinfo=GetScreenDrawInfo(screen))
   && (blab=Makebuttonarray(blabels)))
   {  if(cb=Openclasses())
      {  labtext=Wraptext(text);
         txtlab=LabelObject,
            LABEL_DrawInfo,drawinfo,
            LABEL_Text,labtext?labtext:text,
            LABEL_Underscore,0,
         End;
         if(txtlab)
         {  if(ar=Makeawebrequest2(screen,txtlab,blab,title,
               NULL,NULL,cb,0,TRUE,NULL,NULL,NULL,url))
            {  ULONG sigmask=0,getmask;
               ULONG result;
               BOOL done=FALSE;
               GetAttr(WINDOW_SigMask,ar->winobj,&sigmask);
               Busypointer(TRUE);
               if(delay)
               {  Setgadgetattrs(ar->firstbutton,ar->window,NULL,GA_Disabled,TRUE,TAG_END);
                  Delay(delay);
                  /* Empty event queue */
                  while((result=RA_HandleInput(ar->winobj,NULL))!=WMHI_LASTMSG);
                  Setgadgetattrs(ar->firstbutton,ar->window,NULL,GA_Disabled,FALSE,TAG_END);
               }
               while(!done)
               {  getmask=Wait(sigmask|SIGBREAKF_CTRL_C);
                  if(getmask&SIGBREAKF_CTRL_C) break;
                  while((result=RA_HandleInput(ar->winobj,NULL))!=WMHI_LASTMSG)
                  {  switch(result&WMHI_CLASSMASK)
                     {  case WMHI_CLOSEWINDOW:
                           done=TRUE;
                           break;
                        case WMHI_GADGETUP:
                           id=result&WMHI_GADGETMASK;
                           if(id==GID_COPYURLTOCLIP)
                           {  Clipcopy(url,strlen(url));
                           }
                           else done=TRUE;
                           break;
                        case WMHI_RAWKEY:
                           if((result&WMHI_GADGETMASK)==0x45) done=TRUE;
                           break;
                     }
                  }
               }
               Freeawebrequest(ar);
               Busypointer(FALSE);
            }
         }
         else
         {  Closeclasses(cb);
            FREE(cb);
         }
         if(labtext) FREE(labtext);
      }
      FREE(blab);
   }
   if(drawinfo) FreeScreenDrawInfo(screen,drawinfo);
   if(screen) UnlockPubScreen(NULL,screen);
   if(blabels) FREE(blabels);
   return id;
}

long Syncrequest(UBYTE *title,UBYTE *text,UBYTE *labels,long delay)
{  return Syncrequesta(title,text,labels,delay,NULL);
}

long Syncrequestcc(UBYTE *title,UBYTE *text,UBYTE *labels,UBYTE *url)
{  return Syncrequesta(title,text,labels,0,url);
}

UBYTE *Promptrequest(UBYTE *text,UBYTE *defstr)
{  struct Screen *screen=NULL;
   struct DrawInfo *dri=NULL;
   struct Gadget *strgad;
   void *winobj,*toplayout;
   struct Window *win;
   UBYTE *p,*retstr=NULL,*labtext;
   ULONG sigmask,result;
   BOOL done=FALSE;
   Asetattrs(Aweb(),AOAPP_Iconify,FALSE,TAG_END);
   Agetattrs(Aweb(),
      AOAPP_Screen,(ULONG)&screen,
      AOAPP_Drawinfo,(ULONG)&dri,
      TAG_END);
   if(screen && text)
   {  labtext=Wraptext(text);
      winobj=WindowObject,
         WA_Title,AWEBSTR(MSG_REQUEST_TITLE),
         WA_AutoAdjust,TRUE,
         WA_DragBar,TRUE,
         WA_DepthGadget,TRUE,
         WA_Activate,Awebactive(),
         WA_SimpleRefresh,TRUE,
         WA_PubScreen,screen,
         WA_InnerWidth,screen->Width/3,
         WINDOW_Layout,toplayout=VLayoutObject,
            LAYOUT_SpaceOuter,TRUE,
            StartImage,LabelObject,
               LABEL_Text,labtext?labtext:text,
               LABEL_Underscore,0,
            EndImage,
            StartMember,strgad=StringObject,
               STRINGA_TextVal,defstr,
               STRINGA_MaxChars,127,
            EndMember,
            StartMember,HLayoutObject,
               LAYOUT_EvenSize,TRUE,
               StartMember,ButtonObject,
                  GA_ID,1,
                  GA_RelVerify,TRUE,
                  GA_Text,AWEBSTR(MSG_JSPROMPT_OK),
               EndMember,
               CHILD_WeightedWidth,0,
               CHILD_NominalSize,TRUE,
               StartMember,ButtonObject,
                  GA_ID,0,
                  GA_RelVerify,TRUE,
                  GA_Text,AWEBSTR(MSG_JSPROMPT_CANCEL),
               EndMember,
               CHILD_WeightedWidth,0,
               CHILD_NominalSize,TRUE,
            EndMember,
         End,
      EndWindow;
      if(winobj)
      {  win=RA_OpenWindow(winobj);
         GetAttr(WINDOW_SigMask,winobj,&sigmask);
         Busypointer(TRUE);
         ActivateLayoutGadget(toplayout,win,NULL, (ULONG) strgad);
         while(!done)
         {  Wait(sigmask|SIGBREAKF_CTRL_C);
            while((result=RA_HandleInput(winobj,NULL))!=WMHI_LASTMSG)
            {  switch(result&WMHI_CLASSMASK)
               {  case WMHI_CLOSEWINDOW:
                     done=TRUE;
                     break;
                  case WMHI_GADGETUP:
                     done=TRUE;
                     if(result&WMHI_GADGETMASK)
                     {  p=(UBYTE *)Getvalue(strgad,STRINGA_TextVal);
                        retstr=Dupstr(p,-1);
                     }
                     break;
                  case WMHI_RAWKEY:
                     if((result&WMHI_GADGETMASK)==0x45) done=TRUE;
                     break;
               }
            }
         }
         DisposeObject(winobj);
         Busypointer(FALSE);
      }
      if(labtext) FREE(labtext);
   }
   return retstr;
}

void Closeprogressreq(struct Progressreq *pr)
{  if(pr)
   {  if(pr->winobj) DisposeObject(pr->winobj);
      if(pr->label) DisposeObject(pr->label);
      if(pr->cb)
      {  Closeclasses(pr->cb);
         FREE(pr->cb);
      }
      FREE(pr);
   }
}

struct Progressreq *Openprogressreq(UBYTE *labeltext)
{  struct Progressreq *pr;
   ULONG labelw,labelh;
   Asetattrs(Aweb(),AOAPP_Iconify,FALSE,TAG_END);
   if(pr=ALLOCSTRUCT(Progressreq,1,MEMF_CLEAR))
   {  if(pr->cb=Openclasses())
      {  if(pr->label=LabelObject,
               LABEL_DrawInfo,Agetattr(Aweb(),AOAPP_Drawinfo),
               LABEL_Text,labeltext,
               LABEL_Underscore,0,
            End)
         {  GetAttr(IA_Width,pr->label,&labelw);
            GetAttr(IA_Height,pr->label,&labelh);
            pr->winobj=WindowObject,
               WA_Title,AWEBSTR(MSG_PROGRESS_TITLE),
               WA_AutoAdjust,TRUE,
               WA_DragBar,TRUE,
               WA_DepthGadget,TRUE,
               WA_Activate,Awebactive(),
               WA_SimpleRefresh,TRUE,
               WA_PubScreen,Agetattr(Aweb(),AOAPP_Screen),
               WA_BackFill,&requestbackfillhook,
               WINDOW_Position,WPOS_CENTERSCREEN,
               WINDOW_Layout,VLayoutObject,
                  LAYOUT_SpaceOuter,TRUE,
                  LAYOUT_HorizAlignment,LALIGN_CENTER,
                  StartMember,ButtonObject,
                     GA_Image,pr->label,
                     GA_ReadOnly,TRUE,
                  EndMember,
                  CHILD_MinWidth,labelw+32,
                  CHILD_MinHeight,labelh+16,
                  StartMember,pr->fuelgad=FuelGaugeObject,
                     FUELGAUGE_Min,0,
                     FUELGAUGE_Max,100,
                     FUELGAUGE_Level,0,
                     FUELGAUGE_Percent,TRUE,
                     FUELGAUGE_Ticks,11,
                  EndMember,
                  StartMember,ButtonObject,
                     GA_ID,1,
                     GA_RelVerify,TRUE,
                     GA_Text,AWEBSTR(MSG_PROGRESS_CANCEL),
                  EndMember,
                  CHILD_WeightedWidth,0,
               End,
            EndWindow;
            if(pr->winobj)
            {  if(pr->window=(struct Window *)RA_OpenWindow(pr->winobj))
               {  return pr;
               }
            }
         }
      }
      Closeprogressreq(pr);
   }
   return NULL;
}

void Setprogressreq(struct Progressreq *pr,long level,long max)
{  if(pr && pr->window)
   {  Setgadgetattrs(pr->fuelgad,pr->window,NULL,
         FUELGAUGE_Max,max,
         FUELGAUGE_Level,level,
         TAG_END);
   }
}

BOOL Checkprogressreq(struct Progressreq *pr)
{  ULONG result;
   BOOL done=FALSE;
   while((result=RA_HandleInput(pr->winobj,NULL))!=WMHI_LASTMSG)
   {  switch(result&WMHI_CLASSMASK)
      {  case WMHI_CLOSEWINDOW:
            done=TRUE;
            break;
         case WMHI_GADGETUP:
            done=TRUE;
            break;
      }
   }
   return done;
}

void Progresstofront(struct Progressreq *pr)
{  WindowToFront(pr->window);
}

void Demorequest(void)
{
#ifdef NETDEMO
#ifndef OSVERSION
   Syncrequest("AWeb DEMO version",
      "This is the DEMO version of the AWeb-II " FULLRELEASE " browser.\n"
      "It has a time limit of 30 minutes.\n"
      " \n"
      "The full featured version can be obtained directly from\n"
      "AmiTrix Development or from our dealers.\n"
      " \n"
      "AmiTrix can be contacted via e-mail at: sales@amitrix.com\n"
      "and via WWW on: http://www.amitrix.com/\n"
      " (easily accessable by the \"Help/AWeb home page\" menu item).\n"
      " \n"
      "See the documentation for a list of features available\n"
      "in the full version.\n"
      " \n"
      "(This requester will block for 5 seconds)\n",
      "_Ok",
#ifdef DEVELOPER
      0
#else
      250
#endif
      );
#endif
#endif
}
