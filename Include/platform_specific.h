/**********************************************************************


   This file is part of the AWeb-II distribution

   Copyright (C) 2002 Yvon Rozijn
   Copyright (c) 2002 The AWeb Developement Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the AWeb Public License as included in this
   distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   AWeb Public License for more details.

   $Id: platform_specific.h,v 1.12 2009/06/15 17:03:02 opi Exp $

   Desc: Collection of macros which constitute part of the platform isolation
         layer.

***********************************************************************/

/*
    This file contains a bunch of macros which serve to handle functions
    with registerized parameters in the Amiga fashion even on non Amiga
    architectures and with different kind of compilers, providing one single
    and simple interface for the programmer.

    The macros are of the form

        USRFUNC_Xn
        (
            rtype, name,           // Return type and Name of the function
            type1, name1, reg1     // Respectively type, name and register of the first parameter
            ...
            typen, namen, regn     // Respectively type, name and register of the n-th parameter
        )

        in "USRFUNC_Xn", "X" can either be

            "P", for function prototypes

        or

            "H", for function headers

        and "n" has to be replaced by the number of arguments that the function accepts.

        Registers have to be expressed in capital letters and without quotes, in this way:

            A0, A1, A2,... An and D0, D1, D2, ..., Dn

        The macros will take care of converting their arguments in a proper source code suited
        for the platform in use.

        Example of usage: say the function "foo" accepts 3 arguments, which have to be stored in
        A0, D0 and D1, and say the function is static and returns an integeg;

        This is the code for the prototype:

            USRFUNC_P3
            (
                static int, foo,
                int *, bar,   A0,
                int,   baz,   D0,
                long,  dummy, D1
            );

         The code for the function itself is almost equal, except for the fact that "P" is
         substituted with "H", and two new macros are used:

            USRFUNC_H3
            (
                static int, foo,
                int *, bar,   A0,
                int,   baz,   D0,
                long,  dummy, D1
            )
            {
                USRFUNC_INIT

                int res = 0;

                ...
                ...
                ...

                return res;

                USRFUNC_EXIT
            }

        Two new macros are introduced:

            USRFUNC_INIT - Always put this at the top of the function body,
                           before any code

            USRFUNC_EXIT - Always put this at the end of the function body,
                           after any code

        Those above macros are ALWAYS to be used when writing a function with registerized
        paramenters. Don't forget the USRFUNC_#? macros!

       NB for libraries, aweblibs, and plugins the LIBFUNC_xx macros should be used
       For each library / plugin two additional macros need to be defined for each
       OS / environment.  These are defined in the respective library_types_xx.h file.

       xxxx_TYPE the type of the library arg, eg struct Library * for os3.x or
                 struct Interface * for OS4 +
       xxxx_NAME the name of the library arg, often Self...
       A LIBFUNC_xx macro is then called thus


       LIBFUNC_P1
       (
            rtype, fname,
            type, arg1 , reg1,
            LIBB_TYPE, LIBB_NAME
       )

       VARARGS
       Varargs functions are handled differenty by differing variations of
       compliler/processor/os, for correct handling on all build environents
       the following mcros should be used. (The old VARARGS macro defined in
       awebdefs.h is now depricated, [and will shortly be removed] it will only
       work on classic m68k amigas)


       the first group are for standard va_list code and are used much as the lower
       case equivilents in stdarg.h

       VA_LIST argv          - defines a variable (argv) of type va_list (or equiv)
       VA_START(argv, last)  - initialise a va_list
       VA_ARG(argv, type)    - get the next arg of type type in the list
       VA_END(argv)          - cleanup va_list

       the second set are for situations when you need the va_list to be a 68k style
       stack based list, usually for passing to old code (and some  OS functions)
       that expects this. additional code that requires varags to be passes this way
       must be declared VARARGS68K (for 0s4.0 compatabilty)

       VA_STARTLIN(argv, last) - init va_list 68k style
       VA_GETLIN(argv, type)   - get the first(?) arg in the va_list



*/


