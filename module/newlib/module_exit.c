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

   $Id: module_exit.c,v 1.2 2009/06/15 17:05:29 opi Exp $

   Desc: Exit functions for clib2

***********************************************************************/

#if defined(__amigaos4__)

#include <stdio.h>
#include <exec/types.h>

#include "module.h"

void AwebModuleExit(void)
{

    CloseModuleLibraries();
}

#endif
