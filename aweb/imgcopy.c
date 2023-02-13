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

/* imgcopy.c - AWeb image copy driver object */

#include "aweb.h"
#include "imgprivate.h"
#include "copydriver.h"
#include "copy.h"
#include <graphics/scale.h>
#include <proto/graphics.h>
#include <proto/utility.h>

#if defined(__amigaos4__)
#include <proto/Picasso96API.h>
#else
#include <cybergraphx/cybergraphics.h>
#include <proto/cybergraphics.h>
#endif
/*------------------------------------------------------------------------*/

struct Imgcopy
{  struct Copydriver cdv;
   void *copy;
   long swidth,sheight;          /* Suggested width,height or 0 */
   struct Imgsource *source;
   struct BitMap *bitmap;        /* Same as source's bitmap, or our own (scaled) */
   UBYTE *mask;                  /* Same as source's mask */
   long width,height;            /* Size of bitmap */
   UWORD flags;
};

#define IMGF_OURBITMAP  0x0001   /* Bitmap is ours */

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/* Source has a new bitmap for us (or none at all) */
static void Newbitmap(struct Imgcopy *img)
{
   int memfchip = 0;
   if(img->flags&IMGF_OURBITMAP)
   {  if(img->bitmap) FreeBitMap(img->bitmap);
      if(img->mask) FreeVec(img->mask);
   }
   img->flags&=~IMGF_OURBITMAP;

   if(img->source->bitmap)
   {  if(img->swidth && !img->sheight)
      {  img->sheight=img->source->height*img->swidth/img->source->width;
         if(img->sheight<1) img->sheight=1;
      }
      if(img->sheight && !img->swidth)
      {  img->swidth=img->source->width*img->sheight/img->source->height;
         if(img->swidth<1) img->swidth=1;
      }
   }

   if(img->source->bitmap && img->swidth && img->sheight  /*&& !img->source->mask*/
   && (img->swidth!=img->source->width || img->sheight!=img->source->height))
   {  /* Create our own scaled bitmap */
      if(img->bitmap=AllocBitMap(img->swidth,img->sheight,img->source->depth,
         BMF_MINPLANES,img->source->bitmap))
      {
         struct BitScaleArgs bsa={0};
         bsa.bsa_SrcX=0;
         bsa.bsa_SrcY=0;
         bsa.bsa_SrcWidth=img->source->width;
         bsa.bsa_SrcHeight=img->source->height;
         bsa.bsa_DestX=0;
         bsa.bsa_DestY=0;
         bsa.bsa_XSrcFactor=img->source->width;
         bsa.bsa_XDestFactor=img->swidth;
         bsa.bsa_YSrcFactor=img->source->height;
         bsa.bsa_YDestFactor=img->sheight;
         bsa.bsa_SrcBitMap=img->source->bitmap;
         bsa.bsa_DestBitMap=img->bitmap;
         bsa.bsa_Flags=0;
         BitMapScale(&bsa);
         img->flags|=IMGF_OURBITMAP;
         img->mask=NULL;
         img->width=img->swidth;
         img->height=img->sheight;

         if(img->source->mask)
         {  struct BitMap sbm={0},dbm={0};
            if(GetBitMapAttr(img->bitmap,BMA_FLAGS)&BMF_STANDARD)
            {
               memfchip=MEMF_CHIP;
            }
            sbm.BytesPerRow=GetBitMapAttr(img->source->bitmap,BMA_WIDTH)/8;
            sbm.Rows=GetBitMapAttr(img->source->bitmap,BMA_HEIGHT);
            dbm.BytesPerRow=GetBitMapAttr(img->bitmap,BMA_WIDTH)/8;
            dbm.Rows=GetBitMapAttr(img->bitmap,BMA_HEIGHT);

            if(sbm.BytesPerRow && dbm.BytesPerRow)
            {
               int width = dbm.BytesPerRow;
               int height = dbm.Rows;

               if(img->mask=(UBYTE *)AllocVec(width*height,memfchip /*|MEMF_CLEAR */))
               {

                   sbm.Depth=1;
                   sbm.Planes[0]=img->source->mask;
                   dbm.Depth=1;
                   dbm.Planes[0]=img->mask;
                   bsa.bsa_SrcX=0;
                   bsa.bsa_SrcY=0;
                   bsa.bsa_SrcWidth=img->source->width;
                   bsa.bsa_SrcHeight=img->source->height;
                   bsa.bsa_DestX=0;
                   bsa.bsa_DestY=0;
                   bsa.bsa_XSrcFactor=img->source->width;
                   bsa.bsa_XDestFactor=img->swidth;
                   bsa.bsa_YSrcFactor=img->source->height;
                   bsa.bsa_YDestFactor=img->sheight;
                   bsa.bsa_SrcBitMap=&sbm;
                   bsa.bsa_DestBitMap=&dbm;
                   bsa.bsa_Flags=0;
                   BitMapScale(&bsa);
               }
            }
         }

      }
   }

   if(!(img->flags&IMGF_OURBITMAP))
   {  img->bitmap=img->source->bitmap;
      img->mask=img->source->mask;
      img->width=img->source->width;
      img->height=img->source->height;
   }
}

/*------------------------------------------------------------------------*/

static long Measureimgcopy(struct Imgcopy *img,struct Ammeasure *amm)
{  if(img->bitmap)
   {  img->cdv.aow=img->width;
      img->cdv.aoh=img->height;
      if(amm->ammr)
      {  amm->ammr->width=amm->ammr->minwidth=img->cdv.aow;
      }
   }
   return 0;
}

