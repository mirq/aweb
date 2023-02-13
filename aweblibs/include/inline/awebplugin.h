/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBPLUGIN_H
#define _INLINE_AWEBPLUGIN_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AWEBPLUGIN_BASE_NAME
#define AWEBPLUGIN_BASE_NAME AwebPluginBase
#endif /* !AWEBPLUGIN_BASE_NAME */

#define Initplugin(___plugininfo) __Initplugin_WB(AWEBPLUGIN_BASE_NAME, ___plugininfo)
#define __Initplugin_WB(___base, ___plugininfo) \
   LP1(0x1e, ULONG, Initplugin , struct Plugininfo *, ___plugininfo, a0, ,(___base)\
)

#define Queryplugin(___pluginquery) __Queryplugin_WB(AWEBPLUGIN_BASE_NAME, ___pluginquery)
#define __Queryplugin_WB(___base, ___pluginquery) \
   LP1NR(0x24, Queryplugin , struct Pluginquery *, ___pluginquery, a0, ,(___base)\
)

#define Commandplugin(___plugincommand) __Commandplugin_WB(AWEBPLUGIN_BASE_NAME, ___plugincommand)
#define __Commandplugin_WB(___base, ___plugincommand) \
   LP1NR(0x2a, Commandplugin , struct Plugincommand *, ___plugincommand, a0, ,(___base)\
)

#define Filterplugin(___pf) __Filterplugin_WB(AWEBPLUGIN_BASE_NAME, ___pf)
#define __Filterplugin_WB(___base, ___pf) \
   LP1NR(0x30, Filterplugin , struct Pluginfilter *, ___pf, a0, ,(___base)\
)

#endif /* !_INLINE_AWEBPLUGIN_H */
