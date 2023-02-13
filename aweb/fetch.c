
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

/* fetch.c - AWeb html fetch object */

#include "aweb.h"
#include "fetch.h"
#include "url.h"
#include "fetchdriver.h"
#include "application.h"
#include "task.h"
#include "window.h"
#include "jslib.h"
#include "form.h"
#include "versions.h"
#include <proto/utility.h>
#include "proto/awebsubtask.h"
/*------------------------------------------------------------------------*/

struct Fetch
{
    struct Aobject  object;
    void           *url;
    struct Fetchdriver *fd;
    UBYTE          *name;
    UBYTE          *postmsg;
    struct Multipartdata *mpd;
    UBYTE          *referer;
    ULONG           flags;
    void           *jframe;
    long            total, sofar;
    ULONG           serverdate;
    ULONG           responsetime;               /* Local time at which response is received */
    UBYTE          *etag;
    UBYTE           statusbuf[STATUSBUFSIZE];
    void           *netstat;
    ULONG           validate;
    ULONG           windowkey;
    ULONG           loadflags;
    long            channelid;
    void            *fdbase;
    void            (*driverfun) (struct Fetchdriver *);
    void           *task;
};

#define FCHF_RUNNING       0x00000001   /* Fetchdriver is running */
#define FCHF_NOCACHE       0x00000002   /* Don't use caches */
#define FCHF_CACHE         0x00000004   /* Cache reload */
#define FCHF_CANCELLED     0x00000008   /* Cancelled, don't show 'incomplete' requester */
#define FCHF_NOTMODIFIED   0x00000010   /* Not modified, terminate when data arrives */
#define FCHF_DISPOSED      0x00000020   /* We are disposed, don't forward anything but terminate */
#define FCHF_INCOMPLETE    0x00000040   /* Incomplete requester is open */
#define FCHF_COMMANDS      0x00000080   /* Shell, ARexx commands are allowed */
#define FCHF_LOCALSLOT     0x00000100   /* Takes up local file slot. */
#define FCHF_NETSLOT       0x00000200   /* Takes up network connection slot. */
#define FCHF_QUEUED        0x00000400   /* Queued, must be removed if disposed. */
#define FCHF_USESLOT       0x00000800   /* Occupies a slot */
#define FCHF_NOPROXY       0x00001000   /* Don't use proxy */
#define FCHF_POSTNOGOOD    0x00002000   /* Retry with GET when driver terminates */
#define FCHF_IMAGE         0x00004000   /* Fetch counts as image fetch */
#define FCHF_FORMWARN      0x00008000   /* Warn if form is sent over unsecure link */
#define FCHF_CHANNEL       0x00010000   /* This is a channel fetch */

static LIST(Fetch) netqueue;
    static LIST(Fetch) localqueue;
    static LIST(Fetch) running; /* Also fetches in authorization state */
    static LIST(Fetch) channels;

    static long nrlocal, nrnet;

    static long channelid = 0;

    static BOOL Startdriver(struct Fetch *fch);

    struct Waitrequest
    {
        NODE(Waitrequest);
        struct Arexxcmd *cmd;
        ULONG windowkey;
        BOOL doc, img;
        void           *url;
    };

    static LIST(Waitrequest) waitrequests;

#define AOFCC_Arexxcmd     (AOFCC_Dummy+64)

#ifdef DEVELOPER
extern BOOL charsetdebug;
#endif
/*------------------------------------------------------------------------*/


/* Error scheme task. The validate field is mis-used as msg id. */

void Errorschemetask(struct Fetchdriver *fd)
{
    UBYTE          *scheme = NULL, *p;
    long            length;

    if (fd->name)
    {
        p = strchr(fd->name, ':');
        if (p)
            scheme = Dupstr(fd->name, p - fd->name);
        else
            scheme = Dupstr(fd->name, -1);
        if (scheme)
        {
            if (haiku)
            {
                strcpy(fd->block, "<html>");
                switch (fd->validate)
                {
                    case MSG_EPART_NOPROGRAM:
                        strcat(fd->block, HAIKU9);
                        break;
                    case MSG_EPART_NOAWEBLIB:
                        strcat(fd->block, HAIKU14);
                        break;
                    case MSG_EPART_ADDRSCHEME:
                        strcat(fd->block, HAIKU15);
                        break;
                }
                length = strlen(fd->block);
            }
            else
            {
                length =
                    sprintf(fd->block,
                            "<html><h1>%s</h1>%s %.1000s<p><strong>",
                            AWEBSTR(MSG_EPART_ERROR),
                            AWEBSTR(MSG_EPART_RETURL), fd->name);
                length +=
                    sprintf(fd->block + length, AWEBSTR(fd->validate),
                            scheme);
            }
            Updatetaskattrs(AOURL_Error, TRUE,
                            AOURL_Data, fd->block,
                            AOURL_Datalength, length, TAG_END);
            FREE(scheme);
        }
    }

    Updatetaskattrs(AOTSK_Async, TRUE,
                    AOURL_Eof, TRUE, AOURL_Terminate, TRUE, TAG_END);
}

/*------------------------------------------------------------------------*/

/* Javascript: URL generated text task. The postmsg field is mis-used as
 * text buffer; the validate field is mis-used as date-to-expire. */

void Javascripttask(struct Fetchdriver *fd)
{
    if (fd->postmsg)
    {
        Updatetaskattrs(AOURL_Expires, fd->validate,
                        AOURL_Data, fd->postmsg,
                        AOURL_Datalength, strlen(fd->postmsg), TAG_END);
    }
    Updatetaskattrs(AOTSK_Async, TRUE,
                    AOURL_Eof, TRUE, AOURL_Terminate, TRUE, TAG_END);
}

/*------------------------------------------------------------------------*/

/* JS generated text task. The postmsg field is mis-used as text buffer. */

void  Jsgeneratedtask(struct Fetchdriver *fd)
{
    if (fd->postmsg)
    {
        Updatetaskattrs(AOURL_Contenttype, "text/html",
                        AOURL_Jsopen, BOOLVAL(fd->flags & FDVF_JSOPEN),
                        AOURL_Data, fd->postmsg,
                        AOURL_Datalength, strlen(fd->postmsg), TAG_END);
    }
    Updatetaskattrs(AOTSK_Async, TRUE,
                    AOURL_Eof, TRUE, AOURL_Terminate, TRUE, TAG_END);
}

/*------------------------------------------------------------------------*/

/* Channel task. */

