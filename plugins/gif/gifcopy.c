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

/* gifcopy.c - AWebgif copydriver */

#include "pluginlib.h"
#include "awebgif.h"
#include "libraries/awebsupport.h"
#include "libraries/awebclib.h"
#include <cybergraphx/cybergraphics.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/scale.h>

#undef NO_INLINE_STDARG
#include <proto/awebsupport.h>
#define NO_INLINE_STDARG
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/cybergraphics.h>

/*--------------------------------------------------------------------*/
/* General data structures                                            */
/*--------------------------------------------------------------------*/

struct Gifcopy
{  struct Copydriver copydriver;    /* Our superclass object instance */
   struct Aobject *copy;            /* The AOTP_COPY object we belong to */
   struct BitMap *bitmap;           /* Same as source's bitmap or our own (scaled) */
   UBYTE *mask;                     /* Transparent mask (sources or our own) */
   long width,height;               /* Dimensions of our bitmap */
   long ready;                      /* Last complete row number in (scaled) bitmap */
   long srcready;                   /* Last complete row in src bitmap */
   long swidth,sheight;             /* Suggested width and height or 0 */
   USHORT flags;                    /* See below */
   struct BitMap *srcbitmap;        /* Our source's bitmap for scaling */
   UBYTE *srcmask;                  /* Our source's mask for scaling */
   long srcwidth,srcheight;         /* Original source's height for scaling */
   struct RastPort *bgrp;           /* Rastport containing out background */
   long maxloops;                   /* Max number of loops to show */
   long loops;                      /* Number of loops shown */
};

/* Gifcopy flags: */
#define GIFCF_DISPLAYED    0x0001   /* We are allowed to render ourselves
                                     * if we have a context frame */
#define GIFCF_READY        0x0002   /* Image or frame is ready */
#define GIFCF_OURBITMAP    0x0004   /* Bitmap and mask are ours */
#define GIFCF_JSREADY      0x0008   /* Decoding is ready */
#define GIFCF_RESCALE      0x0010   /* we have a pending rescale */

/*--------------------------------------------------------------------*/
/* Misc functions                                                     */
/*--------------------------------------------------------------------*/

/* Limit coordinate (x), offset (dx), width (w) to a region (minx,maxx) */
static void Clipcopy(long *x,long *dx,long *w,long minx,long maxx)
{  if(minx>*x)
   {  (*dx)+=minx-*x;
      (*w)-=minx-*x;
      *x=minx;
   }
   if(maxx<*x+*w)
   {  *w=maxx-*x+1;
   }
}

/* Render these rows of our bitmap. */
static void Renderrows(struct Gifcopy *gc,struct Coords *coo,
   long minx,long miny,long maxx,long maxy,long minrow,long maxrow)
{  long x,y;      /* Resulting rastport x,y to blit to */
   long dx,dy;    /* Offset within bitmap to blit from */
   long w,h;      /* Width and height of portion to blit */
   coo=Clipcoords(gc->copydriver.cframe,coo);
   if(coo && coo->rp)
   {  x=gc->copydriver.aox+coo->dx;
      y=gc->copydriver.aoy+coo->dy+minrow;
      dx=0;
      dy=minrow;
      w=gc->width;
      h=maxrow-minrow+1;
      Clipcopy(&x,&dx,&w,coo->minx,coo->maxx);
      Clipcopy(&y,&dy,&h,coo->miny,coo->maxy);
      Clipcopy(&x,&dx,&w,minx+coo->dx,maxx+coo->dx);
      Clipcopy(&y,&dy,&h,miny+coo->dy,maxy+coo->dy);
      if(w>0 && h>0)
      {  if(gc->mask)
         {  BltMaskBitMapRastPort(gc->bitmap,dx,dy,coo->rp,x,y,w,h,0xe0,gc->mask);
         }
         else
         {  BltBitMapRastPort(gc->bitmap,dx,dy,coo->rp,x,y,w,h,0xc0);
         }
      }
   }
   Unclipcoords(coo);

   if(gc->flags&GIFCF_JSREADY)
   {  Asetattrs(gc->copy,AOCPY_Onimgload,TRUE,TAG_END);
   }

   /* For animated GIF images, the situation is different. The image must
    * notify its owning copy (when decoding is complete) every time the
    * animation renders its last frame.
    * This must NOT happen after receiving every AOM_RENDER message (like
    * static images), but only when the animation playback sequence hits
    * the last frame.
    *
    * Probably this is not the location in the source where it would
    * happen, but anyway here is what has to be set:
         Asetattrs(gc->copy,AOCPY_Onimganim,TRUE,TAG_END);
    */
}

