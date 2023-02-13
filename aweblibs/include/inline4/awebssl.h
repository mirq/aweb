/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSSL_H
#define _INLINE_AWEBSSL_H

#define Assl_cleanup(___assl) __Assl_cleanup_WB(IAwebSsl, ___assl)
#define __Assl_cleanup_WB(___base, ___assl) \
   (((struct AwebSslIFace *)(___base))->Assl_cleanup)((___assl))

#define Assl_openssl(___assl) __Assl_openssl_WB(IAwebSsl, ___assl)
#define __Assl_openssl_WB(___base, ___assl) \
   (((struct AwebSslIFace *)(___base))->Assl_openssl)((___assl))

#define Assl_closessl(___assl) __Assl_closessl_WB(IAwebSsl, ___assl)
#define __Assl_closessl_WB(___base, ___assl) \
   (((struct AwebSslIFace *)(___base))->Assl_closessl)((___assl))

#define Assl_connect(___assl, ___sock, ___hostname) __Assl_connect_WB(IAwebSsl, ___assl, ___sock, ___hostname)
#define __Assl_connect_WB(___base, ___assl, ___sock, ___hostname) \
   (((struct AwebSslIFace *)(___base))->Assl_connect)((___assl), (___sock), (___hostname))

#define Assl_geterror(___assl, ___errbuf) __Assl_geterror_WB(IAwebSsl, ___assl, ___errbuf)
#define __Assl_geterror_WB(___base, ___assl, ___errbuf) \
   (((struct AwebSslIFace *)(___base))->Assl_geterror)((___assl), (___errbuf))

#define Assl_write(___assl, ___buffer, ___length) __Assl_write_WB(IAwebSsl, ___assl, ___buffer, ___length)
#define __Assl_write_WB(___base, ___assl, ___buffer, ___length) \
   (((struct AwebSslIFace *)(___base))->Assl_write)((___assl), (___buffer), (___length))

#define Assl_read(___assl, ___buffer, ___length) __Assl_read_WB(IAwebSsl, ___assl, ___buffer, ___length)
#define __Assl_read_WB(___base, ___assl, ___buffer, ___length) \
   (((struct AwebSslIFace *)(___base))->Assl_read)((___assl), (___buffer), (___length))

#define Assl_getcipher(___assl) __Assl_getcipher_WB(IAwebSsl, ___assl)
#define __Assl_getcipher_WB(___base, ___assl) \
   (((struct AwebSslIFace *)(___base))->Assl_getcipher)((___assl))

#define Assl_libname(___assl) __Assl_libname_WB(IAwebSsl, ___assl)
#define __Assl_libname_WB(___base, ___assl) \
   (((struct AwebSslIFace *)(___base))->Assl_libname)((___assl))

#endif /* !_INLINE_AWEBSSL_H */