/* Process headers (shouldn't this be shared with http.c?) */
static void Channelheader(struct Fetchdriver *fd, UBYTE * header)
{
    Updatetaskattrs(AOURL_Header, header, TAG_END);

    if (STRNIEQUAL(header, "Date:", 5))
    {
        fd->serverdate = Scandate(header + 5);
        fd->responsetime = Today();
        Updatetaskattrs(AOURL_Serverdate, fd->serverdate,
                        AOFCH_Responsetime,fd->responsetime,
                         TAG_END);

    }
    else if (STRNIEQUAL(header, "Last-Modified:", 14))
    {
        ULONG           date = Scandate(header + 14);

        Updatetaskattrs(AOURL_Lastmodified, date, TAG_END);
    }
    else if (STRNIEQUAL(header, "Expires:", 8))
    {
        ULONG           expires = Scandate(header + 8);

        Updatetaskattrs(AOURL_Expires, expires, TAG_END);
    }
    else if (STRNIEQUAL(header, "Pragma:", 7))
    {
        UBYTE          *p;

        for (p = header + 7; *p && isspace(*p); p++);
        if (STRNIEQUAL(p, "No-Cache", 8))
        {
            Updatetaskattrs(AOURL_Nocache, TRUE, TAG_END);
        }
    }
    else if (STRNIEQUAL(header, "Content-Length:", 15))
    {
        long            i = 0;

        sscanf(header + 15, " %ld", &i);
        Updatetaskattrs(AOURL_Contentlength, i, TAG_END);
    }
    else if (STRNIEQUAL(header, "Content-Type:", 13))
    {
        if (!prefs.network.ignoremime)
        {
            UBYTE          *p, *q, *r;
            UBYTE          charset[32]="";

            for (p = header + 13; *p && isspace(*p); p++);
            q = strchr(p, ';');
            if (q)
                *q = '\0';
            if (strlen(p) > 31)
                p[31] = '\0';
            if(q && STRNIEQUAL(p,"TEXT/",5))
            {
               for(q++;*q && !STRNIEQUAL(q,"CHARSET=",8);q++);
               if(*q)
               {  q+=8;
                  while(*q && isspace(*q)) q++;
                  if(*q=='"')
                  {  q++;
                     for(r=q;*r && *r!='"';r++);
                     *r='\0';
                  }
                  else
                  {  for(r=q;*r && !isspace(*r);r++);
                     *r='\0';
                  }
                  strcpy(charset,q);
#ifdef DEVELOPER
                  if (charsetdebug) printf ("fetch.c/Channelheader(): determined character set: %s\n",charset);
#endif
               }
#ifdef DEVELOPER
               else
                  if (charsetdebug) printf ("fetch.c/Channelheader(): character set not specified\n");
#endif
            }
#ifdef DEVELOPER
            else
               if (charsetdebug) printf ("fetch.c/Readheader(): non-text document or no additional specification\n");
#endif
            Updatetaskattrs(AOURL_Contenttype, p, AOURL_Charset, charset, TAG_END);
        }
    }
    else if (STRNIEQUAL(header, "Set-Cookie:", 11))
    {
        if (prefs.network.cookies)
        {
            Storecookie(fd->name, header + 11, fd->serverdate, fd->responsetime);
        }
    }
    else if (STRNIEQUAL(header, "Refresh:", 8))
    {
        Updatetaskattrs(AOURL_Clientpull, header + 8, TAG_END);
    }
}

/* The actual subtask */

void Channeltask(struct Fetchdriver *fd)
{
    struct Taskmsg *msg;
    struct Amset   *ams;
    struct TagItem *tstate, *tag;
    BOOL            done = FALSE;

    while (!done)
    {
        Waittask(0);
        while (!done && (msg = Gettaskmsg()))
        {
            if (msg->amsg && msg->amsg->method == AOM_SET)
            {
                ams = (struct Amset *)msg->amsg;
                if (tstate = ams->tags)
                {
                    while (!done && (tag = NextTagItem(&tstate)))
                    {
                        switch (tag->ti_Tag)
                        {
                            case AOTSK_Stop:
                                if (tag->ti_Data)
                                    done = TRUE;
                                break;
                            case AOFCC_Header:
                                Channelheader(fd, (UBYTE *) tag->ti_Data);
                                break;
                            case AOFCC_Data:
                                Updatetaskattrs(AOURL_Data, tag->ti_Data,
                                                AOURL_Datalength,
                                                strlen((UBYTE *) tag->
                                                       ti_Data), TAG_END);
                                break;
                            case AOFCC_Newline:
                                if (tag->ti_Data)
                                {
                                    Updatetaskattrs(AOURL_Data, "\n",
                                                    AOURL_Datalength, 1,
                                                    TAG_END);
                                }
                                break;
                            case AOFCC_Close:
                                if (tag->ti_Data)
                                    done = TRUE;
                                break;
                        }
                    }
                }
            }
            Replytaskmsg(msg);
        }

    }
    Updatetaskattrs(AOTSK_Async, TRUE,
                    AOURL_Eof, TRUE, AOURL_Terminate, TRUE, TAG_END);
}

/*------------------------------------------------------------------------*/

/* Find out if this host is in the no-cache or no-proxy list */
static BOOL Isinlist(void * alist, UBYTE * name)
{
    LIST(Nocache) * list=alist;
    UBYTE          *p, *q;
    struct Nocache *nc;
    BOOL            result = FALSE, scheme;
    short           l, plen;

#ifndef LOCALONLY
    p = strstr(name, "://");
    if (p)
        p += 3;
    else
        p = name;
    if ((*p == '/' || STRNIEQUAL(p, "localhost", 9)) &&
        !prefs.network.cachelocalhost)
        result = TRUE;
    else
    {
        q = strchr(p, '/');
        plen = strlen(p);
        for (nc = list->first; !result && nc->next; nc = nc->next)
        {
            if (nc->pattern)
            {
                scheme = BOOLVAL(strstr(nc->name, "://"));
                result = MatchPatternNoCase(nc->pattern, scheme ? name : p);
            }
            else
            {
                if (strchr(nc->name, '/'))
                {
                    l = strlen(nc->name);
                    result = STRNIEQUAL(nc->name, p, l);
                }
                else
                {               /* no-name is domain only - match full domain */
                    if (q)
                        l = q - p;
                    else
                        l = plen;
                    if (l == strlen(nc->name))
                        result = STRNIEQUAL(nc->name, p, l);
                }
            }
        }
    }
#else
    result = TRUE;
#endif
    return result;
}

