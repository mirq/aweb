/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSUBTASK_H
#define _INLINE_AWEBSUBTASK_H

#ifndef __INLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AWEBSUBTASK_BASE_NAME
#define AWEBSUBTASK_BASE_NAME AwebSubtaskBase
#endif /* !AWEBSUBTASK_BASE_NAME */

#define AwebGetTaskFunc(___id) __AwebGetTaskFunc_WB(AWEBSUBTASK_BASE_NAME, ___id)
#define __AwebGetTaskFunc_WB(___base, ___id) \
   LP1(0x1e, Subtaskfunction *, AwebGetTaskFunc, ULONG, ___id, d0,\
   , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_INLINE_AWEBSUBTASK_H */
