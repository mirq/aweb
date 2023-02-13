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

#include "aweb.h"

#include <proto/exec.h>

#if !defined(__amigaos4__)
static struct SignalSemaphore memsema;
#endif
#if defined(__amigaos4__)
static APTR globalpool = NULL;
#else
static void *fastpool;
#endif

#ifdef BETAKEYFILE
extern BOOL nopool;
#endif

#if defined(__amigaos4__) || defined(__MORPHOS__)
#define FAST_FLAGS MEMF_CLEAR
#define CHIP_FLAGS MEMF_CLEAR
#else
#define FAST_FLAGS MEMF_PUBLIC|MEMF_CLEAR
#define CHIP_FLAGS MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR
static void *chippool;
#endif

#if defined(__amigaos4__)

BOOL Initmemory(void)
{
	if(!(globalpool = AllocSysObjectTags(ASOT_MEMPOOL, ASOPOOL_Puddle, PUDDLESIZE, ASOPOOL_Threshold, TRESHSIZE, ASOPOOL_Protected, TRUE, ASOPOOL_MFlags,MEMF_CLEAR,TAG_DONE)))
	{
		return FALSE;
	}
	
	return TRUE;
}

void Freememory(void)
{
	if(globalpool)
	{
		FreeSysObject(ASOT_MEMPOOL,globalpool);
	}
}

void *Pallocmem(long size,ULONG flags,void *pool)
{
   void *mem;
#ifdef BETAKEYFILE
   if(nopool) pool=NULL;
#endif

   if(pool) mem=AllocVecPooled(pool,size+4);
   else mem=AllocVecTags(size+4,AVT_Type, MEMF_PRIVATE, AVT_ClearWithValue,0,TAG_DONE);
   if(mem)
   {  
      *(void **)mem=pool;
      return (void *)((ULONG)mem+4);
   }
   else return NULL;
}

void *Allocmem(long size,ULONG flags)
{  

   void *mem;

   mem=Pallocmem(size,flags,globalpool);

   return mem;
}

void Freemem(void *mem)
{  void *pool;

   if(mem)
   {
   	  pool=*(void **)((ULONG)mem-4);

      if(pool) FreeVecPooled(pool,(void *)((ULONG)mem-4));
      else FreeVec((void *)((ULONG)mem-4));

   }
}

#else

/*-----------------------------------------------------------------------*/

BOOL Initmemory(void)
{  InitSemaphore(&memsema);
   if(!(fastpool=CreatePool(FAST_FLAGS,PUDDLESIZE,TRESHSIZE)))
      return FALSE;
#if !defined(__amigaos4__) && !defined(__MORPHOS__)
   if(!(chippool=CreatePool(CHIP_FLAGS,PUDDLESIZE,TRESHSIZE)))
      return FALSE;
#endif
   return TRUE;
}

void Freememory(void)
{
#if !defined(__amigaos4__) && !defined(__MORPHOS__)
   if(chippool) DeletePool(chippool);
#endif
   if(fastpool) DeletePool(fastpool);
}

void *Pallocmem(long size,ULONG flags,void *pool)
{  void *mem;
#ifdef BETAKEYFILE
   if(nopool) pool=NULL;
#endif
   ObtainSemaphore(&memsema);
   if(pool) mem=AllocPooled(pool,size+8);
   else mem=AllocMem(size+8,flags|FAST_FLAGS);
   ReleaseSemaphore(&memsema);
   if(mem)
   {  *(void **)mem=pool;
      *(long *)((ULONG)mem+4)=size+8;
      return (void *)((ULONG)mem+8);
   }
   else return NULL;
}

void *Allocmem(long size,ULONG flags)
{  void *pool,*mem;
   ObtainSemaphore(&memsema);
#if !defined(__amigaos4__) && !defined(__MORPHOS__)
   if(flags&MEMF_CHIP) pool=chippool;
   else
#endif
   pool=fastpool;
   mem=Pallocmem(size,flags,pool);
   ReleaseSemaphore(&memsema);
   return mem;
}

void Freemem(void *mem)
{  void *pool;
   long size;
   if(mem)
   {  pool=*(void **)((ULONG)mem-8);
      size=*(long *)((ULONG)mem-4);
      ObtainSemaphore(&memsema);
      if(pool) FreePooled(pool,(void *)((ULONG)mem-8),size);
      else FreeMem((void *)((ULONG)mem-8),size);
      ReleaseSemaphore(&memsema);
   }
}

#endif
