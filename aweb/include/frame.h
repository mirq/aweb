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

/* frame.h - AWeb frame element object */

#ifndef AWEB_FRAME_H
#define AWEB_FRAME_H

#include "element.h"

/*--- frame tags ---*/

#define AOFRM_Dummy        AOBJ_DUMMYTAG(AOTP_FRAME)

#define AOFRM_Displaycopy  (AOFRM_Dummy+3)
   /* (void *) The COPY object that wants to be displayed. */

#define AOFRM_Fragment     (AOFRM_Dummy+5)
   /* (UBYTE *) Fragment name to locate */

#define AOFRM_Leftpos      (AOFRM_Dummy+6)
#define AOFRM_Toppos       (AOFRM_Dummy+7)
   /* (long) left,top scroll position */

#define AOFRM_Url          (AOFRM_Dummy+8)
   /* (struct Url *) NEW default URL, or GET URL displayed in this frame */

#define AOFRM_Innerwidth   (AOFRM_Dummy+10)
#define AOFRM_Innerheight  (AOFRM_Dummy+11)
   /* (long) Inner frame dimensions */

#define AOFRM_Bgcolor      (AOFRM_Dummy+12)
#define AOFRM_Textcolor    (AOFRM_Dummy+13)
#define AOFRM_Linkcolor    (AOFRM_Dummy+14)
#define AOFRM_Vlinkcolor   (AOFRM_Dummy+15)
#define AOFRM_Alinkcolor   (AOFRM_Dummy+16)
   /* (short) Pen numbers to use, or (-1) to reset */

#define AOFRM_Bgimage      (AOFRM_Dummy+17)
   /* (void *) Background image to use, or NULL to reset */

#define AOFRM_Id           (AOFRM_Dummy+18)
   /* (UBYTE *) This frame's hierarchical frame id */

#define AOFRM_Seqnr        (AOFRM_Dummy+19)
   /* (UBYTE) Sequence number of frame within owner (document) */

#define AOFRM_Topframe     (AOFRM_Dummy+20)
   /* (BOOL) This is the top frame of the window */

#define AOFRM_Reload       (AOFRM_Dummy+21)
   /* (BOOL) Start a reload for this frame's contents */

#define AOFRM_Updatecopy   (AOFRM_Dummy+22)
   /* (BOOL) Relayout and rerender because of changed contents */

#define AOFRM_Title        (AOFRM_Dummy+23)
   /* (UBYTE *) GET Title of frame contents */

#define AOFRM_Makevisible  (AOFRM_Dummy+24)
   /* (struct Arect *) Scroll so that this rectangle is visible.
    * Coordinates are frame content relative. */

#define AOFRM_Border       (AOFRM_Dummy+25)     /* NEW,GET */
   /* (short) Border thickness around frame. */

#define AOFRM_Scrolling    (AOFRM_Dummy+26)     /* NEW */
   /* (BOOL) Allow scrollers. */

#define AOFRM_Resize       (AOFRM_Dummy+27)     /* NEW,GET */
   /* (BOOL) Allow resizing of frame. */

#define AOFRM_Marginwidth  (AOFRM_Dummy+28)     /* NEW */
#define AOFRM_Marginheight (AOFRM_Dummy+29)     /* NEW */
   /* (long) Margin dimensions. */

#define AOFRM_Width        (AOFRM_Dummy+30)     /* NEW */
#define AOFRM_Pixelwidth   (AOFRM_Dummy+31)     /* NEW */
#define AOFRM_Height       (AOFRM_Dummy+32)     /* NEW */
#define AOFRM_Pixelheight  (AOFRM_Dummy+33)     /* NEW */
   /* (long) Suggested dimensions for embedded frames. */

#define AOFRM_Name         (AOFRM_Dummy+34)     /* NEW,GET */
   /* (UBYTE *) Target name of frame. */

#define AOFRM_Defaultborder (AOFRM_Dummy+35)    /* NEW,SET */
   /* (BOOL) Set or unset border only if no AOFRM_Border was set before */

#define AOFRM_Resizeleft   (AOFRM_Dummy+36)     /* GET */
#define AOFRM_Resizeright  (AOFRM_Dummy+37)     /* GET */
#define AOFRM_Resizetop    (AOFRM_Dummy+38)     /* GET */
#define AOFRM_Resizebottom (AOFRM_Dummy+39)     /* GET */
   /* (BOOL) If frame or frameset may be resized on this edge */

#define AOFRM_Search       (AOFRM_Dummy+40)     /* SET */
   /* (BOOL) Open or close a search requester. */

#define AOFRM_Contentheight (AOFRM_Dummy+41)    /* GET */
   /* (long) Height of frame contents */

