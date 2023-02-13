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

/* defprefs.c - AWeb default prefs */

#define NOCFGLOCALE
#include "keyfile.h"
#include "awebcfg.h"
#include "awebprefs.h"
#include "libraries/awebclib.h"
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#define CATCOMP_NUMBERS
#include "locale.h"

#define FREEANDNULL(p) if(p) FREE(p);p=NULL

/* Initial empty LIST() */
#define EMPTYLIST(str,list) { (struct str *)&list.tail,NULL,(struct str *)&list.first }

#define NDTRUE          TRUE
#define DEFMAXCONNECT   4

struct AwebPrefs defprefs=
{  {  /* browser prefs */
#if defined(__amigaos4__)

      {  {  /* font [0] normal */
            {  "Bitstream Vera Serif.font",     12,NULL },
            {  "Bitstream Vera Serif.font",     13,NULL },
            {  "Bitstream Vera Serif.font",     15,NULL },
            {  "Bitstream Vera Serif.font",     18,NULL },
            {  "Bitstream Vera Serif.font",     24,NULL },
            {  "Bitstream Vera Serif.font",     30,NULL },
            {  "Bitstream Vera Serif.font",     36,NULL },
         },
         {  /* font [1] fixed */
            {  "Bitstream Vera Sans Mono.font",   11,NULL },
            {  "Bitstream Vera Sans Mono.font",   13,NULL },
            {  "Bitstream Vera Sans Mono.font",   14,NULL },
            {  "Bitstream Vera Sans Mono.font",   18,NULL },
            {  "Bitstream Vera Sans Mono.font",   24,NULL },
            {  "Bitstream Vera Sans Mono.font",   30,NULL },
            {  "Bitstream Vera Sans Mono.font",   36,NULL },
         },

#elif defined(__MORPHOS__)

      {  {  /* font [0] normal */
            {  "WebArial.font",     12,NULL },
            {  "WebArial.font",     14,NULL },
            {  "WebArial.font",     17,NULL },
            {  "WebArial.font",     20,NULL },
            {  "WebArial.font",     24,NULL },
            {  "WebArial.font",     30,NULL },
            {  "WebArial.font",     36,NULL },
         },
         {  /* font [1] fixed */
            {  "WebCourier.font",   11,NULL },
            {  "WebCourier.font",   13,NULL },
            {  "WebCourier.font",   15,NULL },
            {  "WebCourier.font",   18,NULL },
            {  "WebCourier.font",   24,NULL },
            {  "WebCourier.font",   30,NULL },
            {  "WebCourier.font",   36,NULL },
         },

#elif defined(__amigaos__)

      {  {  /* font [0] normal */
            {  "times.font",     11,NULL },
            {  "times.font",     13,NULL },
            {  "times.font",     15,NULL },
            {  "times.font",     18,NULL },
            {  "times.font",     24,NULL },
            {  "times.font",     30,NULL },
            {  "times.font",     36,NULL },
         },
         {  /* font [1] fixed */
            {  "courier.font",   11,NULL },
            {  "courier.font",   13,NULL },
            {  "courier.font",   15,NULL },
            {  "courier.font",   18,NULL },
            {  "courier.font",   24,NULL },
            {  "courier.font",   30,NULL },
            {  "courier.font",   36,NULL },
         },

#else
#error "Your OS is not supported"
#endif


      },
      EMPTYLIST(Fontalias,defprefs.browser.aliaslist), /* font alias */
      {  /* styles */
         {  0,3,FALSE,0 },                   /* NORMAL */
         {  0,6,FALSE,0 },                   /* H1 */
         {  0,5,FALSE,0 },                   /* H2 */
         {  0,4,FALSE,0 },                   /* H3 */
         {  0,3,FALSE,0 },                   /* H4 */
         {  0,2,FALSE,0 },                   /* H5 */
         {  0,1,FALSE,0 },                   /* H6 */
         {  0,2,TRUE,0 },                    /* BIG */
         {  0,-2,TRUE,0 },                   /* SMALL */
         {  0,-2,TRUE,0 },                   /* SUB */
         {  0,-2,TRUE,0 },                   /* SUP */
         {  0,0,TRUE,FSF_ITALIC },           /* ADDRESS */
         {  0,0,TRUE,FSF_ITALIC },           /* BLOCKQUOTE */
         {  0,0,TRUE,FSF_ITALIC },           /* CITE */
         {  1,0,TRUE,0 },                    /* CODE */
         {  0,0,TRUE,FSF_ITALIC },           /* DFN */
         {  0,0,TRUE,FSF_ITALIC },           /* EM */
         {  1,0,TRUE,0 },                    /* KBD */
         {  1,0,TRUE,0 },                    /* PRE */
         {  1,0,TRUE,0 },                    /* SAMP */
         {  0,1,TRUE,FSF_BOLD },             /* STRONG */
         {  1,0,TRUE,FSF_ITALIC },           /* VAR */
      },
      { 0,0,0xffffffff },                    /* new link */
      { 0,0,0x99999999 },                    /* old link */
      { 0xaaaaaaaa,0,0 },                    /* selected link */
      { 0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa },  /* background */
      { 0,0,0 },                             /* text */
      FALSE,                                 /* screenpens */
      HTML_TOLERANT,                         /* html mode */
      TRUE,                                  /* underline links */
      TRUE,                                  /* do colours */
      TRUE,                                  /* do bg sound */
      5,                                     /* blink rate */
      DOJS_11,                               /* do javascript */
      "iso-8859-1",                          /* default document encoding */
      TRUE,                                  /* do frames */
      TRUE,                                 /* incremental table display */
      FALSE,                                 /* nominal frame height */
      FALSE,                                 /* show JS errors */
      FALSE,                                 /* suppress banner windows */
      TRUE,                                  /* tooltips */
      TRUE,                                  /* hand pointer */
      TRUE,                                  /* JS watch */
      TRUE,                                  /* Temporay RastPort */
      EMPTYLIST(Mimeinfo,defprefs.browser.mimelist), /* mime */
      TRUE                                 /* draw border around images */
   },
   {  /* program prefs */
      "RAM:",                                /* save path */
      "T:",                                  /* temp path */
      32,                                    /* overlap */
      FALSE,                                 /* commands */
      TRUE,                                  /* hl auto close */
      FALSE,                                 /* hl requester */
      FALSE,                                 /* hl singleclick */
      TRUE,                                  /* wh auto close */
      NDTRUE,                                /* save icons */
      NDTRUE,                                /* allow clip dragging */
      FALSE,                                 /* auto open hotlist */
      FALSE,                                 /* auto open winhis */
      FALSE,                                 /* auto open netstat */
      FALSE,                                 /* center requesters */
      "C:Ed",                                /* editor cmd */
      "\"%f\"",                              /* editor args */
      "SYS:Utilities/MultiView",             /* viewer cmd */
      "\"%f\" pubscreen \"%n\"",             /* viewer args */
      "",                                    /* image viewer cmd */
      "",                                    /* image viewer args */
      "CON:////%t/AUTO/CLOSE/WAIT/SCREEN %n",/* console name */
      "",                                    /* startup script */
      "",                                    /* shutdown script */
      SCRTYPE_DEFAULT,                       /* screen type */
      "Workbench",                           /* screen name */
      PAL_MONITOR_ID|HIRES_KEY,              /* screen mode */
      8,                                     /* screen depth */
      640,256,                               /* screen width, height */
      LOADPAL_COLORS,                        /* load palette */
      {  /* screen palette */
         0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa, 0x00000000,0x00000000,0x00000000,
         0xffffffff,0xffffffff,0xffffffff, 0x66666666,0x88888888,0xbbbbbbbb,
         0xeeeeeeee,0x44444444,0x44444444, 0x55555555,0xdddddddd,0x55555555,
         0x00000000,0x44444444,0xdddddddd, 0xeeeeeeee,0x99999999,0x00000000,
      },
      {  /* screen pens */
         1,2,1,2,1,3,1,0,2,1,2,1,-1,
      },
      FALSE,                                 /* four colour mode */
   },
   {  /* gui prefs */
      IEQUALIFIER_LALT|IEQUALIFIER_RALT|IEQUALIFIER_CONTROL|IEQUALIFIER_MIDBUTTON,
                                             /* popup key */
      TRUE,                                  /* show buttons */
      TRUE,                                  /* show navigation controls */
      1,                                     /* button position trad AWeb look by default as this works on all screen sizes*/
      0,                                     /* buttons have image only by default */
      TRUE,                                  /* butons have borders by default */
      0,                                     /* windows have borders and gadget by default */
      EMPTYLIST(Menuentry,defprefs.gui.menus),   /* menus */
      EMPTYLIST(Userbutton,defprefs.gui.buttons),/* user buttons */
      {  EMPTYLIST(Popupitem,defprefs.gui.popupmenu[PUPT_IMAGE]),  /* popup menu items */
         EMPTYLIST(Popupitem,defprefs.gui.popupmenu[PUPT_LINK]),   /* popup menu items */
         EMPTYLIST(Popupitem,defprefs.gui.popupmenu[PUPT_FRAME]),  /* popup menu items */
      },
      EMPTYLIST(Userkey,defprefs.gui.keys),      /* user keys */
      {  {NULL},{NULL},{NULL},{NULL},{NULL}, /* navigation buttons */
         {NULL},{NULL},{NULL},{NULL},{NULL}
      },
   },
   {  /* network prefs */
      LOADIMG_ALL,                           /* load images */
      DEFMAXCONNECT,                         /* max connections */
      2,                                     /* max disk read */
      "file:///AWebPath:Docs/AWeb Manual.html",     /* home url */
      "index.html",                          /* local index */
      "http://www.google.com/search?hl=en&ie=ISO-8859-1&q=%s",   /* search url */
      "",                                    /* spoof ID */
      FALSE,                                 /* ignore mime */
      TRUE,                                  /* start home page */
      NDTRUE,                                /* auto search */
      FALSE,                                 /* continuous animation */
      FALSE,                                 /* restrict images to same host */
      NULL,NULL,                             /* telnet cmd,arg */
      NULL,NULL,                             /* starttcp cmd,arg */
      NULL,NULL,                             /* endtcp cmd,arg */
      NULL,                                  /* http proxy */
      NULL,                                  /* ftp proxy */
      NULL,                                  /* gopher proxy */
      NULL,                                  /* telnet proxy */
      FALSE,                                 /* limit proxy */
      FALSE,                                 /* passive FTP */
      EMPTYLIST(Noproxy,defprefs.network.noproxy),   /* no proxy sites */
      "AWebPath:Cache",                      /* cache path */
      1024,10240,                            /* memsize, disksize */
      100,0,                                 /* min free chip,fast */
      CAVERIFY_ALWAYS,                       /* verification mode for dynamic */ /* static versus dynamic is determined by prescence of last-modified */
      CAVERIFY_ONCE,                         /* verification mode for static  */
      FALSE,                                 /* fast response dynamic */
      TRUE,                                  /* fast response static */
      EMPTYLIST(Nocache,defprefs.network.nocache),   /* no cache sites */
      TRUE,                                  /* referer */
      TRUE,                                  /* form warn */
      TRUE,                                  /* spam block */
      TRUE,                                  /* ftp email address */
      COOKIES_ASK,                           /* use cookies */
      FALSE,                                 /* RFC 2109 cookies */
      EMPTYLIST(Nocookie,defprefs.network.nocookie), /* no cookie domains */
      "",                                    /* email address */
      "",                                    /* reply address */
      "",                                    /* full name */
      "",                                    /* organization */
      "",                                    /* smtp host */
      "",                                    /* nntp host */
      "",                                    /* signature file */
      "",                                    /* news auth username */
      "",                                    /* news auth password */
      "On %d you wrote about %s:",           /* mail quote header */
      "On %d, %n wrote about %s:",           /* news quote header */
      FALSE,                                 /* external mailer */
      NULL,NULL,                             /* mailto cmd,arg */
      FALSE,                                 /* external newsreader */
      NULL,NULL,                             /* news cmd,arg */
      0,                                     /* max articles news */
      TRUE,                                  /* framed news */
      TRUE,                                  /* sorted news */
      FALSE,                                 /* long headers news */
      FALSE,                                 /* proportional news */
      FALSE,                                 /* news by article number */

      FALSE,            /* cachelocalhost */
   },
   {  /* window prefs */
      0,0,9999,9999,                         /* browser window */
      0,0,100,100,                           /* alternate size */
      9999,60,200,100,                       /* netstat window */
      0,0,0,0,                               /* hotlist viewer */
      0,0,0,0,                               /* hotlist manager */
      0,0,0,0,                               /* history window */
      0,0,0,0,                               /* cachebrowser window */
      0,0,0,0,                               /* authorization edit window */
      0,0,0,0,                               /* info window */
      0,0,0,0,                               /* open file requesters */
      0,0,0,0,                               /* save file requesters */
   }
};

UBYTE configname[32]="";

enum SAVEFORMATS
{  SVF_WORD=1,SVF_LONG,SVF_COLOR,
   SVF_STRING,SVF_NLSTR,
   SVF_FONT,SVF_ALIAS,SVF_STYLE,
   SVF_SCRM,SVF_PALL,SVF_DPEN,
   SVF_WINDOW,SVF_MIME,SVF_MENU,SVF_NOCACH,SVF_NOPROX,SVF_NOCOOK,
   SVF_BUTTON,SVF_POPUP,SVF_KEY,SVF_NAV
};

struct Saveformat
{  UBYTE *label;
   short format;
   short savepass;
   short offset;
};

/* savepass field determines in which pass this item is saved.
 * It is read in either pass. Separate passes may use different
 * file handles. */

#define SEPARATOR    0x7f

#define OFFSET(s,e) (long)(&((struct s *)NULL)->e)

static struct Saveformat browsersave[]=
{  {"FON1",SVF_FONT,   0,OFFSET(Browserprefs,font[0][0])},
   {"FON2",SVF_FONT,   0,OFFSET(Browserprefs,font[0][1])},
   {"FON3",SVF_FONT,   0,OFFSET(Browserprefs,font[0][2])},
   {"FON4",SVF_FONT,   0,OFFSET(Browserprefs,font[0][3])},
   {"FON5",SVF_FONT,   0,OFFSET(Browserprefs,font[0][4])},
   {"FON6",SVF_FONT,   0,OFFSET(Browserprefs,font[0][5])},
   {"FON7",SVF_FONT,   0,OFFSET(Browserprefs,font[0][6])},
   {"FOF1",SVF_FONT,   0,OFFSET(Browserprefs,font[1][0])},
   {"FOF2",SVF_FONT,   0,OFFSET(Browserprefs,font[1][1])},
   {"FOF3",SVF_FONT,   0,OFFSET(Browserprefs,font[1][2])},
   {"FOF4",SVF_FONT,   0,OFFSET(Browserprefs,font[1][3])},
   {"FOF5",SVF_FONT,   0,OFFSET(Browserprefs,font[1][4])},
   {"FOF6",SVF_FONT,   0,OFFSET(Browserprefs,font[1][5])},
   {"FOF7",SVF_FONT,   0,OFFSET(Browserprefs,font[1][6])},
   {"FONA",SVF_ALIAS,  0,OFFSET(Browserprefs,aliaslist)},
   /* no entry for STYLE_NORMAL - must never change */
   {"FSH1",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_H1])},
   {"FSH2",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_H2])},
   {"FSH3",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_H3])},
   {"FSH4",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_H4])},
   {"FSH5",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_H5])},
   {"FSH6",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_H6])},
   {"FSBI",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_BIG])},
   {"FSSM",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_SMALL])},
   {"FSSB",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_SUB])},
   {"FSSP",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_SUP])},
   {"FSAD",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_ADDRESS])},
   {"FSBQ",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_BLOCKQUOTE])},
   {"FSCI",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_CITE])},
   {"FSCO",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_CODE])},
   {"FSDF",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_DFN])},
   {"FSEM",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_EM])},
   {"FSKB",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_KBD])},
   {"FSPR",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_PRE])},
   {"FSSA",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_SAMP])},
   {"FSST",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_STRONG])},
   {"FSVA",SVF_STYLE,  0,OFFSET(Browserprefs,styles[STYLE_VAR])},
   {"CONL",SVF_COLOR,  0,OFFSET(Browserprefs,newlink)},
   {"COOL",SVF_COLOR,  0,OFFSET(Browserprefs,oldlink)},
   {"COSL",SVF_COLOR,  0,OFFSET(Browserprefs,selectlink)},
   {"COBG",SVF_COLOR,  0,OFFSET(Browserprefs,background)},
   {"COTX",SVF_COLOR,  0,OFFSET(Browserprefs,text)},
   {"SBTP",SVF_WORD,   0,OFFSET(Browserprefs,screenpens)},
   {"HTML",SVF_WORD,   0,OFFSET(Browserprefs,htmlmode)},
   {"ULLK",SVF_WORD,   0,OFFSET(Browserprefs,ullink)},
   {"IMGB",SVF_WORD,   0,OFFSET(Browserprefs,imgborder)},
   {"DOCO",SVF_WORD,   0,OFFSET(Browserprefs,docolors)},
   {"BGSO",SVF_WORD,   0,OFFSET(Browserprefs,dobgsound)},
   {"BLNK",SVF_WORD,   0,OFFSET(Browserprefs,blinkrate)},
   {"DOJS",SVF_WORD,   0,OFFSET(Browserprefs,dojs)},
   {"CHRS",SVF_STRING, 0,OFFSET(Browserprefs,charset)},
   {"DOFR",SVF_WORD,   0,OFFSET(Browserprefs,doframes)},
   {"INCT",SVF_WORD,   0,OFFSET(Browserprefs,inctable)},
   {"NFRH",SVF_WORD,   0,OFFSET(Browserprefs,nominalframe)},
   {"JSER",SVF_WORD,   0,OFFSET(Browserprefs,jserrors)},
   {"NBAN",SVF_WORD,   0,OFFSET(Browserprefs,nobanners)},
   {"TTIP",SVF_WORD,   0,OFFSET(Browserprefs,tooltips)},
   {"HAND",SVF_WORD,   0,OFFSET(Browserprefs,handpointer)},
   {"JWTC",SVF_WORD,   0,OFFSET(Browserprefs,jswatch)},
   {"TPRP",SVF_WORD,   0,OFFSET(Browserprefs,temprp)},
   {"MIMD",SVF_MIME,   0},
   {NULL,0,0}
};

