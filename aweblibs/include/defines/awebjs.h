/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBJS_H
#define _INLINE_AWEBJS_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBJS_BASE_NAME
#define AWEBJS_BASE_NAME AWebJSBase
#endif /* !AWEBJS_BASE_NAME */

#define Newjcontext(___screenname) __Newjcontext_WB(AWEBJS_BASE_NAME, ___screenname)
#define __Newjcontext_WB(___base, ___screenname) \
   AROS_LC1(struct Jcontext *, Newjcontext, \
   AROS_LCA(UBYTE *, (___screenname), A0), \
   struct Library *, (___base), 5, Awebjs)

#define Freejcontext(___jc) __Freejcontext_WB(AWEBJS_BASE_NAME, ___jc)
#define __Freejcontext_WB(___base, ___jc) \
   AROS_LC1(void, Freejcontext, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   struct Library *, (___base), 6, Awebjs)

#define Runjprogram(___jc, ___fscope, ___source, ___jthis, ___gwtab, ___protkey, ___userdata) __Runjprogram_WB(AWEBJS_BASE_NAME, ___jc, ___fscope, ___source, ___jthis, ___gwtab, ___protkey, ___userdata)
#define __Runjprogram_WB(___base, ___jc, ___fscope, ___source, ___jthis, ___gwtab, ___protkey, ___userdata) \
   AROS_LC7(BOOL, Runjprogram, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jobject *, (___fscope), A1), \
   AROS_LCA(UBYTE *, (___source), A2), \
   AROS_LCA(struct Jobject *, (___jthis), A3), \
   AROS_LCA(struct Jobject **, (___gwtab), A4), \
   AROS_LCA(ULONG, (___protkey), D0), \
   AROS_LCA(ULONG, (___userdata), D1), \
   struct Library *, (___base), 7, Awebjs)

#define Newjobject(___jc) __Newjobject_WB(AWEBJS_BASE_NAME, ___jc)
#define __Newjobject_WB(___base, ___jc) \
   AROS_LC1(struct Jobject *, Newjobject, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   struct Library *, (___base), 8, Awebjs)

#define Disposejobject(___jo) __Disposejobject_WB(AWEBJS_BASE_NAME, ___jo)
#define __Disposejobject_WB(___base, ___jo) \
   AROS_LC1(void, Disposejobject, \
   AROS_LCA(struct Jobject *, (___jo), A0), \
   struct Library *, (___base), 9, Awebjs)

#define AddjfunctionA(___jc, ___jo, ___name, ___code, ___args) __AddjfunctionA_WB(AWEBJS_BASE_NAME, ___jc, ___jo, ___name, ___code, ___args)
#define __AddjfunctionA_WB(___base, ___jc, ___jo, ___name, ___code, ___args) \
   AROS_LC5(struct Jobject *, AddjfunctionA, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jobject *, (___jo), A1), \
   AROS_LCA(UBYTE *, (___name), A2), \
   AROS_LCA(void (*)(struct Jcontext *), (___code), A3), \
   AROS_LCA(UBYTE **, (___args), A4), \
   struct Library *, (___base), 10, Awebjs)

