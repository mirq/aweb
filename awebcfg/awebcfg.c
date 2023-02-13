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

/* awebcfg.c - AWeb configuration tool */

#define NOLOCALE
#include "awebcfg.h"

#ifdef _SASC
__near
#endif

#ifdef __MORPHOS__
long __stack=(16*1024);
#else
long __stack=8192;
#endif

#ifdef __MORPHOS__
struct Library *UtilityBase = NULL;
#else
struct UtilityBase *UtilityBase = NULL;
#endif
struct IntuitionBase * IntuitionBase = NULL;

#if defined(__amigaos4__) || defined(__MORPHOS__)
struct Library*    LocaleBase = NULL;
#else
struct LocaleBase *LocaleBase = NULL;
#endif

struct GfxBase*       GfxBase = NULL;

struct Library *IconBase = NULL,*GadToolsBase = NULL,*AslBase = NULL,
   *ColorWheelBase = NULL,*GradientSliderBase = NULL;

struct Library *WindowBase = NULL,*LayoutBase = NULL,*ButtonBase = NULL,
   *ListBrowserBase = NULL,*ChooserBase = NULL,*IntegerBase = NULL,*SpaceBase = NULL,*CheckBoxBase = NULL,
   *StringBase = NULL,*LabelBase = NULL,*PaletteBase = NULL,*GlyphBase = NULL,*ClickTabBase = NULL;


#if defined(__amigaos4__)
/* Interfaces */

struct IntuitionIFace *IIntuition= NULL;
struct GraphicsIFace *IGraphics= NULL;
struct LocaleIFace *ILocale= NULL;
struct UtilityIFace *IUtility= NULL;

//struct DiskfontIFace *IDiskfont= NULL;
//struct LayersIFace *ILayers= NULL;
struct ColorWheelIFace *IColorWheel= NULL;
struct GadToolsIFace *IGadTools= NULL;
//struct DataTypesIFace *IDataTypes= NULL;
struct AslIFace *IAsl= NULL;
//struct KeymapIFace *IKeymap= NULL;
struct IconIFace *IIcon= NULL;
struct GradientSliderIFace *IGradientSlider= NULL;
//struct IFFParseIFace *IIFFParse= NULL;
//struct WorkbenchIFace *IWorkbench= NULL;


struct WindowIFace *IWindow= NULL;
struct LayoutIFace *ILayout= NULL;
struct ButtonIFace *IButton= NULL;
struct ListBrowserIFace *IListBrowser= NULL;
struct ChooserIFace *IChooser= NULL;
struct IntegerIFace *IInteger= NULL;
struct SpaceIFace   *ISpace= NULL;
struct CheckBoxIFace *ICheckBox= NULL;
struct StringIFace *IString= NULL;
struct LabelIFace  *ILabel= NULL;
struct PaletteIFace *IPalette= NULL;
struct GlyphIFace  *IGlyph= NULL;
struct ClickTabIFace *IClickTab= NULL;
//struct FuelGaugeIFace *IFuelGauge= NULL;
//struct BitMapIFace *IBitMap= NULL;
//struct BevelIFace *IBevel= NULL;
//struct DrawListIFace *IDrawList= NULL;
//struct SpeedBarIFace *ISpeedBar= NULL;
//struct ScrollerIFace *IScroller= NULL;
//struct PenMapIFace *IPenMap= NULL;


#else

void *IDummy = NULL;

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

CONST TEXT versionstring[]="\0$VER:AWebCfg " AWEBVERSION RELEASECLASS " (" __AMIGADATE__ ") " CPU;

UBYTE *argtemplate="BROWSER/S,PROGRAM/S,GUI/S,NETWORK/S,PUBSCREEN/K,CONFIG/K";

struct Screen *pubscreen;
struct DrawInfo *drawinfo;
void *visualinfo;
struct Image *amigaimg;

static UBYTE screenname[32]="";
static BOOL browser,program,gui,network;
UWORD cfgcommand;   /* queued CFGCLASS_xxx see below */
static BOOL newdimensions=FALSE;
BOOL nopool=FALSE;
BOOL has35=FALSE;

struct NewMenu menubase[]=
{   { NM_TITLE,(STRPTR)MSG_SET_PROJECT_MENU,    0,0,0,0},
    {  NM_ITEM,(STRPTR)MSG_SET_PROJECT_OPEN,    0,0,0,(APTR)MID_OPEN},
    {  NM_ITEM,(STRPTR)MSG_SET_PROJECT_SAVEAS,  0,0,0,(APTR)MID_SAVEAS},
    {  NM_ITEM,NM_BARLABEL,                     0,0,0,0},
    {  NM_ITEM,(STRPTR)MSG_SET_PROJECT_QUIT,    0,0,0,(APTR)MID_QUIT},
    { NM_TITLE,(STRPTR)MSG_SET_EDIT_MENU,       0,0,0,0},
    {  NM_ITEM,(STRPTR)MSG_SET_EDIT_DEFAULTS,   0,0,0,(APTR)MID_DEFAULTS},
    {  NM_ITEM,(STRPTR)MSG_SET_EDIT_LASTSAVED,  0,0,0,(APTR)MID_LASTSAVED},
    {  NM_ITEM,(STRPTR)MSG_SET_EDIT_RESTORE,    0,0,0,(APTR)MID_RESTORE},
    { NM_TITLE,(STRPTR)MSG_SET_WINDOWS_MENU,    0,0,0,0},
    {  NM_ITEM,(STRPTR)MSG_SET_WINDOWS_BROWSER, 0,0,0,(APTR)MID_BROWSER},
    {  NM_ITEM,(STRPTR)MSG_SET_WINDOWS_PROGRAM, 0,0,0,(APTR)MID_PROGRAM},
    {  NM_ITEM,(STRPTR)MSG_SET_WINDOWS_GUI,     0,0,0,(APTR)MID_GUI},
    {  NM_ITEM,(STRPTR)MSG_SET_WINDOWS_NETWORK, 0,0,0,(APTR)MID_NETWORK},
#ifndef NEED35
    {  NM_ITEM,(STRPTR)MSG_SET_WINDOWS_CLASSACT,0,0,0,(APTR)MID_CLASSACT},
#endif
    {  NM_ITEM,NM_BARLABEL,                     0,0,0,0},
    {  NM_ITEM,(STRPTR)MSG_SET_WINDOWS_SNAPSHOT,0,0,0,(APTR)MID_SNAPSHOT},
    { NM_END,0,0,0,0,0}
};

