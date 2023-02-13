#ifndef PROTO_AWEBSSL_H
#define PROTO_AWEBSSL_H

/*
**   $Id: awebssl.h,v 1.5 2009/06/15 17:05:08 opi Exp $
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
#ifndef LIBRARIES_AWEBSSL_H
#include <libraries/awebssl.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * AwebSslBase;
 #else
  extern struct Library * AwebSslBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebssl.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebssl.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBSSL_PROTOS_H
  #define CLIB_AWEBSSL_PROTOS_H 1
 #endif /* CLIB_AWEBSSL_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AwebSslIFace *IAwebSsl;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/awebssl.h>
#else
 #ifndef CLIB_AWEBSSL_PROTOS_H
  #include <clib/awebssl_protos.h>
 #endif /* CLIB_AWEBSSL_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebssl.h>
  #else
   #include <ppcinline/awebssl.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebssl_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebssl_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBSSL_H */
