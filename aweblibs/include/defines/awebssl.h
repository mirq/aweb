/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSSL_H
#define _INLINE_AWEBSSL_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBSSL_BASE_NAME
#define AWEBSSL_BASE_NAME AwebSslBase
#endif /* !AWEBSSL_BASE_NAME */

#define Assl_cleanup(___assl) __Assl_cleanup_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_cleanup_WB(___base, ___assl) \
   AROS_LC1(void, Assl_cleanup, \
   AROS_LCA(struct Assl *, (___assl), A0), \
   struct Library *, (___base), 5, Awebssl)

#define Assl_openssl(___assl) __Assl_openssl_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_openssl_WB(___base, ___assl) \
   AROS_LC1(BOOL, Assl_openssl, \
   AROS_LCA(struct Assl *, (___assl), A0), \
   struct Library *, (___base), 6, Awebssl)

#define Assl_closessl(___assl) __Assl_closessl_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_closessl_WB(___base, ___assl) \
   AROS_LC1(void, Assl_closessl, \
   AROS_LCA(struct Assl *, (___assl), A0), \
   struct Library *, (___base), 7, Awebssl)

#define Assl_connect(___assl, ___sock, ___hostname) __Assl_connect_WB(AWEBSSL_BASE_NAME, ___assl, ___sock, ___hostname)
#define __Assl_connect_WB(___base, ___assl, ___sock, ___hostname) \
   AROS_LC3(long, Assl_connect, \
   AROS_LCA(struct Assl *, (___assl), A0), \
   AROS_LCA(long, (___sock), D0), \
   AROS_LCA(UBYTE *, (___hostname), A1), \
   struct Library *, (___base), 8, Awebssl)

#define Assl_geterror(___assl, ___errbuf) __Assl_geterror_WB(AWEBSSL_BASE_NAME, ___assl, ___errbuf)
#define __Assl_geterror_WB(___base, ___assl, ___errbuf) \
   AROS_LC2(char *, Assl_geterror, \
   AROS_LCA(struct Assl *, (___assl), A0), \
   AROS_LCA(char *, (___errbuf), A1), \
   struct Library *, (___base), 9, Awebssl)

#define Assl_write(___assl, ___buffer, ___length) __Assl_write_WB(AWEBSSL_BASE_NAME, ___assl, ___buffer, ___length)
#define __Assl_write_WB(___base, ___assl, ___buffer, ___length) \
   AROS_LC3(long, Assl_write, \
   AROS_LCA(struct Assl *, (___assl), A0), \
   AROS_LCA(char *, (___buffer), A1), \
   AROS_LCA(long, (___length), D0), \
   struct Library *, (___base), 10, Awebssl)

#define Assl_read(___assl, ___buffer, ___length) __Assl_read_WB(AWEBSSL_BASE_NAME, ___assl, ___buffer, ___length)
#define __Assl_read_WB(___base, ___assl, ___buffer, ___length) \
   AROS_LC3(long, Assl_read, \
   AROS_LCA(struct Assl *, (___assl), A0), \
   AROS_LCA(char *, (___buffer), A1), \
   AROS_LCA(long, (___length), D0), \
   struct Library *, (___base), 11, Awebssl)

#define Assl_getcipher(___assl) __Assl_getcipher_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_getcipher_WB(___base, ___assl) \
   AROS_LC1(char *, Assl_getcipher, \
   AROS_LCA(struct Assl *, (___assl), A0), \
   struct Library *, (___base), 12, Awebssl)

#define Assl_libname(___assl) __Assl_libname_WB(AWEBSSL_BASE_NAME, ___assl)
#define __Assl_libname_WB(___base, ___assl) \
   AROS_LC1(char *, Assl_libname, \
   AROS_LCA(struct Assl *, (___assl), A0), \
   struct Library *, (___base), 13, Awebssl)

#endif /* !_INLINE_AWEBSSL_H */
