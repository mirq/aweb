
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#ifndef LOCALONLY

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
    struct Assl    *assl;       /* AwebSSL context */
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
    long            chunk_remaining;    /* Bytes remaining in current chunk */
    UWORD           chunk_state;        /* Current chunk parsing state */
};

struct Httpconn
{
    NODE(Httpconn);
    UBYTE          *hostname;   /* Host name */
    long            port;       /* Port number */
    BOOL            ssl;        /* SSL connection flag */
    long            sock;       /* Socket descriptor */
    struct Assl    *assl;       /* SSL context for HTTPS */
    struct Library *socketbase; /* Socket library base */
    ULONG           lastused;   /* Last used timestamp */
    ULONG           created;    /* Connection creation time */
    long            requests;   /* Number of requests on this connection */
    long            maxrequests; /* Max requests allowed (from Keep-Alive header) */
    long            timeout;    /* Keep-alive timeout in seconds */
    BOOL            keepalive;  /* Server supports keep-alive */
};

static LIST(Httpconn) connectionpool;
static struct SignalSemaphore poolsema;
static long totalconnections = 0;

#define MAX_CONNECTIONS_PER_HOST 6
#define MAX_TOTAL_CONNECTIONS 30
#define DEFAULT_KEEPALIVE_TIMEOUT 15
#define CONNECTION_IDLE_TIMEOUT 60

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
#define HTTPIF_CONN_CLOSE   0x0400       /* Connection: close header received */
#define HTTPIF_CONN_KEEPALIVE 0x0800     /* Connection: keep-alive header received */
#define HTTPIF_CHUNKED      0x1000       /* Response uses chunked transfer encoding */

/* Chunk parsing states */
#define CHUNK_SIZE          0            /* Reading chunk size */
#define CHUNK_DATA          1            /* Reading chunk data */
#define CHUNK_TRAILER       2            /* Reading chunk trailer CRLF */
#define CHUNK_DONE          3            /* All chunks processed */

static UBYTE   *httprequest = "GET %.7000s HTTP/1.1\r\n";

static UBYTE   *httppostrequest = "POST %.7000s HTTP/1.1\r\n";

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

static UBYTE   *fixedheaders = "Accept: */*;q=1\r\nAccept-Encoding: gzip\r\nConnection: keep-alive\r\n";

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
        result = Assl_read(hi->assl, buffer, length);
    }
    else
    {
        result = a_recv(hi->sock, buffer, length, 0, hi->socketbase);
    }

#ifdef DEVELOPER
    if (result > 0 && httpdebug)
    {
        printf("Receive: got %ld bytes: ", result);
        for (int i = 0; i < MIN(20, result); i++) {
            printf("%02x ", (unsigned char)buffer[i]);
        }
        printf("\n");
    }
#endif
    return result;
}