/* Transparent image: clear the background and render the bitmap. The background is
 * obtained, this is copied a temporary bitmap, then the image is copied over the
 * temporary bitmap, and finally the temporary bitmap is blitted into display. */
static void Renderclearbg(struct Gifcopy *gc)
{  struct Coords *coo=NULL;
   struct RastPort temprp={0};
   int depth;
   long x,y,w,h,dx=0,dy=0;
   coo=Clipcoords(gc->copydriver.cframe,coo);
   if(coo && coo->rp && gc->mask)
   {  x=gc->copydriver.aox+coo->dx;
      y=gc->copydriver.aoy+coo->dy;
      w=gc->width;
      h=gc->height;
      Clipcopy(&x,&dx,&w,coo->minx,coo->maxx);
      Clipcopy(&y,&dy,&h,coo->miny,coo->maxy);
      if(w>0 && h>0)
      {  if(!gc->bgrp)
         {  gc->bgrp=Obtainbgrp(gc->copydriver.cframe,coo,
               gc->copydriver.aox,gc->copydriver.aoy,gc->copydriver.aox+gc->width-1,gc->copydriver.aoy+gc->height-1);
         }
         if(gc->bgrp)
         {  depth=GetBitMapAttr(gc->bgrp->BitMap,BMA_DEPTH);
            InitRastPort(&temprp);
            if(temprp.BitMap=AllocBitMap(gc->width,gc->height,depth,
               BMF_MINPLANES,gc->bgrp->BitMap))
            {  BltBitMap(gc->bgrp->BitMap,0,0,temprp.BitMap,0,0,
                  gc->width,gc->height,0xc0,0xff,NULL);
               BltMaskBitMapRastPort(gc->bitmap,0,0,&temprp,0,0,
                  gc->width,gc->height,0xe0,gc->mask);
               BltBitMapRastPort(temprp.BitMap,dx,dy,coo->rp,x,y,w,h,0xc0);
               FreeBitMap(temprp.BitMap);
            }
         }
      }
   }
   Unclipcoords(coo);
}

/* Set a new bitmap and mask. If scaling required, allocate our own bitmap
 * and mask. */
static void Newbitmap(struct Gifcopy *gc,struct BitMap *bitmap,UBYTE *mask)
{  int width,height,depth;
   ULONG memfchip=0;
   if(gc->flags&GIFCF_OURBITMAP)
   {  if(gc->bitmap) FreeBitMap(gc->bitmap);
      if(gc->mask) FreeVec(gc->mask);
      gc->flags&=~GIFCF_OURBITMAP;
   }

   if(bitmap)
   {  if(gc->swidth && !gc->sheight)
      {  gc->sheight=gc->srcheight*gc->swidth/gc->srcwidth;
         if(gc->sheight<1) gc->sheight=1;
      }
      if(gc->sheight && !gc->swidth)
      {  gc->swidth=gc->srcwidth*gc->sheight/gc->srcheight;
         if(gc->swidth<1) gc->swidth=1;
      }
   }

   if(bitmap && gc->swidth && gc->sheight
   && (gc->srcwidth!=gc->swidth || gc->srcheight!=gc->sheight))
   {  depth=GetBitMapAttr(bitmap,BMA_DEPTH);
      if(gc->bitmap=AllocBitMap(gc->swidth,gc->sheight,depth,BMF_MINPLANES,bitmap))
      {  gc->flags|=GIFCF_OURBITMAP;
         gc->mask=NULL;
         gc->srcbitmap=bitmap;
         gc->srcmask=mask;
         gc->width=gc->swidth;
         gc->height=gc->sheight;

         if(mask)
         {  if(GetBitMapAttr(bitmap,BMA_FLAGS)&BMF_STANDARD)
            {  width=gc->bitmap->BytesPerRow;
               height=gc->bitmap->Rows;
               memfchip=MEMF_CHIP;
            }
            else if(CyberGfxBase && GetCyberMapAttr(gc->bitmap,CYBRMATTR_ISCYBERGFX))
            {  width=GetCyberMapAttr(gc->bitmap,CYBRMATTR_WIDTH)/8;
               height=GetCyberMapAttr(gc->bitmap,CYBRMATTR_HEIGHT);
            }
            else width=height=0;
            if(width)
            {  gc->mask=(UBYTE *)AllocVec(width*height,memfchip|MEMF_CLEAR);
            }
         }
      }
   }

   if(bitmap && !(gc->flags&GIFCF_OURBITMAP))
   {  gc->bitmap=bitmap;
      gc->mask=mask;
      gc->width=gc->srcwidth;
      gc->height=gc->srcheight;
      gc->srcbitmap=bitmap;
      gc->srcmask=mask;
   }
   if(!(bitmap))
   {
      gc->bitmap=0;
      gc->mask=0;
   }
}

