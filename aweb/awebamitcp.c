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

/* awebamitcp.c - AWeb AmiTCP function library. Compile this with AmiTCP SDK */

#include "platform_specific.h"
#if defined(__amigaos4__)
/* We only need this for OS4 and it causes probs with amitcp sys/types on m68k */
#include "aweb.h"
#endif

#include <exec/types.h>
#include <exec/libraries.h>
#if defined(__amigaos4__)
#include <exec/interfaces.h>
#endif

#if defined(__amigaos4__)
#include <proto/bsdsocket.h>
#else
#include <proto/socket.h>
#endif

#include <proto/exec.h>

#ifdef __SASC
typedef long ssize_t;
#endif

#include <netinet/in.h>
#include <netdb.h>

#ifdef __AROS__
#    error this file does not work with AROS
#endif

#if defined(__amigaos4__)

/* We only need this cutdown version here as ultimately this library is */
/* accessed using AwebTcpIFace */

struct AwebAmiTCPIFace
{
    struct InterfaceData Data;
};

static struct Library *awebamitcplib;

/* for os4 we compile with raodshow includes,*/
/* type u_long appears as the more amigalike ULONG */

#define u_long ULONG

#endif

/*--------------------------------------------------------------------*/
/* Library management functions */

LIBFUNC_H1
(
    struct Library *, amitcp_Open,
    long,             version, D0,
    LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT

#if defined(__amigaos4__)
    struct Library *awebamitcpbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
    struct Library *awebamitcpbase = LIBMAN_NAME;
#endif

    awebamitcpbase->lib_OpenCnt++;
    return awebamitcpbase;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
    long, amitcp_Close,
    LIBMAN_TYPE, LIBMAN_NAME
)

{
    LIBFUNC_INIT

#if defined(__amigaos4__)
    struct Library *awebamitcpbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
    struct Library *awebamitcpbase = LIBMAN_NAME;
#endif

    awebamitcpbase->lib_OpenCnt--;
    return 0;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
    long, amitcp_Expunge,
    LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT
#if defined(__amigaos4__)
    struct Library *awebamitcpbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
    struct Library *awebamitcpbase = LIBMAN_NAME;
#endif

    if(awebamitcpbase->lib_OpenCnt == 0)
    {
        Remove((struct Node *)awebamitcpbase);
    }
    return 0;

    LIBFUNC_EXIT
}

long amitcp_Extfunc(void)
{
    return 0;
}


/*--------------------------------------------------------------------*/

LIBFUNC_H5
(
    int, amitcp_recv,
    int,              a,          D0,
    char *,           b,          A0,
    int,              c,          D1,
    int,              d,          D2,
    SOCKET_TYPE, SOCKET_NAME, A1,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return recv(a, b, c, d);

    LIBFUNC_EXIT
}

LIBFUNC_H5
(
    int, amitcp_send,
    int,              a,          D0,
    char *,           b,          A0,
    int,              c,          D1,
    int,              d,          D2,
    SOCKET_TYPE, SOCKET_NAME, A1,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return send(a, b, c, d);

    LIBFUNC_EXIT
}

LIBFUNC_H4
(
    int, amitcp_socket,
    int,              a,          D0,
    int,              b,          D1,
    int,              c,          D2,
    SOCKET_TYPE, SOCKET_NAME, A0,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return socket(a, b, c);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    struct hostent *, amitcp_gethostbyname,
    char *,           a,          A0,
    SOCKET_TYPE, SOCKET_NAME, A1,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT


    return gethostbyname(a);

    LIBFUNC_EXIT
}

LIBFUNC_H4
(
    int, amitcp_connect,
    int,              a,          D0,
    struct hostent *, hent,       A0,
    int,              port,       D1,
    SOCKET_TYPE, SOCKET_NAME, A1,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    struct sockaddr_in sad = {0};



    sad.sin_len         = sizeof(sad);
    sad.sin_family      = hent->h_addrtype;
    sad.sin_port        = port;
    sad.sin_addr.s_addr = *(u_long *)(*hent->h_addr_list);

    return connect(a, (struct sockaddr *)&sad, sizeof(sad));

    LIBFUNC_EXIT
}

LIBFUNC_H5
(
    int, amitcp_connect2,
    int,              a,          D0,
    int,              addrtype,   D1,
    u_long,           addr,       D2,
    int,              port,       D3,
    SOCKET_TYPE, SOCKET_NAME, A0,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    struct sockaddr_in sad = {0};

    sad.sin_len         = sizeof(sad);
    sad.sin_family      = addrtype;
    sad.sin_port        = port;
    sad.sin_addr.s_addr = addr;

    return connect(a, (struct sockaddr *)&sad, sizeof(sad));

    LIBFUNC_EXIT
}

LIBFUNC_H4
(
    int, amitcp_getsockname,
    int,               a,          D0,
    struct sockaddr *, b,          A0,
    int *,             c,          A1,
    SOCKET_TYPE, SOCKET_NAME, A2,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return getsockname(a, b, (LONG *)c);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    int, amitcp_gethostname,
    char *,           a,          A0,
    int,              b,          D0,
    SOCKET_TYPE, SOCKET_NAME, A1,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return gethostname(a, b);

    LIBFUNC_EXIT
}