/* Read remainder of block. Returns FALSE if eof or error. */
static BOOL Readblock(struct Httpinfo *hi)
{
    long            n;

    n = Receive(hi, hi->fd->block + hi->blocklength,
                hi->fd->blocksize - hi->blocklength);
    if (n < 0 || Checktaskbreak())
    {
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
#ifdef DEVELOPER
            printf("Readheaders: found Content-Length header: %s\n", hi->fd->block + 15);
#endif

            /* temporary work arround for unkown size of decoded gzip */

            if(!(hi->flags & HTTPIF_GZIPENCODED))
            {
                sscanf(hi->fd->block + 15, " %ld", &i);
#ifdef DEVELOPER
                printf("Readheaders: parsed Content-Length: %ld bytes\n", i);
#endif
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
        else if (STRNIEQUAL(hi->fd->block, "Connection:", 11))
        {
            UBYTE *p;
            for (p = hi->fd->block + 11; *p && isspace(*p); p++);
            if (STRNIEQUAL(p, "close", 5))
            {
                hi->flags |= HTTPIF_CONN_CLOSE;
            }
            else if (STRNIEQUAL(p, "keep-alive", 10))
            {
                hi->flags |= HTTPIF_CONN_KEEPALIVE;
            }
        }
        else if (STRNIEQUAL(hi->fd->block, "Transfer-Encoding:", 18))
        {
            UBYTE *p;
            for (p = hi->fd->block + 18; *p && isspace(*p); p++);
            if (STRNIEQUAL(p, "chunked", 7))
            {
#ifdef DEVELOPER
                printf("Readheaders: detected chunked transfer encoding\n");
#endif
                hi->flags |= HTTPIF_CHUNKED;
            }
        }
        else if (STRNIEQUAL(hi->fd->block, "Keep-Alive:", 11))
        {
            UBYTE *p, *q;
            for (p = hi->fd->block + 11; *p && isspace(*p); p++);
            while (*p)
            {
                if (STRNIEQUAL(p, "timeout=", 8))
                {
                    p += 8;
                    hi->fd->keepalive_timeout = atol(p);
                }
                else if (STRNIEQUAL(p, "max=", 4))
                {
                    p += 4;
                    hi->fd->keepalive_max = atol(p);
                }
                while (*p && *p != ',' && *p != ' ') p++;
                while (*p && (*p == ',' || *p == ' ')) p++;
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
#ifdef DEVELOPER
            printf("Multipart: Content-Length: %ld bytes\n", hi->partlength);
#endif
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
#ifdef DEVELOPER
    printf("Readdata: starting, current blocklength=%ld\n", hi->blocklength);
    if (hi->blocklength > 0) {
        printf("Readdata: buffer content: '%.50s'\n", hi->fd->block);
    }
    printf("Readdata: flags=0x%x, CHUNKED=%s\n", hi->flags, (hi->flags & HTTPIF_CHUNKED) ? "TRUE" : "FALSE");
#endif
    UBYTE     *bdcopy = NULL;
    long      bdlength = 0, blocklength = 0;
    BOOL      result = FALSE, boundary, partial, eof;

    long      gzip_buffer_size=INPUTBLOCKSIZE;

    UBYTE     *gzipbuffer=NULL;
    long      gziplength=0;
    long      err=0;
    UWORD     gzip_end=0;
    z_stream  d_stream;

    /* Separate buffer for accumulating chunked gzip data */
    UBYTE     *chunk_gzip_buffer=NULL;
    long      chunk_gzip_size=0;
    long      chunk_gzip_length=0;


    if (hi->boundary)
    {
        bdlength = strlen(hi->boundary);
        bdcopy = ALLOCTYPE(UBYTE, bdlength + 1, 0);
    }

    /* Initialize accumulation buffer for chunked+gzip */
    if ((hi->flags & HTTPIF_CHUNKED) && (hi->flags & HTTPIF_GZIPENCODED))
    {
        chunk_gzip_size = 64 * 1024; /* 64KB initial buffer */
        chunk_gzip_buffer = ALLOCTYPE(UBYTE, chunk_gzip_size, 0);
        chunk_gzip_length = 0;
#ifdef DEVELOPER
        printf("Readdata: allocated %ld bytes for chunked gzip accumulation\n", chunk_gzip_size);
#endif
    }
    for (;;)
    {
#ifdef DEVELOPER
        printf("Readdata: loop iteration, blocklength=%ld\n", hi->blocklength);
#endif
        if (hi->blocklength)
        {
#ifdef DEVELOPER
            printf("Readdata: entering data processing block\n");
#endif


            /* Handle chunked transfer encoding FIRST (before gzip) */
            if (hi->flags & HTTPIF_CHUNKED)
            {
#ifdef DEVELOPER
                printf("Readdata: processing chunked data, state=%d, remaining=%ld\n", 
                       hi->chunk_state, hi->chunk_remaining);
#endif
                UBYTE *src = hi->fd->block;
                UBYTE *src_end = src + hi->blocklength;
                UBYTE *dest = hi->fd->block;
                long extracted_data = 0;
                BOOL chunk_complete = FALSE;

#ifdef DEVELOPER
                printf("Readdata: raw buffer (%ld bytes): ", hi->blocklength);
                for (int i = 0; i < MIN(31, hi->blocklength); i++) {
                    printf("%02x ", (unsigned char)hi->fd->block[i]);
                }
                printf("\n");
#endif

                while (src < src_end && !chunk_complete)
                {
                    switch (hi->chunk_state)
                    {
                        case CHUNK_SIZE:
                        {
                            /* Find end of chunk size line */
                            UBYTE *line_end = src;
                            while (line_end < src_end && *line_end != '\r' && *line_end != '\n')
                                line_end++;

                            if (line_end >= src_end)
                            {
#ifdef DEVELOPER
                                printf("Readdata: incomplete chunk size line, need more data\n");
#endif
                                chunk_complete = TRUE; /* Need more data */
                                break;
                            }

                            /* Parse chunk size */
                            UBYTE saved = *line_end;
                            *line_end = '\0';
                            char *endptr;
#ifdef DEVELOPER
                            printf("Readdata: parsing chunk size string: '%s'\n", (char*)src);
#endif
                            hi->chunk_remaining = strtol((char*)src, &endptr, 16);

                            if (endptr == (char*)src || hi->chunk_remaining < 0)
                            {
#ifdef DEVELOPER
                                printf("Readdata: malformed chunk size\n");
#endif
                                *line_end = saved;
                                hi->chunk_state = CHUNK_DONE;
                                chunk_complete = TRUE;
                                break;
                            }

#ifdef DEVELOPER
                            printf("Readdata: parsed chunk size: %ld bytes (0x%s)\n", hi->chunk_remaining, (char*)src);
#endif
                            *line_end = saved;

                            /* Skip CRLF after chunk size */
                            src = line_end;
                            if (src < src_end && *src == '\r') src++;
                            if (src < src_end && *src == '\n') src++;
                            else
                            {
                                /* Incomplete CRLF */
                                chunk_complete = TRUE;
                                break;
                            }

                            if (hi->chunk_remaining == 0)
                            {
#ifdef DEVELOPER
                                printf("Readdata: final chunk (size 0), chunked transfer complete\n");
#endif
                                hi->chunk_state = CHUNK_DONE;
                                hi->flags &= ~HTTPIF_CHUNKED; /* Clear chunked flag */
                                chunk_complete = TRUE;
                            }
                            else
                            {
                                hi->chunk_state = CHUNK_DATA;
                            }
                            break;
                        }

                        case CHUNK_DATA:
                        {
                            /* Extract chunk data */
                            long available = src_end - src;
                            long to_copy = MIN(hi->chunk_remaining, available);

                            if (to_copy > 0)
                            {
#ifdef DEVELOPER
                                printf("Readdata: extracting %ld bytes of chunk data\n", to_copy);
#endif

                                /* If chunked+gzip, accumulate in separate buffer */
                                if (chunk_gzip_buffer)
                                {
                                    /* Ensure buffer is large enough */
                                    if (chunk_gzip_length + to_copy > chunk_gzip_size)
                                    {
                                        long new_size = chunk_gzip_size * 2;
                                        while (chunk_gzip_length + to_copy > new_size)
                                            new_size *= 2;
                                        UBYTE *new_buffer = ALLOCTYPE(UBYTE, new_size, 0);
                                        if (new_buffer)
                                        {
                                            if (chunk_gzip_length > 0)
                                                memcpy(new_buffer, chunk_gzip_buffer, chunk_gzip_length);
                                            FREE(chunk_gzip_buffer);
                                            chunk_gzip_buffer = new_buffer;
                                            chunk_gzip_size = new_size;
#ifdef DEVELOPER
                                            printf("Readdata: expanded gzip buffer to %ld bytes\n", new_size);
#endif
                                        }
                                    }

                                    /* Copy chunk data to accumulation buffer */
                                    memcpy(chunk_gzip_buffer + chunk_gzip_length, src, to_copy);
                                    chunk_gzip_length += to_copy;
#ifdef DEVELOPER
                                    printf("Readdata: accumulated %ld bytes, total=%ld\n", to_copy, chunk_gzip_length);
#endif
                                }
                                else
                                {
                                    /* Normal chunk processing */
                                    if (dest != src)
                                    {
                                        memmove(dest, src, to_copy);
                                    }
                                    dest += to_copy;
                                    extracted_data += to_copy;
                                }

                                src += to_copy;
                                hi->chunk_remaining -= to_copy;
                            }

                            if (hi->chunk_remaining == 0)
                            {
                                hi->chunk_state = CHUNK_TRAILER;
                            }
                            else
                            {
                                /* Need more data for this chunk */
                                chunk_complete = TRUE;
                            }
                            break;
                        }

                        case CHUNK_TRAILER:
                        {
                            /* Skip chunk trailing CRLF */
#ifdef DEVELOPER
                            printf("Readdata: CHUNK_TRAILER state, src < src_end=%s, available=%ld\n",
                                   (src < src_end) ? "TRUE" : "FALSE", src_end - src);
                            if (src < src_end) {
                                printf("Readdata: next bytes: %02x %02x %02x\n",
                                       (unsigned char)*src,
                                       (src+1 < src_end) ? (unsigned char)*(src+1) : 0,
                                       (src+2 < src_end) ? (unsigned char)*(src+2) : 0);
                            }
#endif
                            if (src < src_end && *src == '\r') src++;
                            if (src < src_end && *src == '\n')
                            {
                                src++;
                                hi->chunk_state = CHUNK_SIZE; /* Ready for next chunk */
#ifdef DEVELOPER
                                printf("Readdata: chunk trailer consumed, ready for next chunk\n");
#endif
                            }
                            else
                            {
                                /* Incomplete trailing CRLF */
#ifdef DEVELOPER
                                printf("Readdata: incomplete trailing CRLF, need more data\n");
#endif
                                chunk_complete = TRUE;
                            }
                            break;
                        }

                        case CHUNK_DONE:
                        default:
                            chunk_complete = TRUE;
                            break;
                    }
                }

                /* Update buffer with extracted data (only for normal chunks) */
                if (!chunk_gzip_buffer)
                {
                    hi->blocklength = extracted_data;
                }
                else
                {
                    /* For chunked+gzip, clear the main buffer since data is in accumulation buffer */
                    hi->blocklength = 0;
                }

                /* Remove processed bytes from buffer */
                if (src < src_end && src > hi->fd->block)
                {
                    long remaining = src_end - src;
                    memmove(hi->fd->block + extracted_data, src, remaining);
                    hi->blocklength += remaining;
#ifdef DEVELOPER
                    printf("Readdata: moved %ld unprocessed bytes after extracted data\n", remaining);
#endif
                }

#ifdef DEVELOPER
                printf("Readdata: extracted %ld bytes total, buffer now has %ld bytes\n", 
                       extracted_data, hi->blocklength);
                if (extracted_data > 0)
                {
                    printf("Readdata: extracted data (first 20 bytes): ");
                    for (int i = 0; i < MIN(20, extracted_data); i++) {
                        printf("%02x ", (unsigned char)hi->fd->block[i]);
                    }
                    printf("\n");
                }
#endif

                /* If chunks are done, process gzip if needed */
                if (hi->chunk_state == CHUNK_DONE)
                {
#ifdef DEVELOPER
                    printf("Readdata: chunked transfer complete, checking for gzip (flags=0x%x, accumulated=%ld)\n",
                           hi->flags, chunk_gzip_length);
#endif
                    /* If we have gzip encoded data, decompress it now */
                    if ((hi->flags & HTTPIF_GZIPENCODED) && chunk_gzip_buffer && chunk_gzip_length > 0)
                    {
#ifdef DEVELOPER
                        printf("Readdata: starting gzip decompression of %ld accumulated bytes\n", chunk_gzip_length);
#endif
                        /* Use accumulated buffer directly for gzip processing */
#ifdef DEVELOPER
                        printf("Readdata: using accumulation buffer directly for gzip (%ld bytes)\n", chunk_gzip_length);
#endif
                        /* Set up gzip processing with accumulated data */
                        hi->flags |= HTTPIF_GZIPDECODING;
                        hi->blocklength = 0; /* Clear main buffer for gzip output */
#ifdef DEVELOPER
                        printf("Readdata: set GZIPDECODING flag, flags=0x%x\n", hi->flags);
                        printf("Readdata: continuing to gzip processing\n");
#endif
                        /* Continue to gzip processing instead of breaking */
                        goto start_gzip_processing;
                    }
                    else
                    {
                        /* No gzip, we're done if no more extracted data */
                        if (extracted_data == 0 && chunk_gzip_length == 0)
                        {
                            break;
                        }
                    }
                }

                /* Continue with chunk processing if we have extracted data */
                if (extracted_data == 0)
                {
                    /* Need more data but no extracted data to process */
                    continue;
                }
            }

            // Skip content processing only during chunked data collection, not after gzip
            if (chunk_gzip_buffer && hi->chunk_state != CHUNK_DONE)
            {
#ifdef DEVELOPER
                printf("Readdata: skipping content processing, collecting chunks for gzip\n");
#endif
                /* Skip boundary/content processing during chunk collection */
            }
            else
            {
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

            /* Normal content processing */
#ifdef DEVELOPER
            printf("Content: sending %ld bytes of data\n", blocklength);
            if (blocklength > 0) {
                printf("Content: first 100 chars: '%.100s'\n", hi->fd->block);
            }
#endif
            Updatetaskattrs(AOURL_Data, hi->fd->block,
                            AOURL_Datalength, blocklength, TAG_END);

            if (blocklength < hi->blocklength)
            {
                memmove(hi->fd->block, hi->fd->block + blocklength,
                        hi->blocklength - blocklength);
            }
            hi->blocklength -= blocklength;

            if (hi->flags & HTTPIF_GZIPDECODING)
            {
              d_stream.next_out = hi->fd->block;
              d_stream.avail_out = hi->fd->blocksize;
            }

            if (boundary)
            {
                result = !eof;
                break;
            }
            } /* End of content processing else block */
        }

start_gzip_processing:
#ifdef DEVELOPER
        printf("Readdata: reached gzip processing section, flags=0x%x, GZIPDECODING=%s\n",
               hi->flags, (hi->flags & HTTPIF_GZIPDECODING) ? "TRUE" : "FALSE");
#endif
        /* Initialize gzip decompression if needed */
        if ((hi->flags & HTTPIF_GZIPDECODING) && !gzipbuffer)
        {
#ifdef DEVELOPER
            printf("Readdata: initializing gzip decompression for complete data (blocklength=%ld)\n", hi->blocklength);
#endif
            gzipbuffer = ALLOCTYPE(UBYTE, gzip_buffer_size, 0);
            if (!gzipbuffer)
            {
                break; /* Out of memory */
            }

            d_stream.zalloc = Z_NULL;
            d_stream.zfree = Z_NULL;
            d_stream.opaque = Z_NULL;
            d_stream.avail_in = 0;
            d_stream.next_in = Z_NULL;

            err = inflateInit2(&d_stream, 16+15); /* gzip format */
            if (err != Z_OK)
            {
#ifdef DEVELOPER
                printf("Readdata: zlib init failed: %d\n", err);
#endif
                FREE(gzipbuffer);
                gzipbuffer = NULL;
                break;
            }

            /* Use accumulated chunk data or current block data for gzip */
            if (chunk_gzip_buffer && chunk_gzip_length > 0)
            {
                /* Use the accumulated chunked data directly */
                d_stream.next_in = chunk_gzip_buffer;
                d_stream.avail_in = chunk_gzip_length;
                gziplength = chunk_gzip_length;
#ifdef DEVELOPER
                printf("Readdata: using %ld bytes from accumulation buffer for gzip processing\n", gziplength);
#endif
            }
            else
            {
                /* Use current block data */
                gziplength = hi->blocklength;
                if (gziplength > 0)
                {
                    memcpy(gzipbuffer, hi->fd->block, gziplength);
                    d_stream.next_in = gzipbuffer;
                    d_stream.avail_in = gziplength;
#ifdef DEVELOPER
                    printf("Readdata: copied %ld bytes to gzip buffer for processing\n", gziplength);
#endif
                }
                else
                {
#ifdef DEVELOPER
                    printf("Readdata: no data in block for gzip processing\n");
#endif
                }
            }
            hi->blocklength = 0;
        }

        if (hi->flags & HTTPIF_GZIPDECODING)
        {
          if ( gzip_end > 0 )  break;

#ifdef DEVELOPER
          printf("Readdata: gzip decompression loop, avail_in=%d\n", d_stream.avail_in);
#endif

          /* Set output buffer */
          d_stream.next_out = hi->fd->block;
          d_stream.avail_out = hi->fd->blocksize;

          err=inflate(&d_stream, Z_SYNC_FLUSH);
#ifdef DEVELOPER
          printf("Readdata: gzip inflate result=%ld, avail_out=%d\n", err, d_stream.avail_out);
#endif

          if (err != Z_OK && err != Z_STREAM_END)
          {
#ifdef DEVELOPER
            if(httpdebug)
            {
                if (err==Z_DATA_ERROR) printf ("zlib DATA ERROR\n");
                if (err==Z_STREAM_ERROR) printf ("zlib STREAM ERROR\n");
                if (err==Z_NEED_DICT) printf ("zlib NEED DICT\n");
                if (err==Z_MEM_ERROR) printf ("zlib MEM ERROR\n");
                if (err==Z_BUF_ERROR) printf ("zlib BUF ERROR\n");
            }
#endif
            gzip_end = 1;
          }

          if (err == Z_STREAM_END)
          {
#ifdef DEVELOPER
              printf("Readdata: gzip decompression complete\n");
#endif
              gzip_end = 1;
          }

          /* Check if we need more input data */
          if ((err == Z_OK || err == Z_BUF_ERROR) && d_stream.avail_in == 0 && !gzip_end)
          {
             gziplength = Receive(hi, gzipbuffer, gzip_buffer_size);
             if( gziplength <= 0 ){
               gzip_end = 1;
             }else{
               d_stream.next_in = gzipbuffer;
               d_stream.avail_in = gziplength;
             }
          }

          hi->blocklength = hi->fd->blocksize - d_stream.avail_out;

        }else{
#ifdef DEVELOPER
          printf("Readdata: reading more data (not gzip)...\n");
#endif
          if (!Readblock(hi)) {
#ifdef DEVELOPER
              printf("Readdata: Readblock() failed, connection ended\n");
#endif
              break;
          }
#ifdef DEVELOPER
          printf("Readdata: read %ld more bytes (not gzip)\n", hi->blocklength);
#endif

        }

    }
    if (bdcopy)
        FREE(bdcopy);

    if(hi->flags & HTTPIF_GZIPDECODING){
      FREE(gzipbuffer);
      inflateEnd(&d_stream);
    }

    /* Cleanup accumulation buffer */
    if (chunk_gzip_buffer)
        FREE(chunk_gzip_buffer);

#ifdef DEVELOPER
    printf("Readdata: function exiting, final flags=0x%x, result=%s\n",
           hi->flags, result ? "TRUE" : "FALSE");
#endif

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
#ifdef DEVELOPER
        printf("Httpresponse: response read successfully, processing headers...\n");
#endif
        Nextline(hi);
        hi->flags |= HTTPIF_HEADERS;
        if (Readheaders(hi))
        {
#ifdef DEVELOPER
            printf("Httpresponse: headers processed successfully\n");
#endif
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
#ifdef DEVELOPER
                    printf("Httpresponse: reading content data (non-multipart)...\n");
#endif
                    Readdata(hi);
                }
            }
        }
#ifdef DEVELOPER
        else
        {
            printf("Httpresponse: Readheaders() failed\n");
        }
#endif
    }
    else
    {
#ifdef DEVELOPER
        printf("Httpresponse: Readresponse() failed, reading as plain data...\n");
#endif
        Readdata(hi);
    }
}

/* Send a message */
static long Send(struct Httpinfo *hi, UBYTE * request, long reqlen)
{
    long            result = -1;

    if (hi->flags & HTTPIF_SSL)
    {
        result = Assl_write(hi->assl, request, reqlen);
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

#ifdef DEVELOPER
    printf("Openlibraries: opening TCP library\n");
#endif
    Opentcp(&hi->socketbase, hi->fd, !hi->fd->validate);
    if (hi->socketbase)
    {
#ifdef DEVELOPER
        printf("Openlibraries: TCP library opened successfully\n");
#endif
        result = TRUE;
        if (hi->flags & HTTPIF_SSL)
        {
#ifdef DEVELOPER
            printf("Openlibraries: SSL requested, opening SSL library\n");
#endif
            if (hi->assl = Tcpopenssl(hi->socketbase))
            {
#ifdef DEVELOPER
                printf("Openlibraries: SSL library opened successfully\n");
#endif
            }
            else
            {
#ifdef DEVELOPER
                printf("Openlibraries: SSL library failed to open\n");
#endif
                if (Securerequest
                    (hi, haiku ? HAIKU12 : AWEBSTR(MSG_SSLWARN_SSL_NO_SSL2)))
                {
#ifdef DEVELOPER
                    printf("Openlibraries: user chose to continue without SSL\n");
#endif
                    hi->flags &= ~HTTPIF_SSL;
                }
                else
                {
#ifdef DEVELOPER
                    printf("Openlibraries: user cancelled, no SSL connection\n");
#endif
                    result = FALSE;
                }
            }
        }
    }
    else
    {
#ifdef DEVELOPER
        printf("Openlibraries: TCP library failed to open\n");
#endif
    }
    return result;
}

/* Create SSL context, SSL and socket */
static long Opensocket(struct Httpinfo *hi, struct hostent *hent)
{
    long            sock;

#ifdef DEVELOPER
    printf("Opensocket: creating socket\n");
#endif

    if (hi->flags & HTTPIF_SSL)
    {
#ifdef DEVELOPER
        printf("Opensocket: SSL requested, opening SSL context\n");
#endif
        if (!Assl_openssl(hi->assl))
        {
#ifdef DEVELOPER
            printf("Opensocket: Assl_openssl() failed\n");
#endif
            return -1;
        }
#ifdef DEVELOPER
        printf("Opensocket: SSL context opened successfully\n");
#endif
    }
    sock = a_socket(hent->h_addrtype, SOCK_STREAM, 0, hi->socketbase);
#ifdef DEVELOPER
    printf("Opensocket: a_socket() returned %ld\n", sock);
#endif
    if (sock < 0)
    {
        Assl_closessl(hi->assl);
    }
    return sock;
}

/* Connect and make SSL connection. Returns TRUE if success. */
static BOOL Connect(struct Httpinfo *hi, struct hostent *hent)
{
    BOOL            ok = FALSE;

#ifdef DEVELOPER
    printf("Connect: starting connection to %s:%ld ssl=%d\n", hi->hostname, hi->port, BOOLVAL(hi->flags & HTTPIF_SSL));
#endif

    if (hi->port == -1)
    {
        if (hi->flags & HTTPIF_SSL)
            hi->port = 443;
        else
            hi->port = 80;
    }

#ifdef DEVELOPER
    printf("Connect: using port %ld\n", hi->port);
#endif

    if (!a_connect(hi->sock, hent, hi->port, hi->socketbase))
    {
#ifdef DEVELOPER
        printf("Connect: TCP connection established\n");
#endif
        if (hi->flags & HTTPIF_SSL)
        {
#ifdef DEVELOPER
            printf("Connect: starting SSL handshake\n");
#endif
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
#ifdef DEVELOPER
                printf("Connect: attempting SSL connection to %s\n", hi->hostname);
#endif
                long            result =
                    Assl_connect(hi->assl, hi->sock, hi->hostname);
                ok = (result == ASSLCONNECT_OK);
#ifdef DEVELOPER
                printf("Connect: SSL connect result=%ld (OK=%d, DENIED=%d)\n", result, ASSLCONNECT_OK, ASSLCONNECT_DENIED);
                printf("Connect: ok=%s, flags=0x%x, NOSSLREQ=%s\n",
                       ok ? "TRUE" : "FALSE",
                       hi->flags,
                       (hi->flags & HTTPIF_NOSSLREQ) ? "TRUE" : "FALSE");
#endif
                if (result == ASSLCONNECT_DENIED)
                {
#ifdef DEVELOPER
                    printf("Connect: SSL connection denied\n");
#endif
                    hi->flags |= HTTPIF_NOSSLREQ;
                }
                if (!ok && !(hi->flags & HTTPIF_NOSSLREQ))
                {
#ifdef DEVELOPER
                    printf("Connect: SSL failed, entering error handling\n");
#endif
                    UBYTE           errbuf[128], *p;

                    p = Assl_geterror(hi->assl, errbuf);
#ifdef DEVELOPER
                    printf("Connect: SSL error: %s\n", p ? (char*)p : "unknown");
#endif
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
#ifdef DEVELOPER
    printf("Connect: returning %s\n", ok ? "TRUE" : "FALSE");
#endif
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

#ifdef DEVELOPER
    printf("Httpretrieve: starting retrieval for %s\n", fd->name);
#endif

    hi->blocklength = 0;
    hi->nextscanpos = 0;
    hi->chunk_remaining = 0;
    hi->chunk_state = CHUNK_SIZE;
    if (fd->flags & FDVF_SSL)
    {
        hi->flags |= HTTPIF_SSL;
#ifdef DEVELOPER
        printf("Httpretrieve: SSL flag set\n");
#endif
    }
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
#ifdef DEVELOPER
            printf("Httpretrieve: looking up %s\n", hi->connect);
#endif
            if (hent = Lookup(hi->connect, hi->socketbase))
            {
#ifdef DEVELOPER
                printf("Httpretrieve: DNS lookup successful\n");
#endif
                if ((hi->sock = Opensocket(hi, hent)) >= 0)
                {
#ifdef DEVELOPER
                    printf("Httpretrieve: socket opened, sock=%ld\n", hi->sock);
#endif
                    Updatetaskattrs(AOURL_Netstatus, NWS_CONNECT, TAG_END);
                    /* hent->h_name is the A entry, might be punyencoded */
                    Tcpmessage(fd, TCPMSG_CONNECT,
                               hi->flags & HTTPIF_SSL ? "HTTPS" : "HTTP",
                               hent->h_name);
#ifdef DEVELOPER
                    printf("Httpretrieve: attempting to connect...\n");
#endif
                    if (Connect(hi, hent))
                    {
#ifdef DEVELOPER
                        printf("Httpretrieve: Connect() succeeded, building request...\n");
#endif

                        if (hi->flags & HTTPIF_SSL)
                        {
#ifdef DEVELOPER
                            printf("Httpretrieve: SSL connection, getting cipher info...\n");
#endif
                            p = Assl_getcipher(hi->assl);
                            q = Assl_libname(hi->assl);
                            if (p || q)
                            {
                                Updatetaskattrs(AOURL_Cipher, p,
                                                AOURL_Ssllibrary, q, TAG_END);
                            }
                        }

                        reqlen = Buildrequest(fd, hi, &request);
#ifdef DEVELOPER
                        printf("Httpretrieve: built request, length=%ld\n", reqlen);
#endif
                        result = (Send(hi, request, reqlen) == reqlen);
#ifdef DEVELOPER
                        printf("Httpretrieve: send result=%s (expected %ld bytes)\n", result ? "SUCCESS" : "FAILED", reqlen);
#endif
#ifdef BETAKEYFILE
                        if (httpdebug)
                        {
                            Write(Output(), "\n", 1);
                            Write(Output(), request, reqlen);
                        }
#endif
                        if (result)
                        {
#ifdef DEVELOPER
                            printf("Httpretrieve: request sent successfully, processing data...\n");
#endif
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
#ifdef DEVELOPER
                            printf("Httpretrieve: waiting for response...\n");
#endif
                            Updatetaskattrs(AOURL_Netstatus, NWS_WAIT,
                                            TAG_END);
                            Tcpmessage(fd, TCPMSG_WAITING,
                                       hi->
                                       flags & HTTPIF_SSL ? "HTTPS" : "HTTP");
                            Httpresponse(hi, TRUE);
#ifdef DEVELOPER
                            printf("Httpretrieve: response processing completed\n");
#endif
                        }
                        else
                        {
#ifdef DEVELOPER
                            printf("Httpretrieve: send failed, setting error=TRUE\n");
#endif
                            error = TRUE;
                        }
                        if (hi->assl)
                        {
                            Assl_closessl(hi->assl);
                        }
                        a_close(hi->sock, hi->socketbase);
                    }
                    else if (!(hi->flags & HTTPIF_RETRYNOSSL) &&
                             hi->status != 407)
                    {
#ifdef DEVELOPER
                        printf("Httpretrieve: connection failed, status=%ld\n", hi->status);
#endif
                        Tcperror(fd, TCPERR_NOCONNECT,
                                 (hi->flags & HTTPIF_SSLTUNNEL) ? hi->
                                 hostport : (UBYTE *) hent->h_name);
                    }
                }
                else
                {
#ifdef DEVELOPER
                    printf("Httpretrieve: Opensocket() failed\n");
#endif
                    error = TRUE;
                }
            }
            else
            {
#ifdef DEVELOPER
                printf("Httpretrieve: DNS lookup failed for %s\n", hi->connect);
#endif
                Tcperror(fd, TCPERR_NOHOST, hi->hostname);
            }
            a_cleanup(hi->socketbase);
        }
        else
        {
            Tcperror(fd, TCPERR_NOLIB);
            if (hi->assl)
            {
                Assl_cleanup(hi->assl);
                hi->assl = NULL;
            }
            if (hi->socketbase)
            {
                Closeaweblib(hi->socketbase);   /* this is now closed by Closeaweblib as hi->socketbase may be a lib os3 or an interface os4 */
                hi->socketbase = NULL;
            }
        }
    }
    if (error)
    {
        Updatetaskattrs(AOURL_Error, TRUE, TAG_END);
    }
}

/*-----------------------------------------------------------------------*/
/* Connection Pool Management Functions */
/*-----------------------------------------------------------------------*/

static void Initconnectionpool(void)
{
    NEWLIST(&connectionpool);
    InitSemaphore(&poolsema);
    totalconnections = 0;
}

static void Freeconnectionpool(void)
{
    struct Httpconn *conn;
    ObtainSemaphore(&poolsema);
    while (conn = (struct Httpconn *)REMHEAD(&connectionpool))
    {
        if (conn->assl)
            Assl_closessl(conn->assl);
        if (conn->sock >= 0)
            a_close(conn->sock, conn->socketbase);
        if (conn->hostname)
            FREE(conn->hostname);
        FREE(conn);
    }
    totalconnections = 0;
    ReleaseSemaphore(&poolsema);
}

static struct Httpconn *Findconnection(UBYTE *hostname, long port, BOOL ssl)
{
    struct Httpconn *conn;
    ULONG now = Today();

    ObtainSemaphore(&poolsema);
    for (conn = connectionpool.first; conn->next; conn = conn->next)
    {
        if (conn->port == port && conn->ssl == ssl &&
            STRIEQUAL(conn->hostname, hostname))
        {
            if (now - conn->lastused < CONNECTION_IDLE_TIMEOUT &&
                conn->requests < conn->maxrequests)
            {
                REMOVE(conn);
                ReleaseSemaphore(&poolsema);
                return conn;
            }
        }
    }
    ReleaseSemaphore(&poolsema);
    return NULL;
}

static void Returnconnection(struct Httpconn *conn, BOOL closeconn)
{
    if (!conn || !conn->keepalive || closeconn)
    {
        if (conn)
        {
            if (conn->assl)
                Assl_closessl(conn->assl);
            if (conn->sock >= 0)
                a_close(conn->sock, conn->socketbase);
            if (conn->hostname)
                FREE(conn->hostname);
            FREE(conn);
            ObtainSemaphore(&poolsema);
            totalconnections--;
            ReleaseSemaphore(&poolsema);
        }
        return;
    }

    conn->lastused = Today();
    ObtainSemaphore(&poolsema);
    ADDTAIL(&connectionpool, conn);
    ReleaseSemaphore(&poolsema);
}

static struct Httpconn *Createconnection(UBYTE *hostname, long port, BOOL ssl,
                                        struct Library *socketbase, struct Assl *assl, long sock)
{
    struct Httpconn *conn;

    if (conn = ALLOCSTRUCT(Httpconn, 1, MEMF_CLEAR))
    {
        conn->hostname = Dupstr(hostname, -1);
        conn->port = port;
        conn->ssl = ssl;
        conn->sock = sock;
        conn->assl = assl;
        conn->socketbase = socketbase;
        conn->lastused = Today();
        conn->created = Today();
        conn->requests = 0;
        conn->maxrequests = 100; /* Default, will be updated from Keep-Alive header */
        conn->timeout = DEFAULT_KEEPALIVE_TIMEOUT;
        conn->keepalive = TRUE;

        ObtainSemaphore(&poolsema);
        totalconnections++;
        ReleaseSemaphore(&poolsema);
    }
    return conn;
}

static struct Httpconn *Getconnection(struct Httpinfo *hi, struct hostent *hent)
{
    struct Httpconn *conn = NULL;

#ifdef DEVELOPER
    printf("Getconnection: host=%s port=%ld ssl=%d\n", hi->hostname, hi->port, BOOLVAL(hi->flags & HTTPIF_SSL));
#endif

    /* Check if keep-alive is enabled in preferences */
    if (!prefs.network.keepalive)
    {
#ifdef DEVELOPER
        printf("Getconnection: keep-alive disabled\n");
#endif
        return NULL;
    }

    /* Try to find existing connection */
    conn = Findconnection(hi->hostname, hi->port, BOOLVAL(hi->flags & HTTPIF_SSL));

    if (!conn)
    {
#ifdef DEVELOPER
        printf("Getconnection: no existing connection, creating new\n");
#endif
        /* Create new connection */
        if (Openlibraries(hi))
        {
#ifdef DEVELOPER
            printf("Getconnection: libraries opened successfully\n");
#endif
            if ((hi->sock = Opensocket(hi, hent)) >= 0)
            {
#ifdef DEVELOPER
                printf("Getconnection: socket opened successfully, sock=%ld\n", hi->sock);
#endif
                if (Connect(hi, hent))
                {
#ifdef DEVELOPER
                    printf("Getconnection: connection established successfully\n");
#endif
                    conn = Createconnection(hi->hostname, hi->port,
                                          BOOLVAL(hi->flags & HTTPIF_SSL),
                                          hi->socketbase, hi->assl, hi->sock);
                }
                else
                {
#ifdef DEVELOPER
                    printf("Getconnection: Connect() failed\n");
#endif
                }
            }
            else
            {
#ifdef DEVELOPER
                printf("Getconnection: Opensocket() failed\n");
#endif
            }
        }
        else
        {
#ifdef DEVELOPER
            printf("Getconnection: Openlibraries() failed\n");
#endif
        }
    }
    else
    {
#ifdef DEVELOPER
        printf("Getconnection: reusing existing connection\n");
#endif
        /* Reuse existing connection */
        hi->sock = conn->sock;
        hi->assl = conn->assl;
        hi->socketbase = conn->socketbase;
        conn->requests++;
    }

    return conn;
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
    Initconnectionpool();
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
    Freeconnectionpool();
#endif
}
