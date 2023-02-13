/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBPRINT_H
#define _INLINE_AWEBPRINT_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBPRINT_BASE_NAME
#define AWEBPRINT_BASE_NAME AwebPrintBase
#endif /* !AWEBPRINT_BASE_NAME */

#define Printdebugprefs(___prt) __Printdebugprefs_WB(AWEBPRINT_BASE_NAME, ___prt)
#define __Printdebugprefs_WB(___base, ___prt) \
   AROS_LC1(void, Printdebugprefs, \
   AROS_LCA(struct Print *, (___prt), A0), \
   struct Library *, (___base), 5, Awebprint)

#define Printfinddimensions(___prt) __Printfinddimensions_WB(AWEBPRINT_BASE_NAME, ___prt)
#define __Printfinddimensions_WB(___base, ___prt) \
   AROS_LC1(void, Printfinddimensions, \
   AROS_LCA(struct Print *, (___prt), A0), \
   struct Library *, (___base), 6, Awebprint)

#define Printprintsection(___prt) __Printprintsection_WB(AWEBPRINT_BASE_NAME, ___prt)
#define __Printprintsection_WB(___base, ___prt) \
   AROS_LC1(void, Printprintsection, \
   AROS_LCA(struct Print *, (___prt), A0), \
   struct Library *, (___base), 7, Awebprint)

#define Printclosedebug(___prt) __Printclosedebug_WB(AWEBPRINT_BASE_NAME, ___prt)
#define __Printclosedebug_WB(___base, ___prt) \
   AROS_LC1(void, Printclosedebug, \
   AROS_LCA(struct Print *, (___prt), A0), \
   struct Library *, (___base), 8, Awebprint)

#endif /* !_INLINE_AWEBPRINT_H */
