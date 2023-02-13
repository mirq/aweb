#ifndef _INLINE_AWEBJS_H
#define _INLINE_AWEBJS_H

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

   $Id: awebjs.h,v 1.12 2009/06/15 17:04:26 opi Exp $

   Desc:

***********************************************************************/



#ifndef AWEBJS_BASE_NAME
#define AWEBJS_BASE_NAME AWebJSBase
#endif

#define Newjcontext(screenname) ({ \
  UBYTE * _Newjcontext_screenname = (screenname); \
  ({ \
  register char * _Newjcontext__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jcontext * (*)(char * __asm("a6"), UBYTE * __asm("a0"))) \
  (_Newjcontext__bn - 30))(_Newjcontext__bn, _Newjcontext_screenname); \
});})

#define Freejcontext(jc) ({ \
  struct Jcontext * _Freejcontext_jc = (jc); \
  ({ \
  register char * _Freejcontext__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"))) \
  (_Freejcontext__bn - 36))(_Freejcontext__bn, _Freejcontext_jc); \
});})

#define Runjprogram(jc, fscope, source, jthis, gwtab, protkey, userdata) ({ \
  struct Jcontext * _Runjprogram_jc = (jc); \
  struct Jobject * _Runjprogram_fscope = (fscope); \
  UBYTE * _Runjprogram_source = (source); \
  struct Jobject * _Runjprogram_jthis = (jthis); \
  struct Jobject ** _Runjprogram_gwtab = (gwtab); \
  ULONG _Runjprogram_protkey = (protkey); \
  ULONG _Runjprogram_userdata = (userdata); \
  ({ \
  register char * _Runjprogram__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jobject * __asm("a1"), UBYTE * __asm("a2"), struct Jobject * __asm("a3"), struct Jobject ** __asm("a4"), ULONG __asm("d0"), ULONG __asm("d1"))) \
  (_Runjprogram__bn - 42))(_Runjprogram__bn, _Runjprogram_jc, _Runjprogram_fscope, _Runjprogram_source, _Runjprogram_jthis, _Runjprogram_gwtab, _Runjprogram_protkey, _Runjprogram_userdata); \
});})

#define Newjobject(jc) ({ \
  struct Jcontext * _Newjobject_jc = (jc); \
  ({ \
  register char * _Newjobject__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jobject * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"))) \
  (_Newjobject__bn - 48))(_Newjobject__bn, _Newjobject_jc); \
});})

#define Disposejobject(jo) ({ \
  struct Jobject * _Disposejobject_jo = (jo); \
  ({ \
  register char * _Disposejobject__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jobject * __asm("a0"))) \
  (_Disposejobject__bn - 54))(_Disposejobject__bn, _Disposejobject_jo); \
});})

#define AddjfunctionA(jc, jo, name, code, args) ({ \
  struct Jcontext * _AddjfunctionA_jc = (jc); \
  struct Jobject * _AddjfunctionA_jo = (jo); \
  UBYTE * _AddjfunctionA_name = (name); \
  void (*_AddjfunctionA_code)(struct Jcontext *) = (code); \
  UBYTE ** _AddjfunctionA_args = (args); \
  ({ \
  register char * _AddjfunctionA__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jobject * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jobject * __asm("a1"), UBYTE * __asm("a2"), void (*)(struct Jcontext *) __asm("a3"), UBYTE ** __asm("a4"))) \
  (_AddjfunctionA__bn - 60))(_AddjfunctionA__bn, _AddjfunctionA_jc, _AddjfunctionA_jo, _AddjfunctionA_name, _AddjfunctionA_code, _AddjfunctionA_args); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ struct Jobject * ___Addjfunction(struct Library * AWebJSBase, struct Jcontext * jc, struct Jobject * jo, UBYTE * name, void (*code)(struct Jcontext *), APTR args, ...)
{
  return AddjfunctionA(jc, jo, name, code, (UBYTE **) &args);
}

#define Addjfunction(jc, jo, name, code...) ___Addjfunction((struct Library *)AWEBJS_BASE_NAME, jc, jo, name, code)
#endif

#define Jfargument(jc, n) ({ \
  struct Jcontext * _Jfargument_jc = (jc); \
  long _Jfargument_n = (n); \
  ({ \
  register char * _Jfargument__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jvar * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), long __asm("d0"))) \
  (_Jfargument__bn - 66))(_Jfargument__bn, _Jfargument_jc, _Jfargument_n); \
});})

