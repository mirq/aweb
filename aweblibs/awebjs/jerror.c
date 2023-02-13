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

/* jerror.c - AWeb js Error object */

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

static void Errortostring(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Variable *var;
    struct Jbuffer *bf;
    if((bf = Newjbuffer(jc->pool)))
    {
        if((var = Getproperty(jo,"name")))
        {
            Tostring(&var->val,jc);
            Addtojbuffer(bf,var->val.value.svalue,-1);
        }
        if((var = Getproperty(jo,"message")))
        {
            Tostring(&var->val,jc);
            if(*var->val.value.svalue != '\0')
            {
                Addtojbuffer(bf,": ",-1);
                Addtojbuffer(bf,var->val.value.svalue,-1);
            }
        }
        Asgstring(RETVAL(jc),bf->buffer,jc->pool);
        Freejbuffer(bf);
    }

    else
    {
        Asgstring(RETVAL(jc),"",jc->pool);
    }

}


/*-----------------------------------------------------------------------*/

/* Make (jthis) a new Error object */
static void Constructor(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Variable *arg;
    arg=jc->functions.first->local.first;


    if(jo && jc->flags & EXF_CONSTRUCT)
    {
        jo->type = OBJT_ERROR;
        if(arg->next && arg->val.type!=VTP_UNDEFINED)
        {
            struct Variable *var;
            Tostring(&arg->val,jc);
            if((var = Addproperty(jo,"message")))
            {
                Asgvalue(&var->val,&arg->val);
            }
        }

    }
    else
    if(!(jc->flags & EXF_CONSTRUCT))
    {
        /* Called as function */
        UBYTE *p = NULL;
        if(arg->next && arg->val.type!=VTP_UNDEFINED)
        {
            Tostring(&arg->val,jc);
            p = arg->val.value.svalue;
        }

        if((jo = Newerror(jc,p)))
        {
            Asgobject(RETVAL(jc),jo);
        }
    }
}

/*-----------------------------------------------------------------------*/

