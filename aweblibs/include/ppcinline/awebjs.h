/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_AWEBJS_H
#define _PPCINLINE_AWEBJS_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef AWEBJS_BASE_NAME
#define AWEBJS_BASE_NAME AWebJSBase
#endif /* !AWEBJS_BASE_NAME */

#define Jgetuserdata(__p0) \
   LP1(180, ULONG , Jgetuserdata, \
      struct Jcontext *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Freejobject(__p0) \
   LP1NR(198, Freejobject, \
      struct Jobject *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jisarray(__p0, __p1) \
   LP2(162, BOOL , Jisarray, \
      struct Jcontext *, __p0, a0, \
      struct Jobject *, __p1, a1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jerrors(__p0, __p1, __p2, __p3) \
   LP4NR(252, Jerrors, \
      struct Jcontext *, __p0, a0, \
      BOOL , __p1, d0, \
      long , __p2, d1, \
      BOOL , __p3, d2, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Clearjobject(__p0, __p1) \
   LP2NR(192, Clearjobject, \
      struct Jobject *, __p0, a0, \
      UBYTE **, __p1, a1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jfindarray(__p0, __p1, __p2) \
   LP3(168, struct Jobject *, Jfindarray, \
      struct Jcontext *, __p0, a0, \
      struct Jobject *, __p1, a1, \
      UBYTE *, __p2, a2, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jisnumber(__p0) \
   LP1(186, BOOL , Jisnumber, \
      struct Jvar *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jproperty(__p0, __p1, __p2) \
   LP3(96, struct Jvar *, Jproperty, \
      struct Jcontext *, __p0, a0, \
      struct Jobject *, __p1, a1, \
      UBYTE *, __p2, a2, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jsetobjasfunc(__p0, __p1) \
   LP2NR(276, Jsetobjasfunc, \
      struct Jobject *, __p0, a0, \
      BOOL , __p1, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jdebug(__p0, __p1) \
   LP2NR(246, Jdebug, \
      struct Jcontext *, __p0, a0, \
      BOOL , __p1, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jdisposehook(__p0) \
   LP1(234, Objdisposehookfunc *, Jdisposehook, \
      struct Jobject *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jointernal(__p0) \
   LP1(114, void *, Jointernal, \
      struct Jobject *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jnewarrayelt(__p0, __p1) \
   LP2(138, struct Jvar *, Jnewarrayelt, \
      struct Jcontext *, __p0, a0, \
      struct Jobject *, __p1, a1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Disposejobject(__p0) \
   LP1NR(54, Disposejobject, \
      struct Jobject *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jgarbagecollect(__p0) \
   LP1NR(264, Jgarbagecollect, \
      struct Jcontext *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Newjcontext(__p0) \
   LP1(30, struct Jcontext *, Newjcontext, \
      UBYTE *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jpprotect(__p0, __p1) \
   LP2NR(216, Jpprotect, \
      struct Jvar *, __p0, a0, \
      ULONG , __p1, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jsetfeedback(__p0, __p1) \
   LP2NR(240, Jsetfeedback, \
      struct Jcontext *, __p0, a0, \
      Jfeedback *, __p1, a1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jasgobject(__p0, __p1, __p2) \
   LP3NR(84, Jasgobject, \
      struct Jcontext *, __p0, a0, \
      struct Jvar *, __p1, a1, \
      struct Jobject *, __p2, a2, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jasgstring(__p0, __p1, __p2) \
   LP3NR(78, Jasgstring, \
      struct Jcontext *, __p0, a0, \
      struct Jvar *, __p1, a1, \
      UBYTE *, __p2, a2, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jthis(__p0) \
   LP1(108, struct Jobject *, Jthis, \
      struct Jcontext *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jtostring(__p0, __p1) \
   LP2(72, UBYTE *, Jtostring, \
      struct Jcontext *, __p0, a0, \
      struct Jvar *, __p1, a1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jtoobject(__p0, __p1) \
   LP2(144, struct Jobject *, Jtoobject, \
      struct Jcontext *, __p0, a0, \
      struct Jvar *, __p1, a1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jasgboolean(__p0, __p1, __p2) \
   LP3NR(120, Jasgboolean, \
      struct Jcontext *, __p0, a0, \
      struct Jvar *, __p1, a1, \
      BOOL , __p2, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jkeepobject(__p0, __p1) \
   LP2NR(258, Jkeepobject, \
      struct Jobject *, __p0, a0, \
      BOOL , __p1, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jsetlinenumber(__p0, __p1) \
   LP2NR(270, Jsetlinenumber, \
      struct Jcontext *, __p0, a0, \
      long , __p1, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jgetreturnvalue(__p0) \
   LP1(210, struct Jvar *, Jgetreturnvalue, \
      struct Jcontext *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jdumpobjects(__p0) \
   LP1NR(204, Jdumpobjects, \
      struct Jcontext *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Newjobject(__p0) \
   LP1(48, struct Jobject *, Newjobject, \
      struct Jcontext *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jsetprototype(__p0, __p1, __p2) \
   LP3NR(174, Jsetprototype, \
      struct Jcontext *, __p0, a0, \
      struct Jobject *, __p1, a1, \
      struct Jobject *, __p2, a2, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jtoboolean(__p0, __p1) \
   LP2(126, BOOL , Jtoboolean, \
      struct Jcontext *, __p0, a0, \
      struct Jvar *, __p1, a1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Setjobject(__p0, __p1, __p2, __p3) \
   LP4NR(90, Setjobject, \
      struct Jobject *, __p0, a0, \
      Objhookfunc *, __p1, a1, \
      void *, __p2, a2, \
      Objdisposehookfunc *, __p3, a3, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jasgnumber(__p0, __p1, __p2) \
   LP3NR(156, Jasgnumber, \
      struct Jcontext *, __p0, a0, \
      struct Jvar *, __p1, a1, \
      long , __p2, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jcprotect(__p0, __p1) \
   LP2NR(222, Jcprotect, \
      struct Jcontext *, __p0, a0, \
      ULONG , __p1, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Newjscope(__p0) \
   LP1(300, struct Jobject *, Newjscope, \
      struct Jcontext *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Freejcontext(__p0) \
   LP1NR(36, Freejcontext, \
      struct Jcontext *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Newjarray(__p0) \
   LP1(132, struct Jobject *, Newjarray, \
      struct Jcontext *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jfargument(__p0, __p1) \
   LP2(66, struct Jvar *, Jfargument, \
      struct Jcontext *, __p0, a0, \
      long , __p1, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jaddeventhandler(__p0, __p1, __p2, __p3) \
   LP4NR(288, Jaddeventhandler, \
      struct Jcontext *, __p0, a0, \
      struct Jobject *, __p1, a1, \
      UBYTE *, __p2, a2, \
      UBYTE *, __p3, a3, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jallowgc(__p0, __p1) \
   LP2NR(312, Jallowgc, \
      struct Jcontext *, __p0, a0, \
      BOOL , __p1, d0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jtonumber(__p0, __p1) \
   LP2(150, long , Jtonumber, \
      struct Jcontext *, __p0, a0, \
      struct Jvar *, __p1, a1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jsetscreen(__p0, __p1) \
   LP2NR(282, Jsetscreen, \
      struct Jcontext *, __p0, a0, \
      UBYTE *, __p1, a1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Runjprogram(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
   LP7(42, BOOL , Runjprogram, \
      struct Jcontext *, __p0, a0, \
      struct Jobject *, __p1, a1, \
      UBYTE *, __p2, a2, \
      struct Jobject *, __p3, a3, \
      struct Jobject **, __p4, a4, \
      ULONG , __p5, d0, \
      ULONG , __p6, d1, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jaddproperty(__p0, __p1, __p2) \
   LP3(294, struct Jvar *, Jaddproperty, \
      struct Jcontext *, __p0, a0, \
      struct Jobject *, __p1, a1, \
      UBYTE *, __p2, a2, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddjfunctionA(__p0, __p1, __p2, __p3, __p4) \
   LP5FP(60, struct Jobject *, AddjfunctionA, \
      struct Jcontext *, __p0, a0, \
      struct Jobject *, __p1, a1, \
      UBYTE *, __p2, a2, \
      __ft, __p3, a3, \
      UBYTE **, __p4, a4, \
      , AWEBJS_BASE_NAME, void (* __ft)(struct Jcontext *), 0, 0, 0, 0, 0, 0)

#define Setjproperty(__p0, __p1, __p2) \
   LP3NR(102, Setjproperty, \
      struct Jvar *, __p0, a0, \
      Varhookfunc *, __p1, a1, \
      void *, __p2, a2, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Jpname(__p0) \
   LP1(228, UBYTE *, Jpname, \
      struct Jvar *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Disposejscope(__p0) \
   LP1NR(306, Disposejscope, \
      struct Jobject *, __p0, a0, \
      , AWEBJS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#if defined(USE_INLINE_STDARG) && !defined(__STRICT_ANSI__)

#include <stdarg.h>

#define Addjfunction(__p0, __p1, __p2, __p3, ...) \
   ({ULONG _tags[] = { __VA_ARGS__ }; \
   AddjfunctionA(__p0, __p1, __p2, __p3, (UBYTE **)_tags);})

#endif

#endif /* !_PPCINLINE_AWEBJS_H */