#ifndef NO_INLINE_VARARGS
#define Addjfunction(___jc, ___jo, ___name, ___code, ___args, ...) __Addjfunction_WB(AWEBJS_BASE_NAME, ___jc, ___jo, ___name, ___code, ___args, ## __VA_ARGS__)
#define __Addjfunction_WB(___base, ___jc, ___jo, ___name, ___code, ___args, ...) \
   ({APTR _message[] = { ___args, ## __VA_ARGS__ }; __AddjfunctionA_WB((___base), (___jc), (___jo), (___name), (___code), (UBYTE **) _message); })
#endif /* !NO_INLINE_VARARGS */

#define Jfargument(___jc, ___n) __Jfargument_WB(AWEBJS_BASE_NAME, ___jc, ___n)
#define __Jfargument_WB(___base, ___jc, ___n) \
   AROS_LC2(struct Jvar *, Jfargument, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(long, (___n), D0), \
   struct Library *, (___base), 11, Awebjs)

#define Jtostring(___jc, ___jv) __Jtostring_WB(AWEBJS_BASE_NAME, ___jc, ___jv)
#define __Jtostring_WB(___base, ___jc, ___jv) \
   AROS_LC2(UBYTE *, Jtostring, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jvar *, (___jv), A1), \
   struct Library *, (___base), 12, Awebjs)

#define Jasgstring(___jc, ___jv, ___string) __Jasgstring_WB(AWEBJS_BASE_NAME, ___jc, ___jv, ___string)
#define __Jasgstring_WB(___base, ___jc, ___jv, ___string) \
   AROS_LC3(void, Jasgstring, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jvar *, (___jv), A1), \
   AROS_LCA(UBYTE *, (___string), A2), \
   struct Library *, (___base), 13, Awebjs)

#define Jasgobject(___jc, ___jv, ___jo) __Jasgobject_WB(AWEBJS_BASE_NAME, ___jc, ___jv, ___jo)
#define __Jasgobject_WB(___base, ___jc, ___jv, ___jo) \
   AROS_LC3(void, Jasgobject, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jvar *, (___jv), A1), \
   AROS_LCA(struct Jobject *, (___jo), A2), \
   struct Library *, (___base), 14, Awebjs)

#define Setjobject(___jo, ___hook, ___internal, ___dispose) __Setjobject_WB(AWEBJS_BASE_NAME, ___jo, ___hook, ___internal, ___dispose)
#define __Setjobject_WB(___base, ___jo, ___hook, ___internal, ___dispose) \
   AROS_LC4(void, Setjobject, \
   AROS_LCA(struct Jobject *, (___jo), A0), \
   AROS_LCA(Objhookfunc *, (___hook), A1), \
   AROS_LCA(void *, (___internal), A2), \
   AROS_LCA(Objdisposehookfunc *, (___dispose), A3), \
   struct Library *, (___base), 15, Awebjs)

#define Jproperty(___jc, ___jo, ___name) __Jproperty_WB(AWEBJS_BASE_NAME, ___jc, ___jo, ___name)
#define __Jproperty_WB(___base, ___jc, ___jo, ___name) \
   AROS_LC3(struct Jvar *, Jproperty, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jobject *, (___jo), A1), \
   AROS_LCA(UBYTE *, (___name), A2), \
   struct Library *, (___base), 16, Awebjs)

#define Setjproperty(___jv, ___hook, ___hookdata) __Setjproperty_WB(AWEBJS_BASE_NAME, ___jv, ___hook, ___hookdata)
#define __Setjproperty_WB(___base, ___jv, ___hook, ___hookdata) \
   AROS_LC3(void, Setjproperty, \
   AROS_LCA(struct Jvar *, (___jv), A0), \
   AROS_LCA(Varhookfunc *, (___hook), A1), \
   AROS_LCA(void *, (___hookdata), A2), \
   struct Library *, (___base), 17, Awebjs)

#define Jthis(___jc) __Jthis_WB(AWEBJS_BASE_NAME, ___jc)
#define __Jthis_WB(___base, ___jc) \
   AROS_LC1(struct Jobject *, Jthis, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   struct Library *, (___base), 18, Awebjs)

#define Jointernal(___jo) __Jointernal_WB(AWEBJS_BASE_NAME, ___jo)
#define __Jointernal_WB(___base, ___jo) \
   AROS_LC1(void *, Jointernal, \
   AROS_LCA(struct Jobject *, (___jo), A0), \
   struct Library *, (___base), 19, Awebjs)

