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

/* awebjfif.c - AWeb jfif plugin main */

#include "pluginlib.h"
#include "awebjfif.h"
#include "libraries/awebsupport.h"
#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"

#undef NO_INLINE_STDARG
#include <proto/awebsupport.h>
#define NO_INLINE_STDARG
#include <proto/exec.h>

struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;
struct UtilityBase *UtilityBase;
struct Library *CyberGfxBase;
struct Library *AwebSupportBase;

#if defined(__MORPHOS__)
struct Library *JFIFBase;
#endif

#if defined(__amigaos4__)

struct GraphicsIFace *IGraphics;
struct UtilityIFace *IUtility;
struct CyberGfxIFace *ICyberGfx = NULL;
struct AwebSupportIFace *IAwebSupport;
struct DOSIface *IDOS;

#endif



ULONG Initpluginlib(struct AwebJfifBase *base)
{
   if(!(AwebModuleInit())) return FALSE;

   if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39))) return FALSE;
   if(!(UtilityBase = (struct UtilityBase *)OpenLibrary("utility.library",39))) return FALSE;
   if(!(AwebSupportBase = OpenLibrary("awebsupport.library",0))) return FALSE;
   if(!(DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",37))) return FALSE;

   /*Cybergraphics is not critical*/
   CyberGfxBase = OpenLibrary("cybergraphics.library",0);

#if defined(__MORPHOS__)
   if (!(JFIFBase = OpenLibrary("jfif.library", 51))) return FALSE;
#endif

#if defined(__amigaos4__)

    if(!(IGraphics = (struct GraphicsIFace *)GetInterface(GfxBase,"main",1,0))) return FALSE;
    if(!(IUtility = (struct UtilityIFace *)GetInterface(UtilityBase,"main",1,0))) return FALSE;
    if(!(IAwebSupport = (struct AwebpluginIFace *)GetInterface(AwebSupportBase,"main",1,0))) return FALSE;
    if(!(IDOS = (struct DOSIFace *)GetInterface(DOSBase,"main",1,0))) return FALSE;
    if(CyberGfxBase)
    {
        ICyberGfx = GetInterface(CyberGfxBase,"main",1,0);
    }

#endif

   return TRUE;
}

void Expungepluginlib(struct AwebJfifBase *base)
{
   if(base->sourcedriver) Amethod(NULL,AOM_INSTALL,(struct Aobject *)base->sourcedriver,NULL);
   if(base->copydriver) Amethod(NULL,AOM_INSTALL,(struct Aobject *)base->copydriver,NULL);


#if defined(__amigaos4__)
    if(ICyberGfx)DropInterface((struct Interface *)ICyberGfx);
    if(IAwebSupport)DropInterface((struct Interface *)IAwebSupport);
    if(IGraphics)DropInterface((struct Interface *)IGraphics);
    if(IUtility)DropInterface((struct Interface *)IUtility);
    if(IDOS)DropInterface((struct Interface *)IDOS);
#endif

#if defined(__MORPHOS__)
   CloseLibrary(JFIFBase);
#endif

   if(CyberGfxBase)    CloseLibrary(CyberGfxBase);
   if(AwebSupportBase)  CloseLibrary(AwebSupportBase);
   if(UtilityBase)     CloseLibrary((struct Library *)UtilityBase);
   if(GfxBase)       CloseLibrary((struct Library *)GfxBase);
   if(DOSBase != NULL) CloseLibrary((struct Library *)DOSBase);


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
   {  AwebPluginBase->sourcedriver=Amethod(NULL,AOM_INSTALL,0,Dispatchersource);
   }
   if(!AwebPluginBase->copydriver)
   {  AwebPluginBase->copydriver=Amethod(NULL,AOM_INSTALL,0,Dispatchercopy);
   }
   pi->sourcedriver=AwebPluginBase->sourcedriver;
   pi->copydriver=AwebPluginBase->copydriver;
   return (ULONG)(AwebPluginBase->sourcedriver && AwebPluginBase->copydriver);

    LIBFUNC_EXIT
}

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

#ifdef __MORPHOS__

void *malloc(LONG len)
{
   return AllocVec(len, MEMF_PUBLIC);
}

void free(void *p)
{
   FreeVec(p);
}

#endif /* __MORPHOS__ */
