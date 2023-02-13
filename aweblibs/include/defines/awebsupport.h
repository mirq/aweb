/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSUPPORT_H
#define _INLINE_AWEBSUPPORT_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef AWEBSUPPORT_BASE_NAME
#define AWEBSUPPORT_BASE_NAME AwebSupportBase
#endif /* !AWEBSUPPORT_BASE_NAME */

#define AmethodA(___par1, ___tags) __AmethodA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AmethodA_WB(___base, ___par1, ___tags) \
   AROS_LC2(ULONG, AmethodA, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct Amessage *, (___tags), A1), \
   struct Library *, (___base), 5, Awebsupport)

#ifndef NO_INLINE_VARARGS
#define Amethod(___par1, ___dummy, ...) __Amethod_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Amethod_WB(___base, ___par1, ___dummy, ...) \
   ({APTR _message[] = { ___dummy, ## __VA_ARGS__ }; __AmethodA_WB((___base), (___par1), (struct Amessage *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define AmethodasA(___par1, ___par2, ___tags) __AmethodasA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __AmethodasA_WB(___base, ___par1, ___par2, ___tags) \
   AROS_LC3(ULONG, AmethodasA, \
   AROS_LCA(ULONG, (___par1), D0), \
   AROS_LCA(struct Aobject *, (___par2), A0), \
   AROS_LCA(struct Amessage *, (___tags), A1), \
   struct Library *, (___base), 6, Awebsupport)

#ifndef NO_INLINE_VARARGS
#define Amethodas(___par1, ___par2, ___dummy, ...) __Amethodas_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___dummy, ## __VA_ARGS__)
#define __Amethodas_WB(___base, ___par1, ___par2, ___dummy, ...) \
   ({APTR _message[] = { ___dummy, ## __VA_ARGS__ }; __AmethodasA_WB((___base), (___par1), (___par2), (struct Amessage *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define AnewobjectA(___par1, ___tags) __AnewobjectA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AnewobjectA_WB(___base, ___par1, ___tags) \
   AROS_LC2(APTR, AnewobjectA, \
   AROS_LCA(ULONG, (___par1), D0), \
   AROS_LCA(struct TagItem *, (___tags), A0), \
   struct Library *, (___base), 7, Awebsupport)

#ifndef NO_INLINE_STDARG
#define Anewobject(___par1, ___dummy, ...) __Anewobject_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Anewobject_WB(___base, ___par1, ___dummy, ...) \
   ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AnewobjectA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Adisposeobject(___last) __Adisposeobject_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Adisposeobject_WB(___base, ___last) \
   AROS_LC1(void, Adisposeobject, \
   AROS_LCA(struct Aobject *, (___last), A0), \
   struct Library *, (___base), 8, Awebsupport)

#define AsetattrsA(___par1, ___tags) __AsetattrsA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AsetattrsA_WB(___base, ___par1, ___tags) \
   AROS_LC2(ULONG, AsetattrsA, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct TagItem *, (___tags), A1), \
   struct Library *, (___base), 9, Awebsupport)

#ifndef NO_INLINE_STDARG
#define Asetattrs(___par1, ___dummy, ...) __Asetattrs_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Asetattrs_WB(___base, ___par1, ___dummy, ...) \
   ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AsetattrsA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define AgetattrsA(___par1, ___tags) __AgetattrsA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AgetattrsA_WB(___base, ___par1, ___tags) \
   AROS_LC2(ULONG, AgetattrsA, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct TagItem *, (___tags), A1), \
   struct Library *, (___base), 10, Awebsupport)

#ifndef NO_INLINE_STDARG
#define Agetattrs(___par1, ___dummy, ...) __Agetattrs_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Agetattrs_WB(___base, ___par1, ___dummy, ...) \
   ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AgetattrsA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Agetattr(___par1, ___last) __Agetattr_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Agetattr_WB(___base, ___par1, ___last) \
   AROS_LC2(ULONG, Agetattr, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(ULONG, (___last), D0), \
   struct Library *, (___base), 11, Awebsupport)

#define AupdateattrsA(___par1, ___par2, ___tags) __AupdateattrsA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __AupdateattrsA_WB(___base, ___par1, ___par2, ___tags) \
   AROS_LC3(ULONG, AupdateattrsA, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct TagItem *, (___par2), A1), \
   AROS_LCA(struct TagItem *, (___tags), A2), \
   struct Library *, (___base), 12, Awebsupport)

#ifndef NO_INLINE_STDARG
#define Aupdateattrs(___par1, ___par2, ___dummy, ...) __Aupdateattrs_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___dummy, ## __VA_ARGS__)
#define __Aupdateattrs_WB(___base, ___par1, ___par2, ___dummy, ...) \
   ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AupdateattrsA_WB((___base), (___par1), (___par2), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Arender(___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___par7, ___last) __Arender_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___par7, ___last)
#define __Arender_WB(___base, ___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___par7, ___last) \
   AROS_LC8(ULONG, Arender, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct Coords *, (___par2), A1), \
   AROS_LCA(LONG, (___par3), D0), \
   AROS_LCA(LONG, (___par4), D1), \
   AROS_LCA(LONG, (___par5), D2), \
   AROS_LCA(LONG, (___par6), D3), \
   AROS_LCA(USHORT, (___par7), D4), \
   AROS_LCA(APTR, (___last), A2), \
   struct Library *, (___base), 13, Awebsupport)

#define Aaddchild(___par1, ___par2, ___last) __Aaddchild_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Aaddchild_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(ULONG, Aaddchild, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct Aobject *, (___par2), A1), \
   AROS_LCA(ULONG, (___last), D0), \
   struct Library *, (___base), 14, Awebsupport)

#define Aremchild(___par1, ___par2, ___last) __Aremchild_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Aremchild_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(ULONG, Aremchild, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct Aobject *, (___par2), A1), \
   AROS_LCA(ULONG, (___last), D0), \
   struct Library *, (___base), 15, Awebsupport)

