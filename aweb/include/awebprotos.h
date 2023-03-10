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

/* awebprotos.h */

#ifndef AWEBPROTOS_H
#define AWEBPROTOS_H

#include <intuition/intuition.h>

struct Jobject;
struct Jcontext;

/* This structure is already defined in aweb.h */
#ifndef AWEB_H
struct Coords;
#endif

/*-----------------------------------------------------------------------*/
/*-- objects ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Installapplication(void);
extern BOOL Installscroller(void);
extern BOOL Installdocsource(void);
extern BOOL Installdocument(void);
extern BOOL Installdocext(void);
extern BOOL Installbody(void);
extern BOOL Installlink(void);
extern BOOL Installelement(void);
extern BOOL Installtext(void);
extern BOOL Installbreak(void);
extern BOOL Installruler(void);
extern BOOL Installbullet(void);
extern BOOL Installtable(void);
extern BOOL Installmap(void);
extern BOOL Installarea(void);
extern BOOL Installwinhis(void);
extern BOOL Installname(void);
extern BOOL Installframe(void);
extern BOOL Installframeset(void);
extern BOOL Installform(void);
extern BOOL Installfield(void);
extern BOOL Installinput(void);
extern BOOL Installcheckbox(void);
extern BOOL Installradio(void);
extern BOOL Installtextarea(void);
extern BOOL Installselect(void);
extern BOOL Installbutton(void);
extern BOOL Installhidden(void);
extern BOOL Installfilefield(void);
extern BOOL Installurl(void);
extern BOOL Installfetch(void);
extern BOOL Installsource(void);
extern BOOL Installcopy(void);
extern BOOL Installfile(void);
extern BOOL Installcache(void);
extern BOOL Installwindow(void);
extern BOOL Installwhiswindow(void);
extern BOOL Installfilereq(void);
extern BOOL Installsaveas(void);
extern BOOL Installimgsource(void);
extern BOOL Installimgcopy(void);
extern BOOL Installpopup(void);
extern BOOL Installextprog(void);
extern BOOL Installnetstatwin(void);
extern BOOL Installhotlist(void);
extern BOOL Installcabrowse(void);
extern BOOL Installeditor(void);
extern BOOL Installsearch(void);
extern BOOL Installprintwindow(void);
extern BOOL Installprint(void);
extern BOOL Installsaveiff(void);
extern BOOL Installtask(void);
extern BOOL Installsourcedriver(void);
extern BOOL Installcopydriver(void);
extern BOOL Installsoundsource(void);
extern BOOL Installsoundcopy(void);
extern BOOL Installtimer(void);
extern BOOL Installinfo(void);
extern BOOL Installauthedit(void);

   // Frees all classes
extern void Freeobject(void);

/*-----------------------------------------------------------------------*/
/*-- object initializers ------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initcache(void);
extern void Exitcache(void);

extern BOOL Initurl(void);
extern BOOL Initurl2(void);

extern BOOL Initscroller(void);
extern BOOL Initselect(void);
extern BOOL Initcheckbox(void);
extern BOOL Initradio(void);

extern void Freeapplication(void);

/*-----------------------------------------------------------------------*/
/*-- arexx --------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

struct Arexxcmd;

extern BOOL Initarexx(void);
extern void Freearexx(void);

   // Open port for this window. returns port name
extern UBYTE *Openarexxport(ULONG windowkey);

   // Close port for this window
extern void Closearexxport(ULONG windowkey);

   // Send an ARexx command.
extern void Sendarexxcmd(ULONG windowkey,UBYTE *cmd);

   // Get the port number
extern short Arexxportnr(ULONG windowkey);

   // Reply a stalled Arexx command
extern void Replyarexxcmd(struct Arexxcmd *ac);

   // Set variable <stem>.<index>[.<field>]
extern void Setstemvar(struct Arexxcmd *ac,UBYTE *stem,long index,
   UBYTE *field,UBYTE *value);

   // Get variable <stem>.<index>[.<field>]
extern UBYTE *Getstemvar(struct Arexxcmd *ac,UBYTE *stem,long index,UBYTE *field);

   // Free the result returned by Getstemvar()

extern void Freestemvar(UBYTE *value);

   // Execute one command with parameter substitution
VARARGS68K_PROTO(extern void Execarexxcmd(ULONG windowkey,UBYTE *cmd,UBYTE *argspec,...));

   // Find the port number for this window
extern long Arexxportnumber(ULONG windowkey);

   // Execute one command from support library
extern long Supportarexxcmd(long portnr,UBYTE *cmd,UBYTE *resultbuf,long length);

/*-----------------------------------------------------------------------*/
/*-- author -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initauthor(void);
extern void Freeauthor(void);

   // Returns (copy of) known authorization, or a new one
extern struct Authorize *Newauthorize(UBYTE *server,UBYTE *realm);
extern void Freeauthorize(struct Authorize *auth);

   // Returns guessed authorization, or NULL
extern struct Authorize *Guessauthorize(UBYTE *server);

extern struct Authorize *Dupauthorize(struct Authorize *auth);

   // Start authorization dialog
extern BOOL Doauthorize(struct Authorize *auth,void *fetch);

   // Flush all authorizations
extern void Flushauthor(void);

   // Save authorizations
extern void Saveauthor(void);

   // Forget authorization details
extern void Forgetauthorize(struct Authorize *auth);

   // Set authorization details
extern void Setauthorize(struct Authorize *auth,UBYTE *userid,UBYTE *passwd);

struct Fetchdriver;

extern void Authorize(struct Fetchdriver *fd,struct Authorize *auth,BOOL proxy);

extern void Openauthedit(void);
extern void Closeauthedit(void);

extern BOOL Isopenauthedit(void);

/*-----------------------------------------------------------------------*/
/*-- aweb ---------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // find the translation table for a given charset

extern UBYTE *Findcharset(UBYTE *name);

   // ensure buffer is large enough to add (size)
extern BOOL Expandbuffer(struct Buffer *buf,ULONG size);

   // free buffer contents
extern void Freebuffer(struct Buffer *buf);

   // add string to buffer
extern BOOL Addtobuffer(struct Buffer *buf,UBYTE *text,long length);

   // insert string into buffer on position (pos)
extern BOOL Insertinbuffer(struct Buffer *buf,UBYTE *text,ULONG length,
   ULONG pos);

   // delete characters from buffer
extern void Deleteinbuffer(struct Buffer *buf,ULONG pos,ULONG length);

   // duplicate string (dynamic). If (length)<0, strlen(str) is taken
extern UBYTE *Dupstr(UBYTE *str,long length);

   // build a HTML tag string from components
extern void AddtagstrA(struct Buffer *buf,UBYTE *keywd,UWORD f,ULONG value);
#define Addtagstr(b,k,f,v) AddtagstrA(b,k,f,(ULONG)(v))

   // copy our pathlist
extern long Copypathlist(void);
extern void Freepathlist(long plbptr);

   // sprintf type but with letter-identified argument specifiers
extern long Pformatlength(UBYTE *format,UBYTE *argspec,UBYTE **params);
extern UBYTE *Pformat(UBYTE *buffer,UBYTE *format,UBYTE *argspec,UBYTE **params,BOOL quote);

   // spawn external command. if (del), delete file named (1st param) afterwards
VARARGS68K_PROTO(extern BOOL Spawn(BOOL del,UBYTE *cmd,UBYTE *args,UBYTE *argspec,...));

   // display quit requester or quit immediate
extern void Quit(BOOL immediate);

   // Defer iconification
extern void Iconify(BOOL iconify);

   // Do a SetGadgetAttrs and a RefreshGList if necessary
VARARGS68K_PROTO(extern void Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...));

   // Get BOOPSI object attribute
extern long Getvalue(struct Gadget *gad, ULONG tag);

   // Get GA_Selected gadget attribute
extern BOOL Getselected(struct Gadget *gad);

   // sprintf() with locale support
extern long LprintfA(UBYTE *buffer,UBYTE *fmt,void *args);
VARARGS68K_PROTO(extern long Lprintf(UBYTE *buffer,UBYTE *fmt,...));

   // sprintf() for locale formatted date
extern long Lprintdate(UBYTE *buffer,UBYTE *fmt,struct DateStamp *ds);

   // show a requester for low-level error message
extern void Lowlevelreq(STRPTR msg,...);

   // Open classact class

   struct Interface;

extern struct Library *Openclass(UBYTE *name,long version,struct Library **base,struct Interface **iface);

   // Open a library;

extern struct Library *Openlib(UBYTE* name, long version,struct Library **base, struct Interface **iface);
extern struct Library *Openlibnofail(UBYTE* name, long version,struct Library **base, struct Interface **iface);

extern void  Closelib(struct Library **base, struct Interface **iface);

   // General clip and unclip functions
extern ULONG Clipto(struct RastPort *rp,short minx,short miny,short maxx,short maxy);
extern void Unclipto(ULONG key);

   // Easy Coords fallback interface. Call Clipcoords with your frame and either
   // an existing Coords or NULL. If NULL, a new Coords will be created and
   // the rastport will be clipped.
   // Match every Clipcoords() with an Unclipcoords() call.
extern struct Coords *Clipcoords(void *cframe,struct Coords *coo);
extern void Unclipcoords(struct Coords *coo);

   // Is character valid in url?
extern BOOL Isurlchar(UBYTE c);

   // Is character really printable?
extern BOOL Isprint(UBYTE c);

   // Is character whitespace?
extern BOOL Isspace(UBYTE c);

   // Is character valid in SGML name?
extern BOOL Issgmlchar(UBYTE c);

   // Is character alphabetic (latin1)?
extern BOOL Isalpha(UBYTE c);

   // Is character alphanumeric?
extern BOOL Isalnum(UBYTE c);

   // Today's time stamp
extern ULONG Today(void);

   // Make datestamp from string
extern ULONG Scandate(UBYTE *buf);

   // Make HTTP date format from stamp. (buf) must be >=30 bytes
extern void Makedate(ULONG stamp, UBYTE *buf);

   // Get the application object
extern struct Aobject *Aweb(void);

   // Safely close a window with shared port
extern void Safeclosewindow(struct Window *w);

   // Create a dynamic string with the full name for this url from the save path
extern UBYTE *Savepath(void *url);

   // Set a signal processing function
extern void Setprocessfun(short sigbit,void (*fun)(void));

   // get full name in dynamic string. Free yourself after use.
extern UBYTE *Fullname(UBYTE *name);

   // Check if file is read-only
extern BOOL Readonlyfile(UBYTE *name);

   // One or more layouts have changed
extern void Changedlayout(void);

   // Flush excess sources when idle
extern void Deferflushmem(void);

   // Wait for signals plus extramask, process signals, return on any extramask bit.
extern ULONG Waitprocessaweb(ULONG extramask);

   // Queue this object to receive AOM_SET asap with this AOBJ_Queueid.
   // Set queueid to 0 to cancel.
extern void Queuesetmsg(void *object,ULONG queueid);
extern void Queuesetmsgdata(void *object,ULONG queueid,ULONG userdata);

   // Open aweblib module with exact version and revision
extern void *Openaweblib(UBYTE *name,ULONG version);

   // Open the awebjs library if not already open
extern void *Openjslib(void);


   // Close a library
   // Arg is either a library base for 3.x or a Interface for 4.x
extern void Closeaweblib(void*);

extern void Remaweblib(void*);


   // Load requester
extern void Openloadreq(struct Screen *screen);
extern void Setloadreqstate(UWORD state);
extern void Setloadreqlevel(long ready,long total);

   // Return TRUE if less than 4 kB left on the stack
extern BOOL Stackoverflow(void);

   // Return TRUE if AWeb owns the active window
extern BOOL Awebactive(void);

#ifdef BETAKEYFILE
extern void Mprintf(UBYTE *fmt,...);
#endif

/*-----------------------------------------------------------------------*/
/*--- Awebamissl --------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

    // Initialise The AwebAmiSsl Library
extern BOOL Initawebamissl(void);
    // Cleanup awebamissl
extern void Freeawebamissl(void);

/*-----------------------------------------------------------------------*/
/*--- Awebamictp --------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

    // Initialise The AWebAmiTcp Library
extern BOOL Initawebamitcp(void);
    // Cleanup awebamitcp
extern void Freeawebamitcp(void);


/*-----------------------------------------------------------------------*/
/*--- Awebtcp --------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

    // Initialise The AWebTcp Library
extern BOOL Initawebtcp(void);
    // Cleanup awebtcp
extern void Freeawebtcp(void);



/*-----------------------------------------------------------------------*/
/*-- body ---------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

struct Body;

   // Find the owner FRAME object
extern void *Bodyframe(struct Body *bd);

   // Get background info, and forward to owner Framecoords()
extern void Bodycoords(struct Body *body,struct Coords *coo);

/*-----------------------------------------------------------------------*/
/*-- boopsi -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // initialize classes
extern BOOL Initboopsi(void);

   // close classes
extern void Freeboopsi(void);

   // return class pointer
extern void *Gadimgclass(void);
extern void *Stagadclass(void);
extern void *Ledgadclass(void);

/*-----------------------------------------------------------------------*/
/*-- cabrowse -----------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Open, close the cache browser
extern void Opencabrowse(void);
extern void Closecabrowse(void);

extern BOOL Isopencabrowse(void);

/*-----------------------------------------------------------------------*/
/*-- clip ---------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Write a string to the clipboard
extern void Clipcopy(UBYTE *text,long length);

   // Read a string from the clipboard. Returns nr of bytes read (or <0 if error)
extern long Clippaste(UBYTE *buf,long length);

/*-----------------------------------------------------------------------*/
/*-- cookie -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initcookie(void);
extern void Freecookie(void);

   // Store info from Set-Cookie: line. originator is URL, cookiespec is
   // data after Set-Cookie:
extern void Storecookie(UBYTE *originator,UBYTE *cookiespec,ULONG serverdate, ULONG responsetime);

   // Build a Cookie: header string terminated by CRLF
extern UBYTE *Findcookies(UBYTE *url,BOOL secure);

   // Build the JS cookie string */
