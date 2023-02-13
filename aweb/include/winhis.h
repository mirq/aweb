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

/* winhis.h - AWeb window history object */

#ifndef AWEB_WINHIS
#define AWEB_WINHIS

#include "object.h"

/* winhis tags */

#define AOWHS_Dummy        AOBJ_DUMMYTAG(AOTP_WINHIS)

#define AOWHS_Key          (AOWHS_Dummy+1)
   /* (ULONG) Window key to identify the corresponding window */

#define AOWHS_Windownr     (AOWHS_Dummy+2)
   /* (long) Window number as the user knows it */

#define AOWHS_Copyfrom     (AOWHS_Dummy+3)
   /* (void *) Another WINHIS object that must be copied, but
    * all info for the specified frame must be replaced. If
    * the URL for that frame is different from the current URL,
    * all info for the frame's children must be removed from
    * the copy. This way, if the URL is the same (e.g. only
    * fragment is different), the children Iframes will keep
    * their current contents.
    * If no frameid specified, everything will be copied. */

#define AOWHS_Frameid      (AOWHS_Dummy+4)
   /* (UBYTE *) Unique frame identifier in document hierarchy.
    * It consists of n bytes, one for each hierarchy level.
    * Every frame adds its sequence number to the id of its
    * owner frame. Top level frame has id NULL.
    * Use this as a qualifier before setting or getting one of
    * the frame-specific tags. */

#define AOWHS_Url          (AOWHS_Dummy+5)
   /* (struct Url *) Url displayed in this frame */

#define AOWHS_Fragment     (AOWHS_Dummy+6)
   /* (UBYTE *) Fragment to be displayed in this frame */

#define AOWHS_Leftpos      (AOWHS_Dummy+7)
#define AOWHS_Toppos       (AOWHS_Dummy+8)
   /* (long) Current scroll position for this frame */

#define AOWHS_Previous     (AOWHS_Dummy+9)
#define AOWHS_Next         (AOWHS_Dummy+10)
   /* (void *) GET previous and next WINHIS objects relative to this one
    * for the same window, and respecting the mainline hierarchy. */

#define AOWHS_Skipfrom     (AOWHS_Dummy+11)
   /* (void *) SET. User skipped from the winhis passed in this tag
    * to the addressed winhis. */

#define AOWHS_Title        (AOWHS_Dummy+12)
   /* (UBYTE *) If the specified frame id is the leading one for this winhis,
    * set the title to be used in the history window */

#define AOWHS_Loadnr       (AOWHS_Dummy+13)
   /* (ULONG) Sequence number of loading of object in this frame. */

#define AOWHS_Break        (AOWHS_Dummy+14)
   /* (BOOL) Break leading frame's input */

#define AOWHS_History      (AOWHS_Dummy+15)
   /* (BOOL) This is a historic winhis, not a new retrieve */

#define AOWHS_Isleading    (AOWHS_Dummy+16)  /* GET */
   /* (BOOL) If this frameid is the leading frame for this window
    * (the one that changed) */

#define AOWHS_Clearbelow   (AOWHS_Dummy+17)  /* SET */
   /* (BOOL) Clear all frames below this one. */

#define AOWHS_Commands     (AOWHS_Dummy+18)  /* SET,GET */
   /* (BOOL) Shell/ARexx commands allowed. */

#define AOWHS_Previnframe  (AOWHS_Dummy+19)  /* GET */
   /* (void *) Get the previous winhis object relative to this one for
    * the supplied leading frameid or one of its parents.
    * NULL is returned if no previous exists, or the starting winhis is
    * not for the given frameid.
    * Get this attribute repeatedly until it returns NULL and you've got
    * all older history objects for this frame including the initial. */

#define AOWHS_Nextinframe  (AOWHS_Dummy+20)  /* GET */
   /* (void *) Get the next winhis object relative to this one
    * with the same leading frameid as supplied, or NULL. */

#define AOWHS_Leadingwinhis (AOWHS_Dummy+21) /* GET */
   /* (void *) Get the latest winhis object upto (including) this one
    * in which the supplied frameid or a parent of this id is the leading frameid */

#define AOWHS_Input        (AOWHS_Dummy+22)  /* GET */
   /* (BOOL) If any URL in this winhis is inputting. */

#define AOWHS_Cancel       (AOWHS_Dummy+23)  /* SET */
   /* (BOOL) Cancel fetches for all URLs inthis winhis. */

#define AOWHS_    (AOWHS_Dummy+)
#define AOWHS_    (AOWHS_Dummy+)

#endif