/* Start the driver subtask */
static BOOL Dostartdriver(struct Fetch *fch)
{
    if (fch->task = Anewobject(AOTP_TASK,
                               AOTSK_Entry,(Tag) fch->driverfun,
                               AOTSK_Name, (Tag)"AWeb retrieve",
                               AOTSK_Userdata, (Tag)fch->fd,
                               AOBJ_Target, (Tag)fch, TAG_END))
    {
        Asetattrs(fch->task, AOTSK_Start, TRUE, TAG_END);
        if (Agetattr(fch->task, AOTSK_Started))
        {
            fch->flags |= FCHF_RUNNING | FCHF_USESLOT;
            if (fch->netstat)
                Chgnetstat(fch->netstat, NWS_STARTED, 0, 0);
            else
                fch->netstat =
                    Addnetstat(fch,
                               (UBYTE *) Agetattr(fch->url, AOURL_Url),
                               NWS_STARTED,
                               BOOLVAL(fch->flags & FCHF_NETSLOT));
            if (fch->flags & FCHF_LOCALSLOT)
                nrlocal++;
            else if (fch->flags & FCHF_NETSLOT)
                nrnet++;
        }
        else
        {
            Adisposeobject(fch->task);
            fch->task = NULL;
        }
    }
    return BOOLVAL(fch->task);
}

/* Check if any requests in this list matches this Waitrequest.
 * Returns TRUE if no match. */
static BOOL Matchwaitrequest(struct Waitrequest *wr, void * alist)
{
    LIST(Fetch) * list=alist;
    BOOL            ready = TRUE;
    struct Fetch   *fch;

    for (fch = list->first; ready && fch->object.next; fch = fch->object.next)
    {
        if (wr->url)
        {
            if (wr->url == fch->url)
                ready = FALSE;
        }
        else if (!wr->windowkey || wr->windowkey == fch->windowkey)
        {
            if (fch->flags & FCHF_IMAGE)
            {
                if (wr->img || !wr->doc)
                    ready = FALSE;
            }
            else
            {
                if (wr->doc || !wr->img)
                    ready = FALSE;
            }
        }
    }
    return ready;
}

/* Check outstanding wait requests */
static void Checkwaitrequests(void)
{
    struct Waitrequest *wr, *wrnext;
    BOOL            ready;

    for (wr = waitrequests.first; wr->next; wr = wrnext)
    {
        wrnext = wr->next;
        ready = Matchwaitrequest(wr, &running)
            && Matchwaitrequest(wr, &netqueue)
            && Matchwaitrequest(wr, &localqueue)
            && Matchwaitrequest(wr, &channels);
        if (ready)
        {
            REMOVE(wr);
            if (wr->cmd)
                Replyarexxcmd(wr->cmd);
            FREE(wr);
        }
    }
}

/* Put fetch in a queue, non-image fetches at higher priority. */
static void Addtoqueue(void * alist, struct Fetch *fch)
{
    LIST(Fetch) * queue=alist;
    struct Fetch   *fa;

    if (fch->flags & FCHF_IMAGE)
    {
        ADDTAIL(queue, fch);
    }
    else
    {
        for (fa = queue->first; fa->object.next; fa = fa->object.next)
        {
            if (fa->flags & FCHF_IMAGE)
                break;
        }
        INSERT(queue, fch, fa->object.prev);
    }
}

/* Check the queues as a result of this fetch terminating */
static void Checkqueues(struct Fetch *tfch)
{
    struct Fetch   *fch;

    if (tfch && (tfch->flags & FCHF_USESLOT))
    {
        if (tfch->flags & FCHF_NETSLOT)
            nrnet--;
        if (tfch->flags & FCHF_LOCALSLOT)
            nrlocal--;
        tfch->flags &= ~FCHF_USESLOT;
    }
    while (nrnet < prefs.network.maxconnect && (fch = (struct Fetch *)REMHEAD(&netqueue)))
    {
        ADDTAIL(&running, fch);
        fch->flags &= ~FCHF_QUEUED;
        if (!Dostartdriver(fch))
        {
            Asrcupdatetags(fch->url, (struct Aobject *)fch, AOURL_Terminate, TRUE, TAG_END);
            Adisposeobject((struct Aobject *)fch);
        }
    }
    while (nrlocal < prefs.network.maxdiskread &&
           (fch = (struct Fetch *)REMHEAD(&localqueue)))
    {
        ADDTAIL(&running, fch);
        fch->flags &= ~FCHF_QUEUED;
        if (!Dostartdriver(fch))
        {
            Asrcupdatetags(fch->url, (struct Aobject *)fch, AOURL_Terminate, TRUE, TAG_END);
            Adisposeobject((struct Aobject *)fch);
        }
    }
}

/* Fill in the proxy */
static void Setproxy(struct Fetchdriver *fd, UBYTE * proxy)
{
    UBYTE          *p;

    if (STRNIEQUAL(proxy, "https://", 8))
    {
        p = proxy + 8;
        fd->flags |= FDVF_SSL;
    }
    else
        p = proxy + 7;
    fd->proxy = Dupstr(p, -1);
}

static int Hex(char p)
{
    if (p >= '0' && p <= '9')
        return p - '0';
    if (p >= 'A' && p <= 'F')
        return p - 'A' + 10;
    if (p >= 'a' && p <= 'f')
        return p - 'a' + 10;
    return 0;
}

/* Unescape the URL */
static UBYTE   *Unescape(UBYTE * url)
{
    UBYTE          *p, *q;

    for (p = url, q = url; *p; p++, q++)
    {
        if (*p == '%' && p[1] && p[2])
        {
            *q = 16 * Hex(p[1]) + Hex(p[2]);
            p += 2;
        }
        else if (p != q)
        {
            *q = *p;
        }
    }
    *q = '\0';
    return url;
}

/* Find out what driver function to start. In case of external program,
 * do that here and don't fill in a function. */
