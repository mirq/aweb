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

/* frame.c - AWeb frame system */

#include "aweb.h"
#include "scroller.h"
#include "frame.h"
#include "winhis.h"
#include "url.h"
#include "copy.h"
#include "copydriver.h"
#include "source.h"
#include "window.h"
#include "application.h"
#include "popup.h"
#include "info.h"
#include "search.h"
#include "timer.h"
#include "frprivate.h"
#include "body.h"   /* added */
#include "table.h"  /* added */
#include "jslib.h"  /* added */

#include <reaction/reaction.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/graphics.h>
#include <proto/layers.h>


#if defined(__amigaos4__)
#include <graphics/blitattr.h>
#endif

/*-----------------------------------------------------------------------*/

static LIST(Framename) framenames;

struct Backfillinfo
{  struct Window *window;
   struct RastPort *rp;
   struct Frame *frame;
   struct Coords *coo;
   struct BitMap *bitmap;
   ULONG aox;              /* object x coord */
   ULONG aoy;              /* object y coord */
   UBYTE *mask;            /* transparent mask or NULL */
   ULONG alpha;             /* has an alpha channel */
   short bmw,bmh;          /* bitmap dimensions */
   short bgpen;
   short left;
   short top;
};

static struct Hook backfillhook;

static struct TagItem maphscroll[]={{ AOSCR_Top,AOFRM_Leftpos},{TAG_END }};
static struct TagItem mapvscroll[]={{ AOSCR_Top,AOFRM_Toppos },{TAG_END }};

#define POPUPAREA 16

static ULONG mapclientpull[]={ AOTIM_Ready,AOFRM_Timercpready,TAG_END };

/*-----------------------------------------------------------------------*/

/* Tile a portion of the window with the background image.
 * Coordinates are window rastport coordinates. */
static void Drawbackground(struct RastPort *rp,struct Backfillinfo *bf,
   short x1,short y1,short x2,short y2)
{  short ww,wh;   /* dimensions of window section to fill */
   short bx1,by1; /* blit start coordinates for first tile part */
   short bw1,bh1; /* blit dimensions for first tile part */
   short bw2,bh2; /* blit dimensions for second parts */
   short pos;     /* exponential blit destination position */
   short size;    /* exponential blit destination size */
   /* adjust rectangle bounds to fit in window */
   if(x1<bf->coo->minx) x1=bf->coo->minx;
   if(x2>bf->coo->maxx) x2=bf->coo->maxx;
   if(y1<bf->coo->miny) y1=bf->coo->miny;
   if(y2>bf->coo->maxy) y2=bf->coo->maxy;
   ww=x2-x1+1;
   wh=y2-y1+1;
   /* compute partial tile dimensions to build first tile-sized image */

   bx1=(x1-bf->coo->dx - bf->aox) % bf->bmw;
   if(bx1<0) bx1+=bf->bmw;
   bw1=MIN(bf->bmw-bx1,ww);

   by1=(y1-bf->coo->dy - bf->aoy) % bf->bmh;
   if(by1<0) by1+=bf->bmh;
   bh1=MIN(bf->bmh-by1,wh);

   /* for second part: offset2=0 and width2==offset1 but only if room allows */
   bw2=MIN(bx1,ww-bw1);
   bh2=MIN(by1,wh-bh1);
   /* convert window coordinates to absolute bitmap coordinates: */

    x1+=bf->left;
    x2+=bf->left;
    y1+=bf->top;
    y2+=bf->top;

/*
   if(bf->window)
   {  x1+=bf->window->LeftEdge;
      x2+=bf->window->LeftEdge;
      y1+=bf->window->TopEdge;
      y2+=bf->window->TopEdge;
   }
*/

   /* Wait until blitter is free */
   WaitBlit();
   /* Construct the first tile-sized image at (wx,wy) */
   /* If it if transparent, fill the first image background and do transparent blits */
#if defined(__amigaos4__)
/* Amigaos4 alpha channel support */
#warning "this code included"
   if(bf->alpha)
   {
      SetAPen(rp,bf->bgpen);
      RectFill(rp,x1,y1,MIN(x2,x1+bf->bmw-1),MIN(y2,y1+bf->bmh-1));
            BltBitMapTags(
                          BLITA_DestType,BLITT_RASTPORT,
                          BLITA_Dest,rp,
                          BLITA_DestX,x1,
                          BLITA_DestY,y1,
                          BLITA_Width,bw1,
                          BLITA_Height,bh1,
                          BLITA_SrcType,BLITT_BITMAP,
                          BLITA_Source,bf->bitmap,
                          BLITA_SrcX,bx1,
                          BLITA_SrcY,by1,
                          BLITA_UseSrcAlpha,TRUE,
                          TAG_DONE);

      if(bx1 && ww>bw1) /* first blit was partial width and room for second part */
      {
            BltBitMapTags(
                          BLITA_DestType,BLITT_RASTPORT,
                          BLITA_Dest,rp,
                          BLITA_DestX,x1+bw1,
                          BLITA_DestY,y1,
                          BLITA_Width,bw2,
                          BLITA_Height,bh1,
                          BLITA_SrcType,BLITT_BITMAP,
                          BLITA_Source,bf->bitmap,
                          BLITA_SrcX,0,
                          BLITA_SrcY,by1,
                          BLITA_UseSrcAlpha,TRUE,
                          TAG_DONE);

      }
      if(by1 && wh>bh1) /* first blits were partial height and room for second part */
      {
            BltBitMapTags(
                          BLITA_DestType,BLITT_RASTPORT,
                          BLITA_Dest,rp,
                          BLITA_DestX,x1,
                          BLITA_DestY,y1+bh1,
                          BLITA_Width,bw1,
                          BLITA_Height,bh2,
                          BLITA_SrcType,BLITT_BITMAP,
                          BLITA_Source,bf->bitmap,
                          BLITA_SrcX,bx1,
                          BLITA_SrcY,0,
                          BLITA_UseSrcAlpha,TRUE,
                          TAG_DONE);

         if(bx1 && ww>bw1)
         {
            BltBitMapTags(
                          BLITA_DestType,BLITT_RASTPORT,
                          BLITA_Dest,rp,
                          BLITA_DestX,x1+bw1,
                          BLITA_DestY,y1+bh1,
                          BLITA_Width,bw2,
                          BLITA_Height,bh2,
                          BLITA_SrcType,BLITT_BITMAP,
                          BLITA_Source,bf->bitmap,
                          BLITA_SrcX,0,
                          BLITA_SrcY,0,
                          BLITA_UseSrcAlpha,TRUE,
                          TAG_DONE);

         }
      }

   }
   else
#elif defined(__MORPHOS__)
   if(bf->alpha)
   {
      STATIC CONST struct TagItem tags[] = { { BLTBMA_USESOURCEALPHA, TRUE }, { TAG_DONE, 0 } };
      SetAPen(rp,bf->bgpen);
      RectFill(rp,x1,y1,MIN(x2,x1+bf->bmw-1),MIN(y2,y1+bf->bmh-1));
      BltBitMapRastPortAlpha(bf->bitmap,bx1,by1,rp,x1,y1,bw1,bh1,&tags);
      if(bx1 && ww>bw1) /* first blit was partial width and room for second part */
      {  BltBitMapRastPortAlpha(bf->bitmap,0,by1,rp,x1+bw1,y1,bw2,bh1,&tags);
      }
      if(by1 && wh>bh1) /* first blits were partial height and room for second part */
      {  BltBitMapRastPortAlpha(bf->bitmap,bx1,0,rp,x1,y1+bh1,bw1,bh2,&tags);
         if(bx1 && ww>bw1)
         {  BltBitMapRastPortAlpha(bf->bitmap,0,0,rp,x1+bw1,y1+bh1,bw2,bh2,&tags);
         }
      }
   }
   else
#endif
   if(bf->mask)
   {
      SetAPen(rp,bf->bgpen);
      RectFill(rp,x1,y1,MIN(x2,x1+bf->bmw-1),MIN(y2,y1+bf->bmh-1));
      BltMaskBitMapRastPort(bf->bitmap,bx1,by1,rp,x1,y1,bw1,bh1,0xe0,bf->mask);
      if(bx1 && ww>bw1) /* first blit was partial width and room for second part */
      {  BltMaskBitMapRastPort(bf->bitmap,0,by1,rp,x1+bw1,y1,bw2,bh1,0xe0,bf->mask);
      }
      if(by1 && wh>bh1) /* first blits were partial height and room for second part */
      {  BltMaskBitMapRastPort(bf->bitmap,bx1,0,rp,x1,y1+bh1,bw1,bh2,0xe0,bf->mask);
         if(bx1 && ww>bw1)
         {  BltMaskBitMapRastPort(bf->bitmap,0,0,rp,x1+bw1,y1+bh1,bw2,bh2,0xe0,bf->mask);
         }
      }
   }
   else
   {  BltBitMap(bf->bitmap,bx1,by1,rp->BitMap,x1,y1,bw1,bh1,0xc0,-1,NULL);
      if(bx1 && ww>bw1) /* first blit was partial width and room for second part */
      {  BltBitMap(bf->bitmap,0,by1,rp->BitMap,x1+bw1,y1,bw2,bh1,0xc0,-1,NULL);
      }
      if(by1 && wh>bh1) /* first blits were partial height and room for second part */
      {  BltBitMap(bf->bitmap,bx1,0,rp->BitMap,x1,y1+bh1,bw1,bh2,0xc0,-1,NULL);
         if(bx1 && ww>bw1)
         {  BltBitMap(bf->bitmap,0,0,rp->BitMap,x1+bw1,y1+bh1,bw2,bh2,0xc0,-1,NULL);
         }
      }
   }
   /* Copy the first image exponentially to create the first row */
   pos=x1+bf->bmw;
   size=MIN(bf->bmw,x2-pos+1);
   while(pos<=x2)
   {  BltBitMap(rp->BitMap,x1,y1,rp->BitMap,pos,y1,size,MIN(bf->bmh,wh),0xc0,-1,NULL);
      pos+=size;
      size=MIN(size<<1,x2-pos+1);
   }
   /* Copy the first row exponentially over the rest of the area */
   pos=y1+bf->bmh;
   size=MIN(bf->bmh,y2-pos+1);
   while(pos<=y2)
   {  BltBitMap(rp->BitMap,x1,y1,rp->BitMap,x1,pos,ww,size,0xc0,-1,NULL);
      pos+=size;
      size=MIN(size<<1,y2-pos+1);
   }
   WaitBlit();
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

   struct RastPort rp=*lrp;
   struct Backfillinfo *bf=(struct Backfillinfo *)hook->h_Data;
   short left = msg->rect.MinX - msg->xoffset;
   short top  = msg->rect.MinY - msg->yoffset;
   bf->left = left;
   bf->top  = top;

   rp.Layer=NULL;
   if(bf->bitmap)
   {  Drawbackground(&rp,bf,
         msg->rect.MinX-left,msg->rect.MinY-top,msg->rect.MaxX-left,msg->rect.MaxY-top);
   }
   else
   {  SetAPen(&rp,bf->bgpen);
      RectFill(&rp,msg->rect.MinX,msg->rect.MinY,msg->rect.MaxX,msg->rect.MaxY);
   }

   USRFUNC_EXIT
}

static void Installbg(struct Backfillinfo *bf)
{
   //struct Layer *layer=(bf->window)?bf->window->WLayer:bf->rp->Layer;
   struct Layer *layer=bf->rp->Layer;
   backfillhook.h_Entry=(HOOKFUNC)GIVEME_HOOKENTRY(Backfillhook);
   backfillhook.h_Data=bf;
   InstallLayerHook(layer,&backfillhook);
}

static void Uninstallbg(struct Backfillinfo *bf)
{
   //struct Layer *layer=(bf->window)?bf->window->WLayer:bf->rp->Layer;
   struct Layer *layer=bf->rp->Layer;
   InstallLayerHook(layer,LAYERS_BACKFILL);
}

/*-----------------------------------------------------------------------*/

static void Rendercontents(struct Frame *fr,struct Coords *coo,
   long minx,long miny,long maxx,long maxy,UWORD flags)
{  ULONG clipkey;
   struct Coords coords={0};
   if(!coo)
   {  Framecoords(fr,&coords);
      coo=&coords;
   }
/*
printf("Renderconts %08x scroll=(%d,%d)\n",fr,fr->left,fr->top);
printf("            rdr=(%d,%d)-(%d,%d) coo:d=(%d,%d) clip=(%d,%d)-(%d,%d)\n",
minx,miny,maxx,maxy,coo->dx,coo->dy,coo->minx,coo->miny,coo->maxx,coo->maxy);
*/
   if(coo->win && miny<fr->top+fr->h && maxy>=fr->top
   && !Agetattr(coo->win,AOWIN_Resized) && fr->copy)
   {  if(!fr->frame) flags|=AMRF_DISPTITLE;
      clipkey=Clipto(coo->rp,coo->minx,coo->miny,coo->maxx,coo->maxy);
      Arender(fr->copy,coo,minx,miny,maxx,maxy,flags,NULL);
      Unclipto(clipkey);
   }
}

/* Draw a resize rubberband */
static void Rendersize(struct Frame *fr,struct Coords *coo)
{  coo=Clipcoords(fr->frame,coo);
   if(coo && coo->rp)
   {  SetDrMd(coo->rp,COMPLEMENT);
      Move(coo->rp,fr->sizex+coo->dx,fr->sizey+coo->dy);
      Draw(coo->rp,fr->sizex+fr->sizew-1+coo->dx,fr->sizey+coo->dy);
      Draw(coo->rp,fr->sizex+fr->sizew-1+coo->dx,fr->sizey+fr->sizeh-1+coo->dy);
      Draw(coo->rp,fr->sizex+coo->dx,fr->sizey+fr->sizeh-1+coo->dy);
      Draw(coo->rp,fr->sizex+coo->dx,fr->sizey+coo->dy);
      SetDrMd(coo->rp,JAM1);
   }
   Unclipcoords(coo);
}

