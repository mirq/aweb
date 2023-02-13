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

/* filefield.h - AWeb html document file upload form field element object */

#ifndef AWEB_FILEFIELD
#define AWEB_FILEFIELD

#include "field.h"

/* filefield tags */

#define AOFUF_Dummy        AOBJ_DUMMYTAG(AOTP_FILEFIELD)

#define AOFUF_Size         (AOFUF_Dummy+1)   /* NEW */
   /* (long) Size of field in characters */

#define AOFUF_Input        (AOFUF_Dummy+2)   /* GET */
   /* (struct Aobject *) The AOTP_INPUT field contained in this field. */

#define AOFUF_    (AOFUF_Dummy+)
#define AOFUF_    (AOFUF_Dummy+)

#endif
