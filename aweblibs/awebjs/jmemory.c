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

/* memory.c AWeb memory manager */

#include "awebjs.h"
#include "libraries/awebclib.h"

#include <utility/hooks.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>


/*-----------------------------------------------------------------------*/

/*
typedef char (* __asm SegTrack(register __a0 ULONG Address,
   register __a1 ULONG *SegNum,register __a2 ULONG *Offset));

struct SegSem
{  struct SignalSemaphore seg_Semaphore;
   SegTrack *seg_Find;
};

void Debugmem(UBYTE *f,void *mem,long size,void *stack)
{  UBYTE *segname;
   ULONG segnum,offset;
   ULONG *p;
   short n;
   struct SegSem *segsem;
   KPrintF("%s %08lx %6ld        ",f,mem,size);
   if(segsem=(struct SegSem *)FindSemaphore("SegTracker"))
   {  for(p=(ULONG *)stack,n=0;n<6;p++)
      {  if((segname=segsem->seg_Find(*p,&segnum,&offset)) && segnum==0)
         {  KPrintF("%ld:%08lx ",segnum,offset);
            n++;
         }
      }
   }
   KPrintF("\n");
}
*/



#if defined(__amigaos4__)
// OS4 segment tracking code
/*
struct TrackResult
{
    STRPTR name;
    uint32 segnum;
    uint32 offset;
};

BOOL trackhook(struct Hook *hook, APTR address, struct FindTrackedAddressMsg *ftam)
{
    ((struct TrackResult *)hook->h_Data)->name=ftam->ftam_Name;
    ((struct TrackResult *)hook->h_Data)->segnum=ftam->ftam_SegmentNumber;
    ((struct TrackResult *)hook->h_Data)->offset=ftam->ftam_SegmentOffset;
    return FALSE;
}

static BOOL track = FALSE;
extern APTR libseglist;

void Debugmem(UBYTE *f,void *pool,void *mem,long size,void *stack)
{
    struct Hook hook = {0};
    struct TrackResult tr ={0};
    uint32 *p;
    uint32 n;
    uint32 q;

    hook.h_Entry=(HOOKFUNC)trackhook;
    hook.h_Data=(APTR)&tr;
    if(size == sizeof(struct Jobject))
    {
        adebug("%s pool %08lx addr %08lx size %6ld\n",f,pool,mem,size);


        for(p=(uint32 *)stack,n=0,q=0;n<60 && q<6;p++,n++)
        {
            FindTrackedAddress(*p,&hook);
            if(tr.offset>0){
                adebug("%08lx %08lx %s %ld:%08lx\n",p,*p,tr.name,tr.segnum,tr.offset);
                q++;
            }
            tr.offset = 0;
            tr.name= NULL;
            tr.segnum =0;
        }
    }
}

*/

void Dumpjobject(struct Jobject *jo)
{
    adebug("\nDump Jobject at %08lx\n",jo);
    if(jo)
    {
        adebug("\t%08lx->next\t\t\t %08lx\n",&jo->next,jo->next);
        adebug("\t%08lx->prev\t\t\t %08lx\n",&jo->prev,jo->prev);
        adebug("\t%08lx->constructor\t\t %08lx\n",&jo->constructor,jo->constructor);
        adebug("\t%08lx->properties.first\t %08lx\n",&jo->properties.first,jo->properties.first);
        adebug("\t%08lx->properties.tail\t %08lx\n",&jo->properties.tail,jo->properties.tail);
        adebug("\t%08lx->properties.last\t %08lx\n",&jo->properties.last,jo->properties.last);
        adebug("\t%08lx->function\t\t %08lx\n",&jo->function,jo->function);
        adebug("\t%08lx->hook\t\t\t %08lx\n",&jo->hook,jo->hook);
        adebug("\t%08lx->dispose\t\t %08lx\n",&jo->dispose,jo->dispose);
        adebug("\t%08lx->flags\t\t\t %08lx\n",&jo->flags,(uint32)jo->flags);
        adebug("\t%08lx->dumpnr\t\t %08ld\n",&jo->dumpnr,(uint32)jo->dumpnr);
        adebug("\t%08lx->keepnr\t\t %08ld\n",&jo->keepnr,jo->keepnr);
        adebug("\t%08lx->notdisposed\t\t %08lx\n",&jo->notdisposed,(int)jo->notdisposed);
        adebug("\t%08lx->var\t\t %08ld\n",&jo->var,(int)jo->var);
        adebug("\n");
    }
    else
    {
        adebug("\tObject pointer was Null!\n\n");
    }
}

#endif



/*-----------------------------------------------------------------------*/

void *Pallocmem(long size,ULONG flags,void *pool)
{  void *mem;
   if(pool) mem=AllocPooled(pool,size+8);
   else mem=AllocMem(size+8,flags|MEMF_ANY|MEMF_CLEAR);
   //Debugmem("ALLOC",pool,mem,size,&pool);
   if(mem)
   {  *(void **)mem=pool;
      *(long *)((ULONG)mem+4)=size+8;
      return (void *)((ULONG)mem+8);
   }
   else return NULL;
}

void Freemem(void *mem)
{  void *pool;
   long size;
   if(mem)
   {  pool=*(void **)((ULONG)mem-8);
      size=*(long *)((ULONG)mem-4);
   //Debugmem("FREE ",pool,(void *)((ULONG)mem-8),(size-8),&mem);
      if(pool) FreePooled(pool,(void *)((ULONG)mem-8),size);
      else FreeMem((void *)((ULONG)mem-8),size);
   }
}

void *Getpool(void *p)
{  return *(void **)((ULONG)p-8);
}
