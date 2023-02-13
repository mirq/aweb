
#ifndef __M_HOOK_H__
#define __M_HOOK_H__

#ifdef __MORPHOS__

#include <exec/types.h>
#include <emul/emulregs.h>
#include <emul/emulinterface.h>

#define M_HOOK(n, y, z) \
   static LONG n##_GATE(void); \
   static LONG n##_GATE2(struct Hook *hook, y, z); \
   static struct EmulLibEntry n = { \
   TRAP_LIB, 0, (void (*)(void))n##_GATE }; \
   static LONG n##_GATE(void) { \
   return (n##_GATE2((void *)REG_A0, (void *)REG_A2, (void *)REG_A1)); } \
   static struct Hook hook_##n = { {0, 0}, (void *)&n }; \
   static LONG n##_GATE2(struct Hook *hook, y, z)

#define L_HOOK(n, y) \
   static LONG n##_GATE(void); \
   static LONG n##_GATE2(struct Hook *h, y); \
   struct EmulLibEntry n = { \
   TRAP_LIB, 0, (void (*)(void))n##_GATE }; \
   static LONG n##_GATE(void) { \
   return (n##_GATE2((void *)REG_A0, (void *)REG_A2)); } \
   static struct Hook hook_##n = { {0, 0}, (void *)&n }; \
   static LONG n##_GATE2(struct Hook *h, y)

#define MUI_DISPATCH_DECL(x) struct EmulLibEntry x
#define MUI_DISPATCH(x) \
   static ULONG x ## _gate(void); \
   static ULONG x ## _gate2(struct IClass *cl, Object *obj, Msg msg); \
   struct EmulLibEntry x = { \
   TRAP_LIB, 0, (void (*)(void)) x ## _gate }; \
   static ULONG x ## _gate(void) { \
   return ( x ## _gate2((void *)REG_A0, (void *)REG_A2, (void *)REG_A1)); } \
   static struct Hook hook_##x = { {0, 0}, (void *)&x }; \
   static ULONG x ## _gate2(struct IClass *cl, Object *o, Msg msg)

#define MUI_CALLBACK( n ) \
   static void n##_GATE(void); \
   static void n##_GATE2(APTR obj); \
   struct EmulLibEntry n = { \
   TRAP_LIB, 0, (void (*)(void))n##_GATE }; \
   static void n##_GATE(void) { \
   (n##_GATE2((void *)REG_A2)); } \
   static struct Hook hook_##n  = { {0, 0}, (void *)&n, NULL, NULL }; \
   static void n##_GATE2(APTR obj)


#else


#define M_HOOK(n, y, z) \
   static ASM(ULONG) SAVEDS n (REG(a0, struct Hook *h), \
   REG(a2, y), REG(a1, z)); \
   static struct Hook hook_##n = { {0, 0}, (void *) n }; \
   static ASM(ULONG) SAVEDS n (REG(a0, struct Hook *h), \
   REG(a2, y), REG(a1, z))

#define L_HOOK(n, y) \
   static ASM(ULONG) SAVEDS n (REG(a0, struct Hook *h), REG(a2, y)); \
   static struct Hook hook_ ## n = { {0, 0}, (void *) n }; \
   static ASM(ULONG) SAVEDS n (REG(a0, struct Hook *h), REG(a2, y))

#define M_ICLASS(n) \
   SAVEDS ASM ULONG n (REG(a0) struct IClass *cl, REG(a2) Object *obj, REG(a1) Msg msg)

#define MUI_CALLBACK( f ) \
   static SAVEDS ASM(void) f ( REG(a2, APTR obj ));\
   static const struct Hook hook_##f = {\
   { NULL,NULL }, (void *)f, NULL, NULL\
   };\
   static SAVEDS ASM(void) f ( REG(a2, APTR obj ))

#define MUI_DISPATCH_DECL(x) \
   ASM(ULONG) SAVEDS x (REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))

#define MUI_DISPATCH(x) \
   ASM(ULONG) SAVEDS x (REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))

#endif

#endif
