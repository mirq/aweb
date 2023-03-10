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

/* aweb.c aweb main */
#if defined(__amigaos4__)
#define __USE_OLD_TIMEVAL__
#endif
#include "aweb.h"
#include "url.h"
#include "application.h"
#include "window.h"
#include "source.h"
#include "frame.h"
#include "jslib.h"
#include "task.h"
#include "splashimages.h"
#include "versions.h"
#include <signal.h>

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/gadgetclass.h>
#include <dos/dostags.h>
#include <dos/rdargs.h>
#include <dos/dosextens.h>
#include <exec/execbase.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <utility/date.h>
#include <libraries/locale.h>
#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/icon.h>
#include <proto/locale.h>
#include <proto/layers.h>
#include <proto/graphics.h>
#include <proto/utility.h>

#if defined(__amigaos4__)
#include <proto/Picasso96API.h>
#else
#include <proto/cybergraphics.h>
#endif

#include <proto/awebstartup.h>


#if defined(__amigaos4__)

unsigned int __stack_size = 65356;

#elif defined(__MORPHOS__)

long __stack = 64*1024;

#else

#ifdef __SASC

__near

#endif

long __stack_size = 16384;

#endif


#ifdef DEVELOPER
#define PROFILE
extern void Setoodebug(UBYTE *types);
extern void Setoomethod(UBYTE *types);
extern void Setoodelay(void);
extern BOOL ookdebug;
extern BOOL charsetdebug;
#endif
#include "profile.h"

struct Library *DiskfontBase = NULL,*LayersBase = NULL,
   *ColorWheelBase = NULL,*GadToolsBase = NULL,*DataTypesBase = NULL,*AslBase = NULL,*KeymapBase = NULL,*IconBase = NULL,
   *GradientSliderBase = NULL,*IFFParseBase = NULL,*WorkbenchBase = NULL,*CyberGfxBase = NULL;

struct AWebJSBase *AWebJSBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;

#if defined(__amigaos4__) || defined(__MORPHOS__)
struct Library *LocaleBase = NULL;
#else
struct LocaleBase *LocaleBase = NULL;

#endif

#ifdef __MORPHOS__
struct Library *UtilityBase = NULL;
#else
struct UtilityBase *UtilityBase =NULL;
#endif


struct Library *WindowBase = NULL,*LayoutBase =NULL,*ButtonBase = NULL,*ListBrowserBase = NULL,
   *ChooserBase = NULL,*IntegerBase = NULL,*SpaceBase = NULL,*CheckBoxBase = NULL,*StringBase = NULL,
   *LabelBase = NULL,*PaletteBase = NULL,*GlyphBase = NULL,*ClickTabBase = NULL,*FuelGaugeBase = NULL,
   *BitMapBase = NULL,*BevelBase = NULL,*DrawListBase = NULL,*SpeedBarBase = NULL,*ScrollerBase = NULL,
   *PenMapBase = NULL;

struct Library *AwebStartupBase = NULL;

#if defined(__MORPHOS__)
struct Library *ZBase;
#endif

#if defined(__amigaos4__)

struct Library *P96Base = NULL;
/* Interfaces */

struct AWebJSIFace *IAWebJS = NULL;
struct AwebStartupIFace *IAwebStartup = NULL;
struct IntuitionIFace *IIntuition = NULL;
struct GraphicsIFace *IGraphics = NULL;
struct LocaleIFace *ILocale = NULL;
struct UtilityIFace *IUtility = NULL;

struct DiskfontIFace *IDiskfont = NULL;
struct LayersIFace *ILayers = NULL;
struct ColorWheelIFace *IColorWheel = NULL;
struct GadToolsIFace *IGadTools = NULL;
struct DataTypesIFace *IDataTypes = NULL;
struct AslIFace *IAsl = NULL;
struct KeymapIFace *IKeymap = NULL;
struct IconIFace *IIcon = NULL;
struct GradientSliderIFace *IGradientSlider = NULL;
struct IFFParseIFace *IIFFParse = NULL;
struct WorkbenchIFace *IWorkbench = NULL;
struct CyberGfxIFace *ICyberGfx = NULL;

struct WindowIFace *IWindow = NULL;
struct LayoutIFace *ILayout = NULL;
struct ButtonIFace *IButton = NULL;
struct ListBrowserIFace *IListBrowser = NULL;
struct ChooserIFace *IChooser = NULL;
struct IntegerIFace *IInteger = NULL;
struct SpaceIFace   *ISpace = NULL;
struct CheckBoxIFace *ICheckBox = NULL;
struct StringIFace *IString = NULL;
struct LabelIFace  *ILabel = NULL;
struct PaletteIFace *IPalette = NULL;
struct GlyphIFace  *IGlyph = NULL;
struct ClickTabIFace *IClickTab = NULL;
struct FuelGaugeIFace *IFuelGauge = NULL;
struct BitMapIFace *IBitMap = NULL;
struct BevelIFace *IBevel = NULL;
struct DrawListIFace *IDrawList = NULL;
struct SpeedBarIFace *ISpeedBar = NULL;
struct ScrollerIFace *IScroller = NULL;
struct PenMapIFace *IPenMap = NULL;

struct P96IFace *IP96 = NULL;

#else

static void *IDummy = NULL;

#define IAWebJS IDummy

#define IIntuition IDummy
#define IGraphics IDummy
#define ILocale IDummy
#define IUtility IDummy

#define IDiskfont IDummy
#define ILayers IDummy
#define IColorWheel IDummy
#define IGadTools IDummy
#define IDataTypes IDummy
#define IAsl IDummy
#define IKeymap IDummy
#define IIcon IDummy
#define IGradientSlider IDummy
#define IIFFParse IDummy
#define IWorkbench IDummy
#define ICyberGfx IDummy

#define IWindow IDummy
#define ILayout IDummy
#define IButton IDummy
#define IListBrowser IDummy
#define IChooser IDummy
#define IInteger IDummy
#define ISpace IDummy
#define ICheckBox IDummy
#define IString IDummy
#define ILabel IDummy
#define IPalette IDummy
#define IGlyph IDummy
#define IClickTab IDummy
#define IFuelGauge IDummy
#define IBitMap IDummy
#define IBevel IDummy
#define IDrawList IDummy
#define ISpeedBar IDummy
#define IScroller IDummy
#define IPenMap IDummy

#endif

struct LocaleInfo localeinfo;
struct Locale *locale;
void *maincatalog;

static BOOL quit=FALSE;
static BOOL icondefer=FALSE,iconify;
static BOOL openedca=FALSE;
static BOOL updateframes=FALSE;
static BOOL flushsources=FALSE;
static BOOL awebpath=FALSE;

BOOL httpdebug=FALSE;
BOOL specdebug=FALSE;
BOOL usetemp=FALSE;
long localblocksize=INPUTBLOCKSIZE;
BOOL profile=FALSE;
BOOL nopool=FALSE;
BOOL has35=FALSE;
BOOL haiku=FALSE;
BOOL noiconify=FALSE;

UBYTE *initialurls[16];
UBYTE localinitialurl[16];

#define AWEBCONTROLPORTNAME   "AWebControlPort"

static struct MsgPort *awebcontrolport;

struct Awebcontrolmsg
{  struct Message msg;
   UBYTE **urls;
   UBYTE *local;
};

UBYTE programname[256];

struct PathList
{  BPTR next;
   BPTR lock;
};

struct PathList *ourpathlist;

struct Clipinfo
{  struct Layer *layer;
   struct Region *region;
   struct Region *oldregion;
   struct Window *window;     /* only if refreshing */
};

struct Clipcoords
{  struct Coords coords;
   ULONG clipkey;
};

