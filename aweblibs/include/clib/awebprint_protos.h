#ifndef CLIB_AWEBPRINT_PROTOS_H
#define CLIB_AWEBPRINT_PROTOS_H


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

   $Id: awebprint_protos.h,v 1.3 2009/06/15 17:04:08 opi Exp $

   Desc:

***********************************************************************/

#include <exec/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void Printdebugprefs(struct Print * prt);
void Printfinddimensions(struct Print * prt);
void Printprintsection(struct Print * prt);
void Printclosedebug(struct Print * prt);

#ifdef __cplusplus
}
#endif

#endif   /*  CLIB_AWEBPRINT_PROTOS_H  */
