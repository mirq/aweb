/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSSL_H
#define _INLINE_AWEBSSL_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AWEBSSL_BASE_NAME
#define AWEBSSL_BASE_NAME AwebSslBase
#endif /* !AWEBSSL_BASE_NAME */

#define Assl_cleanup(___assl) __Assl_cleanup_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_cleanup_WB(___base, ___assl) \
   LP1NR(0x1e, Assl_cleanup , struct Assl *, ___assl, a0, ,(___base)\
)

#define Assl_openssl(___assl) __Assl_openssl_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_openssl_WB(___base, ___assl) \
   LP1(0x24, BOOL, Assl_openssl , struct Assl *, ___assl, a0, ,(___base)\
)

#define Assl_closessl(___assl) __Assl_closessl_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_closessl_WB(___base, ___assl) \
   LP1NR(0x2a, Assl_closessl , struct Assl *, ___assl, a0, ,(___base)\
)

#define Assl_connect(___assl, ___sock, ___hostname) __Assl_connect_WB(AWEBSSL_BASE_NAME, ___assl, ___sock, ___hostname)
#define __Assl_connect_WB(___base, ___assl, ___sock, ___hostname) \
   LP3(0x30, long, Assl_connect , struct Assl *, ___assl, a0, long, ___sock, d0, UBYTE *, ___hostname, a1, ,(___base)\
)

#define Assl_geterror(___assl, ___errbuf) __Assl_geterror_WB(AWEBSSL_BASE_NAME, ___assl, ___errbuf)
#define __Assl_geterror_WB(___base, ___assl, ___errbuf) \
   LP2(0x36, char *, Assl_geterror , struct Assl *, ___assl, a0, char *, ___errbuf, a1, ,(___base)\
)

#define Assl_write(___assl, ___buffer, ___length) __Assl_write_WB(AWEBSSL_BASE_NAME, ___assl, ___buffer, ___length)
#define __Assl_write_WB(___base, ___assl, ___buffer, ___length) \
   LP3(0x3c, long, Assl_write , struct Assl *, ___assl, a0, char *, ___buffer, a1, long, ___length, d0, ,(___base)\
)

#define Assl_read(___assl, ___buffer, ___length) __Assl_read_WB(AWEBSSL_BASE_NAME, ___assl, ___buffer, ___length)
#define __Assl_read_WB(___base, ___assl, ___buffer, ___length) \
   LP3(0x42, long, Assl_read , struct Assl *, ___assl, a0, char *, ___buffer, a1, long, ___length, d0, ,(___base)\
)

#define Assl_getcipher(___assl) __Assl_getcipher_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_getcipher_WB(___base, ___assl) \
   LP1(0x48, char *, Assl_getcipher , struct Assl *, ___assl, a0, ,(___base)\
)

#define Assl_libname(___assl) __Assl_libname_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_libname_WB(___base, ___assl) \
   LP1(0x4e, char *, Assl_libname , struct Assl *, ___assl, a0, ,(___base)\
)

#endif /* !_INLINE_AWEBSSL_H */