/* Update the scrollers */
static void Setscrollers(struct Frame *fr,BOOL moved)
{  long total=fr->doch,visible=fr->h;
   if(visible>prefs.program.overlap)
   {  total=MAX(0,total-prefs.program.overlap);
      visible-=prefs.program.overlap;
   }
   if(fr->flags&FRMF_TOPFRAME)
   {  Asetattrs(fr->win,
         AOWIN_Vslidertotal,total,
         AOWIN_Vslidervisible,visible,
         moved?AOWIN_Vslidertop:TAG_IGNORE,fr->top,
         AOWIN_Hslidertotal,fr->docw,
         AOWIN_Hslidervisible,fr->w,
         moved?AOWIN_Hslidertop:TAG_IGNORE,fr->left,
         TAG_END);
   }
   else
   {  if(fr->vscroll && (fr->flags&FRMF_VSCROLL))
      {  Asetattrs(fr->vscroll,
            AOSCR_Total,total,
            AOSCR_Visible,visible,
            moved?AOSCR_Top:TAG_IGNORE,fr->top,
            AOSCR_Update,fr->elt.eltflags&ELTF_ALIGNED,
            TAG_END);
      }
      if(fr->hscroll && (fr->flags&FRMF_HSCROLL))
      {  Asetattrs(fr->hscroll,
            AOSCR_Total,fr->docw,
            AOSCR_Visible,fr->w,
            moved?AOSCR_Top:TAG_IGNORE,fr->left,
            AOSCR_Update,fr->elt.eltflags&ELTF_ALIGNED,
            TAG_END);
      }
   }
}

/* Find out if this url is in our hierarchy */
static BOOL Nestedurl(struct Frame *fr,void *url)
{  void *frurl=NULL;
   while(fr->frame)
   {  fr=fr->frame;
      Agetattrs(fr->whis,
         AOWHS_Frameid,(Tag)fr->id,
         AOWHS_Url,(Tag)&frurl,
         TAG_END);
      if(frurl==url) return TRUE;
   }
   return FALSE;
}

/*-----------------------------------------------------------------------*/

/* Set coords pen numbers. */
#define PICKCOLOR(c1,c2) ((prefs.browser.docolors && (c1)>=0)?(c1):(c2))

static void Setcolors(struct Frame *fr,struct Coords *coo)
{  long bg=1,text=1,link=1,vlink=1,alink=1;
   if(fr->flags&FRMF_NOBACKGROUND)
   {  Agetattrs(Aweb(),
         AOAPP_Whitepen,(Tag)&bg,
         AOAPP_Blackpen,(Tag)&text,
         TAG_END);
      coo->bgcolor=bg;
      coo->textcolor=coo->linkcolor=coo->vlinkcolor=coo->alinkcolor=text;
      coo->bgimage=NULL;
      coo->bgalign=NULL;
   }
   else
   {  Agetattrs(Aweb(),
         AOAPP_Browsebgpen,(Tag)&bg,
         AOAPP_Textpen,(Tag)&text,
         AOAPP_Linkpen,(Tag)&link,
         AOAPP_Vlinkpen,(Tag)&vlink,
         AOAPP_Alinkpen,(Tag)&alink,
         TAG_END);
      coo->bgcolor=PICKCOLOR(fr->bgcolor,bg);
      coo->textcolor=PICKCOLOR(fr->textcolor,text);
      coo->linkcolor=PICKCOLOR(fr->linkcolor,link);
      coo->vlinkcolor=PICKCOLOR(fr->vlinkcolor,vlink);
      coo->alinkcolor=PICKCOLOR(fr->alinkcolor,alink);
      coo->bgimage=fr->bgimage;
      coo->bgalign=fr->bgalign;
   }
}

/* Update coords (copy) to our frame */
static void Newcoords(struct Frame *fr,struct Coords *coo)
{  long minx,miny,maxx,maxy;
   minx=fr->elt.aox+coo->dx+fr->x;
   maxx=minx+fr->w-1;
   miny=fr->elt.aoy+coo->dy+fr->y;
   maxy=miny+fr->h-1;
   if(minx>coo->minx) coo->minx=minx;
   if(miny>coo->miny) coo->miny=miny;
   if(maxx<coo->maxx) coo->maxx=maxx;
   if(maxy<coo->maxy) coo->maxy=maxy;
   coo->dx+=fr->elt.aox+fr->x-fr->left;
   coo->dy+=fr->elt.aoy+fr->y-fr->top;
   Setcolors(fr,coo);
}

/*-----------------------------------------------------------------------*/

/* Scroll contents of this frame, and repair previously obscured parts as necessary.
 * (coo) is used to offset our coordinates, (clipcoo) is used to clip
 * the coordinates after offsetting. */
static void Scroll(struct Frame *fr,void *win,struct Coords *coo,
   struct Coords *clipcoo,long dx,long dy,long x1,long y1,long x2,long y2)
{  struct Backfillinfo bfinfo={0};
   long bmw,bmh;
   if(!win) return;
   Agetattrs(win,
      AOWIN_Window,(Tag)&bfinfo.window,
      AOWIN_Rastport,(Tag)&bfinfo.rp,
      TAG_END);
   bfinfo.frame=fr;
   bfinfo.coo=clipcoo;
   if(clipcoo->bgimage && prefs.browser.docolors)
   {  Agetattrs(fr->bgimage,
         AOCDV_Imagebitmap,(Tag)&bfinfo.bitmap,
         AOCDV_Imagemask,(Tag)&bfinfo.mask,
         AOCDV_Alpha,(Tag)&bfinfo.alpha,
         AOCDV_Imagewidth,(Tag)&bmw,
         AOCDV_Imageheight,(Tag)&bmh,
         TAG_END);
      bfinfo.bmw=bmw;
      bfinfo.bmh=bmh;
   }
   if(prefs.browser.docolors && fr->bgcolor>=0) bfinfo.bgpen=fr->bgcolor;
   else bfinfo.bgpen=clipcoo->bgcolor;
   LockLayerInfo(bfinfo.rp->Layer->LayerInfo);
   LockLayer(0,bfinfo.rp->Layer);
   Installbg(&bfinfo);
   if(!Agetattr(win,AOWIN_Resized))
   {  x1+=coo->dx;
      y1+=coo->dy;
      x2+=coo->dx;
      y2+=coo->dy;
      x1=MAX(x1,clipcoo->minx);
      y1=MAX(y1,clipcoo->miny);
      x2=MIN(x2,clipcoo->maxx);
      y2=MIN(y2,clipcoo->maxy);
      ScrollRasterBF(bfinfo.rp,dx,dy,x1,y1,x2,y2);
      if(bfinfo.rp->Layer->Flags&LAYERREFRESH)
      {  Asetattrs(win,AOWIN_Refresh,TRUE,TAG_END);
      }
   }
   Uninstallbg(&bfinfo);
   UnlockLayer(bfinfo.rp->Layer);
   UnlockLayerInfo(bfinfo.rp->Layer->LayerInfo);
}

/* Handle scrolls */
static void Scrollframe(struct Frame *fr,long newleft,long newtop,BOOL setscroller)
{  long oldleft=fr->left,oldtop=fr->top;
   BOOL redisplay=FALSE;
   /* mycoo = coords for me, used to offset the scroll from.
    * doccoo = coords for my contents, after change in my top/left. Used to
    *          render my contents after scrolling, and used to clip the
    *          scroll to.
    * Note doccoo has to be obtained twice because of changing left,top values.
    */
   struct Coords mycoo={0},doccoo;
   /* dispw,disph = actual visible width and height. If scrolled amount is
    * equal or larger, then do a redisplay, else scroll what can be preserved */
   long dispw,disph;
   if(fr->frame)
   {  Framecoords(fr->frame,&mycoo);
   }
   else
   {  mycoo.win=fr->win;
      mycoo.rp=(struct RastPort *)Agetattr(fr->win,AOWIN_Rastport);
      mycoo.dri=(struct DrawInfo *)Agetattr(Aweb(),AOAPP_Drawinfo);
      mycoo.maxx=mycoo.maxy=AMRMAX;
   }
   if(!(fr->flags&FRMF_DUMBFRAME))
   {  if(newleft+fr->w>fr->docw) newleft=fr->docw-fr->w;
      if(newleft<0) newleft=0;
      if(newtop+fr->h>fr->doch) newtop=fr->doch-fr->h;
      if(newtop<0) newtop=0;
   }
   fr->top=newtop;
   doccoo=mycoo;
   Newcoords(fr,&doccoo);
   dispw=doccoo.maxx-doccoo.minx+1;
   disph=doccoo.maxy-doccoo.miny+1;
/*
printf("doccoo: (%d,%d)-(%d,%d) disph=%d newtop=%d oldtop=%d\n",
doccoo.minx,doccoo.miny,doccoo.maxx,doccoo.maxy,disph,newtop,oldtop);
*/
   if(newtop>oldtop)
   {  if(newtop-oldtop>=disph) redisplay=TRUE;
      else
      {  Scroll(fr,doccoo.win,&mycoo,&doccoo,0,newtop-oldtop,
            fr->elt.aox+fr->x,fr->elt.aoy+fr->y,fr->elt.aox+fr->x+fr->w-1,fr->elt.aoy+fr->y+fr->h-1);
         Rendercontents(fr,&doccoo,
            fr->left,fr->top+disph-(newtop-oldtop),
            fr->left+fr->w-1,fr->top+fr->h-1,AMRF_CLEAR);
      }
   }
   else if(newtop<oldtop)
   {  if(oldtop-newtop>=disph) redisplay=TRUE;
      else
      {  Scroll(fr,doccoo.win,&mycoo,&doccoo,0,newtop-oldtop,
            fr->elt.aox+fr->x,fr->elt.aoy+fr->y,fr->elt.aox+fr->x+fr->w-1,fr->elt.aoy+fr->y+fr->h-1);
         Rendercontents(fr,&doccoo,
            fr->left,fr->top,
            fr->left+fr->w-1,fr->top+fr->h-disph+(oldtop-newtop)-1,AMRF_CLEAR);
      }
   }
   fr->left=newleft;
   doccoo=mycoo;
   Newcoords(fr,&doccoo);
   if(newleft>oldleft)
   {  if(newleft-oldleft>=dispw) redisplay=TRUE;
      else
      {  Scroll(fr,doccoo.win,&mycoo,&doccoo,newleft-oldleft,0,
            fr->elt.aox+fr->x,fr->elt.aoy+fr->y,fr->elt.aox+fr->x+fr->w-1,fr->elt.aoy+fr->y+fr->h-1);
         Rendercontents(fr,&doccoo,
            fr->left+dispw-(newleft-oldleft),fr->top,
            fr->left+fr->w-1,fr->top+fr->h-1,AMRF_CLEAR);
      }
   }
   else if(newleft<oldleft)
   {  if(oldleft-newleft>=dispw) redisplay=TRUE;
      else
      {  Scroll(fr,doccoo.win,&mycoo,&doccoo,newleft-oldleft,0,
            fr->elt.aox+fr->x,fr->elt.aoy+fr->y,fr->elt.aox+fr->x+fr->w-1,fr->elt.aoy+fr->y+fr->h-1);
         Rendercontents(fr,&doccoo,
            fr->left,fr->top,
            fr->left+fr->w-dispw+(oldleft-newleft)-1,fr->top+fr->h-1,AMRF_CLEAR);
      }
   }
   if(redisplay)
   {  Rendercontents(fr,&doccoo,fr->left,fr->top,fr->left+fr->w,fr->top+fr->h,AMRF_CLEAR);
   }
   if(setscroller)
   {  if(fr->frame)
      {  if(fr->flags&FRMF_VSCROLL)
         {  Asetattrs(fr->vscroll,
               AOSCR_Top,fr->top,
               AOSCR_Update,TRUE,
               TAG_END);
         }
         if(fr->flags&FRMF_HSCROLL)
         {  Asetattrs(fr->hscroll,
               AOSCR_Top,fr->left,
               AOSCR_Update,TRUE,
               TAG_END);
         }
      }
      else
      {  Asetattrs(fr->win,
            AOWIN_Vslidertop,fr->top,
            AOWIN_Hslidertop,fr->left,
            TAG_END);
      }
   }
   Asetattrs(fr->whis,
      AOWHS_Frameid,(Tag)fr->id,
      AOWHS_Leftpos,fr->left,
      AOWHS_Toppos,fr->top,
      TAG_END);
   if(fr->search)
   {  Asetattrs(fr->search,AOSRH_Scrolled,TRUE,TAG_END);
   }
}

/* Layout our child and decide if our scrollers have to be displayed.
 * Return TRUE if scrollers have (dis)appeared). */
