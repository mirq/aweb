/* Automatically generated header! Do not edit! */

#ifndef CLIB_LIBRARYMANAGER_PROTOS_H
#define CLIB_LIBRARYMANAGER_PROTOS_H

/*
**   $VER: librarymanager_protos.h 1.3 (28.01.2005)
**
**   C prototypes. For use with 32 bit integers only.
**
**   Copyright (C) 2001 Amiga, Inc.
**       All Rights Reserved
*/

#include <exec/libraries.h>
#include <dos/dos.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct Library * LibOpen(ULONG version);
BPTR LibClose(void);
BPTR LibExpunge(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIB_LIBRARYMANAGER_PROTOS_H */