static struct Saveformat programsave[]=
{  {"SAVE",SVF_STRING,0,OFFSET(Programprefs,savepath)},
   {"TEMP",SVF_STRING,0,OFFSET(Programprefs,temppath)},
   {"OVLP",SVF_LONG,  0,OFFSET(Programprefs,overlap)},
   {"CMDS",SVF_WORD,  0,OFFSET(Programprefs,commands)},
   {"HLAC",SVF_WORD,  0,OFFSET(Programprefs,hlautoclose)},
   {"HLRQ",SVF_WORD,  0,OFFSET(Programprefs,hlrequester)},
   {"HLSC",SVF_WORD,  0,OFFSET(Programprefs,hlsingleclick)},
   {"AAHL",SVF_WORD,  0,OFFSET(Programprefs,aahotlist)},
   {"WHAC",SVF_WORD,  0,OFFSET(Programprefs,whautoclose)},
   {"SAVI",SVF_WORD,  0,OFFSET(Programprefs,saveicons)},
   {"DRAG",SVF_WORD,  0,OFFSET(Programprefs,clipdrag)},
   {"AAWH",SVF_WORD,  0,OFFSET(Programprefs,aawinhis)},
   {"AANS",SVF_WORD,  0,OFFSET(Programprefs,aanetstat)},
   {"CENR",SVF_WORD,  0,OFFSET(Programprefs,centerreq)},
   {"SCRT",SVF_WORD,  0,OFFSET(Programprefs,screentype)},
   {"VIWC",SVF_STRING,0,OFFSET(Programprefs,viewcmd)},
   {"VIWA",SVF_STRING,0,OFFSET(Programprefs,viewargs)},
   {"EDTC",SVF_STRING,0,OFFSET(Programprefs,editcmd)},
   {"EDTA",SVF_STRING,0,OFFSET(Programprefs,editargs)},
   {"IMVC",SVF_STRING,0,OFFSET(Programprefs,imgvcmd)},
   {"IMVA",SVF_STRING,0,OFFSET(Programprefs,imgvargs)},
   {"CONS",SVF_STRING,0,OFFSET(Programprefs,console)},
   {"STUP",SVF_STRING,0,OFFSET(Programprefs,startupscript)},
   {"SHDN",SVF_STRING,0,OFFSET(Programprefs,shutdownscript)},
   {"SCRM",SVF_SCRM,  0,0},
   {"SCRN",SVF_STRING,0,OFFSET(Programprefs,screenname)},
   {"SCRP",SVF_WORD,  0,OFFSET(Programprefs,loadpalette)},
   {"PALL",SVF_PALL,  0,0},
   {"DPEN",SVF_DPEN,  0,0},
/*
   {"4COL",SVF_WORD, 0,OFFSET(Programprefs,fourcolors)},
*/
   {NULL,0,0}
};

static struct Saveformat guisave[]=
{  {"PUKY",SVF_WORD,  0,OFFSET(Guiprefs,popupkey)},
   {"UBUS",SVF_WORD,  0,OFFSET(Guiprefs,showbuttons)},
   {"NAVS",SVF_WORD,  0,OFFSET(Guiprefs,shownav)},
   {"BUTP",SVF_WORD,  0,OFFSET(Guiprefs,buttonspos)},
   {"BTEX",SVF_WORD,  0,OFFSET(Guiprefs,textbuttons)},
   {"NBEV",SVF_WORD,  0,OFFSET(Guiprefs,nobevel)},
   {"WBRD",SVF_WORD,  0,OFFSET(Guiprefs,windowborder)},
   {"MENU",SVF_MENU,  0,OFFSET(Guiprefs,menus)},
   {"UBUT",SVF_BUTTON,0,OFFSET(Guiprefs,buttons)},
   {"PUPI",SVF_POPUP, 0,OFFSET(Guiprefs,popupmenu[PUPT_IMAGE])},
   {"PUPL",SVF_POPUP, 0,OFFSET(Guiprefs,popupmenu[PUPT_LINK])},
   {"PUPF",SVF_POPUP, 0,OFFSET(Guiprefs,popupmenu[PUPT_FRAME])},
   {"UKEY",SVF_KEY,   0,OFFSET(Guiprefs,keys)},
   {"NAVB",SVF_NAV,   0,OFFSET(Guiprefs,navs)},
   { NULL,0,0}
};

static struct Saveformat networksave[]=
{  {"LIMG",SVF_WORD,  0,OFFSET(Networkprefs,loadimg)},
   {"PARI",SVF_LONG,  0,OFFSET(Networkprefs,maxconnect)},
   {"MAXD",SVF_LONG,  0,OFFSET(Networkprefs,maxdiskread)},
   {"HOME",SVF_STRING,0,OFFSET(Networkprefs,homeurl)},
   {"LOIX",SVF_STRING,0,OFFSET(Networkprefs,localindex)},
   {"SRCH",SVF_STRING,0,OFFSET(Networkprefs,searchurl)},
   {"SPOF",SVF_STRING,0,OFFSET(Networkprefs,spoofid)},
   {"IGSM",SVF_WORD,  0,OFFSET(Networkprefs,ignoremime)},
   {"STHP",SVF_WORD,  0,OFFSET(Networkprefs,starthomepage)},
   {"ASRH",SVF_WORD,  0,OFFSET(Networkprefs,autosearch)},
   {"CANI",SVF_WORD,  0,OFFSET(Networkprefs,contanim)},
   {"RSTI",SVF_WORD,  0,OFFSET(Networkprefs,restrictimages)},
   {"TLNC",SVF_NLSTR, 0,OFFSET(Networkprefs,telnetcmd)},
   {"TLNA",SVF_NLSTR, 0,OFFSET(Networkprefs,telnetargs)},
   {"TCPC",SVF_NLSTR, 0,OFFSET(Networkprefs,starttcpcmd)},
   {"TCPA",SVF_NLSTR, 0,OFFSET(Networkprefs,starttcpargs)},
   {"TCEC",SVF_NLSTR, 0,OFFSET(Networkprefs,endtcpcmd)},
   {"TCEA",SVF_NLSTR, 0,OFFSET(Networkprefs,endtcpargs)},
   {"HTPX",SVF_NLSTR, 0,OFFSET(Networkprefs,httpproxy)},
   {"FTPX",SVF_NLSTR, 0,OFFSET(Networkprefs,ftpproxy)},
   {"GOPX",SVF_NLSTR, 0,OFFSET(Networkprefs,gopherproxy)},
   {"TEPX",SVF_NLSTR, 0,OFFSET(Networkprefs,telnetproxy)},
   {"LMPX",SVF_WORD,  0,OFFSET(Networkprefs,limitproxy)},
   {"PFTP",SVF_WORD,  0,OFFSET(Networkprefs,passiveftp)},
   {"NOPR",SVF_NOPROX,0,OFFSET(Networkprefs,noproxy)},
   {"CAPA",SVF_STRING,0,OFFSET(Networkprefs,cachepath)},
   {"CAME",SVF_LONG,  0,OFFSET(Networkprefs,camemsize)},
   {"CADI",SVF_LONG,  0,OFFSET(Networkprefs,cadisksize)},
   {"FREC",SVF_LONG,  0,OFFSET(Networkprefs,minfreechip)},
   {"FREF",SVF_LONG,  0,OFFSET(Networkprefs,minfreefast)},
   {"CAVD",SVF_WORD,  0,OFFSET(Networkprefs,caverify_dyn)},
   {"CAVS",SVF_WORD,  0,OFFSET(Networkprefs,caverify_stat)},
   {"CAFD",SVF_WORD,  0,OFFSET(Networkprefs,fastresponse_dyn)},
   {"CAFS",SVF_WORD,  0,OFFSET(Networkprefs,fastresponse_stat)},
   {"NOCA",SVF_NOCACH,0,OFFSET(Networkprefs,nocache)},
   {"REFE",SVF_WORD,  0,OFFSET(Networkprefs,referer)},
   {"FOWA",SVF_WORD,  0,OFFSET(Networkprefs,formwarn)},
   {"SPAM",SVF_WORD,  0,OFFSET(Networkprefs,spamblock)},
   {"FTPE",SVF_WORD,  0,OFFSET(Networkprefs,ftpemailaddr)},
   {"COOK",SVF_WORD,  0,OFFSET(Networkprefs,cookies)},
   {"RFCC",SVF_WORD,  0,OFFSET(Networkprefs,rfc2109)},
   {"NOCO",SVF_NOCOOK,1,OFFSET(Networkprefs,nocookie)},
   {"MNAD",SVF_STRING,0,OFFSET(Networkprefs,emailaddr)},
   {"MNRE",SVF_STRING,0,OFFSET(Networkprefs,replyaddr)},
   {"MNFN",SVF_STRING,0,OFFSET(Networkprefs,fullname)},
   {"MNOR",SVF_STRING,0,OFFSET(Networkprefs,organization)},
   {"MNSH",SVF_STRING,0,OFFSET(Networkprefs,smtphost)},
   {"MNNH",SVF_STRING,0,OFFSET(Networkprefs,nntphost)},
   {"MNSI",SVF_STRING,0,OFFSET(Networkprefs,sigfile)},
   {"MNAU",SVF_STRING,0,OFFSET(Networkprefs,newsauthuser)},
   {"MNAP",SVF_STRING,0,OFFSET(Networkprefs,newsauthpass)},
   {"MNMQ",SVF_STRING,0,OFFSET(Networkprefs,mailquotehdr)},
   {"MNNQ",SVF_STRING,0,OFFSET(Networkprefs,newsquotehdr)},
   {"MNEM",SVF_WORD,  0,OFFSET(Networkprefs,extmailer)},
   {"MLTC",SVF_NLSTR, 0,OFFSET(Networkprefs,mailtocmd)},
   {"MLTA",SVF_NLSTR, 0,OFFSET(Networkprefs,mailtoargs)},
   {"MNEN",SVF_WORD,  0,OFFSET(Networkprefs,extnewsreader)},
   {"NWSC",SVF_NLSTR, 0,OFFSET(Networkprefs,newscmd)},
   {"NWSA",SVF_NLSTR, 0,OFFSET(Networkprefs,newsargs)},
   {"MNMA",SVF_LONG,  0,OFFSET(Networkprefs,maxartnews)},
   {"MNFR",SVF_WORD,  0,OFFSET(Networkprefs,framednews)},
   {"MNSO",SVF_WORD,  0,OFFSET(Networkprefs,sortednews)},
   {"MNLO",SVF_WORD,  0,OFFSET(Networkprefs,longhdrnews)},
   {"MNPR",SVF_WORD,  0,OFFSET(Networkprefs,propnews)},
   {"MNNN",SVF_WORD,  0,OFFSET(Networkprefs,newsbynum)},
#ifdef DEVELOPER
   {"CALH",SVF_WORD,  0,OFFSET(Networkprefs,cachelocalhost)},
#endif
   {NULL,0,0}
};

