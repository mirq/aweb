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

/* element.h - AWeb general HTML element object superclass */

#ifndef AWEB_ELEMENT_H
#define AWEB_ELEMENT_H

#include <graphics/text.h>

#include "object.h"

/*--- element tags ---*/

#define AOELT_Dummy        AOBJ_DUMMYTAG(AOTP_ELEMENT)

#define AOELT_Textpos      (AOELT_Dummy+1)
   /* (long) Position in a text buffer */

#define AOELT_Textlength   (AOELT_Dummy+2)
   /* (long) Length of text */

#define AOELT_Preformat    (AOELT_Dummy+3)
   /* (BOOL) Text is preformatted */

#define AOELT_Halign       (AOELT_Dummy+4)
   /* (short) Horizontal alignment, see below */

#define AOELT_Valign       (AOELT_Dummy+5)
   /* (short) Vertical alignment, see below */

#define AOELT_Font         (AOELT_Dummy+6)
   /* (struct TextFont *) font for text */

#define AOELT_Style        (AOELT_Dummy+7)
   /* (UWORD) Text style */

#define AOELT_Defhalign    (AOELT_Dummy+8)
   /* (short) Default horizontal alignment. Ignored if Halign previously set */

#define AOELT_Color        (AOELT_Dummy+10)
   /* (struct Colorinfo *) Current text colorinfo */

#define AOELT_Supfont      (AOELT_Dummy+11)
   /* (struct TextFont *) Reference font for superscript text */

#define AOELT_Link         (AOELT_Dummy+12)
   /* (void *) Link object for this element */

#define AOELT_Leftindent   (AOELT_Dummy+13)
#define AOELT_Rightindent  (AOELT_Dummy+14)
   /* (UBYTE) Indentation levels */

#define AOELT_Bullet       (AOELT_Dummy+15)
   /* (BOOL) Gives special left out-dentation */

#define AOELT_Floating     (AOELT_Dummy+16)
   /* (short) Set to HALIGN_FLOATxxx for floating objects */

#define AOELT_Resetlayout  (AOELT_Dummy+17)
   /* (BOOL) Reset all layout info */

#define AOELT_Visible      (AOELT_Dummy+18)  /* NEW,SET,GET */
   /* (BOOL) If element adds something visible. Default TRUE */

#define AOELT_Nobr         (AOELT_Dummy+19)  /* NEW,SET,GET */
   /* (BOOL) Element should not line break. */

#define AOELT_Incrementaly (AOELT_Dummy+20)  /* GET */
   /* (BOOL) Y coordinate to clear background from (and below) if
    * element supports incremental display. Default 0 = entire element.
    * Up to subclasses to implement. */

#define AOELT_Bgupdate      (AOELT_Dummy+21) /* GET */
    /* (ULONG) return value of bgupdate to avoid multiple renderings
     * during nested background rerenders */

#define AOELT_   (AOELT_Dummy+)
#define AOELT_   (AOELT_Dummy+)


/* Horizontal alignments */
#define HALIGN_LEFT        0
#define HALIGN_CENTER      1
#define HALIGN_RIGHT       2

#define HALIGN_BULLET      0x10  /* flag set for special left out-dentation */
#define HALIGN_FLOATLEFT   0x40  /* flag set for floating left object */
#define HALIGN_FLOATRIGHT  0x60  /* flag set for floating right object */

/* Vertical alignments */
#define VALIGN_BOTTOM      0
#define VALIGN_MIDDLE      1
#define VALIGN_TOP         2
#define VALIGN_SUP         3
#define VALIGN_SUB         4
#define VALIGN_BASELINE    5

/*--- element data ---*/

struct Element
{  struct Aobject object;  /* Object */
   long aox,aoy;           /* Left,top within context */
   long aow,aoh;           /* Object width and height */
   void *cframe;           /* The context */
   UBYTE halign;           /* Horizontal alignment */
   UBYTE valign;           /* Vertical alignment */
   long textpos;           /* Text offset within a buffer or 0 */
   long length;            /* Text length */
   UWORD eltflags;
   struct TextFont *font;
   UWORD style;
   void *link;
   UBYTE leftindent,rightindent;
                           /* Left and right indentation level */
   ULONG bgupdate;          /* Value of bgupdate last time this was rendered */
};

#define ELTF_MEASURED   0x0001   /* Gone through AOM_MEASURE */
#define ELTF_LAYEDOUT   0x0002   /* Gone through AOM_LAYOUT */
#define ELTF_ALIGNED    0x0004   /* Gone through AOM_ALIGN */
#define ELTF_PREFORMAT  0x0008   /* Text is preformatted */
#define ELTF_HALIGN     0x0010   /* AOELT_Halign was explicitly set */
#define ELTF_INVISIBLE  0x0020   /* Element is not visible */
#define ELTF_NOBR       0x0040   /* Element should not line break */

/*--- General goodies ---*/

extern struct RastPort *mrp;
   /* RastPort, can be used for measuring porposes */

#endif
