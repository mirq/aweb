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

/* ILBMExample.awebplugin
 *
 * Example AWeb plugin module - library code
 *
 * This example implements a simple progressive ILBM decoder.
 * It is slow, uses only standard bitmaps, doesn't support
 * scaling or transparency.
 * It is a fairly good example of how to write an AWeb plugin
 * module, though.
 *
 */

#include "pluginlib.h"
#include "ilbmexample.h"
#include <libraries/awebsupport.h>
#include <libraries/awebmodule.h>
#include <clib/awebsupport_protos.h>
/*#include <clib/exec_protos.h>*/
#undef   NO_INLINE_STDARG
#include <proto/awebsupport.h>
#define  NO_INLINE_STDARG
#include <proto/exec.h>

/* Library bases we use */
struct GfxBase *GfxBase;
struct UtilityBase *UtilityBase;
struct Library *AwebSupportBase;

/* Actions to be performed when our library has been loaded */
ULONG Initpluginlib(struct ILBMExampleBase *base)
{
   /* Initialise libnix */
   if(!AwebModuleInit()) return FALSE;

   /* Open the libraries we need */
   if(!(GfxBase=OpenLibrary("graphics.library",39))) return FALSE;
   if(!(UtilityBase=OpenLibrary("utility.library",39))) return FALSE;
   if(!(AwebSupportBase=OpenLibrary("awebsupport.library",0))) return FALSE;

   return TRUE;
}

/* Actions to be performed when our library is about to be expunged */
void Expungepluginlib(struct ILBMExampleBase *base)
{
   /* If we have installed our dispatchers, deinstall them now. */
   if(base->sourcedriver) Amethod(NULL,AOM_INSTALL,base->sourcedriver,NULL);
   if(base->copydriver) Amethod(NULL,AOM_INSTALL,base->copydriver,NULL);

   /* Close all opened libraries */
   if(AwebSupportBase) CloseLibrary(AwebSupportBase);
   if(UtilityBase) CloseLibrary(UtilityBase);
   if(GfxBase) CloseLibrary(GfxBase);
   AwebModuleExit();
}

/* The first library function */
USRFUNC_H1
(
__saveds ULONG  , Initplugin,
struct Plugininfo *,pi,A0
)
{
    USRFUNC_INIT

   /* Install our dispatchers */
   if(!AwebPluginBase->sourcedriver)
   {  AwebPluginBase->sourcedriver=Amethod(NULL,AOM_INSTALL,0,Dispatchersource);
   }
   if(!AwebPluginBase->copydriver)
   {  AwebPluginBase->copydriver=Amethod(NULL,AOM_INSTALL,0,Dispatchercopy);
   }

   /* Return the class IDs */
   pi->sourcedriver=AwebPluginBase->sourcedriver;
   pi->copydriver=AwebPluginBase->copydriver;

   /* Return nonzero if success */
   return (ULONG)(AwebPluginBase->sourcedriver && AwebPluginBase->copydriver);

    USRFUNC_EXIT
}

/* Query function. Handle in a safe way. There is generally no need to
 * change this code, it is generated from the #defines in pluginlib.h */

#define ISSAFE(s,f) (s->structsize>=((long)&s->f-(long)s+sizeof(s->f)))

USRFUNC_H1
(
__saveds void  , Queryplugin,
struct Pluginquery *,pq,A0
)
{
    USRFUNC_INIT

#ifdef PLUGIN_COMMANDPLUGIN
   if(ISSAFE(pq,command)) pq->command=TRUE;
#endif
#ifdef PLUGIN_FILTERPLUGIN
   if(ISSAFE(pq,filter)) pq->filter=TRUE;
#endif

    USRFUNC_EXIT
}
