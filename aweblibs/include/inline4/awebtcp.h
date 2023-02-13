/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBTCP_H
#define _INLINE_AWEBTCP_H

#define a_setup(___SocketBase) __a_setup_WB(IAwebTcp, ___SocketBase)
#define __a_setup_WB(___base, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_setup)((___SocketBase))

#define a_cleanup(___SocketBase) __a_cleanup_WB(IAwebTcp, ___SocketBase)
#define __a_cleanup_WB(___base, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_cleanup)((___SocketBase))

#define a_gethostbyname(___a, ___SocketBase) __a_gethostbyname_WB(IAwebTcp, ___a, ___SocketBase)
#define __a_gethostbyname_WB(___base, ___a, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_gethostbyname)((___a), (___SocketBase))

#define a_socket(___a, ___b, ___c, ___SocketBase) __a_socket_WB(IAwebTcp, ___a, ___b, ___c, ___SocketBase)
#define __a_socket_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_socket)((___a), (___b), (___c), (___SocketBase))

#define a_close(___a, ___SocketBase) __a_close_WB(IAwebTcp, ___a, ___SocketBase)
#define __a_close_WB(___base, ___a, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_close)((___a), (___SocketBase))

#define a_connect(___a, ___hent, ___port, ___SocketBase) __a_connect_WB(IAwebTcp, ___a, ___hent, ___port, ___SocketBase)
#define __a_connect_WB(___base, ___a, ___hent, ___port, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_connect)((___a), (___hent), (___port), (___SocketBase))

#define a_connect2(___a, ___addrtype, ___addr, ___port, ___SocketBase) __a_connect2_WB(IAwebTcp, ___a, ___addrtype, ___addr, ___port, ___SocketBase)
#define __a_connect2_WB(___base, ___a, ___addrtype, ___addr, ___port, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_connect2)((___a), (___addrtype), (___addr), (___port), (___SocketBase))

#define a_bind(___a, ___b, ___c, ___SocketBase) __a_bind_WB(IAwebTcp, ___a, ___b, ___c, ___SocketBase)
#define __a_bind_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_bind)((___a), (___b), (___c), (___SocketBase))

#define a_listen(___a, ___b, ___SocketBase) __a_listen_WB(IAwebTcp, ___a, ___b, ___SocketBase)
#define __a_listen_WB(___base, ___a, ___b, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_listen)((___a), (___b), (___SocketBase))

#define a_accept(___a, ___b, ___c, ___SocketBase) __a_accept_WB(IAwebTcp, ___a, ___b, ___c, ___SocketBase)
#define __a_accept_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_accept)((___a), (___b), (___c), (___SocketBase))

#define a_shutdown(___a, ___b, ___SocketBase) __a_shutdown_WB(IAwebTcp, ___a, ___b, ___SocketBase)
#define __a_shutdown_WB(___base, ___a, ___b, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_shutdown)((___a), (___b), (___SocketBase))

#define a_send(___a, ___b, ___c, ___d, ___SocketBase) __a_send_WB(IAwebTcp, ___a, ___b, ___c, ___d, ___SocketBase)
#define __a_send_WB(___base, ___a, ___b, ___c, ___d, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_send)((___a), (___b), (___c), (___d), (___SocketBase))

#define a_recv(___a, ___b, ___c, ___d, ___SocketBase) __a_recv_WB(IAwebTcp, ___a, ___b, ___c, ___d, ___SocketBase)
#define __a_recv_WB(___base, ___a, ___b, ___c, ___d, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_recv)((___a), (___b), (___c), (___d), (___SocketBase))

#define a_getsockname(___a, ___b, ___c, ___SocketBase) __a_getsockname_WB(IAwebTcp, ___a, ___b, ___c, ___SocketBase)
#define __a_getsockname_WB(___base, ___a, ___b, ___c, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_getsockname)((___a), (___b), (___c), (___SocketBase))

#define a_gethostname(___a, ___b, ___SocketBase) __a_gethostname_WB(IAwebTcp, ___a, ___b, ___SocketBase)
#define __a_gethostname_WB(___base, ___a, ___b, ___SocketBase) \
   (((struct AwebTcpIFace *)(___base))->a_gethostname)((___a), (___b), (___SocketBase))

#endif /* !_INLINE_AWEBTCP_H */
