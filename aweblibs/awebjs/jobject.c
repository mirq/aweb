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

/* jobject.c - AWeb js generic Object object */

#include "awebjs.h"
#include "jprotos.h"

/* Find the string value of Nth argument */
static UBYTE *Strargument(struct Jcontext *jc,long n)
{  struct Variable *var;
   for(var=jc->functions.first->local.first;n && var->next;var=var->next,n--);
   if(var->next)
   {  Tostring(&var->val,jc);
      return var->val.value.svalue;
   }
   return "";
}

/*--------------------------------------------------------------------------------*/


static void Objectlocalestring(struct Jcontext *jc)
{
    /* default locale string function just calls toString */
    struct Jobject *jo = jc->jthis;
    if( Callproperty(jc,jo,"toString") && jc->val->type == VTP_STRING)
    {
        Asgvalue(RETVAL(jc),jc->val);
    }
    else
    {
        Defaulttostring(jc);
    }
}

static void Objecthasownproperty(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Variable *var;
    UBYTE *propname;
    BOOL *result = FALSE;

    propname = Strargument(jc,0);
    if(propname && (strlen(propname) > 0))
    {
        if((var = Getownproperty(jo,propname)))
        {
            result = TRUE;
        }
    }

    Asgboolean(RETVAL(jc),result);
}

static void Objectpropertyisenum(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Variable *var;
    UBYTE *propname;
    BOOL *result = FALSE;

    propname = Strargument(jc,0);
    if(propname && (strlen(propname) > 0))
    {
        if((var = Getownproperty(jo,propname)))
        {
            if(!(var->flags & VARF_HIDDEN))result = TRUE;
        }
    }

    Asgboolean(RETVAL(jc),result);
}

static void Objectisprototypeof(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Jobject *p;
    BOOL result = FALSE;
    struct Variable *arg=jc->functions.first->local.first;

    if(arg->next && (arg->val.type == VTP_OBJECT) && (p = arg->val.value.obj.ovalue))
    {
        while(p = p->prototype)
        {
            if(jo == p) result = TRUE;
        }
    }
    Asgboolean(RETVAL(jc),result);
}


/*-----------------------------------------------------------------------*/

/* Make (jthis) a new Object object */
static void Constructor(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Jobject *passed = NULL;
    struct Variable *arg;
    arg=jc->functions.first->local.first;

    if(arg->next && arg->val.type!=VTP_UNDEFINED)
    {
        Toobject(&arg->val,jc);
        passed = arg->val.value.obj.ovalue;
    }

    if(jc->flags & EXF_CONSTRUCT)
    {
        if(passed)
        {
            jc->o = passed;
        }
        else
        if(jo)
        {
            jo->type=OBJT_GENERIC;
            jc->o = jo;
        }

    }
    else
    {
        /* Called as function */
        if(passed)
        {
            Asgobject(RETVAL(jc),passed);
        }
        else
        {
            if((jo = Newobject(jc)))
            {
                jo->type = OBJT_GENERIC;
                Initconstruct(jc,jo,"Object",jc->object);
                Asgobject(RETVAL(jc),jo);
            }
        }
    }
}

/*-----------------------------------------------------------------------*/

void Initobject(struct Jcontext *jc, struct Jobject *jscope)
{  struct Jobject *jo,*f;
   struct Variable *prop;
   if(jo=Internalfunction(jc,"Object",(Internfunc *)Constructor,"objValue",NULL))
   {
      Keepobject(jo,TRUE);

      Addprototype(jc,jo,NULL);

     // Addglobalfunction(jc,jo);
      if(!jscope)
      {
         jc->object=jo;
      }
      else
      if((prop = Addproperty(jscope,"Object")))
      {
          Asgobject(&prop->val,jo);
          prop->flags |= VARF_DONTDELETE;
          Keepobject(jo,FALSE);
      }

      if(f=Internalfunction(jc,"toLocaleString",(Internfunc *)Objectlocalestring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"hasOwnProperty",(Internfunc *)Objecthasownproperty,"property",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"propertyIsEnumerable",(Internfunc *)Objectpropertyisenum,"property",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"isPrototypeOf",(Internfunc *)Objectisprototypeof,"object",NULL))
      {  Addtoprototype(jc,jo,f);
      }


      Initconstruct(jc,jo,NULL,jo);

   }
}
