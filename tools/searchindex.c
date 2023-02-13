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

/* searchindex.c - Search index of HTML docs */

#include "platform_specific.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define STRNIEQUAL(a,b,n)  !strnicmp(a,b,n)
#define STRNEQUAL(a,b,n)   !strncmp(a,b,n)
#define STRIEQUAL(a,b)     !stricmp(a,b)
#define STREQUAL(a,b)      !strcmp(a,b)

#define ALLOCTYPE(t,n,f)   (t*)AllocVec((n)*sizeof(t),(f)|MEMF_PUBLIC)
#define ALLOCSTRUCT(s,n,f) ALLOCTYPE(struct s,n,f)
#define FREE(p)            FreeVec(p)

static UBYTE outbuf[1024];

static void Writeout(long to,UBYTE *file,UBYTE *frag,UBYTE *line,UBYTE **a)
{  short i;
   UBYTE *p,*q;
   strcpy(outbuf,"<li><a href=file:///");
   strcat(outbuf,file);
   if(frag && *frag)
   {  strcat(outbuf,"#");
      strcat(outbuf,frag);
   }
   strcat(outbuf,">");
   for(p=line,q=outbuf+strlen(outbuf);*p;p++,q++)
   {  if(*p=='"') *q++='*';
      *q=*p;
   }
   *q='\0';
   strcat(outbuf,"</a>*N<br>");
   for(i=0;i<10 && a[i];i++)
   {  if(i>0) strcat(outbuf,", ");
      for(p=a[i],q=outbuf+strlen(outbuf);*p;p++,q++)
      {  if(*p=='"') *q++='*';
         *q=*p;
      }
      *q='\0';
   }
   strcat(outbuf,"<br>*N<a href=file:///");
   strcat(outbuf,file);
   if(frag && *frag)
   {  strcat(outbuf,"#");
      strcat(outbuf,frag);
   }
   strcat(outbuf,">file:///");
   strcat(outbuf,file);
   if(frag && *frag)
   {  strcat(outbuf,"#");
      strcat(outbuf,frag);
   }
   strcat(outbuf,"</a><br><br>*N");
   Write(to,outbuf,strlen(outbuf));
}

static void Scanindex(UBYTE *buf,long to,UBYTE *arg)
{  UBYTE *p,*q,*eol;
   UBYTE *a[10]={ NULL },*file=NULL,*frag=NULL;
   UBYTE level='8';
   short dosearch=0,i;
   long l=strlen(arg);

   for(p=buf;*p;p++)
   {  if(eol=strchr(p,'\n')) *eol='\0';
      else eol=p+strlen(p);

      dosearch=0;
      if(*p=='F')
      {  file=p+2;
         frag=NULL;
         level='8';
      }
      else if(*p=='A')
      {  frag=p+2;
      }
      else if(*p=='K')
      {  dosearch=2;
         *p='1';
      }
      else if(*p>='1' && *p<=level)
      {  i=*p-'1';
         a[i]=p+2;
         for(i++;i<10;i++) a[i]=NULL;
         dosearch=1;
      }
      if(dosearch)
      {  for(q=p+2;*q;q++)
         {  if(STRNIEQUAL(q,arg,l))
            {  Writeout(to,file,frag,dosearch==2?a[0]:p+2,a);
               level=*p;
               break;
            }
         }
      }
      p=eol;
   }
}

int main(void)
{  long args[4];
   UBYTE *argtemplate="INDEX/A,TO/A,SEARCH/A/F";
   struct RDArgs *rda;
   long index,to,read;
   UBYTE *buf=NULL;
   struct FileInfoBlock *fib;

   if(rda=ReadArgs(argtemplate,args,NULL))
   {  if(index=Open((UBYTE *)args[0],MODE_OLDFILE))
      {  if(fib=AllocDosObjectTags(DOS_FIB,TAG_END))
         {  if(ExamineFH(index,fib))
            {  if(buf=ALLOCTYPE(UBYTE,fib->fib_Size+1,0))
               {  read=Read(index,buf,fib->fib_Size);
                  buf[read]='\0';
               }
            }
            FreeDosObject(DOS_FIB,fib);
         }
         Close(index);
      }
      if(to=Open((UBYTE *)args[1],MODE_NEWFILE))
      {  Scanindex(buf,to,(UBYTE *)args[2]);
         Close(to);
      }
      FreeArgs(rda);
   }
   if(buf) FREE(buf);
   return 0;
}
