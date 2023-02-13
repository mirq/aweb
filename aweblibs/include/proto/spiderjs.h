#ifndef PROTO_SPIDERJS_H
#define PROTO_SPIDERJS_H

/*
**	$Id$
**	Includes Release 50.1
**
**	Prototype/inline/pragma header file combo
**
**	(C) Copyright 2003-2005 Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef LIBRARIES_SPIDER_H
#include <libraries/spider.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * SpiderJSBase;
 #else
  extern struct SpiderJSBase * SpiderJSBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/spiderjs.h>
 #ifdef __USE_INLINE__
  #include <inline4/spiderjs.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_SPIDERJS_PROTOS_H
  #define CLIB_SPIDERJS_PROTOS_H 1
 #endif /* CLIB_SPIDERJS_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct SpiderJSIFace *ISpiderJS;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_SPIDERJS_PROTOS_H
  #include <clib/spiderjs_protos.h>
 #endif /* CLIB_SPIDERJS_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/spiderjs.h>
  #else
   #include <ppcinline/spiderjs.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/spiderjs_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/spiderjs_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_SPIDERJS_H */
