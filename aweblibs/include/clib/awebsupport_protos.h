#ifndef CLIB_AWEBSUPPORT_PROTOS_H
#define CLIB_AWEBSUPPORT_PROTOS_H


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

   $Id: awebsupport_protos.h,v 1.3 2009/06/15 17:04:08 opi Exp $

   Desc:

***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#include "object.h"
#include "fetchdriver.h"


ULONG AmethodA(struct Aobject *object,struct Amessage *amessage);
ULONG Amethod(struct Aobject *object,...);
ULONG AmethodasA(ULONG objtype,struct Aobject *object,
   struct Amessage *amessage);
ULONG Amethodas(ULONG objtype,struct Aobject *object,...);

APTR AnewobjectA(ULONG objtype,struct TagItem *taglist);
APTR Anewobject(ULONG objtype,...);
void Adisposeobject(struct Aobject *object);
ULONG AsetattrsA(struct Aobject *object,struct TagItem *taglist);
ULONG Asetattrs(struct Aobject *object,...);
ULONG AgetattrsA(struct Aobject *object,struct TagItem *taglist);
ULONG Agetattrs(struct Aobject *object,...);
ULONG Agetattr(struct Aobject *object,ULONG attrid);
ULONG AupdateattrsA(struct Aobject *object,struct TagItem *maplist,
   struct TagItem *taglist);
ULONG Aupdateattrs(struct Aobject *object,struct TagItem *maplist,...);
ULONG Arender(struct Aobject *object,struct Coords *coords,LONG minx,
   LONG miny,LONG maxx,LONG maxy,USHORT flags,APTR text);
ULONG Aaddchild(struct Aobject *object,struct Aobject *child,ULONG relation);
ULONG Aremchild(struct Aobject *object,struct Aobject *child,ULONG relation);
ULONG Anotify(struct Aobject *object,struct Amessage *amessage);

APTR Allocobject(ULONG objtype,LONG size,struct Amset *amset);
ULONG AnotifysetA(struct Aobject *object,struct TagItem *taglist);
ULONG Anotifyset(struct Aobject *object,...);

APTR Allocmem(ULONG size,ULONG flags);
STRPTR Dupstr(STRPTR string,LONG length);
void Freemem(APTR mem);

struct Coords *Clipcoords(struct Aobject *frame,struct Coords *coords);
void Unclipcoords(struct Coords *coords);
void Erasebg(struct Aobject *frame,struct Coords *coords,LONG minx,LONG miny,
   LONG maxx,LONG maxy);

struct Aobject *Aweb(void);

void AsetattrsasyncA(struct Aobject *task,struct TagItem *taglist);
void Asetattrsasync(struct Aobject *task,...);
ULONG Waittask(ULONG signals);
struct Taskmsg *Gettaskmsg(void);
void Replytaskmsg(struct Taskmsg *taskmsg);
ULONG Checktaskbreak(void);
ULONG Updatetask(struct Amessage *amessage);
ULONG UpdatetaskattrsA(struct TagItem *taglist);
ULONG Updatetaskattrs(ULONG attrid,...);
ULONG Obtaintasksemaphore(struct SignalSemaphore *semaphore);

LONG Avprintf(char *format,ULONG *args);
LONG Aprintf(char *format,...);

/*--- private ---*/

UBYTE *Awebstr(ULONG id);
void TcperrorA(struct Fetchdriver *fd,ULONG err,ULONG *args);
void Tcperror(struct Fetchdriver *fd,ULONG err,...);
void TcpmessageA(struct Fetchdriver *fd,ULONG msg,ULONG *args);
void Tcpmessage(struct Fetchdriver *fd,ULONG msg,...);
struct Library *Opentcp(struct Library **base,struct Fetchdriver *fd,BOOL autocon);

struct hostent *Lookup(UBYTE *name,struct Library *base);

BOOL Expandbuffer(struct Buffer *buf,long size);
BOOL Insertinbuffer(struct Buffer *buf,UBYTE *text,long length,long pos);
BOOL Addtobuffer(struct Buffer *buf,UBYTE *text,long length);
void Deleteinbuffer(struct Buffer *buf,long pos,long length);
void Freebuffer(struct Buffer *buf);

struct Locale *Locale(void);
long Lprintdate(UBYTE *buffer,UBYTE *fmt,struct DateStamp *ds);

/*--- public ---*/

struct RastPort *Obtainbgrp(struct Aobject *frame,struct Coords *coords,
   LONG minx,LONG miny,LONG maxx,LONG maxy);
void Releasebgrp(struct RastPort *bgrp);

/*--- private ---*/

void Setstemvar(struct Arexxcmd *ac,UBYTE *stem,long index,UBYTE *field,UBYTE *value);
void Copyprefs(struct AwebPrefs *from,struct AwebPrefs *to);
void Saveprefs(struct AwebPrefs *prefs);
void Disposeprefs(struct AwebPrefs *prefs);
void Freemenuentry(struct Menuentry *me);
struct Menuentry *Addmenuentry(void *list,USHORT type,UBYTE *title,UBYTE scut,UBYTE *cmd);
void Freemimeinfo(struct Mimeinfo *mi);
struct Mimeinfo *Addmimeinfo(struct MimeinfoList *list,UBYTE *type,UBYTE *subtype,
   UBYTE *extensions,USHORT driver,UBYTE *cmd,UBYTE *args);
void Freenocache(struct Nocache *nc);
struct Nocache *Addnocache(struct NocacheList *list,UBYTE *name);
UBYTE *Getstemvar(struct Arexxcmd *ac,UBYTE *stem,long index,UBYTE *field);
void Freeuserbutton(struct Userbutton *ub);
struct Userbutton *Adduserbutton(void *list,UBYTE *label,UBYTE *cmd);
void Freepopupitem(struct Popupitem *pi);
struct Popupitem *Addpopupitem(void *list,USHORT flags,UBYTE *title,UBYTE *cmd);
UBYTE *Awebversion(void);
long Syncrequest(UBYTE *title,UBYTE *text,UBYTE *labels);

/* Allocate buffer and formatted print */
UBYTE *PprintfA(UBYTE *fmt,UBYTE *argspec,UBYTE **params);
UBYTE *Pprintf(UBYTE *fmt,UBYTE *argspec,...);

ULONG Today(void);

void Freeuserkey(struct Userkey *uk);
struct Userkey *Adduserkey(void *list,USHORT key,UBYTE *cmd);

long LprintfA(UBYTE *buffer,UBYTE *fmt,void *params);
long Lprintf(UBYTE *buffer,UBYTE *fmt,...);

BOOL Fullversion(void);

void Setfiltertype(void *handle, UBYTE * type);
void Writefilter(void *handle, UBYTE *type, long length);
long Awebcommand(long portnr, UBYTE *cmd, UBYTE *resultbuf, long length);

BOOL Awebactive(void);

void Freefontalias(struct Fontalias *fa);
struct Fontalias *Addfontalias(struct FontaliasList *list,UBYTE *alias);

void Freestemvar(UBYTE *value);

#ifdef __cplusplus
}
#endif

#endif/*  CLIB_AWEBJS_PROTOS_H  */