LIBFUNC_H4
(
    int, amitcp_bind,
    int,               a,          D0,
    struct sockaddr *, b,          A0,
    int,               c,          D1,
    SOCKET_TYPE, SOCKET_NAME, A1,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return bind(a, b, c);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    int, amitcp_listen,
    int,              a,          D0,
    int,              b,          D1,
    SOCKET_TYPE, SOCKET_NAME, A0,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return listen(a, b);

    LIBFUNC_EXIT
}

LIBFUNC_H4
(
    int, amitcp_accept,
    int,               a,          D0,
    struct sockaddr *, b,          A0,
    int *,             c,          A1,
    SOCKET_TYPE, SOCKET_NAME, A2,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return accept(a, b, (LONG *)c);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    int, amitcp_shutdown,
    int,              a,          D0,
    int,              b,          D1,
    SOCKET_TYPE, SOCKET_NAME, A0,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return shutdown(a, b);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    int, amitcp_close,
    int,              a,          D0,
    SOCKET_TYPE, SOCKET_NAME, A0,
    AWEBAMITCP_TYPE, AWEBAMITCP_NAME
)
{
    LIBFUNC_INIT

    return CloseSocket(a);

    LIBFUNC_EXIT
}

void amitcp_dummy(void) {}

static UBYTE version[]="awebamitcp.library";

/*-----------------------------------------------------------------------*/

#if defined (__amigaos4__)
/* Make an OS4.0 Library */

struct Library *awebamitcpbase = NULL;
static struct Library *aatbKeeper = NULL;

USRFUNC_H3
(
 static  __saveds struct Library *, Initlib,
 struct Library *, libBase, D0,
 APTR, seglist, A0,
 struct ExecIFace *, exec, A6
)
{
    struct ExecIFace *IExec
#ifdef __GNUC__
        __attribute__((unused))
#endif
        = (struct ExecIFace *)exec;


    libBase->lib_Node.ln_Type = NT_LIBRARY;
    libBase->lib_Node.ln_Pri  = 0;
    libBase->lib_Node.ln_Name = version;
    libBase->lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
    libBase->lib_Version      = 1;
    libBase->lib_Revision     = 0;
    libBase->lib_IdString     = version;

    // libseglist = seglist;

    return (struct Library *)libBase;
}

/* function tables */



/* ------------------- Manager Interface ------------------------ */
static LONG _manager_Obtain(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount++;
}

static ULONG _manager_Release(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount--;
}

/* Manager interface vectors */
static void *lib_manager_vectors[] =
{
    (void *)_manager_Obtain,
    (void *)_manager_Release,
    (void *)0,
    (void *)0,
    (void *)amitcp_Open,
    (void *)amitcp_Close,
    (void *)amitcp_Expunge,
    (void *)0,
    (void *)-1,
};

static LONG amitcp_Obtain(struct AwebAmiTCPIFace *Self)
{
    return Self->Data.RefCount++;
}

static ULONG amitcp_Release(struct AwebAmiTCPIFace *Self)
{
    return Self->Data.RefCount--;
}


static void *main_vectors[] = {
        (void *)amitcp_Obtain,
        (void *)amitcp_Release,
        (void *)NULL,
        (void *)NULL,
        (void *)amitcp_dummy,
        (void *)amitcp_dummy,
        (void *)amitcp_gethostbyname,
        (void *)amitcp_socket,
        (void *)amitcp_close,
        (void *)amitcp_connect,
        (void *)amitcp_connect2,
        (void *)amitcp_bind,
        (void *)amitcp_listen,
        (void *)amitcp_accept,
        (void *)amitcp_shutdown,
        (void *)amitcp_send,
        (void *)amitcp_recv,
        (void *)amitcp_getsockname,
        (void *)amitcp_gethostname,
        (void *)-1
};




/* taglists */

/* "__library" interface tag list */
static struct TagItem lib_managerTags[] =
{
    {MIT_Name,             (ULONG)"__library"},
    {MIT_VectorTable,      (ULONG)lib_manager_vectors},
    {MIT_Version,          1},
    {TAG_DONE,             0}
};


static struct TagItem mainTags[] =
{
    {MIT_Name,              (uint32)"main"},
    {MIT_VectorTable,       (uint32)main_vectors},
    {MIT_Version,           1},
    {TAG_DONE,              0}
};

static uint32 libInterfaces[] =
{
    (uint32)lib_managerTags,
    (uint32)mainTags,
    (uint32)0
};

static struct TagItem libCreateTags[] =
{
    {CLT_DataSize,         (uint32)(sizeof(struct Library))},
    {CLT_InitFunc,         (uint32)Initlib},
    {CLT_Interfaces,       (uint32)libInterfaces},
    {TAG_DONE,             0}
};



/* Aweb init and free functions */

