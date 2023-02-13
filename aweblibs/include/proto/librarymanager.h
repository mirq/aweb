#ifndef PROTO_LIBRARYMANAGER_H
#define PROTO_LIBRARYMANAGER_H

/*
**   $Id: librarymanager.h,v 1.2 2009/06/15 17:05:08 opi Exp $
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
  extern struct Library * LibraryManagerBase;
 #else
  extern struct Library * LibraryManagerBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/librarymanager.h>
 #ifdef __USE_INLINE__
  #include <inline4/librarymanager.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_LIBRARYMANAGER_PROTOS_H
  #define CLIB_LIBRARYMANAGER_PROTOS_H 1
 #endif /* CLIB_LIBRARYMANAGER_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct LibraryManagerIFace *ILibraryManager;
 #endif /* __NOGLOBALIFACE__ */
#elif defined __AROS__
 #include <defines/librarymanager.h>
#else
 #ifndef CLIB_LIBRARYMANAGER_PROTOS_H
  #include <clib/librarymanager_protos.h>
 #endif /* CLIB_LIBRARYMANAGER_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/librarymanager.h>
  #else
   #include <ppcinline/librarymanager.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/librarymanager_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/librarymanager_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_LIBRARYMANAGER_H */
