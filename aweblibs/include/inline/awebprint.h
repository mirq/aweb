/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBPRINT_H
#define _INLINE_AWEBPRINT_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AWEBPRINT_BASE_NAME
#define AWEBPRINT_BASE_NAME AwebPrintBase
#endif /* !AWEBPRINT_BASE_NAME */

#define Printdebugprefs(___prt) __Printdebugprefs_WB(AWEBPRINT_BASE_NAME, ___prt)
#define __Printdebugprefs_WB(___base, ___prt) \
   LP1NR(0x1e, Printdebugprefs , struct Print *, ___prt, a0, ,(___base)\
)

#define Printfinddimensions(___prt) __Printfinddimensions_WB(AWEBPRINT_BASE_NAME, ___prt)
#define __Printfinddimensions_WB(___base, ___prt) \
   LP1NR(0x24, Printfinddimensions , struct Print *, ___prt, a0, ,(___base)\
)

#define Printprintsection(___prt) __Printprintsection_WB(AWEBPRINT_BASE_NAME, ___prt)
#define __Printprintsection_WB(___base, ___prt) \
   LP1NR(0x2a, Printprintsection , struct Print *, ___prt, a0, ,(___base)\
)

#define Printclosedebug(___prt) __Printclosedebug_WB(AWEBPRINT_BASE_NAME, ___prt)
#define __Printclosedebug_WB(___base, ___prt) \
   LP1NR(0x30, Printclosedebug , struct Print *, ___prt, a0, ,(___base)\
)

#endif /* !_INLINE_AWEBPRINT_H */
