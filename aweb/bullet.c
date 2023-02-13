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

/* bullet.c - AWeb html document bullet element object */

#include "aweb.h"
#include "bullet.h"
#include "application.h"
#include <reaction/reaction.h>

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

struct Bullet
{  struct Element elt;
   short type;
   UWORD flags;
   void *image;
};

#define BULF_HIGHLIGHT  0x0001   /* Highlight */

static ULONG palette[]=
{  4,
   0x00000000,0x88888888,0x00000000,
   0xffffffff,0xffffffff,0xffffffff,
   0x00000000,0x00000000,0x00000000,
   0x80808080,0x80808080,0x80808080,
   0x00000000,0x00000000,0x00000000,
};

static __aligned UBYTE discdata[]=
{  0,9, 0,9,
   0,0,1,1,1,0,0,0,0,
   0,1,1,2,2,2,0,0,0,
   1,1,2,2,2,2,2,0,0,
   1,2,2,2,2,2,2,2,0,
   1,2,2,2,2,2,2,2,3,
   0,2,2,2,2,2,2,2,3,
   0,0,2,2,2,2,2,3,3,
   0,0,0,2,2,2,3,3,0,
   0,0,0,0,3,3,3,0,0,
};

static __aligned UBYTE circledata[]=
{  0,9, 0,9,
   0,0,1,1,1,0,0,0,0,
   0,1,1,2,2,2,0,0,0,
   1,1,2,2,2,2,2,0,0,
   1,2,2,3,0,0,2,2,0,
   1,2,2,0,0,0,2,2,3,
   0,2,2,0,0,1,2,2,3,
   0,0,2,2,2,2,2,3,3,
   0,0,0,2,2,2,3,3,0,
   0,0,0,0,3,3,3,0,0,
};

static __aligned UBYTE squaredata[]=
{  0,9, 0,9,
   1,1,1,1,1,1,1,1,3,
   1,2,2,2,2,2,2,2,3,
   1,2,2,2,2,2,2,2,3,
   1,2,2,2,2,2,2,2,3,
   1,2,2,2,2,2,2,2,3,
   1,2,2,2,2,2,2,2,3,
   1,2,2,2,2,2,2,2,3,
   1,2,2,2,2,2,2,2,3,
   1,3,3,3,3,3,3,3,3,
};

static __aligned UBYTE diamonddata[]=
{  0,9, 0,9,
   0,0,0,1,0,0,0,0,0,
   0,0,1,1,2,0,0,0,0,
   0,1,1,2,2,2,0,0,0,
   1,1,2,2,3,2,2,0,0,
   0,2,2,3,0,1,2,2,0,
   0,0,2,2,1,2,2,3,3,
   0,0,0,2,2,2,3,3,0,
   0,0,0,0,2,3,3,0,0,
   0,0,0,0,0,3,0,0,0,
};

static __aligned UBYTE soliddiadata[]=
{  0,9, 0,9,
   0,0,0,1,0,0,0,0,0,
   0,0,1,1,2,0,0,0,0,
   0,1,1,2,2,2,0,0,0,
   1,1,2,2,2,2,2,0,0,
   0,2,2,2,2,2,2,2,0,
   0,0,2,2,2,2,2,3,3,
   0,0,0,2,2,2,3,3,0,
   0,0,0,0,2,3,3,0,0,
   0,0,0,0,0,3,0,0,0,
};

static __aligned UBYTE rectangledata[]=
{  0,9, 0,9,
   1,1,1,1,1,1,1,1,3,
   1,2,2,2,2,2,2,2,3,
   1,2,3,3,3,3,1,2,3,
   1,2,3,0,0,0,1,2,3,
   1,2,3,0,0,0,1,2,3,
   1,2,3,0,0,0,1,2,3,
   1,2,3,1,1,1,1,2,3,
   1,2,2,2,2,2,2,2,3,
   1,3,3,3,3,3,3,3,3,
};

static UBYTE *typedata[]=
{  discdata,circledata,squaredata,diamonddata,soliddiadata,rectangledata,
};

