/**********************************************************************
 *
 * This file is part of the AWeb-II distribution
 *
 * Copyright (C) 2002 Yvon Rozijn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the AWeb Public License as included in this
 * distribution.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * AWeb Public License for more details.
 *
 **********************************************************************/

/* support.c - AWeb awebsupport.library */

#include "aweb.h"
#include "object.h"
#include "task.h"
#include "source.h"
#include "locale.h"
#include <proto/exec.h>
#include <proto/utility.h>


#if defined(__amigaos4__)
#undef __USE_INLINE__
#include "proto/awebsupport.h"
#endif

#ifdef __AROS__
#    error this file does not work with AROS
#endif

/*-----------------------------------------------------------------------*/


LIBFUNC_H1
(
    struct Library *, apsOpen,
    long,             version, D0,
    LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT

#if defined(__amigaos4__)
    struct Library *AwebSupportBase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
    struct Library *AwebSupportBase = LIBMAN_NAME;
#endif

    AwebSupportBase->lib_OpenCnt++;
    return AwebSupportBase;;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
    long, apsClose,
    LIBMAN_TYPE, LIBMAN_NAME
)

{
    LIBFUNC_INIT

#if defined(__amigaos4__)
    struct Library *AwebSupportBase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
    struct Library *AwebSupportBase = LIBMAN_NAME;
#endif

    AwebSupportBase->lib_OpenCnt--;
    return 0;

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
    long, apsExpunge,
    LIBMAN_TYPE, LIBMAN_NAME
)
{
    LIBFUNC_INIT
#if defined(__amigaos4__)
    struct Library *AwebSupportBase = (struct Library *)LIBMAN_NAME->Data.LibBase;
#else
    struct Library *AwebSupportBase = LIBMAN_NAME;
#endif

    if(AwebSupportBase->lib_OpenCnt == 0)
    {
        Remove((struct Node *)AwebSupportBase);
    }
    return 0;

    LIBFUNC_EXIT
}

long apsExtfunc(void)
{
    return 0;
}

LIBFUNC_H2
(
    __saveds ULONG, apsAmethodA,
    void *, object,   A0,
    void *, amessage, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return AmethodA(object,amessage);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    __saveds ULONG, apsAmethodasA,
    long,   objtype,  D0,
    void *, object,   A0,
    void *, amessage, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return AmethodasA(objtype,object,amessage);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds void *, apsAnewobjectA,
    long,             objtype,  D0,
    struct TagItem *, taglist,  A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return (void *)AnewobjectA(objtype, taglist);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsAdisposeobject,
    void *, object, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Adisposeobject(object);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds ULONG, apsAsetattrsA,
    void *,           object,  A0,
    struct TagItem *, taglist, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return AsetattrsA(object, taglist);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds ULONG, apsAgetattrsA,
    void *,           object,  A0,
    struct TagItem *, taglist, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return AgetattrsA(object, taglist);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds ULONG, apsAgetattr,
    void *, object, A0,
    ULONG,  attrid, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Agetattr(object, attrid);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    __saveds ULONG, apsAupdateattrsA,
    void *,           object,  A0,
    struct TagItem *, maplist, A1,
    struct TagItem *, taglist, A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return AupdateattrsA(object, maplist, taglist);

    LIBFUNC_EXIT
}

LIBFUNC_H8
(
    __saveds ULONG, apsArender,
    void *,          object, A0,
    struct Coords *, coords, A1,
    long,            minx,   D0,
    long,            miny,   D1,
    long,            maxx,   D2,
    long,            maxy,   D3,
    UWORD,           flags,  D4,
    void *,          text,   A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Arender(object,coords,minx,miny,maxx,maxy,flags,text);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    __saveds ULONG, apsAaddchild,
    void *, object,   A0,
    void *, child,    A1,
    long,   relation, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Aaddchild(object, child, relation);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    __saveds ULONG, apsAremchild,
    void *, object,   A0,
    void *, child,    A1,
    long,   relation, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Aremchild(object, child, relation);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds ULONG, apsAnotify,
    void *, object,   A0,
    void *, amessage, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Anotify(object, amessage);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    __saveds void *, apsAllocobject,
    long,   objtype, D0,
    long,   size,    D1,
    void *, amset,   A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Allocobject(objtype, size, amset);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds ULONG, apsAnotifysetA,
    void *,           object,  A0,
    struct TagItem *, taglist, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    struct Amset ams;
    struct Amnotify amn = {{0},0};

    ams.amsg.method = AOM_SET;
    ams.tags        = taglist;
    amn.amsg.method = AOM_NOTIFY;
    amn.nmsg        = &ams.amsg;

    return AmethodA((struct Aobject *)object, &amn.amsg);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds void *, apsAllocmem,
    long,  size,  D0,
    ULONG, flags, D1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Allocmem(size,flags);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds UBYTE *, apsDupstr,
    UBYTE *, string, A0,
    long,    length, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
     LIBFUNC_INIT

     return Dupstr(string, length);

     LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreemem,
    void *, mem, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Freemem(mem);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds struct Coords *, apsClipcoords,
    void *,          frame,  A0,
    struct Coords *, coords, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Clipcoords(frame, coords);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsUnclipcoords,
    struct Coords *, coords, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Unclipcoords(coords);

    LIBFUNC_EXIT
}