static void Driverfunction(struct Fetch *fch)
{
    if (fch->fdbase)
    {
        Closeaweblib(fch->fdbase);
        fch->fdbase = NULL;
    }
#ifndef LOCALONLY
    if (fch->flags & FCHF_CACHE)
    {
        fch->driverfun = Localfiletask;
        fch->fd->name = fch->name;
        fch->fd->flags |= FDVF_CACHERELOAD;
        fch->flags |= FCHF_LOCALSLOT;
    }
    else if (fch->flags & FCHF_CHANNEL)
    {
        fch->driverfun = Channeltask;
        fch->fd->name = fch->name;
        fch->channelid = ++channelid;
    }
    else if (STRNIEQUAL(fch->name, "HTTP://", 7))
    {
        if (prefs.network.httpproxy && !(fch->flags & FCHF_NOPROXY))
        {
            fch->driverfun = Httptask;
            fch->fd->name = fch->name;
            Setproxy(fch->fd, prefs.network.httpproxy);
            fch->flags |= FCHF_NETSLOT;
        }
        else
        {
            fch->driverfun = Httptask;
            fch->fd->name = fch->name;
            fch->flags |= FCHF_NETSLOT;
        }
    }
    else if (STRNIEQUAL(fch->name, "HTTPS://", 8))
    {
        if (prefs.network.httpproxy && !(fch->flags & FCHF_NOPROXY))
        {
            fch->driverfun = Httptask;
            fch->fd->name = fch->name;
            Setproxy(fch->fd, prefs.network.httpproxy);
            fch->fd->flags |= FDVF_SSL;
            fch->flags |= FCHF_NETSLOT;
        }
        else
        {
            fch->driverfun = Httptask;
            fch->fd->name = fch->name;
            fch->fd->flags |= FDVF_SSL;
            fch->flags |= FCHF_NETSLOT;
        }
    }
    else if (STRNIEQUAL(fch->name, "FTP://", 6))
    {
        if (prefs.network.ftpproxy && !(fch->flags & FCHF_NOPROXY))
        {
            fch->driverfun = Httptask;
            fch->fd->name = fch->name;
            Setproxy(fch->fd, prefs.network.ftpproxy);
            fch->flags |= FCHF_NETSLOT;
        }
        else if (fch->fdbase = Openaweblib(AWEBLIBPATH FTP_AWEBLIB, FTP_VERSION))
        {
            fch->driverfun = (void *)__AwebGetTaskFunc_WB(fch->fdbase, 0);
            fch->fd->name = Unescape(fch->name + 6);
            fch->flags |= FCHF_NETSLOT;
        }
        else
        {
            fch->driverfun = Errorschemetask;
            fch->fd->name = fch->name;
            fch->fd->validate = MSG_EPART_NOAWEBLIB;
        }
    }
    else if (STRNIEQUAL(fch->name, "GOPHER://", 9))
    {
        if (prefs.network.gopherproxy && !(fch->flags & FCHF_NOPROXY))
        {
            fch->driverfun = Httptask;
            fch->fd->name = fch->name;
            Setproxy(fch->fd, prefs.network.gopherproxy);
            fch->flags |= FCHF_NETSLOT;
        }
        else if (fch->fdbase = Openaweblib(AWEBLIBPATH GOPHER_AWEBLIB, GOPHER_VERSION))
        {
            fch->driverfun = (void *)__AwebGetTaskFunc_WB(fch->fdbase, 0);
            fch->fd->name = Unescape(fch->name + 9);
            fch->flags |= FCHF_NETSLOT;
        }
        else
        {
            fch->driverfun = Errorschemetask;
            fch->fd->name = fch->name;
            fch->fd->validate = MSG_EPART_NOAWEBLIB;
        }
    }
    else if (STRNIEQUAL(fch->name, "TELNET:", 7))
    {
        if (prefs.network.telnetproxy && !(fch->flags & FCHF_NOPROXY))
        {
            fch->driverfun = Httptask;
            fch->fd->name = fch->name;
            Setproxy(fch->fd, prefs.network.telnetproxy);
            fch->flags |= FCHF_NETSLOT;
        }
        else if (prefs.network.telnetcmd)
        {
            UBYTE          *p, *q, *path = Dupstr(fch->name + 7, -1);
            UBYTE          *ln = "", *pw = "(none)", *hn = "", *pn = "23";

            if (path)
            {
                q = path;
                if (q[0] == '/' && q[1] == '/')
                    q += 2;
                if (p = strchr(q, '@'))
                {
                    ln = q;
                    *p = '\0';
                    hn = p + 1;
                    if (p = strchr(ln, ':'))
                    {
                        *p = '\0';
                        pw = p + 1;
                    }
                }
                else
                    hn = q;
                if (p = strchr(hn, '/'))
                    *p = '\0';
                if (p = strchr(hn, ':'))
                {
                    *p = '\0';
                    pn = p + 1;
                }
                Spawn(FALSE, prefs.network.telnetcmd,
                      prefs.network.telnetargs ? prefs.network.
                      telnetargs : NULLSTRING, "lwhpn", ln, pw, hn, pn,
                      Agetattr(Aweb(), AOAPP_Screenname));
                FREE(path);
            }
        }
        else
        {
            fch->driverfun = Errorschemetask;
            fch->fd->name = fch->name;
            fch->fd->validate = MSG_EPART_NOPROGRAM;
        }
    }
    else if (STRNIEQUAL(fch->name, "MAILTO:", 7))
    {
        if (prefs.network.extmailer && prefs.network.mailtocmd)
        {
            Spawn(FALSE, prefs.network.mailtocmd,
                  prefs.network.mailtoargs ? prefs.network.
                  mailtoargs : NULLSTRING, "en", Unescape(fch->name + 7),
                  Agetattr(Aweb(), AOAPP_Screenname));
        }
        else if (fch->fdbase = Openaweblib(AWEBLIBPATH MAIL_AWEBLIB, MAIL_VERSION))
        {
            fch->driverfun = (void *)__AwebGetTaskFunc_WB(fch->fdbase, 0);
            fch->fd->name = fch->name;
            Asetattrs(fch->url,
                      AOURL_Cacheable, FALSE, AOURL_Volatile, TRUE, TAG_END);
        }
        else
        {
            fch->driverfun = Errorschemetask;
            fch->fd->name = fch->name;
            fch->fd->validate = MSG_EPART_NOPROGRAM;
        }
    }
    else if (STRNIEQUAL(fch->name, "NEWS:", 5) ||
             STRNIEQUAL(fch->name, "NNTP://", 7))
    {
        if (prefs.network.extnewsreader && prefs.network.newscmd)
        {
            Spawn(FALSE, prefs.network.newscmd,
                  prefs.network.newsargs ? prefs.network.
                  newsargs : NULLSTRING, "an", Unescape(fch->name + 5),
                  Agetattr(Aweb(), AOAPP_Screenname));
        }
        else if (fch->fdbase = Openaweblib(AWEBLIBPATH NEWS_AWEBLIB, NEWS_VERSION))
        {
            fch->driverfun = (void *)__AwebGetTaskFunc_WB(fch->fdbase, 0);
            fch->fd->name = Unescape(fch->name);
            Asetattrs(fch->url,
                      AOURL_Cacheable, FALSE, AOURL_Volatile, TRUE, TAG_END);
        }
        else
        {
            fch->driverfun = Errorschemetask;
            fch->fd->name = fch->name;
            fch->fd->validate = MSG_EPART_NOPROGRAM;
        }
    }
    else
#endif /* !LOCALONLY */
    if (STRNIEQUAL(fch->name, "FILE://localhost/", 17)
            || STRNIEQUAL(fch->name, "FILE:///", 8))
    {
        fch->driverfun = Localfiletask;
        fch->fd->name = strchr(Unescape(fch->name + 7), '/') + 1;
        fch->flags |= FCHF_LOCALSLOT;
        if (!prefs.network.cachelocalhost)
        {
            Asetattrs(fch->url, AOURL_Cacheable, FALSE, TAG_END);
        }
    }
    else if (STRNIEQUAL(fch->name, "JAVASCRIPT:", 11))
    {
        struct Jcontext *jc;
        struct Jvar    *jv;
        UBYTE          *result;

        if (prefs.browser.dojs && Openjslib())
        {
            Runjavascript(fch->jframe, Unescape(fch->name + 11), NULL);
            jc = (struct Jcontext *)Agetattr(Aweb(), AOAPP_Jcontext);
            if (jv = Jgetreturnvalue(jc))
            {
                result = Jtostring(jc, jv);
                if (fch->postmsg)
                    FREE(fch->postmsg);
                fch->postmsg = Dupstr(result, -1);
                fch->fd->postmsg = fch->postmsg;
                fch->fd->validate = Today();
                fch->driverfun = Javascripttask;
                Asetattrs(fch->url, AOURL_Cacheable, FALSE, TAG_END);
            }
            else
            {                   /* No defined result from script, do nothing */
            }
        }
    }
    else if (STRNIEQUAL(fch->name, "X-AWEB:MAIL/", 12))
    {
        if (fch->fdbase = Openaweblib(AWEBLIBPATH MAIL_AWEBLIB, MAIL_VERSION))
        {
            fch->driverfun = (void *)__AwebGetTaskFunc_WB(fch->fdbase, 0);
            fch->fd->name = fch->name;
            Asetattrs(fch->url,
                      AOURL_Cacheable, FALSE, AOURL_Volatile, TRUE, TAG_END);
        }
    }
    else if (STRNIEQUAL(fch->name, "X-AWEB:NEWS", 11))
    {
        if (fch->fdbase = Openaweblib(AWEBLIBPATH NEWS_AWEBLIB, NEWS_VERSION))
        {
            fch->driverfun = (void *)__AwebGetTaskFunc_WB(fch->fdbase, 0);
            fch->fd->name = fch->name;
            Asetattrs(fch->url,
                      AOURL_Cacheable, FALSE, AOURL_Volatile, TRUE, TAG_END);
        }
    }
    else if (STRNIEQUAL(fch->name, "X-AWEB:", 7))
    {
        fch->driverfun = Xawebtask;
        fch->fd->name = fch->name + 7;
        Asetattrs(fch->url, AOURL_Cacheable, FALSE, TAG_END);
    }
    else if (STRNIEQUAL(fch->name, "X-JSGENERATED:", 14))
    {
        if (fch->postmsg)
            FREE(fch->postmsg);
        if (!Getjsgeneratedtext(fch->name, &fch->postmsg))
        {
            fch->fd->flags |= FDVF_JSOPEN;
        }
        if (fch->postmsg)
        {
            fch->fd->postmsg = fch->postmsg;
            fch->driverfun = Jsgeneratedtask;
            Asetattrs(fch->url, AOURL_Cacheable, FALSE, TAG_END);
        }
    }
    else if (STRNIEQUAL(fch->name, "X-NIL:", 6))
    {                           /* Do nothing */
    }
    else
#ifdef LOCALONLY
    {                           /* Do nothing */
    }
#else
    {
        fch->driverfun = Errorschemetask;
        fch->fd->name = fch->name;
        fch->fd->validate = MSG_EPART_ADDRSCHEME;
    }
#endif
}