static struct Saveformat windowsave[]=
{  {"WIND",SVF_WINDOW,0,OFFSET(Windowprefs,winx)},
   {"WINA",SVF_WINDOW,0,OFFSET(Windowprefs,wiax)},
   {"NWSD",SVF_WINDOW,0,OFFSET(Windowprefs,nwsx)},
   {"HOTD",SVF_WINDOW,0,OFFSET(Windowprefs,hotx)},
   {"HOMD",SVF_WINDOW,0,OFFSET(Windowprefs,homx)},
   {"WHID",SVF_WINDOW,0,OFFSET(Windowprefs,whisx)},
   {"CBRD",SVF_WINDOW,0,OFFSET(Windowprefs,cabrx)},
   {"AUTD",SVF_WINDOW,0,OFFSET(Windowprefs,autx)},
   {"INFD",SVF_WINDOW,0,OFFSET(Windowprefs,infx)},
   {"OPEN",SVF_WINDOW,0,OFFSET(Windowprefs,openx)},
   {"SAVE",SVF_WINDOW,0,OFFSET(Windowprefs,savex)},
   {NULL,0,0}
};

static struct Saveformat settingssave[]=
{  {"BRWW",SVF_WINDOW,0,OFFSET(Settingsprefs,brwx)},
   {"PRGW",SVF_WINDOW,0,OFFSET(Settingsprefs,prgx)},
   {"GUIW",SVF_WINDOW,0,OFFSET(Settingsprefs,guix)},
   {"NETW",SVF_WINDOW,0,OFFSET(Settingsprefs,netx)},
   {NULL,0,0}
};

static UBYTE *mdrivername[]=
{  "NONE","INT","PLI","EXT","EXP","ENF","SAV","CAN",NULL,
};

static UBYTE iobuf[512],param2buf[8];

/*---------------------------------------------------------------------------*/

/* Load prefs from file (fh) into prefs (prf) after (svf) */
static void Loadprefs(struct Saveformat *svf,long fh,void *prf)
{  struct Saveformat *s;
   UBYTE *p;
   void *item;
   long i;
   BOOL menus=FALSE,buttons=FALSE,popup=FALSE,keys=FALSE,alias=FALSE,mime=FALSE,
      nocache=FALSE,noproxy=FALSE,nocookie=FALSE,imagemime=FALSE;
   while(FGets(fh,iobuf,512))
   {  p=iobuf+strlen(iobuf)-1;
      if(*p=='\n') *p='\0';
      for(s=svf;s->format;s++)
      {  if(STRNEQUAL(iobuf,s->label,4)) break;
      }
      item=(char *)prf+s->offset;


      switch(s->format)
      {  case SVF_WORD:
            sscanf(iobuf+4," %hd",(short int *)item);
            break;
         case SVF_LONG:
            sscanf(iobuf+4," %ld",(long int *)item);
            break;
         case SVF_COLOR:
            {  struct Colorprefs *cp=(struct Colorprefs *)item;
               sscanf(iobuf+4," %lx %lx %lx",
                  &cp->red,&cp->green,&cp->blue);
            }
            break;
         case SVF_STRING:
            {  UBYTE **ptr=(UBYTE **)item;
               if(p=Dupstr(iobuf+5,strlen(iobuf+5)))
               {  if(*ptr) FREE(*ptr);
                  *ptr=p;
               }
            }
            break;
         case SVF_NLSTR:
            {  UBYTE **ptr=(UBYTE **)item;
               if(*ptr) FREE(*ptr);
               *ptr=Dupstr(iobuf+5,strlen(iobuf+5));
            }
            break;
         case SVF_FONT:
            {  struct Fontprefs *fp=(struct Fontprefs *)item;
               UBYTE *q,*r;
               for(q=iobuf+4;isspace(*q);q++);
               p=q;
               while(r=strchr(p+1,' ')) p=r;
               sscanf(p," %hd",&fp->fontsize);
               while(isspace(*p)) p--;
               if(p=Dupstr(q,p-q+1))
               {  if(fp->fontname) FREE(fp->fontname);
                  fp->fontname=p;
               }
            }
            break;
         case SVF_ALIAS:
            {  struct Browserprefs *bp=(struct Browserprefs *)prf;
               struct Fontalias *fa;
               UBYTE *aname,*pfn,*fname,*pfsize;
               long fn=0,fsize=8;
               if(!alias)
               {  while(fa=(struct Fontalias *)REMHEAD(&bp->aliaslist)) Freefontalias(fa);
               }
               alias=TRUE;
               for(p=iobuf+4;isspace(*p);p++);
               aname=p;
               if(p=strchr(p+1,SEPARATOR))
               {  *p='\0';
                  pfn=p+1;
                  if(p=strchr(pfn,SEPARATOR))
                  {  *p='\0';
                     fname=p+1;
                     if(p=strchr(fname,SEPARATOR))
                     {  *p='\0';
                        pfsize=p+1;
                        sscanf(pfn,"%ld",&fn);
                        sscanf(pfsize,"%ld",&fsize);
                        if(fn>0 && fn<=NRFONTS)
                        {  fn--;
                           if(fa=Addfontalias((struct FontaliasList *)&bp->aliaslist,aname))
                           {  fa->fp[fn].fontname=Dupstr(fname,-1);
                              fa->fp[fn].fontsize=fsize;
                           }
                        }
                     }
                  }
               }
            }
            break;
         case SVF_STYLE:
            {  struct Styleprefs *sp=(struct Styleprefs *)item;
               sscanf(iobuf+4," %hd %hd %s",&sp->fonttype,&sp->fontsize,param2buf);
               sp->style=0;
               for(p=param2buf;*p;p++)
               {  if(*p=='B') sp->style|=FSF_BOLD;
                  else if(*p=='I') sp->style|=FSF_ITALIC;
                  else if(*p=='U') sp->style|=FSF_UNDERLINED;
               }
            }
            break;
         case SVF_SCRM:
            {  struct Programprefs *pp=(struct Programprefs *)prf;
               sscanf(iobuf+4," %lx %hd %hd %hd",&pp->screenmode,
                  &pp->screendepth,
                  &pp->screenwidth,&pp->screenheight);
            }
            break;
         case SVF_PALL:
            {  struct Programprefs *pp=(struct Programprefs *)prf;
               sscanf(iobuf+4," %ld",&i);
               sscanf(iobuf+6," %lx %lx %lx",&pp->scrpalette[3*i],
                  &pp->scrpalette[3*i+1],&pp->scrpalette[3*i+2]);
            }
            break;
         case SVF_DPEN:
            {  struct Programprefs *pp=(struct Programprefs *)prf;
               sscanf(iobuf+4,
                  " %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd",
                  &pp->scrdrawpens[0],&pp->scrdrawpens[1],
                  &pp->scrdrawpens[2],&pp->scrdrawpens[3],
                  &pp->scrdrawpens[4],&pp->scrdrawpens[5],
                  &pp->scrdrawpens[6],&pp->scrdrawpens[7],
                  &pp->scrdrawpens[8],&pp->scrdrawpens[9],
                  &pp->scrdrawpens[10],&pp->scrdrawpens[11]);
            }
            break;
         case SVF_WINDOW:
            {  short *ptr=(short *)item;
               sscanf(iobuf+4," %hd %hd %hd %hd",
                  &ptr[0],&ptr[1],&ptr[2],&ptr[3]);
            }
            break;
         case SVF_MIME:
            {  struct Browserprefs *bp=(struct Browserprefs *)prf;
               UBYTE *type,*subt,*exts,*drv,*cmd,*args;
               UWORD driver;
               struct Mimeinfo *mi;
               if(!mime)
               {  while(mi=(struct Mimeinfo *)REMHEAD(&bp->mimelist)) Freemimeinfo(mi);
               }
               mime=TRUE;
               type=iobuf+5;
               if(p=strchr(type,'/'))
               {  *p='\0';
                  subt=p+1;
                  if(p=strchr(subt,';'))
                  {  *p='\0';
                     exts=p+1;
                     if(p=strchr(exts,';'))
                     {  *p='\0';
                        drv=p+1;
                        if(p=strchr(drv,';'))
                        {  *p='\0';
                           cmd=p+1;
                           if(p=strchr(cmd,';'))
                           {  *p='\0';
                              args=p+1;
                              for(driver=0;mdrivername[driver];driver++)
                              {  if(STRNIEQUAL(mdrivername[driver],drv,3)) break;
                              }
                              if(!mdrivername[driver]) driver=0;
                              Addmimeinfo((struct MimeinfoList *)&bp->mimelist,type,subt,exts,driver,cmd,args);
                           }
                        }
                     }
                  }
               }
            }
            break;
         case SVF_BUTTON:
            {  struct Guiprefs *gp=(struct Guiprefs *)prf;
               UBYTE *label,*cmd;
               struct Userbutton *ub;
               if(!buttons)
               {  while(ub=(struct Userbutton *)REMHEAD(&gp->buttons)) Freeuserbutton(ub);
               }
               buttons=TRUE;
               label=iobuf+5;
               if(p=strchr(label,SEPARATOR))
               {  *p='\0';
                  cmd=p+1;
                  Adduserbutton(&gp->buttons,label,cmd);
               }
            }
            break;
         case SVF_POPUP:
            {  struct Guiprefs *gp=(struct Guiprefs *)prf;
               LIST(Popupitem) *list=item;
               UBYTE *flagsp,*title,*cmd;
               struct Popupitem *pi;
               short i;
               UWORD flags;
               if(!popup)
               {  for(i=0;i<NRPOPUPMENUS;i++)
                  {  while(pi=(struct Popupitem *)REMHEAD(&gp->popupmenu[i])) Freepopupitem(pi);
                  }
               }
               popup=TRUE;
               flagsp=iobuf+5;
               if(p=strchr(flagsp,SEPARATOR))
               {  *p='\0';
                  sscanf(flagsp,"%hd",&flags);
                  title=p+1;
                  if(p=strchr(title,SEPARATOR))
                  {  *p='\0';
                     cmd=p+1;
                     Addpopupitem(list,flags,title,cmd);
                  }
               }
            }
            break;
         case SVF_MENU:
            {  struct Guiprefs *gp=(struct Guiprefs *)prf;
               UBYTE *cmd=NULL,*title=NULL;
               UWORD type;
               UBYTE scut='\0';
               struct Menuentry *me;
               if(!menus)
               {  while(me=(struct Menuentry *)REMHEAD(&gp->menus)) Freemenuentry(me);
               }
               menus=TRUE;
               p=iobuf+5;
               switch(*p)
               {  case 'M':type=AMENU_MENU;break;
                  case 'I':type=AMENU_ITEM;break;
                  case 'S':type=AMENU_SUB;break;
                  default: type=AMENU_SEPARATOR;
               }
               p++;
               if(p[0]=='/')
               {  scut=p[1];
                  p+=2;
               }
               if(*p==';')
               {  p++;
                  cmd=p;
                  if(p=strrchr(cmd,';'))
                  {  *p='\0';
                     title=p+1;
                  }
               }
               Addmenuentry(&gp->menus,type,title,scut,cmd);
            }
            break;
         case SVF_NAV:
            {  struct Guiprefs *gp=(struct Guiprefs *)prf;
               long n=atoi(iobuf+5);
               p=strchr(iobuf+5,' ');
               if(n>=0 && n<NRNAVBUTTONS && p)
               {  while(*p==' ') p++;
                  if(p=Dupstr(p,-1))
                  {  if(gp->navs[n].cmd) FREE(gp->navs[n].cmd);
                     gp->navs[n].cmd=p;
                  }
               }
            }
            break;
         case SVF_KEY:
            {  struct Guiprefs *gp=(struct Guiprefs *)prf;
               UBYTE *keyp,*cmd;
               UWORD key;
               struct Userkey *uk;
               if(!keys)
               {  while(uk=(struct Userkey *)REMHEAD(&gp->keys)) Freeuserkey(uk);
               }
               keys=TRUE;
               keyp=iobuf+5;
               if(p=strchr(keyp,' '))
               {  *p='\0';
                 // sscanf(keyp,"%hx",&key);
#if defined(__amigaos4__)

                  while((*keyp=='0')&&(keyp<p))keyp++;

#endif
                  sscanf(keyp,"%hx",&key);

                  cmd=p+1;
                  Adduserkey(&gp->keys,key,cmd);
               }
            }
            break;
         case SVF_NOCACH:
            {  struct Networkprefs *np=(struct Networkprefs *)prf;
               struct Nocache *nc;
               if(!nocache)
               {  while(nc=(struct Nocache *)REMHEAD(&np->nocache)) Freenocache(nc);
               }
               nocache=TRUE;
               Addnocache((struct NocacheList *)&np->nocache,iobuf+5);
            }
            break;
         case SVF_NOPROX:
            {  struct Networkprefs *np=(struct Networkprefs *)prf;
               struct Noproxy *nc;
               if(!noproxy)
               {  while(nc=(struct Noproxy *)REMHEAD(&np->noproxy)) Freenoproxy(nc);
               }
               noproxy=TRUE;
               Addnoproxy((struct NocacheList *)&np->noproxy,iobuf+5);
            }
            break;
         case SVF_NOCOOK:
            {  struct Networkprefs *np=(struct Networkprefs *)prf;
               struct Nocookie *nc;
               if(!nocookie)
               {  while(nc=(struct Nocookie *)REMHEAD(&np->nocookie)) Freenocookie(nc);
               }
               nocookie=TRUE;
               Addnocookie((struct NocacheList *)&np->nocookie,iobuf+5);
            }
            break;
         default:
            if(STRNIEQUAL(iobuf,"MIME",4) && svf==browsersave)   /* Auto convert to MIMD */
            {  struct Browserprefs *bp=(struct Browserprefs *)prf;
               UBYTE *type,*subt,*exts,*cmd,*args;
               UWORD driver;
               struct Mimeinfo *mi;
               if(!mime)
               {  imagemime=TRUE;
                  while(mi=(struct Mimeinfo *)REMHEAD(&bp->mimelist)) Freemimeinfo(mi);
               }
               mime=TRUE;
               type=iobuf+5;
               if(p=strchr(type,'/'))
               {  *p='\0';
                  subt=p+1;
                  if(p=strchr(subt,';'))
                  {  *p='\0';
                     exts=p+1;
                     if(p=strchr(exts,';'))
                     {  *p='\0';
                        cmd=p+1;
                        if(p=strchr(cmd,';'))
                        {  *p='\0';
                           args=p+1;
                           if(STRIEQUAL(type,"TEXT")
                           && ((STRIEQUAL(subt,"PLAIN") || STRIEQUAL(subt,"HTML"))))
                           {  driver=MDRIVER_INTERNAL;
                              *cmd=*args='\0';
                           }
                           else if(STRIEQUAL(type,"IMAGE"))
                           {  driver=MDRIVER_INTERNAL;
                              *cmd=*args='\0';
                              if(STRIEQUAL(subt,"*")) imagemime=FALSE;
                           }
                           else if(*cmd && *args)
                           {  driver=MDRIVER_EXTERNAL;
                           }
                           else
                           {  driver=MDRIVER_NONE;
                           }
                           Addmimeinfo((struct MimeinfoList *)&bp->mimelist,type,subt,exts,driver,cmd,args);
                        }
                     }
                  }
               }
            }
            else if(STRNIEQUAL(iobuf,"AREX",4) && svf==guisave) /* Auto convert to MENU */
            {  struct Guiprefs *gp=(struct Guiprefs *)prf;
               UBYTE *cmdp,*macro,*title;
               short cmd=0;
               UBYTE scut='\0';
               UBYTE *buf;
               cmdp=iobuf+5;
               if(p=strchr(cmdp,';'))
               {  *p='\0';
                  sscanf(cmdp,"%hd",&cmd);
                  if(cmd==10) scut='0';
                  else if(cmd) scut='0'+cmd;
                  macro=p+1;
                  if(p=strchr(macro,';'))
                  {  *p='\0';
                     title=p+1;
                     if(macro[0]=='"')
                     {  p=macro+strlen(macro)-1;
                        if(*p=='"') *p='\0';
                        macro++;
                        Addmenuentry(&gp->menus,AMENU_ITEM,title,scut,macro);
                     }
                     else if(buf=ALLOCTYPE(UBYTE,strlen(macro)+8,0))
                     {  strcpy(buf,"RUN ");
                        strcat(buf,macro);
                        Addmenuentry(&gp->menus,AMENU_ITEM,title,scut,buf);
                        FREE(buf);
                     }
                  }
               }
            }
            else if(STRNIEQUAL(iobuf,"_ADD",4))
            {  p=iobuf+5;
               if(STRNIEQUAL(p,"MIMD",4)) mime=TRUE;
               else if(STRNIEQUAL(p,"UBUT",4)) buttons=TRUE;
               else if(STRNIEQUAL(p,"PUPX",4)) popup=TRUE;
               else if(STRNIEQUAL(p,"UKEY",4)) keys=TRUE;
            }
      }
   }
   if(imagemime)
   {  Addmimeinfo((struct MimeinfoList *)&((struct Browserprefs *)prf)->mimelist,"IMAGE","*",
         "",MDRIVER_INTERNAL,"","");
   }
}