LIBFUNC_H6
(
    __saveds void, apsErasebg,
    void *,         frame,  A0,
   struct Coords *, coords, A1,
   long,            xmin,   D0,
   long,            ymin,   D1,
   long,            xmax,   D2,
   long,            ymax,   D3
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Erasebg(frame, coords, xmin, ymin, xmax, ymax);

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds void *, apsAweb,
  AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
                LIBFUNC_INIT
    return Aweb();
                LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds void, apsAsetattrsasyncA,
    void *,           task,    A0,
    struct TagItem *, taglist, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    AsetattrsasyncA(task,taglist);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
     __saveds ULONG, apsWaittask,
     ULONG, signals, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Waittask(signals);

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds struct Taskmsg *, apsGettaskmsg,
  AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Gettaskmsg();

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsReplytaskmsg,
    struct Taskmsg *, taskmsg, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Replytaskmsg(taskmsg);

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds ULONG, apsChecktaskbreak,
   AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Checktaskbreak();

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds ULONG, apsUpdatetask,
    void *, amessage, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return (ULONG)Updatetask(amessage);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds ULONG, apsUpdatetaskattrsA,
    struct TagItem *, taglist, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
                LIBFUNC_INIT
    return (ULONG)UpdatetaskattrsA(taglist);
                LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds ULONG, apsObtaintasksemaphore,
    struct SignalSemaphore *, semaphore, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Obtaintasksemaphore(semaphore);

    LIBFUNC_EXIT
}

