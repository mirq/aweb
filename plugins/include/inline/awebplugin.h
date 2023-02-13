#ifndef _INLINE_AWEBSUPPORT_H
#define _INLINE_AWEBSUPPORT_H

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

   $Id: awebplugin.h,v 1.4 2009/06/15 17:05:43 opi Exp $

   Desc:

***********************************************************************/



#ifndef CLIB_AWEBSUPPORT_PROTOS_H
#define CLIB_AWEBSUPPORT_PROTOS_H
#endif

#include <exec/types.h>

#ifndef AWEBPLUGIN_BASE_NAME
#define AWEBPLUGIN_BASE_NAME AwebSupportBase
#endif

#define AmethodA(par1, tags) ({ \
  struct Aobject * _AmethodA_par1 = (par1); \
  struct Amessage * _AmethodA_tags = (tags); \
  ({ \
  register char * _AmethodA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct Amessage * __asm("a1"))) \
  (_AmethodA__bn - 30))(_AmethodA__bn, _AmethodA_par1, _AmethodA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___Amethod(struct Library * AwebSupportBase, struct Aobject * par1, ...)
{
  return AmethodA(par1, (struct Amessage *) ((ULONG) &par1 + sizeof(struct Aobject *)));
}

#define Amethod(par1...) ___Amethod(AWEBPLUGIN_BASE_NAME, par1)
#endif

#define AmethodasA(par1, par2, tags) ({ \
  ULONG _AmethodasA_par1 = (par1); \
  struct Aobject * _AmethodasA_par2 = (par2); \
  struct Amessage * _AmethodasA_tags = (tags); \
  ({ \
  register char * _AmethodasA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), ULONG __asm("d0"), struct Aobject * __asm("a0"), struct Amessage * __asm("a1"))) \
  (_AmethodasA__bn - 36))(_AmethodasA__bn, _AmethodasA_par1, _AmethodasA_par2, _AmethodasA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___Amethodas(struct Library * AwebSupportBase, ULONG par1, struct Aobject * par2, ...)
{
  return AmethodasA(par1, par2, (struct Amessage *) ((ULONG) &par2 + sizeof(struct Aobject *)));
}

#define Amethodas(par1, par2...) ___Amethodas(AWEBPLUGIN_BASE_NAME, par1, par2)
#endif

#define AnewobjectA(par1, tags) ({ \
  ULONG _AnewobjectA_par1 = (par1); \
  struct TagItem * _AnewobjectA_tags = (tags); \
  ({ \
  register char * _AnewobjectA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((APTR (*)(char * __asm("a6"), ULONG __asm("d0"), struct TagItem * __asm("a0"))) \
  (_AnewobjectA__bn - 42))(_AnewobjectA__bn, _AnewobjectA_par1, _AnewobjectA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ APTR ___Anewobject(struct Library * AwebSupportBase, ULONG par1, ...)
{
  return AnewobjectA(par1, (struct TagItem *) ((ULONG) &par1 + sizeof(ULONG)));
}

#define Anewobject(par1...) ___Anewobject(AWEBPLUGIN_BASE_NAME, par1)
#endif

#define Adisposeobject(last) ({ \
  struct Aobject * _Adisposeobject_last = (last); \
  ({ \
  register char * _Adisposeobject__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Aobject * __asm("a0"))) \
  (_Adisposeobject__bn - 48))(_Adisposeobject__bn, _Adisposeobject_last); \
});})

#define AsetattrsA(par1, tags) ({ \
  struct Aobject * _AsetattrsA_par1 = (par1); \
  struct TagItem * _AsetattrsA_tags = (tags); \
  ({ \
  register char * _AsetattrsA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct TagItem * __asm("a1"))) \
  (_AsetattrsA__bn - 54))(_AsetattrsA__bn, _AsetattrsA_par1, _AsetattrsA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___Asetattrs(struct Library * AwebSupportBase, struct Aobject * par1, ...)
{
  return AsetattrsA(par1, (struct TagItem *) ((ULONG) &par1 + sizeof(struct Aobject *)));
}

#define Asetattrs(par1...) ___Asetattrs(AWEBPLUGIN_BASE_NAME, par1)
#endif

#define AgetattrsA(par1, tags) ({ \
  struct Aobject * _AgetattrsA_par1 = (par1); \
  struct TagItem * _AgetattrsA_tags = (tags); \
  ({ \
  register char * _AgetattrsA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct TagItem * __asm("a1"))) \
  (_AgetattrsA__bn - 60))(_AgetattrsA__bn, _AgetattrsA_par1, _AgetattrsA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___Agetattrs(struct Library * AwebSupportBase, struct Aobject * par1, ...)
{
  return AgetattrsA(par1, (struct TagItem *) ((ULONG) &par1 + sizeof(struct Aobject *)));
}

#define Agetattrs(par1...) ___Agetattrs(AWEBPLUGIN_BASE_NAME, par1)
#endif

#define Agetattr(par1, last) ({ \
  struct Aobject * _Agetattr_par1 = (par1); \
  ULONG _Agetattr_last = (last); \
  ({ \
  register char * _Agetattr__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), ULONG __asm("d0"))) \
  (_Agetattr__bn - 66))(_Agetattr__bn, _Agetattr_par1, _Agetattr_last); \
});})