static BOOL Layoutcontents(struct Frame *fr,UWORD flags)
{  ULONG oldf=fr->flags&(FRMF_HSCROLL|FRMF_VSCROLL);
   ULONG lresult;
   BOOL changed;
   long scrw,scrh,w=0,h=0,vlayout=0,hlayout=0;
   if(fr->flags&FRMF_RETRYSCROLL)
   {  fr->flags&=~(FRMF_RETRYSCROLL|FRMF_HSCROLL|FRMF_VSCROLL);
      oldf=0;
      flags&=~AMLF_CHANGED;
   }
   if((fr->flags&FRMF_TOPFRAME) || !(fr->flags&FRMF_SCROLLING))
   {  if((fr->flags&FRMF_TOPFRAME) && prefs.browser.nominalframe && fr->layouth<400)
      {  h=400;
      }
      else h=fr->layouth;
      Alayout(fr->copy,fr->w,h,flags,NULL,0,NULL);
   }
   else if(fr->hscroll && fr->vscroll)
   {  scrw=Agetattr(fr->vscroll,AOBJ_Width);
      scrh=Agetattr(fr->hscroll,AOBJ_Height);
      fr->w=fr->elt.aow-2*fr->border;
      fr->h=fr->elt.aoh-2*fr->border;
      if(flags&AMLF_CHANGED)
      {  if(fr->flags&FRMF_VSCROLL)
         {  /* Let document expand itself. */
            fr->w-=scrw;
            if(fr->flags&FRMF_HSCROLL)
            {  fr->h-=scrh;
            }
            lresult=Alayout(fr->copy,fr->w,fr->h,flags,NULL,0,NULL);
            /* If it actually shrunk, and is less than frame size
             * without scrollers, try again without scrollers. */
            Agetattrs(fr->copy,
               AOBJ_Width,(Tag)&w,
               AOBJ_Height,(Tag)&h,
               TAG_END);
            fr->w=fr->elt.aow-2*fr->border;
            fr->h=fr->elt.aoh-2*fr->border;
            if(((fr->flags&FRMF_VSCROLL) && h<=fr->h)
            || ((fr->flags&FRMF_HSCROLL) && w<=fr->w))
            {  fr->flags&=~(FRMF_VSCROLL|FRMF_HSCROLL);
               oldf=0;
               lresult=Alayout(fr->copy,fr->w,fr->h,
                  (flags&~AMLF_CHANGED)|AMLF_FITHEIGHT,NULL,0,NULL);
            }
         }
         else
         {  /* Let document expand itself, and see if it still fits without
             * scrollers. */
            lresult=Alayout(fr->copy,fr->w,fr->h,flags|AMLF_FITHEIGHT,NULL,0,NULL);
         }
      }
      else
      {  /* First try without scrollers */
         fr->flags&=~(FRMF_VSCROLL|FRMF_HSCROLL);
         oldf=0;
         lresult=Alayout(fr->copy,fr->w,fr->h,flags|AMLF_FITHEIGHT,NULL,0,NULL);
      }
      Agetattrs(fr->copy,
         AOBJ_Width,(Tag)&w,
         AOBJ_Height,(Tag)&h,
         AOCPY_Vlayout,(Tag)&vlayout,
         AOCPY_Hlayout,(Tag)&hlayout,
         TAG_END);
      if(h>fr->h)
      {  /* Too high. Enable vertical scroller. Enable horizontal too if
          * contents is already known to be too wide even without v scroller. */
         fr->flags|=FRMF_VSCROLL;
         if(w>fr->w)
         {  fr->flags|=FRMF_HSCROLL;
            fr->h-=scrh;
         }
         fr->w-=scrw;
      }
      else if(w>fr->w)
      {  /* Too wide. Enable both scrollers. */
         fr->flags|=FRMF_VSCROLL|FRMF_HSCROLL;
         fr->w-=scrw;
         fr->h-=scrh;
      }
      /* If scrollers added or removed, and contents sensitive to change in widh/height,
       * relayout it. */
      if((lresult&AMLR_FHAGAIN)
      || ((fr->flags&FRMF_VSCROLL)!=(oldf&FRMF_VSCROLL) && hlayout)
      || ((fr->flags&FRMF_HSCROLL)!=(oldf&FRMF_HSCROLL) && vlayout))
      {  Alayout(fr->copy,fr->w,fr->h,flags&~AMLF_CHANGED,NULL,0,NULL);
         w=Agetattr(fr->copy,AOBJ_Width);
         /* If, due to the relayout, contents has become wider and no h scroller yet,
          * add a h scroller. Relayout again if contents sensitive to height. */
         if(!(fr->flags&FRMF_HSCROLL) && w>fr->w)
         {  fr->flags|=FRMF_HSCROLL;
            fr->h-=scrh;
            if(vlayout)
            {  Alayout(fr->copy,fr->w,fr->h,flags&~AMLF_CHANGED,NULL,0,NULL);
            }
         }
      }
      fr->layouth=fr->h;
      /* If scrollers have appeared, set and render them. */
      if(fr->elt.eltflags&ELTF_ALIGNED)
      {  if((fr->flags&FRMF_HSCROLL) && !(oldf&FRMF_HSCROLL))
         {  Asetattrs(fr->hscroll,
               AOBJ_Left,fr->elt.aox+fr->border,
               AOBJ_Top,fr->elt.aoy+fr->elt.aoh-fr->border-scrh,
               AOBJ_Width,fr->elt.aow-2*fr->border-scrw,
               AOSCR_Total,w,
               AOSCR_Visible,fr->w,
               AOSCR_Top,fr->left,
               TAG_END);
            Agetattrs(fr->hscroll,
               AOBJ_Left,(Tag)&fr->hsx,
               AOBJ_Top,(Tag)&fr->hsy,
               AOBJ_Width,(Tag)&fr->hsw,
               AOBJ_Height,(Tag)&fr->hsh,
               TAG_END);
            Arender(fr->hscroll,NULL,0,0,0,0,0,NULL);
         }
         if((fr->flags&FRMF_VSCROLL) && !(oldf&FRMF_VSCROLL))
         {  long total=h,visible=fr->h;
            if(visible>prefs.program.overlap)
            {  total=MAX(0,total-prefs.program.overlap);
               visible-=prefs.program.overlap;
            }
            Asetattrs(fr->vscroll,
               AOBJ_Left,fr->elt.aox+fr->elt.aow-fr->border-scrw,
               AOBJ_Top,fr->elt.aoy+fr->border,
               AOBJ_Height,fr->elt.aoh-2*fr->border,
               AOSCR_Total,total,
               AOSCR_Visible,visible,
               AOSCR_Top,fr->top,
               TAG_END);
            Agetattrs(fr->vscroll,
               AOBJ_Left,(Tag)&fr->vsx,
               AOBJ_Top,(Tag)&fr->vsy,
               AOBJ_Width,(Tag)&fr->vsw,
               AOBJ_Height,(Tag)&fr->vsh,
               TAG_END);
            Arender(fr->vscroll,NULL,0,0,0,0,0,NULL);
         }
      }
   }
   else
   {  Alayout(fr->copy,fr->w,fr->layouth,flags,NULL,0,NULL);
   }
   changed=(oldf!=(fr->flags&(FRMF_HSCROLL|FRMF_VSCROLL)));
   fr->docw=Agetattr(fr->copy,AOBJ_Width);
   fr->doch=Agetattr(fr->copy,AOBJ_Height);
   if(!(fr->flags&FRMF_FOCUSED) && (fr->flags&FRMF_SCROLLING)
   && (fr->docw>fr->w || fr->doch>fr->h))
   {  Asetattrs(fr->win,AOWIN_Focus,(Tag)fr,TAG_END);
      fr->flags|=FRMF_FOCUSED;
   }
   return changed;
}

/* Get the minimum width for this frame */
static long Minwidth(struct Frame *fr)
{  long vw=Agetattr(fr->vscroll,AOBJ_Width);
   long minhw=Agetattr(fr->hscroll,AOSCR_Minwidth);
   long w=2*fr->mwidth+vw+MAX(2,minhw)+2*fr->border;
   return w;
}

/* Get the minimum height for this frame */
static long Minheight(struct Frame *fr)
{  long hh=Agetattr(fr->hscroll,AOBJ_Height);
   long minvh=Agetattr(fr->vscroll,AOSCR_Minheight);
   long h=2*fr->mheight+MAX(minvh,2+hh)+2*fr->border;
   return h;
}

/*-----------------------------------------------------------------------*/

/* Get the currently displayed Url */
static void *Currenturl(struct Frame *fr)
{  void *url=NULL;
   Agetattrs(fr->whis,
      AOWHS_Frameid,(Tag)fr->id,
      AOWHS_Url,(Tag)&url,
      TAG_END);
   return url;
}

/* Our contents has changed, or we want to display at another position */
static void Updatecopy(struct Frame *fr,BOOL changed)
{  UBYTE *fragment=NULL;
   long y=-1,left=fr->left,top=fr->top;
   BOOL moved;
//printf("Updatecopy %08x\n",fr);
   /* Don't attempt to layout when we are iconified. */
   if(!fr->win) return;
   if(changed)
   {  Ameasure(fr->copy,fr->w,fr->h,0,AMMF_CHANGED,NULL,NULL);
      Layoutcontents(fr,AMLF_CHANGED);
      if(top>fr->doch-fr->h) top=MAX(0,fr->doch-fr->h);
      if(left>fr->docw-fr->w) left=MAX(0,fr->docw-fr->w);
      Anotifyset(fr->copy,AOFRM_Updatecopy,TRUE,TAG_END);
   }
   if(fr->flags&FRMF_USEHISPOS)
   {  Agetattrs(fr->whis,
         AOWHS_Frameid,(Tag)fr->id,
         AOWHS_Leftpos,(Tag)&left,
         AOWHS_Toppos,(Tag)&top,
         TAG_END);
      if(top+fr->h<=fr->doch)
      {  /* We found the position at this update */
         fr->flags&=~FRMF_USEHISPOS;
      }
      else
      {  top=MIN(fr->top,fr->doch-fr->h);
         if(top<0) top=0;
      }
   }
   else if(fr->flags&FRMF_USEFRAGMENT)
   {  Agetattrs(fr->whis,
         AOWHS_Frameid,(Tag)fr->id,
         AOWHS_Fragment,(Tag)&fragment,
         TAG_END);
      if(fragment)
      {  Agetattrs(fr->copy,
            AOCDV_Fragmentname,(Tag)fragment,
            AOCDV_Fragmentpos,(Tag)&y,
            TAG_END);
         if(y>=0)
         {  top=MIN(y,fr->doch-fr->h);
            if(top<0) top=0;
            left=0;
            if(top==y) fr->flags&=~FRMF_USEFRAGMENT;
         }
      }
      else
      {  left=top=0;
         fr->flags&=~FRMF_USEFRAGMENT;
      }
      Asetattrs(fr->whis,
         AOWHS_Frameid,(Tag)fr->id,
         AOWHS_Leftpos,left,
         AOWHS_Toppos,top,
         TAG_END);
   }
   if(changed)
   {  moved=(left!=fr->left || top!=fr->top);
      fr->left=left;
      fr->top=top;
      Setscrollers(fr,moved);
      if((fr->flags&FRMF_TOPFRAME) || (fr->elt.eltflags&ELTF_ALIGNED))
      {  Rendercontents(fr,NULL,fr->left,fr->top,fr->left+fr->w-1,fr->top+fr->h-1,
            AMRF_CLEAR|(moved?0:AMRF_CHANGED));
      }
      fr->flags&=~(FRMF_CHANGEDCHILD|FRMF_NEWCHILD);
   }
   else if(left!=fr->left || top!=fr->top)
   {  Scrollframe(fr,left,top,TRUE);
   }
}

/* Make the waiting inputcopy the current displayed copy */
static void Newdisplay(struct Frame *fr)
{  void *url;
   if(fr->copy)
   {  Asetattrs(fr->copy,
         AOBJ_Frame,(Tag)NULL,
         AOBJ_Cframe,(Tag)NULL,
         AOBJ_Window,(Tag)NULL,
         TAG_END);
      /* This new display can happen as a result of JavaScript history.go(-1)
       * during parse of document source. Disposing the copy would dispose
       * the document while it is still parsing itself... */
      Asetattrs(fr->copy,AOCPY_Deferdispose,TRUE,TAG_END);
      if(fr->defstatus)
      {  FREE(fr->defstatus);
         fr->defstatus=NULL;
      }
      fr->jgenerated=NULL;
   }
   fr->bgcolor=fr->textcolor=fr->linkcolor=fr->alinkcolor=fr->vlinkcolor=-1;
   fr->bgimage=NULL;
   fr->bgalign=NULL;
   fr->copy=fr->inputcopy;
   fr->flags=(fr->flags&~(FRMF_USEHISPOS|FRMF_USEFRAGMENT|FRMF_FOCUSED))|fr->inputflags;
   fr->inputflags=0;
   fr->inputcopy=NULL;
   // Clearjframe(fr);
   Freejframe(fr);
   Asetattrs(fr->inputwhis,AOWHS_Skipfrom,(Tag)fr->whis,TAG_END);
   fr->whis=fr->inputwhis;
   fr->inputwhis=NULL;
   if(fr->win && (fr->flags&FRMF_TOPFRAME))
   {  url=(void *)Agetattr(fr->copy,AOCPY_Url);
      Asetattrs(fr->win,
         AOWIN_Currenturl,Agetattr(url,AOURL_Url),
         AOWIN_Bgsound,FALSE,
         TAG_END);
   }
   Asetattrs(fr->copy,
      AOBJ_Frame,(Tag)fr,
      AOBJ_Cframe,(Tag)fr,
      AOBJ_Window,(Tag)fr->win,
      AOBJ_Winhis,(Tag)fr->whis,
      TAG_END);
   fr->flags|=FRMF_CHANGEDCHILD|FRMF_NEWCHILD|FRMF_RETRYSCROLL;
   if(fr->win)
   {
      Asetattrs(fr->win,AOBJ_Winhis,(Tag)fr->whis,TAG_END);
   }
   if(fr->search)
   {  Asetattrs(fr->search,AOSRH_Reset,TRUE,TAG_END);
   }
   if(fr->info)
   {  Asetattrs(fr->copy,AOCPY_Info,(Tag)fr->info,TAG_END);
   }
}

