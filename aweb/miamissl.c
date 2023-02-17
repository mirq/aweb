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

/* miamissl.c - AWeb SSL function library. Compile this with MiamiSSL SDK */

#if !( defined(__amigaos4__)  || defined(__MORPHOS__) )

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <proto/exec.h>
#include <proto/awebssl.h>
#include "aweb.h"
#include "task.h"
#include "http.h"
#define NO_BLOWFISH
#include <proto/miamissl.h>
#include <proto/miami.h>
#include <libraries/miamissl.h>
#include <ssl.h>
#include <err.h>
#include <crypto.h>

/*-----------------------------------------------------------------------*/

struct Assl
{  struct Library *miamibase;
   struct Library *miamisslbase;
   SSL_CTX *sslctx;
   SSL *ssl;
   UBYTE *hostname;
   BOOL denied;
};

/*-----------------------------------------------------------------------*/

struct Assl *Assl_initmiamissl(void)
{  struct Library *MiamiBase;
   struct Assl *assl=NULL;
   if(assl=ALLOCSTRUCT(Assl,1,MEMF_CLEAR))
   {  if(MiamiBase=assl->miamibase=OpenLibrary("miami.library",7))
      {  assl->miamisslbase=MiamiOpenSSL(0);
         if(!assl->miamisslbase)
         {  CloseLibrary(MiamiBase);
            assl->miamibase=NULL;
         }
      }
      if(!assl->miamibase)
      {  FREE(assl);
         assl=NULL;
      }
   }
   return assl;
}

/*-----------------------------------------------------------------------*/
static int __saveds __stdargs
   Certcallback(int ok,X509 *xs,X509 *xi,int depth,int error,char *arg)
{  char *s=NULL,*u=NULL;
   struct Assl *assl;
   if(!ok)
   {  assl=Gettaskuserdata();
      if(assl)
      {  struct Library *MiamiSSLBase=assl->miamisslbase;
         s=(char *)X509_NAME_oneline(X509_get_subject_name(xs));
         u=assl->hostname;
         ok=Httpcertaccept(u,s);
         if(!ok) assl->denied=TRUE;
         if(s) S_Free(s);
      }
   }
   return ok;
}

USRFUNC_H1
(
void  , Asslm_cleanup,
struct Assl *,assl,A0
)
{
    USRFUNC_INIT
  if(assl)
   {  if(assl->miamisslbase)
      {  struct Library *MiamiBase=assl->miamibase;
         MiamiCloseSSL();
      }
      if(assl->miamibase)
      {  CloseLibrary(assl->miamibase);
      }
      FREE(assl);
   }

    USRFUNC_EXIT
}

USRFUNC_H1
(
BOOL  , Asslm_openssl,
struct Assl *,assl,A0
)
{
    USRFUNC_INIT
  if(assl && assl->miamisslbase)
   {  struct Library *MiamiSSLBase=assl->miamisslbase;
      if(assl->sslctx=SSL_CTX_new())
      {  SSL_set_default_verify_paths(assl->sslctx);
         if(assl->ssl=SSL_new(assl->sslctx))
         {  Settaskuserdata(assl);
            SSL_set_verify(assl->ssl,SSL_VERIFY_FAIL_IF_NO_PEER_CERT,(APTR) Certcallback);
         }
      }
   }
   return (BOOL)(assl && assl->ssl);

    USRFUNC_EXIT
}

USRFUNC_H1
(
void  , Asslm_closessl,
struct Assl *,assl,A0
)
{
    USRFUNC_INIT
  if(assl && assl->miamisslbase)
   {  struct Library *MiamiSSLBase=assl->miamisslbase;
      if(assl->ssl)
      {  SSL_free(assl->ssl);
         assl->ssl=NULL;
      }
      if(assl->sslctx)
      {  SSL_CTX_free(assl->sslctx);
         assl->sslctx=NULL;
      }
   }

    USRFUNC_EXIT
}

