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

/* jstring.c - AWeb js internal String object */

#include "awebjs.h"
#include "jprotos.h"
#include "regexp.h"

#include <proto/locale.h>


struct String           /* Used as internal object value */
{  UBYTE *svalue;
};

/*-----------------------------------------------------------------------*/

/* Find the numeric value of Nth argument */
static double Numargument(struct Jcontext *jc,long n,BOOL *validp)
{  struct Variable *var;
   for(var=jc->functions.first->local.first;n && var->next;var=var->next,n--);
   if(var->next)
   {  Tonumber(&var->val,jc);
      if(var->val.attr==VNA_VALID)
      {  if(validp) *validp=TRUE;
         return var->val.value.nvalue;
      }
   }
   if(validp) *validp=FALSE;
   return 0;
}

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

/* Find the arguments array */
static struct Jobject *Findarguments(struct Jcontext *jc)
{  struct Variable *var;
   for(var=jc->functions.first->local.first;var && var->next;var=var->next)
   {  if(var->name && STRIEQUAL(var->name,"arguments")
      && var->val.type==VTP_OBJECT) return var->val.value.obj.ovalue;
   }
   return NULL;
}

static void Surround(struct Jcontext *jc,UBYTE *s1,UBYTE *s2)
{  struct Jobject *jo=jc->jthis;
   UBYTE *str,*sur;
   long l;
   if(jo && jo->internal)
   {  str=((struct String *)jo->internal)->svalue;
      l=strlen(str)+strlen(s1)+strlen(s2)+5;
      if(sur=ALLOCTYPE(UBYTE,l,0,jc->pool))
      {  sprintf(sur,"<%s>%s<%s>",s1,str,s2);
         Asgstring(RETVAL(jc),sur,jc->pool);
         FREE(sur);
      }
   }
}

static void Surroundattr(struct Jcontext *jc,UBYTE *s1,UBYTE *s2)
{  struct Jobject *jo=jc->jthis;
   UBYTE *attr=Strargument(jc,0);
   UBYTE *str,*sur;
   long l;
   if(jo && jo->internal)
   {  str=((struct String *)jo->internal)->svalue;
      l=strlen(str)+strlen(s1)+strlen(s2)+strlen(attr)+7;
      if(sur=ALLOCTYPE(UBYTE,l,0,jc->pool))
      {  sprintf(sur,"<%s\"%s\">%s<%s>",s1,attr,str,s2);
         Asgstring(RETVAL(jc),sur,jc->pool);
         FREE(sur);
      }
   }
}

/*-----------------------------------------------------------------------*/

/* Get string value of (jthis) */
static void Stringtostring(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   UBYTE *p="";
   if(jo && jo->internal && jo->type == OBJT_STRING )
   {  p=((struct String *)jo->internal)->svalue;
   }
   else
   {
       Runtimeerror(jc,NTE_TYPE,jc->elt,"String.prototype.toString called on incompatable object type");
   }
   Asgstring(RETVAL(jc),p,jc->pool);
}

static void Stringindexof(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   UBYTE *str,*p;
   UBYTE *sval=Strargument(jc,0);
   long start=(long)
   Numargument(jc,1,NULL);
   long index=-1;

   struct Value v = {0};
   Asgobject(&v,jo);
   Tostring(&v,jc);
   str = v.value.svalue;

   if(str && start>=0 && start<strlen(str))
   {  p=str+start;
         if(p=strstr(p,sval))
      {  index=p-str;
      }
   }

   Asgnumber(RETVAL(jc),VNA_VALID,(double)index);
   Clearvalue(&v);
}

static void Stringlastindexof(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   struct Variable *var;
   UBYTE *str,*p;
   UBYTE *sval=Strargument(jc,0);
   long start;
   long l,n,index=-1;

   struct Value v = {0};
   Asgobject(&v,jo);
   Tostring(&v,jc);
   str = v.value.svalue;

      start=strlen(str)-1;
      for(n=1,var=jc->functions.first->local.first;n && var->next;var=var->next,n--);
      if(var->next)
      {  Tonumber(&var->val,jc);
         if(var->val.attr==VNA_VALID)
         {  start=(long)var->val.value.nvalue;
         }
      }
      if(str && start<strlen(str))
      {  p=str+start;
         l=strlen(sval);
         while(p>=str)
         {  if(STRNEQUAL(p,sval,l))
            {  index=p-str;
               break;
            }
            p--;
         }
      }
   Asgnumber(RETVAL(jc),VNA_VALID,(double)index);
   Clearvalue(&v);
}

/* substring(a,b) returns substring from pos a upto b-1 inclusive */
static void Stringsubstring(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   UBYTE *str,*sub;
   BOOL ibvalid=FALSE;
   long ia=(long)Numargument(jc,0,NULL);
   long ib=(long)Numargument(jc,1,&ibvalid);
   long l;

   struct Value v = {0};
   Asgobject(&v,jo);
   Tostring(&v,jc);
   str = v.value.svalue;

      l=strlen(str);
      if(!ibvalid)
      {  ib=l;
      }
      if(ia>ib)
      {  long ic=ia;
         ia=ib;
         ib=ic;
      }
      if(ia<0) ia=0;
      if(ib>l) ib=l;
      Asgstringlen(RETVAL(jc),str+ia,ib-ia,jc->pool);
    Clearvalue(&v);
}

