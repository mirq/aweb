/******.****************************************************************
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

/* window.c - AWeb window object */

#include "aweb.h"
#include "window.h"
#include "url.h"
#include "frame.h"
#include "application.h"
#include "winhis.h"
#include "winprivate.h"
#include "jslib.h"
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <intuition/pointerclass.h>
#include <intuition/icclass.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#undef NO_INLINE_STDARG
#include <reaction/reaction.h>

#define NO_INLINE_STDARG

#ifdef __MORPHOS__
#include <intuition/extensions.h>
#endif

//#include "awebboopsimacros.h"

//#if defined (__amigaos4__)
//#define STATUSGAD (IIntuition->NewObject)(Stagadclass(), NULL
//#else
#define STATUSGAD NewObject (Stagadclass(), NULL
//#endif

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>
#include <proto/utility.h>
#include <proto/wb.h>

#if !defined(__MORPHOS__) && !defined(__amigaos4__)
#include <images/titlebar.h>
#include <proto/titlebarimage.h>

struct Library *TitlebarImageBase = NULL;
#endif

#ifdef REACTION_SpecialPens
#undef REACTION_SpecialPens
#endif
#define REACTION_SpecialPens TAG_IGNORE

AwindowList_t windows;

static short nextx=-1,nexty;
static UBYTE buf2[MAXSTRBUFCHARS],buf3[MAXSTRBUFCHARS];
static long windowkey=0;

struct Awindow *activewindow;

#if defined(__amigaos4__)

static ULONG idcmpflags=IDCMP_CHANGEWINDOW|IDCMP_INTUITICKS|IDCMP_REFRESHWINDOW|
   IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|
   IDCMP_MOUSEMOVE|IDCMP_GADGETUP|IDCMP_MENUPICK|IDCMP_IDCMPUPDATE|
   IDCMP_RAWKEY|IDCMP_SIZEVERIFY|
   IDCMP_GADGETDOWN|IDCMP_INACTIVEWINDOW|IDCMP_ACTIVEWINDOW|IDCMP_EXTENDEDMOUSE;

#else

static ULONG idcmpflags=IDCMP_CHANGEWINDOW|IDCMP_INTUITICKS|IDCMP_REFRESHWINDOW|
   IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|
   IDCMP_MOUSEMOVE|IDCMP_GADGETUP|IDCMP_MENUPICK|IDCMP_IDCMPUPDATE|
   IDCMP_RAWKEY|IDCMP_SIZEVERIFY|
   IDCMP_GADGETDOWN|IDCMP_INACTIVEWINDOW|IDCMP_ACTIVEWINDOW;

#endif

UBYTE *urlpops[]=
{  "http://www.",
   "http://",
   "https://",
   "ftp://ftp.",
   "gopher://gopher.",
   "mailto:",
   "news:",
   "file://localhost/",
   NULL,
};

/*------------------------------------------------------------------------*/
/* Default images */

#define DEFAULTWIDTH    16
#define DEFAULTHEIGHT   14

static UWORD backdata[]=
{  0x0000,0x0000,0x0000,0x0200,0x0200,0x0201,0x0001,0x8001,
   0x43ff,0x2200,0x1200,0x0e00,0x0000,0x0000,
   0x0000,0x0000,0x0e00,0x1000,0x2000,0x41fe,0x8000,0x0000,
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};
static UWORD fwddata[]=
{  0x0000,0x0000,0x0000,0x0008,0x0004,0x0002,0x0001,0x0001,
   0x7fc2,0x0004,0x0008,0x0030,0x0000,0x0000,
   0x0000,0x0000,0x0070,0x0040,0x0040,0xffc0,0x8000,0x8000,
   0x8000,0x0040,0x0040,0x0040,0x0000,0x0000,
};
static UWORD homedata[]=
{  0x0000,0x0000,0x0004,0x0064,0x001c,0x0006,0x0e72,0x0842,
   0x0842,0x0802,0x0802,0x0802,0x3ffe,0x0000,
   0x0000,0x0000,0x0198,0x0610,0x1800,0x6000,0x4108,0x4108,
   0x4138,0x4100,0x4100,0x4700,0x4000,0x0000,
};
static UWORD hotdata[]=
{  0x0000,0x0000,0x1004,0x13fc,0x1001,0x13ff,0x1080,0x1380,
   0x7004,0x03fc,0x1001,0x73ff,0x0000,0x0000,
   0x0000,0x0000,0xe7f8,0x8400,0x87fe,0x8400,0x8700,0x8400,
   0x87f8,0x0400,0xe7fe,0x8400,0x0000,0x0000,
};
static UWORD canceldata[]=
{  0x0000,0x0000,0x0002,0x2004,0x1008,0x0810,0x0420,0x0000,
   0x0000,0x0180,0x0240,0x3c3e,0x0000,0x0000,
   0x0000,0x0000,0x7c3c,0x0240,0x0180,0x0000,0x0000,0x0420,
   0x0810,0x1008,0x2004,0x4000,0x0000,0x0000,
};
static UWORD nwsdata[]=
{  0x0000,0x0000,0x0000,0x0000,0x0040,0x0042,0x0042,0x0606,
   0x1818,0x2060,0x0180,0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,0x01c0,0x0606,0x1818,0x6020,0x4000,
   0x4200,0x4200,0x0200,0x0000,0x0000,0x0000,
};
static UWORD rlddata[]=
{  0x0000,0x0000,0x0004,0x07e2,0x0802,0x080e,0x0800,0x0808,
   0x0004,0x0002,0x1fe4,0x0008,0x0010,0x0000,
   0x0000,0x1ff8,0x2000,0x4000,0x4010,0x4010,0x4030,0x4020,
   0x47e0,0x2000,0x0000,0x0020,0x0020,0x0000,
};
static UWORD imgdata[]=
{  0x0000,0x0000,0x0200,0x0100,0x0202,0x1c02,0x0002,0x0002,
   0x011e,0x0100,0x0080,0x1e80,0x03c0,0x0000,
   0x0000,0x1c00,0x2000,0x4000,0x203c,0x0020,0x0020,0x0220,
   0x0420,0x0800,0x1000,0x2000,0x0000,0x0000,
};
static UWORD addhotdata[]=
{  0x0000,0x0000,0x0204,0x027c,0x0201,0x023f,0x0060,0x0060,
   0x73c4,0x027c,0x0201,0x0e7f,0x0000,0x0000,
   0x0000,0x0000,0x1cf8,0x1080,0x10fe,0xf1c0,0x8000,0x8000,
   0x8038,0x1080,0x10fe,0x1080,0x0000,0x0000,
};
static UWORD searchdata[]=
{  0x0000,0x0000,0x0e00,0x3180,0x2020,0x4020,0x4020,0x4000,
   0x2020,0x2060,0x0099,0x1f06,0x0000,0x0000,
   0x0000,0x1f00,0x2080,0x4040,0x8080,0x8040,0x8040,0x8060,
   0x8098,0x5186,0x2e00,0x0000,0x0000,0x0000,
};
static UWORD unsecuredata[]=
{  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};
static UWORD securedata[]=
{  0x0000,0x0000,0x01e0,0x0210,0x0210,0x0000,0x0008,0x01c8,
   0x01c8,0x0088,0x01c8,0x0008,0x07f8,0x0000,
   0x0000,0x01c0,0x0200,0x0420,0x0420,0x0ff8,0x0800,0x0800,
   0x0800,0x0800,0x0800,0x0800,0x0800,0x0000,
};

/*------------------------------------------------------------------------*/

/* Find the NewMenu item */
static struct NewMenu *Findmenudata(struct NewMenu *menus,UBYTE *cmd)
{  struct NewMenu *nm=menus;
   struct Menuentry *me;
   for(me=prefs.gui.menus.first;me->next;me=me->next,nm++)
   {  if(STRIEQUAL(me->cmd,cmd)) return nm;
   }
   return NULL;
}

/* Check or uncheck one NewMenu item */
static void Checkmenu(struct NewMenu *nm,BOOL check)
{  if(check) nm->nm_Flags|=CHECKED;
   else nm->nm_Flags&=~CHECKED;
}

/* Check or uncheck NewMenu items */
static void Setmenus(struct Awindow *win,struct NewMenu *nmenus)
{  struct NewMenu *nm;
   if(nm=Findmenudata(nmenus,"@LOADIMGOFF"))
      Checkmenu(nm,prefs.network.loadimg==LOADIMG_OFF);
   if(nm=Findmenudata(nmenus,"@LOADIMGMAPS"))
      Checkmenu(nm,prefs.network.loadimg==LOADIMG_MAPS);
   if(nm=Findmenudata(nmenus,"@LOADIMGALL"))
      Checkmenu(nm,prefs.network.loadimg==LOADIMG_ALL);
   if(nm=Findmenudata(nmenus,"@DRAGGING"))
      Checkmenu(nm,BOOLVAL(win->flags&WINF_CLIPDRAG));
   if(nm=Findmenudata(nmenus,"@NOPROXY"))
      Checkmenu(nm,BOOLVAL(win->flags&WINF_NOPROXY));
   if(nm=Findmenudata(nmenus,"@BGIMAGES"))
      Checkmenu(nm,prefs.browser.docolors);
   if(nm=Findmenudata(nmenus,"@BGSOUND"))
      Checkmenu(nm,prefs.browser.dobgsound);
}

/* Set a new window title */
static void Settitle(struct Awindow *win,UBYTE *title)
{  UBYTE *newtitle;
   if(newtitle=Windowtitle(win,(UBYTE *)Agetattr(win->frame,AOFRM_Name),title))
   {  if(win->window && (prefs.gui.windowborder == 0)) SetWindowTitles(win->window,newtitle,(UBYTE *)~0);
      if(win->wintitle) FREE(win->wintitle);
      win->wintitle=newtitle;
   }
}

/* Enable or disable the history gadgets and menu items */
static void Sethisgadgets(struct Awindow *win)
{  void *prev,*next;
   if(win && win->window)
   {  prev=(void *)Agetattr(win->hiswhis,AOWHS_Previous);
      next=(void *)Agetattr(win->hiswhis,AOWHS_Next);
      if(win->backgad)
      {  Setgadgetattrs(win->backgad,win->window,NULL,
            GA_Disabled,!prev,
            TAG_END);
      }
      if(win->fwdgad)
      {  Setgadgetattrs(win->fwdgad,win->window,NULL,
            GA_Disabled,!next,
            TAG_END);
      }
   }
}

/* Enable or disable the cancel gadget and menu item */
static void Setcancel(struct Awindow *win,BOOL onoff)
{  if(win->window )
   {  if(win->cancelgad)
      {  Setgadgetattrs(win->cancelgad,win->window,NULL,
            GA_Disabled,!onoff,
            GA_Selected,FALSE,
            TAG_END);
      }
   }
}

