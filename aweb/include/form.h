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

/* form.h - AWeb html form object */

#ifndef AWEB_FORM
#define AWEB_FORM

#include "object.h"

/*--- form tags ---*/

#define AOFOR_Dummy        AOBJ_DUMMYTAG(AOTP_FORM)

#define AOFOR_Method       (AOFOR_Dummy+1)
   /* (UWORD) Action, see below */

#define AOFOR_Action       (AOFOR_Dummy+2)
   /* (void *) The action Url */

#define AOFOR_Complete     (AOFOR_Dummy+3)
   /* (BOOL) SET when definition is complete */

#define AOFOR_Target       (AOFOR_Dummy+4)
   /* (UBYTE *) Target frame name */

#define AOFOR_Name         (AOFOR_Dummy+5)
   /* (UBYTE *) Form's name (js) */

#define AOFOR_Jelements    (AOFOR_Dummy+6)   /* GET */
   /* (struct Jobject *) JS "elements" array object */

#define AOFOR_Onreset      (AOFOR_Dummy+7)   /* NEW,SET */
#define AOFOR_Onsubmit     (AOFOR_Dummy+8)   /* NEW,SET */
   /* (UBYTE *) JS event handlers */

#define AOFOR_Multipart    (AOFOR_Dummy+9)   /* NEW */
   /* (BOOL) If the encoding type is miltipart/form-data instead of
    * application/x-www-form-urlencoded. */

#define AOFOR_Radioname    (AOFOR_Dummy+10)  /* GET */
#define AOFOR_Radioselected (AOFOR_Dummy+11) /* GET */
   /* (Aobject *) Supply a (UBYTE *) as Radioname, and get
    * the currently selected AOTP_RADIO object or NULL. */

#define AOFOR_Charset      (AOFOR_Dummy+12)  /* NEW */
   /* (UBYTE *) character set in which we should post data */

#define AOFOR_    (AOFOR_Dummy+)
#define AOFOR_    (AOFOR_Dummy+)

/* Form HTTP methods */
#define FORMTH_GET   0
#define FORMTH_POST  1
#define FORMTH_INDEX 2

/*--- form methods ---*/

#define AFO_DUMMY          AOM_DUMMYTAG(AOTP_FORM)

#define AFO_SUBMIT         (AFO_DUMMY+1)
   /* Submit this form */

#define AFO_RESET          (AFO_DUMMY+2)
   /* Reset this form */

#define AFO_RADIOSELECT    (AFO_DUMMY+3)
   /* This radiobutton is selected, unselect others in group */

#define AFO_NEXTINPUT      (AFO_DUMMY+4)
   /* Find the next or previous input field in the form */

/*--- messages ---*/

/* AFO_SUBMIT */
/* Generate the form request. If (submit) is a AOTP_INPUT field, only submit
 * if it is the only input field in the form. */
struct Afosubmit
{  struct Amessage amsg;
   struct Aobject *submit;    /* Field that causes submit */
   long x,y;                  /* X and Y coordinates for image fields */
};

/* AFO_RESET */
/* All fields should be reset to their initial values.
 * Implemented by setting AOFLD_Reset for all fields in this form */
/* AFO_RESET uses a standard Amessage structure */

/* AFO_RADIOSELECT */
/* A radiobutton was selected. Unselect other buttons of the same name */
struct Aforadioselect
{  struct Amessage amsg;
   struct Aobject *select;    /* The selected field */
   UBYTE *name;               /* Name of this radiobutton */
};

/* AFO_NEXTINPUT */
/* Find the next (or previous if direction=-1) AOTP_INPUT field in this
 * form. Return value is a pointer to the new object */
struct Afonextinput
{  struct Amessage amsg;
   struct Aobject *current;   /* Starting point */
   long direction;            /* +1 or -1 */
};

/*--- multipart data ---*/

struct Multipartdata
{  struct Buffer buf;         /* Holds the fields. First string in buffer is the boundary. */
   LIST(Multipartpart) parts; /* List of parts making up the entire message */
   long length;               /* Total content length */
};

/* One Multipartpart holds a fragment of the total message. It may contain
 * more than one field. All parts together form the entire message body,
 * including boundaries.
 * If (start) is zero, the file pointed to by the lock should be sent */
struct Multipartpart
{  NODE(Multipartpart);
   long start;                /* Starting point in buffer, or zero for file */
   long length;               /* Text length in buffer, or length of file */
   long lock;                 /* Lock on file to inlude. When the lock has become
                               * invalid, make it NULL! */
};

extern void Freemultipartdata(struct Multipartdata *mpd);
   /* Dispose the multipartdata and all related structures. */

#endif
