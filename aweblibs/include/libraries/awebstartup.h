#ifndef LIBRARIES_AWEBSTARTUP_H
#define LIBRARIES_AWEBSTARTUP_H

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

   $Id: awebstartup.h,v 1.3 2009/06/15 17:04:49 opi Exp $

   Desc: AWeb startup window module

***********************************************************************/

/* States for Startupstate() */
enum LOADREQ_STATES
{
    LRQ_FONTS,
    LRQ_IMAGES,
    LRQ_CACHE
};

#endif
