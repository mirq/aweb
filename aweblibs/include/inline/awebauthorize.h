/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBAUTHORIZE_H
#define _INLINE_AWEBAUTHORIZE_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AWEBAUTHORIZE_BASE_NAME
#define AWEBAUTHORIZE_BASE_NAME AwebAuthorizeBase
#endif /* !AWEBAUTHORIZE_BASE_NAME */

#define AuthGetTaskFunc(___id) __AuthGetTaskFunc_WB(AWEBAUTHORIZE_BASE_NAME, ___id)
#define __AuthGetTaskFunc_WB(___base, ___id) \
   LP1(0x1e, Subtaskfunction *, AuthGetTaskFunc , ULONG, ___id, d0, ,(___base)\
)

#define Authorreq(___areq, ___onlypw) __Authorreq_WB(AWEBAUTHORIZE_BASE_NAME, ___areq, ___onlypw)
#define __Authorreq_WB(___base, ___areq, ___onlypw) \
   LP2NR(0x24, Authorreq , struct Authorreq *, ___areq, a0, BOOL, ___onlypw, d0, ,(___base)\
)

#define Authorset(___auth, ___userid, ___passwd) __Authorset_WB(AWEBAUTHORIZE_BASE_NAME, ___auth, ___userid, ___passwd)
#define __Authorset_WB(___base, ___auth, ___userid, ___passwd) \
   LP3NR(0x2a, Authorset , struct Authorize *, ___auth, a0, UBYTE *, ___userid, a1, UBYTE *, ___passwd, a2, ,(___base)\
)

#endif /* !_INLINE_AWEBAUTHORIZE_H */