#define AupdateattrsA(par1, par2, tags) ({ \
  struct Aobject * _AupdateattrsA_par1 = (par1); \
  struct TagItem * _AupdateattrsA_par2 = (par2); \
  struct TagItem * _AupdateattrsA_tags = (tags); \
  ({ \
  register char * _AupdateattrsA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct TagItem * __asm("a1"), struct TagItem * __asm("a2"))) \
  (_AupdateattrsA__bn - 72))(_AupdateattrsA__bn, _AupdateattrsA_par1, _AupdateattrsA_par2, _AupdateattrsA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___Aupdateattrs(struct Library * AwebSupportBase, struct Aobject * par1, struct TagItem * par2, ...)
{
  return AupdateattrsA(par1, par2, (struct TagItem *) ((ULONG) &par2 + sizeof(struct TagItem *)));
}

#define Aupdateattrs(par1, par2...) ___Aupdateattrs(AWEBPLUGIN_BASE_NAME, par1, par2)
#endif

#define Arender(par1, par2, par3, par4, par5, par6, par7, last) ({ \
  struct Aobject * _Arender_par1 = (par1); \
  struct Coords * _Arender_par2 = (par2); \
  LONG _Arender_par3 = (par3); \
  LONG _Arender_par4 = (par4); \
  LONG _Arender_par5 = (par5); \
  LONG _Arender_par6 = (par6); \
  USHORT _Arender_par7 = (par7); \
  APTR _Arender_last = (last); \
  ({ \
  register char * _Arender__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct Coords * __asm("a1"), LONG __asm("d0"), LONG __asm("d1"), LONG __asm("d2"), LONG __asm("d3"), USHORT __asm("d4"), APTR __asm("a2"))) \
  (_Arender__bn - 78))(_Arender__bn, _Arender_par1, _Arender_par2, _Arender_par3, _Arender_par4, _Arender_par5, _Arender_par6, _Arender_par7, _Arender_last); \
});})

#define Aaddchild(par1, par2, last) ({ \
  struct Aobject * _Aaddchild_par1 = (par1); \
  struct Aobject * _Aaddchild_par2 = (par2); \
  ULONG _Aaddchild_last = (last); \
  ({ \
  register char * _Aaddchild__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct Aobject * __asm("a1"), ULONG __asm("d0"))) \
  (_Aaddchild__bn - 84))(_Aaddchild__bn, _Aaddchild_par1, _Aaddchild_par2, _Aaddchild_last); \
});})

#define Aremchild(par1, par2, last) ({ \
  struct Aobject * _Aremchild_par1 = (par1); \
  struct Aobject * _Aremchild_par2 = (par2); \
  ULONG _Aremchild_last = (last); \
  ({ \
  register char * _Aremchild__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct Aobject * __asm("a1"), ULONG __asm("d0"))) \
  (_Aremchild__bn - 90))(_Aremchild__bn, _Aremchild_par1, _Aremchild_par2, _Aremchild_last); \
});})

#define Anotify(par1, last) ({ \
  struct Aobject * _Anotify_par1 = (par1); \
  struct Amessage * _Anotify_last = (last); \
  ({ \
  register char * _Anotify__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct Amessage * __asm("a1"))) \
  (_Anotify__bn - 96))(_Anotify__bn, _Anotify_par1, _Anotify_last); \
});})

#define Allocobject(par1, par2, last) ({ \
  ULONG _Allocobject_par1 = (par1); \
  LONG _Allocobject_par2 = (par2); \
  struct Amset * _Allocobject_last = (last); \
  ({ \
  register char * _Allocobject__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((APTR (*)(char * __asm("a6"), ULONG __asm("d0"), LONG __asm("d1"), struct Amset * __asm("a0"))) \
  (_Allocobject__bn - 102))(_Allocobject__bn, _Allocobject_par1, _Allocobject_par2, _Allocobject_last); \
});})

#define AnotifysetA(par1, tags) ({ \
  struct Aobject * _AnotifysetA_par1 = (par1); \
  struct TagItem * _AnotifysetA_tags = (tags); \
  ({ \
  register char * _AnotifysetA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct TagItem * __asm("a1"))) \
  (_AnotifysetA__bn - 108))(_AnotifysetA__bn, _AnotifysetA_par1, _AnotifysetA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___Anotifyset(struct Library * AwebSupportBase, struct Aobject * par1, ...)
{
  return AnotifysetA(par1, (struct TagItem *) ((ULONG) &par1 + sizeof(struct Aobject *)));
}

#define Anotifyset(par1...) ___Anotifyset(AWEBPLUGIN_BASE_NAME, par1)
#endif

#define Allocmem(par1, last) ({ \
  ULONG _Allocmem_par1 = (par1); \
  ULONG _Allocmem_last = (last); \
  ({ \
  register char * _Allocmem__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((APTR (*)(char * __asm("a6"), ULONG __asm("d0"), ULONG __asm("d1"))) \
  (_Allocmem__bn - 114))(_Allocmem__bn, _Allocmem_par1, _Allocmem_last); \
});})

