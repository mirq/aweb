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

/* awebprefs.h - AWeb preference definition */

/* this header file contains all definitions common to AWeb and AWebCfg
   regarding prefs */

#ifndef AWEBPREFS_H
#define AWEBPREFS_H

enum DOJS_TYPES
{  DOJS_OFF,DOJS_11,DOJS_ALL
};

#define NRFONTS   7

struct Fontprefs           /* font info */
{  UBYTE *fontname;        /* font name */
   short fontsize;         /* font size */
   struct TextFont *font;  /* open textfont */
};

struct Fontalias           /* font alias */
{  NODE(Fontalias);
   UBYTE *alias;           /* alias font names, comma separated */
   struct Fontprefs fp[NRFONTS]; /* font info for each size for this alias */
};

struct Styleprefs          /* style info */
{  short fonttype;         /* 0=normal or 1=fixed */
   short fontsize;         /* abs or rel size */
   BOOL relsize;           /* fontsize is relative to fontbase */
   ULONG style;            /* font style */
};

enum STYLE_TYPES
{  STYLE_NORMAL,STYLE_H1,STYLE_H2,STYLE_H3,STYLE_H4,STYLE_H5,STYLE_H6,
   STYLE_BIG,STYLE_SMALL,STYLE_SUB,STYLE_SUP,
   STYLE_ADDRESS,STYLE_BLOCKQUOTE,STYLE_CITE,STYLE_CODE,STYLE_DFN,STYLE_EM,
   STYLE_KBD,STYLE_PRE,STYLE_SAMP,STYLE_STRONG,STYLE_VAR,
   NRSTYLES,   /* must be last */
};

/* All strings in Mimeinfo are non-null. Missing fields are represented
 * by empty strings. */
struct Mimeinfo            /* prefs window - mime database interface */
{  NODE(Mimeinfo);
   UBYTE *type;
   UBYTE *subtype;
   UBYTE *extensions;
   UWORD driver;          /* Driver type, see below */
   UBYTE *cmd;
   UBYTE *args;
   BOOL deleteable;
};

#define MDRIVER_NONE       0  /* No driver, try wildcard definition */
#define MDRIVER_INTERNAL   1  /* Internal driver */
#define MDRIVER_PLUGIN     2  /* External driver */
#define MDRIVER_EXTERNAL   3  /* External program */
#define MDRIVER_EXTPIPE    4  /* External program with pipe */
#define MDRIVER_NOFETCH    5  /* External program no */
#define MDRIVER_SAVELOCAL  6  /* Save to local disk */
#define MDRIVER_CANCEL     7  /* Cancel and discard */

struct Colorprefs          /* rgb values for some color */
{  ULONG red,green,blue;   /* rgb */
};

struct Menuentry           /* entry in menu */
{  NODE(Menuentry);
   UWORD type;            /* see below */
   UBYTE *title;           /* menu text */
   UBYTE *cmd;             /* Special code or ARexx command to execute */
   UBYTE scut[2];          /* shortcut string */
   UWORD menunum;         /* menu num (filled at runtime) */
};

#define AMENU_MENU      0  /* menu */
#define AMENU_ITEM      1  /* menu item */
#define AMENU_SUB       2  /* menu subitem */
#define AMENU_SEPARATOR 3  /* menu item separator */

struct Userbutton          /* user defined button */
{  NODE(Userbutton);
   UBYTE *label;           /* button label */
   UBYTE *cmd;             /* ARexx command to execute */
};

struct Popupitem           /* popup menu item */
{  NODE(Popupitem);
   UWORD flags;           /* in memory or not, or both, see below */
   UBYTE *title;           /* entry title */
   UBYTE *cmd;             /* ARexx command to execute */
};

#define PUPF_INMEM      0x0001   /* to appear when object is in memory */
#define PUPF_NOTINMEM   0x0002   /* to appear when object is not in memory */

enum POPUPMENU_TYPES
{  PUPT_IMAGE,PUPT_LINK,PUPT_FRAME,
   NRPOPUPMENUS
};

struct Userkey             /* User defined key */
{  NODE(Userkey);
   UWORD key;             /* See below */
   UBYTE *cmd;             /* ARexx command to execute */
};

/* Userkey key composition */
#define UKEY_SHIFT   0x0400   /* Key is shifted */
#define UKEY_ALT     0x0200   /* Key is Alt'ed */
#define UKEY_ASCII   0x0100   /* Mask nybble is mapped ASCII, else rawkey */
#define UKEY_MASK    0x00ff   /* Key id, mapped ASCII or rawkey */

