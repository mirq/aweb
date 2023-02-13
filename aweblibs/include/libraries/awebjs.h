#ifndef LIBRARIES_AWEBJS_H
#define LIBRARIES_AWEBJS_H

/**********************************************************************

   This file is part of the AWeb-II distribution

   Copyright (C) 2002 Yvon Rozijn
   Copyright (C) 2002-2003 The AWeb Developement Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the AWeb Public License as included in this
   distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   AWeb Public License for more details.

   $Id: awebjs.h,v 1.3 2009/06/15 17:04:49 opi Exp $

   Desc: AWeb js aweblib module interface data structures and defines

***********************************************************************/

/* Object types:
 *  Jcontext            = general Jscript context
 *  Jobject             = an object, with methods and properties
 *  Jvar                = a Jscript variable value, can contain several types
 */

struct Jcontext;
struct Jobject;
struct Jvar;

/* Runtime error values */
#define JERRORS_CONTINUE   -1 /* Don't show errors and try to continue script */
#define JERRORS_OFF        0  /* Don't show errors and stop script */
#define JERRORS_ON         1  /* Show errors and stop script */

typedef BOOL Jfeedback(struct Jcontext *jc);

struct Objhookdata
{  struct Jcontext *jc;       /* Execution context */
   UWORD code;               /* Function code */
   struct Jobject *jo;        /* Object affected */
   UBYTE *name;               /* Name of property to add */
};
#define OHC_ADDPROPERTY 1  /* Add a property */

typedef BOOL Objhookfunc(struct Objhookdata *data);

/* Hook to call when object is disposed. */
typedef void Objdisposehookfunc(void *internal);

struct Varhookdata
{  struct Jcontext *jc;       /* Execution context */
   UWORD code;               /* Function code VHC_xxx */
   struct Jvar *var;          /* Variable affected (that has this hook defined) */
   void *hookdata;            /* Private data for hook */
   struct Jvar *value;        /* Value to set variable to (VHC_SET) or get into (VHC_GET) */
   UBYTE *name;               /* Name of variable to get or set */
};
#define VHC_SET         1  /* Set variable to this value */
#define VHC_GET         2  /* Get variable value */

typedef BOOL Varhookfunc(struct Varhookdata *data);

/* Special values for Varhookfunc: */

   /* Use as property hook to make property read-only. */
   #define JPROPHOOK_READONLY    (Varhookfunc *)-1

   /* This is a synonym, use the variable passed as (hookdata) instead. */
   #define JPROPHOOK_SYNONYM     (Varhookfunc *)-2

#endif
