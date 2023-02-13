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

/* search.h - AWeb search function object */

#ifndef AWEB_SEARCH_H
#define AWEB_SEARCH_H

#include "object.h"

/* search tags */

#define AOSRH_Dummy        AOBJ_DUMMYTAG(AOTP_SEARCH)

#define AOSRH_Reset        (AOSRH_Dummy+1)   /* SET */
   /* (BOOL) Frame contents has changed. */

#define AOSRH_Scrolled     (AOSRH_Dummy+2)   /* SET */
   /* (BOOL) Frame contents has moved. */

#define AOSRH_Activate     (AOSRH_Dummy+3)   /* SET */
   /* (BOOL) Bring to front and activate */

#define AOSRH_    (AOSRH_Dummy+)
#define AOSRH_    (AOSRH_Dummy+)

#endif