/*------------------------------------------------------------------------*/

/* Dispose a Fetchdriver */
static void Disposefd(struct Fetchdriver *fd)
{
    if (fd)
    {                           /* referer and postmsg are shared with Fetch - don't free these here. */
        if (fd->proxy)
            FREE(fd->proxy);
        if (fd->block)
            FREE(fd->block);
        FREE(fd);
    }
}

/* Create the fetch driver process */
static BOOL Startdriver(struct Fetch *fch)
{
    BOOL            result = FALSE;

    fch->driverfun = NULL;
    if (!(fch->fd = ALLOCSTRUCT(Fetchdriver, 1, MEMF_CLEAR)))
        return FALSE;
    fch->fd->fetch = fch;
    if (!(fch->fd->block = ALLOCTYPE(UBYTE, INPUTBLOCKSIZE, 0)))
        return FALSE;
    fch->fd->blocksize = INPUTBLOCKSIZE;
    fch->fd->validate = fch->validate;
    if (fch->flags & FCHF_NOCACHE)
    {
        fch->fd->flags |= FDVF_NOCACHE;
    }
    if (fch->flags & FCHF_COMMANDS)
    {
        fch->fd->flags |= FDVF_COMMANDS;
    }
    if (fch->flags & FCHF_FORMWARN)
    {
        fch->fd->flags |= FDVF_FORMWARN;
    }
    fch->fd->postmsg = fch->postmsg;
    fch->fd->multipart = fch->mpd;
    fch->total = 0;
    fch->sofar = 0;
    fch->fd->prefs = &prefs;
    fch->fd->prefssema = &prefssema;

/*
   Include referer only if it is not localhost, and if not browsing anonymously.
   * But: if target is x-aweb: or mailto:, always include referer.
 */
    fch->fd->etag = fch->etag;
    fch->fd->referer = fch->referer;
    ObtainSemaphore(&prefssema);
    if (fch->fd->referer
        && !STRNIEQUAL(fch->name, "X-AWEB:", 7)
        && !STRNIEQUAL(fch->name, "MAILTO:", 7))
    {
        if (STRNIEQUAL(fch->fd->referer, "FILE://", 7)
            || STRNIEQUAL(fch->fd->referer, "X-AWEB:", 7)
            || STRNIEQUAL(fch->fd->referer, "JAVASCRIPT:",11)
           )
            fch->fd->referer = NULL;
        if (!prefs.network.referer)
            fch->fd->referer = NULL;
    }

/*
   Don't proxy if limited proxy usage and it's a POST, or
   * if it is in the noproxy list.
 */
    if (prefs.network.limitproxy)
    {
        if (fch->postmsg || fch->mpd)
        {
            fch->flags |= FCHF_NOPROXY;
        }
    }
    if (Isinlist(&prefs.network.noproxy, fch->name))
    {
        fch->flags |= FCHF_NOPROXY;
    }
    if (Isinlist(&prefs.network.nocache, fch->name))
    {
        Asetattrs(fch->url, AOURL_Cacheable, FALSE, TAG_END);
    }
    Driverfunction(fch);
    ReleaseSemaphore(&prefssema);
    if (fch->driverfun)
    {
        if ((fch->flags & FCHF_LOCALSLOT) &&
            nrlocal >= prefs.network.maxdiskread)
        {
            REMOVE(fch);
            Addtoqueue(&localqueue, fch);
            fch->flags |= FCHF_QUEUED;
            if (fch->netstat)
                Chgnetstat(fch->netstat, NWS_QUEUED, 0, 0);
            else
                fch->netstat =
                    Addnetstat(fch,
                               (UBYTE *) Agetattr(fch->url, AOURL_Url),
                               NWS_QUEUED, FALSE);
            result = TRUE;
        }
        else if ((fch->flags & FCHF_NETSLOT) &&
                 nrnet >= prefs.network.maxconnect)
        {
            REMOVE(fch);
            Addtoqueue(&netqueue, fch);
            fch->flags |= FCHF_QUEUED;
            if (fch->netstat)
                Chgnetstat(fch->netstat, NWS_QUEUED, 0, 0);
            else
                fch->netstat =
                    Addnetstat(fch,
                               (UBYTE *) Agetattr(fch->url, AOURL_Url),
                               NWS_QUEUED, TRUE);
            result = TRUE;
        }
        else
        {
            if (fch->flags & FCHF_CHANNEL)
            {
                REMOVE(fch);
                ADDTAIL(&channels, fch);
            }

            result = Dostartdriver(fch);

        }
    }
    return result;
}

