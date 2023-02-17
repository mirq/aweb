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

/* amissl.c - AWeb SSL function library. Compile this with AmiSSL SDK */

#ifdef __AROS__
#    error This file does not work with AROS
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "aweb.h"
#include "task.h"
#include "http.h"

#include <libraries/awebssl.h>
#include <libraries/amisslmaster.h>
#include <libraries/amissl.h>

//#define NO_BLOWFISH

#include <proto/exec.h>

#undef NO_INLINE_STDARG
#include <proto/amissl.h>
#include <proto/amisslmaster.h>
#include <amissl/amissl.h>

struct Library *AmiSSLMasterBase = NULL;
struct Library *AmiSSLBase = NULL;
#if defined(__amigaos4__)
struct AmiSSLIFace *IAmiSSL = NULL;
struct AmiSSLMasterIFace *IAmiSSLMaster = NULL;
#endif

/*-----------------------------------------------------------------------*/

struct Assl
{
   struct Library *amisslbase;         /* For Os3.x this AmisSSLBase for Os4 IAmisSSL */
#if defined(__amigaos4__)
   struct AmiSSLIFace *iamissl;
#endif
   SSL_CTX *sslctx;
   SSL *ssl;
   STRPTR hostname;
   BOOL denied;
};

/*-----------------------------------------------------------------------*/

/* we only need this cut down version here as ultimatelt this library will */
/* be accessed through AwebSslIface */

#if defined(__amigaos4__)
struct AwebAmiSSLIFace
{
    struct InterfaceData Data;
};

static struct Library *awebamissllib;

#endif
static struct SignalSemaphore *awebamisslsema = NULL;

static BOOL Initawebamisslsema()
{
    BOOL result = FALSE;
#if defined(__amigaos4__)
    if((awebamisslsema = AllocSysObject(ASOT_SEMAPHORE,TAG_DONE)))
    {
        result = TRUE;
    }

#else

    if((awebamisslsema = ALLOCSTRUCT(SignalSemaphore, 1, 0)))
    {
        InitSemaphore(awebamisslsema);
        result = TRUE;
    }
#endif
    return result;
}

static void Freeawebamisslsema()
{
#if defined(__amigaos4__)
    FreeSysObject(ASOT_SEMAPHORE, awebamisslsema);
#else
    if(awebamisslsema)
    {
        FREE(awebamisslsema);
        awebamisslsema = NULL;
    }
#endif

}

static void Closeamissl()
{

    if(AmiSSLMasterBase)
    {
#if defined(__amigaos4__)
        if(IAmiSSLMaster)
        {
#endif
            if(AmiSSLBase)
            {
#if defined(__amigaos4__)
                if(IAmiSSL)
                {
                    DropInterface((struct Interface *)IAmiSSL);
                    IAmiSSL = NULL;
                }
#endif
                CloseAmiSSL();
                AmiSSLBase = NULL;
            }
#if defined(__amigaos4__)
                DropInterface((struct Interface *)IAmiSSLMaster);
                IAmiSSLMaster = NULL;
        }
#endif

        CloseLibrary(AmiSSLMasterBase);
        AmiSSLMasterBase = NULL;
    }


}

static BOOL Openamissl()
{

        if(!(AmiSSLMasterBase = OpenLibrary("amisslmaster.library",AMISSLMASTER_MIN_VERSION)))
        {
            return FALSE;
        }
#if defined(__amigaos4__)
        if(!(IAmiSSLMaster = (struct AmiSSLMasterIFace *)GetInterface(AmiSSLMasterBase,"main",1,NULL)))
        {
            Closeamissl();
            return FALSE;
        }
#endif
        if(!(InitAmiSSLMaster(AMISSL_CURRENT_VERSION, TRUE)))
        {
            Closeamissl();
            return FALSE;

        }
        if((AmiSSLBase = OpenAmiSSL()))
        {
#if defined(__amigaos4__)
            if(!(IAmiSSL = (struct AmiSSLIFace *)GetInterface((struct Library *)AmiSSLBase,"main",1,NULL)))
            {
                Closeamissl();
                return FALSE;
            }
#endif
        }
        else
        {
            Closeamissl();
            return FALSE;
        }

    return TRUE;
}