static struct MsgPort *cfgport;

struct Settingsprefs setprefs;

void *maincatalog;

#if !defined(__amigaos4__)
struct Interface ;
#endif

struct Library *Openlib(UBYTE *name, ULONG version,struct Library **base,struct Interface **iface);
struct Library *Openclass(UBYTE *, ULONG version, struct Library **base, struct Interface **iface);
void Closelib(struct Library **base, struct Interface **iface);

/*---------------------------------------------------------------------------*/

STRPTR Dupstr(STRPTR str,long length)
{  
   STRPTR dup;
   if(!str) return NULL;
   if(length<0) length=strlen(str);
   if(dup=ALLOCTYPE(UBYTE,length+1,0))
   {  memmove(dup,str,length);
      dup[length]='\0';
   }
   return dup;
}


void Cleanup(void)
{  if(cfgport)
   {  struct Message *msg;
      Forbid();
      RemPort(cfgport);
      while(msg=GetMsg(cfgport)) ReplyMsg(msg);
      Permit();
      ADeletemsgport(cfgport);
   }
   if(visualinfo) FreeVisualInfo(visualinfo);
   if(drawinfo) FreeScreenDrawInfo(pubscreen,drawinfo);
   if(pubscreen) UnlockPubScreen(NULL,pubscreen);
   Freedefprefs();
   Freememory();
   if(maincatalog) CloseCatalog(maincatalog);
   if(localeinfo.li_Catalog) CloseCatalog(localeinfo.li_Catalog);
   Closelib(&WindowBase,(struct Interface **)&IWindow);
   Closelib(&LayoutBase,(struct Interface **)&ILayout);
   Closelib(&ButtonBase,(struct Interface **)&IButton);
   Closelib(&ListBrowserBase,(struct Interface **)&IListBrowser);
   Closelib(&ChooserBase,(struct Interface **)&IChooser);
   Closelib(&IntegerBase,(struct Interface **)&IInteger);
   Closelib(&SpaceBase,(struct Interface **)&ISpace);
   Closelib(&CheckBoxBase,(struct Interface **)&ICheckBox);
   Closelib(&StringBase,(struct Interface **)&IString);
   Closelib(&LabelBase,(struct Interface **)&ILabel);
   Closelib(&PaletteBase,(struct Interface **)&IPalette);
   Closelib(&GlyphBase,(struct Interface **)&IGlyph);
   Closelib(&ClickTabBase,(struct Interface **)&IClickTab);
   Closelib(&GradientSliderBase,(struct Interface **)&IGradientSlider);
   Closelib(&ColorWheelBase,(struct Interface **)&IColorWheel);
   Closelib((struct Library **)&IntuitionBase,(struct Interface **)&IIntuition);
   Closelib(&IconBase,(struct Interface **)&IIcon);
   Closelib((struct Library **)&LocaleBase,(struct Interface **)&ILocale);
   Closelib((struct Library **)&GfxBase,(struct Interface **)&IGraphics);
   Closelib(&GadToolsBase,(struct Interface **)&IGadTools);
   Closelib((struct Library **)&UtilityBase,(struct Interface **)&IUtility);
   Closelib(&AslBase,(struct Interface **)&IAsl);
   exit(0);
}