#define AOFRM_Dumbframe    (AOFRM_Dummy+41)     /* NEW */
   /* (BOOL) Frame shouldn't try to adjust scroll positions */

#define AOFRM_Layoutheight (AOFRM_Dummy+42)     /* NEW */
   /* (long) Height to layout against for dumb frames. */

#define AOFRM_Inline       (AOFRM_Dummy+43)     /* NEW */
   /* (BOOL) Inline document object. If FALSE, it's the top frame or part of
    * frameset. */

#define AOFRM_Commands     (AOFRM_Dummy+44)     /* SET */
   /* (BOOL) Shell/ARexx commands allowed. Forwarded to WINHIS objects. */

#define AOFRM_Noreferer    (AOFRM_Dummy+45)     /* SET */
   /* (BOOL) When set together with AOBJ_Winhis, no referer is used. */

#define AOFRM_Info         (AOFRM_Dummy+46)     /* SET */
   /* (BOOL) Open an info window for this frame's copy */

#define AOFRM_Cancelcopy   (AOFRM_Dummy+47)     /* SET */
   /* (void *) This COPY object doesn't want to be displayed after all */

#define AOFRM_Reloadverify (AOFRM_Dummy+48)     /* SET */
   /* (BOOL) When set together with AOBJ_Winhis, do a forced verify. */

#define AOFRM_Setscroller  (AOFRM_Dummy+49)     /* SET */
   /* (BOOL) If toppos or leftpos changed, set scroller too. */

#define AOFRM_Jprotect     (AOFRM_Dummy+50)     /* SET */
   /* (ULONG) JS protection key to use for this frame */

#define AOFRM_Jdocument    (AOFRM_Dummy+51)     /* SET */
   /* (struct Jobject *) JS document object to add to global scope for this frame. */

#define AOFRM_Jgenerated   (AOFRM_Dummy+52)     /* SET */
   /* (struct Buffer *) JS generated text to be loaded in this frame. */

#define AOFRM_Jsopen       (AOFRM_Dummy+53)     /* SET */
   /* (BOOL) If JS generated document is still open */

#define AOFRM_Onfocus      (AOFRM_Dummy+54)     /* SET */
#define AOFRM_Onblur       (AOFRM_Dummy+55)     /* SET */
   /* (UBYTE *) JS scripts to run when focus/blur */

#define AOFRM_Focus        (AOFRM_Dummy+56)     /* SET */
   /* (BOOL) Frame got or lost focus */

#define AOFRM_Reposfragment (AOFRM_Dummy+57)    /* SET */
   /* (BOOL) Child has renewed contents. Reposition again at fragment at
    * coming updatecopies (unless history) */

#define AOFRM_Prepreset    (AOFRM_Dummy+58)     /* NOTIFY/SET */
   /* (BOOL) Prepare for resetframe. Next layout must do relayout even
    * if dimensions are not changed. */

#define AOFRM_Resetframe   (AOFRM_Dummy+59)     /* SET */
   /* (BOOL) Force a re-layout of frame contents. */

#define AOFRM_Contentwidth (AOFRM_Dummy+60)     /* GET */
   /* (long) Width of frame contents */

#define AOFRM_Bgalign      (AOFRM_Dummy+61)     /* SET */
   /* (struct Aobject *) Object to align background too */

#define AOFRM_Copy         (AOFRM_Dummy+62)     /* GET */
   /* (struct Aobject *) Frames Copy */

#define AOFRM_    (AOFRM_Dummy+)
#define AOFRM_    (AOFRM_Dummy+)


/*--- frame functions ---*/
#include "frprivate.h"

void *Targetframe(struct Frame *frame,UBYTE *name);
   /* Find the frame for this target name. */

#ifdef __MORPHOS__
# include <cybergraphx/cybergraphics.h>
# include <proto/cybergraphics.h>
# ifndef BLTBMA_USESOURCEALPHA
# define BLTBMA_USESOURCEALPHA   0x88802001
# endif
# ifndef BltBitMapRastPortAlpha
# define BltBitMapRastPortAlpha(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8) \
       LP9(240, ULONG , BltBitMapRastPortAlpha, \
               struct BitMap *, __p0, a0, \
               WORD , __p1, d0, \
               WORD , __p2, d1, \
               struct RastPort *, __p3, a1, \
               WORD , __p4, d2, \
               WORD , __p5, d3, \
               WORD , __p6, d4, \
               WORD , __p7, d5, \
               struct TagItem *, __p8, a2, \
               , CYBERGRAPHICS_BASE_NAME, 0, 0, 0, 0, 0, 0)
# endif
#endif

#endif