extern UBYTE *Getjcookies(UBYTE *url);

   // Flush all cookies beyond this memory limit
extern void Flushcookies(long max);

   // Get/Set all cookies for ARexx
extern void Getrexxcookies(struct Arexxcmd *ac,UBYTE *stem);
extern void Setrexxcookies(struct Arexxcmd *ac,UBYTE *stem,BOOL add);

/*-----------------------------------------------------------------------*/
/*-- copyjs -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Add the JS Image() constructor to this object */
extern void Addimageconstructor(struct Jcontext *jc,struct Jobject *parent);

/*-----------------------------------------------------------------------*/
/*-- element ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Calculate text length, working around word arithmatic bug in OS
extern long Textlength(struct RastPort *rp,UBYTE *text,long count);

   // Calculate text length, and return real pixel length in (extent)
extern long Textlengthext(struct RastPort *rp,UBYTE *text,long count,long *extent);

/*-----------------------------------------------------------------------*/
/*-- event --------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern void Processwindow(void);

   // relayout and render changed frames
extern void Doupdateframes(void);

   // re-display all windows
extern void Redisplayall(void);

   // set load images menu checkmark, start loading for all windows.
   // also set backround images and sound checkmarks
extern void Setloadimg(void);
extern void Setdocolors(void);
extern void Setdobgsound(void);

   // input a new url in a frame in this window
extern void Inputwindoc(void *win, void *url,UBYTE *fragment,UBYTE *frameid);
extern void Inputwindocnoref(void *win, void *url,UBYTE *fragment,UBYTE *frameid);
extern void Inputwindocreload(void *win, void *url,UBYTE *fragment,UBYTE *frameid);

   // Input a new url the smart way
extern void Inputwindocsmart(void *win,UBYTE *url,UBYTE *frameid);

struct Awindow;

   // Go forward or backward in history
extern void Gohistory(struct Awindow *win,long n);

   // Process refresh events
extern void Refreshevents(void);

   // Open a file requester for open with optional pattern
extern void Openfilereq(struct Awindow *win,UBYTE *pattern);

   // Open URL requester for open
extern void Openurlreq(struct Awindow *win);

/*-----------------------------------------------------------------------*/
/*-- form ---------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Copy (p) to (buf), URLencoded if necessary
extern void Urlencode(struct Buffer *buf,UBYTE *p,long len);

/*-----------------------------------------------------------------------*/
/*-- frame --------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

struct Frame;

   // get coordinate offsets for this frame
extern void Framecoords(struct Frame *frame,struct Coords *coo);

extern void Erasebg(struct Frame *frame,struct Coords *coo,long xmin,long ymin,long xmax,long ymax);
   /* Erase background with image or colour from Coords, coordinates are
    * relative to Coords offsets. */

