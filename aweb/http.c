
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

/* http.c - aweb http protocol client */

#include "aweb.h"
#include "tcperr.h"
#include "fetch.h"
#include "fetchdriver.h"
#include "application.h"
#include "task.h"
#include "form.h"
#include "idn.h"

#include "awebtcp.h"
#include "zlib.h"

#include <dos/dosextens.h>

#include <proto/exec.h>
#include <proto/amissl.h>
#include <proto/amisslmaster.h>
#include <amissl/amissl.h>
#include <libraries/amisslmaster.h>
#include <libraries/amissl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#ifndef LOCALONLY

/* SSL context structure for AmiSSL */
struct HttpSSL
{
    struct Library *amisslmaster_base;
    struct Library *amissl_base;
#if defined(__amigaos4__)
    struct AmiSSLMasterIFace *iamisslmaster;
    struct AmiSSLIFace *iamissl;
#endif
    SSL_CTX *ctx;
    SSL *ssl;
    BOOL initialized;
};

struct Httpinfo
{
    long            status;     /* Response status */
    UWORD           flags;
    struct Authorize *prxauth;  /* Proxy authorization */
    struct Authorize *auth;     /* Normal authorization */
    UBYTE          *connect;    /* Connect to this host or proxy */
    long            port;       /* .. using this port. -1 means use default (80/443) */
    UBYTE          *tunnel;     /* Host and port to tunnel to */
    UBYTE          *hostport;   /* Host and port for use in Host: header */
    UBYTE          *hostname;   /* Host name to match authorization for */
    UBYTE          *abspath;    /* Abs path, or full url, to use in GET request */
    UBYTE          *boundary;   /* Multipart boundary including leading "--" */
    struct Fetchdriver *fd;
    struct Library *socketbase;
    long            sock;
    struct HttpSSL *ssl;        /* AmiSSL context */
    long            blocklength;        /* Length of data in block */
    long            nextscanpos;        /* Block position to scan */
    long            linelength; /* Length of current header line */
    long            readheaders;        /* Number of header bytes read */
    ULONG           movedto;    /* AOURL_ tag if 301 302 303 307 status */
    UBYTE          *movedtourl; /* URL string moved to */
    UBYTE           parttype[32];       /* Content-type for this part */
    long            partlength; /* Content-length for this part */
    UBYTE          *userid;     /* Userid from URL */
    UBYTE          *passwd;     /* Password from URL */
};

#define HTTPIF_AUTH         0x0001       /* Tried with a known to be valid auth */
#define HTTPIF_PRXAUTH      0x0002       /* Tried with a known to be valid prxauth */
#define HTTPIF_HEADERS      0x0004       /* Doing headers, issue bytes read messages */
#define HTTPIF_SSL          0x0008       /* Use secure transfer */
#define HTTPIF_RETRYNOSSL   0x0010       /* Retry with no secure transfer */
#define HTTPIF_NOSSLREQ     0x0020       /* Don't put on another SSL requester */
#define HTTPIF_SSLTUNNEL    0x0040       /* Tunnel SSL request through proxy */
#define HTTPIF_TUNNELOK     0x0080       /* Tunnel response was ok */
#define HTTPIF_GZIPENCODED  0x0100       /* response is gzip encoded */
#define HTTPIF_GZIPDECODING 0x0200       /* decoding gziped response has begun */

/* SSL connection status codes */
#define ASSLCONNECT_OK      0
#define ASSLCONNECT_DENIED  2

/* Global AmiSSL variables */
static struct Library *LocalAmiSSLMasterBase = NULL;
static struct Library *LocalAmiSSLBase = NULL;
#if defined(__amigaos4__)
static struct AmiSSLMasterIFace *LocalIAmiSSLMaster = NULL;
static struct AmiSSLIFace *LocalIAmiSSL = NULL;
#endif
static BOOL LocalAmiSSLInitialized = FALSE;
static struct SignalSemaphore SSLSema;
static SSL_CTX *GlobalSSLContext = NULL;

static UBYTE   *httprequest = "GET %.7000s HTTP/1.0\r\n";

static UBYTE   *httppostrequest = "POST %.7000s HTTP/1.0\r\n";

#ifdef __MORPHOS__
static UBYTE   *useragent = "User-Agent: MorphOS-AWeb/%s\r\n";
#else
static UBYTE   *useragent = "User-Agent: Amiga-AWeb/%s\r\n";
#endif

#ifdef __MORPHOS__
static UBYTE   *useragentspoof =
    "User-Agent: %s; (Spoofed by MorphOS-AWeb/%s)\r\n";
#else
static UBYTE   *useragentspoof =
    "User-Agent: %s; (Spoofed by Amiga-AWeb/%s)\r\n";
#endif

static UBYTE   *fixedheaders = "Accept: */*;q=1\r\nAccept-Encoding: gzip\r\n";

//   "Accept: text/html;level=3, text/html;version=3.0, */*;q=1\r\n";

static UBYTE   *host = "Host: %s\r\n";

static UBYTE   *ifmodifiedsince = "If-modified-since: %s\r\n";

static UBYTE   *ifnonematch = "If-none-match: %s\r\n";          //gvb+

static UBYTE   *authorization = "Authorization: Basic %s\r\n";

static UBYTE   *proxyauthorization = "Proxy-Authorization: Basic %s\r\n";

static UBYTE   *nocache = "Pragma: no-cache\r\n";

static UBYTE   *referer = "Referer: %s\r\n";

static UBYTE   *httppostcontent =
    "Content-Length: %d\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n";

static UBYTE   *httpmultipartcontent =
    "Content-Length: %d\r\n"
    "Content-Type: multipart/form-data; boundary=%s\r\n";

static UBYTE   *tunnelrequest = "CONNECT %.200s HTTP/1.0\r\n";

/* Unverifyable certificates that the user accepted */
struct Certaccept
{
    NODE(Certaccept);
    UBYTE          *hostname;
    UBYTE          *certname;
};

static LIST(Certaccept) certaccepts;
    static struct SignalSemaphore certsema;

#ifdef DEVELOPER
BOOL charsetdebug=FALSE;
#endif

/*-----------------------------------------------------------------------*/

    static void Messageread(struct Fetchdriver *fd, long n)
{
    UBYTE           buf[64];

    strcpy(buf, AWEBSTR(MSG_AWEB_BYTESREAD));
    strcat(buf, ": ");
    sprintf(buf + strlen(buf), "%ld", n);
    Updatetaskattrs(AOURL_Status, buf, TAG_END);
}

/*-----------------------------------------------------------------------*/
/* AmiSSL initialization and cleanup functions */
/*-----------------------------------------------------------------------*/

#if defined(__amigaos4__)
#define GETINTERFACE(iface, base) (iface = (APTR)GetInterface((struct Library *)(base), "main", 1L, NULL))
#define DROPINTERFACE(iface)      (DropInterface((struct Interface *)iface), iface = NULL)
#else
#define GETINTERFACE(iface, base) TRUE
#define DROPINTERFACE(iface)
#endif

/* Forward declarations */
static void CleanupAmiSSLLibraries(void);

/* Initialize AmiSSL libraries */
static BOOL InitAmiSSLLibraries(void)
{
    LocalAmiSSLInitialized = FALSE;

    if (!(LocalAmiSSLMasterBase = OpenLibrary("amisslmaster.library", AMISSLMASTER_MIN_VERSION)))
        return FALSE;
    else if (!GETINTERFACE(LocalIAmiSSLMaster, LocalAmiSSLMasterBase))
        return FALSE;
    else if (!InitAmiSSLMaster(AMISSL_CURRENT_VERSION, TRUE))
        return FALSE;
    else if (!(LocalAmiSSLBase = OpenAmiSSL()))
        return FALSE;
    else if (!GETINTERFACE(LocalIAmiSSL, LocalAmiSSLBase))
        return FALSE;
#if defined(__amigaos4__)
    else if (InitAmiSSL(AmiSSL_ErrNoPtr, &errno,
                        TAG_DONE) != 0)
#else
    else if (InitAmiSSL(AmiSSL_ErrNoPtr, &errno,
                        TAG_DONE) != 0)
#endif
        return FALSE;
    else
        LocalAmiSSLInitialized = TRUE;

    if (!LocalAmiSSLInitialized)
        CleanupAmiSSLLibraries();

    return LocalAmiSSLInitialized;
}

