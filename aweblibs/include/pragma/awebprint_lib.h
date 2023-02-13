#ifndef _INCLUDE_PRAGMA_AWEBPRINT_LIB_H
#define _INCLUDE_PRAGMA_AWEBPRINT_LIB_H

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

   $Id: awebprint_lib.h,v 1.2 2009/06/15 17:04:59 opi Exp $

   Desc:

***********************************************************************/


#ifndef CLIB_AWEBPRINT_PROTOS_H
#include <clib/awebprint_protos.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(AwebPrintBase,0x01e,Printdebugprefs(a0))
#pragma amicall(AwebPrintBase,0x024,Printfinddimensions(a0))
#pragma amicall(AwebPrintBase,0x02a,Printprintsection(a0))
#pragma amicall(AwebPrintBase,0x030,Printclosedebug(a0))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall AwebPrintBase Printdebugprefs        01e 801
#pragma  libcall AwebPrintBase Printfinddimensions    024 801
#pragma  libcall AwebPrintBase Printprintsection      02a 801
#pragma  libcall AwebPrintBase Printclosedebug        030 801
#endif

#ifdef __cplusplus
}
#endif

#endif   /*  _INCLUDE_PRAGMA_AWEBPRINT_LIB_H  */
