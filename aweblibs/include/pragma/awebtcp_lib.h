#ifndef _INCLUDE_PRAGMA_AWEBTCP_LIB_H
#define _INCLUDE_PRAGMA_AWEBTCP_LIB_H

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

   $Id: awebtcp_lib.h,v 1.3 2009/06/15 17:05:00 opi Exp $

   Desc:

***********************************************************************/


#ifndef CLIB_AWEBTCP_PROTOS_H
#include <clib/awebtcp_protos.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(AwebTcpBase,0x01e,a_setup(a0))
#pragma amicall(AwebTcpBase,0x024,a_cleanup(a0))
#pragma amicall(AwebTcpBase,0x02a,a_gethostbyname(a0,a1))
#pragma amicall(AwebTcpBase,0x030,a_socket(d0,d1,d2,a0))
#pragma amicall(AwebTcpBase,0x036,a_close(d0,a0))
#pragma amicall(AwebTcpBase,0x03c,a_connect(d0,a0,d1,a1))
#pragma amicall(AwebTcpBase,0x042,a_connect2(d0,d1,d2,d3,a0))
#pragma amicall(AwebTcpBase,0x048,a_bind(d0,a0,d1,a1))
#pragma amicall(AwebTcpBase,0x04e,a_listen(d0,d1,a0))
#pragma amicall(AwebTcpBase,0x054,a_accept(d0,a0,a1,a2))
#pragma amicall(AwebTcpBase,0x05a,a_shutdown(d0,d1,a0))
#pragma amicall(AwebTcpBase,0x060,a_send(d0,a0,d1,d2,a1))
#pragma amicall(AwebTcpBase,0x066,a_recv(d0,a0,d1,d2,a1))
#pragma amicall(AwebTcpBase,0x06c,a_getsockname(d0,a0,a1,a2))
#pragma amicall(AwebTcpBase,0x072,a_gethostname(a0,d0,a1))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall AwebTcpBase a_setup          01e 801
#pragma  libcall AwebTcpBase a_cleanup          024 801
#pragma  libcall AwebTcpBase a_gethostbyname       02a 9802
#pragma  libcall AwebTcpBase a_socket          030 821004
#pragma  libcall AwebTcpBase a_close          036 8002
#pragma  libcall AwebTcpBase a_connect          03c 918004
#pragma  libcall AwebTcpBase a_connect2        042 8321005
#pragma  libcall AwebTcpBase a_bind          048 918004
#pragma  libcall AwebTcpBase a_listen          04e 81003
#pragma  libcall AwebTcpBase a_accept          054 a98004
#pragma  libcall AwebTcpBase a_shutdown        05a 81003
#pragma  libcall AwebTcpBase a_send          060 9218005
#pragma  libcall AwebTcpBase a_recv          066 9218005
#pragma  libcall AwebTcpBase a_getsockname       06c a98004
#pragma  libcall AwebTcpBase a_gethostname       072 90803
#endif

#ifdef __cplusplus
}
#endif

#endif   /*  _INCLUDE_PRAGMA_AWEBTCP_LIB_H  */
