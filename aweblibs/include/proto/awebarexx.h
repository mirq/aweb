#ifndef PROTO_AWEBAREXX_H
#define PROTO_AWEBAREXX_H

/*
**   $Id: awebarexx.h,v 1.3 2009/06/15 17:05:07 opi Exp $
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
  extern struct Library * AwebArexxBase;
 #else
  extern struct Library * AwebArexxBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebarexx.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebarexx.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBAREXX_PROTOS_H
  #define CLIB_AWEBAREXX_PROTOS_H 1
 #endif /* CLIB_AWEBAREXX_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AwebArexxIFace *IAwebArexx;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/awebarexx.h>
#else
 #ifndef CLIB_AWEBAREXX_PROTOS_H
  #include <clib/awebarexx_protos.h>
 #endif /* CLIB_AWEBAREXX_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebarexx.h>
  #else
   #include <ppcinline/awebarexx.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebarexx_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebarexx_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBAREXX_H */
