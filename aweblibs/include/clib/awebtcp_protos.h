#ifndef CLIB_AWEBTCP_PROTOS_H
#define CLIB_AWEBTCP_PROTOS_H


/**********************************************************************

   This file is part of the AWeb-II distribution

   Copyright (C) 2002 Yvon Rozijn
   Copyright (C) 2002-2003 The AWeb Developement Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the AWeb Public License as included in this
   distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   AWeb Public License for more details.

   $Id: awebtcp_protos.h,v 1.4 2009/06/15 17:04:08 opi Exp $

   Desc:

***********************************************************************/

#include <exec/types.h>

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

int a_setup(struct Library * SocketBase);
void a_cleanup(struct Library * SocketBase);
struct hostent * a_gethostbyname(char * a, struct Library * SocketBase);
int a_socket(int a, int b, int c, struct Library * SocketBase);
int a_close(int a, struct Library * SocketBase);
int a_connect(int a, struct hostent * hent, int port, struct Library * SocketBase);
int a_connect2(int a, int addrtype, u_long addr, int port, struct Library * SocketBase);
int a_bind(int a, struct sockaddr * b, int c, struct Library * SocketBase);
int a_listen(int a, int b, struct Library * SocketBase);
int a_accept(int a, struct sockaddr * b, int * c, struct Library * SocketBase);
int a_shutdown(int a, int b, struct Library * SocketBase);
int a_send(int a, char * b, int c, int d, struct Library * SocketBase);
int a_recv(int a, char * b, int c, int d, struct Library * SocketBase);
int a_getsockname(int a, struct sockaddr * b, int * c, struct Library * SocketBase);
int a_gethostname(char * a, int b, struct Library * SocketBase);

#ifdef __cplusplus
}
#endif

#endif   /*  CLIB_AWEBTCP_PROTOS_H  */
