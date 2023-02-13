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

/* bullet.h - AWeb html bullet element object */

#ifndef AWEB_BULLET
#define AWEB_BULLET

#include "element.h"

/*--- bullet tags ---*/

#define AOBUL_Dummy        AOBJ_DUMMYTAG(AOTP_BULLET)

#define AOBUL_Type         (AOBUL_Dummy+1)
   /* (short) Bullet type */

#define AOBUL_    (AOBUL_Dummy+)
#define AOBUL_    (AOBUL_Dummy+)
#define AOBUL_    (AOBUL_Dummy+)
#define AOBUL_    (AOBUL_Dummy+)


/*--- bullet types ---*/

#define BULTP_DISC      1  /* The same values as body's BDBT types */
#define BULTP_CIRCLE    2
#define BULTP_SQUARE    3
#define BULTP_DIAMOND   4
#define BULTP_SOLIDDIA  5
#define BULTP_RECTANGLE 6

#endif
