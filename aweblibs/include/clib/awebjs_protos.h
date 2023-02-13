#ifndef CLIB_AWEBJS_PROTOS_H
#define CLIB_AWEBJS_PROTOS_H


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

   $Id: awebjs_protos.h,v 1.6 2009/06/15 17:04:08 opi Exp $

   Desc:

***********************************************************************/

#include <exec/types.h>
#include <libraries/awebjs.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Jcontext * Newjcontext(UBYTE * screenname);
void Freejcontext(struct Jcontext * jc);
BOOL Runjprogram(struct Jcontext * jc, struct Jobject * fscope, UBYTE * source,
   struct Jobject * jthis, struct Jobject ** gwtab, ULONG protkey,
   ULONG userdata);
struct Jobject * Newjobject(struct Jcontext * jc);
void Disposejobject(struct Jobject * jo);
struct Jobject * AddjfunctionA(struct Jcontext * jc, struct Jobject * jo, UBYTE * name,
   void (*code)(struct Jcontext *), UBYTE ** args);
struct Jobject * Addjfunction(struct Jcontext * jc, struct Jobject * jo, UBYTE * name,
   void (*code)(struct Jcontext *), ...);
struct Jvar * Jfargument(struct Jcontext * jc, long n);
UBYTE * Jtostring(struct Jcontext * jc, struct Jvar * jv);
void Jasgstring(struct Jcontext * jc, struct Jvar * jv, UBYTE * string);
void Jasgobject(struct Jcontext * jc, struct Jvar * jv, struct Jobject * jo);
void Setjobject(struct Jobject * jo, Objhookfunc * hook, void * internal,
   Objdisposehookfunc * dispose);
struct Jvar * Jproperty(struct Jcontext * jc, struct Jobject * jo, UBYTE * name);
void Setjproperty(struct Jvar * jv, Varhookfunc * hook, void * hookdata);
struct Jobject * Jthis(struct Jcontext * jc);
void * Jointernal(struct Jobject * jo);
void Jasgboolean(struct Jcontext * jc, struct Jvar * jv, BOOL bvalue);
BOOL Jtoboolean(struct Jcontext * jc, struct Jvar * jv);
struct Jobject * Newjarray(struct Jcontext * jc);
struct Jvar * Jnewarrayelt(struct Jcontext * jc, struct Jobject * jo);
struct Jobject * Jtoobject(struct Jcontext * jc, struct Jvar * jv);
long Jtonumber(struct Jcontext * jc, struct Jvar * jv);
void Jasgnumber(struct Jcontext * jc, struct Jvar * jv, long nvalue);
BOOL Jisarray(struct Jcontext * jc, struct Jobject * jo);
struct Jobject * Jfindarray(struct Jcontext * jc, struct Jobject * jo, UBYTE * name);
void Jsetprototype(struct Jcontext * jc, struct Jobject * jo, struct Jobject * proto);
ULONG Jgetuserdata(struct Jcontext * jc);
BOOL Jisnumber(struct Jvar * jv);
void Clearjobject(struct Jobject * jo, UBYTE ** except);
void Freejobject(struct Jobject * jo);
void Jdumpobjects(struct Jcontext * jc);
struct Jvar * Jgetreturnvalue(struct Jcontext * jc);
void Jpprotect(struct Jvar * var, ULONG protkey);
void Jcprotect(struct Jcontext * jc, ULONG protkey);
UBYTE * Jpname(struct Jvar * var);
Objdisposehookfunc * Jdisposehook(struct Jobject * jo);
void Jsetfeedback(struct Jcontext * jc, Jfeedback * jf);
void Jdebug(struct Jcontext * jc, BOOL debugon);
void Jerrors(struct Jcontext * jc, BOOL comperrors, long runerrrs, BOOL watch);
void Jkeepobject(struct Jobject * jo, BOOL used);
void Jgarbagecollect(struct Jcontext * jc);
void Jsetlinenumber(struct Jcontext * jc, long linenr);
void Jsetobjasfunc(struct Jobject * jo, BOOL asfunc);
void Jsetscreen(struct Jcontext * jc, UBYTE * screenname);
void Jaddeventhandler(struct Jcontext * jc, struct Jobject * jo, UBYTE * name, UBYTE * source);
struct Jvar * Jaddproperty(struct Jcontext * jc, struct Jobject * jo, UBYTE * name);
void Jallowgc(struct Jcontext *jc, BOOL allow);
struct Jobject * Newjscope(struct Jcontext *jc);
void Disposejscope(struct Jobject *jo);

#ifdef __cplusplus
}
#endif

#endif   /*  CLIB_AWEBJS_PROTOS_H  */
