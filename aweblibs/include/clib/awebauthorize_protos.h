/* Automatically generated header! Do not edit! */

#ifndef CLIB_AWEBAUTHORIZE_PROTOS_H
#define CLIB_AWEBAUTHORIZE_PROTOS_H

/*
**   $VER: awebauthorize_protos.h 1.1 (28.01.2005)
**
**   C prototypes. For use with 32 bit integers only.
**
**   Copyright (C) 2001 Amiga, Inc.
**       All Rights Reserved
*/

#include <exec/types.h>
#include <libraries/awebauthorize.h>
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Subtaskfunction * AuthGetTaskFunc(ULONG id);
void Authorreq(struct Authorreq *areq, BOOL onlypw);
void Authorset(struct Authorize *auth, UBYTE *userid, UBYTE *passwd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIB_AWEBAUTHORIZE_PROTOS_H */
