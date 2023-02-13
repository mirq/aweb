/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBAUTHORIZE_H
#define _INLINE_AWEBAUTHORIZE_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBAUTHORIZE_BASE_NAME
#define AWEBAUTHORIZE_BASE_NAME AwebAuthorizeBase
#endif /* !AWEBAUTHORIZE_BASE_NAME */

#define AuthGetTaskFunc(___id) __AuthGetTaskFunc_WB(AWEBAUTHORIZE_BASE_NAME, ___id)
#define __AuthGetTaskFunc_WB(___base, ___id) \
   AROS_LC1(Subtaskfunction *, AuthGetTaskFunc, \
   AROS_LCA(ULONG, (___id), D0), \
   struct Library *, (___base), 5, Awebauthorize)

#define Authorreq(___areq) __Authorreq_WB(AWEBAUTHORIZE_BASE_NAME, ___areq, ___onlypw)
#define __Authorreq_WB(___base, ___areq. ___onlypw) \
   AROS_LC1(void, Authorreq, \
   AROS_LCA(struct Authorreq *, (___areq), A0), \
   AROS_LCA(BOOL, (___onlypw), D0), \
   struct Library *, (___base), 6, Awebauthorize)

#define Authorset(___auth, ___userid, ___passwd) __Authorset_WB(AWEBAUTHORIZE_BASE_NAME, ___auth, ___userid, ___passwd)
#define __Authorset_WB(___base, ___auth, ___userid, ___passwd) \
   AROS_LC3(void, Authorset, \
   AROS_LCA(struct Authorize *, (___auth), A0), \
   AROS_LCA(UBYTE *, (___userid), A1), \
   AROS_LCA(UBYTE *, (___passwd), A2), \
   struct Library *, (___base), 7, Awebauthorize)

#endif /* !_INLINE_AWEBAUTHORIZE_H */