/* Cleanup AmiSSL libraries */
static void CleanupAmiSSLLibraries(void)
{
    if (LocalAmiSSLInitialized)
    {
        CleanupAmiSSLA(NULL);
        LocalAmiSSLInitialized = FALSE;
    }

    if (LocalAmiSSLBase)
    {
        DROPINTERFACE(LocalIAmiSSL);
        CloseAmiSSL();
        LocalAmiSSLBase = NULL;
    }

    DROPINTERFACE(LocalIAmiSSLMaster);
    CloseLibrary(LocalAmiSSLMasterBase);
    LocalAmiSSLMasterBase = NULL;
}

/* Create SSL context */
static SSL_CTX *CreateSSLContext(void)
{
    SSL_CTX *ctx;

    if (!InitAmiSSLLibraries())
        return NULL;

    OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT | OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS, NULL);

    if ((ctx = SSL_CTX_new(TLS_client_method())) != NULL)
    {
        SSL_CTX_set_default_verify_paths(ctx);
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    }

    return ctx;
}

/* Create new SSL connection */
static struct HttpSSL *CreateHTTPSSL(void)
{
    struct HttpSSL *httpssl;

    if (!(httpssl = ALLOCTYPE(struct HttpSSL, 1, MEMF_CLEAR)))
        return NULL;

    ObtainSemaphore(&SSLSema);

    if (!GlobalSSLContext)
        GlobalSSLContext = CreateSSLContext();

    if (GlobalSSLContext && (httpssl->ssl = SSL_new(GlobalSSLContext)))
    {
        httpssl->ctx = GlobalSSLContext;
        httpssl->initialized = TRUE;
    }
    else
    {
        FREE(httpssl);
        httpssl = NULL;
    }

    ReleaseSemaphore(&SSLSema);
    return httpssl;
}

/* Cleanup SSL connection */
static void FreeHTTPSSL(struct HttpSSL *httpssl)
{
    if (httpssl)
    {
        if (httpssl->ssl)
        {
            SSL_shutdown(httpssl->ssl);
            SSL_free(httpssl->ssl);
        }
        FREE(httpssl);
    }
}

/*-----------------------------------------------------------------------*/

/* escape non acceptable characters in path */

static BOOL Isnoturl (UBYTE c) {

    if (Isspace(c)) return TRUE;
    if (c >= 128)  return TRUE;
    if ((c == '<') ||
        (c == '>') ||
        (c == '"') ||
        (c == '#') ||
        (c == '%'))
        return TRUE;

    return FALSE;

}

static void Urlescape(struct Buffer *buf,UBYTE *p,long len)
{  UBYTE *q=p+len;
   UBYTE enc[4];

   if(len<=0){
      Addtobuffer(buf,"",-1);
   }
   while(p<q)
   {
      if( (*p == '%') && isxdigit(*(p+1)) && isxdigit(*(p+2)) )
      {
         Addtobuffer(buf,p,1);
      }
      else if(Isnoturl(*p))
      {
         sprintf(enc,"%%%02X",*p);
         Addtobuffer(buf,enc,3);

      }
      else
      {
         Addtobuffer(buf,p,1);
      }
      p++;
   }
}


static BOOL Makehttpaddr(struct Httpinfo *hi, UBYTE * proxy, UBYTE * url,
                         BOOL ssl)
{
    UBYTE          *p, *q, *r, *u, *ace;
    UBYTE          *userid = NULL, *passwd = NULL;
    long            l;
    BOOL            gotport = FALSE;
    struct Buffer buf = {0};
    /* we should never be called with a null url but ... */
    if (!url) return FALSE;

    if (u = strchr(url, ':'))
        u++;                    /* Should always be found */
    else
        u = url;
    if (u[0] == '/' && u[1] == '/')
        u += 2;
    if (proxy)
    {
        p = strchr(proxy, ':');
        hi->connect = Dupstr(proxy, p ? p - proxy : -1);
        hi->port = p ? atol(p + 1) : 8080;
        p = stpbrk(u, ":/");
        if (p && *p == ':' && (q = strchr(p, '@')) &&
            (!(r = strchr(p, '/')) || q < r))
        {                       /* userid:passwd@host[:port][/path] */
            userid = Dupstr(u, p - u);
            passwd = Dupstr(p + 1, q - p - 1);
            u = q + 1;
            p = stpbrk(u, ":/");
        }
        r = Dupstr(u, p ? p-u : -1);
        /* IDNA for HTTP header, proxied */
//      PutStr("Proxied HTTP\n");
        ace = Dorfc3490(r, IDN_ENCODE);
        FREE(r);
        hi->hostname = ace;
        gotport = (p && *p == ':');
        p = strchr(u, '/');
        hi->hostport = Dupstr(u, p ? p - u : -1);
        if (ssl)
        {                       /* Will be tunneled. Use abspath like with no proxy */
            if (gotport)
            {
                hi->tunnel = Dupstr(hi->hostport, -1);
            }
            else
            {
                hi->tunnel = ALLOCTYPE(UBYTE, strlen(hi->hostname) + 5, 0);
                if (hi->tunnel)
                {
                    strcpy(hi->tunnel, hi->hostname);
                    strcat(hi->tunnel, ":443");
                }
            }
            hi->abspath = Dupstr(p ? p : (UBYTE *) "/", -1);
            hi->flags |= HTTPIF_SSLTUNNEL;
        } /* if (ssl) */
        else
        {
            if (p)
            {
                hi->abspath = Dupstr(url, -1);
            }
            else
            {                   /* append '/' */
                l = strlen(url);
                if (hi->abspath = Dupstr(url, l + 1))
                    hi->abspath[l] = '/';
            }
        }
    } /* if (proxy) */
    else
    {
        p = stpbrk(u, ":/");
        if (p && *p == ':' && (q = strchr(p, '@')) &&
            (!(r = strchr(p, '/')) || q < r))
        {                       /* userid:password@host[:port][/path] */
            userid = Dupstr(u, p - u);
            passwd = Dupstr(p + 1, q - p - 1);
            u = q + 1;
            p = stpbrk(u, ":/");
        }
        r = Dupstr(u, p ? p-u : -1);
        /* IDNA for DNS moved to nameserv.c */
//      printf("setting connect() target to %s\n",r);
        hi->connect = r;
        if (p && *p == ':')
        {
            hi->port = atol(p + 1);
        }
        else
        {
            hi->port = -1;
        }

        p = strchr(u, '/');
        r = Dupstr(u, p ? p - u : -1);
        /* IDNA for Host: header */
//      PutStr("encoding for Host: header\n");
        hi->hostport = Dorfc3490(r, IDN_ENCODE);
        FREE(r);
        hi->hostname = Dorfc3490(hi->connect, IDN_ENCODE);
        hi->abspath = Dupstr(p ? p : (UBYTE *) "/", -1);
    }
    if (userid && passwd)
    {
        if (hi->auth)
            Freeauthorize(hi->auth);
        if (hi->auth = Newauthorize(hi->hostport, "dummyrealm"))
        {
            Setauthorize(hi->auth, userid, passwd);
            hi->flags |= HTTPIF_AUTH;
        }
    }
        if (userid)
           FREE(userid);
        if (passwd)
           FREE(passwd);

    if(hi->abspath)
    {
        Urlescape(&buf,hi->abspath,strlen(hi->abspath));
        FREE(hi->abspath);
        hi->abspath = Dupstr(buf.buffer,buf.length);
        Freebuffer(&buf);
    }


    return (BOOL) (hi->connect && hi->hostport && hi->abspath &&
                   hi->hostname);
}

/* Build a HTTP request. The length is returned.
 * (*request) is either fd->block or a dynamic string if fd->block was too small */