#ifndef PLATFORM_SPECIFIC_H
#define PLATFORM_SPECIFIC_H

#if !defined(__AMIGADATE__)
#    define __AMIGADATE__ __DATE__
#endif

#if !defined(__AROS__)

#include <exec/types.h>

#if !defined(__MORPHOS__)
typedef ULONG IPTR;
#endif

#if defined(__GNUC__) && defined(__amigaos__)

#if defined(__amigaos4__)

#     define __saveds
#     define __aligned
#     define __stdargs

// for awebtcp

#     define u_long ULONG

#     define ASMARG(type, name, reg) type name
#     define __ASMFPREFIX

//    No need to redefineM ASM_## macros as we don`t use them for OS4

//    These setup using os3.x style function calls (ie no Interface->)

#     define __USE_INLINE__
#     define __USE_BASETYPE__

#define LIBSTART_DUMMY void _start (void) {}

#include "library_types_os4.h"

//    Functions that have changed between os3.x and os4.0

#define DoMethod IDoMethod
#define DoMethodA IDoMethodA
#define DoSuperMethod IDoSuperMethod
#define DoSuperMethodA IDoSuperMethodA
#define CoerceMethod ICoerceMethod
#define CoerceMethodA ICoercrMethodA
#define SetSuperAttrs ISetSuperAttrs
#define SetSuperAttrsA ISetSuperAttrsA

#define CreateExtIO(a,b) CreateIORequest((struct MsgPOrt *)(a),(ULONG)(b))
#define DeleteExtIO(a) DeleteIORequest((struct IORequest *)(a))




//    VA_LIST MACROS

#define VARARGS68K_PROTO(func)   VARARGS68K func
#define VARARGS68K_DECLARE(func) VARARGS68K func

 
#define VA_LIST          va_list
#define VA_START(a,b)    va_start(a,b)
#define VA_ARG(a,b)      va_arg(a,b)
#define VA_END(a)        va_end(a)

#define VA_STARTLIN(a,b) va_startlinear(a,b)
#define VA_GETLIN(a,b)   va_getlinearva(a,b)

#ifndef CPU
#define CPU "OS4.0 PPC"
#endif


#define GIVEME_HOOK(name) name
#define GIVEME_HOOKENTRY(name)  name

#define DECLARE_HOOK(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3) \
        USRFUNC_H3(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3)

#define DECLARE_DISPATCH(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3) \
        USRFUNC_H3(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3)


/*******************/
/*******************/
/*******************/
/*******************/
/*******************/
/* MorphOS Section */
/*******************/
/*******************/
/*******************/
/*******************/
/*******************/
/*******************/
#elif defined(__MORPHOS__)

#define NO_LIST_MACROS

#include <proto/exec.h>


#ifndef SDTA_Cycles
#define SDTA_Cycles (TAG_USER + 0x1000 + 506)
#endif

#include "M_Hook.h"
#include "declgate.h"

#define GIVEME_HOOKENTRY(name)  (APTR) (&name)
#define GIVEME_HOOK(name)      (APTR) (&hook_##name)

#define DECLARE_HOOK(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3) \
        M_HOOK(func_name, type2 name2, type3 name3)

#define DECLARE_DISPATCH(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3) \
        MUI_DISPATCH(func_name)


#undef __saveds
#undef __aligned

#define __saveds
#define __aligned
#define __stdargs

// for awebtcp

//#     define u_long ULONG

#     define ASMARG(type, name, reg) type name
#     define __ASMFPREFIX

//#                     define USE_INLINE_STDARG

#define LIBSTART_DUMMY void _start (void) {}

#include "library_types_morphos.h"

//    VA_LIST MACROS

#define VARARGS68K                __FIXME__
#define VARARGS68K_PROTO(func)   func  __attribute__((varargs68k))
#define VARARGS68K_DECLARE(func) VARARGS68K_PROTO(func); func

