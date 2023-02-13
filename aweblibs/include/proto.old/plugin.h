#ifndef _PROTO_PLUGIN_H
#define _PROTO_PLUGIN_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef CLIB_PLUGIN_PROTOS_H
#include <clib/plugin_protos.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *PluginBase;
#endif

#ifdef __GNUC__
#include <inline/plugin.h>
#elif defined(__VBCC__)
#if defined(__MORPHOS__) || !defined(__PPC__)
#include <inline/plugin_protos.h>
#endif
#else
#include <pragma/plugin_lib.h>
#endif

#endif	/*  _PROTO_PLUGIN_H  */