static UBYTE days[7][4]=
{  "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
static UBYTE months[12][4]=
{  "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

static struct Aobject *aweb;

static ULONG waitmask=0;
static void (*processfun[16])(void);

/* General purpose notify-when-back-in-main queue */
struct Smqnode
{  NODE(Smqnode);
   void *object;
   ULONG queueid;
   ULONG userdata;
   BOOL selected;
};
static LIST(Smqnode) setmsgqueue;

/* Generic JS event handler invokations */
UBYTE *awebonclick="if(this.onclick) return this.onclick();";
UBYTE *awebonchange="if(this.onchange) return this.onchange();";
UBYTE *awebonblur="if(this.onblur) return this.onblur();";
UBYTE *awebonfocus="if(this.onfocus) return this.onfocus();";
UBYTE *awebonmouseover="if(this.onmouseover) return this.onmouseover();";
UBYTE *awebonmouseout="if(this.onmouseout) return this.onmouseout();";
UBYTE *awebonreset="if(this.onreset) return this.onreset();";
UBYTE *awebonsubmit="if(this.onsubmit) return this.onsubmit();";
UBYTE *awebonselect="if(this.onselect) return this.onselect();";
UBYTE *awebonload="if(this.onload) return this.onload();";
UBYTE *awebonunload="if(this.onunload) return this.onunload();";
UBYTE *awebonerror="if(this.onerror) return this.onerror();";
UBYTE *awebonabort="if(this.onabort) return this.onabort();";


/*-----------------------------------------------------------------------*/

/* 0x01 = printable
   0x02 = alphabetic
   0x04 = numeric
   0x08 =
   0x10 = url (7-bit-alphanumeric . - _ @)
   0x20 = space (space,cr,nl,ff,tab,nbsp)
   0x40 = sgml (low-ascii non separator)
*/

static UBYTE istab[]=
{  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x00,0x20,0x20,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x21,0x41,0x01,0x41,0x41,0x41,0x01,0x01,0x41,0x41,0x51,0x41,0x01,0x51,0x51,0x41,
   0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x41,0x01,0x01,0x01,0x01,0x41,
   0x51,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,
   0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x41,0x41,0x41,0x41,0x51,
   0x01,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,
   0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x53,0x41,0x41,0x41,0x41,0x00,
   0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,
   0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x01,
   0x21,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
   0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
   0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x01,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
   0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
   0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x01,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
};

BOOL Isurlchar(UBYTE c)
{  if(istab[c]&0x10) return TRUE;
   else return FALSE;
}

BOOL Isprint(UBYTE c)
{  if(istab[c]&0x01) return TRUE;
   else return FALSE;
}

BOOL Isspace(UBYTE c)
{  return BOOLVAL(istab[c]&0x20);
}

BOOL Issgmlchar(UBYTE c)
{  return BOOLVAL(istab[c]&0x40);
}

BOOL Isalpha(UBYTE c)
{  return BOOLVAL(istab[c]&0x02);
}

BOOL Isalnum(UBYTE c)
{  return BOOLVAL(istab[c]&0x06);
}

/*-----------------------------------------------------------------------*/


/**
 * ensures a buffer has sufficient space to add size bytes to it
 *
 * buf->length + size will be greater or equal to than buf->size
 * after this call.
 *
 * @param buf the buffer
 * @param size the minimum excess size required
 */
BOOL Expandbuffer(struct Buffer *buf,ULONG size)
{

   if(buf->length+size>buf->size || buf->size==0)
   {
      UBYTE *newbuf;
      long newsize=
         ((buf->length+size+TEXTBLOCKSIZE-1)/TEXTBLOCKSIZE)*TEXTBLOCKSIZE;
      if(newsize < TEXTBLOCKSIZE)newsize=TEXTBLOCKSIZE;
      newbuf=ALLOCTYPE(UBYTE,newsize,0);
      if(!newbuf) return FALSE;
      if(buf->size)
      {  memmove(newbuf,buf->buffer,buf->size);
         FREE(buf->buffer);
      }
      buf->buffer=newbuf;
      buf->size=newsize;
   }
   return TRUE;
}

void Freebuffer(struct Buffer *buf)
{  if(buf->buffer) FREE(buf->buffer);
   buf->buffer=NULL;
   buf->size=buf->length=0;
}

/**
 * Inserts a block of bytes in a buffer, expanding it if necesary
 *
 * beware that buf->buffer may change, check your pointers!
 *
 * @param buf the buffer
 * @param text the bytes to add
 * @param length the number of bytes to add
 * @param pos the position in the buffer to insert
 * @return true if insertion successful
 */
BOOL Insertinbuffer(struct Buffer *buf,UBYTE *text,ULONG length,ULONG pos)
{  if (pos > buf->length) {
      //KPrintF("*************Insertinbuffer pos %ld greater than length %ld!\n", pos, buf->length);
      pos = buf->length;
   }
   if(Expandbuffer(buf,length))
   {  if(pos<buf->length)
         memmove(buf->buffer+pos+length,buf->buffer+pos,buf->length-pos);
      if(length) memmove(buf->buffer+pos,text,length);
      buf->length+=length;
      return TRUE;
   }
   return FALSE;
}

BOOL Addtobuffer(struct Buffer *buf,UBYTE *text,long length)
{
   if(text == NULL) printf("Warning: Null text passed to Addtobuffer\n");
   if(length<0) length=strlen(text);
   return Insertinbuffer(buf,text,length,buf->length);
}

/**
 * deletes a section from a buffer
 *
 * @param buf the buffer to delete from
 * @param pos position in the buffer to start deleting
 * @param length number of bytes to remove
 */
void Deleteinbuffer(struct Buffer *buf,ULONG pos,ULONG length)
{  /*if (pos > buf->length) {
      KPrintF("****************** Deleteinbuffer pos %ld > buf->length %ld\n", pos, buf->length);
   }
   if (pos + length > buf->length) {
      KPrintF("****************** Deleteinbuffer pos %ld + length %ld > buf->length %ld\n", pos, length, buf->length);
   }*/
   if(pos+length>buf->length) length=buf->length-pos;
   if(pos+length<buf->length)
   {  memmove(buf->buffer+pos,buf->buffer+pos+length,
         buf->length-pos-length);
   }
   buf->length-=length;
}

VARARGS68K_DECLARE(void Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...))
{
   VA_LIST va;
   struct TagItem *tags;

   VA_STARTLIN(va,req);
   tags = (struct TagItem *)VA_GETLIN(va, struct TagItem *);
   if(SetGadgetAttrsA(gad,win,req,tags)) RefreshGList(gad,win,req,1);
   VA_END(va);
}

long Getvalue(struct Gadget *gad,ULONG tag)
{  long value=0;
   GetAttr(tag,gad,(ULONG *)&value);
   return value;
}

BOOL Getselected(struct Gadget *gad)
{  return (BOOL)Getvalue(gad,GA_Selected);
}

UBYTE *Dupstr(UBYTE *str,long length)
{  UBYTE *dup;
   if(!str) return NULL;
   if(length<0) length=strlen(str);
   if(dup=ALLOCTYPE(UBYTE,length+1,0))
   {  memmove(dup,str,length);
      dup[length]='\0';
   }
   return dup;
}

void AddtagstrA(struct Buffer *buf,UBYTE *keywd,UWORD f,ULONG value)
{  UBYTE *p;
   UBYTE b[16];
   if(keywd)
   {  if(buf->buffer && buf->buffer[buf->length-1]!='>' && *keywd!='<' && *keywd!='>')
      {  Addtobuffer(buf," ",1);
      }
      Addtobuffer(buf,keywd,strlen(keywd));
   }
   switch(f)
   {  case ATSF_STRING:
         Addtobuffer(buf,"=\"",2);
         for(p=(UBYTE *)value;*p;p++)
         {  if(*p=='"') Addtobuffer(buf,"&quot;",6);
            else Addtobuffer(buf,p,1);
         }
         Addtobuffer(buf,"\"",1);
         break;
      case ATSF_NUMBER:
         sprintf(b,"=\"%d\"",(int)value);
         Addtobuffer(buf,b,strlen(b));
         break;
   }
}

/* This must not be done like this for OS4 !!!! FIXME FIXME FIXME use AllocCmdPathList et al */
/* needn't remove memf_public as all code will be replace for os4 */

long Copypathlist(void)
{  struct PathList *pl,*plfirst=NULL,*pllast=NULL,*plnew;
   for(pl=ourpathlist;pl;pl=(struct PathList *)BADDR(pl->next))
   {  if(plnew=(struct PathList *)AllocVec(sizeof(struct PathList),MEMF_PUBLIC))
      {  if(plnew->lock=DupLock(pl->lock))
         {  plnew->next=(BPTR)NULL;
            if(pllast) pllast->next=MKBADDR(plnew);
            else plfirst=plnew;
            pllast=plnew;
         }
         else FreeVec(plnew);
      }
   }
   return MKBADDR(plfirst);
}

void Freepathlist(long plbptr)
{  struct PathList *pl,*plnext;
   for(pl=BADDR(plbptr);pl;pl=plnext)
   {  plnext=BADDR(pl->next);
      if(pl->lock) UnLock(pl->lock);
      FreeVec(pl);
   }
}

long Pformatlength(UBYTE *format,UBYTE *argspec,UBYTE **params)
{  long len=0;
   UBYTE *s=format;  /* source */
   UBYTE *a,*p;
   BOOL quoted=FALSE;
   while(*s)
   {  if(*s=='%' && s[1])
      {  if(a=strchr(argspec,s[1]))
         {  if(!quoted) len++;
            for(p=params[a-argspec];p && *p;p++)
            {  if(*p=='"' || *p=='*') len++;
               len++;
            }
            if(!quoted) len++;
            s++; /* skip %, param specifier is skipped at end of loop */
         }
         else len++;
      }
      else
      {  if(*s=='"') quoted=!quoted;
         len++;
      }
      s++;
   }
   return len;
}

UBYTE *Pformat(UBYTE *buffer,UBYTE *format,UBYTE *argspec,UBYTE **params,BOOL quote)
{  UBYTE *d=buffer;  /* destination */
   UBYTE *s=format;  /* source */
   UBYTE *a,*p;
   BOOL quoted=FALSE;
   while(*s)
   {  if(*s=='%' && s[1])
      {  if(a=strchr(argspec,s[1]))
         {  if(quote && !quoted) *d++='"';
            for(p=params[a-argspec];p && *p;p++)
            {  if(*p=='"' || *p=='*') *d++='*';
               *d++=*p;
            }
            if(quote && !quoted) *d++='"';
            s++; /* skip %, param specifier is skipped at end of loop */
         }
         else *d++=*s;
      }
      else
      {  if(*s=='"') quoted=!quoted;
         *d++=*s;
      }
      s++;
   }
   *d='\0';
   return d;
}

VARARGS68K_DECLARE(BOOL  Spawn(BOOL del,UBYTE *cmd,UBYTE *args,UBYTE *argspec,...))
{
   VA_LIST va;
   UBYTE **params;
   UBYTE *conparams[2];
   BOOL result=FALSE;
   long out,len,conlen;
   UBYTE *buffer = NULL;
   UBYTE *conbuf = NULL;
   long pl=Copypathlist();
   long lock;
   __aligned struct FileInfoBlock fib={0};
   BOOL script=FALSE;

   VA_STARTLIN(va,argspec);
   params = (UBYTE **)VA_GETLIN(va, UBYTE **);

   if(lock=Lock(cmd,SHARED_LOCK))
   {  if(Examine(lock,&fib))
      {  if(fib.fib_Protection&FIBF_SCRIPT) script=TRUE;
      }
      UnLock(lock);
   }
   len=20+strlen(cmd)+1+Pformatlength(args,argspec,params);
   if(del) len+=24+strlen(params[0]);
   conparams[0]=AWEBSTR(MSG_AWEB_EXTWINTITLE);
   conparams[1]=(UBYTE *)Agetattr(Aweb(),AOAPP_Screenname);
   conlen=2+Pformatlength(prefs.program.console,"tn",conparams);
   if((buffer=ALLOCTYPE(UBYTE,len,0))
   && (conbuf=ALLOCTYPE(UBYTE,conlen,0)))
   {  strcpy(buffer,"failat 30\n");
      if(!script) strcat(buffer,"\"");
      strcat(buffer,cmd);
      if(!script) strcat(buffer,"\"");
      strcat(buffer," ");
      Pformat(buffer+strlen(buffer),args,argspec,params,TRUE);
      if(del) sprintf(buffer+strlen(buffer),"\ndelete \"%s\" quiet",params[0]);
      Pformat(conbuf,prefs.program.console,"tn",conparams,FALSE);
      if(!(out=Open(conbuf,MODE_NEWFILE)))
      {  out=Open("NIL:",MODE_NEWFILE);
      }
      if(out && 0<=SystemTags(buffer,
         SYS_Input,out,
         SYS_Output,0,
         SYS_Asynch,TRUE,
         NP_Path,pl,
         TAG_END)) result=TRUE;
      if(!result)
      {  if(out) Close(out);
         if(pl) Freepathlist(pl);
      }
   }
   VA_END(va);
   if(buffer) FREE(buffer);
   if(conbuf) FREE(conbuf);
   return result;
}

static BOOL Openinitialdoc(UBYTE *initialurl,UBYTE local,BOOL veryfirst)
{  void *win;
   struct Url *url;
   UBYTE *urlname,*fragment;
   BOOL opened=FALSE;
   if(local)
   {  if(urlname=ALLOCTYPE(UBYTE,strlen(initialurl)+20,0))
      {  strcpy(urlname,"file://localhost/");
         strcat(urlname,initialurl);
         url=Findurl("",urlname,0);
         FREE(urlname);
      }
      else url=NULL;
   }
   else url=Findurl("",initialurl,0);
   fragment=Fragmentpart(initialurl);
   if(url)
   {  if(veryfirst) win=Firstwindow();
      else win=Anewobject(AOTP_WINDOW,TAG_END);
      if(win) Inputwindoc(win,url,fragment,0);
      opened=TRUE;
   }
   return opened;
}

static void Processcontrol(void)
{  struct Awebcontrolmsg *amsg;
   short i;
   while(amsg=(struct Awebcontrolmsg *)GetMsg(awebcontrolport))
   {  Iconify(FALSE);
      for(i=0;i<16;i++)
      {  if(amsg->urls[i])
            Openinitialdoc(amsg->urls[i],amsg->local[i],FALSE);
      }
      ReplyMsg((struct Message *)amsg);
   }
   Asetattrs(Aweb(),AOAPP_Tofront,TRUE,TAG_END);
}

static BOOL Dupstartupcheck(void)
{  struct Awebcontrolmsg amsg={{{0}}};
   struct MsgPort *port;
   short i;
   if(awebcontrolport=CreateMsgPort())
   {  Forbid();
      if(port=FindPort(AWEBCONTROLPORTNAME))
      {  amsg.msg.mn_ReplyPort=awebcontrolport;
         amsg.urls=initialurls;
         amsg.local=localinitialurl;
         PutMsg(port,(struct Message *)&amsg);
      }
      else
      {  awebcontrolport->mp_Node.ln_Name=AWEBCONTROLPORTNAME;
         AddPort(awebcontrolport);
         Setprocessfun(awebcontrolport->mp_SigBit,Processcontrol);
      }
      Permit();
      if(port)
      {  WaitPort(awebcontrolport);
         GetMsg(awebcontrolport);
         DeleteMsgPort(awebcontrolport);
         awebcontrolport=NULL;
         for(i=0;i<16;i++)
         {  if(initialurls[i]) FREE(initialurls[i]);
         }
         return FALSE;
      }
      else return TRUE;
   }
   return FALSE;
}

UBYTE *Getmainstr(ULONG msg)
{  return AWEBSTR(msg);
}

/* Locale hook. (hook)->h_Data points at next buffer position. */
DECLARE_HOOK
(
    static void __saveds, Lputchar,
    struct Hook *, hook, A0,
    APTR, dummy, A2,
    UBYTE    *    , c   , A1
)
{
   USRFUNC_INIT

   UBYTE *p=hook->h_Data;
   *p++=c;
   hook->h_Data=p;

   USRFUNC_EXIT
}

long LprintfA(UBYTE *buffer,UBYTE *fmt,void *args)
{  struct Hook hook;
   hook.h_Entry=(HOOKFUNC)GIVEME_HOOKENTRY(Lputchar);
   hook.h_Data=buffer;
   FormatString(locale,fmt,args,&hook);
   return (UBYTE *)hook.h_Data-buffer;
}

VARARGS68K_DECLARE(long Lprintf(UBYTE *buffer,UBYTE *fmt,...))
{
   VA_LIST va;
   void *args;
   long result;
   VA_STARTLIN(va,fmt);
   args = (void *)VA_GETLIN(va,void *);
   result = LprintfA(buffer,fmt,args);
   VA_END(va);
   return result;
}

long Lprintdate(UBYTE *buffer,UBYTE *fmt,struct DateStamp *ds)
{  struct Hook hook;
   hook.h_Entry=(HOOKFUNC)GIVEME_HOOKENTRY(Lputchar);
   hook.h_Data=buffer;
   FormatDate(locale,fmt,ds,&hook);
   return (UBYTE *)hook.h_Data-buffer;
}

ULONG Clipto(struct RastPort *rp,short minx,short miny,short maxx,short maxy)
{  struct Clipinfo *ci=ALLOCSTRUCT(Clipinfo,1,MEMF_CLEAR);
   struct Rectangle rect;
   rect.MinX=minx;
   rect.MinY=miny;
   rect.MaxX=maxx;
   rect.MaxY=maxy;
   if(ci)
   {  if(ci->region=NewRegion())
      {  if(OrRectRegion(ci->region,&rect))
         {  ci->layer=rp->Layer;
            if((ci->window=rp->Layer->Window)
             && Agetattr((void *)ci->window->UserData,AOWIN_Refreshing))
            {  EndRefresh(ci->window,FALSE);
            }
            else ci->window=NULL;
            LockLayerInfo(ci->layer->LayerInfo);
            LockLayer(0,ci->layer);
            ci->oldregion=InstallClipRegion(ci->layer,ci->region);
            if(ci->window) BeginRefresh(ci->window);
         }
      }
   }
   return (ULONG)ci;
}

void Unclipto(ULONG p)
{  struct Clipinfo *ci=(struct Clipinfo *)p;
   if(ci)
   {  if(ci->region)
      {  if(ci->layer)
         {  if(ci->window) EndRefresh(ci->window,FALSE);
            InstallClipRegion(ci->layer,ci->oldregion);
            UnlockLayer(ci->layer);
            UnlockLayerInfo(ci->layer->LayerInfo);
            if(ci->window) BeginRefresh(ci->window);
         }
         DisposeRegion(ci->region);
      }
      FREE(ci);
   }
}

struct Coords *Clipcoords(void *cframe,struct Coords *coo)
{  struct Clipcoords *clc;
   if(!coo)
   {  clc=ALLOCSTRUCT(Clipcoords,1,MEMF_CLEAR);
      Framecoords(cframe,(struct Coords *)clc);
      if(clc->coords.win && clc->coords.rp)
      {  clc->clipkey=Clipto(clc->coords.rp,clc->coords.minx,clc->coords.miny,clc->coords.maxx,clc->coords.maxy);
      }
      coo=(struct Coords *)clc;
      if(coo) coo->nestcount=1;
   }
   if(coo && coo->nestcount) coo->nestcount++;
   return coo;
}

void Unclipcoords(struct Coords *coo)
{  struct Clipcoords *clc=(struct Clipcoords *)coo;
   if(coo)
   {  coo->nestcount--;
      if(coo->nestcount==1)
      {  if(clc->clipkey) Unclipto(clc->clipkey);
         FREE(clc);
      }
   }
}

ULONG Today(void)
{  ULONG sec,mic;
   CurrentTime(&sec,&mic);
   return (ULONG)(sec+60*locale->loc_GMTOffset);
}

static UBYTE *Getdatetoken(UBYTE *p,UBYTE *token)
{  while(*p && !isalnum(*p)) p++;
   while(*p && isalnum(*p)) *token++=*p++;
   *token='\0';
   return p;
}


ULONG Scandate(UBYTE *buf)
{  UBYTE *p;
   UBYTE token[16];
   struct ClockData cd={0};
   BOOL ok,first=TRUE;
   short i;
   ULONG stamp;
   p=buf;


   for(;;)
   {  p=Getdatetoken(p,token);
      if(!*token) break;
      ok=FALSE;
      for(i=0;i<12;i++)
      {  if(STRNIEQUAL(token,months[i],3))
         {  cd.month=i+1;
            ok=TRUE;
            break;
         }
      }
      if(ok) continue;
      if(*p==':')
      {  sscanf(token,"%hd",&i);
         cd.hour=i;


         p=Getdatetoken(p,token);

         sscanf(token,"%hd",&i);
         cd.min=i;
         p=Getdatetoken(p,token);
         sscanf(token,"%hd",&i);
         cd.sec=i;
         continue;
      }
      if(sscanf(token,"%hd",&i) > 0)
      {  if(first)
         {  cd.mday=i;
            first=FALSE;
         }
         else
         {  if(i<78) i+=2000;          /* e.g. 01 -> 2001 */
            else if(i<1978) i+=1900;   /* e.g. 96 -> 1996 */
                                       /* else: e.g. 1996 */
            cd.year=i;
         }
      }

   }
   stamp=CheckDate(&cd);
   /* In case of invalid date, make sure it is in the past */
   if(!stamp) stamp=1;
   return stamp;
}

void Makedate(ULONG stamp,UBYTE *buf)
{  struct ClockData cd={0};
   Amiga2Date(stamp,&cd);
   sprintf(buf,"%s, %02d %s %4d %02d:%02d:%02d GMT",
      days[cd.wday],cd.mday,months[cd.month-1],cd.year,cd.hour,cd.min,cd.sec);
}

struct Aobject *Aweb(void)
{  return aweb;
}

void Safeclosewindow(struct Window *w)
{  struct IntuiMessage *imsg;
   struct Node *succ;
   Forbid();
   if(w->UserPort)
   {  imsg=(struct IntuiMessage *)w->UserPort->mp_MsgList.lh_Head;
      while(succ=imsg->ExecMessage.mn_Node.ln_Succ)
      {  if(imsg->IDCMPWindow==w)
         {  Remove((struct Node *)imsg);
            ReplyMsg((struct Message *)imsg);
         }
         imsg=(struct IntuiMessage *)succ;
      }
   }
   w->UserPort=NULL;
   ModifyIDCMP(w,0);
   Permit();
   CloseWindow(w);
}

UBYTE *Savepath(void *url)
{  UBYTE *path,*name=NULL,*fname;
   long len;
   if(url && (path=(UBYTE *)Agetattr(Aweb(),AOAPP_Savepath)))
   {  if(fname=(UBYTE *)Agetattr(url,AOURL_Url))
      {  fname=Urlfilename(fname);
      }
      if(!fname || !*fname) fname=Dupstr(prefs.network.localindex,-1);
      len=strlen(path);
      if(fname) len+=strlen(fname);
      if(name=ALLOCTYPE(UBYTE,len+4,0))
      {  strcpy(name,path);
         AddPart(name,fname?fname:NULLSTRING,len+3);
      }
      if(fname) FREE(fname);
   }
   return name;
}

void Setprocessfun(short sigbit,void (*fun)(void))
{  if(sigbit>=16 && sigbit<32)
   {  processfun[sigbit-16]=fun;
      if(fun) waitmask|=1<<sigbit;
      else waitmask&=~(1<<sigbit);
   }
}

UBYTE *Fullname(UBYTE *name)
{  long lock=Lock(name,SHARED_LOCK);
   UBYTE *buf=NULL;
   if(lock)
   {  if(buf=ALLOCTYPE(UBYTE,STRINGBUFSIZE+1,0))
      {  if(!NameFromLock(lock,buf,STRINGBUFSIZE))
         {  FREE(buf);
            buf=NULL;
         }
      }
      UnLock(lock);
   }
   return buf;
}

BOOL Readonlyfile(UBYTE *name)
{  long lock;
   __aligned struct InfoData id={0};
   struct FileInfoBlock *fib;
   BOOL readonly=FALSE;
   if(lock=Lock(name,SHARED_LOCK))
   {  if(Info(lock,&id))
      {  readonly=(id.id_DiskState!=ID_VALIDATED);
      }
      if(!readonly && (fib=AllocDosObjectTags(DOS_FIB,TAG_END)))
      {  if(Examine(lock,fib))
         {  readonly=(fib->fib_DirEntryType>=0) || (fib->fib_Protection&FIBF_DELETE);
         }
         FreeDosObject(DOS_FIB,fib);
      }
      UnLock(lock);
   }
   else readonly=TRUE;
   return readonly;
}

void Changedlayout(void)
{  updateframes=TRUE;
}

void Deferflushmem(void)
{  flushsources=TRUE;
}

ULONG Waitprocessaweb(ULONG extramask)
{  ULONG getmask;
   short i;
   do
   {  getmask=Wait(waitmask|extramask);
      for(i=0;i<16;i++)
      {  if((getmask&(1<<(i+16))) && processfun[i])
         {  processfun[i]();
         }
      }
   } while(!(getmask&extramask));
   return (getmask&extramask);
}

void Queuesetmsgdata(void *object,ULONG queueid,ULONG userdata)
{  struct Smqnode *qn,*qnn;
   if(queueid)
   {  if(qn=ALLOCSTRUCT(Smqnode,1,0))
      {  qn->object=object;
         qn->queueid=queueid;
         qn->userdata=userdata;
         AddTail((struct List *)&setmsgqueue,(struct Node *)qn);
      }
   }
   else
   {  for(qn=setmsgqueue.first;qn->next;qn=qnn)
      {  qnn=qn->next;
         if(qn->object==object)
         {  Remove((struct Node *)qn);
            FREE(qn);
         }
      }
   }
}

void Queuesetmsg(void *object,ULONG queueid)
{  Queuesetmsgdata(object,queueid,0);
}

static void Setmsgqueue(void)
{  struct Smqnode *qn;
   /* First set the selected flag on all present nodes, then process only
    * the selected ones. */
   for(qn=setmsgqueue.first;qn->next;qn=qn->next)
   {  qn->selected=TRUE;
   }
   while(setmsgqueue.first->next && setmsgqueue.first->selected)
   {  qn=(struct Smqnode *)RemHead((struct List *)&setmsgqueue);
      Asetattrs(qn->object,
         AOBJ_Queueid,qn->queueid,
         AOBJ_Queuedata,qn->userdata,
         TAG_END);
      FREE(qn);
   }
}
VARARGS68K_DECLARE(void Lowlevelreq(STRPTR msg,...))
{  struct EasyStruct es;
   BOOL opened=FALSE;
   struct Window *window=(struct Window *)Agetattr(Firstwindow(),AOWIN_Window);
   VA_LIST av;
   ULONG *args;
   VA_STARTLIN(av,msg);
   args = (ULONG *)VA_GETLIN(av,ULONG *);

   if(!IntuitionBase)
   {
       if(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",36)) opened=TRUE;
#if defined(__amigaos4__)
       IIntuition = (struct IntuitionIFace *)GetInterface((struct Library *)IntuitionBase,"main",1,0);
#endif
   }
   if(IntuitionBase)
   {  es.es_StructSize=sizeof(struct EasyStruct);
      es.es_Flags=0;
      es.es_Title="AWeb";
      es.es_TextFormat=msg;
      es.es_GadgetFormat="Ok";
      EasyRequestArgs(window,&es,NULL,args);
      if(opened)
      {
#if defined(__amigaos4__)
         DropInterface((struct Interface *)IIntuition);
         IIntuition = NULL;
#endif
         CloseLibrary((struct Library *)IntuitionBase);
         IntuitionBase=NULL;
      }
   }
   else
   {  vprintf(msg,args);
      printf("\n");
   }
   VA_END(av);
}



void *Openaweblib(UBYTE *name,ULONG version)
{
   struct Library *base=OpenLibrary(name,version);
   if(!base)
   {  Lowlevelreq(AWEBSTR(MSG_ERROR_CANTOPEN),name);
   }
#if !defined(__amigaos4__)
   return base;
#else
   {
       struct Interface *mainiface = NULL;
       if(base)
       {
           mainiface= GetInterface(base,"main",1,0);
           if(!mainiface)
           {
               Lowlevelreq(AWEBSTR(MSG_ERROR_CANTOPEN),name);
           }
       }
       return mainiface;
   }
#endif
}

void *Openjslib(void)
{  if(!AWebJSBase)
   {  if(!(AWebJSBase=(struct AWebJSBase *)OpenLibrary("aweblib/" JAVASCRIPT_AWEBLIB,JAVASCRIPT_VERSION))
      && !(AWebJSBase=(struct AWebJSBase *)OpenLibrary(AWEBLIBPATH JAVASCRIPT_AWEBLIB,JAVASCRIPT_VERSION))
      && !(AWebJSBase=(struct AWebJSBase *)OpenLibrary("PROGDIR:aweblib/"JAVASCRIPT_AWEBLIB,JAVASCRIPT_VERSION)))
      {  Lowlevelreq(AWEBSTR(MSG_ERROR_CANTOPEN),JAVASCRIPT_AWEBLIB);
      }
   }
#if !defined(__amigaos4__)
   return AWebJSBase;
#else
    if(AWebJSBase)
    {
        if(!IAWebJS)
        {
            if(!(IAWebJS = (struct AWebJSIFace *)GetInterface((struct Library *)AWebJSBase,"main",1,0)))
            {
                 Lowlevelreq(AWEBSTR(MSG_ERROR_CANTOPEN),"Javascript Interface");
            }
        }
    }
    return IAWebJS;

#endif


}

/* Close an aweblib or plugin (or ordinary library ) */
/* For OS4.0 ptr is the interface for os3.x ptr is the libbase */
/* Intended for use with local library ptrs */

void Closeaweblib(void *ptr)
{
#if defined(__amigaos4__)
    if(ptr)
    {
        struct Library *base = ((struct Interface *)ptr)->Data.LibBase;
        DropInterface((struct Interface *)ptr);
        if(base)CloseLibrary(base);

    }

#else
    if(ptr)CloseLibrary((struct Library *)ptr);
#endif
}

/* Attempt to remove an aweblib or plugin. (or at least trigger the expunge code */

void Remaweblib(void *ptr)
{
#if defined(__amigaos4__)
    if(ptr)
    {
        struct Library *base = ((struct Interface *)ptr)->Data.LibBase;
        if(base)RemLibrary(base);
    }
#else
    if(ptr)RemLibrary((struct Library *)ptr);
#endif
}


BOOL Stackoverflow(void)
{
#if defined(__MORPHOS__)
   LONG free;
   struct Task *task=FindTask(NULL);

   if (SysBase->LibNode.lib_Version == 50 && SysBase->LibNode.lib_Revision <= 69)
   {
      free = (ULONG)&task - (ULONG)task->tc_ETask->PPCSPLower;
   }
   else
   {
      ULONG total, used;
      NewGetTaskAttrsA(task, &total, sizeof(ULONG), TASKINFOTYPE_STACKSIZE, NULL);
      NewGetTaskAttrsA(task, &used, sizeof(ULONG), TASKINFOTYPE_USEDSTACKSIZE, NULL); // Broken in 1.4 when SysBase->ThisTask == task
      free = total - used;
   }

   return (BOOL)(free < 4000);
#else
   struct Task *task=FindTask(NULL);
   /* Since (task) is a stack variable, (&task) is a good approximation of
    * the current stack pointer */
   return (BOOL)((ULONG)&task<(ULONG)task->tc_SPLower+4000);
#endif
}

BOOL Awebactive(void)
{  struct IntuitionBase *ibase=(struct IntuitionBase *)IntuitionBase;
   ULONG lock=LockIBase(0);
   struct Task *wtask=NULL;
   if(ibase->ActiveWindow && ibase->ActiveWindow->UserPort)
   {  wtask=ibase->ActiveWindow->UserPort->mp_SigTask;
   }
   UnlockIBase(lock);
   return Isawebtask(wtask);
}

/*-----------------------------------------------------------------------*/

void Openloadreq(struct Screen *screen)
{
// Load Requester removed for MOS as AWEB loads on MOS
// so fast that is it not even shown for a 10th of a second

#if !defined(__MORPHOS__)
   if(!AwebStartupBase) AwebStartupBase=OpenLibrary("aweblib/" STARTUP_AWEBLIB,STARTUP_VERSION);
   if(!AwebStartupBase) AwebStartupBase=OpenLibrary(AWEBLIBPATH STARTUP_AWEBLIB,STARTUP_VERSION);
   if(AwebStartupBase)
   {
#if defined(__amigaos4__)
     if((IAwebStartup = (struct AwebStartupIFace *)GetInterface(AwebStartupBase,"main",1,0)))
#endif
       AwebStartupOpen(screen,awebversion,imagepalette,imagedata);
   }
#endif

}

static void Closeloadreq(void)
{  if(AwebStartupBase)
   {
      AwebStartupClose();
#if defined(__amigaos4__)
      DropInterface((struct Interface *)IAwebStartup);
      IAwebStartup = NULL;
#endif
      CloseLibrary(AwebStartupBase);
      AwebStartupBase=NULL;
   }
}

void Setloadreqstate(UWORD state)
{
#if defined(__amigaos4__)
    if(IAwebStartup) AwebStartupState(state);
#else
    if(AwebStartupBase) AwebStartupState(state);
#endif
}

void Setloadreqlevel(long ready,long total)
{
#if defined(__amigaos4__)
  if(IAwebStartup) AwebStartupLevel(ready,total);
#else
  if(AwebStartupBase) AwebStartupLevel(ready,total);
#endif
}

/*-----------------------------------------------------------------------*/
/* Charset Handling Code */

struct Charset
{
    NODE(Charset);
    UBYTE *name;
    UBYTE *table;
};

static LIST(Charset) charsets;

static UBYTE *charsetdummy=NULL;;

static BOOL Initcharsets()
{
    struct Charset *c;
    int i;

    NewList((struct List *)&charsets);

    /* create the dummy charset */

    c = ALLOCSTRUCT(Charset,1,MEMF_CLEAR);
    c->name=Dupstr("dummyname",-1);     /* duplicate it so it can be free'd */
    c->table=ALLOCTYPE(UBYTE,512,0);

    for(i=0;i<256;i++)
    {
        c->table[i]=i;
        c->table[i+256]=i;
    }

    AddTail((struct List *)&charsets,(struct Node *)c);

    charsetdummy=c->table;

    return TRUE;
}

static void Freecharsets()
{
    struct Charset *c;
    /* Initcharsets may not have been called if this is a dupstart*/
    /* There use the value of charsetdummy to check */

    if(charsetdummy)
    {
    while(c = (struct Charset *)RemHead((struct List *)&charsets))
    {
        if(c->name)FREE(c->name);
        if(c->table)FREE(c->table);
        FREE(c);
    }
    }
}

/* Find a charset with a given name and return the table */
/* If the charset is not found, attempt to open the corresponding file */
/* If that fails return the default charset */

UBYTE * Findcharset(UBYTE *name)
{
    struct Charset *c;
    UBYTE *chname;
    UBYTE filename[256];
    BPTR fptr;

    if(name)
        chname=name;
    else
        chname=prefs.browser.charset;

    for(c=charsets.first;c->next;c=c->next)
    {
        if(STRIEQUAL(chname,c->name))
        {
            return c->table;
        }
    }
    /* if we get here then the charset hasn't been found ready loaded */
    /* try and load the charset */
    sprintf(filename,"AWebPath:Charsets/%s.charset",chname);
    if((fptr = Open(filename,MODE_OLDFILE)))
    {
        c = ALLOCSTRUCT(Charset,1,MEMF_CLEAR);
        c->name = Dupstr(chname,-1);
        c->table=ALLOCTYPE(UBYTE,512,0);
        Read(fptr,c->table,512);
        Close(fptr);
        AddTail((struct List *)&charsets,(struct Node *)c);
        return c->table;
    }

    /* we get here then we couldn't load the requested charset so return */
    /* the dummy table */

    return charsetdummy;
}


/*-----------------------------------------------------------------------*/



void Cleanup(void)
{
   Exitcache();
   Freetooltip();
   Freemime();
   Freeboopsi();
   Freeauthor();
   Freetcp();     /* MUST be called before Freerequest(), Freeprefs() */
   Freecookie();
   if(aweb)
   {  Adisposeobject(aweb);   /* MUST be called before Freerequest() */
      aweb=NULL;
   }
   Freecharsets(); /* not sure exactly where this needs to be done as yet */
   Freerequest();
   Freeobject();  /* MUST be called before Freesupport(), Freeapplication() */

   Freeprefs();         /* after Freeobjects bcs running tasks might acces prefs */
   Freeapplication();   /* after Freeobject bcz it frees the JS context */
   Freearexx();   /* after Freeobject() bcz it waits for scripts to complete */
   Freehttp();    /* after Freeobject() bcz tasks must be stopped */
   Freenameserv();/* after Freeobject() bcz network tasks use hent structures freed here */
   Freeawebtcp(); /* free the resources used for awebtcp library relocation */
   Freesupport();
   Freememory();  /* MUST be the very last! */
   if(locale) CloseLocale(locale);
   if(localeinfo.li_Catalog) CloseCatalog(localeinfo.li_Catalog);
   if(AWebJSBase) Closelib((struct Library **)&AWebJSBase,(struct Interface **)&IAWebJS);
   if(ButtonBase) Closelib((struct Library **)&ButtonBase,(struct Interface **)&IButton);
   if(openedca)
   {  if(BitMapBase) Closelib((struct Library **)&BitMapBase,(struct Interface **)&IButton);
      if(BevelBase) Closelib((struct Library **)&BevelBase,(struct Interface **)&IBevel);
      if(GlyphBase) Closelib((struct Library **)&GlyphBase,(struct Interface **)&IGlyph);
      if(WindowBase) Closelib((struct Library **)&WindowBase,(struct Interface **)&IWindow);
      if(LayoutBase) Closelib((struct Library **)&LayoutBase,(struct Interface **)&ILayout);
      if(StringBase) Closelib((struct Library **)&StringBase,(struct Interface **)&IString);
      if(SpaceBase) Closelib((struct Library **)&SpaceBase,(struct Interface **)&ISpace);
      if(LabelBase) Closelib((struct Library **)&LabelBase,(struct Interface **)&ILabel);
      if(DrawListBase) Closelib((struct Library **)&DrawListBase,(struct Interface **)&IDrawList);
      if(CheckBoxBase) Closelib((struct Library **)&CheckBoxBase,(struct Interface **)&ICheckBox);
      if(IntegerBase) Closelib((struct Library **)(struct Interface **)&IntegerBase,(struct Interface **)&IInteger);
      if(ChooserBase) Closelib((struct Library **)&ChooserBase,(struct Interface **)&IChooser);
      if(ListBrowserBase) Closelib((struct Library **)&ListBrowserBase,(struct Interface **)&IListBrowser);
      if(SpeedBarBase) Closelib((struct Library **)&SpeedBarBase,(struct Interface **)&ISpeedBar);
      if(ScrollerBase) Closelib((struct Library **)&ScrollerBase,(struct Interface **)&IScroller);
      if(FuelGaugeBase) Closelib((struct Library **)&FuelGaugeBase,(struct Interface **)&IFuelGauge);
      if(PenMapBase) Closelib((struct Library **)&PenMapBase,(struct Interface **)&IPenMap);
   }
   if(WorkbenchBase) Closelib(&WorkbenchBase,(struct Interface **)&IWorkbench);
   if(IFFParseBase) Closelib(&IFFParseBase,(struct Interface **)&IIFFParse);
   if(LocaleBase) Closelib((struct Library **)&LocaleBase,(struct Interface **)&ILocale);
   if(IconBase) Closelib(&IconBase,(struct Interface **)&IIcon);
   if(KeymapBase) Closelib(&KeymapBase,(struct Interface **)&IKeymap);
   if(AslBase) Closelib(&AslBase,(struct Interface **)&IAsl);
   if(DataTypesBase) Closelib(&DataTypesBase,(struct Interface **)&IDataTypes);
   if(GadToolsBase) Closelib(&GadToolsBase,(struct Interface **)&IGadTools);
   if(ColorWheelBase) Closelib(&ColorWheelBase,(struct Interface **)&IColorWheel);
   if(LayersBase) Closelib(&LayersBase,(struct Interface **)&ILayers);
   if(DiskfontBase) Closelib(&DiskfontBase,(struct Interface **)&IDiskfont);
   if(UtilityBase) Closelib((struct Library **)&UtilityBase,(struct Interface **)&IUtility);
   if(GfxBase) Closelib((struct Library **)&GfxBase,(struct Interface **)&IGraphics);
   if(IntuitionBase) Closelib((struct Library **)&IntuitionBase,(struct Interface **)&IIntuition);
   if(CyberGfxBase) Closelib(&CyberGfxBase, (struct Interface **)&ICyberGfx);
#if defined(__amigaos4__)
   if(P96Base) Closelib((struct Library **)&P96Base,(struct Interface **)&IP96);
#endif
#if defined(__MORPHOS__)
   CloseLibrary(ZBase);
#endif
   if(awebcontrolport)
   {  struct Message *msg;
      Forbid();
      RemPort(awebcontrolport);
      while(msg=GetMsg(awebcontrolport)) ReplyMsg(msg);
      Permit();
      DeleteMsgPort(awebcontrolport);
   }

   report();
}

void Quit(BOOL immediate)
{  if(immediate || !Quitreq()) quit=TRUE;
}

void Iconify(BOOL i)
{  icondefer=TRUE;
   iconify=i;
}


static BOOL Initall(void)
{  long lock;
   /* Must be done here before classes initialize */

   if(lock=Lock("PROGDIR:",ACCESS_READ))
   {  if(AssignLock("AWebPath",lock)) awebpath=TRUE;
      else UnLock(lock);
   }
   if(!Initcharsets()) return FALSE;
   if(!(locale=OpenLocale(NULL))) return FALSE;
   if(!Initversion()) return FALSE;
   if(!Initarexx()) return FALSE;   /* must be inited before window */
   if(!Initmime()) return FALSE;    /* must be inited before prefs */
   if(!Initdefprefs()) return FALSE;/* must be inited before prefs */
   if(!Initprefs()) return FALSE;   /* must be inited before window,layout, cache */
   if(!Initboopsi()) return FALSE;  /* must be inited before window */
   if(!Initrequest()) return FALSE; /* must be inited before io */
   if(!Installapplication()) return FALSE;   /* Opens the startup window */
   if(!Installurl()) return FALSE;
   if(!Installscroller()) return FALSE;
   if(!Installdocsource()) return FALSE;
   if(!Installdocument()) return FALSE;
   if(!Installdocext()) return FALSE;
   if(!Installbody()) return FALSE;
   if(!Installlink()) return FALSE;
   if(!Installelement()) return FALSE;
   if(!Installtext()) return FALSE;
   if(!Installbreak()) return FALSE;
   if(!Installruler()) return FALSE;
   if(!Installbullet()) return FALSE;
   if(!Installtable()) return FALSE;
   if(!Installmap()) return FALSE;
   if(!Installarea()) return FALSE;
   if(!Installwinhis()) return FALSE;
   if(!Installname()) return FALSE;
   if(!Installframe()) return FALSE;
   if(!Installframeset()) return FALSE;
   if(!Installform()) return FALSE;
   if(!Installfield()) return FALSE;
   if(!Installbutton()) return FALSE;
   if(!Installcheckbox()) return FALSE;
   if(!Installradio()) return FALSE;
   if(!Installinput()) return FALSE;
   if(!Installselect()) return FALSE;
   if(!Installtextarea()) return FALSE;
   if(!Installhidden()) return FALSE;
   if(!Installfilefield()) return FALSE;
   if(!Installfetch()) return FALSE;
   if(!Installsource()) return FALSE;
   if(!Installcopy()) return FALSE;
   if(!Installfile()) return FALSE;
   if(!Installcache()) return FALSE;
   if(!Installwindow()) return FALSE;
   if(!Installwhiswindow()) return FALSE;
   if(!Installfilereq()) return FALSE;
   if(!Installsaveas()) return FALSE;
   if(!Installimgsource()) return FALSE;
   if(!Installimgcopy()) return FALSE;
   if(!Installpopup()) return FALSE;
   if(!Installextprog()) return FALSE;
   if(!Installnetstatwin()) return FALSE;
   if(!Installhotlist()) return FALSE;
   if(!Installcabrowse()) return FALSE;
   if(!Installeditor()) return FALSE;
   if(!Installsearch()) return FALSE;
   if(!Installprintwindow()) return FALSE;
   if(!Installprint()) return FALSE;
   if(!Installsaveiff()) return FALSE;
   if(!Installtask()) return FALSE;
   if(!Installsourcedriver()) return FALSE;
   if(!Installcopydriver()) return FALSE;
   if(!Installsoundsource()) return FALSE;
   if(!Installsoundcopy()) return FALSE;
   if(!Installtimer()) return FALSE;
   if(!Installinfo()) return FALSE;
   if(!Installauthedit()) return FALSE;

   if(!(aweb=Anewobject(AOTP_APPLICATION,TAG_END))) return FALSE;
   Setloadreqstate(LRQ_FONTS);
   if(!Initprefs2()) return FALSE;
   if(!Initurl()) return FALSE;
   Setloadreqstate(LRQ_CACHE);
   if(!Initcache()) return FALSE;   /* must be inited before url(2) */
   if(!Initurl2()) return FALSE;
   if(!Initnameserv()) return FALSE;
   if(!Initauthor()) return FALSE;
   if(!Initcookie()) return FALSE;
   if(!Inittcp()) return FALSE;
   if(!Inithttp()) return FALSE;
   if(!Initselect()) return FALSE;
   if(!Initcheckbox()) return FALSE;
   if(!Initradio()) return FALSE;

   Closeloadreq();

   if(!Anewobject(AOTP_WINDOW,TAG_END)) return FALSE;
   if(!Initscroller()) return FALSE;   /* Needs a window */
   return TRUE;
}

static void Getprogramname(struct WBStartup *wbs)
{  long progdir;
   UBYTE progbuf[40];
   if(wbs)
   {  if(NameFromLock(wbs->sm_ArgList[0].wa_Lock,programname,256))
      {  AddPart(programname,wbs->sm_ArgList[0].wa_Name,256);
      }
   }
   else
   {  if((progdir=GetProgramDir())
      && NameFromLock(progdir,programname,256)
      && GetProgramName(progbuf,40))
      {  AddPart(programname,FilePart(progbuf),256);
      }
   }
}

static void Getarguments(struct WBStartup *wbs)
{  long args[17]={0};
   UBYTE *argtemplate="URL/M,LOCAL/S,CONFIG/K,HOTLIST/K"
      ",VVIIV/S"
#ifdef BETAKEYFILE
      ",HTTPDEBUG/S,NOPOOL/S,SPECDEBUG/S"
#ifdef DEVELOPER
      ",BLOCK/K/N,PROFILE/S,OODEBUG/K,OOMETHOD/K,OODELAY/S,USETEMP/S,OOKDEBUG/S,CHARSETDEBUG/S"
#endif
#endif
      ;
   long i,nurl=0;
   struct Process *process;
   if(wbs)
   {  long oldcd=CurrentDir(wbs->sm_ArgList[0].wa_Lock);
      struct DiskObject *dob=GetDiskObject(wbs->sm_ArgList[0].wa_Name);
      if(dob)
      {  UBYTE **ttp;
         for(ttp=dob->do_ToolTypes;ttp && *ttp;ttp++)
         {  if(STRNIEQUAL(*ttp,"URL=",4))
            {  if(nurl<16)
               {  localinitialurl[nurl]=0;
                  initialurls[nurl++]=Dupstr(*ttp+4,-1);
               }
            }
            else if(STRIEQUAL(*ttp,"LOCAL")) args[1]=TRUE;
            else if(STRNIEQUAL(*ttp,"CONFIG=",7)) Setprefsname(*ttp+7);
            else if(STRNIEQUAL(*ttp,"HOTLIST=",8)) Sethotlistname(*ttp+8);
            else if(STRIEQUAL(*ttp,"VVIIV")) haiku=TRUE;
            else if(STRIEQUAL(*ttp,"NOICONIFY")) noiconify=TRUE;
#ifdef BETAKEYFILE
            else if(STRIEQUAL(*ttp,"HTTPDEBUG")) httpdebug=TRUE;
            else if(STRIEQUAL(*ttp,"NOPOOL")) nopool=TRUE;
#ifdef DEVELOPER
            else if(STRNIEQUAL(*ttp,"BLOCK=",6)) sscanf(*ttp+6," %ld",&localblocksize);
            else if(STRIEQUAL(*ttp,"CHARSETDEBUG")) charsetdebug=TRUE;
#endif
#endif
         }
         FreeDiskObject(dob);
      }
      CurrentDir(oldcd);
      for(i=1;i<wbs->sm_NumArgs && nurl<16;i++)
      {  UBYTE buffer[256];
         if(NameFromLock(wbs->sm_ArgList[i].wa_Lock,buffer,255)
         && AddPart(buffer,wbs->sm_ArgList[i].wa_Name,255))
         {  localinitialurl[nurl]=1;
            initialurls[nurl++]=Dupstr(buffer,-1);
         }
      }
      process=(struct Process *)wbs->sm_Message.mn_ReplyPort->mp_SigTask;
   }
   else
   {  struct RDArgs *rda=ReadArgs(argtemplate,args,NULL);
      if(rda)
      {  UBYTE **p;
         for(p=(UBYTE **)args[0];p && *p;p++)
         {  if(nurl<16)
            {  localinitialurl[nurl]=0;
               initialurls[nurl++]=Dupstr(*p,-1);
            }
         }
         if(args[2]) Setprefsname((UBYTE *)args[2]);
         if(args[3]) Sethotlistname((UBYTE *)args[3]);
         if(args[4]) haiku=TRUE;
#ifdef BETAKEYFILE
         if(args[5]) httpdebug=TRUE;
         if(args[6]) nopool=TRUE;
         if(args[7]) specdebug=TRUE;
#ifdef DEVELOPER
         if(args[8]) localblocksize=*(long *)args[8];
         if(args[9]) profile=TRUE;
         if(args[10]) Setoodebug((UBYTE *)args[10]);
         if(args[11]) Setoomethod((UBYTE *)args[11]);
         if(args[12]) Setoodelay();
         if(args[13]) usetemp=TRUE;
         if(args[14]) ookdebug=TRUE;
         if(args[15]) charsetdebug=TRUE;
#endif
#endif
         FreeArgs(rda);
      }
      process=(struct Process *)FindTask(NULL);
   }
   if(args[1])
   {  for(i=0;i<16;i++) localinitialurl[i]=1;
   }
   if(process && process->pr_CLI)
   {  struct CommandLineInterface *cli=(struct CommandLineInterface *)BADDR(process->pr_CLI);
      ourpathlist=(struct PathList *)BADDR(cli->cli_CommandDir);
   }
}

static BOOL Initialdocs(void)
{  short i;
   BOOL opened=FALSE;
   for(i=0;i<16;i++)
   {  if(initialurls[i])
      {  opened=Openinitialdoc(initialurls[i],localinitialurl[i],i==0);
         FREE(initialurls[i]);
      }
   }
   return opened;
}

void dummy(void)
{
    signal(SIGINT, (void *) dummy);
}

int main(int fromcli,char **argv)
{  ULONG getmask;
   struct Url *home;
   short i;
   struct WBStartup *wbs=0;

    //struct Library *execbase=*((struct Library **)4);

#ifdef NETDEMO
#ifndef OSVERSION
   ULONG secs,mics,nextannoy;
   #define ANNOYPERIOD 30*60
#endif
#endif

   if(fromcli==0) wbs=(struct WBStartup *)argv;

   NewList((struct List *)&setmsgqueue);
   Openlib("locale.library",OSNEED(0,44),(struct Library **)&LocaleBase,(struct Interface **)&ILocale);

#ifdef NEED35
   WorkbenchBase=OpenLibrary("workbench.library",44);
   if(WorkbenchBase)
   {  CloseLibrary(WorkbenchBase);
   }
   else
   {  UBYTE *msg=AWEBSTR(MSG_ERROR_NEEDOS30);
      /* leave this one MEMF_PUBLIC as I not sure whether to replace
         with memf_any or memf_shared */
      UBYTE *buf=(UBYTE *)AllocVec(strlen(msg)+1,MEMF_PUBLIC);
      if(buf)
      {  UBYTE *p;
         strcpy(buf,msg);
         for(p=buf;*p && strncmp(p,"3.0",3);p++);
         if(*p) p[2]='5';
         Lowlevelreq(buf);
         FreeVec(buf);
      }
      return 0;
   }
#else
 //  if(execbase->lib_Version<39)
 //  {  Lowlevelreq(AWEBSTR(MSG_ERROR_NEEDOS30));
 //    return 0;
 //  }
#endif

   signal(SIGINT, (void *) dummy);

#ifdef NOKEYFILE
   Openlib("utility.library",OSNEED(39,40),(struct Library **)&UtilityBase,(struct Interface **)&IUtility);
   Openlib("intuition.library",OSNEED(39,40),(struct Library **)&IntuitionBase,(struct Interface **)&IIntuition);
   Openlib("graphics.library",OSNEED(39,40),(struct Library **)&GfxBase,(struct Interface **)&IGraphics);
   Openlib("diskfont.library",OSNEED(39,44),&DiskfontBase,(struct Interface **)&IDiskfont);
   Openlib("layers.library",OSNEED(39,40),&LayersBase,(struct Interface **)&ILayers);
   Openlib("asl.library",OSNEED(39,44),&AslBase,(struct Interface **)&IAsl);
   Openlib("gadgets/colorwheel.gadget",OSNEED(39,44),&ColorWheelBase,(struct Interface **)&IColorWheel);
   Openlib("gadtools.library",OSNEED(39,40),&GadToolsBase,(struct Interface **)&IGadTools);
   Openlib("icon.library",OSNEED(39,44),&IconBase,(struct Interface **)&IIcon);
   Openlib("datatypes.library",OSNEED(39,44),&DataTypesBase,(struct Interface **)&IDataTypes);
   Openlib("keymap.library",OSNEED(37,40),&KeymapBase,(struct Interface **)&IKeymap);
#endif
   Getprogramname(fromcli?NULL:wbs);
   if(!Initmemory())
   {  Cleanup();
      exit(10);
   }
   Openlib("iffparse.library",OSNEED(39,40),&IFFParseBase,(struct Interface **)&IIFFParse);
   Openlib("workbench.library",OSNEED(39,44),&WorkbenchBase,(struct Interface **)&IWorkbench);
   if(WorkbenchBase->lib_Version>=44) has35=TRUE;

#if defined(__amigaos4__)
   Openlib("Picasso96API.library",OSNEED(0,44),&P96Base,(struct Interface **)&IP96);
#endif
#if defined(__MORPHOS__)
   Openlib("z.library", OSNEED(51,51), &ZBase, &IDummy);
#endif

   /* We don't care if cybergraphics isn't present so use Openlibnofail() */

   Openlibnofail("cybergraphics.library", OSNEED(0,44),&CyberGfxBase,(struct Interface **)&ICyberGfx);

   Openclass("gadgets/button.gadget",OSNEED(0,44),&ButtonBase,(struct Interface **)&IButton);
   openedca=TRUE;

   Openclass("images/bitmap.image",OSNEED(0,44),&BitMapBase,(struct Interface **)&IBitMap);
   Openclass("images/bevel.image",OSNEED(0,44),&BevelBase,(struct Interface **)&IBevel);
   Openclass("images/glyph.image",OSNEED(0,44),&GlyphBase,(struct Interface **)&IGlyph);
   Openclass("window.class",OSNEED(0,44),&WindowBase,(struct Interface **)&IWindow);
   Openclass("gadgets/layout.gadget",OSNEED(0,44),&LayoutBase,(struct Interface **)&ILayout);
   Openclass("gadgets/string.gadget",OSNEED(0,44),&StringBase,(struct Interface **)&IString);
   Openclass("gadgets/space.gadget",OSNEED(0,44),&SpaceBase,(struct Interface **)&ISpace);
   Openclass("images/label.image",OSNEED(0,44),&LabelBase,(struct Interface **)&ILabel);
   Openclass("images/drawlist.image",OSNEED(0,44),&DrawListBase,(struct Interface **)&IDrawList);
   Openclass("gadgets/checkbox.gadget",OSNEED(0,44),&CheckBoxBase,(struct Interface **)&ICheckBox);
   Openclass("gadgets/integer.gadget",OSNEED(0,44),&IntegerBase,(struct Interface **)&IInteger);
   Openclass("gadgets/chooser.gadget",OSNEED(0,44),&ChooserBase,(struct Interface **)&IChooser);
   Openclass("gadgets/listbrowser.gadget",OSNEED(0,44),&ListBrowserBase,(struct Interface **)&IListBrowser);
   Openclass("gadgets/speedbar.gadget",OSNEED(0,44),&SpeedBarBase,(struct Interface **)&ISpeedBar);
   Openclass("gadgets/scroller.gadget",OSNEED(0,44),&ScrollerBase,(struct Interface **)&IScroller);
   Openclass("gadgets/fuelgauge.gadget",OSNEED(0,44),&FuelGaugeBase,(struct Interface **)&IFuelGauge);
   Openclass("images/penmap.image",OSNEED(0,44),&PenMapBase,(struct Interface **)&IPenMap);
   Initawebtcp();
   Initsupport();

   localeinfo.li_LocaleBase=(struct Library *)LocaleBase;
   localeinfo.li_Catalog=OpenCatalogA(NULL,"aweb.catalog",NULL);
   maincatalog=localeinfo.li_Catalog;
   Getarguments(fromcli?NULL:wbs);
   setupprof();
   if(Dupstartupcheck()
   && Initall())
   {  Initialrequester(Aboutreq,NULL);
      if(*prefs.program.startupscript)
      {  Sendarexxcmd(Agetattr(Firstwindow(),AOWIN_Key),prefs.program.startupscript);
      }
      if(!Initialdocs())
      {  UBYTE *fragment;
         if(prefs.network.homeurl && *prefs.network.homeurl && prefs.network.starthomepage
          && (home=Findurl("",prefs.network.homeurl,0)))
         {  fragment=Fragmentpart(prefs.network.homeurl);
            Inputwindoc(Firstwindow(),home,fragment,NULL);
         }
      }
      {  void *win=Firstwindow();
         void *whis=NULL;
         ULONG key=0,nr=0;
         Agetattrs(win,
            AOBJ_Winhis,(ULONG)&whis,
            AOWIN_Key,(ULONG)&key,
            AOWIN_Windownr,(ULONG)&nr,
            TAG_END);
         if(prefs.program.aanetstat) Opennetstat();
         if(prefs.program.aawinhis) Openwhiswindow(whis,nr);
         if(prefs.program.aahotlist) Hotlistviewer(key);
         Delay(2);
         Asetattrs(Firstwindow(),AOWIN_Activate,TRUE,TAG_END);
      }
#ifdef NETDEMO
#ifndef OSVERSION
      Demorequest();
      CurrentTime(&secs,&mics);
      nextannoy=secs+ANNOYPERIOD;
#endif
#endif
      while(!quit)
      {  /* Don't wait if there are still queuenodes queued */
         if(ISEMPTY(&setmsgqueue))
         {  getmask=Wait(waitmask|SIGBREAKF_CTRL_C);
         }
         else
         {  getmask=SetSignal(0,0);
         }
         if(getmask&SIGBREAKF_CTRL_C)
         {  Quit(TRUE);
            break;
         }
         for(i=0;i<16;i++)
         {  if((getmask&(1<<(i+16))) && processfun[i])
            {  processfun[i]();
            }
         }
         if(flushsources)
         {  Flushsources(SRCFT_EXCESS);
            flushsources=FALSE;
         }
         if(updateframes)
         {  Doupdateframes();
            updateframes=FALSE;
         }
         Setmsgqueue();
#ifdef NETDEMO
#ifndef OSVERSION
         CurrentTime(&secs,&mics);
         if(secs>=nextannoy)
         {  quit=TRUE;
         }
#endif
#endif
         if(icondefer)
         {  Asetattrs(Aweb(),AOAPP_Iconify,iconify,TAG_END);
            icondefer=FALSE;
         }
      }
      if(*prefs.program.shutdownscript)
      {  Sendarexxcmd(Agetattr(Firstwindow(),AOWIN_Key),prefs.program.shutdownscript);
      }
#ifdef NETDEMO
#ifndef OSVERSION
      Demorequest();
#endif
#endif
   }
   Closeloadreq();
   Cleanup();
   if(awebpath) AssignLock("AWebPath",(BPTR)NULL);
   return 0;
}

#ifdef PROFILE
#define __NOLIBBASE__
#define __NOGLOBALIFACE__
#include <proto/timer.h>
struct prof
{  long secs,mics,n;
   char *string;
};
static struct prof profs[100];
static long nprof;
#ifdef __MORPHOS__
static struct Library *TimerBase;
#else
static struct Device *TimerBase;
#endif
static struct MsgPort *profport;
static struct timerequest *profrequest;

#if defined(__amigaos4__)
static struct TimerIFace *ITimer;
#endif

void setupprof(void)
{  long i;
   if(profile
   && (profport=CreateMsgPort())
   && (profrequest=(struct timerequest *)CreateStdIO(profport))
   && !OpenDevice("timer.device",UNIT_MICROHZ,(struct IORequest *)profrequest,0))
   {  TimerBase=profrequest->tr_node.io_Device;
#if defined(__amigaos4__)
    ITimer = (struct TimerIFace *)GetInterface(TimerBase,"main",1,0);
#endif
      for(i=0;i<10000;i++)
      {  prolog("....Profile Reference");
         epilog("....Profile Reference");
      }
   }
}

void prolog(char *p)
{  struct timeval tv;
   long n;
   if(*p)
   {  /* virgin string */
      profs[nprof].string=p+4;
      *(long *)p=nprof;
      nprof++;
   }
   n=*(long *)p;
   if(TimerBase && profile && n>=0 && n<nprof)
   {  GetSysTime(&tv);
      profs[n].secs-=tv.tv_secs;
      profs[n].mics-=tv.tv_micro;
      profs[n].n++;
   }
}
void epilog(char *p)
{  struct timeval tv;
   long n=*(long *)p;
   if(TimerBase && profile && n>=0 && n<nprof)
   {  GetSysTime(&tv);
      profs[n].secs+=tv.tv_secs;
      profs[n].mics+=tv.tv_micro;
   }
}

void report(void)
{  int i;
   if(profile)
   {  for(i=0;i<nprof;i++)
      {  printf("%40.40s %12ld %6ld %12ld\n",profs[i].string,
            1000000*profs[i].secs+profs[i].mics,
            profs[i].n,
            profs[i].n?((1000000*profs[i].secs+profs[i].mics)/profs[i].n):0);
      }
   }
#if defined(__amigaos4__)
    if(ITimer)DropInterface((struct Interface *)ITimer);
#endif
   if(TimerBase) CloseDevice((struct IORequest *)profrequest);
   if(profrequest) DeleteStdIO((struct IOStdReq *)profrequest);
   if(profport) DeleteMsgPort(profport);
}
#endif

#ifdef BETAKEYFILE
void Mprintf(UBYTE *fmt,...)
{  static struct SignalSemaphore sema;
   static BOOL inited=FALSE;
   VA_LIST p;
   Forbid();
   if(!inited)
   {  InitSemaphore(&sema);
      inited=TRUE;
   }
   Permit();
   ObtainSemaphore(&sema);
   VA_START(p,fmt);
   vprintf(fmt,p);
   VA_END(p);
   ReleaseSemaphore(&sema);
}
#endif
