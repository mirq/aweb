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

/* winrexx.c - AWeb window ARexx command handler */

#include "aweb.h"
#include "frame.h"
#include "winhis.h"
#include "url.h"
#include "window.h"
#include "winprivate.h"
#include "application.h"
#include "source.h"
#include "copy.h"
#include "fetch.h"
#include "cache.h"
#include "copydriver.h"
#include "filereq.h"
#include "plugin.h"
#include "jslib.h"
#include "versions.h"

#include "libraries/awebarexx.h"

#include <reaction/reaction.h>
#include <proto/intuition.h>
#include "proto/awebarexx.h"

static BOOL Doopen(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *urlname,UBYTE *targetname,BOOL reload,UBYTE *post,BOOL smart);

struct Reqdata
{
    UBYTE           *title;
    struct Arexxcmd *ac;
    UBYTE           *string;
};

/*-----------------------------------------------------------------------*/

/* Scroll this frame */
static void Scrollframe(void *frame,long n,ULONG toptag,ULONG heighttag,BOOL page,BOOL sfar)
{  long top,h;
   if(sfar)
   {  if(n<0) top=0;
      else top=AMRMAX;
   }
   else
   {  Agetattrs(frame,
         toptag,(Tag)&top,
         heighttag,(Tag)&h,
         TAG_END);
      if(page)
      {  if(h>prefs.program.overlap) top+=n*(h-prefs.program.overlap);
         else top+=n*h;
      }
      else
      {  top+=n;
      }
   }
   Asetattrs(frame,
      toptag,top,
      AOFRM_Setscroller,TRUE,
      TAG_END);
}

/* Stub functions */
static void Doclearcache(short code,void *data)
{  if(code) Flushcache(CACFT_ALL);
}
static void Dodeleteimages(short code,void *data)
{  if(code) Flushcache(CACFT_IMAGES);
}
static void Dodeletedocs(short code,void *data)
{  if(code) Flushcache(CACFT_DOCUMENTS);
}

static void Disposereqdata(short code,struct Reqdata *rd)
{  TEXT buf[16];
   if(rd)
   {  if(rd->title) FREE(rd->title);
      if(rd->ac)
      {  if(rd->string)
         {  rd->ac->result=Dupstr(rd->string,-1);
         }
         else
         {  sprintf(buf,"%d",code);
            rd->ac->result=Dupstr(buf,-1);
         }
         if(!code) rd->ac->errorlevel=RXERR_WARNING;
         Replyarexxcmd(rd->ac);
      }
      FREE(rd->string);
      FREE(rd);
   }
}

/*-----------------------------------------------------------------------*/

/*
static BOOL Do(struct Arexxcmd *ac,struct Awindow *win)
{
   return TRUE;
}

*/