/* substr(a,b) returns substring from pos a upto a+b-1 inclusive */
static void Stringsubstr(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   UBYTE *str,*sub;
   BOOL ibvalid=FALSE;
   long ia=(long)Numargument(jc,0,NULL);
   long ib=(long)Numargument(jc,1,&ibvalid);
   long l;

   struct Value v = {0};
   Asgobject(&v,jo);
   Tostring(&v,jc);
   str = v.value.svalue;

      l=strlen(str);
      if(!ibvalid)
      {  ib=l;
      }
      if(ia<0) ia=0;
      ib+=ia;
      if(ib>l) ib=l;
      Asgstringlen(RETVAL(jc),str+ia,ib-ia,jc->pool);
   Clearvalue(&v);
}

static void Stringcharat(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   UBYTE *str;
   UBYTE buf[2];
   long i=(long)Numargument(jc,0,NULL);

   struct Value v = {0};
   Asgobject(&v,jo);
   Tostring(&v,jc);
   str = v.value.svalue;

      if(i<0 || i>=strlen(str))
      {  buf[0]='\0';
      }
      else
      {  buf[0]=str[i];
         buf[1]='\0';
      }
      Asgstring(RETVAL(jc),buf,jc->pool);
   Clearvalue(&v);
}

/* NB: This next cannot be truly conforming until the libray swaps to UTF16 */

static void Stringcharcodeat(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Value v = {0};
    UBYTE *str;
    BOOL valid;
    int pos;
    UWORD val;

    Asgobject(&v,jo);
    Tostring(&v,jc);

    str = v.value.svalue;

    pos = (int)Numargument(jc,0,&valid);
    if(valid && (pos >= 0) && (pos < strlen(str)))
    {

        val = (UWORD) str[pos];
        Asgnumber(RETVAL(jc),VNA_VALID,(double)val);
    }
    else
    {
        Asgnumber(RETVAL(jc),VNA_NAN,0);
    }
    Clearvalue(&v);
}

static void Stringfromcharcode(struct Jcontext *jc)
{
    struct Jobject *args;
    struct Variable *length;
    struct Variable *var;
    UBYTE *str = NULL;

    if((args=Findarguments(jc)))
    {
        if((length = Getproperty(args,"length")) && length->val.type == VTP_NUMBER)
        {
            int len = (int)length->val.value.nvalue;
            int i;
            if((str = ALLOCTYPE(UBYTE, len + 1, 0, jc->pool)))
            {
                for(i = 0; i <len;i++)
                {
                    var = Arrayelt(args,i);
                    Tonumber(&var->val,jc);
                    if(var->val.attr == VNA_VALID)
                    {
                        str[i] = (UBYTE)var->val.value.nvalue;
                    }
                    else
                    {
                        str[i] = '\0'; // ????
                    }

                }
            }
        }
    }
    if(str == NULL)
    {
        str = Jdupstr("",-1,jc->pool);
    }
    Asgstring(RETVAL(jc),str,jc->pool);
    FREE(str);
}

static void Stringconcat(struct Jcontext *jc)
{
    struct Jobject *jo=jc->jthis,*args;
    struct Variable *var = jc->functions.first->local.first;
    struct Jbuffer *buffer = Newjbuffer(jc->pool);
    UBYTE *result;
    struct Variable *length;
    UBYTE *str;

    struct Value v = {0};
    Asgobject(&v,jo);
    Tostring(&v,jc);
    str = v.value.svalue;


    if(str)
    {
        Addtojbuffer(buffer,str,-1);
        if((args=Findarguments(jc)))
        {
            if((length = Getproperty(args,"length")) && length->val.type==VTP_NUMBER)
            {
                int i;
                for(i=0; i<length->val.value.nvalue;i++)
                {
                    var = Arrayelt(args,i);
                    Tostring(&var->val,jc);
                    Addtojbuffer(buffer,var->val.value.svalue,-1);
                }
            }
        }


    }
    Asgstringlen(RETVAL(jc),buffer->buffer,buffer->length,jc->pool);
    Freejbuffer(buffer);
    Clearvalue(&v);
}

static void Stringtolowercase(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   UBYTE *str,*to,*p;
   struct Value v = {0};
   Asgobject(&v,jo);
   Tostring(&v,jc);
   str = v.value.svalue;

      if(to=Jdupstr(str,-1,jc->pool))
      {  for(p=to;*p;p++) *p=ConvToLower(locale,*p);
         Asgstring(RETVAL(jc),to,jc->pool);
         FREE(to);
      }
   Clearvalue(&v);
}

static void Stringtouppercase(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   UBYTE *str,*to,*p;
   struct Value v = {0};
   Asgobject(&v,jo);
   Tostring(&v,jc);
   str = v.value.svalue;

      if(to=Jdupstr(str,-1,jc->pool))
      {  for(p=to;*p;p++) *p=ConvToUpper(locale,*p);
         Asgstring(RETVAL(jc),to,jc->pool);
         FREE(to);
      }
   Clearvalue(&v);
}

