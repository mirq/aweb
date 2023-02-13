#ifndef PROTO_AWEBSUPPORT_H
#define PROTO_AWEBSUPPORT_H

/*
**   $Id: awebsupport.h,v 1.2 2009/06/15 17:05:45 opi Exp $
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
  extern struct Library * AwebSupportBase;
 #else
  extern struct Library * AwebSupportBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebsupport.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebsupport.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBSUPPORT_PROTOS_H
  #define CLIB_AWEBSUPPORT_PROTOS_H 1
 #endif /* CLIB_AWEBSUPPORT_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AwebSupportIFace *IAwebSupport;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_AWEBSUPPORT_PROTOS_H
  #include <clib/awebsupport_protos.h>
 #endif /* CLIB_AWEBSUPPORT_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebsupport.h>
  #else
   #include <ppcinline/awebsupport.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebsupport_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebsupport_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBSUPPORT_H */
