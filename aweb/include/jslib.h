#undef NO_INLINE_STDARG
#define __USE_BASETYPE__
#define __USE_INLINE__
#if defined(__MORPHOS__)
//#define USE_INLINE_STDARG
#endif
#include <proto/awebjs.h>
#if defined(__MORPHOS__)
#undef USE_INLINE_STDARG
#endif
#define NO_INLINE_STDARG
