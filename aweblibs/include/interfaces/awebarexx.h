#ifndef AWEBAREXX_INTERFACE_DEF_H
#define AWEBAREXX_INTERFACE_DEF_H

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


struct AwebArexxIFace
{
   struct InterfaceData Data;

   ULONG APICALL (*Obtain)(struct AwebArexxIFace *Self);
   ULONG APICALL (*Release)(struct AwebArexxIFace *Self);
   void APICALL (*Expunge)(struct AwebArexxIFace *Self);
   struct Interface * APICALL (*Clone)(struct AwebArexxIFace *Self);
   void APICALL (*AwebArexxGetCfg)(struct AwebArexxIFace *Self, struct Arexxcmd *ac, struct AwebPrefs *prefs);
   void APICALL (*AwebArexxSetCfg)(struct AwebArexxIFace *Self, struct Arexxcmd *ac, struct AwebPrefs *prefs);

};

#endif /* AWEBAREXX_INTERFACE_DEF_H */
