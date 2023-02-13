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

/* awebgif.h - AWeb gif plugin general definitions */

#include <libraries/awebsupport.h>

/* Base pointers of libraries needed */
extern struct ExecBase *SysBase;         /* Defined in startup.c */
extern struct DosLibrary *DOSBase;
extern struct Library *AwebSupportBase;
extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library *CyberGfxBase;

#ifdef __MORPHOS__
extern struct Library *UtilityBase;
#else
extern struct UtilityBase *UtilityBase;
#endif

#define max(a,b) ((a > b ? a : b))

/* Pointer to our own library base */
extern struct AwebGifBase *AwebPluginBase;


struct Gifsource;
struct Gifcopy;

/* Declarations of the OO dispatcher functions */
USRFUNC_P2
(
extern  ULONG  , Dispatchersource,
struct Gifsource *,,A0,
struct Amessage *,,A1
);

USRFUNC_P2
(
extern  ULONG  , Dispatchercopy,
struct Gifcopy *,,A0,
struct Amessage *,,A1
);

/* Declaration of common flags */
extern BOOL animate;

/* Definition of attribute IDs that are used internally. */

#define AOGIF_Dummy     AOBJ_DUMMYTAG(AOTP_PLUGIN)

#define AOGIF_Data      (AOGIF_Dummy+1)
   /* (BOOL) Let decoder task know that there is new data, or EOF was
    * reached. */

#define AOGIF_Readyfrom (AOGIF_Dummy+2)
#define AOGIF_Readyto   (AOGIF_Dummy+3)
   /* (long) The rows in the bitmap that has gotten new valid data. */

#define AOGIF_Error     (AOGIF_Dummy+4)
   /* (BOOL) The decoding process discovered an error */

#define AOGIF_Bitmap    (AOGIF_Dummy+5)
   /* (struct BitMap *) The bitmap to use, or NULL to remove
    * the bitmap. */

#define AOGIF_Width     (AOGIF_Dummy+6)
#define AOGIF_Height    (AOGIF_Dummy+7)
   /* (long) Dimensions of the bitmap. */

#define AOGIF_Imgready  (AOGIF_Dummy+8)
   /* (BOOL) The image is ready */

#define AOGIF_Mask      (AOGIF_Dummy+9)
   /* (UBYTE *) Transparent mask for the image */

#define AOGIF_Decodeready (AOGIF_Dummy+10)
   /* (BOOL) Image or animation decoding is complete */

#define AOGIF_Animframe (AOGIF_Dummy+11)
   /* (BOOL) This is a new animation frame. If it is transparent, clear
    * the background. */

#define AOGIF_Jsready   (AOGIF_Dummy+12)
   /* (BOOL) Decoding is complete */

#define AOGIF_Jsanim    (AOGIF_Dummy+13)
   /* (BOOL) The last animation frame is complete */

#define AOGIF_Memory    (AOGIF_Dummy+14)
   /* (long) Add this amount of memory to current usage */

#define AOGIF_Maxloops  (AOGIF_Dummy+15)
   /* (long) Maximum number of loops to show, or -1 for infinite. */

#define AOGIF_Newloop   (AOGIF_Dummy+16)
   /* (BOOL) This animation frame starts a new loop */

#define AOGIF_Loops     (AOGIF_Dummy+17)
   /* (long) Number of loops to show for this image file, or 0 for infinite. */
