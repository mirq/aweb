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

/* break.c - AWeb html document line break element object */

#include "aweb.h"
#include "break.h"
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

struct Break
{
    struct Element elt;
    UWORD flags;
};

#define BRKF_CLRLEFT    0x0001   /* Wants clear left margin */
#define BRKF_CLRRIGHT   0x0002   /* Wants clear right margin */
#define BRKF_WBR        0x0004   /* Soft break */

/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/

static long Measurebreak(struct Break *br,struct Ammeasure *amm)
{  br->elt.aow=0;
   br->elt.aoh=br->elt.font->tf_YSize;
   if(amm->ammr)
   {  if(br->flags&BRKF_WBR)
      {  amm->ammr->width=amm->addwidth;
      }
      else
      {  amm->ammr->newline=TRUE;
      }
   }
   return 0;
}

static long Layoutbreak(struct Break *br,struct Amlayout *aml)
{  /* Always return NEWLINE if no soft break.
    * Set total height to normal font height so consecutive breaks will generate
    * vertical space, but only if break is first (and only) in this line;
    * otherwise use the generated height from the other elements (like
    * small images).
    * If soft break, always return OK so a break will appear if necessary.
    */
   if(aml->amlr)
   {  aml->amlr->endx=aml->startx;
      if(aml->flags&AMLF_FIRST) aml->amlr->toph=br->elt.aoh;
      switch(br->flags&(BRKF_CLRLEFT|BRKF_CLRRIGHT))
      {  case 0:
            if(br->flags&BRKF_WBR)
            {  aml->amlr->result=AMLR_OK;
            }
            else
            {  aml->amlr->result=AMLR_NEWLINE;
            }
            break;
         case BRKF_CLRLEFT:
            aml->amlr->result=AMLR_NLCLRLEFT;
            break;
         case BRKF_CLRRIGHT:
            aml->amlr->result=AMLR_NLCLRRIGHT;
            break;
         case BRKF_CLRLEFT|BRKF_CLRRIGHT:
            aml->amlr->result=AMLR_NLCLRALL;
            break;
      }
   }
   return 0;
}

static long Alignbreak(struct Break *br,struct Amalign *ama)
{  br->elt.aox+=ama->dx;
   br->elt.aoy=ama->y;
   return 0;
}

static long Setbreak(struct Break *br,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=Amethodas(AOTP_ELEMENT,br,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBRK_Clearleft:
            if(tag->ti_Data) br->flags|=BRKF_CLRLEFT;
            else br->flags&=~BRKF_CLRLEFT;
            break;
         case AOBRK_Clearright:
            if(tag->ti_Data) br->flags|=BRKF_CLRRIGHT;
            else br->flags&=~BRKF_CLRRIGHT;
            break;
         case AOBRK_Wbr:
            SETFLAG(br->flags,BRKF_WBR,tag->ti_Data);
            break;
      }
   }
   return result;
}

static struct Break *Newbreak(struct Amset *ams)
{  struct Break *br;
   if(br=Allocobject(AOTP_BREAK,sizeof(struct Break),ams))
   {  Setbreak(br,ams);
   }
   return br;
}

static long Getbreak(struct Break *br,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=AmethodasA(AOTP_ELEMENT,(struct Aobject *)br,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Clipdrag:
            PUTATTR(tag,TRUE);
            break;
      }
   }
   return result;
}

static long Hittestbreak(struct Break *br,struct Amhittest *amh)
{  return 0;
}

static long Dragtestbreak(struct Break *br,struct Amdragtest *amd)
{  long result=0;
   long x,y;
   x=amd->xco-amd->coords->dx;
   y=amd->yco-amd->coords->dy;
   if((br->elt.aoy<=y && br->elt.aoy+br->elt.aoh>y && br->elt.aox<=x) || br->elt.aoy>y)
   {  result=AMDR_HIT;
      if(amd->amdr)
      {  amd->amdr->object=br;
      }
   }
   return result;
}

static long Dragrenderbreak(struct Break *br,struct Amdragrender *amd)
{  if(br==amd->startobject)
   {  switch(amd->state)
      {  case AMDS_BEFORE:
            amd->state=AMDS_RENDER;
            if(br==amd->endobject)
            {  amd->state=AMDS_AFTER;
            }
            break;
         case AMDS_REVERSE:
            amd->state=AMDS_AFTER;
            break;
      }
   }
   else if(br==amd->endobject)
   {  switch(amd->state)
      {  case AMDS_BEFORE:
            amd->state=AMDS_REVERSE;
            break;
         case AMDS_RENDER:
            amd->state=AMDS_AFTER;
            break;
      }
   }
   return 0;
}

static long Dragcopybreak(struct Break *br,struct Amdragcopy *amd)
{  BOOL add=FALSE;
   if(br==amd->startobject)
   {  switch(amd->state)
      {  case AMDS_BEFORE:
            amd->state=AMDS_RENDER;
            add=TRUE;
            if(br==amd->endobject)
            {  amd->state=AMDS_AFTER;
            }
            break;
         case AMDS_REVERSE:
            amd->state=AMDS_AFTER;
            add=TRUE;
            break;
      }
   }
   else if(br==amd->endobject)
   {  switch(amd->state)
      {  case AMDS_BEFORE:
            amd->state=AMDS_REVERSE;
            add=TRUE;
            break;
         case AMDS_RENDER:
            amd->state=AMDS_AFTER;
            add=TRUE;
            break;
      }
   }
   else
   {  switch(amd->state)
      {  case AMDS_RENDER:
         case AMDS_REVERSE:
            add=TRUE;
            break;
      }
   }
   if(add)
   {  Addtobuffer(amd->clip,"\n",1);
   }
   return 0;
}

USRFUNC_H2
(
static long  , Break_Dispatcher,
struct Break *,br,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newbreak((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setbreak(br,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getbreak(br,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measurebreak(br,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutbreak(br,(struct Amlayout *)amsg);
         break;
      case AOM_ALIGN:
         result=Alignbreak(br,(struct Amalign *)amsg);
         break;
      case AOM_HITTEST:
         result=Hittestbreak(br,(struct Amhittest *)amsg);
         break;
      case AOM_DRAGTEST:
         result=Dragtestbreak(br,(struct Amdragtest *)amsg);
         break;
      case AOM_DRAGRENDER:
         result=Dragrenderbreak(br,(struct Amdragrender *)amsg);
         break;
      case AOM_DRAGCOPY:
         result=Dragcopybreak(br,(struct Amdragcopy *)amsg);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         result=AmethodasA(AOTP_ELEMENT,(struct Aobject *)br,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installbreak(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_BREAK,(Tag)Break_Dispatcher)) return FALSE;
   return TRUE;
}
