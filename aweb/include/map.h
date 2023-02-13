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

/* map.h - AWeb html map description object */

#ifndef AWEB_MAP
#define AWEB_MAP

#include "object.h"

/* map tags */

#define AOMAP_Dummy        AOBJ_DUMMYTAG(AOTP_MAP)

#define AOMAP_Name         (AOMAP_Dummy+1)
   /* (UBYTE *) Name of this map (relative to document) */

/* When used in a AOM_GET, pass these two with a value to get AOMAP_Area */
#define AOMAP_Xco          (AOMAP_Dummy+2)   /* GET */
#define AOMAP_Yco          (AOMAP_Dummy+3)   /* GET */
   /* (long) Map relative mouse position */

#define AOMAP_Area         (AOMAP_Dummy+4)   /* SET,GET */
   /* (void *) An AREA object to add to this map, or to get from coordinates */

#define AOMAP_Extcopy      (AOMAP_Dummy+5)   /* NEW */
   /* (void *) COPY element that contains the external MAP definition */

#define AOMAP_Extname      (AOMAP_Dummy+6)   /* NEW */
   /* (UBYTE *) Map name to get from the external document */

#define AOMAP_    (AOMAP_Dummy+)
#define AOMAP_    (AOMAP_Dummy+)

#endif