/*
Original javascript 1.1 version kept here.
WE need to add some tecnique to distinguish between versions
as the javascript spec split and some other methods behave differently
*/

#if 0

static void Stringsplit11(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   struct Jobject *array;
   struct Variable *elt;
   UBYTE *str,*sub,*p,*q;
   UBYTE *sep=Strargument(jc,0);

   struct Value v = {0};
   Asgobject(&v,jo);
   Tostring(&v,jc);
   str = v.value.svalue;

      if(array=Newarray(jc))
      {  if(*sep)
         {  p=str;
            for(;;)
            {  if(!(q=strstr(p,sep))) q=p+strlen(p);
               if(sub=Jdupstr(p,q-p,jc->pool))
               {  if(elt=Addarrayelt(jc,array))
                  {  Asgstring(&elt->val,sub,jc->pool);
                  }
                  FREE(sub);
               }
               if(*q)
               {  p=q+strlen(sep);
               }
               else break;
            }
         }
         else
         {  UBYTE c[2]={ 0,0 };
            for(p=str;*p;p++)
            {  c[0]=*p;
               if(elt=Addarrayelt(jc,array))
               {  Asgstring(&elt->val,c,jc->pool);
               }
            }
         }
         Asgobject(RETVAL(jc),array);
      }
      Clearvalue(&v);
}
#endif

/* Javascript >1.2 version */

/* seperator maybe regexp or " " or a string */

static BOOL iswhite(char *q)
{
    if(*q == ' ')return TRUE;
    if(*q == '\n')return TRUE;
    if(*q == '\t')return TRUE;
    if(*q == '\xa0')return TRUE;
    return FALSE;
}

static void Stringsplit(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   struct Jobject *array;
   struct Variable *elt;
   struct Variable *var;

   UBYTE *str,*sub,*p,*q;
   UBYTE *sep = NULL;
   struct Jobject *re = NULL;
   unsigned int limit = 0xffffffff;
   double lim;
   BOOL lvalid;

   struct Value v = {0};
   Asgobject(&v,jo);
   Tostring(&v,jc);
   str = v.value.svalue;

   /* Find out whether the seperator arg is a regexp */

   var = jc->functions.first->local.first;
   if(var->next && var->val.type == VTP_OBJECT)
   {
      if(var->val.value.obj.ovalue->type == OBJT_REGEXP)
      {
        re = var->val.value.obj.ovalue;
      }
      else
      {
        sep = Strargument(jc,0);
      }
   }
   else
   {
      sep = Strargument(jc,0);
   }
   lim = Numargument(jc,1,&lvalid);
   if(lvalid) limit = (unsigned int)lim;


      if(re)
      {
          Asgobject(RETVAL(jc),Splitregexp(jc,re,str,limit));
      }
      else if(array=Newarray(jc))
      {
         if(sep && *sep)
         {  p=str;
            if(iswhite(sep))
            {
                for(;;)
                {
                   if(limit-- == 0) break;
                   for(q=p;!iswhite(q) && q < p +strlen(p); q++);
                   if(elt=Addarrayelt(jc,array))
                   {  Asgstringlen(&elt->val,p,q-p,jc->pool);
                   }
                   while (*q && iswhite(q)) q++;
                   p=q;
                   if(!*q)break;
                }

            }
            else
            {
                for(;;)
                {
                   if(limit-- == 0) break;
                   if(!(q=strstr(p,sep))) q=p+strlen(p);
                   if(elt=Addarrayelt(jc,array))
                   {  Asgstringlen(&elt->val,p,q-p,jc->pool);
                   }
                   if(*q)
                   {  p=q+strlen(sep);
                   }
                   else break;
                }
            }
         }
         else
         {  UBYTE c[2]={ 0,0 };
            for(p=str;*p;p++)
            {
               if(limit-- == 0) break;
               c[0]=*p;
               if(elt=Addarrayelt(jc,array))
               {  Asgstring(&elt->val,c,jc->pool);
               }
            }
         }
         Asgobject(RETVAL(jc),array);
      }
   Clearvalue(&v);
}

static void Stringslice(struct Jcontext *jc)
{
    struct Value v = {0};
    UBYTE *str,*slice;
    long is,ie,l;
    BOOL ievalid;
    Asgobject(&v,jc->jthis);
    Tostring(&v,jc);
    str = v.value.svalue;
    l=strlen(str);
    is = Numargument(jc,0,NULL);
    ie = Numargument(jc,1,&ievalid);
    if(!ievalid)
    {
        ie = l;
    }
    if(is < 0) is += l;
    if(ie < 0 ) ie += l;
    if(ie > is)
    {
        Asgstringlen(RETVAL(jc),str + is, ie - is,jc->pool);
    }
    else
    {
        Asgstring(RETVAL(jc),"",jc->pool);
    }
    Clearvalue(&v);
}

