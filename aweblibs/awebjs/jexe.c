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

/* jexe.c - AWeb js executor */

#include "awebjs.h"
#include "jprotos.h"
#include <exec/tasks.h>
#include <math.h>

/*-----------------------------------------------------------------------*/

static UBYTE *emsg_noproperty="No such property '%s'";
static UBYTE *emsg_nofunction="Not a function";
static UBYTE *emsg_notallowed="Access to '%s' not allowed";
static UBYTE *emsg_stackoverflow="Stack overflow";

/* Display the run time error message */
void Runtimeerror(struct Jcontext *jc, STRPTR type, struct Element *elt, UBYTE *msg, ...)
{
    struct Jbuffer *jb=NULL;
    char buf[256];

    BOOL debugger;

    VA_LIST args;

    if(jc->try)
    {
        struct Jobject *e;
        VA_START(args,msg);
        vsnprintf(buf,sizeof(buf),msg,args);
        VA_END(args);
        // adebug("Through Runtime Error of type %ld\n",type);
        if(type != NULL)
        {
            e = Newnativeerror(jc,type,buf);
        }
        else
        {
            e = Newerror(jc,buf);
        }
        // adebug("%08lx\n",e);
        Asgobject(jc->val,e);
        Asgvalue(&jc->functions.first->retval,jc->val);
        Asgvalue(jc->throwval,jc->val);
        jc->complete = ECO_THROW;
    }
    else
    if(!(jc->flags&(EXF_STOP|EXF_DONTSTOP)))
    {
        if(jc->flags&EXF_ERRORS)
        {
            jb = Jdecompile(jc, elt);

            VA_START(args, msg);
            debugger = Errorrequester(jc, elt->linenr, jb?jb->buffer:NULL, -1, msg, args);
            VA_END(args);
        }
        else
        {
            debugger=FALSE;
        }

        if(debugger)
        {
            Startdebugger(jc);
            Setdebugger(jc, elt);
        }
        else
        {
            jc->flags |= EXF_STOP;
        }

        if(jb) Freejbuffer(jb);
    }
}

/* Check for stack overflow; keep a margin for CA requester. Return TRUE if ok. */
static BOOL Stackcheck(struct Jcontext *jc,struct Element *elt)
{
   struct Task *task=FindTask(NULL);
   BOOL ok=TRUE;

#if defined(__MORPHOS__)

   LONG free;

   free = task->tc_ETask->PPCRegFrame - task->tc_ETask->PPCSPLower;
//   KPrintF(__FILE__ "/%s/%ld: free=%lu\n", __FUNCTION__, __LINE__, free);

   if (free < 8192)
#else

   /* Since (task) is a stack variable, (&task) is a good approximation of
    * the current stack pointer */
   if((ULONG)&task<(ULONG)task->tc_SPLower+8192)
#endif
   {
      Runtimeerror(jc,NTE_GENERAL,elt,emsg_stackoverflow);
      ok=FALSE;
      // adebug("stack panick!\n");
   }
   return ok;
}

/*-----------------------------------------------------------------------*/


/* Add a variable to this list */
static struct Variable *Addvar(struct Jcontext *jc,void *vlist,UBYTE *name)
{  struct Variable *var=Newvar(name,jc);
   if(var)
   {  AddTail((struct List *)vlist,(struct Node *)var);
   }
   return var;
}

/* Find a variable in the current scope. Create a global one if not found.
 * *(pthis) is set to object from with stack */

#if 0 /* original code */
struct Variable *Findvar(struct Jcontext *jc,STRPTR name,struct Jobject **pthis)
{  struct Variable *var;
   struct With *w;
   /* Search the with stack for this function */
   for(w=jc->functions.first->with.first;w->next;w=w->next)
   {  if(var=Getproperty(w->jo,name))
      {  if(pthis) *pthis=w->jo;
         return var;
      }
   }
   if(jc->functions.first->next->next)
   {  /* Try local variables for function */
      for(var=jc->functions.first->local.first;var->next;var=var->next)
      {  if(var->name && STREQUAL(var->name,name)) return var;
      }
   }
   /* Try global data scope for this function. */
   if(var=Getproperty(jc->functions.first->fscope,name))
   {  return var;
   }
   /* Try properties of this */
   if(var=Getproperty(jc->jthis,name))
   {  return var;
   }
   /* Try top level global data scopes. */
   for(w=jc->functions.last->with.first;w->next;w=w->next)
   {  if((w->flags&WITHF_GLOBAL) && (var=Getproperty(w->jo,name)))
      {  if(pthis) *pthis=w->jo;
         return var;
      }
   }
   /* Try JS global variables */
   for(var=jc->functions.last->local.first;var->next;var=var->next)
   {  if(var->name && STREQUAL(var->name,name)) return var;
   }
   /* Not found, add variable to the global scope. */
   var=Addproperty(jc->functions.last->fscope,name);
   return var;
}

#endif
struct Variable *Findvar(struct Jcontext *jc,STRPTR name,struct Jobject **pthis)
{  struct Variable *var;
   struct With *w;
   struct Function *f;
   //adebug("search for VAR %s\n",name);
   /* Search the with stack for this function */
   for(w=jc->functions.first->with.first;w->next;w=w->next)
   {  if(var=Getproperty(w->jo,name))
      {  if(pthis) *pthis=w->jo;
         return var;
      }
   }
   if(jc->functions.first->next->next)
   {  /* Try local variables for function */
      for(var=jc->functions.first->local.first;var->next;var=var->next)
      {  if(var->name && STREQUAL(var->name,name)) return var;
      }
   }
   /* Try global data scope for this function. */
   if(var=Getproperty(jc->functions.first->fscope,name))
   {  return var;
   }
   /* Try properties of this */
   if(var=Getproperty(jc->jthis,name))
   {  return var;
   }
   /* Try top level global data scopes. */

   for(f = jc->functions.first->next;f && f->next;f=f->next)
   {

       for(w=f->with.first;w->next;w=w->next)
       {  if((w->flags&WITHF_GLOBAL) && (var=Getproperty(w->jo,name)))
          {  if(pthis) *pthis=w->jo;
             return var;
          }
       }
       /* Try JS global variables */
       for(var=f->local.first;var->next;var=var->next)
       {  if(var->name && STREQUAL(var->name,name)) return var;
       }
   }
   /* Not found, add variable to the global scope. */
   var=Addproperty(jc->functions.last->fscope,name);
   return var;
}



/* Find a local variable. Create a new one if not found.
 * Falls back to global variables if not within function scope. */
static struct Variable *Findlocalvar(struct Jcontext *jc,UBYTE *name)
{  struct Variable *var;
   // adebug("search for LOCAL VAR %s\n",name);

   if(jc->functions.first->next->next)
   {  /* Within function scope */
      for(var=jc->functions.first->local.first;var->next;var=var->next)
      {  if(var->name && STREQUAL(var->name,name)) return var;
      }
      /* Not found, add variable. */
      return Addvar(jc,&jc->functions.first->local,name);
   }
   else
   {  /* Use global variables */
      if(var=Getproperty(jc->functions.last->fscope,name)) return var;
      return Addproperty(jc->functions.last->fscope,name);
   }
}

/*-----------------------------------------------------------------------*/

/* Add a with level */
static struct With *Addwith(struct Jcontext *jc,struct Jobject *jo)
{  struct With *w;
   if(w=ALLOCSTRUCT(With,1,0,jc->pool))
   {  w->jo=jo;
      AddHead((struct List *)&jc->functions.first->with,(struct Node *)w);
   }
   return w;
}

/* Dispose a with level */
static void Disposewith(struct With *w)
{  if(w)
   {  FREE(w);
   }
}

/*-----------------------------------------------------------------------*/

/* Create a new function. Add to head of function stack yourself! */
struct Function *Newfunction(struct Jcontext *jc,struct Elementfunc *func)
{  struct Function *f=ALLOCSTRUCT(Function,1,0,jc->pool);
   if(f)
   {  NewList((struct List *)&f->local);
      NewList((struct List *)&f->with);
      f->arguments=oldNewarray(jc);
      if(f->arguments) Keepobject(f->arguments,TRUE);
      if(func)
      {  f->fscope=func->fscope;
      }
   }
   return f;
}

/* Dispose a function */
void Disposefunction(struct Function *f)
{  struct Variable *var;
   struct With *w;
   if(f)
   {  while(var=(struct Variable *)RemHead((struct List *)&f->local)) Disposevar(var);
      while(w=(struct With *)RemHead((struct List *)&f->with)) Disposewith(w);
      if(f->arguments) Keepobject(f->arguments,FALSE);
      Clearvalue(&f->retval);
      FREE(f);
   }

}

/*-----------------------------------------------------------------------*/

/* Evaluate first argument (but only if a string) */
static void Eval(struct Jcontext *jc)
{  struct Variable *var;
   struct Function *f;
   var=jc->functions.first->local.first;
   if(var->next)
   {
       f=(struct Function *)RemHead((struct List *)&jc->functions);
       if(var->val.type == VTP_STRING)
       {
           Jeval(jc,var->val.value.svalue);
           Asgvalue(&f->retval,jc->val);
       }
       else
       {
           Asgvalue(&f->retval,&var->val);
       }
       AddHead((struct List *)&jc->functions,(struct Node *)f);
   }
   /* Compile and execute the thing. Execution must take place in the
    * caller's context, not the eval() context. So pop the top function
    * and put it back later */
}

/* Parse string to integer */
static void Parseint(struct Jcontext *jc)
{  struct Variable *var;
   UBYTE *s="";
   long radix=0;
   long n=0,d;
   short sign=1;
   UBYTE attr=VNA_NAN;
   var=jc->functions.first->local.first;
   if(var->next)
   {  Tostring(&var->val,jc);
      s=var->val.value.svalue;
      var=var->next;
      if(var->next)
      {  Tonumber(&var->val,jc);
         if(var->val.attr==VNA_VALID)
         {  radix=(long)var->val.value.nvalue;
         }
      }
   }
   if(s)
   {
       if(*s=='-')
       {  sign=-1;
          s++;
       }
       if(radix==0)
       {  if(s[0]=='0')
          {  if(toupper(s[1])=='X')
             {  radix=16;
                s+=2;
             }
             else
             {  radix=8;
                s++;
                attr=VNA_VALID;
             }
          }
          else radix=10;
       }
       if(radix>=2 && radix<=36)
       {  for(;;)
          {  if(isdigit(*s)) d=*s-'0';
             else
             {  d=toupper(*s)-'A'+10;
                if(d<10) d=radix;
             }
             if(d>=radix) break;
             n=radix*n+d;
             attr=VNA_VALID;
             s++;
          }
          n*=sign;
       }
   }
   Asgnumber(RETVAL(jc),attr,(double)n);
}

/* Parse string to float */
static void Parsefloat(struct Jcontext *jc)
{  struct Variable *var;
   UBYTE *s="";
   long exp=0;
   double ffrac,f;
   short sign=1,exps=1;
   UBYTE attr=VNA_NAN;
   var=jc->functions.first->local.first;
   if(var->next)
   {  Tostring(&var->val,jc);
      s=var->val.value.svalue;
   }
   f=0.0;
   if(*s=='-')
   {  sign=-1;
      s++;
   }
   while(isdigit(*s))
   {  f=f*10.0+(*s-'0');
      attr=VNA_VALID;
      s++;
   }
   if(*s=='.')
   {  s++;
      ffrac=1.0;
      while(isdigit(*s))
      {  ffrac/=10.0;
         f+=ffrac*(*s-'0');
         s++;
         attr=VNA_VALID;
      }
   }
   if(toupper(*s)=='E')
   {  s++;
      switch(*s)
      {  case '-':   exps=-1;s++;break;
         case '+':   s++;break;
      }
      while(isdigit(*s))
      {  exp=10*exp+(*s-'0');
         s++;
      }
   }
   f*=pow(10.0,(double)(exps*exp))*sign;
   Asgnumber(RETVAL(jc),attr,f);
}