/* A new winhis was set, this is a request to display something else. */
static void Setnewwinhis(struct Frame *fr,void *whis,BOOL noreferer)
{  void *url=NULL;
   UBYTE *fragment=NULL,*oldfragment=NULL;
   ULONG history=FALSE,isleading=FALSE,loadflags=0;
   void *oldurl=NULL;
   void *referer=NULL;
   long loadnr;


   /* Get what is currently displayed */
   Agetattrs(fr->whis,
      AOWHS_Frameid,(Tag)fr->id,
      AOWHS_Url,(Tag)&oldurl,
      AOWHS_Fragment,(Tag)&oldfragment,
      TAG_END);
   /* Let's see what to display. */
   Agetattrs(whis,
      AOWHS_Frameid,(Tag)fr->id,
      AOWHS_Url,(Tag)&url,
      AOWHS_Fragment,(Tag)&fragment,
      AOWHS_History,(Tag)&history,
      AOWHS_Isleading,(Tag)&isleading,
      TAG_END);
   /* If the winhis contains an URL for us, it was a history or a new url.
    * Remember to use the old scroll positions, they will be zero for
    * a new url.
    * Otherwise use our default and register it in the new winhis. */
   if(url)
   {
      if(history)
      {  fr->inputflags|=FRMF_USEHISPOS;
         fr->inputflags&=~FRMF_USEFRAGMENT;
      }
      else
      {  fr->inputflags&=~FRMF_USEHISPOS;
         if(fragment || url!=oldurl)
         {  /* Position at fragment or at the top for a new document. */
            fr->inputflags|=FRMF_USEFRAGMENT;
         }
      }
   }
   else
   {  Asetattrs(whis,
         AOWHS_Frameid,(Tag)fr->id,
         AOWHS_Url,(Tag)fr->orgurl,
         AOWHS_Fragment,(Tag)fr->orgfragment,
         TAG_END);
      url=fr->orgurl;
      fragment=fr->orgfragment;
      fr->inputflags&=~FRMF_USEHISPOS;
      if(fragment || url!=oldurl)
      {  /* Position at fragment or at the top for a new document. */
         fr->inputflags|=FRMF_USEFRAGMENT;
      }
   }
   /* If the url is the same and not expired, just use the old copy. */
   /* Also if we are the leading frame check that the url was not temp moved */
   if((url==oldurl) && !(fr->flags&FRMF_RELOADVERIFY)
   && (history || Agetattr(url,AOURL_Sourcevalid))
   && !(((struct Url *)url)->flags&URLF_TEMPMOVED && isleading) )
   {

      if(!isleading
      && ((!fragment && !oldfragment)
           || (fragment && oldfragment && STRIEQUAL(fragment,oldfragment))))
      {  /* Fragment is the same and we're not the leading frame in the new winhis.
          * Don't move to fragment again. Don't set Skipfrom etc. */
         fr->inputflags&=~FRMF_USEFRAGMENT;
      }
      else
      {  /* Only set Skipfrom and window's Winhis if fragment is different because:
          * if another frame's source changes, but doesn't result in a new document,
          * we don't want to make this winhis a "real" one. If the changed frame
          * gets the new document, IT will set skipfrom etc. */
         Asetattrs(whis,AOWHS_Skipfrom,(Tag)fr->whis,TAG_END);
         if(fr->win)
         {  Asetattrs(fr->win,AOBJ_Winhis,(Tag)whis,TAG_END);
         }
      }
      fr->whis=whis;
      fr->flags=(fr->flags&~(FRMF_USEHISPOS|FRMF_USEFRAGMENT))|fr->inputflags;
      fr->inputflags=0;
      Updatecopy(fr,FALSE);
      /* Forward the new winhis (might not be ours) to our copy */
      Asetattrs(fr->copy,AOBJ_Winhis,(Tag)whis,TAG_END);
   }
   /* Otherwise load in the object, provided that it isn't yet in the hierarchy above. */
   else if(!Nestedurl(fr,url))
   {  /* New document, forget about pending clientpull */

      fr->pullurl=NULL;
      if(fr->inputcopy) Adisposeobject(fr->inputcopy);
      if((fr->flags&FRMF_TOPFRAME) && fr->win)
      {  Asetattrs(fr->win,AOWIN_Activeurl,(Tag)url,TAG_END);
      }

   /*Find the referer from the oldurl */
   if (!noreferer)
   {
      referer=(void *)Agetattr(oldurl,AOURL_Finalurlptr);
   }

      fr->jgenerated=NULL;
      fr->inputcopy=Anewobject(AOTP_COPY,
         AOCPY_Url,(Tag)url,
         AOCPY_Defaulttype,(Tag)"text/plain",
         AOCPY_Reloadverify,BOOLVAL(fr->flags&FRMF_RELOADVERIFY),
         AOCPY_Mwidth,fr->mwidth,
         AOCPY_Mheight,fr->mheight,
         AOCPY_Referer,(Tag)referer,
         AOBJ_Layoutparent,(Tag)fr,
         AOBJ_Nobackground,BOOLVAL(fr->flags&FRMF_NOBACKGROUND),
         TAG_END);
      if(history) loadflags|=AUMLF_HISTORY;
      if(Agetattr(fr->win,AOWIN_Noproxy)) loadflags|=AUMLF_NOPROXY;
      if(fr->flags&FRMF_RELOADVERIFY)
      {  loadflags|=AUMLF_VERIFY;
         fr->flags&=~FRMF_RELOADVERIFY;
      }
      Auload(url,loadflags,referer,NULL,fr);
      if(loadnr=Agetattr(url,AOURL_Loadnr))
      {  Asetattrs(whis,
            AOWHS_Frameid,(Tag)fr->id,
            AOWHS_Loadnr,loadnr,
            TAG_END);
      }
   }
   /* If any object was in memory, then it must be processed now since there is no
    * fetch process that will send AOM_SRCUPDATE. */
   if(fr->flags&FRMF_TOPFRAME) Doupdateframes();
}

/* Frame has been resized */
static void Resizeframe(struct Frame *fr,long newaow,long newaoh)
{  BOOL neww=(newaow!=fr->elt.aow),newh=(newaoh!=fr->elt.aoh);
//printf("Resizeframe %08x w:%d->%d h:%d->%d\n",fr,fr->elt.aow,newaow,fr->elt.aoh,newaoh);
   Busypointer(TRUE);
   fr->elt.aow=newaow;
   fr->elt.aoh=newaoh;
   if(fr->frame)
   {  fr->x=fr->border;
      fr->y=fr->border;
      fr->w=fr->elt.aow-2*fr->border;
      fr->h=fr->elt.aoh-2*fr->border;
      if(fr->flags&FRMF_VSCROLL) fr->w-=Agetattr(fr->vscroll,AOBJ_Width);
      if(fr->flags&FRMF_HSCROLL) fr->h-=Agetattr(fr->hscroll,AOBJ_Height);
   }
   else
   {  Agetattrs(fr->win,
         AOWIN_Innerleft,(Tag)&fr->x,
         AOWIN_Innertop,(Tag)&fr->y,
         AOWIN_Innerwidth,(Tag)&fr->w,
         AOWIN_Innerheight,(Tag)&fr->h,
         TAG_END);
   }
   if(!(fr->flags&FRMF_DUMBFRAME)) fr->layouth=fr->h;
   if(fr->flags&FRMF_CHANGEDCHILD)
   {  Updatecopy(fr,TRUE);
   }
   else if(neww || newh || (fr->flags&FRMF_RESET))
   {  Layoutcontents(fr,0);
   }
   if(fr->top+fr->h>fr->doch) fr->top=fr->doch-fr->h;
   if(fr->top<0) fr->top=0;
   if(fr->left+fr->w>fr->docw) fr->left=fr->docw-fr->w;
   if(fr->left<0) fr->left=0;
   Setscrollers(fr,TRUE);
   Busypointer(FALSE);
}

/* A reload was requested. If url==NULL, use current or original url. */
static void Reloadframe(struct Frame *fr,void *url)
{  ULONG loadflags=AUMLF_RELOAD;
   long isleading;
   fr->jgenerated=NULL;
   Agetattrs(fr->whis,
      AOWHS_Frameid,(Tag)fr->id,
      url?TAG_IGNORE:AOWHS_Url,(Tag)&url,
      AOWHS_Isleading,(Tag)&isleading,
      TAG_END);
   if(!url) url=fr->orgurl;
   fr->inputflags&=~FRMF_USEHISPOS;
   fr->inputflags|=FRMF_USEFRAGMENT;
   /* Force any new subordinate frames to use their own default
    * contents.
    * If this frame is the leading one in the current winhis,
    * just clear out the subordinate frames. Else create a copy
    * of the winhis upto this frame */
   if(isleading)
   {
       fr->inputwhis=fr->whis;
   }
   else
   {  fr->inputwhis=Anewobject(AOTP_WINHIS,
         AOWHS_Copyfrom,(Tag)fr->whis,
         TAG_END);
   }
   Asetattrs(fr->inputwhis,
      AOWHS_Frameid,(Tag)fr->id,
      AOWHS_Clearbelow,TRUE,
      AOWHS_History,FALSE,
      AOWHS_Url,(Tag)url,
      TAG_END);
   /* First reload the URL, then create the copy. Otherwise in case of a
    * temporarily moved URL, the copy will attach itself to the source of
    * the second URL, and if relocation has changed for the reload of the
    * first URL, the copy will never notice the new source */
   if(Agetattr(fr->win,AOWIN_Noproxy)) loadflags|=AUMLF_NOPROXY;
   Auload(url,loadflags,url,NULL,fr);
   if(fr->inputcopy) Adisposeobject(fr->inputcopy);
   fr->inputcopy=Anewobject(AOTP_COPY,
      AOCPY_Url,(Tag)url,
      AOCPY_Defaulttype,(Tag)"text/plain",
      AOCPY_Reloadverify,TRUE,
      AOCPY_Mwidth,fr->mwidth,
      AOCPY_Mheight,fr->mheight,
      AOBJ_Layoutparent,(Tag)fr,
      TAG_END);
   if((fr->flags&FRMF_TOPFRAME) && fr->win)
   {  Asetattrs(fr->win,AOWIN_Activeurl,(Tag)url,TAG_END);
   }
}

/* Build a popup menu */
static void Popupinquire(struct Frame *fr,void *pup)
{  struct Popupitem *pi;
   for(pi=prefs.gui.popupmenu[PUPT_FRAME].first;pi->next;pi=pi->next)
   {  Asetattrs(pup,
         AOPUP_Title,(Tag)pi->title,
         AOPUP_Command,(Tag)pi->cmd,
         TAG_END);
   }
}

/* A popup menu item was selected */
static void Popupselectframe(struct Frame *fr,UBYTE *cmd)
{  UBYTE *target=NULL,*p;
   short i;
   if(cmd)
   {  if(fr->id)
      {  if(target=ALLOCTYPE(UBYTE,4*strlen(fr->id)+2,0))
         {  for(i=0,p=target;fr->id[i];i++)
            {  p+=sprintf(p,"%c%d",i?'.':'#',fr->id[i]);
            }
         }
      }
      Execarexxcmd(Agetattr(fr->win,AOWIN_Key),cmd,"ui",
         Agetattr(Currenturl(fr),AOURL_Url),
         target);
      if(target) FREE(target);
   }
   fr->popup=NULL;
}

/* Do the client pull */
static void Doclientpull(struct Frame *fr)
{  void *url=fr->pullurl;
   if(url)
   {  fr->pullurl=NULL;
      if(fr->flags&FRMF_PULLRELOAD)
      {  Reloadframe(fr,url);
      }
      else
      {  Inputwindoc(fr->win,url,NULL,fr->id);
      }
   }
}

/* Find a comma or semicolon */
static UBYTE *Strcsc(UBYTE *p)
{  UBYTE *q,*r;
   q=strchr(p,';');
   r=strchr(p,',');
   if(!q || (r && r<q)) q=r;
   return q;
}

/* A new clientpull string was set */
static void Setclientpull(struct Frame *fr,UBYTE *ref)
{  long wait=0;
   void *url=NULL,*currenturl,*src;
   BOOL reload=FALSE;
   UBYTE *p,*q,*r;
   ULONG history=FALSE;
   Agetattrs(fr->whis,
      AOWHS_Frameid,(Tag)fr->id,
      AOWHS_History,(Tag)&history,
      TAG_END);
   src=(void *)Agetattr(fr->copy,AOCPY_Source);
   currenturl=(void *)Agetattr(src,AOSRC_Url);
   if(!history && (sscanf(ref," %ld",&wait)>0))
   {  for(p=Strcsc(ref);p && *p;p=Strcsc(p))
      {  p++;
         while(*p && isspace(*p)) p++;
         if(!*p) break;
         if(STRNIEQUAL(p,"RELOAD",6))
         {  reload=TRUE;
         }
         else
         {  if(STRNIEQUAL(p,"URL=",4)) p+=4;
            if(*p=='"')
            {  p++;
               q=strchr(p,'"');
            }
            else
            {  q=Strcsc(p);
            }
            if(!q) q=p+strlen(p);
            if(r=Dupstr(p,q-p))
            {  url=Findurl((UBYTE *)Agetattr(currenturl,AOURL_Url),r,0);
               FREE(r);
            }
         }
      }
      if(!url)
      {  url=currenturl;
         reload=TRUE;
      }
      else if(url==currenturl)
      {  reload=TRUE;
      }
      if(reload && Agetattr(url,AOURL_Postnr))
      {  url=Findurl("",(UBYTE *)Agetattr(currenturl,AOURL_Url),0);
         reload=FALSE;
      }
      SETFLAG(fr->flags,FRMF_PULLRELOAD,reload);
      fr->pullurl=url;
      if(!fr->pulltimer)
      {  fr->pulltimer=Anewobject(AOTP_TIMER,
            AOBJ_Target,(Tag)fr,
            AOBJ_Map,(Tag)mapclientpull,
            TAG_END);
      }
      if(fr->pulltimer)
      {  Asetattrs(fr->pulltimer,
            AOTIM_Waitseconds,wait,
            AOTIM_Waitmicros,1000,
            TAG_END);
      }
   }
}

/*-----------------------------------------------------------------------*/

/* Make a new frameid from our parents id and our seqnr. */
static void Makeframeid(struct Frame *fr)
{  UBYTE *hid;
   long len;
   if(fr->id)
   {  FREE(fr->id);
      fr->id=NULL;
   }
   if(fr->frame)
   {  hid=(UBYTE *)Agetattr(fr->frame,AOFRM_Id);
      if(!hid) hid="";
      len=strlen(hid);
      if(fr->id=Dupstr(hid,len+1))
      {  fr->id[len]=fr->seqnr;
      }
   }
}

/* Delete this frames name */
static void Delframename(struct Frame *fr)
{  if(fr && fr->name)
   {  REMOVE(fr->name);
      if(fr->name->name) FREE(fr->name->name);
      FREE(fr->name);
      fr->name=NULL;
   }
}

/* Add a name to a frame */
static void Addframename(struct Frame *fr,UBYTE *name)
{  struct Framename *frn;
   Delframename(fr);
   if(frn=ALLOCSTRUCT(Framename,1,MEMF_CLEAR))
   {  if(name) frn->name=Dupstr(name,-1);
      frn->frame=fr;
      ADDTAIL(&framenames,frn);
      fr->name=frn;
   }
}

/*-----------------------------------------------------------------------*/

