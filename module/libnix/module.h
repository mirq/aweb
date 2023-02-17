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

   $Id: module.h,v 1.4 2009/06/15 17:05:26 opi Exp $

   Desc: Header file file for libnix initialisation cd used by aweblibs
   and awebsupports

***********************************************************************/

#define INIT_SUCCESS   1
#define INIT_FAIL   0

#define ASCENDING   1
#define DESCENDING   0


struct   FunctionListItem
{
    void (*fli_Func)(void);
    int   fli_Pri;
};

struct FunctionList
{
    int     fl_Size;
    struct  FunctionListItem  fl_Funcs[0];
};

extern struct DosLibrary *__DOSBase;
extern struct Library *__UtilityBase;
extern struct Library *__MathIeeeDoubTransBase;
extern struct Library *__MathIeeeDoubBasBase;
extern struct Library *__MathIeeeSingBasBase;
extern struct WBStartup *_WBenchMsg;

extern struct FunctionList __INIT_LIST__ , __EXIT_LIST__;


extern int  AwebModuleInit(void);
extern void AwebModuleExit(void);

extern int OpenModuleLibraries(void);
extern void CloseModuleLibraries(void);
extern void CallFunctionList(struct FunctionList *funcs, int direction);