static void Stringsearch(struct Jcontext *jc)
{
    struct Jobject *jo=jc->jthis;
    struct Value v ={0};
    struct Variable *var;
    struct Jobject *re = NULL;
    double index = -1.0;

    UBYTE *pat = NULL;
    UBYTE *str;

    Asgobject(&v,jo);
    Tostring(&v,jc);
    str = v.value.svalue;

    /* is arg a reg exp ? */

    var = jc->functions.first->local.first;
    if(var->next && var->val.type == VTP_OBJECT)
    {
        if(var->val.value.obj.ovalue->type == OBJT_REGEXP)
        {
            re = var->val.value.obj.ovalue;
        }
        else
        {
            pat = Strargument(jc,0);
        }
    }
    else
    {
        pat = Strargument(jc,0);
    }

    if(re)
    {
        /* ECMA says we must ingnore lastIndex and global, but we must not */
        /* change them either so save them away */

        struct Jobject *result;
        BOOL global;

        int lastIndex = ((struct Regexp *)re->internal)->lastIndex;
        ((struct Regexp *)re->internal)->lastIndex = 0;
        /* add save global here */

        global = (((struct Regexp *)re->internal)->flags & REF_GLOBAL)?TRUE:FALSE;

        ((struct Regexp *)re->internal)->flags &= ~REF_GLOBAL;

        if((result = Applyregexp(jc,re,str)))
        {

            struct Variable *var;

            /* extract index here */
            if(var = Getproperty(result, "index"))
            {
                Tonumber(&var->val,jc);
                if(var->val.attr == VNA_VALID)
                {
                    index = var->val.value.nvalue;
                }
            }
            /* dispose of result */
            Remove((struct Node *)result);
            Disposeobject(result);
        }

        ((struct Regexp *)re->internal)->lastIndex = lastIndex;
        if (global) ((struct Regexp *)re->internal)->flags |= REF_GLOBAL;
    }
    else
    {
        /* create a regexp here then apply as above */
        /* dispose of the regexp after we finished with it */
        if(re = Newregexp(jc,pat,""))
        {
            struct Jobject *result;

            if((result = Applyregexp(jc,re,str)))
            {

                struct Variable *var;

                /* extract index here */
                if(var = Getproperty(result, "index"))
                {
                    Tonumber(&var->val,jc);
                    if(var->val.attr == VNA_VALID)
                    {
                        index = var->val.value.nvalue;
                    }
                }

                /* dispose of result */
                Remove((struct Node *)result);
                Disposeobject(result);
            }


            Remove((struct Node *)re);
            Disposeobject(re);
        }
    }

    Clearvalue(&v);
    Asgnumber(RETVAL(jc),VNA_VALID,index);
}

static void Stringmatch(struct Jcontext *jc)
{

    struct Jobject *jo=jc->jthis;
    struct Value v ={0};
    struct Variable *var;
    struct Jobject *re = NULL;
    struct Jobject *result = NULL;
    BOOL reisours = FALSE;

    UBYTE *pat = NULL;
    UBYTE *str;

    Asgobject(&v,jo);
    Tostring(&v,jc);
    str = v.value.svalue;

    /* is arg a reg exp ? */

    var = jc->functions.first->local.first;
    if(var->next && var->val.type == VTP_OBJECT)
    {
        if(var->val.value.obj.ovalue->type == OBJT_REGEXP)
        {
            re = var->val.value.obj.ovalue;
        }
        else
        {
            pat = Strargument(jc,0);
        }
    }
    else
    {
        pat = Strargument(jc,0);
    }

    /* if !re then get our own regexp */
    if(re == NULL)
    {
        reisours = TRUE;
        re = Newregexp(jc,pat,"");
    }
    if(re)
    {
        if(!(((struct Regexp *)re->internal)->flags & REF_GLOBAL))
        {
            /* not global just as exec */
            result = Applyregexp(jc,re,str);
        }
        else
        {
            /* global case */
            if(result = Newarray(jc))
            {
                int prevlastIndex = 0;
                struct Jobject *r = NULL;
                ((struct Regexp *)re->internal)->lastIndex = 0;

                while(r = Applyregexp(jc,re,str))
                {
                    struct Variable *var;
                    struct Variable *elt;

                    if((var  = Arrayelt(r,0)))
                    {
                        if((elt = Addarrayelt(jc,result)))
                        {
                            Asgvalue(&elt->val,&var->val);
                        }
                    }
                    Remove((struct Node *)r);
                    Disposeobject(r);

                    if(((struct Regexp *)re->internal)->lastIndex == prevlastIndex)
                    {
                        ((struct Regexp *)re->internal)->lastIndex++;
                    }
                    prevlastIndex = ((struct Regexp *)re->internal)->lastIndex;

                }

            }

        }
    }
    else
    {
        /* runtime error */
        /* deal with it at some point :-) */
    }
    if(reisours && re)
    {
        Remove((struct Node *)re);
        Disposeobject(re);
    }

    Clearvalue(&v);
    Asgobject(RETVAL(jc),result);
}

/* invoke the provided replace function with attributes from the regexp result */