void Initerror(struct Jcontext *jc, struct Jobject *jscope)
{  struct Jobject *jerror,*jo,*f,*p;
   struct Variable *prop;
   if(jerror=Internalfunction(jc,"Error",(Internfunc *)Constructor,"errorMessage",NULL))
   {
      Initconstruct(jc,jerror,"Object",jc->object);
      Addprototype(jc,jerror,Getprototype(jerror->constructor));
      if(jscope && (prop = Addproperty(jscope,"Error")))
      {
          Asgobject(&prop->val,jerror);
          prop->flags |= VARF_DONTDELETE;
      }

      p = Getprototype(jerror);
      if(f=Internalfunction(jc,"toString",(Internfunc *)Errortostring,NULL))
      {  Addtoprototype(jc,jerror,f);
      }
      if((prop = Addproperty(p,"message")))
      {
          Asgstring(&prop->val,"",jc->pool);

      }
      if((prop = Addproperty(p,"name")))
      {
         Asgstring(&prop->val, "Error", jc->pool);
      }

       if(jo=Internalfunction(jc,NTE_TYPE,(Internfunc *)Constructor,"errorMessage",NULL))
       {
          Addprototype(jc,jo,Getprototype(jerror));
    //      Addglobalfunction(jc,jo);
    //      jc->nativeErrors[NTE_TYPE] = jo;
    //      Keepobject(jo,TRUE);

          if(jscope && (prop = Addproperty(jscope,NTE_TYPE)))
          {
              Asgobject(&prop->val,jo);
              prop->flags |= VARF_DONTDELETE;
          }

          p = Getprototype(jo);
          if(f=Internalfunction(jc,"toString",(Internfunc *)Errortostring,NULL))
          {  Addtoprototype(jc,jo,f);
          }
          if((prop = Addproperty(p,"message")))
          {
              Asgstring(&prop->val,"",jc->pool);

          }
          if((prop = Addproperty(p,"name")))
          {
              Asgstring(&prop->val,NTE_TYPE,jc->pool);

          }


       }
       if(jo=Internalfunction(jc,NTE_EVAL,(Internfunc *)Constructor,"errorMessage",NULL))
       {
          Addprototype(jc,jo,Getprototype(jerror));
    //      Addglobalfunction(jc,jo);
    //      jc->nativeErrors[NTE_EVAL] = jo;
    //      Keepobject(jo,TRUE);

          if(jscope && (prop = Addproperty(jscope,NTE_EVAL)))
          {
              Asgobject(&prop->val,jo);
              prop->flags |= VARF_DONTDELETE;
          }


          p = Getprototype(jo);
          if(f=Internalfunction(jc,"toString",(Internfunc *)Errortostring,NULL))
          {  Addtoprototype(jc,jo,f);
          }
          if((prop = Addproperty(p,"message")))
          {
              Asgstring(&prop->val,"",jc->pool);

          }
          if((prop = Addproperty(p,"name")))
          {
              Asgstring(&prop->val,NTE_EVAL,jc->pool);

          }
       }
       if(jo=Internalfunction(jc,NTE_RANGE,(Internfunc *)Constructor,"errorMessage",NULL))
       {
          Addprototype(jc,jo,Getprototype(jerror));
          if(jscope && (prop = Addproperty(jscope,NTE_RANGE)))
          {
              Asgobject(&prop->val,jo);
              prop->flags |= VARF_DONTDELETE;
          }
          p = Getprototype(jo);
          if(f=Internalfunction(jc,"toString",(Internfunc *)Errortostring,NULL))
          {  Addtoprototype(jc,jo,f);
          }
          if((prop = Addproperty(p,"message")))
          {
              Asgstring(&prop->val,"",jc->pool);

          }
          if((prop = Addproperty(p,"name")))
          {
              Asgstring(&prop->val,NTE_RANGE,jc->pool);

          }

       }
       if(jo=Internalfunction(jc,NTE_SYNTAX,(Internfunc *)Constructor,"errorMessage",NULL))
       {
          Addprototype(jc,jo,Getprototype(jerror));
          p = Getprototype(jo);
          if(jscope && (prop = Addproperty(jscope,NTE_SYNTAX)))
          {
              Asgobject(&prop->val,jo);
              prop->flags |= VARF_DONTDELETE;
          }

          if(f=Internalfunction(jc,"toString",(Internfunc *)Errortostring,NULL))
          {  Addtoprototype(jc,jo,f);
          }
          if((prop = Addproperty(p,"message")))
          {
              Asgstring(&prop->val,"",jc->pool);

          }
          if((prop = Addproperty(p,"name")))
          {
              Asgstring(&prop->val,NTE_SYNTAX,jc->pool);

          }


       }
       if(jo=Internalfunction(jc,NTE_REFERENCE,(Internfunc *)Constructor,"errorMessage",NULL))
       {
          Addprototype(jc,jo,Getprototype(jerror));
          p = Getprototype(jo);
          if(jscope && (prop = Addproperty(jscope,NTE_REFERENCE)))
          {
              Asgobject(&prop->val,jo);
              prop->flags |= VARF_DONTDELETE;
          }
          if(f=Internalfunction(jc,"toString",(Internfunc *)Errortostring,NULL))
          {  Addtoprototype(jc,jo,f);
          }
          if((prop = Addproperty(p,"message")))
          {
              Asgstring(&prop->val,"",jc->pool);

          }
          if((prop = Addproperty(p,"name")))
          {
              Asgstring(&prop->val,NTE_REFERENCE,jc->pool);

          }

       }
       if(jo=Internalfunction(jc,NTE_URI,(Internfunc *)Constructor,"errorMessage",NULL))
       {
          Addprototype(jc,jo,Getprototype(jerror));
          if(jscope && (prop = Addproperty(jscope,NTE_URI)))
          {
              Asgobject(&prop->val,jo);
              prop->flags |= VARF_DONTDELETE;
          }

          p = Getprototype(jo);

          if(f=Internalfunction(jc,"toString",(Internfunc *)Errortostring,NULL))
          {  Addtoprototype(jc,jo,f);
          }
          if((prop = Addproperty(p,"message")))
          {
              Asgstring(&prop->val,"",jc->pool);

          }
          if((prop = Addproperty(p,"name")))
          {
              Asgstring(&prop->val,NTE_URI,jc->pool);

          }
       }
   }
}

struct Jobject *Newerror(struct Jcontext *jc, UBYTE *message)
{
    struct Jobject *jo =NULL;
    if((jo = Newobject(jc)))
    {
        Initconstruct(jc,jo,"Error",jc->error);
        if(message)
        {
            struct Variable *var;
            if((var = Addproperty(jo,"message")))
            {
                Asgstring(&var->val,message,jc->pool);
            }
        }
    }
    return jo;
}

struct Jobject *Newnativeerror(struct Jcontext *jc, STRPTR type, UBYTE *message)
{
    struct Jobject *jo =NULL;
    if((jo = Newobject(jc)))
    {
        Initconstruct(jc,jo,type,NULL);
        if(message)
        {
            struct Variable *var;
            if((var = Addproperty(jo,"message")))
            {
                Asgstring(&var->val,message,jc->pool);
            }
        }
    }
    return jo;
}
