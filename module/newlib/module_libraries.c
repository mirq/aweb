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

   $Id: module_libraries.c,v 1.2 2009/06/15 17:05:29 opi Exp $

   Desc: Open and Close private copies of system libraries

***********************************************************************/
#if defined(__amigaos4__)

#define __USE_BASETYPE__

#include <exec/types.h>
#include <exec/execbase.h>
#include <dos/dosextens.h>
#include <proto/exec.h>

#include "module.h"

struct ExecIFace *IExec = NULL;
struct Interface *INewlib = NULL;
struct Library *newlibbase = NULL;
int OpenModuleLibraries(void)
{

    SysBase = *(struct ExecBase **)4;
    IExec=(struct ExecIFace *)((struct ExecBase *)SysBase)->MainInterface;
    if((newlibbase = IExec->OpenLibrary("newlib.library",52)))
    {
    if((INewlib = IExec->GetInterface(newlibbase, "main", 1, NULL)))
    {
       return TRUE;
    }
    else
    {
       IExec->CloseLibrary(newlibbase);
       newlibbase = NULL;
    }

    }
    return FALSE;
}

void CloseModuleLibraries()
{
    if(INewlib)IExec->DropInterface(INewlib);
    if(newlibbase)IExec->CloseLibrary(newlibbase);
}
#endif