static long Renderframe(struct Frame *fr,struct Amrender *amr)
{  struct Coords coords={0},co2,*coo;
   BOOL clip=FALSE;
   long scrw,scrh,x,y;
   short i;
   struct RastPort *rp;
   ULONG clipkey=0;
   if(!(amr->flags&(AMRF_UPDATESELECTED|AMRF_UPDATENORMAL)))
   {  coo=amr->coords;
      if(!coo)
      {  if(fr->frame)
         {  Framecoords(fr->frame,&coords);
            clip=TRUE;
         }
         else
         {  coords.win=fr->win;
            coords.rp=(struct RastPort *)Agetattr(fr->win,AOWIN_Rastport);
            coords.dri=(struct DrawInfo *)Agetattr(Aweb(),AOAPP_Drawinfo);
            coords.maxx=coords.maxy=AMRMAX;
         }
         coo=&coords;
      }
/*
printf("Renderframe %08x ao=(%d,%d)+(%d,%d) window=(%d,%d)+(%d,%d)\n",
fr,fr->elt.aox,fr->elt.aoy,fr->elt.aow,fr->elt.aoh,fr->x,fr->y,fr->w,fr->h);
printf("            amr=(%d,%d)-(%d,%d) coo:d=(%d,%d) clip=(%d,%d)-(%d,%d)\n",
amr->rect.minx,amr->rect.miny,amr->maxx,amr->maxy,coo->dx,coo->dy,coo->minx,coo->miny,coo->maxx,coo->maxy);
*/
      if(coo->rp && coo->minx<=coo->maxx && coo->miny<=coo->maxy)
      {  rp=coo->rp;
         if(fr->copy)
         {  co2=*coo;
            Newcoords(fr,&co2);
            /* Render contents. Limit rendering to intersection of our visible window
             * (left,top)-(w,h) and our Amrender limits, transformed to the new
             * Coords offsets (+coo->dx gives RP coordinates, -co2.dx gives frame
             * contents coordinates). */
            Rendercontents(fr,&co2,
               MAX(fr->left,amr->rect.minx+coo->dx-co2.dx),
               MAX(fr->top,amr->rect.miny+coo->dy-co2.dy),
               MIN(fr->left+fr->w,amr->rect.maxx+coo->dx-co2.dx),
               MIN(fr->top+fr->h,amr->rect.maxy+coo->dy-co2.dy),
               AMRF_CLEAR);   /***..... is CLEAR reallyy needed? (embedded) ....***/
         }
         if(fr->vscroll || fr->hscroll || (fr->border))
         {  if(clip) clipkey=Clipto(rp,coo->minx,coo->miny,coo->maxx,coo->maxy);
            x=fr->elt.aox+coo->dx;
            y=fr->elt.aoy+coo->dy;
            for(i=0;i<fr->border;i++)
            {  SetAPen(rp,coo->dri->dri_Pens[SHADOWPEN]);
               Move(rp,x+i,y+fr->elt.aoh-i-1);
               Draw(rp,x+i,y+i);
               Draw(rp,x+fr->elt.aow-i-1,y+i);
               SetAPen(rp,coo->dri->dri_Pens[SHINEPEN]);
               Draw(rp,x+fr->elt.aow-i-1,y+fr->elt.aoh-i-1);
               Draw(rp,x+i,y+fr->elt.aoh-i-1);
            }
            scrh=Agetattr(fr->hscroll,AOBJ_Height);
            scrw=Agetattr(fr->vscroll,AOBJ_Width);
            if(fr->hscroll && (fr->flags&FRMF_HSCROLL))
            {  Asetattrs(fr->hscroll,
                  AOBJ_Left,fr->elt.aox+fr->border,
                  AOBJ_Top,fr->elt.aoy+fr->elt.aoh-fr->border-scrh,
                  AOBJ_Width,fr->elt.aow-2*fr->border-scrw,
                  AOSCR_Visible,fr->w,
                  TAG_END);
               Agetattrs(fr->hscroll,
                  AOBJ_Left,(Tag)&fr->hsx,
                  AOBJ_Top,(Tag)&fr->hsy,
                  AOBJ_Width,(Tag)&fr->hsw,
                  AOBJ_Height,(Tag)&fr->hsh,
                  TAG_END);
               Arender(fr->hscroll,coo,0,0,0,0,0,NULL);
            }
            if(fr->vscroll && (fr->flags&FRMF_VSCROLL))
            {  Asetattrs(fr->vscroll,
                  AOBJ_Left,fr->elt.aox+fr->elt.aow-fr->border-scrw,
                  AOBJ_Top,fr->elt.aoy+fr->border,
                  AOBJ_Height,fr->elt.aoh-2*fr->border,
                  AOSCR_Visible,fr->h-(fr->h>prefs.program.overlap?prefs.program.overlap:0),
                  TAG_END);
               Agetattrs(fr->vscroll,
                  AOBJ_Left,(Tag)&fr->vsx,
                  AOBJ_Top,(Tag)&fr->vsy,
                  AOBJ_Width,(Tag)&fr->vsw,
                  AOBJ_Height,(Tag)&fr->vsh,
                  TAG_END);
               Arender(fr->vscroll,coo,0,0,0,0,0,NULL);
            }
            if(clip) Unclipto(clipkey);
         }
      }
   }
   return 0;
}

static long Setframe(struct Frame *fr,struct Amset *ams)
{  long result=0;
   struct TagItem *tag,*tstate=ams->tags;
   void *newwhis=NULL,*target=NULL;
   struct Arect *mvis=NULL;
   long newleft=fr->left,newtop=fr->top;
   long newaow=fr->elt.aow,newaoh=fr->elt.aoh;
   UBYTE *name=NULL;
   BOOL noreferer=FALSE,setscroller=FALSE,timeout=FALSE,scroll=FALSE;
   BOOL qinputwindoc=FALSE,qhistory=FALSE;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOFRM_Border:
            fr->border=tag->ti_Data;
            fr->flags|=FRMF_BORDERSET;
            break;
         case AOFRM_Defaultborder:
            if(!(fr->flags&FRMF_BORDERSET))
            {  fr->border=tag->ti_Data;
            }
            break;
         case AOFRM_Scrolling:
            SETFLAG(fr->flags,FRMF_SCROLLING,tag->ti_Data);
            break;
         case AOFRM_Resize:
            /* Always allow resizing */
            /* SETFLAG(fr->flags,FRMF_RESIZE,tag->ti_Data); */
            break;
         case AOFRM_Marginwidth:
            fr->mwidth=tag->ti_Data;
            break;
         case AOFRM_Marginheight:
            fr->mheight=tag->ti_Data;
            break;
         case AOFRM_Width:
            fr->width=tag->ti_Data;
            fr->flags&=~FRMF_PIXELWIDTH;
            break;
         case AOFRM_Pixelwidth:
            fr->width=tag->ti_Data;
            fr->flags|=FRMF_PIXELWIDTH;
            break;
         case AOFRM_Height:
            fr->height=tag->ti_Data;
            fr->flags&=~FRMF_PIXELHEIGHT;
            break;
         case AOFRM_Pixelheight:
            fr->height=tag->ti_Data;
            fr->flags|=FRMF_PIXELHEIGHT;
            break;
         case AOBJ_Window:
            if(fr->win && !tag->ti_Data)
            {  Asetattrs(fr->win,
                  AOWIN_Goinactive,(Tag)fr,
                  AOWIN_Nofocus,(Tag)fr,
                  TAG_END);
               if(fr->popup)
               {  Adisposeobject(fr->popup);
                  fr->popup=NULL;
               }
            }
            if(!fr->win && tag->ti_Data)
            {
                 if(fr->whis)
                 {
                    Asetattrs((void *)tag->ti_Data,
                        AOBJ_Winhis,(Tag)fr->whis,
                        TAG_END);
                 }
            }
            fr->win=(void *)tag->ti_Data;
            /* Let copy obtain/release its pens */
            Asetattrs(fr->copy,AOBJ_Window,(Tag)fr->win,TAG_END);
            if(!fr->win)
            {  /* Reset so new window will cause relayout */
               fr->elt.aow=fr->elt.aoh=0;
               /* Reset too to prevent resize logic now */
               newaow=newaoh=0;
               /* Close any info window */
               if(fr->info)
               {  Asetattrs(fr->copy,AOCPY_Info,(Tag)NULL,TAG_END);
                  Adisposeobject(fr->info);
                  fr->info=NULL;
               }
               fr->pullurl=NULL;
            }
            break;
         case AOBJ_Frame:
            fr->frame=(struct Frame *)tag->ti_Data;
            if(fr->hscroll) Asetattrs(fr->hscroll,AOBJ_Cframe,(Tag)fr->frame,TAG_END);
            if(fr->vscroll) Asetattrs(fr->vscroll,AOBJ_Cframe,(Tag)fr->frame,TAG_END);
            tag->ti_Tag=TAG_IGNORE;
            /* If we are bound, compose our id and start loading ourselves. */
            if(fr->frame)
            {  Makeframeid(fr);
               if(fr->inputwhis) newwhis=fr->inputwhis;
               /* Also reset our dimensions so we'll layout again */
               fr->elt.aow=fr->elt.aoh=0;
            }
            else
            {  /* we are unbound, unbind our copy */
               Clearjframe(fr);
               Asetattrs(fr->copy,AOCPY_Deferdispose,TRUE,TAG_END);
               fr->copy=NULL;
               fr->whis=NULL;
               if(fr->id)
               {  FREE(fr->id);
                  fr->id=NULL;
               }
               if(fr->search)
               {  Adisposeobject(fr->search);
                  fr->search=NULL;
               }
            }
            break;
         case AOBJ_Winhis:
            if((void *)tag->ti_Data!=fr->inputwhis)
            {  fr->inputwhis=(void *)tag->ti_Data;
               /* Only start loading our doc if we are displayed, to avoid nesting limit */
               if(fr->frame || (fr->flags&FRMF_TOPFRAME)) newwhis=fr->inputwhis;
            }
            break;
         case AOBJ_Layoutparent:
            fr->layoutparent=(void *)tag->ti_Data;
            break;
         case AOFRM_Leftpos:
            newleft=tag->ti_Data;
            scroll=TRUE;
            break;
         case AOFRM_Toppos:
            newtop=tag->ti_Data;
            scroll=TRUE;
            break;
         case AOFRM_Setscroller:
            setscroller=BOOLVAL(tag->ti_Data);
            break;
         case AOFRM_Url:
            fr->orgurl=(void *)tag->ti_Data;
            break;
         case AOFRM_Fragment:
            if(fr->orgfragment) FREE(fr->orgfragment);
            fr->orgfragment=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOBJ_Width:
            newaow=tag->ti_Data;
            tag->ti_Tag=TAG_IGNORE;
            if(fr->flags&FRMF_RESET) fr->elt.aow=0;
            break;
         case AOBJ_Height:
            newaoh=tag->ti_Data;
            tag->ti_Tag=TAG_IGNORE;
            if(fr->flags&FRMF_RESET) fr->elt.aoh=0;
            break;
         case AOBJ_Changedchild:
            if((void *)tag->ti_Data==fr->copy)
            {  fr->flags|=FRMF_CHANGEDCHILD;
            }
            /* If we are not layed out yet, register with our parent */
            if((void *)tag->ti_Data==fr->inputcopy)
            {  if(fr->layoutparent && (!fr->elt.aow || !fr->elt.aoh))
               {  Asetattrs(fr->layoutparent,AOBJ_Changedchild,(Tag)fr,TAG_END);
               }
            }
            break;
         case AOFRM_Bgcolor:
            fr->bgcolor=(short)tag->ti_Data;
            break;
         case AOFRM_Textcolor:
            fr->textcolor=(short)tag->ti_Data;
            break;
         case AOFRM_Linkcolor:
            fr->linkcolor=(short)tag->ti_Data;
            break;
         case AOFRM_Vlinkcolor:
            fr->vlinkcolor=(short)tag->ti_Data;
            break;
         case AOFRM_Alinkcolor:
            fr->alinkcolor=(short)tag->ti_Data;
            break;
         case AOFRM_Bgimage:
            fr->bgimage=(void *)tag->ti_Data;
            break;
         case AOFRM_Bgalign:
            fr->bgalign=(struct Aobject *)tag->ti_Data;
            break;
         case AOFRM_Seqnr:
            fr->seqnr=(UBYTE)tag->ti_Data;
            break;
         case AOFRM_Topframe:
            SETFLAG(fr->flags,FRMF_TOPFRAME,tag->ti_Data);
            break;
         case AOFRM_Reload:
            if(tag->ti_Data) Reloadframe(fr,NULL);
            break;
         case AOFRM_Updatecopy:
            if(fr->flags&FRMF_CHANGEDCHILD)
            {  Updatecopy(fr,TRUE);
            }
            else if(fr->copy)
            {  Anotifyset(fr->copy,AOFRM_Updatecopy,TRUE,TAG_END);
            }
            break;
         case AOFRM_Makevisible:
            mvis=(struct Arect *)tag->ti_Data;
            break;
         case AOFRM_Displaycopy:
            if((void *)tag->ti_Data==fr->inputcopy)
            {  Newdisplay(fr);
            }
            break;
         case AOFRM_Name:
            name=(UBYTE *)tag->ti_Data;
            if(name && !*name) name=NULL;
            break;
         case AOFRM_Search:
            if(tag->ti_Data)
            {  if(fr->search)
               {  Asetattrs(fr->search,AOSRH_Activate,TRUE,TAG_END);
               }
               else
               {  fr->search=Anewobject(AOTP_SEARCH,
                     AOBJ_Frame,(Tag)fr,
                     TAG_END);
               }
            }
            else if(fr->search)
            {  Adisposeobject(fr->search);
               fr->search=NULL;
            }
            break;
         case AOFRM_Dumbframe:
            SETFLAG(fr->flags,FRMF_DUMBFRAME,tag->ti_Data);
            break;
         case AOFRM_Layoutheight:
            fr->layouth=tag->ti_Data;
            break;
         case AOFRM_Inline:
            SETFLAG(fr->flags,FRMF_INLINE,tag->ti_Data);
            break;
         case AOFRM_Commands:
            Asetattrs(fr->inputwhis?fr->inputwhis:fr->whis,AOWHS_Commands,tag->ti_Data,TAG_END);
            break;
         case AOFRM_Noreferer:
            noreferer=BOOLVAL(tag->ti_Data);
            break;
         case AOFRM_Info:
            if(tag->ti_Data && !fr->info)
            {  fr->info=Anewobject(AOTP_INFO,
                  AOINF_Frame,(Tag)fr,
                  TAG_END);
               Asetattrs(fr->copy,AOCPY_Info,(Tag)fr->info,TAG_END);
            }
            else if(!tag->ti_Data && fr->info)
            {  Asetattrs(fr->copy,AOCPY_Info,(Tag)NULL,TAG_END);
               Adisposeobject(fr->info);
               fr->info=NULL;
            }
            break;
         case AOFRM_Cancelcopy:
            if((void *)tag->ti_Data==fr->inputcopy)
            {  Adisposeobject(fr->inputcopy);
               fr->inputcopy=NULL;
            }
            break;
         case AOFRM_Reloadverify:
            SETFLAG(fr->flags,FRMF_RELOADVERIFY,tag->ti_Data);
            break;
         case AOFRM_Jdocument:
            fr->jdscope=(struct Jobject *)tag->ti_Data;
            break;
         case AOBJ_Nobackground:
            SETFLAG(fr->flags,FRMF_NOBACKGROUND,tag->ti_Data);
            break;
         case AOAPP_Overlapsetting:
            if(tag->ti_Data) Resizeframe(fr,fr->elt.aow,fr->elt.aoh);
            break;
         case AOURL_Clientpull:
            Setclientpull(fr,(UBYTE *)tag->ti_Data);
            break;
         case AOFRM_Timercpready:
            Doclientpull(fr);
            break;
         case AOFRM_Timertoready:
            timeout=TRUE;
            break;
         case AOBJ_Target:
            target=(void *)tag->ti_Data;
            break;
         case AOPUP_Inquire:
            if(!(fr->flags&FRMF_TOPFRAME))
            {  Popupinquire(fr,(void *)tag->ti_Data);
            }
            break;
         case AOPUP_Command:
            Popupselectframe(fr,(UBYTE *)tag->ti_Data);
            break;
         case AOFRM_Jgenerated:
            if(tag->ti_Data)
            {  if(!fr->jgenerated) Queuesetmsg(fr,FQID_LOADJGENERATED);
               fr->jgenerated=(struct Buffer *)tag->ti_Data;
            }
            break;
         case AOBJ_Queueid:
            switch(tag->ti_Data)
            {  case FQID_LOADJGENERATED:
                  Loadjgenerated(fr);
                  break;
               case FQID_INPUTWINDOC:
                  /* Only process after we got the userdata */
                  qinputwindoc=TRUE;
                  break;
               case FQID_HISTORY:
                  /* Only process after we got the userdata */
                  qhistory=TRUE;
            }
            break;
         case AOBJ_Queuedata:
            if(qinputwindoc && tag->ti_Data)
            {  struct Queuedinputwindoc *qi=(struct Queuedinputwindoc *)tag->ti_Data;
               Inputwindoc(fr->win,qi->url,qi->fragment,qi->id?qi->id:fr->id);
               if(qi->fragment) FREE(qi->fragment);
               if(qi->id) FREE(qi->id);
               FREE(qi);
            }
            else if(qhistory && tag->ti_Data)
            {  Asetattrs(fr->win,
                  AOWIN_Status,(Tag)NULL,
                  AOWIN_Hiswinhis,tag->ti_Data,
                  TAG_END);
            }
            qinputwindoc=FALSE;
            qhistory=FALSE;
            break;
         case AOFRM_Jsopen:
            SETFLAG(fr->flags,FRMF_JSOPEN,tag->ti_Data);
            break;
         case AOFRM_Onfocus:
            fr->onfocus=(UBYTE *)tag->ti_Data;
            if(fr->jobject && fr->onfocus)
            {
                struct Jcontext *jc = (struct Jcontext *)Agetattr(Aweb(),AOAPP_Jcontext);
                Jaddeventhandler(jc,fr->jobject,"onfocus",fr->onfocus);
            }
            break;
         case AOFRM_Onblur:
            fr->onblur=(UBYTE *)tag->ti_Data;
            if(fr->jobject && fr->onblur)
            {
                struct Jcontext *jc = (struct Jcontext *)Agetattr(Aweb(),(Tag)AOAPP_Jcontext);
                Jaddeventhandler(jc,fr->jobject,"onblur",fr->onblur);
            }
            break;
         case AOFRM_Focus:
            if(tag->ti_Data)
            {  if(fr->onfocus || AWebJSBase)
               {  Runjavascript(fr,awebonfocus,&fr->jobject);
               }
            }
            else
            {  if(fr->onblur || AWebJSBase)
               {  Runjavascript(fr,awebonblur,&fr->jobject);
               }
            }
            break;
         case AOFRM_Jprotect:
            Jprotframe(fr,tag->ti_Data);
            break;
         case AOBJ_Jscancel:
            if(tag->ti_Data)
            {  Cleartimeouts(fr);
               if(fr->copy) Asetattrs(fr->copy,AOBJ_Jscancel,TRUE,TAG_END);
            }
            break;
         case AOFRM_Reposfragment:
            if(tag->ti_Data)
            {  ULONG history=0,fragment=0;
               Agetattrs(fr->whis,
                  AOWHS_Frameid,(Tag)fr->id,
                  AOWHS_History,(Tag)&history,
                  AOWHS_Fragment,(Tag)&fragment,
                  TAG_END);
               if(fragment && !history) fr->flags|=FRMF_USEFRAGMENT;
            }
            break;
         case AOFRM_Prepreset:
            SETFLAG(fr->flags,FRMF_RESET,tag->ti_Data);
            if(fr->flags&FRMF_INLINE)
            {  fr->flags=(fr->flags&~(FRMF_PIXELWIDTH|FRMF_PIXELHEIGHT))|fr->sflags;
               fr->width=fr->swidth;
               fr->height=fr->sheight;
            }
            break;
         case AOFRM_Resetframe:
            if(tag->ti_Data)
            {  /* Reset our aow,aoh so a Resize will be triggered. */
               fr->elt.aow=0;
               fr->elt.aoh=0;
            }
            break;
      }
   }
   Amethodas(AOTP_ELEMENT,fr,AOM_SET,(Tag)ams->tags);
   if(timeout) Triggertimeout(fr,target);
   if(name || !fr->name) Addframename(fr,name);
   if(newaow!=fr->elt.aow || newaoh!=fr->elt.aoh) Resizeframe(fr,newaow,newaoh);
   if(mvis && (fr->flags&(FRMF_SCROLLING|FRMF_TOPFRAME)))
   {  if(newleft>mvis->minx) newleft=mvis->minx;
      else if(newleft+fr->w<=mvis->maxx) newleft=mvis->maxx-fr->w+1;
      if(newtop>mvis->miny) newtop=mvis->miny;
      else if(newtop+fr->h<=mvis->maxy) newtop=mvis->maxy-fr->h+1;
      scroll=TRUE;
   }
   if(scroll && (newleft!=fr->left || newtop!=fr->top))
   {  Scrollframe(fr,newleft,newtop,setscroller);
   }
   if(newwhis) Setnewwinhis(fr,newwhis,noreferer);
   return result;
}