static void Scalebitmap(struct Gifcopy *gc,long sfrom,long sheight,long dfrom)
{
   struct BitScaleArgs bsa={0};
   bsa.bsa_SrcX=0;
   bsa.bsa_SrcY=sfrom;
   bsa.bsa_SrcWidth=gc->srcwidth;
   bsa.bsa_SrcHeight=sheight;
   bsa.bsa_DestX=0;
   bsa.bsa_DestY=dfrom;
   bsa.bsa_XSrcFactor=gc->srcwidth;
   bsa.bsa_XDestFactor=gc->width;
   bsa.bsa_YSrcFactor=gc->srcheight;
   bsa.bsa_YDestFactor=gc->height;
   bsa.bsa_SrcBitMap=gc->srcbitmap;
   bsa.bsa_DestBitMap=gc->bitmap;
   bsa.bsa_Flags=0;
   BitMapScale(&bsa);
   if(gc->mask)
   {  struct BitMap sbm={0},dbm={0};
      if(GetBitMapAttr(gc->bitmap,BMA_FLAGS)&BMF_STANDARD)
      {  sbm.BytesPerRow=gc->srcbitmap->BytesPerRow;
         sbm.Rows=gc->srcbitmap->Rows;
         dbm.BytesPerRow=gc->bitmap->BytesPerRow;
         dbm.Rows=gc->bitmap->Rows;
      }
      else if(CyberGfxBase)
      {  if(GetCyberMapAttr(gc->srcbitmap,CYBRMATTR_ISCYBERGFX))
         {  sbm.BytesPerRow=GetCyberMapAttr(gc->srcbitmap,CYBRMATTR_WIDTH)/8;
            sbm.Rows=GetCyberMapAttr(gc->srcbitmap,CYBRMATTR_HEIGHT);
         }
         if(GetCyberMapAttr(gc->bitmap,CYBRMATTR_ISCYBERGFX))
         {  dbm.BytesPerRow=GetCyberMapAttr(gc->bitmap,CYBRMATTR_WIDTH)/8;
            dbm.Rows=GetCyberMapAttr(gc->bitmap,CYBRMATTR_HEIGHT);
         }
      }
      if(sbm.BytesPerRow && dbm.BytesPerRow)
      {  sbm.Depth=1;
         sbm.Planes[0]=gc->srcmask;
         dbm.Depth=1;
         dbm.Planes[0]=gc->mask;
         bsa.bsa_SrcX=0;
         bsa.bsa_SrcY=sfrom;
         bsa.bsa_SrcWidth=gc->srcwidth;
         bsa.bsa_SrcHeight=sheight;
         bsa.bsa_DestX=0;
         bsa.bsa_DestY=dfrom;
         bsa.bsa_XSrcFactor=gc->srcwidth;
         bsa.bsa_XDestFactor=gc->width;
         bsa.bsa_YSrcFactor=gc->srcheight;
         bsa.bsa_YDestFactor=gc->height;
         bsa.bsa_SrcBitMap=&sbm;
         bsa.bsa_DestBitMap=&dbm;
         bsa.bsa_Flags=0;
         BitMapScale(&bsa);
      }
   }
}

