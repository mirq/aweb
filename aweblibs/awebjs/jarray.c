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

/* jarray.c - AWeb js internal Array object */

#include "awebjs.h"
#include "jprotos.h"

#define CHUNKSIZE   16  /* Array storage is allocated in multiples of this */

struct Array            /* Used as internal object value */
{
    long length;            /* current array length of data*/
    struct Variable **array; /* pointer to current array */
    long array_length;       /* current length of storage */
};

struct Tabelt           /* Array of this is used to reverse or sort the array */
{  struct Asortinfo *asi;  /* All tabelts point to the same Asortinfo */
   struct Value val;
   UBYTE *svalue;       /* Result of toString or NULL */
};

struct Asortinfo        /* Common info while sorting */
{  struct Jcontext *jc;
   struct Elementfunc *cfunc; /* Sort compare function or NULL */
   struct Jobject *cthis;     /* (this) for compare function */
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



/* Find the string value of Nth argument or NULL */
static UBYTE *Strargument(struct Jcontext *jc,long n)
{  struct Variable *var;
   for(var=jc->functions.first->local.first;n && var->next;var=var->next,n--);
   if(var->next && var->val.type!=VTP_UNDEFINED)
   {  Tostring(&var->val,jc);
      return var->val.value.svalue;
   }
   return NULL;
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

/* Make a Tabelt array from an Array object. (asi->jc) must be set. */
struct Tabelt *Maketabelts(struct Asortinfo *asi,struct Jobject *jo,long n)
{
   struct Variable *elt;
   struct Tabelt *tabelts;
   long i;
   UBYTE nname[16];

   if(tabelts=ALLOCSTRUCT(Tabelt,n,0,asi->jc->pool))
   {  for(i=0;i<n;i++)
      {
         sprintf(nname,"%d",i);
         if(elt=Getproperty(jo,nname))
         {
             Asgvalue(&tabelts[i].val,&elt->val);
         }
         tabelts[i].asi=asi;
      }
   }
   return tabelts;
}

/* Assign the Tabelt elements back to the Array object */
void Returntabelts(struct Jobject *jo,struct Tabelt *tabelts,long n)
{  struct Variable *elt;
   UBYTE nname[16];
   long i;
   for(i=0;i<n;i++)
   {  sprintf(nname,"%d",(int)i);
      if(!(elt=Getownproperty(jo,nname)))
      {  elt=Addproperty(jo,nname);
      }
      if(elt)
      {  Asgvalue(&elt->val,&tabelts[i].val);
      }
      Clearvalue(&tabelts[i].val);
      if(tabelts[i].svalue) FREE(tabelts[i].svalue);
   }
   FREE(tabelts);
}

/* General qsort compare function */
static int Asortcompare(struct Tabelt *ta,struct Tabelt *tb)
{  struct Value val = {0};
   int c;
   if(ta->val.type == VTP_UNDEFINED && tb->val.type == VTP_UNDEFINED)
   {
      return 0;
   }
   if(ta->val.type == VTP_UNDEFINED) return(1);
   if(tb->val.type == VTP_UNDEFINED) return(-1);


   if(ta->asi->cfunc)
   {  /* Compare function supplied, use it */
      Callfunctionargs(ta->asi->jc,ta->asi->cfunc,ta->asi->cthis,&ta->val,&tb->val,NULL);
      Tonumber(ta->asi->jc->val,ta->asi->jc);
      c=(int)ta->asi->jc->val->value.nvalue;
   }
   else
   {  /* Convert both elements to string and do string compare */
      if(!ta->svalue)
      {  val.type=0;
         Asgvalue(&val,&ta->val);
         Tostring(&val,ta->asi->jc);
         ta->svalue=Jdupstr(val.value.svalue,-1,ta->asi->jc->pool);
         Clearvalue(&val);
      }
      if(!tb->svalue)
      {  val.type=0;
         Asgvalue(&val,&tb->val);
         Tostring(&val,tb->asi->jc);
         tb->svalue=Jdupstr(val.value.svalue,-1,tb->asi->jc->pool);
         Clearvalue(&val);
      }
      c=strcmp(ta->svalue,tb->svalue);
   }
   return c;
}

/*-----------------------------------------------------------------------*/

/* Do the actual array joining, adapted to be generic */
static void Joinarray(struct Jcontext *jc,UBYTE *sep)
{  struct Jobject *jo=jc->jthis;
   struct Variable *elt;
   struct Variable *length;
   struct Value val;
   struct Jbuffer *buf;
   unsigned long n,len,sepl;
   UBYTE nname[16];

   if(!sep) sep=",";
   sepl=strlen(sep);
   val.type=0;
   len = 0;
   if((length = Getproperty(jo,"length")))
   {
       Tonumber(&length->val,jc);
       len = (unsigned long)length->val.value.nvalue;
   }
      if(buf=Newjbuffer(jc->pool))
      {  for(n=0;n<len;n++)
         {
            sprintf(nname,"%d",n);
            if((elt=Getproperty(jo,nname)) && elt->val.type != VTP_UNDEFINED)
            {
               Asgvalue(&val,&elt->val);
               Tostring(&val,jc);
               if(n>0) Addtojbuffer(buf,sep,sepl);
               Addtojbuffer(buf,val.value.svalue,strlen(val.value.svalue));
            }
            else
            {
                if(n>0) Addtojbuffer(buf,sep,sepl);
            }
         }
         Asgstring(RETVAL(jc),buf->buffer?buf->buffer:(UBYTE *)"",jc->pool);
         Freejbuffer(buf);
      }
}

static void Arraytostring(struct Jcontext *jc)
{
     struct Jobject *jo = jc->jthis;
     if(jo && jo->internal && Isarray(jo))
     {
         Joinarray(jc,",");
     }
     else
     {
          Runtimeerror(jc,NTE_TYPE,jc->elt,"Array.prototype.toString called on incompatable object");
     }
}

/* This function is not generic so we can use Arrelt etc. */
static void Arraytolocalestring(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    ULONG len;
    ULONG i;
    struct Jbuffer *buf;
    struct Variable *elt;

    UBYTE *sep = ","; /* Could use group sepaerator from struct Locale */

    if(jo && jo->internal && Isarray(jo))
    {
        len = ((struct Array *)jo->internal)->length;
        if((buf = Newjbuffer(jc->pool)))
        {
            for(i = 0; i < len; i++)
            {
                if((elt = Arrayelt(jo,i)))
                {
                    struct Value v = {0};
                    Asgvalue(&v,&elt->val);
                    Toobject(&v,jc);
                    if(i> 0) Addtojbuffer(buf,sep,-1);
                    if(v.value.obj.ovalue && Callproperty(jc,v.value.obj.ovalue,"toLocaleString") && jc->val->type == VTP_STRING)
                    {
                        Addtojbuffer(buf,jc->val->value.svalue,-1);
                    }
                    Clearvalue(&v);
                }
                else
                {
                    if(i > 0) Addtojbuffer(buf,sep,-1);
                }
            }
            Asgstring(RETVAL(jc),buf->buffer?buf->buffer:(UBYTE *)"",jc->pool);
            Freejbuffer(buf);
        }
    }
    Runtimeerror(jc,NTE_TYPE,jc->elt,"Array.prototype.toLocaleString called on incompatable object");
}

static void Arrayjoin(struct Jcontext *jc)
{  UBYTE *sep=Strargument(jc,0);
   Joinarray(jc,sep);
}

static void Arrayreverse(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   struct Variable *hivar,*lovar;
   struct Variable *ownhivar,*ownlovar;
   struct Variable *length;
   struct Value v = {0};
   ULONG len,i,j;
   UBYTE lname[16];
   UBYTE hname[16];
   len = 0;

    if((length = Getproperty(jo,"length")))
    {
        Tonumber(&length->val,jc);
        len = (ULONG)length->val.value.nvalue;
    }

    for(i=0,j=len-1;i<len/2;i++,j--)
    {
        sprintf(lname,"%d",(int)i);
        lovar = Getproperty(jo,lname);
        sprintf(hname,"%d",(int)j);
        hivar = Getproperty(jo,hname);
        if(lovar && hivar)
        {
            Asgvalue(&v,&lovar->val);
            if(!(ownlovar = Getownproperty(jo,lname)))
            {
                ownlovar = Addproperty(jo,lname);
            }
            if(ownlovar)
            {
                Asgvalue(&ownlovar->val,&hivar->val);
            }
            if(!(ownhivar = Getownproperty(jo,hname)))
            {
                ownhivar = Addproperty(jo,hname);
            }
            if(ownhivar)
            {
                Asgvalue(&ownhivar->val,&v);
            }
            Clearvalue(&v);
        }
        else
        if(lovar)
        {
            struct Variable *var;
            if((var  = Addproperty(jo,hname)))
            {
                Asgvalue(&var->val,&lovar->val);
            }

            if(!(ownlovar = Getownproperty(jo,lname)))
            {
                ownlovar = Addproperty(jo,lname);
            }
            if(ownlovar)
            {
                Clearvalue(&ownlovar->val);
            }
        }
        else
        if(hivar)
        {
            struct Variable *var;
            if((var  = Addproperty(jo,lname)))
            {
                Asgvalue(&var->val,&hivar->val);
            }
            if(!(ownhivar = Getownproperty(jo,hname)))
            {
                ownhivar = Addproperty(jo,hname);
            }
            if(ownhivar)
            {
                Clearvalue(&ownhivar->val);
            }


        }

    }

    Asgobject(RETVAL(jc),jo);
}


static void Arraysort(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   struct Tabelt *tabelts;
   struct Asortinfo asi={0};
   struct Variable *cvar;
   struct Variable *length;
   ULONG len = 0;

   if((length = Getproperty(jo,"length")))
   {
      Tonumber(&length->val,jc);
      len = (ULONG)length->val.value.nvalue;
   }

   if(len > 1)
   {
      asi.jc=jc;
      if((cvar=jc->functions.first->local.first) && cvar->next)
      {  Tofunction(&cvar->val,jc);
         if(cvar->val.value.obj.ovalue)
         {  asi.cfunc=cvar->val.value.obj.ovalue->function;
            asi.cthis=cvar->val.value.obj.fthis;
         }
      }
      if(tabelts=Maketabelts(&asi,jo,len))
      {  /* sort the table */
         qsort(tabelts,len,sizeof(struct Tabelt),Asortcompare);
         Returntabelts(jo,tabelts,len);
      }
   }
   Asgobject(RETVAL(jc),jo);
}

/* add all the elements of array1 to array2 */

static void Addelements(struct Jcontext *jc, struct Jobject *to, struct Jobject *from)
{
    /* we assume that we've been passed valid arrays */
    int length = ((struct Array *)from->internal)->length;
    int i;

    for(i = 0; i < length; i++)
    {
        struct Variable *elt1,*elt2;
        if((elt2 = Addarrayelt(jc,to)))
        {
            if((elt1 = Arrayelt(from,i)))
            {
                Asgvalue(&elt2->val,&elt1->val);
            }
        }
    }


}

static void Arrayconcat(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Jobject *newarray = NULL;
    struct Jobject *args;
    struct Variable *elt;
    struct Variable *arg;

    if((newarray = Newarray(jc)))
    {
        if(Isarray(jo))
        {
            Addelements(jc,newarray,jo);
        }
        else
        {
            struct Value v = {0};
            Asgobject(&v,jo);
            Tostring(&v,jc);
            if((elt = Addarrayelt(jc,newarray)))
            {
                Asgvalue(&elt->val,&v);
            }
            Clearvalue(&v);
        }
        /* Now do the same with the args */
        if((args = Findarguments(jc)))
        {
            struct Variable *length;
            if((length = Getproperty(args,"length")) && length->val.type == VTP_NUMBER)
            {
                int l = (int)length->val.value.nvalue;
                int i;
                for(i = 0; i < l ; i++)
                {
                    arg = Arrayelt(args,i);
                    if(arg->val.type == VTP_OBJECT && Isarray(arg->val.value.obj.ovalue))
                    {
                        Addelements(jc,newarray,arg->val.value.obj.ovalue);
                    }
                    else
                    {
                        Tostring(&arg->val,jc);
                        if((elt = Addarrayelt(jc,newarray)))
                        {
                            Asgvalue(&elt->val,&arg->val);
                        }
                    }
                }
            }
        }

    }
    Asgobject(RETVAL(jc),newarray);
}

static void Arraypop(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Variable *length;

    struct Value result = {0};
    result.type = VTP_UNDEFINED;

    if((length = Getproperty(jo,"length")))
    {
        ULONG len =0;
        Tonumber(&length->val,jc);
        if(length->val.attr == VNA_VALID)
        {
            len = (ULONG)length->val.value.nvalue;
            if(len > 0)
            {
                UBYTE nname[16];
                struct Variable *elt;
                len --;
                sprintf(nname,"%d",(int)len);
                if((elt = Getproperty(jo,nname)))
                {
                    Asgvalue(&result,&elt->val);
                    Deleteownproperty(jo,nname);
                    if(Isarray(jo))
                    {
                        ((struct Array *)jo->internal)->length--;
                    }
                }
            }
        }
        if(!(length = Getownproperty(jo,"length")))
        {
            length = Addproperty(jo,"length");
        }
        if(length)
        {
            Asgnumber(&length->val,VNA_VALID,(double)len);
        }
    }
    else
    {
        if(length = Addproperty(jo,"length"))
        {
            Asgnumber(&length->val,VNA_VALID,0);
        }
    }
    Asgvalue(RETVAL(jc),&result);

    Clearvalue(&result);

}

static void Arraypush(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Variable *length,*argslen;
    struct Jobject *args;
    ULONG len = 0;

    if((length = Getproperty(jo,"length")))
    {
        Tonumber(&length->val,jc);
        if(length->val.attr == VNA_VALID)
        {
            len = (ULONG)length->val.value.nvalue;
        }
    }
    if((args = Findarguments(jc)))
    {
        if((argslen = Getproperty(args,"length")) && argslen->val.type == VTP_NUMBER )
        {
            int i;
            for(i = 0; i < (int)argslen->val.value.nvalue;i++)
            {
                struct Variable *arg,*elt;
                arg = Arrayelt(args,i);
                if(Isarray(jo))
                {
                    if((elt = Addarrayelt(jc,jo)))
                    {
                        Asgvalue(&elt->val,&arg->val);
                    }
                }
                else
                {
                    /* This Object is not a true array so do it the hard way */
                    UBYTE nname[16];
                    sprintf(nname,"%d",(int)len);
                    if((elt = Getownproperty(jo,nname)))
                    {
                        Asgvalue(&elt->val,&arg->val);
                    }
                    else
                    {
                        if((elt = Addproperty(jo,nname)))
                        {
                            Asgvalue(&elt->val,&arg->val);
                        }
                    }
                }
                len++;
            }
        }
    }
    if(!(length = Getownproperty(jo,"length")))
    {
        length = Addproperty(jo,"length");
    }
    if(length)
    {
        Asgnumber(&length->val,VNA_VALID,len);
    }
    Asgnumber(RETVAL(jc),VNA_VALID,len);
}

static void Arrayshift(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Variable *length;
    struct Variable *var;
    struct Variable *shift;
    ULONG len = 0;
    ULONG i;
    UBYTE nname[16];

    struct Value result = {0};
    result.type = VTP_UNDEFINED;

    if((length = Getproperty(jo,"length")))
    {
        Tonumber(&length->val,jc);
        if(length->val.attr == VNA_VALID)
        {
            len = (ULONG)length->val.value.nvalue;
        }
        if(len >0)
        {
            if((var = Getproperty(jo,"0")))
            {
                Asgvalue(&result,&var->val);
            }
            /* we've got our value now we need to shift the array members */
            /* down one, and adjust the internal length value if a true array */

            for(i = 1; i < len;i++)
            {
                sprintf(nname,"%d",(int)i);
                var = Getproperty(jo,nname);
                sprintf(nname,"%d",(int)i-1);
                if(!(shift = Getownproperty(jo,nname)))
                {
                    shift = Addproperty(jo,nname);
                }
                if(var)
                {
                    if(shift)
                    {
                        Asgvalue(&shift->val,&var->val);
                    }
                }
                else if(shift)
                {
                    Clearvalue(&shift->val);
                }
            }
            len--;
            sprintf(nname,"%d",(int)len);
            if((Deleteownproperty(jo,nname)))
            {
                if(Isarray(jo))
                {
                    ((struct Array *)jo->internal)->length--;
                }
            }
        }
        if(!(length = Getownproperty(jo,"length")))
        {
            length = Addproperty(jo,"length");
        }
        if(length)
        {
            Asgnumber(&length->val,VNA_VALID,len);
        }
    }
    else
    {
        if(length = Addproperty(jo,"length"))
        {
            Asgnumber(&length->val,VNA_VALID,len);
        }
    }
    Asgvalue(RETVAL(jc),&result);
    Clearvalue(&result);
}

static void Arrayunshift(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Jobject *args;

    struct Variable *length,*argslen;
    struct Variable *var,*shift;
    ULONG len = 0;
    ULONG arglen=0;
    ULONG i;
    UBYTE nname[16];


    if((length = Getproperty(jo,"length")))
    {
        Tonumber(&length->val,jc);
        if(length->val.attr == VNA_VALID)
        {
            len = (ULONG)length->val.value.nvalue;
        }
    }
    if((args = Findarguments(jc)))
    {
        if((argslen = Getproperty(args,"length")) && argslen->val.type == VTP_NUMBER)
        {
            arglen = (ULONG)argslen->val.value.nvalue;
        }
        if(arglen > 0)
        {
            /* shift all the args up by arglen places */
            /* adjust internal length if tue array */
            for(i = len; i > 0; i--)
            {
                sprintf(nname,"%d",(int)(i-1));
                var = Getproperty(jo,nname);
                sprintf(nname,"%d",(int)(i + arglen -1));
                if(!(shift = Getownproperty(jo,nname)))
                {
                    shift = Addproperty(jo,nname);
                }
                if(var)
                {
                    if(shift)
                    {
                        Asgvalue(&shift->val,&var->val);
                    }
                }
                else if(shift)
                {
                    Clearvalue(&shift->val);
                }

            }
            len += arglen;
            if(Isarray(jo))
            {
                ((struct Array *)jo->internal)->length += arglen;
            }
            for(i = 0;i<arglen;i++)
            {
                var = Arrayelt(args,i);
                sprintf(nname,"%d",(int)i);
                if(!(shift = Getownproperty(jo,nname)))
                {
                    shift = Addproperty(jo,nname);
                }
                if(var && shift)
                {
                    Asgvalue(&shift->val,&var->val);
                }
            }
        }
    }

    if(!(length = Getownproperty(jo,"length")))
    {
        length = Addproperty(jo,"length");
    }
    if(length)
    {
        Asgnumber(&length->val,VNA_VALID, len);
    }
    Asgnumber(RETVAL(jc),VNA_VALID,len);
}

static void Arrayslice(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Jobject *array = NULL;
    struct Variable *length;
    struct Variable *elt,*newelt;

    LONG  ie,is;
    ULONG l=0;
    BOOL svalid,evalid;
    UBYTE nname[16];

    if((length = Getproperty(jo,"length")))
    {
        Tonumber(&length->val,jc);
        if(length->val.attr == VNA_VALID)
        {
            l = (ULONG)length->val.value.nvalue;
        }
    }

    if((array = Newarray(jc)))
    {
        is = (long)Numargument(jc,0,&svalid);
        ie = (long)Numargument(jc,1,&evalid);

        if(svalid)
        {

            if(!evalid)
            {
                ie = l;
            }
            if(is < 0) is += l;
            if(is < 0) is = 0;
            if(is > l) is = l;
            if(ie < 0) ie += l;
            if(ie < 0) ie = 0;
            if(ie > l) ie = l;

            while(is < ie)
            {
                ((struct Array*)array->internal)->length++;
                sprintf(nname,"%d",(int)is++);
                if(elt = Getproperty(jo,nname))
                {
                    sprintf(nname,"%d",(int)((struct Array*)array->internal)->length -1);
                    if((newelt = Addproperty(array,nname)))
                    {
                        Asgvalue(&newelt->val,&elt->val);
                    }
                }
            }
        }
        if((elt = Getownproperty(array,"length")))
        {
            Asgnumber(&elt->val,VNA_VALID,(double)((struct Array*)array->internal)->length);
        }
    }
    Asgobject(RETVAL(jc),array);
}

static void Arraysplice(struct Jcontext *jc)
{
    struct Jobject *jo = jc->jthis;
    struct Jobject *array = NULL;
    struct Jobject *args;
    struct Variable *var;
    struct Variable *elt;
    struct Variable *length;

    ULONG arglen = 0;
    ULONG l = 0;
    ULONG k;
    UBYTE nname[16];
    int is, dc;
    is = dc = 0;
    if((args = Findarguments(jc)))
    {
        var = Getproperty(args,"length");
        Tonumber(&var->val,jc);
        arglen = (ULONG)var->val.value.nvalue;
    }

    if((array = Newarray(jc)) && arglen >= 2)
    {
        var = Arrayelt(args,0);
        Tonumber(&var->val,jc);
        is = (int)var->val.value.nvalue;

        var = Arrayelt(args,1);
        Tonumber(&var->val,jc);
        dc =(int)var->val.value.nvalue;
        if (dc < 0) dc = 0;

        if((length = Getproperty(jo,"length")))
        {
            Tonumber(&length->val,jc);
            l = (ULONG)length->val.value.nvalue;
        }
        if(is < 0) is += l;
        if(is < 0) is = 0;

        if ((l - is) < dc) dc = l - is;
        for(k = is; k < is + dc; k++)
        {
            if((elt = Addarrayelt(jc,array)))
            {
                sprintf(nname,"%d",(int)k);
                if((var = Getproperty(jo,nname)))
                {
                    Asgvalue(&elt->val,&var->val);
                }
            }
        }
        arglen -= 2;
        if( arglen < dc)
        {
            /* we must close and clear the gap */
            for(k = is; k < l - dc + arglen; k++)
            {
                struct Variable *from,*to;
                sprintf(nname,"%d",(int)(k+dc));
                from = Getproperty(jo,nname);
                sprintf(nname,"%d",(int)(k+arglen));
                if(!(to = Getownproperty(jo,nname)))
                {
                    to = Addproperty(jo,nname);
                }
                if(from)
                {
                    if(to)
                    {
                        Asgvalue(&to->val,&from->val);
                    }
                }
                else
                if(to)
                {
                    Clearvalue(&to->val);
                }
            }
            for(k =l; k > l-dc + arglen;k--)
            {
                sprintf(nname,"%d",(int)k-1);
                Deleteownproperty(jo,nname);
            }

        }
        if(arglen > dc)
        {
            /* we must widen the gap */
            for( k = l - dc; k > is ; k--)
            {
                struct Variable *from, *to;
                sprintf(nname,"%d",(int) k + dc -1);
                from = Getproperty(jo,nname);
                sprintf(nname,"%d",(int) k + arglen -1);
                if(!(to = Getownproperty(jo,nname)))
                {
                    to = Addproperty(jo,nname);
                }
                if(from)
                {
                    if(to)
                    {
                        Asgvalue(&to->val,&from->val);
                    }
                }
                else
                {
                    if(to)
                    {
                        Clearvalue(&to->val);
                    }
                }

            }
        }
        /* now we should have the right size "gap" to copy the new items into */
        if(arglen)
        {
            for(k = 0; k < arglen; k++)
            {
                sprintf(nname,"%d",(int) k + is);
                if((elt = Arrayelt(args,k + 2)))
                {
                    if(!(var = Getownproperty(jo,nname)))
                    {
                        var = Addproperty(jo,nname);
                    }
                    if(var)
                    {
                        Asgvalue(&var->val,&elt->val);
                    }
                }
            }
        }
        if(!(length = Getownproperty(jo,"length")))
        {
            length = Addproperty(jo,"length");
        }
        if(length)
        {
            Asgnumber(&length->val,VNA_VALID,(double)(l - dc + arglen));
        }
        if(Isarray(jo))
        {
            ((struct Array *)jo->internal)->length = (l -dc + arglen);
        }
    }
    Asgobject(RETVAL(jc),array);
}

/* When "length" is set, set internal and truncate array aas necessary */
/* This only aplies to when length is set by javascript code, internally ie here */
/* we must take care to do all this manually */

static BOOL Arraylhook(struct Varhookdata *v)
{
    struct Jobject *jo = v->hookdata;
    struct Array *a = jo->internal;;

    switch(v->code)
    {
        case VHC_GET:
            Asgnumber(&v->value->val, VNA_VALID, (double)a->length);
            return TRUE;
            break;

        case VHC_SET:
            {
                ULONG oldlen = a->length;
                ULONG newlen;
                UBYTE nname[16];
                Tonumber(&v->value->val,v->jc);
                if(v->value->val.attr == VNA_VALID)
                {
                    newlen = (ULONG)v->value->val.value.nvalue;
                    if(((double)newlen) != v->value->val.value.nvalue)
                    {
                        Runtimeerror(v->jc,NTE_RANGE,v->jc->elt,"Array length must be positive integer");
                    }
                    if(newlen < oldlen)
                    {
                        int i;
                        struct Variable *elt;
                        for(i = newlen; i < oldlen; i++)
                        {
                            sprintf(nname,"%d",i);
                            Deleteownproperty(jo,nname);
                        }
                    }
                    a->length = newlen;
                    Asgnumber(&v->var->val,VNA_VALID,(double)newlen);
                }
                return TRUE;
            }
            break;
    }
    return FALSE;
}

/* Add a property to an array, increase .length if necessary */
static BOOL Arrayohook(struct Objhookdata *h)
{  BOOL result=FALSE;
   struct Variable *prop;
   struct Array *a=(h->jo)?h->jo->internal:NULL;
   long n;
   switch(h->code)
   {  case OHC_ADDPROPERTY:
         if(!(prop=Getownproperty(h->jo,h->name)))
         {  prop=Addproperty(h->jo,h->name);
         }
         if(prop && a)
         {  n=atoi(h->name);
            if(n>=a->length)
            {  a->length=n+1;
               if(prop=Getownproperty(h->jo,"length"))
               {  Asgnumber(&prop->val,VNA_VALID,(double)a->length);
               }
            }
         }
         result=TRUE;
         break;
   }
   return result;
}

/* Dispose an Array object */
static void Destructor(struct Array *a)
{
    if(a)
    {
        if(a->array)
        {
            int i;
            for(i=0;i<a->array_length;i++)
            {
                if(a->array[i])
                {
                    Disposevar(a->array[i]);
                    a->array[i] = NULL;
                }
            }
            FREE(a->array);
        }
        a->array = NULL;
        FREE(a);
    }
}

/* Make (jthis) a new Array object */
static void Constructor(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis,*args;
   struct Array *a = NULL;
   struct Variable *prop,*length,*arg,*narg;
   BOOL constructed=FALSE;
   long n;
   if(!(jc->flags & EXF_CONSTRUCT))
   {
        jo = Newarray(jc);
        a = (struct Array *)jo->internal;

   }
   else
   if(jo)
   {
      if(a=ALLOCSTRUCT(Array,1,0,jc->pool))
      {  jo->internal=a;
         jo->dispose=(Objdisposehookfunc *)Destructor;
         jo->hook=Arrayohook;
         jo->type=OBJT_ARRAY;
         a->length=0;
         a->array_length=0;
         a->array = NULL;
      }
   }
   if(a)
   {
      if(args=Findarguments(jc))
      {  if((length=Getproperty(args,"length")) && length->val.type==VTP_NUMBER)
         {  if((long)length->val.value.nvalue==1)
            {  /* Only 1 argument, see if it's numeric */
               if((arg=Arrayelt(args,0)) && arg->val.type==VTP_NUMBER)
               {

                  a->length=(long)arg->val.value.nvalue;
                  if(((double)a->length) != arg->val.value.nvalue)
                  {
                    Runtimeerror(jc,NTE_RANGE,jc->elt,"Array length must be a postive integer");
                  }
                  constructed=TRUE;
               }
            }
         }
         if(!constructed)
         {  /* Build new array from arguments array */
            for(n=0;arg=Arrayelt(args,n);n++)
            {  if(narg=Addarrayelt(jc,jo))
               {  Asgvalue(&narg->val,&arg->val);
               }
            }
         }
      }
      if(!(prop = Getownproperty(jo,"length")))
      {
          if(prop=Addproperty(jo,"length"))
          {
              prop->flags|=VARF_HIDDEN;
              prop->hook=Arraylhook;
              prop->hookdata = jo;
          }
      }
      if(prop)
      {
          Asgnumber(&prop->val,VNA_VALID,(double)a->length);
      }
   }

     if(!(jc->flags & EXF_CONSTRUCT))
     {
        Asgobject(RETVAL(jc),jo);
     }

}

/*-----------------------------------------------------------------------*/

void Initarray(struct Jcontext *jc, struct Jobject *jscope)
{  struct Jobject *jo,*f;
   struct Variable *prop;
   if(jo=Internalfunction(jc,"Array",(Internfunc *)Constructor,"arrayLength",NULL))
   {
      /* keep our object to avoid garbage collection triggered by any object creation below */
      Keepobject(jo,TRUE);

      Initconstruct(jc,jo,"Object",jc->object);
      Addprototype(jc,jo,Getprototype(jo->constructor));
      //Addglobalfunction(jc,jo);
      /* If invoke with NULL this must be added to the jcontext */
      /* This is to allow adding arrays to DOM etc before the frame scope */
      /* Has been created, there may be a better way to do this! */

      if(jscope == NULL)
      {
          jc->array=jo;
      }
      else
      if((prop = Addproperty(jscope,"Array")))
      {
          Asgobject(&prop->val,jo);
          prop->flags |= VARF_DONTDELETE;
          /* no longer need to keep as assigned to scope */
          Keepobject(jo,FALSE);

      }


      if(f=Internalfunction(jc,"toString",(Internfunc *)Arraytostring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toLocaleString",(Internfunc *)Arraytolocalestring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"join",(Internfunc *)Arrayjoin,"separator",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"reverse",(Internfunc *)Arrayreverse,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"sort",(Internfunc *)Arraysort,"compareFunction",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"concat",(Internfunc *)Arrayconcat,"items",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"pop",(Internfunc *)Arraypop,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"push",(Internfunc *)Arraypush,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"shift",(Internfunc *)Arrayshift,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"unshift",(Internfunc *)Arrayunshift,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"slice",(Internfunc *)Arrayslice,"start","end",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"splice",(Internfunc *)Arraysplice,"start","deleteCount",NULL))
      {  Addtoprototype(jc,jo,f);
      }

   }
}

struct Jobject *oldNewarray(struct Jcontext *jc)
{  struct Jobject *jo;
   struct Array *a;
   struct Variable *prop;
   if(jo=Newobject(jc))
   {  Initconstruct(jc,jo,NULL,jc->array);
      if(a=ALLOCSTRUCT(Array,1,0,jc->pool))
      {  jo->internal=a;
         jo->dispose=(Objdisposehookfunc *)Destructor;
         jo->hook=Arrayohook;
         jo->type=OBJT_ARRAY;

         a->length=0;
         a->array_length =0;
         a->array=NULL;

         if(prop=Addproperty(jo,"length"))
         {  prop->flags|=VARF_HIDDEN;
            Asgnumber(&prop->val,VNA_VALID,0.0);
            prop->hook=Arraylhook;
            prop->hookdata = jo;
         }
      }
   }
   return jo;
}

/* Create a new empty Array object. */
struct Jobject *Newarray(struct Jcontext *jc)
{  struct Jobject *jo;
   struct Array *a;
   struct Variable *prop;

   if(jo=Newobject(jc))
   {

      Initconstruct(jc,jo,"Array",NULL);
      if(a=ALLOCSTRUCT(Array,1,0,jc->pool))
      {  jo->internal=a;
         jo->dispose=(Objdisposehookfunc *)Destructor;
         jo->hook=Arrayohook;
         jo->type=OBJT_ARRAY;

         a->length=0;
         a->array_length=0;
         a->array=NULL;

         if(prop=Addproperty(jo,"length"))
         {  prop->flags|=VARF_HIDDEN;
            Asgnumber(&prop->val,VNA_VALID,0.0);
            prop->hook=Arraylhook;
            prop->hookdata = jo;
         }
      }
   }
   return jo;
}

/* Find the nth array element, or NULL */
struct Variable *Arrayelt(struct Jobject *jo,long n)
{  UBYTE nname[16];

   struct Array *a;
   struct Variable *prop=NULL;
   if(jo && jo->internal && jo->hook==Arrayohook)
   {  a=(struct Array *)jo->internal;
      if(n>=0 && n<a->length)
      {  sprintf(nname,"%d",(int)n);
         prop=Getownproperty(jo,nname);
      }
   }
   return prop;
}

/* Add an element to this array */
struct Variable *Addarrayelt(struct Jcontext *jc,struct Jobject *jo)
{  UBYTE nname[16];
   struct Array *a;
   struct Variable *prop=NULL,*length;
   if(jo && jo->internal && jo->hook==Arrayohook)
   {  a=(struct Array *)jo->internal;
      sprintf(nname,"%d",(int)a->length);
      if(prop=Addproperty(jo,nname))
      {  a->length++;
         if(length=Getownproperty(jo,"length"))
         {  Asgnumber(&length->val,VNA_VALID,(double)a->length);
         }
      }
   }
   return prop;
}

/* Tests if this object is an array */
BOOL Isarray(struct Jobject *jo)
{  return (BOOL)(jo && jo->internal && jo->hook==Arrayohook);
}

BOOL Touint32(STRPTR str, ULONG *num)
{
    BOOL success = FALSE;
    STRPTR p = str;
    STRPTR end;
    ULONG result = 0;
    ULONG oldresult = 0;
    size_t len = 0;
    if(str)
    {
        len = strlen(str);
    }
    if(len > 0)
    {
        end = str + len;
        for(p = str;p<end;p++)
        {
            if(isdigit((int)*p))
            {
                if(result > 429496729)
                {
                    /* over flow inevitable */
                    return FALSE;
                }
                result = 10 * result + (*p - '0');
            }
            else
            {
                return FALSE;
            }
        }
        *num = result;
        success = TRUE;
    }
    return success;
}

struct Variable *_Array_Addproperty(struct Jobject *jo, STRPTR name)
{
    ULONG index;
    struct Variable * var = NULL;
    if(Touint32(name,&index) && jo->internal)
    {
        struct Array *a = jo->internal;

        if(a->array_length > index)
        {
//            if(a->length <= index) a->length = index;
            if(a->array[index])
            {
                Disposevar(a->array[index]);
            }
            a->array[index] = Newvar(name,jo->jc);
            var = a->array[index];
        }
        else
        {
            /* storage is not long enough */
            /* allocate new copy date and replace */
            long new_length = ((index / CHUNKSIZE) + 1) *CHUNKSIZE;
            struct Variable **new_array = ALLOCTYPE(struct Variable *,new_length,MEMF_CLEAR,Getpool(jo));
            if(new_array)
            {
                if(a->array)
                {
                    /* copy into new_array and free */
                    memcpy(new_array,a->array,a->array_length * sizeof(struct Variable *));
                    FREE(a->array);
                }
                a->array = new_array;
                a->array_length = new_length;

          //  if(a->length <= index) a->length = index +1;
            if(a->array[index])
            {
                Disposevar(a->array[index]);
            }
                a->array[index] = Newvar(name,jo->jc);
                var = a->array[index];
            }
        }
    }
    else
    {
        var = _Generic_Addproperty(jo,name);
    }
    return var;
}

struct Variable *_Array_Getownproperty(struct Jobject *jo, STRPTR name)
{
    ULONG index;
    if(Touint32(name,&index) && jo->internal)
    {
        struct Array *a = jo->internal;
        if((index < a->length) && (index < a->array_length))
        {
            return a->array[index];
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return _Generic_Getownproperty(jo,name);
    }

}
BOOL _Array_Deleteownproperty(struct Jobject *jo, STRPTR name)
{
    ULONG index;
    if(Touint32(name,&index) && jo->internal)
    {
        struct Array *a = jo->internal;
        if((index < a->length) && (index < a->array_length))
        {
            if(a->array[index])
            {
                Disposevar(a->array[index]);
                a->array[index]=0;
                return TRUE;
            }
            else
            return FALSE;
        }
    }
    else
    {
        return _Generic_Deleteownproperty(jo,name);
    }

}
