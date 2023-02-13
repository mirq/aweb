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

/* awebtcp.h - prototypes and pragmas for AWeb tcp and ssl switch libraries */

#ifndef AWEBTCP_H
#define AWEBTCP_H

#include <proto/awebtcp.h>
#include <proto/awebssl.h>

struct Library;
struct Assl;

/* defined in awebtcp.c */

/* Try to open TCP library. */
extern struct Library *Tcpopenlib(void);

/* Try to open SSL library. */
extern struct Assl *Tcpopenssl(struct Library *socketbase);

#endif
