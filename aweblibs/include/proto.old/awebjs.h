#ifndef _PROTO_AWEBJS_H
#define _PROTO_AWEBJS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef CLIB_AWEBJS_PROTOS_H
#include <clib/awebjs_protos.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *AWebJSBase;
#endif

#ifdef __GNUC__
#include <inline/awebjs.h>
#elif defined(__VBCC__)
#if defined(__MORPHOS__) || !defined(__PPC__)
#include <inline/awebjs_protos.h>
#endif
#else
#include <pragma/awebjs_lib.h>
#endif

#endif	/*  _PROTO_AWEBJS_H  */
