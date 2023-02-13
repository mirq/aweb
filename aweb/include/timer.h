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

/* timer.h - AWeb timer object */

#ifndef AWEB_TIMER_H
#define AWEB_TIMER_H

#include "object.h"

/*--- timer tags ---*/

#define AOTIM_Dummy        AOBJ_DUMMYTAG(AOTP_TIMER)

#define AOTIM_Waitseconds  (AOTIM_Dummy+1)   /* NEW,SET,GET */
#define AOTIM_Waitmicros   (AOTIM_Dummy+2)   /* NEW,SET,GET */
   /* (long) Time to wait before an AOM_UPDATE will be sent.
    * When GET, the time remaining is returned. */

#define AOTIM_Seconds      (AOTIM_Dummy+3)   /* GET */
#define AOTIM_Micros       (AOTIM_Dummy+4)   /* GET */
   /* (long) Current system time */

#define AOTIM_Ready        (AOTIM_Dummy+5)   /* UPDATE */
   /* (BOOL) Wait time is over */

#define AOTIM_    (AOTIM_Dummy+)
#define AOTIM_    (AOTIM_Dummy+)


#endif
