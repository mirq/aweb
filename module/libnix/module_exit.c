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

   $Id: module_exit.c,v 1.3 2009/06/15 17:05:26 opi Exp $

   Desc: Exit functions for libnix

***********************************************************************/

#include <stdio.h>
#include <exec/types.h>

#include "module.h"

struct FunctionList __EXIT_LIST__ = {0};

void AwebModuleExit(void)
{

    CallFunctionList(&__EXIT_LIST__, DESCENDING);
    CloseModuleLibraries();

}
