#ifndef _PROTO_AWEBSSL_H
#define _PROTO_AWEBSSL_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef CLIB_AWEBSSL_PROTOS_H
#include <clib/awebssl_protos.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *AwebSslBase;
#endif

#ifdef __GNUC__
#include <inline/awebssl.h>
#elif defined(__VBCC__)
#if defined(__MORPHOS__) || !defined(__PPC__)
#include <inline/awebssl_protos.h>
#endif
#else
#include <pragma/awebssl_lib.h>
#endif

#endif	/*  _PROTO_AWEBSSL_H  */
