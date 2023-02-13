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

/* awebcfg.h - AWeb configuration tool */

/* This header file contains all includes, defines and prototypes for the
   AWebCfg program files. Many are copied from aweb.h */

#ifndef AWEB_AWEBCFG_H
#define AWEB_AWEBCFG_H

#include "platform_specific.h"
#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dostags.h>
#include <dos/rdargs.h>
#if defined(__amigaos4__)
#include <dos/anchorpath.h>
#endif
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <intuition/intuition.h>
#include <intuition/icclass.h>
#include <graphics/displayinfo.h>
#include <libraries/gadtools.h>
#include <gadgets/colorwheel.h>
#include <gadgets/gradientslider.h>
#include <reaction/reaction.h>

#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>
#include <proto/asl.h>
#include <proto/colorwheel.h>
#include <proto/utility.h>
#include <proto/datatypes.h>

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ezlists.h>

#include "awebprefs.h"
#include "keyfile.h"
#include "libraries/awebclib.h"

#ifndef NOCFGLOCALE
#define CATCOMP_NUMBERS
#include "cfglocale.h"
#endif

#ifndef ALLOCTYPE
#define ALLOCPLUS(t,n,p,f) (t*)Allocmem((n)*sizeof(t)+(p),(f)|MEMF_PUBLIC)
#define ALLOCTYPE(t,n,f)   (t*)Allocmem((n)*sizeof(t),(f)|MEMF_PUBLIC)
#define ALLOCSTRUCT(s,n,f) ALLOCTYPE(struct s,n,f)
#define FREE(p)            Freemem(p)
#endif

#ifndef STREQUAL
#define STRNIEQUAL(a,b,n)  !strnicmp(a,b,n)
#define STRNEQUAL(a,b,n)   !strncmp(a,b,n)
#define STRIEQUAL(a,b)     !stricmp(a,b)
#define STREQUAL(a,b)      !strcmp(a,b)
#endif

#ifndef BOOLVAL
#define BOOLVAL(x)         (BOOL)((x)!=0)
#endif

#ifndef NULLSTRING
#define NULLSTRING         (UBYTE *)""
#endif

extern BOOL has35;

#ifdef NEED35
#define OSNEED(a,b) (b)
#define OSDEP(a,b) (b)
#else
#define OSNEED(a,b) (a)
#define OSDEP(a,b) (has35?(b):(a))
#endif

extern struct MsgPort *ACreatemsgport();
extern VOID ADeletemsgport(struct MsgPort *port);
extern BPTR ASetcurrentdir(BPTR lock);

extern struct LocaleInfo localeinfo;

extern STRPTR GetString(struct LocaleInfo *li, long stringnum);

#define AWEBSTR(n)   GetString(&localeinfo,(n))

extern void *maincatalog;
extern UBYTE *Getmainstr(ULONG msg);

extern UBYTE config[];              /* configuration name */
extern struct Screen *pubscreen;
extern struct DrawInfo *drawinfo;
extern void *visualinfo;
extern struct Image *amigaimg;

extern struct NewMenu menubase[];
enum MENU_IDS
{  MID_OPEN=1,MID_SAVEAS,MID_QUIT,
   MID_DEFAULTS,MID_LASTSAVED,MID_RESTORE,
   MID_BROWSER,MID_PROGRAM,MID_GUI,MID_NETWORK,MID_CLASSACT,MID_SNAPSHOT,
};

extern UWORD cfgcommand;

extern struct Settingsprefs setprefs;

/* awebcfg */

extern STRPTR Dupstr(STRPTR str,long length);
extern void Makechooserlist(struct List *list,UBYTE **labels,BOOL readonly);
extern void Freechooserlist(struct List *list);
extern void Makeclicktablist(struct List *list,UBYTE **labels);
extern void Freeclicktablist(struct List *list);
extern void Makeradiolist(struct List *list,UBYTE **labels);
extern void Freeradiolist(struct List *list);
extern void Freebrowserlist(struct List *list);
VARARGS68K_DECLARE(extern void Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...));
extern struct Node *Getnode(struct List *list,long n);
extern long Getvalue(struct Gadget *gad,ULONG tag);
extern void Getstringvalue(UBYTE **ptr,struct Gadget *gad);
extern long Activelabel(struct List *list,UBYTE *label);
extern void Getnonnullstringvalue(UBYTE **ptr,struct Gadget *gad);
extern BOOL Getselected(struct Gadget *gad);
extern void Adjustintgad(struct Window *window,struct Gadget *gad);
extern long Reqheight(struct Screen *screen);
extern void Popdrawer(void *winobj,struct Window *window,struct Gadget *layout,
   UBYTE *title,struct Gadget *gad);
extern void Popfile(void *winobj,struct Window *window,struct Gadget *layout,
   UBYTE *title,struct Gadget *gad);
extern UBYTE *Filereq(void *winobj,struct Window *window,struct Gadget *layout,
   UBYTE *title,UBYTE *name,BOOL save);
extern UBYTE *Prefsscreenmodename(ULONG modeid,long w,long h,long d);
extern UBYTE *Hotkey(UBYTE *label);
extern BOOL Popcolor(void *winobj,struct Window *pwin,struct Gadget *layout,
   struct Colorprefs *cp,long pen);
extern long Moveselected(struct Window *win,struct Gadget *gad,struct List *list,short n);
extern BOOL Ownscreen(void);
extern void Dimensions(struct Window *window,short *dim);
extern void Insertinstringgadget(struct Window *window, struct Gadget *gad,UBYTE *text);

/* memory */

extern BOOL Initmemory(void);
extern void Freememory(void);

extern void *Allocmem(long size,ULONG flags);
extern void Freemem(void *mem);

/* cfgbr */

extern BOOL Openbrowser(void);
extern ULONG brmask;
extern BOOL Processbrowser(void);
extern void Closebrowser(void);

/* cfgpr */

extern BOOL Openprogram(void);
extern ULONG prmask;
extern BOOL Processprogram(void);
extern void Closeprogram(void);

/* cfgui */

extern BOOL Opengui(void);
extern ULONG uimask;
extern BOOL Processgui(void);
extern void Closegui(void);

/* cfgnw */

extern BOOL Opennetwork(void);
extern ULONG nwmask;
extern BOOL Processnetwork(void);
extern void Closenetwork(void);

#endif
