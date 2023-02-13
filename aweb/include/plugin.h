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

/* plugin.h - AWeb plugin interface */

#ifndef AWEB_AWEBPLUGIN_H
#define AWEB_AWEBPLUGIN_H

struct Plugininfo
{  ULONG sourcedriver;
   ULONG copydriver;
};

struct Pluginquery
{  long structsize;     /* Set to struct size by caller */
   BOOL command;        /* Set to TRUE if plugin supports
                         * Commandplugin() */
   BOOL filter;         /* Set to TRUE if this plugin acts as a filter */
};

struct Plugincommand
{  long structsize;     /* Set to struct size by caller */
   UBYTE *command;      /* Set to command by caller, modifications
                         * by plugin allowed. */
   UBYTE *result;       /* Result allocated by plugin, freed by caller.
                         * Leave NULL if no result returned. */
   long rc;             /* Return code. 0 for success, nonzero for
                         * warning or failure. */
};

struct Pluginfilter     /* All fields except userdata are read-only! */
{  long structsize;     /* Set to struct size by caller */
   void *handle;        /* Use in Setfiltertype() and Writefilter() */
   STRPTR data;         /* Block of data or NULL */
   long length;         /* Length of data */
   BOOL eof;            /* TRUE if EOF was reached */
   STRPTR contenttype;  /* MIME content-type of data */
   STRPTR url;          /* URL of this file */
   void *userdata;      /* Plugin private data */
   STRPTR encoding;     /* encoding of text data */
};

extern ULONG Initplugin(struct Plugininfo *plugininfo);
   /* If not done yet, initialize plugin and install its dispatchers.
    * Returns the class IDs for source and copy drivers, or zero, in
    * plugininfo.
    * Returns nonzero if success, zero if failure. */

extern void Queryplugin(struct Pluginquery *pluginquery);
   /* Only assumed to exist if the library's NegSize >30.
    * Fills in the Pluginquery structure with the supported
    * library entries */

extern void Commandplugin(struct Plugincommand *plugincommand);
   /* Only assumed to exist if Pluginquery.command is set.
    * Executes the command and optionally returns a value. */

extern void Filterplugin(struct Pluginfilter *pf);
   /* Only assumed to exist if Pluginquery.filter is set.
    * Filters incoming data. */

#include <proto/awebplugin.h>

#endif
