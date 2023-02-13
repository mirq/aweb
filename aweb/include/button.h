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

/* button.h - AWeb html button element object */

#ifndef AWEB_BUTTON
#define AWEB_BUTTON

#include "field.h"

/* button tags */

#define AOBUT_Dummy        AOBJ_DUMMYTAG(AOTP_BUTTON)

#define AOBUT_Type         (AOBUT_Dummy+1)   /* NEW */
   /* (UWORD) Button type, see below */

#define AOBUT_Custom       (AOBUT_Dummy+2)   /* NEW */
   /* (BOOL) This is a <BUTTON> type button with custom label */

#define AOBUT_Body         (AOBUT_Dummy+3)   /* GET */
   /* (struct Aobject *) Get the body to add elements to for a custom button. */

#define AOBUT_Complete     (AOBUT_Dummy+4)   /* SET */
   /* (BOOL) Custom body definition is complete. */

#define AOBUT_    (AOBUT_Dummy+)
#define AOBUT_    (AOBUT_Dummy+)
#define AOBUT_    (AOBUT_Dummy+)

/* button types */
#define BUTTP_SUBMIT    1
#define BUTTP_RESET     2
#define BUTTP_BUTTON    3

#endif
