#ifndef _INCLUDE_PRAGMA_AWEBJS_LIB_H
#define _INCLUDE_PRAGMA_AWEBJS_LIB_H

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

   $Id: awebjs_lib.h,v 1.3 2009/06/15 17:04:59 opi Exp $

   Desc:

***********************************************************************/


#ifndef CLIB_AWEBJS_PROTOS_H
#include <clib/awebjs_protos.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(AWebJSBase,0x01e,Newjcontext(a0))
#pragma amicall(AWebJSBase,0x024,Freejcontext(a0))
#pragma amicall(AWebJSBase,0x02a,Runjprogram(a0,a1,a2,a3,a4,d0,d1))
#pragma amicall(AWebJSBase,0x030,Newjobject(a0))
#pragma amicall(AWebJSBase,0x036,Disposejobject(a0))
#pragma amicall(AWebJSBase,0x03c,AddjfunctionA(a0,a1,a2,a3,a4))
#pragma amicall(AWebJSBase,0x042,Jfargument(a0,d0))
#pragma amicall(AWebJSBase,0x048,Jtostring(a0,a1))
#pragma amicall(AWebJSBase,0x04e,Jasgstring(a0,a1,a2))
#pragma amicall(AWebJSBase,0x054,Jasgobject(a0,a1,a2))
#pragma amicall(AWebJSBase,0x05a,Setjobject(a0,a1,a2,a3))
#pragma amicall(AWebJSBase,0x060,Jproperty(a0,a1,a2))
#pragma amicall(AWebJSBase,0x066,Setjproperty(a0,a1,a2))
#pragma amicall(AWebJSBase,0x06c,Jthis(a0))
#pragma amicall(AWebJSBase,0x072,Jointernal(a0))
#pragma amicall(AWebJSBase,0x078,Jasgboolean(a0,a1,d0))
#pragma amicall(AWebJSBase,0x07e,Jtoboolean(a0,a1))
#pragma amicall(AWebJSBase,0x084,Newjarray(a0))
#pragma amicall(AWebJSBase,0x08a,Jnewarrayelt(a0,a1))
#pragma amicall(AWebJSBase,0x090,Jtoobject(a0,a1))
#pragma amicall(AWebJSBase,0x096,Jtonumber(a0,a1))
#pragma amicall(AWebJSBase,0x09c,Jasgnumber(a0,a1,d0))
#pragma amicall(AWebJSBase,0x0a2,Jisarray(a0,a1))
#pragma amicall(AWebJSBase,0x0a8,Jfindarray(a0,a1,a2))
#pragma amicall(AWebJSBase,0x0ae,Jsetprototype(a0,a1,a2))
#pragma amicall(AWebJSBase,0x0b4,Jgetuserdata(a0))
#pragma amicall(AWebJSBase,0x0ba,Jisnumber(a0))
#pragma amicall(AWebJSBase,0x0c0,Clearjobject(a0,a1))
#pragma amicall(AWebJSBase,0x0c6,Freejobject(a0))
#pragma amicall(AWebJSBase,0x0cc,Jdumpobjects(a0))
#pragma amicall(AWebJSBase,0x0d2,Jgetreturnvalue(a0))
#pragma amicall(AWebJSBase,0x0d8,Jpprotect(a0,d0))
#pragma amicall(AWebJSBase,0x0de,Jcprotect(a0,d0))
#pragma amicall(AWebJSBase,0x0e4,Jpname(a0))
#pragma amicall(AWebJSBase,0x0ea,Jdisposehook(a0))
#pragma amicall(AWebJSBase,0x0f0,Jsetfeedback(a0,a1))
#pragma amicall(AWebJSBase,0x0f6,Jdebug(a0,d0))
#pragma amicall(AWebJSBase,0x0fc,Jerrors(a0,d0,d1,d2))
#pragma amicall(AWebJSBase,0x102,Jkeepobject(a0,d0))
#pragma amicall(AWebJSBase,0x108,Jgarbagecollect(a0))
#pragma amicall(AWebJSBase,0x10e,Jsetlinenumber(a0,d0))
#pragma amicall(AWebJSBase,0x114,Jsetobjasfunc(a0,d0))
#pragma amicall(AWebJSBase,0x11a,Jsetscreen(a0,a1))
#pragma amicall(AWebJSBase,0x120,Jaddeventhandler(a0,a1,a2,a3))
#pragma amicall(AWebJSBase,0x060,Jaddproperty(a0,a1,a2))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall AWebJSBase Newjcontext       01e 801
#pragma  libcall AWebJSBase Freejcontext      024 801
#pragma  libcall AWebJSBase Runjprogram       02a 10cba9807
#pragma  libcall AWebJSBase Newjobject         030 801
#pragma  libcall AWebJSBase Disposejobject      036 801
#pragma  libcall AWebJSBase AddjfunctionA      03c cba9805
#pragma  libcall AWebJSBase Jfargument         042 0802
#pragma  libcall AWebJSBase Jtostring         048 9802
#pragma  libcall AWebJSBase Jasgstring         04e a9803
#pragma  libcall AWebJSBase Jasgobject         054 a9803
#pragma  libcall AWebJSBase Setjobject         05a ba9804
#pragma  libcall AWebJSBase Jproperty         060 a9803
#pragma  libcall AWebJSBase Setjproperty      066 a9803
#pragma  libcall AWebJSBase Jthis         06c 801
#pragma  libcall AWebJSBase Jointernal         072 801
#pragma  libcall AWebJSBase Jasgboolean       078 09803
#pragma  libcall AWebJSBase Jtoboolean         07e 9802
#pragma  libcall AWebJSBase Newjarray         084 801
#pragma  libcall AWebJSBase Jnewarrayelt      08a 9802
#pragma  libcall AWebJSBase Jtoobject         090 9802
#pragma  libcall AWebJSBase Jtonumber         096 9802
#pragma  libcall AWebJSBase Jasgnumber         09c 09803
#pragma  libcall AWebJSBase Jisarray         0a2 9802
#pragma  libcall AWebJSBase Jfindarray         0a8 a9803
#pragma  libcall AWebJSBase Jsetprototype      0ae a9803
#pragma  libcall AWebJSBase Jgetuserdata      0b4 801
#pragma  libcall AWebJSBase Jisnumber         0ba 801
#pragma  libcall AWebJSBase Clearjobject      0c0 9802
#pragma  libcall AWebJSBase Freejobject       0c6 801
#pragma  libcall AWebJSBase Jdumpobjects      0cc 801
#pragma  libcall AWebJSBase Jgetreturnvalue      0d2 801
#pragma  libcall AWebJSBase Jpprotect         0d8 0802
#pragma  libcall AWebJSBase Jcprotect         0de 0802
#pragma  libcall AWebJSBase Jpname         0e4 801
#pragma  libcall AWebJSBase Jdisposehook      0ea 801
#pragma  libcall AWebJSBase Jsetfeedback      0f0 9802
#pragma  libcall AWebJSBase Jdebug         0f6 0802
#pragma  libcall AWebJSBase Jerrors         0fc 210804
#pragma  libcall AWebJSBase Jkeepobject       102 0802
#pragma  libcall AWebJSBase Jgarbagecollect      108 801
#pragma  libcall AWebJSBase Jsetlinenumber      10e 0802
#pragma  libcall AWebJSBase Jsetobjasfunc      114 0802
#pragma  libcall AWebJSBase Jsetscreen         11a 9802
#pragma  libcall AWebJSBase Jaddeventhandler      120 ba9804
#pragma  libcall AWebJSBase Jaddproperty      126 a9803

#endif
#ifdef __STORM__
#pragma tagcall(AWebJSBase,0x03c,Addjfunction(a0,a1,a2,a3,a4))
#endif
#ifdef __SASC_60
#pragma  tagcall AWebJSBase Addjfunction      03c cba9805
#endif

#ifdef __cplusplus
}
#endif

#endif   /*  _INCLUDE_PRAGMA_AWEBJS_LIB_H  */
