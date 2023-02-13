/* Automatically generated header! Do not edit! */

#ifndef _INLINE_AWEBSUPPORT_H
#define _INLINE_AWEBSUPPORT_H

#ifndef __INLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AWEBSUPPORT_BASE_NAME
#define AWEBSUPPORT_BASE_NAME AwebSupportBase
#endif /* !AWEBSUPPORT_BASE_NAME */

#define AmethodA(___par1, ___tags) __AmethodA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AmethodA_WB(___base, ___par1, ___tags) \
        LP2(0x1e, ULONG, AmethodA, struct Aobject *, ___par1, a0, struct Amessage *, ___tags, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_VARARGS
#define Amethod(___par1, ___dummy, ...) __Amethod_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Amethod_WB(___base, ___par1, ___dummy, ...) \
        ({APTR _message[] = { ___dummy, ## __VA_ARGS__ }; __AmethodA_WB((___base), (___par1), (struct Amessage *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define AmethodasA(___par1, ___par2, ___tags) __AmethodasA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __AmethodasA_WB(___base, ___par1, ___par2, ___tags) \
        LP3(0x24, ULONG, AmethodasA, ULONG, ___par1, d0, struct Aobject *, ___par2, a0, struct Amessage *, ___tags, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_VARARGS
#define Amethodas(___par1, ___par2, ___dummy, ...) __Amethodas_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___dummy, ## __VA_ARGS__)
#define __Amethodas_WB(___base, ___par1, ___par2, ___dummy, ...) \
        ({APTR _message[] = { ___dummy, ## __VA_ARGS__ }; __AmethodasA_WB((___base), (___par1), (___par2), (struct Amessage *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define AnewobjectA(___par1, ___tags) __AnewobjectA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AnewobjectA_WB(___base, ___par1, ___tags) \
        LP2(0x2a, APTR, AnewobjectA, ULONG, ___par1, d0, struct TagItem *, ___tags, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_STDARG
#define Anewobject(___par1, ___dummy, ...) __Anewobject_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Anewobject_WB(___base, ___par1, ___dummy, ...) \
        ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AnewobjectA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Adisposeobject(___last) __Adisposeobject_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Adisposeobject_WB(___base, ___last) \
        LP1NR(0x30, Adisposeobject, struct Aobject *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AsetattrsA(___par1, ___tags) __AsetattrsA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AsetattrsA_WB(___base, ___par1, ___tags) \
        LP2(0x36, ULONG, AsetattrsA, struct Aobject *, ___par1, a0, struct TagItem *, ___tags, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_STDARG
#define Asetattrs(___par1, ___dummy, ...) __Asetattrs_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Asetattrs_WB(___base, ___par1, ___dummy, ...) \
        ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AsetattrsA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define AgetattrsA(___par1, ___tags) __AgetattrsA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AgetattrsA_WB(___base, ___par1, ___tags) \
        LP2(0x3c, ULONG, AgetattrsA, struct Aobject *, ___par1, a0, struct TagItem *, ___tags, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_STDARG
#define Agetattrs(___par1, ___dummy, ...) __Agetattrs_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Agetattrs_WB(___base, ___par1, ___dummy, ...) \
        ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AgetattrsA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Agetattr(___par1, ___last) __Agetattr_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Agetattr_WB(___base, ___par1, ___last) \
        LP2(0x42, ULONG, Agetattr, struct Aobject *, ___par1, a0, ULONG, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AupdateattrsA(___par1, ___par2, ___tags) __AupdateattrsA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __AupdateattrsA_WB(___base, ___par1, ___par2, ___tags) \
        LP3(0x48, ULONG, AupdateattrsA, struct Aobject *, ___par1, a0, struct TagItem *, ___par2, a1, struct TagItem *, ___tags, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_STDARG
#define Aupdateattrs(___par1, ___par2, ___dummy, ...) __Aupdateattrs_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___dummy, ## __VA_ARGS__)
#define __Aupdateattrs_WB(___base, ___par1, ___par2, ___dummy, ...) \
        ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AupdateattrsA_WB((___base), (___par1), (___par2), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Arender(___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___par7, ___last) __Arender_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___par7, ___last)
#define __Arender_WB(___base, ___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___par7, ___last) \
        LP8(0x4e, ULONG, Arender, struct Aobject *, ___par1, a0, struct Coords *, ___par2, a1, LONG, ___par3, d0, LONG, ___par4, d1, LONG, ___par5, d2, LONG, ___par6, d3, USHORT, ___par7, d4, APTR, ___last, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Aaddchild(___par1, ___par2, ___last) __Aaddchild_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Aaddchild_WB(___base, ___par1, ___par2, ___last) \
        LP3(0x54, ULONG, Aaddchild, struct Aobject *, ___par1, a0, struct Aobject *, ___par2, a1, ULONG, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Aremchild(___par1, ___par2, ___last) __Aremchild_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Aremchild_WB(___base, ___par1, ___par2, ___last) \
        LP3(0x5a, ULONG, Aremchild, struct Aobject *, ___par1, a0, struct Aobject *, ___par2, a1, ULONG, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Anotify(___par1, ___last) __Anotify_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Anotify_WB(___base, ___par1, ___last) \
        LP2(0x60, ULONG, Anotify, struct Aobject *, ___par1, a0, struct Amessage *, ___last, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Allocobject(___par1, ___par2, ___last) __Allocobject_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Allocobject_WB(___base, ___par1, ___par2, ___last) \
        LP3(0x66, APTR, Allocobject, ULONG, ___par1, d0, LONG, ___par2, d1, struct Amset *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AnotifysetA(___par1, ___tags) __AnotifysetA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AnotifysetA_WB(___base, ___par1, ___tags) \
        LP2(0x6c, ULONG, AnotifysetA, struct Aobject *, ___par1, a0, struct TagItem *, ___tags, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_STDARG
#define Anotifyset(___par1, ___dummy, ...) __Anotifyset_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Anotifyset_WB(___base, ___par1, ___dummy, ...) \
        ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AnotifysetA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Allocmem(___par1, ___last) __Allocmem_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Allocmem_WB(___base, ___par1, ___last) \
        LP2(0x72, APTR, Allocmem, ULONG, ___par1, d0, ULONG, ___last, d1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Dupstr(___par1, ___last) __Dupstr_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Dupstr_WB(___base, ___par1, ___last) \
        LP2(0x78, STRPTR, Dupstr, STRPTR, ___par1, a0, LONG, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freemem(___last) __Freemem_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freemem_WB(___base, ___last) \
        LP1NR(0x7e, Freemem, APTR, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Clipcoords(___par1, ___last) __Clipcoords_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Clipcoords_WB(___base, ___par1, ___last) \
        LP2(0x84, struct Coords *, Clipcoords, struct Aobject *, ___par1, a0, struct Coords *, ___last, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Unclipcoords(___last) __Unclipcoords_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Unclipcoords_WB(___base, ___last) \
        LP1NR(0x8a, Unclipcoords, struct Coords *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Erasebg(___par1, ___par2, ___par3, ___par4, ___par5, ___last) __Erasebg_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___par5, ___last)
#define __Erasebg_WB(___base, ___par1, ___par2, ___par3, ___par4, ___par5, ___last) \
        LP6NR(0x90, Erasebg, struct Aobject *, ___par1, a0, struct Coords *, ___par2, a1, LONG, ___par3, d0, LONG, ___par4, d1, LONG, ___par5, d2, LONG, ___last, d3,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Aweb() __Aweb_WB(AWEBSUPPORT_BASE_NAME)
#define __Aweb_WB(___base) \
        LP0(0x96, struct Aobject *, Aweb,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define AsetattrsasyncA(___par1, ___tags) __AsetattrsasyncA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___tags)
#define __AsetattrsasyncA_WB(___base, ___par1, ___tags) \
        LP2NR(0x9c, AsetattrsasyncA, struct Aobject *, ___par1, a0, struct TagItem *, ___tags, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_STDARG
#define Asetattrsasync(___par1, ___dummy, ...) __Asetattrsasync_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___dummy, ## __VA_ARGS__)
#define __Asetattrsasync_WB(___base, ___par1, ___dummy, ...) \
        ({APTR _tags[] = { ___dummy, ## __VA_ARGS__ }; __AsetattrsasyncA_WB((___base), (___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Waittask(___last) __Waittask_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Waittask_WB(___base, ___last) \
        LP1(0xa2, ULONG, Waittask, ULONG, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Gettaskmsg() __Gettaskmsg_WB(AWEBSUPPORT_BASE_NAME)
#define __Gettaskmsg_WB(___base) \
        LP0(0xa8, struct Taskmsg *, Gettaskmsg,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Replytaskmsg(___last) __Replytaskmsg_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Replytaskmsg_WB(___base, ___last) \
        LP1NR(0xae, Replytaskmsg, struct Taskmsg *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Checktaskbreak() __Checktaskbreak_WB(AWEBSUPPORT_BASE_NAME)
#define __Checktaskbreak_WB(___base) \
        LP0(0xb4, ULONG, Checktaskbreak,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Updatetask(___last) __Updatetask_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Updatetask_WB(___base, ___last) \
        LP1(0xba, ULONG, Updatetask, struct Amessage *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define UpdatetaskattrsA(___tags) __UpdatetaskattrsA_WB(AWEBSUPPORT_BASE_NAME, ___tags)
#define __UpdatetaskattrsA_WB(___base, ___tags) \
        LP1(0xc0, ULONG, UpdatetaskattrsA, struct TagItem *, ___tags, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_STDARG
#define Updatetaskattrs(___tags, ...) __Updatetaskattrs_WB(AWEBSUPPORT_BASE_NAME, ___tags, ## __VA_ARGS__)
#define __Updatetaskattrs_WB(___base, ___tags, ...) \
        ({APTR _tags[] = { ___tags, ## __VA_ARGS__ }; __UpdatetaskattrsA_WB((___base), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define Obtaintasksemaphore(___last) __Obtaintasksemaphore_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Obtaintasksemaphore_WB(___base, ___last) \
        LP1(0xc6, ULONG, Obtaintasksemaphore, struct SignalSemaphore *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Avprintf(___par1, ___last) __Avprintf_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Avprintf_WB(___base, ___par1, ___last) \
        LP2(0xcc, LONG, Avprintf, char *, ___par1, a0, ULONG *, ___last, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_VARARGS
#define Aprintf(___par1, ...) __Aprintf_WB(AWEBSUPPORT_BASE_NAME, ___par1, ## __VA_ARGS__)
#define __Aprintf_WB(___base, ___par1, ...) \
        ({APTR _args[] = { __VA_ARGS__ }; __Avprintf_WB((___base), (___par1), (ULONG *) _args); })
#endif /* !NO_INLINE_VARARGS */

#define Awebstr(___last) __Awebstr_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Awebstr_WB(___base, ___last) \
        LP1(0xd2, UBYTE *, Awebstr, ULONG, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define TcperrorA(___par1, ___par2, ___tags) __TcperrorA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __TcperrorA_WB(___base, ___par1, ___par2, ___tags) \
        LP3NR(0xd8, TcperrorA, struct Fetchdriver *, ___par1, a0, ULONG, ___par2, d0, ULONG *, ___tags, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_VARARGS
#define Tcperror(___par1, ___par2, ...) __Tcperror_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ## __VA_ARGS__)
#define __Tcperror_WB(___base, ___par1, ___par2, ...) \
        ({APTR _args[] = { __VA_ARGS__ }; __TcperrorA_WB((___base), (___par1), (___par2), (ULONG *) _args); })
#endif /* !NO_INLINE_VARARGS */

#define TcpmessageA(___par1, ___par2, ___tags) __TcpmessageA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __TcpmessageA_WB(___base, ___par1, ___par2, ___tags) \
        LP3NR(0xde, TcpmessageA, struct Fetchdriver *, ___par1, a0, ULONG, ___par2, d0, ULONG *, ___tags, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_VARARGS
#define Tcpmessage(___par1, ___par2, ...) __Tcpmessage_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ## __VA_ARGS__)
#define __Tcpmessage_WB(___base, ___par1, ___par2, ...) \
        ({APTR _args[] = { __VA_ARGS__ }; __TcpmessageA_WB((___base), (___par1), (___par2), (ULONG *) _args); })
#endif /* !NO_INLINE_VARARGS */

#define Opentcp(___par1, ___par2, ___last) __Opentcp_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Opentcp_WB(___base, ___par1, ___par2, ___last) \
        LP3(0xe4, struct Library *, Opentcp, struct Library **, ___par1, a0, struct Fetchdriver *, ___par2, a1, BOOL, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Lookup(___par1, ___last) __Lookup_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Lookup_WB(___base, ___par1, ___last) \
        LP2(0xea, struct hostent *, Lookup, UBYTE *, ___par1, a0, struct Library *, ___last, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Expandbuffer(___par1, ___last) __Expandbuffer_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Expandbuffer_WB(___base, ___par1, ___last) \
        LP2(0xf0, BOOL, Expandbuffer, struct Buffer *, ___par1, a0, long, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Insertinbuffer(___par1, ___par2, ___par3, ___last) __Insertinbuffer_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___last)
#define __Insertinbuffer_WB(___base, ___par1, ___par2, ___par3, ___last) \
        LP4(0xf6, BOOL, Insertinbuffer, struct Buffer *, ___par1, a0, UBYTE *, ___par2, a1, long, ___par3, d0, long, ___last, d1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Addtobuffer(___par1, ___par2, ___last) __Addtobuffer_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Addtobuffer_WB(___base, ___par1, ___par2, ___last) \
        LP3(0xfc, BOOL, Addtobuffer, struct Buffer *, ___par1, a0, UBYTE *, ___par2, a1, long, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Deleteinbuffer(___par1, ___par2, ___last) __Deleteinbuffer_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Deleteinbuffer_WB(___base, ___par1, ___par2, ___last) \
        LP3NR(0x102, Deleteinbuffer, struct Buffer *, ___par1, a0, long, ___par2, d0, long, ___last, d1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freebuffer(___last) __Freebuffer_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freebuffer_WB(___base, ___last) \
        LP1NR(0x108, Freebuffer, struct Buffer *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Locale() __Locale_WB(AWEBSUPPORT_BASE_NAME)
#define __Locale_WB(___base) \
        LP0(0x10e, struct Locale *, Locale,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Lprintdate(___par1, ___par2, ___last) __Lprintdate_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Lprintdate_WB(___base, ___par1, ___par2, ___last) \
        LP3(0x114, long, Lprintdate, UBYTE *, ___par1, a0, UBYTE *, ___par2, a1, struct DateStamp *, ___last, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Obtainbgrp(___par1, ___par2, ___par3, ___par4, ___par5, ___last) __Obtainbgrp_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___par5, ___last)
#define __Obtainbgrp_WB(___base, ___par1, ___par2, ___par3, ___par4, ___par5, ___last) \
        LP6(0x11a, struct RastPort *, Obtainbgrp, struct Aobject *, ___par1, a0, struct Coords *, ___par2, a1, LONG, ___par3, d0, LONG, ___par4, d1, LONG, ___par5, d2, LONG, ___last, d3,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Releasebgrp(___last) __Releasebgrp_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Releasebgrp_WB(___base, ___last) \
        LP1NR(0x120, Releasebgrp, struct RastPort *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Setstemvar(___par1, ___par2, ___par3, ___par4, ___last) __Setstemvar_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___last)
#define __Setstemvar_WB(___base, ___par1, ___par2, ___par3, ___par4, ___last) \
        LP5NR(0x126, Setstemvar, struct Arexxcmd *, ___par1, a0, UBYTE *, ___par2, a1, long, ___par3, d0, UBYTE *, ___par4, a2, UBYTE *, ___last, a3,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Copyprefs(___par1, ___last) __Copyprefs_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Copyprefs_WB(___base, ___par1, ___last) \
        LP2NR(0x12c, Copyprefs, struct AwebPrefs *, ___par1, a0, struct AwebPrefs *, ___last, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Saveprefs(___last) __Saveprefs_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Saveprefs_WB(___base, ___last) \
        LP1NR(0x132, Saveprefs, struct AwebPrefs *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Disposeprefs(___last) __Disposeprefs_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Disposeprefs_WB(___base, ___last) \
        LP1NR(0x138, Disposeprefs, struct AwebPrefs *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freemenuentry(___last) __Freemenuentry_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freemenuentry_WB(___base, ___last) \
        LP1NR(0x13e, Freemenuentry, struct Menuentry *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Addmenuentry(___par1, ___par2, ___par3, ___par4, ___last) __Addmenuentry_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___last)
#define __Addmenuentry_WB(___base, ___par1, ___par2, ___par3, ___par4, ___last) \
        LP5(0x144, struct Menuentry *, Addmenuentry, void *, ___par1, a0, USHORT, ___par2, d0, UBYTE *, ___par3, a1, UBYTE, ___par4, d1, UBYTE *, ___last, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freemimeinfo(___last) __Freemimeinfo_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freemimeinfo_WB(___base, ___last) \
        LP1NR(0x14a, Freemimeinfo, struct Mimeinfo *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Addmimeinfo(___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___last) __Addmimeinfo_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___last)
#define __Addmimeinfo_WB(___base, ___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___last) \
        LP7(0x150, struct Mimeinfo *, Addmimeinfo, void *, ___par1, a0, UBYTE *, ___par2, a1, UBYTE *, ___par3, a2, UBYTE *, ___par4, a3, USHORT, ___par5, d0, UBYTE *, ___par6, a4, UBYTE *, ___last, d1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freenocache(___last) __Freenocache_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freenocache_WB(___base, ___last) \
        LP1NR(0x156, Freenocache, struct Nocache *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Addnocache(___par1, ___last) __Addnocache_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Addnocache_WB(___base, ___par1, ___last) \
        LP2(0x15c, struct Nocache *, Addnocache, void *, ___par1, a0, UBYTE *, ___last, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Getstemvar(___par1, ___par2, ___par3, ___last) __Getstemvar_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___last)
#define __Getstemvar_WB(___base, ___par1, ___par2, ___par3, ___last) \
        LP4(0x162, UBYTE *, Getstemvar, struct Arexxcmd *, ___par1, a0, UBYTE *, ___par2, a1, long, ___par3, d0, UBYTE *, ___last, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freeuserbutton(___last) __Freeuserbutton_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freeuserbutton_WB(___base, ___last) \
        LP1NR(0x168, Freeuserbutton, struct Userbutton *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Adduserbutton(___par1, ___par2, ___last) __Adduserbutton_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Adduserbutton_WB(___base, ___par1, ___par2, ___last) \
        LP3(0x16e, struct Userbutton *, Adduserbutton, void *, ___par1, a0, UBYTE *, ___par2, a1, UBYTE *, ___last, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freepopupitem(___last) __Freepopupitem_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freepopupitem_WB(___base, ___last) \
        LP1NR(0x174, Freepopupitem, struct Popupitem *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Addpopupitem(___par1, ___par2, ___par3, ___last) __Addpopupitem_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___last)
#define __Addpopupitem_WB(___base, ___par1, ___par2, ___par3, ___last) \
        LP4(0x17a, struct Popupitem *, Addpopupitem, void *, ___par1, a0, USHORT, ___par2, d0, UBYTE *, ___par3, a1, UBYTE *, ___last, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Awebversion() __Awebversion_WB(AWEBSUPPORT_BASE_NAME)
#define __Awebversion_WB(___base) \
        LP0(0x180, UBYTE *, Awebversion,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Syncrequest(___par1, ___par2, ___last) __Syncrequest_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Syncrequest_WB(___base, ___par1, ___par2, ___last) \
        LP3(0x186, long, Syncrequest, UBYTE *, ___par1, a0, UBYTE *, ___par2, a1, UBYTE *, ___last, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define PprintfA(___par1, ___par2, ___tags) __PprintfA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __PprintfA_WB(___base, ___par1, ___par2, ___tags) \
        LP3(0x18c, UBYTE *, PprintfA, UBYTE *, ___par1, a0, UBYTE *, ___par2, a1, UBYTE **, ___tags, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_VARARGS
#define Pprintf(___par1, ___par2, ___dummy, ...) __Pprintf_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___dummy, ## __VA_ARGS__)
#define __Pprintf_WB(___base, ___par1, ___par2, ___dummy, ...) \
        ({APTR _message[] = { ___dummy, ## __VA_ARGS__ }; __PprintfA_WB((___base), (___par1), (___par2), (UBYTE **) _message); })
#endif /* !NO_INLINE_VARARGS */

#define Today() __Today_WB(AWEBSUPPORT_BASE_NAME)
#define __Today_WB(___base) \
        LP0(0x192, ULONG, Today,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freeuserkey(___last) __Freeuserkey_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freeuserkey_WB(___base, ___last) \
        LP1NR(0x198, Freeuserkey, struct Userkey *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Adduserkey(___par1, ___par2, ___last) __Adduserkey_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Adduserkey_WB(___base, ___par1, ___par2, ___last) \
        LP3(0x19e, struct Userkey *, Adduserkey, void *, ___par1, a0, USHORT, ___par2, d0, UBYTE *, ___last, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define LprintfA(___par1, ___par2, ___tags) __LprintfA_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___tags)
#define __LprintfA_WB(___base, ___par1, ___par2, ___tags) \
        LP3(0x1a4, long, LprintfA, UBYTE *, ___par1, a0, UBYTE *, ___par2, a1, void *, ___tags, a2,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_INLINE_VARARGS
#define Lprintf(___par1, ___par2, ___dummy, ...) __Lprintf_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___dummy, ## __VA_ARGS__)
#define __Lprintf_WB(___base, ___par1, ___par2, ___dummy, ...) \
        ({APTR _message[] = { ___dummy, ## __VA_ARGS__ }; __LprintfA_WB((___base), (___par1), (___par2), (void *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define Fullversion() __Fullversion_WB(AWEBSUPPORT_BASE_NAME)
#define __Fullversion_WB(___base) \
        LP0(0x1aa, BOOL, Fullversion,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Setfiltertype(___par1, ___last) __Setfiltertype_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Setfiltertype_WB(___base, ___par1, ___last) \
        LP2NR(0x1b0, Setfiltertype, void *, ___par1, a0, UBYTE *, ___last, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Writefilter(___par1, ___par2, ___last) __Writefilter_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___last)
#define __Writefilter_WB(___base, ___par1, ___par2, ___last) \
        LP3NR(0x1b6, Writefilter, void *, ___par1, a0, UBYTE *, ___par2, a1, long, ___last, d0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Awebcommand(___par1, ___par2, ___par3, ___last) __Awebcommand_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___par2, ___par3, ___last)
#define __Awebcommand_WB(___base, ___par1, ___par2, ___par3, ___last) \
        LP4(0x1bc, long, Awebcommand, long, ___par1, d0, UBYTE *, ___par2, a0, UBYTE *, ___par3, a1, long, ___last, d1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Awebactive() __Awebactive_WB(AWEBSUPPORT_BASE_NAME)
#define __Awebactive_WB(___base) \
        LP0(0x1c2, BOOL, Awebactive,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freefontalias(___last) __Freefontalias_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freefontalias_WB(___base, ___last) \
        LP1NR(0x1c8, Freefontalias, struct Fontalias *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Addfontalias(___par1, ___last) __Addfontalias_WB(AWEBSUPPORT_BASE_NAME, ___par1, ___last)
#define __Addfontalias_WB(___base, ___par1, ___last) \
        LP2(0x1ce, struct Fontalias *, Addfontalias, void *, ___par1, a0, UBYTE *, ___last, a1,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define Freestemvar(___last) __Freestemvar_WB(AWEBSUPPORT_BASE_NAME, ___last)
#define __Freestemvar_WB(___base, ___last) \
        LP1NR(0x1d4, Freestemvar, UBYTE *, ___last, a0,\
        , (___base), IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /* !_INLINE_AWEBSUPPORT_H */
