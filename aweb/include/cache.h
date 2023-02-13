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

/* cache.h - AWeb cache object */

#ifndef AWEB_CACHE_H
#define AWEB_CACHE_H

#include "object.h"

/*--- cache tags ---*/

#define AOCAC_Dummy        AOBJ_DUMMYTAG(AOTP_CACHE)

#define AOCAC_Url          (AOCAC_Dummy+1)
   /* (void *) URL this cache is for. */

#define AOCAC_Name         (AOCAC_Dummy+2)
   /* (UBYTE *) Fully qualified file name. */

#define AOCAC_Number       (AOCAC_Dummy+3)
   /* (long) Cache sequence number */

#define AOCAC_Cachedate    (AOCAC_Dummy+4)
   /* (ULONG) Date object went into cache, in seconds from 1-1-1978 */

#define AOCAC_Lastmodified (AOCAC_Dummy+5)
   /* (ULONG) Last modified date, remote time, in system seconds. */

#define AOCAC_Contenttype  (AOCAC_Dummy+6)
   /* (UBYTE *) Contenttype as it was cached. */

#define AOCAC_Expired      (AOCAC_Dummy+7)
   /* (BOOL) Object is past expiry date. */

#define AOCAC_Touched      (AOCAC_Dummy+8)   /* SET */
   /* (BOOL) Object is touched */

#define AOCAC_Sendinfo     (AOCAC_Dummy+9)   /* SET */
   /* (void *) Send cache's url SRCUPDATE with headers, use this as FETCH pointer */

#define AOCAC_Charset      (AOCAC_Dummy+10)
   /* (UBYTE*) Charset as it was cached */

#define AOCAC_Expires      (AOCAC_Dummy+11) /* GET */
   /* (ULONG) Expires date of cached item */

#define AOCAC_Etag      (AOCAC_Dummy+12)
   /* (UBYTE*) Etag as it was cached */

#define AOCAC_    (AOCAC_Dummy+)
#define AOCAC_    (AOCAC_Dummy+)
#define AOCAC_    (AOCAC_Dummy+)


/*--- external functions ---*/

extern UBYTE *Cachename(void);
   /* Fully qualified name of the cache directory (static) */

extern void Flushcache(UWORD type);
extern void Flushcachepattern(UWORD type,UBYTE *pattern);
   /* Delete cache files from disk */

extern void Fixcache(BOOL force);
   /* Synchronize cache and registration. */

extern void Getcachecontents(struct Arexxcmd *ac,UBYTE *stem,UBYTE *pattern);
   /* Obtain cache contents in an ARexx stem variable */

/* flush types */

#define CACFT_DOCUMENTS    1  /* Document types (text/...) */
#define CACFT_IMAGES       2  /* Nondocument types */
#define CACFT_EXCESS       3  /* All files beyond disk limit */
#define CACFT_ALL          4  /* All files (erase cache) */

#endif
