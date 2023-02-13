#ifndef _INCLUDE_PRAGMA_AWEBSSL_LIB_H
#define _INCLUDE_PRAGMA_AWEBSSL_LIB_H

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

   $Id: awebssl_lib.h,v 1.2 2009/06/15 17:04:59 opi Exp $

   Desc:

***********************************************************************/


#ifndef CLIB_AWEBSSL_PROTOS_H
#include <clib/awebssl_protos.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(AwebSslBase,0x01e,Assl_cleanup(a0))
#pragma amicall(AwebSslBase,0x024,Assl_openssl(a0))
#pragma amicall(AwebSslBase,0x02a,Assl_closessl(a0))
#pragma amicall(AwebSslBase,0x030,Assl_connect(a0,d0,a1))
#pragma amicall(AwebSslBase,0x036,Assl_geterror(a0,a1))
#pragma amicall(AwebSslBase,0x03c,Assl_write(a0,a1,d0))
#pragma amicall(AwebSslBase,0x042,Assl_read(a0,a1,d0))
#pragma amicall(AwebSslBase,0x048,Assl_getcipher(a0))
#pragma amicall(AwebSslBase,0x04e,Assl_libname(a0))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall AwebSslBase Assl_cleanup           01e 801
#pragma  libcall AwebSslBase Assl_openssl           024 801
#pragma  libcall AwebSslBase Assl_closessl          02a 801
#pragma  libcall AwebSslBase Assl_connect           030 90803
#pragma  libcall AwebSslBase Assl_geterror          036 9802
#pragma  libcall AwebSslBase Assl_write             03c 09803
#pragma  libcall AwebSslBase Assl_read              042 09803
#pragma  libcall AwebSslBase Assl_getcipher         048 801
#pragma  libcall AwebSslBase Assl_libname           04e 801
#endif

#ifdef __cplusplus
}
#endif

#endif   /*  _INCLUDE_PRAGMA_AWEBSSL_LIB_H  */
