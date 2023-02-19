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

/* print.c - AWeb print process */

#include "aweb.h"
#include "window.h"
#include "frame.h"
#include "application.h"
#include "printwin.h"
#include "winhis.h"
#include "print.h"
#include "versions.h"
#include <intuition/intuition.h>
#include <devices/printer.h>
#include <devices/prtbase.h>

#if defined(__amigaos4__)
#warning "CHANGED 04/22 -js-" // no cgx for OS4
// use p96
#else
#include <cybergraphx/cybergraphics.h>
#include <clib/alib_protos.h>
#endif

#undef NO_INLINE_STDARG
#include <reaction/reaction.h>
#define NO_INLINE_STDARG

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/graphics.h>

#if defined(__amigaos4__)
#warning "CHANGED 04/22 -js-" // no cgx for OS4
#include <proto/picasso96api.h>
#else
#include <proto/cybergraphics.h>
#endif

#ifndef NETDEMO

/*------------------------------------------------------------------------*/

enum PRINT_PARAM_GADGET_IDS
{  PPGID_PRINT=1,PPGID_CANCEL
};

#define AOPRT_Dummy        AOBJ_DUMMYTAG(AOTP_PRINT)

#define AOPRT_Activate     (AOPRT_Dummy+1)
   /* (BOOL) Activate current print requester */

#define AOPRT_Width        (AOPRT_Dummy+2)
#define AOPRT_Height       (AOPRT_Dummy+3)
   /* (long) Current window dimensions */

#define AOPRT_Scale        (AOPRT_Dummy+4)
   /* (long) Given scale. */

#define AOPRT_Flags        (AOPRT_Dummy+5)
   /* (UWORD) Given flags. Reset scale to 100%. */

#define AOPRT_Wait         (AOPRT_Dummy+6)
   /* (struct Arexxcmd *) ARexx msg to return when finished. */

#define AOPRT_Debug        (AOPRT_Dummy+7)
   /* (BOOL) Debug printer actions */

#define AOPRT_Docwidth     (AOPRT_Dummy+8)
   /* (long) Current document dimensions */

#define AOPRT_    (AOPRT_Dummy+)

static struct Print *print;

/* Initial/remembered values */
static short scale=100;
static UWORD flags=PRTF_FORMFEED|PRTF_BACKGROUND;

struct Library *AwebPrintBase;

#if !defined(__amigaos4__)
struct AwebPrintIFace;
#endif

struct AwebPrintIFace *IAwebPrint;

/*---------------------------------------------------------------------*/

/* Print one strip. Render the document, with 1 pixel row overlap at top and
 * bottom to allow smoothing.
 * Notwithstanding this overlap, prt->top is the top row actually printed (but
 * prt->printheight is the height including overlaps).
 * If this print would be the last one on the page (and not the last one of the
 * document), add a formfeed.
 */
static void Printsection(struct Print *prt)
{
   if(prt->top==0)
   {  /* first strip, leave top row blank. AOPRW_Cleartop was already set. */
      Asetattrs(prt->prwin,AOBJ_Top,prt->top,TAG_END);
   }
   else
   {  /* render with 1 row overlap */
      Asetattrs(prt->prwin,
         AOPRW_Cleartop,FALSE,
         AOBJ_Top,prt->top-1,
         TAG_END);
   }
   Printprintsection(prt);
   prt->flags|=PRTF_PRINTING;
   prt->top+=prt->printheight-2;
   Setprogressreq(prt->progressreq,prt->top,prt->totalheight);
}

/* Compute acceptable rastport height. Loop (increasing or decreasing scale)
 * until a height is actually found */
static BOOL Finddimensions(struct Print *prt)
{  Printfinddimensions(prt);
   return BOOLVAL(prt->printheight);
}

/* If this is in the 3rd longword of pd_OldStk, it's a turboprint device. If
 * device version >=39 it can handle >8 bit bitmaps.
 * Copied from turboprint.h: */
#define TPMATCHWORD  0xf10a57ef

/* Create offscreen bitmap, layer. Create mimic windowinfo. Measure and layout doc.
 * Print first section. */