/*------------------------------------------------------------------------*/

/* Return from 'incomplete' requester */
static void Incompletereturn(short code, struct Fetch *fch)
{
    switch (code)
    {
        case 0:                /* cancel */
            Asrcupdatetags(fch->url, (struct Aobject *)fch,
                           AOURL_Error, TRUE,
                           AOURL_Eof, TRUE,
                           AOURL_Terminate, TRUE,
                           TAG_END);
            fch->flags |= FCHF_DISPOSED;
            break;
        case 1:                /* ok */
            Asrcupdatetags(fch->url, (struct Aobject *)fch,
                           AOURL_Eof, TRUE,
                           AOURL_Terminate, TRUE,
                           TAG_END);
            fch->flags |= FCHF_DISPOSED;
            break;
        case 2:                /* retry */
            if (Startdriver(fch))
            {
                Asrcupdatetags(fch->url, (struct Aobject *)fch,
                               AOURL_Reload, TRUE,
                               TAG_END);
            }
            else
            {
                Asrcupdatetags(fch->url, (struct Aobject *)fch,
                               AOURL_Eof, TRUE,
                               AOURL_Terminate, TRUE,
                               TAG_END);
                fch->flags |= FCHF_DISPOSED;
            }
            break;
    }
    fch->flags &= ~FCHF_INCOMPLETE;
    if (fch->flags & FCHF_DISPOSED)
    {
        Adisposeobject((struct Aobject *)fch);
    }
}

/* Open 'incomplete' requester */
static BOOL Incompleterequest(struct Fetch *fch)
{
    UBYTE          *p = AWEBSTR(MSG_FILE_INCTEXT);
    UBYTE          *buf, *url;
    long            len;
    BOOL            ok = FALSE;

    url = (UBYTE *) Agetattr(fch->url, AOURL_Url);
    len = strlen(p) + strlen(url) + 80;
    if (buf = ALLOCTYPE(UBYTE, len, 0))
    {
        Lprintf(buf, p, url, fch->total, fch->sofar);
        if (haiku)
            strcpy(buf, HAIKU22);
        ok = Asyncrequestcc(AWEBSTR(MSG_REQUEST_TITLE), buf,
                            AWEBSTR(MSG_FILE_INCBUTTONS2),
                            (void *)Incompletereturn, fch, url);
        if (ok)
        {
            fch->flags &= ~FCHF_RUNNING;
            fch->flags |= FCHF_INCOMPLETE;
        }
        else
        {
            FREE(buf);
        }
    }
    return ok;
}

/*------------------------------------------------------------------------*/