USRFUNC_H3
(
long  , Asslm_connect,
struct Assl *,assl,A0,
long ,sock,D0,
UBYTE *,hostname,A1
)
{
    USRFUNC_INIT
  long result=ASSLCONNECT_FAIL;
   if(assl && assl->miamisslbase)
   {  struct Library *MiamiSSLBase=assl->miamisslbase;
      assl->hostname=hostname;
      if(SSL_set_fd(assl->ssl,sock))
      {  if(SSL_connect(assl->ssl)>0)
         {  result=ASSLCONNECT_OK;
         }
         else if(assl->denied)
         {  result=ASSLCONNECT_DENIED;
         }
      }
   }
   return result;

    USRFUNC_EXIT
}

USRFUNC_H2
(
char * , Asslm_geterror,
struct Assl *,assl,A0,
char *,errbuf,A1
)
{
    USRFUNC_INIT
  long err;
   UBYTE *p=NULL;
   short i;
   if(assl && assl->miamisslbase)
   {  struct Library *MiamiSSLBase=assl->miamisslbase;
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

    USRFUNC_EXIT
}

USRFUNC_H3
(
long  , Asslm_write,
struct Assl *,assl,A0,
char *,buffer,A1,
long ,length,D0
)
{
    USRFUNC_INIT
  long result=-1;
   if(assl && assl->miamisslbase)
   {  struct Library *MiamiSSLBase=assl->miamisslbase;
      result=SSL_write(assl->ssl,buffer,length);
   }
   return result;

    USRFUNC_EXIT
}

USRFUNC_H3
(
long  , Asslm_read,
struct Assl *,assl,A0,
char *,buffer,A1,
long ,length,D0
)
{
    USRFUNC_INIT
  long result=-1;
   if(assl && assl->miamisslbase)
   {  struct Library *MiamiSSLBase=assl->miamisslbase;
      result=SSL_read(assl->ssl,buffer,length);
   }
   return result;

    USRFUNC_EXIT
}

USRFUNC_H1
(
char * , Asslm_getcipher,
struct Assl *,assl,A0
)
{
    USRFUNC_INIT
  char *result=NULL;
   if(assl && assl->miamisslbase)
   {  struct Library *MiamiSSLBase=assl->miamisslbase;
      result=SSL_get_cipher(assl->ssl);
   }
   return result;

    USRFUNC_EXIT
}

USRFUNC_H1
(
char * , Asslm_libname,
struct Assl *,assl,A0
)
{
    USRFUNC_INIT
  char *result=NULL;
   if(assl && assl->miamisslbase)
   {  struct Library *MiamiSSLBase=assl->miamisslbase;
      result=(char *)MiamiSSLBase->lib_IdString;
   }
   return result;

    USRFUNC_EXIT
}

USRFUNC_H0(void, Asslm_dummy)
{
    USRFUNC_INIT

    USRFUNC_EXIT
}

/*-----------------------------------------------------------------------*/

static UBYTE version[]="AwebMiamiSSL.library";

struct Jumptab
{  UWORD jmp;
   void *function;
};
#define JMP 0x4ef9

struct Jumptab miamissl_jumptab[]=
{
   {JMP,Asslm_libname},
   {JMP,Asslm_getcipher},
   {JMP,Asslm_read},
   {JMP,Asslm_write},
   {JMP,Asslm_geterror},
   {JMP,Asslm_connect},
   {JMP,Asslm_closessl},
   {JMP,Asslm_openssl},
   {JMP,Asslm_cleanup},
   {JMP,Asslm_dummy}, /* Extfunc */
   {JMP,Asslm_dummy}, /* Expunge */
   {JMP,Asslm_dummy}, /* Close */
   {JMP,Asslm_dummy}  /* Open */
};
static struct Library awebmiamissllib=
{  {  NULL,NULL,NT_LIBRARY,0,version },
   0,0,
   sizeof(miamissl_jumptab),
   sizeof(struct Library),
   1,0,
   version,
   0,0
};

struct Library *AwebMiamisslBase=&awebmiamissllib;

#endif
