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

/* editor.h - AWeb external editor object */

#ifndef AWEB_EDITOR_H
#define AWEB_EDITOR_H

#include "object.h"

/* Spawns an external editor. When the file is updated, the AOBJ_Target
 * is sent an AOM_UPDATE message. This message contains the AOEDT_Data
 * and AOEDT_Datalength attributes. */

/*--- editor tags ---*/

#define AOEDT_Dummy        AOBJ_DUMMYTAG(AOTP_EDITOR)

#define AOEDT_Data         (AOEDT_Dummy+1)   /* NEW,UPDATE */
   /* (UBYTE *) Data to edit. */

#define AOEDT_Datalength   (AOEDT_Dummy+2)   /* NEW,UPDATE */
   /* (long) Length of data to edit. */

#define AOEDT_Filename     (AOEDT_Dummy+3)   /* NEW */
   /* (UBYTE *) Name of file to edit. By default editor creates a new temporary file. */

#define AOEDT_Filedate     (AOEDT_Dummy+4)   /* UPDATE */
   /* (ULONG) If editor was created with AOEDT_Filename, the last file
    * date is passed with UPDATE messages too. */

#define AOEDT_    (AOEDT_Dummy+)
#define AOEDT_    (AOEDT_Dummy+)


#endif