#define VA_LIST          va_list
#define VA_START(a,b)    va_start(a,b)
#define VA_ARG(a,b)      va_arg(a,b)
#define VA_END(a)        va_end(a)

#define VA_STARTLIN(a,b) va_start(a,b)
#define VA_GETLIN(a,b)   ( (b) (a)->overflow_arg_area )

#ifndef CPU
#define CPU "MorphOS/PowerPC"
#endif

// Move LIBFUNC_xx to declgate macros
#define LIBFUNC_H0(type, name, libtype, libname) type name##(void) { DECLARG_1(a6, libtype, libname)
#define LIBFUNC_H1(type, name, t1, n1, r1, libtype, libname) type name##(void) { DECLARG_2(r1, t1, n1, a6, libtype, libname)
#define LIBFUNC_H2(type, name, t1, n1, r1, t2, n2, r2, libtype, libname) type name##(void) { DECLARG_3(r1, t1, n1, r2, t2, n2, a6, libtype, libname)
#define LIBFUNC_H3(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, libtype, libname) type name##(void) { DECLARG_4(r1, t1, n1, r2, t2, n2, r3, t3, n3, a6, libtype, libname)
#define LIBFUNC_H4(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, libtype, libname) type name##(void) { DECLARG_5(r1, t1, n1, r2, t2, n2, r3, t3, n3, r4, t4, n4, a6, libtype, libname)
#define LIBFUNC_H5(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, t5, n5, r5, libtype, libname) type name##(void) { DECLARG_6(r1, t1, n1, r2, t2, n2, r3, t3, n3, r4, t4, n4, r5, t5, n5, a6, libtype, libname)
#define LIBFUNC_H6(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, t5, n5, r5, t6, n6, r6,libtype, libname) type name##(void) { DECLARG_7(r1, t1, n1, r2, t2, n2, r3, t3, n3, r4, t4, n4, r5, t5, n5, r6, t6, n6, a6, libtype, libname)
#define LIBFUNC_H7(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, t5, n5, r5, t6, n6, r6, t7, n7, r7, libtype, libname) type name##(void) \
    { DECLARG_8(r1, t1, n1, r2, t2, n2, r3, t3, n3, r4, t4, n4, r5, t5, n5, r6, t6, n6, r7, t7, n7, a6, libtype, libname)
#define LIBFUNC_H8(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, t5, n5, r5, t6, n6, r6, t7, n7, r7, t8, n8, r8, libtype, libname) type name##(void) \
    { DECLARG_9(r1, t1, n1, r2, t2, n2, r3, t3, n3, r4, t4, n4, r5, t5, n5, r6, t6, n6, r7, t7, n7, r8, t8, n8, a6, libtype, libname)


#define LIBFUNC_P0(type, name, libtype, libname) type name##(void)
#define LIBFUNC_P1(type, name, t1, n1, r1, libtype, libname) type name##(void)
#define LIBFUNC_P2(type, name, t1, n1, r1, t2, n2, r2, libtype, libname) type name##(void)
#define LIBFUNC_P3(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, libtype, libname) type name##(void)
#define LIBFUNC_P4(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, libtype, libname) type name##(void)
#define LIBFUNC_P5(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, t5, n5, r5, libtype, libname) type name##(void)
#define LIBFUNC_P6(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, t5, n5, r5, t6, n6, r6,libtype, libname) type name##(void)
#define LIBFUNC_P7(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, t5, n5, r5, t6, n6, r6, t7, n7, r7, libtype, libname) type name##(void)
#define LIBFUNC_P8(type, name, t1, n1, r1, t2, n2, r2, t3, n3, r3, t4, n4, r4, t5, n5, r5, t6, n6, r6, t7, n7, r7, t8, n8, r8, libtype, libname) type name##(void)

#define LIBFUNC_INIT
#define LIBFUNC_EXIT    }

#define USRFUNC_INIT
#define USRFUNC_EXIT

#elif defined(__amigaos__)

/*******************/
/*******************/
/*******************/
/*******************/
/*******************/
/* AmigaOS Section */
/*******************/
/*******************/
/*******************/
/*******************/
/*******************/
/*******************/

#     define NO_INLINE_STDARG
#     define VARARGS68K

#     define ASM_A0 __asm("a0")
#     define ASM_A1 __asm("a1")
#     define ASM_A2 __asm("a2")
#     define ASM_A3 __asm("a3")
#     define ASM_A4 __asm("a4")
#     define ASM_A5 __asm("a5")
#     define ASM_A6 __asm("a6")
#     define ASM_A7 __asm("a7")

#     define ASM_D0 __asm("d0")
#     define ASM_D1 __asm("d1")
#     define ASM_D2 __asm("d2")
#     define ASM_D3 __asm("d3")
#     define ASM_D4 __asm("d4")
#     define ASM_D5 __asm("d5")
#     define ASM_D6 __asm("d6")
#     define ASM_D7 __asm("d7")

#     define ASMARG(type, name, reg) register type name ASM_##reg
#     define __ASMFPREFIX

#ifndef CPU

#ifdef mc68060
#define CPU "mc68060"
#endif

#ifdef mc68040
#define CPU "mc68040"
#endif

#ifdef mc68030
#define CPU "mc68030"
#endif

#ifdef mc68020
#define CPU "mc68020"
#endif

#ifndef CPU

#ifdef mc68000
#define CPU "mc68000"
#endif

#endif // CPU

#endif // CPU

#define LIBSTART_DUMMY USRFUNC_H0(LONG __saveds, Libstart) { USRFUNC_INIT return -1; USRFUNC_EXIT }

#include "library_types_os3.h"

// functions that have changed between os 3 and os 4

#define DOMETHOD DoMethod

//    VA_LIST MACROS

#define VARARGS68K_PROTO(func)   func
#define VARARGS68K_DECLARE(func) func

#define VA_LIST       va_list
#define VA_START(a,b) va_start(a,b)
#define VA_ARG(a,b)   va_arg(a,b)
#define VA_END(a)     va_end(a)

#define VA_STARTLIN(a,b) va_start(a,b)
#define VA_GETLIN(a,b)   a


#define GIVEME_HOOK(name)      name
#define GIVEME_HOOKENTRY(name) name

#define DECLARE_HOOK(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3) \
        USRFUNC_H3(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3)

#define DECLARE_DISPATCH(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3) \
        USRFUNC_H3(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3)

#endif // amigaos

#elif defined(__SASC)

#     define ASM_A0 __a0
#     define ASM_A1 __a1
#     define ASM_A2 __a2
#     define ASM_A3 __a3
#     define ASM_A4 __a4
#     define ASM_A5 __a5
#     define ASM_A6 __a6
#     define ASM_A7 __a7

#     define ASM_D0 __d0
#     define ASM_D1 __d1
#     define ASM_D2 __d2
#     define ASM_D3 __d3
#     define ASM_D4 __d4
#     define ASM_D5 __d5
#     define ASM_D6 __d6
#     define ASM_D7 __d7

#     define ASMARG(type, name, reg) register ASM_##reg type name
#     define __ASMFPREFIX __asm

#else

#    error Your compiler and/or operating system are/is not supported.

#endif

#ifndef __MORPHOS__

#    define USRFUNC_INIT
#    define USRFUNC_EXIT


#    define LIBFUNC_INIT
#    define LIBFUNC_EXIT

#define NATLIBFUNC_H1(ftype, fname, t1, n1, r1, libtype, libname) \
        USRFUNC_H1(ftype, fname, t1, a1, r1)

#define NATLIBFUNC_P1(type, name, t1, n1, r1, libtype, libname) \
        USRFUNC_P1(ftype, fname, t1, a1, r1)

#endif

#    define USRFUNC_P0(rtype, fname) \
     __ASMFPREFIX rtype fname() \

#    define USRFUNC_P1(rtype, fname, a1t, a1n, a1r) \
     __ASMFPREFIX rtype fname   \
     (                          \
         ASMARG(a1t, a1n, a1r)  \
     )

#    define USRFUNC_P2(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r) \
     __ASMFPREFIX rtype fname   \
     (                          \
         ASMARG(a1t, a1n, a1r), \
         ASMARG(a2t, a2n, a2r)  \
     )

#    define USRFUNC_P3(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r) \
     __ASMFPREFIX rtype fname   \
     (                          \
         ASMARG(a1t, a1n, a1r), \
         ASMARG(a2t, a2n, a2r), \
         ASMARG(a3t, a3n, a3r)  \
     )

#    define USRFUNC_P4(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r) \
     __ASMFPREFIX rtype fname   \
     (                          \
         ASMARG(a1t, a1n, a1r), \
         ASMARG(a2t, a2n, a2r), \
         ASMARG(a3t, a3n, a3r), \
         ASMARG(a4t, a4n, a4r)  \
     )

#    define USRFUNC_P5(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r) \
     __ASMFPREFIX rtype fname   \
     (                          \
         ASMARG(a1t, a1n, a1r), \
         ASMARG(a2t, a2n, a2r), \
         ASMARG(a3t, a3n, a3r), \
         ASMARG(a4t, a4n, a4r), \
         ASMARG(a5t, a5n, a5r)  \
     )

#    define USRFUNC_P6(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                                     a6t, a6n, a6r) \
     __ASMFPREFIX rtype fname   \
     (                          \
         ASMARG(a1t, a1n, a1r), \
         ASMARG(a2t, a2n, a2r), \
         ASMARG(a3t, a3n, a3r), \
         ASMARG(a4t, a4n, a4r), \
         ASMARG(a5t, a5n, a5r), \
         ASMARG(a6t, a6n, a6r)  \
     )

#    define USRFUNC_P7(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                                     a6t, a6n, a6r, a7t, a7n, a7r) \
     __ASMFPREFIX rtype fname   \
     (                          \
         ASMARG(a1t, a1n, a1r), \
         ASMARG(a2t, a2n, a2r), \
         ASMARG(a3t, a3n, a3r), \
         ASMARG(a4t, a4n, a4r), \
         ASMARG(a5t, a5n, a5r), \
         ASMARG(a6t, a6n, a6r), \
         ASMARG(a7t, a7n, a7r)  \
     )

#    define USRFUNC_P8(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                                     a6t, a6n, a6r, a7t, a7n, a7r, a8t, a8n, a8r) \
     __ASMFPREFIX rtype fname   \
     (                          \
         ASMARG(a1t, a1n, a1r), \
         ASMARG(a2t, a2n, a2r), \
         ASMARG(a3t, a3n, a3r), \
         ASMARG(a4t, a4n, a4r), \
         ASMARG(a5t, a5n, a5r), \
         ASMARG(a6t, a6n, a6r), \
         ASMARG(a7t, a7n, a7r), \
         ASMARG(a8t, a8n, a8r)  \
     )

#    define USRFUNC_P9(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                                     a6t, a6n, a6r, a7t, a7n, a7r, a8t, a8n, a8r, a9t, a9n, a9r) \
     __ASMFPREFIX rtype fname   \
     (                          \
         ASMARG(a1t, a1n, a1r), \
         ASMARG(a2t, a2n, a2r), \
         ASMARG(a3t, a3n, a3r), \
         ASMARG(a4t, a4n, a4r), \
         ASMARG(a5t, a5n, a5r), \
         ASMARG(a6t, a6n, a6r), \
         ASMARG(a7t, a7n, a7r), \
         ASMARG(a8t, a8n, a8r), \
         ASMARG(a9t, a9n, a9r)  \
     )


#    define USRFUNC_H0(rtype, fname) \
     USRFUNC_P0(rtype, fname)

#    define USRFUNC_H1(rtype, fname, a1t, a1n, a1r) \
     USRFUNC_P1(rtype, fname, a1t, a1n, a1r)

#    define USRFUNC_H2(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r) \
     USRFUNC_P2(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r)

#    define USRFUNC_H3(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r) \
     USRFUNC_P3(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r)

#    define USRFUNC_H4(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r) \
     USRFUNC_P4(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r)

#    define USRFUNC_H5(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r) \
     USRFUNC_P5(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r)

#    define USRFUNC_H6(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                                     a6t, a6n, a6r) \
     USRFUNC_P6(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                              a6t, a6n, a6r)

#    define USRFUNC_H8(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                                     a6t, a6n, a6r, a7t, a7n, a7r, a8t, a8n, a8r) \
     USRFUNC_P8(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                              a6t, a6n, a6r, a7t, a7n, a7r, a8t, a8n, a8r)

#    define USRFUNC_H7(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                                     a6t, a6n, a6r, a7t, a7n, a7r) \
     USRFUNC_P7(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                              a6t, a6n, a6r, a7t, a7n, a7r)

#    define USRFUNC_H9(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                                     a6t, a6n, a6r, a7t, a7n, a7r, a8t, a8n, a8r, a9t, a9n, a9r) \
     USRFUNC_P9(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, \
                              a6t, a6n, a6r, a7t, a7n, a7r, a8t, a8n, a8r, a9t, a9n, a9r)

#ifndef __MORPHOS__
#    define LIBFUNC_P0(rtype, fname, bt, bn)\
     USRFUNC_P1(rtype, fname, bt, bn, A6)

#    define LIBFUNC_P1(rtype, fname, a1t, a1n, a1r, bt, bn)\
     USRFUNC_P2(rtype, fname, bt, bn, A6, a1t, a1n, a1r)

#    define LIBFUNC_P2(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, bt, bn)\
     USRFUNC_P3(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r)

#    define LIBFUNC_P3(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, bt, bn)\
     USRFUNC_P4(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r)

#    define LIBFUNC_P4(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, bt, bn)\
     USRFUNC_P5(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r)

#    define LIBFUNC_P5(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, bt, bn)\
     USRFUNC_P6(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r)

#    define LIBFUNC_P6(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n ,a6r , bt, bn)\
     USRFUNC_P7(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n, a6r)

#    define LIBFUNC_P7(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n ,a6r, a7t, a7n, a7r, bt, bn)\
     USRFUNC_P8(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n, a6r, a7t, a7n, a7r)

#    define LIBFUNC_P8(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n ,a6r, a7t, a7n, a7r, a8t, a8n, a8r, bt, bn)\
     USRFUNC_P9(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n, a6r, a7t, a7n, a7r, a8t, a8n, a8r)

#    define LIBFUNC_H0(rtype, fname, bt, bn)\
     USRFUNC_H1(rtype, fname, bt, bn, A6)

#    define LIBFUNC_H1(rtype, fname, a1t, a1n, a1r, bt, bn)\
     USRFUNC_H2(rtype, fname, bt, bn, A6, a1t, a1n, a1r)

#    define LIBFUNC_H2(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, bt, bn)\
     USRFUNC_H3(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r)

#    define LIBFUNC_H3(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, bt, bn)\
     USRFUNC_H4(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r)

#    define LIBFUNC_H4(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, bt, bn)\
     USRFUNC_H5(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r)

#    define LIBFUNC_H5(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, bt, bn)\
     USRFUNC_H6(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r)

#    define LIBFUNC_H6(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n, a6r, bt, bn)\
     USRFUNC_H7(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n, a6r)

#    define LIBFUNC_H7(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n, a6r, a7t, a7n, a7r, bt, bn)\
     USRFUNC_H8(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n, a6r, a7t, a7n, a7r)

#    define LIBFUNC_H8(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n, a6r, a7t, a7n, a7r, a8t, a8n, a8r, bt, bn)\
     USRFUNC_H9(rtype, fname, bt, bn, A6, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, a4t, a4n, a4r, a5t, a5n, a5r, a6t, a6n, a6r, a7t, a7n, a7r, a8t, a8n, a8r)


#endif

#else /* !__AROS__ */

#    define VARARGS68K_PROTO(func)   func
#    define VARARGS68K_DECLARE(func) func

#    define VA_LIST       va_list
#    define VA_START(a,b) va_start(a,b)
#    define VA_ARG(a,b)   va_arg(a,b)
#    define VA_END(a)     va_end(a)

#    define VA_STARTLIN(a,b) va_start(a,b)
#    define VA_GETLIN(a,b)   a

#    define GIVEME_HOOK(name)      name
#    define GIVEME_HOOKENTRY(name) name

#    define DECLARE_HOOK(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3) \
         USRFUNC_H3(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3)

#    define DECLARE_DISPATCH(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3) \
         USRFUNC_H3(type, func_name, type1, name1, reg1, type2, name2,  re2, type3, name3, reg3)

#define __saveds
#define __aligned
#define __stdargs

#    include <aros/asmcall.h>
#    include <aros/libcall.h>

#    define USRFUNC_INIT AROS_USERFUNC_INIT
#    define USRFUNC_EXIT AROS_USERFUNC_EXIT

#    define LIBFUNC_INIT AROS_LIBFUNC_INIT
#    define LIBFUNC_EXIT AROS_LIBFUNC_EXIT

#    define USRFUNC_P1(rtype, fname, a1t, a1n, a1r) \
     AROS_UFP1                    \
     (                            \
         rtype, fname,            \
         AROS_UFPA(a1t, a1n, a1r)  \
     )

#    define USRFUNC_P2(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r) \
     AROS_UFP2                    \
     (                            \
         rtype, fname,            \
         AROS_UFPA(a1t, a1n, a1r), \
         AROS_UFPA(a2t, a2n, a2r)  \
     )

#    define USRFUNC_P3(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r) \
     AROS_UFP3                    \
     (                            \
         rtype, fname,            \
         AROS_UFPA(a1t, a1n, a1r), \
         AROS_UFPA(a2t, a2n, a2r), \
         AROS_UFPA(a3t, a3n, a3r)  \
     )

#    define USRFUNC_H1(rtype, fname, a1t, a1n, a1r) \
     AROS_UFH1                    \
     (                            \
         rtype, fname,            \
         AROS_UFHA(a1t, a1n, a1r)  \
     )

#    define USRFUNC_H2(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r) \
     AROS_UFH2                    \
     (                            \
         rtype, fname,            \
         AROS_UFHA(a1t, a1n, a1r), \
         AROS_UFHA(a2t, a2n, a2r)  \
     )

#    define USRFUNC_H3(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r) \
     AROS_UFH3                     \
     (                             \
         rtype, fname,             \
         AROS_UFHA(a1t, a1n, a1r), \
         AROS_UFHA(a2t, a2n, a2r), \
         AROS_UFHA(a3t, a3n, a3r)  \
     )

#    define LIBFUNC_P1(rtype, fname, a1t, a1n, a1r, bt, bn) \
     AROS_LP1                     \
     (                            \
         rtype, fname,            \
         AROS_LPA(a1t, a1n, a1r), \
    bt, bn                   \
     )

#    define LIBFUNC_P2(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, bt, bn) \
     AROS_LP2                     \
     (                            \
         rtype, fname,            \
         AROS_LPA(a1t, a1n, a1r), \
         AROS_LPA(a2t, a2n, a2r), \
    bt, bn                   \
     )

#    define LIBFUNC_P3(rtype, fname, a1t, a1n, a1r, a2t, a2n, a2r, a3t, a3n, a3r, bt, bn) \
     AROS_LP3                     \
     (                            \
         rtype, fname,            \
         AROS_LPA(a1t, a1n, a1r), \
         AROS_LPA(a2t, a2n, a2r), \
         AROS_LPA(a3t, a3n, a3r), \
    bt, bn                   \
     )


#endif /* !AROS */

#endif /* !PLATFORM_SPECIFIC_H */