struct Navbutton           /* Navigation button */
{  UBYTE *cmd;             /* ARexx command to execute */
};
#define NRNAVBUTTONS    10

struct Nocache             /* entry in no-cache-for list */
{  NODE(Nocache);
   UBYTE *name;            /* name or pattern */
   UBYTE *pattern;         /* parsed DOS pattern */
};

/* entry in no-proxy list */
#define Noproxy Nocache

/* entry in no-cookie list */
#define Nocookie Nocache

struct Browserprefs
{  /* fonts page */
   struct Fontprefs font[2][NRFONTS];  /* [0]=normal, [1]=fixed */
   LIST(Fontalias) aliaslist;
   /* style page */
   struct Styleprefs styles[NRSTYLES];
   /* colours page */
   struct Colorprefs newlink;
   struct Colorprefs oldlink;
   struct Colorprefs selectlink;
   struct Colorprefs background;
   struct Colorprefs text;
   BOOL screenpens;
   /* options page */
   short htmlmode;
   BOOL ullink;            /* underline links? */
   BOOL docolors;
   BOOL dobgsound;
   short blinkrate;        /* in 0.1 s */
   short dojs;
   UBYTE *charset;         /* Defaul character set */
   BOOL doframes;
   BOOL inctable;
   BOOL nominalframe;
   BOOL jserrors;
   BOOL nobanners;
   BOOL tooltips;
   BOOL handpointer;
   BOOL jswatch;
   BOOL temprp;
   LIST(Mimeinfo) mimelist;
   BOOL imgborder;         /* draw border around images? */
};

struct Programprefs
{  /* options page */
   UBYTE *savepath;
   UBYTE *temppath;
   long overlap;           /* pixels overlap */
   BOOL commands;
   BOOL hlautoclose;
   BOOL hlrequester;
   BOOL hlsingleclick;
   BOOL whautoclose;
   BOOL saveicons;
   BOOL clipdrag;
   BOOL aahotlist;         /* auto open */
   BOOL aawinhis;
   BOOL aanetstat;
   BOOL centerreq;
   /* ext programs page */
   UBYTE *editcmd,*editargs;
   UBYTE *viewcmd,*viewargs;
   UBYTE *imgvcmd,*imgvargs;
   UBYTE *console;
   UBYTE *startupscript;
   UBYTE *shutdownscript;
   /* screen page */
   short screentype;       /* see below */
   UBYTE *screenname;      /* for SCRTYPE_NAMED */
   ULONG screenmode;
   short screendepth;
   short screenwidth,screenheight;
   short loadpalette;      /* see below */
   /* palette page */
   ULONG scrpalette[24];   /* 8 rgb values */
   WORD scrdrawpens[13];   /* 12 drawinfo pens + (-1) */
   BOOL fourcolors;        /* use 4 instead of 8 colours */
};

struct Guiprefs
{  /* options page */
   UWORD popupkey;        /* IEQUALIFIER mask */
   BOOL showbuttons;
   BOOL shownav;
   WORD buttonspos;         /* alignment of navigation buttons.*/
   WORD textbuttons;        /* whether buttons have text labels and or images*/
   BOOL nobevel;            /* buttons are borderless */
   UWORD windowborder;      /* do AWeb windows have borders and system gadgets */ /* 0 normal 1 gadgetsless 2 borderless */
   /* menu page */
   LIST(Menuentry) menus;
   /* button page */
   LIST(Userbutton) buttons;
   /* popup page */
   LIST(Popupitem) popupmenu[NRPOPUPMENUS];
   /* keys page */
   LIST(Userkey) keys;
   /* nav page */
   struct Navbutton navs[NRNAVBUTTONS];
};

