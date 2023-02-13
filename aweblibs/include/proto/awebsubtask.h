#ifndef PROTO_AWEBSUBTASK_H
#define PROTO_AWEBSUBTASK_H

/*
**   $Id: awebsubtask.h,v 1.2 2009/06/15 17:05:08 opi Exp $
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
  extern struct Library * AwebSubtaskBase;
 #else
  extern struct Library * AwebSubtaskBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebsubtask.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebsubtask.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBSUBTASK_PROTOS_H
  #define CLIB_AWEBSUBTASK_PROTOS_H 1
 #endif /* CLIB_AWEBSUBTASK_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AwebSubtaskIFace *IAwebSubtask;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/awebsubtask.h>
#else
 #ifndef CLIB_AWEBSUBTASK_PROTOS_H
  #include <clib/awebsubtask_protos.h>
 #endif /* CLIB_AWEBSUBTASK_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebsubtask.h>
  #else
   #include <ppcinline/awebsubtask.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebsubtask_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebsubtask_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBSUBTASK_H */
