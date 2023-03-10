#ifndef AWEBAUTHORIZE_INTERFACE_DEF_H
#define AWEBAUTHORIZE_INTERFACE_DEF_H

/*
** This file was machine generated by idltool 51.3.
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

#ifndef LIBRARIES_AWEBAUTHORIZE_H
#include <libraries/awebauthorize.h>
#endif
#ifndef TASK_H
#include <task.h>
#endif

struct AwebAuthorizeIFace
{
        struct InterfaceData Data;

        ULONG APICALL (*Obtain)(struct AwebAuthorizeIFace *Self);
        ULONG APICALL (*Release)(struct AwebAuthorizeIFace *Self);
        void APICALL (*Expunge)(struct AwebAuthorizeIFace *Self);
        struct Interface * APICALL (*Clone)(struct AwebAuthorizeIFace *Self);
        Subtaskfunction * (*AuthGetTaskFunc)(struct AwebAuthorizeIFace *Self, ULONG id) APICALL;
        void APICALL (*Authorreq)(struct AwebAuthorizeIFace *Self, struct Authorreq * areq, BOOL onlypw);
        void APICALL (*Authorset)(struct AwebAuthorizeIFace *Self, struct Authorize * auth, UBYTE * userid, UBYTE * passwd);
};

#endif /* AWEBAUTHORIZE_INTERFACE_DEF_H */
