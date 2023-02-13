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

/* clip.c - AWeb clipboard interface */

#include "aweb.h"
#include <libraries/iffparse.h>
#include <devices/clipboard.h>
#include <proto/iffparse.h>

#define ID_FTXT   MAKE_ID('F','T','X','T')
#define ID_CHRS   MAKE_ID('C','H','R','S')

/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/

void Clipcopy(UBYTE *text,long length)
{
   struct IFFHandle *iff;
   UBYTE *p;
   for(p=text;p<text+length;p++)
   {  if(*p==0xa0) *p=' ';
   }
   if(iff=AllocIFF())
   {  if(iff->iff_Stream=(ULONG)OpenClipboard(PRIMARY_CLIP))
      {  InitIFFasClip(iff);
         if(!OpenIFF(iff,IFFF_WRITE))
         {  if(PushChunk(iff,ID_FTXT,ID_FORM,IFFSIZE_UNKNOWN)) goto err;
            if(PushChunk(iff,ID_FTXT,ID_CHRS,length)) goto err;
            if(WriteChunkBytes(iff,text,length)!=length) goto err;
            if(PopChunk(iff)) goto err;
            if(PopChunk(iff)) goto err;

err:
            CloseIFF(iff);
         }
         CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
      }
      FreeIFF(iff);
   }
}

long Clippaste(UBYTE *buf,long length)
{  struct IFFHandle *iff;
   long l=-1;
   LONG e;
   struct ContextNode *cn;
   long left=length;
   if(iff=AllocIFF())
   {  if(iff->iff_Stream=(ULONG)OpenClipboard(PRIMARY_CLIP))
      {  InitIFFasClip(iff);
         if(!OpenIFF(iff,IFFF_READ))
         {  if(StopChunk(iff,ID_FTXT,ID_CHRS)) goto err;
            while(left>0)
            {  e=ParseIFF(iff,IFFPARSE_SCAN);
               if(e==IFFERR_EOC) continue;
               if(e) break;
               cn=CurrentChunk(iff);
               if(cn && cn->cn_Type==ID_FTXT && cn->cn_ID==ID_CHRS)
               {  l=ReadChunkBytes(iff,buf+length-left,left);
                  if(l<0) goto err;
                  left-=l;
               }
            }
            l=length-left;
err:
            CloseIFF(iff);
         }
         CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
      }
      FreeIFF(iff);
   }
   return l;
}
