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

/* jfifcopy.c - AWebjfif copydriver */

#include "pluginlib.h"
#include "awebjfif.h"
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

struct Jfifcopy
{  struct Copydriver copydriver;    /* Our superclass object instance */
   struct Aobject *copy;            /* The AOTP_COPY object we belong to */
   struct BitMap *bitmap;           /* Same as source's bitmap or our own (scaled) */
   long width,height;               /* Dimensions of our bitmap */
   long ready;                      /* Last complete row number in bitmap */
   long srcready;                   /* Last complete row in src bitmap */
   long swidth,sheight;             /* Suggested width and height or 0 */
   USHORT flags;                    /* See below */
   struct BitMap *srcbitmap;        /* Our source's bitmap for scaling */
   long srcwidth,srcheight;         /* Original source's height for scaling */
};

/* Jfifcopy flags: */
#define JFIFCF_DISPLAYED    0x0001   /* We are allowed to render ourselves
                                     * if we have a context frame */
#define JFIFCF_READY        0x0002   /* Image is ready */
#define JFIFCF_OURBITMAP    0x0004   /* Bitmap and mask are ours */
#define JFIFCF_RESCALE      0x0008   /* We have a pending rescale */

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
static void Renderrows(struct Jfifcopy *jc,struct Coords *coo,
   long minx,long miny,long maxx,long maxy,long minrow,long maxrow)
{  long x,y;      /* Resulting rastport x,y to blit to */
   long dx,dy;    /* Offset within bitmap to blit from */
   long w,h;      /* Width and height of portion to blit */
   coo=Clipcoords(jc->copydriver.cframe,coo);
   if(coo && coo->rp)
   {  x=jc->copydriver.aox+coo->dx;
      y=jc->copydriver.aoy+coo->dy+minrow;
      dx=0;
      dy=minrow;
      w=jc->width;
      h=maxrow-minrow+1;
      Clipcopy(&x,&dx,&w,coo->minx,coo->maxx);
      Clipcopy(&y,&dy,&h,coo->miny,coo->maxy);
      Clipcopy(&x,&dx,&w,minx+coo->dx,maxx+coo->dx);
      Clipcopy(&y,&dy,&h,miny+coo->dy,maxy+coo->dy);
      if(w>0 && h>0)
      {  BltBitMapRastPort(jc->bitmap,dx,dy,coo->rp,x,y,w,h,0xc0);
      }
   }
   Unclipcoords(coo);
   if(jc->flags&JFIFCF_READY)
   {  Asetattrs(jc->copy,AOCPY_Onimgload,TRUE,TAG_END);
   }
}

/* Set a new bitmap and mask. If scaling required, allocate our own bitmap
 * and mask. */
static void Newbitmap(struct Jfifcopy *jc,struct BitMap *bitmap)
{  short depth;
   if(jc->flags&JFIFCF_OURBITMAP)
   {  if(jc->bitmap) FreeBitMap(jc->bitmap);
      jc->flags&=~JFIFCF_OURBITMAP;
   }

   if(bitmap)
   {  if(jc->swidth && !jc->sheight)
      {  jc->sheight=jc->srcheight*jc->swidth/jc->srcwidth;
         if(jc->sheight<1) jc->sheight=1;
      }
      if(jc->sheight && !jc->swidth)
      {  jc->swidth=jc->srcwidth*jc->sheight/jc->srcheight;
         if(jc->swidth<1) jc->swidth=1;
      }
   }

   if(bitmap && jc->swidth && jc->sheight
   && (jc->srcwidth!=jc->swidth || jc->srcheight!=jc->sheight))
   {  depth=GetBitMapAttr(bitmap,BMA_DEPTH);
      if(jc->bitmap=AllocBitMap(jc->swidth,jc->sheight,depth,BMF_MINPLANES,bitmap))
      {  jc->flags|=JFIFCF_OURBITMAP;
         jc->srcbitmap=bitmap;
         jc->width=jc->swidth;
         jc->height=jc->sheight;
      }
   }

   if(bitmap && !(jc->flags&JFIFCF_OURBITMAP))
   {  jc->bitmap=bitmap;
      jc->width=jc->srcwidth;
      jc->height=jc->srcheight;
   }

   if(!(bitmap))
   {  jc->bitmap = 0;
   }
}

static void Scalebitmap(struct Jfifcopy *jc,long sfrom,long sheight,long dfrom)
{  struct BitScaleArgs bsa={0};
   bsa.bsa_SrcX=0;
   bsa.bsa_SrcY=sfrom;
   bsa.bsa_SrcWidth=jc->srcwidth;
   bsa.bsa_SrcHeight=sheight;
   bsa.bsa_DestX=0;
   bsa.bsa_DestY=dfrom;
   bsa.bsa_XSrcFactor=jc->srcwidth;
   bsa.bsa_XDestFactor=jc->width;
   bsa.bsa_YSrcFactor=jc->srcheight;
   bsa.bsa_YDestFactor=jc->height;
   bsa.bsa_SrcBitMap=jc->srcbitmap;
   bsa.bsa_DestBitMap=jc->bitmap;
   bsa.bsa_Flags=0;
   BitMapScale(&bsa);
}