#define Anotify(___par1, ___last) __Anotify_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Anotify_WB(___base, ___par1, ___last) \
   AROS_LC2(ULONG, Anotify, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct Amessage *, (___last), A1), \
   struct Library *, (___base), 16, Awebsupport)

#define Allocobject(___par1, ___par2, ___last) __Allocobject_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Allocobject_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(APTR, Allocobject, \
   AROS_LCA(ULONG, (___par1), D0), \
   AROS_LCA(LONG, (___par2), D1), \
   AROS_LCA(struct Amset *, (___last), A0), \
   struct Library *, (___base), 17, Awebsupport)

#define AnotifysetA(___par1, ___tags) __AnotifysetA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AnotifysetA_WB(___base, ___par1, ___tags) \
   AROS_LC2(ULONG, AnotifysetA, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct TagItem *, (___tags), A1), \
   struct Library *, (___base), 18, Awebsupport)

#ifndef NO_INLINE_STDARG
#define Anotifyset(___par1, ___dummy, ...) __Anotifyset_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Anotifyset_WB(___base, ___par1, ___dummy, ...) \
   ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AnotifysetA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Allocmem(___par1, ___last) __Allocmem_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Allocmem_WB(___base, ___par1, ___last) \
   AROS_LC2(APTR, Allocmem, \
   AROS_LCA(ULONG, (___par1), D0), \
   AROS_LCA(ULONG, (___last), D1), \
   struct Library *, (___base), 19, Awebsupport)

#define Dupstr(___par1, ___last) __Dupstr_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Dupstr_WB(___base, ___par1, ___last) \
   AROS_LC2(STRPTR, Dupstr, \
   AROS_LCA(STRPTR, (___par1), A0), \
   AROS_LCA(LONG, (___last), D0), \
   struct Library *, (___base), 20, Awebsupport)

#define Freemem(___last) __Freemem_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freemem_WB(___base, ___last) \
   AROS_LC1(void, Freemem, \
   AROS_LCA(APTR, (___last), A0), \
   struct Library *, (___base), 21, Awebsupport)

#define Clipcoords(___par1, ___last) __Clipcoords_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Clipcoords_WB(___base, ___par1, ___last) \
   AROS_LC2(struct Coords *, Clipcoords, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct Coords *, (___last), A1), \
   struct Library *, (___base), 22, Awebsupport)

#define Unclipcoords(___last) __Unclipcoords_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Unclipcoords_WB(___base, ___last) \
   AROS_LC1(void, Unclipcoords, \
   AROS_LCA(struct Coords *, (___last), A0), \
   struct Library *, (___base), 23, Awebsupport)