static void Callrepfunction(struct Jcontext *jc, struct Jobject *funcobj, struct Jobject *functhis, struct Jobject *res, UBYTE *str)
{
    /* the replace function must be called with the args in the following order */
    /* repf(match,captures....,index,string) */
    struct Function *f;
    struct Jobject *oldthis;
    struct Variable* var, *elt ;
    struct Elementfunc *func;
    UWORD oldflags;

    func = funcobj->function;

    if((f = Newfunction(jc,func)))
    {

        int length = 0;
        int i;
        if((var = Getproperty(res, "length")))
        {
            Tonumber(&var->val,jc);
            length=(int)var->val.value.nvalue;
        }

        /* add match and captures */
        for(i=0;i<length;i++)
        {
            elt = Arrayelt(res,i);
            if((var = Newvar(NULL,jc)))
            {
                AddTail((struct List *)&f->local,(struct Node *)var);
                Asgvalue(&var->val, &elt->val);
                {
                    Tostring(&var->val,jc);
                }
            }
            if((var = Addarrayelt(jc,f->arguments)))
            {
                Asgvalue(&var->val, &elt->val);
            }

        }
        if ((elt = Getproperty(res,"index")))
        {
            if((var = Newvar(NULL,jc)))
            {
                AddTail((struct List *)&f->local,(struct Node *)var);
                Asgvalue(&var->val, &elt->val);
            }
            if((var = Addarrayelt(jc,f->arguments)))
            {
                Asgvalue(&var->val, &elt->val);
            }
        }

        if((var = Newvar(NULL,jc)))
        {
            AddTail((struct List *)&f->local,(struct Node *)var);
            Asgstring(&var->val, str,jc->pool);
        }
        if((var = Addarrayelt(jc,f->arguments)))
        {
            Asgstring(&var->val, str,jc->pool);
        }

        AddHead((struct List *)&jc->functions,(struct Node *)f);



        oldthis    =  jc->jthis;
        jc->jthis  =  functhis;
        oldflags   =  jc->flags;
        jc->flags &= ~EXF_CONSTRUCT;

        Executeelem(jc, (struct Element *)func);

        jc->jthis = oldthis;
        jc->flags = oldflags;
        Remove((struct Node *)f);
        Disposefunction(f);
    }
}

static void Callrepfunction2(struct Jcontext *jc, struct Jobject *funcobj, struct Jobject *functhis, UBYTE *match, int index, UBYTE *str)
{
    /* the replace function must be called with the args in the following order */
    /* repf(match,captures....,index,string) */
    /* inthis case though captures = 0 */
    struct Function *f;
    struct Jobject *oldthis;
    struct Elementfunc *func;
    struct Variable *var;

    UWORD oldflags;

    func = funcobj->function;

    if((f = Newfunction(jc,func)))
    {

        if((var = Newvar(NULL,jc)))
        {
            AddTail((struct List *)&f->local,(struct Node *)var);
            Asgstring(&var->val,match,jc->pool);
        }
        if((var = Addarrayelt(jc,f->arguments)))
        {
            Asgstring(&var->val,match,jc->pool);
        }
        if((var = Newvar(NULL,jc)))
        {
            AddTail((struct List *)&f->local,(struct Node *)var);
            Asgnumber(&var->val,VNA_VALID,(double)index);
        }
        if((var = Addarrayelt(jc,f->arguments)))
        {
            Asgnumber(&var->val,VNA_VALID,index);
        }

        if((var = Newvar(NULL,jc)))
        {
            AddTail((struct List *)&f->local,(struct Node *)var);
            Asgstring(&var->val, str,jc->pool);
        }
        if((var = Addarrayelt(jc,f->arguments)))
        {
            Asgstring(&var->val, str,jc->pool);
        }

        AddHead((struct List *)&jc->functions,(struct Node *)f);


        oldthis    =  jc->jthis;
        jc->jthis  =  functhis;
        oldflags   =  jc->flags;
        jc->flags &= ~EXF_CONSTRUCT;

        Executeelem(jc, (struct Element *)func);

        jc->jthis = oldthis;
        jc->flags = oldflags;
        Remove((struct Node *)f);
        Disposefunction(f);
    }
}


/* build the replace string using attributes from the regexp result */


static UBYTE *Doreplacevalue(struct Jcontext *jc, UBYTE *replaceValue, struct Jobject *res, UBYTE *str)
{
    struct Jbuffer *buf = Newjbuffer(jc->pool);
    UBYTE *result;
    UBYTE *p,*q;
    struct Variable *var;


    if(replaceValue)
    {
        p = replaceValue;
        q = p;
        while(*q!=0)
        {
            while (*q  != '$' && *q != 0) q++;
            Addtojbuffer(buf,p,q-p);
            p = q;
            if(*q == '$')
            {
                q++;
                switch( *q)
                {
                    case '&':
                        /* matched susbtring */
                        if((var = Getproperty(res,"0")))
                        {
                            Tostring(&var->val,jc);
                            Addtojbuffer(buf,var->val.value.svalue, -1);
                        }
                        q++;
                        p+=2;
                        break;

                    case '`':
                        /* part of string that precedes match */
                        if((var = Getproperty(jc->regexp,"leftContext")))
                        {

                            Tostring(&var->val,jc);
                            Addtojbuffer(buf,var->val.value.svalue,-1);
                        }
                        q++;
                        p += 2;
                        break;
                    case '\'':
                        /* part of string following natch */
                        if((var = Getproperty(jc->regexp,"rightContext")))
                        {
                            Tostring(&var->val,jc);
                            Addtojbuffer(buf,var->val.value.svalue,-1);
                        }

                        q++;
                        p+=2;
                        break;

                    case '$':
                        q++;
                        p++;
                        break;
                    default:
                        if(isdigit(*q))
                        {
                            int num = *q - '0';
                            q++;
                            p +=2;
                            if(isdigit(*q))
                            {
                                num = 10 * num + *q - '0';
                                q++;
                                p++;
                            }
                            if (var = Arrayelt(res,num))
                            {
                                Tostring(&var->val,jc);
                                Addtojbuffer(buf,var->val.value.svalue,-1);
                            }
                        }

                        break;
                }
            }
        }
        if(p < q && *p != 0)
        Addtojbuffer(buf,p,q-p);


    }

    result = Jdupstr(buf->buffer,-1,jc->pool);
    Freejbuffer(buf);
    return result;
}