/*--------------------------------------------------------------------*/
/* Plugin copydriver object dispatcher functions                      */
/*--------------------------------------------------------------------*/

static ULONG Measurecopy(struct Jfifcopy *jc,struct Ammeasure *ammeasure)
{  if(jc->bitmap)
   {  jc->copydriver.aow=jc->width;
      jc->copydriver.aoh=jc->height;
      if(ammeasure->ammr)
      {  ammeasure->ammr->width=jc->copydriver.aow;
         ammeasure->ammr->minwidth=jc->copydriver.aow;
      }
   }
   return 0;
}

static ULONG Layoutcopy(struct Jfifcopy *jc,struct Amlayout *amlayout)
{  if(jc->bitmap)
   {  jc->copydriver.aow=jc->width;
      jc->copydriver.aoh=jc->height;
   }
   return 0;
}

static ULONG Rendercopy(struct Jfifcopy *jc,struct Amrender *amrender)
{  struct Coords *coo;
   if(jc->bitmap && !(amrender->flags&(AMRF_UPDATESELECTED|AMRF_UPDATENORMAL)))
   {  coo=Clipcoords(jc->copydriver.cframe,amrender->coords);
      if(coo && coo->rp)
      {  if(amrender->flags&AMRF_CLEAR)
         {  Erasebg(jc->copydriver.cframe,coo,amrender->rect.minx,amrender->rect.miny,
               amrender->rect.maxx,amrender->rect.maxy);
         }
         Renderrows(jc,coo,amrender->rect.minx,amrender->rect.miny,
            amrender->rect.maxx,amrender->rect.maxy,0,jc->ready);
      }
      Unclipcoords(coo);
   }
   return 0;
}