/*--------------------------------------------------------------------*/
/* Plugin copydriver object dispatcher functions                      */
/*--------------------------------------------------------------------*/

static ULONG Measurecopy(struct Gifcopy *gc,struct Ammeasure *ammeasure)
{  if(gc->bitmap)
   {  gc->copydriver.aow=gc->width;
      gc->copydriver.aoh=gc->height;
      if(ammeasure->ammr)
      {  ammeasure->ammr->width=gc->copydriver.aow;
         ammeasure->ammr->minwidth=gc->copydriver.aow;
      }
   }
   return 0;
}

static ULONG Layoutcopy(struct Gifcopy *gc,struct Amlayout *amlayout)
{  if(gc->bitmap)
   {  gc->copydriver.aow=gc->width;
      gc->copydriver.aoh=gc->height;
   }
   return 0;
}

static ULONG Rendercopy(struct Gifcopy *gc,struct Amrender *amrender)
{  struct Coords *coo;
   if(gc->bitmap && !(amrender->flags&(AMRF_UPDATESELECTED|AMRF_UPDATENORMAL)))
   {  coo=Clipcoords(gc->copydriver.cframe,amrender->coords);
      if(coo && coo->rp)
      {  if(amrender->flags&AMRF_CLEAR)
         {  Erasebg(gc->copydriver.cframe,coo,amrender->rect.minx,amrender->rect.miny,
               amrender->rect.maxx,amrender->rect.maxy);
         }
         Renderrows(gc,coo,amrender->rect.minx,amrender->rect.miny,
            amrender->rect.maxx,amrender->rect.maxy,0,gc->ready);
      }
      Unclipcoords(coo);
   }
   return 0;
}