#define Erasebg(___par1, ___par2, ___par3, ___par4, ___par5, ___last) __Erasebg_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___par5, ___last)
#define __Erasebg_WB(___base, ___par1, ___par2, ___par3, ___par4, ___par5, ___last) \
   AROS_LC6(void, Erasebg, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct Coords *, (___par2), A1), \
   AROS_LCA(LONG, (___par3), D0), \
   AROS_LCA(LONG, (___par4), D1), \
   AROS_LCA(LONG, (___par5), D2), \
   AROS_LCA(LONG, (___last), D3), \
   struct Library *, (___base), 24, Awebsupport)

#define Aweb() __Aweb_WB(AWEBSUPPORT_BASE_NAME)
#define __Aweb_WB(___base) \
   AROS_LC0(struct Aobject *, Aweb, \
   struct Library *, (___base), 25, Awebsupport)

#define AsetattrsasyncA(___par1, ___tags) __AsetattrsasyncA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AsetattrsasyncA_WB(___base, ___par1, ___tags) \
   AROS_LC2(void, AsetattrsasyncA, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct TagItem *, (___tags), A1), \
   struct Library *, (___base), 26, Awebsupport)

#ifndef NO_INLINE_STDARG
#define Asetattrsasync(___par1, ___dummy, ...) __Asetattrsasync_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Asetattrsasync_WB(___base, ___par1, ___dummy, ...) \
   ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AsetattrsasyncA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Waittask(___last) __Waittask_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Waittask_WB(___base, ___last) \
   AROS_LC1(ULONG, Waittask, \
   AROS_LCA(ULONG, (___last), D0), \
   struct Library *, (___base), 27, Awebsupport)

#define Gettaskmsg() __Gettaskmsg_WB(AWEBSUPPORT_BASE_NAME)
#define __Gettaskmsg_WB(___base) \
   AROS_LC0(struct Taskmsg *, Gettaskmsg, \
   struct Library *, (___base), 28, Awebsupport)

#define Replytaskmsg(___last) __Replytaskmsg_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Replytaskmsg_WB(___base, ___last) \
   AROS_LC1(void, Replytaskmsg, \
   AROS_LCA(struct Taskmsg *, (___last), A0), \
   struct Library *, (___base), 29, Awebsupport)

#define Checktaskbreak() __Checktaskbreak_WB(AWEBSUPPORT_BASE_NAME)
#define __Checktaskbreak_WB(___base) \
   AROS_LC0(ULONG, Checktaskbreak, \
   struct Library *, (___base), 30, Awebsupport)

#define Updatetask(___last) __Updatetask_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Updatetask_WB(___base, ___last) \
   AROS_LC1(ULONG, Updatetask, \
   AROS_LCA(struct Amessage *, (___last), A0), \
   struct Library *, (___base), 31, Awebsupport)

#define UpdatetaskattrsA(___tags) __UpdatetaskattrsA_WB(AWEBSUPPORT_BASE_NAME, ___tags)
#define __UpdatetaskattrsA_WB(___base, ___tags) \
   AROS_LC1(ULONG, UpdatetaskattrsA, \
   AROS_LCA(struct TagItem *, (___tags), A0), \
   struct Library *, (___base), 32, Awebsupport)

