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

/* select.h - AWeb html selector form field element object */

#ifndef AWEB_SELECT
#define AWEB_SELECT

#include "field.h"

/* select tags */

#define AOSEL_Dummy        AOBJ_DUMMYTAG(AOTP_SELECT)

#define AOSEL_Size         (AOSEL_Dummy+1)
   /* (short) Number of visible items. */

#define AOSEL_Multiple     (AOSEL_Dummy+2)
   /* (BOOL) Allow multiple item selection. */

#define AOSEL_Option       (AOSEL_Dummy+3)
   /* (BOOL) Start a new option. */

#define AOSEL_Optionvalue  (AOSEL_Dummy+4)
   /* (UBYTE *) This option's value */

#define AOSEL_Selected     (AOSEL_Dummy+5)
   /* (BOOL) Option is selected */

#define AOSEL_Optiontext   (AOSEL_Dummy+6)
   /* (UBYTE *) Option contents. More than one text will be concatenated. */

#define AOSEL_Complete     (AOSEL_Dummy+7)
   /* (BOOL) Definition is complete. */

#define AOSEL_Listtop      (AOSEL_Dummy+8)
   /* (long) Top item of list, internal use only */

#define AOSEL_    (AOSEL_Dummy+)
#define AOSEL_    (AOSEL_Dummy+)
#define AOSEL_    (AOSEL_Dummy+)


#endif
