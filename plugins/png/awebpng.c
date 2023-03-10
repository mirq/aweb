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

/* awebpng.c - AWeb png plugin main */

#include "pluginlib.h"
#include "awebpng.h"
#include "libraries/awebsupport.h"
#include "libraries/awebmodule.h"
#include  "libraries/awebclib.h"
#include <exec/semaphores.h>

#undef NO_INLINE_STDARG
#include <proto/awebsupport.h>
#define NO_INLINE_STDARG
#include <proto/exec.h>

struct GfxBase *GfxBase = NULL;
struct UtilityBase *UtilityBase = NULL;
struct Library *AwebSupportBase = NULL;

#if !defined(__amigaos4__)

struct Library *CyberGfxBase = NULL;

#else

struct Library *P96Base;

struct P96IFace *IP96;
struct GraphicsIFace *IGraphics;
struct UtilityIFace *IUtility;
struct AwebSupportIFace *IAwebSupport;

#endif

#if defined(__MORPHOS__)
struct IntuitionBase *IntuitionBase;
struct Library *PNGBase;
#endif

ULONG Initpluginlib(struct AwebPngBase *base)
{
   if(!AwebModuleInit()) return FALSE;

   if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39))) return FALSE;
   if(!(UtilityBase = (struct UtilityBase *)OpenLibrary("utility.library",39))) return FALSE;
   if(!(AwebSupportBase = OpenLibrary("awebsupport.library",0))) return FALSE;

#if defined(__MORPHOS__)
   if(!(CyberGfxBase = OpenLibrary("cybergraphics.library",50))) return FALSE;
   if(!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0))) return FALSE;
   if(!(PNGBase = OpenLibrary("png.library", 51))) return FALSE;
#elif !defined(__amigaos4__)
   /*Cybergraphics is not critical*/
   CyberGfxBase = OpenLibrary("cybergraphics.library",0);
#endif

#if defined(__amigaos4__)
    if(!(P96Base = OpenLibrary("Picasso96API.library",0))) return FALSE;
    if(!(IP96 = (struct P96IFace *)GetInterface(P96Base,"main",1,0))) return FALSE;

    if(!(IGraphics = (struct GraphicsIFace *)GetInterface(GfxBase,"main",1,0))) return FALSE;
    if(!(IUtility = (struct UtilityIFace *)GetInterface(UtilityBase,"main",1,0))) return FALSE;
    if(!(IAwebSupport = (struct AwebpluginIFace *)GetInterface(AwebSupportBase,"main",1,0))) return FALSE;

#endif

   return TRUE;
}

void Expungepluginlib(struct AwebPngBase *base)
{
   if(base->sourcedriver) Amethod(NULL,AOM_INSTALL,base->sourcedriver,NULL);
   if(base->copydriver) Amethod(NULL,AOM_INSTALL,base->copydriver,NULL);

#if defined(__amigaos4__)
    if(IAwebSupport)DropInterface((struct Interface *)IAwebSupport);
    if(IGraphics)DropInterface((struct Interface *)IGraphics);
    if(IUtility)DropInterface((struct Interface *)IUtility);

    if(IP96)DropInterface((struct Interface*)IP96);
    if(P96Base)CloseLibrary(P96Base);
#else

   if(CyberGfxBase) CloseLibrary(CyberGfxBase);
#endif
   if(AwebSupportBase) CloseLibrary(AwebSupportBase);
   if(UtilityBase) CloseLibrary((struct Library *)UtilityBase);
   if(GfxBase) CloseLibrary((struct Library *)GfxBase);

#if defined(__MORPHOS__)
   CloseLibrary(PNGBase);
   CloseLibrary((struct Library *)IntuitionBase);
#endif

   AwebModuleExit();
}

LIBFUNC_H1
(
__saveds ULONG  , Initplugin,
struct Plugininfo *,pi,A0,
PLUGIN_TYPE, PLUGIN_NAME
)
{
    LIBFUNC_INIT
  if(!AwebPluginBase->sourcedriver)
   {  AwebPluginBase->sourcedriver=Amethod(NULL,AOM_INSTALL,0,(Tag)Dispatchersource);
   }
   if(!AwebPluginBase->copydriver)
   {  AwebPluginBase->copydriver=Amethod(NULL,AOM_INSTALL,0,(Tag)Dispatchercopy);
   }
   pi->sourcedriver=AwebPluginBase->sourcedriver;
   pi->copydriver=AwebPluginBase->copydriver;
   return (ULONG)(AwebPluginBase->sourcedriver && AwebPluginBase->copydriver);

    LIBFUNC_EXIT
}

#define ISSAFE(s,f) (s->structsize>=((long)&s->f-(long)s+sizeof(s->f)))



LIBFUNC_H1
(
__saveds void  , Queryplugin,
struct Pluginquery *,pq,A0,
PLUGIN_TYPE, PLUGIN_NAME
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