/* Enable or disable the cancel gadget depending on if fetches are going on */
static void Setwincancel(struct Awindow *win)
{  BOOL input=Agetattr(win->whis,AOWHS_Input);
   if(input && !(win->flags&WINF_INPUT))
   {  win->flags|=WINF_INPUT;
      Setcancel(win,TRUE);
   }
   else if(!input && (win->flags&WINF_INPUT))
   {  win->flags&=~WINF_INPUT;
      Setcancel(win,FALSE);
   }
}

/* Set a custom pointer */
void Setawinpointer(struct Awindow *win,UWORD ptrtype)
{  void *ptr;
   /* Only set the pointer if it changes */
   if(win->window && ptrtype!=win->ptrtype)
   {  
#if defined(__amigaos4__)
	switch(ptrtype)
	{
		case APTR_DEFAULT:
		{
		    SetWindowPointer(win->window,WA_PointerType,POINTERTYPE_NORMAL,WA_PointerDelay,TRUE,TAG_END);
		    break;
		}
		case APTR_RESIZEHOR:
		{
		    SetWindowPointer(win->window,WA_PointerType,POINTERTYPE_EASTWESTRESIZE,WA_PointerDelay,TRUE,TAG_END);
		    break;
		}
		case APTR_RESIZEVERT:
		{
		    SetWindowPointer(win->window,WA_PointerType,POINTERTYPE_NORTHSOUTHRESIZE,WA_PointerDelay,TRUE,TAG_END);
		    break;
		}
		case APTR_HAND:
		{
		    SetWindowPointer(win->window,WA_PointerType,POINTERTYPE_LINK,WA_PointerDelay,TRUE,TAG_END);
		    break;
		}
	}
#else
      ptr=Apppointer((struct Application *)Aweb(),ptrtype);
      SetWindowPointer(win->window,WA_Pointer,ptr,WA_PointerDelay,TRUE,TAG_END);
#endif
      win->ptrtype=ptrtype;
   }
}

/* Make menu bar and set it */
static void Newwindowmenus(struct Awindow *win,struct NewMenu *menus)
{  struct DrawInfo *drinfo=NULL;
   void *vinfo=NULL;
   if(menus)
   {  Agetattrs(Aweb(),
         AOAPP_Drawinfo,(Tag)&drinfo,
         AOAPP_Visualinfo,(Tag)&vinfo,
         TAG_END);
      Setmenus(win,menus);
      if((win->menu=CreateMenus(menus,
         GTMN_FrontPen,drinfo->dri_Pens[BARDETAILPEN],
         TAG_END))
      && LayoutMenus(win->menu,vinfo,TAG_END)
      && win->window)
      {  SetMenuStrip(win->window,win->menu);
      }
   }
}

/* Remove menu bar and free it */
static void Freewindowmenus(struct Awindow *win)
{  if(win->window)
   {  ClearMenuStrip(win->window);
   }
   if(win->menu)
   {  FreeMenus(win->menu);
      win->menu=NULL;
   }
}

/* Update the slider. Scale attributes if necessary. If top==~0, use existing
 * top value. */
static void Setslider(struct Awindow *win,struct Scrollgad *gs,
   ULONG total,ULONG vis,ULONG top)
{  short shift=0;
   if(total!=gs->total || vis!=gs->vis || top!=~0)
   {  gs->total=total;
      gs->vis=vis;
      while(total>0xffff)
      {  total>>=1;
         shift++;
      }
      vis>>=shift;
      if(top==~0 && shift!=gs->shift)
      {  /* We must scale the existing top value too */
         GetAttr(PGA_Top,gs->gad,&top);
         top<<=gs->shift;
      }
      gs->shift=shift;
      if(top!=~0) top>>=shift;
      Setgadgetattrs(gs->gad,win->window,NULL,
         PGA_Total,total,
         PGA_Visible,vis,
         top!=~0?PGA_Top:TAG_IGNORE,top,
         TAG_END);
   }
}

/* Speedbar sets its minimum size too large if too many buttons are there, but
 * needs at least 1 button to get its height right.
 * Actually it needs 2 because with 1 button the height isn't calculated correctly...
 */

/* Add a speedbar node for this button */
static void Addspeedbutton(struct Awindow *win,struct DrawInfo *dri,
   struct Userbutton *ub,short n)
{  struct Node *node;
   void *label;
   label=LabelObject,
         LABEL_DrawInfo,dri,
         LABEL_Text,Dupstr(ub->label,-1),
            /* will dangle in the memory pool after disposal of the label...
             * must copy it because new buttons will trash the label text otherwise */
      End;
   if(label)
   {  SetAttrs(label,IA_Width,Getvalue(label,IA_Width)+4,TAG_END);
      node=AllocSpeedButtonNode(n,
         SBNA_Image,label,
         SBNA_Spacing,0,
         SBNA_Enabled,BOOLVAL(n>=0),
         TAG_END);
      if(node)
      {  AddTail(&win->userbutlist,node);
      }
   }
}

/* Create the user button row with 2 bogus buttons */
static void *Makebuttonrow(struct Awindow *win,struct DrawInfo *dri)
{  void *buttonrow=NULL;
   win->ubutgad=NULL;
   if(!ISEMPTY(&prefs.gui.buttons) && prefs.gui.showbuttons && (win->flags&WINF_BUTTONS))
   {  Addspeedbutton(win,dri,prefs.gui.buttons.first,-1);
      Addspeedbutton(win,dri,prefs.gui.buttons.first,-1);
      buttonrow=HLayoutObject,
         LAYOUT_SpaceOuter,TRUE,
#ifdef __MORPHOS__
         LAYOUT_BottomSpacing,1,
         LAYOUT_LeftSpacing,1,
         LAYOUT_RightSpacing,1,
#endif
         LAYOUT_SpaceInner,FALSE,
         LAYOUT_TopSpacing,0,
         StartMember,win->ubutgad=SpeedBarObject,
            GA_ID,GID_UBUTTON,
            GA_RelVerify,TRUE,
            SPEEDBAR_EvenSize,TRUE,
            SPEEDBAR_Buttons,&win->userbutlist,
            SPEEDBAR_BevelStyle,BVS_NONE,
            REACTION_SpecialPens,&win->capens,
         EndMember,
      End;
   }
   return buttonrow;
}

/* Create the the real user buttons */
static void Completebuttonrow(struct Awindow *win,struct DrawInfo *dri)
{  struct Userbutton *ub;
   short n;
   if(win->ubutgad)
   {  Setgadgetattrs(win->ubutgad,NULL,NULL,SPEEDBAR_Buttons,~0,TAG_END);
      for(ub=prefs.gui.buttons.first,n=0;ub->next;ub=ub->next,n++)
      {  Addspeedbutton(win,dri,ub,n);
      }
      Setgadgetattrs(win->ubutgad,NULL,NULL,SPEEDBAR_Buttons,&win->userbutlist,TAG_END);
   }
}

static void Rebuildbuttonrow(struct Awindow *win,struct DrawInfo *dri)
{  struct Node *node;
   void *label;
   struct Userbutton *ub;
   short n;
   if(win->ubutgad)
   {  Setgadgetattrs(win->ubutgad,win->window,NULL,SPEEDBAR_Buttons,~0,TAG_END);
      while(node=RemHead(&win->userbutlist))
      {  GetSpeedButtonNodeAttrs(node,SBNA_Image,&label,TAG_END);
         DisposeObject((Object *)label);
         FreeSpeedButtonNode(node);
      }
      Addspeedbutton(win,dri,prefs.gui.buttons.first,-1);
      Addspeedbutton(win,dri,prefs.gui.buttons.first,-1);
      for(ub=prefs.gui.buttons.first,n=0;ub->next;ub=ub->next,n++)
      {  Addspeedbutton(win,dri,ub,n);
      }
      Setgadgetattrs(win->ubutgad,win->window,NULL,SPEEDBAR_Buttons,&win->userbutlist,TAG_END);
   }
}

/* not used!!!
static struct DrawInfo *Copydrawinfo(struct DrawInfo *drawinfo)
{
    struct DrawInfo *newdrawinfo = ALLOCSTRUCT(DrawInfo,1,0);

    if(newdrawinfo)
    {
        memcpy(newdrawinfo,drawinfo,sizeof(struct DrawInfo));
        newdrawinfo->dri_Pens = ALLOCTYPE(UWORD,drawinfo->dri_NumPens,0);
        memcpy(newdrawinfo->dri_Pens, drawinfo->dri_Pens, drawinfo->dri_NumPens * sizeof(UWORD));
    }
    return newdrawinfo;
}
*/

/* This function *must only* be used to free drawinfo's allocated with */
/* Copydrawinfo */

/* not used!!!
static void Freedrawinfo(struct DrawInfo *drawinfo)
{
    FREE(drawinfo->dri_Pens);
    FREE(drawinfo);
}
*/

void Makebutton(struct Awindow *win, struct Gadget **button, struct Image *img, struct Image *selimg, UBYTE *text, ULONG gid, BOOL disabled)
{
    struct img *label,*selectlabel = NULL;
    struct DrawInfo *drwinfo = (struct DrawInfo *)Agetattr(Aweb(),AOAPP_Drawinfo);

    label = LabelObject,
              ((prefs.gui.textbuttons<2)?LABEL_Image:TAG_IGNORE),img,
              LABEL_DrawInfo,drwinfo,
              ((prefs.gui.textbuttons == 1)?LABEL_Text:TAG_IGNORE),"\n",
              ((prefs.gui.textbuttons > 0)?LABEL_Text:TAG_IGNORE),text,
              LABEL_Justification,LJ_CENTER,
            End;

    selectlabel = LabelObject,
              ((prefs.gui.textbuttons<2)?LABEL_Image:TAG_IGNORE),selimg?selimg:img,
              LABEL_DrawInfo,drwinfo,
              ((prefs.gui.textbuttons>0)?IA_FGPen:TAG_IGNORE),drwinfo->dri_Pens[FILLPEN],
              ((prefs.gui.textbuttons == 1)?LABEL_Text:TAG_IGNORE),"\n",
              ((prefs.gui.textbuttons>0)?LABEL_Text:TAG_IGNORE),text,
              LABEL_Justification,LJ_CENTER,
            End;

    *button = ButtonObject,
                     GA_Disabled,disabled,
                     GA_ID,gid,
                     GA_Image,label,
                     GA_SelectRender,selectlabel,
                     GA_RelVerify,TRUE,
                     GA_Immediate,FALSE,
                     REACTION_SpecialPens,&win->capens,
                     (prefs.gui.nobevel?BUTTON_BevelStyle:TAG_IGNORE),BVS_NONE,
                     BUTTON_Transparent,TRUE,
              End;
}