#define Jtostring(jc, jv) ({ \
  struct Jcontext * _Jtostring_jc = (jc); \
  struct Jvar * _Jtostring_jv = (jv); \
  ({ \
  register char * _Jtostring__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((UBYTE * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jvar * __asm("a1"))) \
  (_Jtostring__bn - 72))(_Jtostring__bn, _Jtostring_jc, _Jtostring_jv); \
});})

#define Jasgstring(jc, jv, string) ({ \
  struct Jcontext * _Jasgstring_jc = (jc); \
  struct Jvar * _Jasgstring_jv = (jv); \
  UBYTE * _Jasgstring_string = (string); \
  ({ \
  register char * _Jasgstring__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jvar * __asm("a1"), UBYTE * __asm("a2"))) \
  (_Jasgstring__bn - 78))(_Jasgstring__bn, _Jasgstring_jc, _Jasgstring_jv, _Jasgstring_string); \
});})

#define Jasgobject(jc, jv, jo) ({ \
  struct Jcontext * _Jasgobject_jc = (jc); \
  struct Jvar * _Jasgobject_jv = (jv); \
  struct Jobject * _Jasgobject_jo = (jo); \
  ({ \
  register char * _Jasgobject__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jvar * __asm("a1"), struct Jobject * __asm("a2"))) \
  (_Jasgobject__bn - 84))(_Jasgobject__bn, _Jasgobject_jc, _Jasgobject_jv, _Jasgobject_jo); \
});})

#define Setjobject(jo, hook, internal, dispose) ({ \
  struct Jobject * _Setjobject_jo = (jo); \
  Objhookfunc * _Setjobject_hook = (hook); \
  void * _Setjobject_internal = (internal); \
  Objdisposehookfunc * _Setjobject_dispose = (dispose); \
  ({ \
  register char * _Setjobject__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jobject * __asm("a0"), Objhookfunc * __asm("a1"), void * __asm("a2"), Objdisposehookfunc * __asm("a3"))) \
  (_Setjobject__bn - 90))(_Setjobject__bn, _Setjobject_jo, _Setjobject_hook, _Setjobject_internal, _Setjobject_dispose); \
});})

#define Jproperty(jc, jo, name) ({ \
  struct Jcontext * _Jproperty_jc = (jc); \
  struct Jobject * _Jproperty_jo = (jo); \
  UBYTE * _Jproperty_name = (name); \
  ({ \
  register char * _Jproperty__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jvar * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jobject * __asm("a1"), UBYTE * __asm("a2"))) \
  (_Jproperty__bn - 96))(_Jproperty__bn, _Jproperty_jc, _Jproperty_jo, _Jproperty_name); \
});})

#define Setjproperty(jv, hook, hookdata) ({ \
  struct Jvar * _Setjproperty_jv = (jv); \
  Varhookfunc * _Setjproperty_hook = (hook); \
  void * _Setjproperty_hookdata = (hookdata); \
  ({ \
  register char * _Setjproperty__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jvar * __asm("a0"), Varhookfunc * __asm("a1"), void * __asm("a2"))) \
  (_Setjproperty__bn - 102))(_Setjproperty__bn, _Setjproperty_jv, _Setjproperty_hook, _Setjproperty_hookdata); \
});})

#define Jthis(jc) ({ \
  struct Jcontext * _Jthis_jc = (jc); \
  ({ \
  register char * _Jthis__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jobject * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"))) \
  (_Jthis__bn - 108))(_Jthis__bn, _Jthis_jc); \
});})

#define Jointernal(jo) ({ \
  struct Jobject * _Jointernal_jo = (jo); \
  ({ \
  register char * _Jointernal__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void * (*)(char * __asm("a6"), struct Jobject * __asm("a0"))) \
  (_Jointernal__bn - 114))(_Jointernal__bn, _Jointernal_jo); \
});})

#define Jasgboolean(jc, jv, bvalue) ({ \
  struct Jcontext * _Jasgboolean_jc = (jc); \
  struct Jvar * _Jasgboolean_jv = (jv); \
  LONG _Jasgboolean_bvalue = (bvalue); \
  ({ \
  register char * _Jasgboolean__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jvar * __asm("a1"), LONG __asm("d0"))) \
  (_Jasgboolean__bn - 120))(_Jasgboolean__bn, _Jasgboolean_jc, _Jasgboolean_jv, _Jasgboolean_bvalue); \
});})

