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

/* whisprivate.h - Private winhis & whiswindow & whistask data */

struct Winhis
{  struct Aobject object;
   LIST(Framehis) frames;
   ULONG key;                 /* Window key */
   long windownr;             /* Window number as know by the user */
   UWORD wflags;
   ULONG whisnr;              /* Sequence number to know order */
   UBYTE *title;              /* Title to show in history window */
   UBYTE *frameid;            /* Id of leading frame (the one that changed). Same string
                               * as used in Framehis, may be NULL for top frame. */
   ULONG loadnr;              /* Object load sequence number */
   void *titleurl;            /* Url to show in history window, use as default title,
                                 show in URL gadget. */
};

#define WINHF_SKIP      0x0001   /* Not part of mainline */
#define WINHF_HISTORY   0x0002   /* Historic winhis, use positions not fragment */
#define WINHF_COMMANDS  0x0004   /* Shell,ARexx commands allowed. */
#define WINHF_NEW       0x0008   /* Not _yet_ part of mainline (WINHF_SKIP is set too) */

struct Framehis               /* History for one frame */
{  NODE(Framehis);
   UBYTE *id;
   void *url;                 /* Url displayed */
   UBYTE *fragment;           /* Fragment displayed or NULL */
   long left,top;             /* Current scroll position */
};


typedef LIST(Winhis) WinhisList_t;
extern WinhisList_t winhis;
extern struct SignalSemaphore whissema;

#define URLBUFSIZE   128

struct Whiswindow
{  struct Aobject object;
   void *task;
   struct SignalSemaphore *whissema;
   LIST(Winhis) *winhislist;
   struct Library *libbase;
   long windownr;
   void *winobj;
   UBYTE *screenname;
   struct Screen *screen;        /* locked while window open */
   struct Window *window;
   ULONG winsigmask;
   UWORD status;
   void *listgad,*filtergad,*wingad,*ordergad,*titlegad,*urlgad;
   struct List gadlist,orderlist;
   UWORD order;
   short x,y,w,h;
   UWORD lastclick;
   BOOL autoclose;
   BOOL filter;
   struct Winhis *current;       /* current for this window */
   long currentnode;             /* node number of current winhis */
   UBYTE urlbuf[URLBUFSIZE];
   UWORD flags;
};

#define WHWF_BREAKING      0x0001   /* Breaking the subtask */
#define WHWF_WASOPEN       0x0002   /* Window was open when screen became invalid */

#define AOWHW_Dummy        AOBJ_DUMMYTAG(AOTP_WHISWINDOW)

#define AOWHW_Close        (AOWHW_Dummy+1)
   /* (BOOL) Window was closed. */

#define AOWHW_Dimx         (AOWHW_Dummy+2)
#define AOWHW_Dimy         (AOWHW_Dummy+3)
#define AOWHW_Dimw         (AOWHW_Dummy+4)
#define AOWHW_Dimh         (AOWHW_Dummy+5)
   /* (long) Changed window dimensions */

#define AOWHW_Display      (AOWHW_Dummy+6)
   /* (struct Winhis *) Winhis to redisplay */

#define AOWHW_Windownr     (AOWHW_Dummy+7)
   /* (long) Window number to use, or to get current winhis from */

#define AOWHW_Getcurrent   (AOWHW_Dummy+8)
   /* (BOOL) Fill in current winhis for windownr (in Whiswindow->current) */

#define AOWHW_Current      (AOWHW_Dummy+9)
   /* (struct Winhis *) Current winhis */

#define AOWHW_Tofront      (AOWHW_Dummy+10)
   /* (long) Push window to front, use this windownr as current */

#define AOWHW_Changed      (AOWHW_Dummy+11)
   /* (long) Windownr for which winhis has changed */
