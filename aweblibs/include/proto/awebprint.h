#ifndef PROTO_AWEBPRINT_H
#define PROTO_AWEBPRINT_H

/*
**   $Id: awebprint.h,v 1.5 2009/06/15 17:05:08 opi Exp $
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
  extern struct Library * AwebPrintBase;
 #else
  extern struct Library * AwebPrintBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebprint.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebprint.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBPRINT_PROTOS_H
  #define CLIB_AWEBPRINT_PROTOS_H 1
 #endif /* CLIB_AWEBPRINT_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AwebPrintIFace *IAwebPrint;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/awebprint.h>
#else
 #ifndef CLIB_AWEBPRINT_PROTOS_H
  #include <clib/awebprint_protos.h>
 #endif /* CLIB_AWEBPRINT_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebprint.h>
  #else
   #include <ppcinline/awebprint.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebprint_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebprint_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBPRINT_H */