static void Saveprefsfmt(struct Saveformat *svf,long fh,void *prf,short pass)
{  struct Saveformat *s;
   void *item;
   UBYTE *p;
   long i;
   strcpy(iobuf,"** Do not modify this file by hand! **\n");
   Write(fh,iobuf,strlen(iobuf));
   for(s=svf;s->format;s++)
   {  if(s->savepass==pass)
      {  *iobuf='\0';
         item=(char *)prf+s->offset;
         switch(s->format)
         {  case SVF_WORD:
               sprintf(iobuf,"%s %d\n",s->label,*(short *)item);
               break;
            case SVF_LONG:
               sprintf(iobuf,"%s %ld\n",s->label,*(long *)item);
               break;
            case SVF_COLOR:
               {  struct Colorprefs *cp=(struct Colorprefs *)item;
                  sprintf(iobuf,"%s %08lX %08lX %08lX\n",s->label,
                     cp->red,cp->green,cp->blue);
               }
               break;
            case SVF_STRING:
               sprintf(iobuf,"%s %s\n",s->label,*(UBYTE **)item);
               break;
            case SVF_NLSTR:
               p=*(UBYTE **)item;
               if(p) sprintf(iobuf,"%s %s\n",s->label,p);
               break;
            case SVF_FONT:
               {  struct Fontprefs *fp=(struct Fontprefs *)item;
                  sprintf(iobuf,"%s %s %d\n",s->label,fp->fontname,fp->fontsize);
               }
               break;
            case SVF_ALIAS:
               {  struct Browserprefs *bp=(struct Browserprefs *)prf;
                  struct Fontalias *fa;
                  if(ISEMPTY(&bp->aliaslist))
                  {  /* Force defaults to be cleared from list on subsequent load */
                     strcpy(iobuf,"FONA -!-\n");
                  }
                  else
                  {  for(fa=bp->aliaslist.first;fa->next;fa=fa->next)
                     {  if(fa->alias && *fa->alias)
                        {  for(i=0;i<NRFONTS;i++)
                           {  sprintf(iobuf,"FONA %s%c%ld%c%s%c%d\n",
                                 fa->alias,SEPARATOR,i+1,SEPARATOR,fa->fp[i].fontname,
                                 SEPARATOR,fa->fp[i].fontsize);
                              Write(fh,iobuf,strlen(iobuf));
                           }
                        }
                     }
                     *iobuf='\0';
                  }
               }
               break;
            case SVF_STYLE:
               {  struct Styleprefs *sp=(struct Styleprefs *)item;
                  *param2buf='\0';
                  if(sp->style&FSF_BOLD) strcat(param2buf,"B");
                  if(sp->style&FSF_ITALIC) strcat(param2buf,"I");
                  if(sp->style&FSF_UNDERLINED) strcat(param2buf,"U");
                  sprintf(iobuf,"%s %d %d .%s\n",s->label,sp->fonttype,sp->fontsize,param2buf);
               }
               break;
            case SVF_SCRM:
               {  struct Programprefs *pp=(struct Programprefs *)prf;
                  sprintf(iobuf,"SCRM %08lx %d %d %d\n",pp->screenmode,
                     pp->screendepth,pp->screenwidth,pp->screenheight);
               }
               break;
            case SVF_PALL:
               {  struct Programprefs *pp=(struct Programprefs *)prf;
                  for(i=0;i<8;i++)
                  {  sprintf(iobuf,"PALL %ld %08lX %08lX %08lX\n",i,
                        pp->scrpalette[3*i],pp->scrpalette[3*i+1],
                        pp->scrpalette[3*i+2]);
                     Write(fh,iobuf,strlen(iobuf));
                  }
                  *iobuf='\0';
               }
               break;
            case SVF_DPEN:
               {  struct Programprefs *pp=(struct Programprefs *)prf;
                  p=iobuf+sprintf(iobuf,"DPEN");
                  for(i=0;i<12;i++) p+=sprintf(p," %d",pp->scrdrawpens[i]);
                  sprintf(p,"\n");
               }
               break;
            case SVF_WINDOW:
               {  short *ptr=(short *)item;
                  sprintf(iobuf,"%s %d %d %d %d\n",s->label,
                     ptr[0],ptr[1],ptr[2],ptr[3]);
               }
               break;
            case SVF_MIME:
               {  struct Browserprefs *bp=(struct Browserprefs *)prf;
                  struct Mimeinfo *mi;
                  for(mi=bp->mimelist.first;mi->next;mi=mi->next)
                  {  sprintf(iobuf,"MIMD %s/%s;%s;%s;%s;%s\n",mi->type,mi->subtype,
                        mi->extensions?mi->extensions:NULLSTRING,
                        mdrivername[mi->driver],
                        mi->cmd?mi->cmd:NULLSTRING,mi->args?mi->args:NULLSTRING);
                     Write(fh,iobuf,strlen(iobuf));
                  }
                  *iobuf='\0';
               }
               break;
            case SVF_BUTTON:
               {  struct Guiprefs *gp=(struct Guiprefs *)prf;
                  struct Userbutton *ub;
                  for(ub=gp->buttons.first;ub->next;ub=ub->next)
                  {  sprintf(iobuf,"UBUT %s%c%s\n",ub->label,SEPARATOR,ub->cmd);
                     Write(fh,iobuf,strlen(iobuf));
                  }
                  *iobuf='\0';
               }
               break;
            case SVF_POPUP:
               {  LIST(Popupitem) *list=item;
                  struct Popupitem *pi;
                  for(pi=list->first;pi->next;pi=pi->next)
                  {  sprintf(iobuf,"%s %d%c%s%c%s\n",s->label,
                        pi->flags,SEPARATOR,pi->title,SEPARATOR,pi->cmd);
                     Write(fh,iobuf,strlen(iobuf));
                  }
                  *iobuf='\0';
               }
               break;
            case SVF_MENU:
               {  struct Guiprefs *gp=(struct Guiprefs *)prf;
                  struct Menuentry *me;
                  for(me=gp->menus.first;me->next;me=me->next)
                  {  p=iobuf+sprintf(iobuf,"MENU %c","MIS-"[me->type]);
                     if(me->scut[0])
                     {  *p++='/';
                        *p++=me->scut[0];
                     }
                     sprintf(p,";%s;%s\n",me->cmd?me->cmd:NULLSTRING,
                        me->title?me->title:NULLSTRING);
                     Write(fh,iobuf,strlen(iobuf));
                  }
                  *iobuf='\0';
               }
               break;
            case SVF_NAV:
               {  struct Guiprefs *gp=(struct Guiprefs *)prf;
                  int n;
                  for(n=0;n<NRNAVBUTTONS;n++)
                  {  sprintf(iobuf,"NAVB %d %s\n",n,gp->navs[n].cmd);
                     Write(fh,iobuf,strlen(iobuf));
                  }
                  *iobuf='\0';
               }
               break;
            case SVF_KEY:
               {  struct Guiprefs *gp=(struct Guiprefs *)prf;
                  struct Userkey *uk;
                  for(uk=gp->keys.first;uk->next;uk=uk->next)
                  {  sprintf(iobuf,"UKEY %04x %s\n",uk->key,uk->cmd);
                     Write(fh,iobuf,strlen(iobuf));
                  }
                  *iobuf='\0';
               }
               break;
            case SVF_NOCACH:
            case SVF_NOPROX:
            case SVF_NOCOOK:
               {  LIST(Nocache) *list=item;
                  struct Nocache *nc;
                  for(nc=list->first;nc->next;nc=nc->next)
                  {  sprintf(iobuf,"%s %s\n",s->label,nc->name);
                     Write(fh,iobuf,strlen(iobuf));
                  }
                  *iobuf='\0';
               }
               break;
         }
         if(*iobuf) Write(fh,iobuf,strlen(iobuf));
      }
   }
}

