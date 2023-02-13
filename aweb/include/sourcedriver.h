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

/* sourcedriver.h - AWeb source driver interface */

#ifndef AWEB_SOURCEDRIVER_H
#define AWEB_SOURCEDRIVER_H

#include "object.h"

/*--- sourcedriver tags ---*/

#define AOSDV_Dummy        AOBJ_DUMMYTAG(AOTP_SOURCEDRIVER)

#define AOSDV_Source       (AOSDV_Dummy+1)   /* NEW */
   /* (void *) The source object owning this driver. */

#define AOSDV_Name         (AOSDV_Dummy+2)   /* NEW */
#define AOSDV_Arguments    (AOSDV_Dummy+3)   /* NEW */
   /* (UBYTE *) Additional parameters, only understood by EXTPROG and Plugin drivers. */

#define AOSDV_Saveable     (AOSDV_Dummy+4)   /* GET */
   /* (BOOL) If driver can save its source. */

#define AOSDV_Viewable     (AOSDV_Dummy+5)   /* GET */
   /* (BOOL) If source is in readable text form. */

#define AOSDV_Savesource   (AOSDV_Dummy+6)   /* SET */
   /* (void *) A AOTP_FILE object. The source must be written to this file. */

#define AOSDV_Displayed    (AOSDV_Dummy+7)   /* SET,GET */
   /* (BOOL) Set when becoming displayed or undisplayed. */

#define AOSDV_Volatile     (AOSDV_Dummy+8)   /* GET */
   /* (BOOL) If driver can not be reused for the next fetch */

#define AOSDV_Getsource    (AOSDV_Dummy+9)   /* GET */
   /* (UBYTE *) Get the object's source. */

#define AOSDV_Getable      (AOSDV_Dummy+10)  /* GET */
   /* (BOOL) If driver can return its source data on AOSDV_Getsource. */

#define AOSDV_Editsource   (AOSDV_Dummy+11)  /* SET */
   /* (BOOL) If driver supports it, start editor to edit source. */

#define AOSDV_Pipe         (AOSDV_Dummy+12)  /* NEW */
   /* (BOOL) External program driver should use pipe */

#define AOSDV_Noicon       (AOSDV_Dummy+13)  /* NEW */
   /* (BOOL) Saving driver should not create icons */

#define AOSDV_Unknowntype  (AOSDV_Dummy+14)  /* NEW */
   /* (UBYTE *) This driver is used because this MIME type is unknown. */

#define AOSDV_Nofetch       (AOSDV_Dummy + 15) /* NEW */
    /* (BOOL) External program driver does not require data fetch */

#define AOSDV_    (AOSDV_Dummy+)
#define AOSDV_    (AOSDV_Dummy+)

/*--- sourcedriver data ---*/

struct Sourcedriver
{  struct Aobject object;     /* Object header */
   void *extension;           /* Private extension data */
};

#endif
