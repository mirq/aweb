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

/* jboolean.c - AWeb js internal Boolean object */

#include "awebjs.h"
#include "jprotos.h"

struct Boolean          /* Used as internal object value */
{  BOOL bvalue;
};

/*-----------------------------------------------------------------------*/

/* Convert (jthis) to string */
static void Booleantostring(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   UBYTE *p="false";
   BOOL b;
   if(jo && jo->internal && jo->type == OBJT_BOOLEAN)
   {  b=((struct Boolean *)jo->internal)->bvalue;
      if(b) p="true";
   }
   else
   {
       Runtimeerror(jc,NTE_TYPE,jc->elt,"Boolean.prototype.toString called on incompatable object type");
   }
   Asgstring(RETVAL(jc),p,jc->pool);
}

/* Get value of (jthis) */
static void Booleanvalueof(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   BOOL b=FALSE;
   if(jo && jo->internal && jo->type == OBJT_BOOLEAN)
   {  b=((struct Boolean *)jo->internal)->bvalue;
   }
   else
   {
       Runtimeerror(jc,NTE_TYPE,jc->elt,"Boolean.prototype.valueOf called on incompatable object type");
   }
   Asgboolean(RETVAL(jc),b);
}

/* Dispose a Boolean object */
static void Destructor(struct Boolean *b)
{  FREE(b);
}

/* Make (jthis) a new Boolean object */
static void Constructor(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   struct Boolean *b;
   if(jo && jc->flags & EXF_CONSTRUCT)
   {  if(b=ALLOCSTRUCT(Boolean,1,0,jc->pool))
      {  struct Variable *arg;

         jo->internal=b;
         jo->dispose=(Objdisposehookfunc *)Destructor;
         jo->type=OBJT_BOOLEAN;

         arg=jc->functions.first->local.first;
         if(arg->next && arg->val.type!=VTP_UNDEFINED)
         {  Toboolean(&arg->val,jc);
            b->bvalue=jc->val->value.bvalue;
         }
         else
         {  b->bvalue=FALSE;
         }
      }
   }
   else
   if(!(jc->flags & EXF_CONSTRUCT))
   {
        struct Value v = {0};
        struct Variable *arg;
        arg=jc->functions.first->local.first;
        if(arg->next && arg->val.type!=VTP_UNDEFINED)
        {
            Asgvalue(&v,&arg->val);
        }
        Toboolean(&v,jc);
        Asgvalue(RETVAL(jc),&v);
        Clearvalue(&v);
   }
}

/*-----------------------------------------------------------------------*/

void Initboolean(struct Jcontext *jc,struct Jobject *jscope)
{  struct Jobject *jo,*f;
   struct Variable *prop;
   if(jo=Internalfunction(jc,"Boolean",(Internfunc *)Constructor,"BooleanLiteral",NULL))
   {
      Keepobject(jo,TRUE);

      Initconstruct(jc,jo,"Object",jc->object);
      Addprototype(jc,jo,Getprototype(jo->constructor));

      //Addglobalfunction(jc,jo);
      if(!jscope)
      {
          jc->boolean=jo;
      }
      else
      if((prop = Addproperty(jscope,"Boolean")))
      {
           Asgobject(&prop->val,jo);
           prop->flags |= VARF_DONTDELETE;
           Keepobject(jo,FALSE);
      }


      if(f=Internalfunction(jc,"toString",(Internfunc *)Booleantostring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"valueOf",(Internfunc *)Booleanvalueof,NULL))
      {  Addtoprototype(jc,jo,f);
      }
   }
}

/* Create a new Boolean object. */
struct Jobject *Newboolean(struct Jcontext *jc,BOOL bvalue)
{  struct Jobject *jo;
   struct Boolean *b;
   if(jo=Newobject(jc))
   {  Initconstruct(jc,jo,"Boolean",jc->boolean);
      if(b=ALLOCSTRUCT(Boolean,1,0,jc->pool))
      {  jo->internal=b;
         jo->dispose=(Objdisposehookfunc *)Destructor;
         jo->type=OBJT_BOOLEAN;
         b->bvalue=bvalue;
      }
   }
   return jo;
}