/*
   This opens and initialises ammissl. For Os3.x socketbase is indeed the
   bsdsocketbase but for OS4 it's the interface of the bsdsocket.library
*/


struct Assl *Assl_initamissl(void *socketbase)
{
   struct Assl *assl=NULL;

#if defined(__amigaos4__)
   if(IAmiSSL == NULL)
#else
   if(AmiSSLBase == NULL)
#endif
   {
       /* We must be the first process here, try to open aamissl */
       if(awebamisslsema)
       {
            ObtainSemaphore(awebamisslsema);
            /* check library base or interface again incase some task opened it while we waited for the semaphore */
#if defined(__amigaos4__)
            if(IAmiSSL == NULL)
#else
            if(AmiSSLBase == NULL)
#endif
            {
                if(!Openamissl())
                {
                    /* No amissl, no go */
                    ReleaseSemaphore(awebamisslsema);
                    return NULL;
                }
            }
            ReleaseSemaphore(awebamisslsema);
       }
       else
       {
          /* If the semaphore is not valid, something went wrong along time ago */
          return NULL;
       }
   }
   if(assl=ALLOCSTRUCT(Assl,1,MEMF_CLEAR))
   {

      if((assl->amisslbase = AmiSSLBase))
      {

#if defined(__amigaos4__)
          if((assl->iamissl = IAmiSSL))
          {

#endif
#if defined(__amigaos4__)
             if(InitAmiSSL(
                      AmiSSL_ISocket,socketbase,
                   TAG_END))
#else
             if(InitAmiSSL(
                      AmiSSL_SocketBase,(Tag)socketbase,
                   TAG_END))
#endif

             {
                CleanupAmiSSL(TAG_END);
                assl->amisslbase = NULL;
             }
#if defined(__amigaos4__)
          }
#endif

      }
      if(!assl->amisslbase)
      {  FREE(assl);
         assl=NULL;
      }
   }
   return assl;
}



/*-----------------------------------------------------------------------*/

