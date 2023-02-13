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

/* ruler.h - AWeb html ruler element object */

#ifndef AWEB_RULER
#define AWEB_RULER

#include "element.h"

/* ruler tags */

#define AORUL_Dummy        AOBJ_DUMMYTAG(AOTP_RULER)

#define AORUL_Width        (AORUL_Dummy+1)
#define AORUL_Pixelwidth   (AORUL_Dummy+2)
   /* (short) Percentage or pixel width. Default is 100% */

#define AORUL_Color        (AORUL_Dummy+3)
   /* (struct Colorinfo *) Color for this ruler. Implies Noshade */

#define AORUL_Noshade      (AORUL_Dummy+4)
   /* (BOOL) No 3d effects */

#define AORUL_Size         (AORUL_Dummy+5)
   /* (short) Vertical size of ruler */

#define AORUL_    (AORUL_Dummy+)
#define AORUL_    (AORUL_Dummy+)
#define AORUL_    (AORUL_Dummy+)
#define AORUL_    (AORUL_Dummy+)
#define AORUL_    (AORUL_Dummy+)
#define AORUL_    (AORUL_Dummy+)
#define AORUL_    (AORUL_Dummy+)

#endif