/* Process output from driver */
static long Updatefetch(struct Fetch *fch, struct Amset *ams)
{
    struct TagItem *tag, *tstate = ams->tags;
    BOOL            forward = TRUE, dispose = FALSE, retryget =
        FALSE, statusset = FALSE;
    ULONG           netstat = 0;
    UBYTE           buf[40];
    ULONG           statustag = TAG_IGNORE;

    if (tstate)
    {
        while (tag = NextTagItem(&tstate))
        {
            switch (tag->ti_Tag)
            {
                case AOURL_Contentlength:
                    fch->total = tag->ti_Data;
                    break;
                case AOURL_Datalength:
                    fch->sofar += tag->ti_Data;
                    if (!netstat)
                        netstat = NWS_READ;
                    break;
                case AOURL_Data:
                /*
                   Stop driver if not modified
                 */
                    if (fch->flags & FCHF_NOTMODIFIED)
                    {
                        Asetattrs(fch->task,
                                  AOTSK_Stop, TRUE,
                                  AOTSK_Async, TRUE, TAG_END);
                        forward = FALSE;
                    }
                    break;
                case AOURL_Terminate:
                /*
                   Forward and dispose only if no error after redirected POST
                 */
                    if (fch->flags & FCHF_POSTNOGOOD)
                    {
                        forward = FALSE;
                        retryget = TRUE;
                    }
                    else if (!
                             (fch->
                              flags & (FCHF_CANCELLED | FCHF_NOTMODIFIED |
                                       FCHF_DISPOSED)) &&
                             fch->sofar < fch->total)
                    {           /* Don't forward, open 'incomplete' requester. */
                        if (Incompleterequest(fch))
                        {
                            forward = FALSE;
                        }
                        else
                        {
                            dispose = TRUE;
                        }
                    }
                    else
                    {
                        dispose = TRUE;
                    }
                    Adisposeobject(fch->task);
                    fch->task = NULL;
                    Checkqueues(fch);
                    break;
                case AOURL_Status:
                    strncpy(fch->statusbuf, (UBYTE *) tag->ti_Data,
                            STATUSBUFSIZE - 1);
                    tag->ti_Data = (ULONG) fch->statusbuf;
                    statusset = TRUE;
                    break;
                case AOURL_Netstatus:
                    netstat = tag->ti_Data;
                    forward = FALSE;
                    break;
                case AOURL_Notmodified:
                    if (tag->ti_Data)
                        fch->flags |= FCHF_NOTMODIFIED;
                    else
                        fch->flags &= ~FCHF_NOTMODIFIED;
                    forward = FALSE;
                    break;
                case AOURL_Serverdate:
                    fch->serverdate = tag->ti_Data;
                    forward = FALSE;
                    break;
                case AOURL_Expires:
                /*
                   translate to local time:
                 */
                    if (fch->serverdate)
                    {
                        tag->ti_Data =
                            tag->ti_Data - fch->serverdate + fch->responsetime; // Today();
                    }
                    break;
                case AOURL_Arexxcommand:
                    Sendarexxcmd(fch->windowkey, (UBYTE *) tag->ti_Data);
                    break;
                case AOURL_Postnogood:
                    if (tag->ti_Data)
                    {
                        Asetattrs(fch->task,
                                  AOTSK_Stop, TRUE,
                                  AOTSK_Async, TRUE, TAG_END);
                        fch->flags |= FCHF_POSTNOGOOD;
                        forward = FALSE;
                    }
                    else
                        fch->flags &= ~FCHF_POSTNOGOOD;
                    break;
                case AOURL_Reload:
                    fch->total = 0;
                    fch->sofar = 0;
                    break;
                case AOURL_Movedto:
                case AOURL_Tempmovedto:
                case AOURL_Seeother:
                    fch->total = 0;     /* Prevent "incomplete file" requester */
                    break;
                case AOURL_Gohistory:
                    Gohistory(Findwindow(fch->windowkey), (long)tag->ti_Data);
                    break;
                case AOFCH_Responsetime:
                    fch->responsetime = tag->ti_Data;
                    break;
            }
        }
    }

/*
   If we are already disposed, or we should retry, don't forward.
 */
    if (fch->flags & (FCHF_DISPOSED | FCHF_POSTNOGOOD))
    {
        forward = FALSE;
    }

/*
   Forward this message. Build status text
 */
    if (forward && fch->sofar && !statusset)
    {
        strcpy(buf, AWEBSTR(MSG_AWEB_BYTESREAD));
        if (fch->total)
        {
            strcat(buf, ": %d/%d");
            sprintf(fch->statusbuf, buf, fch->sofar, fch->total);
        }
        else
        {
            strcat(buf, ": %d");
            sprintf(fch->statusbuf, buf, fch->sofar);
        }
        statustag = AOURL_Status;
    }
    if (forward)
    {
        Asrcupdatetags(fch->url, (struct Aobject *)fch,
                       statustag, (Tag)fch->statusbuf,
                       AOURL_Contentlength, fch->total,
                       AOURL_Datatotal, fch->sofar,
                       ams->tags ? TAG_MORE : TAG_END, (Tag)ams->tags);
    }
    if (netstat && fch->netstat)
    {
        Chgnetstat(fch->netstat, netstat, fch->sofar, fch->total);
    }

/*
   Try again with no post message
 */
    if (retryget)
    {
        fch->flags &= ~(FCHF_RUNNING | FCHF_POSTNOGOOD);
        Disposefd(fch->fd);
        fch->fd = NULL;
        if (fch->postmsg)
        {
            FREE(fch->postmsg);
            fch->postmsg = NULL;
        }
        if (fch->mpd)
        {
            Freemultipartdata(fch->mpd);
            fch->mpd = NULL;
        }
        if (!Startdriver(fch))
        {
            Asrcupdatetags(fch->url, (struct Aobject *)fch, AOURL_Terminate, TRUE, TAG_END);
            Checkqueues(fch);
            Adisposeobject((struct Aobject *)fch);
        }
    }

/*
   Dispose ourselves if our time has come
 */
    if (dispose)
    {
        fch->flags &= ~FCHF_RUNNING;
        Adisposeobject((struct Aobject *)fch);
    }
    return 0;
}

static long Setfetch(struct Fetch *fch, struct Amset *ams)
{
    struct TagItem *tag, *tstate = ams->tags;
    BOOL            cancel = FALSE, cancelerr = FALSE;

    while (tag = NextTagItem(&tstate))
    {
        switch (tag->ti_Tag)
        {
            case AOFCH_Url:
                fch->url = (void *)tag->ti_Data;
                break;
            case AOFCH_Name:
                fch->name = Dupstr((UBYTE *) tag->ti_Data, -1);
                break;
            case AOFCH_Ifmodifiedsince:
                fch->validate = tag->ti_Data;
                break;
            case AOFCH_Nocache:
                if (tag->ti_Data)
                    fch->flags |= FCHF_NOCACHE;
                else
                    fch->flags &= ~FCHF_NOCACHE;
                break;
            case AOFCH_Cache:
                if (tag->ti_Data)
                    fch->flags |= FCHF_CACHE;
                else
                    fch->flags &= ~FCHF_CACHE;
                break;
            case AOFCH_Cancel:
                if (tag->ti_Data)
                    cancel = cancelerr = TRUE;
                break;
            case AOFCH_Cancellocal:
                if (tag->ti_Data && (fch->flags & FCHF_LOCALSLOT))
                    cancel = TRUE;
                break;
            case AOFCH_Postmsg:
                if (fch->postmsg)
                    FREE(fch->postmsg);
                fch->postmsg = Dupstr((UBYTE *) tag->ti_Data, -1);
                break;
            case AOFCH_Multipartdata:
                if (fch->mpd)
                    Freemultipartdata(fch->mpd);
                fch->mpd = (struct Multipartdata *)tag->ti_Data;
                break;
            case AOFCH_Referer:
                if (fch->referer)
                {
                    FREE(fch->referer);
                    fch->referer = NULL;
                }
                if(tag->ti_Data)
                {
                    fch->referer = Dupstr((UBYTE *) tag->ti_Data, -1);
                }
                break;
            case AOFCH_Etag:
                if (fch->etag)
                {
                    FREE(fch->etag);
                    fch->etag = NULL;
                }
                if(tag->ti_Data)
                {
                    fch->etag = Dupstr((UBYTE *) tag->ti_Data, -1);
                }
                break;

            case AOFCH_Windowkey:
                fch->windowkey = tag->ti_Data;
                break;
            case AOFCH_Noproxy:
                if (tag->ti_Data)
                    fch->flags |= FCHF_NOPROXY;
                else
                    fch->flags &= ~FCHF_NOPROXY;
                break;
            case AOFCH_Loadflags:
                fch->loadflags = tag->ti_Data;
                break;
            case AOFCH_Imagefetch:
                SETFLAG(fch->flags, FCHF_IMAGE, tag->ti_Data);
                break;
            case AOFCH_Commands:
                SETFLAG(fch->flags, FCHF_COMMANDS, tag->ti_Data);
                break;
            case AOFCH_Jframe:
                fch->jframe = (void *)tag->ti_Data;
                break;
            case AOFCH_Formwarn:
                SETFLAG(fch->flags, FCHF_FORMWARN, tag->ti_Data);
                break;
            case AOFCH_Channel:
                SETFLAG(fch->flags, FCHF_CHANNEL, tag->ti_Data);
                break;
        }
    }
    if (cancel)
    {
        fch->flags |= FCHF_CANCELLED;
        if (fch->flags & FCHF_RUNNING)
        {
            if (cancelerr)
            {
                Asrcupdatetags(fch->url, (struct Aobject *)fch, AOURL_Error, TRUE, TAG_END);
            }
            Asetattrs(fch->task,
                      AOTSK_Stop, TRUE, AOTSK_Async, TRUE, TAG_END);
        /*
           Will be disposed when task returns
         */
        }
        else if (fch->flags & FCHF_QUEUED)
        {
            Asrcupdatetags(fch->url, (struct Aobject *)fch, AOURL_Terminate, TRUE, TAG_END);
            Adisposeobject((struct Aobject *)fch);
        }
    }
    return 0;
}

