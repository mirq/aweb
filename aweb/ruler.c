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

/* ruler.c - AWeb html document ruler element object */

#include "aweb.h"
#include "ruler.h"

#include <proto/graphics.h>
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

struct Ruler
{  struct Element elt;
   short width;            /* horizontal width */
   short size;             /* vertical size */
   struct Colorinfo *color;
   UWORD flags;
};

#define RULF_PIXELS     0x0001   /* width is in pixels, else percentage */
#define RULF_NOSHADE    0x0002   /* no 3d effects */

/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/

/* Nothing to measure - ruler adapts to width on Layout */
static long Measureruler(struct Ruler *rul,struct Ammeasure *amm)
{  if(amm->ammr)
   {  if(rul->flags&RULF_PIXELS)
      {  amm->ammr->width=rul->width;
         amm->ammr->minwidth=rul->width;
      }
      else
      {  amm->ammr->width=0;
         amm->ammr->minwidth=2;
      }
   }
   return 0;
}

static long Layoutruler(struct Ruler *rul,struct Amlayout *aml)
{  if(rul->flags&RULF_PIXELS)
   {  rul->elt.aow=MIN(rul->width,aml->width-aml->startx);
   }
   else
   {  rul->elt.aow=(aml->width-aml->startx)*rul->width/100;
   }
   rul->elt.aoh=rul->size+6;
   rul->elt.aox=aml->startx;
   if(aml->amlr)
   {  aml->amlr->result=AMLR_NEWLINE;
      aml->amlr->endx=rul->elt.aox+rul->elt.aow;
      aml->amlr->toph=rul->elt.aoh;
   }
   return 0;
}

static long Alignruler(struct Ruler *rul,struct Amalign *ama)
{  rul->elt.aox+=ama->dx;
   rul->elt.aoy=ama->y;
   return 0;
}

static long Renderruler(struct Ruler *rul,struct Amrender *amr)
{  struct Coords *coo,coords={0};
   BOOL clip=FALSE;
   ULONG clipkey=0;
   struct RastPort *rp;
   long x,y;
   short pen;
   if(!(amr->flags&(AMRF_UPDATESELECTED|AMRF_UPDATENORMAL)))
   {  if(!(coo=amr->coords))
      {  Framecoords(rul->elt.cframe,&coords);
         coo=&coords;
         clip=TRUE;
      }
      if(coo->rp)
      {  x=rul->elt.aox+coo->dx;
         y=rul->elt.aoy+coo->dy+3;
         rp=coo->rp;
         if(clip) clipkey=Clipto(rp,coo->minx,coo->miny,coo->maxx,coo->maxy);
         if(rul->flags&RULF_NOSHADE)
         {  if(rul->color && rul->color->pen>=0) pen=rul->color->pen;
            else pen=coo->textcolor;
            SetAPen(rp,pen);
            RectFill(rp,x,y,x+rul->elt.aow-1,y+rul->size-1);
         }
         else
         {  SetAPen(rp,coo->dri->dri_Pens[SHADOWPEN]);
            Move(rp,x,y+rul->size-1);
            Draw(rp,x,y);
            Draw(rp,x+rul->elt.aow-2,y);
            SetAPen(rp,coo->dri->dri_Pens[SHINEPEN]);
            Move(rp,x+1,y+rul->size-1);
            Draw(rp,x+rul->elt.aow-1,y+rul->size-1);
            Draw(rp,x+rul->elt.aow-1,y);
         }
         if(clip) Unclipto(clipkey);
      }
   }
   return 0;
}

static long Setruler(struct Ruler *rul,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=Amethodas(AOTP_ELEMENT,rul,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AORUL_Width:
            rul->width=MAX(tag->ti_Data,2);
            rul->flags&=~RULF_PIXELS;
            break;
         case AORUL_Pixelwidth:
            rul->width=MAX(tag->ti_Data,1);
            rul->flags|=RULF_PIXELS;
            break;
         case AORUL_Color:
            rul->color=(struct Colorinfo *)tag->ti_Data;
            break;
         case AORUL_Noshade:
            if(tag->ti_Data) rul->flags|=RULF_NOSHADE;
            else rul->flags&=~RULF_NOSHADE;
            break;
         case AORUL_Size:
            rul->size=MIN(MAX(tag->ti_Data,1),16000);
            break;
      }
   }
   if(rul->color) rul->flags|=RULF_NOSHADE;
   return result;
}

static struct Ruler *Newruler(struct Amset *ams)
{  struct Ruler *rul;
   if(rul=Allocobject(AOTP_RULER,sizeof(struct Ruler),ams))
   {  rul->width=100;
      rul->size=2;
      /* Default alignment is centered, should only be overruled by our own NEW tags*/
      Asetattrs((struct Aobject *)rul,AOELT_Halign,HALIGN_CENTER,TAG_END);
      Setruler(rul,ams);
   }
   return rul;
}

USRFUNC_H2
(
static long  , Ruler_Dispatcher,
struct Ruler *,rul,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newruler((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setruler(rul,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measureruler(rul,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutruler(rul,(struct Amlayout *)amsg);
         break;
      case AOM_ALIGN:
         result=Alignruler(rul,(struct Amalign *)amsg);
         break;
      case AOM_RENDER:
         result=Renderruler(rul,(struct Amrender *)amsg);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         result=AmethodasA(AOTP_ELEMENT,(struct Aobject *)rul,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installruler(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_RULER,(Tag)Ruler_Dispatcher)) return FALSE;
   return TRUE;
}
