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

/* awebjfif.h - AWeb jfif plugin general definitions */

#include <libraries/awebsupport.h>

#define max(a,b) ((a > b ? a : b))


/* Base pointers of libraries needed */
extern struct ExecBase *SysBase;    /* Defined in startup.c */
extern struct Library *AwebSupportBase;
extern struct DosLibrary *DOSBase;
extern struct GfxBase *GfxBase;
extern struct Library *CyberGfxBase;

/* Pointer to our own library base */
extern struct AwebJfifBase *AwebPluginBase;

/* Declarations of the OO dispatcher functions */
struct Jfifsource;
struct Jfifcopy;

USRFUNC_P2
(
extern __saveds ULONG  , Dispatchersource,
struct Jfifsource *,,A0,
struct Amessage *,,A1
);

USRFUNC_P2
(
extern __saveds ULONG  , Dispatchercopy,
struct Jfifcopy *,,A0,
struct Amessage *,,A1
);

/* Definition of attribute IDs that are used internally. */

#define AOJFIF_Dummy    AOBJ_DUMMYTAG(AOTP_PLUGIN)

#define AOJFIF_Data    (AOJFIF_Dummy+1)
   /* (BOOL) Let decoder task know that there is new data, or EOF was
    * reached. */

#define AOJFIF_Readyfrom (AOJFIF_Dummy+2)
#define AOJFIF_Readyto    (AOJFIF_Dummy+3)
   /* (long) The rows in the bitmap that has gotten new valid data. */

#define AOJFIF_Error    (AOJFIF_Dummy+4)
   /* (BOOL) The decoding process discovered an error */

#define AOJFIF_Bitmap    (AOJFIF_Dummy+5)
   /* (struct BitMap *) The bitmap to use, or NULL to remove
    * the bitmap. */

#define AOJFIF_Width    (AOJFIF_Dummy+6)
#define AOJFIF_Height    (AOJFIF_Dummy+7)
   /* (long) Dimensions of the bitmap. */

#define AOJFIF_Imgready  (AOJFIF_Dummy+8)
   /* (BOOL) The image is ready */

#define AOJFIF_Memory    (AOJFIF_Dummy+9)
   /* (long) Add this amount of memory to current usage */
