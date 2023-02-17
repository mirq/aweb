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

/* local.c aweb localfile */

#include "aweb.h"
#include "fetchdriver.h"
#include "tcperr.h"
#include "window.h"
#include "asyncio.h"
#include "task.h"
#include "form.h"
#include <exec/ports.h>

#include <string.h>

#ifdef DEVELOPER
extern long localblocksize;
#endif

/*-----------------------------------------------------------------------*/

void Localfiletask(struct Fetchdriver *fd)
{
   long lock;
   void *fh;
   struct FileInfoBlock *fib;
   UBYTE *name,*buf=NULL,*p;
   UBYTE c;
   BOOL skipvalid=FALSE;
   long actual;
   ULONG date=0;


   if(fd->postmsg)
   {  Write(Output(),"\nPOST:\n",7);
      Write(Output(),fd->postmsg,strlen(fd->postmsg));
      Write(Output(),"\n",1);
   }
   if(fd->multipart)
   {  struct Multipartpart *mpp;
      Write(Output(),"\nPOST multipart/form-data:\n",27);
      for(mpp=fd->multipart->parts.first;mpp->next;mpp=mpp->next)
      {  if(mpp->lock)
         {  long fh;
            long r;
            if(fh=OpenFromLock(mpp->lock))
            {  mpp->lock=0;
               while(r=Read(fh,fd->block,fd->blocksize))
               {  Write(Output(),fd->block,r);
               }
               Close(fh);
            }
         }
         else
         {  Write(Output(),fd->multipart->buf.buffer+mpp->start,mpp->length);
         }
      }
   }
   name=fd->name;
   c=fd->name[strlen(fd->name)-1];
   if(c=='/' || c==':')
   {  ObtainSemaphore(&prefssema);
      if(buf=ALLOCTYPE(UBYTE,strlen(fd->name)+strlen(prefs.network.localindex)+2,0))
      {  strcpy(buf,fd->name);
         strcat(buf,prefs.network.localindex);
         name=buf;
      }
      ReleaseSemaphore(&prefssema);
   }
   lock=Lock(name,SHARED_LOCK);
   /* Replace MS-DOS \ by / */
   if(!lock && strchr(name,'\\'))
   {  if(buf || (buf=Dupstr(name,-1)))
      {  name=buf;
         while(p=strchr(name,'\\')) *p='/';
         lock=Lock(name,SHARED_LOCK);
      }
   }
   if(lock)
   {  if(fib=AllocDosObjectTags(DOS_FIB,TAG_END))
      {  if(Examine(lock,fib))
         {  if(fib->fib_DirEntryType<0)
            {  Updatetaskattrs(AOURL_Contentlength,fib->fib_Size,TAG_END);
            }
            date=fib->fib_Date.ds_Days*86400 +
                 fib->fib_Date.ds_Minute*60 +
                 fib->fib_Date.ds_Tick/TICKS_PER_SECOND;
            if(fd->validate && date<=fd->validate) skipvalid=TRUE;
         }
         FreeDosObject(DOS_FIB,fib);
      }
      if(!date) date=Today();
      if(skipvalid)
      {  if(!(fd->flags&FDVF_CACHERELOAD))
         {  Updatetaskattrs(AOURL_Notmodified,TRUE,TAG_END);
         }
      }
      else
      {  if(!(fd->flags&FDVF_CACHERELOAD))
         {  Updatetaskattrs(AOURL_Lastmodified,date,TAG_END);
         }
         if(fh=OpenAsync(name,MODE_READ,INPUTBLOCKSIZE))
         {  for(;;)
            {
#ifdef DEVELOPER
               actual=ReadAsync(fh,fd->block,MIN(localblocksize,fd->blocksize));
#else
               actual=ReadAsync(fh,fd->block,fd->blocksize);
#endif
               if(Checktaskbreak()) break;
               Updatetaskattrs(
                  AOURL_Data,fd->block,
                  AOURL_Datalength,actual,
                  TAG_END);
               if(!actual) break;
            }
            CloseAsync(fh);
         }
         else
         {  Tcperror(fd,TCPERR_NOFILE,name);
         }
      }
      UnLock(lock);
   }
   else
   {  Tcperror(fd,TCPERR_NOFILE,name);
   }
   if(buf) FREE(buf);
   Updatetaskattrs(AOTSK_Async,TRUE,
      AOURL_Eof,TRUE,
      AOURL_Terminate,TRUE,
      TAG_END);
}

/*-----------------------------------------------------------------------*/
