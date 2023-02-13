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

/* url.h - AWeb URL object */

#ifndef AWEB_URL_H
#define AWEB_URL_H

#include "object.h"

struct Url
{  struct Aobject object;
   UBYTE *url;                /* URL string */
   ULONG postnr;              /* POST id */
   UWORD flags;
   struct Url *movedto;       /* This URL is moved to (movedto) */
   struct Aobject *source;    /* Object source interpreter */
   struct Aobject *cache;     /* Object is in cache */
   struct Aobject *fetch;     /* Normal, new fetch */
   struct Aobject *ocache;    /* Old cache object before reload */
   struct Aobject *rfetch;    /* Reload from cache */
   struct Aobject *vfetch;    /* Verifying fetch */
   struct Aobject *vcache;    /* Preliminary cache for verified object */
   struct Aobject *vsource;   /* Preliminary source for verified object */
   struct Aobject *ssource;   /* Saveas source */
   struct Aobject *dsource;   /* Docext source */
   LIST(Child) childs;        /* Childs pointing to this URL */
   long loadnr;               /* Load sequence number */
};

#define URLF_VISITED    0x0001   /* Url is visited before */
#define URLF_TEMPMOVED  0x0002   /* Url was temporary moved, otherwise permanently */
#define URLF_CACHEABLE  0x0004   /* Url is cacheable */
#define URLF_VERIFIED   0x0008   /* Url has been validated in this session */
#define URLF_VOLATILE   0x0010   /* Never use existing copy */
#define URLF_DEXFETCH   0x0020   /* Current fetch is for docext, don't pass to source */
#define URLF_WASDUP     0x0040   /* Url was moved because it was duplicate */

/*--- url tags ---*/

#define AOURL_Dummy        AOBJ_DUMMYTAG(AOTP_URL)

#define AOURL_Url          (AOURL_Dummy+1)
   /* (UBYTE *) URL string */

#define AOURL_Postnr       (AOURL_Dummy+2)
   /* (ULONG) POST request ID */

#define AOURL_Visited      (AOURL_Dummy+3)
   /* (BOOL) Was this URL ever visited before */

#define AOURL_Source       (AOURL_Dummy+4)
   /* (void *) GET the source object for this URL. If none exists, one will be created. */

#define AOURL_Cacheable    (AOURL_Dummy+5)
   /* (BOOL) If object should ever be cached. Defaults to TRUE. */

#define AOURL_Cache        (AOURL_Dummy+6)   /* SET,GET */
   /* (void *) Current cache object for this URL. */

#define AOURL_Realurl      (AOURL_Dummy+7)   /* GET */
   /* (UBYTE *) Real URL string, not considering relocations. */

#define AOURL_Loadnr       (AOURL_Dummy+8)   /* GET */
   /* (long) Load sequence number */

#define AOURL_Isinmem      (AOURL_Dummy+9)   /* GET */
   /* (BOOL) If object for this url is in memory. */

#define AOURL_Sourcevalid  (AOURL_Dummy+10)  /* GET */
   /* (BOOL) If the current source is still valid, i.e. not expired and no fetch going on. */

#define AOURL_Saveassource (AOURL_Dummy+11)  /* GET */
   /* (void *) Get the downloading source object, or NULL. */

#define AOURL_Cachefetch   (AOURL_Dummy+12)  /* SET */
   /* (BOOL) When set to FALSE, the current fetch will not cache, but
    * future ones will (unless AOURL_Cacheable was set to FALSE). */

#define AOURL_Volatile     (AOURL_Dummy+13)  /* SET */
   /* (BOOL) Never used a cached copy or existing source of this URL. */

#define AOURL_Input        (AOURL_Dummy+14)  /* GET */
   /* (BOOL) If this URL is inputting */

#define AOURL_Docextsource (AOURL_Dummy+15)  /* GET */
   /* (void *) Get the document extension source, or NULL. */

#define AOURL_Finalurl     (AOURL_Dummy+16)  /* GET */
   /* (UBYTE *) Final URL string, solving all relocations (temporary and permanent). */

#define AOURL_Finalurlptr  (AOURL_Dummy+17)  /* GET */
   /* (void *) Final URL object, solving all relocations (temporary and permanent). */

#define AOURL_    (AOURL_Dummy+)
#define AOURL_    (AOURL_Dummy+)


/* Tags used in AOM_SRCUPDATE in fetch process, also passed to source objects. */

#define AOURL_Contenttype  (AOURL_Dummy+101)
   /* (UBYTE *) MIME type of object.
    * Also GET from URL object. */

#define AOURL_Contentlength (AOURL_Dummy+102)
   /* (long) Expected size of object */

#define AOURL_Lastmodified (AOURL_Dummy+103)
   /* (long) Last modified date of object, remote time, in system seconds */

#define AOURL_Expires      (AOURL_Dummy+104)
   /* (long) Expiry date of object, local time, in system seconds */

