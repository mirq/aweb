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

/* filereq.h - AWeb asynchroneous filerequester object */

#ifndef AWEB_FILEREQ_H
#define AWEB_FILEREQ_H

#include "object.h"

/*--- filereq tags ---*/

#define AOFRQ_Dummy        AOBJ_DUMMYTAG(AOTP_FILEREQ)

#define AOFRQ_Title        (AOFRQ_Dummy+1)
   /* (UBYTE *) Requester title */

#define AOFRQ_Filename     (AOFRQ_Dummy+2)
   /* (UBYTE *) Path and file name. Upon requester close, the full path
    * and file name will be sent in an AOM_UPDATE message, or NULL if
    * the requester was cancelled. The object will dispose itself. */

#define AOFRQ_Pattern      (AOFRQ_Dummy+3)
   /* (UBYTE *) Initial pattern. If nonnull, a pattern gadget is added and
    * the pattern on exit is sent in an AOM_UPDATE message. */

#define AOFRQ_Userdata     (AOFRQ_Dummy+4)
   /* (ULONG) Will be passed with AOFRQ_Filename upon requester close. */

#define AOFRQ_Targetwindow (AOFRQ_Dummy+5)
   /* (ULONG) This window key will be used to find the target instead of
    * AOBJ_Target. */

#define AOFRQ_Savemode     (AOFRQ_Dummy+6)
   /* (BOOL) This will be a save requester. */

#define AOFRQ_Savecheck    (AOFRQ_Dummy+7)
   /* (BOOL) When TRUE, it will check if the selected file exists and
    * ask for overwrite/append/new name. */

#define AOFRQ_Append       (AOFRQ_Dummy+8)
   /* (BOOL) Passed on UPDATE if user requested append to existing file. */

#define AOFRQ_Dirsonly     (AOFRQ_Dummy+9)   /* NEW */
   /* (BOOL) Make it a directory only requester */

#define AOFRQ_Arexx        (AOFRQ_Dummy+10)  /* NEW */
   /* (struct Arexxcmd *) To be replied when requester closes. */

#define AOFRQ_Unkurl       (AOFRQ_Dummy+11)  /* NEW */
   /* (UBYTE *) URL name for unknown type. If AOFRQ_Unkurl, AOFRQ_Unktype
    * and AOFRQ_Unkext are present, the "unknown MIME type" requester is
    * shown before the actual file requester is opened. */

#define AOFRQ_Unktype      (AOFRQ_Dummy+12)  /* NEW */
   /* (UBYTE *) MIME type for unknown type requester. */

#define AOFRQ_Unkext       (AOFRQ_Dummy+13)  /* NEW */
   /* (UBYTE *) File extension for unknown type requester. */

#define AOFRQ_    (AOFRQ_Dummy+)
#define AOFRQ_    (AOFRQ_Dummy+)

#endif
