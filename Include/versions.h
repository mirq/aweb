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

   $Id: versions.h,v 1.6 2009/06/15 17:03:02 opi Exp $

   Desc:

***********************************************************************/

/*
The file defines the required version numbers and names for the aweblibs
It should be included by any file using the openaweblib function.
*/

/* default awelib path */

#define AWEBLIBPATH "AWebPath:aweblib/"
/* aweblib names */

#define JAVASCRIPT_AWEBLIB   "javascript.aweblib"
#define AREXX_AWEBLIB        "arexx.aweblib"
#define AUTHORIZE_AWEBLIB    "authorize.aweblib"
#define CACHEBROWSER_AWEBLIB "cachebrowser.aweblib"
#define FTP_AWEBLIB          "ftp.aweblib"
#define GOPHER_AWEBLIB       "gopher.aweblib"
#define HISTORY_AWEBLIB      "history.aweblib"
#define HOTLIST_AWEBLIB      "hotlist.aweblib"
#define MAIL_AWEBLIB         "mail.aweblib"
#define NEWS_AWEBLIB         "news.aweblib"
#define PRINT_AWEBLIB        "print.aweblib"
#define STARTUP_AWEBLIB      "startup.aweblib"


/* aweblib required versions */
/*
NOTE: these are the versions of each library that aweb requires not
necessarily the most recent version of any given aweblib
*/

#define JAVASCRIPT_VERSION   38
#define AREXX_VERSION        37
#define AUTHORIZE_VERSION    36
#define CACHEBROWSER_VERSION 37
#define FTP_VERSION          37
#define GOPHER_VERSION       36
#define HISTORY_VERSION      36
#define HOTLIST_VERSION      36
#define MAIL_VERSION         37
#define NEWS_VERSION         37
#define PRINT_VERSION        37
#define STARTUP_VERSION      36
