#ifndef PLUGIN_INTERFACE_DEF_H
#define PLUGIN_INTERFACE_DEF_H

/*
** This file was machine generated by idltool 50.13.
** Do not edit
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_EXEC_H
#include <exec/exec.h>
#endif
#ifndef EXEC_INTERFACES_H
#include <exec/interfaces.h>
#endif


struct AwebPluginIFace
{
   struct InterfaceData Data;

   ULONG APICALL (*Obtain)(struct AwebPluginIFace *Self);
   ULONG APICALL (*Release)(struct AwebPluginIFace *Self);
   void APICALL (*Expunge)(struct AwebPluginIFace *Self);
   struct Interface * APICALL (*Clone)(struct AwebPluginIFace *Self);
   ULONG APICALL (*Initplugin)(struct AwebPluginIFace *Self, struct Plugininfo * plugininfo);
   void APICALL (*Queryplugin)(struct AwebPluginIFace *Self, struct Pluginquery * pluginquery);
   void APICALL (*Commandplugin)(struct AwebPluginIFace *Self, struct Plugincommand * plugincommand);
   void APICALL (*Filterplugin)(struct AwebPluginIFace *Self, struct Pluginfilter * pf);
};

#endif /* PLUGIN_INTERFACE_DEF_H */