static void *bullet[6];    /* Re-usable images */
static long usecnt[6];     /* If usecnt becomes zero, the bullet image is disposed. */

/*------------------------------------------------------------------------*/

/* Use a bullet image, create new one if necessary */
static void *Usebullet(UWORD type)
{  short n=type-1;
   if(usecnt[n]==0)
   {  bullet[n]=PenMapObject,
         IA_Data,typedata[n],
         PENMAP_Palette,palette,
         PENMAP_Screen,Agetattr(Aweb(),AOAPP_Screen),
         PENMAP_Transparent,TRUE,
         PENMAP_MaskBlit,TRUE,
      End;
   }
   usecnt[n]++;
   return bullet[n];
}

/* Unuse a bullet, dispose if not used any more because of possible
 * screen change */
static void Unusebullet(UWORD type)
{  short n=type-1;
   if(usecnt[n])
   {  usecnt[n]--;
      if(!usecnt[n])
      {  DisposeObject(bullet[n]);
         bullet[n]=NULL;
      }
   }
}

/*------------------------------------------------------------------------*/

static long Renderbullet(struct Bullet *bul,struct Amrender *amr)
{  struct Coords *coo,coords={0};
   BOOL clip=FALSE;
   ULONG clipkey=0;
   struct RastPort *rp;
   long x,y;
   if(!(amr->flags&(AMRF_UPDATESELECTED|AMRF_UPDATENORMAL)))
   {  if(!(coo=amr->coords))
      {  Framecoords(bul->elt.cframe,&coords);
         coo=&coords;
         clip=TRUE;
      }
      if(coo->rp)
      {  rp=coo->rp;
         if(clip) clipkey=Clipto(rp,coo->minx,coo->miny,coo->maxx,coo->maxy);
         x=bul->elt.aox+coo->dx;
         y=bul->elt.aoy+coo->dy;
         if(amr->flags&AMRF_CLEAR)
         {
             Erasebg
             (
                 bul->elt.cframe,
                 coo,
                 bul->elt.aox,
                 bul->elt.aoy,
                 bul->elt.aox+bul->elt.aow-1,
                 bul->elt.aoy+bul->elt.aoh-1
             );
         }
         if(bul->image)
         {  DrawImage(rp,bul->image,x,y);
         }
         if(bul->flags&BULF_HIGHLIGHT)
         {  SetDrMd(rp,COMPLEMENT);
            RectFill(rp,x,y,x+bul->elt.aow-1,y+bul->elt.aoh-1);
            SetDrMd(rp,JAM1);
         }
         if(clip) Unclipto(clipkey);
      }
   }
   return 0;
}

static long Setbullet(struct Bullet *bul,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=Amethodas(AOTP_ELEMENT,bul,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBUL_Type:
            bul->type=tag->ti_Data;
            if(bul->type>6) bul->type=6;
            if(bul->type<1) bul->type=1;
            break;
         case AOBJ_Cframe:
            if(!tag->ti_Data)
            {  bul->flags&=~BULF_HIGHLIGHT;
            }
            break;
         case AOBJ_Window:
            /* Use Window as trigger to create/dispose penmap image -
             * it will surround AOAPP_Screenvalid notification. */
            if(tag->ti_Data)
            {  if(!bul->image)
               {  bul->image=Usebullet(bul->type);
               }
            }
            else
            {  if(bul->image)
               {  Unusebullet(bul->type);
                  bul->image=NULL;
               }
            }
            break;
      }
   }
   return result;
}

static struct Bullet *Newbullet(struct Amset *ams)
{  struct Bullet *bul;
   if(bul=Allocobject(AOTP_BULLET,sizeof(struct Bullet),ams))
   {  bul->elt.aow=9;
      bul->elt.aoh=9;
      bul->type=BULTP_DISC;
      Setbullet(bul,ams);
   }
   return bul;
}