#define Dupstr(par1, last) ({ \
  STRPTR _Dupstr_par1 = (par1); \
  LONG _Dupstr_last = (last); \
  ({ \
  register char * _Dupstr__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((STRPTR (*)(char * __asm("a6"), STRPTR __asm("a0"), LONG __asm("d0"))) \
  (_Dupstr__bn - 120))(_Dupstr__bn, _Dupstr_par1, _Dupstr_last); \
});})

#define Freemem(last) ({ \
  APTR _Freemem_last = (last); \
  ({ \
  register char * _Freemem__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), APTR __asm("a0"))) \
  (_Freemem__bn - 126))(_Freemem__bn, _Freemem_last); \
});})

#define Clipcoords(par1, last) ({ \
  struct Aobject * _Clipcoords_par1 = (par1); \
  struct Coords * _Clipcoords_last = (last); \
  ({ \
  register char * _Clipcoords__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Coords * (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct Coords * __asm("a1"))) \
  (_Clipcoords__bn - 132))(_Clipcoords__bn, _Clipcoords_par1, _Clipcoords_last); \
});})

#define Unclipcoords(last) ({ \
  struct Coords * _Unclipcoords_last = (last); \
  ({ \
  register char * _Unclipcoords__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Coords * __asm("a0"))) \
  (_Unclipcoords__bn - 138))(_Unclipcoords__bn, _Unclipcoords_last); \
});})

#define Erasebg(par1, par2, par3, par4, par5, last) ({ \
  struct Aobject * _Erasebg_par1 = (par1); \
  struct Coords * _Erasebg_par2 = (par2); \
  LONG _Erasebg_par3 = (par3); \
  LONG _Erasebg_par4 = (par4); \
  LONG _Erasebg_par5 = (par5); \
  LONG _Erasebg_last = (last); \
  ({ \
  register char * _Erasebg__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct Coords * __asm("a1"), LONG __asm("d0"), LONG __asm("d1"), LONG __asm("d2"), LONG __asm("d3"))) \
  (_Erasebg__bn - 144))(_Erasebg__bn, _Erasebg_par1, _Erasebg_par2, _Erasebg_par3, _Erasebg_par4, _Erasebg_par5, _Erasebg_last); \
});})

#define Aweb() ({ \
  register char * _Aweb__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Aobject * (*)(char * __asm("a6"))) \
  (_Aweb__bn - 150))(_Aweb__bn); \
})

#define AsetattrsasyncA(par1, tags) ({ \
  struct Aobject * _AsetattrsasyncA_par1 = (par1); \
  struct TagItem * _AsetattrsasyncA_tags = (tags); \
  ({ \
  register char * _AsetattrsasyncA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct TagItem * __asm("a1"))) \
  (_AsetattrsasyncA__bn - 156))(_AsetattrsasyncA__bn, _AsetattrsasyncA_par1, _AsetattrsasyncA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ void ___Asetattrsasync(struct Library * AwebSupportBase, struct Aobject * par1, ...)
{
  AsetattrsasyncA(par1, (struct TagItem *) ((ULONG) &par1 + sizeof(struct Aobject *)));
}

#define Asetattrsasync(par1...) ___Asetattrsasync(AWEBPLUGIN_BASE_NAME, par1)
#endif

#define Waittask(last) ({ \
  ULONG _Waittask_last = (last); \
  ({ \
  register char * _Waittask__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), ULONG __asm("d0"))) \
  (_Waittask__bn - 162))(_Waittask__bn, _Waittask_last); \
});})

#define Gettaskmsg() ({ \
  register char * _Gettaskmsg__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Taskmsg * (*)(char * __asm("a6"))) \
  (_Gettaskmsg__bn - 168))(_Gettaskmsg__bn); \
})

#define Replytaskmsg(last) ({ \
  struct Taskmsg * _Replytaskmsg_last = (last); \
  ({ \
  register char * _Replytaskmsg__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Taskmsg * __asm("a0"))) \
  (_Replytaskmsg__bn - 174))(_Replytaskmsg__bn, _Replytaskmsg_last); \
});})

#define Checktaskbreak() ({ \
  register char * _Checktaskbreak__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"))) \
  (_Checktaskbreak__bn - 180))(_Checktaskbreak__bn); \
})

