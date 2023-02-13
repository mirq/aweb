#ifndef CLIB_AWEBSSL_PROTOS_H
#define CLIB_AWEBSSL_PROTOS_H


/**********************************************************************

   This file is part of the AWeb-II distribution

   Copyright (C) 2002 Yvon Rozijn
   Copyright (C) 2002-2003 The AWeb Developement Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the AWeb Public License as included in this
   distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   AWeb Public License for more details.

   $Id: awebssl_protos.h,v 1.4 2009/06/15 17:04:08 opi Exp $

   Desc:

***********************************************************************/

#include <exec/types.h>
#include <libraries/awebssl.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Assl;

void  Assl_cleanup(struct Assl * assl);
BOOL  Assl_openssl(struct Assl * assl);
void  Assl_closessl(struct Assl * assl);
long  Assl_connect(struct Assl * assl, long sock, UBYTE * hostname);
char *Assl_geterror(struct Assl * assl, char * errbuf);
long  Assl_write(struct Assl * assl, char * buffer, long length);
long  Assl_read(struct Assl * assl, char * buffer, long length);
char *Assl_getcipher(struct Assl * assl);
char * Assl_libname(struct Assl * assl);

#ifdef __cplusplus
}
#endif

#endif   /*  CLIB_AWEBSSL_PROTOS_H  */