/* Escape string */
static void Escape(struct Jcontext *jc)
{  struct Variable *var;
   UBYTE *s="";
   struct Jbuffer *jb;
   UBYTE buf[6];
   var=jc->functions.first->local.first;
   if(var->next)
   {  Tostring(&var->val,jc);
      s=var->val.value.svalue;
   }
   if(jb=Newjbuffer(jc->pool))
   {  for(;*s;s++)
      {  if(*s==' ')
         {  Addtojbuffer(jb,"+",1);
         }
         else if(isalnum(*s))
         {  Addtojbuffer(jb,s,1);
         }
         else
         {  sprintf(buf,"%%%02X",*s);
            Addtojbuffer(jb,buf,-1);
         }
      }
      Addtojbuffer(jb,"",1);
      Asgstring(RETVAL(jc),jb->buffer,jc->pool);
      Freejbuffer(jb);
   }
}

/* Unescape string */
static void Unescape(struct Jcontext *jc)
{  struct Variable *var;
   UBYTE *s="",c;
   struct Jbuffer *jb;
   var=jc->functions.first->local.first;
   if(var->next)
   {  Tostring(&var->val,jc);
      s=var->val.value.svalue;
   }
   if(jb=Newjbuffer(jc->pool))
   {  for(;*s;s++)
      {  if(*s=='%')
         {  c=0;
            if(isxdigit(s[1]))
            {  s++;
               c=16*c+(isdigit(*s)?(*s-'0'):(toupper(*s)-'A'+10));
               if(isxdigit(s[1]))
               {  s++;
                  c=16*c+(isdigit(*s)?(*s-'0'):(toupper(*s)-'A'+10));
               }
            }
            Addtojbuffer(jb,&c,1);
         }
         else if(*s=='+')
         {  Addtojbuffer(jb," ",1);
         }
         else
         {  Addtojbuffer(jb,s,1);
         }
      }
      Addtojbuffer(jb,"",1);
      Asgstring(RETVAL(jc),jb->buffer,jc->pool);
      Freejbuffer(jb);
   }
}

/* Check if value is NaN */
static void Isnan(struct Jcontext *jc)
{  struct Variable *var;
   BOOL nan=FALSE;
   var=jc->functions.first->local.first;
   if(var->next && var->val.type==VTP_NUMBER && var->val.attr==VNA_NAN)
   {  nan=TRUE;
   }
   Asgboolean(RETVAL(jc),nan);
}

static BOOL Member(struct Jcontext *jc,struct Element *elt,struct Jobject *jo,
   UBYTE *mbrname,BOOL asgonly)
{  struct Variable *mbr;
   BOOL ok=FALSE;
   //adebug("search member %s in %08lx\n",mbrname,jo);
   if(!asgonly)
   {
       mbr=Getproperty(jo,mbrname);
   }
   else
   {
       mbr=Getownproperty(jo,mbrname);
   }
   if(!mbr)
   {  if(Callohook(jo,jc,OHC_ADDPROPERTY,mbrname))
      {  mbr=Getownproperty(jo,mbrname);
      }
      else
      {  mbr=Addproperty(jo,mbrname);
      }
   }
   if(mbr)
   {  /* Resolve synonym reference */
      while((mbr->flags&VARF_SYNONYM) && mbr->hookdata) mbr=mbr->hookdata;

      /* Only access member if protection allows it. But keep the reference
       * even without protection to allow assignments (e.g. location.href) */
      if(!jc->protkey || !mbr->protkey || jc->protkey==mbr->protkey || asgonly)
      {  if(mbr->val.type==VTP_OBJECT && mbr->val.value.obj.ovalue && mbr->val.value.obj.ovalue->function)
         {  Asgfunction(jc->val,mbr->val.value.obj.ovalue,jo);
         }
         else
         {  if(!Callvhook(mbr,jc,VHC_GET,jc->val))
            {  Asgvalue(jc->val,&mbr->val);
            }
         }
      }
      else
      {  Runtimeerror(jc,NTE_GENERAL,elt,emsg_notallowed,mbrname);
         Clearvalue(jc->val);
      }
      jc->varref=mbr;
      jc->flags|=EXF_KEEPREF;
      ok=TRUE;
   }
   else Runtimeerror(jc,NTE_REFERENCE,elt,emsg_noproperty,mbrname);
   return ok;
}

/*-----------------------------------------------------------------------*/

inline extern void Executeelem(struct Jcontext *jc,struct Element *elt);

static void Exeprogram(struct Jcontext *jc,struct Elementlist *elist)
{  struct Elementnode *enode,*enext;
   struct Element *elt;
   long generation=jc->generation;
   for(enode=elist->subs.first;enode->next;enode=enext)
   {  enext=enode->next;
      elt=enode->sub;
      if(elt && elt->generation==generation)
      {  Executeelem(jc,enode->sub);
         /* When used in eval() (not top level call), break out after return */
         if(jc->functions.first->next->next && jc->complete) break;
      }
   }
   /* Finally, dispose all statements for this generation */
   for(enode=elist->subs.first;enode->next;enode=enext)
   {  enext=enode->next;
      elt=enode->sub;
      if(elt && elt->generation==generation)
      {  Remove((struct Node *)enode);
         FREE(enode);
         Jdispose(elt);
      }
   }
}

/* Call this function without parameters with this object as "this" */
void Callfunctionbody(struct Jcontext *jc,struct Elementfunc *func,struct Jobject *jthis)
{  struct Function *f;
   struct Jobject *oldthis;
   struct This *tnode;
   UWORD oldflags;
   if(f=Newfunction(jc,func))
   {  AddHead((struct List *)&jc->functions,(struct Node *)f);
      oldthis=jc->jthis;
      jc->jthis=jthis;
      oldflags=jc->flags;
      jc->flags&=~EXF_CONSTRUCT;
      if((tnode = ALLOCSTRUCT(This,1,MEMF_CLEAR,jc->pool)))
      {
         tnode->this = oldthis;
         AddHead((struct List *)&jc->thislist,(struct Node *)tnode);
      }
      Executeelem(jc,(struct Element *)func);
      jc->jthis=oldthis;
      if(tnode)
      {
         Remove((struct Node *)tnode);
         FREE(tnode);
      }

      jc->flags=oldflags /*&~EXF_ERRORS*/ ;  /* Why did I put that here????? */
      Remove(f);
      Disposefunction(f);
   }
}

/* Call this function with supplied arguments (must be struct Value *, NULL terminated) */
void Callfunctionargs
(
    struct Jcontext *jc, struct Elementfunc *func, struct Jobject *jthis, ...
)
{
    struct Function *f;
    struct Jobject  *oldthis;
    struct Value    *val;
    struct Variable *var;
    UWORD oldflags;

    VA_LIST args;

    if ((f = Newfunction(jc, func)))
    {
        VA_START(args, jthis);

        while ((val = VA_ARG(args, struct Value *)))
        {
            if((var = Newvar(NULL, jc)))
            {
                AddTail((struct List *)&f->local,(struct Node *)var);
                Asgvalue(&var->val, val);
            }
        }

        VA_END(args);

        VA_START(args, jthis);

        while ((val = VA_ARG(args, struct Value *)))
        {
            if (var = Addarrayelt(jc, f->arguments))
                Asgvalue(&var->val, val);
        }

        VA_END(args);

        AddHead((struct List *)&jc->functions,(struct Node *)f);

        oldthis    =  jc->jthis;
        jc->jthis  =  jthis;
        oldflags   =  jc->flags;
        jc->flags &= ~EXF_CONSTRUCT;

        Executeelem(jc, (struct Element *)func);

        jc->jthis = oldthis;
        jc->flags = oldflags /*&~EXF_ERRORS*/ ;

        Remove((struct Node *)f);
        Disposefunction(f);
    }
}

/* Call this function with this object as "this" */
static void Callfunction(struct Jcontext *jc,struct Elementlist *elist,
   struct Jobject *jthis,BOOL construct)
{  struct Elementnode *enode;
   struct Elementfunc *func=NULL;
   struct Function *f;
   struct Variable *arg,*argv;
   struct Jobject *oldthis,*fdef;
   UWORD oldflags;

   struct This *tnode = NULL;;

   if(!Stackcheck(jc,(struct Element *)elist)) return;
   Executeelem(jc,(struct Element *)elist->subs.first->sub);
   Tofunction(jc->val,jc);

 //  Addobjectlist(jc);                 // install temp list
                                      // all new objects now added to this list and if
                                      // garbage colection is triggered it will scan
                                      // against this list, protecting any "floating" objects
                                      // created by syntax like (new Object()).method()

   if(jc->val->value.obj.ovalue)
   {  func=jc->val->value.obj.ovalue->function;
      fdef=jc->val->value.obj.ovalue;
   }
   if(jc->val->value.obj.fthis && !construct)
   {  jthis=jc->val->value.obj.fthis;
   }
   if(construct && jthis && jc->val->value.obj.ovalue)
   {  Initconstruct(jc,jthis,NULL,jc->val->value.obj.ovalue);
   }
   /* Call constructor */
   if(func)
   {  if(f=Newfunction(jc,func))
      {  f->def=fdef;
         oldflags=jc->flags;
         jc->flags&=~EXF_CONSTRUCT;

         /* Create local variables for all actual parameters */
         for(enode=elist->subs.first->next;enode && enode->next;enode=enode->next)
         {
            Executeelem(jc,enode->sub);
            if(arg=Newvar(NULL,jc))
            {  AddTail((struct List *)&f->local,(struct Node *)arg);
               Asgvalue(&arg->val,jc->val);
            }
         }
         /* Create the arguments array n*/
         for(argv=f->local.first;argv->next;argv=argv->next)
         {  if(arg=Addarrayelt(jc,f->arguments))
            {  Asgvalue(&arg->val,&argv->val);
            }
         }
         AddHead((struct List *)&jc->functions,(struct Node *)f);
         oldthis=jc->jthis;
         jc->jthis=jthis;
         if((tnode = ALLOCSTRUCT(This,1,MEMF_CLEAR,jc->pool)))
         {
            tnode->this = oldthis;
            AddHead((struct List *)&jc->thislist,(struct Node *)tnode);
         }
         if(construct) jc->flags|=EXF_CONSTRUCT;
         Executeelem(jc,func);
         jc->jthis=oldthis;
         if(tnode)
         {
            Remove((struct Node *)tnode);
            FREE(tnode);
         }
         jc->flags=oldflags;
         Remove((struct Node *)f);
         Disposefunction(f);
      }
   }
   else if(jc->val->value.obj.ovalue && (jc->val->value.obj.ovalue->flags&OBJF_ASFUNCTION))
   {  /* Copy of Exeindex() here: */
      enode=elist->subs.first->next;
      if(enode && enode->next)
      {
         struct Value val,sval;
         BOOL ok=FALSE;
         val.type=sval.type=0;

         Asgvalue(&val,jc->val);

         Executeelem(jc,enode->sub);
         Asgvalue(&sval,jc->val);
         Tostring(&sval,jc);
         ok=Member(jc,(struct Element *)elist,val.value.obj.ovalue,sval.value.svalue,FALSE);
         Clearvalue(&val);
         if(!ok) Clearvalue(jc->val);
         Clearvalue(&sval);
      }
      else
      {  Runtimeerror(jc,NTE_TYPE,(struct Element *)elist,emsg_nofunction);
      }
   }
   else
   {  Runtimeerror(jc,NTE_TYPE,(struct Element *)elist,emsg_nofunction);
   }

   // Removeobjectlist(jc);

