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

/* libraries.c open and close libraries */

#include "aweb.h"
#include <exec/exec.h>
#include <intuition/intuition.h>

#include <proto/exec.h>
#include <proto/intuition.h>

VARARGS68K_DECLARE(void Lowlevelreq(STRPTR msg,...));
void Cleanup(void);

#if !defined(__amigaos4__)
struct Interface;
#endif

struct Library *Openlib(STRPTR name,long version, struct Library **base, struct Interface **iface)
{
   *base=OpenLibrary(name,version);
   if(!*base)
   { 
      STRPTR msg1 = AWEBSTR(MSG_ERROR_CANTOPENV);
      STRPTR msg2 = "Can't open %s version %ld";
      Lowlevelreq((strcmp(name,"locale.library"))?msg1:msg2,name,version);
      Cleanup();
      exit(10);
   }
#if defined(__amigaos4__)
   *iface = GetInterface(*base,"main",1,0);
#endif
   return *base;
}

struct Library *Openlibnofail(STRPTR name,long version, struct Library **base, struct Interface **iface)
{
   *base=OpenLibrary(name,version);
#if defined(__amigaos4__)
   if(*base)
   {
       *iface = GetInterface(*base,"main",1,0);
   }
#endif
   return *base;
}


struct Library *Openclass(STRPTR name,long version, struct Library **base, struct Interface **iface)
{
   if(!(*base=OpenLibrary(name,version)))
   { 
      STRPTR msg=AWEBSTR(MSG_ERROR_CANTOPENV);
      Lowlevelreq(msg,name,version);
      Cleanup();
      exit(10);
   }
#if defined(__amigaos4__)
   *iface = GetInterface(*base,"main",1,0);
#endif
   return *base;
}

void Closelib(struct Library **base, struct Interface **iface)
{
#if defined(__amigaos4__)
    if(*iface)DropInterface(*iface);
    *iface = NULL;
#endif
    if(*base)CloseLibrary(*base);
    *base = NULL;
}


struct MsgPort *ACreatemsgport()
{
#if !defined(__amigaos4__)
    return CreateMsgPort();
#else
    return AllocSysObjectTags(ASOT_PORT,TAG_DONE);
#endif     
}

VOID ADeletemsgport(struct MsgPort *port)
{
#if !defined(__amigaos4__)
    return DeleteMsgPort(port);
#else
    return FreeSysObject(ASOT_PORT,port);
#endif     
}

BPTR ASetcurrentdir( BPTR lock)
{
#if defined(__amigaos4__)
	return SetCurrentDir(lock);
#else
	return CurrentDir(lock);
#endif
}

VOID ADeletefile( STRPTR filename)
{
#if defined(__amigaos4__)
	return Delete(filename);
#else
	return DeleteFile(filename);
#endif
}