struct Networkprefs
{  /* general page */
   UWORD loadimg;         /* see below */
   long maxconnect;
   long maxdiskread;
   UBYTE *homeurl;
   UBYTE *localindex;
   UBYTE *searchurl;
   UBYTE *spoofid;
   BOOL ignoremime;
   BOOL starthomepage;
   BOOL autosearch;
   BOOL contanim;
   BOOL restrictimages;
   /* ext programs page */
   UBYTE *telnetcmd,*telnetargs;
   UBYTE *starttcpcmd,*starttcpargs;
   UBYTE *endtcpcmd,*endtcpargs;
   /* proxies page */
   UBYTE *httpproxy;
   UBYTE *ftpproxy;
   UBYTE *gopherproxy;
   UBYTE *telnetproxy;
   BOOL limitproxy;
   BOOL passiveftp;
   LIST(Noproxy) noproxy;
   /* cache page */
   UBYTE *cachepath;       /* cache's temp path */
   long camemsize;
   long cadisksize;
   long minfreechip,minfreefast;
   short caverify_dyn;
   short caverify_stat;
   BOOL fastresponse_dyn;
   BOOL fastresponse_stat;
   LIST(Nocache) nocache;
   /* privacy page */
   BOOL referer;
   BOOL formwarn;
   BOOL spamblock;
   BOOL ftpemailaddr;
   short cookies;
   BOOL rfc2109;
   LIST(Nocookie) nocookie;
   /* Mail page */
   UBYTE *emailaddr;
   UBYTE *replyaddr;
   UBYTE *fullname;
   UBYTE *organization;
   UBYTE *smtphost;
   UBYTE *nntphost;
   UBYTE *sigfile;
   UBYTE *newsauthuser;
   UBYTE *newsauthpass;
   UBYTE *mailquotehdr;
   UBYTE *newsquotehdr;
   BOOL extmailer;
   UBYTE *mailtocmd,*mailtoargs;
   BOOL extnewsreader;
   UBYTE *newscmd,*newsargs;
   long maxartnews;        /* Max # of articles to fetch in 1 go */
   BOOL framednews;
   BOOL sortednews;
   BOOL longhdrnews;
   BOOL propnews;          /* Proportional font */
   BOOL newsbynum;

/* developer kludge */
BOOL cachelocalhost;
/* ---------------- */

};

struct Windowprefs
{  short winx,winy,winw,winh;
   short wiax,wiay,wiaw,wiah;    /* alternate size */
   short nwsx,nwsy,nwsw,nwsh;
   short hotx,hoty,hotw,hoth;
   short homx,homy,homw,homh;    /* hotlist manager */
   short whisx,whisy,whisw,whish;
   short cabrx,cabry,cabrw,cabrh;
   short autx,auty,autw,auth;
   short infx,infy,infw,infh;
   short openx,openy,openw,openh;/* file requesters */
   short savex,savey,savew,saveh;
};

struct Settingsprefs       /* only in awebcfg program */
{  short brwx,brwy,brww,brwh;
   short prgx,prgy,prgw,prgh;
   short guix,guiy,guiw,guih;
   short netx,nety,netw,neth;
};

struct AwebPrefs               /* all preferences bundled */
{  struct Browserprefs browser;
   struct Programprefs program;
   struct Guiprefs gui;
   struct Networkprefs network;
   struct Windowprefs window;
};

#define LOADIMG_OFF     0  /* don't load images */
#define LOADIMG_MAPS    1  /* load only maps */
#define LOADIMG_ALL     2  /* load all */

#define SCRTYPE_DEFAULT 0  /* open on default pub screen */
#define SCRTYPE_NAMED   1  /* open on named pub screen */
#define SCRTYPE_OWN     2  /* open on own screen */

#define LOADPAL_OFF     0  /* don't load palette */
#define LOADPAL_COLORS  1  /* load color palette */
#define LOADPAL_GRAY    2  /* load grayscale palette */

#define HTML_STRICT     0
#define HTML_TOLERANT   1
#define HTML_COMPATIBLE 2

#define CAVERIFY_ALWAYS  0
#define CAVERIFY_ONCE    1
#define CAVERIFY_NEVER   2


#define COOKIES_NEVER   0  /* never store or send cookies */
#define COOKIES_ASK     1  /* send cookies, but store only after confirmation */
#define COOKIES_QUIET   2  /* send and store cookies */

extern UBYTE configname[32];

/* AWebCfg command port name */
#define AWEBCFGPORTNAME "AWebCfg"

struct Cfgmsg              /* Message format for AWebCfg port */
{  struct Message msg;
   UWORD cfgclass;        /* see below */
};

#define CFGCLASS_BROWSER   0x0001   /* open browser window */
#define CFGCLASS_PROGRAM   0x0002   /* open program window */
#define CFGCLASS_NETWORK   0x0004   /* open network window */
#define CFGCLASS_QUIT      0x0008   /* quit */
#define CFGCLASS_SNAPSHOT  0x0010   /* save snapshot */
#define CFGCLASS_CLASSACT  0x0020   /* open classact settings */
#define CFGCLASS_GUI       0x0040   /* open GUI window */


/* the default preferences */
extern struct AwebPrefs defprefs;

/* prototypes for defprefs.c */

extern BOOL Initdefprefs(void);
extern void Freedefprefs(void);