   //Garbage collect against old list.
 //  if((jc->nogc<=0))
 //  {
 //      jc->gc = GC_THRESHOLD;
 //      Garbagecollect(jc);
 //
 //  }
}

static void Execall(struct Jcontext *jc,struct Elementlist *elist)
{  Callfunction(jc,elist,jc->jthis,FALSE);
}

static void Execompound(struct Jcontext *jc,struct Elementlist *elist)
{  struct Elementnode *enode;
   for(enode=elist->subs.first;enode->next;enode=enode->next)
   {  if(enode->sub)
      {  Executeelem(jc,enode->sub);
         if(jc->complete) break;
      }
   }
}

static void Exevarlist(struct Jcontext *jc,struct Elementlist *elist)
{  struct Elementnode *enode;
   for(enode=elist->subs.first;enode->next;enode=enode->next)
   {  if(enode->sub)
      {  Executeelem(jc,enode->sub);
         /* Keep reference to last variable */
         jc->flags|=EXF_KEEPREF;
      }
   }
}

static void Exefunction(struct Jcontext *jc,struct Elementfunc *func)
{  struct Elementnode *enode;
   struct Elementstring *arg;
   struct Function *f;
   struct Variable *var,*avar;
   f=jc->functions.first;
   /* Map formal parameter names to pre-allocated local variables */
   for(enode=func->subs.first,var=f->local.first;
      enode->next && var->next;
      enode=enode->next,var=var->next)
   {  arg=enode->sub;
      if(arg && arg->type==ET_IDENTIFIER)
      {  if(var->name) FREE(var->name);
         var->name=Jdupstr(arg->svalue,-1,jc->pool);
      }
   }
   /* Add empty local variables for unassigned formal parameters */
   for(;enode && enode->next;enode=enode->next)
   {  arg=enode->sub;
      if(arg && arg->type==ET_IDENTIFIER)
      {  if(var=Newvar(arg->svalue,jc))
         {  AddTail((struct List *)&f->local,(struct Node *)var);
         }
      }
   }
   /* Link the arguments array to a local variable
    * and to the function object .arguments property */
   if(var=Newvar("arguments",jc))
   {  AddTail((struct List *)&f->local,(struct Node *)var);
      Asgobject(&var->val,f->arguments);
      var->flags |= VARF_DONTDELETE;
   }
   if((avar=Getownproperty(f->def,"arguments"))
   || (avar=Addproperty(f->def,"arguments")))
   {  Asgobject(&avar->val,f->arguments);
      avar->flags |= VARF_DONTDELETE;
   }
   /* Link the caller to a variable */
   if(var=Newvar("caller",jc))
   {  AddTail((struct List *)&f->local,(struct Node *)var);
      Asgobject(&var->val,f->next->def);
      var->flags |= VARF_DONTDELETE;
   }
   /* Create function call object */
   if(var=Newvar(func->name,jc))
   {  AddTail((struct List *)&f->local,(struct Node *)var);
      Asgfunction(&var->val,f->def,NULL);
      var->flags |= VARF_DONTDELETE;
   }
   /* Execute the function body */
   Executeelem(jc,func->body);
   if(jc->complete<=ECO_RETURN)
   {  jc->complete=ECO_NORMAL;
   }
   /* Clear the .arguments property */
   if(avar)
   {  Clearvalue(&avar->val);
   }
   /* Set the function result. */
   Asgvalue(jc->val,&f->retval);
}

static void Exefuncref(struct Jcontext *jc, struct Elementfuncref *funcref)
{
    Asgobject(jc->val, funcref->func);
}

static struct Label * Findlabel(struct Jcontext * jc, UBYTE *name)
{
    struct Label *label;
    for(label = jc->labels.first;label->next;label=label->next)
    {
        if(STREQUAL(name,label->name))
        {
            return label;
        }
    }
    return NULL;
}

static void Exebreak(struct Jcontext *jc,struct Element *elt)
{  jc->complete=ECO_BREAK;
   if(elt->sub1)
   {
        /* Can only be ET_INDENTIFIER or null so no need to check */

        jc->curlabel = Findlabel(jc,((struct Elementstring *)elt->sub1)->svalue);
   }
   else
   {
       jc->curlabel=NULL;
   }
}

static void Execontinue(struct Jcontext *jc,struct Element *elt)
{  jc->complete=ECO_CONTINUE;
   if(elt->sub1)
   {
        /* Can only be ET_INDENTIFIER or null so no need to check */

        jc->curlabel = Findlabel(jc,((struct Elementstring *)elt->sub1)->svalue);
   }
   else
   {
       jc->curlabel=NULL;
   }

}

static void Exethis(struct Jcontext *jc,struct Element *elt)
{
    struct Variable *var;
    Asgobject(jc->val,jc->jthis);
    if((var = Findlocalvar(jc,"this")))
    {
        Asgobject(&var->val,jc->jthis);
    }

}

static void Exenull(struct Jcontext *jc,struct Element *elt)
{  Asgobject(jc->val,NULL);
}

static void Exenegative(struct Jcontext *jc,struct Element *elt)
{  double n;
   UBYTE v;
   Executeelem(jc,elt->sub1);
   Tonumber(jc->val,jc);
   switch(jc->val->attr)
   {  case VNA_VALID:
         n=-jc->val->value.nvalue;
         v=VNA_VALID;
         break;
      case VNA_INFINITY:
         v=VNA_NEGINFINITY;
         break;
      case VNA_NEGINFINITY:
         v=VNA_INFINITY;
         break;
      default:
         v=VNA_NAN;
         break;
   }
   Asgnumber(jc->val,v,n);
}

static void Exepositive(struct Jcontext *jc,struct Element *elt)
{
   Executeelem(jc,elt->sub1);
   Tonumber(jc->val,jc);
}

static void Exenot(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   Toboolean(jc->val,jc);
   Asgboolean(jc->val,!jc->val->value.bvalue);
}

static void Exebitneg(struct Jcontext *jc,struct Element *elt)
{
   LONG n=0;
   Executeelem(jc,elt->sub1);
   Tonumber(jc->val,jc);
   if(jc->val->attr==VNA_VALID)
   {  n=~((long)jc->val->value.nvalue);
   }
   Asgnumber(jc->val,VNA_VALID,(double)n);
}

static void Exepreinc(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   if(jc->varref)
   {  Tonumber(&jc->varref->val,jc);
      if(jc->varref->val.attr==VNA_VALID)
      {  jc->varref->val.value.nvalue+=1.0;
      }
      Asgvalue(jc->val,&jc->varref->val);
   }
}

static void Exepredec(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   if(jc->varref)
   {  Tonumber(&jc->varref->val,jc);
      if(jc->varref->val.attr==VNA_VALID)
      {  jc->varref->val.value.nvalue-=1.0;
      }
      Asgvalue(jc->val,&jc->varref->val);
   }
}

static void Exepostinc(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   if(jc->varref)
   {  Tonumber(&jc->varref->val,jc);
      Asgvalue(jc->val,&jc->varref->val);
      if(jc->varref->val.attr==VNA_VALID)
      {  jc->varref->val.value.nvalue+=1.0;
      }
   }
}

static void Exepostdec(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   if(jc->varref)
   {  Tonumber(&jc->varref->val,jc);
      Asgvalue(jc->val,&jc->varref->val);
      if(jc->varref->val.attr==VNA_VALID)
      {  jc->varref->val.value.nvalue-=1.0;
      }
   }
}

static long objnum = 0;

static void Exenew(struct Jcontext *jc,struct Element *elt)
{  struct Jobject *jo;
   struct Element *ctr=elt->sub1;
   struct Elementstring *ident = NULL;
   BOOL kept;;
   if(jo=Newobject(jc))
   {
      Keepobject(jo,TRUE);
      kept = TRUE;
      if(ctr)
      {  if(ctr->type==ET_CALL)
         {
            ident = (struct Elementstring *)((struct Elementnode *)((struct Elementlist *)ctr)->subs.first)->sub;
            Callfunction(jc,elt->sub1,jo,TRUE);
         }
         else if(ctr->type==ET_IDENTIFIER)
         {  /* "new Constructor" called, without ().
             * Build CALL type element on the fly and execute it */
            struct Elementlist call={0};
            struct Elementnode node={0};
            call.type=ET_CALL;
            call.generation=elt->generation;
            call.linenr=elt->linenr;
            NewList((struct List *)&call.subs);
            node.sub=ctr;
            AddTail((struct List *)&call.subs,(struct Node *)&node);
            Callfunction(jc,&call,jo,TRUE);
            ident = (struct Elementstring *)ctr;
         }
         if(ident && ident->type == ET_IDENTIFIER)
         {
            if(ident->svalue && STREQUAL(ident->svalue,"Object"))
            {
                /* Object Constructor is special */
                if(jc->o)
                {
                    if(jc->o != jo)
                    {
                        Remove((struct Node *)jo);
                        Keepobject(jo,FALSE);
                        Disposeobject(jo);
                        jo = jc->o;
                        jc->o = NULL;
                        kept = FALSE;
                    }

                }
            }
         }

      }
      if(kept)Keepobject(jo,FALSE);
      /* experimental code */
      /* assign to a hidden var in local scope to prevent disposal before scope is exited */
      {
          char b[11];
          struct Variable *var;
          snprintf(b,10,"\t%08x",objnum++);
          if((var = Findlocalvar(jc,b)))
          {
             Asgobject(&var->val,jo);
          }
      }
      Asgobject(jc->val,jo);
      //adebug("new object %08lx\n",jo);
   }
}

static void Exedelete(struct Jcontext *jc,struct Element *elt)
{
   BOOL result = FALSE;
   struct Variable *rhs;
   if( ((struct Element *)elt->sub1)->type == ET_IDENTIFIER ||
       ((struct Element *)elt->sub1)->type == ET_DOT  ||
       ((struct Element *)elt->sub1)->type == ET_INDEX
     )
   {
       jc->flags|=EXF_ASGONLY;
       Executeelem(jc,elt->sub1);
       jc->flags&=~EXF_ASGONLY;
       rhs=jc->varref;
   }
   else
   {
       rhs = NULL;
       result = TRUE;
   }
   if(rhs)
   {
       if(!(rhs->flags & VARF_DONTDELETE))
       {
           // adebug("*** deleteing variable %s\n",rhs->name);
           Remove((struct Node *)rhs);
           Disposevar(rhs);
           result = TRUE;
       }
       else
       {
           result = FALSE;
       }
   }
   Asgboolean(jc->val,result);
}

static void Exetypeof(struct Jcontext *jc,struct Element *elt)
{  UBYTE *tp;
   Executeelem(jc,elt->sub1);
   switch(jc->val->type)
   {  case VTP_NUMBER:  tp="number";break;
      case VTP_BOOLEAN: tp="boolean";break;
      case VTP_STRING:  tp="string";break;
      case VTP_OBJECT:
         if(jc->val->value.obj.ovalue && jc->val->value.obj.ovalue->function)
         {  tp="function";
         }
         else
         {  tp="object";
         }
         break;
      default:          tp="undefined";break;
   }
   Asgstring(jc->val,tp,jc->pool);
}

static void Exevoid(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   Clearvalue(jc->val);
}

static void Exereturn(struct Jcontext *jc,struct Element *elt)
{  if(elt->sub1)
   {  Executeelem(jc,elt->sub1);
      Asgvalue(&jc->functions.first->retval,jc->val);
   }
   else
   {  Clearvalue(&jc->functions.first->retval);
   }
   jc->complete=ECO_RETURN;
}