VARARGS68K_DECLARE(void Lowlevelreq(UBYTE *msg,...))
{  struct EasyStruct es;
   BOOL opened=FALSE;
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
      es.es_Title="AWebCfg";
      es.es_TextFormat=msg;
      es.es_GadgetFormat="Ok";
      EasyRequestArgs(NULL,&es,NULL,args);
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


/*
void Lowlevelreq(UBYTE *msg,...)
{  struct EasyStruct es;
   BOOL opened=FALSE;
   va_list args;
   va_start(args,msg);
   if(!IntuitionBase)
   {  if(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",36)) opened=TRUE;
   }
   if(IntuitionBase)
   {  es.es_StructSize=sizeof(struct EasyStruct);
      es.es_Flags=0;
      es.es_Title="AWebCfg";
      es.es_TextFormat=msg;
      es.es_GadgetFormat="Ok";
      EasyRequestArgs(NULL,&es,NULL,args);
      if(opened)
      {  CloseLibrary(IntuitionBase);
         IntuitionBase=NULL;
      }
   }
   else
   {  vprintf(msg,args);
      printf("\n");
   }
   va_end(args);
}
*/

/*
static struct Library *Openlib(UBYTE *name,long version)
{  struct Library *lib=OpenLibrary(name,version);
   if(!lib)
   {  Lowlevelreq(AWEBSTR(MSG_ERROR_CANTOPENV),name,version);
      Cleanup();
      exit(10);
   }
   return lib;
}
*/



static void Getarguments(struct WBStartup *wbs)
{  long args[8]={0};
   if(wbs)
   {  long oldcd= ASetcurrentdir(wbs->sm_ArgList[0].wa_Lock);
      struct DiskObject *dob=GetDiskObject(wbs->sm_ArgList[0].wa_Name);
      if(dob)
      {  UBYTE **ttp;
         for(ttp=dob->do_ToolTypes;*ttp;ttp++)
         {  if(STRIEQUAL(*ttp,"BROWSER")) browser=TRUE;
            else if(STRIEQUAL(*ttp,"PROGRAM")) program=TRUE;
            else if(STRIEQUAL(*ttp,"GUI")) gui=TRUE;
            else if(STRIEQUAL(*ttp,"NETWORK")) network=TRUE;
            else if(STRNIEQUAL(*ttp,"CONFIG=",7)) strncpy(configname,*ttp+7,31);
            else if(STRNIEQUAL(*ttp,"PUBSCREEN=",10)) strncpy(screenname,*ttp+10,31);
         }
         FreeDiskObject(dob);
      }
      ASetcurrentdir(oldcd);
   }
   else
   {  struct RDArgs *rda=ReadArgs(argtemplate,args,NULL);
      if(rda)
      {  if(args[0]) browser=TRUE;
         if(args[1]) program=TRUE;
         if(args[2]) gui=TRUE;
         if(args[3]) network=TRUE;
         if(args[4]) strncpy(screenname,(UBYTE *)args[4],31);
         if(args[5]) strncpy(configname,(UBYTE *)args[5],31);
         FreeArgs(rda);
      }
   }
}

/* check if duplicate startup. Notify first copy */
static BOOL Dupstartupcheck(void)
{  struct Cfgmsg msg={{{0}}};
   struct MsgPort *port;
   if(cfgport=ACreatemsgport())
   {  Forbid();
      if(port=FindPort(AWEBCFGPORTNAME))
      {  msg.msg.mn_ReplyPort=cfgport;
         if(browser) msg.cfgclass|=CFGCLASS_BROWSER;
         if(program) msg.cfgclass|=CFGCLASS_PROGRAM;
         if(gui) msg.cfgclass|=CFGCLASS_GUI;
         if(network) msg.cfgclass|=CFGCLASS_NETWORK;
         PutMsg(port,(struct Message *)&msg);
      }
      else
      {  cfgport->mp_Node.ln_Name=AWEBCFGPORTNAME;
         AddPort(cfgport);
      }
      Permit();
      if(port)
      {  WaitPort(cfgport);
         GetMsg(cfgport);
         ADeletemsgport(cfgport);
         cfgport=NULL;
         return FALSE;
      }
      else return TRUE;
   }
   return FALSE;
}

/* check messages in cfgport and set (command) */
static void Processcfgport(void)
{  struct Cfgmsg *msg;
   while(msg=(struct Cfgmsg *)GetMsg(cfgport))
   {  cfgcommand|=msg->cfgclass;
      ReplyMsg((struct Message *)msg);
   }
}

static void Adjustmenus35(void)
{  short i;
   BOOL shift=FALSE;
   for(i=0;menubase[i].nm_Type!=NM_END;i++)
   {  if(menubase[i].nm_Type==NM_ITEM
      && menubase[i].nm_UserData==(APTR)MID_CLASSACT)
      {  shift=TRUE;
      }
      if(shift)
      {  menubase[i]=menubase[i+1];
      }
   }
}

static void Localizemenus(void)
{  short i;
   UBYTE *str;
   for(i=0;menubase[i].nm_Type!=NM_END;i++)
   {  if(menubase[i].nm_Label!=NM_BARLABEL)
      {  str=AWEBSTR((long)menubase[i].nm_Label);
         if(strlen(str)>2 && str[1]=='/')
         {  menubase[i].nm_Label=str+2;
            menubase[i].nm_CommKey=str;
         }
         else menubase[i].nm_Label=str;
      }
   }
}

static void Openclassact(void)
{  UBYTE buf[64];
   long out;
   BOOL result=FALSE;
   strcpy(buf,"SYS:Prefs/ClassAct");
   if(*screenname)
   {  strcat(buf," PUBSCREEN ");
      strcat(buf,screenname);
   }
   out=Open("NIL:",MODE_NEWFILE);
   if(out && 0<=SystemTags(buf,
      SYS_Input,out,
      SYS_Output,NULL,
      SYS_Asynch,TRUE,
      TAG_END)) result=TRUE;
   if(!result && out) Close(out);
}

void Makechooserlist(struct List *list,UBYTE **labels,BOOL readonly)
{  struct Node *node;
   short i;
   for(i=0;labels[i];i++)
   {  if(node=AllocChooserNode(
         CNA_Text,labels[i],
         CNA_ReadOnly,readonly,
         TAG_END))
         AddTail(list,node);
   }
}

void Freechooserlist(struct List *list)
{  struct Node *node;
   if(list->lh_Head)
   {  while(node=RemHead(list)) FreeChooserNode(node);
   }
}

void Makeclicktablist(struct List *list,UBYTE **labels)
{  struct Node *node;
   short i;
   for(i=0;labels[i];i++)
   {  if(node=AllocClickTabNode(
         TNA_Text,labels[i],
         TNA_Number,i,
//         TNA_Enabled,TRUE,
         TNA_Spacing,4,
         TAG_END))
         AddTail(list,node);
   }
}

void Freeclicktablist(struct List *list)
{  struct Node *node;
   if(list->lh_Head)
   {  while(node=RemHead(list)) FreeClickTabNode(node);
   }
}

/*
void Makeradiolist(struct List *list,UBYTE **labels)
{  struct Node *node;
   short i;
   for(i=0;labels[i];i++)
   {  if(node=AllocRadioButtonNode(i,
         RBNA_Labels,labels[i],
         TAG_END))
         AddTail(list,node);
   }
}

void Freeradiolist(struct List *list)
{  struct Node *node;
   if(list->lh_Head)
   {  while(node=RemHead(list)) FreeRadioButtonNode(node);
   }
}
*/

void Freebrowserlist(struct List *list)
{  struct Node *node;
   if(list->lh_Head)
   {  while(node=RemHead(list)) FreeListBrowserNode(node);
   }
}

VARARGS68K_DECLARE(void Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...))
{
   VA_LIST va;
   struct TagItem *tags;

   VA_STARTLIN(va,req);
   tags = (struct TagItem *)VA_GETLIN(va, struct TagItem *);
   if(SetGadgetAttrsA(gad,win,req,tags) && win) RefreshGList(gad,win,req,1);
   VA_END(va);
}
/*
void Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...)
{
struct TagItem *tags=(struct TagItem *)((ULONG *)&req+1);
   if(SetGadgetAttrsA(gad,win,req,tags) && win) RefreshGList(gad,win,req,1);
}
*/
struct Node *Getnode(struct List *list,long n)
{  struct Node *node=list->lh_Head;
   while(node->ln_Succ && n)
   {  node=node->ln_Succ;
      n--;
   }
   if(node->ln_Succ) return node;
   else return NULL;
}

