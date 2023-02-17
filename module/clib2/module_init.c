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

   $Id: module_init.c,v 1.5 2009/06/15 17:05:23 opi Exp $

   Desc: Initialisations for clib2

***********************************************************************/

#if defined(__amigaos4__)

#include <stdio.h>
#include <dos.h>

#include "module.h"


#include <exec/types.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

//void Debug(STRPTR *str)
//{
//    struct ExecIFace *myExec = (struct ExecIFace*)(*(struct ExecBase **)4)->MainInterface;
//    struct Library *myDOSBase = myExec->OpenLibrary("dos.library",0);
//    struct DOSIFace *myDOS = myExec->GetInterface(myDOSBase,"main",1,0);
//    BPTR debug = myDOS->Open("ram:debug",MODE_OLDFILE);
//    myDOS->Seek(debug,0,OFFSET_END);
//    myDOS->Write(debug,str,10);
//    myDOS->Close(debug);
//    myExec->DropInterface(myDOS);
//    myExec->CloseLibrary(myDOSBase);
//
//}

BOOL __check_abort_enabled = FALSE;


int AwebModuleInit(void)
{

    if(!OpenModuleLibraries())
    {

        CloseModuleLibraries();
        return INIT_FAIL;

    }

    __lib_init(SysBase);

    return INIT_SUCCESS;
}


/*
void exit()
{

}
*/

#endif
