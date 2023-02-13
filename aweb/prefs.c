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

/* prefs.c - AWeb preference task */

#include "aweb.h"
#include "window.h"
#include "cache.h"
#include "application.h"
#include "jslib.h"
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>

#define PCMD_OVERLAP       0x00000001
#define PCMD_LOADIMG       0x00000002
#define PCMD_CACHE         0x00000004
#define PCMD_NEWSCREEN     0x00000008
#define PCMD_NEWLINKPENS   0x00000010
#define PCMD_NEWSAVEPATH   0x00000020
#define PCMD_BLINKRATE     0x00000040
#define PCMD_NEWBUTTONS    0x00000080
#define PCMD_NEWMENUS      0x00000100
#define PCMD_DOCOLORS      0x00000200
#define PCMD_DOBGSOUND     0x00000400
#define PCMD_BROWSER       0x00000800
#define PCMD_SHOWBUTTONS   0x00001000
#define PCMD_NEWMIME       0x00002000
#define PCMD_CONTANIM      0x00004000
#define PCMD_WINDOWBORDER  0x00008000
#define PCMD_SAVEPREFS     0x10000000

struct Fontprefs *fonts;

struct AwebPrefs prefs;
struct SignalSemaphore prefssema;

static UBYTE brprefsname[64],prprefsname[64],uiprefsname[64],nwprefsname[64],ncprefsname[64];
static struct MsgPort *notifyport;
static struct NotifyRequest nfreqbr,nfreqpr,nfrequi,nfreqnw,nfreqnc;

#define NFCODE_BROWSER  0x0001
#define NFCODE_PROGRAM  0x0002
#define NFCODE_NETWORK  0x0004
#define NFCODE_GUI      0x0008
#define NFCODE_COOKIE   0x0010

static struct Jobject *jmimetypes;
static struct Jobject *jplugins;

/*-----------------------------------------------------------------------*/

static void Methodpluginsrefresh(struct Jcontext *jc)
{  /* Everything is automatically refreshed */
}

/* Rebuild the JS mimetypes and plugin array objects */
static void Jmimetypes(struct Jcontext * jc)
{  struct Jvar *jv;
   struct Jobject *jo,*jpo;
   struct Mimeinfo *mi,*mj;
   long mlength=0,plength=0;
   UBYTE buf[64],namebuf[32];
   UBYTE *p,*q,*name;

   if(jc == NULL) return;

   /* First create all MIME types */
   for(mi=prefs.browser.mimelist.first;mi->next;mi=mi->next)
   {
      if(jo=Newjobject(jc))
      {
         if(jv=Jproperty(jc,jo,"description"))
         {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
            if(STREQUAL(mi->subtype,"*")) strcpy(buf,mi->type);
            else if(STRNIEQUAL(mi->subtype,"X-",2)) strcpy(buf,mi->subtype+2);
            else strcpy(buf,mi->subtype);
            for(p=buf;*p;p++) *p=toupper(*p);
            strcat(buf," file");
            Jasgstring(jc,jv,buf);
         }
         if(jv=Jproperty(jc,jo,"enabledPlugin"))
         {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
            Jasgobject(jc,jv,NULL);
         }
         if(jv=Jproperty(jc,jo,"type"))
         {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
            sprintf(buf,"%s/%s",mi->type,mi->subtype);
            for(p=buf;*p;p++) *p=tolower(*p);
            Jasgstring(jc,jv,buf);
         }
         if(jv=Jproperty(jc,jo,"suffixes"))
         {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
            for(p=buf;*p;p++) *p=tolower(*p);
            Jasgstring(jc,jv,mi->extensions);
         }
         if(jmimetypes)
         {  sprintf(buf,"%ld",mlength++);
            if(jv=Jproperty(jc,jmimetypes,buf))
            {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
               Jasgobject(jc,jv,jo);
            }
            sprintf(buf,"%s/%s",mi->type,mi->subtype);
            for(p=buf;*p;p++) *p=tolower(*p);
            if(jv=Jproperty(jc,jmimetypes,buf))
            {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
               Jasgobject(jc,jv,jo);
            }
         }
         Freejobject(jo);
      }
   }
   if(jv=Jproperty(jc,jmimetypes,"length"))
   {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
      Jasgnumber(jc,jv,mlength);
   }
   /* Scan the mime list for plugins. */
   for(mi=prefs.browser.mimelist.first;mi->next;mi=mi->next)
   {  if(mi->driver==MDRIVER_PLUGIN && mi->cmd)
      {  name=mi->cmd+strlen(mi->cmd)-1;
         for(;*name && *name!='/' && *name!=':';name--);
         name++;
         for(p=name,q=namebuf;*p && *p!='.';*q++=*p++);
         *q='\0';
         /* See if we haven't got this plugin already */
         if(jv=Jproperty(jc,jplugins,namebuf))
         {  if(Jtoobject(jc,jv)==NULL)
            {  /* New plugin */
               if(jpo=Newjobject(jc))
               {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
                  Jasgobject(jc,jv,jpo);
                  if(jv=Jproperty(jc,jpo,"description"))
                  {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
                     Jasgstring(jc,jv,name);
                  }
                  if(jv=Jproperty(jc,jpo,"filename"))
                  {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
                     Jasgstring(jc,jv,mi->cmd);
                  }
                  if(jv=Jproperty(jc,jpo,"name"))
                  {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
                     Jasgstring(jc,jv,namebuf);
                  }
                  if(jv=Jproperty(jc,jpo,"length"))
                  {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
                     Jasgnumber(jc,jv,0);
                  }
                  sprintf(buf,"%ld",plength++);
                  if(jv=Jproperty(jc,jplugins,buf))
                  {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
                     Jasgobject(jc,jv,jpo);
                  }

                  /* Scan the mime list and add all MIME types for this plugin,
                   * and add this plugin to these MIME types. */
                  mlength=0;
                  for(mj=mi;mj->next;mj=mj->next)
                  {  if(mj->driver==MDRIVER_PLUGIN && mi->cmd && STRIEQUAL(mi->cmd,mj->cmd))
                     {  sprintf(buf,"%s/%s",mj->type,mj->subtype);
                        for(p=buf;*p;p++) *p=tolower(*p);
                        if(jv=Jproperty(jc,jmimetypes,buf))
                        {  jo=Jtoobject(jc,jv);
                           if(jv=Jproperty(jc,jo,"enabledPlugin"))
                           {  Jasgobject(jc,jv,jpo);
                           }
                           sprintf(buf,"%ld",mlength++);
                           if(jv=Jproperty(jc,jpo,buf))
                           {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
                              Jasgobject(jc,jv,jo);
                           }
                        }
                     }
                  }
                  if(jv=Jproperty(jc,jpo,"length"))
                  {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
                     Jasgnumber(jc,jv,mlength);
                  }
                  Freejobject(jpo);
               }
            }
         }
      }
   }
   if(jv=Jproperty(jc,jplugins,"length"))
   {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
      Jasgnumber(jc,jv,plength);
   }
   Addjfunction(jc,jplugins,"refresh",Methodpluginsrefresh,"reload",NULL);
}