static BOOL Doactivatewindow(struct Arexxcmd *ac,struct Awindow *win)
{  if(win && win->window) ActivateWindow(win->window);
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Doaddhotlist(struct Arexxcmd *ac,struct Awindow *win,UBYTE *urlname,
   UBYTE *title,UBYTE *targetname,UBYTE *group)
{  void *url=NULL,*target;
   if(urlname)
   {  url=Findurl(NULL,urlname,0);
   }
   else if(win)
   {  title=NULL;
      if(target=Targetframe(win->frame,targetname))
      {  Agetattrs(target,
            AOFRM_Url,(Tag)&url,
            AOFRM_Title,(Tag)&title,
            TAG_END);
      }
   }
   if(url) Addtohotlist(url,title,group);
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Doallowcmd(struct Arexxcmd *ac,struct Awindow *win)
{  if(win)
   {  Asetattrs(win->frame,AOFRM_Commands,TRUE,TAG_END);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dobackground(struct Arexxcmd *ac,struct Awindow *win,
   BOOL on,BOOL off)
{  if(!on && !off)
   {  on=!prefs.browser.docolors;
   }
   Prefsdocolors(on);
   return TRUE;
}

static BOOL Dobgsound(struct Arexxcmd *ac,struct Awindow *win,
   BOOL on,BOOL off)
{  if(!on && !off)
   {  on=!prefs.browser.dobgsound;
   }
   Prefsdobgsound(on);
   return TRUE;
}

static BOOL Docancel(struct Arexxcmd *ac,struct Awindow *win,long *ploadid,BOOL all)
{  if(all)
   {  Cancelnetstatall();
   }
   else if(ploadid)
   {  void *url=Findurlloadnr(*ploadid);
      if(url)
      {  Auspecial(url,AUMST_CANCELFETCH);
      }
   }
   else if(win)
   {  Auspecial(win->activeurl,AUMST_CANCELFETCH);
   }
   return TRUE;
}

static BOOL Dochanclose(struct Arexxcmd *ac,struct Awindow *win,  STRPTR ch)
{  BOOL done;
   long id=atoi(ch);
   if(Channelfetch(ac,id,AOFCC_Close,TRUE,TAG_END))
   {  done=FALSE;
   }
   else
   {  done=TRUE;
      ac->errorlevel=RXERR_WARNING;
   }
   return done;
}

static BOOL Dochanheader(struct Arexxcmd *ac,struct Awindow *win, STRPTR ch,  STRPTR header)
{  BOOL done=FALSE;
   long id=atoi(ch);
   UBYTE *headerdup;
   if(headerdup=Dupstr(header,-1))
   {  if(Channelfetch(ac,id,AOFCC_Header,headerdup,TAG_END))
      {  done=FALSE;
      }
      else
      {  done=TRUE;
         ac->errorlevel=RXERR_WARNING;
         FREE(headerdup);
      }
   }
   return done;
}

static BOOL Dochandata(struct Arexxcmd *ac,struct Awindow *win, STRPTR ch,
   UBYTE *data,BOOL newline)
{  BOOL done=FALSE;
   long id=atoi(ch);
   UBYTE *datadup;
   if(datadup=Dupstr(data,-1))
   {  if(Channelfetch(ac,id,AOFCC_Data,datadup,AOFCC_Newline,newline,TAG_END))
      {  done=FALSE;
      }
      else
      {  done=TRUE;
         ac->errorlevel=RXERR_WARNING;
         FREE(datadup);
      }
   }
   return done;
}

static BOOL Dochanopen(struct Arexxcmd *ac,struct Awindow *win, STRPTR urlname)
{  void *url;
   long id=0;
   TEXT  buf[8];
   if(url=Findurl("",urlname,0))
   {  id=Auload(url,AUMLF_CHANNEL,NULL,NULL,NULL);
      if(id)
      {  sprintf(buf,"%ld",id);
         ac->result=Dupstr(buf,-1);
      }
      else ac->errorlevel=RXERR_WARNING;
   }
   return TRUE;
}

static BOOL Doclose(struct Arexxcmd *ac,struct Awindow *win,BOOL force)
{  if(win)
   {  win->cmd|=CMD_CLOSE;
      if(force) win->cmd|=CMD_CLOSEFORCE;
      Signal(FindTask(NULL),
         1<<((struct MsgPort *)Agetattr(Aweb(),AOAPP_Windowport))->mp_SigBit);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Doclearselection(struct Arexxcmd *ac,struct Awindow *win)
{  if(win)
   {  Dragclear(win);
   }
   return TRUE;
}

static BOOL Docopyblock(struct Arexxcmd *ac,struct Awindow *win)
{  struct Buffer buf={0};
   if(win && win->dragstartobject)
   {  Adragcopy(win->frame,win->dragstartobject,win->dragstartobjpos,
         win->dragendobject,win->dragendobjpos,AMDS_BEFORE,&buf);
      Clipcopy(buf.buffer,buf.length);
      Dragclear(win);
   }
   Freebuffer(&buf);
   return TRUE;
}

static BOOL Docopyurl(struct Arexxcmd *ac,struct Awindow *win,UBYTE *targetname)
{  void *target;
   STRPTR url;
   if(win && (target=Targetframe(win->frame,targetname)))
   {  url=(STRPTR)Agetattr((void *)Agetattr(target,AOFRM_Url),AOURL_Url);
      if(url)
      {  Clipcopy(url,strlen(url));
      }
   }
   return TRUE;
}

static BOOL Dodeletecache(struct Arexxcmd *ac,struct Awindow *win,
   BOOL images,BOOL docs,BOOL force, STRPTR pattern)
{  short type;
   if(pattern)
   {  if(images && !docs) type=CACFT_IMAGES;
      else if(docs && !images) type=CACFT_DOCUMENTS;
      else type=CACFT_ALL;
      Flushcachepattern(type,pattern);
   }
   else if(images || docs)
   {  if(force)
      {  if(images && docs) type=CACFT_ALL;
         else if(images) type=CACFT_IMAGES;
         else type=CACFT_DOCUMENTS;
         Flushcache(type);
      }
      else
      {  if(images && docs)
         {  Asyncrequest(AWEBSTR(MSG_REQUEST_TITLE),
               Dupstr(haiku?HAIKU19:AWEBSTR(MSG_FIXCACHE_ERASE),-1),
               AWEBSTR(MSG_FIXCACHE_BUTTONS),Doclearcache,NULL);
         }
         else if(images)
         {  Asyncrequest(AWEBSTR(MSG_REQUEST_TITLE),
               Dupstr(haiku?HAIKU17:AWEBSTR(MSG_FIXCACHE_DELETEIMAGES),-1),
               AWEBSTR(MSG_FIXCACHE_BUTTONS),Dodeleteimages,NULL);
         }
         else
         {  Asyncrequest(AWEBSTR(MSG_REQUEST_TITLE),
               Dupstr(haiku?HAIKU18:AWEBSTR(MSG_FIXCACHE_DELETEDOCS),-1),
               AWEBSTR(MSG_FIXCACHE_BUTTONS),Dodeletedocs,NULL);
         }
      }
   }
   return TRUE;
}

static BOOL Dodragging(struct Arexxcmd *ac,struct Awindow *win,
   BOOL on,BOOL off)
{  UWORD minum;
   struct MenuItem *mi;
   if(win)
   {  if(!on && !off) on=!BOOLVAL(win->flags&WINF_CLIPDRAG);
      SETFLAG(win->flags,WINF_CLIPDRAG,on);
      minum=Menunumfromcmd("@DRAGGING");
      if(mi=ItemAddress(win->menu,minum))
      {  ClearMenuStrip(win->window);
         if(on) mi->Flags|=CHECKED;
         else mi->Flags&=~CHECKED;
         ResetMenuStrip(win->window,win->menu);
      }
   }
   return TRUE;
}

static BOOL Doeditsource(struct Arexxcmd *ac,struct Awindow *win,UBYTE *urlname)
{  void *url=Findurl("",urlname,0);
   if(url)
   {  Auspecial(url,AUMST_EDITSOURCE);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dofixcache(struct Arexxcmd *ac,struct Awindow *win,BOOL force)
{  Fixcache(force);
   return TRUE;
}

static BOOL Doflushcache(struct Arexxcmd *ac,struct Awindow *win,
   BOOL images,BOOL docs,BOOL all,UBYTE *urlname)
{  if(images)
   {  Flushsources(all?SRCFT_ALLIMAGES:SRCFT_NDIMAGES);
   }
   if(docs)
   {  Flushsources(SRCFT_NDDOCUMENTS);
   }
   if(urlname)
   {  void *url=Findurl("",urlname,0);
      if(url)
      {  Auspecial(url,AUMST_FLUSHSOURCE);
      }
   }
   return TRUE;
}

static BOOL Dofocus(struct Arexxcmd *ac,struct Awindow *win,UBYTE *targetname)
{  void *target;
   if(win && (target=Targetframe(win->frame,targetname)))
   {  win->focus=target;
   }
   return TRUE;
}

static BOOL Doget(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *item,UBYTE *targetname,UBYTE *varname,UBYTE *stem,UBYTE *pattern,BOOL all)
{  UBYTE *p;
   short i;
   void *target;
   UBYTE buf[40];
   if(ac->flags&ARXCF_ALLOWGET)
   {  if(varname && (ac->varname=Dupstr(varname,-1)))
      {  for(p=ac->varname;*p;p++) *p=toupper(*p);
      }
      if(STRIEQUAL(item,"SCREEN"))
      {  ac->result=Dupstr((UBYTE *)Agetattr(Aweb(),AOAPP_Screenname),-1);
      }
      else if(STRIEQUAL(item,"ACTIVEPORT"))
      {  if(win && !*activeport) Setactiveport(win->portname);
         ac->result=Dupstr(activeport,-1);
      }
      else if(STRIEQUAL(item,"WINDOW"))
      {  if(win && win->window)
         {  sprintf(buf,"%d,%d,%d,%d",win->window->LeftEdge,win->window->TopEdge,
               win->window->Width,win->window->Height);
            ac->result=Dupstr(buf,-1);
         }
         else ac->errorlevel=RXERR_WARNING;
      }
      else if(STRIEQUAL(item,"FOCUS"))
      {  if(win)
         {  ac->result=Rexxframeid(win->focus?win->focus:win->frame);
         }
      }
      else if(STRIEQUAL(item,"WINDOWS"))
      {  if((ac->flags&ARXCF_ALLOWSTEM) && stem)
         {  i=0;
            for(win=windows.first;win->object.next;win=win->object.next)
            {  if(win->window)
               {  sprintf(buf,"%d,%d,%d,%d",win->window->LeftEdge,win->window->TopEdge,
                     win->window->Width,win->window->Height);
                  i++;
                  Setstemvar(ac,stem,i,"PORT",win->portname?win->portname:NULLSTRING);
                  Setstemvar(ac,stem,i,"RECT",buf);
                  Setstemvar(ac,stem,i,"ZOOMED",(win->window->Flags&WFLG_ZOOMED)?"1":"0");
               }
            }
            sprintf(buf,"%d",i);
            Setstemvar(ac,stem,0,NULL,buf);
         }
         else ac->errorlevel=RXERR_INVARGS;
      }
      else if(STRIEQUAL(item,"CACHE"))
      {  if((ac->flags&ARXCF_ALLOWSTEM) && stem)
         {  Getcachecontents(ac,stem,pattern);
         }
         else ac->errorlevel=RXERR_INVARGS;
      }
      else if(STRIEQUAL(item,"HOTLIST"))
      {  if((ac->flags&ARXCF_ALLOWSTEM) && stem)
         {  Gethotlistcontents(ac,stem,FALSE);
         }
         else ac->errorlevel=RXERR_INVARGS;
      }
      else if(STRIEQUAL(item,"HOTLISTGROUPS"))
      {  if((ac->flags&ARXCF_ALLOWSTEM) && stem)
         {  Gethotlistcontents(ac,stem,TRUE);
         }
         else ac->errorlevel=RXERR_INVARGS;
      }
      else if(STRIEQUAL(item,"SELECTION"))
      {  if(win && win->dragstartobject)
         {  struct Buffer buf={0};
            UBYTE *p;
            Adragcopy(win->frame,win->dragstartobject,win->dragstartobjpos,
               win->dragendobject,win->dragendobjpos,AMDS_BEFORE,&buf);
            ac->result=Dupstr(buf.buffer,buf.length);
            Freebuffer(&buf);
            for(p=ac->result;*p;p++)
            {  if(*p==0xa0) *p=' ';
            }
         }
         else ac->errorlevel=RXERR_WARNING;
      }
      else if(STRIEQUAL(item,"VERSION"))
      {  ac->result=Dupstr(awebversion,-1);
      }
      else if(STRIEQUAL(item,"CLIP"))
      {  UBYTE *buf;
         long len;
         if(buf=ALLOCTYPE(UBYTE,8192,0))
         {  len=Clippaste(buf,8192);
            ac->result=Dupstr(buf,len);
            FREE(buf);
         }
      }
      else if(STRIEQUAL(item,"TRANSFERS"))
      {  if((ac->flags&ARXCF_ALLOWSTEM) && stem)
         {  Gettransfers(ac,stem);
         }
         else ac->errorlevel=RXERR_INVARGS;
      }
      else if(STRIEQUAL(item,"STATUS"))
      {  if(win)
         {  ac->result=Dupstr((UBYTE *)Agetattr((struct Aobject *)win,AOWIN_Status),-1);
         }
         else ac->errorlevel=RXERR_WARNING;
      }
      else if(STRIEQUAL(item,"STATUSD"))
      {  if(win)
         {  ac->result=Dupstr((UBYTE *)Agetattr((struct Aobject *)win,AOWIN_Hpstatus),-1);
         }
         else ac->errorlevel=RXERR_WARNING;
      }
      else if(STRIEQUAL(item,"COOKIES"))
      {  if((ac->flags&ARXCF_ALLOWSTEM) && stem)
         {  Getrexxcookies(ac,stem);
         }
         else ac->errorlevel=RXERR_INVARGS;
      }
      else
      {  if(win && win->whis
         && (target=Targetframe(win->frame,targetname)))
         {  if(STRIEQUAL(item,"URL"))
            {  UBYTE *url=(UBYTE *)Agetattr((void *)Agetattr(target,AOFRM_Url),AOURL_Url);
               ac->result=Dupstr(url,-1);
            }
            else if(STRIEQUAL(item,"FINALURL"))
            {  UBYTE *url=(UBYTE *)Agetattr((void *)Agetattr(target,AOFRM_Url),AOURL_Finalurl);
               ac->result=Dupstr(url,-1);
            }
            else if(STRIEQUAL(item,"SOURCE"))
            {  void *src=(void *)Agetattr((void *)Agetattr(target,AOFRM_Url),AOURL_Source);
               UBYTE *data=(UBYTE *)Agetattr(src,AOSRC_Getsource);
               if(data)
               {  ac->result=Dupstr(data,-1);
               }
               else
               {  ac->errorlevel=RXERR_WARNING;
               }
            }
            else if(STRIEQUAL(item,"TITLE"))
            {  UBYTE *title=(UBYTE *)Agetattr(target,AOFRM_Title);
               if(!title)
               {  title=(UBYTE *)Agetattr((void *)Agetattr(target,AOFRM_Url),AOURL_Url);
               }
               ac->result=Dupstr(title,-1);
            }
            else if(STRIEQUAL(item,"MIME"))
            {  UBYTE *mime=(UBYTE *)Agetattr((void *)Agetattr(target,AOFRM_Url),AOURL_Contenttype);
               if(mime) ac->result=Dupstr(mime,-1);
               else ac->result=Dupstr("",-1);
            }
            else if(STRIEQUAL(item,"PORT"))
            {  if(targetname) win=(struct Awindow *)Agetattr(target,AOBJ_Window);
               if(win && win->portname)
               {  ac->result=Dupstr(win->portname,-1);
               }
               else
               {  ac->errorlevel=RXERR_WARNING;
               }
            }
            else if(STRIEQUAL(item,"FRAMESET"))
            {  UBYTE *p;
               if(Agetattr(target,AOBJ_Isframeset))
               {  p="1";
               }
               else
               {  p="0";
               }
               ac->result=Dupstr(p,-1);
            }
            else if(STRIEQUAL(item,"SECURE"))
            {  if(Agetattr(target,AOBJ_Secure))
               {  p="1";
               }
               else
               {  p="0";
               }
               ac->result=Dupstr(p,-1);
            }
            else if((ac->flags&ARXCF_ALLOWSTEM) && stem)
            {  struct Amgetrexx amg={{0}};
               amg.amsg.method=AOM_GETREXX;
               amg.frame=target;
               amg.ac=ac;
               amg.stem=stem;
               amg.index=0;
               if(STRIEQUAL(item,"FRAMES"))
               {  if(all)
                  {  amg.info=AMGRI_ALLFRAMES;
                     amg.frame=NULL;
                  }
                  else
                  {  amg.info=AMGRI_FRAMES;
                  }
               }
               else if(STRIEQUAL(item,"LINKS"))
               {  amg.info=AMGRI_LINKS;
               }
               else if(STRIEQUAL(item,"IMAGES"))
               {  amg.info=AMGRI_IMAGES;
               }
               else if(STRIEQUAL(item,"NAMES"))
               {  amg.info=AMGRI_NAMES;
               }
               else if(STRIEQUAL(item,"INFO"))
               {  amg.info=AMGRI_INFO;
               }
               else ac->errorlevel=RXERR_INVARGS;
               if(amg.info)
               {  Anotify(target,(struct Amessage *)&amg);
                  sprintf(buf,"%ld",amg.index);
                  Setstemvar(ac,stem,0,NULL,buf);
               }
            }
            else ac->errorlevel=RXERR_INVARGS;
         }
         else ac->errorlevel=RXERR_WARNING;
      }
   }
   return TRUE;
}

static BOOL Dogetcfg(struct Arexxcmd *ac)
{
    void *base;

    if (ac->flags & ARXCF_ALLOWGET)
    {
        if ((base = Openaweblib(AWEBLIBPATH AREXX_AWEBLIB, AREXX_VERSION)))
        {
            __AwebArexxGetCfg_WB(base, ac, &prefs);

            Closeaweblib(base);
        }
    }
    return TRUE;
}

static BOOL Dogethistory(struct Arexxcmd *ac,struct Awindow *win,long *window,BOOL mainline,
   UBYTE *stem)
{  Historyarexx(ac,window,mainline,stem);
   return TRUE;
}

static BOOL Dogo(struct Arexxcmd *ac,struct Awindow *win,
   long *np,BOOL back,BOOL fwd,BOOL home)
{  ULONG tag=0;
   long i;
   void *whis,*nwhis;
   if(win)
   {  if(home)
      {  if(*prefs.network.homeurl) Followurlname(win,prefs.network.homeurl,0);
      }
      else
      {  if(back)
         {  tag=AOWHS_Previous;
         }
         else if(fwd)
         {  tag=AOWHS_Next;
         }
         else ac->errorlevel=RXERR_INVARGS;
         if(tag)
         {  if(np)
            {  i=MAX(1,*np);
            }
            else
            {  i=1;
            }
            whis=win->hiswhis;
            nwhis=NULL;
            for(;i && whis;i--)
            {  nwhis=whis;
               whis=(void *)Agetattr(whis,tag);
            }
            if(!whis) whis=nwhis;
            if(whis && whis!=win->hiswhis)
            {  Followhis(win,whis);
            }
         }
      }
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dohotlist(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *set,BOOL save,BOOL restore)
{  if(restore)
   {  Restorehotlist();
   }
   else
   {  if(set)
      {  Sethotlistcontents(ac,set);
      }
      if(save)
      {  Savehotlist();
      }
   }
   if(win && !restore && !set && !save)
   {  if(prefs.program.hlrequester) Hotlistviewer(win->key);
      else Doopen(ac,win,"x-aweb:hotlist",NULL,FALSE,NULL,FALSE);
   }
   return TRUE;
}

static BOOL Doiconify(struct Arexxcmd *ac,struct Awindow *win,BOOL hide,BOOL show)
{  BOOL iconify;
   if(hide) iconify=TRUE;
   else if(show) iconify=FALSE;
   else iconify=!Agetattr(Aweb(),AOAPP_Iconified);
   Iconify(iconify);
   return TRUE;
}

static BOOL Doimageloading(struct Arexxcmd *ac,struct Awindow *win,
   BOOL off,BOOL maps,BOOL all)
{  short type;
   if(off) type=LOADIMG_OFF;
   else if(maps) type=LOADIMG_MAPS;
   else if(all) type=LOADIMG_ALL;
   else type=(prefs.network.loadimg+1)%3;
   Prefsloadimg(type);
   return TRUE;
}

static BOOL Doinfo(struct Arexxcmd *ac,struct Awindow *win,UBYTE *targetname)
{  void *target;
   if(win)
   {  if(targetname)
      {  target=Targetframe(win->frame,targetname);
      }
      else
      {  target=win->frame;
      }
      Asetattrs(target,AOFRM_Info,TRUE,TAG_END);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dojavascript(struct Arexxcmd *ac,struct Awindow *win,UBYTE *source,
   UBYTE *file,UBYTE *targetname,UBYTE *varname)
{  void *target;
   struct Jcontext *jc;
   struct Jvar *jv;
   UBYTE *result,*p;
   UBYTE *srcbuf=NULL;
   if(win && prefs.browser.dojs)
   {  if(!source && file)
      {  long fh;
         struct FileInfoBlock *fib;
         long size;
         if(fh=Open(file,MODE_OLDFILE))
         {  if(fib=AllocDosObjectTags(DOS_FIB,TAG_END))
            {  ExamineFH(fh,fib);
               if(srcbuf=ALLOCTYPE(UBYTE,fib->fib_Size+1,0))
               {  size=Read(fh,srcbuf,fib->fib_Size);
                  if(size<0) size=0;
                  srcbuf[size]='\0';
                  source=srcbuf;
               }
               FreeDosObject(DOS_FIB,fib);
            }
            Close(fh);
         }
         else ac->errorlevel=RXERR_WARNING;
      }
      if(source)
      {  if(varname && (ac->varname=Dupstr(varname,-1)))
         {  for(p=ac->varname;*p;p++) *p=toupper(*p);
         }
         if(targetname)
         {  target=Targetframe(win->frame,targetname);
         }
         else
         {  target=win->frame;
         }
         Runjavascript(target,source,NULL);
         jc=(struct Jcontext *)Agetattr(Aweb(),AOAPP_Jcontext);
         if(jv=Jgetreturnvalue(jc))
         {  result=Jtostring(jc,jv);
         }
         else result="";
         ac->result=Dupstr(result,-1);
      }
      if(srcbuf) FREE(srcbuf);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dojsbreak(struct Arexxcmd *ac,struct Awindow *win)
{  if(win)
   {  Asetattrs(win->frame,AOBJ_Jscancel,TRUE,TAG_END);
   }
   return TRUE;
}

static BOOL Dojsdebug(struct Arexxcmd *ac,struct Awindow *win,BOOL on,BOOL off)
{  struct MenuItem *mi;
   BOOL wason;
   if(win && win->window && (mi=ItemAddress(win->menu,Menunumfromcmd("@DEBUGJS"))))
   {  ClearMenuStrip(win->window);
      wason=BOOLVAL(mi->Flags&CHECKED);
      if(on) mi->Flags|=CHECKED;
      else if(off) mi->Flags&=~CHECKED;
      else mi->Flags^=CHECKED;
      ResetMenuStrip(win->window,win->menu);
   }
   return TRUE;
}

static BOOL Doload(struct Arexxcmd *ac,struct Awindow *win,UBYTE *urlname,
   UBYTE *saveas,BOOL reload,BOOL append,BOOL savereq,BOOL noicon,UBYTE *post)
{  void *url=Findurl("",urlname,post?-1:0);
   void *src;
   ULONG flags=0;
   if(url)
   {  if(saveas || savereq)
      {  flags|=AUMLF_DOWNLOAD;
         if(noicon) flags|=AUMLF_NOICON;
      }
      if(reload) flags|=AUMLF_RELOAD;
      if(win && win->flags&WINF_NOPROXY) flags|=AUMLF_NOPROXY;
      Auload(url,flags,url,post,win?win->frame:NULL);
      if(saveas)
      {  if(src=(void *)Agetattr(url,AOURL_Saveassource))
         {  Asetattrs(src,
               AOSRC_Savename,(Tag)saveas,
               AOSRC_Saveappend,append,
               TAG_END);
         }
      }
   }
   return TRUE;
}

static BOOL Doloadimages(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *targetname,BOOL maps,BOOL restricted)
{  void *target;
   ULONG flags=0;
   if(win && (target=Targetframe(win->frame,targetname)))
   {  if(maps) flags|=ACMLF_MAPSONLY;
      if(restricted) flags|=ACMLF_RESTRICT;
      Anotifycload(target,flags);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Doloadsettings(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *path,BOOL request)
{  if(request)
   {  if(win) Loadsettingsreq(win);
   }
   else if(path)
   {  Loadsettings(path);
   }
   return TRUE;
}

static BOOL Domimetype(struct Arexxcmd *ac,struct Awindow *win,UBYTE *name)
{  UBYTE *type=Mimetypefromext(name);
   if(!type) type="";
   ac->result=Dupstr(type,-1);
   return TRUE;
}

static BOOL Donew(struct Arexxcmd *ac,struct Awindow *oldwin,
   UBYTE *url,UBYTE *name,BOOL reload,UBYTE *post,BOOL smart,BOOL nonav, LONG *left, LONG *top, ULONG *width, ULONG *height,STRPTR pubscreenname)
{  struct Awindow *win;
   win=Anewobject(AOTP_WINDOW,
      AOWIN_Name,(Tag)name,
      AOWIN_Navigation,nonav?FALSE:TRUE,
      AOWIN_Buttonbar,nonav?FALSE:TRUE, 
      width?AOWIN_Innerwidth:TAG_IGNORE,width?*width:0,
      height?AOWIN_Innerheight:TAG_IGNORE,height?*height:0,
      left?AOWIN_Left:TAG_IGNORE,left?*left:0,    
      top?AOWIN_Top:TAG_IGNORE,top?*top:0,
      AOWIN_PubScreenName,pubscreenname,
      AOWIN_Noproxy,Agetattr((struct Aobject *)oldwin,AOWIN_Noproxy),
      TAG_END);
   if(win)
   {  ac->result=Dupstr(win->portname,-1);
      Doopen(ac,win,url,name,reload,post,smart);
   }
   else ac->errorlevel=RXERR_FATAL;
   return TRUE;
}

static BOOL Doopen(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *urlname,UBYTE *targetname,BOOL reload,UBYTE *post,BOOL smart)
{  void *url,*target,*twin=NULL;
   UBYTE *frag,*id;
   if(win)
   {  if(targetname)
      {  if(target=Targetframe(win->frame,targetname))
         {  Agetattrs(target,
               AOBJ_Window,(Tag)&twin,
               AOFRM_Id,(Tag)&id,
               TAG_END);
         }
         else ac->errorlevel=RXERR_WARNING;
         if(twin) win=twin;
      }
      else
      {  id=NULL;
         target=win->frame;
      }
      if(smart)
      {  Inputwindocsmart(win,urlname,id);
      }
      else
      {  url=Findurl("",urlname,post?-1:0);
         frag=Fragmentpart(urlname);
         if(reload || post)
         {  Auload(url,
               (reload?AUMLF_RELOAD:0)|(win->flags&WINF_NOPROXY?AUMLF_NOPROXY:0),
               url,post,target);
         }
         if(!ac->errorlevel)
         {  if(reload) Inputwindocreload(win,url,frag,id);
            else Inputwindoc(win,url,frag,id);
         }
      }
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Doopenreq(struct Arexxcmd *ac,struct Awindow *win,BOOL file,UBYTE *pattern)
{  if(win)
   {  if(file)
      {  Openfilereq(win,pattern);
      }
      else
      {  Openurlreq(win);
      }
   }
   return TRUE;
}

static BOOL Doplaybgsound(struct Arexxcmd *ac,struct Awindow *win)
{  if(win)
   {  Anotifyset(win->frame,AOCDV_Playsound,TRUE,TAG_END);
   }
   return TRUE;
}

#if !defined(__amigaos4__)
struct AwebPluginIFace;
#endif

/* See notes in source.c about Query plugin under Os3 / Os4 */

static BOOL Doplugin(struct Arexxcmd *ac,struct Awindow *win,UBYTE *plugin,UBYTE *command)
{
   struct Library *AwebPluginBase;
   struct Pluginquery pq={0};
   struct Plugincommand pc={0};
   struct AwebPluginIFace *IAwebPlugin;

   pq.structsize=sizeof(pq);

   if((AwebPluginBase = OpenLibrary(plugin,0)))
   {

#if defined(__amigaos4__)
      if((IAwebPlugin = GetInterface(AwebPluginBase,"main",1,0)))
      {
          if(TRUE)
#else
          if(AwebPluginBase->lib_NegSize>=36)
#endif
          {
             Queryplugin(&pq);
             if(pq.command)
             {  pc.structsize=sizeof(pc);
                pc.command=Dupstr(command,-1);
                Commandplugin(&pc);
                ac->result=pc.result;
                ac->errorlevel=pc.rc;
                FREE(pc.command);
             }
             else ac->errorlevel=RXERR_WARNING;
          }
          else ac->errorlevel=RXERR_WARNING;
          Closelib(&AwebPluginBase,(struct Interface **)&IAwebPlugin);
#if defined(__amigaos4__)
      }
#endif
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Doprint(struct Arexxcmd *ac,struct Awindow *win,long *scale,
   BOOL center,BOOL noformfeed,BOOL nobackground,BOOL wait,BOOL request,BOOL debug)
{  BOOL done=TRUE;
   if(request)
   {  Printdoc(win,debug);
   }
   else
   {  if(!(ac->flags&ARXCF_TRUEREXX)) wait=FALSE;
      if(Printarexx(win,scale?*scale:0,center,!noformfeed,!nobackground,wait?ac:NULL,debug))
      {  if(wait) done=FALSE;
      }
      else ac->errorlevel=RXERR_WARNING;
   }
   return done;
}

static BOOL Doquit(struct Arexxcmd *ac,BOOL force)
{  Quit(force);
   return TRUE;
}

static BOOL Doreload(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *targetname,BOOL images)
{  void *target;
   if(win && (target=Targetframe(win->frame,targetname)))
   {  if(images)
      {  Anotifycload(target,ACMLF_RELOAD);
      }
      else
      {  Asetattrs(target,AOFRM_Reload,TRUE,TAG_END);
      }
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dorequest(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *title,UBYTE *body,UBYTE *gadgets,BOOL nowait)
{  UBYTE *bodydup=NULL;
   BOOL ok=FALSE,done=TRUE;
   struct Reqdata *rd;
   if(!(ac->flags&ARXCF_TRUEREXX)) nowait=TRUE;
   if((rd=ALLOCSTRUCT(Reqdata,1,MEMF_CLEAR))
   && (rd->title=Dupstr(title,-1))
   && (bodydup=Dupstr(body,-1)))
   {  if(!nowait) rd->ac=ac;
      ok=Asyncrequest(rd->title,bodydup,gadgets,(requestfunc *)Disposereqdata,rd);
   }
   if(ok)
   {  done=nowait;
   }
   else
   {  if(rd)
      {  rd->ac=NULL;
         Disposereqdata(0,rd);
      }
   }
   return done;
}

static BOOL Dorequestfile(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *title,UBYTE *file,UBYTE *pattern,BOOL savemode,BOOL dirsonly)
{  BOOL done=TRUE;
   if(Anewobject(AOTP_FILEREQ,
      AOFRQ_Title,(Tag)title,
      AOFRQ_Filename,(Tag)file,
      AOFRQ_Pattern,(Tag)pattern,
      AOFRQ_Savemode,savemode,
      AOFRQ_Dirsonly,dirsonly,
      AOFRQ_Arexx,(Tag)ac,
      TAG_END))
   {  done=FALSE;
   }
   return done;
}

static BOOL Dorequeststring(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *title,UBYTE *body,UBYTE *gadgets,UBYTE *defstring)
{  BOOL ok=FALSE,done=TRUE;
   struct Reqdata *rd;
   if((rd=ALLOCSTRUCT(Reqdata,1,MEMF_CLEAR))
   && (rd->string=ALLOCTYPE(UBYTE,128,MEMF_CLEAR))
   && (rd->title=Dupstr(title,-1)))
   {  rd->ac=ac;
      if(defstring) strcpy(rd->string,defstring);
      ok=Asyncpromptrequest(rd->title,body,gadgets,(requestfunc *)Disposereqdata,rd,rd->string);
   }
   if(ok)
   {  done=FALSE;
   }
   else
   {  if(rd)
      {  rd->ac=NULL;
         Disposereqdata(0,rd);
      }
   }
   return done;
}

static BOOL Doresetframe(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *targetname)
{  void *target;
   if(win && (target=Targetframe(win->frame,targetname)))
   {  Anotifyset(target,AOFRM_Prepreset,TRUE,TAG_END);
      Asetattrs(target,AOFRM_Resetframe,TRUE,TAG_END);
      Arender(target,NULL,0,0,AMRMAX,AMRMAX,AMRF_CLEAR,NULL);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dorun(struct Arexxcmd *ac,struct Awindow *win,UBYTE *name)
{  if(name && win)
   {  Sendarexxcmd(win->key,name);
   }
   return TRUE;
}

static BOOL Dosaveas(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *name,BOOL append,UBYTE *targetname,BOOL noicon)
{  void *target,*src;
   if(win && win->whis
   && (target=Targetframe(win->frame,targetname)))
   {  src=(void *)Agetattr((void *)Agetattr(target,AOFRM_Url),AOURL_Source);
      Asetattrs(src,
         AOSRC_Savesource,TRUE,
         name?AOSRC_Savename:TAG_IGNORE,(Tag)name,
         name?AOSRC_Saveappend:TAG_IGNORE,append,
         AOSRC_Noicon,noicon,
         TAG_END);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dosaveauthorize(struct Arexxcmd *ac,struct Awindow *win)
{  Saveauthor();
   return TRUE;
}

static BOOL Dosaveiff(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *name,BOOL noicon,BOOL wait)
{  BOOL done=TRUE;
   if(win)
   {  done=!wait;
      Saveasiff(win,name,noicon,wait?ac:NULL);
   }
   return done;
}

static BOOL Dosavesettings(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *path,BOOL request)
{  if(request)
   {  if(win) Savesettingsreq(win);
   }
   else if(path)
   {  Savesettingsas(path);
   }
   else
   {  Saveallsettings();
   }
   return TRUE;
}

static BOOL Doscreentoback(struct Arexxcmd *ac)
{  Asetattrs(Aweb(),AOAPP_Tofront,FALSE,TAG_END);
   return TRUE;
}

static BOOL Doscreentofront(struct Arexxcmd *ac)
{  Asetattrs(Aweb(),AOAPP_Tofront,TRUE,TAG_END);
   return TRUE;
}

static BOOL Doscroll(struct Arexxcmd *ac,struct Awindow *win,
   long *np,BOOL up,BOOL down,BOOL left,BOOL right,BOOL page,BOOL sfar,UBYTE *targetname)
{  long n;
   void *target;
   if(win
/* && win->whis ** Why was this here? */
   && (target=Targetframe(win->frame,targetname)))
   {  if(np)
      {  n=MAX(1,*np);
      }
      else n=1;
      if(up)
      {  Scrollframe(target,-n,AOFRM_Toppos,AOFRM_Innerheight,page,sfar);
      }
      else if(down)
      {  Scrollframe(target,n,AOFRM_Toppos,AOFRM_Innerheight,page,sfar);
      }
      if(left)
      {  Scrollframe(target,-n,AOFRM_Leftpos,AOFRM_Innerwidth,page,sfar);
      }
      else if(right)
      {  Scrollframe(target,n,AOFRM_Leftpos,AOFRM_Innerwidth,page,sfar);
      }
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dosearch(struct Arexxcmd *ac,struct Awindow *win,UBYTE *targetname)
{  void *target;
   if(win)
   {  if(targetname)
      {  target=Targetframe(win->frame,targetname);
      }
      else
      {  target=win->frame;
      }
      Asetattrs(target,AOFRM_Search,TRUE,TAG_END);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dosetcfg(struct Arexxcmd *ac)
{
    struct Library *base = Openaweblib(AWEBLIBPATH AREXX_AWEBLIB, AREXX_VERSION);

    if(base)
    {
        __AwebArexxSetCfg_WB(base, ac, &prefs);

        Closeaweblib(base);
    }

    return TRUE;
}

static BOOL Dosetclip(struct Arexxcmd *ac,struct Awindow *win,UBYTE *value)
{  if(value)
   {  Clipcopy(value,strlen(value));
   }
   return TRUE;
}

static BOOL Dosetcookies(struct Arexxcmd *ac,struct Awindow *win,UBYTE *stem,BOOL add)
{  if(ac->flags&ARXCF_ALLOWSTEM)
   {  Setrexxcookies(ac,stem,add);
   }
   return TRUE;
}

static BOOL Dosnapshot(struct Arexxcmd *ac,struct Awindow *win)
{  Snapshotwindows(win);
   return TRUE;
}

static BOOL Dostatusfield(struct Arexxcmd *ac,struct Awindow *win,UBYTE *set)
{  if(win)
   {  Asetattrs((struct Aobject *)win,AOWIN_Status,(Tag)set,TAG_END);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dosubwindow(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *type,BOOL open,BOOL close)
{  if(STRIEQUAL(type,"HOTVIEWER"))
   {  if(open) Hotlistviewer(win?win->key:0);
      else if(close) Closehotlistviewer();
   }
   else if(STRIEQUAL(type,"HOTMANAGER"))
   {  if(open) Hotlistmanager(win?win->key:0);
      else if(close) Closehotlistmanager();
   }
   else if(STRIEQUAL(type,"CACHEBROWSER"))
   {  if(open) Opencabrowse();
      else if(close) Closecabrowse();
   }
   else if(STRIEQUAL(type,"HISTORY"))
   {  if(open) Openwhiswindow(win?win->hiswhis:NULL,win?win->windownr:0);
      else if(close) Closewhiswindow();
   }
   else if(STRIEQUAL(type,"NETSTATUS"))
   {  if(open) Opennetstat();
      else if(close) Closenetstat();
   }
   else if(STRIEQUAL(type,"AUTHORIZE"))
   {  if(open) Openauthedit();
      else if(close) Closeauthedit();
   }
   else if(STRIEQUAL(type,"ABOUT"))
   {  if(open) Aboutreq(win?win->portname:NULLSTRING);
      else if(close) Closeabout();
   }
   else ac->errorlevel=RXERR_INVARGS;
   return TRUE;
}

static BOOL Dosystem(struct Arexxcmd *ac,struct Awindow *win,UBYTE *cmd,UBYTE *args)
{  Spawn(FALSE,cmd,args?args:NULLSTRING,"");
   return TRUE;
}

static BOOL Dourlencode(struct Arexxcmd *ac,struct Awindow *win,UBYTE *string,UBYTE *varname)
{  struct Buffer buf={0};
   UBYTE *p;
   if(varname && (ac->varname=Dupstr(varname,-1)))
   {  for(p=ac->varname;*p;p++) *p=toupper(*p);
   }
   Urlencode(&buf,string,strlen(string));
   if(buf.length) ac->result=Dupstr(buf.buffer,buf.length);
   else ac->result=Dupstr("",0);
   return TRUE;
}

static BOOL Dourlfield(struct Arexxcmd *ac,struct Awindow *win,BOOL activate,
   long *pos,UBYTE *set,BOOL paste)
{  UBYTE buf[MAXSTRBUFCHARS];
   UBYTE *start,*p;
   long l;
   if(win && win->window && win->urlgad)
   {  if(paste)
      {  l=Clippaste(buf,MAXSTRBUFCHARS-1);
         if(l)
         {  buf[l]='\0';
            for(start=buf;*start && !Isprint(*start);start++);
            for(p=start;*p && Isprint(*p);p++);
            *p='\0';
            Setgadgetattrs(win->urlgad,win->window,NULL,
               STRINGA_TextVal,start,
               STRINGA_BufferPos,p-start,
               STRINGA_DispPos,0,
               TAG_END);
         }
      }
      else if(set)
      {  Setgadgetattrs(win->urlgad,win->window,NULL,
            STRINGA_TextVal,set,
            STRINGA_DispPos,0,
            STRINGA_BufferPos,pos?MAX(0,(*pos)-1):strlen(set),
            TAG_END);
      }
      else if(pos || activate)
      {  Setgadgetattrs(win->urlgad,win->window,NULL,
            STRINGA_BufferPos,pos?MAX(0,(*pos)-1):strlen(win->urlbuf),
            TAG_END);
      }
      if(activate)
      {  ActivateLayoutGadget(win->layoutgad,win->window,NULL,(ULONG) win->urlgad);
      }
   }
   return TRUE;
}

static BOOL Douseproxy(struct Arexxcmd *ac,struct Awindow *win,
   BOOL enable,BOOL disable)
{  UWORD minum;
   struct MenuItem *mi;
   if(win)
   {  if(!enable && !disable) disable=!BOOLVAL(win->flags&WINF_NOPROXY);
      SETFLAG(win->flags,WINF_NOPROXY,disable);
      minum=Menunumfromcmd("@NOPROXY");
      if(mi=ItemAddress(win->menu,minum))
      {  ClearMenuStrip(win->window);
         if(disable) mi->Flags|=CHECKED;
         else mi->Flags&=~CHECKED;
         ResetMenuStrip(win->window,win->menu);
      }
   }
   return TRUE;
}

static BOOL Doviewsource(struct Arexxcmd *ac,struct Awindow *win,UBYTE *urlname)
{  void *url=Findurl("",urlname,0);
   if(url)
   {  Auspecial(url,AUMST_VIEWSOURCE);
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dowait(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *urlname,BOOL docs,BOOL imgs,BOOL all)
{  BOOL done=TRUE;
   void *url;
   if(ac->flags&ARXCF_TRUEREXX)
   {  if(win && urlname)
      {  url=Findurl("",urlname,0);
         Addwaitrequest(ac,win->key,FALSE,FALSE,url);
         done=FALSE;
      }
      else if(all)
      {  Addwaitrequest(ac,0,FALSE,FALSE,NULL);
         done=FALSE;
      }
      else if(win)
      {  Addwaitrequest(ac,win->key,docs,imgs,NULL);
         done=FALSE;
      }
      else ac->errorlevel=RXERR_WARNING;
   }
   return done;
}

static BOOL Dowindow(struct Arexxcmd *ac,struct Awindow *win,
   UBYTE *rect,BOOL activate,BOOL tofront,BOOL toback,BOOL zip,long *next)
{  long x,y,w,h;
   if(win && next && *next)
   {  win=Nextwindow(win,*next);
   }
   if(win && win->window)
   {  if(rect)
      {  if(sscanf(rect,"%ld,%ld,%ld,%ld",&x,&y,&w,&h)==4)
         {  ChangeWindowBox(win->window,x,y,w,h);
         }
         else ac->errorlevel=RXERR_INVARGS;
      }
      else if(zip)
      {  ZipWindow(win->window);
      }
      if(activate)
      {  ActivateWindow(win->window);
      }
      if(tofront)
      {  WindowToFront(win->window);
      }
      else if(toback)
      {  WindowToBack(win->window);
      }
   }
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dowindowtoback(struct Arexxcmd *ac,struct Awindow *win)
{  if(win && win->window) WindowToBack(win->window);
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

static BOOL Dowindowtofront(struct Arexxcmd *ac,struct Awindow *win)
{  if(win && win->window) WindowToFront(win->window);
   else ac->errorlevel=RXERR_WARNING;
   return TRUE;
}

#ifdef BETAKEYFILE
static BOOL Dohttpdebug(struct Arexxcmd *ac,BOOL on)
{  if(ac->parameter[0]) httpdebug=TRUE;
   else httpdebug=FALSE;
   return TRUE;
}
#endif

/*-----------------------------------------------------------------------*/

BOOL Doarexxcmd(struct Arexxcmd *ac)
{  struct Awindow *win=Findwindow(ac->windowkey);
   BOOL done=TRUE;
   switch(ac->command)
   {
/*
      case ARX_:
         done=Do(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1]);
         break;
*/
      case ARX_ACTIVATEWINDOW:
         done=Doactivatewindow(ac,win);
         break;
      case ARX_ADDHOTLIST:
         done=Doaddhotlist(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            (UBYTE *)ac->parameter[2],(UBYTE *)ac->parameter[3]);
         break;
      case ARX_ALLOWCMD:
         done=Doallowcmd(ac,win);
         break;
      case ARX_BACKGROUND:
         done=Dobackground(ac,win,ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_BGSOUND:
         done=Dobgsound(ac,win,ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_CANCEL:
         done=Docancel(ac,win,(long *)ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_CHANCLOSE:
         done=Dochanclose(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_CHANDATA:
         done=Dochandata(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            ac->parameter[2]);
         break;
      case ARX_CHANHEADER:
         done=Dochanheader(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1]);
         break;
      case ARX_CHANOPEN:
         done=Dochanopen(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_CLOSE:
         done=Doclose(ac,win,ac->parameter[0]);
         break;
      case ARX_CLEARSELECTION:
         done=Doclearselection(ac,win);
         break;
      case ARX_COPYBLOCK:
         done=Docopyblock(ac,win);
         break;
      case ARX_COPYURL:
         done=Docopyurl(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_DELETECACHE:
         done=Dodeletecache(ac,win,ac->parameter[0],ac->parameter[1],ac->parameter[2],
            (UBYTE *)ac->parameter[3]);
         break;
      case ARX_DRAGGING:
         done=Dodragging(ac,win,ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_EDITSOURCE:
         done=Doeditsource(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_FIXCACHE:
         done=Dofixcache(ac,win,ac->parameter[0]);
         break;
      case ARX_FLUSHCACHE:
         done=Doflushcache(ac,win,ac->parameter[0],ac->parameter[1],ac->parameter[2],
            (UBYTE *)ac->parameter[3]);
         break;
      case ARX_FOCUS:
         done=Dofocus(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_GET:
         done=Doget(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            (UBYTE *)ac->parameter[2],(UBYTE *)ac->parameter[3],
            (UBYTE *)ac->parameter[4],ac->parameter[5]);
         break;
      case ARX_GETCFG:
         done=Dogetcfg(ac);
         break;
      case ARX_GETHISTORY:
         done=Dogethistory(ac,win,(long *)ac->parameter[0],ac->parameter[1],
            (UBYTE *)ac->parameter[2]);
         break;
      case ARX_GO:
         done=Dogo(ac,win,(long *)ac->parameter[0],ac->parameter[1],ac->parameter[2],
            ac->parameter[3]);
         break;
      case ARX_HOTLIST:
         done=Dohotlist(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1],ac->parameter[2]);
         break;
      case ARX_ICONIFY:
         done=Doiconify(ac,win,ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_IMAGELOADING:
         done=Doimageloading(ac,win,ac->parameter[0],ac->parameter[1],ac->parameter[2]);
         break;
      case ARX_INFO:
         done=Doinfo(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_JAVASCRIPT:
         done=Dojavascript(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            (UBYTE *)ac->parameter[2],(UBYTE *)ac->parameter[3]);
         break;
      case ARX_JSBREAK:
         done=Dojsbreak(ac,win);
         break;
      case ARX_JSDEBUG:
         done=Dojsdebug(ac,win,ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_LOAD:
         done=Doload(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            ac->parameter[2],ac->parameter[3],ac->parameter[4],ac->parameter[5],
            (UBYTE *)ac->parameter[6]);
         break;
      case ARX_LOADIMAGES:
         done=Doloadimages(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1],
            ac->parameter[2]);
         break;
      case ARX_LOADSETTINGS:
         done=Doloadsettings(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_MIMETYPE:
         done=Domimetype(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_NEW:
         done=Donew(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            ac->parameter[2],(UBYTE *)ac->parameter[3],ac->parameter[4],ac->parameter[5],
            (LONG *)ac->parameter[8],(LONG *)ac->parameter[9],
            (ULONG *)ac->parameter[6],(ULONG *)ac->parameter[7],(STRPTR)ac->parameter[10]);
         break;
      case ARX_OPEN:
         done=Doopen(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            ac->parameter[2],(UBYTE *)ac->parameter[3],ac->parameter[4]);
         break;
      case ARX_OPENREQ:
         done=Doopenreq(ac,win,ac->parameter[0],(UBYTE *)ac->parameter[1]);
         break;
      case ARX_PLAYBGSOUND:
         done=Doplaybgsound(ac,win);
         break;
      case ARX_PLUGIN:
         done=Doplugin(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1]);
         break;
      case ARX_PRINT:
         done=Doprint(ac,win,(long *)ac->parameter[0],ac->parameter[1],ac->parameter[2],
            ac->parameter[3],ac->parameter[4],ac->parameter[5],ac->parameter[6]);
         break;
      case ARX_QUIT:
         done=Doquit(ac,ac->parameter[0]);
         break;
      case ARX_RELOAD:
         done=Doreload(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_REQUEST:
         done=Dorequest(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            (UBYTE *)ac->parameter[2],ac->parameter[3]);
         break;
      case ARX_REQUESTFILE:
         done=Dorequestfile(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            (UBYTE *)ac->parameter[2],ac->parameter[3],ac->parameter[4]);
         break;
      case ARX_REQUESTSTRING:
         done=Dorequeststring(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1],
            (UBYTE *)ac->parameter[2],(UBYTE *)ac->parameter[3]);
         break;
      case ARX_RESETFRAME:
         done=Doresetframe(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_RUN:
         done=Dorun(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_SAVEAS:
         done=Dosaveas(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1],
            (UBYTE *)ac->parameter[2],ac->parameter[3]);
         break;
      case ARX_SAVEAUTHORIZE:
         done=Dosaveauthorize(ac,win);
         break;
      case ARX_SAVEIFF:
         done=Dosaveiff(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1],
            ac->parameter[2]);
         break;
      case ARX_SAVESETTINGS:
         done=Dosavesettings(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_SCREENTOBACK:
         done=Doscreentoback(ac);
         break;
      case ARX_SCREENTOFRONT:
         done=Doscreentofront(ac);
         break;
      case ARX_SCROLL:
         done=Doscroll(ac,win,(long *)ac->parameter[0],ac->parameter[1],ac->parameter[2],
            ac->parameter[3],ac->parameter[4],ac->parameter[5],ac->parameter[6],
            (UBYTE *)ac->parameter[7]);
         break;
      case ARX_SEARCH:
         done=Dosearch(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_SETCFG:
         done=Dosetcfg(ac);
         break;
      case ARX_SETCLIP:
         done=Dosetclip(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_SETCOOKIES:
         done=Dosetcookies(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_SNAPSHOT:
         done=Dosnapshot(ac,win);
         break;
      case ARX_STATUSFIELD:
         done=Dostatusfield(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_SUBWINDOW:
         done=Dosubwindow(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1],ac->parameter[2]);
         break;
      case ARX_SYSTEM:
         done=Dosystem(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1]);
         break;
      case ARX_URLENCODE:
         done=Dourlencode(ac,win,(UBYTE *)ac->parameter[0],(UBYTE *)ac->parameter[1]);
         break;
      case ARX_URLFIELD:
         done=Dourlfield(ac,win,ac->parameter[0],(long *)ac->parameter[1],
            (UBYTE *)ac->parameter[2],ac->parameter[3]);
         break;
      case ARX_USEPROXY:
         done=Douseproxy(ac,win,ac->parameter[0],ac->parameter[1]);
         break;
      case ARX_VIEWSOURCE:
         done=Doviewsource(ac,win,(UBYTE *)ac->parameter[0]);
         break;
      case ARX_WAIT:
         done=Dowait(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1],ac->parameter[2],
            ac->parameter[3]);
         break;
      case ARX_WINDOW:
         done=Dowindow(ac,win,(UBYTE *)ac->parameter[0],ac->parameter[1],ac->parameter[2],
            ac->parameter[3],ac->parameter[4],(long *)ac->parameter[5]);
         break;
      case ARX_WINDOWTOBACK:
         done=Dowindowtoback(ac,win);
         break;
      case ARX_WINDOWTOFRONT:
         done=Dowindowtofront(ac,win);
         break;
#ifdef BETAKEYFILE
      case ARX_HTTPDEBUG:
         done=Dohttpdebug(ac,ac->parameter[0]);
         break;
#endif
   }
   return done;
}
