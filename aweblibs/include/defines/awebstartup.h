/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSTARTUP_H
#define _INLINE_AWEBSTARTUP_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBSTARTUP_BASE_NAME
#define AWEBSTARTUP_BASE_NAME AwebStartupBase
#endif /* !AWEBSTARTUP_BASE_NAME */

#define AwebStartupOpen(___screen, ___version, ___imagepalette, ___imagedata) __AwebStartupOpen_WB(AWEBSTARTUP_BASE_NAME, ___screen, ___version, ___imagepalette, ___imagedata)
#define __AwebStartupOpen_WB(___base, ___screen, ___version, ___imagepalette, ___imagedata) \
   AROS_LC4(void, AwebStartupOpen, \
   AROS_LCA(struct Screen *, (___screen), A0), \
   AROS_LCA(UBYTE *, (___version), A1), \
   AROS_LCA(ULONG *, (___imagepalette), A2), \
   AROS_LCA(UBYTE *, (___imagedata), A3), \
   struct Library *, (___base), 5, Awebstartup)

#define AwebStartupState(___state) __AwebStartupState_WB(AWEBSTARTUP_BASE_NAME, ___state)
#define __AwebStartupState_WB(___base, ___state) \
   AROS_LC1(void, AwebStartupState, \
   AROS_LCA(ULONG, (___state), D0), \
   struct Library *, (___base), 6, Awebstartup)

#define AwebStartupLevel(___ready, ___total) __AwebStartupLevel_WB(AWEBSTARTUP_BASE_NAME, ___ready, ___total)
#define __AwebStartupLevel_WB(___base, ___ready, ___total) \
   AROS_LC2(void, AwebStartupLevel, \
   AROS_LCA(long, (___ready), D0), \
   AROS_LCA(long, (___total), D1), \
   struct Library *, (___base), 7, Awebstartup)

#define AwebStartupClose() __AwebStartupClose_WB(AWEBSTARTUP_BASE_NAME)
#define __AwebStartupClose_WB(___base) \
   AROS_LC0(void, AwebStartupClose, \
   struct Library *, (___base), 8, Awebstartup)

#endif /* !_INLINE_AWEBSTARTUP_H */
