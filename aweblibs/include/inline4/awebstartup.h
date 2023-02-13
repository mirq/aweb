/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSTARTUP_H
#define _INLINE_AWEBSTARTUP_H

#define AwebStartupOpen(___screen, ___version, ___imagepalette, ___imagedata) __AwebStartupOpen_WB(IAwebStartup, ___screen, ___version, ___imagepalette, ___imagedata)
#define __AwebStartupOpen_WB(___base, ___screen, ___version, ___imagepalette, ___imagedata) \
   (((struct AwebStartupIFace *)(___base))->AwebStartupOpen)((___screen), (___version), (___imagepalette), (___imagedata))

#define AwebStartupState(___state) __AwebStartupState_WB(IAwebStartup, ___state)
#define __AwebStartupState_WB(___base, ___state) \
   (((struct AwebStartupIFace *)(___base))->AwebStartupState)((___state))

#define AwebStartupLevel(___ready, ___total) __AwebStartupLevel_WB(IAwebStartup, ___ready, ___total)
#define __AwebStartupLevel_WB(___base, ___ready, ___total) \
   (((struct AwebStartupIFace *)(___base))->AwebStartupLevel)((___ready), (___total))

#define AwebStartupClose() __AwebStartupClose_WB(IAwebStartup)
#define __AwebStartupClose_WB(___base) \
   (((struct AwebStartupIFace *)(___base))->AwebStartupClose)()

#endif /* !_INLINE_AWEBSTARTUP_H */
