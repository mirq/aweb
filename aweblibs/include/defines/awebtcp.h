/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBTCP_H
#define _INLINE_AWEBTCP_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBTCP_BASE_NAME
#define AWEBTCP_BASE_NAME AwebTcpBase
#endif /* !AWEBTCP_BASE_NAME */

#define a_setup(___SocketBase) __a_setup_WB(AWEBTCP_BASE_NAME, ___SocketBase)
#define __a_setup_WB(___base, ___SocketBase) \
   AROS_LC1(int, a_setup, \
   AROS_LCA(struct Library *, (___SocketBase), A0), \
   struct Library *, (___base), 5, Awebtcp)

#define a_cleanup(___SocketBase) __a_cleanup_WB(AWEBTCP_BASE_NAME, ___SocketBase)
#define __a_cleanup_WB(___base, ___SocketBase) \
   AROS_LC1(void, a_cleanup, \
   AROS_LCA(struct Library *, (___SocketBase), A0), \
   struct Library *, (___base), 6, Awebtcp)

#define a_gethostbyname(___a, ___SocketBase) __a_gethostbyname_WB(AWEBTCP_BASE_NAME, ___a, ___SocketBase)
#define __a_gethostbyname_WB(___base, ___a, ___SocketBase) \
   AROS_LC2(struct hostent *, a_gethostbyname, \
   AROS_LCA(char *, (___a), A0), \
   AROS_LCA(struct Library *, (___SocketBase), A1), \
   struct Library *, (___base), 7, Awebtcp)

#define a_socket(___a, ___b, ___c, ___SocketBase) __a_socket_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___SocketBase)
#define __a_socket_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   AROS_LC4(int, a_socket, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(int, (___b), D1), \
   AROS_LCA(int, (___c), D2), \
   AROS_LCA(struct Library *, (___SocketBase), A0), \
   struct Library *, (___base), 8, Awebtcp)

#define a_close(___a, ___SocketBase) __a_close_WB(AWEBTCP_BASE_NAME, ___a, ___SocketBase)
#define __a_close_WB(___base, ___a, ___SocketBase) \
   AROS_LC2(int, a_close, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(struct Library *, (___SocketBase), A0), \
   struct Library *, (___base), 9, Awebtcp)

#define a_connect(___a, ___hent, ___port, ___SocketBase) __a_connect_WB(AWEBTCP_BASE_NAME, ___a, ___hent, ___port, ___SocketBase)
#define __a_connect_WB(___base, ___a, ___hent, ___port, ___SocketBase) \
   AROS_LC4(int, a_connect, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(struct hostent *, (___hent), A0), \
   AROS_LCA(int, (___port), D1), \
   AROS_LCA(struct Library *, (___SocketBase), A1), \
   struct Library *, (___base), 10, Awebtcp)

#define a_connect2(___a, ___addrtype, ___addr, ___port, ___SocketBase) __a_connect2_WB(AWEBTCP_BASE_NAME, ___a, ___addrtype, ___addr, ___port, ___SocketBase)
#define __a_connect2_WB(___base, ___a, ___addrtype, ___addr, ___port, ___SocketBase) \
   AROS_LC5(int, a_connect2, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(int, (___addrtype), D1), \
   AROS_LCA(u_long, (___addr), D2), \
   AROS_LCA(int, (___port), D3), \
   AROS_LCA(struct Library *, (___SocketBase), A0), \
   struct Library *, (___base), 11, Awebtcp)

#define a_bind(___a, ___b, ___c, ___SocketBase) __a_bind_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___SocketBase)
#define __a_bind_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   AROS_LC4(int, a_bind, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(struct sockaddr *, (___b), A0), \
   AROS_LCA(int, (___c), D1), \
   AROS_LCA(struct Library *, (___SocketBase), A1), \
   struct Library *, (___base), 12, Awebtcp)

#define a_listen(___a, ___b, ___SocketBase) __a_listen_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___SocketBase)
#define __a_listen_WB(___base, ___a, ___b, ___SocketBase) \
   AROS_LC3(int, a_listen, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(int, (___b), D1), \
   AROS_LCA(struct Library *, (___SocketBase), A0), \
   struct Library *, (___base), 13, Awebtcp)

#define a_accept(___a, ___b, ___c, ___SocketBase) __a_accept_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___SocketBase)
#define __a_accept_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   AROS_LC4(int, a_accept, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(struct sockaddr *, (___b), A0), \
   AROS_LCA(int *, (___c), A1), \
   AROS_LCA(struct Library *, (___SocketBase), A2), \
   struct Library *, (___base), 14, Awebtcp)

#define a_shutdown(___a, ___b, ___SocketBase) __a_shutdown_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___SocketBase)
#define __a_shutdown_WB(___base, ___a, ___b, ___SocketBase) \
   AROS_LC3(int, a_shutdown, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(int, (___b), D1), \
   AROS_LCA(struct Library *, (___SocketBase), A0), \
   struct Library *, (___base), 15, Awebtcp)

#define a_send(___a, ___b, ___c, ___d, ___SocketBase) __a_send_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___d, ___SocketBase)
#define __a_send_WB(___base, ___a, ___b, ___c, ___d, ___SocketBase) \
   AROS_LC5(int, a_send, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(char *, (___b), A0), \
   AROS_LCA(int, (___c), D1), \
   AROS_LCA(int, (___d), D2), \
   AROS_LCA(struct Library *, (___SocketBase), A1), \
   struct Library *, (___base), 16, Awebtcp)

#define a_recv(___a, ___b, ___c, ___d, ___SocketBase) __a_recv_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___d, ___SocketBase)
#define __a_recv_WB(___base, ___a, ___b, ___c, ___d, ___SocketBase) \
   AROS_LC5(int, a_recv, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(char *, (___b), A0), \
   AROS_LCA(int, (___c), D1), \
   AROS_LCA(int, (___d), D2), \
   AROS_LCA(struct Library *, (___SocketBase), A1), \
   struct Library *, (___base), 17, Awebtcp)

#define a_getsockname(___a, ___b, ___c, ___SocketBase) __a_getsockname_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___c, ___SocketBase)
#define __a_getsockname_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   AROS_LC4(int, a_getsockname, \
   AROS_LCA(int, (___a), D0), \
   AROS_LCA(struct sockaddr *, (___b), A0), \
   AROS_LCA(int *, (___c), A1), \
   AROS_LCA(struct Library *, (___SocketBase), A2), \
   struct Library *, (___base), 18, Awebtcp)

#define a_gethostname(___a, ___b, ___SocketBase) __a_gethostname_WB(AWEBTCP_BASE_NAME, ___a, ___b, ___SocketBase)
#define __a_gethostname_WB(___base, ___a, ___b, ___SocketBase) \
   AROS_LC3(int, a_gethostname, \
   AROS_LCA(char *, (___a), A0), \
   AROS_LCA(int, (___b), D0), \
   AROS_LCA(struct Library *, (___SocketBase), A1), \
   struct Library *, (___base), 19, Awebtcp)

#endif /* !_INLINE_AWEBTCP_H */