#ifdef __MORPHOS__
#warning FIX ME
LIBFUNC_H2
(
    __saveds long, apsAvprintf,
    char *, format, A0,
    VA_LIST *, args,  A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
#else
LIBFUNC_H2
(
    __saveds long, apsAvprintf,
    char *, format, A0,
    VA_LIST, args,  A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
#endif
{
    LIBFUNC_INIT

    return vprintf(format,args);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds UBYTE *, apsAwebstr,
    ULONG, id, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
                LIBFUNC_INIT
    return AWEBSTR(id);
                LIBFUNC_EXIT
}


LIBFUNC_H3
(
     __saveds void, apsTcperrorA,
     struct Fetchdriver *, fd,   A0,
     ULONG,                err,  D0,
     ULONG *,              args, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    TcperrorA(fd,err,args);

    LIBFUNC_EXIT
}


LIBFUNC_H3
(
     __saveds void, apsTcpmessageA,
     struct Fetchdriver *, fd,   A0,
     ULONG,                err,  D0,
     ULONG *,              args, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    TcpmessageA(fd,err,args);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
     __saveds struct Library *, apsOpentcp,
     struct Library **,    base,    A0,
     struct Fetchdriver *, fd,      A1,
     BOOL,                 autocon, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Opentcp(base,fd,autocon);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds struct hostent *, apsLookup,
    UBYTE *,          name, A0,
    struct Library *, base, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Lookup(name,base);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds BOOL, apsExpandbuffer,
    struct Buffer *, buf, A0,
    long, size, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Expandbuffer(buf,size);

    LIBFUNC_EXIT
}

LIBFUNC_H4
(
    __saveds BOOL, apsInsertinbuffer,
    struct Buffer *, buf,    A0,
    UBYTE *,         text,   A1,
    long,            length, D0,
    long,            pos,    D1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Insertinbuffer(buf,text,length,pos);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    __saveds BOOL, apsAddtobuffer,
    struct Buffer *, buf,    A0,
    UBYTE *,         text,   A1,
    long,            length, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Addtobuffer(buf,text,length);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    __saveds void, apsDeleteinbuffer,
    struct Buffer *, buf,    A0,
    long,            pos,    D0,
    long,            length, D1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Deleteinbuffer(buf, pos, length);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreebuffer,
    struct Buffer *, buf, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Freebuffer(buf);

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
 __saveds struct Locale *, apsLocale,
    AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT
    return locale;
    LIBFUNC_EXIT
}

LIBFUNC_H3
(
     __saveds long, apsLprintdate,
     UBYTE *,            buffer, A0,
     UBYTE *,            fmt,    A1,
     struct DateStamp *, ds,     A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Lprintdate(buffer, fmt, ds);

    LIBFUNC_EXIT
}

LIBFUNC_H6
(
     __saveds struct RastPort *, apsObtainbgrp,
     void *,          frame,  A0,
     struct Coords *, coords, A1,
     long,            xmin,   D0,
     long,            ymin,   D1,
     long,            xmax,   D2,
     long,            ymax,   D3
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Obtainbgrp(frame, coords, xmin, ymin, xmax, ymax);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsReleasebgrp,
    struct RastPort *, bgrp, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Releasebgrp(bgrp);

    LIBFUNC_EXIT
}

LIBFUNC_H5
(
    __saveds void, apsSetstemvar,
    struct Arexxcmd *, ac,    A0,
    UBYTE *,           stem,  A1,
    long,              index, D0,
    UBYTE *,           field, A2,
    UBYTE *,           value, A3
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Setstemvar(ac, stem, index, field, value);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds void, apsCopyprefs,
    struct AwebPrefs *, from, A0,
    struct AwebPrefs *, to,   A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Copyprefs(from,to);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsSaveprefs,
    struct AwebPrefs *, prefs, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Saveprefs(prefs);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsDisposeprefs,
    struct AwebPrefs *, prefs, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Disposeprefs(prefs);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreemenuentry,
    struct Menuentry *, me, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Freemenuentry(me);

    LIBFUNC_EXIT
}

LIBFUNC_H5
(
    __saveds struct Menuentry *, apsAddmenuentry,
    void *,  list,  A0,
    UWORD,   type,  D0,
    UBYTE *, title, A1,
    UBYTE,   scut,  D1,
    UBYTE *, cmd,   A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Addmenuentry(list, type, title, scut, cmd);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreemimeinfo,
    struct Mimeinfo *, mi, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Freemimeinfo(mi);

    LIBFUNC_EXIT
}

LIBFUNC_H7
(
    __saveds struct Mimeinfo *, apsAddmimeinfo,
    struct MimeinfoList *, list,       A0,
    UBYTE *,               type,       A1,
    UBYTE *,               subtype,    A2,
    UBYTE *,               extensions, A3,
    UWORD,                 driver,     D0,
    UBYTE *,               cmd,        A4,
    UBYTE *,               args,       D1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Addmimeinfo(list, type, subtype, extensions, driver, cmd, args);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreenocache,
    struct Nocache *, nc, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Freenocache(nc);

    LIBFUNC_EXIT
}


LIBFUNC_H2
(
    __saveds struct Nocache *, apsAddnocache,
    void *,  list, A0,
    UBYTE *, name, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Addnocache(list, name);

    LIBFUNC_EXIT
}


LIBFUNC_H4
(
    __saveds UBYTE *, apsGetstemvar,
    struct Arexxcmd *, ac,    A0,
    UBYTE *,           stem,  A1,
    long,              index, D0,
    UBYTE *,           field, A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Getstemvar(ac, stem, index, field);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreeuserbutton,
    struct Userbutton *, ub, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Freeuserbutton(ub);

    LIBFUNC_EXIT
}


LIBFUNC_H3
(
    __saveds struct Userbutton *, apsAdduserbutton,
    void *,  list,  A0,
    UBYTE *, label, A1,
    UBYTE *, cmd,   A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Adduserbutton(list,label,cmd);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreepopupitem,
    struct Popupitem *, pi, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Freepopupitem(pi);

    LIBFUNC_EXIT
}


LIBFUNC_H4
(
    __saveds struct Popupitem *, apsAddpopupitem,
    void *,  list,  A0,
    UWORD,   flags, D0,
    UBYTE *, title, A1,
    UBYTE *, cmd,   A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
   LIBFUNC_INIT

   return Addpopupitem(list, flags, title, cmd);

   LIBFUNC_EXIT
}

LIBFUNC_H0
(
 __saveds UBYTE *, apsAwebversion,
    AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT
    return awebversion;
    LIBFUNC_EXIT
}


LIBFUNC_H3
(
    __saveds long, apsSyncrequest,
    UBYTE *, title,  A0,
    UBYTE *, text,   A1,
    UBYTE *, labels, A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Syncrequest(title, text, labels, 0);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    __saveds UBYTE *, apsPprintfA,
    UBYTE *,  fmt,     A0,
    UBYTE *,  argspec, A1,
    UBYTE **, params,  A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    UBYTE *p;
    long len;

     len = Pformatlength(fmt, argspec, params);

     if((p = ALLOCTYPE(UBYTE,len+1, 0)))
     {
         Pformat(p, fmt, argspec, params, FALSE);
     }

     return p;

     LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds ULONG, apsToday,
    AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT
    return Today();
    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreeuserkey,
    struct Userkey *,uk, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Freeuserkey(uk);

    LIBFUNC_EXIT
}


LIBFUNC_H3
(
    __saveds struct Userkey *, apsAdduserkey,
    void *,  list, A0,
    UWORD,   key,  D0,
    UBYTE *, cmd,  A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Adduserkey(list, key, cmd);

    LIBFUNC_EXIT
}


LIBFUNC_H3
(
    __saveds long, apsLprintfA,
    UBYTE *, buffer, A0,
    UBYTE *, fmt,    A1,
    void *,  params, A2
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return LprintfA(buffer,fmt,params);

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds BOOL, apsFullversion,
   AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
   LIBFUNC_INIT

   return TRUE;

   LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds void, apsSetfiltertype,
    void *,  handle, A0,
    UBYTE *, type,   A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME

)
{
    LIBFUNC_INIT

    Srcsetfiltertype(handle, type);

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    __saveds void, apsWritefilter,
    void *,  handle, A0,
    UBYTE *, data,   A1,
    long,    length, D0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME

)
{
    LIBFUNC_INIT

    Srcwritefilter(handle, data, length);

    LIBFUNC_EXIT
}

LIBFUNC_H4
(
    __saveds long, apsAwebcommand,
    long,    portnr,    D0,
    UBYTE *, cmd,       A0,
    UBYTE *, resultbuf, A1,
    long,    length,    D1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Supportarexxcmd(portnr, cmd, resultbuf, length);

    LIBFUNC_EXIT
}

LIBFUNC_H0
(
__saveds BOOL, apsAwebactive,
    AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT
    return Awebactive();
    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreefontalias,
    struct Fontalias *, fa, A0
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    Freefontalias(fa);

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    __saveds struct Fontalias *, apsAddfontalias,
    void *,  list,  A0,
    UBYTE *, alias, A1
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    return Addfontalias(list, alias);

    LIBFUNC_EXIT
}

LIBFUNC_H1
(
    __saveds void, apsFreestemvar,
    UBYTE *, value, A0,
    AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT
    Freestemvar(value);
    LIBFUNC_EXIT
}

/*-----------------------------------------------------------------------*/
/* varags stubs for various of the above function, needed for OS.4 not   */
/* used for OS3.x but inprinciple could be called internally but better  */
/* not to! Must be declared VARARGS68K                                                              */
/*-----------------------------------------------------------------------*/

#if defined(__amigaos4__)

LIBFUNC_H2
(
    VARARGS68K ULONG, apsAmethod,
    struct Aobject *, object, A0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    struct Amessage *m;
    ULONG result;

    VA_STARTLIN(ap,object);
    m = VA_GETLIN(ap, struct Amessage *);
    result = AmethodA((struct Aobject *)object,(struct Amessage *)m);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    VARARGS68K ULONG, apsAmethodas,
    ULONG, type, D0,
    struct Aobject *, object, A0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    struct Amessage *m;
    ULONG result;

    VA_STARTLIN(ap,object);
    m = VA_GETLIN(ap, struct Amessage *);
    result = AmethodasA((ULONG) type,(struct Aobject *)object, (struct Amessage *)m);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    VARARGS68K APTR, apsAnewobject,
    ULONG, objtype, D0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    struct TagItem *tags;
    APTR result;

    VA_STARTLIN(ap,objtype);
    tags = VA_GETLIN(ap, struct TagItem *);
    result = AnewobjectA((ULONG) objtype, (struct TagItem *)tags);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    VARARGS68K ULONG, apsAsetattrs,
    struct Aobject *, object, A0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    struct TagItem* tags;
    ULONG result;

    VA_STARTLIN(ap,object);
    tags = VA_GETLIN(ap, struct TagItem *);
    result = AsetattrsA((struct Aobject *)object, (struct TagItem *)tags);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    VARARGS68K ULONG, apsAgetattrs,
    struct Aobject *, object, A0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    struct TagItem* tags;
    ULONG result;

    VA_STARTLIN(ap,object);
    tags = VA_GETLIN(ap, struct TagItem *);
    result = AgetattrsA((struct Aobject *)object, (struct TagItem *)tags);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    VARARGS68K ULONG, apsAupdateattrs,
    struct Aobject *, object, A0,
    struct TagItem *, maplist,A1,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    struct TagItem* tags;
    ULONG result;

    VA_STARTLIN(ap,maplist);
    tags = VA_GETLIN(ap, struct TagItem *);
    result = AupdateattrsA((struct Aobject *)object, (struct TagItem *)maplist, (struct TagItem *)tags);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    VARARGS68K ULONG, apsAnotifyset,
    struct Aobject *, object, A0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    struct TagItem *tags;
    struct Amset ams;
    struct Amnotify amn;
    ULONG result;

    VA_STARTLIN(ap,object);
    tags = VA_GETLIN(ap, struct TagItem *);

    ams.amsg.method = AOM_SET;
    amn.amsg.method = AOM_NOTIFY;
    ams.tags = tags;
    amn.nmsg = &ams.amsg;

    result = AmethodA((struct Aobject *)object, &amn.amsg);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    VARARGS68K void, apsAsetattrsasync,
    struct Aobject *, object, A0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    struct TagItem *tags;
    ULONG result;

    VA_STARTLIN(ap,object);
    tags = VA_GETLIN(ap, struct TagItem *);
    AsetattrsasyncA((struct Aobject *)object, (struct TagItem *)tags);
    VA_END(ap);


    LIBFUNC_EXIT
}

/*
   the next function has no arg but self to precede ... but we can use self as this
   only OS4 and thus self is allways first in an other OS it might be last!
*/
LIBFUNC_H1
(
    VARARGS68K ULONG, apsUpdatetaskattrs,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    struct TagItem *tags;
    ULONG result;

    VA_STARTLIN(ap,Self);
    tags = VA_GETLIN(ap, struct TagItem *);
    result = UpdatetaskattrsA((struct TagItem *)tags);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}

LIBFUNC_H2
(
    VARARGS68K LONG, apsAprintf,
    char*, fmt, A0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    ULONG *args;
    ULONG result;

    VA_STARTLIN(ap,fmt);
    args = VA_GETLIN(ap, ULONG *);
    result = vprintf((char *)fmt, (ULONG *)args);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}


LIBFUNC_H3
(
    VARARGS68K void, apsTcperror,
    struct Fetchdriver *, fd, A0,
    ULONG, err, D0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    ULONG *args;
    ULONG result;

    VA_STARTLIN(ap,err);
    args = VA_GETLIN(ap, ULONG *);
    TcperrorA((struct Fetchdriver *)fd, (ULONG )err,(ULONG *)args);
    VA_END(ap);


    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    VARARGS68K void, apsTcpmessage,
    struct Fetchdriver *, fd, A0,
    ULONG, message, D0,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    ULONG *args;
    ULONG result;

    VA_STARTLIN(ap,message);
    args = VA_GETLIN(ap, ULONG *);
    TcpmessageA((struct Fetchdriver *)fd, (ULONG) message, (ULONG *)args);
    VA_END(ap);


    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    VARARGS68K ULONG, apsPprintf,
    UBYTE *, fmt, A0,
    UBYTE *, argspec, A1,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    UBYTE *p;
    long len;
    VA_LIST ap;
    UBYTE **params;
    ULONG result;

    VA_STARTLIN(ap,argspec);
    params = VA_GETLIN(ap, UBYTE **);

     len = Pformatlength(fmt, argspec, params);

     if((p = ALLOCTYPE(UBYTE,len+1, 0)))
     {
         Pformat(p, fmt, argspec, params, FALSE);
     }


    VA_END(ap);
    return p;

    LIBFUNC_EXIT
}

LIBFUNC_H3
(
    VARARGS68K ULONG, apsLprintf,
    UBYTE *, buffer, A0,
    UBYTE *, fmt, A1,
    ,...,
    , AWEBPLUGIN_TYPE, AWEBPLUGIN_NAME
)
{
    LIBFUNC_INIT

    VA_LIST ap;
    void *params;
    ULONG result;

    VA_STARTLIN(ap,fmt);
    params = VA_GETLIN(ap, void*);
    result = LprintfA((UBYTE *)buffer, (UBYTE *) fmt, (void *)params);
    VA_END(ap);
    return result;

    LIBFUNC_EXIT
}

#endif

/*-----------------------------------------------------------------------*/

#define APSVERSION      2
#define APSREVISION     3
#define APSVERSIONSTR   "2.3"

static UBYTE version[] = "awebsupport.library";
static UBYTE idstring[]= "awebsupport " APSVERSIONSTR " " __AMIGADATE__ " " CPU;

/*-----------------------------------------------------------------------*/

#if defined (__amigaos4__)
/* Make an OS4.0 Library */

struct Library *AwebSupportBase = NULL;
struct Library *Keeper = NULL;

USRFUNC_H3
(
static __saveds struct Library *, Initlib,
 struct Library *, libBase, D0,
 APTR, seglist, A0,
 struct ExecIFace *, exec, A6
)
{
    struct ExecIFace *IExec
#ifdef __GNUC__
        __attribute__((unused))
#endif
        = (struct ExecIFace *)exec;


    libBase->lib_Node.ln_Type = NT_LIBRARY;
    libBase->lib_Node.ln_Pri  = 0;
    libBase->lib_Node.ln_Name = version;
    libBase->lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
    libBase->lib_Version      = APSVERSION;
    libBase->lib_Revision     = APSREVISION;
    libBase->lib_IdString     = idstring;

    // libseglist = seglist;

    return (struct Library *)libBase;
}

/* function tables */



/* ------------------- Manager Interface ------------------------ */
static LONG _manager_Obtain(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount++;
}

static ULONG _manager_Release(struct LibraryManagerInterface *Self)
{
    return Self->Data.RefCount--;
}

/* Manager interface vectors */
static void *lib_manager_vectors[] =
{
    (void *)_manager_Obtain,
    (void *)_manager_Release,
    (void *)0,
    (void *)0,
    (void *)apsOpen,
    (void *)apsClose,
    (void *)apsExpunge,
    (void *)0,
    (void *)-1,
};

static LONG apsObtain(struct AwebSupportIFace *Self)
{
    return Self->Data.RefCount++;
}

static ULONG apsRelease(struct AwebSupportIFace *Self)
{
    return Self->Data.RefCount--;
}


static void *main_vectors[] = {
        (void *)apsObtain,
        (void *)apsRelease,
        (void *)NULL,
        (void *)NULL,
        (void *)apsAmethodA,
        (void *)apsAmethod,
        (void *)apsAmethodasA,
        (void *)apsAmethodas,
        (void *)apsAnewobjectA,
        (void *)apsAnewobject,
        (void *)apsAdisposeobject,
        (void *)apsAsetattrsA,
        (void *)apsAsetattrs,
        (void *)apsAgetattrsA,
        (void *)apsAgetattrs,
        (void *)apsAgetattr,
        (void *)apsAupdateattrsA,
        (void *)apsAupdateattrs,
        (void *)apsArender,
        (void *)apsAaddchild,
        (void *)apsAremchild,
        (void *)apsAnotify,
        (void *)apsAllocobject,
        (void *)apsAnotifysetA,
        (void *)apsAnotifyset,
        (void *)apsAllocmem,
        (void *)apsDupstr,
        (void *)apsFreemem,
        (void *)apsClipcoords,
        (void *)apsUnclipcoords,
        (void *)apsErasebg,
        (void *)apsAweb,
        (void *)apsAsetattrsasyncA,
        (void *)apsAsetattrsasync,
        (void *)apsWaittask,
        (void *)apsGettaskmsg,
        (void *)apsReplytaskmsg,
        (void *)apsChecktaskbreak,
        (void *)apsUpdatetask,
        (void *)apsUpdatetaskattrsA,
        (void *)apsUpdatetaskattrs,
        (void *)apsObtaintasksemaphore,
        (void *)apsAvprintf,
        (void *)apsAprintf,
        (void *)apsAwebstr,
        (void *)apsTcperrorA,
        (void *)apsTcperror,
        (void *)apsTcpmessageA,
        (void *)apsTcpmessage,
        (void *)apsOpentcp,
        (void *)apsLookup,
        (void *)apsExpandbuffer,
        (void *)apsInsertinbuffer,
        (void *)apsAddtobuffer,
        (void *)apsDeleteinbuffer,
        (void *)apsFreebuffer,
        (void *)apsLocale,
        (void *)apsLprintdate,
        (void *)apsObtainbgrp,
        (void *)apsReleasebgrp,
        (void *)apsSetstemvar,
        (void *)apsCopyprefs,
        (void *)apsSaveprefs,
        (void *)apsDisposeprefs,
        (void *)apsFreemenuentry,
        (void *)apsAddmenuentry,
        (void *)apsFreemimeinfo,
        (void *)apsAddmimeinfo,
        (void *)apsFreenocache,
        (void *)apsAddnocache,
        (void *)apsGetstemvar,
        (void *)apsFreeuserbutton,
        (void *)apsAdduserbutton,
        (void *)apsFreepopupitem,
        (void *)apsAddpopupitem,
        (void *)apsAwebversion,
        (void *)apsSyncrequest,
        (void *)apsPprintfA,
        (void *)apsPprintf,
        (void *)apsToday,
        (void *)apsFreeuserkey,
        (void *)apsAdduserkey,
        (void *)apsLprintfA,
        (void *)apsLprintf,
        (void *)apsFullversion,
        (void *)apsSetfiltertype,
        (void *)apsWritefilter,
        (void *)apsAwebcommand,
        (void *)apsAwebactive,
        (void *)apsFreefontalias,
        (void *)apsAddfontalias,
        (void *)apsFreestemvar,
        (void *)-1
};




/* taglists */

/* "__library" interface tag list */
static struct TagItem lib_managerTags[] =
{
    {MIT_Name,             (ULONG)"__library"},
    {MIT_VectorTable,      (ULONG)lib_manager_vectors},
    {MIT_Version,          1},
    {TAG_DONE,             0}
};


static struct TagItem mainTags[] =
{
    {MIT_Name,              (uint32)"main"},
    {MIT_VectorTable,       (uint32)main_vectors},
    {MIT_Version,           1},
    {TAG_DONE,              0}
};

static uint32 libInterfaces[] =
{
    (uint32)lib_managerTags,
    (uint32)mainTags,
    (uint32)0
};

static struct TagItem libCreateTags[] =
{
    {CLT_DataSize,         (uint32)(sizeof(struct Library))},
    {CLT_InitFunc,         (uint32)Initlib},
    {CLT_Interfaces,       (uint32)libInterfaces},
    {TAG_DONE,             0}
};



/* Aweb init and free functions */

BOOL Initsupport()
{
    if ((AwebSupportBase = CreateLibrary((struct TagItem *)libCreateTags)))
    {
        AddLibrary(AwebSupportBase);
        /* open the library to prevent expunge */
        Keeper = OpenLibrary("awebsupport.library",0);
        return TRUE;
    }
    return FALSE;
}

void Freesupport()
{
    if(Keeper)CloseLibrary(Keeper);
    if(AwebSupportBase)
    {
        Forbid();

        while(AwebSupportBase->lib_OpenCnt>0)
        {
            Permit();

            Lowlevelreq(AWEBSTR(MSG_ERROR_CANTQUIT), "awebsupport.library");

            Forbid();
        }


        RemLibrary(AwebSupportBase);
        Permit();

        DeleteLibrary(AwebSupportBase);
        AwebSupportBase = NULL;
    }

}

#elif defined(__MORPHOS__)

/*
 * MORPHOS Section
*/

static ULONG jumptab[]=
{
FUNCARRAY_32BIT_NATIVE,
   (ULONG)&apsOpen,
   (ULONG)&apsClose,
   (ULONG)&apsExpunge,
   (ULONG)&apsExtfunc,
   (ULONG)&apsAmethodA,
   (ULONG)&apsAmethodasA,
   (ULONG)&apsAnewobjectA,
   (ULONG)&apsAdisposeobject,
   (ULONG)&apsAsetattrsA,
   (ULONG)&apsAgetattrsA,
   (ULONG)&apsAgetattr,
   (ULONG)&apsAupdateattrsA,
   (ULONG)&apsArender,
   (ULONG)&apsAaddchild,
   (ULONG)&apsAremchild,
   (ULONG)&apsAnotify,
   (ULONG)&apsAllocobject,
   (ULONG)&apsAnotifysetA,
   (ULONG)&apsAllocmem,
   (ULONG)&apsDupstr,
   (ULONG)&apsFreemem,
   (ULONG)&apsClipcoords,
   (ULONG)&apsUnclipcoords,
   (ULONG)&apsErasebg,
   (ULONG)&apsAweb,
   (ULONG)&apsAsetattrsasyncA,
   (ULONG)&apsWaittask,
   (ULONG)&apsGettaskmsg,
   (ULONG)&apsReplytaskmsg,
   (ULONG)&apsChecktaskbreak,
   (ULONG)&apsUpdatetask,
   (ULONG)&apsUpdatetaskattrsA,
   (ULONG)&apsObtaintasksemaphore,
   (ULONG)&apsAvprintf,
   (ULONG)&apsAwebstr,
   (ULONG)&apsTcperrorA,
   (ULONG)&apsTcpmessageA,
   (ULONG)&apsOpentcp,
   (ULONG)&apsLookup,
   (ULONG)&apsExpandbuffer,
   (ULONG)&apsInsertinbuffer,
   (ULONG)&apsAddtobuffer,
   (ULONG)&apsDeleteinbuffer,
   (ULONG)&apsFreebuffer,
   (ULONG)&apsLocale,
   (ULONG)&apsLprintdate,
   (ULONG)&apsObtainbgrp,
   (ULONG)&apsReleasebgrp,
   (ULONG)&apsSetstemvar,
   (ULONG)&apsCopyprefs,
   (ULONG)&apsSaveprefs,
   (ULONG)&apsDisposeprefs,
   (ULONG)&apsFreemenuentry,
   (ULONG)&apsAddmenuentry,
   (ULONG)&apsFreemimeinfo,
   (ULONG)&apsAddmimeinfo,
   (ULONG)&apsFreenocache,
   (ULONG)&apsAddnocache,
   (ULONG)&apsGetstemvar,
   (ULONG)&apsFreeuserbutton,
   (ULONG)&apsAdduserbutton,
   (ULONG)&apsFreepopupitem,
   (ULONG)&apsAddpopupitem,
   (ULONG)&apsAwebversion,
   (ULONG)&apsSyncrequest,
   (ULONG)&apsPprintfA,
   (ULONG)&apsToday,
   (ULONG)&apsFreeuserkey,
   (ULONG)&apsAdduserkey,
   (ULONG)&apsLprintfA,
   (ULONG)&apsFullversion,
   (ULONG)&apsSetfiltertype,
   (ULONG)&apsWritefilter,
   (ULONG)&apsAwebcommand,
   (ULONG)&apsAwebactive,
   (ULONG)&apsFreefontalias,
   (ULONG)&apsAddfontalias,
        (ULONG) -1
};

static struct Library *libbase = NULL;

/*-----------------------------------------------------------------------*/

BOOL Initsupport(void)
{
        libbase = NewCreateLibraryTags(
                LIBTAG_FUNCTIONINIT, (ULONG)jumptab,
        LIBTAG_TYPE, NT_LIBRARY,
                        LIBTAG_BASESIZE, sizeof(struct Library),
        LIBTAG_NAME, (ULONG)version,
                        LIBTAG_FLAGS, 0,
                        LIBTAG_VERSION, APSVERSION,
                        LIBTAG_REVISION, APSREVISION,
                        LIBTAG_IDSTRING, (ULONG)idstring,
        LIBTAG_PUBLIC, TRUE,
                LIBTAG_MACHINE, MACHINE_PPC,
                TAG_DONE);

        if (libbase)
        {
                Forbid();
                libbase->lib_OpenCnt++;
                Permit();
                return TRUE;
        }
        return FALSE;
}

void Freesupport(void)
{
    if(libbase->lib_Node.ln_Succ)
    {
        Forbid();

        while(libbase->lib_OpenCnt>1)
        {
            Permit();

            Lowlevelreq(AWEBSTR(MSG_ERROR_CANTQUIT), "awebsupport.library");

            Forbid();
        }

        libbase->lib_OpenCnt--;
        RemLibrary(libbase);

        Permit();
    }
}



#elif defined (__amigaos__)

struct Jumptab
{
    UWORD  jmp;
    void  *function;
};
#define JMP 0x4ef9

struct Jumptab jumptab[]=
{
   {JMP,apsFreestemvar},
   {JMP,apsAddfontalias},
   {JMP,apsFreefontalias},
   {JMP,apsAwebactive},
   {JMP,apsAwebcommand},
   {JMP,apsWritefilter},
   {JMP,apsSetfiltertype},
   {JMP,apsFullversion},
   {JMP,apsLprintfA},
   {JMP,apsAdduserkey},
   {JMP,apsFreeuserkey},
   {JMP,apsToday},
   {JMP,apsPprintfA},
   {JMP,apsSyncrequest},
   {JMP,apsAwebversion},
   {JMP,apsAddpopupitem},
   {JMP,apsFreepopupitem},
   {JMP,apsAdduserbutton},
   {JMP,apsFreeuserbutton},
   {JMP,apsGetstemvar},
   {JMP,apsAddnocache},
   {JMP,apsFreenocache},
   {JMP,apsAddmimeinfo},
   {JMP,apsFreemimeinfo},
   {JMP,apsAddmenuentry},
   {JMP,apsFreemenuentry},
   {JMP,apsDisposeprefs},
   {JMP,apsSaveprefs},
   {JMP,apsCopyprefs},
   {JMP,apsSetstemvar},
   {JMP,apsReleasebgrp},
   {JMP,apsObtainbgrp},
   {JMP,apsLprintdate},
   {JMP,apsLocale},
   {JMP,apsFreebuffer},
   {JMP,apsDeleteinbuffer},
   {JMP,apsAddtobuffer},
   {JMP,apsInsertinbuffer},
   {JMP,apsExpandbuffer},
   {JMP,apsLookup},
   {JMP,apsOpentcp},
   {JMP,apsTcpmessageA},
   {JMP,apsTcperrorA},
   {JMP,apsAwebstr},
   {JMP,apsAvprintf},
   {JMP,apsObtaintasksemaphore},
   {JMP,apsUpdatetaskattrsA},
   {JMP,apsUpdatetask},
   {JMP,apsChecktaskbreak},
   {JMP,apsReplytaskmsg},
   {JMP,apsGettaskmsg},
   {JMP,apsWaittask},
   {JMP,apsAsetattrsasyncA},
   {JMP,apsAweb},
   {JMP,apsErasebg},
   {JMP,apsUnclipcoords},
   {JMP,apsClipcoords},
   {JMP,apsFreemem},
   {JMP,apsDupstr},
   {JMP,apsAllocmem},
   {JMP,apsAnotifysetA},
   {JMP,apsAllocobject},
   {JMP,apsAnotify},
   {JMP,apsAremchild},
   {JMP,apsAaddchild},
   {JMP,apsArender},
   {JMP,apsAupdateattrsA},
   {JMP,apsAgetattr},
   {JMP,apsAgetattrsA},
   {JMP,apsAsetattrsA},
   {JMP,apsAdisposeobject},
   {JMP,apsAnewobjectA},
   {JMP,apsAmethodasA},
   {JMP,apsAmethodA},
   {JMP,apsExtfunc},
   {JMP,apsExpunge},
   {JMP,apsClose},
   {JMP,apsOpen}
};

struct Library libbase=
{  {  NULL,NULL,NT_LIBRARY,0,version }, /* Node */
   0,                      /* Flags */
   0,                      /* pad */
   sizeof(jumptab),        /* NegSize */
   sizeof(struct Library), /* PosSize */
   APSVERSION,             /* Version */
   APSREVISION,            /* Revision */
   idstring,               /* IdString */
   0,                      /* Sum */
   1,                      /* OpenCnt. Initially 1 to prevent expunging. */
};

/*-----------------------------------------------------------------------*/

BOOL Initsupport(void)
{
    AddLibrary(&libbase);
    return TRUE;
}

void Freesupport(void)
{
    if(libbase.lib_Node.ln_Succ)
    {
        Forbid();

        while(libbase.lib_OpenCnt>1)
        {
            Permit();

            Lowlevelreq(AWEBSTR(MSG_ERROR_CANTQUIT), "awebsupport.library");

            Forbid();
        }

        libbase.lib_OpenCnt--;
        RemLibrary(&libbase);

        Permit();
    }
}

#endif // !(__amigaos4__)
