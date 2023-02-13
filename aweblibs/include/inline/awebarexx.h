/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBAREXX_H
#define _INLINE_AWEBAREXX_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AWEBAREXX_BASE_NAME
#define AWEBAREXX_BASE_NAME AwebArexxBase
#endif /* !AWEBAREXX_BASE_NAME */

#define AwebArexxGetCfg(___ac, ___prefs) __AwebArexxGetCfg_WB(AWEBAREXX_BASE_NAME, ___ac, ___prefs)
#define __AwebArexxGetCfg_WB(___base, ___ac, ___prefs) \
   LP2NR(0x1e, AwebArexxGetCfg , struct Arexxcmd *, ___ac, a0, struct AwebPrefs *, ___prefs, a1, ,(___base)\
)

#define AwebArexxSetCfg(___ac, ___prefs) __AwebArexxSetCfg_WB(AWEBAREXX_BASE_NAME, ___ac, ___prefs)
#define __AwebArexxSetCfg_WB(___base, ___ac, ___prefs) \
   LP2NR(0x24, AwebArexxSetCfg , struct Arexxcmd *, ___ac, a0, struct AwebPrefs *, ___prefs, a1, ,(___base)\
)

#endif /* !_INLINE_AWEBAREXX_H */
