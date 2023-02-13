#ifndef _PROTO_AWEBPLUGIN_H
#define _PROTO_AWEBPLUGIN_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#if !defined(CLIB_AWEBPLUGIN_PROTOS_H) && !defined(__GNUC__)
#include <clib/awebplugin_protos.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *AwebPluginBase;
#endif

#ifdef __GNUC__
#include <inline/awebplugin.h>
#elif defined(__VBCC__)
#if defined(__MORPHOS__) || !defined(__PPC__)
#include <inline/awebplugin_protos.h>
#endif
#else
#include <pragma/awebplugin_lib.h>
#endif

#endif  /*  _PROTO_AWEBPLUGIN_H  */
