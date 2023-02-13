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

/* pngcopy.c - AWebpng copydriver */

#include "pluginlib.h"
#include "awebpng.h"
#include <libraries/awebsupport.h>
#include <cybergraphx/cybergraphics.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/scale.h>

#undef NO_INLINE_STDARG
#include <proto/awebsupport.h>
#define NO_INLINE_STDARG
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/cybergraphics.h>

/*--------------------------------------------------------------------*/
/* General data structures                                            */
/*--------------------------------------------------------------------*/

struct Pngcopy
{  struct Copydriver copydriver;    /* Our superclass object instance */
   struct Aobject *copy;            /* The AOTP_COPY object we belong to */
   UBYTE *argb;                     /* Same as source's bitmap or our own (scaled) */
   long width,height;               /* Dimensions of our bitmap */
   long alpha;
   long ready;                      /* Last complete row number in bitmap */
   long srcready;                   /* Last complete row in src bitmap */
   long swidth,sheight;             /* Suggested width and height or 0 */
   USHORT flags;                    /* See below */
   UBYTE *srcargb;                  /* Our source's bitmap for scaling */
   long srcwidth,srcheight;         /* Original source's height for scaling */
   struct BitMap *bitmap;           /* BitMap for background rendering */
   ULONG timestamp;
};

/* Pngcopy flags: */
#define PNGCF_DISPLAYED    0x0001   /* We are allowed to render ourselves
                                     * if we have a context frame */
#define PNGCF_READY        0x0002   /* Image is ready */
#define PNGCF_OURBITMAP    0x0004   /* Bitmap and mask are ours */
#define PNGCF_RESCALE      0x0008   /* We have a pending rescale */


#ifndef WritePixelArrayAlpha
#define WritePixelArrayAlpha(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8, __p9) \
    LP10(216, ULONG , WritePixelArrayAlpha, \
        APTR , __p0, a0, \
        UWORD , __p1, d0, \
        UWORD , __p2, d1, \
        UWORD , __p3, d2, \
        struct RastPort *, __p4, a1, \
        UWORD , __p5, d3, \
        UWORD , __p6, d4, \
        UWORD , __p7, d5, \
        UWORD , __p8, d6, \
        ULONG , __p9, d7, \
        , CYBERGRAPHICS_BASE_NAME, 0, 0, 0, 0, 0, 0)
#endif

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
static void Renderrows(struct Pngcopy *pc,struct Coords *coo,
   long minx,long miny,long maxx,long maxy,long minrow,long maxrow)
{  long x,y;      /* Resulting rastport x,y to blit to */
   long dx,dy;    /* Offset within bitmap to blit from */
   long w,h;      /* Width and height of portion to blit */

   coo=Clipcoords(pc->copydriver.cframe,coo);

   if(coo && coo->rp)
   {  x=pc->copydriver.aox+coo->dx;
      y=pc->copydriver.aoy+coo->dy+minrow;
      dx=0;
      dy=minrow;
      w=pc->width;
      h=maxrow-minrow+1;
      Clipcopy(&x,&dx,&w,coo->minx,coo->maxx);
      Clipcopy(&y,&dy,&h,coo->miny,coo->maxy);
      Clipcopy(&x,&dx,&w,minx+coo->dx,maxx+coo->dx);
      Clipcopy(&y,&dy,&h,miny+coo->dy,maxy+coo->dy);
      if(w>0 && h>0)
      {
         WritePixelArrayAlpha(pc->argb, dx, dy, pc->width*4, coo->rp, x, y, w, h, 0xffffffff);
      }
   }
   Unclipcoords(coo);
   if(pc->flags&PNGCF_READY)
   {  Asetattrs(pc->copy,AOCPY_Onimgload,TRUE,TAG_END);
   }

   /* For animated PNG images, the situation is different. The image must
    * notify its owning copy (when decoding is complete) every time the
    * animation renders its last frame.
    * This must NOT happen after receiving every AOM_RENDER message (like
    * static images), but only when the animation playback sequence hits
    * the last frame.
    *
    * Probably this is not the location in the source where it would
    * happen, but anyway here is what has to be set:
         Asetattrs(pc->copy,AOCPY_Onimganim,TRUE,TAG_END);
    */
}

/* Set a new bitmap. If scaling required, allocate our own bitmap
 * and mask. */