static void Makedir(UBYTE *dir)
{  long lock=Lock(dir,SHARED_LOCK);
   if(!lock) lock=CreateDir(dir);
   if(lock) UnLock(lock);
}

static long Openname(BOOL saved,UBYTE *name,UBYTE *file,long mode)
{  if(name) strcpy(iobuf,name);
   else
   {  sprintf(iobuf,"%s:" DEFAULTCFG "%c%s",
         saved?"ENVARC":"ENV",
         *configname?'/':'\0',
         *configname?configname:(UBYTE *)"");
      if(mode==MODE_NEWFILE) Makedir(iobuf);
      if(file) strcat(iobuf,file);
   }
   return Open(iobuf,mode);
}

/* Copy optional strings (may be NULL) */
static BOOL Copyoptstring(UBYTE *from,UBYTE **to)
{  BOOL ok=TRUE;
   if(*to)
   {  FREE(*to);
      *to=NULL;
   }
   if(from)
   {  if(!(*to=Dupstr(from,-1))) ok=FALSE;
   }
   return ok;
}

static void Adddeffontalias(UBYTE *alias,short size,UBYTE *font,short fsize)
{  struct Fontalias *fa;
   if(fa=Addfontalias((struct FontaliasList *)&defprefs.browser.aliaslist,alias))
   {  fa->fp[size].fontname=Dupstr(font,-1);
      fa->fp[size].fontsize=fsize;
   }
}

static void Adddefmenu(UWORD type,ULONG msg,UBYTE *cmd)
{
   char *title = NULL;
   UBYTE scut='\0';

   if(msg)
   {
       title = Getmainstr(msg);
   }

   if(!title) title="";
   if(title[0] && title[1]=='/')
   {  scut=title[0];
      title+=2;
   }
   Addmenuentry(&defprefs.gui.menus,type,title,scut,cmd?cmd:NULLSTRING);
}

/*---------------------------------------------------------------------------*/

BOOL Initdefprefs(void)
{  struct Screen *screen;
   struct DrawInfo *dri;
   UBYTE *alias,*font;
   if(screen=LockPubScreen(NULL))
   {  defprefs.program.screenmode=GetVPModeID(&screen->ViewPort);
      defprefs.program.screenwidth=screen->Width;
      defprefs.program.screenheight=screen->Height;
      if(dri=GetScreenDrawInfo(screen))
      {  defprefs.program.screendepth=dri->dri_Depth;
         FreeScreenDrawInfo(screen,dri);
      }
      UnlockPubScreen(NULL,screen);
   }

   alias="Arial,Helvetica";
#if defined(__amigaos4__)
   font="Bitstream Vera Sans.font";
   Adddeffontalias(alias,0,font,12);
   Adddeffontalias(alias,1,font,13);
   Adddeffontalias(alias,2,font,15);
   Adddeffontalias(alias,3,font,18);
   Adddeffontalias(alias,4,font,24);
   Adddeffontalias(alias,5,font,30);
   Adddeffontalias(alias,6,font,36);

#elif defined(__MORPHOS__)

   font="WebArial.font";
   Adddeffontalias(alias,0,font,9);
   Adddeffontalias(alias,1,font,11);
   Adddeffontalias(alias,2,font,13);
   Adddeffontalias(alias,3,font,15);
   Adddeffontalias(alias,4,font,18);
   Adddeffontalias(alias,5,font,24);
   Adddeffontalias(alias,6,font,24);

#elif defined(__amigaos__)

   font="helvetica.font";
   Adddeffontalias(alias,0,font,9);
   Adddeffontalias(alias,1,font,11);
   Adddeffontalias(alias,2,font,13);
   Adddeffontalias(alias,3,font,15);
   Adddeffontalias(alias,4,font,18);
   Adddeffontalias(alias,5,font,24);
   Adddeffontalias(alias,6,font,24);

#else
#error Your OS is not supported
#endif

   if(!(defprefs.browser.charset=Dupstr("iso-8859-1",-1))) return FALSE;

   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "TEXT","HTML","html htm shtml phtml asp",MDRIVER_INTERNAL,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "TEXT","PLAIN","txt",MDRIVER_INTERNAL,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "TEXT","X-AGUIDE","guide",MDRIVER_NONE,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "TEXT","*","",MDRIVER_EXTERNAL,
      "SYS:Utilities/MultiView","%f pubscreen %n")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "IMAGE","GIF","gif",
      MDRIVER_PLUGIN,"AWebPath:AWebPlugin/awebgif.awebplugin","PROGRESS=0 LOOPS=-1 LOWPRI"
      )) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "IMAGE","JPEG","jpg jpeg jpe jfif",
      MDRIVER_PLUGIN,"AWebPath:AWebPlugin/awebjfif.awebplugin","PROGRESS=0 MULTIPASS=1 DITHER=2  MAXMEM=1024 LOWPRI"
      )) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "IMAGE","X-IFF","iff ilbm ham ham8",MDRIVER_INTERNAL,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "IMAGE","PNG","png",
      MDRIVER_PLUGIN,"AWebPath:AWebPlugin/awebpng.awebplugin","MULTIPASS=1 PROGRESS=0 GAMMA=2.0 LOWPRI"
      )) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "IMAGE","XBITMAP","xbm",MDRIVER_INTERNAL,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "IMAGE","*","",MDRIVER_INTERNAL,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "VIDEO","MPEG","mpg mpeg",MDRIVER_NONE,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "VIDEO","QUICKTIME","qt",MDRIVER_NONE,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "VIDEO","X-ANIM","ani anim",MDRIVER_EXTERNAL,
      "SYS:Utilities/MultiView","%f screen")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "AUDIO","BASIC","au snd",MDRIVER_NONE,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "AUDIO","MIDI","mid midi",MDRIVER_NONE,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "AUDIO","WAV","wav",MDRIVER_NONE,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "AUDIO","X-8SVX","8svx svx",MDRIVER_EXTERNAL,
      "SYS:Utilities/MultiView","%f pubscreen %n")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "AUDIO","X-MOD","mod",MDRIVER_NONE,"","")) return FALSE;
   if(!Addmimeinfo((struct MimeinfoList *)&defprefs.browser.mimelist,
      "AUDIO","*","",MDRIVER_INTERNAL,"","")) return FALSE;

   Adddefmenu(AMENU_MENU,MSG_PROJECT_MENU,NULL);
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_NEWWINDOW,"NEW \"\" ");
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_CLOSEWINDOW,"CLOSE");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
#ifndef LOCALONLY
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_OPENURL,"OPENREQ");
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_OPENWWW,"URLFIELD SET \"http://www.\" ACTIVATE");
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_OPENNEWS,"OPEN \"news:\"");
#endif
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_OPENLOCAL,"OPENREQ FILE");
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_OPENSEARCH,"OPEN \"file:///AWebPath:extras/search.html\"");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_SEARCH,"SEARCH TARGET %i");
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_SOURCE,"VIEWSOURCE %u");
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_EDIT,"EDITSOURCE %u");
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_SAVEHTML,"SAVEAS");
#ifndef NETDEMO
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_SAVEIFF,"SAVEIFF");
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_PRINT,"PRINT REQUEST");
#endif
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_INFO,"INFO");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_ABOUT,"SUBWINDOW ABOUT OPEN");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_ICONIFY,"ICONIFY HIDE");
   Adddefmenu(AMENU_ITEM,MSG_PROJECT_QUIT,"QUIT FORCE");

   Adddefmenu(AMENU_MENU,MSG_CONTROL_MENU,NULL);
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_LOADNOW,NULL);
   Adddefmenu(AMENU_SUB, MSG_CONTROL_LOADNOWALL,"LOADIMAGES");
   Adddefmenu(AMENU_SUB, MSG_CONTROL_LOADNOWMAPS,"LOADIMAGES MAPS");
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_PLAYBGSOUND,"PLAYBGSOUND");
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_RESET,"RESETFRAME");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_COPYURL,"COPYURL");
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_PASTEURL,"URLFIELD PASTE ACTIVATE");
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_COPYBLOCK,"COPYBLOCK");
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_DRAGGING,"@DRAGGING");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_NETWORKSTATUS,"SUBWINDOW NETSTATUS OPEN");
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_RLOAD,NULL);
   Adddefmenu(AMENU_SUB, MSG_CONTROL_RLOADDOC,"RELOAD");
   Adddefmenu(AMENU_SUB, MSG_CONTROL_RLOADIMGS,"RELOAD IMAGES");
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_CANCEL,"CANCEL");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_NEXTWINDOW,"WINDOW TOFRONT ACTIVATE NEXT 1");
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_PREVWINDOW,"WINDOW TOFRONT ACTIVATE NEXT -1");
#ifndef LOCALONLY
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_NOPROXY,"@NOPROXY");
#endif
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_BREAKJS,"@BREAKJS");
   Adddefmenu(AMENU_ITEM,MSG_CONTROL_DEBUGJS,"@DEBUGJS");

   Adddefmenu(AMENU_MENU,MSG_CACHE_MENU,NULL);
   Adddefmenu(AMENU_ITEM,MSG_CACHE_BROWSER,"SUBWINDOW CACHEBROWSER OPEN");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_CACHE_FLUSH,NULL);
    Adddefmenu(AMENU_SUB,MSG_CACHE_FLSHIMGSND,"FLUSHCACHE IMAGES");
    Adddefmenu(AMENU_SUB,MSG_CACHE_FLSHIMGS,"FLUSHCACHE IMAGES ALL");
    Adddefmenu(AMENU_SUB,MSG_CACHE_FLSHDOCSND,"FLUSHCACHE DOCUMENTS");
#ifndef LOCALONLY
   Adddefmenu(AMENU_ITEM,MSG_CACHE_DELETE,NULL);
   Adddefmenu(AMENU_SUB, MSG_CACHE_DELIMGS,"DELETECACHE IMAGES");
   Adddefmenu(AMENU_SUB, MSG_CACHE_DELDOCS,"DELETECACHE DOCUMENTS");
   Adddefmenu(AMENU_SUB, MSG_CACHE_CLEAR,"DELETECACHE DOCUMENTS IMAGES");
   Adddefmenu(AMENU_ITEM,MSG_CACHE_FIX,"FIXCACHE");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_CACHE_SAVEAUTH,"SAVEAUTHORIZE");
   Adddefmenu(AMENU_ITEM,MSG_CACHE_EDITAUTH,"SUBWINDOW AUTHORIZE OPEN");