long Getvalue(struct Gadget *gad,ULONG tag)
{  long value=0;
   GetAttr(tag,gad,(ULONG *)&value);
   return value;
}

void Getstringvalue(UBYTE **ptr,struct Gadget *gad)
{  UBYTE *v=(UBYTE *)Getvalue(gad,STRINGA_TextVal);
   UBYTE *p;
   if(v && (p=Dupstr(v,-1)))
   {  if(*ptr) FREE(*ptr);
      *ptr=p;
   }
}

long Activelabel(struct List *list,UBYTE *label)
{  struct Node *node;
   UBYTE *s;
   long n=0;
   for(node=list->lh_Head;node->ln_Succ;node=node->ln_Succ)
   {
      GetChooserNodeAttrs(node,CNA_Text,&s,TAG_END);
      if(STRIEQUAL(s,label))
         return n;
      n++;
   }
   return 0;
}

void Getnonnullstringvalue(UBYTE **ptr,struct Gadget *gad)
{  UBYTE *v=(UBYTE *)Getvalue(gad,STRINGA_TextVal);
   if(*ptr) FREE(*ptr);
   if(v && *v) *ptr=Dupstr(v,-1);
   else *ptr=NULL;
}

BOOL Getselected(struct Gadget *gad)
{  return (BOOL) ((gad->Flags & GFLG_SELECTED) != 0);
}

void Adjustintgad(struct Window *window,struct Gadget *gad)
{  long value=Getvalue(gad,INTEGER_Number);
   Setgadgetattrs(gad,window,NULL,INTEGER_Number,value,TAG_END);
}

long Reqheight(struct Screen *screen)
{  struct DimensionInfo dim={{0}};
   long height;
   if(GetDisplayInfoData(NULL,(UBYTE *)&dim,sizeof(dim),DTAG_DIMS,
      GetVPModeID(&screen->ViewPort)))
   {  height=(dim.Nominal.MaxY-dim.Nominal.MinY+1)*4/5;
   }
   else
   {  height=screen->Height*4/5;
   }
   return height;
}

void Popdrawer(void *winobj,struct Window *window,struct Gadget *layout,
   UBYTE *title,struct Gadget *gad)
{  struct FileRequester *fr;
   if(fr=AllocAslRequestTags(ASL_FileRequest,
      ASLFR_Window,window,
      ASLFR_TitleText,title,
      ASLFR_InitialHeight,Reqheight(window->WScreen),
      ASLFR_InitialDrawer,Getvalue(gad,STRINGA_TextVal),
      ASLFR_DoSaveMode,TRUE,
      ASLFR_DrawersOnly,TRUE,
      TAG_END))
   {  SetGadgetAttrs(layout,window,NULL,LAYOUT_DeferLayout,FALSE,TAG_END);
      SetAttrs(winobj,WA_BusyPointer,TRUE,GA_ReadOnly,TRUE,TAG_END);
      if(AslRequest(fr,NULL))
      {  UBYTE *p=Dupstr(fr->fr_Drawer,-1);
         if(p)
         {  Setgadgetattrs(gad,window,NULL,
               STRINGA_TextVal,p,
               STRINGA_DispPos,0,
               TAG_END);
         }
      }
      SetAttrs(winobj,WA_BusyPointer,FALSE,GA_ReadOnly,FALSE,TAG_END);
      SetGadgetAttrs(layout,window,NULL,LAYOUT_DeferLayout,TRUE,TAG_END);
      FreeAslRequest(fr);
   }
}

void Popfile(void *winobj,struct Window *window,struct Gadget *layout,
   UBYTE *title,struct Gadget *gad)
{  struct FileRequester *fr;
   BYTE *path=(BYTE *)Getvalue(gad,STRINGA_TextVal);
   UBYTE *drawer=NULL,*file=NULL;
   if(path)
   {
      BYTE *pp = PathPart(path);
      drawer=Dupstr(path,pp - path);
      file=FilePart(path);
   }
   if(fr=AllocAslRequestTags(ASL_FileRequest,
      ASLFR_Window,window,
      ASLFR_TitleText,title,
      ASLFR_InitialHeight,Reqheight(window->WScreen),
      ASLFR_InitialDrawer,drawer?drawer:NULLSTRING,
      ASLFR_InitialFile,file?file:NULLSTRING,
      ASLFR_RejectIcons,TRUE,
      TAG_END))
   {  SetGadgetAttrs(layout,window,NULL,LAYOUT_DeferLayout,FALSE,TAG_END);
      SetAttrs(winobj,WA_BusyPointer,TRUE,GA_ReadOnly,TRUE,TAG_END);
      if(AslRequest(fr,NULL))
      {  long len=strlen(fr->fr_Drawer)+strlen(fr->fr_File)+2;
         if(path=ALLOCTYPE(UBYTE,len,0))
         {  strcpy(path,fr->fr_Drawer);
            AddPart(path,fr->fr_File,len);
            Setgadgetattrs(gad,window,NULL,
               STRINGA_TextVal,path,
               STRINGA_DispPos,0,
               TAG_END);
            FREE(path);
         }
      }
      SetAttrs(winobj,WA_BusyPointer,FALSE,GA_ReadOnly,FALSE,TAG_END);
      SetGadgetAttrs(layout,window,NULL,LAYOUT_DeferLayout,TRUE,TAG_END);
      FreeAslRequest(fr);
   }
   if(drawer) FREE(drawer);
}