static void Newbitmap(struct Pngcopy *pc,UBYTE *argb)
{
   FreeBitMap(pc->bitmap);

   pc->bitmap = NULL;

   if(pc->flags&PNGCF_OURBITMAP)
   {
      FreeVecTaskPooled(pc->argb);
      pc->argb = NULL;
      pc->flags&=~PNGCF_OURBITMAP;
   }

   if(argb)
   {  if(pc->swidth && !pc->sheight)
      {  pc->sheight=pc->srcheight*pc->swidth/pc->srcwidth;
         if(pc->sheight<1) pc->sheight=1;
      }
      if(pc->sheight && !pc->swidth)
      {  pc->swidth=pc->srcwidth*pc->sheight/pc->srcheight;
         if(pc->swidth<1) pc->swidth=1;
      }
   }

   if(argb && pc->swidth && pc->sheight
   && (pc->srcwidth!=pc->swidth || pc->srcheight!=pc->sheight))
   {
      if((pc->argb=AllocVecTaskPooled(pc->swidth*pc->sheight*4)))
      {  pc->flags|=PNGCF_OURBITMAP;
         pc->srcargb=argb;
         pc->width=pc->swidth;
         pc->height=pc->sheight;
         pc->timestamp=0;
      }
   }

   if(argb && !(pc->flags&PNGCF_OURBITMAP))
   {  pc->argb=argb;
      pc->width=pc->srcwidth;
      pc->height=pc->srcheight;
      pc->timestamp=0;
   }
   if(!(argb))
   {  pc->argb = 0;
   }

}

#define SCALE_MULTIPLIER 16384

static void ScaleArray(struct Pngcopy *pc,long sfrom,long sheight,long dfrom)
{
        ULONG   yy, prev_z;
   ULONG width;
   LONG x, y, maxdst;

   width = pc->width;

   if (width > pc->srcwidth)
      x = (width * SCALE_MULTIPLIER) / pc->srcwidth;
   else
      x = (pc->srcwidth * SCALE_MULTIPLIER) / width;

   if (pc->height > pc->srcheight)
      y = (pc->height * SCALE_MULTIPLIER) / pc->srcheight;
   else
      y = (pc->srcheight * SCALE_MULTIPLIER) / pc->height;

        yy = sfrom * y;
   prev_z = dfrom ? dfrom * y / SCALE_MULTIPLIER : 1;
   maxdst = pc->height - dfrom;

        while (sheight > 0 && maxdst > 0)
        {
      maxdst--;

                if (prev_z == (yy / SCALE_MULTIPLIER) && prev_z == sfrom)
                {
        CopyMemQuick(&pc->argb[(dfrom-1) * width * 4], &pc->argb[dfrom * width * 4], width * 4);
                }
                else
                {
                        ULONG   *src, *dst, j, z;

                        prev_z  = yy / SCALE_MULTIPLIER;
         src      = (ULONG *)&pc->srcargb[(yy + y) / SCALE_MULTIPLIER * pc->srcwidth * 4];
                        dst             = (ULONG *)&pc->argb[dfrom * width * 4];
                        z                       = 0;

                        for (j = 0; j < width; j++)
                        {
                                *dst++  = src[ z / SCALE_MULTIPLIER ];
            z += x;
                        }

                        sheight--;
         sfrom++;
                }

                dfrom++;
                yy += y;
        }
}

/*--------------------------------------------------------------------*/
/* Plugin copydriver object dispatcher functions                      */
/*--------------------------------------------------------------------*/

static ULONG Measurecopy(struct Pngcopy *pc,struct Ammeasure *ammeasure)
{

   if(pc->argb)
   {  pc->copydriver.aow=pc->width;
      pc->copydriver.aoh=pc->height;
      if(ammeasure->ammr)
      {  ammeasure->ammr->width=pc->copydriver.aow;
         ammeasure->ammr->minwidth=pc->copydriver.aow;
      }
   }
   return 0;
}

static ULONG Layoutcopy(struct Pngcopy *pc,struct Amlayout *amlayout)
{

   if(pc->argb)
   {  pc->copydriver.aow=pc->width;
      pc->copydriver.aoh=pc->height;
   }
   return 0;
}

static ULONG Rendercopy(struct Pngcopy *pc,struct Amrender *amrender)
{  struct Coords *coo;

   if(pc->argb && !(amrender->flags&(AMRF_UPDATESELECTED|AMRF_UPDATENORMAL)))
   {
      if (!(pc->flags & PNGCF_READY))
      {
         ULONG secs, micros;
         CurrentTime(&secs, &micros);
         if (secs == pc->timestamp)
            return 0;
         pc->timestamp = secs;
      }

      coo=Clipcoords(pc->copydriver.cframe,amrender->coords);
      if(coo && coo->rp)
      {  if(amrender->flags&AMRF_CLEAR)
         {  Erasebg(pc->copydriver.cframe,coo,amrender->rect.minx,amrender->rect.miny,
               amrender->rect.maxx,amrender->rect.maxy);
         }
         Renderrows(pc,coo,amrender->rect.minx,amrender->rect.miny,
            amrender->rect.maxx,amrender->rect.maxy,0,pc->ready);
      }
      Unclipcoords(coo);
   }
   return 0;
}

