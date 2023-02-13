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

/* aweblib.h - AWeb AWebLib module general include file */

#ifndef AWEBLIB_H
#define AWEBLIB_H

#define NOPROTOTYPES

#ifndef AWEBDEF_H
#include "awebdef.h"
#endif

#undef NO_INLINE_STDARG
#include "proto/awebsupport.h"
#define NO_INLINE_STDARG

#define AWEBSTR(n)   Awebstr(n)

#endif
