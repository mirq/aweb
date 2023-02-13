#ifndef _PROTO_AWEBTCP_H
#define _PROTO_AWEBTCP_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef CLIB_AWEBTCP_PROTOS_H
#include <clib/awebtcp_protos.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *AwebTcpBase;
#endif

#ifdef __GNUC__
#include <inline/awebtcp.h>
#elif defined(__VBCC__)
#if defined(__MORPHOS__) || !defined(__PPC__)
#include <inline/awebtcp_protos.h>
#endif
#else
#include <pragma/awebtcp_lib.h>
#endif

#endif	/*  _PROTO_AWEBTCP_H  */