#endif

   Adddefmenu(AMENU_MENU,MSG_NAVIGATE_MENU,NULL);
   Adddefmenu(AMENU_ITEM,MSG_NAVIGATE_BACK,"GO BACK");
   Adddefmenu(AMENU_ITEM,MSG_NAVIGATE_FWD,"GO FORWARD");
   Adddefmenu(AMENU_ITEM,MSG_NAVIGATE_HOME,"GO HOME");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_NAVIGATE_HISTORY,"SUBWINDOW HISTORY OPEN");

   Adddefmenu(AMENU_MENU,MSG_HOTLIST_MENU,NULL);
   Adddefmenu(AMENU_ITEM,MSG_HOTLIST_ADDHOT,"ADDHOTLIST %u %t");
   Adddefmenu(AMENU_ITEM,MSG_HOTLIST_SHOWHOT,"OPEN \"x-aweb:hotlist\"");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_HOTLIST_VIEW,"SUBWINDOW HOTVIEWER OPEN");
   Adddefmenu(AMENU_ITEM,MSG_HOTLIST_MGR,"SUBWINDOW HOTMANAGER OPEN");
   Adddefmenu(AMENU_ITEM,MSG_HOTLIST_SAVE,"HOTLIST SAVE");
   Adddefmenu(AMENU_ITEM,MSG_HOTLIST_RESTORE,"HOTLIST RESTORE");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_HOTLIST_AMHOTRX,"OPEN \"x-aweb:amhotlist.rexx\"");
   Adddefmenu(AMENU_ITEM,MSG_HOTLIST_AMHOT20,"OPEN \"x-aweb:amhotlist.20\"");
   Adddefmenu(AMENU_ITEM,MSG_HOTLIST_IBHOT,"@OTHERHOT");

   Adddefmenu(AMENU_MENU,MSG_SETTINGS_MENU,NULL);
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_LOADIMG,NULL);
   Adddefmenu(AMENU_SUB, MSG_SETTINGS_LOADIMGALL,"@LOADIMGALL");
   Adddefmenu(AMENU_SUB, MSG_SETTINGS_LOADIMGMAPS,"@LOADIMGMAPS");
   Adddefmenu(AMENU_SUB, MSG_SETTINGS_LOADIMGOFF,"@LOADIMGOFF");
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_BGIMAGES,"@BGIMAGES");
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_BGSOUND,"@BGSOUND");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_BROWSER,"SYSTEM AWebPath:AWebCfg BROWSER CONFIG %c PUBSCREEN %n");
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_PROGRAM,"SYSTEM AWebPath:AWebCfg PROGRAM CONFIG %c PUBSCREEN %n");
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_GUI,"SYSTEM AWebPath:AWebCfg GUI CONFIG %c PUBSCREEN %n");
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_NETWORK,"SYSTEM AWebPath:AWebCfg NETWORK CONFIG %c PUBSCREEN %n");
#ifndef NEED35
   if(!has35)
   {  Adddefmenu(AMENU_ITEM,MSG_SETTINGS_CLASSACT,"SYSTEM SYS:Prefs/ClassAct PUBSCREEN %n");
   }
#endif
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_SAVEALL,"SAVESETTINGS");
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_SAVEAS,"SAVESETTINGS REQUEST");
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_LOAD,"LOADSETTINGS REQUEST");
   Adddefmenu(AMENU_ITEM,MSG_SETTINGS_SNAPSHOT,"SNAPSHOT");

   Adddefmenu(AMENU_MENU,MSG_HELP_MENU,NULL);
   Adddefmenu(AMENU_ITEM,MSG_HELP_HELP,"OPEN file:///AWebPath:Docs/AWeb Manual.html");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);
   Adddefmenu(AMENU_ITEM,MSG_HELP_AWEBHOME,"OPEN http://aweb.sunsite.dk");

   Adddefmenu(AMENU_MENU,MSG_AREXX_MENU,NULL);
   Adddefmenu(AMENU_ITEM,MSG_AREXX_AREXX,"@AREXX");
   Adddefmenu(AMENU_SEPARATOR,0,NULL);

#ifdef OSVERSION
   if(!Adduserbutton(&defprefs.gui.buttons,"Modes",
      "RUN AWebPath:plugins/awebmodes.awebrx")) return FALSE;
   if(!Adduserbutton(&defprefs.gui.buttons,"DZone",
      "RUN AWebPath:plugins/dropzone.awebrx")) return FALSE;
   if(!Adduserbutton(&defprefs.gui.buttons,"HTTX",
      "RUN AWebPath:plugins/httx/httxplugincp.awebrx")) return FALSE;
   if(!Adduserbutton(&defprefs.gui.buttons,"ToolBar",
      "RUN AWebPath:plugins/toolbar.awebrx")) return FALSE;
#endif
#ifndef LOCALONLY
   if(!Adduserbutton(&defprefs.gui.buttons,Getmainstr(MSG_USERBUTTON_CACHE),
      "SUBWINDOW CACHEBROWSER OPEN")) return FALSE;
   if(!Adduserbutton(&defprefs.gui.buttons,"News:",
      "OPEN news:")) return FALSE;
#endif
   if(!Adduserbutton(&defprefs.gui.buttons,Getmainstr(MSG_USERBUTTON_CLOCK),
      "SYSTEM SYS:Utilities/Clock pubscreen %n digital format 2 top 0 left 9999")) return FALSE;

   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_IMAGE],
      PUPF_INMEM,Getmainstr(MSG_POPUP_FLUSHIMAGE),"FLUSHCACHE URL %u")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_IMAGE],
      PUPF_NOTINMEM,Getmainstr(MSG_POPUP_LOADIMAGE),"LOAD %u")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_IMAGE],
      PUPF_INMEM,Getmainstr(MSG_POPUP_RELOADIMAGE),"LOAD %u RELOAD")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_IMAGE],
      PUPF_NOTINMEM,Getmainstr(MSG_POPUP_DOWNLOADIMAGE),"LOAD %u SAVEREQ")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_IMAGE],
      PUPF_INMEM,Getmainstr(MSG_POPUP_SAVEIMAGE),"LOAD %u SAVEREQ")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_IMAGE],
      PUPF_INMEM|PUPF_NOTINMEM,Getmainstr(MSG_POPUP_SHOWIMAGE),"OPEN %u")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_IMAGE],
      PUPF_INMEM|PUPF_NOTINMEM,Getmainstr(MSG_POPUP_IMAGE2CLIP),"SETCLIP %u")) return FALSE;

   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_LINK],
      PUPF_INMEM|PUPF_NOTINMEM,Getmainstr(MSG_POPUP_OPENLINK),"OPEN %u TARGET %i")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_LINK],
      PUPF_INMEM|PUPF_NOTINMEM,Getmainstr(MSG_POPUP_OPENLINKNW),"NEW %u")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_LINK],
      PUPF_NOTINMEM,Getmainstr(MSG_POPUP_LOADLINK),"LOAD %u")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_LINK],
      PUPF_NOTINMEM,Getmainstr(MSG_POPUP_DOWNLOADLINK),"LOAD %u SAVEREQ")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_LINK],
      PUPF_INMEM,Getmainstr(MSG_POPUP_SAVELINK),"LOAD %u SAVEREQ")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_LINK],
      PUPF_INMEM|PUPF_NOTINMEM,Getmainstr(MSG_POPUP_ADDLINK),"ADDHOTLIST %u %t")) return FALSE;

   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_FRAME],
      0,Getmainstr(MSG_POPUP_RELOADFRAME),"RELOAD TARGET %i")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_FRAME],
      0,Getmainstr(MSG_POPUP_SAVEFRAME),"SAVEAS TARGET %i")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_FRAME],
      0,Getmainstr(MSG_POPUP_VIEWFRAME),"VIEWSOURCE %u")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_FRAME],
      0,Getmainstr(MSG_POPUP_SHOWFRAME),"OPEN %u")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_FRAME],
      0,Getmainstr(MSG_POPUP_SEARCHFRAME),"SEARCH TARGET %i")) return FALSE;
   if(!Addpopupitem(&defprefs.gui.popupmenu[PUPT_FRAME],
      0,Getmainstr(MSG_POPUP_INFOFRAME),"INFO TARGET %i")) return FALSE;

   Adduserkey(&defprefs.gui.keys,0x45,"CANCEL");
   Adduserkey(&defprefs.gui.keys,0x41,"SCROLL PAGE UP TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x40,"SCROLL PAGE DOWN TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4c,"SCROLL 8 UP TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4d,"SCROLL 8 DOWN TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4e,"SCROLL 8 RIGHT TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4f,"SCROLL 8 LEFT TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4c|UKEY_SHIFT,"SCROLL PAGE UP TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4d|UKEY_SHIFT,"SCROLL PAGE DOWN TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4e|UKEY_SHIFT,"SCROLL PAGE RIGHT TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4f|UKEY_SHIFT,"SCROLL PAGE LEFT TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4c|UKEY_ALT,"SCROLL FAR UP TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4d|UKEY_ALT,"SCROLL FAR DOWN TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x4e|UKEY_ALT,"GO FORWARD");
   Adduserkey(&defprefs.gui.keys,0x4f|UKEY_ALT,"GO BACK");
   Adduserkey(&defprefs.gui.keys,0x3d,"SCROLL FAR UP TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x3e,"SCROLL 8 UP TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x3f,"SCROLL PAGE UP TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x2d,"SCROLL 8 LEFT TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x2f,"SCROLL 8 RIGHT TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x1d,"SCROLL FAR DOWN TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x1e,"SCROLL 8 DOWN TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x1f,"SCROLL PAGE DOWN TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x5f,"OPEN \"file:///AWebPath:Docs/AWeb Manual.html\"");
   Adduserkey(&defprefs.gui.keys,0x7a,"SCROLL 80 UP TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x7b,"SCROLL 80 DOWN TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x7a|UKEY_SHIFT,"SCROLL PAGE UP TARGET %i");
   Adduserkey(&defprefs.gui.keys,0x7b|UKEY_SHIFT,"SCROLL PAGE DOWN TARGET %i");

   if(!(defprefs.gui.navs[0].cmd=Dupstr("GO BACK",-1))) return FALSE;
   if(!(defprefs.gui.navs[1].cmd=Dupstr("GO FORWARD",-1))) return FALSE;
   if(!(defprefs.gui.navs[2].cmd=Dupstr("GO HOME",-1))) return FALSE;
   if(!(defprefs.gui.navs[3].cmd=Dupstr("ADDHOTLIST %u %t",-1))) return FALSE;
   if(!(defprefs.gui.navs[4].cmd=Dupstr("HOTLIST",-1))) return FALSE;
   if(!(defprefs.gui.navs[5].cmd=Dupstr("CANCEL",-1))) return FALSE;
   if(!(defprefs.gui.navs[6].cmd=Dupstr("SUBWINDOW NETSTATUS OPEN",-1))) return FALSE;
   if(!(defprefs.gui.navs[7].cmd=Dupstr("SEARCH TARGET %i",-1))) return FALSE;
   if(!(defprefs.gui.navs[8].cmd=Dupstr("RELOAD",-1))) return FALSE;
   if(!(defprefs.gui.navs[9].cmd=Dupstr("LOADIMAGES",-1))) return FALSE;

   if(AvailMem(MEMF_CHIP)>300*1024) defprefs.network.minfreechip=300;
   if(AvailMem(MEMF_FAST)>500*1024) defprefs.network.minfreefast=500;

   return TRUE;
}

void Freedefprefs(void)
{  void *p;
   short i;
   while(p=REMHEAD(&defprefs.browser.mimelist)) Freemimeinfo(p);
   while(p=REMHEAD(&defprefs.browser.aliaslist)) Freefontalias(p);
   while(p=REMHEAD(&defprefs.gui.buttons)) Freeuserbutton(p);
   while(p=REMHEAD(&defprefs.gui.menus)) Freemenuentry(p);
   while(p=REMHEAD(&defprefs.gui.keys)) Freeuserkey(p);
   for(i=0;i<NRPOPUPMENUS;i++)
   {  while(p=REMHEAD(&defprefs.gui.popupmenu[i])) Freepopupitem(p);
   }
   for(i=0;i<NRNAVBUTTONS;i++)
   {  FREEANDNULL(defprefs.gui.navs[i].cmd);
   }
   FREEANDNULL(defprefs.browser.charset);
}

/* Free a menu entry */
void Freemenuentry(struct Menuentry *me)
{  if(me)
   {  if(me->title) FREE(me->title);
      if(me->cmd) FREE(me->cmd);
      FREE(me);
   }
}

/* Add a new menu entry to the list */
struct Menuentry *Addmenuentry(void *list,UWORD type,UBYTE *title,UBYTE scut,UBYTE *cmd)
{  struct Menuentry *me=ALLOCSTRUCT(Menuentry,1,MEMF_PUBLIC|MEMF_CLEAR);
   if(me
   && (me->title=Dupstr(title,-1))
   && (me->cmd=Dupstr(cmd,-1)))
   {  me->type=type;
      me->scut[0]=scut;
      ADDTAIL(list,me);
      return me;
   }
   else Freemenuentry(me);
   return NULL;
}

/* Free a Userbutton */
void Freeuserbutton(struct Userbutton *ub)
{  if(ub)
   {  if(ub->label) FREE(ub->label);
      if(ub->cmd) FREE(ub->cmd);
      FREE(ub);
   }
}

/* Add a new Userbutton to the list */
struct Userbutton *Adduserbutton(void *list,UBYTE *label,UBYTE *cmd)
{  struct Userbutton *ub=ALLOCSTRUCT(Userbutton,1,MEMF_PUBLIC|MEMF_CLEAR);
   if(ub
   && (ub->label=Dupstr(label,-1))
   && (ub->cmd=Dupstr(cmd,-1)))
   {  ADDTAIL(list,ub);
   }
   else
   {  Freeuserbutton(ub);
      ub=NULL;
   }
   return ub;
}