#define Updatetask(last) ({ \
  struct Amessage * _Updatetask_last = (last); \
  ({ \
  register char * _Updatetask__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct Amessage * __asm("a0"))) \
  (_Updatetask__bn - 186))(_Updatetask__bn, _Updatetask_last); \
});})

#define UpdatetaskattrsA(tags) ({ \
  struct TagItem * _UpdatetaskattrsA_tags = (tags); \
  ({ \
  register char * _UpdatetaskattrsA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct TagItem * __asm("a0"))) \
  (_UpdatetaskattrsA__bn - 192))(_UpdatetaskattrsA__bn, _UpdatetaskattrsA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___Updatetaskattrs(struct Library * AwebSupportBase, ULONG tags, ...)
{
  return UpdatetaskattrsA((struct TagItem *) &tags);
}

#define Updatetaskattrs(tags...) ___Updatetaskattrs(AWEBPLUGIN_BASE_NAME, tags)
#endif

#define Obtaintasksemaphore(last) ({ \
  struct SignalSemaphore * _Obtaintasksemaphore_last = (last); \
  ({ \
  register char * _Obtaintasksemaphore__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"), struct SignalSemaphore * __asm("a0"))) \
  (_Obtaintasksemaphore__bn - 198))(_Obtaintasksemaphore__bn, _Obtaintasksemaphore_last); \
});})

#define Avprintf(par1, last) ({ \
  char * _Avprintf_par1 = (par1); \
  ULONG * _Avprintf_last = (last); \
  ({ \
  register char * _Avprintf__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), char * __asm("a0"), ULONG * __asm("a1"))) \
  (_Avprintf__bn - 204))(_Avprintf__bn, _Avprintf_par1, _Avprintf_last); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___Aprintf(struct Library * AwebSupportBase, char * par1, ...)
{
  return Avprintf(par1, (ULONG *) ((ULONG) &par1 + sizeof(char *)));
}

#define Aprintf(par1...) ___Aprintf(AWEBPLUGIN_BASE_NAME, par1)
#endif

#define Awebstr(last) ({ \
  ULONG _Awebstr_last = (last); \
  ({ \
  register char * _Awebstr__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((UBYTE * (*)(char * __asm("a6"), ULONG __asm("d0"))) \
  (_Awebstr__bn - 210))(_Awebstr__bn, _Awebstr_last); \
});})

#define TcperrorA(par1, par2, tags) ({ \
  struct Fetchdriver * _TcperrorA_par1 = (par1); \
  ULONG _TcperrorA_par2 = (par2); \
  ULONG * _TcperrorA_tags = (tags); \
  ({ \
  register char * _TcperrorA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Fetchdriver * __asm("a0"), ULONG __asm("d0"), ULONG * __asm("a1"))) \
  (_TcperrorA__bn - 216))(_TcperrorA__bn, _TcperrorA_par1, _TcperrorA_par2, _TcperrorA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ void ___Tcperror(struct Library * AwebSupportBase, struct Fetchdriver * par1, ULONG par2, ...)
{
  TcperrorA(par1, par2, (ULONG *) ((ULONG) &par2 + sizeof(ULONG)));
}

#define Tcperror(par1, par2...) ___Tcperror(AWEBPLUGIN_BASE_NAME, par1, par2)
#endif

#define TcpmessageA(par1, par2, tags) ({ \
  struct Fetchdriver * _TcpmessageA_par1 = (par1); \
  ULONG _TcpmessageA_par2 = (par2); \
  ULONG * _TcpmessageA_tags = (tags); \
  ({ \
  register char * _TcpmessageA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Fetchdriver * __asm("a0"), ULONG __asm("d0"), ULONG * __asm("a1"))) \
  (_TcpmessageA__bn - 222))(_TcpmessageA__bn, _TcpmessageA_par1, _TcpmessageA_par2, _TcpmessageA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ void ___Tcpmessage(struct Library * AwebSupportBase, struct Fetchdriver * par1, ULONG par2, ...)
{
  TcpmessageA(par1, par2, (ULONG *) ((ULONG) &par2 + sizeof(ULONG)));
}

#define Tcpmessage(par1, par2...) ___Tcpmessage(AWEBPLUGIN_BASE_NAME, par1, par2)
#endif

#define Opentcp(par1, par2, last) ({ \
  struct Library ** _Opentcp_par1 = (par1); \
  struct Fetchdriver * _Opentcp_par2 = (par2); \
  BOOL _Opentcp_last = (last); \
  ({ \
  register char * _Opentcp__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Library * (*)(char * __asm("a6"), struct Library ** __asm("a0"), struct Fetchdriver * __asm("a1"), BOOL __asm("d0"))) \
  (_Opentcp__bn - 228))(_Opentcp__bn, _Opentcp_par1, _Opentcp_par2, _Opentcp_last); \
});})

#define Lookup(par1, last) ({ \
  UBYTE * _Lookup_par1 = (par1); \
  struct Library * _Lookup_last = (last); \
  ({ \
  register char * _Lookup__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct hostent * (*)(char * __asm("a6"), UBYTE * __asm("a0"), struct Library * __asm("a1"))) \
  (_Lookup__bn - 234))(_Lookup__bn, _Lookup_par1, _Lookup_last); \
});})

