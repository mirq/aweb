#ifndef PROTO_AWEBJS_H
#define PROTO_AWEBJS_H

/*
**      $Id: awebjs.h,v 1.4 2005/01/25 19:09:29 falemagn Exp $
**      Includes Release 50.1
**
**      Prototype/inline/pragma header file combo
**
**      (C) Copyright 2003-2004 Amiga, Inc.
**          All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef LIBRARIES_AWEBJS_H
#include <libraries/awebjs.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * AWebJSBase;
 #else
  extern struct AWebJSBase * AWebJSBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebjs.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebjs.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBJS_PROTOS_H
  #define CLIB_AWEBJS_PROTOS_H 1
 #endif /* CLIB_AWEBJS_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AWebJSIFace *IAWebJS;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/awebjs.h>
#else
 #ifndef CLIB_AWEBJS_PROTOS_H
  #include <clib/awebjs_protos.h>
 #endif /* CLIB_AWEBJS_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebjs.h>
  #else
   #include <ppcinline/awebjs.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebjs_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebjs_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBJS_H */
