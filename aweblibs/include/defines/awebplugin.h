/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBPLUGIN_H
#define _INLINE_AWEBPLUGIN_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBPLUGIN_BASE_NAME
#define AWEBPLUGIN_BASE_NAME AwebPluginBase
#endif /* !AWEBPLUGIN_BASE_NAME */

#define Initplugin(___plugininfo) __Initplugin_WB(AWEBPLUGIN_BASE_NAME, ___plugininfo)
#define __Initplugin_WB(___base, ___plugininfo) \
   AROS_LC1(ULONG, Initplugin, \
   AROS_LCA(struct Plugininfo *, (___plugininfo), A0), \
   struct Library *, (___base), 5, Awebplugin)

#define Queryplugin(___pluginquery) __Queryplugin_WB(AWEBPLUGIN_BASE_NAME, ___pluginquery)
#define __Queryplugin_WB(___base, ___pluginquery) \
   AROS_LC1(void, Queryplugin, \
   AROS_LCA(struct Pluginquery *, (___pluginquery), A0), \
   struct Library *, (___base), 6, Awebplugin)

#define Commandplugin(___plugincommand) __Commandplugin_WB(AWEBPLUGIN_BASE_NAME, ___plugincommand)
#define __Commandplugin_WB(___base, ___plugincommand) \
   AROS_LC1(void, Commandplugin, \
   AROS_LCA(struct Plugincommand *, (___plugincommand), A0), \
   struct Library *, (___base), 7, Awebplugin)

#define Filterplugin(___pf) __Filterplugin_WB(AWEBPLUGIN_BASE_NAME, ___pf)
#define __Filterplugin_WB(___base, ___pf) \
   AROS_LC1(void, Filterplugin, \
   AROS_LCA(struct Pluginfilter *, (___pf), A0), \
   struct Library *, (___base), 8, Awebplugin)

#endif /* !_INLINE_AWEBPLUGIN_H */