static ULONG Setcopy(struct Pngcopy *pc,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   BOOL newbitmap=FALSE;   /* Remember if we got a new bitmap */
   BOOL chgbitmap=FALSE;   /* Remember if we got a bitmap data change */
   BOOL dimchanged=FALSE;  /* If our dimenasions have changed */
   BOOL rescale=FALSE;     /* Create new scaled bitmap as demanded by owner */
   long readyfrom=0,readyto=-1;
   UBYTE *argb=NULL;


   Amethodas(AOTP_COPYDRIVER,(struct Aobject *)pc,AOM_SET,amset->tags);
   tstate=amset->tags;
   while((tag=NextTagItem(&tstate)))
   {  switch(tag->ti_Tag)
      {  case AOCDV_Copy:
            pc->copy=(struct Aobject *)tag->ti_Data;
            break;
         case AOCDV_Sourcedriver:
            break;
         case AOCDV_Displayed:
            if(tag->ti_Data) pc->flags|=PNGCF_DISPLAYED;
            else pc->flags&=~PNGCF_DISPLAYED;
            break;
         case AOCDV_Width:
            if(tag->ti_Data && tag->ti_Data!=pc->width) pc->flags |= PNGCF_RESCALE;
            pc->swidth=tag->ti_Data;
            break;
         case AOCDV_Height:
            if(tag->ti_Data && tag->ti_Data!=pc->height) pc->flags |= PNGCF_RESCALE;
            pc->sheight=tag->ti_Data;
            break;
         case AOPNG_PixelArray:
            if(!(argb=(UBYTE *)tag->ti_Data))
            {
                pc->flags&=~PNGCF_READY;
            }

            newbitmap=TRUE;
            if(!pc->argb)
            {  pc->width=0;
               pc->height=0;
               pc->ready=-1;
               pc->flags&=~PNGCF_READY;
               dimchanged=TRUE;
            }
            break;
         case AOPNG_Width:
            if(tag->ti_Data!=pc->srcwidth) dimchanged=TRUE;
            pc->srcwidth=tag->ti_Data;
            break;
         case AOPNG_Height:
            if(tag->ti_Data!=pc->srcheight) dimchanged=TRUE;
            pc->srcheight=tag->ti_Data;
            break;
         case AOPNG_Readyfrom:
            readyfrom=tag->ti_Data;
            chgbitmap=TRUE;
            break;
         case AOPNG_Readyto:
            readyto=tag->ti_Data;
            pc->srcready=readyto;
            chgbitmap=TRUE;
            break;
         case AOPNG_Imgready:
            if(tag->ti_Data) pc->flags|=PNGCF_READY;
            else pc->flags&=~PNGCF_READY;
            chgbitmap=TRUE;
            if(pc->alpha)
            {
                /* force rerender on completion of interlaced alpha png */
                dimchanged = TRUE;
            }
            break;
         case AOPNG_Alpha:
            pc->alpha = tag->ti_Data;
            break;
      }
   }

   /* If a new scaling is requested, set magic so that a new bitmap is allocated and
    * scaled into. */
   /* but don't bother trying rescale if we are not ready! */

   if(pc->flags & PNGCF_RESCALE && pc->flags & PNGCF_READY)
   {
      if(!argb) argb=pc->srcargb;
      pc->flags &= ~ PNGCF_RESCALE;
      rescale = TRUE;
   }


   if((newbitmap && argb!=pc->argb) || rescale)
   {  Newbitmap(pc,argb);
   }
   if(((chgbitmap && readyto>=readyfrom) || rescale) && (pc->flags&PNGCF_OURBITMAP))
   {  long sfrom,sheight;
      if(pc->flags&PNGCF_READY)
      {  readyfrom=0;
         readyto=pc->height-1;
         sfrom=0;
         sheight=pc->srcheight;
         pc->ready=readyto;
      }
      else
      {  if(rescale)
         {  sfrom=0;
            sheight=pc->srcready+1;
            readyfrom=0;
         }
         else
         {  sfrom=readyfrom;
            sheight=readyto-readyfrom+1;
            if(pc->height>pc->srcheight && sfrom>0)
            {  /* Make sure there is no gap */
               sfrom--;
               sheight++;
            }
            readyfrom=sfrom*pc->height/pc->srcheight;
         }
         readyto=readyfrom+ScalerDiv(sheight,pc->height,pc->srcheight)-1;
         if(rescale) pc->ready=readyto;
      }
      ScaleArray(pc,sfrom,sheight,readyfrom);
   }

   if(readyto>pc->ready) pc->ready=readyto;

   /* If our dimensions have changed, let our AOTP_COPY object know, and
    * eventually we will receive an AOM_RENDER message. */
   if(dimchanged)
   {  Asetattrs(pc->copy,AOBJ_Changedchild,pc,TAG_END);
   }
   /* If the bitmap was changed but the dimensions stayed the same,
    * and we are allowed to render ourselves, render the new row(s) now. */
   else if(chgbitmap && (pc->flags&PNGCF_DISPLAYED) && pc->copydriver.cframe)
   {  Renderrows(pc,NULL,0,0,AMRMAX,AMRMAX,readyfrom,readyto);
   }
   /* If we are not allowed to render ourselves, let our AOTP_COPY object
    * know when the bitmap is complete. */
   else if(chgbitmap && !(pc->flags&PNGCF_DISPLAYED) && pc->flags&PNGCF_READY)
   {  Asetattrs(pc->copy,AOBJ_Changedchild,pc,TAG_END);
   }

   if(newbitmap && pc->flags&PNGCF_READY)
   {  Asetattrs(pc->copy,AOCPY_Onimgload,TRUE,TAG_END);
   }

   /* More notes for animations (animated PNG) only:
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

static struct Pngcopy *Newcopy(struct Amset *amset)
{  struct Pngcopy *pc;

   if((pc=Allocobject(AwebPluginBase->copydriver,sizeof(struct Pngcopy),amset)))
   {  pc->ready=-1;
      Setcopy(pc,amset);
   }
   return pc;
}

static ULONG Getcopy(struct Pngcopy *pc,struct Amset *amset)
{  struct TagItem *tag,*tstate;

   AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)pc,(struct Amessage *)amset);
   tstate=amset->tags;
   while((tag=NextTagItem(&tstate)))
   {  switch(tag->ti_Tag)
      {
         case AOCDV_Ready:
            PUTATTR(tag,pc->argb?TRUE:FALSE);
            /*PUTATTR(tag,(pc->argb && (pc->flags&PNGCF_READY))?TRUE:FALSE);*/
            break;

         case AOCDV_Imagebitmap:
            if (pc->flags & PNGCF_READY)
            {
               if (!pc->bitmap)
               {
                  pc->bitmap = AllocBitMap(pc->width, pc->height, 32, BMF_MINPLANES, NULL);

                  if (pc->bitmap)
                  {
                     struct RastPort rp;
                     InitRastPort(&rp);
                     rp.BitMap = pc->bitmap;
                     WritePixelArray(pc->argb, 0, 0, pc->width * 4, &rp, 0, 0, pc->width, pc->height, RECTFMT_ARGB);
                  }
               }

            }

            PUTATTR(tag,(pc->flags&PNGCF_READY)?pc->bitmap:NULL);
            break;

         case AOCDV_Imagemask:
            PUTATTR(tag,NULL);
            break;
         case AOCDV_Imagewidth:
            PUTATTR(tag,(pc->argb && pc->flags&PNGCF_READY)?pc->width:0);
            break;
         case AOCDV_Imageheight:
            PUTATTR(tag,(pc->argb && pc->flags&PNGCF_READY)?pc->height:0);
            break;
         case AOCDV_Alpha:
            PUTATTR(tag,pc->alpha);
            break;
      }
   }
   return 0;
}

static void Disposecopy(struct Pngcopy *pc)
{

   if(pc->flags&PNGCF_OURBITMAP)
   {
      FreeVecTaskPooled(pc->argb);
   }

   FreeBitMap(pc->bitmap);

   Amethodas(AOTP_COPYDRIVER,(struct Aobject *)pc,AOM_DISPOSE);
}

USRFUNC_H2
(
__saveds  ULONG  , Dispatchercopy,
struct Pngcopy *,pc,A0,
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
         result=Setcopy(pc,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getcopy(pc,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measurecopy(pc,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutcopy(pc,(struct Amlayout *)amsg);
         break;
      case AOM_RENDER:
         result=Rendercopy(pc,(struct Amrender *)amsg);
         break;
      case AOM_DISPOSE:
         Disposecopy(pc);
         break;
      default:
         result=AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)pc,amsg);
         break;
   }
   return result;

    USRFUNC_EXIT
}