extern struct RastPort *Obtainbgrp(struct Frame *frame,struct Coords *coo,
   long xmin,long ymin,long xmax,long ymax);
   /* Obtain a RastPort and BitMap initialized to the current background. */

extern void Releasebgrp(struct RastPort *rp);
   /* Free the rastport obtained with Obtainbgrp(). */

extern UBYTE *Rexxframeid(struct Frame *frame);
   /* Create Rexx frame ID string (dynamic) */

/*-----------------------------------------------------------------------*/
/*-- framejs ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Run JavaScript program. Use frame's object if jthisp is NULL or points
   // to NULL. A pointer is used because the actual object may not yet exist
   // when this function is called, in which case it is created from within
   // this function.
   // Returns TRUE if failed, FALSE means run ok and cancel default action.
extern BOOL Runjavascript(struct Frame *frame,UBYTE *script,struct Jobject **jthisp);

   // Same but suppress banner windows if so configured
extern BOOL Runjsnobanners(struct Frame *frame,UBYTE *script,struct Jobject **jthisp);

   // Same as above but the with object is asked for an extra JS object
   // to be included in the global with scope
BOOL Runjavascriptwith(struct Frame *fr,UBYTE *script,struct Jobject **jthisp,
   struct Aobject *with);

   // Get the frame object from the JS context
extern void *Getjsframe(struct Jcontext *jc);

   // Get the document (copydriver) object from the JS context
extern void *Getjsdocument(struct Jcontext *jc);

   // Get the current JS generated text if it is for this url.
   // Returns TRUE if text is complete (closed). Bufferp will be set to
   // NULL if no text is available.
extern BOOL Getjsgeneratedtext(UBYTE *urlname,UBYTE **bufferp);

   // Get the current URL name (static) in the frame that is running the JS program
   // In case of redirected URL, this is the actual URL of the source.
extern UBYTE *Getjscurrenturlname(struct Jcontext *jc);

/*-----------------------------------------------------------------------*/
/*-- hotlist ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // open, close the window
extern void Hotlistviewer(ULONG windowkey);
extern void Hotlistmanager(ULONG windowkey);
extern void Closehotlistviewer(void);
extern void Closehotlistmanager(void);

extern BOOL Isopenhotlistviewer(void);
extern BOOL Isopenhotlistmanager(void);

   // build html formatted hotlist, write to temp file
extern void Buildhtmlhotlist(void *tf);

   // Add doc to hotlist
extern void Addtohotlist(void *url,UBYTE *title,UBYTE *group);

   // Set hotlist name
extern void Sethotlistname(UBYTE *name);

   // Save hotlist if changed
extern void Savehotlist(void);

   // Restore hotlist
extern void Restorehotlist(void);

   // Get, set hotlist contents (ARexx)
extern void Gethotlistcontents(struct Arexxcmd *ac,UBYTE *stem,BOOL groupsonly);
extern void Sethotlistcontents(struct Arexxcmd *ac,UBYTE *stem);

/*-----------------------------------------------------------------------*/
/*-- http ---------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Inithttp(void);
extern void Freehttp(void);

   // http protocol subtask

extern void Httptask(struct Fetchdriver *fd);

/*-----------------------------------------------------------------------*/
/*-- info ---------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Returns info window's dimensions
extern void Getinfodim(short *x,short *y,short *w,short *h);

/*-----------------------------------------------------------------------*/
/*-- key ----------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern short Initkey(void);
extern void Freekey(void);

   // return dynamic string
extern unsigned char *Aboutstring(void);

extern BOOL Registered(void);

extern void Checkregisterinfo(void);

/*-----------------------------------------------------------------------*/
/*-- local --------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // local file protocol subtask

extern void Localfiletask(struct Fetchdriver *fd);

/*-----------------------------------------------------------------------*/
/*-- locale -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

#include "platform_specific.h"

STRPTR GetString(struct LocaleInfo *, long);
/*
USRFUNC_P2
(
    STRPTR, GetString,
    struct LocaleInfo *, li,        A0,
    long,                stringnum, D0
);
*/

