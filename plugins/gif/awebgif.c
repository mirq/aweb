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

/* awebgif.c - AWeb gif plugin main */

#include "pluginlib.h"
#include "awebgif.h"
#include <string.h>
#include "libraries/awebsupport.h"
#include "libraries/awebmodule.h"
#include "libraries/awebclib.h"

#undef NO_INLINE_STDARG
#include <proto/awebsupport.h>
#define NO_INLINE_STDARG
#include <proto/exec.h>
#include <proto/utility.h>

/* olsen: use library bases rather than (void *). */
struct GfxBase * GfxBase;
struct IntuitionBase * IntuitionBase;
struct Library * CyberGfxBase = NULL;
struct Library * AwebSupportBase;
struct DosLibrary * DOSBase;

#ifdef __MORPHOS__
struct Library *UtilityBase;
#else
struct UtilityBase *UtilityBase;
#endif

#if defined(__amigaos4__)

struct GraphicsIFace *IGraphics;
struct IntuitionIFace *IIntuition;
struct UtilityIFace *IUtility;
struct CyberGfxIFace *ICyberGfx = NULL;
struct AwebSupportIFace *IAwebSupport;
struct DOSIface *IDOS;

#endif



BOOL animate;

ULONG Initpluginlib(struct AwebGifBase *base)
{
   if(!(AwebModuleInit())) return FALSE;

   if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39))) return FALSE;
   if(!(IntuitionBase = (struct InituitionBase *)OpenLibrary("intuition.library",39))) return FALSE;
   if(!(UtilityBase = (struct UtilityBase *)OpenLibrary("utility.library",39))) return FALSE;
   if(!(AwebSupportBase = OpenLibrary("awebsupport.library",0))) return FALSE;
   /* olsen: I need StrToLong() in the library. */
   if(!(DOSBase = (struct DosLibarary *)OpenLibrary("dos.library",37))) return FALSE;



   /*Cybergraphics is not critical*/
   CyberGfxBase = OpenLibrary("cybergraphics.library",0);

#if defined(__amigaos4__)

    if(!(IGraphics = (struct GraphicsIFace *)GetInterface(GfxBase,"main",1,0))) return FALSE;
    if(!(IUtility = (struct UtilityIFace *)GetInterface(UtilityBase,"main",1,0))) return FALSE;
    if(!(IAwebSupport = (struct AwebpluginIFace *)GetInterface(AwebSupportBase,"main",1,0))) return FALSE;
    if(!(IIntuition = (struct IntuitionIFace *)GetInterface(IntuitionBase,"main",1,0))) return FALSE;
    if(!(IDOS = (struct DOSIFace *)GetInterface(DOSBase,"main",1,0))) return FALSE;
    if(CyberGfxBase)
    {
        ICyberGfx = GetInterface(CyberGfxBase,"main",1,0);
    }

#endif

   animate=TRUE;

   return TRUE;
}

void Expungepluginlib(struct AwebGifBase *base)
{
   if(base->sourcedriver) Amethod(NULL,AOM_INSTALL,base->sourcedriver,NULL);
   if(base->copydriver) Amethod(NULL,AOM_INSTALL,base->copydriver,NULL);

#if defined(__amigaos4__)
    if(ICyberGfx)DropInterface((struct Interface *)ICyberGfx);
    if(IAwebSupport)DropInterface((struct Interface *)IAwebSupport);
    if(IGraphics)DropInterface((struct Interface *)IGraphics);
    if(IIntuition)DropInterface((struct Interface *)IIntuition);
    if(IUtility)DropInterface((struct Interface *)IUtility);
    if(IDOS)DropInterface((struct Interface *)IDOS);
#endif

   if(CyberGfxBase)    CloseLibrary(CyberGfxBase);
   if(AwebSupportBase)  CloseLibrary(AwebSupportBase);
   if(UtilityBase)     CloseLibrary((struct Library *)UtilityBase);
   if(IntuitionBase)   CloseLibrary((struct Library *)IntuitionBase);
   if(GfxBase)         CloseLibrary((struct Library *)GfxBase);
   if(DOSBase != NULL) CloseLibrary((struct Library *)DOSBase);

    AwebModuleExit();
}

LIBFUNC_H1
(
ULONG  , Initplugin,
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

#define ISSAFE(s,f) (s->structsize>=((long)&s->f-(long)s+sizeof(s->f)))

LIBFUNC_H1
(
void  , Queryplugin,
struct Pluginquery *,pq,A0,
PLUGIN_TYPE, PLUGIN_NAME
)
{
    LIBFUNC_INIT

   if(ISSAFE(pq,command)) pq->command=TRUE;

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
void  , Commandplugin,
struct Plugincommand *,pc,A0,
PLUGIN_TYPE, PLUGIN_NAME

)
{
    LIBFUNC_INIT
  if(pc->structsize<sizeof(struct Plugincommand)) return;
   /* olsen: use utility.library instead of the compiler runtime library. */
   if(!Stricmp(pc->command,"STARTANIM"))
   {  animate=TRUE;
   }
   else if(!Stricmp(pc->command,"STOPANIM"))
   {  animate=FALSE;
   }
   else if(!Stricmp(pc->command,"ANIM"))
   {  pc->result=Dupstr(animate?"1":"0",-1);
   }
   else
   {  pc->rc=10;
   }

    LIBFUNC_EXIT
}
