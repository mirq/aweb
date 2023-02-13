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

/* textarea.h - AWeb html textarea form field element object */

#ifndef AWEB_TEXTAREA
#define AWEB_TEXTAREA

#include "field.h"

/* textarea tags */

#define AOTXA_Dummy        AOBJ_DUMMYTAG(AOTP_TEXTAREA)

#define AOTXA_Cols         (AOTXA_Dummy+1)
#define AOTXA_Rows         (AOTXA_Dummy+2)
   /* (long) Number of columns, rows */

#define AOTXA_Text         (AOTXA_Dummy+3)
   /* (UBYTE *) Add text to textarea field */

#define AOTXA_Complete     (AOTXA_Dummy+4)
   /* (BOOL) Definition is complete */

#define AOTXA_Left         (AOTXA_Dummy+5)
#define AOTXA_Top          (AOTXA_Dummy+6)
   /* (long) Private set scroll position */

#define AOTXA_    (AOTXA_Dummy+)
#define AOTXA_    (AOTXA_Dummy+)
#define AOTXA_    (AOTXA_Dummy+)

#endif