extern void Freemenuentry(struct Menuentry *me);
extern struct Menuentry *Addmenuentry(void *list,UWORD type,UBYTE *title,UBYTE scut,UBYTE *cmd);
extern void Freeuserbutton(struct Userbutton *ub);
extern struct Userbutton *Adduserbutton(void *list,UBYTE *label,UBYTE *cmd);
extern void Freepopupitem(struct Popupitem *pi);
extern struct Popupitem *Addpopupitem(void *list,UWORD flags,UBYTE *title,UBYTE *cmd);
extern void Freeuserkey(struct Userkey *uk);
extern struct Userkey *Adduserkey(void *list,UWORD key,UBYTE *cmd);
extern struct Userkey *Finduserkey(void *list,UWORD key);
extern void Freefontalias(struct Fontalias *fa);

struct FontaliasList
{
    struct Fontalias *first;
    struct Fontalias *tail;
    struct Fontalias *last;
};

extern struct Fontalias *Addfontalias(struct FontaliasList *list,UBYTE *alias);
//extern struct Fontalias *Addfontalias(LIST(Fontalias) *list,UBYTE *alias);

struct MimeinfoList
{
    struct Mimeinfo *first;
    struct Mimeinfo *tail;
    struct Mimeinfo *last;
};

extern void Freemimeinfo(struct Mimeinfo *mi);
extern struct Mimeinfo *Addmimeinfo(struct MimeinfoList *list,UBYTE *type,UBYTE *subtype,
   UBYTE *extensions,UWORD driver,UBYTE *cmd,UBYTE *args);

struct NocacheList
{
    struct Nocache *first;
    struct Nocache *tail;
    struct Nocache *last;
};

extern void Freenocache(struct Nocache *nc);
extern struct Nocache *Addnocache(struct NocacheList *list,UBYTE *name);

#define Freenoproxy(nc) Freenocache(nc)
#define Addnoproxy(l,n) Addnocache(l,n)

#define Freenocookie(nc) Freenocache(nc)
#define Addnocookie(l,n) Addnocache(l,n)

extern void Disposebrowserprefs(struct Browserprefs *p);
extern void Disposeprogramprefs(struct Programprefs *p);
extern void Disposeguiprefs(struct Guiprefs *p);
extern void Disposenetworkprefs(struct Networkprefs *p);
extern void Disposecookieprefs(struct Networkprefs *p);
extern BOOL Copybrowserprefs(struct Browserprefs *from,struct Browserprefs *to);
extern BOOL Copyprogramprefs(struct Programprefs *from,struct Programprefs *to);
extern BOOL Copyguiprefs(struct Guiprefs *from,struct Guiprefs *to);
extern BOOL Copynetworkprefs(struct Networkprefs *from,struct Networkprefs *to);
extern BOOL Copywindowprefs(struct Windowprefs *from,struct Windowprefs *to);
extern void Loadbrowserprefs(struct Browserprefs *bp,BOOL saved,UBYTE *name);
extern void Loadprogramprefs(struct Programprefs *pp,BOOL saved,UBYTE *name);
extern void Loadguiprefs(struct Guiprefs *pp,BOOL saved,UBYTE *name);
extern void Loadnetworkprefs(struct Networkprefs *np,BOOL saved,UBYTE *name);
extern void Loadcookieprefs(struct Networkprefs *np,BOOL saved,UBYTE *name);
extern void Loadwindowprefs(struct Windowprefs *np,BOOL saved,UBYTE *name);
extern void Loadsettingsprefs(struct Settingsprefs *sp,BOOL saved,UBYTE *name);
extern void Savebrowserprefs(struct Browserprefs *bp,BOOL saved,UBYTE *name);
extern void Saveprogramprefs(struct Programprefs *pp,BOOL saved,UBYTE *name);
extern void Saveguiprefs(struct Guiprefs *pp,BOOL saved,UBYTE *name);
extern void Savenetworkprefs(struct Networkprefs *np,BOOL saved,UBYTE *name);
extern void Savenocookieprefs(struct Networkprefs *np,BOOL saved,UBYTE *name);
extern void Savewindowprefs(struct Windowprefs *np,BOOL saved,UBYTE *name);
extern void Savesettingsprefs(struct Settingsprefs *sp,BOOL saved,UBYTE *name);

extern void Copyprefs(struct AwebPrefs *from,struct AwebPrefs *to);
extern void Saveprefs(struct AwebPrefs *prefs);
extern void Disposeprefs(struct AwebPrefs *prefs);

#endif