static UBYTE *Doreplacevalue2(struct Jcontext *jc, UBYTE *replaceValue, UBYTE* match, int index, UBYTE *str)
{
    struct Jbuffer *buf = Newjbuffer(jc->pool);
    UBYTE *result;
    UBYTE *p,*q;

    if(replaceValue)
    {
        p = replaceValue;
        q = p;
        while(*q!=0)
        {
            while (*q  != '$' && *q != 0) q++;
            Addtojbuffer(buf,p,q-p);
            p = q;
            if(*q == '$')
            {
                q++;
                switch( *q)
                {
                    case '&':
                        /* matched susbtring */
                        Addtojbuffer(buf,match, -1);
                        q++;
                        p+=2;
                        break;

                    case '`':
                        /* part of string that precedes match */
                        Addtojbuffer(buf,str,index);
                        q++;
                        p += 2;
                        break;
                    case '\'':
                        /* part of string following natch */
                        Addtojbuffer(buf,str + index + strlen(match),-1);
                        q++;
                        p+=2;
                        break;

                    case '$':
                        q++;
                        p++;
                        break;
                    default:
                        if(isdigit(*q))
                        {
                            /* can't a have submatches so skip */
                            q++;
                            p +=2;
                            if(isdigit(*q))
                            {
                                q++;
                                p++;
                            }
                        }

                        break;
                }
            }
        }
        if(p < q && *p != 0)
        Addtojbuffer(buf,p,q-p);


    }

    result = Jdupstr(buf->buffer,-1,jc->pool);
    Freejbuffer(buf);
    return result;
}



