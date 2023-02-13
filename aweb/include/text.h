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

/* text.h - AWeb html text element object */

#ifndef AWEB_TEXT
#define AWEB_TEXT

#include "element.h"

/* text tags */

#define AOTXT_Dummy        AOBJ_DUMMYTAG(AOTP_TEXT)

#define AOTXT_Blink        (AOTXT_Dummy+1)   /* NEW */
   /* (BOOL) Text should blink */

#define AOTXT_Text         (AOTXT_Dummy+2)   /* NEW */
   /* (struct Buffer *) Text buffer of parent, to use when rendering */

#define AOTXT_    (AOTXT_Dummy+)
#define AOTXT_    (AOTXT_Dummy+)

#endif