static struct Frame *Newframe(struct Amset *ams)
{  struct Frame *fr=Allocobject(AOTP_FRAME,sizeof(struct Frame),ams);
   if(fr)
   {  NEWLIST(&fr->timeouts);
      fr->flags=FRMF_SCROLLING|FRMF_RESIZE;
      fr->width=fr->height=50;   /* Default 50% size */
      fr->mwidth=fr->mheight=3;
      fr->border=2;
      Setframe(fr,ams);
      fr->swidth=fr->width;
      fr->sheight=fr->height;
      fr->sflags=fr->flags&(FRMF_PIXELWIDTH|FRMF_PIXELHEIGHT);
      if(fr->flags&FRMF_TOPFRAME)
      {  fr->flags&=~FRMF_SCROLLING;
         fr->border=0;
         fr->mwidth=fr->mheight=9;
      }
      if(fr->flags&FRMF_SCROLLING)
      {  fr->hscroll=Anewobject(AOTP_SCROLLER,
            AOSCR_Orient,AOSCRORIENT_HORIZ,
            AOSCR_Delta,8,
            AOBJ_Cframe,(Tag)fr->frame,
            AOBJ_Target,(Tag)fr,
            AOBJ_Map,(Tag)maphscroll,
            TAG_END);
         fr->vscroll=Anewobject(AOTP_SCROLLER,
            AOSCR_Orient,AOSCRORIENT_VERT,
            AOSCR_Delta,8,
            AOBJ_Cframe,(Tag)fr->frame,
            AOBJ_Target,(Tag)fr,
            AOBJ_Map,(Tag)mapvscroll,
            TAG_END);
      }
      Aaddchild(Aweb(),(struct Aobject *)fr,AOREL_APP_USE_OVERLAP);
      fr->copy=Anewobject(AOTP_COPY,
         AOCPY_Url,(Tag)Emptyurl(),
         AOBJ_Layoutparent,(Tag)fr,
         AOBJ_Frame,(Tag)fr,
         AOBJ_Cframe,(Tag)fr,
         AOBJ_Window,(Tag)fr->win,
         TAG_END);
      Auload(Emptyurl(),AUMLF_HISTORY,NULL,NULL,fr);
   }
   return fr;
}

static long Getframe(struct Frame *fr,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   long result;
   result=AmethodasA(AOTP_ELEMENT,(struct Aobject *)fr,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Jobject:
            PUTATTR(tag,fr->jobject);
            break;
         case AOFRM_Leftpos:
            PUTATTR(tag,fr->left);
            break;
         case AOFRM_Toppos:
            PUTATTR(tag,fr->top);
            break;
         case AOFRM_Innerwidth:
            PUTATTR(tag,fr->w);
            break;
         case AOFRM_Innerheight:
            PUTATTR(tag,fr->h);
            break;
         case AOFRM_Id:
            PUTATTR(tag,fr->id);
            break;
         case AOBJ_Window:
            PUTATTR(tag,fr->win);
            break;
         case AOBJ_Winhis:
            PUTATTR(tag,fr->whis);
            break;
         case AOFRM_Url:
            {  void *url=Currenturl(fr);
               if(!url && fr->inputwhis)
               {  Agetattrs(fr->inputwhis,
                     AOWHS_Frameid,(Tag)fr->id,
                     AOWHS_Url,(Tag)&url,
                     TAG_END);
               }
               PUTATTR(tag,url);
            }
            break;
         case AOFRM_Title:
            PUTATTR(tag,Agetattr(fr->copy,AOCDV_Title));
            break;
         case AOFRM_Name:
            PUTATTR(tag,fr->name?fr->name->name:NULL);
            break;
         case AOFRM_Resizeleft:
         case AOFRM_Resizeright:
         case AOFRM_Resizetop:
         case AOFRM_Resizebottom:
            PUTATTR(tag,BOOLVAL(fr->flags&FRMF_RESIZE));
            break;
         case AOFRM_Contentheight:
            PUTATTR(tag,Agetattr(fr->copy,AOBJ_Height));
            break;
         case AOFRM_Contentwidth:
            PUTATTR(tag,Agetattr(fr->copy,AOBJ_Width));
            break;
         case AOFRM_Border:
            PUTATTR(tag,fr->border);
            break;
         case AOFRM_Topframe:
            PUTATTR(tag,BOOLVAL(fr->flags&FRMF_TOPFRAME));
            break;
         case AOBJ_Secure:
            PUTATTR(tag,Agetattr(fr->copy,AOBJ_Secure));
            break;
         case AOBJ_Isframeset:
            PUTATTR(tag,Agetattr(fr->copy,AOBJ_Isframeset));
            break;
         case AOBJ_Statustext:
            PUTATTR(tag,fr->defstatus);
            break;
         case AOFRM_Copy:
            PUTATTR(tag,fr->copy);
            break;
      }
   }
   return result;
}

static long Measureframe(struct Frame *fr,struct Ammeasure *amm)
{  long minw=Minwidth(fr);
   long w;
   if(fr->flags&FRMF_PIXELWIDTH) w=fr->width;
   else w=fr->width*amm->width/100;
   if(amm->ammr)
   {  amm->ammr->width=MAX(w,minw);
      if(fr->flags&FRMF_INLINE) amm->ammr->minwidth=amm->ammr->width;
      else amm->ammr->minwidth=minw;
      if(amm->flags&AMMF_MINHEIGHT) amm->ammr->minheight=Minheight(fr);
   }
   return 0;
}

static long Layoutframe(struct Frame *fr,struct Amlayout *aml)
{  short result;
   long newaow,newaoh,minw,minh;
   if((aml->flags&AMLF_RETRY) && (fr->elt.eltflags&ELTF_ALIGNED))
   {  if(aml->amlr)
      {  aml->amlr->result=AMLR_OK;
         aml->amlr->endx=aml->startx;
      }
   }
   else
   {  fr->elt.eltflags&=~ELTF_ALIGNED;
      if(aml->flags&AMLF_BREAK)
      {  result=AMLR_OK;
      }
      else
      {  minw=Minwidth(fr);
         minh=Minheight(fr);
         if(fr->flags&FRMF_PIXELWIDTH) newaow=fr->width;
         else newaow=fr->width*aml->width/100;
         if(newaow<minw) newaow=minw;
         if(fr->flags&FRMF_PIXELHEIGHT) newaoh=fr->height;
         else newaoh=fr->height*aml->height/100;
         if(newaoh<minh) newaoh=minh;
//printf("Layoutframe %08x\n",fr);
         fr->elt.aox=aml->startx;
         fr->elt.aoy=0;  /* For frameset compatibility */
         if(fr->elt.aox+newaow>aml->width && !(aml->flags&AMLF_FORCE))
         {  result=AMLR_NOFIT;
         }
         else
         {  if(newaow!=fr->elt.aow || newaoh!=fr->elt.aoh
            || (fr->flags&FRMF_RESET))
            {  Resizeframe(fr,newaow,newaoh);
               fr->flags&=~FRMF_RESET;
            }
            if(fr->elt.halign&HALIGN_FLOATLEFT)
            {  result=AMLR_FLOATING;
            }
            else
            {  result=AMLR_OK;
            }
         }
      }
      if(aml->amlr)
      {  aml->amlr->result=result;
         aml->amlr->endx=fr->elt.aox+fr->elt.aow;
         switch(fr->elt.valign)
         {  case VALIGN_BOTTOM:
               aml->amlr->above=fr->elt.aoh;
               break;
            case VALIGN_MIDDLE:
               aml->amlr->above=fr->elt.aoh/2;
               aml->amlr->below=fr->elt.aoh-aml->amlr->above;
               break;
            case VALIGN_TOP:
               aml->amlr->toph=fr->elt.aoh;
               break;
         }
      }
   }
   return 0;
}