static void Startprintdoc(struct Print *prt)
{  struct Screen *screen=(struct Screen *)Agetattr(Aweb(),AOAPP_Screen);
   struct PrinterData *pd=(struct PrinterData *)prt->ioreq->io_Device;
   BOOL Candodeeprp = ((((ULONG *)(pd->pd_OldStk))[2]==TPMATCHWORD) && (pd->pd_Device.dd_Device.lib_Version>=39))
                       || (pd->pd_Device.dd_Device.lib_Version>=44);

   pd->pd_Preferences.PrintAspect=ASPECT_HORIZ;
   if(!(prt->progressreq=Openprogressreq(AWEBSTR(MSG_PRINT_PROGRESS)))) goto err;
   if(!(prt->prwin=Anewobject(AOTP_PRINTWINDOW,
      AOPRW_Width,prt->printwidth,
      AOPRW_Height,PRINTWINH,
      AOPRW_Layoutheight,prt->height,
      AOBJ_Nobackground,!(prt->flags&PRTF_BACKGROUND),
      AOPRW_Turboprint,Candodeeprp,
      TAG_END))) goto err;
   prt->rp=(struct RastPort *)Agetattr(prt->prwin,AOWIN_Rastport);
   prt->cmap=screen->ViewPort.ColorMap;
   prt->screenmode=GetVPModeID(&screen->ViewPort);

#if defined(__amigaos4__)
#warning "CGX CODE!"
   if (!p96GetModeIDAttr( prt->screenmode, P96IDA_ISP96 ))
#else
   if(!CyberGfxBase || !IsCyberModeID(prt->screenmode))
#endif
   {
        /* Get aspect ratio right for old hardware 1:2 screens */
        prt->screenmode=HIRES|LACE;
   }

   if(!Finddimensions(prt)) goto err;
   Asetattrs(prt->prwin,
      AOPRW_Height,prt->printheight,
      AOBJ_Winhis,(Tag)prt->whis,
      AOPRW_Cleartop,TRUE,
      TAG_END);
   Asetattrs(prt->prwin,AOPRW_Update,TRUE,TAG_END);
   prt->totalheight=Agetattr(prt->prwin,AOPRW_Totalheight);
   prt->numprinted=0;
   Printsection(prt);
   return;
err:
   Adisposeobject((struct Aobject *)prt);
}

/* Process print i/o request returning */
static void Processprint(void)
{  if(print && (print->flags&PRTF_PRINTING) && CheckIO((struct IORequest *)print->ioreq))
   {  WaitIO((struct IORequest *)print->ioreq);
      if(Checkprogressreq(print->progressreq)
      || print->top>=print->totalheight
      || print->ioreq->io_Error)
      {
          Adisposeobject((struct Aobject *)print);
      }
      else
      {  Printsection(print);
      }
   }
}

/*---------------------------------------------------------------------*/

/* Close the parameter requester */
static void Closeparam(struct Print *prt)
{  struct Node *node;
   if(prt->winobj)
   {  DisposeObject(prt->winobj);
      prt->winobj=NULL;
      prt->window=NULL;
   }
   while(node=RemHead(&prt->layoutlist)) FreeChooserNode(node);
}

/* Process input from parameter requester */
static void Processparam(void)
{  ULONG result;
   BOOL done=FALSE,doprint=FALSE;
   int sel;
   if(print && print->winobj)
   {  while((result=RA_HandleInput(print->winobj,NULL))!=WMHI_LASTMSG)
      {  switch(result&WMHI_CLASSMASK)
         {  case WMHI_CLOSEWINDOW:
               done=TRUE;
               break;
            case WMHI_RAWKEY:
               switch(result&WMHI_GADGETMASK)
               {  case 0x45:     /* esc */
                     done=TRUE;
                     break;
               }
               break;
            case WMHI_GADGETUP:
               switch(result&WMHI_GADGETMASK)
               {  case PPGID_PRINT:
                     doprint=TRUE;
                     break;
                  case PPGID_CANCEL:
                     done=TRUE;
                     break;
               }
               break;
         }
      }
      if(done) Adisposeobject((struct Aobject *)print);
      else if(doprint)
      {  sel=Getvalue(print->layoutgad,CHOOSER_Active);
         switch(sel)
         {  case 0:  print->printwidth=print->width;break;
            case 1:  print->printwidth=print->docwidth;break;
            case 2:  print->printwidth=print->scrwidth;break;
         }
         SETFLAG(print->flags,PRTF_BACKGROUND,Getselected(print->bggad));
         SETFLAG(print->flags,PRTF_CENTER,Getselected(print->centergad));
         SETFLAG(print->flags,PRTF_FORMFEED,Getselected(print->ffgad));
         print->scale=Getvalue(print->scalegad,INTEGER_Number);
         Closeparam(print);
         scale=print->scale;
         flags=print->flags;
         Startprintdoc(print);
      }
   }
}