UBYTE *Prefsscreenmodename(ULONG modeid,long w,long h,long d)
{  struct NameInfo ni={{0}};
   UBYTE *p=NULL;
   long len;
   if(!GetDisplayInfoData(NULL,(UBYTE *)&ni,sizeof(ni),DTAG_NAME,modeid))
      strcpy(ni.Name,"(????)");
   len=strlen(ni.Name)+24;
   if(p=ALLOCTYPE(UBYTE,len,MEMF_PUBLIC))
   {  sprintf(p,"%s, %ld × %ld × %d",ni.Name,w,h,1<<d);
   }
   return p;
}

/* If (cp) nonnull, find best pen for this rgb. If (cp) null, use (pen) */
enum CGADGET_IDS
{  CGID_OK=1,CGID_CANCEL,CGID_GRADIENT,CGID_WHEEL,
   CGID_RED,CGID_GREEN,CGID_BLUE,
};
BOOL Popcolor(void *winobj,struct Window *pwin,struct Gadget *layout,
   struct Colorprefs *cp,long pen)
{  struct Window *win=NULL;
   struct IntuiMessage *msg;
   struct Gadget *cwgad=0,*gsgad=0,*rgad,*ggad,*bgad;
   struct Screen *scr=pwin->WScreen;
   struct ColorMap *cmap=scr->ViewPort.ColorMap;
   struct ColorWheelRGB rgb,undo;
   struct ColorWheelHSB hsb;
   struct NewGadget ng={0};
   struct Gadget *glist=NULL,*gad;
   struct TextAttr ta={0};
   UBYTE rbuf[]="R:%3ld",bbuf[]="B:%3ld",gbuf[]="G:%3ld";
   BOOL result=FALSE;
   WORD pens[3];
   BOOL done=FALSE;
   ULONG value32;
   short gadh,gadw,labelw,sliderw,winw,winh,offx,offy;
   ULONG windowmask,gotmask;
   SetGadgetAttrs(layout,pwin,NULL,LAYOUT_DeferLayout,FALSE,TAG_END);
   SetAttrs(winobj,
      WA_BusyPointer,TRUE,
      GA_ReadOnly,TRUE,
      TAG_END);
   if(cp)
   {  pen=ObtainBestPen(cmap,cp->red,cp->green,cp->blue,TAG_END);
      rgb.cw_Red=cp->red;
      rgb.cw_Green=cp->green;
      rgb.cw_Blue=cp->blue;
   }
   else
   {  GetRGB32(cmap,pen,1,&rgb.cw_Red);
      undo=rgb;
   }
   pens[0]=ObtainBestPen(cmap,0xffffffff,0xffffffff,0xffffffff,TAG_END);
   pens[1]=ObtainBestPen(cmap,0x00000000,0x00000000,0x00000000,TAG_END);
   pens[2]=-1;
   offx=scr->WBorLeft;
   offy=scr->RastPort.TxHeight+scr->WBorTop+1;
   gadh=scr->RastPort.TxHeight+4;
   gadw=TextLength(&scr->RastPort,AWEBSTR(MSG_SET_COLOUR_CANCEL),6)+16;
   labelw=6*scr->RastPort.Font->tf_XSize;
   winh=80+4*gadh+6*4;
   winw=2*gadw+16;
   if(winw<152) winw=152;
   sliderw=winw-8-labelw;
   ta.ta_Name=scr->RastPort.Font->tf_Message.mn_Node.ln_Name;
   ta.ta_YSize=scr->RastPort.Font->tf_YSize;
   gad=CreateContext(&glist);
   ng.ng_TextAttr=&ta;
   ng.ng_VisualInfo=visualinfo;
   ng.ng_LeftEdge=offx+4;
   ng.ng_TopEdge=offy+3*gadh+100;
   ng.ng_Width=gadw;
   ng.ng_Height=gadh;
   ng.ng_GadgetText=AWEBSTR(MSG_SET_COLOUR_OK);
   ng.ng_GadgetID=CGID_OK;
   gad=CreateGadget(BUTTON_KIND,gad,&ng,TAG_END);
   ng.ng_LeftEdge=offx+winw-4-gadw;
   ng.ng_GadgetText=AWEBSTR(MSG_SET_COLOUR_CANCEL);
   ng.ng_GadgetID=CGID_CANCEL;
   gad=CreateGadget(BUTTON_KIND,gad,&ng,TAG_END);
   ng.ng_LeftEdge=offx+4;
   ng.ng_TopEdge=offy+88;
   ng.ng_Width=sliderw;
   ng.ng_Height=gadh;
   ng.ng_GadgetText=NULL;
   ng.ng_GadgetID=CGID_RED;
   rbuf[0]=*AWEBSTR(MSG_SET_COLOUR_RED);
   gbuf[0]=*AWEBSTR(MSG_SET_COLOUR_GREEN);
   bbuf[0]=*AWEBSTR(MSG_SET_COLOUR_BLUE);
   rgad=gad=CreateGadget(SLIDER_KIND,gad,&ng,
      GTSL_Min,0,
      GTSL_Max,255,
      GTSL_Level,rgb.cw_Red>>24,
      GTSL_LevelFormat,rbuf,
      GTSL_MaxLevelLen,5,
      GTSL_LevelPlace,PLACETEXT_RIGHT,
      TAG_END);
   ng.ng_TopEdge+=4+gadh;
   ng.ng_GadgetID=CGID_GREEN;
   ggad=gad=CreateGadget(SLIDER_KIND,gad,&ng,
      GTSL_Min,0,
      GTSL_Max,255,
      GTSL_Level,rgb.cw_Green>>24,
      GTSL_LevelFormat,gbuf,
      GTSL_MaxLevelLen,5,
      GTSL_LevelPlace,PLACETEXT_RIGHT,
      TAG_END);
   ng.ng_TopEdge+=4+gadh;
   ng.ng_GadgetID=CGID_BLUE;
   bgad=gad=CreateGadget(SLIDER_KIND,gad,&ng,
      GTSL_Min,0,
      GTSL_Max,255,
      GTSL_Level,rgb.cw_Blue>>24,
      GTSL_LevelFormat,bbuf,
      GTSL_MaxLevelLen,5,
      GTSL_LevelPlace,PLACETEXT_RIGHT,
      TAG_END);
   if(gad
   && (gsgad=(struct Gadget *)NewObject(NULL,"gradientslider.gadget",
      GA_Top,offy+4,
      GA_Left,offx+88,
      GA_Width,16,
      GA_Height,80,
      GRAD_PenArray,pens,
      PGA_Freedom,LORIENT_VERT,
      GA_ID,CGID_GRADIENT,
      ICA_TARGET,ICTARGET_IDCMP,
      TAG_END))
   && (cwgad=(struct Gadget *)NewObject(NULL,"colorwheel.gadget",
      GA_Top,offy+4,
      GA_Left,offx+4,
      GA_Width,80,
      GA_Height,80,
      WHEEL_RGB,&rgb,
      WHEEL_Screen,scr,
      WHEEL_GradientSlider,gsgad,
      GA_Previous,gsgad,
      GA_ID,CGID_WHEEL,
      ICA_TARGET,ICTARGET_IDCMP,
      TAG_END))
   && (win=OpenWindowTags(NULL,
      WA_Left,pwin->LeftEdge+20,
      WA_Top,pwin->TopEdge+20,
      WA_InnerWidth,winw,
      WA_InnerHeight,winh,
      WA_PubScreen,scr,
      WA_DragBar,TRUE,
      WA_DepthGadget,TRUE,
      WA_CloseGadget,TRUE,
      WA_AutoAdjust,TRUE,
      WA_Activate,TRUE,
      WA_SimpleRefresh,TRUE,
      WA_Title,AWEBSTR(MSG_SET_COLOUR_TITLE),
      WA_IDCMP,IDCMP_CLOSEWINDOW|IDCMP_IDCMPUPDATE|IDCMP_REFRESHWINDOW|
         BUTTONIDCMP|SLIDERIDCMP,
      TAG_END)))
   {  AddGList(win,gsgad,-1,-1,NULL);
      AddGList(win,glist,-1,-1,NULL);
      RefreshGList(gsgad,win,NULL,-1);
      GT_RefreshWindow(win,NULL);
      DrawBevelBox(win->RPort,offx+108,offy+4,winw-112,80,
         GT_VisualInfo,visualinfo,
         GTBB_Recessed,TRUE,
         TAG_END);
      SetAPen(win->RPort,pen);
      RectFill(win->RPort,offx+110,offy+5,offx+winw-7,offy+82);
      windowmask=1<<win->UserPort->mp_SigBit;
      while(!done)
      {  gotmask=Wait(windowmask);
         while(msg=GT_GetIMsg(win->UserPort))
         {  switch(msg->Class)
            {  case IDCMP_CLOSEWINDOW:
                  done=TRUE;
                  break;
               case IDCMP_IDCMPUPDATE:
                  GetAttr(WHEEL_HSB,cwgad,(ULONG *)&hsb);
                  ConvertHSBToRGB(&hsb,&rgb);
                  GT_SetGadgetAttrs(rgad,win,NULL,
                     GTSL_Level,rgb.cw_Red>>24,TAG_END);
                  GT_SetGadgetAttrs(ggad,win,NULL,
                     GTSL_Level,rgb.cw_Green>>24,TAG_END);
                  GT_SetGadgetAttrs(bgad,win,NULL,
                     GTSL_Level,rgb.cw_Blue>>24,TAG_END);
                  if(cp)
                  {  ReleasePen(cmap,pen);
                     pen=ObtainBestPen(cmap,
                        rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue,TAG_END);
                     SetAPen(win->RPort,pen);
                     RectFill(win->RPort,offx+110,offy+5,offx+winw-7,offy+82);
                  }
                  else SetRGB32(&scr->ViewPort,pen,
                     rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue);
                  break;
               case IDCMP_GADGETUP:
                  switch(((struct Gadget *)msg->IAddress)->GadgetID)
                  {  case CGID_OK:
                        if(cp)
                        {  cp->red=rgb.cw_Red;
                           cp->green=rgb.cw_Green;
                           cp->blue=rgb.cw_Blue;
                        }
                        done=TRUE;
                        result=TRUE;
                        break;
                     case CGID_CANCEL:
                        if(!cp)
                        {  SetRGB32(&scr->ViewPort,pen,
                              undo.cw_Red,undo.cw_Green,undo.cw_Blue);
                        }
                        done=TRUE;
                        break;
                  }
                  break;
               case IDCMP_MOUSEMOVE:
                  value32=msg->Code;
                  value32|=value32<<8;
                  value32|=value32<<16;
                  switch(((struct Gadget *)msg->IAddress)->GadgetID)
                  {  case CGID_RED:    rgb.cw_Red=value32;break;
                     case CGID_GREEN:  rgb.cw_Green=value32;break;
                     case CGID_BLUE:   rgb.cw_Blue=value32;break;
                  }
                  Setgadgetattrs(cwgad,win,NULL,
                     WHEEL_RGB,&rgb,TAG_END);
                  if(cp)
                  {  ReleasePen(cmap,pen);
                     pen=ObtainBestPen(cmap,
                        rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue,TAG_END);
                     SetAPen(win->RPort,pen);
                     RectFill(win->RPort,offx+110,offy+5,offx+winw-7,offy+82);
                  }
                  else SetRGB32(&scr->ViewPort,pen,
                     rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue);
                  break;
               case IDCMP_REFRESHWINDOW:
                  GT_BeginRefresh(win);
                  GT_EndRefresh(win,TRUE);
                  break;
            }
            GT_ReplyIMsg(msg);
         }
      }
   }
   if(win) CloseWindow(win);
   if(cwgad) DisposeObject(cwgad);
   if(gsgad) DisposeObject(gsgad);
   FreeGadgets(glist);
   if(pens[0]>=0) ReleasePen(cmap,pens[0]);
   if(pens[1]>=0) ReleasePen(cmap,pens[1]);
   SetAttrs(winobj,
      WA_BusyPointer,FALSE,
      GA_ReadOnly,FALSE,
      TAG_END);
   SetGadgetAttrs(layout,pwin,NULL,LAYOUT_DeferLayout,TRUE,TAG_END);
   return result;
}

