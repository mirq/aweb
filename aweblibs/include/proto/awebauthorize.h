#ifndef PROTO_AWEBAUTHORIZE_H
#define PROTO_AWEBAUTHORIZE_H

/*
**   $Id: awebauthorize.h,v 1.3 2009/06/15 17:05:07 opi Exp $
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

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * AwebAuthorizeBase;
 #else
  extern struct Library * AwebAuthorizeBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebauthorize.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebauthorize.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBAUTHORIZE_PROTOS_H
  #define CLIB_AWEBAUTHORIZE_PROTOS_H 1
 #endif /* CLIB_AWEBAUTHORIZE_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AwebAuthorizeIFace *IAwebAuthorize;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/awebauthorize.h>
#else
 #ifndef CLIB_AWEBAUTHORIZE_PROTOS_H
  #include <clib/awebauthorize_protos.h>
 #endif /* CLIB_AWEBAUTHORIZE_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebauthorize.h>
  #else
   #include <ppcinline/awebauthorize.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebauthorize_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebauthorize_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBAUTHORIZE_H */