#ifndef NO_INLINE_STDARG
#define Updatetaskattrs(___tags, ...) __Updatetaskattrs_WB(AWEBSUPPORT_BASE_NAME, ___tags, ## __VA_ARGS__)
#define __Updatetaskattrs_WB(___base, ___tags, ...) \
   ({APTR _tags[] = { ___tags, ## __VA_ARGS__ }; __UpdatetaskattrsA_WB((___base), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Obtaintasksemaphore(___last) __Obtaintasksemaphore_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Obtaintasksemaphore_WB(___base, ___last) \
   AROS_LC1(ULONG, Obtaintasksemaphore, \
   AROS_LCA(struct SignalSemaphore *, (___last), A0), \
   struct Library *, (___base), 33, Awebsupport)

#define Avprintf(___par1, ___last) __Avprintf_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Avprintf_WB(___base, ___par1, ___last) \
   AROS_LC2(LONG, Avprintf, \
   AROS_LCA(char *, (___par1), A0), \
   AROS_LCA(ULONG *, (___last), A1), \
   struct Library *, (___base), 34, Awebsupport)

#ifndef NO_INLINE_VARARGS
#define Aprintf(___par1, ...) __Aprintf_WB(AWEBSUPPORT_BASE_NAME, ___par1, ## __VA_ARGS__)
#define __Aprintf_WB(___base, ___par1, ...) \
   ({APTR _args[] = { __VA_ARGS__ }; __Avprintf_WB((___base), (___par1), (ULONG *) _args); })
#endif /* !NO_INLINE_VARARGS */

#define Awebstr(___last) __Awebstr_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Awebstr_WB(___base, ___last) \
   AROS_LC1(UBYTE *, Awebstr, \
   AROS_LCA(ULONG, (___last), D0), \
   struct Library *, (___base), 35, Awebsupport)

#define TcperrorA(___par1, ___par2, ___tags) __TcperrorA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __TcperrorA_WB(___base, ___par1, ___par2, ___tags) \
   AROS_LC3(void, TcperrorA, \
   AROS_LCA(struct Fetchdriver *, (___par1), A0), \
   AROS_LCA(ULONG, (___par2), D0), \
   AROS_LCA(ULONG *, (___tags), A1), \
   struct Library *, (___base), 36, Awebsupport)

#ifndef NO_INLINE_VARARGS
#define Tcperror(___par1, ___par2, ...) __Tcperror_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ## __VA_ARGS__)
#define __Tcperror_WB(___base, ___par1, ___par2, ...) \
   ({APTR _args[] = { __VA_ARGS__ }; __TcperrorA_WB((___base), (___par1), (___par2), (ULONG *) _args); })
#endif /* !NO_INLINE_VARARGS */

#define TcpmessageA(___par1, ___par2, ___tags) __TcpmessageA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __TcpmessageA_WB(___base, ___par1, ___par2, ___tags) \
   AROS_LC3(void, TcpmessageA, \
   AROS_LCA(struct Fetchdriver *, (___par1), A0), \
   AROS_LCA(ULONG, (___par2), D0), \
   AROS_LCA(ULONG *, (___tags), A1), \
   struct Library *, (___base), 37, Awebsupport)

#ifndef NO_INLINE_VARARGS
#define Tcpmessage(___par1, ___par2, ...) __Tcpmessage_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ## __VA_ARGS__)
#define __Tcpmessage_WB(___base, ___par1, ___par2, ...) \
   ({APTR _args[] = { __VA_ARGS__ }; __TcpmessageA_WB((___base), (___par1), (___par2), (ULONG *) _args); })
#endif /* !NO_INLINE_VARARGS */

#define Opentcp(___par1, ___par2, ___last) __Opentcp_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Opentcp_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(struct Library *, Opentcp, \
   AROS_LCA(struct Library **, (___par1), A0), \
   AROS_LCA(struct Fetchdriver *, (___par2), A1), \
   AROS_LCA(BOOL, (___last), D0), \
   struct Library *, (___base), 38, Awebsupport)

#define Lookup(___par1, ___last) __Lookup_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Lookup_WB(___base, ___par1, ___last) \
   AROS_LC2(struct hostent *, Lookup, \
   AROS_LCA(UBYTE *, (___par1), A0), \
   AROS_LCA(struct Library *, (___last), A1), \
   struct Library *, (___base), 39, Awebsupport)

#define Expandbuffer(___par1, ___last) __Expandbuffer_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Expandbuffer_WB(___base, ___par1, ___last) \
   AROS_LC2(BOOL, Expandbuffer, \
   AROS_LCA(struct Buffer *, (___par1), A0), \
   AROS_LCA(long, (___last), D0), \
   struct Library *, (___base), 40, Awebsupport)

#define Insertinbuffer(___par1, ___par2, ___par3, ___last) __Insertinbuffer_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___last)
#define __Insertinbuffer_WB(___base, ___par1, ___par2, ___par3, ___last) \
   AROS_LC4(BOOL, Insertinbuffer, \
   AROS_LCA(struct Buffer *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(long, (___par3), D0), \
   AROS_LCA(long, (___last), D1), \
   struct Library *, (___base), 41, Awebsupport)

#define Addtobuffer(___par1, ___par2, ___last) __Addtobuffer_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Addtobuffer_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(BOOL, Addtobuffer, \
   AROS_LCA(struct Buffer *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(long, (___last), D0), \
   struct Library *, (___base), 42, Awebsupport)

#define Deleteinbuffer(___par1, ___par2, ___last) __Deleteinbuffer_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Deleteinbuffer_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(void, Deleteinbuffer, \
   AROS_LCA(struct Buffer *, (___par1), A0), \
   AROS_LCA(long, (___par2), D0), \
   AROS_LCA(long, (___last), D1), \
   struct Library *, (___base), 43, Awebsupport)

#define Freebuffer(___last) __Freebuffer_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freebuffer_WB(___base, ___last) \
   AROS_LC1(void, Freebuffer, \
   AROS_LCA(struct Buffer *, (___last), A0), \
   struct Library *, (___base), 44, Awebsupport)

#define Locale() __Locale_WB(AWEBSUPPORT_BASE_NAME)
#define __Locale_WB(___base) \
   AROS_LC0(struct Locale *, Locale, \
   struct Library *, (___base), 45, Awebsupport)

#define Lprintdate(___par1, ___par2, ___last) __Lprintdate_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Lprintdate_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(long, Lprintdate, \
   AROS_LCA(UBYTE *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(struct DateStamp *, (___last), A2), \
   struct Library *, (___base), 46, Awebsupport)

#define Obtainbgrp(___par1, ___par2, ___par3, ___par4, ___par5, ___last) __Obtainbgrp_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___par5, ___last)
#define __Obtainbgrp_WB(___base, ___par1, ___par2, ___par3, ___par4, ___par5, ___last) \
   AROS_LC6(struct RastPort *, Obtainbgrp, \
   AROS_LCA(struct Aobject *, (___par1), A0), \
   AROS_LCA(struct Coords *, (___par2), A1), \
   AROS_LCA(LONG, (___par3), D0), \
   AROS_LCA(LONG, (___par4), D1), \
   AROS_LCA(LONG, (___par5), D2), \
   AROS_LCA(LONG, (___last), D3), \
   struct Library *, (___base), 47, Awebsupport)

#define Releasebgrp(___last) __Releasebgrp_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Releasebgrp_WB(___base, ___last) \
   AROS_LC1(void, Releasebgrp, \
   AROS_LCA(struct RastPort *, (___last), A0), \
   struct Library *, (___base), 48, Awebsupport)

#define Setstemvar(___par1, ___par2, ___par3, ___par4, ___last) __Setstemvar_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___last)
#define __Setstemvar_WB(___base, ___par1, ___par2, ___par3, ___par4, ___last) \
   AROS_LC5(void, Setstemvar, \
   AROS_LCA(struct Arexxcmd *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(long, (___par3), D0), \
   AROS_LCA(UBYTE *, (___par4), A2), \
   AROS_LCA(UBYTE *, (___last), A3), \
   struct Library *, (___base), 49, Awebsupport)

#define Copyprefs(___par1, ___last) __Copyprefs_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Copyprefs_WB(___base, ___par1, ___last) \
   AROS_LC2(void, Copyprefs, \
   AROS_LCA(struct AwebPrefs *, (___par1), A0), \
   AROS_LCA(struct AwebPrefs *, (___last), A1), \
   struct Library *, (___base), 50, Awebsupport)

#define Saveprefs(___last) __Saveprefs_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Saveprefs_WB(___base, ___last) \
   AROS_LC1(void, Saveprefs, \
   AROS_LCA(struct AwebPrefs *, (___last), A0), \
   struct Library *, (___base), 51, Awebsupport)

#define Disposeprefs(___last) __Disposeprefs_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Disposeprefs_WB(___base, ___last) \
   AROS_LC1(void, Disposeprefs, \
   AROS_LCA(struct AwebPrefs *, (___last), A0), \
   struct Library *, (___base), 52, Awebsupport)

#define Freemenuentry(___last) __Freemenuentry_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freemenuentry_WB(___base, ___last) \
   AROS_LC1(void, Freemenuentry, \
   AROS_LCA(struct Menuentry *, (___last), A0), \
   struct Library *, (___base), 53, Awebsupport)

#define Addmenuentry(___par1, ___par2, ___par3, ___par4, ___last) __Addmenuentry_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___last)
#define __Addmenuentry_WB(___base, ___par1, ___par2, ___par3, ___par4, ___last) \
   AROS_LC5(struct Menuentry *, Addmenuentry, \
   AROS_LCA(void *, (___par1), A0), \
   AROS_LCA(USHORT, (___par2), D0), \
   AROS_LCA(UBYTE *, (___par3), A1), \
   AROS_LCA(UBYTE, (___par4), D1), \
   AROS_LCA(UBYTE *, (___last), A2), \
   struct Library *, (___base), 54, Awebsupport)

#define Freemimeinfo(___last) __Freemimeinfo_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freemimeinfo_WB(___base, ___last) \
   AROS_LC1(void, Freemimeinfo, \
   AROS_LCA(struct Mimeinfo *, (___last), A0), \
   struct Library *, (___base), 55, Awebsupport)

#define Addmimeinfo(___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___last) __Addmimeinfo_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___last)
#define __Addmimeinfo_WB(___base, ___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___last) \
   AROS_LC7(struct Mimeinfo *, Addmimeinfo, \
   AROS_LCA(void *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(UBYTE *, (___par3), A2), \
   AROS_LCA(UBYTE *, (___par4), A3), \
   AROS_LCA(USHORT, (___par5), D0), \
   AROS_LCA(UBYTE *, (___par6), A4), \
   AROS_LCA(UBYTE *, (___last), A5), \
   struct Library *, (___base), 56, Awebsupport)

#define Freenocache(___last) __Freenocache_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freenocache_WB(___base, ___last) \
   AROS_LC1(void, Freenocache, \
   AROS_LCA(struct Nocache *, (___last), A0), \
   struct Library *, (___base), 57, Awebsupport)

#define Addnocache(___par1, ___last) __Addnocache_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Addnocache_WB(___base, ___par1, ___last) \
   AROS_LC2(struct Nocache *, Addnocache, \
   AROS_LCA(void *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___last), A1), \
   struct Library *, (___base), 58, Awebsupport)

#define Getstemvar(___par1, ___par2, ___par3, ___last) __Getstemvar_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___last)
#define __Getstemvar_WB(___base, ___par1, ___par2, ___par3, ___last) \
   AROS_LC4(UBYTE *, Getstemvar, \
   AROS_LCA(struct Arexxcmd *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(long, (___par3), D0), \
   AROS_LCA(UBYTE *, (___last), A2), \
   struct Library *, (___base), 59, Awebsupport)

#define Freeuserbutton(___last) __Freeuserbutton_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freeuserbutton_WB(___base, ___last) \
   AROS_LC1(void, Freeuserbutton, \
   AROS_LCA(struct Userbutton *, (___last), A0), \
   struct Library *, (___base), 60, Awebsupport)

#define Adduserbutton(___par1, ___par2, ___last) __Adduserbutton_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Adduserbutton_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(struct Userbutton *, Adduserbutton, \
   AROS_LCA(void *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(UBYTE *, (___last), A2), \
   struct Library *, (___base), 61, Awebsupport)

#define Freepopupitem(___last) __Freepopupitem_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freepopupitem_WB(___base, ___last) \
   AROS_LC1(void, Freepopupitem, \
   AROS_LCA(struct Popupitem *, (___last), A0), \
   struct Library *, (___base), 62, Awebsupport)

#define Addpopupitem(___par1, ___par2, ___par3, ___last) __Addpopupitem_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___last)
#define __Addpopupitem_WB(___base, ___par1, ___par2, ___par3, ___last) \
   AROS_LC4(struct Popupitem *, Addpopupitem, \
   AROS_LCA(void *, (___par1), A0), \
   AROS_LCA(USHORT, (___par2), D0), \
   AROS_LCA(UBYTE *, (___par3), A1), \
   AROS_LCA(UBYTE *, (___last), A2), \
   struct Library *, (___base), 63, Awebsupport)

#define Awebversion() __Awebversion_WB(AWEBSUPPORT_BASE_NAME)
#define __Awebversion_WB(___base) \
   AROS_LC0(UBYTE *, Awebversion, \
   struct Library *, (___base), 64, Awebsupport)

#define Syncrequest(___par1, ___par2, ___last) __Syncrequest_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Syncrequest_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(long, Syncrequest, \
   AROS_LCA(UBYTE *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(UBYTE *, (___last), A2), \
   struct Library *, (___base), 65, Awebsupport)

#define PprintfA(___par1, ___par2, ___tags) __PprintfA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __PprintfA_WB(___base, ___par1, ___par2, ___tags) \
   AROS_LC3(UBYTE *, PprintfA, \
   AROS_LCA(UBYTE *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(UBYTE **, (___tags), A2), \
   struct Library *, (___base), 66, Awebsupport)

#ifndef NO_INLINE_VARARGS
#define Pprintf(___par1, ___par2, ___dummy, ...) __Pprintf_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___dummy, ## __VA_ARGS__)
#define __Pprintf_WB(___base, ___par1, ___par2, ___dummy, ...) \
   ({APTR _message[] = { ___dummy, ## __VA_ARGS__ }; __PprintfA_WB((___base), (___par1), (___par2), (UBYTE **) _message); })
#endif /* !NO_INLINE_VARARGS */

#define Today() __Today_WB(AWEBSUPPORT_BASE_NAME)
#define __Today_WB(___base) \
   AROS_LC0(ULONG, Today, \
   struct Library *, (___base), 67, Awebsupport)

#define Freeuserkey(___last) __Freeuserkey_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freeuserkey_WB(___base, ___last) \
   AROS_LC1(void, Freeuserkey, \
   AROS_LCA(struct Userkey *, (___last), A0), \
   struct Library *, (___base), 68, Awebsupport)

#define Adduserkey(___par1, ___par2, ___last) __Adduserkey_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Adduserkey_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(struct Userkey *, Adduserkey, \
   AROS_LCA(void *, (___par1), A0), \
   AROS_LCA(USHORT, (___par2), D0), \
   AROS_LCA(UBYTE *, (___last), A1), \
   struct Library *, (___base), 69, Awebsupport)

#define LprintfA(___par1, ___par2, ___tags) __LprintfA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __LprintfA_WB(___base, ___par1, ___par2, ___tags) \
   AROS_LC3(long, LprintfA, \
   AROS_LCA(UBYTE *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(void *, (___tags), A2), \
   struct Library *, (___base), 70, Awebsupport)

#ifndef NO_INLINE_VARARGS
#define Lprintf(___par1, ___par2, ___dummy, ...) __Lprintf_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___dummy, ## __VA_ARGS__)
#define __Lprintf_WB(___base, ___par1, ___par2, ___dummy, ...) \
   ({APTR _message[] = { ___dummy, ## __VA_ARGS__ }; __LprintfA_WB((___base), (___par1), (___par2), (void *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define Fullversion() __Fullversion_WB(AWEBSUPPORT_BASE_NAME)
#define __Fullversion_WB(___base) \
   AROS_LC0(BOOL, Fullversion, \
   struct Library *, (___base), 71, Awebsupport)

#define Setfiltertype(___par1, ___last) __Setfiltertype_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Setfiltertype_WB(___base, ___par1, ___last) \
   AROS_LC2(void, Setfiltertype, \
   AROS_LCA(void *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___last), A1), \
   struct Library *, (___base), 72, Awebsupport)

#define Writefilter(___par1, ___par2, ___last) __Writefilter_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Writefilter_WB(___base, ___par1, ___par2, ___last) \
   AROS_LC3(void, Writefilter, \
   AROS_LCA(void *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___par2), A1), \
   AROS_LCA(long, (___last), D0), \
   struct Library *, (___base), 73, Awebsupport)

#define Awebcommand(___par1, ___par2, ___par3, ___last) __Awebcommand_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___last)
#define __Awebcommand_WB(___base, ___par1, ___par2, ___par3, ___last) \
   AROS_LC4(long, Awebcommand, \
   AROS_LCA(long, (___par1), D0), \
   AROS_LCA(UBYTE *, (___par2), A0), \
   AROS_LCA(UBYTE *, (___par3), A1), \
   AROS_LCA(long, (___last), D1), \
   struct Library *, (___base), 74, Awebsupport)

#define Awebactive() __Awebactive_WB(AWEBSUPPORT_BASE_NAME)
#define __Awebactive_WB(___base) \
   AROS_LC0(BOOL, Awebactive, \
   struct Library *, (___base), 75, Awebsupport)

#define Freefontalias(___last) __Freefontalias_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freefontalias_WB(___base, ___last) \
   AROS_LC1(void, Freefontalias, \
   AROS_LCA(struct Fontalias *, (___last), A0), \
   struct Library *, (___base), 76, Awebsupport)

#define Addfontalias(___par1, ___last) __Addfontalias_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Addfontalias_WB(___base, ___par1, ___last) \
   AROS_LC2(struct Fontalias *, Addfontalias, \
   AROS_LCA(void *, (___par1), A0), \
   AROS_LCA(UBYTE *, (___last), A1), \
   struct Library *, (___base), 77, Awebsupport)

#define Freestemvar(___last) __Freestemvar_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freestemvar_WB(___base, ___last) \
   AROS_LC1(void, Freestemvar, \
   AROS_LCA(UBYTE *, (___last), A0), \
   struct Library *, (___base), 78, Awebsupport)

#endif /* !_INLINE_AWEBSUPPORT_H */
