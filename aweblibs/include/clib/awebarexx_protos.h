/* Automatically generated header! Do not edit! */

#ifndef CLIB_AWEBAREXX_PROTOS_H
#define CLIB_AWEBAREXX_PROTOS_H

/*
**   $VER: arexx_protos.h 1.3 (28.01.2005)
**
**   C prototypes. For use with 32 bit integers only.
**
**   Copyright (C) 2001 Amiga, Inc.
**       All Rights Reserved
*/

#include <exec/types.h>
#include <libraries/awebarexx.h>
#include "awebprefs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void AwebArexxGetCfg(struct Arexxcmd *ac, struct AwebPrefs *prefs);
void AwebArexxSetCfg(struct Arexxcmd *ac, struct AwebPrefs *prefs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIB_AREXX_PROTOS_H */