static long Dragtestbullet(struct Bullet *bul,struct Amdragtest *amd)
{  long result=0;
   long x,y;
   x=amd->xco-amd->coords->dx;
   y=amd->yco-amd->coords->dy;
   if
   (
       (
           bul->elt.aoy                <= y &&
           bul->elt.aoy + bul->elt.aoh >  y &&
           bul->elt.aox + bul->elt.aow >  x
       ) ||
       bul->elt.aoy > y
   )
   {  result=AMDR_HIT;
      if(amd->amdr)
      {  amd->amdr->object=bul;
      }
   }
   return result;
}

/* Returns new state */
static UWORD Draglimits(struct Bullet *bul,void *startobject,void *endobject,
   UWORD state,BOOL *hi)
{  if(bul==startobject)
   {  switch(state)
      {  case AMDS_BEFORE:
            state=AMDS_RENDER;
            *hi=TRUE;
            if(bul==endobject)
            {  state=AMDS_AFTER;
            }
            break;
         case AMDS_REVERSE:
            state=AMDS_AFTER;
            *hi=TRUE;
            break;
      }
   }
   else if(bul==endobject)
   {  switch(state)
      {  case AMDS_BEFORE:
            state=AMDS_REVERSE;
            *hi=TRUE;
            break;
         case AMDS_RENDER:
            state=AMDS_AFTER;
            *hi=TRUE;
            break;
      }
   }
   else
   {  switch(state)
      {  case AMDS_BEFORE:
            *hi=FALSE;
            break;
         case AMDS_RENDER:
         case AMDS_REVERSE:
            *hi=TRUE;
            break;
         case AMDS_AFTER:
            *hi=FALSE;
            if(!(bul->flags&BULF_HIGHLIGHT))
            {  state=AMDS_DONE;
            }
            break;
      }
   }
   return state;
}

static long Dragrenderbullet(struct Bullet *bul,struct Amdragrender *amd)
{  BOOL hi=FALSE;
   amd->state=Draglimits(bul,amd->startobject,amd->endobject,amd->state,&hi);
   if(BOOLVAL(hi)!=BOOLVAL(bul->flags&BULF_HIGHLIGHT))
   {  SETFLAG(bul->flags,BULF_HIGHLIGHT,hi);
      Arender((struct Aobject *)bul,amd->coords,0,0,AMRMAX,AMRMAX,AMRF_CLEAR,NULL);
   }
   return 0;
}

static long Dragcopybullet(struct Bullet *bul,struct Amdragcopy *amd)
{  BOOL add=FALSE;
   short i,n;
   amd->state=Draglimits(bul,amd->startobject,amd->endobject,amd->state,&add);
   if(add)
   {  if(bul->elt.leftindent>1
      && (!amd->clip->length || amd->clip->buffer[amd->clip->length-1]=='\n'))
      {  n=bul->elt.leftindent-1;
         for(i=0;i<n;i++)
         {  Addtobuffer(amd->clip,"    ",4);
         }
      }
      Addtobuffer(amd->clip,"  o ",4);
   }
   return 0;
}

static void Disposebullet(struct Bullet *bul)
{  if(bul->image) Unusebullet(bul->type);
   Amethodas(AOTP_ELEMENT,bul,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Bullet_Dispatcher,
struct Bullet *,bul,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newbullet((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setbullet(bul,(struct Amset *)amsg);
         break;
      case AOM_RENDER:
         result=Renderbullet(bul,(struct Amrender *)amsg);
         break;
      case AOM_DRAGTEST:
         result=Dragtestbullet(bul,(struct Amdragtest *)amsg);
         break;
      case AOM_DRAGRENDER:
         result=Dragrenderbullet(bul,(struct Amdragrender *)amsg);
         break;
      case AOM_DRAGCOPY:
         result=Dragcopybullet(bul,(struct Amdragcopy *)amsg);
         break;
      case AOM_DISPOSE:
         Disposebullet(bul);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         AmethodasA(AOTP_ELEMENT,(struct Aobject *)bul,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installbullet(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_BULLET,(Tag)Bullet_Dispatcher)) return FALSE;
   return TRUE;
}