static long Layoutimgcopy(struct Imgcopy *img,struct Amlayout *aml)
{  if(img->bitmap)
   {  img->cdv.aow=img->width;
      img->cdv.aoh=img->height;
   }
   return 0;
}

static long Renderimgcopy(struct Imgcopy *img,struct Amrender *amr)
{  struct Coords *coo=NULL;
        long dx,dy,w,h;
   if(img->bitmap && !(amr->flags&(AMRF_UPDATESELECTED|AMRF_UPDATENORMAL)))
   {  coo=Clipcoords(img->cdv.cframe,amr->coords);
      if(coo && coo->rp)
      {  if(amr->flags&AMRF_CLEAR)
         {  Erasebg((struct Frame *)img->cdv.cframe,coo,amr->rect.minx,amr->rect.miny,amr->rect.maxx,amr->rect.maxy);
         }
         if(img->cdv.aox<=amr->rect.maxx && img->cdv.aoy<=amr->rect.maxy
         && img->cdv.aox+img->width>amr->rect.minx && img->cdv.aoy+img->height>amr->rect.miny)
         {  dx=MAX(0,amr->rect.minx-img->cdv.aox);
            dy=MAX(0,amr->rect.miny-img->cdv.aoy);
            w=MIN(amr->rect.maxx-img->cdv.aox+1,img->width)-dx;
            h=MIN(amr->rect.maxy-img->cdv.aoy+1,img->height)-dy;
            if(img->mask)
                 {  BltMaskBitMapRastPort(img->bitmap,dx,dy,coo->rp,
                  img->cdv.aox+dx+coo->dx,img->cdv.aoy+dy+coo->dy,w,h,
                  0xe0,img->mask);
            }
            else
            {  BltBitMapRastPort(img->bitmap,dx,dy,coo->rp,
                  img->cdv.aox+dx+coo->dx,img->cdv.aoy+dy+coo->dy,w,h,
                  0xc0);
            }
         }
      }
      Unclipcoords(coo);
   }
   /* If we render, we are always completely decoded. Also, no animations with datatypes. */
   Asetattrs(img->copy,AOCPY_Onimgload,TRUE,TAG_END);
   return 0;
}

static long Setimgcopy(struct Imgcopy *img,struct Amset *ams)
{  long result=0;
   struct TagItem *tag,*tstate=ams->tags;
   BOOL rescale=FALSE;
   Amethodas(AOTP_COPYDRIVER,img,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOCDV_Copy:
            img->copy=(void *)tag->ti_Data;
            break;
         case AOCDV_Sourcedriver:
            img->source=(struct Imgsource *)tag->ti_Data;
            break;
         case AOCDV_Width:
            if(tag->ti_Data && img->width!=tag->ti_Data) rescale=TRUE;
            img->swidth=tag->ti_Data;
            break;
         case AOCDV_Height:
            if(tag->ti_Data && img->height!=tag->ti_Data) rescale=TRUE;
            img->sheight=tag->ti_Data;
            break;
         case AOIMP_Srcupdate:
            Newbitmap(img);
            Asetattrs(img->copy,
               AOBJ_Changedchild,(Tag)img,
               AOCPY_Onimgload,TRUE,
               TAG_END);
            break;

/*
#ifdef DEVELOPER
case AOCDV_Objectparams:
{ struct Objectparam *op=(struct Objectparam *)tag->ti_Data;
for(;op->next;op=op->next)
{ printf("Param name=%s value=%s vtype=%s type=%s\n",
   op->name?op->name:NULLSTRING,op->value?op->value:NULLSTRING,
   op->valuetype?op->valuetype:NULLSTRING,op->type?op->type:NULLSTRING);
}
}
break;
#endif
*/
      }
   }
   if(rescale) Newbitmap(img);
   return result;
}

static void Disposeimgcopy(struct Imgcopy *img)
{  if(img->flags&IMGF_OURBITMAP)
   {  if(img->bitmap) FreeBitMap(img->bitmap);
      if(img->mask) FreeVec(img->mask);
   }
   Amethodas(AOTP_COPYDRIVER,img,AOM_DISPOSE);
}

static struct Imgcopy *Newimgcopy(struct Amset *ams)
{  struct Imgcopy *img=Allocobject(AOTP_IMGCOPY,sizeof(struct Imgcopy),ams);
   if(img)
   {  Setimgcopy(img,ams);
   }
   return img;
}

static long Getimgcopy(struct Imgcopy *img,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   long result;
   result=AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)img,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOCDV_Ready:
            PUTATTR(tag,BOOLVAL(img->bitmap));
            break;
         case AOCDV_Imagebitmap:
            PUTATTR(tag,img->bitmap);
            break;
         case AOCDV_Imagemask:
            PUTATTR(tag,img->mask);
            break;
         case AOCDV_Imagewidth:
            PUTATTR(tag,img->bitmap?img->width:0);
            break;
         case AOCDV_Imageheight:
            PUTATTR(tag,img->bitmap?img->height:0);
            break;
      }
   }
   return result;
}

USRFUNC_H2
(
static long  , Imgcopy_Dispatcher,
struct Imgcopy *,img,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newimgcopy((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setimgcopy(img,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getimgcopy(img,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measureimgcopy(img,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutimgcopy(img,(struct Amlayout *)amsg);
         break;
      case AOM_RENDER:
         result=Renderimgcopy(img,(struct Amrender *)amsg);
         break;
      case AOM_DISPOSE:
         Disposeimgcopy(img);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         result=AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)img,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installimgcopy(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_IMGCOPY, (Tag)Imgcopy_Dispatcher)) return FALSE;
   return TRUE;
}
