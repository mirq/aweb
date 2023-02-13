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

/* area.h - AWeb html area object */

#ifndef AWEB_AREA
#define AWEB_AREA

#include "link.h"
#include "linkprivate.h"

/* area tags */

#define AOARA_Dummy        AOBJ_DUMMYTAG(AOTP_AREA)

#define AOARA_Shape        (AOARA_Dummy+1)   /* NEW,GET */
   /* (UWORD) Shape */

#define AOARA_Coords       (AOARA_Dummy+2)   /* NEW */
   /* (UBYTE *) Comma separated list of coordinates or NULL for MAPSHAPE_DEFAULT */

#define AOARA_Textpos      (AOARA_Dummy+3)   /* NEW */
   /* (long) Position in text buffer of ALT text */

#define AOARA_Textlength   (AOARA_Dummy+4)   /* NEW */
   /* (long) Length of text */

#define AOARA_Tooltip      (AOARA_Dummy+5)   /* GET */
   /* (UBYTE *) Tooltip text */

#define AOARA_    (AOARA_Dummy+)
#define AOARA_    (AOARA_Dummy+)

/* area shapes */

#define AREASHAPE_DEFAULT   0
#define AREASHAPE_RECTANGLE 1
#define AREASHAPE_CIRCLE    2
#define AREASHAPE_POLYGON   3

struct Area
{  struct Link link;
   UWORD shape;
   long nrcoords;          /* Number of coordinates */
   long *coords;           /* Array of coordinates, type dependent */
   long textpos,textlen;   /* Position of ALT text in parents buffer */
   UBYTE *tooltip;         /* Dynamic string */
};

/* Special hittest function */

extern BOOL Areahit(struct Area *area,long x,long y);
   /* Returns TRUE if these coordinates fall within this area */

extern BOOL Areaonclick(struct Area *area);
   /* Returns TRUE if link should be followed */

#endif
