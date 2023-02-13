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

/* break.h - AWeb html line break element object */

#ifndef AWEB_BREAK
#define AWEB_BREAK

#include "element.h"

/* break tags */

#define AOBRK_Dummy        AOBJ_DUMMYTAG(AOTP_BREAK)

#define AOBRK_Clearleft    (AOBRK_Dummy+1)
#define AOBRK_Clearright   (AOBRK_Dummy+2)
   /* (BOOL) Wants clear to left and/or right margin */

#define AOBRK_Wbr          (AOBRK_Dummy+3) /* NEW,SET */
   /* (BOOL) Soft word break, don't break unless no fit */

#define AOBRK_    (AOBRK_Dummy+)
#define AOBRK_    (AOBRK_Dummy+)

#endif
