/**********************************************************************
 *
 * This file is part of the AWeb-II distribution
 *
 * Copyright (C) 2002 Yvon Rozijn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the AWeb Public License as included in this
 * distribution.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * AWeb Public License for more details.
 *
 **********************************************************************/

/* awebinet225.c - AWeb INet225 function library. Compile this with INet225 SDK */

#include <proto/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <exec/libraries.h>

extern struct Library *SockBase;

__asm int inet_recv(register __d0 int a,
   register __a0 char *b,
   register __d1 int c,
   register __d2 int d,
   register __a1 struct Library *SockBase)
{  return recv(a, b, c, d);
}

__asm int inet_send(register __d0 int a,
   register __a0 char *b,
   register __d1 int c,
   register __d2 int d,
   register __a1 struct Library *SockBase)
{  return send(a, b, c, d);
}

__asm int inet_socket(register __d0 int a,
   register __d1 int b,
   register __d2 int c,
   register __a0 struct Library *SockBase)
{  return socket(a, b, c);
}

__asm struct hostent *inet_gethostbyname (register __a0 char *a,
   register __a1 struct Library *SockBase)
{  return gethostbyname(a);
}

__asm int inet_connect(register __d0 int a,
   register __a0 struct hostent *hent,
   register __d1 int port,
   register __a1 struct Library *SockBase)
{
   struct sockaddr_in sad = {0};
   sad.sin_family=hent->h_addrtype;
   sad.sin_port=port;
   sad.sin_addr.s_addr=*(u_long *)(*hent->h_addr_list);
   return connect(a, (struct sockaddr *)&sad, sizeof(sad));
}

__asm int inet_connect2(register __d0 int a,
   register __d1 int addrtype,
   register __d2 u_long addr,
   register __d3 int port,
   register __a0 struct Library *SockBase)
{
   struct sockaddr_in sad = {0};
   sad.sin_family=addrtype;
   sad.sin_port=port;
   sad.sin_addr.s_addr=addr;
   return connect(a, (struct sockaddr *)&sad, sizeof(sad));
}

__asm int inet_getsockname(register __d0 int a,
   register __a0 struct sockaddr *b,
   register __a1 int *c,
   register __a2 struct Library *SockBase)
{
   return getsockname(a, b, c);
}

__asm int inet_bind(register __d0 int a,
   register __a0 struct sockaddr *b,
   register __d1 int c,
   register __a1 struct Library *SockBase)
{
   return bind(a, b, c);
}

__asm int inet_listen(register __d0 int a,
   register __d1 int b,
   register __a0 struct Library *SockBase)
{
   return listen(a, b);
}

__asm int inet_accept(register __d0 int a,
   register __a0 struct sockaddr *b,
   register __a1 int *c,
   register __a2 struct Library *SockBase)
{
   return accept(a, b, c);
}

__asm int inet_shutdown(register __d0 int a,
   register __d1 int b,
   register __a0 struct Library *SockBase)
{  return shutdown(a, b);
}

__asm int inet_close(register __d0 int a,
   register __a0 struct Library *SockBase)
{  return s_close(a);
}

__asm int inet_setup(register __a0 struct Library *SockBase)
{
   return (int)setup_sockets(4, &errno); /* max 4 sockets per process here */
}

__asm void inet_cleanup(register __a0 struct Library *SockBase)
{
   cleanup_sockets();
}

__asm void inet_dummy(void)
{  return;
}

static UBYTE version[]="AwebInet225.library";

struct Jumptab
{  UWORD jmp;
   void *function;
};
#define JMP 0x4ef9

static struct Jumptab jumptab[]=
{
   JMP,inet_getsockname,
   JMP,inet_recv,
   JMP,inet_send,
   JMP,inet_shutdown,
   JMP,inet_accept,
   JMP,inet_listen,
   JMP,inet_bind,
   JMP,inet_connect2,
   JMP,inet_connect,
   JMP,inet_close,
   JMP,inet_socket,
   JMP,inet_gethostbyname,
   JMP,inet_cleanup,
   JMP,inet_setup,
   JMP,inet_dummy, /* Extfunc */
   JMP,inet_dummy, /* Expunge */
   JMP,inet_dummy, /* Close */
   JMP,inet_dummy, /* Open */
};
static struct Library awebinetlib=
{  {  NULL,NULL,NT_LIBRARY,0,version },
   0,0,
   sizeof(jumptab),
   sizeof(struct Library),
   1,0,
   version,
   0,0
};

struct Library *AwebInet225Base=&awebinetlib;
