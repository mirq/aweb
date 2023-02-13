/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSTARTUP_H
#define _INLINE_AWEBSTARTUP_H

#ifndef __INLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AWEBSTARTUP_BASE_NAME
#define AWEBSTARTUP_BASE_NAME AwebStartupBase
#endif /* !AWEBSTARTUP_BASE_NAME */

#define AwebStartupOpen(___screen, ___version, ___imagepalette, ___imagedata) __AwebStartupOpen_WB(AWEBSTARTUP_BASE_NAME, ___screen, ___version, ___imagepalette, ___imagedata)
#define __AwebStartupOpen_WB(___base, ___screen, ___version, ___imagepalette, ___imagedata) \
   LP4NR(0x1e, AwebStartupOpen, struct Screen *, ___screen, a0, UBYTE *, ___version, a1, ULONG *, ___imagepalette, a2, UBYTE *, ___imagedata, a3,\
   , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AwebStartupState(___state) __AwebStartupState_WB(AWEBSTARTUP_BASE_NAME, ___state)
#define __AwebStartupState_WB(___base, ___state) \
   LP1NR(0x24, AwebStartupState, ULONG, ___state, d0,\
   , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AwebStartupLevel(___ready, ___total) __AwebStartupLevel_WB(AWEBSTARTUP_BASE_NAME, ___ready, ___total)
#define __AwebStartupLevel_WB(___base, ___ready, ___total) \
   LP2NR(0x2a, AwebStartupLevel, long, ___ready, d0, long, ___total, d1,\
   , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AwebStartupClose() __AwebStartupClose_WB(AWEBSTARTUP_BASE_NAME)
#define __AwebStartupClose_WB(___base) \
   LP0NR(0x30, AwebStartupClose,\
   , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_INLINE_AWEBSTARTUP_H */