void *Makenavigationblock(struct Awindow *win)
{
    void *navblock = NULL;

    Makebutton(win,&win->backgad,win->backimg,win->backselimg,AWEBSTR(MSG_NAVGAD_BACK),GID_NAV + 0,TRUE);
    Makebutton(win,&win->fwdgad,win->fwdimg,win->fwdselimg,AWEBSTR(MSG_NAVGAD_FORWARD),GID_NAV + 1,TRUE);
    Makebutton(win,&win->homegad,win->homeimg,win->homeselimg,AWEBSTR(MSG_NAVGAD_HOME),GID_NAV + 2,FALSE);
    Makebutton(win,&win->addhotgad,win->addhotimg,win->addhotselimg,AWEBSTR(MSG_NAVGAD_ADDHOTLIST),GID_NAV + 3,FALSE);
    Makebutton(win,&win->hotgad,win->hotimg,win->hotselimg,AWEBSTR(MSG_NAVGAD_HOTLIST),GID_NAV + 4,FALSE);
    Makebutton(win,&win->cancelgad,win->cancelimg,win->cancelselimg,AWEBSTR(MSG_NAVGAD_CANCEL),GID_NAV + 5,TRUE);

    Makebutton(win,&win->nwsgad,win->nwsimg,win->nwsselimg,AWEBSTR(MSG_NAVGAD_STATUS),GID_NAV + 6,FALSE);
    Makebutton(win,&win->searchgad,win->searchimg,win->searchselimg,AWEBSTR(MSG_NAVGAD_SEARCH),GID_NAV + 7,FALSE);
    Makebutton(win,&win->rldgad,win->rldimg,win->rldselimg,AWEBSTR(MSG_NAVGAD_RELOAD),GID_NAV + 8,FALSE);
    Makebutton(win,&win->imggad,win->imgimg,win->imgselimg,AWEBSTR(MSG_NAVGAD_IMAGES),GID_NAV + 9,FALSE);

    if(prefs.gui.buttonspos <=1)
    {

       navblock=
            VLayoutObject,
               LAYOUT_InnerSpacing,1,
               StartMember,HLayoutObject,
                  LAYOUT_InnerSpacing,1,
                  LAYOUT_EvenSize,TRUE,
                  LAYOUT_AddChild,win->backgad,
                  LAYOUT_AddChild,win->fwdgad,
                  LAYOUT_AddChild, win->homegad,
                  LAYOUT_AddChild, win->addhotgad,
                  LAYOUT_AddChild, win->hotgad,
               EndMember,
               StartMember,HLayoutObject,
                  LAYOUT_InnerSpacing,1,
                  LAYOUT_EvenSize,TRUE,
                  LAYOUT_AddChild, win->cancelgad,
                  LAYOUT_AddChild, win->nwsgad,
                  LAYOUT_AddChild, win->searchgad,
                  LAYOUT_AddChild, win->rldgad,
                  LAYOUT_AddChild, win->imggad,
               EndMember,
            End;


    }
    else if(prefs.gui.buttonspos==2 || prefs.gui.buttonspos==3)
    {
        navblock = HLayoutObject,
                LAYOUT_InnerSpacing,5,
                (prefs.gui.buttonspos == 3)?LAYOUT_AddChild:TAG_IGNORE,SpaceObject,
                EndMember,
           //     (prefs.gui.buttonspos == 3)?CHILD_WeightedWidth:TAG_IGNORE,0,

               StartMember,HLayoutObject,
                  LAYOUT_InnerSpacing,1,
                  LAYOUT_EvenSize,TRUE,
                  LAYOUT_AddChild, win->cancelgad,
                  LAYOUT_AddChild,win->backgad,
                  LAYOUT_AddChild,win->fwdgad,
                  LAYOUT_AddChild, win->homegad,
                  StartMember,SpaceObject,
                  End,
                  LAYOUT_AddChild, win->rldgad,
                  LAYOUT_AddChild, win->imggad,
                  StartMember,SpaceObject,
                  End,
                  LAYOUT_AddChild, win->addhotgad,
                  LAYOUT_AddChild, win->hotgad,
                  LAYOUT_AddChild, win->nwsgad,
                  LAYOUT_AddChild, win->searchgad,

               EndMember,

                  CHILD_WeightedWidth,0,
                (prefs.gui.buttonspos == 2)?LAYOUT_AddChild:TAG_IGNORE,SpaceObject,
                EndMember,
           //     (prefs.gui.buttonspos == 2)?CHILD_WeightedWidth:TAG_IGNORE,0,

               End;

    }else{

            navblock=
            VLayoutObject,
               LAYOUT_InnerSpacing,1,
               StartMember,HLayoutObject,
                  LAYOUT_InnerSpacing,1,
                  LAYOUT_EvenSize,TRUE,
                  LAYOUT_AddChild,win->backgad,
                  LAYOUT_AddChild,win->fwdgad,
                  LAYOUT_AddChild, win->cancelgad,
                  LAYOUT_AddChild, win->rldgad,
                  LAYOUT_AddChild, win->homegad,
               EndMember,
            End;

    }
    return navblock;
}