long Moveselected(struct Window *win,struct Gadget *gad,struct List *list,short n)
{  long selected;
   BOOL update=FALSE;
   selected=Getvalue(gad,LISTBROWSER_Selected);
   if(n<0 && selected>0)
   {  selected--;
      update=TRUE;
   }
   else if(n>0)
   {  struct Node *node=Getnode(list,selected);
      if(node && node->ln_Succ->ln_Succ)
      {  selected++;
         update=TRUE;
      }
   }
   if(update)
   {  Setgadgetattrs(gad,win,NULL,
         LISTBROWSER_Selected,selected,
         LISTBROWSER_MakeVisible,selected,
         TAG_END);
   }
   return update?selected:-1;
}

/* Aks for file name to load/save settings. Returns dynamic string */
UBYTE *Filereq(void *winobj,struct Window *window,struct Gadget *layout,
   UBYTE *title,UBYTE *name,BOOL save)
{  struct FileRequester *fr;
   UBYTE *path=NULL;
   if(fr=AllocAslRequestTags(ASL_FileRequest,
      ASLFR_Window,window,
      ASLFR_SleepWindow,TRUE,
      ASLFR_TitleText,title,
      ASLFR_InitialHeight,Reqheight(window->WScreen),
      ASLFR_InitialDrawer,"ENVARC:" DEFAULTCFG,
      ASLFR_InitialFile,name,
      ASLFR_RejectIcons,TRUE,
      ASLFR_DoSaveMode,save,
      TAG_END))
   {  SetGadgetAttrs(layout,window,NULL,LAYOUT_DeferLayout,FALSE,TAG_END);
      SetAttrs(winobj,WA_BusyPointer,TRUE,GA_ReadOnly,TRUE,TAG_END);
      if(AslRequest(fr,NULL))
      {  long len=strlen(fr->fr_Drawer)+strlen(fr->fr_File)+2;
         if(path=ALLOCTYPE(UBYTE,len,0))
         {  strcpy(path,fr->fr_Drawer);
            AddPart(path,fr->fr_File,len);
         }
      }
      SetAttrs(winobj,WA_BusyPointer,FALSE,GA_ReadOnly,FALSE,TAG_END);
      SetGadgetAttrs(layout,window,NULL,LAYOUT_DeferLayout,TRUE,TAG_END);
      FreeAslRequest(fr);
   }
   return path;
}