#define Jasgboolean(___jc, ___jv, ___bvalue) __Jasgboolean_WB(AWEBJS_BASE_NAME, ___jc, ___jv, ___bvalue)
#define __Jasgboolean_WB(___base, ___jc, ___jv, ___bvalue) \
   AROS_LC3(void, Jasgboolean, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jvar *, (___jv), A1), \
   AROS_LCA(BOOL, (___bvalue), D0), \
   struct Library *, (___base), 20, Awebjs)

#define Jtoboolean(___jc, ___jv) __Jtoboolean_WB(AWEBJS_BASE_NAME, ___jc, ___jv)
#define __Jtoboolean_WB(___base, ___jc, ___jv) \
   AROS_LC2(BOOL, Jtoboolean, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jvar *, (___jv), A1), \
   struct Library *, (___base), 21, Awebjs)

#define Newjarray(___jc) __Newjarray_WB(AWEBJS_BASE_NAME, ___jc)
#define __Newjarray_WB(___base, ___jc) \
   AROS_LC1(struct Jobject *, Newjarray, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   struct Library *, (___base), 22, Awebjs)

#define Jnewarrayelt(___jc, ___jo) __Jnewarrayelt_WB(AWEBJS_BASE_NAME, ___jc, ___jo)
#define __Jnewarrayelt_WB(___base, ___jc, ___jo) \
   AROS_LC2(struct Jvar *, Jnewarrayelt, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jobject *, (___jo), A1), \
   struct Library *, (___base), 23, Awebjs)

#define Jtoobject(___jc, ___jv) __Jtoobject_WB(AWEBJS_BASE_NAME, ___jc, ___jv)
#define __Jtoobject_WB(___base, ___jc, ___jv) \
   AROS_LC2(struct Jobject *, Jtoobject, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jvar *, (___jv), A1), \
   struct Library *, (___base), 24, Awebjs)

#define Jtonumber(___jc, ___jv) __Jtonumber_WB(AWEBJS_BASE_NAME, ___jc, ___jv)
#define __Jtonumber_WB(___base, ___jc, ___jv) \
   AROS_LC2(long, Jtonumber, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jvar *, (___jv), A1), \
   struct Library *, (___base), 25, Awebjs)

#define Jasgnumber(___jc, ___jv, ___nvalue) __Jasgnumber_WB(AWEBJS_BASE_NAME, ___jc, ___jv, ___nvalue)
#define __Jasgnumber_WB(___base, ___jc, ___jv, ___nvalue) \
   AROS_LC3(void, Jasgnumber, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jvar *, (___jv), A1), \
   AROS_LCA(long, (___nvalue), D0), \
   struct Library *, (___base), 26, Awebjs)

#define Jisarray(___jc, ___jo) __Jisarray_WB(AWEBJS_BASE_NAME, ___jc, ___jo)
#define __Jisarray_WB(___base, ___jc, ___jo) \
   AROS_LC2(BOOL, Jisarray, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jobject *, (___jo), A1), \
   struct Library *, (___base), 27, Awebjs)

#define Jfindarray(___jc, ___jo, ___name) __Jfindarray_WB(AWEBJS_BASE_NAME, ___jc, ___jo, ___name)
#define __Jfindarray_WB(___base, ___jc, ___jo, ___name) \
   AROS_LC3(struct Jobject *, Jfindarray, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jobject *, (___jo), A1), \
   AROS_LCA(UBYTE *, (___name), A2), \
   struct Library *, (___base), 28, Awebjs)

#define Jsetprototype(___jc, ___jo, ___proto) __Jsetprototype_WB(AWEBJS_BASE_NAME, ___jc, ___jo, ___proto)
#define __Jsetprototype_WB(___base, ___jc, ___jo, ___proto) \
   AROS_LC3(void, Jsetprototype, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jobject *, (___jo), A1), \
   AROS_LCA(struct Jobject *, (___proto), A2), \
   struct Library *, (___base), 29, Awebjs)

#define Jgetuserdata(___jc) __Jgetuserdata_WB(AWEBJS_BASE_NAME, ___jc)
#define __Jgetuserdata_WB(___base, ___jc) \
   AROS_LC1(ULONG, Jgetuserdata, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   struct Library *, (___base), 30, Awebjs)

