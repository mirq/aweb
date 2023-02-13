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

/* scroller.h - AWeb scroller object */

#ifndef AWEB_SCROLLER_H
#define AWEB_SCROLLER_H

#include "object.h"

/* scroller tags */

#define AOSCR_Dummy        AOBJ_DUMMYTAG(AOTP_SCROLLER)

#define AOSCR_Orient       (AOSCR_Dummy+1)
   /* (UWORD) Orientation, see below */

#define AOSCR_Total        (AOSCR_Dummy+2)
#define AOSCR_Visible      (AOSCR_Dummy+3)
#define AOSCR_Top          (AOSCR_Dummy+4)
   /* (long) Scroller model dimensions */

#define AOSCR_Delta        (AOSCR_Dummy+5)
   /* (long) Number to scroll when arrows are pressed */

#define AOSCR_Delay        (AOSCR_Dummy+6)
   /* (long) Delay before arrows repeat */

#define AOSCR_Minwidth     (AOSCR_Dummy+7)
#define AOSCR_Minheight    (AOSCR_Dummy+8)
   /* (long) GET the minimum size of the scroller in the free orientation */

#define AOSCR_Update       (AOSCR_Dummy+9)   /* SET */
   /* (BOOL) Render the knob now, but not the frame. */

#define AOSCR_ (AOSCR_Dummy+)
#define AOSCR_ (AOSCR_Dummy+)

/* scroller orientations */
#define AOSCRORIENT_VERT   0
#define AOSCRORIENT_HORIZ  1

#endif
