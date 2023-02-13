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

/* makeindex.c - Make index for HTML docs */

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

static void Writeindex(UBYTE t,UBYTE *line,long to)
{  UBYTE buf[512];
   sprintf(buf,"%c %s\n",t,line);
   Write(to,buf,strlen(buf));
}

/* Find this attr, copies value to buf */
static void Findattr(UBYTE *p,UBYTE *attr,UBYTE *buf,long len)
{  short i=0;
   short l=strlen(attr);
   UBYTE quot='\0';
   buf[0]='\0';
   for(;;)
   {  while(isspace(*p)) p++;
      if(!*p || *p=='>') break;
      if(STRNIEQUAL(p,attr,l) && (isspace(p[l]) || p[l]=='='))
      {  p+=l;
         while(isspace(*p)) p++;
         if(*p=='=')
         {  p++;
            while(isspace(*p)) p++;
            if(*p=='"' || *p=='\'')
            {  quot=*p;
               p++;
            }
            for(i=0;i<len-1 && *p;i++)
            {  if(quot && *p==quot) break;
               if(!quot && (isspace(*p) || *p=='>')) break;
               buf[i]=*p++;
            }
            buf[i]='\0';
         }
         break;
      }
      else
      {  while(*p && !isspace(*p) && *p!='>') p++;
      }
   }
}

static void Scansource(UBYTE *buf,long to)
{  short level=0;
   UBYTE *p;
   UBYTE line[80];
   UBYTE keywords[256]="";
   UBYTE allkeywords[500]="";
   short i;
   BOOL space=TRUE;

   p=buf;
   i=0;

   while(*p)
   {  if(*p=='<')
      {  p++;
         if(toupper(*p)=='H')
         {  p++;
            if(*p>='1' && *p<='6')
            {  level=*p-'0';
            }
         }
         else if(toupper(*p)=='A')
         {  p++;
            Findattr(p,"NAME",line,sizeof(line));
            if(*line)
            {  Writeindex('A',line,to);
               i=0;
               if(!level) level=7;
            }
         }
         else if(STRNIEQUAL(p,"DT>",3))
         {  if(!level) level=8;
         }
         else if(STRNIEQUAL(p,"META",4))
         {  p+=4;
            Findattr(p,"NAME",line,sizeof(line));
            if(STRIEQUAL(line,"KEYWORDS"))
            {  Findattr(p,"CONTENT",keywords,sizeof(keywords));
               if(*keywords)
               {  if(strlen(allkeywords)<sizeof(allkeywords)-2)
                  {  if(*allkeywords)
                     {  strcat(allkeywords,",");
                     }
                     strncat(allkeywords,keywords,sizeof(allkeywords)-strlen(allkeywords)-2);
                  }
                  *keywords='\0';
               }
            }
         }
         else if((STRNIEQUAL(p,"/H",2) && p[2]>='0' && p[2]<='6')
         || STRNIEQUAL(p,"/A>",3)
         || STRNIEQUAL(p,"DD>",3)
         || STRNIEQUAL(p,"/DL>",4))
         {  if(level)
            {  Writeindex('0'+level,line,to);
               i=0;
               space=TRUE;
               if(*allkeywords)
               {  Writeindex('K',allkeywords,to);
                  *allkeywords='\0';
               }
            }
            level=0;
         }
         while(*p && *p!='>') p++;
      }
      else
      {  if(level)
         {  if(STRNEQUAL(p,"&nbsp;",6))
            {  p+=5;
               *p=' ';
            }
            if(!isspace(*p) || !space)
            {  if(isspace(*p))
               {  *p=' ';
                  space=TRUE;
               }
               else
               {  space=FALSE;
               }
               if(i<sizeof(line)-1)
               {  line[i++]=*p;
                  line[i]='\0';
               }
            }
         }
      }
      if(*p) p++;
   }
}

static void Scanfile(UBYTE *name,long size,long to)
{  long file;
   UBYTE *buf;
   if(file=Open(name,MODE_OLDFILE))
   {  if(buf=ALLOCTYPE(UBYTE,size+1,0))
      {  if(size=Read(file,buf,size))
         {  buf[size]='\0';
            Writeindex('F',name,to);
            Scansource(buf,to);
         }
         FREE(buf);
      }
      Close(file);
   }
}

static void Scandir(UBYTE *dir,long to)
{  long lock;
   struct FileInfoBlock *fib;
   UBYTE dirbuf[512];
   long l;
   if(lock=Lock(dir,SHARED_LOCK))
   {  if(fib=AllocDosObjectTags(DOS_FIB,TAG_END))
      {  if(Examine(lock,fib))
         {  while(ExNext(lock,fib))
            {  if(fib->fib_DirEntryType<0)
               {  l=strlen(fib->fib_FileName);
                  if(STRIEQUAL(fib->fib_FileName+l-5,".html"))
                  {  strncpy(dirbuf,dir,511);
                     if(AddPart(dirbuf,fib->fib_FileName,512))
                     {  Scanfile(dirbuf,fib->fib_Size,to);
                     }
                  }
               }
            }
         }
/*
         if(Examine(lock,fib))
         {  while(ExNext(lock,fib))
            {  if(fib->fib_DirEntryType>0)
               {  strncpy(dirbuf,dir,511);
                  if(AddPart(dirbuf,fib->fib_FileName,512))
                  {  Scandir(dirbuf,to);
                  }
               }
            }
         }
*/
         FreeDosObject(DOS_FIB,fib);
      }
      UnLock(lock);
   }
}

int main(void)
{  long args[4];
   UBYTE *argtemplate="DIR/M,TO/A";
   struct RDArgs *rda;
   UBYTE **dir;
   long to;

   if(rda=ReadArgs(argtemplate,args,NULL))
   {  if(to=Open((UBYTE *)args[1],MODE_NEWFILE))
      {  for(dir=(UBYTE **)(args[0]);*dir;dir++)
         {  Scandir(*dir,to);
         }
         Close(to);
      }
      FreeArgs(rda);
   }
   return 0;
}