static void Stringreplace(struct Jcontext *jc)
{
    struct Jobject *jo=jc->jthis;
    struct Value v = {0};
    struct Variable *var;
    struct Jobject *re = NULL;
    struct Jobject *funcobj = NULL;
    struct Jobject *functhis = NULL;

    UBYTE *searchValue = NULL;
    UBYTE *replaceValue = NULL;
    UBYTE *str;
    struct Jbuffer *jb = Newjbuffer(jc->pool);

    Asgobject(&v,jo);
    Tostring(&v,jc);
    str = v.value.svalue;

    /* is first arg a reg exp ? */

    var = jc->functions.first->local.first;
    if(var->next && var->val.type == VTP_OBJECT)
    {
        if(var->val.value.obj.ovalue->type == OBJT_REGEXP)
        {
            re = var->val.value.obj.ovalue;
        }
        else
        {
            searchValue = Strargument(jc,0);
        }
    }
    else
    {
        searchValue = Strargument(jc,0);
    }

    /* is second arg a function or string */

    if(var->next) var = var->next;
    if(var->next && var->val.type == VTP_OBJECT)
    {
        /* this may not be sufficient */
        /* it wasn't hope this is */

        if(var->val.value.obj.ovalue && var->val.value.obj.ovalue->function)
        {
            funcobj  = var->val.value.obj.ovalue;
            functhis = var->val.value.obj.fthis;
        }
        else
        {
            replaceValue = Strargument(jc,1);
        }
    }
    else
    {
        replaceValue = Strargument(jc,1);
    }

    if(re)
    {

        struct Jobject *res = NULL;
        UBYTE *repstr,*match;
        int index,l = 0;;
        index=0;


        if(!(((struct Regexp *)re->internal)->flags & REF_GLOBAL))
        {
            if((res = Applyregexp(jc,re,str)))
            {
                Keepobject(res,TRUE);
                if((var = Getproperty(res,"index")))
                {
                    Tonumber(&var->val,jc);
                    index =(int)var->val.value.nvalue;
                }
                if((var = Getproperty(res,"0")))
                {
                    Tostring(&var->val,jc);
                    match =(UBYTE*)var->val.value.svalue;
                    l=strlen(match);
                }
                if(funcobj)
                {
                    Callrepfunction(jc,funcobj,functhis,res,str);
                    Tostring(jc->val,jc);
                    repstr = Jdupstr(jc->val->value.svalue,-1,jc->pool);

                }
                else
                {
                    repstr = Doreplacevalue(jc,replaceValue,res,str);
                }

                Addtojbuffer(jb,str,index);
                Addtojbuffer(jb,repstr,-1);

                FREE(repstr);
                Keepobject(res,FALSE);
                Remove((struct Node *)res);
                Disposeobject(res);
            }
            Addtojbuffer(jb,str + index + l,strlen(str) - index - l);

        }
        else
        {
            /* global regexp */
            int prevlastIndex = 0;
            int endlastmatch = 0;

            ((struct Regexp *)re->internal)->lastIndex=0;

            while (res = Applyregexp(jc,re,str))
            {
                Keepobject(res,TRUE);
                if((var = Getproperty(res,"index")))
                {
                    Tonumber(&var->val,jc);
                    index =(int)var->val.value.nvalue;
                }
                if((var = Getproperty(res,"0")))
                {
                    Tostring(&var->val,jc);
                    match =(UBYTE*)var->val.value.svalue;
                    l=strlen(match);
                }

                if(funcobj)
                {
                    Callrepfunction(jc,funcobj,functhis,res,str);
                    Tostring(jc->val,jc);
                    repstr = Jdupstr(jc->val->value.svalue,-1,jc->pool);
                }
                else
                {
                    repstr = Doreplacevalue(jc,replaceValue,res,str);
                }

                Addtojbuffer(jb,str + endlastmatch,index - endlastmatch);
                endlastmatch = index + l;

                Addtojbuffer(jb,repstr,-1);
                FREE(repstr);
                Keepobject(res,FALSE);

                Remove((struct Node *)res);
                Disposeobject(res);

                if(((struct Regexp *)re->internal)->lastIndex == prevlastIndex)
                {
                    ((struct Regexp *)re->internal)->lastIndex++;
                }
                prevlastIndex = ((struct Regexp *)re->internal)->lastIndex;
            }
            Addtojbuffer(jb,str + index + l,strlen(str) - index - l);

        }

    }
    else
    if(searchValue)
    {
        /* searchValue is a simple string to find and replace */
       UBYTE *found;
       int index = 0;
       int l = 0;
       UBYTE * repstr = NULL;

       if((found = strstr(str,searchValue)))
       {
           index = found - str;
           l = strlen(searchValue);
           if(funcobj)
           {
               Callrepfunction2(jc,funcobj,functhis,searchValue,index,str);
               Tostring(jc->val,jc);
               repstr = Jdupstr(jc->val->value.svalue,-1,jc->pool);

           }
           else
           {
               repstr = Doreplacevalue2(jc,replaceValue,searchValue,index,str);
           }
           Addtojbuffer(jb,str,index);
           Addtojbuffer(jb,repstr,-1);
           FREE(repstr);

       }
       Addtojbuffer(jb,str + index + l,strlen(str) - index - l);

    }
    Asgstring(RETVAL(jc),jb->buffer,jc->pool);
    Freejbuffer(jb);
    Clearvalue(&v);

}

static void Stringlocalecompare(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Value v ={0};

    UBYTE *str;
    UBYTE *comp;

    LONG result;

    Asgobject(&v,jo);
    Tostring(&v,jc);

    str = v.value.svalue;

    comp = Strargument(jc,0);

    result = StrnCmp(locale,str,comp,-1,SC_COLLATE2);

    Asgnumber(RETVAL(jc),VNA_VALID,(double)result);

    Clearvalue(&v);
}

static void Stringanchor(struct Jcontext *jc)
{  Surroundattr(jc,"A NAME=","/A");
}

static void Stringbig(struct Jcontext *jc)
{  Surround(jc,"BIG","/BIG");
}

static void Stringblink(struct Jcontext *jc)
{  Surround(jc,"BLINK","/BLINK");
}

static void Stringbold(struct Jcontext *jc)
{  Surround(jc,"B","/B");
}

static void Stringfixed(struct Jcontext *jc)
{  Surround(jc,"TT","/TT");
}

static void Stringfontcolor(struct Jcontext *jc)
{  Surroundattr(jc,"FONT COLOR=","/FONT");
}

static void Stringfontsize(struct Jcontext *jc)
{  Surroundattr(jc,"FONT SIZE=","/FONT");
}

static void Stringitalics(struct Jcontext *jc)
{  Surround(jc,"I","/I");
}

static void Stringlink(struct Jcontext *jc)
{  Surroundattr(jc,"A HREF=","/A");
}

static void Stringsmall(struct Jcontext *jc)
{  Surround(jc,"SMALL","/SMALL");
}

static void Stringstrike(struct Jcontext *jc)
{  Surround(jc,"STRIKE","/STRIKE");
}

static void Stringsub(struct Jcontext *jc)
{  Surround(jc,"SUB","/SUB");
}

static void Stringsup(struct Jcontext *jc)
{  Surround(jc,"SUP","/SUP");
}

/* Dispose a String object */
static void Destructor(struct String *s)
{  if(s->svalue) FREE(s->svalue);
   FREE(s);
}

