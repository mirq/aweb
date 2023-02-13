#ifndef PROTO_AWEBSTARTUP_H
#define PROTO_AWEBSTARTUP_H

/*
**   $Id: awebstartup.h,v 1.4 2009/06/15 17:05:08 opi Exp $
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
#ifndef LIBRARIES_AWEBSTARTUP_H
#include <libraries/awebstartup.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * AwebStartupBase;
 #else
  extern struct Library * AwebStartupBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebstartup.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebstartup.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBSTARTUP_PROTOS_H
  #define CLIB_AWEBSTARTUP_PROTOS_H 1
 #endif /* CLIB_AWEBSTARTUP_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AwebStartupIFace *IAwebStartup;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/awebstartup.h>
#else
 #ifndef CLIB_AWEBSTARTUP_PROTOS_H
  #include <clib/awebstartup_protos.h>
 #endif /* CLIB_AWEBSTARTUP_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebstartup.h>
  #else
   #include <ppcinline/awebstartup.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebstartup_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebstartup_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBSTARTUP_H */
