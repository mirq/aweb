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

/* linkprivate.h - AWeb link and area object common definitions */

struct Link
{  struct Aobject object;
   void *frame;
   void *cframe;
   void *win;
   void *pool;
   void *url;                 /* Url pointed to */
   UBYTE *fragment;           /* Fragment part */
   LIST(Component) components;/* Objects composing this link */
   struct Buffer *text;       /* Text buffer of parent */
   UWORD flags;
   UBYTE *target;             /* Target frame name */
   UBYTE *title;              /* Title to show as prompt */
   void *popup;               /* Popup window open */
   struct Jobject *jobject;   /* JS object */
   UBYTE *onclick;            /* JS event handlers */
   UBYTE *onmouseover;
   UBYTE *onmouseout;
};

#define LNKF_SELECTED      0x0001   /* In selected state */
#define LNKF_CLIPDRAG      0x0002   /* Current component supports clipdrag */
#define LNKF_POST          0x0004   /* Use POST method */

struct Component
{  NODE(Component);
   struct Element *object;
};