#define Jisnumber(___jv) __Jisnumber_WB(AWEBJS_BASE_NAME, ___jv)
#define __Jisnumber_WB(___base, ___jv) \
   AROS_LC1(BOOL, Jisnumber, \
   AROS_LCA(struct Jvar *, (___jv), A0), \
   struct Library *, (___base), 31, Awebjs)

#define Clearjobject(___jo, ___except) __Clearjobject_WB(AWEBJS_BASE_NAME, ___jo, ___except)
#define __Clearjobject_WB(___base, ___jo, ___except) \
   AROS_LC2(void, Clearjobject, \
   AROS_LCA(struct Jobject *, (___jo), A0), \
   AROS_LCA(UBYTE **, (___except), A1), \
   struct Library *, (___base), 32, Awebjs)

#define Freejobject(___jo) __Freejobject_WB(AWEBJS_BASE_NAME, ___jo)
#define __Freejobject_WB(___base, ___jo) \
   AROS_LC1(void, Freejobject, \
   AROS_LCA(struct Jobject *, (___jo), A0), \
   struct Library *, (___base), 33, Awebjs)

#define Jdumpobjects(___jc) __Jdumpobjects_WB(AWEBJS_BASE_NAME, ___jc)
#define __Jdumpobjects_WB(___base, ___jc) \
   AROS_LC1(void, Jdumpobjects, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   struct Library *, (___base), 34, Awebjs)

#define Jgetreturnvalue(___jc) __Jgetreturnvalue_WB(AWEBJS_BASE_NAME, ___jc)
#define __Jgetreturnvalue_WB(___base, ___jc) \
   AROS_LC1(struct Jvar *, Jgetreturnvalue, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   struct Library *, (___base), 35, Awebjs)

#define Jpprotect(___var, ___protkey) __Jpprotect_WB(AWEBJS_BASE_NAME, ___var, ___protkey)
#define __Jpprotect_WB(___base, ___var, ___protkey) \
   AROS_LC2(void, Jpprotect, \
   AROS_LCA(struct Jvar *, (___var), A0), \
   AROS_LCA(ULONG, (___protkey), D0), \
   struct Library *, (___base), 36, Awebjs)

#define Jcprotect(___jc, ___protkey) __Jcprotect_WB(AWEBJS_BASE_NAME, ___jc, ___protkey)
#define __Jcprotect_WB(___base, ___jc, ___protkey) \
   AROS_LC2(void, Jcprotect, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(ULONG, (___protkey), D0), \
   struct Library *, (___base), 37, Awebjs)

#define Jpname(___var) __Jpname_WB(AWEBJS_BASE_NAME, ___var)
#define __Jpname_WB(___base, ___var) \
   AROS_LC1(UBYTE *, Jpname, \
   AROS_LCA(struct Jvar *, (___var), A0), \
   struct Library *, (___base), 38, Awebjs)

#define Jdisposehook(___jo) __Jdisposehook_WB(AWEBJS_BASE_NAME, ___jo)
#define __Jdisposehook_WB(___base, ___jo) \
   AROS_LC1(Objdisposehookfunc *, Jdisposehook, \
   AROS_LCA(struct Jobject *, (___jo), A0), \
   struct Library *, (___base), 39, Awebjs)

#define Jsetfeedback(___jc, ___jf) __Jsetfeedback_WB(AWEBJS_BASE_NAME, ___jc, ___jf)
#define __Jsetfeedback_WB(___base, ___jc, ___jf) \
   AROS_LC2(void, Jsetfeedback, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(Jfeedback *, (___jf), A1), \
   struct Library *, (___base), 40, Awebjs)

#define Jdebug(___jc, ___debugon) __Jdebug_WB(AWEBJS_BASE_NAME, ___jc, ___debugon)
#define __Jdebug_WB(___base, ___jc, ___debugon) \
   AROS_LC2(void, Jdebug, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(BOOL, (___debugon), D0), \
   struct Library *, (___base), 41, Awebjs)