#define AOURL_Movedto      (AOURL_Dummy+105)
#define AOURL_Tempmovedto  (AOURL_Dummy+106)
   /* (UBYTE *) URL string that this object is moved to, permanently or temporary */

#define AOURL_Data         (AOURL_Dummy+107)
   /* (UBYTE *) Block of data */

#define AOURL_Datalength   (AOURL_Dummy+108)
   /* (long) Length of data block */

#define AOURL_Eof          (AOURL_Dummy+110)
   /* (BOOL) End of data. */

#define AOURL_Reload       (AOURL_Dummy+111)
   /* (BOOL) Starting a reload. Treat any previous data as obsolete. */

#define AOURL_Terminate    (AOURL_Dummy+112)
   /* (BOOL) Driver subprocess has terminated; FETCH is about to dispose itself. */

#define AOURL_Datatotal    (AOURL_Dummy+113)
   /* (long) Total length of data read so far */

#define AOURL_Status       (AOURL_Dummy+114)
   /* (UBYTE *) Statustext for windows */

#define AOURL_Error        (AOURL_Dummy+115)
   /* (BOOL) Error. Any response should be considered volatile and not replacing
    * existing disk cache copy. */

#define AOURL_Netstatus    (AOURL_Dummy+116)
   /* (ULONG) Current network status (used by netstat window) */

#define AOURL_Notmodified  (AOURL_Dummy+117)
   /* (BOOL) Object was not modified on a verify. */

#define AOURL_Serverdate   (AOURL_Dummy+118)
   /* (ULONG) Current date reported by server. */

#define AOURL_Arexxcommand (AOURL_Dummy+120)
   /* (UBYTE *) ARexxcommand to start. */

#define AOURL_Postnogood   (AOURL_Dummy+121)
   /* (BOOL) POSTing to this url is not supported, try again with GET */

#define AOURL_Serverpush   (AOURL_Dummy+122)
   /* (void *) The FETCH object to be cancelled when the object is no
    * longer displayed. */

#define AOURL_Clientpull   (AOURL_Dummy+123)
   /* (UBYTE *) The HTTP "Refresh:" header contents (like "30; URL=whatever; RELOAD") */

#define AOURL_Header       (AOURL_Dummy+124)
   /* (UBYTE *) A full HTTP response header */

#define AOURL_Fromcache    (AOURL_Dummy+125)
   /* (BOOL) AOURL_Headers are generated by cache */

#define AOURL_Seeother     (AOURL_Dummy+126)
   /* (UBYTE *) URL string for new location that must be retrieved with GET, not POST. */

#define AOURL_Jsopen       (AOURL_Dummy+127)
   /* (BOOL) This is a nonclosed (or closed if FALSE) JS generated document. */

#define AOURL_Cipher       (AOURL_Dummy+128)
   /* (UBYTE *) Cipher method used in secure connection */

#define AOURL_Gohistory    (AOURL_Dummy+129)
   /* (long) Go forward or backward in history (used from mailer). */

#define AOURL_Nocache      (AOURL_Dummy+130)
   /* (BOOL) Don't cache this response. */

#define AOURL_Contentscripttype (AOURL_Dummy+131)
   /* (UBYTE *) Default script type of contents (e.g. "text/javascript") */

#define AOURL_Foreign      (AOURL_Dummy+131)
   /* (BOOL) Document contains a foreign character set */

#define AOURL_Ssllibrary   (AOURL_Dummy+132)
   /* (UBYTE *) The SSL library used for secure connection */

#define AOURL_Charset      (AOURL_Dummy+133)
   /* (UBYTE *) Character set of object */

#define AOURL_Linkurl      (AOURL_Dummy+134)
   /* (UBYTE *) Url to link to (moved url for permanently moved original other wise) */

#define AOURL_Flushsource  (AOURL_Dummy+135)
   /* (BOOL) Flush the source of this url (used by tempmove */

#define AOURL_Etag      (AOURL_Dummy+136)
   /* (UBYTE *) Etag set of object */

#define AOURL_Filename (AOURL_Dummy+137)
    /* (UBYTE *) Suggested Filename */

#define AOURL_    (AOURL_Dummy+)
#define AOURL_    (AOURL_Dummy+)


/*--- Url relations ---*/

#define AOREL_URL_DUMMY    AOREL_DUMMYTAG(AOTP_URL)

#define AOREL_URL_LINK     (AOREL_URL_DUMMY+1)
   /* Links pointing to this url */

#define AOREL_URL_WINDOW   (AOREL_URL_DUMMY+2)
   /* Windows interested in status changes */


/*--- Url methods, also used by other objects ---*/

#define AUM_DUMMY          AOM_DUMMYTAG(AOTP_URL)

#define AUM_LOAD           (AUM_DUMMY+1)
   /* Load object */

