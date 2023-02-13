#ifndef PROTO_AWEBPLUGIN_H
#define PROTO_AWEBPLUGIN_H

/*
**   $Id: awebplugin.h,v 1.6 2009/06/15 17:05:08 opi Exp $
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
  extern struct Library * AwebPluginBase;
 #else
  extern struct Library * AwebPluginBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/awebplugin.h>
 #ifdef __USE_INLINE__
  #include <inline4/awebplugin.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_AWEBPLUGIN_PROTOS_H
  #define CLIB_AWEBPLUGIN_PROTOS_H 1
 #endif /* CLIB_AWEBPLUGIN_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct AwebPluginIFace *IAwebPlugin;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/awebplugin.h>
#else
 #ifndef CLIB_AWEBPLUGIN_PROTOS_H
  #include <clib/awebplugin_protos.h>
 #endif /* CLIB_AWEBPLUGIN_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/awebplugin.h>
  #else
   #include <ppcinline/awebplugin.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/awebplugin_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/awebplugin_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_AWEBPLUGIN_H */