/* Open intuition window */
static BOOL Openwindow(struct Awindow *win)
{  void *url=(void *)Agetattr(win->frame,AOFRM_Url);
   UBYTE *urlname=(UBYTE *)Agetattr(url,AOURL_Url);
   struct Screen *screen=NULL;
   struct ColorMap *colormap=NULL;
   struct DrawInfo *drinfo=NULL;
   struct TextFont *font=NULL;
   struct MsgPort *windowport=NULL;
   struct MsgPort *appwindowport;
   struct NewMenu *menus=NULL;
   struct LayoutLimits limits={0};
   struct Node *node;
   void *buttonrow;
   void *navblock;
   UWORD innerwidth = 0;
   UWORD innerheight = 0;

   
   struct IBox *bx,*zbx; /* ptrs to our box and zbox structures */
   ULONG bgrgb[3];
   short i;
   
   if(win->pubscreenname)
   {
       if((screen = LockPubScreen(win->pubscreenname)))
       {
           win->screenlocked = TRUE;
       }
   }
   
   if(ISEMPTY(&win->urlpoplist))
   {  for(i=0;urlpops[i];i++)
      {  if(node=AllocChooserNode(CNA_Text,urlpops[i],TAG_END))
         {  AddTail(&win->urlpoplist,node);
         }
      }
   }
   Agetattrs(Aweb(),
      screen?TAG_IGNORE:AOAPP_Screen,(Tag)&screen,
      AOAPP_Colormap,(Tag)&colormap,
      AOAPP_Drawinfo,(Tag)&drinfo,
      AOAPP_Screenfont,(Tag)&font,
      AOAPP_Windowport,(Tag)&windowport,
      AOAPP_Menus,(Tag)&menus,
      TAG_END);

   Asetattrs(Aweb(),
      AOAPP_Processtype,AOTP_WINDOW,
      AOAPP_Processfun,(Tag)Processwindow,
      TAG_END);
     if(!(win->spacegad = (struct Gadget *)NewObject(SPACE_GetClass(),NULL,  TAG_DONE)))
     {
         return FALSE;
     }
            
   if(!win->box.Width)
   {  if(nextx<0)
      {  nextx=prefs.window.winx;
         nexty=prefs.window.winy;
      }

      if(win->leftset)
      {
          win->box.Left = win->newleft;
      }
      else
      {
            win->box.Left=nextx;
            nextx+=20;
      }
      if(win->topset)
      {
          win->box.Top = win->newtop; 
      }
      else
      {
          nexty+=8;
          win->box.Top=nexty;          
      }
      
      if(nextx+prefs.window.winw>screen->Width) nextx=0;
      if(nexty+prefs.window.winh>screen->Height) nexty=0;
      win->box.Width=prefs.window.winw;
      win->box.Height=prefs.window.winh;
      SetAttrs(win->spacegad,
          win->newwidth?SPACE_MinWidth:TAG_IGNORE,win->newwidth,
          win->newheight?SPACE_MinHeight:TAG_IGNORE,win->newheight,
          TAG_DONE);
    
      win->zoombox.Left=prefs.window.wiax;
      win->zoombox.Top=prefs.window.wiay;
      win->zoombox.Width=prefs.window.wiaw;
      win->zoombox.Height=prefs.window.wiah;
   }



   if(prefs.gui.shownav && (win->flags&WINF_NAVS))
   {  if(!(win->backimg=Buttonimage2(Aweb(),BUTF_BACK,FALSE,backdata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->fwdimg=Buttonimage2(Aweb(),BUTF_FORWARD,FALSE,fwddata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->homeimg=Buttonimage2(Aweb(),BUTF_HOME,FALSE,homedata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->addhotimg=Buttonimage2(Aweb(),BUTF_ADDHOTLIST,FALSE,addhotdata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->hotimg=Buttonimage2(Aweb(),BUTF_HOTLIST,FALSE,hotdata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->cancelimg=Buttonimage2(Aweb(),BUTF_CANCEL,FALSE,canceldata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->nwsimg=Buttonimage2(Aweb(),BUTF_NETSTATUS,FALSE,nwsdata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->searchimg=Buttonimage2(Aweb(),BUTF_SEARCH,FALSE,searchdata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->rldimg=Buttonimage2(Aweb(),BUTF_RELOAD,FALSE,rlddata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->imgimg=Buttonimage2(Aweb(),BUTF_LOADIMAGES,FALSE,imgdata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->unsecureimg=Buttonimage(Aweb(),BUTF_UNSECURE,unsecuredata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      if(!(win->secureimg=Buttonimage(Aweb(),BUTF_SECURE,securedata,DEFAULTWIDTH,DEFAULTHEIGHT))) return FALSE;
      win->backselimg=Buttonimage2(Aweb(),BUTF_BACK,TRUE,backdata,DEFAULTWIDTH,DEFAULTHEIGHT);
      win->fwdselimg=Buttonimage2(Aweb(),BUTF_FORWARD,TRUE,fwddata,DEFAULTWIDTH,DEFAULTHEIGHT);
      win->homeselimg=Buttonimage2(Aweb(),BUTF_HOME,TRUE,homedata,DEFAULTWIDTH,DEFAULTHEIGHT);
      win->addhotselimg=Buttonimage2(Aweb(),BUTF_ADDHOTLIST,TRUE,addhotdata,DEFAULTWIDTH,DEFAULTHEIGHT);
      win->hotselimg=Buttonimage2(Aweb(),BUTF_HOTLIST,TRUE,hotdata,DEFAULTWIDTH,DEFAULTHEIGHT);
      win->cancelselimg=Buttonimage2(Aweb(),BUTF_CANCEL,TRUE,canceldata,DEFAULTWIDTH,DEFAULTHEIGHT);
      win->nwsselimg=Buttonimage2(Aweb(),BUTF_NETSTATUS,TRUE,nwsdata,DEFAULTWIDTH,DEFAULTHEIGHT);
      win->searchselimg=Buttonimage2(Aweb(),BUTF_SEARCH,TRUE,searchdata,DEFAULTWIDTH,DEFAULTHEIGHT);
      win->rldselimg=Buttonimage2(Aweb(),BUTF_RELOAD,TRUE,rlddata,DEFAULTWIDTH,DEFAULTHEIGHT);
      win->imgselimg=Buttonimage2(Aweb(),BUTF_LOADIMAGES,TRUE,imgdata,DEFAULTWIDTH,DEFAULTHEIGHT);

   }

   buttonrow=Makebuttonrow(win,drinfo);

   navblock=Makenavigationblock(win);
   win->ledgad=Animgadget(Aweb(),&win->capens);

   win->layoutgad=(struct Gadget *)VLayoutObject,
      GA_ID,GID_LAYOUT,
      LAYOUT_SpaceInner,FALSE,
      TAG_END);
   if(!win->layoutgad) return FALSE;
   if(prefs.gui.shownav && (win->flags&WINF_NAVS))
   {

      if(prefs.gui.buttonspos <= 1 || prefs.gui.buttonspos >= 4 )
      {
          SetAttrs(win->layoutgad,
             LAYOUT_AddChild,
             HLayoutObject,
                LAYOUT_SpaceOuter,TRUE,
#ifdef __MORPHOS__
                LAYOUT_TopSpacing,2,
                LAYOUT_BottomSpacing,2,
                LAYOUT_LeftSpacing,1,
                LAYOUT_RightSpacing,1,
                LAYOUT_InnerSpacing,1,
#endif
                (prefs.gui.buttonspos == 1 || prefs.gui.buttonspos == 5 )?LAYOUT_AddChild:TAG_IGNORE,win->ledgad,
                CHILD_WeightedHeight,0,

                (prefs.gui.buttonspos == 0 || prefs.gui.buttonspos == 4 )?LAYOUT_AddChild:TAG_IGNORE,navblock,
                (prefs.gui.buttonspos == 0 || prefs.gui.buttonspos == 4 )?CHILD_WeightedWidth:TAG_IGNORE,0,

                StartMember,VLayoutObject,

                   LAYOUT_InnerSpacing,1,


                   StartMember,HLayoutObject,
                      LAYOUT_SpaceInner,FALSE,
                      StartMember,win->urlgad=StringObject,
                         GA_ID,GID_URL,
                         GA_RelVerify,TRUE,
                         GA_Immediate,TRUE,
                         STRINGA_MaxChars,MAXSTRBUFCHARS,
                         STRINGA_Buffer,win->urlbuf,
                         STRINGA_UndoBuffer,buf2,
                         STRINGA_WorkBuffer,buf3,
                         STRINGA_TextVal,urlname?urlname:NULLSTRING,
                         REACTION_SpecialPens,&win->capens,
                      EndMember,
                      StartMember,win->urlpopgad=ChooserObject,
                         GA_ID,GID_URLPOP,
                         GA_RelVerify,TRUE,
                         CHOOSER_DropDown,TRUE,
                         CHOOSER_Labels,&win->urlpoplist,
                         CHOOSER_AutoFit,TRUE,
                         REACTION_SpecialPens,&win->capens,
                      EndMember,
                   EndMember,
                   StartMember,HLayoutObject,
                      LAYOUT_SpaceInner,FALSE,
                      StartMember,win->statusgad=STATUSGAD,
                         STATGA_SpecialPens,&win->capens,
                         GA_Text,win->statustext,
                      EndMember,
                      StartMember,win->securegad=ButtonObject,
                         GA_Image,win->unsecureimg,
                         GA_SelectRender,win->secureimg,
                         GA_ReadOnly,TRUE,
                         REACTION_SpecialPens,&win->capens,
                         BUTTON_FillPen,win->capens.sp_LightPen,
                      EndMember,
                      CHILD_WeightedWidth,0,
                   EndMember,
                   CHILD_WeightedHeight,0,
                EndMember,
                (prefs.gui.buttonspos == 1 || prefs.gui.buttonspos == 5 )?LAYOUT_AddChild:TAG_IGNORE,navblock,
                (prefs.gui.buttonspos == 1 || prefs.gui.buttonspos == 5 )?CHILD_WeightedWidth:TAG_IGNORE,0,
                (prefs.gui.buttonspos == 0 || prefs.gui.buttonspos == 4 )?LAYOUT_AddChild:TAG_IGNORE,win->ledgad,

             EndMember,
             CHILD_WeightedHeight,0,
         TAG_END);
      }
      else
      {
          SetAttrs(win->layoutgad,
             LAYOUT_AddChild,
             HLayoutObject,
                LAYOUT_SpaceOuter,TRUE,
#ifdef __MORPHOS__
                LAYOUT_TopSpacing,2,
                LAYOUT_BottomSpacing,2,
                LAYOUT_LeftSpacing,1,
                LAYOUT_RightSpacing,1,
                LAYOUT_InnerSpacing,1,
#endif

                StartMember,win->ledgad,
                CHILD_WeightedHeight,0,


                StartMember,VLayoutObject,

                   LAYOUT_InnerSpacing,1,
                   LAYOUT_AddChild,navblock,


                   StartMember,HLayoutObject,
                      LAYOUT_SpaceInner,FALSE,
                      StartMember,win->urlgad=StringObject,
                         GA_ID,GID_URL,
                         GA_RelVerify,TRUE,
                         GA_Immediate,TRUE,
                         STRINGA_MaxChars,MAXSTRBUFCHARS,
                         STRINGA_Buffer,win->urlbuf,
                         STRINGA_UndoBuffer,buf2,
                         STRINGA_WorkBuffer,buf3,
                         STRINGA_TextVal,urlname?urlname:NULLSTRING,
                         REACTION_SpecialPens,&win->capens,
                      EndMember,
                      StartMember,win->urlpopgad=ChooserObject,
                         GA_ID,GID_URLPOP,
                         GA_RelVerify,TRUE,
                         CHOOSER_DropDown,TRUE,
                         CHOOSER_Labels,&win->urlpoplist,
                         CHOOSER_AutoFit,TRUE,
                         REACTION_SpecialPens,&win->capens,
                      EndMember,
                   EndMember,
                   StartMember,HLayoutObject,
                      LAYOUT_SpaceInner,FALSE,
                      StartMember,win->statusgad=STATUSGAD,
                         STATGA_SpecialPens,&win->capens,
                         GA_Text,win->statustext,
                      EndMember,
                      StartMember,win->securegad=ButtonObject,
                         GA_Image,win->unsecureimg,
                         GA_SelectRender,win->secureimg,
                         GA_ReadOnly,TRUE,
                         REACTION_SpecialPens,&win->capens,
                         BUTTON_FillPen,win->capens.sp_LightPen,
                      EndMember,
                      CHILD_WeightedWidth,0,
                   EndMember,
                EndMember,

             EndMember,
             CHILD_WeightedHeight,0,
         TAG_END);

      }
   }
   else
   {  win->backgad=NULL;
      win->fwdgad=NULL;
      win->rldgad=NULL;
      win->homegad=NULL;
      win->cancelgad=NULL;
      win->hotgad=NULL;
      win->imggad=NULL;
      win->nwsgad=NULL;
      win->urlgad=NULL;
      win->urlpopgad=NULL;
      win->statusgad=NULL;
      win->searchgad=NULL;
      win->ledgad=NULL;
      win->addhotgad=NULL;
      win->securegad=NULL;
   }

   if(buttonrow)
   {  if(!(prefs.gui.shownav && (win->flags&WINF_NAVS)))
      {  SetAttrs(win->layoutgad,
            StartMember,SpaceObject,
            EndMember,
            CHILD_MinHeight,2,
            CHILD_MaxHeight,2,
            TAG_END);
      }
      SetAttrs(win->layoutgad,
         StartMember,buttonrow,
         CHILD_WeightedHeight,0,
         TAG_END);
   }

   SetAttrs(win->layoutgad,
      StartMember,VLayoutObject,
         LAYOUT_BevelStyle,(buttonrow || (prefs.gui.shownav && (win->flags&WINF_NAVS)))?
            BVS_SBAR_VERT:BVS_NONE,
#ifndef __MORPHOS__
         LAYOUT_SpaceOuter,TRUE,
#else
        LAYOUT_SpaceOuter,FALSE,
#endif
         StartMember,win->spacegad,
//         CHILD_MinHeight,40,
//         CHILD_MinWidth,80,
      EndMember,
      TAG_END);
      
   if(win->newwidth || win->newheight)
   {
       LayoutLimits(win->layoutgad,&limits,NULL,screen);
       if(win->newwidth)
       {
           innerwidth = limits.MinWidth;
       }
       
       if(win->newheight)
       {
           innerheight = limits.MinHeight;
       }    
   }
   win->newwidth = 0;
   win->newheight = 0;
   
   GetRGB32(colormap,drinfo->dri_Pens[BACKGROUNDPEN],1,bgrgb);
   if(bgrgb[0] < 0x22222222) bgrgb[0] = 0x22222222;
   if(bgrgb[1] < 0x22222222) bgrgb[1] = 0x22222222;
   if(bgrgb[2] < 0x22222222) bgrgb[2] = 0x22222222;

   if(bgrgb[0] > 0xffffffff - 0x22222222) bgrgb[0] = 0xffffffff - 0x22222222;
   if(bgrgb[1] > 0xffffffff - 0x22222222) bgrgb[1] = 0xffffffff - 0x22222222;
   if(bgrgb[2] > 0xffffffff - 0x22222222) bgrgb[2] = 0xffffffff - 0x22222222;



   win->capens.sp_LightPen=ObtainBestPen(colormap,
      bgrgb[0]+0x22222222,bgrgb[1]+0x22222222,bgrgb[2]+0x22222222,
      OBP_FailIfBad,FALSE,
      OBP_Precision,PRECISION_EXACT,
      TAG_END);
   win->capens.sp_DarkPen=ObtainBestPen(colormap,
      bgrgb[0]-0x22222222,bgrgb[1]-0x22222222,bgrgb[2]-0x22222222,
      OBP_FailIfBad,FALSE,
      OBP_Precision,PRECISION_EXACT,
      TAG_END);
   if(win->flags & WINF_ZOOMED)
   {
       zbx = &win->box;
       bx  = &win->zoombox;
       win->flags &= ~ WINF_ZOOMED;
   }
   else
   {
       bx   = &win->box;
       zbx  = &win->zoombox;

   }
   if(!(win->window=OpenWindowTags(NULL,
      WA_Left,bx->Left,
      WA_Top,bx->Top,
      WA_Borderless,(prefs.gui.windowborder == 2)?TRUE:FALSE,
      innerwidth?TAG_IGNORE:WA_Width,bx->Width,
      innerheight?TAG_IGNORE:WA_Height,bx->Height,
      innerwidth?WA_InnerWidth:TAG_IGNORE,innerwidth,
      innerheight?WA_InnerHeight:TAG_IGNORE,innerheight,
      WA_PubScreen,screen,
      WA_SizeGadget,(prefs.gui.windowborder > 0)?FALSE:TRUE,
      WA_DragBar,(prefs.gui.windowborder > 0)?FALSE:TRUE,
      WA_DepthGadget,(prefs.gui.windowborder > 0)?FALSE:TRUE,
      WA_CloseGadget,(prefs.gui.windowborder > 0)?FALSE:TRUE,
      WA_Activate,TRUE,
      WA_SimpleRefresh,TRUE,
      WA_SizeBRight,TRUE,
      WA_SizeBBottom,TRUE,
      WA_NewLookMenus,TRUE,
      WA_AutoAdjust,TRUE,
      WA_ReportMouse,TRUE,
      WA_MaxWidth,-1,
      WA_MaxHeight,-1,
#ifdef __MORPHOS__
                        WA_ExtraGadget_Iconify, TRUE,
#endif
      (prefs.gui.windowborder > 0)?TAG_IGNORE:WA_Zoom,zbx,
      TAG_END))) 
    {
      return FALSE;
    }


   Settitle(win,AWEBSTR(MSG_AWEB_NODOCTITLE));
   Setactiveport(win->portname);
   activewindow=win;
   win->window->UserData=(BYTE *)win;
   win->window->UserPort=windowport;
   ModifyIDCMP(win->window,idcmpflags);
   Newwindowmenus(win,menus);
   if(!(win->downimg=(struct Image *)NewObject(NULL,"sysiclass",
      SYSIA_DrawInfo,drinfo,
      SYSIA_Which,DOWNIMAGE,
      TAG_END))) return FALSE;
   if(!(win->upimg=(struct Image *)NewObject(NULL,"sysiclass",
      SYSIA_DrawInfo,drinfo,
      SYSIA_Which,UPIMAGE,
      TAG_END))) return FALSE;
   if(!(win->rightimg=(struct Image *)NewObject(NULL,"sysiclass",
      SYSIA_DrawInfo,drinfo,
      SYSIA_Which,RIGHTIMAGE,
      TAG_END))) return FALSE;
   if(!(win->leftimg=(struct Image *)NewObject(NULL,"sysiclass",
      SYSIA_DrawInfo,drinfo,
      SYSIA_Which,LEFTIMAGE,
      TAG_END))) return FALSE;
   if(!(win->iconify_img=(struct Image *)NewObject(NULL,"sysiclass",
      SYSIA_DrawInfo,drinfo,
      SYSIA_Which,ICONIFYIMAGE,
      TAG_END))) return FALSE;      
      
 //    win->iconify_img = (struct Image *)NewObject (NULL, "sysiclass",  SYSIA_DrawInfo, drinfo, SYSIA_Which, TBFRAMEIMAGE, SYSIA_FullFrame, TRUE,TAG_END);

   if(prefs.gui.windowborder == 0)
   {
       if(!(win->downarrow=(struct Gadget *)NewObject(NULL,"buttongclass",
          GA_RelRight,-win->window->BorderRight+1,
          GA_RelBottom,-win->window->BorderBottom-win->downimg->Height+1,
          GA_Image,win->downimg,
          GA_ID,GID_DOWN,
          ICA_TARGET,ICTARGET_IDCMP,
          TAG_END))) return FALSE;
       if(!(win->uparrow=(struct Gadget *)NewObject(NULL,"buttongclass",
          GA_RelRight,-win->window->BorderRight+1,
          GA_RelBottom,-win->window->BorderBottom-
             win->downimg->Height-win->upimg->Height+1,
          GA_Image,win->upimg,
          GA_ID,GID_UP,
          ICA_TARGET,ICTARGET_IDCMP,
          GA_Previous,win->downarrow,
          TAG_END))) return FALSE;
       if(!(win->vslider.gad=(struct Gadget *)NewObject(NULL,"propgclass",
          GA_RelRight,-win->window->BorderRight+5,
          GA_Width,win->window->BorderRight-8,
          GA_Top,win->window->BorderTop+2,
          GA_RelHeight,-win->window->BorderTop-win->window->BorderBottom-
             win->downimg->Height-win->upimg->Height-4,
          GA_ID,GID_VSLIDER,
          PGA_Freedom,FREEVERT,
          PGA_Total,1,
          PGA_Visible,1,
          PGA_NewLook,TRUE,
          PGA_Borderless,TRUE,
          ICA_TARGET,ICTARGET_IDCMP,
          GA_Previous,win->uparrow,
          TAG_END))) return FALSE;
       if(!(win->rightarrow=(struct Gadget *)NewObject(NULL,"buttongclass",
          GA_RelRight,-win->window->BorderRight-win->rightimg->Width+1,
          GA_RelBottom,-win->window->BorderBottom+1,
          GA_Image,win->rightimg,
          GA_ID,GID_RIGHT,
          ICA_TARGET,ICTARGET_IDCMP,
          GA_Previous,win->vslider.gad,
          TAG_END))) return FALSE;
       if(!(win->leftarrow=(struct Gadget *)NewObject(NULL,"buttongclass",
          GA_RelRight,-win->window->BorderRight-
             win->rightimg->Width-win->leftimg->Width+1,
          GA_RelBottom,-win->window->BorderBottom+1,
          GA_Image,win->leftimg,
          GA_ID,GID_LEFT,
          ICA_TARGET,ICTARGET_IDCMP,
          GA_Previous,win->rightarrow,
          TAG_END))) return FALSE;
       if(!(win->hslider.gad=(struct Gadget *)NewObject(NULL,"propgclass",
          GA_Left,win->window->BorderLeft,
          GA_RelBottom,-win->window->BorderBottom+3,
          GA_RelWidth,-win->window->BorderLeft-win->window->BorderRight-
             win->rightimg->Width-win->leftimg->Width-2,
          GA_Height,win->window->BorderBottom-4,
          GA_ID,GID_HSLIDER,
          PGA_Freedom,FREEHORIZ,
          PGA_Total,1,
          PGA_Visible,1,
          PGA_NewLook,TRUE,
          PGA_Borderless,TRUE,
          ICA_TARGET,ICTARGET_IDCMP,
          GA_Previous,win->leftarrow,
          TAG_END))) return FALSE;
   }
   /* remove our sixe restriction on the space gad for newwidth new height */
   FlushLayoutDomainCache(win->layoutgad);
   SetGadgetAttrs(win->spacegad,NULL,NULL,SPACE_MinWidth, 100, SPACE_MinHeight,80,TAG_DONE);   
   
   LayoutLimits(win->layoutgad,&limits,NULL,screen);

   WindowLimits(win->window,
      limits.MinWidth+win->window->BorderLeft+win->window->BorderRight,
      limits.MinHeight+win->window->BorderTop+win->window->BorderBottom,
      0,0);
   SetGadgetAttrs(win->layoutgad,NULL,NULL,
      GA_Top,win->window->BorderTop,
      GA_Left,win->window->BorderLeft,
      GA_RelWidth,-win->window->BorderLeft-win->window->BorderRight,
      GA_RelHeight,-win->window->BorderTop-win->window->BorderBottom,
      ICA_TARGET,ICTARGET_IDCMP,
      TAG_END);
   if(buttonrow)
   {  Completebuttonrow(win,drinfo);
   }

   if(prefs.gui.windowborder == 0)
   {

#ifndef __MORPHOS__
#if !defined(__amigaos4__)
     //   create iconify gadget 
        if ( NULL != TitlebarImageBase )
        {
                       win->iconify_img = NewObject (NULL, "tbiclass", SYSIA_Which, ICONIFYIMAGE, SYSIA_DrawInfo, drinfo, TAG_END);

                     
                       if ( NULL != win->iconify_img )
                       {
                               win->iconify_gad = NewObject(NULL,"buttongclass",
                                       GA_RelRight, TBI_RELPOS(win->iconify_img,2),
                                       GA_Top, 0,
                                       GA_Width, win->iconify_img->Width - 1,
                                       GA_Height, win->iconify_img->Height,
                                       GA_TopBorder, TRUE,
                                       GA_Image, win->iconify_img,
                                       GA_RelVerify, TRUE,
                                       GA_ID, GID_ICONIFY,
                                       ICA_TARGET, ICTARGET_IDCMP,
                                       TAG_END );
                               if ( NULL != win->iconify_gad )
                                       AddGadget (win->window, win->iconify_gad, 0);
                       }

        }
#else

            //           win->iconify_img = (struct Image *)NewObject (NULL, "sysiclass",  SYSIA_DrawInfo, drinfo, SYSIA_Which, TBFRAMEIMAGE, SYSIA_FullFrame, TRUE,TAG_END);
                       
                       if ( NULL != win->iconify_img )
                       {
                               win->iconify_gad = NewObject(NULL,"buttongclass",
                                       GA_RelRight,TRUE,
                                       GA_Titlebar, TRUE,                               
                                       GA_Image, win->iconify_img,
                                       GA_RelVerify, TRUE,
                                       GA_ID, GID_ICONIFY,
                                       ICA_TARGET, ICTARGET_IDCMP,
                                     TAG_END );
                               if ( NULL != win->iconify_gad )
                                       AddGadget (win->window, win->iconify_gad, 0);
                       }

#endif        
#endif

            AddGList(win->window,win->downarrow,-1,-1,NULL);
       ((struct ExtGadget *)win->layoutgad)->MoreFlags&=~GMORE_SCROLLRASTER;
       AddGList(win->window,win->layoutgad,-1,-1,NULL);

            /* refresh gadget list -- iconify gadget will be the first in
            the list if it exists, else downarrow gadget */
            RefreshGList(win->iconify_gad?win->iconify_gad:win->downarrow,win->window,NULL,-1);
   }
   else
   {
       ((struct ExtGadget *)win->layoutgad)->MoreFlags&=~GMORE_SCROLLRASTER;
       AddGList(win->window,win->layoutgad,-1,-1,NULL);
       RefreshGList(win->layoutgad,win->window,NULL,-1);
   }
 
   	
   Asetattrs(win->frame,
      AOBJ_Width,win->window->Width,
      AOBJ_Height,win->window->Height,
      AOBJ_Window,(Tag)win,
      TAG_END);
   /* Set the box to what we actualy opened */
   
   win->box.Width = win->window->Width;
   win->box.Height = win->window->Height;   
   
   SetABPenDrMd(win->window->RPort,1,0,JAM1);
   Arender(win->frame,NULL,0,0,AMRMAX,AMRMAX,0,NULL);
   Adragrender(win->frame,NULL,NULL,0,NULL,0,AMDS_BEFORE);
   Sethisgadgets(win);
//   if(win->flags&WINF_ZOOMED) ZipWindow(win->window);
   Addanimgad(win->window,win->ledgad);
   if(appwindowport=(struct MsgPort *)Agetattr(Aweb(),AOAPP_Appwindowport))
   {  win->appwindow=AddAppWindow(win->key,0,win->window,appwindowport,TAG_END);
   }
   if(win->screenlocked)
   {
       UnlockPubScreen(NULL,screen);
       win->screenlocked = FALSE;
   }
   return TRUE;
}

/* Close the intuition window */
static void Closewindow(struct Awindow *win)
{  struct Node *node;
   struct ColorMap *colormap=(struct ColorMap *)Agetattr(Aweb(),AOAPP_Colormap);
   void *label;
   Remanimgad(win->ledgad);
   if(win->pubscreenname && win->screenlocked)
   {
       UnlockPubScreen(win->pubscreenname,NULL);
       win->screenlocked = FALSE;
   }
   
   if(win->frame)
   {  Asetattrs(win->frame,AOBJ_Window,0,TAG_END);
   }
   if(win->popup)
   {  Adisposeobject(win->popup);
      win->popup=NULL;
   }
   Freewindowmenus(win);
   if(win->appwindow)
   {  RemoveAppWindow(win->appwindow);
      win->appwindow=NULL;
   }
   if(win->window)
   {  Safeclosewindow(win->window);
      win->window=NULL;
   }
   if(win->capens.sp_LightPen!=-1)
   {  ReleasePen(colormap,win->capens.sp_LightPen);
      win->capens.sp_LightPen=-1;
   }
   if(win->capens.sp_DarkPen!=-1)
   {  ReleasePen(colormap,win->capens.sp_DarkPen);
      win->capens.sp_DarkPen=-1;
   }
   while(node=RemHead(&win->userbutlist))
   {  GetSpeedButtonNodeAttrs(node,SBNA_Image,&label,TAG_END);
      DisposeObject((Object *)label);
      FreeSpeedButtonNode(node);
   }

        /* dispose of iconify imagery */
        if ( NULL != win->iconify_gad )
        {
                DisposeObject((Object *)win->iconify_gad);
                win->iconify_gad = NULL;
        }
        if ( NULL != win->iconify_img )
        {
                DisposeObject((Object *)win->iconify_img);
                win->iconify_img = NULL;
        }


   if(win->downimg) DisposeObject((Object *)win->downimg);win->downimg=NULL;
   if(win->upimg) DisposeObject((Object *)win->upimg);win->upimg=NULL;
   if(win->rightimg) DisposeObject((Object *)win->rightimg);win->rightimg=NULL;
   if(win->leftimg) DisposeObject((Object *)win->leftimg);win->leftimg=NULL;

   if(win->backimg) DisposeObject((Object *)win->backimg);win->backimg=NULL;
   if(win->fwdimg) DisposeObject((Object *)win->fwdimg);win->fwdimg=NULL;
   if(win->homeimg) DisposeObject((Object *)win->homeimg);win->homeimg=NULL;
   if(win->addhotimg) DisposeObject((Object *)win->addhotimg);win->addhotimg=NULL;
   if(win->hotimg) DisposeObject((Object *)win->hotimg);win->hotimg=NULL;
   if(win->cancelimg) DisposeObject((Object *)win->cancelimg);win->cancelimg=NULL;
   if(win->nwsimg) DisposeObject((Object *)win->nwsimg);win->nwsimg=NULL;
   if(win->searchimg) DisposeObject((Object *)win->searchimg);win->searchimg=NULL;
   if(win->rldimg) DisposeObject((Object *)win->rldimg);win->rldimg=NULL;
   if(win->imgimg) DisposeObject((Object *)win->imgimg);win->imgimg=NULL;
   if(win->unsecureimg) DisposeObject((Object *)win->unsecureimg);win->unsecureimg=NULL;
   if(win->secureimg) DisposeObject((Object *)win->secureimg);win->secureimg=NULL;
   if(win->backselimg) DisposeObject((Object *)win->backselimg);win->backselimg=NULL;
   if(win->fwdselimg) DisposeObject((Object *)win->fwdselimg);win->fwdselimg=NULL;
   if(win->homeselimg) DisposeObject((Object *)win->homeselimg);win->homeselimg=NULL;
   if(win->addhotselimg) DisposeObject((Object *)win->addhotselimg);win->addhotselimg=NULL;
   if(win->hotselimg) DisposeObject((Object *)win->hotselimg);win->hotselimg=NULL;

   if(win->cancelselimg) DisposeObject((Object *)win->cancelselimg);win->cancelselimg=NULL;
   if(win->nwsselimg) DisposeObject((Object *)win->nwsselimg);win->nwsselimg=NULL;
   if(win->searchselimg) DisposeObject((Object *)win->searchselimg);win->searchselimg=NULL;
   if(win->rldselimg) DisposeObject((Object *)win->rldselimg);win->rldselimg=NULL;
   if(win->imgselimg) DisposeObject((Object *)win->imgselimg);win->imgselimg=NULL;

   if(win->downarrow) DisposeObject((Object *)win->downarrow);win->downarrow=NULL;
   if(win->uparrow) DisposeObject((Object *)win->uparrow);win->uparrow=NULL;
   if(win->vslider.gad) DisposeObject((Object *)win->vslider.gad);win->vslider.gad=NULL;
   if(win->rightarrow) DisposeObject((Object *)win->rightarrow);win->rightarrow=NULL;
   if(win->leftarrow) DisposeObject((Object *)win->leftarrow);win->leftarrow=NULL;
   if(win->hslider.gad) DisposeObject((Object *)win->hslider.gad);win->hslider.gad=NULL;

   if(win->layoutgad) DisposeObject((Object *)win->layoutgad);win->layoutgad=NULL;
   if(win->wintitle) FREE(win->wintitle);win->wintitle=NULL;
   if(win->drawinfo) FREE(win->drawinfo);win->drawinfo=NULL;
}

/* Process the App Message */
static void Processappmessage(struct Awindow *win,struct AppMessage *msg)
{  short i;
   UBYTE buffer[264]="file:///";
   UBYTE *filename=buffer+8;
   for(i=0;i<msg->am_NumArgs;i++)
   {  if(NameFromLock(msg->am_ArgList[i].wa_Lock,filename,256)
      && AddPart(filename,msg->am_ArgList[i].wa_Name,256))
      {  Followurlname(win,buffer,0);
      }
   }
}

/*------------------------------------------------------------------------*/

static long Setwindow(struct Awindow *win,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   UBYTE *status=(UBYTE *)~0,*hpstatus=(UBYTE *)~0;
   long total=0,read=0;
   ULONG htotal=win->hslider.total,hvis=win->hslider.vis,htop=~0;
   ULONG vtotal=win->vslider.total,vvis=win->vslider.vis,vtop=~0;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOAPP_Screenvalid:
            if(tag->ti_Data && !win->window)
            {  
                if(!(Openwindow(win)))
                {
                   Closewindow(win);  // Close window free any partly allocated resources on window open failure.
                }   
            }
            else
            {  Closewindow(win);
            }
            break;
         case AOAPP_Menus:
            if(tag->ti_Data)
            {  if(win->window) Newwindowmenus(win,(struct NewMenu *)tag->ti_Data);
            }
            else
            {  Freewindowmenus(win);
            }
            break;
         case AOWIN_Title:
            if(tag->ti_Data) Settitle(win,(UBYTE *)tag->ti_Data);
            break;
         case AOWIN_Vslidertotal:
            vtotal=tag->ti_Data;
            break;
         case AOWIN_Vslidervisible:
            vvis=tag->ti_Data;
            break;
         case AOWIN_Vslidertop:
            vtop=tag->ti_Data;
            break;
         case AOWIN_Hslidertotal:
            htotal=tag->ti_Data;
            break;
         case AOWIN_Hslidervisible:
            hvis=tag->ti_Data;
            break;
         case AOWIN_Hslidertop:
            htop=tag->ti_Data;
            break;
         case AOWIN_Activeurl:
            if(win->activeurl)
            {  Aremchild(win->activeurl,(struct Aobject *)win,AOREL_URL_WINDOW);
            }
            win->activeurl=(void *)tag->ti_Data;
            if(win->activeurl)
            {  Aaddchild(win->activeurl,(struct Aobject *)win,AOREL_URL_WINDOW);
            }
            status=NULL;
            total=read=0;
            break;
         case AOWIN_Status:
            status=(UBYTE *)tag->ti_Data;
            break;
         case AOWIN_Read:
            read=tag->ti_Data;
            break;
         case AOWIN_Total:
            total=tag->ti_Data;
            break;
         case AOWIN_Hpstatus:
            hpstatus=(UBYTE *)tag->ti_Data;
            break;
         case AOWIN_Goinactive:
            if((!tag->ti_Data && win->activeobject)
            || (win->activeobject==(void *)tag->ti_Data))
            {  if(!tag->ti_Data)
               {  Agoinactive(win->activeobject);
               }
               win->activeobject=NULL;
               if(win->hittext)
               {  if(win->statushptext) FREE(win->statushptext);
                  win->statushptext=NULL;
                  if(win->window && win->statusgad)
                  {  Setgadgetattrs(win->statusgad,win->window,NULL,
                        STATGA_HPText,NULL,
                        TAG_END);
                  }
                  FREE(win->hittext);
                  win->hittext=NULL;
               }
               if(win->window)
               {  win->window->Flags&=~WFLG_RMBTRAP;
               }
            }
            if(!tag->ti_Data || win->hitobject==(void *)tag->ti_Data)
            {  win->hitobject=NULL;
            }
            if(!tag->ti_Data || win->jonmouse==(void *)tag->ti_Data)
            {  win->jonmouse=NULL;
            }
            break;
         case AOWIN_Currenturl:
            if(win->urlgad)
            {
                  UBYTE *temp =NULL;
                  if (tag->ti_Data) temp = Dupstr((UBYTE *)tag->ti_Data,-1);
                  if (strlen(temp) > MAXSTRBUFCHARS) temp[MAXSTRBUFCHARS -1] = '\0';
                  Setgadgetattrs(win->urlgad,win->window,NULL,
                  STRINGA_TextVal,tag->ti_Data?temp:NULLSTRING,
                  STRINGA_DispPos,0,
                  TAG_END);
                  if (temp) FREE(temp);
            }
            break;
         case AOWIN_Noproxy:
            if(!win->window) SETFLAG(win->flags,WINF_NOPROXY,tag->ti_Data);
            break;
         case AOBJ_Winhis:
            win->whis=(void *)tag->ti_Data;
            win->hiswhis=(void *)tag->ti_Data;
            Sethisgadgets(win);
            Setwhiswindow(win->hiswhis);
            win->dragstartobject=NULL;
            win->dragstartobjpos=0;
            win->dragendobject=NULL;
            win->dragendobjpos=0;
            Setsecure(win);
            Setwincancel(win);
            break;
         case AOWIN_Activate:
            if(tag->ti_Data && win->window)
            {  ActivateWindow(win->window);
            }
            break;
         case AOWIN_Nofocus:
            if(win->focus==(void *)tag->ti_Data) win->focus=NULL;
            if(win->nextfocus==(void *)tag->ti_Data) win->nextfocus=NULL;
            break;
         case AOWIN_Focus:
            if(tag->ti_Data && win->focus!=(void *)tag->ti_Data)
            {  Asetattrs(win->focus,AOFRM_Focus,FALSE,TAG_END);
               win->focus=(void *)tag->ti_Data;
               Setsecure(win);
               Asetattrs(win->focus,AOFRM_Focus,TRUE,TAG_END);
            }
            break;
         case AOWIN_Keepselection:
            SETFLAG(win->flags,WINF_KEEPDRAG,tag->ti_Data);
            break;
         case AOWIN_Clearselection:
            if(tag->ti_Data)
            {  Dragclear(win);
            }
            break;
         case AOWIN_Refresh:
            if(tag->ti_Data)
            {  Refreshwindow(win);
            }
            break;
         case AOWIN_Rmbtrap:
            if(win->window)
            {  SETFLAG(win->window->Flags,WFLG_RMBTRAP,tag->ti_Data);
            }
            break;
         case AOWIN_Activeobject:
            Setactiveobject(win,(void *)tag->ti_Data);
            break;
         case AOWIN_Innerwidth:
            win->newwidth=tag->ti_Data;
            break;
         case AOWIN_Innerheight:
            win->newheight=tag->ti_Data;
            break;
         case AOWIN_Left:
            win->newleft=tag->ti_Data;
            win->leftset = TRUE;
            break;            
         case AOWIN_Top:
            win->newtop=tag->ti_Data;
            win->topset = TRUE;
            break;                        
         case AOBJ_Pointertype:
            Setawinpointer(win,tag->ti_Data);
            break;
         case AOWIN_Hiswinhis:
            if(tag->ti_Data)
            {  Followhis(win,(void *)tag->ti_Data);
            }
            break;
         case AOWIN_Appmessage:
            Processappmessage(win,(struct AppMessage *)tag->ti_Data);
            break;
         case AOWIN_Navigation:
            SETFLAG(win->flags,WINF_NAVS,tag->ti_Data);
            break;
         case AOWIN_Buttonbar:
            SETFLAG(win->flags,WINF_BUTTONS,tag->ti_Data);
            break;
         case AOWIN_Forcecharset:
            if(tag->ti_Data)
            {
                win->charset = Findcharset((UBYTE *)tag->ti_Data);
            }
            else
            {
                win->charset = NULL;
            }
            break;
         case AOWIN_PubScreenName:
            if(tag->ti_Data)
            {
               win->pubscreenname = Dupstr(tag->ti_Data,-1);
            }
            else
            {
               FREE(win->pubscreenname);
               win->pubscreenname = NULL;
            }
            break;
      }
   }
   Setslider(win,&win->vslider,vtotal,vvis,vtop);
   Setslider(win,&win->hslider,htotal,hvis,htop);
   if(status!=(UBYTE *)~0)
   {  if(win->statustext) FREE(win->statustext);
      win->statustext=Dupstr(status,-1);
      if(win->window && win->statusgad)
      {  Setgadgetattrs(win->statusgad,win->window,NULL,
            GA_Text,status,
            PGA_Total,total,
            PGA_Visible,read,
            TAG_END);
      }
   }
   if(hpstatus!=(UBYTE *)~0)
   {  if(win->statushptext) FREE(win->statushptext);
      win->statushptext=Dupstr(hpstatus,-1);
      if(win->window && win->statusgad)
      {  Setgadgetattrs(win->statusgad,win->window,NULL,
            STATGA_HPText,hpstatus,
            TAG_END);
      }
   }
   return 0;
}

static void Disposewindow(struct Awindow *win)
{  void *p;
   Closewindow(win);
   Closearexxport(win->key);
   if(win->statustext) FREE(win->statustext);
   if(win->statushptext) FREE(win->statushptext);
   if(win->activeurl) Aremchild(win->activeurl,(struct Aobject *)win,AOREL_URL_WINDOW);
   if(win->frame) Adisposeobject(win->frame);
   if(win->pubscreenname)
   {
       FREE(win->pubscreenname);
       win->pubscreenname = NULL;
   }
   while(p=RemHead(&win->urlpoplist)) FreeChooserNode(p);
   Aremchild(Aweb(),(struct Aobject *)win,AOREL_APP_USE_SCREEN);
   Aremchild(Aweb(),(struct Aobject *)win,AOREL_APP_USE_MENUS);
   Amethodas(AOTP_OBJECT,win,AOM_DISPOSE);

}

static struct Awindow *Newwindow(struct Amset *ams)
{  struct Awindow *win;
   UBYTE *portname;
   BOOL screenvalid;
   if(win=Allocobject(AOTP_WINDOW,sizeof(struct Awindow),ams))
   {  NewList(&win->urlpoplist);
      NewList(&win->userbutlist);
      ADDTAIL(&windows,win);
      win->key=++windowkey;
      win->capens.sp_DarkPen=-1;
      win->capens.sp_LightPen=-1;
      win->flags|=WINF_NAVS|WINF_BUTTONS;
      SETFLAG(win->flags,WINF_CLIPDRAG,prefs.program.clipdrag);
      if(portname=Openarexxport(win->key))
      {  win->portname=Dupstr(portname,-1);
      }
      if(!(win->windownr=Arexxportnr(win->key))) win->windownr=win->key;
      Setwindow(win,ams);
      Aaddchild(Aweb(),(struct Aobject *)win,AOREL_APP_USE_SCREEN);
      Aaddchild(Aweb(),(struct Aobject *)win,AOREL_APP_USE_MENUS);
      screenvalid=Agetattr(Aweb(),AOAPP_Screenvalid);
      win->frame=Anewobject(AOTP_FRAME,
         AOBJ_Window,screenvalid?(Tag)win:0,
         AOFRM_Topframe,TRUE,
         AOFRM_Name,GetTagData(AOWIN_Name,0,ams->tags),
         TAG_END);
      if(screenvalid)
      {  if(!Openwindow(win))
         {  REMOVE(win);
            Disposewindow(win);
            win=NULL;
         }
      }
   }
   return win;
}

static long Getwindow(struct Awindow *win,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOWIN_Window:
            PUTATTR(tag,win->window);
            break;
         case AOWIN_Rastport:
            PUTATTR(tag,win->window?win->window->RPort:NULL);
            break;
         case AOWIN_Innerleft:
            PUTATTR(tag,win->spacegad->LeftEdge);
            break;
         case AOWIN_Innertop:
            PUTATTR(tag,win->spacegad->TopEdge);
            break;
         case AOWIN_Innerwidth:
            PUTATTR(tag,win->spacegad->Width);
            break;
         case AOWIN_Innerheight:
            PUTATTR(tag,win->spacegad->Height);
            break;
         case AOWIN_Resized:
            PUTATTR(tag,BOOLVAL(win->flags&WINF_RESIZED));
            break;
         case AOWIN_Windownr:
            PUTATTR(tag,win->windownr);
            break;
         case AOWIN_Key:
            PUTATTR(tag,win->key);
            break;
         case AOBJ_Winhis:
            PUTATTR(tag,win->whis);
            break;
         case AOBJ_Frame:
            PUTATTR(tag,win->frame);
            break;
         case AOWIN_Noproxy:
            PUTATTR(tag,BOOLVAL(win->flags&WINF_NOPROXY));
            break;
         case AOWIN_Name:
            PUTATTR(tag,Agetattr(win->frame,AOFRM_Name));
            break;
         case AOWIN_Box:
            PUTATTR(tag,&win->box);
            break;
         case AOWIN_Zoombox:
            PUTATTR(tag,&win->zoombox);
            break;
         case AOWIN_Borderright:
            PUTATTR(tag,win->window?win->window->BorderRight:12);
            break;
         case AOWIN_Borderbottom:
            PUTATTR(tag,win->window?win->window->BorderBottom:12);
            break;
         case AOWIN_Commands:
            PUTATTR(tag,Agetattr(win->whis,AOWHS_Commands));
            break;
         case AOWIN_Specialpens:
            PUTATTR(tag,win->window?&win->capens:NULL);
            break;
         case AOWIN_Status:
            PUTATTR(tag,win->statustext?win->statustext:NULLSTRING);
            break;
         case AOWIN_Hpstatus:
            PUTATTR(tag,win->statushptext?win->statushptext:NULLSTRING);
            break;
         case AOWIN_Refreshing:
            PUTATTR(tag,BOOLVAL(win->flags&WINF_REFRESHING));
            break;
         case AOWIN_Animon:
            PUTATTR(tag,BOOLVAL(win->flags&WINF_ANIMON));
            break;
         case AOBJ_Jscancel:
            PUTATTR(tag,Cancelevent(win));
            break;
         case AOWIN_Jsdebug:
            {  struct MenuItem *mi;
               mi=ItemAddress(win->menu,Menunumfromcmd("@DEBUGJS"));
               PUTATTR(tag,mi?BOOLVAL(mi->Flags&CHECKED):FALSE);
            }
            break;
         case AOWIN_Hiswinhis:
            PUTATTR(tag,win->hiswhis);
            break;
         case AOWIN_Chartable:
            PUTATTR(tag,win->charset);
            break;
      }
   }
   return 0;
}

static long Addchildwindow(struct Awindow *win,struct Amadd *ama)
{  switch(ama->relation)
   {  case AOREL_WIN_POPUP:
         if(win->popup) Adisposeobject(win->popup);
         win->popup=ama->child;
         break;
   }
   return 0;
}

static long Remchildwindow(struct Awindow *win,struct Amadd *ama)
{  switch(ama->relation)
   {  case AOREL_WIN_POPUP:
         if(win->popup==ama->child) win->popup=NULL;
         break;
   }
   return 0;
}

static long Jsetupwindow(struct Awindow *win,struct Amjsetup *amj)
{  AmethodA(win->frame,(struct Amessage *)amj);
   return 0;
}

static void Deinstallwindow(void)
{  void *p;
   while(p=REMHEAD(&windows)) Adisposeobject(p);

#if !defined(__MORPHOS__) && !defined(__amigaos4__)
        if ( NULL != TitlebarImageBase )
        {
                CloseLibrary (TitlebarImageBase);
                TitlebarImageBase = NULL;
        }
#endif
}

USRFUNC_H2
(
static long  , Window_Dispatcher,
struct Awindow *,win,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newwindow((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setwindow(win,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getwindow(win,(struct Amset *)amsg);
         break;
      case AOM_UPDATE:
         result=Updatewindow(win,(struct Amset *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchildwindow(win,(struct Amadd *)amsg);
         break;
      case AOM_REMCHILD:
         result=Remchildwindow(win,(struct Amadd *)amsg);
         break;
      case AOM_JSETUP:
         result=Jsetupwindow(win,(struct Amjsetup *)amsg);
         break;
      case AOM_DISPOSE:
         Disposewindow(win);
         break;
      case AOM_DEINSTALL:
         Deinstallwindow();
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

/* Get the next or previous window */
struct Awindow *Nextwindow(struct Awindow *win,long d)
{  struct Awindow *nextw=win;
   if(d>0)
   {  for(;;)
      {  nextw=nextw->object.next;
         if(!nextw->object.next) nextw=windows.first;
         if(nextw->window) return nextw;
      }
   }
   else
   {  for(;;)
      {  nextw=nextw->object.prev;
         if(!nextw->object.prev) nextw=windows.last;
         if(nextw->window) return nextw;
      }
   }
   return win;
}

/* Set the status of the secure gadget */
void Setsecure(struct Awindow *win)
{
   BOOL issecure;
   issecure=BOOLVAL(Agetattr(win->focus?win->focus:win->frame,AOBJ_Secure));
   if(win->securegad)
   {  Setgadgetattrs(win->securegad,win->window,NULL,
      GA_Selected,issecure?TRUE:FALSE,
         TAG_END);
   }
}

/*------------------------------------------------------------------------*/

BOOL Installwindow(void)
{
#if !defined(__MORPHOS__) && !defined(__amigaos4__)
        if ( (FALSE == noiconify) && (prefs.gui.windowborder == 0)  && (NULL == TitlebarImageBase) )
        {
                TitlebarImageBase = OpenLibrary("titlebar.image",40L);
                if ( NULL == TitlebarImageBase)
                        TitlebarImageBase = OpenLibrary("Images/titlebar.image",40L);
                if ( NULL == TitlebarImageBase)
                        TitlebarImageBase = OpenLibrary("Classes/Images/titlebar.image",40L);
                /* don't bother if the titlebar class can't be opened */
        }
#endif

        NEWLIST(&windows);
   if(!Amethod(NULL,AOM_INSTALL,AOTP_WINDOW,(Tag)Window_Dispatcher)) return FALSE;
   return TRUE;
}

void *Firstwindow(void)
{  if(windows.first && windows.first->object.next)
   {  return windows.first;
   }
   else return NULL;
}

void *Findwindow(ULONG key)
{  struct Awindow *win;
   for(win=windows.first;win->object.next;win=win->object.next)
   {  if(win->key==key) return win;
   }
   return NULL;
}

void *Findwindownr(long nr)
{  struct Awindow *win;
   for(win=windows.first;win->object.next;win=win->object.next)
   {  if(win->windownr==nr) return win;
   }
   return NULL;
}

void Busypointer(BOOL busy)
{  struct Awindow *win;
   BOOL set=FALSE;
   static short busynest=0;
   if(busy && ++busynest==1) set=TRUE;
   if(!busy && --busynest==0) set=TRUE;
   if(set)
   {  for(win=windows.first;win->object.next;win=win->object.next)
      {  if(win->window)
         {  if(busy)
            {  SetWindowPointer(win->window,
                  WA_BusyPointer,TRUE,
                  WA_PointerDelay,TRUE,
                  TAG_END);
            }
            else
            {  
#if defined(__amigaos4__)
	switch(win->ptrtype)
	{
		case APTR_DEFAULT:
		{
		    SetWindowPointer(win->window,WA_PointerType,POINTERTYPE_NORMAL,WA_PointerDelay,TRUE,TAG_END);
		    break;
		}
		case APTR_RESIZEHOR:
		{
		    SetWindowPointer(win->window,WA_PointerType,POINTERTYPE_EASTWESTRESIZE,WA_PointerDelay,TRUE,TAG_END);
		    break;
		}
		case APTR_RESIZEVERT:
		{
		    SetWindowPointer(win->window,WA_PointerType,POINTERTYPE_NORTHSOUTHRESIZE,WA_PointerDelay,TRUE,TAG_END);
		    break;
		}
		case APTR_HAND:
		{
		    SetWindowPointer(win->window,WA_PointerType,POINTERTYPE_LINK,WA_PointerDelay,TRUE,TAG_END);
		    break;
		}
	}
#else
            	SetWindowPointer(win->window,
                  WA_Pointer,Apppointer((struct Application *)Aweb(),win->ptrtype),
                  TAG_END);
#endif
            }
         }
      }
   }
}

void Setanimgad(BOOL onoff)
{  struct Awindow *win;
   for(win=windows.first;win->object.next;win=win->object.next)
   {  if(win->ledgad && win->window)
      {  SETFLAG(win->flags,WINF_ANIMON,onoff);
         Setgadgetattrs(win->ledgad,win->window,NULL,
            LEDGGA_Active,onoff,TAG_END);
      }
   }
}

UBYTE *Windowtitle(struct Awindow *win,UBYTE *name,UBYTE *title)
{  UBYTE *newtitle,*p;
   long len=strlen(title)+1;
   if(name) len+=strlen(name)+4;
   if(win->portname) len+=strlen(win->portname)+3;
   else len+=20;
   newtitle=ALLOCTYPE(UBYTE,len,0);
   if(newtitle)
   {  p=newtitle;
      if(win->portname) p+=sprintf(newtitle,"%s",win->portname);
      else p+=sprintf(newtitle,"(AWEB #%ld)",win->key);
      if(name)
      {  p+=sprintf(p," [%s]",name);
      }
      sprintf(p," - %s",title);
   }
   return newtitle;
}

void *Activewindow(void)
{  if(activewindow) return activewindow;
   else return Firstwindow();
}

void Rebuildallbuttons(void)
{  struct Awindow *win;
   struct DrawInfo *dri=(struct DrawInfo *)Agetattr(Aweb(),AOAPP_Drawinfo);
   for(win=windows.first;win->object.next;win=win->object.next)
   {  Rebuildbuttonrow(win,dri);
   }
}

void Reopenallwindows(void)
{  struct Awindow *win;
   for(win=windows.first;win->object.next;win=win->object.next)
   {  Closewindow(win);
      if(!Openwindow(win))
      {
          Closewindow(win);  // Close window free any partly allocated resources associated with window open failure  
      }
   }
}

void Jsetupallwindows(struct Jcontext *jc)
{  struct Awindow *win;
   for(win=windows.first;win->object.next;win=win->object.next)
   {  Ajsetup((struct Aobject *)win,jc,NULL,NULL);
   }
}

void Inputallwindows(BOOL input)
{  struct Awindow *win;
   for(win=windows.first;win->object.next;win=win->object.next)
   {  if(input!=BOOLVAL(win->flags&WINF_INPUT))
      {  Setwincancel(win);
      }
   }
}

static BOOL Parsefeature(UBYTE *p)
{  if(STRNIEQUAL(p,"=0",2)) return FALSE;
   if(STRNIEQUAL(p,"=no",3)) return FALSE;
   if(STRNIEQUAL(p,"=false",6)) return FALSE;
   return TRUE;
}

void *Jopenwindow(struct Jcontext *jc,struct Jobject *opener,
   UBYTE *urlname,UBYTE *name,UBYTE *spec,struct Awindow *oldwin)
{  struct Awindow *win=NULL;
   struct Jobject *jo=NULL;
   struct Jvar *jv;
   void *url;
   UBYTE *frag,*basename;
   UBYTE *p;
   short width=0,height=0;
   short left, top;
   BOOL leftset=FALSE, topset=FALSE;
   
   BOOL navs=TRUE,buttons=TRUE;
   if(spec && *spec)
   {  navs=FALSE;
      buttons=FALSE;
      for(p=spec;*p;p++)
      {  
         if(STRNIEQUAL(p,"width=",6))
         {  width=atoi(p+6);
         }
         if(STRNIEQUAL(p,"left=",5))
         {  
             left=atoi(p+5);
             leftset = TRUE;
         }         
         if(STRNIEQUAL(p,"top=",4))
         {  
             top=atoi(p+4);
             topset = TRUE;
         }                  
         if(STRNIEQUAL(p,"height=",7))
         {  height=atoi(p+7);
         }
         if(STRNIEQUAL(p,"toolbar",7))
         {  navs|=Parsefeature(p+7);
         }
         if(STRNIEQUAL(p,"location",8))
         {  navs|=Parsefeature(p+8);
         }
         if(STRNIEQUAL(p,"status",6))
         {  navs|=Parsefeature(p+6);
         }
         if(STRNIEQUAL(p,"directories",11))
         {  buttons|=Parsefeature(p+11);
         }
      }
   }
   {  win=Anewobject(AOTP_WINDOW,
         AOWIN_Name,(Tag)name,
         AOWIN_Innerwidth,width,
         AOWIN_Innerheight,height,
         leftset?AOWIN_Left:TAG_IGNORE,left,
         topset?AOWIN_Top:TAG_IGNORE,top,         
         AOWIN_Noproxy,Agetattr((struct Aobject *)oldwin,AOWIN_Noproxy),
         AOWIN_Navigation,navs,
         AOWIN_Buttonbar,buttons,
         TAG_END);
      if(win)
      {  win->flags|=WINF_JSCLOSEABLE;
         Ajsetup((struct Aobject *)win,jc,NULL,NULL);
         jo=(struct Jobject *)Agetattr(win->frame,AOBJ_Jobject);
         if(jv=Jproperty(jc,jo,"opener"))
         {  Jasgobject(jc,jv,opener);
         }
         if(*urlname) basename=Getjscurrenturlname(jc);
         else basename="";
         url=Findurl(basename,urlname,0);
         frag=Fragmentpart(urlname);
         Inputwindoc(win,url,frag,NULL);
         Ajsetup((struct Aobject *)win,jc,NULL,NULL);
      }
   }
   return jo;
}

void Jclosewindow(struct Awindow *win)
{  if(win && (win->flags&WINF_JSCLOSEABLE))
   {  /*
      if(STREQUAL(activeport,win->portname)) *activeport='\0';
      REMOVE(win);
      Adisposeobject(win);
      */
      win->cmd|=CMD_CLOSE;
   }
}
