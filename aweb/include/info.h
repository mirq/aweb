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

/* info.h - AWeb info window interface */

#ifndef AWEB_INFO_H
#define AWEB_INFO_H

#include "object.h"

/*--- info tags ---*/

#define AOINF_Dummy        AOBJ_DUMMYTAG(AOTP_INFO)

#define AOINF_Target       (AOINF_Dummy+1)    /* SET */
   /* (void *) Object will be sent AOM_SET of AOINF_Inquire */

#define AOINF_Inquire      (AOINF_Dummy+2)    /* */
   /* (void *) The INFO object that wants to know about the receiver */

#define AOINF_Text         (AOINF_Dummy+4)    /* SET */
   /* (UBYTE *) Header or detail text */

#define AOINF_Header       (AOINF_Dummy+5)    /* SET */
   /* (BOOL) This text is a header line */

#define AOINF_Link         (AOINF_Dummy+6)    /* SET */
   /* (void *) This text is a clickable link to this URL. */

#define AOINF_Windowkey    (AOINF_Dummy+7)    /* SET */
   /* (ULONG) Key of window to open links in. */

#define AOINF_Url          (AOINF_Dummy+8)    /* SET */
   /* (void *) URL this info is for. */

#define AOINF_Frame        (AOINF_Dummy+9)    /* NEW */
   /* (struct Aobject *) Object that will be sent AOFRM_Info,FALSE when
    * info window wants to close. */

#define AOINF_    (AOINF_Dummy+)    /* */
#define AOINF_    (AOINF_Dummy+)    /* */


#endif
