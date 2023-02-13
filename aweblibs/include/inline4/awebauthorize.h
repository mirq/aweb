/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBAUTHORIZE_H
#define _INLINE_AWEBAUTHORIZE_H

#define AuthGetTaskFunc(___id) __AuthGetTaskFunc_WB(IAwebAuthorize, ___id)
#define __AuthGetTaskFunc_WB(___base, ___id) \
   (((struct AwebAuthorizeIFace *)(___base))->AuthGetTaskFunc)((___id))

#define Authorreq(___areq) __Authorreq_WB(IAwebAuthorize, ___areq, ___onlypw)
#define __Authorreq_WB(___base, ___areq, ___onlypw) \
   (((struct AwebAuthorizeIFace *)(___base))->Authorreq)((___areq), (___onlypw))

#define Authorset(___auth, ___userid, ___passwd) __Authorset_WB(IAwebAuthorize, ___auth, ___userid, ___passwd)
#define __Authorset_WB(___base, ___auth, ___userid, ___passwd) \
   (((struct AwebAuthorizeIFace *)(___base))->Authorset)((___auth), (___userid), (___passwd))

#endif /* !_INLINE_AWEBAUTHORIZE_H */
