/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBTCP_H
#define _INLINE_AWEBTCP_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AWEBTCP_BASE_NAME
#define AWEBTCP_BASE_NAME AwebTcpBase
#endif /* !AWEBTCP_BASE_NAME */

#define a_setup(___SocketBase) __a_setup_WB(AWEBTCP_BASE_NAME, ___SocketBase)
#define __a_setup_WB(___base, ___SocketBase) \
   LP1(0x1e, int, a_setup , struct Library *, ___SocketBase, a0, ,(___base)\
)

#define a_cleanup(___SocketBase) __a_cleanup_WB(AWEBTCP_BASE_NAME, ___SocketBase)
#define __a_cleanup_WB(___base, ___SocketBase) \
   LP1NR(0x24, a_cleanup , struct Library *, ___SocketBase, a0, ,(___base)\
)

#define a_gethostbyname(___a, ___SocketBase) __a_gethostbyname_WB(AWEBTCP_BASE_NAME, ___a, ___SocketBase)
#define __a_gethostbyname_WB(___base, ___a, ___SocketBase) \
   LP2(0x2a, struct hostent *, a_gethostbyname , char *, ___a, a0, struct Library *, ___SocketBase, a1, ,(___base)\
)

#define a_socket(___a, ___b, ___c, ___SocketBase) __a_socket_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___SocketBase)
#define __a_socket_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   LP4(0x30, int, a_socket , int, ___a, d0, int, ___b, d1, int, ___c, d2, struct Library *, ___SocketBase, a0, ,(___base)\
)

#define a_close(___a, ___SocketBase) __a_close_WB(AWEBTCP_BASE_NAME, ___a, ___SocketBase)
#define __a_close_WB(___base, ___a, ___SocketBase) \
   LP2(0x36, int, a_close , int, ___a, d0, struct Library *, ___SocketBase, a0, ,(___base)\
)

#define a_connect(___a, ___hent, ___port, ___SocketBase) __a_connect_WB(AWEBTCP_BASE_NAME, ___a, ___hent, ___port, ___SocketBase)
#define __a_connect_WB(___base, ___a, ___hent, ___port, ___SocketBase) \
   LP4(0x3c, int, a_connect , int, ___a, d0, struct hostent *, ___hent, a0, int, ___port, d1, struct Library *, ___SocketBase, a1, ,(___base)\
)

#define a_connect2(___a, ___addrtype, ___addr, ___port, ___SocketBase) __a_connect2_WB(AWEBTCP_BASE_NAME, ___a, ___addrtype, ___addr, ___port, ___SocketBase)
#define __a_connect2_WB(___base, ___a, ___addrtype, ___addr, ___port, ___SocketBase) \
   LP5(0x42, int, a_connect2 , int, ___a, d0, int, ___addrtype, d1, u_long, ___addr, d2, int, ___port, d3, struct Library *, ___SocketBase, a0, ,(___base)\
)

#define a_bind(___a, ___b, ___c, ___SocketBase) __a_bind_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___SocketBase)
#define __a_bind_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   LP4(0x48, int, a_bind , int, ___a, d0, struct sockaddr *, ___b, a0, int, ___c, d1, struct Library *, ___SocketBase, a1, ,(___base)\
)

#define a_listen(___a, ___b, ___SocketBase) __a_listen_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___SocketBase)
#define __a_listen_WB(___base, ___a, ___b, ___SocketBase) \
   LP3(0x4e, int, a_listen , int, ___a, d0, int, ___b, d1, struct Library *, ___SocketBase, a0, ,(___base)\
)

#define a_accept(___a, ___b, ___c, ___SocketBase) __a_accept_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___SocketBase)
#define __a_accept_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   LP4(0x54, int, a_accept , int, ___a, d0, struct sockaddr *, ___b, a0, int *, ___c, a1, struct Library *, ___SocketBase, a2, ,(___base)\
)

#define a_shutdown(___a, ___b, ___SocketBase) __a_shutdown_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___SocketBase)
#define __a_shutdown_WB(___base, ___a, ___b, ___SocketBase) \
   LP3(0x5a, int, a_shutdown , int, ___a, d0, int, ___b, d1, struct Library *, ___SocketBase, a0, ,(___base)\
)

#define a_send(___a, ___b, ___c, ___d, ___SocketBase) __a_send_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___d, ___SocketBase)
#define __a_send_WB(___base, ___a, ___b, ___c, ___d, ___SocketBase) \
   LP5(0x60, int, a_send , int, ___a, d0, char *, ___b, a0, int, ___c, d1, int, ___d, d2, struct Library *, ___SocketBase, a1, ,(___base)\
)

#define a_recv(___a, ___b, ___c, ___d, ___SocketBase) __a_recv_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___d, ___SocketBase)
#define __a_recv_WB(___base, ___a, ___b, ___c, ___d, ___SocketBase) \
   LP5(0x66, int, a_recv , int, ___a, d0, char *, ___b, a0, int, ___c, d1, int, ___d, d2, struct Library *, ___SocketBase, a1, ,(___base)\
)

#define a_getsockname(___a, ___b, ___c, ___SocketBase) __a_getsockname_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___SocketBase)
#define __a_getsockname_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   LP4(0x6c, int, a_getsockname , int, ___a, d0, struct sockaddr *, ___b, a0, int *, ___c, a1, struct Library *, ___SocketBase, a2, ,(___base)\
)

#define a_gethostname(___a, ___b, ___SocketBase) __a_gethostname_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___SocketBase)
#define __a_gethostname_WB(___base, ___a, ___b, ___SocketBase) \
   LP3(0x72, int, a_gethostname , char *, ___a, a0, int, ___b, d0, struct Library *, ___SocketBase, a1, ,(___base)\
)

#endif /* !_INLINE_AWEBTCP_H */