static void Exethrow(struct Jcontext *jc,struct Element *elt)
{
   /* NB must adapt this to check for try catch else error */

   if(elt->sub1)
   {  Executeelem(jc,elt->sub1);
      Asgvalue(&jc->functions.first->retval,jc->val);
      Asgvalue(jc->throwval,jc->val);
   }
   else
   {  Clearvalue(&jc->functions.first->retval);
   }
   jc->complete=ECO_THROW;
}


static void Exeinternal(struct Jcontext *jc,struct Element *elt)
{
   if(elt->sub1)
   {
                        void (*f)(void *);
                        f=(void (*)(void *))elt->sub1;
                f(jc);
   }
}

static void Exefunceval(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   Tostring(jc->val,jc);
   Jeval(jc,jc->val->value.svalue);
}

static void Execase(struct Jcontext *jc,struct Elementcase *elt)
{
    if(elt->expr)
    {
        Executeelem(jc,elt->expr);
    }
}

static void Exeplus(struct Jcontext *jc,struct Element *elt)
{  static UBYTE attrtab[4][4]=
   {/* a+b   valid            nan      +inf              -inf */
   /*valid*/ {VNA_VALID,       VNA_NAN, VNA_INFINITY,     VNA_NEGINFINITY},
   /* nan */ {VNA_NAN,         VNA_NAN, VNA_NAN,          VNA_NAN},
   /* +inf*/ {VNA_INFINITY,    VNA_NAN, VNA_INFINITY,     VNA_NAN},
   /* -inf*/ {VNA_NEGINFINITY, VNA_NAN, VNA_NAN,          VNA_NEGINFINITY}
   };
   struct Value val1,val2;
   struct Variable *lhs;
   BOOL concat=FALSE;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Asgvalue(&val2,jc->val);
   /* If object, see if it string-convertible */
   if(val1.type==VTP_OBJECT && val1.value.obj.ovalue && !val1.value.obj.ovalue->function)
   {  if(!Callproperty(jc,val1.value.obj.ovalue,"valueOf") || jc->val->type>=VTP_STRING)
      {  /* Not number-convertible */
         concat=TRUE;
      }
   }
   else if(val1.type>=VTP_STRING)
   {  concat=TRUE;
   }
   if(!concat && val2.type==VTP_OBJECT && val2.value.obj.ovalue && !val2.value.obj.ovalue->function)
   {  if(!Callproperty(jc,val2.value.obj.ovalue,"valueOf") || jc->val->type>=VTP_STRING)
      {  /* Not number-convertible */
         concat=TRUE;
      }
   }
   else if(val2.type>=VTP_STRING)
   {  concat=TRUE;
   }
   if(concat)
   {  /* Do string concatenation */
      long l;
      UBYTE *s;
      Tostring(&val1,jc);
      Tostring(&val2,jc);
      l=strlen(val1.value.svalue)+strlen(val2.value.svalue);
      if(s=ALLOCTYPE(UBYTE,l+1,0,jc->pool))
      {  strcpy(s,val1.value.svalue);
         strcat(s,val2.value.svalue);
         Asgstring(jc->val,s,jc->pool);
         FREE(s);
      }
   }
   else
   {  /* Do numeric addition */
      double n=0.0;
      UBYTE v;
      Tonumber(&val1,jc);
      Tonumber(&val2,jc);
      if((v=attrtab[val1.attr][val2.attr])==VNA_VALID)
      {
         n = val1.value.nvalue +val2.value.nvalue;
         if (isinf(n))
         {
            v = val1.value.nvalue>0 ? VNA_INFINITY : VNA_NEGINFINITY;
         }
      }
      Asgnumber(jc->val,v,n);
   }
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_APLUS && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exeminus(struct Jcontext *jc,struct Element *elt)
{  static UBYTE attrtab[4][4]=
   {/* a-b   valid            nan      +inf              -inf */
   /*valid*/ {VNA_VALID,       VNA_NAN, VNA_NEGINFINITY,  VNA_INFINITY},
   /* nan */ {VNA_NAN,         VNA_NAN, VNA_NAN,          VNA_NAN},
   /* +inf*/ {VNA_INFINITY,    VNA_NAN, VNA_NAN,          VNA_INFINITY},
   /* -inf*/ {VNA_NEGINFINITY, VNA_NAN, VNA_NEGINFINITY,  VNA_NAN},
   };
   struct Value val1,val2;
   struct Variable *lhs;
   double n=0.0;
   UBYTE v;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);

   if((v=attrtab[val1.attr][val2.attr])==VNA_VALID)
   {
      n = val1.value.nvalue - val2.value.nvalue;
      if(isinf(n))
      {
         if(val1.value.nvalue>0) v=VNA_INFINITY;
         else v=VNA_NEGINFINITY;
      }
   }
   Asgnumber(jc->val,v,n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_AMINUS && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

#define FSIGN(d) (((d)>0.0)?1:(((d)<0.0)?-1:0))

static void Exemult(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   struct Variable *lhs;
   double n=0.0;
   UBYTE v;
   short sign=0;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);
   if(val1.attr==VNA_NAN || val2.attr==VNA_NAN)
   {  v=VNA_NAN;
   }
   else
   {  sign=FSIGN(val1.value.nvalue)*FSIGN(val2.value.nvalue);
      if(val1.attr==VNA_VALID && val2.attr==VNA_VALID)
      {
         n = val1.value.nvalue * val2.value.nvalue;
         if(isinf(n))
         {  v = (sign>0) ? VNA_INFINITY : VNA_NEGINFINITY;
         }
         else
         {  v=VNA_VALID;
         }
      }
      else
      {  /* Either operand is +/- infinity */
         if(sign)
         {  v=(sign>0)?VNA_INFINITY:VNA_NEGINFINITY;
         }
         else
         {  /* infinity times zero */
            v=VNA_NAN;
         }
      }
   }
   Asgnumber(jc->val,v,n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_AMULT && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exediv(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   struct Variable *lhs;
   double n=0.0;
   UBYTE v;
   short sign=0;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);
   if(val1.attr==VNA_NAN || val2.attr==VNA_NAN)
   {  v=VNA_NAN;
   }
   else
   {  sign=FSIGN(val1.value.nvalue)*FSIGN(val2.value.nvalue);
      if(val1.attr==VNA_VALID)
      {  if(val1.value.nvalue==0.0)
         {  if(val2.value.nvalue==0.0)
            {  /* 0/0 */
               v=VNA_NAN;
            }
            else
            {  /* 0/n, 0/i */
               n=0.0;
               v=VNA_VALID;
            }
         }
         else
         {  if(val2.attr==VNA_VALID)
            {  if(val2.value.nvalue==0.0)
               {  /* n/0 */
                  if(FSIGN(val1.value.nvalue)>0) v=VNA_INFINITY;
                  else v=VNA_NEGINFINITY;
               }
               else
               {  /* n/n */
                  n = val1.value.nvalue / val2.value.nvalue;
                  if(isinf(n))
                  {  n=0.0;
                     v=VNA_VALID;
                  }
                  else if(isnan(n))
                  {  v=(sign>0)?VNA_INFINITY:VNA_NEGINFINITY;
                  }
                  else
                  {  v=VNA_VALID;
                  }
               }
            }
            else
            {  /* n/i */
               n=0.0;
               v=VNA_VALID;
            }
         }
      }
      else
      {  if(val2.attr==VNA_VALID)
         {  if(val2.value.nvalue==0.0)
            {  /* i/0 */
               v=val1.attr;
            }
            else
            {  /* i/n */
               v=(sign>0)?VNA_INFINITY:VNA_NEGINFINITY;
            }
         }
         else
         {  /* i/i */
            v=VNA_NAN;
         }
      }
   }
   Asgnumber(jc->val,v,n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_ADIV && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exerem(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   struct Variable *lhs;
   double n=0.0;
   UBYTE v;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);
   if(val1.attr==VNA_NAN || val2.attr==VNA_NAN)
   {  v=VNA_NAN;
   }
   else
   {  if(val1.attr!=VNA_VALID || val2.value.nvalue==0.0)
      {  v=VNA_NAN;
      }
      else if(val2.attr!=VNA_VALID || val1.value.nvalue==0.0)
      {  n=val1.value.nvalue;
         v=VNA_VALID;
      }
      else
      {  n=fmod(val1.value.nvalue,val2.value.nvalue);
         v=VNA_VALID;
      }
   }
   Asgnumber(jc->val,v,n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_AREM && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exebitand(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   struct Variable *lhs;
   ULONG n=0;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);
   if(val1.attr==VNA_VALID && val2.attr==VNA_VALID)
   {  n=((ULONG)val1.value.nvalue) & ((ULONG)val2.value.nvalue);
   }
   Asgnumber(jc->val,VNA_VALID,(double)n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_ABITAND && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exebitor(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   struct Variable *lhs;
   ULONG n=0;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);
   if(val1.attr==VNA_VALID && val2.attr==VNA_VALID)
   {  n=((ULONG)val1.value.nvalue) | ((ULONG)val2.value.nvalue);
   }
   Asgnumber(jc->val,VNA_VALID,(double)n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_ABITOR && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exebitxor(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   struct Variable *lhs;
   ULONG n=0;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);
   if(val1.attr==VNA_VALID && val2.attr==VNA_VALID)
   {  n=((ULONG)val1.value.nvalue) ^ ((ULONG)val2.value.nvalue);
   }
   Asgnumber(jc->val,VNA_VALID,(double)n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_ABITXOR && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exeshleft(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   struct Variable *lhs;
   LONG n = 0;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);
      if(val1.attr==VNA_VALID && val2.attr==VNA_VALID)
   {  n=((LONG)val1.value.nvalue) << (((ULONG)val2.value.nvalue)&0x1f);
   }
   Asgnumber(jc->val,VNA_VALID,(double)n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_ASHLEFT && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exeshright(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   struct Variable *lhs;
   LONG n =0;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);
      if(val1.attr==VNA_VALID && val2.attr==VNA_VALID)
   {  n=((long)val1.value.nvalue) >> (((ULONG)val2.value.nvalue)&0x1f);
   }
   Asgnumber(jc->val,VNA_VALID,(double)n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_ASHRIGHT && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exeushright(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   struct Variable *lhs;
   ULONG n = 0;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   Tonumber(jc->val,jc);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Tonumber(jc->val,jc);
   Asgvalue(&val2,jc->val);
      if(val1.attr==VNA_VALID && val2.attr==VNA_VALID)
   {  n=((ULONG)(long)val1.value.nvalue) >> (((ULONG)val2.value.nvalue)&0x1f);
   }
   Asgnumber(jc->val,VNA_VALID,(double)n);
   Clearvalue(&val1);
   Clearvalue(&val2);
   if(elt->type==ET_AUSHRIGHT && lhs)
   {  if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }
   }
}

static void Exeequality(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   BOOL b;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Asgvalue(&val2,jc->val);
   if(val1.type==VTP_OBJECT && val2.type==VTP_OBJECT)
   {  /* Reference comparison */
      b=(BOOL)(val1.value.obj.ovalue==val2.value.obj.ovalue);
   }
   else if(val1.type==VTP_OBJECT && !val1.value.obj.ovalue)
   {  Toobject(&val2,jc);
      b=!val2.value.obj.ovalue;
   }
   else if(val2.type==VTP_OBJECT && !val2.value.obj.ovalue)
   {  Toobject(&val1,jc);
      b=!val1.value.obj.ovalue;
   }
   else if(val1.type>=VTP_STRING || val2.type>=VTP_STRING)
   {  /* String comparison */
      Tostring(&val1,jc);
      Tostring(&val2,jc);
      b=!strcmp(val1.value.svalue,val2.value.svalue);
   }
   else
   {  /* Numeric comparison */
      Tonumber(&val1,jc);
      Tonumber(&val2,jc);
      if(val1.attr==VNA_NAN || val2.attr==VNA_NAN)
      {  b=FALSE; /* NaN != NaN */
      }
      else if(val1.attr==VNA_VALID && val2.attr==VNA_VALID)
      {  b=(val1.value.nvalue==val2.value.nvalue);
      }
      else
      {  b=(val1.attr==val2.attr);
      }
   }
   if(elt->type==ET_NE) b=!b;
   Asgboolean(jc->val,b);
   Clearvalue(&val1);
   Clearvalue(&val2);
}

BOOL Exactequality(struct Jcontext *jc, struct Value *val1, struct Value *val2)
{
   BOOL b;

   if(val1->type!=val2->type)
   {
      b=FALSE;
   }
   else if(val1->type==VTP_OBJECT)
   {  /* Reference comparison */
      b=(BOOL)(val1->value.obj.ovalue==val2->value.obj.ovalue);
   }
   else if(val1->type>=VTP_STRING)
   {  /* String comparison */
      Tostring(val1,jc);
      Tostring(val2,jc);
      b=!strcmp(val1->value.svalue,val2->value.svalue);
   }
   else if(val1->type == VTP_BOOLEAN)
   {
       b=(val1->value.bvalue == val2->value.bvalue);
   }
   else
   {  /* Numeric comparison */
      Tonumber(val1,jc);
      Tonumber(val2,jc);
      if(val1->attr==VNA_NAN || val2->attr==VNA_NAN)
      {  b=FALSE; /* NaN != NaN */
      }
      else if(val1->attr==VNA_VALID && val2->attr==VNA_VALID)
      {  b=(val1->value.nvalue==val2->value.nvalue);
      }
      else
      {  b=(val1->attr==val2->attr);
      }
   }
   return b;
}

static void Exeexactequality(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   BOOL b;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Asgvalue(&val2,jc->val);

   b = Exactequality(jc,&val1,&val2);

   if(elt->type==ET_NEXEQ) b=!b;
   Asgboolean(jc->val,b);
   Clearvalue(&val1);
   Clearvalue(&val2);
}


static void Exerelational(struct Jcontext *jc,struct Element *elt)
{  struct Value val1,val2;
   double dc;
   long c;
   BOOL b,gotb=FALSE;
   val1.type=val2.type=0;
   Executeelem(jc,elt->sub1);
   Asgvalue(&val1,jc->val);
   Executeelem(jc,elt->sub2);
   Asgvalue(&val2,jc->val);
   /* NS 3 heuristic: if either value is numeric, convert both to numeric */
   if(val1.type==VTP_NUMBER || val2.type==VTP_NUMBER)
   {  Tonumber(&val1,jc);
      Tonumber(&val2,jc);
   }
   if(val1.type>=VTP_STRING || val2.type>=VTP_STRING)
   {  /* String comparison */
      Tostring(&val1,jc);
      Tostring(&val2,jc);
      c=strcmp(val1.value.svalue,val2.value.svalue);
   }
   else
   {  /* Numeric comparison */
      Tonumber(&val1,jc);
      Tonumber(&val2,jc);
      if(val1.attr==VNA_NAN || val2.attr==VNA_NAN)
      {  b=FALSE;
         gotb=TRUE;
      }
      else
      {  if(val1.attr==VNA_VALID && val2.attr==VNA_VALID)
         {  dc=val1.value.nvalue-val2.value.nvalue;
            if(dc>0) c=1;
            else if(dc<0) c=-1;
            else c=0;
         }
         else if(val1.attr==val2.attr)
         {  /* both i with same sign */
            c=0;
         }
         else if(val1.attr==VNA_INFINITY)
         {  /* i > -1 */
            c=1;
         }
         else
         {  /* -1 < i */
            c=-1;
         }
      }
   }
   if(!gotb)
   {  switch(elt->type)
      {  case ET_LT: b=(c<0);break;
         case ET_GT: b=(c>0);break;
         case ET_LE: b=(c<=0);break;
         case ET_GE: b=(c>=0);break;
      }
   }
   Asgboolean(jc->val,b);
   Clearvalue(&val1);
   Clearvalue(&val2);
}

/*
1.o / 1.1 versions

static void Exeand(struct Jcontext *jc,struct Element *elt)
{
   Executeelem(jc,elt->sub1);
   Toboolean(jc->val,jc);
   if(jc->val->value.bvalue)
   {  Executeelem(jc,elt->sub2);
      Toboolean(jc->val,jc);
   }
}

static void Exeor(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   Toboolean(jc->val,jc);
   if(!jc->val->value.bvalue)
   {  Executeelem(jc,elt->sub2);
      Toboolean(jc->val,jc);
   }
}

*/


static void Exeand(struct Jcontext *jc,struct Element *elt)
{
   /* 1.2 > version returns the values with no converion to boolen */
   /* a && b , return a if  a is false else return b */
   struct Value v;
   v.type = 0;
   Executeelem(jc,elt->sub1);
   Asgvalue(&v,jc->val);
   Toboolean(&v,jc);
   if(v.value.bvalue)
   {
       Executeelem(jc,elt->sub2);
   }
   Clearvalue(&v);
}

static void Exeor(struct Jcontext *jc,struct Element *elt)
{
   struct Value v;
   v.type = 0;
   Executeelem(jc,elt->sub1);
   Asgvalue(&v,jc->val);
   Toboolean(&v,jc);
   if(!v.value.bvalue)
   {
       Executeelem(jc,elt->sub2);
   }
   Clearvalue(&v);
}


static void Exeassign(struct Jcontext *jc,struct Element *elt)
{
   struct Variable *lhs;
   jc->flags|=EXF_ASGONLY;
   Executeelem(jc,elt->sub1);
   jc->flags&=~EXF_ASGONLY;
   lhs=jc->varref;
   if(lhs)
   {
    //  Addobjectlist(jc);
      Executeelem(jc,elt->sub2);
      if(!Callvhook(lhs,jc,VHC_SET,jc->val))
      {  Asgvalue(&lhs->val,jc->val);
         lhs->flags&=~VARF_HIDDEN;
      }

   // Removeobjectlist(jc);


      //Garbage collect against old list.
   //   if((jc->nogc<=0))
   //   {
   //      jc->gc = GC_THRESHOLD;
   //       Garbagecollect(jc);
   //   }

   }
}

static void Execomma(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   Executeelem(jc,elt->sub2);
}

static void Exein(struct Jcontext *jc, struct Element *elt)
{
    UBYTE* name;
    Executeelem(jc,elt->sub1);
    Tostring(jc->val,jc);
    name = jc->val->value.svalue;
    Executeelem(jc,elt->sub2);
    if(jc->val->type == VTP_OBJECT && jc->val->value.obj.ovalue)
    {
        BOOL result = FALSE;
        struct Variable *prop;
        if(name && (prop = Getproperty(jc->val->value.obj.ovalue,name)))
        {
            result = TRUE;
        }
        Asgboolean(jc->val,result);
    }
    else
    {
        Runtimeerror(jc,NTE_TYPE,elt,"Right hand side of in statement is not an object");
    }
}

static void Exeinstanceof(struct Jcontext *jc, struct Element *elt)
{
    struct Value res1;
    Executeelem(jc,elt->sub1);
    Asgvalue(&res1,jc->val);
    Executeelem(jc,elt->sub2);
    if(jc->val->type == VTP_OBJECT && jc->val->value.obj.ovalue)
    {
        BOOL result = FALSE;
        struct Variable *proto;
        if(res1.type == VTP_OBJECT)
        {
            if((proto = Getproperty(jc->val->value.obj.ovalue,"prototype")))
            {
                if(proto->val.type == VTP_OBJECT)
                {
                    if(res1.value.obj.ovalue->prototype == proto->val.value.obj.ovalue)
                    {
                        result = TRUE;
                    }
                }
                else
                {
                    Runtimeerror(jc,NTE_TYPE,elt,"Instanceof operand is of wrong type");
                }
            }
        }
        Asgboolean(jc->val,result);
    }
    else
    {
        Runtimeerror(jc,NTE_TYPE,elt,"Instance of operand is of wrong type");
    }
    Clearvalue(&res1);
}


static void Exewhile(struct Jcontext *jc,struct Element *elt)
{  for(;!(jc->flags&EXF_STOP);)
   {  Executeelem(jc,elt->sub1);
      Toboolean(jc->val,jc);
      if(!jc->val->value.bvalue) break;
      Executeelem(jc,elt->sub2);
      if(jc->complete>=ECO_RETURN) break;
      if(jc->complete==ECO_BREAK)
      {
         /* Check the curlabel, if it points us (or NULL) then it was
          * our break otherwise don't reset to ECO_NORMAL,
          * this will then filter up the exe tree
          */
         if(jc->curlabel)
         {
            if(jc->curlabel->elt == elt)
            {
                jc->complete=ECO_NORMAL;
                jc->curlabel=NULL;
            }
         }
         else
         {
            jc->complete=ECO_NORMAL;
         }
         break;
      }
      if(jc->complete==ECO_CONTINUE)
      {
         /* Check the cur label if it's null or points to us continue */
         if(jc->curlabel)
         {
            if(jc->curlabel->elt != elt)
            {
                break;
            }
            jc->curlabel=NULL;
         }
      }
      jc->complete=ECO_NORMAL;
   }
}

static void Exedo(struct Jcontext *jc,struct Element *elt)
{  for(;!(jc->flags&EXF_STOP);)
   {  Executeelem(jc,elt->sub1);
      if(jc->complete>=ECO_RETURN) break;
         if(jc->complete==ECO_BREAK)
         {
            /* Check the curlabel, if it points us (or NULL) then it was
             * our break otherwise don't reset to ECO_NORMAL,
             * this will then filter up the exe tree
             */
            if(jc->curlabel)
            {
               if(jc->curlabel->elt == elt)
               {
                   jc->complete=ECO_NORMAL;
                   jc->curlabel=NULL;
               }
            }
            else
            {
                jc->complete=ECO_NORMAL;
            }
            break;
         }
         if(jc->complete==ECO_CONTINUE)
         {
            /* Check the cur label if it's null or points to us continue */
            if(jc->curlabel)
            {
               if(jc->curlabel->elt != elt)
               {
                   break;
               }
               jc->curlabel=NULL;
            }
         }

      jc->complete=ECO_NORMAL;
      Executeelem(jc,elt->sub2);
      Toboolean(jc->val,jc);
      if(!jc->val->value.bvalue) break;
   }
}

static void Exewith(struct Jcontext *jc,struct Element *elt)
{  struct With *w;
   Executeelem(jc,elt->sub1);
   Toobject(jc->val,jc);
   if(jc->val->value.obj.ovalue)
   {
       if(w=Addwith(jc,jc->val->value.obj.ovalue))
       {  Executeelem(jc,elt->sub2);
          Remove((struct Node *)w);
          Disposewith(w);
       }
   }
   else
   {
       Runtimeerror(jc,NTE_TYPE,elt,"Argument to with is null or undefined");
   }
}

static void Exedot(struct Jcontext *jc,struct Element *elt)
{  struct Elementstring *mbrname=elt->sub2;
   BOOL ok=FALSE,temp=FALSE;
   BOOL asgonly=BOOLVAL(jc->flags&EXF_ASGONLY);
   struct Jobject *jo;
   BOOL newness;
   jc->flags&=~EXF_ASGONLY;
   Executeelem(jc,elt->sub1);
   temp=(jc->val->type!=VTP_OBJECT);
   Toobject(jc->val,jc);

   if(temp && jc->val->value.obj.ovalue)
   {  jc->val->value.obj.ovalue->flags|=OBJF_TEMP;
      Keepobject(jc->val->value.obj.ovalue,TRUE);
   }
   if(mbrname && mbrname->type==ET_IDENTIFIER)
   {
       jo = jc->val->value.obj.ovalue;
       ok=Member(jc,elt,jo,mbrname->svalue,asgonly);
   }
   if(!ok) Clearvalue(jc->val);
}

static void Exeindex(struct Jcontext *jc,struct Element *elt)
{  struct Value val,sval;
   BOOL ok=FALSE,temp=FALSE;
   val.type=sval.type=0;
   Executeelem(jc,elt->sub1);
   temp=(jc->val->type!=VTP_OBJECT);
   Toobject(jc->val,jc);
   if(temp && jc->val->value.obj.ovalue)
   {  jc->val->value.obj.ovalue->flags|=OBJF_TEMP;
      Keepobject(jc->val->value.obj.ovalue,TRUE);

   }
   Asgvalue(&val,jc->val);
   Executeelem(jc,elt->sub2);
   Asgvalue(&sval,jc->val);
   Tostring(&sval,jc);
   ok=Member(jc,elt,val.value.obj.ovalue,sval.value.svalue,FALSE);
   Clearvalue(&val);
   if(!ok) Clearvalue(jc->val);
   Clearvalue(&sval);
}

static void Exevar(struct Jcontext *jc,struct Element *elt)
{  struct Elementstring *vid=elt->sub1;
   struct Variable *var;
   if(vid && vid->type==ET_IDENTIFIER)
   {  var=Findlocalvar(jc,vid->svalue);
      if(elt->sub2)
      {  Executeelem(jc,elt->sub2);
         Asgvalue(&var->val,jc->val);
      }
      var->flags |= VARF_DONTDELETE;
      jc->varref=var;
      jc->flags|=EXF_KEEPREF;
   }
}

static void Execond(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   Toboolean(jc->val,jc);
   if(jc->val->value.bvalue)
   {  Executeelem(jc,elt->sub2);
   }
   else
   {  Executeelem(jc,elt->sub3);
   }
}

static void Exeif(struct Jcontext *jc,struct Element *elt)
{  Executeelem(jc,elt->sub1);
   Toboolean(jc->val,jc);
   if(jc->val->value.bvalue)
   {  Executeelem(jc,elt->sub2);
   }
   else
   {  Executeelem(jc,elt->sub3);
   }
}

static void Exeforin(struct Jcontext *jc,struct Element *elt)
{  struct Variable *var,*lhs;
   struct Jobject *jo;
   long n,i;
   Executeelem(jc,elt->sub1);
   lhs=jc->varref;
   if(lhs)
   {  for(n=0;!(jc->flags&EXF_STOP);n++)
      {  Executeelem(jc,elt->sub2);  /* Specs say must be evaluated for each iteration */
         Toobject(jc->val,jc);
         if(jo=jc->val->value.obj.ovalue)
         {  for(var=jo->properties.first,i=0;var->next && i<n;var=var->next,i++);
            if(!var->next) break;
            if(var->name && !(var->flags&VARF_HIDDEN))
            {  Asgstring(&lhs->val,var->name,jc->pool);
               lhs->flags&=~VARF_HIDDEN;
               Executeelem(jc,elt->sub3);
            }
         }else{break;}

         if(jc->complete>=ECO_RETURN) break;
         if(jc->complete==ECO_BREAK)
         {
            /* Check the curlabel, if it points us (or NULL) then it was
             * our break otherwise don't reset to ECO_NORMAL,
             * this will then filter up the exe tree
             */
            if(jc->curlabel)
            {
               if(jc->curlabel->elt == elt)
               {
                   jc->complete=ECO_NORMAL;
                   jc->curlabel=NULL;
               }
            }
            else
            {
                jc->complete=ECO_NORMAL;
            }
            break;
         }
         if(jc->complete==ECO_CONTINUE)
         {
            /* Check the cur label if it's null or points to us continue */
            if(jc->curlabel)
            {
               if(jc->curlabel->elt != elt)
               {
                   break;
               }
               jc->curlabel=NULL;
            }
         }

         jc->complete=ECO_NORMAL;
      }
   }
}

static void Exeswitch(struct Jcontext *jc,struct Elementswitch *elt)
{
    /* Execution takes two phases,
       1. Execute elt->cond;
       2. work through elementlist exexcuting any ET_CASE
       3. compare with cond
       4. if equal start executeing statements ignoreing any further
          ET_CASE
       5. if no match found and there is a default, start executing from
          default, ingnoring any further ET_CASE
    */
    struct Value val;
    struct Elementnode *enode;
    BOOL Casematch = FALSE;

    val.type = 0;

    Executeelem(jc,elt->cond);
    Asgvalue(&val,jc->val);

    for(enode=elt->subs.first;enode->next;enode=enode->next)
    {
       if(enode->sub && Casematch)
       {
          if(((struct Element *)enode->sub)->type !=ET_CASE)
          {
              Executeelem(jc,enode->sub);
             if(jc->complete==ECO_BREAK)
             {
                /* Check the curlabel, if it points us (or NULL) then it was
                 * our break otherwise don't reset to ECO_NORMAL,
                 * this will then filter up the exe tree
                 */
                if(jc->curlabel)
                {
                   if(jc->curlabel->elt == elt)
                   {
                       jc->complete=ECO_NORMAL;
                       jc->curlabel=NULL;
                   }
                }
                else
                {
                    jc->complete=ECO_NORMAL;
                }
                break;
             }
              if(jc->complete) break;

          }
       }
       else
       {
          if(((struct Element *)enode->sub)->type==ET_CASE)
          {
              if(!((struct Elementcase *)enode->sub)->isdefault)
              {
                  Executeelem(jc,enode->sub);

                  if(Exactequality(jc,&val,jc->val)) Casematch=TRUE;

              }
          }

       }
    }
    if(!Casematch)
    {
        /* no case matched go back and exe the default */
        if(elt->defaultcase)
        {
            for(enode=elt->defaultcase;enode->next;enode=enode->next)
            {
                if(((struct Element *)enode->sub)->type !=ET_CASE)
                {
                    Executeelem(jc,enode->sub);
             if(jc->complete==ECO_BREAK)
             {
                /* Check the curlabel, if it points us (or NULL) then it was
                 * our break otherwise don't reset to ECO_NORMAL,
                 * this will then filter up the exe tree
                 */
                if(jc->curlabel)
                {
                   if(jc->curlabel->elt == elt)
                   {
                       jc->complete=ECO_NORMAL;
                       jc->curlabel=NULL;
                   }
                }
                else
                {
                    jc->complete=ECO_NORMAL;
                }
                break;
             }

                    if(jc->complete) break;
                }
            }
        }
    }

}

static void Exetry(struct Jcontext *jc, struct Elementtry *elt)
{
    /*
        Execution:
            Execute try.
                If return is eco_NORMAL and no final return result;
                If return is ECO_THROW execute catch
            If Final execute final
                If return is normal return the result of try.

    */
    struct Variable *var;
    struct Value val;
    void * oldtry = jc->try;
    val.type = 0;

    jc->flags|=EXF_ASGONLY;
    Executeelem(jc,elt->catchvar);
    jc->flags&=~EXF_ASGONLY;
    var = jc->varref;

    jc->try = elt;

    Executeelem(jc,elt->try);
    if(jc->complete == ECO_THROW)
    {
        Asgvalue(&var->val,jc->throwval);
        var->flags |= VARF_DONTDELETE;
        jc->try = oldtry;
        Executeelem(jc,elt->catch);

    }
    jc->try = oldtry;

    Asgvalue(&val,jc->val);
    if(elt->finally)
    {
        Executeelem(jc,elt->finally);
        if(jc->complete != ECO_NORMAL)
        {
            Asgvalue(&val,jc->val);
        }
    }
    Asgvalue(jc->val,&val);
    Clearvalue(&val);
}



static void Exefor(struct Jcontext *jc,struct Element *elt)
{  BOOL c;
   Executeelem(jc,elt->sub1);
   for(;!(jc->flags&EXF_STOP);)
   {  if(elt->sub2)
      {  Executeelem(jc,elt->sub2);
         Toboolean(jc->val,jc);
         c=jc->val->value.bvalue;
      }
      else c=TRUE;
      if(!c) break;
      Executeelem(jc,elt->sub4);
      if(jc->complete>=ECO_RETURN) break;
      if(jc->complete==ECO_BREAK)
      {
         /* Check the curlabel, if it points us (or NULL) then it was
          * our break otherwise don't reset to ECO_NORMAL,
          * this will then filter up the exe tree
          */
         if(jc->curlabel)
         {
            if(jc->curlabel->elt == elt)
            {
                jc->complete=ECO_NORMAL;
                jc->curlabel=NULL;
            }
         }
         else
         {
             jc->complete=ECO_NORMAL;
         }
         break;
      }
      if(jc->complete==ECO_CONTINUE)
      {
         /* Check the cur label if it's null or points to us continue */
         if(jc->curlabel)
         {
            if(jc->curlabel->elt != elt)
            {
                break;
            }
            jc->curlabel=NULL;
         }
      }
      jc->complete=ECO_NORMAL;
      if(elt->sub3)
      {  Executeelem(jc,elt->sub3);
      }
   }
}

static void Exeinteger(struct Jcontext *jc,struct Elementint *elt)
{  Asgnumber(jc->val,VNA_VALID,(double)elt->ivalue);
}

static void Exefloat(struct Jcontext *jc,struct Elementfloat *elt)
{  Asgnumber(jc->val,VNA_VALID,elt->fvalue);
}

static void Exeboolean(struct Jcontext *jc,struct Elementint *elt)
{  Asgboolean(jc->val,elt->ivalue);
}

static void Exestring(struct Jcontext *jc,struct Elementstring *elt)
{  Asgstring(jc->val,elt->svalue,jc->pool);
}

static void Exeidentifier(struct Jcontext *jc,struct Elementstring *elt)
{  struct Jobject *jthis=NULL;
   struct Variable *var=Findvar(jc,elt->svalue,&jthis);
   if(var)
   {  /* Resolve synonym reference */
      while((var->flags&VARF_SYNONYM) && var->hookdata) var=var->hookdata;

      if(var->val.type==VTP_OBJECT && var->val.value.obj.ovalue && var->val.value.obj.ovalue->function)
      {  Asgfunction(jc->val,var->val.value.obj.ovalue,jthis?jthis:var->val.value.obj.fthis);
      }
      else
      {  if(!Callvhook(var,jc,VHC_GET,jc->val))
         {  Asgvalue(jc->val,&var->val);
         }
      }
      jc->varref=var;
      jc->flags|=EXF_KEEPREF;
   }
   else
   {  Clearvalue(jc->val);
   }
}

static void Exeregexp(struct Jcontext *jc, struct Elementregexp *elt)
{
    struct Jobject *jo;
    jo = Newregexp(jc,elt->pattern,elt->flags);
    Asgobject(jc->val,jo);
}

static void Exearray(struct Jcontext *jc, struct Elementlist *elist)
{
    struct Jobject *array = Newarray(jc);
    struct Elementnode *enode;
    struct Variable *arrayelt;
    if(array)
    {
        Keepobject(array,TRUE);
        for(enode=elist->subs.first;enode && enode->next;enode=enode->next)
        {
            if(((struct Element *)enode->sub)->type == ET_EMPTY)
            {
                Addarrayelt(jc,array);
            }
            else
            {
                Executeelem(jc,enode->sub);
                if((arrayelt = Addarrayelt(jc,array)))
                {
                     Asgvalue(&arrayelt->val,jc->val);
                }
            }
        }
        Keepobject(array,FALSE);
    }
    Asgobject(jc->val,array);
}

static void Exeobject(struct Jcontext *jc, struct Elementlist *elist)
{
    struct Jobject *object = NULL;
    struct Elementnode *enode;
    struct Variable *prop;

    if(object = Newobject(jc))
    {
        Keepobject(object,TRUE);
        Initconstruct(jc,object,"Object",NULL);

        for(enode=elist->subs.first;enode && enode->next;enode=enode->next)
        {
            /* The property can be either a indentifier, string literal
               or number */

            /* If a idenifier use its svalue */
            /* If a stringlit execute and use its value */
            /* If a numeric literal execute and use tostring value */
            /* if other this is an error, (dispose of object, return null?) */
            UBYTE *propname;

            switch(((struct Element *)enode->sub)->type)
            {
                case ET_INTEGER:
                case ET_STRING:

                    Executeelem(jc,enode->sub);
                    Tostring(jc->val,jc);
                    propname=jc->val->value.svalue;
                    break;

                case ET_IDENTIFIER:

                    propname=((struct Elementstring *)enode->sub)->svalue;
                    break;

                default:
                    /* temporay code */
                    enode=enode->next;
                    continue;
                    break;
            }

            if(!(prop = Getownproperty(object,propname)))
            {
                prop = Addproperty(object,propname);
            }
            enode=enode->next;
            Executeelem(jc,enode->sub);
            Asgvalue(&prop->val,jc->val);
        }
        Keepobject(object,FALSE);
    }
    Asgobject(jc->val,object);
}

static void Exelabel(struct Jcontext *jc,struct Element *elt)
{
    struct Label label;
    label.name = ((struct Elementstring *)elt->sub1)->svalue;
    label.elt =  elt->sub2;
    AddHead((struct List *)&jc->labels,(struct Node *)&label);
    Executeelem(jc, elt->sub2);
    RemHead((struct List *)&jc->labels);
}

#ifdef JSDEBUG
static void Exedebug(struct Jcontext *jc,struct Element *elt)
{  struct Value val;
   val.type=0;
   Executeelem(jc,elt->sub1);
   Asgvalue(&val,jc->val);
   Tostring(&val,jc);
   printf("%s\n",val.svalue);
   Clearvalue(&val);
}
#endif

#ifdef JSADDRESS
static void Exeaddress(struct Jcontext *jc,struct Element *elt)
{  struct Value val;
   UBYTE buf[16];
   val.type=0;
   Executeelem(jc,elt->sub1);
   Asgvalue(&val,jc->val);
   Toobject(&val,jc);
   sprintf(buf,"0x%08X",val.ovalue);
   Asgstring(jc->val,buf,jc->pool);
}
#endif

/*-----------------------------------------------------------------------*/
/* NB need to add in and instanceof operators */

typedef void Exeelement(void *,void *);
static Exeelement *exetab[]=
{
   NULL,
   (Exeelement *)Exeprogram,
   (Exeelement *)Execall,
   (Exeelement *)Execompound,
   (Exeelement *)Exevarlist,
   (Exeelement *)Exetry,
   (Exeelement *)Exefunction,
   (Exeelement *)Exefuncref,
   (Exeelement *)Exebreak,
   (Exeelement *)Execontinue,
   (Exeelement *)Exethis,
   (Exeelement *)Exenull,
   NULL,          /* empty */
   (Exeelement *)Exenegative,
   (Exeelement *)Exepositive,
   (Exeelement *)Exenot,
   (Exeelement *)Exebitneg,
   (Exeelement *)Exepreinc,
   (Exeelement *)Exepredec,
   (Exeelement *)Exepostinc,
   (Exeelement *)Exepostdec,
   (Exeelement *)Exenew,
   (Exeelement *)Exedelete,
   (Exeelement *)Exetypeof,
   (Exeelement *)Exevoid,
   (Exeelement *)Exereturn,
   (Exeelement *)Exethrow,
   (Exeelement *)Exeinternal,
   (Exeelement *)Exefunceval,
   (Exeelement *)Execase,
   (Exeelement *)Exeplus,
   (Exeelement *)Exeminus,
   (Exeelement *)Exemult,
   (Exeelement *)Exediv,
   (Exeelement *)Exerem,
   (Exeelement *)Exebitand,
   (Exeelement *)Exebitor,
   (Exeelement *)Exebitxor,
   (Exeelement *)Exeshleft,
   (Exeelement *)Exeshright,
   (Exeelement *)Exeushright,
   (Exeelement *)Exeequality,   /* eq */
   (Exeelement *)Exeequality,   /* ne */
   (Exeelement *)Exeexactequality, /* exeq */
   (Exeelement *)Exeexactequality, /* nexeq */
   (Exeelement *)Exerelational, /* lt */
   (Exeelement *)Exerelational, /* gt */
   (Exeelement *)Exerelational, /* le */
   (Exeelement *)Exerelational, /* ge */
   (Exeelement *)Exeand,
   (Exeelement *)Exeor,
   (Exeelement *)Exeassign,
   (Exeelement *)Exeplus,       /* aplus */
   (Exeelement *)Exeminus,      /* aminus */
   (Exeelement *)Exemult,       /* amult */
   (Exeelement *)Exediv,        /* adiv */
   (Exeelement *)Exerem,        /* arem */
   (Exeelement *)Exebitand,     /* abitand */
   (Exeelement *)Exebitor,      /* abitor */
   (Exeelement *)Exebitxor,     /* abitxor */
   (Exeelement *)Exeshleft,     /* ashleft */
   (Exeelement *)Exeshright,    /* ashright */
   (Exeelement *)Exeushright,   /* aushright */
   (Exeelement *)Execomma,
   (Exeelement *)Exein,
   (Exeelement *)Exeinstanceof,
   (Exeelement *)Exewhile,
   (Exeelement *)Exedo,
   (Exeelement *)Exewith,
   (Exeelement *)Exedot,
   (Exeelement *)Exeindex,
   (Exeelement *)Exevar,
   (Exeelement *)Execond,
   (Exeelement *)Exeif,
   (Exeelement *)Exeforin,
   (Exeelement *)Exeswitch,
   (Exeelement *)Exefor,
   (Exeelement *)Exeinteger,
   (Exeelement *)Exefloat,
   (Exeelement *)Exeboolean,
   (Exeelement *)Exestring,
   (Exeelement *)Exeidentifier,
   (Exeelement *)Exeregexp,
   (Exeelement *)Exearray,
   (Exeelement *)Exeobject,
   (Exeelement *)Exelabel,
#ifdef JSDEBUG
   (Exeelement *)Exedebug,
#endif
#ifdef JSADDRESS
   (Exeelement *)Exeaddress,
#endif
};
static long lastline = 0;

inline void Executeelem(struct Jcontext *jc,struct Element *elt)
{
   BOOL debugover=FALSE;
   struct Jbuffer *jb = NULL;
   struct Jcontext *tc;
   if(elt && exetab[elt->type])
   {
      if(!Feedback(jc))
      {  jc->flags|=EXF_STOP;
         tc = jc;
         while(tc = tc->truecontext)
         {
            tc->flags |= EXF_STOP;
         }
      }
      jc->elt = elt;

      //     jb = Jdecompile(jc,elt);
      //     if(jc->val->type == VTP_STRING)
      //     {
      //        adebug("jc->val string %s\n",jc->val->value.svalue);
      //     }
      //     else if(jc->val->type == VTP_NUMBER)
      //     {
      //       adebug("jc->val number %ld\n",(long)jc->val->value.nvalue);
      //     }
      //     adebug("CODE: %.70s\n",jb->buffer);
      //     if(jb) Freejbuffer(jb);

      if(!(jc->flags&EXF_STOP) && (jc->dflags&DEBF_DBREAK)
      && elt->type!=ET_PROGRAM)
      {  Setdebugger(jc,elt);
         /* Flags are set:
          * DEBUG DBREAK
          *   1      1   step into
          *   1      0   step over
          *   0      0   run or stop
          */
         debugover=(jc->dflags&DEBF_DEBUG) && !(jc->dflags&DEBF_DBREAK);
      }
      if(!(jc->flags&EXF_STOP))
      {  exetab[elt->type](jc,elt);
         if(!(jc->flags&EXF_KEEPREF))
         {  jc->varref=NULL;
         }
      }
      if(!(jc->flags&EXF_STOP) && debugover && (jc->dflags&DEBF_DEBUG))
      {  jc->dflags|=DEBF_DBREAK;
      }
      jc->flags&=~EXF_KEEPREF;
   }
}

/*-----------------------------------------------------------------------*/

/* Create a function object for this internal function.
 * Varargs are argument names (UBYTE *) terminated by NULL. */

struct Jobject *InternalfunctionA(struct Jcontext *jc,UBYTE *name,
   void (*code)(void *),UBYTE **args)
{  struct Jobject *jo=NULL;
   struct Elementfunc *func;
   struct Elementnode *enode;
   struct Element *elt;
   struct Elementstring *arg;
   UBYTE **a;
   ULONG numargs = 0;
   if(func=ALLOCSTRUCT(Elementfunc,1,0,jc->pool))
   {  func->type=ET_FUNCTION;
      func->name=Jdupstr(name,-1,jc->pool);
      NewList((struct List *)&func->subs);
      if(elt=ALLOCSTRUCT(Element,1,0,jc->pool))
      {  elt->type=ET_INTERNAL;
         elt->sub1=(void *)code;
      }
      func->body=elt;
      for(a=args;*a;a++)
      {

         if(arg=ALLOCSTRUCT(Elementstring,1,0,jc->pool))
         {  arg->type=ET_IDENTIFIER;
            arg->svalue=Jdupstr(*a,-1,jc->pool);
            if(enode=ALLOCSTRUCT(Elementnode,1,0,jc->pool))
            {  enode->sub=arg;
               AddTail((struct List *)&func->subs,(struct Node *)enode);
            }
         }
         numargs++;
      }
      if(jo=Newobject(jc))
      {
          struct Variable *var;
          if((var = Addproperty(jo,"length")))
          {
             Asgnumber(&var->val,VNA_VALID,(double)numargs);
             var->flags |= (VARF_DONTDELETE|VARF_HIDDEN);
          }
          jo->function=func;
      }
   }
   return jo;
}

struct Jobject *Internalfunction(struct Jcontext *jc,UBYTE *name,
   void (*code)(void *),...)
{
    VA_LIST ap;

    UBYTE **args;
    struct Jobject *jo;

    VA_STARTLIN(ap, code);
    args = VA_GETLIN(ap, UBYTE **);

    jo =InternalfunctionA(jc,name,code,args);

    VA_END(ap);

    return(jo);

}

/* Adds a function object to global variable list */
void Addglobalfunction(struct Jcontext *jc,struct Jobject *f)
{  struct Variable *var;
   if(f && f->function && f->function->type==ET_FUNCTION)
   {  if(var=Newvar(f->function->name,jc))
      {  AddTail((struct List *)&jc->functions.first->local,(struct Node *)var);
         Asgobject(&var->val,f);
         var->flags |= VARF_DONTDELETE;
      }
   }
}

/* Adds a function object to an object's properties */
struct Variable *Addinternalproperty(struct Jcontext *jc,struct Jobject *jo,struct Jobject *f)
{  struct Variable *var=NULL;
   if(f && f->function && f->function->type==ET_FUNCTION)
   {  if(var=Addproperty(jo,f->function->name))
      {  Asgobject(&var->val,f);
      }
   }
   return var;
}

/* Adds the .prototype property to a function object */
void Addprototype(struct Jcontext *jc,struct Jobject *jo, struct Jobject *prototype)
{  struct Variable *prop;
   struct Jobject *pro;
   if(prop=Addproperty(jo,"prototype"))
   {  if(pro=Newobject(jc))
      {  pro->constructor=jo;
         pro->hook=Prototypeohook;
         pro->prototype=prototype;
         Asgobject(&prop->val,pro);
      }
      prop->flags |= (VARF_DONTDELETE|VARF_HIDDEN);
   }
}

struct Jobject *Getprototype(struct Jobject *jo)
{
    struct Variable *proto;
    if((proto=Getownproperty(jo,"prototype"))
    && proto->val.type==VTP_OBJECT && proto->val.value.obj.ovalue)
    {
        return proto->val.value.obj.ovalue;
    }
    return NULL;
}

/* Add a function object to the object's prototype */
void Addtoprototype(struct Jcontext *jc,struct Jobject *jo,struct Jobject *f)
{  struct Variable *var,*proto;
   if(jo && f && f->function && f->function->type==ET_FUNCTION)
   {  if((proto=Getownproperty(jo,"prototype")) && proto->val.type==VTP_OBJECT && proto->val.value.obj.ovalue)
      {  if(var=Addproperty(proto->val.value.obj.ovalue,f->function->name))
         { // var->hook=Protopropvhook;
           // var->hookdata=proto->val.value.obj.ovalue->constructor;
            Asgobject(&var->val,f);
            var->flags|=VARF_HIDDEN;
         }
      }
   }
}

/* Add .constructor, default .toString() and .prototype properties */
/* Find constructor by name or use the explicit reference if name not present or not found */

void Initconstruct(struct Jcontext *jc,struct Jobject *jo, STRPTR name, struct Jobject *constructor)
{
   struct Variable *proto,*pro,*newpro,*p;
   struct Jobject *fo = NULL;

   if(name)
   {
       struct Variable *var;
       if((var = Findvar(jc,name,NULL)))
       {
          if(var->val.type == VTP_OBJECT &&
             var->val.value.obj.ovalue   &&
             var->val.value.obj.ovalue->function)
          {
              fo = var->val.value.obj.ovalue;
          }
       }
   }
   if(!fo && constructor)
   {
      fo = constructor;
   }
   if(fo)
   {  if(!jo->constructor) jo->constructor=fo;
      if(newpro=Addproperty(jo,"constructor"))
      {  Asgfunction(&newpro->val,fo,NULL);
         newpro->flags|=VARF_HIDDEN;
      }

      if((proto=Getownproperty(fo,"prototype"))
      && proto->val.type==VTP_OBJECT && proto->val.value.obj.ovalue)
      {

         jo->prototype = proto->val.value.obj.ovalue;

/* Old way keep comented for reference just incase .... */
/*
         for(pro=proto->val.value.obj.ovalue->properties.first;pro->next;pro=pro->next)
         {  if(!(newpro=Findproperty(jo,pro->name)))
            {  newpro=Addproperty(jo,pro->name);
            }
            if(newpro)
            {  Asgvalue(&newpro->val,&pro->val);
               newpro->flags|=(pro->flags&VARF_HIDDEN);
            }
         }
*/
      }
      else
      {
         /* this probably shouldn't happen but in just in case */
         /* it might be that we should throw a run time error here */

         jo->prototype = NULL;
      }
   }
   if(!(pro=Getproperty(jo,"toString")))
   {  if(p=Addinternalproperty(jc,jo,jc->tostring))
      {  p->flags|=VARF_HIDDEN;
      }
   }
   if(!(pro=Getproperty(jo,"eval")))
   {  if(p=Addinternalproperty(jc,jo,jc->eval))
      {  p->flags|=VARF_HIDDEN;
      }
   }
}

/*-----------------------------------------------------------------------*/

/* Evaluate this string */
void Jeval(struct Jcontext *jc,UBYTE *s)
{
   // adebug("enter jeval jc %08lx %s\n",jc,s);
   struct Jcontext jc2={0};
   jc2.truecontext = jc;
   jc->nogc ++;
   jc2.pool=jc->pool;
   jc2.objpool=jc->objpool;
   jc2.varpool=jc->varpool;
   jc2.nogc ++;
   NewList((struct List *)&jc2.objects);
   NewList((struct List *)&jc2.functions);
   jc2.generation=jc->generation;
   Jcompile(&jc2,s);
   jc->nogc--;
   if(!(jc2.flags&JCF_ERROR))
   {  Executeelem(jc,jc2.program);
   }
   Jdispose(jc2.program);
   // adebug("exf_stop %s \n",(jc->flags & EXF_STOP)?"TRUE":"FALSE");
   // adebug("leave jeval jc %08lx\n",jc);
}

/*-----------------------------------------------------------------------*/

void Jexecute(struct Jcontext *jc,struct Jobject *jthis,struct Jobject **gwtab)
{  struct Jobject *oldjthis,*oldfscope;
   struct With *w1,*w;
   struct Jobject **jp;
   struct Value oldretval;
   oldretval.type = 0;
   if(jc && jc->program)
   {  if(jc->jthis) Keepobject(jc->jthis,TRUE);
      oldjthis=jc->jthis;
      jc->jthis=jthis;
      /* By setting the last (JS global) function scope to jc->fscope we ensure
       * that the correct global scope is used also when not within a function. */
      oldfscope=jc->functions.last->fscope;
      jc->functions.last->fscope=jc->fscope;
      /* Add With's for global data scopes and function scope. Remember first
       * With, and remove the entire series from that one after execution
       * (backwards because Withs are ADDHEAD'ed). */
      w1=NULL;
      if(gwtab)
      {  for(jp=gwtab;*jp;jp++)
         {  if(w=Addwith(jc,*jp))
            {  w->flags|=WITHF_GLOBAL;
               if(!w1) w1=w;
            }
         }
      }
      Asgvalue(&oldretval,&jc->functions.last->retval);
      /* Set default TRUE return value */
      Asgboolean(&jc->functions.last->retval,TRUE);

      if(oldjthis)Keepobject(oldjthis,TRUE);
      if(oldfscope)Keepobject(oldfscope,TRUE);
      if(oldretval.type == VTP_OBJECT)
      {
          if(oldretval.value.obj.ovalue)Keepobject(oldretval.value.obj.ovalue,TRUE);
          if(oldretval.value.obj.fthis)Keepobject(oldretval.value.obj.fthis,TRUE);

      }

      Executeelem(jc,jc->program);
      if(w1)
      {  for(;w1->prev;w1=w)
         {  w=w1->prev;
            Remove((struct Node *)w1);
            Disposewith(w1);
         }
      }

      if(oldjthis)Keepobject(oldjthis,FALSE);
      if(oldfscope)Keepobject(oldfscope,FALSE);
      if(oldretval.type == VTP_OBJECT)
      {
          if(oldretval.value.obj.ovalue)Keepobject(oldretval.value.obj.ovalue,FALSE);
          if(oldretval.value.obj.fthis)Keepobject(oldretval.value.obj.fthis,FALSE);

      }

      jc->jthis=oldjthis;
      if(jc->jthis) Keepobject(jc->jthis,FALSE);
      jc->functions.last->fscope=oldfscope;
      jc->complete=ECO_NORMAL;
      /* Keep execution result and restore old result */
      Asgvalue(&jc->result->val,jc->val);
      Asgvalue(jc->val,&jc->functions.last->retval);
      Asgvalue(&jc->functions.last->retval,&oldretval);
      Clearvalue(&oldretval);
   }
}

BOOL Newexecute(struct Jcontext *jc)
{  struct Function *f;
   struct Jobject *fo;
   jc->gc = GC_THRESHOLD;
   NewList((struct List *)&jc->functions);
   NewList((struct List *)&jc->labels);
   NewList((struct List *)&jc->thislist);
   if((jc->valvar=Newvar(NULL,jc))
   && (jc->result=Newvar(NULL,jc))
   && (jc->throw=Newvar(NULL,jc))
   && (jc->jthis=Newobject(jc))
   && (f=Newfunction(jc,NULL)))
   {  Keepobject(jc->jthis,TRUE);
      jc->val=&jc->valvar->val;
      jc->throwval=&jc->throw->val;
      AddHead((struct List *)&jc->functions,(struct Node *)f);
      jc->tostring=Internalfunction(jc,"toString",(Internfunc *)Defaulttostring,NULL);
      Keepobject(jc->tostring,TRUE);
      if(jc->eval=Internalfunction(jc,"eval",(Internfunc *)Eval,"Expression",NULL))
      {  Addglobalfunction(jc,jc->eval);
      }
      if(fo=Internalfunction(jc,"parseInt",(Internfunc *)Parseint,"string","radix",NULL))
      {  Addglobalfunction(jc,fo);
      }
      if(fo=Internalfunction(jc,"parseFloat",(Internfunc *)Parsefloat,"string",NULL))
      {  Addglobalfunction(jc,fo);
      }
      if(fo=Internalfunction(jc,"escape",(Internfunc *)Escape,"string",NULL))
      {  Addglobalfunction(jc,fo);
      }
      if(fo=Internalfunction(jc,"unescape",(Internfunc *)Unescape,"string",NULL))
      {  Addglobalfunction(jc,fo);
      }
      if(fo=Internalfunction(jc,"isNaN",(Internfunc *)Isnan,"testValue",NULL))
      {  Addglobalfunction(jc,fo);
      }
      Initobject(jc,NULL);
      Initarray(jc,NULL);
      Initboolean(jc,NULL);
      Initfunction(jc,NULL);
      Initnumber(jc,NULL);
      Initstring(jc,NULL);
      Initregexp(jc,NULL);
      return TRUE;
   }
   Freeexecute(jc);
}

void Freeexecute(struct Jcontext *jc)
{  struct Function *f;
   if(jc)
   {  if(jc->val)
      {  if(jc->valvar) Disposevar(jc->valvar);
         if(jc->result) Disposevar(jc->result);
         if(jc->throw)  Disposevar(jc->throw);
         while(f=(struct Function *)RemHead((struct List *)&jc->functions)) Disposefunction(f);
         Remove(jc->jthis);
         Disposeobject(jc->jthis);
         Remove(jc->tostring);
         Disposeobject(jc->tostring);
      }
   }
}

struct Jobject * Newscope(struct Jcontext *jc)
{
    struct Jobject *jo,*fo;
    struct Variable *func;

    jc->nogc++;

    if((jo=Newobject(jc)))
    {
        /* Keep Object while we add things to it */
        Keepobject(jo,TRUE);
      if(fo=Internalfunction(jc,"parseInt",(Internfunc *)Parseint,"string","radix",NULL))
      {
          if((func = Addproperty(jo,"parseInt")))
          {
              Asgfunction(&func->val,fo,NULL);
          }
      }
      if(fo=Internalfunction(jc,"parseFloat",(Internfunc *)Parsefloat,"string",NULL))
      {
          if((func = Addproperty(jo,"parseFloat")))
          {
              Asgfunction(&func->val,fo,NULL);
          }

      }
      if(fo=Internalfunction(jc,"escape",(Internfunc *)Escape,"string",NULL))
      {
          if((func = Addproperty(jo,"escape")))
          {
              Asgfunction(&func->val,fo,NULL);
          }
      }
      if(fo=Internalfunction(jc,"unescape",(Internfunc *)Unescape,"string",NULL))
      {
          if((func = Addproperty(jo,"unescape")))
          {
              Asgfunction(&func->val,fo,NULL);
          }
      }
      if(fo=Internalfunction(jc,"isNaN",(Internfunc *)Isnan,"testValue",NULL))
      {
          if((func = Addproperty(jo,"isNaN")))
          {
              Asgfunction(&func->val,fo,NULL);
          }
      }

        Initconstruct(jc,jo,NULL,NULL);
        Initobject(jc,jo);
        Initarray(jc,jo);
        Initboolean(jc,jo);
        Initdate(jc,jo);
        Initerror(jc,jo);
        Initfunction(jc,jo);
        Initmath(jc,jo);
        Initnumber(jc,jo);
        Initstring(jc,jo);
        Initregexp(jc,jo);

        /* Not our responsibilty any more ... */
        //Keepobject(jo,FALSE);
    }
    jc->nogc--;
    return jo;
}
