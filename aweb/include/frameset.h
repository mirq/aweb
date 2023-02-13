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

/* frameset.h - AWeb html document frameset object */

#ifndef AWEB_FRAMESET_H
#define AWEB_FRAMESET_H

#include "frame.h"

/*--- frameset tags ---*/

#define AOFRS_Dummy        AOBJ_DUMMYTAG(AOTP_FRAMESET)

#define AOFRS_Cols         (AOFRS_Dummy+1)   /* NEW */
#define AOFRS_Rows         (AOFRS_Dummy+2)   /* NEW */
   /* (UBYTE *) A column or row set specification. */

#define AOFRS_Spacing      (AOFRS_Dummy+4)   /* NEW */
   /* (short) Spacing in pixels between frames. */

#define AOFRS_Defaultspacing (AOFRS_Dummy+5) /* SET */
   /* (short) Spacing in pixels, only if no AOFRS_Spacing was set before. */

#define AOFRS_Endframeset  (AOFRS_Dummy+6)   /* SET */
   /* (-) End frameset definition. */

#define AOFRS_Sensible     (AOFRS_Dummy+7)   /* GET */
   /* (BOOL) Returns if the frameset makes sense, i.e. contains at least
    * 2 columns or rows */

#define AOFRS_    (AOFRS_Dummy+)
#define AOFRS_    (AOFRS_Dummy+)

/* Frameset uses the following tags from frame.h:
 *    AOFRM_Border
 *    AOFRM_Defaultborder
 *    AOFRM_Resizeleft/right/top/bottom
 */

#endif
