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

/* jnumber.c - AWeb js internal Number object */

#include "awebjs.h"
#include "jprotos.h"
#include <math.h>
#include <float.h>
#include <string.h>


struct Number           /* Used as internal object value */
{  UBYTE attr;
   double nvalue;
};

/*-----------------------------------------------------------------------*/

/* Add a constant property */
static void Addnumberproperty(struct Jobject *jo,UBYTE *name,UBYTE attr,double n)
{  struct Variable *var;
   if(var=Addproperty(jo,name))
   {  Asgnumber(&var->val,attr,n);
      var->hook=Constantvhook;
   }
}

/*-----------------------------------------------------------------------*/
UBYTE digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

/* Calculate max number of digits eg 15 for base 10 52(50) for base 2 */
/* Maybe we ought to use a lookup table for speed, later */
static int maxdigits(int radix)
{
    double maxnum = 999999999999999.0;
    int i =0;
    while(maxnum > 1.0)
    {
        maxnum /= (double)radix;
        i++;
    }
    return i;
}

/* The toString(radix) where radix != 10 works well for numbers < 10^15 but needs work for greater numbers */

/* Convert (jthis) to string */
static void Numbertostring(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   UBYTE buffer[64];
   int radix = 10;

   struct Variable *arg;
   struct Number *n;

   arg=jc->functions.first->local.first;
   if(arg->next && arg->val.type!=VTP_UNDEFINED)
   {
        Tonumber(&arg->val,jc);
        if(arg->val.attr == VNA_VALID)
        {
            radix = (int)arg->val.value.nvalue;
        }
   }

   if(jo && jo->type == OBJT_NUMBER && (n=(struct Number *)jo->internal))
   {  switch(n->attr)
      {  case VNA_NAN:
            strcpy(buffer,"NaN");
            break;
         case VNA_INFINITY:
            strcpy(buffer,"+Infinity");
            break;
         case VNA_NEGINFINITY:
            strcpy(buffer,"-Infinity");
            break;
         default:
            if(radix == 10)
            {
                sprintf(buffer,"%.15lg",n->nvalue);
            }
            else if(radix >= 2 && radix <= 36)
            {
                UBYTE *p = buffer;
                int i;
                int r;
                int maxd;
                double number1 = n->nvalue;
                double number2;

                if(number1 < 0.0)
                {
                    *p++ = '-';
                    number1 = -(number1);

                }
                maxd = maxdigits(radix);

                /* count the digits of the "whole" part */
                number2 = number1;
                for(r = 0;number1 > radix ; r++,number1 /= (double)radix);
                number1 = number2;

                for(i=r;i>=0;i--)
                {
                    int digit;
                    digit = (int)fmod(number1,(double)radix);
                    if((p + i) < buffer + sizeof(buffer) -1)
                    {
                        *(p + i) = digits[digit];
                    }
                    number1 = floor(number1 / (double)radix);
                }
                /* now do the fractional part */
                if(r < sizeof(buffer)-2)
                {
                    p = buffer + r +1;
                    *p++ = '.';
                    number1 = number2 - floor(number2);
                    while(p < buffer + sizeof(buffer) -1)
                    {
                        number1 *= (double)radix;
                        *p++ = digits[(int)floor(number1)];
                        number1 -= floor(number1);
                    }
                }
                buffer[sizeof(buffer) -1] = '\0';
                /* now strip trailing zeros and possible the "decimal" point */
                if(strchr(buffer,'.'))
                {
                    /* start at the least significant end and replace each '0' */
                    /* with a '\0' (null) if we find a '.' imediatly after a '0' */
                    /* replace that too */
                    for(i = sizeof(buffer)-2; i >= 0; i--)
                    {
                        if(buffer[i] == '0' || i > maxd)
                        {
                            buffer[i] = '\0';
                            if(i > 0 && buffer[i-1] == '.')
                            {
                                buffer[i-1] = '\0';
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                Runtimeerror(jc,NTE_RANGE,jc->elt,"Radix %d must be between 2 and 36",radix);
            }
            break;
      }
      Asgstring(RETVAL(jc),buffer,jc->pool);
   }
   else
   {
      Runtimeerror(jc,NTE_TYPE,jc->elt,"Number.prototype.toString called with invalid object");
   }
}

static void Numbertolocalestring(struct Jcontext *jc)
{
     /* IMplement this! */
     /* Use locale structure */
     /* For now spec allows me to return toString instead */
}

static void Numbertofixed(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    int fractDigits =-1;
   struct Number *n;
   struct Variable *arg;
   struct Jbuffer *buffer = Newjbuffer(jc->pool);

   arg=jc->functions.first->local.first;
   if(arg->next && arg->val.type!=VTP_UNDEFINED)
   {
        Tonumber(&arg->val,jc);
        if(arg->val.attr == VNA_VALID)
        {
            fractDigits = (int)arg->val.value.nvalue;
        }
   }

   if(jo && (n=(struct Number *)jo->internal))
   {  switch(n->attr)
      {  case VNA_NAN:
            Addtojbuffer(buffer,"NaN",-1);
            break;
         case VNA_INFINITY:
            Addtojbuffer(buffer,"+Infinity",-1);
            break;
         case VNA_NEGINFINITY:
            Addtojbuffer(buffer,"-Infinity",-1);
            break;
         default:
            {
                double num = n->nvalue;
                int numlen;
                int i;
                int sigfig = 0;

                /* If out of range */
                if(num < -1.0e21 || num > 1.0e21 || fractDigits < 0 || fractDigits > 20)
                {
                    struct Value v = {0};
                    Asgnumber(&v,VNA_VALID,num);
                    Tostring(&v,jc);
                    Addtojbuffer(buffer,v.value.svalue,-1);
                    Clearvalue(&v);
                }
                else
                {
                    /* now for real */
                    if(num < 0.0)
                    {
                        Addtojbuffer(buffer,"-",-1);
                        num = -num;
                    }
                    /* Find the number of digits to left of nums decimal */
                    numlen = (int)floor(log10(num));
                    for(i = numlen;i >=0;i--)
                    {
                        double pten = pow(10.0,(double)i);
                        double dig = floor(num / pten);
                        num = num - dig * pten;
                        if(sigfig++ < 17)
                        {
                            Addtojbuffer(buffer,&digits[(int)dig],1);
                        }
                        else
                        {
                            Addtojbuffer(buffer,"0",1);

                        }
                    }
                    if(fractDigits > 0)
                    {
                        Addtojbuffer(buffer,".",-1);
                        num = (n->nvalue - floor(n->nvalue))*pow(10.0,(int)fractDigits);
                        for(i=fractDigits-1;i>=0;i--)
                        {
                            double pten = pow(10.0,(double)i);
                            double dig = floor(num / pten);
                            num = (num - dig * pten);
                            if(sigfig++ < 17)
                            {
                                Addtojbuffer(buffer,&digits[(int)dig],1);
                            }
                            else
                            {
                                Addtojbuffer(buffer,"0",1);

                            }

                        }
                    }

                }

            }
            break;
       }
       Asgstring(RETVAL(jc),buffer->buffer,jc->pool);
       Freejbuffer(buffer);
   }

}


/* Get value of (jthis) */
static void Numbervalueof(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   double n=0.0;
   UBYTE attr=VNA_NAN;
   if(jo && jo->internal && jo->type == OBJT_NUMBER)
   {  n=((struct Number *)jo->internal)->nvalue;
      attr=((struct Number *)jo->internal)->attr;
   }
   else
   {
        Runtimeerror(jc,NTE_TYPE,jc->elt,"Number.prototype.valueOf called with incomptable object");
   }
   Asgnumber(RETVAL(jc),attr,n);
}

/* Dispose a Number object */
static void Destructor(struct Number *n)
{  FREE(n);
}

/* Make (jthis) a new Number object */
static void Constructor(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   struct Number *n;
   if(jo && jc->flags & EXF_CONSTRUCT)
   {  if(n=ALLOCSTRUCT(Number,1,0,jc->pool))
      {  struct Variable *arg;
         jo->internal=n;
         jo->dispose=(Objdisposehookfunc *)Destructor;
         jo->type=OBJT_NUMBER;

         arg=jc->functions.first->local.first;
         if(arg->next && arg->val.type!=VTP_UNDEFINED)
         {  Tonumber(&arg->val,jc);
            n->attr=jc->val->attr;
            n->nvalue=jc->val->value.nvalue;
         }
         else
         {  n->attr=VNA_VALID;
            n->nvalue=0.0;
         }
      }
   }
   else if(!(jc->flags & EXF_CONSTRUCT))
   {
        struct Variable *arg;
        struct Value v = {0};
        Asgnumber(&v,VNA_VALID,0.0);
        arg=jc->functions.first->local.first;
        if(arg->next && arg->val.type!=VTP_UNDEFINED)
        {
           Asgvalue(&v,&arg->val);
           Tonumber(&v,jc);
        }
        Asgvalue(RETVAL(jc),&v);
        Clearvalue(&v);
   }
}

/*-----------------------------------------------------------------------*/

void Initnumber(struct Jcontext *jc, struct Jobject *jscope)
{  struct Jobject *jo,*f;
   struct Variable *prop;
   if(jo=Internalfunction(jc,"Number",(Internfunc *)Constructor,"NumericLiteral",NULL))
   {
      Keepobject(jo,TRUE);

      Initconstruct(jc,jo,"Object",jc->object);
      Addprototype(jc,jo,Getprototype(jo->constructor));
//      Addglobalfunction(jc,jo);
      if(!jscope)
      {
         jc->number=jo;
      }
      else
      if((prop = Addproperty(jscope,"Number")))
      {
          Asgobject(&prop->val,jo);
          prop->flags |= VARF_DONTDELETE;
          Keepobject(jo,FALSE);
      }

      Addnumberproperty(jo,"MAX_VALUE",VNA_VALID,DBL_MAX);
      Addnumberproperty(jo,"MIN_VALUE",VNA_VALID,DBL_MIN);
      Addnumberproperty(jo,"NaN",VNA_NAN,0.0);
      Addnumberproperty(jo,"NEGATIVE_INFINITY",VNA_NEGINFINITY,0.0);
      Addnumberproperty(jo,"POSITIVE_INFINITY",VNA_INFINITY,0.0);
      if(f=Internalfunction(jc,"toString",(Internfunc *)Numbertostring,"radix",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toLocaleString",(Internfunc *)Numbertostring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toFixed",(Internfunc *)Numbertofixed,"fixedDigits",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"valueOf",(Internfunc *)Numbervalueof,NULL))
      {  Addtoprototype(jc,jo,f);
      }
   }
}

/* Create a new Number object. */
struct Jobject *Newnumber(struct Jcontext *jc,UBYTE attr,double nvalue)
{  struct Jobject *jo;
   struct Number *n;
   if(jo=Newobject(jc))
   {  Initconstruct(jc,jo,"Number",jc->number);
      if(n=ALLOCSTRUCT(Number,1,0,jc->pool))
      {  jo->internal=n;
         jo->dispose=(Objdisposehookfunc *)Destructor;
         jo->type=OBJT_NUMBER;
         n->attr=attr;
         n->nvalue=nvalue;
      }
   }
   return jo;
}