static ULONG Setcopy(struct Gifcopy *gc,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   BOOL newbitmap=FALSE;   /* Remember if we got a new bitmap */
   BOOL chgbitmap=FALSE;   /* Remember if we got a bitmap data change */
   BOOL dimchanged=FALSE;  /* If our dimenasions have changed */
   BOOL animframe=FALSE;   /* Clear bg if frame is transparent */
   BOOL rescale=FALSE;     /* Create new scaled bitmap as demanded by owner */
   long readyfrom=0,readyto=-1;
   struct BitMap *bitmap=NULL;
   UBYTE *mask=NULL;
   Amethodas(AOTP_COPYDRIVER,(struct Aobject *)gc,AOM_SET,amset->tags);
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOCDV_Copy:
            gc->copy=(struct Aobject *)tag->ti_Data;
            break;
         case AOCDV_Sourcedriver:
            break;
         case AOCDV_Displayed:
            if(tag->ti_Data) gc->flags|=GIFCF_DISPLAYED;
            else
            {  gc->flags&=~GIFCF_DISPLAYED;
               if(gc->bgrp)
               {  Releasebgrp(gc->bgrp);
                  gc->bgrp=NULL;
               }
               gc->loops=0;
            }
            break;
         case AOCDV_Width:
            if(tag->ti_Data && tag->ti_Data!=gc->width) gc->flags |= GIFCF_RESCALE;
            gc->swidth=tag->ti_Data;
            break;
         case AOCDV_Height:
            if(tag->ti_Data && tag->ti_Data!=gc->height) gc->flags |= GIFCF_RESCALE;
            gc->sheight=tag->ti_Data;
            break;
         case AOCDV_Bgchanged:
            if(tag->ti_Data && gc->bgrp)
            {  Releasebgrp(gc->bgrp);
               gc->bgrp=NULL;
            }
            break;
         case AOGIF_Bitmap:
            bitmap=(struct BitMap *)tag->ti_Data;
            newbitmap=TRUE;
            if(!gc->bitmap)
            {  gc->width=0;
               gc->height=0;
               gc->ready=-1;
               gc->flags&=~GIFCF_READY;
               dimchanged=TRUE;
            }
            break;
         case AOGIF_Mask:
            mask=(UBYTE *)tag->ti_Data;
            break;
         case AOGIF_Width:
            if(tag->ti_Data!=gc->srcwidth) dimchanged=TRUE;
            gc->srcwidth=tag->ti_Data;
            break;
         case AOGIF_Height:
            if(tag->ti_Data!=gc->srcheight) dimchanged=TRUE;
            gc->srcheight=tag->ti_Data;
            break;
         case AOGIF_Readyfrom:
            readyfrom=tag->ti_Data;
            chgbitmap=TRUE;
            break;
         case AOGIF_Readyto:
            readyto=tag->ti_Data;
            gc->srcready=readyto;
            chgbitmap=TRUE;
            break;
         case AOGIF_Imgready:
            if(tag->ti_Data) gc->flags|=GIFCF_READY;
            else gc->flags&=~GIFCF_READY;
            chgbitmap=TRUE;
            break;
         case AOGIF_Animframe:
            animframe=tag->ti_Data;
            break;
         case AOGIF_Jsready:
            if(tag->ti_Data)
            {  gc->flags|=GIFCF_JSREADY;
               Asetattrs(gc->copy,AOCPY_Onimgload,TRUE,TAG_END);
            }
            else gc->flags&=~GIFCF_JSREADY;
            break;
         case AOGIF_Jsanim:
            if(tag->ti_Data)
            {  Asetattrs(gc->copy,AOCPY_Onimganim,TRUE,TAG_END);
            }
            break;
         case AOGIF_Maxloops:
            gc->maxloops=(long)tag->ti_Data;
            break;
         case AOGIF_Newloop:
            if(tag->ti_Data) gc->loops++;
            break;
      }
   }

   if(dimchanged && gc->bgrp)
   {  Releasebgrp(gc->bgrp);
      gc->bgrp=NULL;
   }

   /* If we got a new animation frame, and we are not allowed to render ourselves,
    * forget about the new bitmap. This is to avoid continuously rerendering of
    * animated backgrounds. */
   if(newbitmap && animframe && !(gc->flags&GIFCF_DISPLAYED))
   {  newbitmap=FALSE;
      chgbitmap=FALSE;
   }

   /* If we have shown enough loops, forget about the new bitmap. */
   if(newbitmap && animframe && gc->maxloops>=0 && gc->loops>=gc->maxloops)
   {  newbitmap=FALSE;
      chgbitmap=FALSE;
   }

   /* If a new scaling is requested, set magic so that a new bitmap is allocated and
    * scaled into. */
   /* but don't bother trying to rescale if were not ready */

   if((gc->flags & GIFCF_RESCALE) && (gc->flags&GIFCF_READY))
   {
      if(!bitmap) bitmap=gc->srcbitmap;
      if(!mask) mask=gc->srcmask;
      rescale = TRUE;
      gc->flags &= ~ GIFCF_RESCALE;
   }

   if((newbitmap && bitmap!=gc->bitmap) || rescale)
   {  Newbitmap(gc,bitmap,mask);
   }
   if(((chgbitmap && readyto>=readyfrom) || rescale) && (gc->flags&GIFCF_OURBITMAP))
   {  long sfrom,sheight;
      if(gc->flags&GIFCF_READY)
      {  readyfrom=0;
         readyto=gc->height-1;
         sfrom=0;
         sheight=gc->srcheight;
         gc->ready=readyto;
      }
      else
      {  if(rescale)
         {  sfrom=0;
            sheight=gc->srcready+1;
            readyfrom=0;
         }
         else
         {  sfrom=readyfrom;
            sheight=readyto-readyfrom+1;
            if(gc->height>gc->srcheight && sfrom>0)
            {  /* Make sure there is no gap */
               sfrom--;
               sheight++;
            }
            readyfrom=sfrom*gc->height/gc->srcheight;
         }
         readyto=readyfrom+ScalerDiv(sheight,gc->height,gc->srcheight)-1;
         if(rescale) gc->ready=readyto;
      }
      Scalebitmap(gc,sfrom,sheight,readyfrom);
   }

   if(readyto>gc->ready) gc->ready=readyto;

   /* If our dimensions have changed, let our AOTP_COPY object know, and
    * eventually we will receive an AOM_RENDER message. */
   if(dimchanged)
   {  Asetattrs(gc->copy,AOBJ_Changedchild,gc,TAG_END);
   }
   /* If the bitmap was changed but the dimensions stayed the same,
    * and we are allowed to render ourselves, render the new row(s) now. */
   else if(chgbitmap && (gc->flags&GIFCF_DISPLAYED) && gc->copydriver.cframe)
   {  if(animframe && gc->mask)
      {  Renderclearbg(gc);
      }
      else
      {  Renderrows(gc,NULL,0,0,AMRMAX,AMRMAX,readyfrom,readyto);
      }
   }
   /* If we are not allowed to render ourselves, let our AOTP_COPY object
    * know when the bitmap is complete. */
   else if(chgbitmap && !(gc->flags&GIFCF_DISPLAYED) && (gc->flags&GIFCF_READY))
   {  Asetattrs(gc->copy,AOBJ_Changedchild,gc,TAG_END);
   }

   if(newbitmap && gc->flags&GIFCF_JSREADY)
   {  Asetattrs(gc->copy,AOCPY_Onimgload,TRUE,TAG_END);
   }

   /* More notes for animations (animated GIF) only:
    *
    * When decoding is completed, you must set both AOCPY_Onimgload (because
    * decoding is completed), and AOCPY_Onimganim (because only now the last
    * animation frame is fully displayed). Setting both attributes is safe,
    * provided this is done during the same invokation of this object's
    * dispatcher.
    *
    * However, if that is more convenient for the implementation, it is
    * allowed to set only AOCPY_Onimgload in this situation, and omit
    * setting of AOCPY_Onimganim this first time.
    *
    * When decoding is complete, you MUST set AOCPY_Onimgload regardless of
    * whether you set AOCPY_Onimganim this first time or not.
    */

   return 0;
}