#if defined(__MORPHOS__)
static int __saveds __stdargs
   _Certcallback(void)
{
   int ok = ((int *) REG_A7)[1];
   X509_STORE_CTX *sctx = ((X509_STORE_CTX **) REG_A7)[2];
#else
static int __saveds __stdargs
   _Certcallback(int ok,X509_STORE_CTX *sctx)
{
#endif
   char *s=NULL,*u=NULL;
   struct Assl *assl;
   X509 *xs;
   int err;
   char buf[256];
   if(!ok)
   {  assl=Gettaskuserdata();
      if(assl)
      {
         xs=X509_STORE_CTX_get_current_cert(sctx);
         err=X509_STORE_CTX_get_error(sctx);
         X509_NAME_oneline(X509_get_subject_name(xs),buf,sizeof(buf));
         s=buf;
         u=assl->hostname;
         ok=Httpcertaccept(u,s);
         if(!ok) assl->denied=TRUE;
      }
   }
   return ok;
}

LIBFUNC_H1
(
    static void, _Assl_cleanup,
    struct Assl *, assl, A0,
    ASSL_TYPE, ASSL_NAME
)
{
   LIBFUNC_INIT

   if(assl)
   {  if(assl->amisslbase)
      {
         CleanupAmiSSL(TAG_END);

      }
      FREE(assl);
   }

   LIBFUNC_EXIT
}

LIBFUNC_H1
(
    static BOOL, _Assl_openssl,
    struct Assl *, assl, A0,
    ASSL_TYPE, ASSL_NAME
)
{
   LIBFUNC_INIT

   if(assl && assl->amisslbase)
   {
      SSLeay_add_ssl_algorithms();
      SSL_load_error_strings();
      if(assl->sslctx=SSL_CTX_new(SSLv23_client_method()))
      {
#if defined(__MORPHOS__)
        DECLGATE(static, Certcallback, LIB);
         SSL_CTX_set_default_verify_paths(assl->sslctx);
         SSL_CTX_set_verify(assl->sslctx,/*SSL_VERIFY_PEER|*/SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
            &Certcallback);

#else
         SSL_CTX_set_default_verify_paths(assl->sslctx);
         SSL_CTX_set_verify(assl->sslctx,/*SSL_VERIFY_PEER|*/SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
            (APTR)_Certcallback);
#endif
         if(assl->ssl=SSL_new(assl->sslctx))
         {  Settaskuserdata(assl);
         }
      }
   }
   return (BOOL)(assl && assl->ssl);

   LIBFUNC_EXIT
}

LIBFUNC_H1
(
    static void, _Assl_closessl,
    struct Assl *, assl, A0,
    ASSL_TYPE,ASSL_NAME
)
{
   LIBFUNC_INIT

   if(assl && assl->amisslbase)
   {
      if(assl->ssl)
      {  /* SSL_Shutdown(assl->ssl); */
         SSL_free(assl->ssl);
         assl->ssl=NULL;
      }
      if(assl->sslctx)
      {  SSL_CTX_free(assl->sslctx);
         assl->sslctx=NULL;
      }
   }

   LIBFUNC_EXIT
}

LIBFUNC_H3
(
    static long, _Assl_connect,
    struct Assl *, assl,     A0,
    long,          sock,     D0,
    STRPTR,       hostname, A1,
    ASSL_TYPE, ASSL_NAME

)
{
   LIBFUNC_INIT

   long result=ASSLCONNECT_FAIL;
   if(assl && assl->amisslbase)
   {
      assl->hostname=hostname;
      if(SSL_set_fd(assl->ssl,sock))
      {  if(SSL_connect(assl->ssl)>=0)
         {  result=ASSLCONNECT_OK;
         }
         else if(assl->denied)
         {  result=ASSLCONNECT_DENIED;
         }
      }
   }
   return result;

   LIBFUNC_EXIT
}

LIBFUNC_H2
(
    static char *, _Assl_geterror,
    struct Assl *, assl,   A0,
    char *,        errbuf, A1,
    ASSL_TYPE, ASSL_NAME
)
{
   LIBFUNC_INIT

   long err;
   STRPTR p=NULL;
   short i;
   if(assl && assl->amisslbase)
   {
      ERR_load_SSL_strings();
      err=ERR_get_error();
      ERR_error_string(err,errbuf);
      /* errbuf now contains something like:
         "error:1408806E:SSL routines:SSL_SET_CERTIFICATE:certificate verify failed"
         Find the descriptive text after the 4th colon. */
      for(i=0,p=errbuf;i<4;i++)
      {  p=strchr(p,':');
         if(!p) break;
         p++;
      }
   }
   if(!p) p=errbuf;
   return (char *)p;

   LIBFUNC_EXIT
}

LIBFUNC_H3
(
    static long, _Assl_write,
    struct Assl *, assl,   A0,
    char *,        buffer, A1,
    long,          length, D0,
    ASSL_TYPE, ASSL_NAME
)
{
   LIBFUNC_INIT

   long result=-1;
   if(assl && assl->amisslbase)
   {
      result=SSL_write(assl->ssl,buffer,length);
   }
   return result;

   LIBFUNC_EXIT
}

LIBFUNC_H3
(
    static long, _Assl_read,
    struct Assl *, assl,   A0,
    char *,        buffer, A1,
    long,          length, D0,
    ASSL_TYPE, ASSL_NAME
)
{
   LIBFUNC_INIT

   long result=-1;
   if(assl && assl->amisslbase)
   {
      result=SSL_read(assl->ssl,buffer,length);
   }
   return result;

   LIBFUNC_EXIT
}

LIBFUNC_H1
(
    static char *, _Assl_getcipher,
    struct Assl *, assl, A0,
    ASSL_TYPE, ASSL_NAME
)
{
   LIBFUNC_INIT

   char *result=NULL;
   if(assl && assl->amisslbase)
   {
      result=(char *)SSL_get_cipher(assl->ssl);
   }
   return result;

   LIBFUNC_EXIT
}

LIBFUNC_H1
(
    static char *, _Assl_libname,
    struct Assl *, assl, A0,
    ASSL_TYPE, ASSL_NAME

)
{
   LIBFUNC_INIT

   char *result=NULL;
   if(assl && assl->amisslbase)
   {
       result=AmiSSLBase?(char *)AmiSSLBase->lib_IdString:NULL;
   }
   return result;

   LIBFUNC_EXIT
}
#if !defined(__amigaos4__)
static void _Assl_dummy(void){}
#endif

/*--------------------------------------------------------------------*/
/* Library management functions */

LIBFUNC_H1
(
    struct Library *, _Assl_Open,
    long,             version, D0,
    LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT

#if defined(__amigaos4__)
    struct Library *awebamisslbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
    struct Library *awebamisslbase = LIBMAN_NAME;
#endif

    awebamisslbase->lib_OpenCnt++;
    return awebamisslbase;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
    long, _Assl_Close,
    LIBMAN_TYPE, LIBMAN_NAME
)

{
    LIBFUNC_INIT

#if defined(__amigaos4__)
    struct Library *awebamisslbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
    struct Library *awebamisslbase = LIBMAN_NAME;
#endif

    awebamisslbase->lib_OpenCnt--;
    return 0;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
    long, _Assl_Expunge,
    LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT
#if defined(__amigaos4__)
    struct Library *awebamisslbase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
    struct Library *awebamisslbase = LIBMAN_NAME;
#endif

    if(awebamisslbase->lib_OpenCnt == 0)
    {
        Remove((struct Node *)awebamisslbase);
    }
    return 0;

    LIBFUNC_EXIT
}

long _Assl_Extfunc(void)
{
    return 0;
}


/*--------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/

#define ASSLVERSION      1
#define ASSLREVISION     0
#define ASSLVERSIONSTR   "1.0"


static UBYTE version[]="awebamissl.library";
#if defined(__amigaos4__)
static UBYTE idstring[]= "awebamissl " ASSLVERSIONSTR " " __AMIGADATE__ " " CPU;
#endif

#if defined (__amigaos4__)
/* Make an OS4.x Library */

struct Library *awebamisslbase = NULL;
static struct Library *aasbKeeper = NULL;

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
    libBase->lib_Version      = ASSLVERSION;
    libBase->lib_Revision     = ASSLREVISION;
    libBase->lib_IdString     = idstring;

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
    (void *)_Assl_Open,
    (void *)_Assl_Close,
    (void *)_Assl_Expunge,
    (void *)0,
    (void *)-1,
};