static void Disposefetch(struct Fetch *fch)
{
    if (fch->flags & FCHF_RUNNING)
    {
        Asetattrs(fch->task, AOTSK_Stop, TRUE, AOTSK_Async, TRUE, TAG_END);
        fch->flags |= FCHF_DISPOSED;
    }
    else if (fch->flags & FCHF_INCOMPLETE)
    {
        fch->flags |= FCHF_DISPOSED;
    }
    else
    {
        REMOVE(fch);
        if (fch->netstat)
            Chgnetstat(fch->netstat, NWS_END, 0, 0);
        if (fch->fd)
            Disposefd(fch->fd);
        if (fch->fdbase)
            Closeaweblib(fch->fdbase);
        if (fch->name)
            FREE(fch->name);
        if (fch->postmsg)
            FREE(fch->postmsg);
        if (fch->mpd)
            Freemultipartdata(fch->mpd);
        if (fch->referer)
            FREE(fch->referer);
        Amethodas(AOTP_OBJECT, fch, AOM_DISPOSE);
        Checkwaitrequests();
    }
}

static struct Fetch *Newfetch(struct Amset *ams)
{
    struct Fetch   *fch;
    if (fch = Allocobject(AOTP_FETCH, sizeof(struct Fetch), ams))
    {
        ADDTAIL(&running, fch);
        Setfetch(fch, ams);
        if (!fch->name || !Startdriver(fch))
        {
            Disposefetch(fch);
            fch = NULL;
        }
    }
    return fch;
}

static long Getfetch(struct Fetch *fch, struct Amset *ams)
{
    struct TagItem *tag, *tstate = ams->tags;

    while (tag = NextTagItem(&tstate))
    {
        switch (tag->ti_Tag)
        {
            case AOFCH_Url:
                PUTATTR(tag, fch->url);
                break;
            case AOFCH_Postmsg:
                PUTATTR(tag, fch->postmsg);
                break;
            case AOFCH_Multipartdata:
                PUTATTR(tag, fch->mpd);
                break;
            case AOFCH_Referer:
                PUTATTR(tag, fch->referer);
                break;
            case AOFCH_Windowkey:
                PUTATTR(tag, fch->windowkey);
                break;
            case AOFCH_Loadflags:
                PUTATTR(tag, fch->loadflags);
                break;
            case AOFCH_Channelid:
                PUTATTR(tag, fch->channelid);
                break;
        }
    }
    return 0;
}

USRFUNC_H2
(
    static long, Fetch_Dispatcher,
    struct Fetch *, fch, A0,
    struct Amessage *, amsg, A1
)
{

    USRFUNC_INIT

    long            result = 0;

    switch (amsg->method)
    {
        case AOM_NEW:
            result = (long)Newfetch((struct Amset *)amsg);
            break;
        case AOM_SET:
            result = Setfetch(fch, (struct Amset *)amsg);
            break;
        case AOM_UPDATE:
            result = Updatefetch(fch, (struct Amset *)amsg);
            break;
        case AOM_GET:
            result = Getfetch(fch, (struct Amset *)amsg);
            break;
        case AOM_DISPOSE:
            Disposefetch(fch);
            break;
        case AOM_DEINSTALL:
            break;
    }
    return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installfetch(void)
{
    NEWLIST(&netqueue);
    NEWLIST(&localqueue);
    NEWLIST(&running);
    NEWLIST(&channels);
    NEWLIST(&waitrequests);
    if (!Amethod(NULL, AOM_INSTALL, AOTP_FETCH, (Tag)Fetch_Dispatcher))
        return FALSE;
    return TRUE;
}

BOOL Transferring(void)
{
    return (BOOL) (!ISEMPTY(&running) || !ISEMPTY(&netqueue) ||
                   !ISEMPTY(&localqueue) || !ISEMPTY(&channels));
}

void Addwaitrequest(struct Arexxcmd *ac, ULONG windowkey, BOOL doc, BOOL img,
                    void *url)
{
    struct Waitrequest *wr;

    if (wr = ALLOCSTRUCT(Waitrequest, 1, 0))
    {
        wr->cmd = ac;
        wr->windowkey = windowkey;
        wr->doc = doc;
        wr->img = img;
        wr->url = url;
        ADDTAIL(&waitrequests, wr);
        Checkwaitrequests();
    }
    else
    {
        Replyarexxcmd(ac);
    }
}

static void Channelreplied(void *task, struct Taskmsg *msg)
{
    struct Amset   *ams;
    struct TagItem *tstate, *tag;

    if (msg && msg->amsg && msg->amsg->method == AOM_SET)
    {
        ams = (struct Amset *)msg->amsg;
        if (ams->tags)
        {
            if (tstate = ams->tags)
            {
                while (tag = NextTagItem(&tstate))
                {
                    switch (tag->ti_Tag)
                    {
                        case AOFCC_Arexxcmd:
                            if (tag->ti_Data)
                                Replyarexxcmd((struct Arexxcmd *)tag->
                                              ti_Data);
                            break;
                        case AOFCC_Header:
                        case AOFCC_Data:
                            FREE((void *)tag->ti_Data);
                            break;
                    }
                }
            }
        }
    }
}

VARARGS68K_DECLARE(BOOL Channelfetch(struct Arexxcmd *ac, long id, ...))
{
    BOOL            ok = FALSE;
    struct Fetch   *fch;
    struct TagItem * more;
    VA_LIST ap;

    VA_STARTLIN(ap,id);
    more = (struct TagItem *)VA_GETLIN(ap,struct TagItem*);

    for (fch = channels.first; fch->object.next; fch = fch->object.next)
    {
        if (fch->channelid == id)
            break;
    }
    if (fch && fch->object.next && (fch->flags & FCHF_RUNNING) &&
        !(fch->flags & FCHF_DISPOSED))
    {
        ok = TRUE;
        Asetattrsasync(fch->task,
                       AOTSK_Replied, Channelreplied,
                       AOFCC_Arexxcmd, ac, TAG_MORE, more);
    }
    return ok;
}
