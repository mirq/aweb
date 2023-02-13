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

/* copyprivate.h - AWeb copy private definitions */

#include "copy.h"
#include "copydriver.h"

struct Copy
{  struct Field fld;
   void *frame;
   void *url;                 /* Original url this copy belongs to. Different from
                               * (source)'s url if url was moved. */
   void *source;              /* Source interpreter this copy belongs to. */
   void *referer;             /* Referer url */
   struct Copydriver *driver; /* Copy driver */
   void *parent;              /* Parent to notify if not embedded */
   void *whis;                /* Current winhis */
   struct Colorinfo *color;   /* ALT text colour */
   ULONG flags;
   void *usemap;              /* MAP object to use */
   short border;              /* Link border thickness */
   short hspace,vspace;       /* Extra gutter */
   short mwidth,mheight;      /* Margins to forward to driver */
   long width,height;         /* Suggested dimensions in pixels (even if given in %) */
   long pwidth,pheight;       /* Suggested dimensions as percentage or -1. */
   UBYTE *lasturl;            /* Last constructed url for map links */
   struct Buffer *text;       /* Text buffer to use for in-place rerendering */
   long soundloop;            /* Sound loop */
   long histart,hilength;     /* Highlight section. */
   UBYTE *clientpull;         /* Clientpull string, pass to frame when EOF received */
   void *info;                /* INFO object for this copy */
   LIST(Objectparam) params;  /* Object parameters */
   void *maparea;             /* Current AREA object */
   void *popup;               /* Popup window open */
   UBYTE *name;               /* Image name */
   struct Jobject *jobject;   /* Image JS object */
   UBYTE *onload;
   UBYTE *onerror;
   UBYTE *onabort;
   UBYTE *onclick;
   UBYTE *onmouseover;
   UBYTE *onmouseout;
   void *jsframe;             /* Related frame object for JS Image() ctr object */
   void *jform;               /* FORM object to add named property to */
};

#define CPYF_EMBEDDED      0x00000001  /* Embedded object */
#define CPYF_ISMAP         0x00000002  /* Server side map */
#define CPYF_MAPHIT        0x00000004  /* Hittest returned constructed map url */
#define CPYF_NEWSOURCE     0x00000008  /* Source has changed */
#define CPYF_NEWDRIVER     0x00000010  /* Driver has changed */
#define CPYF_BACKGROUND    0x00000020  /* Background object */
#define CPYF_ERROR         0x00000040  /* Source is in error */
#define CPYF_NOBACKGROUND  0x00000080  /* Don't use backgrounds, fwd to driver */
#define CPYF_BGSOUND       0x00000100  /* Background sound object */
#define CPYF_DISPLAYED     0x00000200  /* Driver is allowed to render itself */
#define CPYF_INITIAL       0x00000400  /* No initial load done yet */
#define CPYF_EOF           0x00000800  /* EOF received, forward clientpull */
#define CPYF_RELOADVERIFY  0x00001000  /* Subject to a reload, must forced verify */
#define CPYF_MAPDOCUMENT   0x00002000  /* HTML document for <MAP> definitions only */
#define CPYF_OBJECTDEF     0x00004000  /* OBJECT definition not complete yet */
#define CPYF_TRUEIMAGE     0x00008000  /* True image (JS Image object needed) */
#define CPYF_JSONLOAD      0x00010000  /* JS onLoad event for static images allowed */
#define CPYF_JSONLOADQ     0x00020000  /* A JS onLoad notification is queued */
#define CPYF_JSIMAGE       0x00040000  /* This is a JS Image() constructor object */
#define CPYF_REFERER       0x00080000  /* Referer was set (maybe NULL) */
#define CPYF_CHANGEDCOPY   0x00100000  /* Driver has changed, rerender */
#define CPYF_BORDERSET     0x00200000  /* Border was explicitly set */
#define CPYF_POPUPHIT      0x00400000  /* Last hittest was for popup */

#define PROXY(c) (Agetattr((c)->fld.win,AOWIN_Noproxy)?AUMLF_NOPROXY:0)


/* copyjs.c */
extern long Jsetupcopy(struct Copy *cop,struct Amjsetup *amj);
extern long Jonmousecopy(struct Copy *cop,struct Amjonmouse *amj);
extern void Freejcopy(struct Copy *cop);