#define Jtoboolean(jc, jv) ({ \
  struct Jcontext * _Jtoboolean_jc = (jc); \
  struct Jvar * _Jtoboolean_jv = (jv); \
  ({ \
  register char * _Jtoboolean__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jvar * __asm("a1"))) \
  (_Jtoboolean__bn - 126))(_Jtoboolean__bn, _Jtoboolean_jc, _Jtoboolean_jv); \
});})

#define Newjarray(jc) ({ \
  struct Jcontext * _Newjarray_jc = (jc); \
  ({ \
  register char * _Newjarray__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jobject * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"))) \
  (_Newjarray__bn - 132))(_Newjarray__bn, _Newjarray_jc); \
});})

#define Jnewarrayelt(jc, jo) ({ \
  struct Jcontext * _Jnewarrayelt_jc = (jc); \
  struct Jobject * _Jnewarrayelt_jo = (jo); \
  ({ \
  register char * _Jnewarrayelt__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jvar * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jobject * __asm("a1"))) \
  (_Jnewarrayelt__bn - 138))(_Jnewarrayelt__bn, _Jnewarrayelt_jc, _Jnewarrayelt_jo); \
});})

#define Jtoobject(jc, jv) ({ \
  struct Jcontext * _Jtoobject_jc = (jc); \
  struct Jvar * _Jtoobject_jv = (jv); \
  ({ \
  register char * _Jtoobject__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jobject * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jvar * __asm("a1"))) \
  (_Jtoobject__bn - 144))(_Jtoobject__bn, _Jtoobject_jc, _Jtoobject_jv); \
});})

#define Jtonumber(jc, jv) ({ \
  struct Jcontext * _Jtonumber_jc = (jc); \
  struct Jvar * _Jtonumber_jv = (jv); \
  ({ \
  register char * _Jtonumber__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((long (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jvar * __asm("a1"))) \
  (_Jtonumber__bn - 150))(_Jtonumber__bn, _Jtonumber_jc, _Jtonumber_jv); \
});})

#define Jasgnumber(jc, jv, nvalue) ({ \
  struct Jcontext * _Jasgnumber_jc = (jc); \
  struct Jvar * _Jasgnumber_jv = (jv); \
  long _Jasgnumber_nvalue = (nvalue); \
  ({ \
  register char * _Jasgnumber__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jvar * __asm("a1"), long __asm("d0"))) \
  (_Jasgnumber__bn - 156))(_Jasgnumber__bn, _Jasgnumber_jc, _Jasgnumber_jv, _Jasgnumber_nvalue); \
});})

#define Jisarray(jc, jo) ({ \
  struct Jcontext * _Jisarray_jc = (jc); \
  struct Jobject * _Jisarray_jo = (jo); \
  ({ \
  register char * _Jisarray__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jobject * __asm("a1"))) \
  (_Jisarray__bn - 162))(_Jisarray__bn, _Jisarray_jc, _Jisarray_jo); \
});})

#define Jfindarray(jc, jo, name) ({ \
  struct Jcontext * _Jfindarray_jc = (jc); \
  struct Jobject * _Jfindarray_jo = (jo); \
  UBYTE * _Jfindarray_name = (name); \
  ({ \
  register char * _Jfindarray__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jobject * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jobject * __asm("a1"), UBYTE * __asm("a2"))) \
  (_Jfindarray__bn - 168))(_Jfindarray__bn, _Jfindarray_jc, _Jfindarray_jo, _Jfindarray_name); \
});})

#define Jsetprototype(jc, jo, proto) ({ \
  struct Jcontext * _Jsetprototype_jc = (jc); \
  struct Jobject * _Jsetprototype_jo = (jo); \
  struct Jobject * _Jsetprototype_proto = (proto); \
  ({ \
  register char * _Jsetprototype__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jobject * __asm("a1"), struct Jobject * __asm("a2"))) \
  (_Jsetprototype__bn - 174))(_Jsetprototype__bn, _Jsetprototype_jc, _Jsetprototype_jo, _Jsetprototype_proto); \
});})

#define Jgetuserdata(jc) ({ \
  struct Jcontext * _Jgetuserdata_jc = (jc); \
  ({ \
  register char * _Jgetuserdata__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Jcontext * __asm("a0"))) \
  (_Jgetuserdata__bn - 180))(_Jgetuserdata__bn, _Jgetuserdata_jc); \
});})

#define Jisnumber(jv) ({ \
  struct Jvar * _Jisnumber_jv = (jv); \
  ({ \
  register char * _Jisnumber__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), struct Jvar * __asm("a0"))) \
  (_Jisnumber__bn - 186))(_Jisnumber__bn, _Jisnumber_jv); \
});})