#define Jerrors(___jc, ___comperrors, ___runerrrs, ___watch) __Jerrors_WB(AWEBJS_BASE_NAME, ___jc, ___comperrors, ___runerrrs, ___watch)
#define __Jerrors_WB(___base, ___jc, ___comperrors, ___runerrrs, ___watch) \
   AROS_LC4(void, Jerrors, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(BOOL, (___comperrors), D0), \
   AROS_LCA(long, (___runerrrs), D1), \
   AROS_LCA(BOOL, (___watch), D2), \
   struct Library *, (___base), 42, Awebjs)

#define Jkeepobject(___jo, ___used) __Jkeepobject_WB(AWEBJS_BASE_NAME, ___jo, ___used)
#define __Jkeepobject_WB(___base, ___jo, ___used) \
   AROS_LC2(void, Jkeepobject, \
   AROS_LCA(struct Jobject *, (___jo), A0), \
   AROS_LCA(BOOL, (___used), D0), \
   struct Library *, (___base), 43, Awebjs)

#define Jgarbagecollect(___jc) __Jgarbagecollect_WB(AWEBJS_BASE_NAME, ___jc)
#define __Jgarbagecollect_WB(___base, ___jc) \
   AROS_LC1(void, Jgarbagecollect, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   struct Library *, (___base), 44, Awebjs)

#define Jsetlinenumber(___jc, ___linenr) __Jsetlinenumber_WB(AWEBJS_BASE_NAME, ___jc, ___linenr)
#define __Jsetlinenumber_WB(___base, ___jc, ___linenr) \
   AROS_LC2(void, Jsetlinenumber, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(long, (___linenr), D0), \
   struct Library *, (___base), 45, Awebjs)

#define Jsetobjasfunc(___jo, ___asfunc) __Jsetobjasfunc_WB(AWEBJS_BASE_NAME, ___jo, ___asfunc)
#define __Jsetobjasfunc_WB(___base, ___jo, ___asfunc) \
   AROS_LC2(void, Jsetobjasfunc, \
   AROS_LCA(struct Jobject *, (___jo), A0), \
   AROS_LCA(BOOL, (___asfunc), D0), \
   struct Library *, (___base), 46, Awebjs)

#define Jsetscreen(___jc, ___screenname) __Jsetscreen_WB(AWEBJS_BASE_NAME, ___jc, ___screenname)
#define __Jsetscreen_WB(___base, ___jc, ___screenname) \
   AROS_LC2(void, Jsetscreen, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(UBYTE *, (___screenname), A1), \
   struct Library *, (___base), 47, Awebjs)

#define Jaddeventhandler(___jc, ___jo, ___name, ___source) __Jaddeventhandler_WB(AWEBJS_BASE_NAME, ___jc, ___jo, ___name, ___source)
#define __Jaddeventhandler_WB(___base, ___jc, ___jo, ___name, ___source) \
   AROS_LC4(void, Jaddeventhandler, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jobject *, (___jo), A1), \
   AROS_LCA(UBYTE *, (___name), A2), \
   AROS_LCA(UBYTE *, (___source), A3), \
   struct Library *, (___base), 48, Awebjs)

#define Jaddproperty(___jc, ___jo, ___name) __Jaddproperty_WB(AWEBJS_BASE_NAME, ___jc, ___jo, ___name)
#define __Jaddproperty_WB(___base, ___jc, ___jo, ___name) \
   AROS_LC3(struct Jvar *, Jaddproperty, \
   AROS_LCA(struct Jcontext *, (___jc), A0), \
   AROS_LCA(struct Jobject *, (___jo), A1), \
   AROS_LCA(UBYTE *, (___name), A2), \
   struct Library *, (___base), 49, Awebjs)

#endif /* !_INLINE_AWEBJS_H */