#define Expandbuffer(par1, last) ({ \
  struct Buffer * _Expandbuffer_par1 = (par1); \
  long _Expandbuffer_last = (last); \
  ({ \
  register char * _Expandbuffer__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), struct Buffer * __asm("a0"), long __asm("d0"))) \
  (_Expandbuffer__bn - 240))(_Expandbuffer__bn, _Expandbuffer_par1, _Expandbuffer_last); \
});})

#define Insertinbuffer(par1, par2, par3, last) ({ \
  struct Buffer * _Insertinbuffer_par1 = (par1); \
  UBYTE * _Insertinbuffer_par2 = (par2); \
  long _Insertinbuffer_par3 = (par3); \
  long _Insertinbuffer_last = (last); \
  ({ \
  register char * _Insertinbuffer__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), struct Buffer * __asm("a0"), UBYTE * __asm("a1"), long __asm("d0"), long __asm("d1"))) \
  (_Insertinbuffer__bn - 246))(_Insertinbuffer__bn, _Insertinbuffer_par1, _Insertinbuffer_par2, _Insertinbuffer_par3, _Insertinbuffer_last); \
});})

#define Addtobuffer(par1, par2, last) ({ \
  struct Buffer * _Addtobuffer_par1 = (par1); \
  UBYTE * _Addtobuffer_par2 = (par2); \
  long _Addtobuffer_last = (last); \
  ({ \
  register char * _Addtobuffer__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), struct Buffer * __asm("a0"), UBYTE * __asm("a1"), long __asm("d0"))) \
  (_Addtobuffer__bn - 252))(_Addtobuffer__bn, _Addtobuffer_par1, _Addtobuffer_par2, _Addtobuffer_last); \
});})

#define Deleteinbuffer(par1, par2, last) ({ \
  struct Buffer * _Deleteinbuffer_par1 = (par1); \
  long _Deleteinbuffer_par2 = (par2); \
  long _Deleteinbuffer_last = (last); \
  ({ \
  register char * _Deleteinbuffer__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Buffer * __asm("a0"), long __asm("d0"), long __asm("d1"))) \
  (_Deleteinbuffer__bn - 258))(_Deleteinbuffer__bn, _Deleteinbuffer_par1, _Deleteinbuffer_par2, _Deleteinbuffer_last); \
});})

#define Freebuffer(last) ({ \
  struct Buffer * _Freebuffer_last = (last); \
  ({ \
  register char * _Freebuffer__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Buffer * __asm("a0"))) \
  (_Freebuffer__bn - 264))(_Freebuffer__bn, _Freebuffer_last); \
});})

#define Locale() ({ \
  register char * _Locale__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Locale * (*)(char * __asm("a6"))) \
  (_Locale__bn - 270))(_Locale__bn); \
})

#define Lprintdate(par1, par2, last) ({ \
  UBYTE * _Lprintdate_par1 = (par1); \
  UBYTE * _Lprintdate_par2 = (par2); \
  struct DateStamp * _Lprintdate_last = (last); \
  ({ \
  register char * _Lprintdate__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((long (*)(char * __asm("a6"), UBYTE * __asm("a0"), UBYTE * __asm("a1"), struct DateStamp * __asm("a2"))) \
  (_Lprintdate__bn - 276))(_Lprintdate__bn, _Lprintdate_par1, _Lprintdate_par2, _Lprintdate_last); \
});})

#define Obtainbgrp(par1, par2, par3, par4, par5, last) ({ \
  struct Aobject * _Obtainbgrp_par1 = (par1); \
  struct Coords * _Obtainbgrp_par2 = (par2); \
  LONG _Obtainbgrp_par3 = (par3); \
  LONG _Obtainbgrp_par4 = (par4); \
  LONG _Obtainbgrp_par5 = (par5); \
  LONG _Obtainbgrp_last = (last); \
  ({ \
  register char * _Obtainbgrp__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct RastPort * (*)(char * __asm("a6"), struct Aobject * __asm("a0"), struct Coords * __asm("a1"), LONG __asm("d0"), LONG __asm("d1"), LONG __asm("d2"), LONG __asm("d3"))) \
  (_Obtainbgrp__bn - 282))(_Obtainbgrp__bn, _Obtainbgrp_par1, _Obtainbgrp_par2, _Obtainbgrp_par3, _Obtainbgrp_par4, _Obtainbgrp_par5, _Obtainbgrp_last); \
});})

#define Releasebgrp(last) ({ \
  struct RastPort * _Releasebgrp_last = (last); \
  ({ \
  register char * _Releasebgrp__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct RastPort * __asm("a0"))) \
  (_Releasebgrp__bn - 288))(_Releasebgrp__bn, _Releasebgrp_last); \
});})