/*-----------------------------------------------------------------------*/
/*-- memory -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initmemory(void);
extern void Freememory(void);

   // allocate in private pool. If pool==NULL, allocate unpooled
extern void *Pallocmem(long size,ULONG flags,void *pool);

   // allocate pooled memory
extern void *Allocmem(long size,ULONG flags);

   // free memory, works for all pools and unpooled memory
extern void Freemem(void *mem);

/*-----------------------------------------------------------------------*/
/*-- mime ---------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initmime(void);
extern void Freemime(void);

   // delete all known mimetypes
extern void Reinitmime(void);

   // add a mimetype. Will modify (*exts).
extern void Addmimetype(UBYTE *type,UBYTE *exts,UWORD driver,UBYTE *cmd,UBYTE *args);

   // Find full MIME type from name
extern UBYTE *Mimetypefromext(UBYTE *name);

   // Check if text MIME type is acceptable, returns FALSE if not.
extern BOOL Checkmimetype(UBYTE *data,long length,UBYTE *type);

   // Find full MIME type from data contents, obeying default type
extern UBYTE *Mimetypefromdata(UBYTE *data,long length,UBYTE *deftype);

   // Get mime driver (MIMEDRV_xxx) for this mime type
   // Name of program or plugin returned in *name, args in *args
extern ULONG Getmimedriver(UBYTE *mimetype,UBYTE *url,UBYTE **name,UBYTE **args);

   // Check if mime type is XBM image
extern BOOL Isxbm(UBYTE *mimetype);

/*-----------------------------------------------------------------------*/
/*-- nameserv -----------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initnameserv(void);
extern void Freenameserv(void);

   // Look up host name
extern struct hostent *Lookup(UBYTE *name,struct Library *base);

/*-----------------------------------------------------------------------*/
/*-- netstat ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Open,close netstat window
extern void Opennetstat(void);
extern void Closenetstat(void);

extern BOOL Isopennetstat(void);

   // Add process, returns key
extern void *Addnetstat(void *fetch,UBYTE *url,ULONG status,BOOL cps);

   // Change process
extern void Chgnetstat(void *key,ULONG status,ULONG read,ULONG total);

   // Returns netstat window's dimensions
extern void Getnetstatdim(short *x,short *y,short *w,short *h);

   // Set new preferred dimensions
extern void Setnetstatdim(short x,short y,short w,short h);

   // Cancel all transfers
extern void Cancelnetstatall(void);

   // Get transfers in ARexx stem
extern void Gettransfers(struct Arexxcmd *ac,UBYTE *stem);

/*-----------------------------------------------------------------------*/
/*-- prefs --------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initprefs(void);
extern BOOL Initprefs2(void);
extern void Freeprefs(void);

extern BOOL Setprefsname(UBYTE *name);

extern void Startsettings(UWORD type);
extern void Closesettings(void);
extern void Snapshotwindows(void *window);
extern void Prefsloadimg(long loadimg);
extern void Prefsdocolors(BOOL docolors);
extern void Prefsdobgsound(BOOL dobgsound);
extern void Saveallsettings(void);
extern void Savesettingsas(UBYTE *path);
extern void Loadsettings(UBYTE *path);

extern void Addtonocookie(UBYTE *name);

struct Jcontext;
struct Jobject;

extern void Jsetupprefs(struct Jcontext *jc,struct Jobject *jnav);

   // Find matching font from font alias list.
extern struct Fontprefs *Matchfont(UBYTE *face,short size,BOOL fixed);

/*-----------------------------------------------------------------------*/
/*-- print --------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Start print for this window's contents
extern void Printdoc(void *win,BOOL debug);

   // Start print from ARexx. Returns TRUE if success.
extern BOOL Printarexx(void *win,long scale,BOOL center,BOOL ff,BOOL bg,
   struct Arexxcmd *wait,BOOL debug);

/*-----------------------------------------------------------------------*/
/*-- request ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initrequest(void);
extern void Freerequest(void);

   // Close all sync requesters
extern void Closerequests(void);

   // Open requester
extern void Aboutreq(UBYTE *portname);
extern void Closeabout(void);
extern BOOL Quitreq(void);
extern void Unregreq(void);

extern BOOL Isopenabout(void);

   // Open general requester. Frees (text) afterwards. (Unless FALSE return)
extern BOOL Asyncrequest(UBYTE *title,UBYTE *text,UBYTE *label,requestfunc *f,void *data);

   // Same, but add button "Copy url to clipboard"
extern BOOL Asyncrequestcc(UBYTE *title,UBYTE *text,UBYTE *label,requestfunc *f,
   void *data,UBYTE *url);

   // Open general prompt requester. Frees (text) afterwards. (Unless FALSE return)
   // (string) must be a buffer of at least 128 bytes where the result is copied.
extern BOOL Asyncpromptrequest(UBYTE *title,UBYTE *text,UBYTE *label,requestfunc *f,
   void *data,UBYTE *string);

   // Show synchroneous requester, return gadget id (1,2,3,0)
extern long Syncrequest(UBYTE *title,UBYTE *text,UBYTE *labels,long delay);

   // Same, but add button "Copy url to clipboard"
extern long Syncrequestcc(UBYTE *title,UBYTE *text,UBYTE *labels,UBYTE *url);

   // Synchroneous text prompt requester. Returns dynamic string or NULL.
extern UBYTE *Promptrequest(UBYTE *text,UBYTE *defstr);

   // General cancellable progress requester
extern struct Progressreq *Openprogressreq(UBYTE *labeltext);
extern void Setprogressreq(struct Progressreq *pr,long level,long max);
extern BOOL Checkprogressreq(struct Progressreq *pr);
extern void Closeprogressreq(struct Progressreq *pr);
extern void Progresstofront(struct Progressreq *pr);

   // Demo annoy requester
extern void Demorequest(void);

/*-----------------------------------------------------------------------*/
/*-- saveiff ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Save a copy of this window as IFF ILBM
extern void Saveasiff(void *win,UBYTE *name,BOOL noicon,struct Arexxcmd *wait);

/*-----------------------------------------------------------------------*/
/*-- search -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initsearch(void);
extern void Freesearch(void);

extern void Opensearch(void *win);
extern void Closesearch(void *win);

/*-----------------------------------------------------------------------*/
/*-- select -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Add JS Object() constructor
extern void Addoptionconstructor(struct Jcontext *jc,struct Jobject *parent);

/*-----------------------------------------------------------------------*/
/*-- string -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initstring(void);
extern void Freestring(void);

extern Class *Stringclass(void);

/*-----------------------------------------------------------------------*/
/*-- support ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initsupport(void);
extern void Freesupport(void);

/*-----------------------------------------------------------------------*/
/*-- tcp ----------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Inittcp(void);
extern void Freetcp(void);

   // Open TCP library. Returns AwebTcpBase if successful.
   // Doesn't auto connect if (autocon) is FALSE
extern struct Library *Opentcp(struct Library **base,struct Fetchdriver *fd,BOOL autocon);

/*-----------------------------------------------------------------------*/
/*-- tcperr -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // build and send html-formatted error message
#ifndef __MORPHOS__
VARARGS68K_PROTO(extern  void Tcperror(struct Fetchdriver *fd,ULONG err,...));
#else
#define Tcperror(___par1, ___par2, ...) ({APTR _args[] = { __VA_ARGS__ }; TcperrorA((___par1), (___par2), (ULONG *) _args); })
#endif
extern void TcperrorA(struct Fetchdriver *fd,ULONG err,ULONG *args);

   // build and send message
#ifndef __MORPHOS__
VARARGS68K_PROTO(extern  void Tcpmessage(struct Fetchdriver *fd,ULONG msg,...));
#else
#define Tcpmessage(___par1, ___par2, ...) ({APTR _args[] = { __VA_ARGS__ }; TcpmessageA((___par1), (___par2), (ULONG *) _args); })
#endif
extern void TcpmessageA(struct Fetchdriver *fd,ULONG msg,ULONG *args);

/*-----------------------------------------------------------------------*/
/*-- tooltip ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Set tooltip text and location. Text must be dynamic; will be taken over.
extern void Tooltip(UBYTE *text,long mousex,long mousey);

   // Set tooltip location with same text
extern void Tooltipmove(long mousex,long mousey);

   // Final cleanup of tooltip
extern void Freetooltip(void);

/*-----------------------------------------------------------------------*/
/*-- version ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

extern BOOL Initversion(void);

extern void Initialrequester(void (*about)(UBYTE *),UBYTE *p);

/*-----------------------------------------------------------------------*/
/*-- window -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Rebuild the user button bar in all windows
extern void Rebuildallbuttons(void);

   // Close and re-open all windows
extern void Reopenallwindows(void);

   // Set or clear the busy pointer for all windows
extern void Busypointer(BOOL busy);

   // Setup javascript environment for all windows
extern void Jsetupallwindows(struct Jcontext *jc);

   // Open a new window in JavaScript. Returns JS object.
extern void *Jopenwindow(struct Jcontext *jc,struct Jobject *opener,
   UBYTE *urlname,UBYTE *name,UBYTE *spec,struct Awindow *oldwin);

   // Close this window in JavaScript.
extern void Jclosewindow(struct Awindow *win);

   // Check if any URL is inputting for this window and en/disable cancel gadget
   // Call this whenever a fetch starts or ends.
extern void Inputallwindows(BOOL input);

/*-----------------------------------------------------------------------*/
/*-- whiswin ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Open, close history window
extern void Openwhiswindow(void *whis,long windownr);
extern void Closewhiswindow(void);

   // Window history has changed
extern void Changewhiswindow(long windownr);

   // Set new current winhis in history window
extern void Setwhiswindow(void *whis);

extern BOOL Isopenwhiswindow(void);

/*-----------------------------------------------------------------------*/
/*-- winhis -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // Get ARexx history
extern void Historyarexx(struct Arexxcmd *ac,long *windownr,BOOL mainline,UBYTE *stem);

/*-----------------------------------------------------------------------*/
/*-- winrexx ------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // do arexx command. Returns TRUE if done with the command
extern BOOL Doarexxcmd(struct Arexxcmd *ac);

/*-----------------------------------------------------------------------*/
/*-- xaweb --------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

   // internal protocol subtask

extern void Xawebtask(struct Fetchdriver *fd);

/*-----------------------------------------------------------------------*/
/*--        -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*--        -------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/


#endif
