#ifndef _PROTO_AWEBSTARTUP_H
#define _PROTO_AWEBSTARTUP_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef CLIB_AWEBSTARTUP_PROTOS_H
#include <clib/awebstartup_protos.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *AwebStartupBase;
#endif

#ifdef __GNUC__
#include <inline/awebstartup.h>
#elif defined(__VBCC__)
#if defined(__MORPHOS__) || !defined(__PPC__)
#include <inline/awebstartup_protos.h>
#endif
#else
#include <pragma/awebstartup_lib.h>
#endif

#endif	/*  _PROTO_AWEBSTARTUP_H  */