static struct Gifcopy *Newcopy(struct Amset *amset)
{  struct Gifcopy *gc;
   if(gc=Allocobject(AwebPluginBase->copydriver,sizeof(struct Gifcopy),amset))
   {  gc->ready=-1;
      gc->maxloops=-1;
      Setcopy(gc,amset);
   }
   return gc;
}

static ULONG Getcopy(struct Gifcopy *gc,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)gc,(struct Amessage *)amset);
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOCDV_Ready:
            PUTATTR(tag,gc->bitmap?TRUE:FALSE);
            break;
         case AOCDV_Imagebitmap:
            PUTATTR(tag,(gc->flags&GIFCF_READY)?gc->bitmap:NULL);
            break;
         case AOCDV_Imagemask:
            PUTATTR(tag,(gc->flags&GIFCF_READY)?gc->mask:NULL);
            break;
         case AOCDV_Imagewidth:
            PUTATTR(tag,(gc->bitmap && gc->flags&GIFCF_READY)?gc->width:0);
            break;
         case AOCDV_Imageheight:
            PUTATTR(tag,(gc->bitmap && gc->flags&GIFCF_READY)?gc->height:0);
            break;
      }
   }
   return 0;
}

static void Disposecopy(struct Gifcopy *gc)
{  if(gc->flags&GIFCF_OURBITMAP)
   {  if(gc->bitmap) FreeBitMap(gc->bitmap);
      if(gc->mask) FreeVec(gc->mask);
   }
   if(gc->bgrp) Releasebgrp(gc->bgrp);
   Amethodas(AOTP_COPYDRIVER,(struct Aobject *)gc,AOM_DISPOSE);
}

USRFUNC_H2
(
ULONG  , Dispatchercopy,
struct Gifcopy *,gc,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  ULONG result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newcopy((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setcopy(gc,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getcopy(gc,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measurecopy(gc,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutcopy(gc,(struct Amlayout *)amsg);
         break;
      case AOM_RENDER:
         result=Rendercopy(gc,(struct Amrender *)amsg);
         break;
      case AOM_DISPOSE:
         Disposecopy(gc);
         break;
      default:
         result=AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)gc,amsg);
         break;
   }
   return result;

    USRFUNC_EXIT
}
