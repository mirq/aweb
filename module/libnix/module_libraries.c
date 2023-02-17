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

   $Id: module_libraries.c,v 1.3 2009/06/15 17:05:26 opi Exp $

   Desc: Open and Close private copies of system libraries

***********************************************************************/

#include <exec/types.h>
#include <exec/execbase.h>
#include <dos/dosextens.h>
#include <proto/exec.h>

#include "module.h"


struct ExecBase *SysBase = NULL;
struct DosLibrary *__DOSBase = NULL;
struct Library *__UtilityBase = NULL;
struct Library *__MathIeeeSingBasBase = NULL;
struct Library *__MathIeeeDoubTransBase = NULL;
struct Library *__MathIeeeDoubBasBase = NULL;
struct WBStartup *_WBenchMsg = NULL;

int OpenModuleLibraries(void)
{

    SysBase = *(struct ExecBase **)4;

    if(
   (__DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",39L)) &&
   (__UtilityBase = OpenLibrary("utility.library",37L)) &&
   (__MathIeeeSingBasBase = OpenLibrary("mathieeesingbas.library",33L)) &&
   (__MathIeeeDoubBasBase = OpenLibrary("mathieeedoubbas.library",33L)) &&
   (__MathIeeeDoubTransBase = OpenLibrary("mathieeedoubtrans.library",33L))
      ) return INIT_SUCCESS;

    CloseModuleLibraries();

    return INIT_FAIL;

}

void CloseModuleLibraries()
{

    if(__MathIeeeDoubTransBase)CloseLibrary(__MathIeeeDoubTransBase);
    if(__MathIeeeDoubBasBase)CloseLibrary(__MathIeeeDoubBasBase);
    if(__MathIeeeSingBasBase)CloseLibrary(__MathIeeeSingBasBase);
    if(__UtilityBase)CloseLibrary(__UtilityBase);
    if(__DOSBase)CloseLibrary((struct Library *)__DOSBase);

}
