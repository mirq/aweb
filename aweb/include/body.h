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

/* body.h - AWeb html document body object */

#ifndef AWEB_BODY_H
#define AWEB_BODY_H

#include "object.h"

/*--- body tags ---*/

#define AOBDY_Dummy        AOBJ_DUMMYTAG(AOTP_BODY)

#define AOBDY_Sethardstyle (AOBDY_Dummy+1)
#define AOBDY_Unsethardstyle (AOBDY_Dummy+2)
   /* (UWORD) Set or unset hard style flags */

#define AOBDY_Align        (AOBDY_Dummy+3)
#define AOBDY_Divalign     (AOBDY_Dummy+4)
   /* (short) Set paragraph or division align. (-1) unsets. */

#define AOBDY_Style        (AOBDY_Dummy+5)   /* SET,GET */
   /* (short) Stack logical style. */

#define AOBDY_Fixedfont    (AOBDY_Dummy+6)
   /* (BOOL) Fixed font, default normal font */

#define AOBDY_Fontsize     (AOBDY_Dummy+7)
   /* (short) Stack absolute font size. */

#define AOBDY_Fontsizerel  (AOBDY_Dummy+8)
   /* (short) Stack relative font size. */

#define AOBDY_Fontcolor    (AOBDY_Dummy+9)
   /* (struct Colorinfo *) Stack font color. */

#define AOBDY_Fontend      (AOBDY_Dummy+10)
   /* (BOOL) Pops font stack (style, size, color). */

#define AOBDY_Basefont     (AOBDY_Dummy+11)
   /* (short) Sets absolute base font size. */

#define AOBDY_Subscript    (AOBDY_Dummy+12)
#define AOBDY_Superscript  (AOBDY_Dummy+13)
   /* (BOOL) Sets sub/superscript mode */

#define AOBDY_Link         (AOBDY_Dummy+14)
   /* (void *) Current hyperlink or NULL */

#define AOBDY_List         (AOBDY_Dummy+15)
   /* (struct Listinfo *) Information about the current list */

#define AOBDY_Dterm        (AOBDY_Dummy+16)
   /* (BOOL) TRUE if a <DT> tag is active in the current <DL> list */

#define AOBDY_Blockquote   (AOBDY_Dummy+17)
   /* (BOOL) Increment or decrement blockquote indent level */

#define AOBDY_Leftmargin   (AOBDY_Dummy+18)
#define AOBDY_Topmargin    (AOBDY_Dummy+19)
   /* (short) Horizontal and vertical body margins */

#define AOBDY_Bgcolor      (AOBDY_Dummy+20)
   /* (short) Pen numbers for background, or (-1) to reset */

#define AOBDY_Bgimage      (AOBDY_Dummy+21)
   /* (void *) Background image to use, or NULL to reset */

#define AOBDY_Nobr         (AOBDY_Dummy+22)
   /* (BOOL) Turns no-break on or off */

#define AOBDY_End          (AOBDY_Dummy+23)  /* SET */
   /* (BOOL) Body definition is complete */

#define AOBDY_Basefontrel  (AOBDY_Dummy+24)  /* SET */
   /* (short) Sets relative basefont size. */

#define AOBDY_Forcebgcolor (AOBDY_Dummy+25)  /* SET */
   /* (short) Use this pen number for background even if bgcolors are off. */

#define AOBDY_Basecolor    (AOBDY_Dummy+26)  /* SET */
   /* (struct Colorinfo *) Sets basefont color. */

#define AOBDY_Fontface     (AOBDY_Dummy+27)  /* SET */
   /* (UBYTE *) Comma-separated list of preferred font face names */

#define AOBDY_Baseface     (AOBDY_Dummy+28)  /* SET */
   /* (UBYTE *) Comma-separated list of preferred basefont face names */

#define AOBDY_Bgalign      (AOBDY_Dummy+29)
   /* (struct Aobject *) Object to align background to */

#define AOBDY_Tcell        (AOBDY_Dummy+30)
   /* (void *) tabel cell that owns body */

#define AOBDY_Bgupdate     (AOBDY_Dummy+31)   /* GET */
   /* ULONG last bgupdate key value */

#define AOBDY_    (AOBDY_Dummy+)
#define AOBDY_    (AOBDY_Dummy+)

/*--- body support structures ---*/

/* Listinfo contains details about the current list.
 * Set AOBDY_List to add a new list level. Set to NULL to
 * remove the last level. Get to obtain info about the list.
 * When set, all information is copied. When get, the real
 * thing is returned and bulletnr may be modified. */
struct Listinfo
{  NODE(Listinfo);
   UWORD type;            /* Type of list */
   UWORD bullettype;      /* Type of bullet */
   UBYTE *bulletsrc;       /* Url of image, dingbat bullet */
   long bulletnr;          /* Last used OL bullet number */
   short level;            /* Current nesting level */
   short indent;           /* Current indent level (same as level except when <DT>) */
};

/* List types */
#define BDLT_UL         1
#define BDLT_OL         2
#define BDLT_DL         3

/* Bullet types */
#define BDBT_DEFAULT    0  /* When SET, will be changed to real type */
#define BDBT_DISC       1  /* Unordered lists. The same values as bullet's types. */
#define BDBT_CIRCLE     2
#define BDBT_SQUARE     3
#define BDBT_DIAMOND    4
#define BDBT_SOLIDDIA   5
#define BDBT_RECTANGLE  6
#define BDBT_IMAGE      7
#define BDBT_PLAIN      8
#define BDBT_NUMBER     9  /* Ordered lists */
#define BDBT_ALPHA      10
#define BDBT_ALPHALOW   11
#define BDBT_ROMAN      12
#define BDBT_ROMANLOW   13

struct Body
{  struct Aobject object;
   long aox,aoy,aow,aoh;      /* Body dimensions */
   void *frame;               /* FRAME object */
   void *cframe;              /* Owner BODY or FRAME object */
   void *pool;                /* memory pool */
   void *parent;              /* Parent document */
   void *tcell;               /* The table cell this body belongs to or NULL*/
   LIST(Element) contents;    /* this body's children */
   short hmargin,vmargin;     /* body's outer margins */
   UWORD flags;
   void *win;                 /* pass to childs */
   LIST(Line) lines;          /* quick vertical index */
   struct Element *chchild;   /* first changed child */
   long rendery;              /* Y position of first line to render CHANGED */
   LIST(Margin) leftmargins;  /* Left side floating margins */
   LIST(Margin) rightmargins; /* Right side floating margins */
   short bgcolor;             /* Pen number for background or -1 */
   void *bgimage;             /* Background image or NULL */
   struct Aobject *bgalign;   /* Object to align background image to */
   ULONG bgupdate;            /* Value of bgupdate lst time this body was renderd */
   LIST(Openfont) openfonts;  /* All fonts used (and therefore opened) by us */
   struct Bodybuild *bld;     /* Variables only needed during growth */
};

/* global references */
extern ULONG bgupdate;

#endif
