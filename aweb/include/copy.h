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

/* copy.h - AWeb copy element object */

#ifndef AWEB_COPY_H
#define AWEB_COPY_H

#include "field.h"

/* copy tags */

#define AOCPY_Dummy        AOBJ_DUMMYTAG(AOTP_COPY)

#define AOCPY_Source       (AOCPY_Dummy+1)
   /* (void *) Source interpreter this copy belongs to */

#define AOCPY_Url          (AOCPY_Dummy+2)
   /* (void *) Url to find source interpreter for */

#define AOCPY_Driver       (AOCPY_Dummy+3)
   /* (void *) Driver for this copy.
    * When set to NULL, existing driver should be disposed of. */

#define AOCPY_Embedded     (AOCPY_Dummy+4)
#define AOCPY_Background   (AOCPY_Dummy+5)
   /* (BOOL) Embedded or background object, otherwise it is a top object */

#define AOCPY_Border       (AOCPY_Dummy+6)
   /* (long) Link border size */

#define AOCPY_Hspace       (AOCPY_Dummy+7)
#define AOCPY_Vspace       (AOCPY_Dummy+8)
   /* (long) Extra gutter around the object */

#define AOCPY_Width        (AOCPY_Dummy+9)
#define AOCPY_Height       (AOCPY_Dummy+10)
   /* (long) Suggested dimensions (scaling) in pixels. */

#define AOCPY_Usemap       (AOCPY_Dummy+11)
   /* (void *) AOTP_MAP object to use for linked URL construction */

#define AOCPY_Ismap        (AOCPY_Dummy+12)
   /* (BOOL) Use coordinates to construct linked URL */

#define AOCPY_Text         (AOCPY_Dummy+13)  /* NEW,SET */
   /* (struct Buffer *) Text buffer to use for in-place rerenderings */

#define AOCPY_Referer      (AOCPY_Dummy+14)  /* NEW,SET,GET */
   /* (void *) URL object to use as referer in AUM_LOAD messages for this object */

#define AOCPY_Error        (AOCPY_Dummy+15)  /* SET */
   /* (BOOL) Source is in error */

#define AOCPY_Defaulttype  (AOCPY_Dummy+16)  /* NEW */
   /* (UBYTE *) Default content type, forward to SOURCE. */

#define AOCPY_Hlayout      (AOCPY_Dummy+17)  /* GET */
#define AOCPY_Vlayout      (AOCPY_Dummy+18)  /* GET */
   /* (BOOL) Forwarded to driver; if object is sensitive to changes in
    * this direction (i.e. if it wants to layout again */

#define AOCPY_Soundloop    (AOCPY_Dummy+19)  /* NEW,SET */
   /* (long) Number of times to play sound. Its appearance implies that it is
    * a background sound. */

#define AOCPY_Reloadverify (AOCPY_Dummy+20)  /* NEW,GET */
   /* (BOOL) This is subject to a reload, forced verify all children. */

#define AOCPY_Info         (AOCPY_Dummy+21)  /* SET */
   /* (struct Aobject *) The AOTP_INFO object to use */

#define AOCPY_Nodisplay    (AOCPY_Dummy+22)  /* SET */
   /* (BOOL) External viewer is used, copy will never be displayed. */

#define AOCPY_Mapdocument  (AOCPY_Dummy+23)  /* NEW */
   /* (BOOL) This is a HTML document for <MAP> definitions only. */

#define AOCPY_Paramname    (AOCPY_Dummy+24)  /* SET */
#define AOCPY_Paramtype    (AOCPY_Dummy+25)  /* SET */
#define AOCPY_Paramvalue   (AOCPY_Dummy+26)  /* SET */
#define AOCPY_Paramvaluetype (AOCPY_Dummy+27) /* SET */
   /* (UBYTE *) <OBJECT> parameter details. Set all for one parameter in one AOM_SET */