/* Open print parameter requester */
static BOOL Openparam(struct Print *prt)
{  struct PrinterData *pd;
   struct Screen *screen;
   struct MsgPort *port;
   static ULONG msgs[]={ MSG_PRINTP_LW_WINDOW,MSG_PRINTP_LW_DOCUMENT,MSG_PRINTP_LW_SCREEN };
   struct Node *node;
   int i;
   pd=(struct PrinterData *)prt->ioreq->io_Device;
   Agetattrs(Aweb(),
      AOAPP_Screen,(Tag)&screen,
      AOAPP_Reactionport,(Tag)&port,
      AOAPP_Screenwidth,(Tag)&prt->scrwidth,
      TAG_END);
   if(screen)
   {  Asetattrs(Aweb(),
         AOAPP_Processtype,AOTP_PRINT,
         AOAPP_Processfun,(Tag)Processparam,
         TAG_END);
      for(i=0;i<3;i++)
      {  if(node=AllocChooserNode(
            CNA_Text,AWEBSTR(msgs[i]),
            TAG_END))
            AddTail(&prt->layoutlist,node);
      }
      prt->winobj=WindowObject,
         WA_Title,AWEBSTR(MSG_PRINTP_TITLE),
         WA_DragBar,TRUE,
         WA_DepthGadget,TRUE,
         WA_CloseGadget,TRUE,
         WA_Activate,TRUE,
         WA_SimpleRefresh,TRUE,
         WA_PubScreen,screen,
         WINDOW_SharedPort,port,
         WINDOW_UserData,prt,
         WINDOW_Position,WPOS_CENTERSCREEN,
         WINDOW_Layout,VLayoutObject,
            LAYOUT_SpaceInner,FALSE,
            LAYOUT_DeferLayout,TRUE,
            StartMember,VLayoutObject,
               LAYOUT_SpaceOuter,TRUE,
               StartMember,VLayoutObject,
                  StartMember,prt->layoutgad=ChooserObject,
                     CHOOSER_PopUp,TRUE,
                     CHOOSER_Labels,&prt->layoutlist,
                     CHOOSER_Active,(prt->width<prt->docwidth)?1:0,
                  EndMember,
                  MemberLabel(AWEBSTR(MSG_PRINTP_LAYOUTWIDTH)),
                  StartMember,prt->scalegad=IntegerObject,
                     INTEGER_Minimum,1,
                     INTEGER_Maximum,100,
                     INTEGER_Number,prt->scale,
                  EndMember,
                  MemberLabel(AWEBSTR(MSG_PRINTP_SCALE)),
               EndMember,
               StartMember,prt->centergad=CheckBoxObject,
                  GA_Text,AWEBSTR(MSG_PRINTP_CENTER),
                  GA_Selected,BOOLVAL(prt->flags&PRTF_CENTER),
               EndMember,
               StartMember,prt->ffgad=CheckBoxObject,
                  GA_Text,AWEBSTR(MSG_PRINTP_FORMFEED),
                  GA_Selected,BOOLVAL(prt->flags&PRTF_FORMFEED),
               EndMember,
               StartMember,prt->bggad=CheckBoxObject,
                  GA_Text,AWEBSTR(MSG_PRINTP_PRINTBG),
                  GA_Selected,BOOLVAL(prt->flags&PRTF_BACKGROUND),
               EndMember,
            EndMember,
            StartMember,HLayoutObject,
               LAYOUT_BevelStyle,BVS_SBAR_VERT,
               LAYOUT_SpaceOuter,TRUE,
               LAYOUT_EvenSize,TRUE,
               StartMember,ButtonObject,
                  GA_ID,PPGID_PRINT,
                  GA_RelVerify,TRUE,
                  GA_Text,AWEBSTR(MSG_PRINTP_PRINT),
               EndMember,
               CHILD_WeightedWidth,0,
               StartMember,ButtonObject,
                  GA_ID,PPGID_CANCEL,
                  GA_RelVerify,TRUE,
                  GA_Text,AWEBSTR(MSG_PRINTP_CANCEL),
               EndMember,
               CHILD_WeightedWidth,0,
            EndMember,
         EndMember,
      End;
      if(prt->winobj)
      {  prt->window=RA_OpenWindow(prt->winobj);
      }
   }
   return BOOLVAL(prt->window);
}

