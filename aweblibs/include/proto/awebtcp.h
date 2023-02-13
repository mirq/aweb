#ifndef PROTO_AWEBTCP_H
#define PROTO_AWEBTCP_H

/*
**   $Id: awebtcp.h,v 1.5 2009/06/15 17:05:08 opi Exp $
**   Includes Release 50.1
**
**   Prototype/inline/pragma header file combo
**
**   (C) Copyright 2003-2004 Amiga, Inc.
**       All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef SYS_SOCKET_H
#include <sys/socket.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * AwebTcpBase;
 #else
  extern struct Library * AwebTcpBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebtcp.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebtcp.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBTCP_PROTOS_H
  #define CLIB_AWEBTCP_PROTOS_H 1
 #endif /* CLIB_AWEBTCP_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AwebTcpIFace *IAwebTcp;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/awebtcp.h>
#else
 #ifndef CLIB_AWEBTCP_PROTOS_H
  #include <clib/awebtcp_protos.h>
 #endif /* CLIB_AWEBTCP_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebtcp.h>
  #else
   #include <ppcinline/awebtcp.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebtcp_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebtcp_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBTCP_H */