static long Buildrequest(struct Fetchdriver *fd, struct Httpinfo *hi,
                         UBYTE ** request)
{
    UBYTE          *p = fd->block;
    UBYTE          *cookies;
    struct Buffer *bf;
    struct Buffer encbuf = {0};

    *request = fd->block;
    if (fd->postmsg || fd->multipart)
        p += sprintf(p, httppostrequest, hi->abspath);
    else
        p += sprintf(p, httprequest, hi->abspath);
    ObtainSemaphore(&prefssema);
    if (*prefs.network.spoofid)
    {
        p += sprintf(p, useragentspoof, prefs.network.spoofid, awebversion);
    }
    else
    {
        p += sprintf(p, useragent, awebversion);
    }
    ReleaseSemaphore(&prefssema);
    p += sprintf(p, fixedheaders);
    if (hi->hostport)
        p += sprintf(p, host, hi->hostport);

    // The clean solution would be to only send
    // the etag header and not the if-modified, if an etag exists.
    // But currently we need to send both headers as we do not support HTTP 1.1
    // Apache accepts ETAG with protocol 1.0 but IIS will ignore them
    //
    // If Etag exists verify this else try time
    if (fd->etag && strlen(fd->etag)>0)
    {
       p += sprintf(p, ifnonematch, fd->etag);
    }
    // send if-modified header
    if (fd->validate)
    {
          UBYTE           date[32];

          Makedate(fd->validate, date);
          p += sprintf(p, ifmodifiedsince, date);

    }



    if (hi->auth && hi->auth->cookie)
        p += sprintf(p, authorization, hi->auth->cookie);
    if (hi->prxauth && hi->prxauth->cookie)
        p += sprintf(p, proxyauthorization, hi->prxauth->cookie);
    if (fd->flags & FDVF_NOCACHE)
        p += sprintf(p, nocache);
    if (fd->referer && (p - fd->block) + strlen(fd->referer) < 7000) {
                /* hacking the referer to ACE, fixing auth passing on the way */
                UBYTE *q, *r, *s;
        /* the code below assumes http:// or htpps:// */
        /* so make sure that's what we've got! */
        /* if not just leave the referer alone */

        q = strchr(fd->referer,':');

        if(q++ && *q++ == '/' && *q == '/' )
        {

            if (bf = ALLOCSTRUCT(Buffer, 1, 0)) {
                    q = strchr(fd->referer, '/'); /* http:_/_/w... */
                    q++, q++;       /* http://_w_... */
                    if (Addtobuffer(bf, fd->referer, q - fd->referer)){
                            /* protocol went ok, continue */
                            r = strchr(q, '/');
                            if (r == NULL) {
                                    /* There is always at least a virtual / */
                                    r = q + strlen(q);
                            }
                            s = strchr(q,'@');
                            if (s && s < r) {
                                    q = ++s; /* first char behind @ */
                            }
                            s = Dupstr(q, r-q);
                            q = Dorfc3490(s, IDN_ENCODE);
                            FREE(s);
                            if (!Addtobuffer(bf, q, -1)) {
                                    /* phew! */
                            }
                            FREE(q);
                            Urlescape(&encbuf,r,strlen(r));
                            if (!Addtobuffer(bf, encbuf.buffer, -1)) {
                                    /* erm... */
                            }
                            Freebuffer(&encbuf);
                    }
                    p += sprintf(p, referer, bf->buffer);
                    Freebuffer(bf);
                    FREE(bf);
            }
        }
    }
    if (fd->multipart)
    {
        p += sprintf(p, httpmultipartcontent,
                     fd->multipart->length, fd->multipart->buf.buffer);
    }
    else if (fd->postmsg)
    {
        p += sprintf(p, httppostcontent, strlen(fd->postmsg));
    }
    if (prefs.network.cookies &&
        (cookies = Findcookies(fd->name, hi->flags & HTTPIF_SSL)))
    {
        long            len = strlen(cookies);

        if ((p - fd->block) + len < 7000)
        {
            strcpy(p, cookies);
            p += len;
        }
        else
        {
            UBYTE          *newreq =
                ALLOCTYPE(UBYTE, (p - fd->block) + len + 16, 0);
            if (newreq)
            {
                strcpy(newreq, fd->block);
                strcpy(newreq + (p - fd->block), cookies);
                *request = newreq;
                p = newreq + (p - fd->block) + len;
            }
        }
        FREE(cookies);
    }
    p += sprintf(p, "\r\n");
    return p - *request;
}

/*-----------------------------------------------------------------------*/

/* Receive a block through SSL or socket. */
static long Receive(struct Httpinfo *hi, UBYTE * buffer, long length)
{
    long            result;

    if (hi->flags & HTTPIF_SSL)
    {
        result = SSL_read(hi->ssl->ssl, buffer, length);
    }
    else
    {
        result = a_recv(hi->sock, buffer, length, 0, hi->socketbase);
    }
    return result;
}

/* Read remainder of block. Returns FALSE if eof or error. */
static BOOL Readblock(struct Httpinfo *hi)
{
    long            n;

#ifdef DEVELOPER
    UBYTE          *block;

    if (!hi->socketbase)
    {
        block =
            fgets(hi->fd->block + hi->blocklength,
                  hi->fd->blocksize - hi->blocklength, (FILE *) hi->sock);
        n = block ? strlen(block) : 0;
    /*
       for some reason, we get a bogus 'G' in the second console window
     */
        if (STRNEQUAL(hi->fd->block, "GHTTP/", 6))
        {
            memmove(hi->fd->block, hi->fd->block + 1, n - 1);
            n--;
        }
    }
    else
#endif
        n = Receive(hi, hi->fd->block + hi->blocklength,
                    hi->fd->blocksize - hi->blocklength);
    if (n < 0 || Checktaskbreak())
    {

/* Don't send error, let source driver keep its partial data if it wants to.
      Updatetaskattrs(
         AOURL_Error,TRUE,
         TAG_END);
*/
        return FALSE;
    }
    if (n == 0)
        return FALSE;
    hi->blocklength += n;
    if (hi->flags & HTTPIF_HEADERS)
    {
        Messageread(hi->fd, hi->readheaders += n);
    }
    return TRUE;
}

/* Remove the first part from the block. */
static void Nextline(struct Httpinfo *hi)
{
    if (hi->nextscanpos < hi->blocklength)
    {
        memmove(hi->fd->block, hi->fd->block + hi->nextscanpos,
                hi->blocklength - hi->nextscanpos);
    }
    hi->blocklength -= hi->nextscanpos;
    hi->nextscanpos = 0;
}

/* Find a complete line. Read again if no complete line found. */
static BOOL Findline(struct Httpinfo *hi)
{
    UBYTE          *p = hi->fd->block;
    UBYTE          *end;

    for (;;)
    {
        end = hi->fd->block + hi->blocklength;
        while (p < end && *p != '\n')
            p++;
        if (p < end)
            break;
        if (!Readblock(hi))
            return FALSE;
    }
/*
   Now we've got a LF. Terminate line here, but if it is preceded by CR ignore that too.
 */
    *p = '\0';
    hi->linelength = p - hi->fd->block;
    hi->nextscanpos = hi->linelength + 1;
    if (hi->linelength)
    {
        p--;
        if (*p == '\r')
        {
            *p = '\0';
            hi->linelength--;
        }
    }
    if (httpdebug)
    {
        Write(Output(), hi->fd->block, hi->linelength);
        Write(Output(), "\n", 1);
    }
    return TRUE;
}

/* Get the authorization details from this buffer */
static struct Authorize *Parseauth(UBYTE * buf, UBYTE * server)
{
    UBYTE          *p, *q;
    struct Authorize *auth;

    for (p = buf; *p == ' '; p++);
    if (!STRNIEQUAL(p, "Basic ", 6))
        return NULL;
    for (p += 6; *p == ' '; p++);
    if (!STRNIEQUAL(p, "realm", 5))
        return NULL;
    for (p += 5; *p == ' '; p++);
    if (*p != '=')
        return FALSE;
    for (p++; *p == ' '; p++);
    if (*p != '"')
        return FALSE;
    q = p + 1;
    for (p++; *p != '"' && *p != '\r' && *p != '\n'; p++);
    *p = '\0';
    auth = Newauthorize(server, q);
    return auth;
}

/* Read and process headers until end of headers. Read when necessary.
 * Returns FALSE if eof or error, or data should be skipped. */
