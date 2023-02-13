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

/* FilterExample.awebplugin
 *
 * Example filter type AWeb plugin module - library code
 *
 * This example implements a simple filter that turns plain text
 * into HTML so it will be displayed in a proportional font.
 * It also implements two plugin commands: 'On' and 'Off'.
 *
 */

#include "pluginlib.h"
#include "filterexample.h"

#include <libraries/awebsupport.h>
#include <libraries/awebmodule.h>
#include <libraries/awebclib.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#undef NO_INLINE_STDARG
#include <proto/awebsupport.h>
#define NO_INLINE_STDARG
#include <proto/exec.h>
#include <string.h>

/* Library bases we use */
struct Library *AwebSupportBase;
#if defined(__amigaos4__)
struct AwebSupportIFace *IAwebSupport;
#endif

/* The on/off status of this plugin */
BOOL filteron;

/* Actions to be performed when our library has been loaded */
ULONG Initpluginlib(struct FilterExampleBase *base)
{
   /* Initilise platform specific environment */
   if (!AwebModuleInit()) return FALSE;

   /* Open the libraries we need */
   /* Filter plugins need version 2 or higher of the awebsupport library */

   if (!(AwebSupportBase=OpenLibrary("awebsupport.library",2))) return FALSE;

#if defined(__amigaos4__)
    if (!(IAwebSupport = (struct AwebSupportIFace *)GetInterface(AwebSupportBase,"main",1,0))) return FALSE;
#endif

   /* Set the filter ON by default: */
   filteron=TRUE;

   /* Return nonzero if success */
   return TRUE;
}

/* Actions to be performed when our library is about to be expunged */
void Expungepluginlib(struct FilterExampleBase *base)
{
   /* Close all opened libraries */
#if defined(__amigaos4__)
    if(IAwebSupport)
    {
        DropInterface((struct Interface *)IAwebSupport);
        IAwebSupport = NULL;
    }
#endif
   if(AwebSupportBase)
   {
        CloseLibrary(AwebSupportBase);
        AwebSupportBase = NULL;
   }
   /* Cleanup platform specifics */
   AwebModuleExit();
}

/* The first library function */
LIBFUNC_H1
(
__saveds ULONG  , Initplugin,
struct Plugininfo *,pi,A0,
PLUGIN_TYPE,PLUGIN_NAME
)
{
    LIBFUNC_INIT

   /* Return zero class IDs to indicate we're a filter plugin */
   pi->sourcedriver=0;
   pi->copydriver=0;

   /* Return nonzero (success) */
   return 1;

    LIBFUNC_EXIT
}

/* Query function. Handle in a safe way. There is generally no need to
 * change this code, it is generated from the #defines in pluginlib.h */

#define ISSAFE(s,f) (s->structsize>=((long)&s->f-(long)s+sizeof(s->f)))

LIBFUNC_H1
(
__saveds void  , Queryplugin,
struct Pluginquery *,pq,A0,
PLUGIN_TYPE,PLUGIN_NAME

)
{
    LIBFUNC_INIT
#ifdef PLUGIN_COMMANDPLUGIN
   if(ISSAFE(pq,command)) pq->command=TRUE;
#endif
#ifdef PLUGIN_FILTERPLUGIN
   if(ISSAFE(pq,filter)) pq->filter=TRUE;
#endif

    LIBFUNC_EXIT
}

/* The command function.
 * This function understands the 'On' and 'Off' commands. */
LIBFUNC_H1
(
__saveds void  , Commandplugin,
struct Plugincommand *,pc,A0,
PLUGIN_TYPE,PLUGIN_NAME
)
{
    LIBFUNC_INIT

   if(!stricmp(pc->command,"ON"))
   {
      filteron=TRUE;
   }
   else if(!stricmp(pc->command,"OFF"))
   {
      filteron=FALSE;
   }
   else
   {
      /* Invalid command */
      pc->rc=10;
   }

    LIBFUNC_EXIT
}

/* The filter function */
LIBFUNC_H1
(
__saveds void  , Filterplugin,
struct Pluginfilter *,pf,A0,
PLUGIN_TYPE,PLUGIN_NAME

)
{
    LIBFUNC_INIT

   STRPTR p,begin,end,replace;
   struct Filterdata *fd;
   STRPTR header1="<html><head><title>";
   STRPTR header2="</title></head><body><nobr>";

   /* See if there is already a userdata for us.
    * If not, allocate and initialize. */
   fd=pf->userdata;
   if(!fd)
   {
      fd=Allocmem(sizeof(struct Filterdata),MEMF_CLEAR);
      if(!fd) return;
      pf->userdata=fd;
      fd->first=TRUE;
      fd->on=filteron;
   }

   if(pf->data)
   {
      if(fd->on)
      {
         /* Before the first data, change the content type
          * and write a proper HTML header */
         if(fd->first)
         {
            Setfiltertype(pf->handle,"text/html");
            Writefilter(pf->handle,header1,strlen(header1));
            Writefilter(pf->handle,pf->url,strlen(pf->url));
            Writefilter(pf->handle,header2,strlen(header2));
            fd->first=FALSE;
         }

         /* Scan the text, replace each newline by "<br>", each '<'
          * by "&lt;", and each '&' by "&amp;".
          *
          * This process is triggered on single characters, so it will
          * work fine even on partial lines. If a more complex processing
          * is required, we would need to remember the partial last line in
          * our userdata and process it together with the next block.
          *
          * A more sophisticated filter would detect multiple spaces and
          * spaces at the beginning of lines, and turn them into "&nbsp;"
          * sequences; maybe detect lines of --- or | and keep these lines
          * preformatted to preserve ASCII art; etc.
          *
          * This filter doesn't do that, to keep the example simple. */

         end=pf->data+pf->length;
         begin=pf->data;
         for(p=begin;p<end;p++)
         {
            switch(*p)
            {
               case '\n':
                  replace="<br>\n";
                  break;

               case '<':
                  replace="&lt;";
                  break;

               case '&':
                  replace="&amp;";
                  break;

               default:
                  replace=NULL;
                  break;
            }
            if(replace)
            {
               Writefilter(pf->handle,begin,p-begin);
               Writefilter(pf->handle,replace,strlen(replace));
               begin=p+1;
            }
         }

         /* Don't forget to write the last line */
         Writefilter(pf->handle,begin,p-begin);
      }
      else
      {
         /* Don't filter data. Just write the entire block unchanged. */
         if(pf->data)
         {
            Writefilter(pf->handle,pf->data,pf->length);
         }
      }
   }

   /* Delete our userdata when eof was reached */
   if(pf->eof)
   {
      Freemem(fd);
      pf->userdata=NULL;
   }

    LIBFUNC_EXIT
}