#define AUM_SPECIAL        (AUM_DUMMY+2)
   /* Special operations */

/*--- Url messages ---*/

/* AUM_LOAD */
/* Load an URL, or reload it.
 * Any replies will be made as AOM_SRCUPDATE to the associated object. */
struct Aumload
{  struct Amessage amsg;
   ULONG flags;
   void *referer;             /* The URL object acting as referer. */
   UBYTE *postmsg;            /* Or (struct Multipartdata *) if AUMLF_MULTIPART is set */
   void *frame;               /* Target frame. Use to run javascript: urls in,
                               * and use its windows key to get the ARexx port,
                               * and the commands allowed switch. */
};

#define AUMLF_IFINMEM      0x00000001  /* React only if this object is already in memory */
#define AUMLF_IFINCACHE    0x00000002  /* React only if this object is in cache */
#define AUMLF_RELOAD       0x00000004  /* Reload this object unconditionally */
#define AUMLF_NOPROXY      0x00000008  /* Don't use proxy */
#define AUMLF_HISTORY      0x00000010  /* Don't verify if still in cache */
#define AUMLF_DOWNLOAD     0x00000020  /* Save or download only */
#define AUMLF_IMAGE        0x00000040  /* This load counts as image load */
#define AUMLF_VERIFY       0x00000080  /* Must verify this object */
#define AUMLF_NOICON       0x00000100  /* Don't create icons wneh saving */
#define AUMLF_FORMWARN     0x00000200  /* Warn if form is sent over unsecure link */
#define AUMLF_CHANNEL      0x00000400  /* Start a channel fetch.
                                        * Method return value is channel ID. */
#define AUMLF_MULTIPART    0x00000800  /* The postmsg message is actually a Multipartdata
                                        * header, not plain text. */
#define AUMLF_DOCEXT       0x00001000  /* Want this as a document extension */


/* AUM_SPECIAL */
/* A request for some special action for this url.
 */
struct Aumspecial
{  struct Amessage amsg;
   ULONG type;                      /* Action type, see below. */
};

#define AUMST_SAVESOURCE   1  /* Save the source */
#define AUMST_VIEWSOURCE   2  /* View the source */
#define AUMST_CANCELFETCH  3  /* Cancel any pending fetch for this url */
#define AUMST_DELETECACHE  4  /* Delete cache object for this url */
#define AUMST_EDITSOURCE   5  /* Edit the source */
#define AUMST_FLUSHSOURCE  6  /* Flush the source */


/*--- Url support functions ---*/

extern void *Findurl(UBYTE *base,UBYTE *url,long postnr);
   /* Return an existing or a new url object */

extern void *Findurlloadnr(long loadnr);
   /* Return the URL for this load number or NULL */

extern UBYTE *Makeabsurl(UBYTE *base,UBYTE *url);
   /* Returns dynamic string with resulting URL */

extern UBYTE *Urlfilename(UBYTE *url);
   /* Returns file name part of URL string (dynamic string) */

extern UBYTE *Urlfilenamefb(UBYTE *url);
   /* Returns file name part of URL string, but falls back on last path segment,
    * host name or scheme if no file name found (dynamic string) */

extern UBYTE *Urlfileext(UBYTE *url);
   /* Returns the filename extension part of URL string, or NULL. (dynamic string) */

extern UBYTE *Fragmentpart(UBYTE *url);
   /* Returns pointer to start of fragment within the supplied string, or NULL */

extern UBYTE *Fixurlname(UBYTE *name);
   /* Strip spaces, prepend default scheme. Returns dynamic string. */

extern UBYTE *Urllocalfilename(UBYTE *url);
   /* Returns local file name part of URL, or NULL */

extern UBYTE *Urladdfragment(UBYTE *url,UBYTE *fragment);
   /* Returns dynamic string with resulting url[#fragment] */

extern void Getjspart(struct Url *url,UWORD which,UBYTE **start,long *length);
   /* Finds this part of the URL's url string */

extern void *Repjspart(struct Url *url,UWORD which,UBYTE *part);
   /* Replaces this part of the URL's url string, and return new URL object */

extern void *Emptyurl(void);
   /* Obtain URL for dummy empty object */

extern BOOL Mailnewsurl(UBYTE *url);
   /* Returns TRUE if URL links to mailto: or news: url (except news article) */

extern BOOL Issamehost(struct Url *url1, struct Url *url2);
   /* Returns TRUE if both URLs contain the same domain names */

/* Getjspart which values */
#define UJP_PROTOCOL 1
#define UJP_HOST     2
#define UJP_PORT     3
#define UJP_HOSTNAME 4
#define UJP_PATHNAME 5
#define UJP_SEARCH   6

/* Message wrappers */
extern long Auload(void *url,ULONG flags,void *referer,UBYTE *postmsg,void *frame);
extern long Auspecial(void *url,ULONG type);

#endif
