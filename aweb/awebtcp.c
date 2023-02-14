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

/* awebtcp.c - AWeb tcp and ssl switch engine */

#include "aweb.h"
#include "proto/awebtcp.h"
#include "proto/awebssl.h"

struct Library *AwebAmiTcpBase;

#ifdef SUPPORT_INET
extern struct Library *AwebInet225Base;
#endif

struct Library *AwebAmiSslBase;
extern struct Library *AwebMiamisslBase;

struct Library *AwebTcpBase;
struct Library *AwebSslBase;

struct Library *AwebAmiTcpBase;

struct Library *SocketBase = NULL;

#if defined(__amigaos4__)

struct Interface    * IAwebAmiTcp;
struct Interface    * IAwebAmiSsl;
struct AwebTcpIFace * IAwebTcp;
struct AwebSslIFace * IAwebSsl;

#endif

/*-----------------------------------------------------------------------*/
/*--- Init and cleanup tcp libries --------------------------------------*/
/*-----------------------------------------------------------------------*/

BOOL Initawebtcp()
{
    Initawebamitcp();
    Initawebamissl();

    /*
     * add int funcs for any other tcp libs here
     * should only be Inet but you never know what might crop up
     * NB Inet is not supported if someone wants to readd Inet they
     * will need to add the init and free funcs for it.
     *
     */

    /* if we're building for OS 4 we must open AwebAmiTcp properly */
    /* OS3 version cheats */

#if defined(__amigaos4__)

     AwebAmiTcpBase = OpenLibrary("awebamitcp.library",0);
     IAwebAmiTcp = GetInterface(AwebAmiTcpBase,"main",1,0);
     AwebAmiSslBase = OpenLibrary("awebamissl.library",0);
     IAwebAmiSsl = GetInterface(AwebAmiSslBase,"main",1,0);
#endif

     if(!AwebAmiTcpBase)return FALSE;

     return TRUE;
}

void Freeawebtcp()
{

#if defined(__amigaos4__)

    if(IAwebAmiSsl)DropInterface(IAwebAmiSsl);
    if(AwebAmiSslBase)CloseLibrary(AwebAmiSslBase);
    if(IAwebAmiTcp)DropInterface(IAwebAmiTcp);
    if(AwebAmiTcpBase)CloseLibrary(AwebAmiTcpBase);

#endif

    Freeawebamissl();
    Freeawebamitcp();

}

/*-----------------------------------------------------------------------*/

/* For os4 Tcpopenlib returns a pointer to the bsdscket Interface for Os3 the library base */

void *Tcpopenlib(void)
{  struct Library *base=NULL;
   if(base=OpenLibrary("bsdsocket.library",0))
   {
      AwebTcpBase=AwebAmiTcpBase;
      SocketBase = AwebTcpBase;
#if defined(__amigaos4__)

      IAwebTcp = (struct AwebTcpIface *)IAwebAmiTcp;

#endif
      a_setup(base);
   }

#ifdef SUPPORT_INET
   else if(base=OpenLibrary("inet:libs/socket.library",4))
   {  AwebTcpBase=AwebInet225Base;
      a_setup(base);
   }
#endif

#if defined(__amigaos4__)
   {
    struct Interface *socketiface;
    socketiface = GetInterface(base,"main",1,0);
    return socketiface;
   }
#else

   return base;
#endif
}

extern struct Assl *Assl_initamissl(struct Library *socketbase);

struct Assl *Tcpopenssl(struct Library *socketbase)
{  struct Assl *assl=NULL;

   if(assl=Assl_initamissl(socketbase))
   {

      AwebSslBase=AwebAmiSslBase;
#if defined(__amigaos4__)

      IAwebSsl = (struct AwebSslIface *)IAwebAmiSsl;

#endif

   }


   return assl;
}
