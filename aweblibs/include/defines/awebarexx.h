/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBAREXX_H
#define _INLINE_AWEBAREXX_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBAREXX_BASE_NAME
#define AWEBAREXX_BASE_NAME AwebArexxBase
#endif /* !AWEBAREXX_BASE_NAME */

#define AwebArexxGetCfg(___ac, ___prefs) __AwebArexxGetCfg_WB(AWEBAREXX_BASE_NAME, ___ac, ___prefs)
#define __AwebArexxGetCfg_WB(___base, ___ac, ___prefs) \
   AROS_LC2(void, AwebArexxGetCfg, \
   AROS_LCA(struct Arexxcmd *, (___ac), A0), \
   AROS_LCA(struct AwebPrefs *, (___prefs), A1), \
   struct Library *, (___base), 5, Awebarexx)

#define AwebArexxSetCfg(___ac, ___prefs) __AwebArexxSetCfg_WB(AWEBAREXX_BASE_NAME, ___ac, ___prefs)
#define __AwebArexxSetCfg_WB(___base, ___ac, ___prefs) \
   AROS_LC2(void, AwebArexxSetCfg, \
   AROS_LCA(struct Arexxcmd *, (___ac), A0), \
   AROS_LCA(struct AwebPrefs *, (___prefs), A1), \
   struct Library *, (___base), 6, Awebarexx)

#endif /* !_INLINE_AWEBAREXX_H */
