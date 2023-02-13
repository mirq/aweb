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

/* hidden.c - AWeb html document hidden element object */

#include "aweb.h"
#include "field.h"
#include "jslib.h"

/*------------------------------------------------------------------------*/

struct Hidden
{  struct Field field;
};

/*------------------------------------------------------------------------*/

/* Get or set value property (JS) */
static BOOL Propertyvalue(struct Varhookdata *vd)
{  BOOL result=FALSE;
   struct Hidden *hid=vd->hookdata;
   UBYTE *value;
   if(hid)
   {  switch(vd->code)
      {  case VHC_SET:
            if(value=Jtostring(vd->jc,vd->value))
            {  if(hid->field.value) FREE(hid->field.value);
               hid->field.value=Dupstr(value,-1);
            }
            result=TRUE;
            break;
         case VHC_GET:
            Jasgstring(vd->jc,vd->value,hid->field.value);
            result=TRUE;
            break;
      }
   }
   return result;
}

static void Methodtostring(struct Jcontext *jc)
{  struct Hidden *hid=Jointernal(Jthis(jc));
   struct Buffer buf={0};
   if(hid)
   {  Addtagstr(&buf,"<input",ATSF_NONE,0);
      Addtagstr(&buf,"type",ATSF_STRING,"hidden");
      if(hid->field.name) Addtagstr(&buf,"name",ATSF_STRING,hid->field.name);
      if(hid->field.value) Addtagstr(&buf,"value",ATSF_STRING,hid->field.value);
      Addtobuffer(&buf,">",2);
      Jasgstring(jc,NULL,buf.buffer);
      Freebuffer(&buf);
   }
}

/*------------------------------------------------------------------------*/

static struct Hidden *Newhidden(struct Amset *ams)
{  struct Hidden *hid;
   if(hid=Allocobject(AOTP_HIDDEN,sizeof(struct Hidden),ams))
   {  Amethodas(AOTP_FIELD,hid,AOM_SET,(Tag)ams->tags);
   }
   return hid;
}

static long Jsetuphidden(struct Hidden *hid,struct Amjsetup *amj)
{  struct Jvar *jv;
   AmethodasA(AOTP_FIELD,(struct Aobject *)hid,(struct Amessage *)amj);
   if(hid->field.jobject)
   {  if(jv=Jproperty(amj->jc,hid->field.jobject,"value"))
      {  Setjproperty(jv,Propertyvalue,hid);
      }
      if(jv=Jproperty(amj->jc,hid->field.jobject,"type"))
      {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
         Jasgstring(amj->jc,jv,"hidden");
      }
      Addjfunction(amj->jc,hid->field.jobject,"toString",Methodtostring,NULL);
   }
   return 0;
}

USRFUNC_H2
(
static long  , Hidden_Dispatcher,
struct Hidden *,hid,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newhidden((struct Amset *)amsg);
         break;
      case AOM_JSETUP:
         result=Jsetuphidden(hid,(struct Amjsetup *)amsg);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         result=AmethodasA(AOTP_FIELD,(struct Aobject *)hid,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installhidden(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_HIDDEN,(Tag)Hidden_Dispatcher)) return FALSE;
   return TRUE;
}
