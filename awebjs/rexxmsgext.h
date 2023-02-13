#ifndef RXMSGEXT_H
#define RXMSGEXT_H

#include <utility/hooks.h>

struct RexxMsgExt {
	struct Hook *rme_SetVarHook;
	struct Hook *rme_GetVarHook;
	APTR         rme_UserData;
};

struct SetVarHookData {
	STRPTR name;
	STRPTR value;
};

struct GetVarHookData {
	STRPTR name;
	STRPTR buffer;           // ARexx assumes a 255 bytes buffer here so .....
};


typedef ULONG (*SetVarHookFunc)(struct Hook *, APTR unused1, struct RexxMsg *rm);
typedef ULONG (*GetVarHookFunc)(struct Hook *, APTR unused1, struct RexxMsg *rm);


#endif