/* Free a Popupitem */
void Freepopupitem(struct Popupitem *pi)
{  if(pi)
   {  if(pi->title) FREE(pi->title);
      if(pi->cmd) FREE(pi->cmd);
      FREE(pi);
   }
}

/* Add a new Popupitem to the list */
struct Popupitem *Addpopupitem(void *list,UWORD flags,UBYTE *title,UBYTE *cmd)
{  struct Popupitem *pi=ALLOCSTRUCT(Popupitem,1,MEMF_PUBLIC|MEMF_CLEAR);
   if(pi
   && (pi->title=Dupstr(title,-1))
   && (pi->cmd=Dupstr(cmd,-1)))
   {  pi->flags=flags;
      ADDTAIL(list,pi);
   }
   else
   {  Freepopupitem(pi);
      pi=NULL;
   }
   return pi;
}

/* Free a Userkey */
void Freeuserkey(struct Userkey *uk)
{  if(uk)
   {  if(uk->cmd) FREE(uk->cmd);
      FREE(uk);
   }
}

/* Add a new Userkey to the list */
struct Userkey *Adduserkey(void *list,UWORD key,UBYTE *cmd)
{  struct Userkey *uk=ALLOCSTRUCT(Userkey,1,MEMF_PUBLIC|MEMF_CLEAR);
   if(uk
   && (uk->cmd=Dupstr(cmd,-1)))
   {  uk->key=key;
      ADDTAIL(list,uk);
   }
   else
   {  Freeuserkey(uk);
      uk=NULL;
   }
   return uk;
}

/* Find a Userkey */
struct Userkey *Finduserkey(void *list,UWORD key)
{  LIST(Userkey) *ulist=(void *)(LIST(Userkey) *)list;
   struct Userkey *uk;
   for(uk=ulist->first;uk->next;uk=uk->next)
   {  if(uk->key==key) return uk;
   }
   return NULL;
}

/* Free a font alias */
void Freefontalias(struct Fontalias *fa)
{  short i;
   if(fa)
   {  if(fa->alias) FREE(fa->alias);
      for(i=0;i<NRFONTS;i++)
      {  if(fa->fp[i].fontname) FREE(fa->fp[i].fontname);
         if(fa->fp[i].font) CloseFont(fa->fp[i].font);
      }
      FREE(fa);
   }
}

/* Add a font alias, or return existing for this alias */
struct Fontalias *Addfontalias(struct FontaliasList *list,UBYTE *alias)
{  struct Fontalias *fa,*fb;
   for(fa=list->first;fa->next;fa=fa->next)
   {  if(STRIEQUAL(fa->alias,alias)) return fa;
   }
   if((fa=ALLOCSTRUCT(Fontalias,1,MEMF_PUBLIC|MEMF_CLEAR))
   && (fa->alias=Dupstr(alias,-1))
   )
   {  for(fb=list->first;fb->next;fb=fb->next)
      {  if(stricmp(fa->alias,fb->alias)<0) break;
      }
      INSERT(list,fa,fb->prev);
   }
   else
   {  Freefontalias(fa);
      fa=NULL;
   }
   return fa;
}

/* Free a Mimeinfo */
void Freemimeinfo(struct Mimeinfo *mi)
{  if(mi)
   {  if(mi->type) FREE(mi->type);
      if(mi->subtype) FREE(mi->subtype);
      if(mi->extensions) FREE(mi->extensions);
      if(mi->cmd) FREE(mi->cmd);
      if(mi->args) FREE(mi->args);
      FREE(mi);
   }
}

/* Add a new Mimeinfo to the list */
struct Mimeinfo *Addmimeinfo(struct MimeinfoList *list,UBYTE *type,UBYTE *subtype,UBYTE *extensions,
   UWORD driver,UBYTE *cmd,UBYTE *args)
{  struct Mimeinfo *mi=ALLOCSTRUCT(Mimeinfo,1,MEMF_PUBLIC|MEMF_CLEAR);
   struct Mimeinfo *mj,*mk;
   int c;
   if(mi
   && (!type || (mi->type=Dupstr(type,-1)))
   && (!subtype || (mi->subtype=Dupstr(subtype,-1)))
   && (!extensions || (mi->extensions=Dupstr(extensions,-1)))
   && (!cmd || (mi->cmd=Dupstr(cmd,-1)))
   && (!args || (mi->args=Dupstr(args,-1)))
   )
   {  mi->driver=driver;
      if(type && subtype && STRIEQUAL(type,"TEXT") &&
         (STRIEQUAL(subtype,"HTML") || STRIEQUAL(subtype,"PLAIN")))
         mi->deleteable=FALSE;
      else mi->deleteable=TRUE;
      /* If new mimeinfo is empty, add to tail of the list
       * Else insert at sorted place, where subtype * comes last */
      if(!mi->type[0]) ADDTAIL(list,mi);
      else
      {  for(mj=list->first;mj->next;mj=mj->next)
         {  c=stricmp(mi->type,mj->type);
            if(c<0) break;
            if(c==0)
            {  if(STRIEQUAL(mi->subtype,mj->subtype))
               {  mk=mj->next;
                //  REMOVE(mj);
                //  Freemimeinfo(mj);
                  mj=mk;
                  break;
               }
               if(mj->subtype[0]=='*') break;
               if(mi->subtype[0]!='*' && stricmp(mi->subtype,mj->subtype)<0) break;
            }
         }
         INSERT(list,mi,mj->prev);
      }
      return mi;
   }
   Freemimeinfo(mi);
   return NULL;
}

void Freenocache(struct Nocache *nc)
{  if(nc)
   {  if(nc->name) FREE(nc->name);
      if(nc->pattern) FREE(nc->pattern);
      FREE(nc);
   }
}

struct Nocache *Addnocache(struct NocacheList *list,UBYTE *name)
{  struct Nocache *nc=ALLOCSTRUCT(Nocache,1,MEMF_PUBLIC|MEMF_CLEAR);
   struct Nocache *na;
   if(nc
   && (nc->name=Dupstr(name,-1)))
   {  for(na=list->first;na->next;na=na->next)
      {  if(stricmp(na->name,name)>0) break;
      }
      INSERT(list,nc,na->prev);
   }
   else
   {  Freenocache(nc);
      nc=NULL;
   }
   return nc;
}

/* Free all dynamic memory from partial prefs */
void Disposebrowserprefs(struct Browserprefs *p)
{  struct Fontalias *fa;
   struct Mimeinfo *mi;
   short i,j;
   for(j=0;j<2;j++)
   {  for(i=0;i<NRFONTS;i++)
      {  if(p->font[j][i].font) CloseFont(p->font[j][i].font);p->font[j][i].font=NULL;
         FREEANDNULL(p->font[j][i].fontname);
      }
   }
   if(p->aliaslist.first)
   {  while(fa=(struct Fontalias *)REMHEAD(&p->aliaslist)) Freefontalias(fa);
   }
   if(p->mimelist.first)
   {  while(mi=(struct Mimeinfo *)REMHEAD(&p->mimelist)) Freemimeinfo(mi);
   }
   FREEANDNULL(p->charset);
}

void Disposeprogramprefs(struct Programprefs *p)
{  FREEANDNULL(p->savepath);
   FREEANDNULL(p->temppath);
   FREEANDNULL(p->viewcmd);
   FREEANDNULL(p->viewargs);
   FREEANDNULL(p->editcmd);
   FREEANDNULL(p->editargs);
   FREEANDNULL(p->imgvcmd);
   FREEANDNULL(p->imgvargs);
   FREEANDNULL(p->console);
   FREEANDNULL(p->startupscript);
   FREEANDNULL(p->shutdownscript);
   FREEANDNULL(p->screenname);
}

void Disposeguiprefs(struct Guiprefs *p)
{  struct Menuentry *me;
   struct Userbutton *ub;
   struct Popupitem *pi;
   struct Userkey *uk;
   short i;
   if(p->menus.first)
   {  while(me=(struct Menuentry *)REMHEAD(&p->menus)) Freemenuentry(me);
   }
   if(p->buttons.first)
   {  while(ub=(struct Userbutton *)REMHEAD(&p->buttons)) Freeuserbutton(ub);
   }
   for(i=0;i<NRPOPUPMENUS;i++)
   {  if(p->popupmenu[i].first)
      {  while(pi=(struct Popupitem *)REMHEAD(&p->popupmenu[i])) Freepopupitem(pi);
      }
   }
   if(p->keys.first)
   {  while(uk=(struct Userkey *)REMHEAD(&p->keys)) Freeuserkey(uk);
   }
   for(i=0;i<NRNAVBUTTONS;i++)
   {  FREEANDNULL(p->navs[i].cmd);
   }
}

void Disposenetworkprefs(struct Networkprefs *p)
{  void *nc;
   FREEANDNULL(p->homeurl);
   FREEANDNULL(p->localindex);
   FREEANDNULL(p->searchurl);
   FREEANDNULL(p->spoofid);
   FREEANDNULL(p->mailtocmd);
   FREEANDNULL(p->mailtoargs);
   FREEANDNULL(p->telnetcmd);
   FREEANDNULL(p->telnetargs);
   FREEANDNULL(p->newscmd);
   FREEANDNULL(p->newsargs);
   FREEANDNULL(p->starttcpcmd);
   FREEANDNULL(p->starttcpargs);
   FREEANDNULL(p->endtcpcmd);
   FREEANDNULL(p->endtcpargs);
   FREEANDNULL(p->httpproxy);
   FREEANDNULL(p->ftpproxy);
   FREEANDNULL(p->gopherproxy);
   FREEANDNULL(p->telnetproxy);
   FREEANDNULL(p->cachepath);
   FREEANDNULL(p->emailaddr);
   FREEANDNULL(p->replyaddr);
   FREEANDNULL(p->fullname);
   FREEANDNULL(p->organization);
   FREEANDNULL(p->smtphost);
   FREEANDNULL(p->nntphost);
   FREEANDNULL(p->sigfile);
   FREEANDNULL(p->newsauthuser);
   FREEANDNULL(p->newsauthpass);
   FREEANDNULL(p->mailquotehdr);
   FREEANDNULL(p->newsquotehdr);
   if(p->nocache.first)
   {  while(nc=REMHEAD(&p->nocache)) Freenocache(nc);
   }
   if(p->noproxy.first)
   {  while(nc=REMHEAD(&p->noproxy)) Freenoproxy(nc);
   }
   if(p->nocookie.first)
   {  while(nc=REMHEAD(&p->nocookie)) Freenocookie(nc);
   }
}

void Disposecookieprefs(struct Networkprefs *p)
{  void *nc;
   if(p->nocookie.first)
   {  while(nc=REMHEAD(&p->nocookie)) Freenocookie(nc);
   }
}


/* Duplicate all items from (from) to (to) */
BOOL Copybrowserprefs(struct Browserprefs *from,struct Browserprefs *to)
{  short i,j;
   struct Fontalias *fa,*fb;
   struct Mimeinfo *mi;
   NEWLIST(&to->aliaslist);
   NEWLIST(&to->mimelist);
   for(j=0;j<2;j++)
   {  for(i=0;i<NRFONTS;i++)
      {  if(!(to->font[j][i].fontname=Dupstr(from->font[j][i].fontname,-1))) return FALSE;
         to->font[j][i].fontsize=from->font[j][i].fontsize;
         to->font[j][i].font=from->font[j][i].font;
      }
   }
   for(fa=from->aliaslist.first;fa->next;fa=fa->next)
   {  fb=Addfontalias((struct FontaliasList *)&to->aliaslist,fa->alias);
      for(i=0;i<NRFONTS;i++)
      {  if(!(fb->fp[i].fontname=Dupstr(fa->fp[i].fontname,-1))) return FALSE;
         fb->fp[i].fontsize=fa->fp[i].fontsize;
      }
   }
   memmove(to->styles,from->styles,sizeof(to->styles));
   to->newlink=from->newlink;
   to->oldlink=from->oldlink;
   to->selectlink=from->selectlink;
   to->background=from->background;
   to->text=from->text;
   to->screenpens=from->screenpens;
   to->htmlmode=from->htmlmode;
   to->ullink=from->ullink;
   to->imgborder=from->imgborder;
   to->docolors=from->docolors;
   to->dobgsound=from->dobgsound;
   to->blinkrate=from->blinkrate;
   to->dojs=from->dojs;
   if(!(to->charset=Dupstr(from->charset,-1))) return FALSE;
   to->doframes=from->doframes;
   to->inctable=from->inctable;
   to->nominalframe=from->nominalframe;
   to->jserrors=from->jserrors;
   to->nobanners=from->nobanners;
   to->tooltips=from->tooltips;
   to->handpointer=from->handpointer;
   to->jswatch=from->jswatch;
   to->temprp=from->temprp;
   for(mi=from->mimelist.first;mi->next;mi=mi->next)
   {  Addmimeinfo((struct MimeinfoList *)&to->mimelist,mi->type,mi->subtype,mi->extensions,
         mi->driver,mi->cmd,mi->args);
   }
   return TRUE;
}