static BOOL Readheaders(struct Httpinfo *hi)
{
    hi->flags &= ~(HTTPIF_GZIPENCODED | HTTPIF_GZIPDECODING);

    for (;;)
    {
        if (!Findline(hi))
            return FALSE;
        if (hi->linelength == 0)
        {
            if (hi->status)
                return FALSE;
            else
                return TRUE;
        }
        Updatetaskattrs(AOURL_Header, hi->fd->block, TAG_END);
        if (STRNIEQUAL(hi->fd->block, "Date:", 5))
        {
            hi->fd->serverdate = Scandate(hi->fd->block + 5);
            Updatetaskattrs(AOURL_Serverdate, hi->fd->serverdate, TAG_END);
        }
        else if (STRNIEQUAL(hi->fd->block, "Last-Modified:", 14))
        {
            ULONG           date = Scandate(hi->fd->block + 14);

            Updatetaskattrs(AOURL_Lastmodified, date, TAG_END);
        }
        else if (STRNIEQUAL(hi->fd->block, "Expires:", 8))
        {
            long            expires = Scandate(hi->fd->block + 8);

            Updatetaskattrs(AOURL_Expires, expires, TAG_END);
        }
        else if (STRNIEQUAL(hi->fd->block, "Content-Length:", 15))
        {
            long            i = 0;

            /* temporary work arround for unkown size of decoded gzip */

            if(!(hi->flags & HTTPIF_GZIPENCODED))
            {
                sscanf(hi->fd->block + 15, " %ld", &i);
                Updatetaskattrs(AOURL_Contentlength, i, TAG_END);
            }
        }

        else if (STRNIEQUAL(hi->fd->block, "ETag:", 5))
        {
                UBYTE          *p,*q;

            for (p = hi->fd->block + 5; *p && isspace(*p); p++);
            for (q = p; *q && !isspace(*q) && *q != ';'; q++);
            *q = '\0';
            if (q-p > 63) p[63] = '\0';
            Updatetaskattrs(AOURL_Etag, p, TAG_END);
//            printf("=> http found etag >%s<\n\n",p);
        }


        else if (STRNIEQUAL(hi->fd->block, "Content-Type:", 13))
        {
            UBYTE           mimetype[32] = "";
            UBYTE           charset[32] = "";
            BOOL            foreign = FALSE;

            if (!prefs.network.ignoremime)
            {
                UBYTE          *p, *q, *r;
                UBYTE           qq;
                long            l;
                BOOL            forward = TRUE;

                for (p = hi->fd->block + 13; *p && isspace(*p); p++);
                for (q = p; *q && !isspace(*q) && *q != ';'; q++);
                qq = *q;
                *q = '\0';
                l = q - p;
                if (qq && !hi->boundary)
                {
                    if (STRIEQUAL(p, "MULTIPART/X-MIXED-REPLACE")
                        || STRIEQUAL(p, "MULTIPART/MIXED-REPLACE"))
                    {
                        for (q++; *q && !STRNIEQUAL(q, "BOUNDARY=", 9); q++);
                        if (*q)
                        {
                            q += 9;
                            if (*q == '"')
                            {
                                q++;
                                for (r = q; *r && *r != '"'; r++);
                                *r = '\0';
                            }
                            if (hi->boundary = Dupstr(q - 2, -1))
                            {
                                hi->boundary[0] = '-';
                                hi->boundary[1] = '-';
                            }
                            forward = FALSE;
                        }
                    }
                }
                if (qq && STRNIEQUAL(p, "TEXT/", 5))
                {
                    for (q++; *q && !STRNIEQUAL(q, "CHARSET=", 8); q++);
                    if (*q)
                    {
                        q += 8;
                        while (*q && isspace(*q))
                            q++;
                        if (*q == '"')
                        {
                            q++;
                            for (r = q; *r && *r != '"'; r++);
                            *r = '\0';
                        }
                        else
                        {
                            for (r = q; *r && !isspace(*r); r++);
                            *r = '\0';
                        }
                        strcpy(charset,q);
#ifdef DEVELOPER
                        if (charsetdebug) printf ("http.c/Readheaders(): determined character set: %s\n",charset);
#endif
                        if (*q && !STRIEQUAL(q, "ISO-8859-1"))
                            foreign = TRUE;
                    }
#ifdef DEVELOPER
                    else
                        if (charsetdebug) printf ("http.c/Readheaders(): character set not specified\n");
#endif
                }
#ifdef DEVELOPER
                else
                    if (charsetdebug) printf ("http.c/Readheaders(): non-text document or no additional specification\n");
#endif
                if (forward)
                {
                    if (l > 31)
                        p[31] = '\0';
                    strcpy(mimetype, p);
                }
            }
            if (*mimetype)
            {
                Updatetaskattrs(AOURL_Contenttype, mimetype,
                                AOURL_Foreign, foreign,
                                AOURL_Charset, charset, TAG_END);
            }
        }
        else if (STRNIEQUAL(hi->fd->block, "Content-script-type:", 20))
        {
            UBYTE          *p, *q;

            for (p = hi->fd->block + 20; *p && isspace(*p); p++);
            for (q = p; *q && !isspace(*q) && *q != ';'; q++);
            *q = '\0';
            Updatetaskattrs(AOURL_Contentscripttype, p, TAG_END);
        }
        else if (STRNIEQUAL(hi->fd->block, "Pragma:", 7))
        {
            UBYTE          *p, *q;

            for (p = hi->fd->block + 7; *p && isspace(*p); p++);
            for (q = p; *q && !isspace(*q) && *q != ';'; q++);
            *q = '\0';
            if (STRIEQUAL(p, "no-cache"))
            {
                Updatetaskattrs(AOURL_Nocache, TRUE, TAG_END);
            }
        }
        else if (hi->movedto && STRNIEQUAL(hi->fd->block, "Location:", 9))
        {
            UBYTE          *p, *q;

            for (p = hi->fd->block + 9; *p && isspace(*p); p++);
            for (q = p + strlen(p) - 1; q > p && isspace(*q); q--);
            if (hi->movedtourl)
                FREE(hi->movedtourl);
            hi->movedtourl = Dupstr(p, q - p + 1);
        }
        else if (hi->status == 401 &&
                 STRNIEQUAL(hi->fd->block, "WWW-Authenticate:", 17))
        {
            struct Authorize *newauth =
                Parseauth(hi->fd->block + 17, hi->hostport);
            if (newauth)
            {
                if (hi->auth)
                    Freeauthorize(hi->auth);
                hi->auth = newauth;
            }
        }
        else if (hi->status == 407 &&
                 STRNIEQUAL(hi->fd->block, "Proxy-Authenticate:", 19) &&
                 hi->fd->proxy)
        {
            if (hi->prxauth)
                Freeauthorize(hi->prxauth);
            hi->prxauth = Parseauth(hi->fd->block + 19, hi->fd->proxy);
        }
        else if (STRNIEQUAL(hi->fd->block, "Set-Cookie:", 11))
        {
            if (prefs.network.cookies)
                Storecookie(hi->fd->name, hi->fd->block + 11,
                            hi->fd->serverdate,hi->fd->responsetime);
        }
        else if (STRNIEQUAL(hi->fd->block, "Refresh:", 8))
        {

            Updatetaskattrs(AOURL_Clientpull, hi->fd->block + 8,AOURL_Nocache ,TRUE,AOURL_Volatile,TRUE , TAG_END);
        }

        // The HTTP spec (RFC 2616) allows the following encoding values
        // gzip, x-gzip, deflate, compress, x-compress
        // the "x-" encodings are the old notations (same otherwise).
        // - gzip and deflate do compress better than compress
        // - gzip and deflate use the same algorythm
        // - but deflate uses a different header with a bit less overheat
        // - the spec is abit unprecisely worded. Sometimes they say gzip when they are talking about deflate.
        // Because of this its recommended to always use gzip instead of deflate to prevent
        // problems with apps having misread the spec.
        //
        // As Aweb only asks for 'gzip' encoding
        // the servers SHOULD only reply with 'Content-Encoding: gzip'
        // some buggy server might encode it correctly but reply with 'Content-Encoding: x-gzip'

        else if (STRNIEQUAL(hi->fd->block,"Content-Encoding:",17))
        {

          if(strstr(hi->fd->block + 18,"gzip"))
          {
              hi->flags |= HTTPIF_GZIPENCODED;
              Updatetaskattrs(AOURL_Contentlength,0,TAG_END);
          }
        }
        else if (STRNIEQUAL(hi->fd->block,"Content-Disposition:",20))
        {
            UBYTE *p,*q;

            for (p = hi->fd->block + 21; *p && isspace(*p); p++);
            for (q = p; *q && !isspace(*q) && *q != ';'; q++);
            *q = '\0';
            if (STRIEQUAL(p, "attachment"))
            {
                p += 11;
                if((q = strstr(p,"filename")))
                {
                    for (p = q + 8; *p && (isspace(*p) || *p == '"' || *p == '='); p++);
                    for (q = p; *q && !isspace(*q) && *q != ';' && *q != '"'; q++);
                    *q = '\0';
                    Updatetaskattrs(AOURL_Filename,p,TAG_END);
                }
            }

        }
        Nextline(hi);
    }
}

