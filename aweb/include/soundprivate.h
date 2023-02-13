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

/* soundprivate.h - AWeb sound drivers private */

#include "object.h"

/*--- Sound source driver ---*/

struct Sndsource
{  struct Aobject object;
   void *source;              /* Our source object */
   void *file;                /* Temporary file holding sound source */
   UBYTE *filename;           /* Name of file holding sound source */
   UWORD flags;
};

#define SNSF_EOF        0x0001   /* EOF was reached on input */

/*--- Private tags: ---*/

#define AOSNP_Dummy        AOBJ_DUMMYTAG(AOTP_SOUNDCOPY)

#define AOSNP_Srcupdate    (AOSNP_Dummy+101)
   /* Source file is available */