UBYTE *Hotkey(UBYTE *label)
{  UBYTE *p=strchr(label,'_');
   if(p) return p+1;
   else return NULL;
}

BOOL Ownscreen(void)
{  return (BOOL)STRIEQUAL(screenname,"AWeb");
}

void Dimensions(struct Window *window,short *dim)
{  short x,y,w,h;
   x=window->LeftEdge;
   y=window->TopEdge;
   w=window->Width-window->BorderLeft-window->BorderRight;
   h=window->Height-window->BorderTop-window->BorderBottom;
   if(x!=dim[0] || y!=dim[1] || w!=dim[2] || h!=dim[3])
   {  dim[0]=x;
      dim[1]=y;
      dim[2]=w;
      dim[3]=h;
      newdimensions=TRUE;
   }
}

/* Insert the first 2 chars of text into a stringgadget at the */
/* current cursorposition and activate the stringgadget afterwards */
void Insertinstringgadget(struct Window *window,struct Gadget *gad,UBYTE *text)
{
   WORD   pos, numchars, maxchars;
   UBYTE *oldtext=NULL, *newtext=NULL;
   struct StringInfo *stringinfo=gad->SpecialInfo;

   pos=stringinfo->BufferPos;
   numchars=stringinfo->NumChars;
   maxchars=stringinfo->MaxChars;
   GetAttr(STRINGA_TextVal,gad,(ULONG *)&oldtext);

   if(numchars < (maxchars-2))
   {
      if(newtext=ALLOCTYPE(UBYTE,maxchars,0))
      {
         strncpy(newtext,oldtext,pos);
         strncat(newtext,text,2);
         strncat(newtext,oldtext+pos,strlen(oldtext)-pos);
         Setgadgetattrs(gad,window,NULL,
                        STRINGA_TextVal,newtext,
                        TAG_END);
         Setgadgetattrs(gad,window,NULL,
                        STRINGA_BufferPos,pos+2,
                        TAG_END);
         FREE(newtext);
         ActivateGadget(gad,window,NULL);
      }
   }
}