BOOL Copyprogramprefs(struct Programprefs *from,struct Programprefs *to)
{  if(!(to->savepath=Dupstr(from->savepath,-1))) return FALSE;
   if(!(to->temppath=Dupstr(from->temppath,-1))) return FALSE;
   to->overlap=from->overlap;
   to->commands=from->commands;
   to->hlautoclose=from->hlautoclose;
   to->hlrequester=from->hlrequester;
   to->hlsingleclick=from->hlsingleclick;
   to->whautoclose=from->whautoclose;
   to->saveicons=from->saveicons;
   to->clipdrag=from->clipdrag;
   to->aahotlist=from->aahotlist;
   to->aawinhis=from->aawinhis;
   to->aanetstat=from->aanetstat;
   to->centerreq=from->centerreq;
   if(!(to->viewcmd=Dupstr(from->viewcmd,-1))) return FALSE;
   if(!(to->viewargs=Dupstr(from->viewargs,-1))) return FALSE;
   if(!(to->editcmd=Dupstr(from->editcmd,-1))) return FALSE;
   if(!(to->editargs=Dupstr(from->editargs,-1))) return FALSE;
   if(!(to->imgvcmd=Dupstr(from->imgvcmd,-1))) return FALSE;
   if(!(to->imgvargs=Dupstr(from->imgvargs,-1))) return FALSE;
   if(!(to->console=Dupstr(from->console,-1))) return FALSE;
   if(!(to->startupscript=Dupstr(from->startupscript,-1))) return FALSE;
   if(!(to->shutdownscript=Dupstr(from->shutdownscript,-1))) return FALSE;
   to->screentype=from->screentype;
   if(!(to->screenname=Dupstr(from->screenname,-1))) return FALSE;
   to->screenmode=from->screenmode;
   to->screendepth=from->screendepth;
   to->screenwidth=from->screenwidth;
   to->screenheight=from->screenheight;
   to->loadpalette=from->loadpalette;
   memmove(to->scrpalette,from->scrpalette,sizeof(from->scrpalette));
   memmove(to->scrdrawpens,from->scrdrawpens,sizeof(from->scrdrawpens));
   to->fourcolors=from->fourcolors;
   return TRUE;
}

BOOL Copyguiprefs(struct Guiprefs *from,struct Guiprefs *to)
{  struct Menuentry *me;
   struct Userbutton *ub;
   struct Popupitem *pi;
   struct Userkey *uk;
   short i;
   NEWLIST(&to->menus);
   NEWLIST(&to->buttons);
   for(i=0;i<NRPOPUPMENUS;i++)
   {  NEWLIST(&to->popupmenu[i]);
   }
   NEWLIST(&to->keys);
   to->popupkey=from->popupkey;
   to->showbuttons=from->showbuttons;
   to->shownav=from->shownav;
   to->buttonspos=from->buttonspos;
   to->textbuttons=from->textbuttons;
   to->nobevel=from->nobevel;
   to->windowborder=from->windowborder;
   for(me=from->menus.first;me->next;me=me->next)
   {  if(!Addmenuentry(&to->menus,me->type,me->title,me->scut[0],me->cmd)) return FALSE;
   }
   for(ub=from->buttons.first;ub->next;ub=ub->next)
   {  if(!Adduserbutton(&to->buttons,ub->label,ub->cmd)) return FALSE;
   }
   for(i=0;i<NRPOPUPMENUS;i++)
   {  for(pi=from->popupmenu[i].first;pi->next;pi=pi->next)
      {  if(!Addpopupitem(&to->popupmenu[i],pi->flags,pi->title,pi->cmd)) return FALSE;
      }
   }
   for(uk=from->keys.first;uk->next;uk=uk->next)
   {  if(!Adduserkey(&to->keys,uk->key,uk->cmd)) return FALSE;
   }
   for(i=0;i<NRNAVBUTTONS;i++)
   {  if(!(to->navs[i].cmd=Dupstr(from->navs[i].cmd,-1))) return FALSE;
   }
   return TRUE;
}

BOOL Copynetworkprefs(struct Networkprefs *from,struct Networkprefs *to)
{  struct Nocache *nc;
   NEWLIST(&to->nocookie);
   NEWLIST(&to->noproxy);
   NEWLIST(&to->nocache);
   to->loadimg=from->loadimg;
   to->maxconnect=from->maxconnect;
   to->maxdiskread=from->maxdiskread;
   if(!(to->homeurl=Dupstr(from->homeurl,-1))) return FALSE;
   if(!(to->localindex=Dupstr(from->localindex,-1))) return FALSE;
   if(!(to->searchurl=Dupstr(from->searchurl,-1))) return FALSE;
   if(!(to->spoofid=Dupstr(from->spoofid,-1))) return FALSE;
   to->ignoremime=from->ignoremime;
   to->starthomepage=from->starthomepage;
   to->autosearch=from->autosearch;
   to->contanim=from->contanim;
   to->restrictimages=from->restrictimages;
   if(!Copyoptstring(from->mailtocmd,&to->mailtocmd)) return FALSE;
   if(!Copyoptstring(from->mailtoargs,&to->mailtoargs)) return FALSE;
   if(!Copyoptstring(from->telnetcmd,&to->telnetcmd)) return FALSE;
   if(!Copyoptstring(from->telnetargs,&to->telnetargs)) return FALSE;
   if(!Copyoptstring(from->newscmd,&to->newscmd)) return FALSE;
   if(!Copyoptstring(from->newsargs,&to->newsargs)) return FALSE;
   if(!Copyoptstring(from->starttcpcmd,&to->starttcpcmd)) return FALSE;
   if(!Copyoptstring(from->starttcpargs,&to->starttcpargs)) return FALSE;
   if(!Copyoptstring(from->endtcpcmd,&to->endtcpcmd)) return FALSE;
   if(!Copyoptstring(from->endtcpargs,&to->endtcpargs)) return FALSE;
   if(!Copyoptstring(from->httpproxy,&to->httpproxy)) return FALSE;
   if(!Copyoptstring(from->ftpproxy,&to->ftpproxy)) return FALSE;
   if(!Copyoptstring(from->gopherproxy,&to->gopherproxy)) return FALSE;
   if(!Copyoptstring(from->telnetproxy,&to->telnetproxy)) return FALSE;
   to->limitproxy=from->limitproxy;
   to->passiveftp=from->passiveftp;
   if(!(to->cachepath=Dupstr(from->cachepath,-1))) return FALSE;
   to->camemsize=from->camemsize;
   to->cadisksize=from->cadisksize;
   to->minfreechip=from->minfreechip;
   to->minfreefast=from->minfreefast;
   to->caverify_dyn=from->caverify_dyn;
   to->caverify_stat=from->caverify_stat;
   to->fastresponse_dyn=from->fastresponse_dyn;
   to->fastresponse_stat=from->fastresponse_stat;
   to->cachelocalhost=from->cachelocalhost;
   for(nc=from->nocache.first;nc->next;nc=nc->next)
   {  Addnocache((struct NocacheList *)&to->nocache,nc->name);
   }
   for(nc=from->noproxy.first;nc->next;nc=nc->next)
   {  Addnoproxy((struct NocacheList *)&to->noproxy,nc->name);
   }
   to->referer=from->referer;
   to->formwarn=from->formwarn;
   to->spamblock=from->spamblock;
   to->ftpemailaddr=from->ftpemailaddr;
   to->cookies=from->cookies;
   to->rfc2109=from->rfc2109;
   for(nc=from->nocookie.first;nc->next;nc=nc->next)
   {  Addnocookie((struct NocacheList *)&to->nocookie,nc->name);
   }
   if(!(to->emailaddr=Dupstr(from->emailaddr,-1))) return FALSE;
   if(!(to->replyaddr=Dupstr(from->replyaddr,-1))) return FALSE;
   if(!(to->fullname=Dupstr(from->fullname,-1))) return FALSE;
   if(!(to->organization=Dupstr(from->organization,-1))) return FALSE;
   if(!(to->smtphost=Dupstr(from->smtphost,-1))) return FALSE;
   if(!(to->nntphost=Dupstr(from->nntphost,-1))) return FALSE;
   if(!(to->sigfile=Dupstr(from->sigfile,-1))) return FALSE;
   if(!(to->newsauthuser=Dupstr(from->newsauthuser,-1))) return FALSE;
   if(!(to->newsauthpass=Dupstr(from->newsauthpass,-1))) return FALSE;
   if(!(to->mailquotehdr=Dupstr(from->mailquotehdr,-1))) return FALSE;
   if(!(to->newsquotehdr=Dupstr(from->newsquotehdr,-1))) return FALSE;
   to->extmailer=from->extmailer;
   to->extnewsreader=from->extnewsreader;
   to->maxartnews=from->maxartnews;
   to->framednews=from->framednews;
   to->sortednews=from->sortednews;
   to->longhdrnews=from->longhdrnews;
   to->propnews=from->propnews;
   to->newsbynum=from->newsbynum;
   return TRUE;
}

BOOL Copywindowprefs(struct Windowprefs *from,struct Windowprefs *to)
{  *to=*from;
   return TRUE;
}

/* load prefs into memory. Use name or construct from config name */
void Loadbrowserprefs(struct Browserprefs *bp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/browser",MODE_OLDFILE))
   {  Loadprefs(browsersave,fh,bp);
      Close(fh);
   }
}

void Loadprogramprefs(struct Programprefs *pp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/program",MODE_OLDFILE))
   {  Loadprefs(programsave,fh,pp);
      Close(fh);
   }
}

void Loadguiprefs(struct Guiprefs *gp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/gui",MODE_OLDFILE))
   {  Loadprefs(guisave,fh,gp);
      Close(fh);
   }
}

void Loadnetworkprefs(struct Networkprefs *np,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/network",MODE_OLDFILE))
   {  Loadprefs(networksave,fh,np);
      Close(fh);
   }
   if(fh=Openname(saved,name,"/nocookie",MODE_OLDFILE))
   {  Loadprefs(networksave,fh,np);
      Close(fh);
   }
}

void Loadcookieprefs(struct Networkprefs *np,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/nocookie",MODE_OLDFILE))
   {  Loadprefs(networksave,fh,np);
      Close(fh);
   }
}


void Loadwindowprefs(struct Windowprefs *wp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/window",MODE_OLDFILE))
   {  Loadprefs(windowsave,fh,wp);
      Close(fh);
   }
}

void Loadsettingsprefs(struct Settingsprefs *sp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/settings",MODE_OLDFILE))
   {  Loadprefs(settingssave,fh,sp);
      Close(fh);
   }
}

/* save prefs. Use name or construct from config name */
void Savebrowserprefs(struct Browserprefs *bp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/browser",MODE_NEWFILE))
   {  Saveprefsfmt(browsersave,fh,bp,0);
      Close(fh);
   }
}

void Saveprogramprefs(struct Programprefs *pp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/program",MODE_NEWFILE))
   {  Saveprefsfmt(programsave,fh,pp,0);
      Close(fh);
   }
}

void Saveguiprefs(struct Guiprefs *gp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/gui",MODE_NEWFILE))
   {  Saveprefsfmt(guisave,fh,gp,0);
      Close(fh);
   }
}

void Savenocookieprefs(struct Networkprefs *np,BOOL saved,UBYTE *name)
{
   long fh;
   if(fh=Openname(saved,name,"/nocookie",MODE_NEWFILE))
   {  Saveprefsfmt(networksave,fh,np,1);
      Close(fh);
   }
}

void Savenetworkprefs(struct Networkprefs *np,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/network",MODE_NEWFILE))
   {  Saveprefsfmt(networksave,fh,np,0);
      Close(fh);
   }
}

void Savewindowprefs(struct Windowprefs *wp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/window",MODE_NEWFILE))
   {  Saveprefsfmt(windowsave,fh,wp,0);
      Close(fh);
   }
}

void Savesettingsprefs(struct Settingsprefs *sp,BOOL saved,UBYTE *name)
{  long fh;
   if(fh=Openname(saved,name,"/settings",MODE_NEWFILE))
   {  Saveprefsfmt(settingssave,fh,sp,0);
      Close(fh);
   }
}

void Copyprefs(struct AwebPrefs *from,struct AwebPrefs *to)
{  Copybrowserprefs(&from->browser,&to->browser);
   Copyprogramprefs(&from->program,&to->program);
   Copyguiprefs(&from->gui,&to->gui);
   Copynetworkprefs(&from->network,&to->network);
}

void Saveprefs(struct AwebPrefs *prefs)
{  Savebrowserprefs(&prefs->browser,FALSE,NULL);
   Saveprogramprefs(&prefs->program,FALSE,NULL);
   Saveguiprefs(&prefs->gui,FALSE,NULL);
   Savenetworkprefs(&prefs->network,FALSE,NULL);
   Savenocookieprefs(&prefs->network,FALSE,NULL);
}

void Disposeprefs(struct AwebPrefs *prefs)
{  Disposebrowserprefs(&prefs->browser);
   Disposeprogramprefs(&prefs->program);
   Disposeguiprefs(&prefs->gui);
   Disposenetworkprefs(&prefs->network);
}