#define Setstemvar(par1, par2, par3, par4, last) ({ \
  struct Arexxcmd * _Setstemvar_par1 = (par1); \
  UBYTE * _Setstemvar_par2 = (par2); \
  long _Setstemvar_par3 = (par3); \
  UBYTE * _Setstemvar_par4 = (par4); \
  UBYTE * _Setstemvar_last = (last); \
  ({ \
  register char * _Setstemvar__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Arexxcmd * __asm("a0"), UBYTE * __asm("a1"), long __asm("d0"), UBYTE * __asm("a2"), UBYTE * __asm("a3"))) \
  (_Setstemvar__bn - 294))(_Setstemvar__bn, _Setstemvar_par1, _Setstemvar_par2, _Setstemvar_par3, _Setstemvar_par4, _Setstemvar_last); \
});})

#define Copyprefs(par1, last) ({ \
  struct AwebPrefs * _Copyprefs_par1 = (par1); \
  struct AwebPrefs * _Copyprefs_last = (last); \
  ({ \
  register char * _Copyprefs__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct AwebPrefs * __asm("a0"), struct AwebPrefs * __asm("a1"))) \
  (_Copyprefs__bn - 300))(_Copyprefs__bn, _Copyprefs_par1, _Copyprefs_last); \
});})

#define Saveprefs(last) ({ \
  struct AwebPrefs * _Saveprefs_last = (last); \
  ({ \
  register char * _Saveprefs__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct AwebPrefs * __asm("a0"))) \
  (_Saveprefs__bn - 306))(_Saveprefs__bn, _Saveprefs_last); \
});})

#define Disposeprefs(last) ({ \
  struct AwebPrefs * _Disposeprefs_last = (last); \
  ({ \
  register char * _Disposeprefs__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct AwebPrefs * __asm("a0"))) \
  (_Disposeprefs__bn - 312))(_Disposeprefs__bn, _Disposeprefs_last); \
});})

#define Freemenuentry(last) ({ \
  struct Menuentry * _Freemenuentry_last = (last); \
  ({ \
  register char * _Freemenuentry__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Menuentry * __asm("a0"))) \
  (_Freemenuentry__bn - 318))(_Freemenuentry__bn, _Freemenuentry_last); \
});})

#define Addmenuentry(par1, par2, par3, par4, last) ({ \
  void * _Addmenuentry_par1 = (par1); \
  USHORT _Addmenuentry_par2 = (par2); \
  UBYTE * _Addmenuentry_par3 = (par3); \
  UBYTE _Addmenuentry_par4 = (par4); \
  UBYTE * _Addmenuentry_last = (last); \
  ({ \
  register char * _Addmenuentry__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Menuentry * (*)(char * __asm("a6"), void * __asm("a0"), USHORT __asm("d0"), UBYTE * __asm("a1"), UBYTE __asm("d1"), UBYTE * __asm("a2"))) \
  (_Addmenuentry__bn - 324))(_Addmenuentry__bn, _Addmenuentry_par1, _Addmenuentry_par2, _Addmenuentry_par3, _Addmenuentry_par4, _Addmenuentry_last); \
});})

#define Freemimeinfo(last) ({ \
  struct Mimeinfo * _Freemimeinfo_last = (last); \
  ({ \
  register char * _Freemimeinfo__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Mimeinfo * __asm("a0"))) \
  (_Freemimeinfo__bn - 330))(_Freemimeinfo__bn, _Freemimeinfo_last); \
});})

#define Addmimeinfo(par1, par2, par3, par4, par5, par6, last) ({ \
  void * _Addmimeinfo_par1 = (par1); \
  UBYTE * _Addmimeinfo_par2 = (par2); \
  UBYTE * _Addmimeinfo_par3 = (par3); \
  UBYTE * _Addmimeinfo_par4 = (par4); \
  USHORT _Addmimeinfo_par5 = (par5); \
  UBYTE * _Addmimeinfo_par6 = (par6); \
  UBYTE * _Addmimeinfo_last = (last); \
  ({ \
  register char * _Addmimeinfo__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Mimeinfo * (*)(char * __asm("a6"), void * __asm("a0"), UBYTE * __asm("a1"), UBYTE * __asm("a2"), UBYTE * __asm("a3"), USHORT __asm("d0"), UBYTE * __asm("a4"), UBYTE * __asm("a5"))) \
  (_Addmimeinfo__bn - 336))(_Addmimeinfo__bn, _Addmimeinfo_par1, _Addmimeinfo_par2, _Addmimeinfo_par3, _Addmimeinfo_par4, _Addmimeinfo_par5, _Addmimeinfo_par6, _Addmimeinfo_last); \
});})

#define Freenocache(last) ({ \
  struct Nocache * _Freenocache_last = (last); \
  ({ \
  register char * _Freenocache__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Nocache * __asm("a0"))) \
  (_Freenocache__bn - 342))(_Freenocache__bn, _Freenocache_last); \
});})