#define Clearjobject(jo, except) ({ \
  struct Jobject * _Clearjobject_jo = (jo); \
  UBYTE ** _Clearjobject_except = (except); \
  ({ \
  register char * _Clearjobject__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jobject * __asm("a0"), UBYTE ** __asm("a1"))) \
  (_Clearjobject__bn - 192))(_Clearjobject__bn, _Clearjobject_jo, _Clearjobject_except); \
});})

#define Freejobject(jo) ({ \
  struct Jobject * _Freejobject_jo = (jo); \
  ({ \
  register char * _Freejobject__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jobject * __asm("a0"))) \
  (_Freejobject__bn - 198))(_Freejobject__bn, _Freejobject_jo); \
});})

#define Jdumpobjects(jc) ({ \
  struct Jcontext * _Jdumpobjects_jc = (jc); \
  ({ \
  register char * _Jdumpobjects__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"))) \
  (_Jdumpobjects__bn - 204))(_Jdumpobjects__bn, _Jdumpobjects_jc); \
});})

#define Jgetreturnvalue(jc) ({ \
  struct Jcontext * _Jgetreturnvalue_jc = (jc); \
  ({ \
  register char * _Jgetreturnvalue__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jvar * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"))) \
  (_Jgetreturnvalue__bn - 210))(_Jgetreturnvalue__bn, _Jgetreturnvalue_jc); \
});})

#define Jpprotect(var, protkey) ({ \
  struct Jvar * _Jpprotect_var = (var); \
  ULONG _Jpprotect_protkey = (protkey); \
  ({ \
  register char * _Jpprotect__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jvar * __asm("a0"), ULONG __asm("d0"))) \
  (_Jpprotect__bn - 216))(_Jpprotect__bn, _Jpprotect_var, _Jpprotect_protkey); \
});})

#define Jcprotect(jc, protkey) ({ \
  struct Jcontext * _Jcprotect_jc = (jc); \
  ULONG _Jcprotect_protkey = (protkey); \
  ({ \
  register char * _Jcprotect__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), ULONG __asm("d0"))) \
  (_Jcprotect__bn - 222))(_Jcprotect__bn, _Jcprotect_jc, _Jcprotect_protkey); \
});})

#define Jpname(var) ({ \
  struct Jvar * _Jpname_var = (var); \
  ({ \
  register char * _Jpname__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((UBYTE * (*)(char * __asm("a6"), struct Jvar * __asm("a0"))) \
  (_Jpname__bn - 228))(_Jpname__bn, _Jpname_var); \
});})

#define Jdisposehook(jo) ({ \
  struct Jobject * _Jdisposehook_jo = (jo); \
  ({ \
  register char * _Jdisposehook__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((Objdisposehookfunc * (*)(char * __asm("a6"), struct Jobject * __asm("a0"))) \
  (_Jdisposehook__bn - 234))(_Jdisposehook__bn, _Jdisposehook_jo); \
});})

#define Jsetfeedback(jc, jf) ({ \
  struct Jcontext * _Jsetfeedback_jc = (jc); \
  Jfeedback * _Jsetfeedback_jf = (jf); \
  ({ \
  register char * _Jsetfeedback__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), Jfeedback * __asm("a1"))) \
  (_Jsetfeedback__bn - 240))(_Jsetfeedback__bn, _Jsetfeedback_jc, _Jsetfeedback_jf); \
});})

#define Jdebug(jc, debugon) ({ \
  struct Jcontext * _Jdebug_jc = (jc); \
  LONG _Jdebug_debugon = (debugon); \
  ({ \
  register char * _Jdebug__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), LONG __asm("d0"))) \
  (_Jdebug__bn - 246))(_Jdebug__bn, _Jdebug_jc, _Jdebug_debugon); \
});})

#define Jerrors(jc, comperrors, runerrrs, watch) ({ \
  struct Jcontext * _Jerrors_jc = (jc); \
  LONG _Jerrors_comperrors = (comperrors); \
  long _Jerrors_runerrrs = (runerrrs); \
  LONG _Jerrors_watch = (watch); \
  ({ \
  register char * _Jerrors__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), LONG __asm("d0"), long __asm("d1"), LONG __asm("d2"))) \
  (_Jerrors__bn - 252))(_Jerrors__bn, _Jerrors_jc, _Jerrors_comperrors, _Jerrors_runerrrs, _Jerrors_watch); \
});})

