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

/* fetchdriver.h - AWeb fetch driver interface */

#ifndef AWEB_FETCHDRIVER_H
#define AWEB_FETCHDRIVER_H

#include "url.h"

/*--- Communications structure ---*/

struct Fetchdriver
{  void *fetch;               /* The owner FETCH object */

   /* Input fields, set by FETCH and read by FETCHDRIVER */

   UBYTE *name;               /* Name to fetch */
   UBYTE *postmsg;            /* Message to POST or NULL */
   struct Multipartdata *multipart; /* Multipart form data to post or NULL */
   UBYTE *proxy;              /* Proxy to use or NULL */
   UBYTE *referer;            /* Referer address or NULL */
   UBYTE *block;              /* Pre-assigned data block. May be changed after msg replied */
   long blocksize;            /* Maximum size of data block */
   ULONG validate;            /* Only retrieve if modified since this date. */
   UWORD flags;              /* See below */
   struct AwebPrefs *prefs;       /* Preferences */
   struct SignalSemaphore *prefssema;  /* Protect preferences */

   /* Temporary storage fields */
   ULONG serverdate;          /* Date as reported by server */
   ULONG responsetime;      /* local time response recieved */
   UBYTE *etag;            /* Etag NULL */
   long keepalive_timeout;    /* Keep-alive timeout from server */
   long keepalive_max;        /* Keep-alive max requests from server */

};

#define FDVF_NOCACHE       0x0001   /* Don't use any cache */
#define FDVF_CACHERELOAD   0x0002   /* This is a cache reload (local file only) */
#define FDVF_COMMANDS      0x0004   /* Shell/ARexx commands are temporarily allowed */
#define FDVF_JSOPEN        0x0008   /* Unclosed JS generated document */
#define FDVF_SSL           0x0010   /* Use secure transfer if possible */
#define FDVF_FORMWARN      0x0020   /* Warn if form is sent over unsecure link */

#endif