/* Read the HTTP response. Returns TRUE if HTTP, FALSE if plain response. */
static BOOL Readresponse(struct Httpinfo *hi)
{
    long            stat = 0;
    BOOL            http = FALSE;

    do
    {
        if (!Readblock(hi))
            return FALSE;
    } while (hi->blocklength < 5);
    if (STRNEQUAL(hi->fd->block, "HTTP/", 5))
    {
        if (!Findline(hi))
            return FALSE;
        hi->movedto = TAG_IGNORE;
        sscanf(hi->fd->block + 5, "%*d.%*d %ld", &stat);
        Updatetaskattrs(AOURL_Header, hi->fd->block, TAG_END);
        if (stat < 400)
        {
            hi->flags |= HTTPIF_TUNNELOK;
            if (stat == 301)
                hi->movedto = AOURL_Movedto;
            else if (stat == 302 || stat == 307)
            {
                hi->movedto = AOURL_Tempmovedto;
                Updatetaskattrs(AOURL_Nocache, TRUE,TAG_END);
            }
            else if (stat == 303)
                hi->movedto = AOURL_Seeother;
            else if (stat == 304)
            {
                Updatetaskattrs(AOURL_Notmodified, TRUE, TAG_END);
            }
        }
        else
        {
            if (stat == 401)
            {
                if (hi->flags & HTTPIF_AUTH)
                {               /* Second attempt */
                    if (hi->auth)
                        Forgetauthorize(hi->auth);
                    Updatetaskattrs(AOURL_Error, TRUE, TAG_END);
                }
                else
                {
                    hi->status = 401;
                }
            }
            else if (stat == 407)
            {
                if (hi->flags & HTTPIF_PRXAUTH)
                {               /* Second attempt */
                    if (hi->prxauth)
                        Forgetauthorize(hi->prxauth);
                    Updatetaskattrs(AOURL_Error, TRUE, TAG_END);
                }
                else
                {
                    hi->status = 407;
                }
            }
            else if ((stat == 405 || stat == 500 || stat == 501) &&
                     hi->fd->postmsg)
            {
                Updatetaskattrs(AOURL_Postnogood, TRUE, TAG_END);
            }
            else
            {
                Updatetaskattrs(AOURL_Error, TRUE, TAG_END);
            }
        }
        http = TRUE;
    }
    return http;
}

/* Read and process part headers until end of headers. Read when necessary.
 * Returns FALSE if eof or error. */
static BOOL Readpartheaders(struct Httpinfo *hi)
{
    hi->partlength = 0;
    *hi->parttype = '\0';
    for (;;)
    {
        if (!Findline(hi))
            return FALSE;
        if (hi->linelength == 0)
        {
            if (hi->status)
                return FALSE;
            else
                return TRUE;
        }
        Updatetaskattrs(AOURL_Header, hi->fd->block, TAG_END);
        if (STRNIEQUAL(hi->fd->block, "Content-Length:", 15))
        {
            sscanf(hi->fd->block + 15, " %ld", &hi->partlength);
        }
        else if (STRNIEQUAL(hi->fd->block, "Content-Type:", 13))
        {
            if (!prefs.network.ignoremime)
            {
                UBYTE          *p, *q;

                for (p = hi->fd->block + 13; *p && isspace(*p); p++);
                q = strchr(p, ';');
                if (q)
                    *q = '\0';
                if (strlen(p) > 31)
                    p[31] = '\0';
                strcpy(hi->parttype, p);
            }
        }
        Nextline(hi);
    }
}

/* Read data and pass to main task. Returns FALSE if error or connection eof, TRUE if
 * multipart boundary found. */
static BOOL Readdata(struct Httpinfo *hi)
{
    UBYTE     *bdcopy = NULL;
    long      bdlength = 0, blocklength = 0;
    BOOL      result = FALSE, boundary, partial, eof;

    long      gzip_buffer_size=INPUTBLOCKSIZE;

    UBYTE     *gzipbuffer=NULL;
    long      gziplength=0;
    long      err=0;
    UWORD     gzip_end=0;
    z_stream  d_stream;


    if (hi->boundary)
    {
        bdlength = strlen(hi->boundary);
        bdcopy = ALLOCTYPE(UBYTE, bdlength + 1, 0);
    }
    for (;;)
    {
        if (hi->blocklength)
        {


            // first block of the encoded data
            // allocate buffer and initialize zlib

            if ((hi->flags & HTTPIF_GZIPENCODED) && !(hi->flags & HTTPIF_GZIPDECODING))
            {
              hi->flags |= HTTPIF_GZIPDECODING;
              gzipbuffer = ALLOCTYPE(UBYTE,gzip_buffer_size,0);   //
              gziplength=hi->blocklength;                  //
              memcpy(gzipbuffer,hi->fd->block,gziplength); // copy date to buffer only needed on first block

              hi->blocklength=0;
              d_stream.zalloc = Z_NULL;
              d_stream.zfree = Z_NULL;
              d_stream.opaque = Z_NULL;
              d_stream.avail_in = 0;
              d_stream.next_in = Z_NULL;

              err=inflateInit2(&d_stream,16+15);           // set zlib to expect 'gzip-header'
              if(err!=Z_OK)  printf ("zlib Init Fail\n");  //

              d_stream.next_in = gzipbuffer;
              d_stream.avail_in = gziplength;
              d_stream.next_out = hi->fd->block;
              d_stream.avail_out = gzip_buffer_size;

          //  printf("zlib initialized  source=%d bytes\n",gziplength);  // debug

              hi->blocklength=0;
              continue;
            }



            boundary = partial = eof = FALSE;
            if (bdcopy)
            {                   /* Look for [CR]LF--<boundary>[--][CR]LF or any possible part thereof. */
                UBYTE          *p = hi->fd->block, *end = p + hi->blocklength;

                for (;;)
                {
                    for (; p < end && *p != '\r' && *p != '\n'; p++);
                    if (p >= end)
                        break;
                    blocklength = p - hi->fd->block;
                    if (*p == '\r' && (p >= end - 1 || p[1] == '\n'))
                    {
                        p++;    /* Skip CR */
                    }
                    p++;        /* Skip LF */
                    if (p >= end)
                        partial = TRUE;
                    else
                    {
                        if (*p == '-')
                        {       /* Create a copy of hi->boundary, with what we have
                                 * in the block, copied over. */
                            strcpy(bdcopy, hi->boundary);
                            strncpy(bdcopy, p, MIN(bdlength, end - p));
                        /*
                           If the result is equal to the boundary, we have a (at least
                           * partial possible) boundary.
                         */
                            if (STREQUAL(bdcopy, hi->boundary))
                            {   /* Now check if it's complete and followed by [CR]LF. */
                                p += bdlength;
                                if (p < end && *p == '-')
                                    p++;
                                if (p < end && *p == '-')
                                {
                                    eof = TRUE;
                                    p++;
                                }
                                if (p < end && *p == '\r')
                                    p++;
                                if (p >= end)
                                    partial = TRUE;
                                else if (*p == '\n')
                                    boundary = TRUE;
                            }
                        }
                    }
                    if (boundary || partial)
                        break;
                /*
                   Look further
                 */
                    p = hi->fd->block + blocklength + 1;
                }
            }
            if (!boundary && !partial)
                blocklength = hi->blocklength;
            Updatetaskattrs(AOURL_Data, hi->fd->block,
                            AOURL_Datalength, blocklength, TAG_END);
            if (blocklength < hi->blocklength)
            {
                memmove(hi->fd->block, hi->fd->block + blocklength,
                        hi->blocklength - blocklength);

            }

            if (hi->flags & HTTPIF_GZIPDECODING)
            {
              d_stream.next_out = hi->fd->block;
              d_stream.avail_out = hi->fd->blocksize;
            }

            hi->blocklength -= blocklength;

            if (boundary)
            {
                result = !eof;
                break;
            }
        }
        if (hi->flags & HTTPIF_GZIPDECODING)
        {
          if ( gzip_end > 0 )  break;

          err=inflate(&d_stream, Z_SYNC_FLUSH);   //

        //printf(" zlib read=%d bytes  written=%d bytes\n",gziplength-d_stream.avail_in,gzip_buffer_size-d_stream.avail_out);
        //if (err==Z_OK) printf ("zlib OK\n");
        //if (err==Z_STREAM_END) printf ("zlib STREAM_END read:%d wrote%d\n\n\n",d_stream.total_in ,d_stream.total_out );

          if (err!=Z_OK)
          {

#ifdef DEVELOPER
            if(httpdebug)
            {
                if (err==Z_DATA_ERROR) printf ("zlib DATA ERROR\n");
                if (err==Z_STREAM_ERROR) printf ("zlib STREAM ERROR\n");
                if (err==Z_NEED_DICT) printf ("zlib NEED DICT\n");  // impossible as not used by HTTP spec (which is a shame)
                if (err==Z_MEM_ERROR) printf ("zlib MEM ERROR\n");
                if (err==Z_BUF_ERROR) printf ("zlib BUF ERROR\n");
            }
#endif
            gzip_end=1;   // Error break !
          }

          if (err==Z_OK && d_stream.avail_in==0)
          {

             gziplength = Receive(hi, gzipbuffer, gzip_buffer_size);
             if( gziplength <= 0 ){
               gzip_end=1;    // Finished or Error
             }else{
               d_stream.next_in = gzipbuffer;
               d_stream.avail_in = gziplength;
             }
             //printf ("LOADING %d Bytes\n",gziplength);
          }

          hi->blocklength=gzip_buffer_size-d_stream.avail_out;

        }else{
          if (!Readblock(hi)) break;


        }

    }
    if (bdcopy)
        FREE(bdcopy);

    if(hi->flags & HTTPIF_GZIPDECODING){
      FREE(gzipbuffer);
      inflateEnd(&d_stream);
    }

    return result;
}