static LONG _Assl_Obtain(struct AwebAmiSSLIFace *Self)
{
    return Self->Data.RefCount++;
}

static ULONG _Assl_Release(struct AwebAmiSSLIFace *Self)
{
    return Self->Data.RefCount--;
}


static void *main_vectors[] = {
        (void *)_Assl_Obtain,
        (void *)_Assl_Release,
        (void *)NULL,
        (void *)NULL,
        (void *)_Assl_cleanup,
        (void *)_Assl_openssl,
        (void *)_Assl_closessl,
        (void *)_Assl_connect,
        (void *)_Assl_geterror,
        (void *)_Assl_write,
        (void *)_Assl_read,
        (void *)_Assl_getcipher,
        (void *)_Assl_libname,
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

BOOL Initawebamissl()
{

    if (!Initawebamisslsema())
    {
        return FALSE;
    }
    if ((awebamissllib = CreateLibrary((struct TagItem *)libCreateTags)))
    {
        AddLibrary(awebamissllib);
        /* open the library to prevent expunge */
        aasbKeeper = OpenLibrary("awebamissl.library",0);
        /* open again for out global base */
        return TRUE;
    }
    return FALSE;
}

void Freeawebamissl()
{
    if(aasbKeeper){
        CloseLibrary(aasbKeeper);
        aasbKeeper = NULL;
    }

    if(awebamissllib)
    {
        Forbid();

        while(awebamissllib->lib_OpenCnt>0)
        {
            Permit();

            Lowlevelreq(AWEBSTR(MSG_ERROR_CANTQUIT), "awebamissl.library");

            Forbid();
        }


        RemLibrary(awebamissllib);
        Permit();

        DeleteLibrary(awebamissllib);
        awebamissllib = NULL;
    }

    Closeamissl();
    Freeawebamisslsema();
}



#else


#if defined(__MORPHOS__)
STATIC struct EmulLibEntry Trap_Assl_cleanup = { TRAP_LIBNR, 0, _Assl_cleanup };
STATIC struct EmulLibEntry Trap_Assl_openssl = { TRAP_LIB, 0, _Assl_openssl };
STATIC struct EmulLibEntry Trap_Assl_closessl = { TRAP_LIBNR, 0, _Assl_closessl };
STATIC struct EmulLibEntry Trap_Assl_connect = { TRAP_LIB, 0, _Assl_connect };
STATIC struct EmulLibEntry Trap_Assl_geterror = { TRAP_LIB, 0, _Assl_geterror };
STATIC struct EmulLibEntry Trap_Assl_write = { TRAP_LIB, 0, _Assl_write };
STATIC struct EmulLibEntry Trap_Assl_read = { TRAP_LIB, 0, _Assl_read };
STATIC struct EmulLibEntry Trap_Assl_getcipher = { TRAP_LIB, 0, _Assl_getcipher };
STATIC struct EmulLibEntry Trap_Assl_libname = { TRAP_LIB, 0, _Assl_libname };
#define _Assl_cleanup &Trap_Assl_cleanup
#define _Assl_openssl &Trap_Assl_openssl
#define _Assl_closessl &Trap_Assl_closessl
#define _Assl_connect &Trap_Assl_connect
#define _Assl_geterror &Trap_Assl_geterror
#define _Assl_write &Trap_Assl_write
#define _Assl_read &Trap_Assl_read
#define _Assl_getcipher &Trap_Assl_getcipher
#define _Assl_libname &Trap_Assl_libname
#endif

#if !defined(__SASC) && defined(__PPC__)
#pragma pack(2)
#endif

struct Jumptab
{  UWORD jmp;
   void *function;
};
#define JMP 0x4ef9
struct awebamisslbase
{
   struct Jumptab   table[13];
   struct Library library;
};

#if !defined(__SASC) && defined(__PPC__)
#pragma pack()
#endif

static struct awebamisslbase _awebamissllib =
{
   {
      { JMP,_Assl_libname },
      { JMP,_Assl_getcipher },
      { JMP,_Assl_read },
      { JMP,_Assl_write },
      { JMP,_Assl_geterror },
      { JMP,_Assl_connect },
      { JMP,_Assl_closessl },
      { JMP,_Assl_openssl },
      { JMP,_Assl_cleanup },
      { JMP,_Assl_dummy }, /* Extfunc */
      { JMP,_Assl_dummy }, /* Expunge */
      { JMP,_Assl_dummy }, /* Close */
      { JMP,_Assl_dummy }  /* Open */
   },

   {
      {  NULL,NULL,NT_LIBRARY,0,version },
      0,0,
      sizeof(struct Jumptab) * 13,
      sizeof(struct Library),
      1,0,
        version,
      0,0
   }
};

extern struct Library *AwebAmiSslBase;

BOOL Initawebamissl()
{

    if (!Initawebamisslsema())
    {
        return FALSE;
    }
AwebAmiSslBase=&_awebamissllib.library;

    return TRUE;
}

void Freeawebamissl()
{
    Closeamissl();
    Freeawebamisslsema();
}


#endif // (!__amigaos4__)
