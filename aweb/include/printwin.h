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

/* printwin.h - AWeb off-screen print window object */

#ifndef AWEB_PRINTWINDOW_H
#define AWEB_PRINTWINDOW_H

#include "object.h"

/*--- printwindow tags ---*/

#define AOPRW_Dummy        AOBJ_DUMMYTAG(AOTP_PRINTWINDOW)

#define AOPRW_Width        (AOPRW_Dummy+1)   /* NEW */
#define AOPRW_Height       (AOPRW_Dummy+2)   /* NEW,SET */
   /* (long) Visible width and height */

#define AOPRW_Totalheight  (AOPRW_Dummy+3)   /* GET */
   /* (long) Total height of contents */

#define AOPRW_Cleartop     (AOPRW_Dummy+4)   /* SET */
   /* (BOOL) If TRUE, don't use the top row. */

#define AOPRW_Update       (AOPRW_Dummy+5)   /* SET */
   /* (BOOL) Update the window's frame */

#define AOPRW_Layoutheight (AOPRW_Dummy+6)   /* NEW */
   /* (long) Height to layout against */

#define AOPRW_Turboprint   (AOPRW_Dummy+7)   /* NEW */
   /* (BOOL) If the printer device can handle >8 bit bitmaps */

#define AOPRW_    (AOPRW_Dummy+)
#define AOPRW_    (AOPRW_Dummy+)

#endif
