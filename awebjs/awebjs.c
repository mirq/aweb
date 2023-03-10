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

char version[]="\0$VER:AWebJS " AWEBVERSION " " CPU " " __AMIGADATE__;

#ifdef _SASC
__near
#endif
long __stack=16348;

struct AWebJSBase *AWebJSBase;
#if defined(__amigaos4__)
struct AWebJSIFace *IAWebJS;
#endif

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
            if(jo=Newjscope(jc))
            {  Addjfunction(jc,jo,"write",aWrite,"string",NULL);
               Addjfunction(jc,jo,"writeln",aWriteln,"string",NULL);
               Addjfunction(jc,jo,"print",aWriteln,"string",NULL);
               Addjfunction(jc,jo,"readln",aReadln,NULL);
               if(args[2])
               {  Jdebug(jc,TRUE);
               }
               for(p=(UBYTE **)args[0];*p;p++)
               {  if(f=fopen(*p,"r"))
                  {  fseek(f,0,SEEK_END);
                     l=ftell(f);
                     fseek(f,0,SEEK_SET);
#if defined(__amigaos4__)
                     if(source=AllocVec(l+1,MEMF_SHARED|MEMF_CLEAR))
#else
                     if(source=AllocVec(l+1,MEMF_PUBLIC|MEMF_CLEAR))
#endif
                     {  fread(source,l,1,f);
                        Runjprogram(jc,jo,source,jo,NULL,0,0);
                        FreeVec(source);
                     }
                     fclose(f);
                  }
                  else fprintf(stderr,"Can't open %s\n",*p);
               }
            }
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