/* Re-build the JS mimetypes and plugins arrays */
static void Rebuildjmime(void)
{  if(jmimetypes && jplugins)
   {  Clearjobject(jmimetypes,NULL);
      Clearjobject(jplugins,NULL);
      Jmimetypes((struct Jcontext *)Agetattr(Aweb(),AOAPP_Jcontext));
   }
}

/*-----------------------------------------------------------------------*/

static void Copyfile(UBYTE *from,UBYTE *to)
{  UBYTE *block;
   long fh1,fh2,l;
   if(block=ALLOCTYPE(UBYTE,INPUTBLOCKSIZE,0))
   {  if(fh1=Open(from,MODE_OLDFILE))
      {  if(ChangeMode(CHANGE_FH,fh1,EXCLUSIVE_LOCK))
         {  if(fh2=Open(to,MODE_NEWFILE))
            {  while((l=Read(fh1,block,INPUTBLOCKSIZE))>0)
               {  Write(fh2,block,l);
               }
               Close(fh2);
            }
         }
         Close(fh1);
      }
      FREE(block);
   }
}


static void Makepatterns(void *listp)
{  LIST(Nocache) *list=listp;
   struct Nocache *nc;
   long len;
   for(nc=list->first;nc->next;nc=nc->next)
   {  if(nc->pattern) FREE(nc->pattern);
      len=2*strlen(nc->name)+4;
      nc->pattern=ALLOCTYPE(UBYTE,len,0);
      if(nc->pattern)
      {  if(ParsePatternNoCase(nc->name,nc->pattern,len)<=0)
         {  FREE(nc->pattern);
            nc->pattern=NULL;
         }
      }
   }
}

