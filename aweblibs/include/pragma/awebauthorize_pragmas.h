/* Automatically generated header! Do not edit! */
#ifndef PRAGMAS_AUTHORIZE_PRAGMAS_H
#define PRAGMAS_AUTHORIZE_PRAGMAS_H

/*
**   $VER: authorize_pragmas.h 1.1 (27.01.2005)
**
**   Direct ROM interface (pragma) definitions.
**
**   Copyright (C) 2001 Amiga, Inc.
**       All Rights Reserved
*/

#if defined(LATTICE) || defined(__SASC) || defined(_DCC)
#ifndef __CLIB_PRAGMA_LIBCALL
#define __CLIB_PRAGMA_LIBCALL
#endif /* __CLIB_PRAGMA_LIBCALL */
#else /* __MAXON__, __STORM__ or AZTEC_C */
#ifndef __CLIB_PRAGMA_AMICALL
#define __CLIB_PRAGMA_AMICALL
#endif /* __CLIB_PRAGMA_AMICALL */
#endif /* */

#if defined(__SASC_60) || defined(__STORM__)
#ifndef __CLIB_PRAGMA_TAGCALL
#define __CLIB_PRAGMA_TAGCALL
#endif /* __CLIB_PRAGMA_TAGCALL */
#endif /* __MAXON__, __STORM__ or AZTEC_C */

#ifdef __CLIB_PRAGMA_LIBCALL
 #pragma libcall AwebAuthorizeBase Authorreq 1e 802
#endif /* __CLIB_PRAGMA_LIBCALL */
#ifdef __CLIB_PRAGMA_AMICALL
 #pragma amicall(AwebAuthorizeBase, 0x1e, Authorreq(a0,d0))
#endif /* __CLIB_PRAGMA_AMICALL */
#ifdef __CLIB_PRAGMA_LIBCALL
 #pragma libcall AwebAuthorizeBase Authedittask 24 801
#endif /* __CLIB_PRAGMA_LIBCALL */
#ifdef __CLIB_PRAGMA_AMICALL
 #pragma amicall(AwebAuthorizeBase, 0x24, Authedittask(a0))
#endif /* __CLIB_PRAGMA_AMICALL */
#ifdef __CLIB_PRAGMA_LIBCALL
 #pragma libcall AwebAuthorizeBase Authorset 2a a9803
#endif /* __CLIB_PRAGMA_LIBCALL */
#ifdef __CLIB_PRAGMA_AMICALL
 #pragma amicall(AwebAuthorizeBase, 0x2a, Authorset(a0,a1,a2))
#endif /* __CLIB_PRAGMA_AMICALL */

#endif /* PRAGMAS_AUTHORIZE_PRAGMAS_H */
