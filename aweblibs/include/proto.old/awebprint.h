#ifndef _PROTO_AWEBPRINT_H
#define _PROTO_AWEBPRINT_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef CLIB_AWEBPRINT_PROTOS_H
#include <clib/awebprint_protos.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *AwebPrintBase;
#endif

#ifdef __GNUC__
#include <inline/awebprint.h>
#elif defined(__VBCC__)
#if defined(__MORPHOS__) || !defined(__PPC__)
#include <inline/awebprint_protos.h>
#endif
#else
#include <pragma/awebprint_lib.h>
#endif

#endif	/*  _PROTO_AWEBPRINT_H  */
