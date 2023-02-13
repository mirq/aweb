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

#ifndef AWEB_AUTHORLIB_H
#define AWEB_AUTHORLIB_H

/* authorlib.h - AWeb authorize aweblib module common */

struct Authorreq
{  struct Authorize *auth;    /* Auth details */
   UBYTE *name;               /* URL name */
   BOOL proxy;
   BOOL onlypw;
   void (*Rememberauth)(struct Authorize *);
};

struct Authnode
{  NODE(Authnode);
   struct Authorize auth;
};

struct Authedit
{  struct Aobject object;
   void *task;
   struct SignalSemaphore *authsema;
   LIST(Authnode) *auths;
   struct Library *libbase;
   UBYTE *screenname;
   struct TextFont *pwfont;
   struct Screen *screen;
   void *winobj;
   struct Window *window;
   ULONG winsigmask;
   struct Gadget *toplayout,*vlayout,*listgad,*delgad,*servergad,*useridgad,*passwordgad,*showpassgad;
   struct List gadlist;
   short x,y,w,h;
   UWORD flags;
   short lastselect;
   BOOL showpass;
};

#define AEWF_BREAKING      0x0001   /* breaking the subtask */
#define AEWF_WASOPEN       0x0002   /* Window was open when screen became invalid */

#define AOAEW_Dummy        AOBJ_DUMMYTAG(AOTP_AUTHEDIT)

/* From subtask to main task: */
#define AOAEW_Close        (AOAEW_Dummy+1)
   /* (BOOL) Window was closed. */

#define AOAEW_Dimx         (AOAEW_Dummy+2)
#define AOAEW_Dimy         (AOAEW_Dummy+3)
#define AOAEW_Dimw         (AOAEW_Dummy+4)
#define AOAEW_Dimh         (AOAEW_Dummy+5)
   /* (long) Changed window dimensions */

#define AOAEW_Authnode     (AOAEW_Dummy+6)
   /* (struct Authnode *) node to change or delete */

#define AOAEW_Delete       (AOAEW_Dummy+7)
   /* (BOOL) Delete the node */

#define AOAEW_Change       (AOAEW_Dummy+8)
   /* (struct Authorize) Change the node's cookie to this one's */

#define AOAEW_Domasterpw   (AOAEW_Dummy+9)
   /* (struct Authorize) Get the master password and/or test it */

/* From main task to subtask: */
#define AOAEW_Tofront      (AOAEW_Dummy+101)
   /* (BOOL) Push window to front */

#define AOAEW_Modified     (AOAEW_Dummy+102)
   /* (BOOL) The list is modified */

#define AOAEW_Showpass     (AOAEW_Dummy+103)
   /* (BOOL) The passwords should be displayed */

#endif /* AUTHORLIB_H */