/*---------------------------------------------------------------------*/

static long Setprint(struct Print *prt,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOAPP_Screenvalid:
            if(!tag->ti_Data)
            {  Adisposeobject((struct Aobject *)prt);
               return 0;
            }
            break;
         case AOPRT_Activate:
            if(prt->window)
            {  WindowToFront(prt->window);
               ActivateWindow(prt->window);
            }
            if(prt->progressreq)
            {  Progresstofront(prt->progressreq);
            }
            break;
         case AOPRT_Width:
            prt->width=tag->ti_Data;
            break;
         case AOPRT_Height:
            prt->height=tag->ti_Data;
            break;
         case AOPRT_Docwidth:
            prt->docwidth=tag->ti_Data;
            break;
         case AOPRT_Flags:
            prt->flags=tag->ti_Data|PRTF_NOOPTIONS;
            prt->scale=100;
            break;
         case AOPRT_Scale:
            if(tag->ti_Data>0 && tag->ti_Data<=100)
            {  prt->scale=tag->ti_Data;
            }
            break;
         case AOPRT_Wait:
            prt->wait=(struct Arexxcmd *)tag->ti_Data;
            break;
         case AOPRT_Debug:
            SETFLAG(prt->flags,PRTF_DEBUG,tag->ti_Data);
            break;
         case AOBJ_Winhis:
            if(tag->ti_Data)
            {  prt->whis=Anewobject(AOTP_WINHIS,
                  AOWHS_Copyfrom,tag->ti_Data,
                  AOWHS_Key,0,
                  AOWHS_History,TRUE,
                  TAG_END);
            }
            break;
      }
   }
   return 0;
}

static void Disposeprint(struct Print *prt)
{
   Closeparam(prt);
   if(prt->debugfile) Printclosedebug(prt);
   if(prt->prwin) Adisposeobject(prt->prwin);
   if(prt->whis) Adisposeobject(prt->whis);
   if(prt->progressreq) Closeprogressreq(prt->progressreq);
   if(prt->ioreq)
   {  if(prt->flags&PRTF_PRINTING)
      {  if(!CheckIO((struct IORequest *)prt->ioreq)) AbortIO((struct IORequest *)prt->ioreq);
         WaitIO((struct IORequest *)prt->ioreq);
      }
      if(prt->ioreq->io_Device) CloseDevice((struct IORequest *)prt->ioreq);
      DeleteExtIO((struct IORequest *)prt->ioreq);
   }
   if(prt->ioport)
   {  Setprocessfun(prt->ioport->mp_SigBit,NULL);
      DeleteMsgPort(prt->ioport);
   }
   Aremchild(Aweb(),(struct Aobject *)prt,AOREL_APP_USE_SCREEN);
   if(prt->wait) Replyarexxcmd(prt->wait);
   Amethodas(AOTP_OBJECT,prt,AOM_DISPOSE);
   Closelib(&AwebPrintBase,(struct Interface **)&IAwebPrint);
   if(prt==print) print=NULL;
}