static long Hittestframe(struct Frame *fr,struct Amhittest *amh)
{  long result=0,popup=0;
   struct Coords coords={0},*coo,co2;
   long x,y;
   void *url;
   UBYTE newhittype=0;
   coo=amh->coords;
   if(!coo)
   {  if(fr->frame)
      {  Framecoords(fr->frame,&coords);
      }
      else
      {  coords.win=fr->win;
         coords.rp=(struct RastPort *)Agetattr(fr->win,AOWIN_Rastport);
         coords.dri=(struct DrawInfo *)Agetattr(Aweb(),AOAPP_Drawinfo);
         coords.maxx=coords.maxy=AMRMAX;
      }
      coo=&coords;
   }
   if(coo->win && coo->minx<=coo->maxx && coo->miny<=coo->maxy)
   {  x=amh->xco-coo->dx;
      y=amh->yco-coo->dy;
      /* Check our scrollers */
      if(fr->vscroll && (fr->flags&FRMF_VSCROLL)
       && x>=fr->vsx && x<fr->vsx+fr->vsw && y>=fr->vsy && y<fr->vsy+fr->vsh)
      {  result=Ahittest(fr->vscroll,coo,amh->xco,amh->yco,amh->flags,amh->oldobject,amh->amhr);
         if(result) Asetattrs(fr->win,AOWIN_Keepselection,TRUE,TAG_END);
      }
      else if(fr->hscroll && (fr->flags&FRMF_HSCROLL)
       && x>=fr->hsx && x<fr->hsx+fr->hsw && y>=fr->hsy && y<fr->hsy+fr->hsh)
      {  result=Ahittest(fr->hscroll,coo,amh->xco,amh->yco,amh->flags,amh->oldobject,amh->amhr);
         if(result) Asetattrs(fr->win,AOWIN_Keepselection,TRUE,TAG_END);
      }
      /* Check our contents */
      else if(fr->copy && x>=fr->elt.aox+fr->x && x<fr->elt.aox+fr->x+fr->w
       && y>=fr->elt.aoy+fr->y && y<fr->elt.aoy+fr->y+fr->h)
      {  co2=*coo;
         Newcoords(fr,&co2);
         result=Ahittest(fr->copy,&co2,amh->xco,amh->yco,amh->flags,amh->oldobject,amh->amhr);
         /* If no object wants this hit, use our default status */
         if(!result && fr->defstatus)
         {  result=AMHR_STATUS;
            newhittype=FHIT_DEFSTATUS;
            if(amh->amhr)
            {  amh->amhr->object=fr;
            }
         }
         /* if object wants hit but didn't set a status text, use default status */
         else if(result==AMHR_NEWHIT && amh->amhr && !amh->amhr->text && fr->defstatus)
         {  amh->amhr->text=Dupstr(fr->defstatus,-1);
         }
      }
      /* If popup and no result yet, check our top left corner */
      if(!result && !(fr->flags&FRMF_TOPFRAME) && fr->copy
       /* && x>=fr->elt.aox && x<fr->elt.aox+POPUPAREA && y>=fr->elt.aoy && y<fr->elt.aoy+POPUPAREA */ )
      {  /* Allow RMB popup on next event */
         popup=AMHR_POPUP;
         if(amh->flags&AMHF_POPUP)
         {  if(amh->oldobject==(struct Aobject *)fr && fr->hittype==FHIT_CORNER)
            {  result=AMHR_OLDHIT;
               newhittype=FHIT_CORNER;
            }
            else
            {  result=AMHR_NEWHIT;
               newhittype=FHIT_CORNER;
               if(amh->amhr)
               {  amh->amhr->object=fr;
                  url=Currenturl(fr);
                  amh->amhr->text=Dupstr((UBYTE *)Agetattr(url,AOURL_Url),-1);
               }
            }
         }
      }
      /* Check resize */
      if(!result && (fr->flags&FRMF_INLINE) && (fr->flags&FRMF_RESIZE))
      {  UWORD ptrtype=0;
         if((x>=fr->elt.aox && x<fr->elt.aox+fr->border+2)
         || (x>=fr->elt.aox+fr->elt.aow-fr->border-2 && x<fr->elt.aox+fr->elt.aow))
         {  ptrtype=APTR_RESIZEHOR;
         }
         if((y>=fr->elt.aoy && y<fr->elt.aoy+fr->border+2)
         || (y>=fr->elt.aoy+fr->elt.aoh-fr->border-2 && y<fr->elt.aoy+fr->elt.aoh))
         {  ptrtype=APTR_RESIZEVERT;
         }
         if(ptrtype)
         {  if(amh->oldobject==(struct Aobject *)fr && fr->hittype==FHIT_RESIZE)
            {  result=AMHR_OLDHIT;
               newhittype=FHIT_RESIZE;
            }
            else
            {  result=AMHR_NEWHIT;
               newhittype=FHIT_RESIZE;
               if(x<fr->elt.aox+fr->border+2) fr->sizeside=FSIDE_LEFT;
               else if(x>=fr->elt.aox+fr->elt.aow-fr->border-2) fr->sizeside=FSIDE_RIGHT;
               else if(y<fr->elt.aoy+fr->border+2) fr->sizeside=FSIDE_TOP;
               else fr->sizeside=FSIDE_BOTTOM;
               if(amh->amhr)
               {  amh->amhr->object=fr;
                  amh->amhr->text=Dupstr(AWEBSTR(MSG_AWEB_FRAME_RESIZE),-1);
                  amh->amhr->ptrtype=ptrtype;
               }
            }
         }
      }
      fr->hittype=newhittype;
      if(amh->amhr)
      {  if(!amh->amhr->focus) amh->amhr->focus=fr;
      }
   }
   return result|popup;
}

static long Goactiveframe(struct Frame *fr,struct Amgoactive *amg)
{  if(fr->hittype==FHIT_RESIZE)
   {  fr->sizex=fr->elt.aox;
      fr->sizey=fr->elt.aoy;
      fr->sizew=fr->elt.aow;
      fr->sizeh=fr->elt.aoh;
      Rendersize(fr,NULL);
   }
   return AMR_ACTIVE;
}

static long Handleinputframe(struct Frame *fr,struct Aminput *ami)
{  long result=AMR_NOCARE;
   struct Coords coords={0},*coo;
   long x,y,w,h,minw,minh;
   if(ami->imsg && !(fr->flags&FRMF_TOPFRAME))
   {  if(fr->hittype==FHIT_RESIZE)
      {  switch(ami->imsg->Class)
         {  case IDCMP_MOUSEMOVE:
               coo=Clipcoords(fr->frame,NULL);
               x=fr->sizex;
               y=fr->sizey;
               w=fr->sizew;
               h=fr->sizeh;
               minw=Minwidth(fr);
               minh=Minheight(fr);
               switch(fr->sizeside)
               {  case FSIDE_LEFT:
                     x=MIN(ami->imsg->MouseX-coo->dx,fr->elt.aox+fr->elt.aow-minw);
                     w=fr->sizex+fr->sizew-x;
                     break;
                  case FSIDE_RIGHT:
                     w=MAX(ami->imsg->MouseX-coo->dx-fr->sizex+1,minw);
                     break;
                  case FSIDE_TOP:
                     y=MIN(ami->imsg->MouseY-coo->dy,fr->elt.aoy+fr->elt.aoh-minh);
                     h=fr->sizey+fr->sizeh-y;
                     break;
                  case FSIDE_BOTTOM:
                     h=MAX(ami->imsg->MouseY-coo->dy-fr->sizey+1,minh);
                     break;
               }
               if(x!=fr->sizex || y!=fr->sizey || w!=fr->sizew || h!=fr->sizeh)
               {  Rendersize(fr,coo);
                  fr->sizex=x;
                  fr->sizey=y;
                  fr->sizew=w;
                  fr->sizeh=h;
                  Rendersize(fr,coo);
               }
               Unclipcoords(coo);
               result=AMR_ACTIVE;
            case IDCMP_MOUSEBUTTONS:
               if(ami->imsg->Code==SELECTUP)
               {  fr->width=fr->sizew;
                  fr->height=fr->sizeh;
                  fr->flags|=FRMF_PIXELWIDTH;
                  fr->flags|=FRMF_PIXELHEIGHT;
                  Asetattrs(fr->layoutparent,AOBJ_Changedchild,(Tag)fr,TAG_END);
                  result=AMR_NOREUSE;
               }
               break;
         }
      }
      else  /* Popup */
      {  switch(ami->imsg->Class)
         {  case IDCMP_MOUSEMOVE:
            case IDCMP_RAWKEY:
               result=AMR_REUSE;
               if(ami->flags&AMHF_POPUP)
               {  Framecoords(fr->frame,&coords);
                  x=ami->imsg->MouseX-coords.dx;
                  y=ami->imsg->MouseY-coords.dy;
                  if(x>=fr->elt.aox && x<fr->elt.aox+POPUPAREA && y>=fr->elt.aoy && y<fr->elt.aoy+POPUPAREA)
                  {  result=AMR_ACTIVE;
                  }
               }
               break;
            case IDCMP_MOUSEBUTTONS:
               if(ami->flags&AMHF_POPUPREL)
               {  fr->popup=Anewobject(AOTP_POPUP,
                     AOPUP_Target,(Tag)fr,
                     AOPUP_Left,ami->imsg->MouseX,
                     AOPUP_Top,ami->imsg->MouseY,
                     AOPUP_Window,(Tag)fr->win,
                     TAG_END);
               }
               result=AMR_NOREUSE;
               break;
         }
      }
   }
   return result;
}

static long Goinactiveframe(struct Frame *fr)
{  if(fr->hittype==FHIT_RESIZE)
   {  Rendersize(fr,NULL);
      fr->hittype=0;
      Doupdateframes();
   }
   return 0;
}

static long Moveframe(struct Frame *fr,struct Ammove *amm)
{  long result;
   if(fr->hscroll)
   {  AmethodA(fr->hscroll,(struct Amessage *)amm);
      fr->hsx+=amm->dx;
      fr->hsy+=amm->dy;
   }
   if(fr->vscroll)
   {  AmethodA(fr->vscroll,(struct Amessage *)amm);
      fr->vsx+=amm->dx;
      fr->vsy+=amm->dy;
   }
   result=AmethodasA(AOTP_ELEMENT,(struct Aobject *)fr,(struct Amessage *)amm);
   return result;
}

static long Notifyframe(struct Frame *fr,struct Amnotify *amn)
{  long result=0;
   BOOL forward=FALSE;
   struct TagItem *tag,*tstate;
   struct Aobject *target;
   /* First process the message */
   AmethodA((struct Aobject *)fr,amn->nmsg);
   /* Then forward it unless it was a SET of AOFRM_Updatecopy, since this
    * has been handled by our processing already. */
   if(amn->nmsg->method==AOM_SET)
   {  tstate=((struct Amset *)amn->nmsg)->tags;
      while(tag=NextTagItem(&tstate))
      {  switch(tag->ti_Tag)
         {  case AOFRM_Updatecopy:
               break;
            default:
               forward=TRUE;
         }
      }
   }
   else if(amn->nmsg->method==AOM_GETREXX)
   {  target=((struct Amgetrexx *)amn->nmsg)->frame;
      forward=(!target || target==(struct Aobject *)fr);
   }
   else forward=TRUE;
   if(forward)
   {  if(fr->copy) result=AmethodA((struct Aobject *)fr->copy,(struct Amessage *)amn);
   }
   return result;
}

static long Searchposframe(struct Frame *fr,struct Amsearch *ams)
{  if(ams->flags&AMSF_CURRENTPOS)
   {  ams->top=fr->top;
   }
   AmethodA(fr->copy,(struct Amessage *)ams);
   return 0;
}

static long Searchsetframe(struct Frame *fr,struct Amsearch *ams)
{  Asetattrs(fr->win,AOWIN_Clearselection,TRUE,TAG_END);
   AmethodA(fr->copy,(struct Amessage *)ams);
   if(ams->top>=0 && !(ams->flags&AMSF_UNHIGHLIGHT))
   {  Asetattrs((struct Aobject *)fr,
         AOFRM_Leftpos,ams->left-fr->w/3,
         AOFRM_Toppos,ams->top,
         AOFRM_Setscroller,TRUE,
         TAG_END);
   }
   return 0;
}

static long Dragtestframe(struct Frame *fr,struct Amdragtest *amd)
{  struct Coords coords={0},*coo,co2;
   long x,y;
   long result=0;
   coo=amd->coords;
   if(!coo)
   {  if(fr->frame)
      {  Framecoords(fr->frame,&coords);
      }
      else
      {  coords.win=fr->win;
         coords.rp=(struct RastPort *)Agetattr(fr->win,AOWIN_Rastport);
         coords.dri=(struct DrawInfo *)Agetattr(Aweb(),AOAPP_Drawinfo);
         coords.maxx=coords.maxy=AMRMAX;
      }
      coo=&coords;
   }
   if(coo->win && coo->minx<=coo->maxx && coo->miny<=coo->maxy)
   {  x=amd->xco-coo->dx;
      y=amd->yco-coo->dy;
      if(fr->copy && x>=fr->elt.aox+fr->x && x<fr->elt.aox+fr->x+fr->w
       && y>=fr->elt.aoy+fr->y && y<fr->elt.aoy+fr->y+fr->h)
      {  co2=*coo;
         Newcoords(fr,&co2);
         result=Adragtest(fr->copy,&co2,amd->xco,amd->yco,amd->amdr);
      }
   }
   return result;
}

static long Dragrenderframe(struct Frame *fr,struct Amdragrender *amdp)
{  struct Coords coords={0},co2,*coo;
   struct RastPort *rp;
   struct Amdragrender amd;
   ULONG clipkey;
   coo=amdp->coords;
   if(!coo)
   {  if(fr->frame)
      {  Framecoords(fr->frame,&coords);
      }
      else
      {  coords.win=fr->win;
         coords.rp=(struct RastPort *)Agetattr(fr->win,AOWIN_Rastport);
         coords.dri=(struct DrawInfo *)Agetattr(Aweb(),AOAPP_Drawinfo);
         coords.maxx=coords.maxy=AMRMAX;
      }
      coo=&coords;
   }
   if(coo->rp && coo->minx<=coo->maxx && coo->miny<=coo->maxy)
   {  rp=coo->rp;
      if(fr->copy)
      {  co2=*coo;
         Newcoords(fr,&co2);
         clipkey=Clipto(co2.rp,co2.minx,co2.miny,co2.maxx,co2.maxy);
         amd=*amdp;
         amd.coords=&co2;
         AmethodA(fr->copy,(struct Amessage *)&amd);
         amdp->state=amd.state;
         Unclipto(clipkey);
      }
   }
   return 0;
}

static long Dragcopyframe(struct Frame *fr,struct Amdragcopy *amd)
{  long result;
   result=AmethodA(fr->copy,(struct Amessage *)amd);
   return result;
}

