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

/* copydriver.c - AWeb general copydriver object superclass */

#include "aweb.h"
#include "copydriver.h"
#include <proto/utility.h>

/*----------------------------------------------------------------------*/

static long Setcopydriver(struct Copydriver *cdv,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Left:
            cdv->aox=tag->ti_Data;
            break;
         case AOBJ_Top:
            cdv->aoy=tag->ti_Data;
            break;
         case AOBJ_Width:
            cdv->aow=tag->ti_Data;
            break;
         case AOBJ_Height:
            cdv->aoh=tag->ti_Data;
            break;
         case AOBJ_Cframe:
            cdv->cframe=(void *)tag->ti_Data;
            break;
      }
   }
   return 0;
}

static long Getcopydriver(struct Copydriver *cdv,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Left:
            PUTATTR(tag,cdv->aox);
            break;
         case AOBJ_Top:
            PUTATTR(tag,cdv->aoy);
            break;
         case AOBJ_Width:
            PUTATTR(tag,cdv->aow);
            break;
         case AOBJ_Height:
            PUTATTR(tag,cdv->aoh);
            break;
         case AOBJ_Cframe:
            PUTATTR(tag,cdv->cframe);
            break;
      }
   }
   return 0;
}

static long Movecopydriver(struct Copydriver *cdv,struct Ammove *amm)
{  cdv->aox+=amm->dx;
   cdv->aoy+=amm->dy;
   return 0;
}

static void Disposecopydriver(struct Copydriver *cdv)
{  Amethodas(AOTP_OBJECT,cdv,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Copydriver_Dispatcher,
struct Copydriver *,cdv,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_SET:
         result=Setcopydriver(cdv,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getcopydriver(cdv,(struct Amset *)amsg);
         break;
      case AOM_MOVE:
         result=Movecopydriver(cdv,(struct Ammove *)amsg);
         break;
      case AOM_DISPOSE:
         Disposecopydriver(cdv);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*----------------------------------------------------------------------*/

BOOL Installcopydriver(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_COPYDRIVER,(Tag)Copydriver_Dispatcher)) return FALSE;
   return TRUE;
}
