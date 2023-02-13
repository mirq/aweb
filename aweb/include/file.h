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

/* file.h - AWeb file object */

#ifndef AWEB_FILE_H
#define AWEB_FILE_H

#include "object.h"

/*--- file tags ---*/

#define AOFIL_Dummy        AOBJ_DUMMYTAG(AOTP_FILE)

#define AOFIL_Name         (AOFIL_Dummy+1)
   /* (UBYTE *) NEW: use this filename, otherwise generate one in the
    * temporary path but use AOFIL_Extension.
    * GET: Return fully qualified file name. */

#define AOFIL_Extension    (AOFIL_Dummy+2)
   /* (UBYTE *) Use this extension for generated name. */

#define AOFIL_Data         (AOFIL_Dummy+3)
   /* (UBYTE *) Data block to write. The block is actually written at the
    * AOFIL_Datalength tag. You may pass any number of AOFIL_Data,AOFIL_Datalength
    * sequences, AOFIL_Copyfile and AOFIL_Stringdata tags in one AOM_SET message. */

#define AOFIL_Datalength   (AOFIL_Dummy+4)
   /* (long) Length of data block to write. */

#define AOFIL_Eof          (AOFIL_Dummy+5)
   /* (BOOL) SET to close file. */

#define AOFIL_Delete       (AOFIL_Dummy+6)
   /* (BOOL) Delete file when disposed. Default TRUE, SET to FALSE to keep the
    * file. */

#define AOFIL_Icontype     (AOFIL_Dummy+7)
   /* (UWORD) Save an icon at EOF. */

#define AOFIL_Comment      (AOFIL_Dummy+8)
   /* (UBYTE *) Set this file comment at EOF. */

#define AOFIL_Append       (AOFIL_Dummy+9)
   /* (BOOL) Open file in append mode. Ignored if no AOFIL_Name is given. */

#define AOFIL_Copyfile     (AOFIL_Dummy+10)
   /* (UBYTE *) Copy all data from this file */

#define AOFIL_Stringdata   (AOFIL_Dummy+11)  /* SET */
   /* (UBYTE *) String to write, length is length of string. */

#define AOFIL_Filesize     (AOFIL_Dummy+12)  /* GET */
   /* (long) The current file size. Also valid after close, and possible
    * modification by an external source. */

#define AOFIL_Datestamp    (AOFIL_Dummy+13)  /* GET */
   /* (ULONG) Datestamp of file in system seconds. */

#define AOFIL_Pipe         (AOFIL_Dummy+14)  /* NEW */
   /* (BOOL) Create file as a unique named PIPE. */

#define AOFIL_Error        (AOFIL_Dummy+15)  /* GET */
   /* (BOOL) Writing to file has errored. */

#define AOFIL_Commonfile   (AOFIL_Dummy+16)  /* NEW */
   /* (BOOL) File will be common, see below */

#define AOFIL_    (AOFIL_Dummy+)
#define AOFIL_    (AOFIL_Dummy+)


/*--- file icon types ---*/

#define FILEICON_NONE      0  /* no icon */
#define FILEICON_TEXT      1  /* project icon with AWeb as default tool */
#define FILEICON_DATA      2  /* project icon with no default tool */


/*--- common files ---*/

/* Create a file with AOFIL_Common,TRUE and use the object's address
 * as ID. Use the function below to find the file object back from
 * it's ID. If no file exists, NULL is returned. */
void *Findcommonfile(ULONG id);

#endif