/* Process the plain or HTTP or multipart response. */
static void Httpresponse(struct Httpinfo *hi, BOOL readfirst)
{
    BOOL            first = TRUE;

    hi->fd->responsetime = Today();
    Updatetaskattrs(AOFCH_Responsetime,hi->fd->responsetime,TAG_END);

    if (!readfirst || Readresponse(hi))
    {
        Nextline(hi);
        hi->flags |= HTTPIF_HEADERS;
        if (Readheaders(hi))
        {
            if (hi->movedto && hi->movedtourl)
            {
                Updatetaskattrs(hi->movedto, hi->movedtourl, TAG_END);
            }
            else
            {
                Nextline(hi);
                if (hi->boundary)
                {
                    for (;;)
                    {
                        if (!Findline(hi))
                            return;
                        if (STREQUAL(hi->fd->block, hi->boundary))
                            break;
                        Nextline(hi);
                    }
                    Nextline(hi);       /* Skip boundary */
                    for (;;)
                    {
                        if (!Readpartheaders(hi))
                            break;
                        Nextline(hi);
                        if (!first)
                        {
                            Updatetaskattrs(AOURL_Reload, TRUE, TAG_END);
                        }
                        if (*hi->parttype || hi->partlength)
                        {
                            Updatetaskattrs(*hi->
                                            parttype ? AOURL_Contenttype :
                                            TAG_IGNORE, hi->parttype,
                                            hi->
                                            partlength ? AOURL_Contentlength :
                                            TAG_IGNORE, hi->partlength,
                                            TAG_END);
                        }
                        if (!Readdata(hi))
                            break;
                        Updatetaskattrs(AOURL_Eof, TRUE,
                                        AOURL_Serverpush, hi->fd->fetch,
                                        TAG_END);
                        if (!Findline(hi))
                            break;
                        Nextline(hi);   /* Skip boundary */
                        first = FALSE;
                    }
                }
                else
                {
                    Readdata(hi);
                }
            }
        }
    }
    else
    {
        Readdata(hi);
    }
}

/* Send a message */
static long Send(struct Httpinfo *hi, UBYTE * request, long reqlen)
{
    long            result = -1;

    if (hi->flags & HTTPIF_SSL)
    {
        result = SSL_write(hi->ssl->ssl, request, reqlen);
    }
    else
    {
        result = a_send(hi->sock, request, reqlen, 0, hi->socketbase);
    }
    return result;
}

/* Warning: Cannot make SSL connection. Retries TRUE if use unsecure link. */
static BOOL Securerequest(struct Httpinfo *hi, UBYTE * reason)
{
    UBYTE          *msg, *msgbuf;
    BOOL            ok = FALSE;

    msg = AWEBSTR(MSG_SSLWARN_SSL_TEXT);
    if (msgbuf =
        ALLOCTYPE(UBYTE,
                  strlen(msg) + strlen(hi->hostname) + strlen(reason) + 8, 0))
    {
        Lprintf(msgbuf, msg, hi->hostname, reason);
        ok = Syncrequest(AWEBSTR(MSG_SSLWARN_SSL_TITLE),
                         haiku ? HAIKU11 : msgbuf,
                         AWEBSTR(MSG_SSLWARN_SSL_BUTTONS), 0);
        FREE(msgbuf);
    }
    return ok;
}

BOOL Httpcertaccept(char *hostname, char *certname)
{
    char           *def = "????";
    UBYTE          *msg, *msgbuf, *h, *c;
    struct Certaccept *ca;
    BOOL            ok = FALSE;

    h = hostname;
    c = certname;
    if (!c)
        c = def;
    if (!h)
        h = def;
    ObtainSemaphore(&certsema);
    for (ca = certaccepts.first; ca->next; ca = ca->next)
    {
        if (STRIEQUAL(ca->hostname, hostname) && STREQUAL(ca->certname, c))
        {
            ok = TRUE;
            break;
        }
    }
    if (!ok)
    {
        msg = AWEBSTR(MSG_SSLWARN_CERT_TEXT);
        if (msgbuf =
            ALLOCTYPE(UBYTE, strlen(msg) + strlen(h) + strlen(c) + 8, 0))
        {
            Lprintf(msgbuf, msg, h, c);
            ok = Syncrequest(AWEBSTR(MSG_SSLWARN_CERT_TITLE),
                             haiku ? HAIKU13 : msgbuf,
                             AWEBSTR(MSG_SSLWARN_CERT_BUTTONS), 0);
            FREE(msgbuf);
            if (hostname)
            {
                if (ok)
                {
                    if (ca =
                        ALLOCSTRUCT(Certaccept, 1, MEMF_CLEAR))
                    {
                        ca->hostname = Dupstr(hostname, -1);
                        ca->certname = Dupstr(c, -1);
                        ADDTAIL(&certaccepts, ca);
                    }
                }
            }
        }
    }
    ReleaseSemaphore(&certsema);
    return ok;
}

/* Open the tcp stack, and optionally the SSL library */
static BOOL Openlibraries(struct Httpinfo *hi)
{
    BOOL            result = FALSE;

    Opentcp(&hi->socketbase, hi->fd, !hi->fd->validate);
    if (hi->socketbase)
    {
        result = TRUE;
        if (hi->flags & HTTPIF_SSL)
        {
            if (hi->assl = Tcpopenssl(hi->socketbase))
            {                   /* ok */
            }
            else
            {                   /* No SSL available */
                if (Securerequest
                    (hi, haiku ? HAIKU12 : AWEBSTR(MSG_SSLWARN_SSL_NO_SSL2)))
                {
                    hi->flags &= ~HTTPIF_SSL;
                }
                else
                {
                    result = FALSE;
                }
            }
        }
    }
    return result;
}