int main(int fromcli,char **argv)
{  struct WBStartup *wbs=0;
   ULONG gotmask,cfgmask;
   short nrwindows=0;
   struct Library *WorkbenchBase;

   if(fromcli==0) wbs=(struct WBStartup *)argv;

   if(WorkbenchBase=OpenLibrary("workbench.library",0))
   {  if(WorkbenchBase->lib_Version>=44) has35=TRUE;
      CloseLibrary(WorkbenchBase);
   }
   if (Openlib("locale.library",OSNEED(0,44),(struct Library **)&LocaleBase,(struct Interface **)&ILocale))
   {
      localeinfo.li_LocaleBase=(struct Library *)LocaleBase;
      localeinfo.li_Catalog=OpenCatalogA(NULL,"awebcfg.catalog",NULL);
      maincatalog=OpenCatalogA(NULL,"aweb.catalog",NULL);

   }
   if(Openlib("intuition.library",OSNEED(39,40),(struct Library **)&IntuitionBase,(struct Interface **)&IIntuition)
   && Openlib("utility.library",OSNEED(39,40),(struct Library **)&UtilityBase,(struct Interface **)&IUtility)
   && Openlib("graphics.library",OSNEED(39,40),(struct Library **)&GfxBase,(struct Interface **)&IGraphics)
   && Openlib("asl.library",OSNEED(39,44),&AslBase,(struct Interface **)&IAsl)
   && Openlib("gadgets/colorwheel.gadget",OSNEED(39,44),&ColorWheelBase,(struct Interface **)&IColorWheel)
   && Openlib("gadtools.library",OSNEED(39,40),&GadToolsBase,(struct Interface **)&IGadTools)
   && Openlib("icon.library",OSNEED(39,44),&IconBase,(struct Interface **)&IIcon)

   && Openclass("images/glyph.image",OSNEED(0,44),&GlyphBase,(struct Interface **)&IGlyph)
   && Openclass("window.class",OSNEED(0,44),&WindowBase,(struct Interface **)&IWindow)
   && Openclass("gadgets/layout.gadget",OSNEED(0,44),&LayoutBase,(struct Interface **)&ILayout)
   && Openclass("gadgets/string.gadget",OSNEED(0,44),&StringBase,(struct Interface **)&IString)
   && Openclass("gadgets/space.gadget",OSNEED(0,44),&SpaceBase,(struct Interface **)&ISpace)
   && Openclass("images/label.image",OSNEED(0,44),&LabelBase,(struct Interface **)&ILabel)
   && Openclass("gadgets/checkbox.gadget",OSNEED(0,44),&CheckBoxBase,(struct Interface **)&ICheckBox)
   && Openclass("gadgets/integer.gadget",OSNEED(0,44),&IntegerBase,(struct Interface **)&IInteger)
   && Openclass("gadgets/chooser.gadget",OSNEED(0,44),&ChooserBase,(struct Interface **)&IChooser)
   && Openclass("gadgets/listbrowser.gadget",OSNEED(0,44),&ListBrowserBase,(struct Interface **)&IListBrowser)
   && Openclass("gadgets/button.gadget",0,&ButtonBase,(struct Interface **)&IButton)
   && Openclass("gadgets/palette.gadget",0,&PaletteBase,(struct Interface **)&IPalette)
   && Openclass("gadgets/clicktab.gadget",0,&ClickTabBase,(struct Interface **)&IClickTab)
   && Openclass("gadgets/gradientslider.gadget",39,&GradientSliderBase,(struct Interface **)&IGradientSlider)

   && Initmemory()
   )
   {
#ifndef NEED35
      if(has35)
      {  Adjustmenus35();
      }
#endif
      Initdefprefs();
      Localizemenus();
      Getarguments(fromcli?NULL:wbs);
      if(Dupstartupcheck())
      {  cfgmask=1<<cfgport->mp_SigBit;
         if((pubscreen=LockPubScreen(*screenname?screenname:NULL))
         && (visualinfo=GetVisualInfo(pubscreen,TAG_END))
         && (drawinfo=GetScreenDrawInfo(pubscreen))
         && (amigaimg=NewObject(NULL,"sysiclass",
               SYSIA_DrawInfo,drawinfo,
               SYSIA_Which,AMIGAKEY,
               TAG_END))
         )
         {  Loadsettingsprefs(&setprefs,FALSE,NULL);
            if(browser && Openbrowser()) nrwindows++;
            if(program && Openprogram()) nrwindows++;
            if(gui && Opengui()) nrwindows++;
            if(network && Opennetwork()) nrwindows++;
            if(nrwindows == 0)
            {
               printf("\n%s %s\n\n",AWEBSTR(MSG_ERROR_COMMANDLINE),argtemplate);
            }

            while(nrwindows)
            {  gotmask=Wait(brmask|prmask|uimask|nwmask|cfgmask|SIGBREAKF_CTRL_C);
               if(gotmask&SIGBREAKF_CTRL_C) break;
               if(gotmask&brmask)
               {  if(!Processbrowser()) nrwindows--;
               }
               if(gotmask&prmask)
               {  if(!Processprogram()) nrwindows--;
               }
               if(gotmask&uimask)
               {  if(!Processgui()) nrwindows--;
               }
               if(gotmask&nwmask)
               {  if(!Processnetwork()) nrwindows--;
               }
               if(gotmask&cfgmask)
               {  Processcfgport();
               }
               if(cfgcommand&CFGCLASS_QUIT) break;
               if((cfgcommand&CFGCLASS_BROWSER) && Openbrowser()) nrwindows++;
               if((cfgcommand&CFGCLASS_PROGRAM) && Openprogram()) nrwindows++;
               if((cfgcommand&CFGCLASS_GUI) && Opengui()) nrwindows++;
               if((cfgcommand&CFGCLASS_NETWORK) && Opennetwork()) nrwindows++;
               if(cfgcommand&CFGCLASS_SNAPSHOT)
               {  Savesettingsprefs(&setprefs,TRUE,NULL);
                  Savesettingsprefs(&setprefs,FALSE,NULL);
                  newdimensions=FALSE;
               }
               if(cfgcommand&CFGCLASS_CLASSACT) Openclassact();
               cfgcommand=0;
            }
            Closebrowser();
            Closeprogram();
            Closegui();
            Closenetwork();
            if(newdimensions) Savesettingsprefs(&setprefs,FALSE,NULL);
         }
      }
   }
   Cleanup();
}