#define AOCPY_Objectready  (AOCPY_Dummy+28)  /* NEW,SET */
   /* (BOOL) When set to FALSE in AOM_NEW, don't attempt to load the object
    * until it is set to TRUE afterwards. */

#define AOCPY_Mwidth       (AOCPY_Dummy+29)  /* NEW */
#define AOCPY_Mheight      (AOCPY_Dummy+30)  /* NEW */
   /* (long) Margins to forward to driver */

#define AOCPY_Trueimage    (AOCPY_Dummy+31)  /* NEW */
   /* (BOOL) True image from <IMG> tag; needs JS Image object */

#define AOCPY_Name         (AOCPY_Dummy+32)  /* NEW */
   /* (UBYTE *) Name attribute if this image */

#define AOCPY_Onload       (AOCPY_Dummy+33)  /* NEW */
#define AOCPY_Onerror      (AOCPY_Dummy+34)  /* NEW */
#define AOCPY_Onabort      (AOCPY_Dummy+35)  /* NEW */
   /* (UBYTE *) JS event handlers */

#define AOCPY_Onimgload    (AOCPY_Dummy+36)  /* SET */
   /* (BOOL) Decoding of (static or animated) image is ready,
    * or static image has rendered completely.
    * If it is an Image() object, or if it is the first time after image
    * is displayed, the JS onLoad event is scheduled. */

#define AOCPY_Onimganim    (AOCPY_Dummy+37)  /* SET */
   /* (BOOL) Animated image has rendered its last frame.
    * If the image is displayed, the JS onLoad event is scheduled. */

#define AOCPY_Jform        (AOCPY_Dummy+38)  /* NEW */
   /* (void *) Form object to create the property with this image's name in,
    * instead of the JS parent. */

#define AOCPY_Infochanged  (AOCPY_Dummy+39)  /* SET */
   /* (BOOL) Driver encounters change of info window contents */

#define AOCPY_Deferdispose (AOCPY_Dummy+40)  /* SET */
   /* (BOOL) Dispose this copy as soon as input processing complete
    * (using queuemessages) */

#define AOCPY_Portnumber   (AOCPY_Dummy+41)  /* GET */
   /* (long) The ARexx port number of the associated window, or 0. */

#define AOCPY_Percentwidth (AOCPY_Dummy+42)  /* NEW */
#define AOCPY_Percentheight (AOCPY_Dummy+43) /* NEW */
   /* (long) Suggested dimensions (scaling) as percentage. */


#define AOCPY_Onclick (AOCPY_Dummy+44)     /* SET */
#define AOCPY_Onmouseout (AOCPY_Dummy+45)  /* SET */
#define AOCPY_Onmouseover (AOCPY_Dummy+46) /* SET */


#define AOCPY_    (AOCPY_Dummy+)
#define AOCPY_    (AOCPY_Dummy+)

/*--- copy methods ---*/

#define ACM_DUMMY          AOM_DUMMYTAG(AOTP_COPY)

#define ACM_LOAD           (ACM_DUMMY+1)
   /* Load or reload embedded objects */

/*--- copy messages ---*/

/* ACM_LOAD */
/* Sent when all objects, or maps only, should be loaded or reloaded. */
struct Acmload
{  struct Amessage amsg;
   ULONG flags;
};

#define ACMLF_MAPSONLY     0x0001   /* Load only maps, not normal objects. */
#define ACMLF_RELOAD       0x0002   /* Reload requested. */
#define ACMLF_BACKGROUND   0x0004   /* Load only backgrounds, not other objects. */
#define ACMLF_BGSOUND      0x0008   /* Load only background sound, not other objects. */
#define ACMLF_RESTRICT     0x0010   /* Restrict loaded images to same host */

/*--- copy functions ---*/

extern long Anotifycload(struct Aobject *object,ULONG flags);
   /* Builds an ACM_LOAD message and does AOM_NOTIFY */


#endif
