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

/* awebjs.c - AWeb js main */

#include "platform_specific.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <stdio.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/locale.h>


#include "jslib.h"
#include "keyfile.h"
#include "arexxjs.h" 

char version[]="\0$VER:AWebJS " AWEBVERSION " " CPU " " __AMIGADATE__;

#ifdef _SASC
__near
#endif
long __stack=16348;

struct AWebJSBase *AWebJSBase;
#if defined(__amigaos4__)
struct AWebJSIFace *IAWebJS;
#endif

static globalscope = NULL;

/* Standard I/O */
static __saveds  void  aWrite( struct Jcontext *jc)
{
  struct Jvar *jv;
   long n;

   for(n=0;jv=Jfargument(jc,n);n++)
   {
      printf("%s",Jtostring(jc,jv));
   }

}

static __saveds  void  aWriteln( struct Jcontext * jc)
{
   aWrite(jc);
   printf("\n");

}

static __saveds  void aReadln( struct Jcontext *jc)
{
   UBYTE buf[80];
   long l;
   if(!fgets(buf,80,stdin)) *buf='\0';
   l=strlen(buf);
   if(l && buf[l-1]=='\n') buf[l-1]='\0';
   Jasgstring(jc,NULL,buf);

}

static __saveds  BOOL  Feedback( struct Jcontext *jc)
{
  if(SetSignal(0,0)&SIGBREAKF_CTRL_C) return FALSE;
  return TRUE;
}

static void require(struct Jcontext *jc)
{
   STRPTR filename = NULL;
   APTR source;
   FILE *fh;
   ULONG length;
   ULONG n;
   ULONG i = 0;
   struct Jobject *jgscope[4];
   struct Jvar *jv;
   
   struct Jobject *jthis = Jthis(jc);
   
   jgscope[i++] = globalscope;

   if(jthis != globalscope)
   {
      jgscope[i++] = jthis;
   }
   jgscope[i] = NULL;
   
   for(n=0;jv=Jfargument(jc,n);n++)
   {
      filename = Jtostring(jc,jv);
      if((fh = fopen(filename,"r")))
      {
          fseek(fh,0,SEEK_END);
          length = ftell(fh);
          fseek(fh,0,SEEK_SET);
          if((source=AllocVec(length+1,MEMF_PRIVATE|MEMF_CLEAR)))
          {  
              fread(source,length,1,fh);
              Jerrors(jc,TRUE,1,FALSE);
              Runjprogram(jc,globalscope,source,jthis,jgscope,0,0);
              FreeVec(source);
          }
          fclose(fh);     
      }
      else
      {
      	printf("failed to load %s\n",filename);
      }
   }
    
}

static void delay(struct Jcontext *jc)
{
    struct Jvar *jv = Jfargument(jc,0);
    if(jv)
    {
        ULONG ticks = Jtonumber(jc,jv);
        Delay(ticks);
    }
}


int main()
{  long args[4]={0};
   UBYTE *argtemplate="FILES/M/A,PUBSCREEN/K,-D=DEBUG/S";
   struct RDArgs *rda;
   UBYTE **p;
   UBYTE *source;
   FILE *f;
   long l;
   void *jc;
   void *jo;
   void *dtbase;
   void *jo_arexx;
   void *jv;
   
/* window.class bug workaround */
dtbase=OpenLibrary("datatypes.library",0);

   if(rda=ReadArgs(argtemplate,args,NULL))
   {  if((AWebJSBase=(struct AWebJSBase *)OpenLibrary("aweblib/javascript.aweblib",0))
      || (AWebJSBase=(struct AWebJSBase *)OpenLibrary("javascript.aweblib",0))
      || (AWebJSBase=(struct AWebJSBase *)OpenLibrary("PROGDIR:aweblib/javascript.aweblib",0))
      || (AWebJSBase=(struct AWebJSBase *)OpenLibrary("PROGDIR:javascript.aweblib",0)))
      {

#if defined(__amigaos4__)
         if(IAWebJS = (struct AWebJSIFace *)GetInterface((struct Library *)AWebJSBase,"main",1,NULL))
         {
#endif
         if(jc=Newjcontext((UBYTE *)args[1]))
         {  Jsetfeedback(jc,Feedback);
            Jerrors(jc,TRUE,JERRORS_ON,TRUE);
            initarexx(jc);
            struct Jobject *jgscope[4];
            
            if(jo=Newjscope(jc))
            {  
               globalscope = jo;
               jgscope[0] = jo;
               jgscope[1] = NULL;
               
               Addjfunction(jc,jo,"write",aWrite,"string",NULL);
               Addjfunction(jc,jo,"writeln",aWriteln,"string",NULL);
               Addjfunction(jc,jo,"print",aWriteln,"string",NULL);
               Addjfunction(jc,jo,"readln",aReadln,NULL);
               Addjfunction(jc,jo,"require",require,"filename",NULL);
               Addjfunction(jc,jo,"delay",delay,"ticks",NULL);   
               
               if((jo_arexx = Newjobject(jc)))
               {
                   if((jv = Jproperty(jc,jo,"ARexx")))
                   {
                       Jasgobject(jc,jv,jo_arexx);
                       Addjfunction(jc,jo_arexx,"SendCommand",SendCommand,"host","command",NULL);
                   }
               }
               
               if(args[2])
               {  Jdebug(jc,TRUE);
               }
               for(p=(UBYTE **)args[0];*p;p++)
               {  if(f=fopen(*p,"r"))
                  {  fseek(f,0,SEEK_END);
                     l=ftell(f);
                     fseek(f,0,SEEK_SET);
#if defined(__amigaos4__)
                     if(source=AllocVec(l+1,MEMF_PRIVATE|MEMF_CLEAR))
#else
                     if(source=AllocVec(l+1,MEMF_PUBLIC|MEMF_CLEAR))
#endif
                     {  fread(source,l,1,f);
                        Jerrors(jc,TRUE,1,FALSE);
                        Runjprogram(jc,jo,source,jo,jgscope,0,0);
                        FreeVec(source);
                     }
                     fclose(f);
                  }
                  else fprintf(stderr,"Can't open %s\n",*p);
               }
            }
            freearexx(jc);
            Freejcontext(jc);

         }
#if defined(__amigaos4__)
        DropInterface((struct Interface *)IAWebJS);
        }
        else
        {
            printf("Couldn`t Open javascript.aweblib Interface\n");
        }
#endif
         CloseLibrary((struct Library *)AWebJSBase);
      }
      else
      {
            printf("Couldn`t Open javascript.aweblib Library\n");

      }
      FreeArgs(rda);
   }
/* window.class bug workaround */
if(dtbase) CloseLibrary(dtbase);
return (0);
}
