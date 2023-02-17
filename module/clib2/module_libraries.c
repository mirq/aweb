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

   $Id: module_libraries.c,v 1.5 2009/06/15 17:05:24 opi Exp $

   Desc: Open and Close private copies of system libraries

***********************************************************************/
#if defined(__amigaos4__)

#define __USE_BASETYPE__

#include <exec/types.h>
#include <exec/execbase.h>
#include <dos/dosextens.h>
#include <proto/exec.h>

#include "module.h"

int OpenModuleLibraries(void)
{

    SysBase = *(struct ExecBase **)4;
#if defined(__amigaos4__)
    IExec=(struct ExecIFace *)((struct ExecBase *)SysBase)->MainInterface;
#endif
}

void CloseModuleLibraries()
{

}
#endif