static struct Print *Newprint(struct Amset *ams)
{  struct Print *prt=NULL;
   if(Openlibnofail(AWEBLIBPATH PRINT_AWEBLIB,PRINT_VERSION,&AwebPrintBase,(struct Interface **)&IAwebPrint))
   {  if(prt=Allocobject(AOTP_PRINT,sizeof(struct Print),ams))
      {  NewList(&prt->layoutlist);
         prt->scale=scale;
         prt->flags=flags;
         Setprint(prt,ams);
         Aaddchild(Aweb(),(struct Aobject *)prt,AOREL_APP_USE_SCREEN);
         if(!(prt->ioport=CreateMsgPort())) goto err;
         Setprocessfun(prt->ioport->mp_SigBit,Processprint);
         if(!(prt->ioreq=(struct IODRPReq *)CreateExtIO(prt->ioport,sizeof(struct IODRPReq)))) goto err;
         if(OpenDevice("printer.device",0,(struct IORequest *)prt->ioreq,0))
         {  UBYTE *buf=Dupstr(AWEBSTR(MSG_PRINT_NOPRINTER),-1);
            if(buf) Asyncrequest(AWEBSTR(MSG_REQUEST_TITLE),buf,AWEBSTR(MSG_PRINT_OK),NULL,NULL);
            goto err;
         }
         Printdebugprefs(prt);
         if((prt->flags&PRTF_NOOPTIONS) && Agetattr(Aweb(),AOAPP_Screenvalid))
         {  Startprintdoc(prt);
         }
         else if(!Openparam(prt)) goto err;
      }
      else Closelib(&AwebPrintBase,(struct Interface **)&IAwebPrint);
   }
   return prt;

err:
   prt->wait=NULL;
   Disposeprint(prt);
   return NULL;
}

USRFUNC_H2
(
static long  , Print_Dispatcher,
struct Print *,prt,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newprint((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setprint(prt,(struct Amset *)amsg);
         break;
      case AOM_DISPOSE:
         Disposeprint(prt);
         break;
      case AOM_DEINSTALL:
         break;
   }
   return result;

    USRFUNC_EXIT
}

#endif /* !NETDEMO */

/*------------------------------------------------------------------------*/

BOOL Installprint(void)
{
#ifndef NETDEMO
   if(!Amethod(NULL,AOM_INSTALL,AOTP_PRINT,(Tag)Print_Dispatcher)) return FALSE;
#endif
   return TRUE;
}

void Printdoc(void *win,BOOL debug)
{
#ifndef NETDEMO
   long w=0,h=0,dw=0;
   void *whis=NULL,*frame=NULL;
   if(print)
   {  Asetattrs((struct Aobject *)print,AOPRT_Activate,TRUE,TAG_END);
   }
   else
   {  Agetattrs(win,
         AOWIN_Innerwidth,(Tag)&w,
         AOWIN_Innerheight,(Tag)&h,
         AOBJ_Winhis,(Tag)&whis,
         AOBJ_Frame,(Tag)&frame,
         TAG_END);
      if(frame)
      {  dw=Agetattr(frame,AOFRM_Contentwidth);
      }
      print=Anewobject(AOTP_PRINT,
         AOPRT_Width,w,
         AOPRT_Height,h,
         AOPRT_Docwidth,dw,
         AOBJ_Winhis,(Tag)whis,
         AOPRT_Debug,debug,
         TAG_END);
   }
#endif
}

BOOL Printarexx(void *win,long scale,BOOL center,BOOL ff,BOOL bg,struct Arexxcmd *wait,
   BOOL debug)
{  long w=0,h=0;
   void *whis=NULL;
   UWORD flags=0;
   BOOL result=FALSE;
#ifndef NETDEMO
   if(print)
   {  Asetattrs((struct Aobject *)print,AOPRT_Activate,TRUE,TAG_END);
   }
   else
   {  if(center) flags|=PRTF_CENTER;
      if(ff) flags|=PRTF_FORMFEED;
      if(bg) flags|=PRTF_BACKGROUND;
      if(debug) flags|=PRTF_DEBUG;
      Agetattrs(win,
         AOWIN_Innerwidth,(Tag)&w,
         AOWIN_Innerheight,(Tag)&h,
         AOBJ_Winhis,(Tag)&whis,
         TAG_END);
      print=Anewobject(AOTP_PRINT,
         AOPRT_Width,w,
         AOPRT_Height,h,
         AOBJ_Winhis,(Tag)whis,
         AOPRT_Flags,flags,
         scale?AOPRT_Scale:TAG_IGNORE,scale,
         AOPRT_Wait,(Tag)wait,
         TAG_END);
      if(print) result=TRUE;
   }
#endif
   return result;
}