BOOL Initawebamitcp()
{
    if ((awebamitcplib = CreateLibrary((struct TagItem *)libCreateTags)))
    {
        AddLibrary(awebamitcplib);
        /* open the library to prevent expunge */
        aatbKeeper = OpenLibrary("awebamitcp.library",0);
        /* open again for out global base */
        return TRUE;
    }
    return FALSE;
}

void Freeawebamitcp()
{
    if(aatbKeeper){
        CloseLibrary(aatbKeeper);
        aatbKeeper = NULL;
    }

    if(awebamitcplib)
    {
        Forbid();

        while(awebamitcplib->lib_OpenCnt>0)
        {
            Permit();

            Lowlevelreq(AWEBSTR(MSG_ERROR_CANTQUIT), "awebamitcp.library");

            Forbid();
        }


        RemLibrary(awebamitcplib);
        Permit();

        DeleteLibrary(awebamitcplib);
        awebamitcplib = NULL;
    }

}

#elif defined(__MORPHOS__)

static ULONG jumptab[]=
{
   FUNCARRAY_32BIT_NATIVE,
   (ULONG)amitcp_dummy, /* Open */
   (ULONG)amitcp_dummy, /* Close */
   (ULONG)amitcp_dummy, /* Expunge */
   (ULONG)amitcp_dummy, /* Extfunc */
   (ULONG)amitcp_dummy, /* setup */
   (ULONG)amitcp_dummy, /* cleanup */
   (ULONG)&amitcp_gethostbyname,
   (ULONG)&amitcp_socket,
   (ULONG)&amitcp_close,
   (ULONG)&amitcp_connect,
   (ULONG)&amitcp_connect2,
   (ULONG)&amitcp_bind,
   (ULONG)&amitcp_listen,
   (ULONG)&amitcp_accept,
   (ULONG)&amitcp_shutdown,
   (ULONG)&amitcp_send,
   (ULONG)&amitcp_recv,
   (ULONG)&amitcp_getsockname,
   0xFFFFFFFF
};

struct Library *libbase = NULL;
extern struct Library *AwebAmiTcpBase;

BOOL Initawebamitcp(void)
{
      libbase = (struct Library *)NewCreateLibraryTags(
      LIBTAG_FUNCTIONINIT, (ULONG)jumptab,
      LIBTAG_TYPE, NT_LIBRARY,
      LIBTAG_BASESIZE, sizeof(struct Library),
      LIBTAG_NAME, (ULONG)version,
      LIBTAG_FLAGS, 0,
      LIBTAG_VERSION, 1,
      LIBTAG_REVISION, 0,
      LIBTAG_IDSTRING, (ULONG)version,
      LIBTAG_PUBLIC, FALSE,
      LIBTAG_MACHINE, MACHINE_PPC,
      TAG_DONE);

   if (libbase)
   {
   Forbid();
      libbase->lib_OpenCnt++;
      Permit();

      AwebAmiTcpBase = libbase;

     return TRUE;
   }
   return FALSE;
}

void Freeawebamitcp(void)
{
    if(libbase)
    {
        Forbid();

        libbase->lib_OpenCnt--;

        // Hits (??)
        //RemLibrary(libbase);

        Permit();

        libbase = NULL;
    }
}


#elif defined (__amigaos__)


struct Jumptab
{  UWORD jmp;
   void *function;
};
#define JMP 0x4ef9

static struct Jumptab jumptab[]=
{
   {JMP,amitcp_gethostname},
   {JMP,amitcp_getsockname},
   {JMP,amitcp_recv},
   {JMP,amitcp_send},
   {JMP,amitcp_shutdown},
   {JMP,amitcp_accept},
   {JMP,amitcp_listen},
   {JMP,amitcp_bind},
   {JMP,amitcp_connect2},
   {JMP,amitcp_connect},
   {JMP,amitcp_close},
   {JMP,amitcp_socket},
   {JMP,amitcp_gethostbyname},
   {JMP,amitcp_dummy}, /* cleanup */
   {JMP,amitcp_dummy}, /* setup */
   {JMP,amitcp_dummy}, /* Extfunc */
   {JMP,amitcp_dummy}, /* Expunge */
   {JMP,amitcp_dummy}, /* Close */
   {JMP,amitcp_dummy}, /* Open */
};

static struct Library awebamitcplib=
{  {  NULL,NULL,NT_LIBRARY,0,version },
   0,0,
   sizeof(jumptab),
   sizeof(struct Library),
   1,0,
   version,
   0,0
};

/* dummies for the initawebamitcp and freeawebamitcp functions */
/* under 3.x these do nothing because we roll our own library, */
/* but this is a bad thing and we shouldb`t really do it for   */
/* ultimateportability we should use makelibrary etc. In that  */
/* the code to do this would go in thses function stubs        */

extern struct Library *AwebAmiTcpBase;

BOOL Initawebamitcp()
{
AwebAmiTcpBase=&awebamitcplib;

    return TRUE;
}

void Freeawebamitcp()
{

}


#endif // (!__amigaos4__)