static void Disposeframe(struct Frame *fr)
{  Aremchild(Aweb(),(struct Aobject *)fr,AOREL_APP_USE_OVERLAP);
   if(fr->popup) Adisposeobject(fr->popup);
   if(fr->win)
   {  Asetattrs(fr->win,
         AOWIN_Goinactive,(Tag)fr,
         AOWIN_Nofocus,(Tag)fr,
         TAG_END);
   }
   if(fr->search) Adisposeobject(fr->search);
   if(fr->copy) Adisposeobject(fr->copy);
   if(fr->info) Adisposeobject(fr->info);
   if(fr->orgfragment) FREE(fr->orgfragment);
   if(fr->hscroll) Adisposeobject(fr->hscroll);
   if(fr->vscroll) Adisposeobject(fr->vscroll);
   if(fr->inputcopy) Adisposeobject(fr->inputcopy);
   if(fr->pulltimer) Adisposeobject(fr->pulltimer);
   if(fr->id) FREE(fr->id);
   if(fr->defstatus) FREE(fr->defstatus);
   if(fr->qifragment) FREE(fr->qifragment);
   Delframename(fr);
   Freejframe(fr);
   Queuesetmsg(fr,0);
   Amethodas(AOTP_ELEMENT,fr,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Frame_Dispatcher,
struct Frame *,fr,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newframe((struct Amset *)amsg);
         break;
      case AOM_SET:
      case AOM_UPDATE:
         result=Setframe(fr,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getframe(fr,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measureframe(fr,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutframe(fr,(struct Amlayout *)amsg);
         break;
      case AOM_RENDER:
         result=Renderframe(fr,(struct Amrender *)amsg);
         break;
      case AOM_HITTEST:
         result=Hittestframe(fr,(struct Amhittest *)amsg);
         break;
      case AOM_GOACTIVE:
         result=Goactiveframe(fr,(struct Amgoactive *)amsg);
         break;
      case AOM_HANDLEINPUT:
         result=Handleinputframe(fr,(struct Aminput *)amsg);
         break;
      case AOM_GOINACTIVE:
         result=Goinactiveframe(fr);
         break;
      case AOM_MOVE:
         result=Moveframe(fr,(struct Ammove *)amsg);
         break;
      case AOM_NOTIFY:
         result=Notifyframe(fr,(struct Amnotify *)amsg);
         break;
      case AOM_SEARCHPOS:
         result=Searchposframe(fr,(struct Amsearch *)amsg);
         break;
      case AOM_SEARCHSET:
         result=Searchsetframe(fr,(struct Amsearch *)amsg);
         break;
      case AOM_DRAGTEST:
         result=Dragtestframe(fr,(struct Amdragtest *)amsg);
         break;
      case AOM_DRAGRENDER:
         result=Dragrenderframe(fr,(struct Amdragrender *)amsg);
         break;
      case AOM_DRAGCOPY:
         result=Dragcopyframe(fr,(struct Amdragcopy *)amsg);
         break;
      case AOM_JSETUP:
         result=Jsetupframe(fr,(struct Amjsetup *)amsg);
         break;
      case AOM_DISPOSE:
         Disposeframe(fr);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         AmethodasA(AOTP_ELEMENT,(struct Aobject *)fr,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*-----------------------------------------------------------------------*/

BOOL Installframe(void)
{  NEWLIST(&framenames);
   if(!Amethod(NULL,AOM_INSTALL,AOTP_FRAME,(Tag)Frame_Dispatcher)) return FALSE;
   return TRUE;
}

void Framecoords(struct Frame *fr,struct Coords *coo)
{  long minx,miny,maxx,maxy;
   if(fr)
   {  if(fr->elt.object.objecttype==AOTP_BODY)
      {  Bodycoords((struct Body *)fr,coo);
      }
      else
      {  if(fr->frame)
         {  Framecoords(fr->frame,coo);
         }
         else if(fr->win)
         {  coo->win=fr->win;
            coo->rp=(struct RastPort *)Agetattr(fr->win,AOWIN_Rastport);
            coo->dri=(struct DrawInfo *)Agetattr(Aweb(),AOAPP_Drawinfo);
            coo->dx=coo->dy=0;
            coo->minx=coo->miny=0;
            coo->maxx=coo->maxy=AMRMAX;
         }
         minx=fr->elt.aox+coo->dx+fr->x;
         maxx=minx+fr->w-1;
         miny=fr->elt.aoy+coo->dy+fr->y;
         maxy=miny+fr->h-1;
         if(minx>coo->minx) coo->minx=minx;
         if(miny>coo->miny) coo->miny=miny;
         if(maxx<coo->maxx) coo->maxx=maxx;
         if(maxy<coo->maxy) coo->maxy=maxy;
         coo->dx+=fr->elt.aox+fr->x-fr->left;
         coo->dy+=fr->elt.aoy+fr->y-fr->top;
         Setcolors(fr,coo);
      }
   }
}

void Erasebg(struct Frame *fr,struct Coords *coo,long xmin,long ymin,long xmax,long ymax)
{  struct Backfillinfo bfinfo={0};
   long bmw,bmh;
   if(fr && fr->elt.object.objecttype==AOTP_BODY)
   {  fr=Bodyframe((struct Body *)fr);
   }
   xmin+=coo->dx;
   ymin+=coo->dy;
   xmax+=coo->dx;
   ymax+=coo->dy;
   if(xmin<=coo->maxx && xmax>=coo->minx && ymin<=coo->maxy && ymax>=coo->miny)
   {  bfinfo.window=(struct Window *)Agetattr(coo->win,AOWIN_Window);
      bfinfo.rp=coo->rp;
      bfinfo.frame=fr;
      bfinfo.coo=coo;
      if(coo->bgimage && fr && prefs.browser.docolors)
      {  Agetattrs(coo->bgimage,
            AOCDV_Imagebitmap,(Tag)&bfinfo.bitmap,
            AOCDV_Imagemask,(Tag)&bfinfo.mask,
            AOCDV_Alpha,(Tag)&bfinfo.alpha,
            AOCDV_Imagewidth,(Tag)&bmw,
            AOCDV_Imageheight,(Tag)&bmh,
            TAG_END);
         if(coo->bgalign)   /* If we got alignment info */
         {
            void *tc = NULL;

            /* If bgalign is a body and is owned by table cell */

            if( (((struct Aobject *)coo->bgalign)->objecttype == AOTP_BODY) &&
                (tc = (void *)Agetattr(coo->bgalign,AOBDY_Tcell))
              )
            {

                ULONG tabx,taby;

                /* Find our parent and it's coords*/
                struct Aobject *tab = (struct Aobject *)Agetattr(coo->bgalign,AOBJ_Layoutparent);

                Agetattrs(tab,
                    AOBJ_Left,(Tag)&tabx,
                    AOBJ_Top,(Tag)&taby,
                    TAG_END);

                /* find the coords of the cell which owns us relative to the parent */
                Gettcellcoords(tc, &bfinfo.aox, &bfinfo.aoy);

                bfinfo.aox += tabx;
                bfinfo.aoy += taby;

            }
            else
            {
               Agetattrs(coo->bgalign,
                  AOBJ_Left,(Tag)&bfinfo.aox,
                  AOBJ_Top,(Tag)&bfinfo.aoy,
                  TAG_END);
            }


         }
         else
         {
            bfinfo.aox = 0;
            bfinfo.aoy = 0;
         }

         bfinfo.bmw=bmw;
         bfinfo.bmh=bmh;
      }
      bfinfo.bgpen=coo->bgcolor;
      Installbg(&bfinfo);
      EraseRect(bfinfo.rp,xmin,ymin,xmax,ymax);
      Uninstallbg(&bfinfo);
   }
}


struct RastPort *Obtainbgrp(struct Frame *fr,struct Coords *coo,
   long xmin,long ymin,long xmax,long ymax)
{  struct Coords coords={0};
   struct Backfillinfo bfinfo={0};
   struct BitMap *bitmap;
   struct RastPort *rp=NULL;
   struct Screen *screen;
   long bmw,bmh;
   if(fr && fr->elt.object.objecttype==AOTP_BODY)
   {  fr=Bodyframe((struct Body *)fr);
   }
   if((screen=(struct Screen *)Agetattr(Aweb(),AOAPP_Screen))
   && (bitmap=AllocBitMap(xmax-xmin+1,ymax-ymin+1,Agetattr(Aweb(),AOAPP_Screendepth),
         BMF_MINPLANES,screen->RastPort.BitMap)))
   {  if(rp=ALLOCSTRUCT(RastPort,1,MEMF_CLEAR))
      {  InitRastPort(rp);
         rp->BitMap=bitmap;
         coords.dx=-xmin;
         coords.dy=-ymin;
         coords.minx=0;
         coords.miny=0;
         coords.maxx=xmax-xmin;
         coords.maxy=ymax-ymin;
         bfinfo.rp=rp;
         bfinfo.coo=&coords;
         if(coo->bgimage && fr && prefs.browser.docolors)
         {  Agetattrs(coo->bgimage,
               AOCDV_Imagebitmap,(Tag)&bfinfo.bitmap,
               AOCDV_Imagemask,(Tag)&bfinfo.mask,
               AOCDV_Imagewidth,(Tag)&bmw,
               AOCDV_Imageheight,(Tag)&bmh,
               TAG_END);

             if(coo->bgalign)   /* If we got alignment info */
             {
                void *tc = NULL;

                /* If bgalign is a body and is owned by table cell */

                if( (((struct Aobject *)coo->bgalign)->objecttype == AOTP_BODY) &&
                    (tc = (void *)Agetattr(coo->bgalign,AOBDY_Tcell))
                  )
                {

                    ULONG tabx,taby;

                    /* Find our parent and it's coords*/
                    struct Aobject  *tab = (struct Aobject *)Agetattr(coo->bgalign,AOBJ_Layoutparent);

                    Agetattrs(tab,
                        AOBJ_Left,(Tag)&tabx,
                        AOBJ_Top,(Tag)&taby,
                        TAG_END);

                    /* find the coords of the cell which owns us relative to the parent */
                    Gettcellcoords(tc, &bfinfo.aox, &bfinfo.aoy);

                    bfinfo.aox += tabx;
                    bfinfo.aoy += taby;

                }
                else
                {
                   Agetattrs(coo->bgalign,
                      AOBJ_Left,(Tag)&bfinfo.aox,
                      AOBJ_Top,(Tag)&bfinfo.aoy,
                      TAG_END);
                }


             }
             else
             {
                bfinfo.aox = 0;
                bfinfo.aoy = 0;
             }

            bfinfo.bmw=bmw;
            bfinfo.bmh=bmh;

         }

         bfinfo.bgpen=coo->bgcolor;
         if(bfinfo.bitmap)
         {  Drawbackground(rp,&bfinfo,0,0,xmax-xmin,ymax-ymin);
         }
         else
         {  SetAPen(rp,coo->bgcolor);
            RectFill(rp,0,0,xmax-xmin,ymax-ymin);
         }
      }
      else FreeBitMap(bitmap);
   }
   return rp;
}

void Releasebgrp(struct RastPort *rp)
{  if(rp)
   {  if(rp->BitMap)
      {  FreeBitMap(rp->BitMap);
      }
      FREE(rp);
   }
}

/* Target frame, but only optionally open new window if no frame found */
struct Frame *Targetframeoptnew(struct Frame *fr,UBYTE *name,BOOL opennew)
{  void *target=fr,*t2=NULL;
   struct Framename *frn;
   void *win;
   if(name)
   {  if(*name=='#')
      {  short i;
         UBYTE *p,*id;
         for(i=0,p=name;p;p=strchr(p+1,'.')) i++;
         if(id=ALLOCTYPE(UBYTE,i+1,MEMF_CLEAR))
         {  for(i=0,p=name;p;i++,p=strchr(p+1,'.'))
            {  id[i]=atoi(p+1);
            }
            if(i==1 && id[0]==0)
            {  /* #0 is top frame */
               while(fr->frame) fr=fr->frame;
               target=fr;
            }
            else
            {  /* return NULL if id not found */
               target=NULL;
               for(frn=framenames.first;frn->next;frn=frn->next)
               {  if(frn->frame->win==fr->win && frn->frame->id && STREQUAL(frn->frame->id,id))
                  {  target=frn->frame;
                     break;
                  }
               }
            }
            FREE(id);
         }
      }
      else
      {  if(STREQUAL(name,"_top") || STREQUAL(name,"_body"))
         {  while(fr->frame) fr=fr->frame;
            target=fr;
         }
         else if(STREQUAL(name,"_parent"))
         {  if(fr->frame) fr=fr->frame;
            target=fr;
         }
         else if(STREQUAL(name,"_blank") || STREQUAL(name,"_new"))
         {
            if(opennew)
            {
               if(win=Anewobject(AOTP_WINDOW,
                  AOWIN_Noproxy,Agetattr(fr->win,AOWIN_Noproxy),
                  TAG_END))
               {  target=(void *)Agetattr(win,AOBJ_Frame);
               }
            }
            else
            {  target=NULL;
            }
         }
         else if(STREQUAL(name,"_self"))
         {  target=fr;
         }
         else if(STREQUAL(name,""))
         {  target=fr;
         }
         else
         {  target=NULL;
            for(frn=framenames.first;frn->next;frn=frn->next)
            {  if(frn->frame->win && frn->name && STREQUAL(frn->name,name))
               {  if(frn->frame->win==fr->win)
                  {  target=frn->frame;
                     break;
                  }
                  else
                  {  t2=frn->frame;
                  }
               }
            }
            if(!target)
            {  if(t2) target=t2;
               else if(opennew)
               {
                  Asetattrs(Aweb(),AOAPP_Iconify,FALSE,TAG_END);
                  if(win=Anewobject(AOTP_WINDOW,
                     AOWIN_Name,(Tag)name,
                     AOWIN_Noproxy,Agetattr(fr->win,AOWIN_Noproxy),
                     TAG_END))
                  {  target=(void *)Agetattr(win,AOBJ_Frame);
                  }
                  if(!target) target=fr;
               }
            }
         }
      }
   }
   return target;
}

void *Targetframe(struct Frame *fr,UBYTE *name)
{  return Targetframeoptnew(fr,name,TRUE);
}

UBYTE *Rexxframeid(struct Frame *fr)
{  UBYTE *buf,*p;
   short i;
   if(fr->id)
   {  if(buf=ALLOCTYPE(UBYTE,4*strlen(fr->id)+2,0))
      {  for(i=0,p=buf;fr->id[i];i++)
         {  p+=sprintf(p,"%c%d",i?'.':'#',fr->id[i]);
         }
         *p='\0';
      }
   }
   else
   {  buf=Dupstr("",-1);
   }
   return buf;
}
