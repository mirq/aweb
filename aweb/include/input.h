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

/* input.h - AWeb html text input form field element object */

#ifndef AWEB_INPUT
#define AWEB_INPUT

#include "field.h"

/* input tags */

#define AOINP_Dummy        AOBJ_DUMMYTAG(AOTP_INPUT)

#define AOINP_Type         (AOINP_Dummy+1)
   /* (UWORD) Field type, see below */

#define AOINP_Maxlength    (AOINP_Dummy+2)
   /* (long) Maximum number of characters to hold */

#define AOINP_Size         (AOINP_Dummy+3)
   /* (long) Size of field in characters */

#define AOINP_    (AOINP_Dummy+)
#define AOINP_    (AOINP_Dummy+)
#define AOINP_    (AOINP_Dummy+)

/* input types */
#define INPTP_TEXT      1
#define INPTP_PASSWORD  2

#endif
