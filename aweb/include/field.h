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

/* field.h - AWeb html general form field element object */

#ifndef AWEB_FIELD
#define AWEB_FIELD

#include "element.h"

/* field tags */

#define AOFLD_Dummy        AOBJ_DUMMYTAG(AOTP_FIELD)

#define AOFLD_Form         (AOFLD_Dummy+1)
   /* (void *) Form object this field is related to. */

#define AOFLD_Name         (AOFLD_Dummy+2)
   /* (UBYTE *) Name of this field or NULL. */

#define AOFLD_Value        (AOFLD_Dummy+3)
   /* (UBYTE *) Initial value, or GET current value. */

#define AOFLD_Reset        (AOFLD_Dummy+4)
   /* (BOOL) Reset this field to its initial state. */

#define AOFLD_Multivalue   (AOFLD_Dummy+5)
   /* (UBYTE *) GET multiple values for fields that support it.
    * The first 4 bytes of the string form a longword contining
    * the number of strings N, followed by N null-terminated strings. */

#define AOFLD_Onblur       (AOFLD_Dummy+6)
#define AOFLD_Onchange     (AOFLD_Dummy+7)
#define AOFLD_Onclick      (AOFLD_Dummy+8)
#define AOFLD_Onfocus      (AOFLD_Dummy+9)
#define AOFLD_Onselect     (AOFLD_Dummy+10)
   /* (UBYTE *) JavaScript event handlers.
    * To be implemented by the field subclass. */

#define AOFLD_    (AOFLD_Dummy+)
#define AOFLD_    (AOFLD_Dummy+)

/* Field data */

struct Field
{  struct Element elt;     /* Element superobject */
   void *win;              /* Window */
   void *form;             /* Form that this field is part of */
   UBYTE *name;            /* Name of this field */
   UBYTE *value;           /* Current value of this field */
   struct Jobject *jobject;/* JS object. AOM_JSETUP creates this object linked to its form,
                              with default readonly form,name properties. */
};


#endif
