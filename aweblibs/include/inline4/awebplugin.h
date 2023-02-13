/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBPLUGIN_H
#define _INLINE_AWEBPLUGIN_H

#define Initplugin(___plugininfo) __Initplugin_WB(IAwebPlugin, ___plugininfo)
#define __Initplugin_WB(___base, ___plugininfo) \
   (((struct AwebPluginIFace *)(___base))->Initplugin)((___plugininfo))

#define Queryplugin(___pluginquery) __Queryplugin_WB(IAwebPlugin, ___pluginquery)
#define __Queryplugin_WB(___base, ___pluginquery) \
   (((struct AwebPluginIFace *)(___base))->Queryplugin)((___pluginquery))

#define Commandplugin(___plugincommand) __Commandplugin_WB(IAwebPlugin, ___plugincommand)
#define __Commandplugin_WB(___base, ___plugincommand) \
   (((struct AwebPluginIFace *)(___base))->Commandplugin)((___plugincommand))

#define Filterplugin(___pf) __Filterplugin_WB(IAwebPlugin, ___pf)
#define __Filterplugin_WB(___base, ___pf) \
   (((struct AwebPluginIFace *)(___base))->Filterplugin)((___pf))

#endif /* !_INLINE_AWEBPLUGIN_H */
