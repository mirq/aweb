/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSUBTASK_H
#define _INLINE_AWEBSUBTASK_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBSUBTASK_BASE_NAME
#define AWEBSUBTASK_BASE_NAME AwebSubtaskBase
#endif /* !AWEBSUBTASK_BASE_NAME */

#define AwebGetTaskFunc(___id) __AwebGetTaskFunc_WB(AWEBSUBTASK_BASE_NAME, ___id)
#define __AwebGetTaskFunc_WB(___base, ___id) \
   AROS_LC1(Subtaskfunction *, AwebGetTaskFunc, \
   AROS_LCA(ULONG, (___id), D0), \
   struct Library *, (___base), 5, Awebsubtask)

#endif /* !_INLINE_AWEBSUBTASK_H */
