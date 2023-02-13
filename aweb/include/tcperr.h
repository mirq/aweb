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

/* tcperr.h - AWeb tcp errors */

enum TCP_ERRORS
{  TCPERR_NOLIB=1,   /* no tcp stack */
   TCPERR_NOHOST,    /* host ... not found */
   TCPERR_NOCONNECT, /* connection failed ... */
   TCPERR_NOFILE,    /* local file not found ... */
   TCPERR_XAWEB,     /* invalid x-aweb name ... */
   TCPERR_NOLOGIN,   /* cannot login at ... as ... */
};

enum TCP_MESSAGES
{  TCPMSG_LOOKUP=1,  /* looking up ... */
   TCPMSG_CONNECT,   /* making ... connection to ... */
   TCPMSG_WAITING,   /* ... awaiting response */
   TCPMSG_TCPSTART,  /* beginning TCP connection */
   TCPMSG_LOGIN,     /* loggin in at ... */
   TCPMSG_NEWSGROUP, /* scanning ... */
   TCPMSG_NEWSSCAN,  /* scanning ... (... articles) */
   TCPMSG_NEWSSORT,  /* sorting ... */
   TCPMSG_NEWSPOST,  /* posting news */
   TCPMSG_MAILSEND,  /* sending mail */
   TCPMSG_UPLOAD,    /* uploading file */
};
