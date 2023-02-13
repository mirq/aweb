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

   $Id: module.h,v 1.2 2009/06/15 17:05:29 opi Exp $

   Desc: Header file file for libnix initialisation cd used by aweblibs
   and awebsupports

***********************************************************************/

#define INIT_SUCCESS    1
#define INIT_FAIL       0

#define OK 0

typedef int (*init_func_ptr)(void);
typedef void (*exit_func_ptr)(void);


extern struct DosLibrary *__DOSBase;
extern struct Library *__UtilityBase;
extern struct Library *__MathIeeeDoubTransBase;
extern struct Library *__MathIeeeDoubBasBase;
extern struct Library *__MathIeeeSingBasBase;
extern struct WBStartup *_WBenchMsg;

extern struct FunctionList __INIT_LIST__ , __EXIT_LIST__;

#include <stddef.h>

extern int      __math_init(void);
extern void     __math_exit(void);
extern int __machine_test(void);
extern int      __stdio_init(void);
extern void     __stdio_exit(void);
extern int      __stdlib_init(void);
extern void     __stdlib_exit(void);
extern int      __stk_init(void);
extern void     __stk_exit(void);
extern void     _exit(int return_code);

extern void * __malloc(size_t size,const char * file,int line);
extern void __free(void * ptr,const char * file,int line);
extern void _do_ctors(void);
extern void _do_dtors(void);


extern int  AwebModuleInit(void);
extern void AwebModuleExit(void);

extern int OpenModuleLibraries(void);
extern void CloseModuleLibraries(void);
extern void CallFunctionList(struct FunctionList *funcs, int direction);
