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

/* awebpng.h - AWeb png plugin general definitions */

#include <libraries/awebsupport.h>

#define max(a,b) ((a > b ? a : b))


/* Base pointers of libraries needed */
extern struct ExecBase *SysBase; /* Defined in startup.c */
extern struct Library *AwebSupportBase;
extern struct GfxBase *GfxBase;
extern struct Library *CyberGfxBase;


/* Pointer to our own library base */
extern struct AwebPngBase *AwebPluginBase;

struct Pngsource;
struct Pngcopy;

/* Declarations of the OO dispatcher functions */
USRFUNC_P2
(
extern __saveds  ULONG  , Dispatchersource,
struct Pngsource *,,A0,
struct Amessage *,,A1
);

USRFUNC_P2
(
extern __saveds  ULONG  , Dispatchercopy,
struct Pngcopy *,,A0,
struct Amessage *,,A1
);

/* Definition of attribute IDs that are used internally. */

#define AOPNG_Dummy     AOBJ_DUMMYTAG(AOTP_PLUGIN)

#define AOPNG_Data      (AOPNG_Dummy+1)
   /* (BOOL) Let decoder task know that there is new data, or EOF was
    * reached. */

#define AOPNG_Readyfrom (AOPNG_Dummy+2)
#define AOPNG_Readyto   (AOPNG_Dummy+3)
   /* (long) The rows in the bitmap that has gotten new valid data. */

#define AOPNG_Error     (AOPNG_Dummy+4)
   /* (BOOL) The decoding process discovered an error */

#define AOPNG_Bitmap    (AOPNG_Dummy+5)
   /* (struct BitMap *) The bitmap to use, or NULL to remove
    * the bitmap. */

#define AOPNG_Width     (AOPNG_Dummy+6)
#define AOPNG_Height    (AOPNG_Dummy+7)
   /* (long) Dimensions of the bitmap. */

#define AOPNG_Imgready  (AOPNG_Dummy+8)
   /* (BOOL) The image is ready */

#define AOPNG_Mask      (AOPNG_Dummy+9)
   /* (UBYTE *) Transparent mask for the image */

#define AOPNG_Memory    (AOPNG_Dummy+10)
   /* (long) Add this amount of memory to current usage */

#define AOPNG_Alpha     (AOPNG_Dummy+11)
  /*  (BOOL)  Using alpha channel */

#define AOPNG_PixelArray    (AOPNG_Dummy+12)
   /* (struct BitMap *) The bitmap to use, or NULL to remove
    * the bitmap. */