#define Jkeepobject(jo, used) ({ \
  struct Jobject * _Jkeepobject_jo = (jo); \
  LONG _Jkeepobject_used = (used); \
  ({ \
  register char * _Jkeepobject__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jobject * __asm("a0"), LONG __asm("d0"))) \
  (_Jkeepobject__bn - 258))(_Jkeepobject__bn, _Jkeepobject_jo, _Jkeepobject_used); \
});})

#define Jgarbagecollect(jc) ({ \
  struct Jcontext * _Jgarbagecollect_jc = (jc); \
  ({ \
  register char * _Jgarbagecollect__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"))) \
  (_Jgarbagecollect__bn - 264))(_Jgarbagecollect__bn, _Jgarbagecollect_jc); \
});})

#define Jsetlinenumber(jc, linenr) ({ \
  struct Jcontext * _Jsetlinenumber_jc = (jc); \
  long _Jsetlinenumber_linenr = (linenr); \
  ({ \
  register char * _Jsetlinenumber__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), long __asm("d0"))) \
  (_Jsetlinenumber__bn - 270))(_Jsetlinenumber__bn, _Jsetlinenumber_jc, _Jsetlinenumber_linenr); \
});})

#define Jsetobjasfunc(jo, asfunc) ({ \
  struct Jobject * _Jsetobjasfunc_jo = (jo); \
  LONG _Jsetobjasfunc_asfunc = (asfunc); \
  ({ \
  register char * _Jsetobjasfunc__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jobject * __asm("a0"), LONG __asm("d0"))) \
  (_Jsetobjasfunc__bn - 276))(_Jsetobjasfunc__bn, _Jsetobjasfunc_jo, _Jsetobjasfunc_asfunc); \
});})

#define Jsetscreen(jc, screenname) ({ \
  struct Jcontext * _Jsetscreen_jc = (jc); \
  UBYTE * _Jsetscreen_screenname = (screenname); \
  ({ \
  register char * _Jsetscreen__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), UBYTE * __asm("a1"))) \
  (_Jsetscreen__bn - 282))(_Jsetscreen__bn, _Jsetscreen_jc, _Jsetscreen_screenname); \
});})

#define Jaddeventhandler(jc, jo, name, source) ({ \
  struct Jcontext * _Jaddeventhandler_jc = (jc); \
  struct Jobject * _Jaddeventhandler_jo = (jo); \
  UBYTE * _Jaddeventhandler_name = (name); \
  UBYTE * _Jaddeventhandler_source = (source); \
  ({ \
  register char * _Jaddeventhandler__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jobject * __asm("a1"), UBYTE * __asm("a2"), UBYTE * __asm("a3"))) \
  (_Jaddeventhandler__bn - 288))(_Jaddeventhandler__bn, _Jaddeventhandler_jc, _Jaddeventhandler_jo, _Jaddeventhandler_name, _Jaddeventhandler_source); \
});})

#define Jaddproperty(jc, jo, name) ({ \
  struct Jcontext * _Jaddproperty_jc = (jc); \
  struct Jobject * _Jaddproperty_jo = (jo); \
  UBYTE * _Jaddproperty_name = (name); \
  ({ \
  register char * _Jaddproperty__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jvar * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), struct Jobject * __asm("a1"), UBYTE * __asm("a2"))) \
  (_Jaddproperty__bn - 294))(_Jaddproperty__bn, _Jaddproperty_jc, _Jaddproperty_jo, _Jaddproperty_name); \
});})

#define Newjscope(jc) ({ \
  struct Jcontext * _Newjscope_jc = (jc); \
  ({ \
  register char * _Newjscope__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((struct Jobject * (*)(char * __asm("a6"), struct Jcontext * __asm("a0"))) \
  (_Newjscope__bn - 300))(_Newjscope__bn, _Newjscope_jc); \
});})

#define Disposejscope(jo) ({ \
  struct Jobject * _Disposejscope_jo = (jo); \
  ({ \
  register char * _Disposejscope__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jobject * __asm("a0"))) \
  (_Disposejscope__bn - 306))(_Disposejscope__bn, _Disposejscope_jo); \
});})

#define Jallowgc(jc, allow) ({ \
  struct Jcontext * _Jallowgc_jc = (jc); \
  BOOL _Jallowgc_allow = (allow); \
  ({ \
  register char * _Jallowgc__bn __asm("a6") = (char *) (AWEBJS_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Jcontext * __asm("a0"), BOOL __asm("a1"))) \
  (_Jallowgc__bn - 312))(_Jallowgc__bn, _Jallowgc_jc, _Jallowgc_allow); \
});})

#endif /*  _INLINE_AWEBJS_H  */
