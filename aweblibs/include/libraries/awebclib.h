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

   $Id: awebclib.h,v 1.4 2009/06/15 17:04:49 opi Exp $

   Desc: Header for some missing functions in os4 clib2

***********************************************************************/

/* This next assumes CLIB2 is used for os4 and libnix otherwise */
/* Extra clib functions from */

#if defined (__amigaos4__)

void srand48(long seed);
double drand48(void);
void timer(long clock[2]);


int VARARGS68K adebug(UBYTE *fmt, ...);
int VARARGS68K araddebug(UBYTE *fmt, ...);

#endif