/* Create SSL context, SSL and socket */
static long Opensocket(struct Httpinfo *hi, struct hostent *hent)
{
    long            sock;

    if (hi->flags & HTTPIF_SSL)
    {
        if (!(hi->ssl = CreateHTTPSSL()))
            return -1;
    }
    sock = a_socket(hent->h_addrtype, SOCK_STREAM, 0, hi->socketbase);
    if (sock < 0)
    {
        if (hi->ssl)
        {
            FreeHTTPSSL(hi->ssl);
            hi->ssl = NULL;
        }
    }
    return sock;
}

/* Connect and make SSL connection. Returns TRUE if success. */
static BOOL Connect(struct Httpinfo *hi, struct hostent *hent)
{
    BOOL            ok = FALSE;

    if (hi->port == -1)
    {
        if (hi->flags & HTTPIF_SSL)
            hi->port = 443;
        else
            hi->port = 80;
    }
    if (!a_connect(hi->sock, hent, hi->port, hi->socketbase))
    {
        if (hi->flags & HTTPIF_SSL)
        {
            if (hi->flags & HTTPIF_SSLTUNNEL)
            {
                UBYTE          *creq, *p;
                long            creqlen =
                    strlen(tunnelrequest) + strlen(hi->tunnel);
                if (hi->prxauth && hi->prxauth->cookie)
                {
                    creqlen +=
                        strlen(proxyauthorization) +
                        strlen(hi->prxauth->cookie);
                }
                creqlen += 16;
                if (creq = ALLOCTYPE(UBYTE, creqlen, 0))
                {
                    p = creq;
                    p += sprintf(p, tunnelrequest, hi->tunnel);
                    if (hi->prxauth && hi->prxauth->cookie)
                        p += sprintf(p, proxyauthorization,
                                     hi->prxauth->cookie);
                    p += sprintf(p, "\r\n");
                    creqlen = p - creq;
#ifdef BETAKEYFILE
                    if (httpdebug)
                    {
                        Write(Output(), "\n", 1);
                        Write(Output(), creq, creqlen);
                    }
#endif
                /*
                   Temporarily turn off SSL since we don't have a SSL connection yet
                 */
                    hi->flags &= ~HTTPIF_SSL;
                    if (Send(hi, creq, creqlen) == creqlen)
                    {
                        if (Readresponse(hi))
                        {
                            Nextline(hi);
                            if (Readheaders(hi))
                            {
                                Nextline(hi);
                                if (hi->flags & HTTPIF_TUNNELOK)
                                {
                                    ok = TRUE;
                                }
                            }
                        }
                    }
                    hi->flags |= HTTPIF_SSL;
                    FREE(creq);
                }
            }
            else
                ok = TRUE;

            if (ok)
            {
                /* Set up SSL connection */
                SSL_set_fd(hi->ssl->ssl, hi->sock);
                SSL_set_tlsext_host_name(hi->ssl->ssl, hi->hostname);

                long result = SSL_connect(hi->ssl->ssl);
                ok = (result == 1);  /* SSL_connect returns 1 on success */
                if (result <= 0)
                {
                    int ssl_error = SSL_get_error(hi->ssl->ssl, result);
                    if (ssl_error == SSL_ERROR_SSL)
                        hi->flags |= HTTPIF_NOSSLREQ;
                }
                if (!ok && !(hi->flags & HTTPIF_NOSSLREQ))
                {
                    UBYTE           errbuf[128], *p;
                    unsigned long   ssl_err = ERR_get_error();

                    p = (UBYTE *)ERR_error_string(ssl_err, errbuf);
                    if (Securerequest(hi, p))
                    {
                        hi->flags |= HTTPIF_RETRYNOSSL;
                    }
                }
            }
        }
        else
        {
            ok = TRUE;
        }
    }
    return ok;
}

/* Send multipart form data. */
static BOOL Sendmultipartdata(struct Httpinfo *hi, struct Fetchdriver *fd,
                              FILE * fp)
{
    struct Multipartpart *mpp;
    long            lock, fh, l;
    BOOL            ok = TRUE;

    for (mpp = fd->multipart->parts.first; ok && mpp->next; mpp = mpp->next)
    {
        if (mpp->lock)
        {
            Updatetaskattrs(AOURL_Netstatus, NWS_UPLOAD, TAG_END);
            Tcpmessage(fd, TCPMSG_UPLOAD);
        /*
           We can't just use the mpp->lock because we might need to send the
           * message again after a 301/302 status.
         */
            if (lock = DupLock(mpp->lock))
            {
                if (fh = OpenFromLock(lock))
                {
                    while (ok && (l = Read(fh, fd->block, fd->blocksize)))
                    {
#ifdef DEVELOPER
                        if (httpdebug)
                            Write(Output(), fd->block, l);
                        if (fp)
                            fwrite(fd->block, l, 1, fp);
                        else
#endif
                            ok = (Send(hi, fd->block, l) == l);
                    }
                    Close(fh);
                }
                else
                    UnLock(lock);
            }
        }
        else
        {
#ifdef DEVELOPER
            if (httpdebug)
                Write(Output(), fd->multipart->buf.buffer + mpp->start,
                      mpp->length);
            if (fp)
                fwrite(fd->multipart->buf.buffer + mpp->start, mpp->length, 1,
                       fp);
            else
#endif
                ok = (Send
                      (hi, fd->multipart->buf.buffer + mpp->start,
                       mpp->length) == mpp->length);
        }
    }
#ifdef DEVELOPER
    if (httpdebug)
        Write(Output(), "\n", 1);
#endif
    return ok;
}

static BOOL Formwarnrequest(void)
{
    return (BOOL) Syncrequest(AWEBSTR(MSG_FORMWARN_TITLE),
                              haiku ? HAIKU16 : AWEBSTR(MSG_FORMWARN_WARNING),
                              AWEBSTR(MSG_FORMWARN_BUTTONS), 0);
}

