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

/* document.h - AWeb document driver object */

#ifndef AWEB_DOCUMENT_H
#define AWEB_DOCUMENT_H

#include "object.h"

/* document tags */

#define AODOC_Dummy        AOBJ_DUMMYTAG(AOTP_DOCUMENT)

#define AODOC_Mapname      (AODOC_Dummy+1)   /* GET */
   /* (UBYTE *) Name of MAP object to forward this GET message to */

#define AODOC_Base         (AODOC_Dummy+2)   /* GET */
   /* (UBYTE *) Base for relative URLs */

#define AODOC_    (AODOC_Dummy+)
#define AODOC_    (AODOC_Dummy+)


#endif
