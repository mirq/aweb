#ifndef _INCLUDE_PRAGMA_AWEBSTARTUP_LIB_H
#define _INCLUDE_PRAGMA_AWEBSTARTUP_LIB_H

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

   $Id: awebstartup_lib.h,v 1.3 2009/06/15 17:05:00 opi Exp $

   Desc:

***********************************************************************/

#ifndef CLIB_AWEBSTARTUP_PROTOS_H
#include <clib/awebstartup_protos.h>
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(AwebStartupBase,0x01e,AwebStartupOpen(a0,a1,a2,a3))
#pragma amicall(AwebStartupBase,0x024,AwebStartupState(d0))
#pragma amicall(AwebStartupBase,0x02a,AwebStartupLevel(d0,d1))
#pragma amicall(AwebStartupBase,0x030,AwebStartupClose())
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall AwebStartupBase AwebStartupOpen        01e ba9804
#pragma  libcall AwebStartupBase AwebStartupState       024 001
#pragma  libcall AwebStartupBase AwebStartupLevel       02a 1002
#pragma  libcall AwebStartupBase AwebStartupClose       030 00
#endif

#endif  /*  _INCLUDE_PRAGMA_AWEBSTARTUP_LIB_H  */