#define Addnocache(par1, last) ({ \
  void * _Addnocache_par1 = (par1); \
  UBYTE * _Addnocache_last = (last); \
  ({ \
  register char * _Addnocache__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Nocache * (*)(char * __asm("a6"), void * __asm("a0"), UBYTE * __asm("a1"))) \
  (_Addnocache__bn - 348))(_Addnocache__bn, _Addnocache_par1, _Addnocache_last); \
});})

#define Getstemvar(par1, par2, par3, last) ({ \
  struct Arexxcmd * _Getstemvar_par1 = (par1); \
  UBYTE * _Getstemvar_par2 = (par2); \
  long _Getstemvar_par3 = (par3); \
  UBYTE * _Getstemvar_last = (last); \
  ({ \
  register char * _Getstemvar__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((UBYTE * (*)(char * __asm("a6"), struct Arexxcmd * __asm("a0"), UBYTE * __asm("a1"), long __asm("d0"), UBYTE * __asm("a2"))) \
  (_Getstemvar__bn - 354))(_Getstemvar__bn, _Getstemvar_par1, _Getstemvar_par2, _Getstemvar_par3, _Getstemvar_last); \
});})

#define Freeuserbutton(last) ({ \
  struct Userbutton * _Freeuserbutton_last = (last); \
  ({ \
  register char * _Freeuserbutton__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Userbutton * __asm("a0"))) \
  (_Freeuserbutton__bn - 360))(_Freeuserbutton__bn, _Freeuserbutton_last); \
});})

#define Adduserbutton(par1, par2, last) ({ \
  void * _Adduserbutton_par1 = (par1); \
  UBYTE * _Adduserbutton_par2 = (par2); \
  UBYTE * _Adduserbutton_last = (last); \
  ({ \
  register char * _Adduserbutton__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Userbutton * (*)(char * __asm("a6"), void * __asm("a0"), UBYTE * __asm("a1"), UBYTE * __asm("a2"))) \
  (_Adduserbutton__bn - 366))(_Adduserbutton__bn, _Adduserbutton_par1, _Adduserbutton_par2, _Adduserbutton_last); \
});})

#define Freepopupitem(last) ({ \
  struct Popupitem * _Freepopupitem_last = (last); \
  ({ \
  register char * _Freepopupitem__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Popupitem * __asm("a0"))) \
  (_Freepopupitem__bn - 372))(_Freepopupitem__bn, _Freepopupitem_last); \
});})

#define Addpopupitem(par1, par2, par3, last) ({ \
  void * _Addpopupitem_par1 = (par1); \
  USHORT _Addpopupitem_par2 = (par2); \
  UBYTE * _Addpopupitem_par3 = (par3); \
  UBYTE * _Addpopupitem_last = (last); \
  ({ \
  register char * _Addpopupitem__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Popupitem * (*)(char * __asm("a6"), void * __asm("a0"), USHORT __asm("d0"), UBYTE * __asm("a1"), UBYTE * __asm("a2"))) \
  (_Addpopupitem__bn - 378))(_Addpopupitem__bn, _Addpopupitem_par1, _Addpopupitem_par2, _Addpopupitem_par3, _Addpopupitem_last); \
});})

#define Awebversion() ({ \
  register char * _Awebversion__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((UBYTE * (*)(char * __asm("a6"))) \
  (_Awebversion__bn - 384))(_Awebversion__bn); \
})

#define Syncrequest(par1, par2, last) ({ \
  UBYTE * _Syncrequest_par1 = (par1); \
  UBYTE * _Syncrequest_par2 = (par2); \
  UBYTE * _Syncrequest_last = (last); \
  ({ \
  register char * _Syncrequest__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((long (*)(char * __asm("a6"), UBYTE * __asm("a0"), UBYTE * __asm("a1"), UBYTE * __asm("a2"))) \
  (_Syncrequest__bn - 390))(_Syncrequest__bn, _Syncrequest_par1, _Syncrequest_par2, _Syncrequest_last); \
});})

#define PprintfA(par1, par2, tags) ({ \
  UBYTE * _PprintfA_par1 = (par1); \
  UBYTE * _PprintfA_par2 = (par2); \
  UBYTE ** _PprintfA_tags = (tags); \
  ({ \
  register char * _PprintfA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((UBYTE * (*)(char * __asm("a6"), UBYTE * __asm("a0"), UBYTE * __asm("a1"), UBYTE ** __asm("a2"))) \
  (_PprintfA__bn - 396))(_PprintfA__bn, _PprintfA_par1, _PprintfA_par2, _PprintfA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ UBYTE * ___Pprintf(struct Library * AwebSupportBase, UBYTE * par1, UBYTE * par2, ...)
{
  return PprintfA(par1, par2, (UBYTE **) ((ULONG) &par2 + sizeof(UBYTE *)));
}

#define Pprintf(par1, par2...) ___Pprintf(AWEBPLUGIN_BASE_NAME, par1, par2)
#endif

#define Today() ({ \
  register char * _Today__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((ULONG (*)(char * __asm("a6"))) \
  (_Today__bn - 402))(_Today__bn); \
})

#define Freeuserkey(last) ({ \
  struct Userkey * _Freeuserkey_last = (last); \
  ({ \
  register char * _Freeuserkey__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Userkey * __asm("a0"))) \
  (_Freeuserkey__bn - 408))(_Freeuserkey__bn, _Freeuserkey_last); \
});})

