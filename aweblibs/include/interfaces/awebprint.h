#ifndef AWEBPRINT_INTERFACE_DEF_H
#define AWEBPRINT_INTERFACE_DEF_H

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


struct AwebPrintIFace
{
   struct InterfaceData Data;

   ULONG APICALL (*Obtain)(struct AwebPrintIFace *Self);
   ULONG APICALL (*Release)(struct AwebPrintIFace *Self);
   void APICALL (*Expunge)(struct AwebPrintIFace *Self);
   struct Interface * APICALL (*Clone)(struct AwebPrintIFace *Self);
   void APICALL (*Printdebugprefs)(struct AwebPrintIFace *Self, struct Print * prt);
   void APICALL (*Printfinddimensions)(struct AwebPrintIFace *Self, struct Print * prt);
   void APICALL (*Printprintsection)(struct AwebPrintIFace *Self, struct Print * prt);
   void APICALL (*Printclosedebug)(struct AwebPrintIFace *Self, struct Print * prt);
};

#endif /* AWEBPRINT_INTERFACE_DEF_H */