static ULONG Setcopy(struct Jfifcopy *jc,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   BOOL newbitmap=FALSE;   /* Remember if we got a new bitmap */
   BOOL chgbitmap=FALSE;   /* Remember if we got a bitmap data change */
   BOOL dimchanged=FALSE;  /* If our dimensions have changed */
   BOOL rescale=FALSE;     /* Create new scaled bitmap as demanded by owner */
   long readyfrom=0,readyto=-1;
   struct BitMap *bitmap=NULL;
   Amethodas(AOTP_COPYDRIVER,(struct Aobject *)jc,AOM_SET,amset->tags);
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOCDV_Copy:
            jc->copy=(struct Aobject *)tag->ti_Data;
            break;
         case AOCDV_Sourcedriver:
            break;
         case AOCDV_Displayed:
            if(tag->ti_Data) jc->flags|=JFIFCF_DISPLAYED;
            else jc->flags&=~JFIFCF_DISPLAYED;
            break;
         case AOCDV_Width:
            if(tag->ti_Data && tag->ti_Data!=jc->width) jc->flags |= JFIFCF_RESCALE;
            jc->swidth=tag->ti_Data;
            break;
         case AOCDV_Height:
            if(tag->ti_Data && tag->ti_Data!=jc->height) jc->flags |= JFIFCF_RESCALE;
            jc->sheight=tag->ti_Data;
            break;
         case AOJFIF_Bitmap:
            bitmap=(struct BitMap *)tag->ti_Data;
            newbitmap=TRUE;
            if(!jc->bitmap)
            {  jc->width=0;
               jc->height=0;
               jc->ready=-1;
               jc->flags&=~JFIFCF_READY;
               dimchanged=TRUE;
            }
            break;
         case AOJFIF_Width:
            if(tag->ti_Data!=jc->srcwidth) dimchanged=TRUE;
            jc->srcwidth=tag->ti_Data;
            break;
         case AOJFIF_Height:
            if(tag->ti_Data!=jc->srcheight) dimchanged=TRUE;
            jc->srcheight=tag->ti_Data;
            break;
         case AOJFIF_Readyfrom:
            readyfrom=tag->ti_Data;
            chgbitmap=TRUE;
            break;
         case AOJFIF_Readyto:
            readyto=tag->ti_Data;
            jc->srcready=readyto;
            chgbitmap=TRUE;
            break;
         case AOJFIF_Imgready:
            if(tag->ti_Data) jc->flags|=JFIFCF_READY;
            else jc->flags&=~JFIFCF_READY;
            chgbitmap=TRUE;
            break;
      }
   }

   /* If a new scaling is requested, set magic so that a new bitmap is allocated and
    * scaled into. */
   /* but don't bother trying to rescale if we are not ready */

   if((jc->flags & JFIFCF_RESCALE) && (jc->flags&JFIFCF_READY))
   {
       if(!bitmap) bitmap=jc->srcbitmap;
       rescale = TRUE;
       jc->flags &= ~ JFIFCF_RESCALE;
   }

   if((newbitmap && bitmap!=jc->bitmap) || rescale)
   {  Newbitmap(jc,bitmap);
   }
   if(((chgbitmap && readyto>=readyfrom) || rescale) && (jc->flags&JFIFCF_OURBITMAP))
   {  long sfrom,sheight;
      if(jc->flags&JFIFCF_READY)
      {  readyfrom=0;
         readyto=jc->height-1;
         sfrom=0;
         sheight=jc->srcheight;
         jc->ready=readyto;
      }
      else
      {  if(rescale)
         {  sfrom=0;
            sheight=jc->srcready+1;
            readyfrom=0;
         }
         else
         {  sfrom=readyfrom;
            sheight=readyto-readyfrom+1;
            if(jc->height>jc->srcheight && sfrom>0)
            {  /* Make sure there is no gap */
               sfrom--;
               sheight++;
            }
            readyfrom=sfrom*jc->height/jc->srcheight;
         }
         readyto=readyfrom+ScalerDiv(sheight,jc->height,jc->srcheight)-1;
         if(rescale) jc->ready=readyto;
      }
      Scalebitmap(jc,sfrom,sheight,readyfrom);
   }

   if(readyto>jc->ready) jc->ready=readyto;

   /* If our dimensions have changed, let our AOTP_COPY object know, and
    * eventually we will receive an AOM_RENDER message. */
   if(dimchanged)
   {  Asetattrs(jc->copy,AOBJ_Changedchild,jc,TAG_END);
   }
   /* If the bitmap was changed but the dimensions stayed the same,
    * and we are allowed to render ourselves, render the new row(s) now. */
   else if(chgbitmap && (jc->flags&JFIFCF_DISPLAYED) && jc->copydriver.cframe)
   {  Renderrows(jc,NULL,0,0,AMRMAX,AMRMAX,readyfrom,readyto);
   }
   /* If we are not allowed to render ourselves, let our AOTP_COPY object
    * know when the bitmap is complete. */
   else if(chgbitmap && !(jc->flags&JFIFCF_DISPLAYED) && jc->flags&JFIFCF_READY)
   {  Asetattrs(jc->copy,AOBJ_Changedchild,jc,TAG_END);
   }

   if(newbitmap && jc->flags&JFIFCF_READY)
   {  Asetattrs(jc->copy,AOCPY_Onimgload,TRUE,TAG_END);
   }

   return 0;
}

static struct Jfifcopy *Newcopy(struct Amset *amset)
{  struct Jfifcopy *jc;
   if(jc=Allocobject(AwebPluginBase->copydriver,sizeof(struct Jfifcopy),amset))
   {  jc->ready=-1;
      Setcopy(jc,amset);
   }
   return jc;
}

static ULONG Getcopy(struct Jfifcopy *jc,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)jc,(struct Amessage *)amset);
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOCDV_Ready:
            PUTATTR(tag,jc->bitmap?TRUE:FALSE);
            break;
         case AOCDV_Imagebitmap:
            PUTATTR(tag,(jc->flags&JFIFCF_READY)?jc->bitmap:NULL);
            break;
         case AOCDV_Imagewidth:
            PUTATTR(tag,(jc->bitmap && jc->flags&JFIFCF_READY)?jc->width:0);
            break;
         case AOCDV_Imageheight:
            PUTATTR(tag,(jc->bitmap && jc->flags&JFIFCF_READY)?jc->height:0);
            break;
      }
   }
   return 0;
}

static void Disposecopy(struct Jfifcopy *jc)
{  if(jc->flags&JFIFCF_OURBITMAP)
   {  if(jc->bitmap) FreeBitMap(jc->bitmap);
   }
   Amethodas(AOTP_COPYDRIVER,(struct Aobject *)jc,AOM_DISPOSE);
}

USRFUNC_H2
(
__saveds  ULONG  , Dispatchercopy,
struct Jfifcopy *,jc,A0,
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
         result=Setcopy(jc,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getcopy(jc,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measurecopy(jc,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutcopy(jc,(struct Amlayout *)amsg);
         break;
      case AOM_RENDER:
         result=Rendercopy(jc,(struct Amrender *)amsg);
         break;
      case AOM_DISPOSE:
         Disposecopy(jc);
         break;
      default:
         result=AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)jc,amsg);
         break;
   }
   return result;

    USRFUNC_EXIT
}