#define Adduserkey(par1, par2, last) ({ \
  void * _Adduserkey_par1 = (par1); \
  USHORT _Adduserkey_par2 = (par2); \
  UBYTE * _Adduserkey_last = (last); \
  ({ \
  register char * _Adduserkey__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Userkey * (*)(char * __asm("a6"), void * __asm("a0"), USHORT __asm("d0"), UBYTE * __asm("a1"))) \
  (_Adduserkey__bn - 414))(_Adduserkey__bn, _Adduserkey_par1, _Adduserkey_par2, _Adduserkey_last); \
});})

#define LprintfA(par1, par2, tags) ({ \
  UBYTE * _LprintfA_par1 = (par1); \
  UBYTE * _LprintfA_par2 = (par2); \
  void * _LprintfA_tags = (tags); \
  ({ \
  register char * _LprintfA__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((long (*)(char * __asm("a6"), UBYTE * __asm("a0"), UBYTE * __asm("a1"), void * __asm("a2"))) \
  (_LprintfA__bn - 420))(_LprintfA__bn, _LprintfA_par1, _LprintfA_par2, _LprintfA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ long ___Lprintf(struct Library * AwebSupportBase, UBYTE * par1, UBYTE * par2, ...)
{
  return LprintfA(par1, par2, (void *) ((ULONG) &par2 + sizeof(UBYTE *)));
}

#define Lprintf(par1, par2...) ___Lprintf(AWEBPLUGIN_BASE_NAME, par1, par2)
#endif

#define Fullversion() ({ \
  register char * _Fullversion__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"))) \
  (_Fullversion__bn - 426))(_Fullversion__bn); \
})

#define Setfiltertype(par1, last) ({ \
  void * _Setfiltertype_par1 = (par1); \
  UBYTE * _Setfiltertype_last = (last); \
  ({ \
  register char * _Setfiltertype__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), void * __asm("a0"), UBYTE * __asm("a1"))) \
  (_Setfiltertype__bn - 432))(_Setfiltertype__bn, _Setfiltertype_par1, _Setfiltertype_last); \
});})

#define Writefilter(par1, par2, last) ({ \
  void * _Writefilter_par1 = (par1); \
  UBYTE * _Writefilter_par2 = (par2); \
  long _Writefilter_last = (last); \
  ({ \
  register char * _Writefilter__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), void * __asm("a0"), UBYTE * __asm("a1"), long __asm("d0"))) \
  (_Writefilter__bn - 438))(_Writefilter__bn, _Writefilter_par1, _Writefilter_par2, _Writefilter_last); \
});})

#define Awebcommand(par1, par2, par3, last) ({ \
  long _Awebcommand_par1 = (par1); \
  UBYTE * _Awebcommand_par2 = (par2); \
  UBYTE * _Awebcommand_par3 = (par3); \
  long _Awebcommand_last = (last); \
  ({ \
  register char * _Awebcommand__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((long (*)(char * __asm("a6"), long __asm("d0"), UBYTE * __asm("a0"), UBYTE * __asm("a1"), long __asm("d1"))) \
  (_Awebcommand__bn - 444))(_Awebcommand__bn, _Awebcommand_par1, _Awebcommand_par2, _Awebcommand_par3, _Awebcommand_last); \
});})

#define Awebactive() ({ \
  register char * _Awebactive__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"))) \
  (_Awebactive__bn - 450))(_Awebactive__bn); \
})

#define Freefontalias(last) ({ \
  struct Fontalias * _Freefontalias_last = (last); \
  ({ \
  register char * _Freefontalias__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((void (*)(char * __asm("a6"), struct Fontalias * __asm("a0"))) \
  (_Freefontalias__bn - 456))(_Freefontalias__bn, _Freefontalias_last); \
});})

#define Addfontalias(par1, last) ({ \
  void * _Addfontalias_par1 = (par1); \
  UBYTE * _Addfontalias_last = (last); \
  ({ \
  register char * _Addfontalias__bn __asm("a6") = (char *) (AWEBPLUGIN_BASE_NAME);\
  ((struct Fontalias * (*)(char * __asm("a6"), void * __asm("a0"), UBYTE * __asm("a1"))) \
  (_Addfontalias__bn - 462))(_Addfontalias__bn, _Addfontalias_par1, _Addfontalias_last); \
});})

#endif /*  _INLINE_AWEBSUPPORT_H  */
