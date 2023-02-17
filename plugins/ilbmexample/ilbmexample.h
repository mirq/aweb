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

/* ilbmexample.h
 *
 * Example AWeb plugin module - project header file
 *
 * This file contains some general definitions shared between
 * the various source files.
 *
 */

#include <libraries/awebsupport.h>
#include <stdio.h>

/* Base pointers of libraries needed */
extern struct ExecBase *SysBase;    /* Defined in startup.c */
extern struct Library *AwebSupportBase;
extern struct GfxBase *GfxBase;

/* Pointer to our own library base */
extern struct ILBMExampleBase *AwebPluginBase;

/* Declarations of the OO dispatcher functions */

struct Ilbmcopy;
struct Ilbmsource;

USRFUNC_P2
(
extern __saveds  ULONG   , Dispatchersource,
struct Ilbmsource *,,A0,
struct Amessage *,,A1
);

USRFUNC_P2
(
extern __saveds  ULONG   , Dispatchercopy,
struct Ilbmcopy *,,A0,
struct Amessage *,,A1
);

#define debug(b)              \
{                    \
    FILE *dbug;               \
    if (dbug = fopen("ram:dbug","a+"))      \
    {                    \
   fprintf(dbug,(b));           \
   fclose(dbug);              \
                    \
    }                    \
}

/* Definition of attribute IDs that are used internally. */

#define AOILBM_Dummy   AOBJ_DUMMYTAG(AOTP_PLUGIN)

#define AOILBM_Data   (AOILBM_Dummy+1)
   /* (BOOL) Let decoder task know that there is new data, or EOF was
    * reached. */

#define AOILBM_Ready   (AOILBM_Dummy+2)
   /* (long) The last row in the bitmap that has valid data */

#define AOILBM_Error   (AOILBM_Dummy+3)
   /* (BOOL) The decoding process discovered an error */

#define AOILBM_Bitmap   (AOILBM_Dummy+4)
   /* (struct BitMap *) The bitmap to use, or NULL to remove
    * the bitmap. */

#define AOILBM_Width   (AOILBM_Dummy+5)
#define AOILBM_Height   (AOILBM_Dummy+6)
   /* (long) Dimensions of the bitmap. */
