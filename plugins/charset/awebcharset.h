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

/* awebcharset.h
 *
 * This file contains some general definitions shared between
 * the various source files.
 *
 */

#include <libraries/awebsupport.h>

/* Base pointers of libraries needed */
extern struct ExecBase *SysBase;         /* Defined in startup.c */
extern struct Library *AwebSupportBase;
extern struct Library *CodesetsBase;
extern struct IntuitionBase *IntuitionBase;
extern struct DosLibrary *DOSBase;

#define ALLOCTYPE(t,n,f)   (t*)Allocmem((n)*sizeof(t),(f))
#define ALLOCSTRUCT(s,n,f) ALLOCTYPE(struct s,n,f)
#define FREE(p)            Freemem(p)

/* Structure as our filter user data */
struct Filterdata
{
   BOOL   first;                 /* The first block is to be processed */
   BOOL   on;                    /* Filter is on */
   BOOL   replace;               /* Replace Mode is on */
   BOOL   request;               /* Show a requester if meta and header charsets differ */
   UBYTE  srccharset[42];        /* Document's (Header, Meta or Default) character set Name */
   UBYTE  destcharset[42];       /* Destination character set Name */
   struct codeset *srcCodeset;   /* srcCodeset from CodesetsFind() call */
   struct codeset *dstCodeset;   /* dstCodeset from CodesetsFind() call */
   UBYTE  *olddata;              /* Pointer to the data cut from the last Block */
   long   olddatalen;            /* Number of Bytes cut from the last Block */
   long   totallen;              /* total Number of needed Bytes for the UTF-8 sequence */
   UBYTE  *buf;                  /* Pointer to an already allocated buffer */
   long   bufsize;               /* size of the allocated buffer */
};