/* Make (jthis) a new String object */
static void Constructor(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   struct String *s;
   struct Variable *prop;
   struct Variable *arg;
   UBYTE *p;

   arg=jc->functions.first->local.first;
   if(arg->next && arg->val.type!=VTP_UNDEFINED)
   {
      Tostring(&arg->val,jc);
      p=arg->val.value.svalue;
   }
   else
   {  p="";
   }
   if(jc->flags&EXF_CONSTRUCT)
   {  if(jo)
      {  if(s=ALLOCSTRUCT(String,1,0,jc->pool))
         {  jo->internal=s;
            jo->dispose=(Objdisposehookfunc *)Destructor;
            jo->type=OBJT_STRING;
            s->svalue=Jdupstr(p,-1,jc->pool);
            if(prop=Addproperty(jo,"length"))
            {  Asgnumber(&prop->val,VNA_VALID,(double)strlen(p));
               prop->hook=Constantvhook;
            }
         }
      }
   }
   else
   {  /* Not called as constructor; return argument string */
      Asgstring(RETVAL(jc),p,jc->pool);
   }
}

/*-----------------------------------------------------------------------*/

void Initstring(struct Jcontext *jc, struct Jobject * jscope)
{  struct Jobject *jo,*f;
   struct Variable *prop;
   if(jo=Internalfunction(jc,"String",(Internfunc *)Constructor,"stringValue",NULL))
   {
      Keepobject(jo,TRUE);

      Initconstruct(jc,jo,"Object",jc->object);
      Addprototype(jc,jo,Getprototype(jo->constructor));

      //Addglobalfunction(jc,jo);
      if(!jscope)
      {
         jc->string=jo;
      }
      else
      if((prop = Addproperty(jscope,"String")))
      {
          Asgobject(&prop->val,jo);
          prop->flags |= VARF_DONTDELETE;
          Keepobject(jo,FALSE);
      }

      if(f=Internalfunction(jc,"toString",(Internfunc *)Stringtostring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"valueOf",(Internfunc *)Stringtostring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"indexOf",(Internfunc *)Stringindexof,"searchValue","fromIndex",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"lastIndexOf",(Internfunc *)Stringlastindexof,"searchValue","fromIndex",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"substring",(Internfunc *)Stringsubstring,"indexA","indexB",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"substr",(Internfunc *)Stringsubstr,"index","length",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"charAt",(Internfunc *)Stringcharat,"index",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"charCodeAt",(Internfunc *)Stringcharcodeat,"pos",NULL))
      {  Addtoprototype(jc,jo,f);
      }

      /* NB the next two pairs of functions are currebtly identical, but */
      /* ultimatley the non Locale versions should use unicode mappings and */
      /* be locale independent */


      if(f=Internalfunction(jc,"toLowerCase",(Internfunc *)Stringtolowercase,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toLocaleLowerCase",(Internfunc *)Stringtolowercase,NULL))
      {  Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"toUpperCase",(Internfunc *)Stringtouppercase,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toLocaleUpperCase",(Internfunc *)Stringtouppercase,NULL))
      {  Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"concat",(Internfunc *)Stringconcat,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"split",(Internfunc *)Stringsplit,"separator",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"slice",(Internfunc *)Stringslice,"beginslice","endslice",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"search",(Internfunc *)Stringsearch,"pattern",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"match",(Internfunc *)Stringmatch,"pattern",NULL))
      {
         Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"replace",(Internfunc *)Stringreplace,"searchValue","replaceValue",NULL))
      {
         Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"localeCompare",(Internfunc *)Stringlocalecompare,"that",NULL))
      {
         Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"anchor",(Internfunc *)Stringanchor,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"big",(Internfunc *)Stringbig,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"blink",(Internfunc *)Stringblink,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"bold",(Internfunc *)Stringbold,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"fixed",(Internfunc *)Stringfixed,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"fontcolor",(Internfunc *)Stringfontcolor,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"fontsize",(Internfunc *)Stringfontsize,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"italics",(Internfunc *)Stringitalics,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"link",(Internfunc *)Stringlink,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"small",(Internfunc *)Stringsmall,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"strike",(Internfunc *)Stringstrike,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"sub",(Internfunc *)Stringsub,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"sup",(Internfunc *)Stringsup,NULL))
      {  Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"fromCharCode",(Internfunc *)Stringfromcharcode,"chars", NULL))
      {
          struct Variable *var;
          if(var =  Addproperty(jo,"fromCharCode"))
          {
            Asgfunction(&var->val,f,jo);
          }
      }



   }
}

/* Create a new String object. */
struct Jobject *Newstring(struct Jcontext *jc,UBYTE *svalue)
{  struct Jobject *jo;
   struct String *s;
   struct Variable *prop;
   if(jo=Newobject(jc))
   {  Initconstruct(jc,jo,"String",jc->string);
      if(s=ALLOCSTRUCT(String,1,0,jc->pool))
      {  jo->internal=s;
         jo->dispose=(Objdisposehookfunc *)Destructor;
         jo->type=OBJT_STRING;
         s->svalue=Jdupstr(svalue,-1,jc->pool);
         if(prop=Addproperty(jo,"length"))
         {  Asgnumber(&prop->val,VNA_VALID,(double)strlen(svalue));
            prop->hook=Constantvhook;
         }
      }
   }
   return jo;
}
