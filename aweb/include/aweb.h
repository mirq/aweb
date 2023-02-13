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

/* aweb.h */

#ifndef AWEB_H
#define AWEB_H

#include <exec/types.h>
#include <exec/ports.h>
#include <graphics/rastport.h>

#ifndef AWEBDEF_H
#include "awebdef.h"
#endif

#include "libraries/awebclib.h"

#define FSF_STRIKE      0x8000   /* addition to OS styles */

struct Colorinfo           /* a color used in the document */
{  NODE(Colorinfo);        /* list node */
   ULONG rgb;              /* palette value */
   short pen;              /* pen assigned */
   UWORD flags;           /* see below */
};

#define COLIF_GLOBAL    0x0001   /* global document color, like text or link */

enum MIME_DRIVERS
{  MIMEDRV_NONE=0,         /* No known driver */
   MIMEDRV_DOCUMENT,       /* Internal text driver */
   MIMEDRV_IMAGE,          /* Internal image driver */
   MIMEDRV_SOUND,          /* Internal sound driver */
   MIMEDRV_EXTPROG,        /* External program driver */
   MIMEDRV_EXTPROGPIPE,    /* External program driver with PIPE: */
   MIMEDRV_EXTPROGNOFTCH,  /* External program driver cancel fetch */
   MIMEDRV_PLUGIN,         /* External plugin driver */
   MIMEDRV_SAVELOCAL,      /* Save to local disk */
   MIMEDRV_CANCEL,         /* Cancel and discard */
};

struct Coords              /* accumulated coordinates for displaying */
{  long dx,dy;             /* add to element coords to get rastport coords */
   long minx,miny,maxx,maxy;  /* resulting max visible limits as window coords */
   struct Aobject *win;    /* the top level window object */
   struct RastPort *rp;    /* the window's rastport */
   struct DrawInfo *dri;   /* the current drawinfo */
   short bgcolor,textcolor,linkcolor,vlinkcolor,alinkcolor;
                           /* default pen numbers for current frame */
   void *bgimage;          /* background image for current frame */
   struct Aobject *bgalign; /* object to align background image to */ /*added for 3.5 */
   short nestcount;
};

struct Layermessage
{  struct Layer *layer;
   struct Rectangle rect;
   long xoffset;
   long yoffset;
};

struct Abackfilldata       /* Hook data for Awebbackfillhook() */
{  long fgpen;
   long bgpen;
   UWORD flags;           /* What to do, see below */
};
#define ABKFIL_BACKFILL    0x0001   /* Fill area with bgpen */
#define ABKFIL_BORDER      0x0002   /* Draw border in fgpen */

extern struct LocaleInfo localeinfo;
#define AWEBSTR(n)      GetString(&localeinfo,(n))
extern UBYTE *Getmainstr(ULONG n);

/* nr of pixels to indent for each list level */
#define LEVELINDENT  40

/* nr of pixels to indent left and right for each blockquote */
#define BQINDENT     40

/* request return function type */
typedef void requestfunc(short code,void *data);

/* Configurable button types */
enum BUTTON_TYPES
{  BUTF_BACK,BUTF_FORWARD,BUTF_HOME,BUTF_ADDHOTLIST,BUTF_HOTLIST,
   BUTF_CANCEL,BUTF_NETSTATUS,BUTF_SEARCH,BUTF_RELOAD,BUTF_LOADIMAGES,
   BUTF_UNSECURE,BUTF_SECURE,

   BUTF_NSCANCEL,BUTF_NSCANCELALL,
   NR_BUTTONS,
};

/* Addtagstr() function constants */
#define ATSF_NONE    0
#define ATSF_STRING  1
#define ATSF_NUMBER  2

/* suggested pool sizes for private pools */
#define PUDDLESIZE         16*1024
#define TRESHSIZE          4*1024

#define PALLOCTYPE(t,n,f,p)   (t*)Pallocmem((n)*sizeof(t),(f)|MEMF_PUBLIC,p)
#define PALLOCSTRUCT(s,n,f,p) PALLOCTYPE(struct s,n,f,p)

extern STRPTR aboutversion;   /* full version string contents */
extern STRPTR awebversion;    /* version.revision string */

/* Generic JS event handler invocations */
extern UBYTE *awebonclick,*awebonchange,*awebonblur,*awebonfocus,*awebonmouseover,
   *awebonmouseout,*awebonreset,*awebonsubmit,*awebonselect,*awebonload,
   *awebonunload,*awebonerror,*awebonabort;

extern struct AwebPrefs prefs;
extern struct Himage *def_image,*def_imagemap,*def_errimage;
extern struct SignalSemaphore prefssema;
extern struct Hook requestbackfillhook;
extern UBYTE programname[];
extern struct Locale *locale;

struct AWebJSBase;
extern struct AWebJSBase *AWebJSBase;

/* boopsi status gadget tags: */
#define STATGA_Dummy          (TAG_USER+0x0AEB0000)
#define STATGA_HPText         (STATGA_Dummy+0x0001)   /* high-priority text */
#define STATGA_HPLen1         (STATGA_Dummy+0x0002)   /* scheme, host, path length */
#define STATGA_HPLen2         (STATGA_Dummy+0x0003)   /* file name length */
#define STATGA_SpecialPens    (STATGA_Dummy+0x0004)   /* ClassAct special pens */

#define LEDGGA_Dummy          (TAG_USER+0x0AEC0000)
#define LEDGGA_Active         (LEDGGA_Dummy+0x0001)
#define LEDGGA_AnimBitMap     (LEDGGA_Dummy+0x0002)
#define LEDGGA_AnimX          (LEDGGA_Dummy+0x0003)
#define LEDGGA_AnimY          (LEDGGA_Dummy+0x0004)
#define LEDGGA_AnimWidth      (LEDGGA_Dummy+0x0005)
#define LEDGGA_AnimHeight     (LEDGGA_Dummy+0x0006)
#define LEDGGA_AnimDeltaX     (LEDGGA_Dummy+0x0007)
#define LEDGGA_AnimDeltaY     (LEDGGA_Dummy+0x0008)
#define LEDGGA_AnimFrames     (LEDGGA_Dummy+0x0009)
#define LEDGGA_RestX          (LEDGGA_Dummy+0x000A)
#define LEDGGA_RestY          (LEDGGA_Dummy+0x000B)
#define LEDGGA_SpecialPens    (LEDGGA_Dummy+0x000C)   /* ClassAct special pens */


#ifndef AWEBPROTOS_H
#include "awebprotos.h"
#endif

extern BOOL httpdebug;
extern BOOL specdebug;
extern BOOL haiku;
extern BOOL noiconify;

#include "haiku.h"

#ifdef DEVELOPER
#define PROFILE
#endif
#include "profile.h"

/**********************************/

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

/**********************************/

#if defined(__MORPHOS__)
/* CGX does not need WaitBlit() and in MorphOS it is usually implemented as a noop.
 * However, in PowerUp systems WaitBlit() is implemented and checks Amiga blitter
 * state to support system friendly blitter use. Because it is not needed for CGX
 * we define it out to let AWeb run at full speed.
 */
#include <proto/graphics.h>
#undef WaitBlit
#define WaitBlit()
#endif

#endif