static BOOL Openfonts(void)
{  short i,j;
   struct TextAttr ta={0};
   for(j=0;j<2;j++)
   {  for(i=0;i<NRFONTS;i++)
      {  ta.ta_Name=prefs.browser.font[j][i].fontname;
         ta.ta_YSize=prefs.browser.font[j][i].fontsize;
         if(!(prefs.browser.font[j][i].font=OpenDiskFont(&ta)))
         {
            struct TextAttr *tt;
            struct Screen *scr;
            if((scr =(struct Screen *)Agetattr(Aweb(),AOAPP_Screen)))
            {
#if defined(__amigaos4__)
                if(GetScreenAttr(scr,SA_Font, &tt, sizeof(tt)))
                {
#else
                if((tt = scr->Font))
                {
#endif
                    if(!(prefs.browser.font[j][i].font=OpenDiskFont(tt))) return FALSE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
         }
      }
      Setloadreqlevel(j+1,2);
   }
   return TRUE;
}

static void Opennewfont(struct Fontprefs *newf)
{  struct TextAttr ta={0};
   ta.ta_Name=newf->fontname;
   ta.ta_YSize=newf->fontsize;
   newf->font=OpenDiskFont(&ta);
}

static ULONG Comparefontprefsarray(struct Fontprefs *oldfa,struct Fontprefs *newfa,BOOL open)
{  ULONG pcmd=0;
   short i;
   struct Fontprefs *oldf,*newf;
   for(i=0;i<NRFONTS;i++)
   {  oldf=&oldfa[i];
      newf=&newfa[i];
      if(!STREQUAL(oldf->fontname,newf->fontname)
      || oldf->fontsize!=newf->fontsize)
      {  pcmd|=PCMD_BROWSER;
         if(open)
         {  Opennewfont(newf);
            if(!newf->font)
            {  newf->font=oldf->font;  /* keep old one */
               oldf->font=NULL;        /* and prevent closing by Dispose below */
            }
         }
         else
         {  newf->font=NULL;           /* open when required */
         }
      }
      else
      {  oldf->font=NULL;              /* don't close, still in use */
      }
   }
   return pcmd;
}

static void Installmimetypes(void)
{  struct Mimeinfo *mi;
   UBYTE buffer[32];
   UBYTE *p;
   Reinitmime();
   for(mi=prefs.browser.mimelist.first;mi->next;mi=mi->next)
   {  if(*mi->type)
      {  strcpy(buffer,mi->type);
         strcat(buffer,"/");
         if(*mi->subtype) p=mi->subtype;
         else p="*";
         strncat(buffer,p,31-strlen(buffer));
         Addmimetype(buffer,mi->extensions,mi->driver,mi->cmd,mi->args);
      }
   }
}

static ULONG Changedbrowser(void)
{  struct Browserprefs oldp={{{{0}}}};
   short i,j;
   ULONG pcmd=0;
   struct Fontalias *fao,*fan;
   struct Styleprefs *olds,*news;
   struct Mimeinfo *min,*mio;
   Copybrowserprefs(&prefs.browser,&oldp);
   Loadbrowserprefs(&prefs.browser,FALSE,NULL);
   Busypointer(TRUE);
   for(j=0;j<2;j++)
   {  pcmd|=Comparefontprefsarray(oldp.font[j],prefs.browser.font[j],TRUE);
   }
   Busypointer(FALSE);
   for(fan=prefs.browser.aliaslist.first;fan->next;fan=fan->next)
   {  for(fao=oldp.aliaslist.first;fao->next;fao=fao->next)
      {  if(STRIEQUAL(fan->alias,fao->alias))
         {  pcmd|=Comparefontprefsarray(fao->fp,fan->fp,FALSE);
            break;
         }
      }
      if(!fao->next) pcmd|=PCMD_BROWSER;
   }
   /* Also check if no alias was deleted */
   for(fao=oldp.aliaslist.first;fao->next;fao=fao->next)
   {  for(fan=prefs.browser.aliaslist.first;fan->next;fan=fan->next)
      {  if(STRIEQUAL(fan->alias,fao->alias)) break;
      }
      if(!fan->next)
      {  pcmd|=PCMD_BROWSER;
         break;
      }
   }
   for(i=0;i<NRSTYLES;i++)
   {  olds=&oldp.styles[i];
      news=&prefs.browser.styles[i];
      if(olds->fonttype!=news->fonttype
      || olds->fontsize!=news->fontsize
      || olds->style!=news->style)
         pcmd|=PCMD_BROWSER;
   }
   if(prefs.browser.ullink!=oldp.ullink) pcmd|=PCMD_BROWSER;
   if(memcmp(&prefs.browser.newlink,&oldp.newlink,sizeof(prefs.browser.newlink)))
      pcmd|=PCMD_NEWLINKPENS;
   if(memcmp(&prefs.browser.oldlink,&oldp.oldlink,sizeof(prefs.browser.oldlink)))
      pcmd|=PCMD_NEWLINKPENS;
   if(memcmp(&prefs.browser.selectlink,&oldp.selectlink,sizeof(prefs.browser.selectlink)))
      pcmd|=PCMD_NEWLINKPENS;
   if(memcmp(&prefs.browser.background,&oldp.background,sizeof(prefs.browser.background)))
      pcmd|=PCMD_NEWLINKPENS;
   if(memcmp(&prefs.browser.text,&oldp.text,sizeof(prefs.browser.text)))
      pcmd|=PCMD_NEWLINKPENS;
   if(prefs.browser.htmlmode!=oldp.htmlmode) pcmd|=PCMD_BROWSER;
   if(prefs.browser.docolors!=oldp.docolors) pcmd|=PCMD_DOCOLORS;
   if(prefs.browser.dobgsound!=oldp.dobgsound) pcmd|=PCMD_DOBGSOUND;
   if(prefs.browser.blinkrate!=oldp.blinkrate) pcmd|=PCMD_BLINKRATE;
   if(prefs.browser.screenpens!=oldp.screenpens) pcmd|=PCMD_NEWLINKPENS;
   if(prefs.browser.dojs!=oldp.dojs) pcmd|=PCMD_BROWSER;
   if(prefs.browser.doframes!=oldp.doframes) pcmd|=PCMD_BROWSER;
   if(prefs.browser.nominalframe!=oldp.nominalframe) pcmd|=PCMD_BROWSER;
   for(min=prefs.browser.mimelist.first;min->next && !(pcmd&PCMD_NEWMIME);min=min->next)
   {  for(mio=oldp.mimelist.first;mio->next;mio=mio->next)
      {  if(STRIEQUAL(min->type,mio->type) && STRIEQUAL(min->subtype,mio->subtype))
         {  if((min->driver==MDRIVER_PLUGIN || mio->driver==MDRIVER_PLUGIN)
            && (min->driver!=mio->driver))
            {  pcmd|=PCMD_NEWMIME;
            }
            else if(min->driver==MDRIVER_PLUGIN && !STREQUAL(min->cmd,mio->cmd))
            {  pcmd|=PCMD_NEWMIME;
            }
            else if(!STRIEQUAL(min->extensions,mio->extensions))
            {  pcmd|=PCMD_NEWMIME;
            }
            break;
         }
      }
      if(!mio->next) pcmd|=PCMD_NEWMIME;
   }
   Installmimetypes();
   Disposebrowserprefs(&oldp);
   return pcmd;
}

static ULONG Changedprogram(void)
{  struct Programprefs oldp={0};
   ULONG pcmd=0;
   Copyprogramprefs(&prefs.program,&oldp);
   Loadprogramprefs(&prefs.program,FALSE,NULL);
   if(prefs.program.screentype!=oldp.screentype) pcmd|=PCMD_NEWSCREEN;
   if(prefs.program.screentype==SCRTYPE_NAMED)
   {  if(!STREQUAL(prefs.program.screenname,oldp.screenname)) pcmd|=PCMD_NEWSCREEN;
   }
   if(prefs.program.screentype==SCRTYPE_OWN)
   {  if(prefs.program.screenmode!=oldp.screenmode
      || prefs.program.screenwidth!=oldp.screenwidth
      || prefs.program.screenheight!=oldp.screenheight
      || prefs.program.screendepth!=oldp.screendepth
      || prefs.program.loadpalette!=oldp.loadpalette) pcmd|=PCMD_NEWSCREEN;
   }
   if(prefs.program.screentype==SCRTYPE_OWN
   && memcmp(prefs.program.scrdrawpens,oldp.scrdrawpens,sizeof(prefs.program.scrdrawpens)))
      pcmd|=PCMD_NEWSCREEN;
   if(!STRIEQUAL(prefs.program.savepath,oldp.savepath)) pcmd|=PCMD_NEWSAVEPATH;
   if(prefs.program.overlap!=oldp.overlap) pcmd|=PCMD_OVERLAP;
   Disposeprogramprefs(&oldp);
   return pcmd;
}

static ULONG Changedgui(void)
{  struct Guiprefs oldp={0};
   struct Menuentry *me;
   struct Userbutton *ub,*uc;
   struct Popupitem *pi;
   struct Userkey *uk;
   short i;
   ULONG pcmd=0;
   Copyguiprefs(&prefs.gui,&oldp);

   /* Delete all existing definitions, so "no entry" in the config file won't
    * leave the exiting definitions */
   while(me=(struct Menuentry *) REMHEAD(&prefs.gui.menus)) Freemenuentry(me);
   while(uk=(struct Userkey *)   REMHEAD(&prefs.gui.keys)) Freeuserkey(uk);
   while(ub=(struct Userbutton *)REMHEAD(&prefs.gui.buttons)) Freeuserbutton(ub);
   for(i=0;i<NRPOPUPMENUS;i++)
   {  while(pi=(struct Popupitem *)REMHEAD(&prefs.gui.popupmenu[i])) Freepopupitem(pi);
   }
   Copyguiprefs(&defprefs.gui,&prefs.gui);
   Loadguiprefs(&prefs.gui,FALSE,NULL);
   if(prefs.gui.showbuttons!=oldp.showbuttons) pcmd|=PCMD_SHOWBUTTONS;
   if(prefs.gui.shownav!=oldp.shownav) pcmd|=PCMD_SHOWBUTTONS;
   if(prefs.gui.buttonspos != oldp.buttonspos) pcmd|=PCMD_SHOWBUTTONS;
   if(prefs.gui.textbuttons != oldp.textbuttons) pcmd |= PCMD_SHOWBUTTONS;
   if(prefs.gui.nobevel != oldp.nobevel) pcmd |= PCMD_SHOWBUTTONS;
   if(prefs.gui.windowborder != oldp.windowborder) pcmd |= PCMD_WINDOWBORDER;
#if 0
   for(me=oldp.menus.first,mf=prefs.menus.first;
      me->next && mf->next;me=me->next,mf=mf->next)
   {  if(me->type!=mf->type
      || !STREQUAL(me->title,mf->title)
      || me->scut[0]!=mf->scut[0]) break;
   }
   if(me->next || mf->next) pcmd|=PCMD_NEWMENUS;   /* old and new menus are different */
#endif
   pcmd|=PCMD_NEWMENUS; /* Always rebuild the menus and the menunums */
   for(ub=oldp.buttons.first,uc=prefs.gui.buttons.first;
      ub->next && uc->next;ub=ub->next,uc=uc->next)
   {  if(!STREQUAL(ub->label,uc->label)) break;
   }
   if(ub->next || uc->next) pcmd|=PCMD_NEWBUTTONS; /* old and new labels are different */
   Disposeguiprefs(&oldp);
   return pcmd;
}

static ULONG Changednetwork( UWORD netchanged )
{  struct Networkprefs oldp={0};
   ULONG pcmd=0;
   Copynetworkprefs(&prefs.network,&oldp);
   if(netchanged)
   {
       /* network has changed load everything */
       Disposenetworkprefs(&prefs.network);
       Loadnetworkprefs(&prefs.network,FALSE,NULL);

   }
   else
   {
       /* only nocookies changed only do cookies */

       Disposecookieprefs(&prefs.network);
       Loadcookieprefs(&prefs.network,FALSE,NULL);


   }
   if(prefs.network.loadimg!=oldp.loadimg) pcmd|=PCMD_LOADIMG;
   if(prefs.network.camemsize<oldp.camemsize) pcmd|=PCMD_CACHE;
   if(prefs.network.cadisksize<oldp.cadisksize) pcmd|=PCMD_CACHE;
   if(prefs.network.minfreechip<oldp.minfreechip) pcmd|=PCMD_CACHE;
   if(prefs.network.minfreefast<oldp.minfreefast) pcmd|=PCMD_CACHE;
   if(prefs.network.contanim!=oldp.contanim) pcmd|=PCMD_CONTANIM;
   if(prefs.network.restrictimages!=oldp.restrictimages) pcmd|=PCMD_LOADIMG;
   Makepatterns(&prefs.network.nocookie);
   Makepatterns(&prefs.network.noproxy);
   Makepatterns(&prefs.network.nocache);
   Disposenetworkprefs(&oldp);
   return pcmd;
}

static void Processprefs(void)
{  struct NotifyMessage *msg;
   UWORD changed=0;
   ULONG pcmd=0;
   ObtainSemaphore(&prefssema);
   while(msg=(struct NotifyMessage *)GetMsg(notifyport))
   {  changed|=msg->nm_NReq->nr_UserData;
      ReplyMsg((struct Message *) msg);
   }
   if(changed&NFCODE_BROWSER)
   {  pcmd|=Changedbrowser();
   }
   if(changed&NFCODE_PROGRAM)
   {  pcmd|=Changedprogram();
   }
   if(changed&NFCODE_GUI)
   {  pcmd|=Changedgui();
   }
   if(changed&(NFCODE_NETWORK|NFCODE_COOKIE))
   {
       pcmd|=Changednetwork(NFCODE_NETWORK & changed);
   }
   ReleaseSemaphore(&prefssema);
   if(pcmd&PCMD_NEWMIME)
   {  Rebuildjmime();
   }
   if(pcmd&PCMD_NEWSCREEN)
   {  Asetattrs(Aweb(),AOAPP_Newprefs,PREFSF_SCREEN,TAG_END);
      pcmd&=~(PCMD_NEWLINKPENS|PCMD_SHOWBUTTONS|PCMD_NEWBUTTONS);
      pcmd|=PCMD_LOADIMG;
   }
   if(pcmd&PCMD_WINDOWBORDER)
   {
      Asetattrs(Aweb(), AOAPP_Newprefs, PREFSF_WINDOWBORDER, TAG_END);
      pcmd&=~(PCMD_SHOWBUTTONS);
      pcmd&=~(PCMD_NEWBUTTONS);
   }
   if(pcmd&PCMD_SHOWBUTTONS)
   {  Asetattrs(Aweb(),AOAPP_Newprefs,PREFSF_SHOWBUTTONS,TAG_END);
      pcmd&=~(PCMD_NEWBUTTONS);
   }
   if(pcmd&PCMD_NEWBUTTONS)
   {  Asetattrs(Aweb(),AOAPP_Newprefs,PREFSF_BUTTONS,TAG_END);
   }
   if(pcmd&PCMD_BROWSER)
   {  Asetattrs(Aweb(),AOAPP_Browsersettings,TRUE,TAG_END);
   }
   if(pcmd&PCMD_NEWLINKPENS)
   {  Asetattrs(Aweb(),AOAPP_Browserpens,TRUE,TAG_END);
   }
   if(pcmd&PCMD_BLINKRATE)
   {  Asetattrs(Aweb(),AOAPP_Blink,TRUE,TAG_END);
   }
   if(pcmd&PCMD_NEWMENUS) Asetattrs(Aweb(),AOAPP_Menus,0,TAG_END);
   if(pcmd&PCMD_NEWSAVEPATH) Asetattrs(Aweb(),AOAPP_Savepath,0,TAG_END);
   if(pcmd&PCMD_CACHE) Flushcache(CACFT_EXCESS);
   if(pcmd&PCMD_LOADIMG) Setloadimg();
   if(pcmd&PCMD_DOCOLORS) Setdocolors();
   if(pcmd&PCMD_OVERLAP) Asetattrs(Aweb(),AOAPP_Overlapsetting,TRUE,TAG_END);
   if(pcmd&PCMD_CONTANIM) Asetattrs(Aweb(),AOAPP_Animgadsetting,TRUE,TAG_END);
   if(pcmd&PCMD_DOBGSOUND) Setdobgsound();
}

/*-----------------------------------------------------------------------*/

BOOL Initprefs(void)
{  UBYTE nfname[64]="ENV:" DEFAULTCFG;
   InitSemaphore(&prefssema);
   Copybrowserprefs(&defprefs.browser,&prefs.browser);
   Copyprogramprefs(&defprefs.program,&prefs.program);
   Copyguiprefs(&defprefs.gui,&prefs.gui);
   Copynetworkprefs(&defprefs.network,&prefs.network);
   Copywindowprefs(&defprefs.window,&prefs.window);
   Loadbrowserprefs(&prefs.browser,FALSE,NULL);
   Loadprogramprefs(&prefs.program,FALSE,NULL);
   Loadguiprefs(&prefs.gui,FALSE,NULL);
   Loadnetworkprefs(&prefs.network,FALSE,NULL);
   Loadwindowprefs(&prefs.window,TRUE,NULL);
   Installmimetypes();
   Makepatterns(&prefs.network.nocookie);
   Makepatterns(&prefs.network.noproxy);
   Makepatterns(&prefs.network.nocache);
   if(!(notifyport=ACreatemsgport())) return FALSE;
   if(*configname) AddPart(nfname,configname,64);
   sprintf(brprefsname,"%s/browser",nfname);
   nfreqbr.nr_Name=brprefsname;
   nfreqbr.nr_stuff.nr_Msg.nr_Port=notifyport;
   nfreqbr.nr_Flags=NRF_SEND_MESSAGE;
   nfreqbr.nr_UserData=NFCODE_BROWSER;
   StartNotify(&nfreqbr);
   sprintf(prprefsname,"%s/program",nfname);
   nfreqpr.nr_Name=prprefsname;
   nfreqpr.nr_stuff.nr_Msg.nr_Port=notifyport;
   nfreqpr.nr_Flags=NRF_SEND_MESSAGE;
   nfreqpr.nr_UserData=NFCODE_PROGRAM;
   StartNotify(&nfreqpr);
   sprintf(uiprefsname,"%s/gui",nfname);
   nfrequi.nr_Name=uiprefsname;
   nfrequi.nr_stuff.nr_Msg.nr_Port=notifyport;
   nfrequi.nr_Flags=NRF_SEND_MESSAGE;
   nfrequi.nr_UserData=NFCODE_GUI;
   StartNotify(&nfrequi);
   sprintf(nwprefsname,"%s/network",nfname);
   nfreqnw.nr_Name=nwprefsname;
   nfreqnw.nr_stuff.nr_Msg.nr_Port=notifyport;
   nfreqnw.nr_Flags=NRF_SEND_MESSAGE;
   nfreqnw.nr_UserData=NFCODE_NETWORK;
   StartNotify(&nfreqnw);
   sprintf(ncprefsname,"%s/nocookie",nfname);
   nfreqnc.nr_Name=ncprefsname;
   nfreqnc.nr_stuff.nr_Msg.nr_Port=notifyport;
   nfreqnc.nr_Flags=NRF_SEND_MESSAGE;
   nfreqnc.nr_UserData=NFCODE_COOKIE;
   StartNotify(&nfreqnc);
   Setprocessfun(notifyport->mp_SigBit,Processprefs);
   return TRUE;
}

BOOL Initprefs2(void)
{  if(!Openfonts()) return FALSE;
   return TRUE;
}

void Freeprefs(void)
{  if(nfreqbr.nr_Name) EndNotify(&nfreqbr);
   if(nfreqpr.nr_Name) EndNotify(&nfreqpr);
   if(nfrequi.nr_Name) EndNotify(&nfrequi);
   if(nfreqnw.nr_Name) EndNotify(&nfreqnw);
   if(nfreqnc.nr_Name) EndNotify(&nfreqnc);
   if(notifyport)
   {  Setprocessfun(notifyport->mp_SigBit,NULL);
      ADeletemsgport(notifyport);
   }
   if(jmimetypes) Freejobject(jmimetypes);
   if(jplugins) Freejobject(jplugins);
   Disposebrowserprefs(&prefs.browser);
   Disposeprogramprefs(&prefs.program);
   Disposeguiprefs(&prefs.gui);
   Disposenetworkprefs(&prefs.network);
}

BOOL Setprefsname(UBYTE *name)
{  strncpy(configname,name,31);
   return TRUE;
}

void Startsettings(UWORD type)
{  struct Cfgmsg msg={{{0}}};
   struct MsgPort *port,*cfgport;
   UBYTE *p,*q;
   if(port=ACreatemsgport())
   {  Forbid();
      if(cfgport=FindPort(AWEBCFGPORTNAME))
      {  msg.msg.mn_ReplyPort=port;
         msg.cfgclass=type;
         PutMsg(cfgport,(struct Message *) &msg);
         Permit();
         WaitPort(port);
      }
      else
      {  Permit();
         switch(type)
         {  case CFGCLASS_BROWSER:p="BROWSER CONFIG %c PUBSCREEN %n";break;
            case CFGCLASS_PROGRAM:p="PROGRAM CONFIG %c PUBSCREEN %n";break;
            case CFGCLASS_GUI:p="GUI CONFIG %c PUBSCREEN %n";break;
            case CFGCLASS_NETWORK:p="NETWORK CONFIG %c PUBSCREEN %n";break;
            default:p=NULL;
         }
         if(p)
         {  if(q=Fullname("AWebPath:AWebCfg"))
            {  Spawn(FALSE,q,p,"cn",configname,Agetattr(Aweb(),AOAPP_Screenname));
               FREE(q);
            }
         }
      }
      ADeletemsgport(port);
   }
}

void Closesettings(void)
{  struct Cfgmsg msg={{{0}}};
   struct MsgPort *port,*cfgport;
   if(port=ACreatemsgport())
   {  Forbid();
      if(cfgport=FindPort(AWEBCFGPORTNAME))
      {  msg.msg.mn_ReplyPort=port;
         msg.cfgclass=CFGCLASS_QUIT;
         PutMsg(cfgport,(struct Message *) &msg);
         Permit();
         WaitPort(port);
      }
      else Permit();
      ADeletemsgport(port);
   }
}

void Snapshotwindows(void *window)
{  struct IBox *box=NULL,*zoombox=NULL;
   ObtainSemaphore(&prefssema);
   Agetattrs(window,
      AOWIN_Box,(ULONG) &box,
      AOWIN_Zoombox,(ULONG) &zoombox,
      TAG_END);
   if(box)
   {  prefs.window.winx=box->Left;
      prefs.window.winy=box->Top;
      prefs.window.winw=box->Width;
      prefs.window.winh=box->Height;
   }
   if(zoombox)
   {  prefs.window.wiax=zoombox->Left;
      prefs.window.wiay=zoombox->Top;
      prefs.window.wiaw=zoombox->Width;
      prefs.window.wiah=zoombox->Height;
   }
   Getnetstatdim(&prefs.window.nwsx,&prefs.window.nwsy,&prefs.window.nwsw,&prefs.window.nwsh);
   Getinfodim(&prefs.window.infx,&prefs.window.infy,&prefs.window.infw,&prefs.window.infh);
   Savewindowprefs(&prefs.window,TRUE,NULL);
   ReleaseSemaphore(&prefssema);
}

void Prefsloadimg(long loadimg)
{  ObtainSemaphore(&prefssema);
   if(loadimg!=prefs.network.loadimg)
   {  prefs.network.loadimg=loadimg;
      Savenetworkprefs(&prefs.network,FALSE,NULL);
      Setloadimg();
   }
   ReleaseSemaphore(&prefssema);
}

void Prefsdocolors(BOOL docolors)
{  ObtainSemaphore(&prefssema);
   if(docolors!=prefs.browser.docolors)
   {  prefs.browser.docolors=docolors;
      Savebrowserprefs(&prefs.browser,FALSE,NULL);
      Setdocolors();
   }
   ReleaseSemaphore(&prefssema);
}

void Prefsdobgsound(BOOL dobgsound)
{  ObtainSemaphore(&prefssema);
   if(dobgsound!=prefs.browser.dobgsound)
   {  prefs.browser.dobgsound=dobgsound;
      Savebrowserprefs(&prefs.browser,FALSE,NULL);
      Setdobgsound();
   }
   ReleaseSemaphore(&prefssema);
}

void Saveallsettings(void)
{  ObtainSemaphore(&prefssema);
   Savebrowserprefs(&prefs.browser,TRUE,NULL);
   Saveprogramprefs(&prefs.program,TRUE,NULL);
   Saveguiprefs(&prefs.gui,TRUE,NULL);
   Savenetworkprefs(&prefs.network,TRUE,NULL);
   Savenocookieprefs(&prefs.network,TRUE,NULL);
   ReleaseSemaphore(&prefssema);
}

void Savesettingsas(UBYTE *path)
{  UBYTE *name;
   ObtainSemaphore(&prefssema);
   if(name=ALLOCTYPE(UBYTE,STRINGBUFSIZE,MEMF_CLEAR))
   {  strncpy(name,path,STRINGBUFSIZE-1);
      if(AddPart(name,"browser",STRINGBUFSIZE-1))
      {  Savebrowserprefs(&prefs.browser,TRUE,name);
      }
      strncpy(name,path,STRINGBUFSIZE-1);
      if(AddPart(name,"program",STRINGBUFSIZE-1))
      {  Saveprogramprefs(&prefs.program,TRUE,name);
      }
      strncpy(name,path,STRINGBUFSIZE-1);
      if(AddPart(name,"gui",STRINGBUFSIZE-1))
      {  Saveguiprefs(&prefs.gui,TRUE,name);
      }
      strncpy(name,path,STRINGBUFSIZE-1);
      if(AddPart(name,"network",STRINGBUFSIZE-1))
      {  Savenetworkprefs(&prefs.network,TRUE,name);
      }
      strncpy(name,path,STRINGBUFSIZE-1);
      if(AddPart(name,"nocookie",STRINGBUFSIZE-1))
      {  Savenocookieprefs(&prefs.network,TRUE,name);
      }
      FREE(name);
   }
   ReleaseSemaphore(&prefssema);
}

void Loadsettings(UBYTE *path)
{  UBYTE *name1,*name2;
   UBYTE *config=(UBYTE *)Agetattr(Aweb(),AOAPP_Configname);
   UBYTE *file[]={ "browser","program","gui","network" };
   short i;
   if(name1=ALLOCTYPE(UBYTE,2*STRINGBUFSIZE,MEMF_CLEAR))
   {  name2=name1+STRINGBUFSIZE;
      for(i=0;i<4;i++)
      {  strncpy(name1,path,STRINGBUFSIZE-1);
         if(!AddPart(name1,file[i],STRINGBUFSIZE-1)) break;
         strcpy(name2,"ENV:" DEFAULTCFG);
         if(!AddPart(name2,config,STRINGBUFSIZE-1)) break;
         if(!AddPart(name2,file[i],STRINGBUFSIZE-1)) break;
         Copyfile(name1,name2);
      }
      FREE(name1);
   }
}

/* Add name to network nocookie list and save */
void Addtonocookie(UBYTE *name)
{  struct Nocache *nc;
   BOOL found=FALSE;
   long len;
   ObtainSemaphore(&prefssema);
   for(nc=prefs.network.nocookie.first;nc->next;nc=nc->next)
   {  if(STRIEQUAL(nc->name,name))
      {  found=TRUE;
         break;
      }
   }
   if(!found)
   {  nc=Addnocookie((APTR) &prefs.network.nocookie,name);
      len=2*strlen(nc->name)+4;
      nc->pattern=ALLOCTYPE(UBYTE,len,0);
      if(nc->pattern)
      {  if(ParsePatternNoCase(nc->name,nc->pattern,len)<=0)
         {  FREE(nc->pattern);
            nc->pattern=NULL;
         }
      }
      Savenocookieprefs(&prefs.network,FALSE,NULL);
      Savenocookieprefs(&prefs.network,TRUE,NULL);
   }
   ReleaseSemaphore(&prefssema);
}

void Jsetupprefs(struct Jcontext *jc,struct Jobject *jnav)
{  struct Jvar *jv;
   ObtainSemaphore(&prefssema);
   if(!jmimetypes)
   {  jmimetypes=Newjobject(jc);
      jplugins=Newjobject(jc);
      Jmimetypes(jc);
      if(jmimetypes)
      {  if(jv=Jproperty(jc,jnav,"mimeTypes"))
         {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
            Jasgobject(jc,jv,jmimetypes);
         }
      }
      if(jplugins)
      {  if(jv=Jproperty(jc,jnav,"plugins"))
         {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
            Jasgobject(jc,jv,jplugins);
         }
      }
   }
   ReleaseSemaphore(&prefssema);
}

/*-----------------------------------------------------------------------*/

struct Namebreak
{  UBYTE buffer[128];
   UBYTE *list;
   UBYTE *current;
};


/* Copy next name to buffer. Return FALSE when done */
static BOOL Nextname(struct Namebreak *nb)
{  UBYTE *p,*q;
   if(!nb->current || !*nb->current) return FALSE;
   p=nb->current;
   while(isspace(*p)) p++;
   if(!*p) return FALSE;
   q=p;
   while(*q && *q!=',' && *q!=';') q++;
   nb->current=*q?(q+1):q;
   if(q-p>127) q=p+127;
   strncpy(nb->buffer,p,q-p);
   q=nb->buffer+(q-p)-1;
   while(q>=nb->buffer && isspace(*q)) q--;
   q[1]='\0';
   return TRUE;
}

struct Fontprefs *Matchfont(UBYTE *face,short size,BOOL fixed)
{  struct Fontalias *fa;
   struct Namebreak nbf,nba;
   nbf.list=nbf.current=face;
   while(Nextname(&nbf))
   {  for(fa=prefs.browser.aliaslist.first;fa->next;fa=fa->next)
      {  nba.list=nba.current=fa->alias;
         while(Nextname(&nba))
         {  if(STRIEQUAL(nbf.buffer,nba.buffer))
            {  if(!fa->fp[size].font)
               {  Opennewfont(&fa->fp[size]);
               }
               if(fa->fp[size].font
               && !(fixed && (fa->fp[size].font->tf_Flags&FPF_PROPORTIONAL)))
               {  return &fa->fp[size];
               }
            }
         }
      }
   }
   return &prefs.browser.font[fixed?1:0][size];
}
