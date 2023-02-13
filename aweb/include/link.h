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

/* link.h - AWeb html link object */

#ifndef AWEB_LINK
#define AWEB_LINK

#include "object.h"

/* link tags */

#define AOLNK_Dummy        AOBJ_DUMMYTAG(AOTP_LINK)

#define AOLNK_Visited      (AOLNK_Dummy+1)
   /* (BOOL) Link has been visited before */

#define AOLNK_Url          (AOLNK_Dummy+2)
   /* (struct Url *) Url for this link */

#define AOLNK_Fragment     (AOLNK_Dummy+3)
   /* (UBYTE *) Fragment name for this link or NULL */

#define AOLNK_Text         (AOLNK_Dummy+4)
   /* (struct Buffer *) Text buffer of parent, to use in AOM_UPDATE messages. */

#define AOLNK_Selected     (AOLNK_Dummy+5)
   /* (BOOL) Link is currently selected */

#define AOLNK_Target       (AOLNK_Dummy+6)   /* NEW */
   /* (UBYTE *) Target frame for this link */

#define AOLNK_Title        (AOLNK_Dummy+7)   /* NEW */
   /* (UBYTE *) Title to display as prompt */

#define AOLNK_Onclick      (AOLNK_Dummy+8)   /* NEW,SET */
#define AOLNK_Onmouseover  (AOLNK_Dummy+9)   /* NEW,SET */
#define AOLNK_Onmouseout   (AOLNK_Dummy+10)  /* NEW,SET */
   /* (UBYTE *) JS event handlers */

#define AOLNK_Post         (AOLNK_Dummy+11)  /* NEW */
   /* (BOOL) Do a POST request instead of GET */

#define AOLNK_    (AOLNK_Dummy+)
#define AOLNK_    (AOLNK_Dummy+)


#endif