static void Httpretrieve(struct Httpinfo *hi, struct Fetchdriver *fd)
{
    struct hostent *hent;
    long            reqlen, msglen, result;
    UBYTE          *request, *p, *q;
    BOOL            error = FALSE;

    hi->blocklength = 0;
    hi->nextscanpos = 0;
    if (fd->flags & FDVF_SSL)
        hi->flags |= HTTPIF_SSL;
    hi->fd = fd;
#ifdef DEVELOPER
    if (STRNEQUAL(fd->name, "&&&&", 4)
        || STRNIEQUAL(fd->name, "http://&&&&", 11)
        || STRNIEQUAL(fd->name, "https://&&&&", 12)
        || STRNIEQUAL(fd->name, "ftp://&&&&", 10))
    {
        UBYTE           name[64] = "CON:20/200/600/200/HTTP/screen ";
        FILE           *f;

        strcat(name, (UBYTE *) Agetattr(Aweb(), AOAPP_Screenname));
        if ((!(hi->fd->flags & FDVF_FORMWARN) || (hi->flags & HTTPIF_SSL) ||
             Formwarnrequest()) && (f = fopen(name, "r+")))
        {
            fprintf(f, "[%s %ld%s]\n", hi->connect, hi->port,
                    (hi->flags & HTTPIF_SSL) ? " SECURE" : "");
            reqlen = Buildrequest(fd, hi, &request);
            fwrite(request, reqlen, 1, f);
            if (fd->multipart)
                Sendmultipartdata(hi, fd, f);
            else if (fd->postmsg)
            {
                fwrite(fd->postmsg, strlen(fd->postmsg), 1, f);
                fwrite("\n", 1, 1, f);
            }
            fflush(f);
            if (request != fd->block)
                FREE(request);
            if (hi->flags & HTTPIF_SSL)
            {
                Updatetaskattrs(AOURL_Cipher, "AWEB-DEBUG", TAG_END);
            }
            Updatetaskattrs(AOURL_Netstatus, NWS_WAIT, TAG_END);
            Tcpmessage(fd, TCPMSG_WAITING,
                       hi->flags & HTTPIF_SSL ? "HTTPS" : "HTTP");
            hi->socketbase = NULL;
            hi->sock = (long)f;
            Httpresponse(hi, TRUE);
            fclose(f);
        }
    }
    else
    {
#endif
        result = Openlibraries(hi);
        if (result && (hi->fd->flags & FDVF_FORMWARN) &&
            !(hi->flags & HTTPIF_SSL))
        {
            result = Formwarnrequest();
        }
        if (result)
        {


            Updatetaskattrs(AOURL_Netstatus, NWS_LOOKUP, TAG_END);
            Tcpmessage(fd, TCPMSG_LOOKUP, hi->connect);
            if (hent = Lookup(hi->connect, hi->socketbase))
            {
                if ((hi->sock = Opensocket(hi, hent)) >= 0)
                {
                    Updatetaskattrs(AOURL_Netstatus, NWS_CONNECT, TAG_END);
                    /* hent->h_name is the A entry, might be punyencoded */
                    Tcpmessage(fd, TCPMSG_CONNECT,
                               hi->flags & HTTPIF_SSL ? "HTTPS" : "HTTP",
                               hent->h_name);
                    if (Connect(hi, hent))
                    {

                        if (hi->flags & HTTPIF_SSL)
                        {
                            p = (UBYTE *)SSL_get_cipher(hi->ssl->ssl);
                            q = (UBYTE *)"AmiSSL";
                            if (p || q)
                            {
                                Updatetaskattrs(AOURL_Cipher, p,
                                                AOURL_Ssllibrary, q, TAG_END);
                            }
                        }

                        reqlen = Buildrequest(fd, hi, &request);
                        result = (Send(hi, request, reqlen) == reqlen);
#ifdef BETAKEYFILE
                        if (httpdebug)
                        {
                            Write(Output(), "\n", 1);
                            Write(Output(), request, reqlen);
                        }
#endif
                        if (result)
                        {
                            if (fd->multipart)
                            {
                                result = Sendmultipartdata(hi, fd, NULL);
                            }
                            else if (fd->postmsg)
                            {
                                msglen = strlen(fd->postmsg);
                                result =
                                    (Send(hi, fd->postmsg, msglen) == msglen);
#ifdef BETAKEYFILE
                                if (httpdebug)
                                {
                                    Write(Output(), fd->postmsg, msglen);
                                    Write(Output(), "\n\n", 2);
                                }
#endif
                            }
                        }
                        if (request != fd->block)
                            FREE(request);
                        if (result)
                        {
                            Updatetaskattrs(AOURL_Netstatus, NWS_WAIT,
                                            TAG_END);
                            Tcpmessage(fd, TCPMSG_WAITING,
                                       hi->
                                       flags & HTTPIF_SSL ? "HTTPS" : "HTTP");
                            Httpresponse(hi, TRUE);
                        }
                        else
                            error = TRUE;
                    }
                    else if (!(hi->flags & HTTPIF_RETRYNOSSL) &&
                             hi->status != 407)
                    {
                        Tcperror(fd, TCPERR_NOCONNECT,
                                 (hi->flags & HTTPIF_SSLTUNNEL) ? hi->
                                 hostport : (UBYTE *) hent->h_name);
                    }
                    if (hi->ssl)
                    {
                        FreeHTTPSSL(hi->ssl);
                        hi->ssl = NULL;
                    }
                    a_close(hi->sock, hi->socketbase);
                }
                else
                    error = TRUE;
            }
            else
            {
                Tcperror(fd, TCPERR_NOHOST, hi->hostname);
            }
            a_cleanup(hi->socketbase);
        }
        else
        {
            Tcperror(fd, TCPERR_NOLIB);
        }
        if (hi->ssl)
        {
            FreeHTTPSSL(hi->ssl);
            hi->ssl = NULL;
        }
        if (hi->socketbase)
        {
            Closeaweblib(hi->socketbase);   /* this is now closed by Closeaweblib as hi->socketbase may be a lib os3 or an interface os4 */
            hi->socketbase = NULL;
        }
#ifdef DEVELOPER
    }
#endif
    if (error)
    {
        Updatetaskattrs(AOURL_Error, TRUE, TAG_END);
    }
}

/*-----------------------------------------------------------------------*/

void Httptask(struct Fetchdriver *fd)
{
    struct Httpinfo hi = { 0 };

    if (Makehttpaddr(&hi, fd->proxy, fd->name, BOOLVAL(fd->flags & FDVF_SSL)))
    {
        if (!prefs.network.limitproxy && !hi.auth)
            hi.auth = Guessauthorize(hi.hostport);
        if (fd->proxy && !prefs.network.limitproxy)
            hi.prxauth = Guessauthorize(fd->proxy);
        for (;;)
        {
            if (fd->proxy && hi.auth && prefs.network.limitproxy)
            {
                if (hi.connect)
                    FREE(hi.connect);
                if (hi.tunnel)
                    FREE(hi.tunnel);
                hi.tunnel = NULL;
                if (hi.hostport)
                    FREE(hi.hostport);
                if (hi.abspath)
                    FREE(hi.abspath);
                if (hi.hostname)
                    FREE(hi.hostname);
                if (!Makehttpaddr
                    (&hi, NULL, fd->name, BOOLVAL(fd->flags & FDVF_SSL)))
                    break;
            }
            hi.status = 0;
            Httpretrieve(&hi, fd);
            if (hi.flags & HTTPIF_RETRYNOSSL)
            {
                UBYTE          *url =
                    ALLOCTYPE(UBYTE, strlen(fd->name) + 6, 0);
                UBYTE          *p;

                strcpy(url, "http");
                if (p = strchr(fd->name, ':'))
                    strcat(url, p);
                Updatetaskattrs(AOURL_Tempmovedto, url, TAG_END);
                FREE(url);
                break;
            }
            if (hi.status == 401 && !(hi.flags & HTTPIF_AUTH) && hi.auth)
            {
                hi.flags |= HTTPIF_AUTH;
                Updatetaskattrs(AOURL_Contentlength, 0,
                                AOURL_Contenttype, "", TAG_END);
                if (!hi.auth->cookie)
                    Authorize(fd, hi.auth, FALSE);
                if (hi.auth->cookie)
                    continue;
                Updatetaskattrs(AOURL_Error, TRUE, TAG_END);
            }
            if (hi.status == 407 && !(hi.flags & HTTPIF_PRXAUTH) &&
                hi.prxauth)
            {
                hi.flags |= HTTPIF_PRXAUTH;
                Updatetaskattrs(AOURL_Contentlength, 0,
                                AOURL_Contenttype, "", TAG_END);
                if (!hi.prxauth->cookie)
                    Authorize(fd, hi.prxauth, TRUE);
                if (hi.prxauth->cookie)
                    continue;
                Updatetaskattrs(AOURL_Error, TRUE, TAG_END);
            }
            break;
        }
    }
    else
    {
        Updatetaskattrs(AOURL_Error, TRUE, TAG_END);
    }
    if (hi.connect)
        FREE(hi.connect);
    if (hi.tunnel)
        FREE(hi.tunnel);
    if (hi.hostport)
        FREE(hi.hostport);
    if (hi.abspath)
        FREE(hi.abspath);
    if (hi.hostname)
        FREE(hi.hostname);
    if (hi.auth)
        Freeauthorize(hi.auth);
    if (hi.prxauth)
        Freeauthorize(hi.prxauth);
    if (hi.movedtourl)
        FREE(hi.movedtourl);
    Updatetaskattrs(AOTSK_Async, TRUE,
                    AOURL_Eof, TRUE, AOURL_Terminate, TRUE, TAG_END);
}

#endif /* LOCALONLY */

/*-----------------------------------------------------------------------*/

BOOL Inithttp(void)
{
#ifndef LOCALONLY
    InitSemaphore(&certsema);
    NEWLIST(&certaccepts);
    InitSemaphore(&SSLSema);
#endif
    return TRUE;
}

void Freehttp(void)
{
#ifndef LOCALONLY
    struct Certaccept *ca;

    if (certaccepts.first)
    {
        while (ca =(struct Certaccept *)REMHEAD(&certaccepts))
        {
            if (ca->hostname)
                FREE(ca->hostname);
            if (ca->certname)
                FREE(ca->certname);
            FREE(ca);
        }
    }

    /* Cleanup SSL resources */
    if (GlobalSSLContext)
    {
        SSL_CTX_free(GlobalSSLContext);
        GlobalSSLContext = NULL;
    }
    CleanupAmiSSLLibraries();
#endif
}
