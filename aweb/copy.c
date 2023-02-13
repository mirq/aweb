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

/* copy.c - AWeb copy element object */

#include "aweb.h"
#include "copydriver.h"
#include "source.h"
#include "url.h"
#include "map.h"
#include "link.h"
#include "frame.h"
#include "application.h"
#include "popup.h"
#include "window.h"
#include "form.h"
#include "info.h"
#include "area.h"
#include "copyprivate.h"
#include "jslib.h"

#include <reaction/reaction.h>

#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>


/*------------------------------------------------------------------------*/

#define CQID_ONLOAD        1  /* Queueid: run onLoad JavaScript */
#define CQID_INFOCHANGED   2  /* Queueid: info window contents changed */
#define CQID_DEFERDISPOSE  3  /* Queueid: dispose ourselves */

/*------------------------------------------------------------------------*/

/* Get the appropriate icon */
static void *Copyicon(struct Copy *cop)
{  long tag=AOAPP_Deficon;
   if(cop->flags&CPYF_ERROR) tag=AOAPP_Deferricon;
   else if(cop->usemap || (cop->flags&CPYF_ISMAP) || cop->fld.form) tag=AOAPP_Defmapicon;
   return (void *)Agetattr(Aweb(),tag);
}

/* Extra horizontal space (border and gutter) */
static long Extraw(struct Copy *cop)
{  long w=cop->hspace;
   /* if(cop->fld.elt.link) */ w+=cop->border;
   return w;
}

/* Extra vertical space (border and gutter) */
static long Extrah(struct Copy *cop)
{  long h=cop->vspace;
   /* if(cop->fld.elt.link) */ h+=cop->border;
   return h;
}

static void Drawframe(struct Copy *cop,struct Coords *coo,struct Amrender *amr,BOOL unloaded)
{  short pen,i,border=0;
   long x,y,w,h,d;
   struct RastPort *rp=coo->rp;
   x=cop->fld.elt.aox+coo->dx+Extraw(cop);
   y=cop->fld.elt.aoy+coo->dy+Extrah(cop);
   w=cop->fld.elt.aow-2*Extraw(cop);
   h=cop->fld.elt.aoh-2*Extrah(cop);
   if(amr->flags&AMRF_UPDATESELECTED)
   {  pen=coo->alinkcolor;
   }
   else
   {  if(cop->fld.elt.link)
      {  if(Agetattr(cop->fld.elt.link,AOLNK_Visited)) pen=coo->vlinkcolor;
         else pen=coo->linkcolor;
      }
      else if(unloaded)
      {  pen=coo->linkcolor;
      }
      else
      {  pen=coo->textcolor;
      }
   }
   SetAPen(rp,pen);
   if(unloaded && !border)
   {  border=1;
      d=0;
   }
   else
   {  border=cop->border;
      d=1;
   }
   for(i=0;i<border;i++)
   {  Move(rp,x-d-i,y-d-i);
      Draw(rp,x+w+d-1+i,y-d-i);
      Draw(rp,x+w+d-1+i,y+h+d-1+i);
      Draw(rp,x-d-i,y+h+d-1+i);
      Draw(rp,x-d-i,y-d-i);
   }
   if(unloaded && (cop->fld.elt.link || cop->fld.form))
   {  Move(rp,x,y+h-1);
      Draw(rp,x+w-1,y);
   }
}

/* Get the map coordinates */
static BOOL Getmapcoords(struct Copy *cop,struct Coords *coo,long xco,long yco,
   long *mapx,long *mapy)
{  struct Coords coords={0};
   long x,y,w,h,dvw,dvh;
   BOOL result=FALSE;
   if(!coo)
   {  Framecoords(cop->fld.elt.cframe,&coords);
      coo=&coords;
   }
   x=cop->fld.elt.aox+Extraw(cop);
   y=cop->fld.elt.aoy+Extrah(cop);
   w=cop->fld.elt.aow-2*Extraw(cop);
   h=cop->fld.elt.aoh-2*Extrah(cop);
   dvw=cop->driver->aow;
   dvh=cop->driver->aoh;
   xco=xco-coo->dx-x;
   yco=yco-coo->dy-y;
   if(xco>=0 && xco<w && yco>=0 && yco<h)
   {  /* Scale rendered coordinates to scaled map coordinates */
      if(dvw>0 && cop->width && cop->width!=dvw)
      {  if(cop->width>dvw)
         {  xco-=(cop->width-dvw)/2;
            if(xco<0) xco=0;
            if(xco>=dvw) xco=dvw-1;
         }
         xco=xco*cop->width/dvw;
      }
      if(dvh>0 && cop->height && cop->height!=dvh)
      {  if(cop->height>dvh)
         {  yco-=(cop->height-dvh)/2;
            if(yco<0) yco=0;
            if(yco>=dvh) yco=dvh-1;
         }
         yco=yco*cop->height/dvh;
      }
      result=TRUE;
      if(mapx) *mapx=xco;
      if(mapy) *mapy=yco;
   }
   return result;
}

/* Get the URL and the target name pointed to, or NULL (dynamic string).
 * Also remembers AREA object (to forward AOM_JONMOUSE messages to). */
static void Getmapurl(struct Copy *cop,struct Coords *coo,long xco,long yco,
   UBYTE **urlp,UBYTE **targetp)
{  long x,y,len;
   UBYTE *mapurl=NULL,*fragment=NULL,*msg,*urlname,*targetname=NULL;
   void *url=NULL;
   cop->maparea=NULL;
   if(Getmapcoords(cop,coo,xco,yco,&x,&y))
   {  if(cop->fld.form)
      {  msg=AWEBSTR(MSG_AWEB_FORMLOCATION);
         len=strlen(msg)+16;
         if(mapurl=ALLOCTYPE(UBYTE,len,0))
         {  Lprintf(mapurl,msg,x,y);
         }
      }
      else if(cop->usemap)
      {  Agetattrs(cop->usemap,
            AOMAP_Xco,x,
            AOMAP_Yco,y,
            AOMAP_Area,(Tag)&cop->maparea,
            TAG_END);
         if(cop->maparea)
         {  Agetattrs(cop->maparea,
               AOLNK_Url,(Tag)&url,
               AOLNK_Fragment,(Tag)&fragment,
               AOLNK_Target,(Tag)&targetname,
               TAG_END);
         }
         if(url && (urlname=(UBYTE *)Agetattr(url,AOURL_Url)))
         {  len=strlen(urlname)+16;
            if(fragment) len+=strlen(fragment)+1;
            if(mapurl=ALLOCTYPE(UBYTE,len,0))
            {  strcpy(mapurl,urlname);
               if(fragment)
               {  strcat(mapurl,"#");
                  strcat(mapurl,fragment);
               }
            }
         }
         if(cop->maparea && !url)
         {  mapurl=Dupstr("",-1);
         }
      }
      else if(cop->flags&CPYF_ISMAP)
      {  if(cop->fld.elt.link)
         {  Agetattrs(cop->fld.elt.link,
               AOLNK_Url,(Tag)&url,
               AOLNK_Fragment,(Tag)&fragment,
               TAG_END);
            if(url && (urlname=(UBYTE *)Agetattr(url,AOURL_Url)))
            {  len=strlen(urlname)+16;
               if(fragment) len+=strlen(fragment)+1;
               if(mapurl=ALLOCTYPE(UBYTE,len,0))
               {  sprintf(mapurl,"%s?%ld,%ld",urlname,x,y);
                  if(fragment)
                  {  strcat(mapurl,"#");
                     strcat(mapurl,fragment);
                  }
               }
            }
         }
      }
   }
   if(urlp) *urlp=mapurl;
   else FREE(mapurl);
   if(targetp) *targetp=targetname;
}

/* Build a popup menu */
static void Popupinquire(struct Copy *cop,void *pup)
{  struct Popupitem *pi;
   void *url=(void *)Agetattr(cop->source,AOSRC_Url);
   BOOL inmem=Agetattr(url,AOURL_Isinmem);
   for(pi=prefs.gui.popupmenu[PUPT_IMAGE].first;pi->next;pi=pi->next)
   {  if((inmem && (pi->flags&PUPF_INMEM))
      || (!inmem && (pi->flags&PUPF_NOTINMEM)))
      {  Asetattrs(pup,
            AOPUP_Title,(Tag)pi->title,
            AOPUP_Command,(Tag)pi->cmd,
            TAG_END);
      }
   }
}

/* User has selected this popup menu choice */
static void Popupselectcopy(struct Copy *cop,UBYTE *cmd)
{  void *url;
   if(cmd)
   {  url=(void *)Agetattr(cop->source,AOSRC_Url);
      Execarexxcmd(Agetattr(cop->fld.win,AOWIN_Key),cmd,"u",Agetattr(url,AOURL_Url));
   }
   cop->popup=NULL;
}

/* Dispose this parameter */
static void Disposeparam(struct Objectparam *op)
{  if(op)
   {  if(op->name) FREE(op->name);
      if(op->value) FREE(op->value);
      if(op->valuetype) FREE(op->valuetype);
      if(op->type) FREE(op->type);
      FREE(op);
   }
}

/* Add a parameter */
static struct Objectparam *Addparam(struct Copy *cop,
   UBYTE *name,UBYTE *value,UBYTE *valuetype,UBYTE *type)
{  struct Objectparam *op;
   BOOL ok=TRUE;
   if(op=ALLOCSTRUCT(Objectparam,1,MEMF_CLEAR))
   {  if(name && !(op->name=Dupstr(name,-1))) ok=FALSE;
      if(value && !(op->value=Dupstr(value,-1))) ok=FALSE;
      if(valuetype && !(op->valuetype=Dupstr(valuetype,-1))) ok=FALSE;
      if(type && !(op->type=Dupstr(type,-1))) ok=FALSE;
      if(ok)
      {  ADDTAIL(&cop->params,op);
      }
      else
      {  Disposeparam(op);
         op=NULL;
      }
   }
   return op;
}

/* Set or unset the DISPLAYED flag and let driver know the changes */
static void Setdisplayed(struct Copy *cop,BOOL displayed)
{  if(cop->driver && BOOLVAL(cop->flags&CPYF_DISPLAYED)!=BOOLVAL(displayed))
   {  Asetattrs((struct Aobject *)cop->driver,AOCDV_Displayed,displayed,TAG_END);
   }
   SETFLAG(cop->flags,CPYF_DISPLAYED,displayed);
}

/* Our image has changed. If it is embedded and still fits in place, render it now
 * if we are displayed.
 * If it is background, only let parent know when driver is ready.
 * Else let our parent know we changed. */
static void Changedcopy(struct Copy *cop)
{  long oldw=cop->fld.elt.aow,oldh=cop->fld.elt.aoh;
   if(cop->flags&CPYF_EMBEDDED)
   {  Ameasure((struct Aobject *)cop,1,1,0,0,cop->text,NULL);
      if(cop->fld.elt.aow==oldw && cop->fld.elt.aoh==oldh)
      {  if(cop->driver)
         {  cop->driver->aox=cop->fld.elt.aox+Extraw(cop);
            cop->driver->aoy=cop->fld.elt.aoy+Extrah(cop);
            /* If driver smaller then suggested, center it */
            if(cop->driver->aow<cop->width)
            {  cop->driver->aox+=(cop->width-cop->driver->aow)/2;
            }
            if(cop->driver->aoh<cop->height)
            {  cop->driver->aoy+=(cop->height-cop->driver->aoh)/2;
            }
         }
         if(cop->flags&CPYF_DISPLAYED)
         {  Arender((struct Aobject *)cop,NULL,cop->fld.elt.aox,cop->fld.elt.aoy,
               cop->fld.elt.aox+cop->fld.elt.aow-1,cop->fld.elt.aoy+cop->fld.elt.aoh-1,AMRF_CLEAR,cop->text);
         }
      }
      else
      {  Setdisplayed(cop,FALSE);
         Asetattrs(cop->parent,AOBJ_Changedchild,(Tag)cop,TAG_END);
      }
   }
   else if(cop->flags&CPYF_BACKGROUND)
   {  /* Only notify parent if bg bitmap is available */

      if(Agetattr((struct Aobject *)cop->driver,AOCDV_Imagebitmap))
      {
         Asetattrs(cop->parent,AOBJ_Changedbgimage,(Tag)cop,TAG_END);
      }

   }
   else
   {  Asetattrs(cop->parent,AOBJ_Changedchild,(Tag)cop,TAG_END);
   }


}

/* If the CHANGEDCOPY flag wasn't set yet, set it now and return TRUE.
 * This is to avoid spurious renderings (as a result of calls to Changedcopy())
 * during a change of drivers. */
static BOOL Setchangedcopy(struct Copy *cop)
{  if(cop->flags&CPYF_CHANGEDCOPY) return FALSE;
   cop->flags|=CPYF_CHANGEDCOPY;
   return TRUE;
}

/* ACM_LOAD message received. Process only if nondocument. */
static void Loadcopy(struct Copy *cop,struct Acmload *acml)
{  void *url=(void *)Agetattr(cop->source,AOSRC_Url);
   if(!cop->driver)
   {  if(acml->flags&ACMLF_RELOAD)
      {  Auload(url,AUMLF_IMAGE|AUMLF_RELOAD|PROXY(cop),cop->referer,NULL,cop->frame);
      }
      else if(acml->flags&ACMLF_BACKGROUND)
      {  if(cop->flags&CPYF_BACKGROUND)
         {  Auload(url,AUMLF_IMAGE|PROXY(cop),cop->referer,NULL,cop->frame);
         }
      }
      else if(acml->flags&ACMLF_BGSOUND)
      {  if(cop->flags&CPYF_BGSOUND)
         {  Auload(url,AUMLF_IMAGE|PROXY(cop),cop->referer,NULL,cop->frame);
         }
      }
      else /* no background or bgsound */
      {  if(!(cop->flags&(CPYF_BACKGROUND|CPYF_BGSOUND)))
         {  if(acml->flags&ACMLF_MAPSONLY)
            {  if(cop->usemap || (cop->flags&CPYF_ISMAP))
               {  if(!(acml->flags&ACMLF_RESTRICT) || Issamehost(url,cop->referer))
                  {  Auload(url,AUMLF_IMAGE|PROXY(cop),cop->referer,NULL,cop->frame);
                  }
               }
            }
            else if(!(acml->flags&ACMLF_RESTRICT) || Issamehost(url,cop->referer))
            {  Auload(url,AUMLF_IMAGE|PROXY(cop),cop->referer,NULL,cop->frame);
            }
         }
      }
   }
   else if(cop->driver && cop->driver->object.objecttype!=AOTP_DOCUMENT)
   {  if(acml->flags&ACMLF_RELOAD)
      {  Auload(url,AUMLF_IMAGE|AUMLF_RELOAD|PROXY(cop),cop->referer,NULL,cop->frame);
      }
   }
}

/* Do initial load of object */
static void Initialloadcopy(struct Copy *cop)
{  void *url=(void *)Agetattr(cop->source,AOSRC_Url);
   ULONG flag=PROXY(cop)|((cop->flags&CPYF_RELOADVERIFY)?AUMLF_VERIFY:0);
   if(cop->flags&CPYF_EMBEDDED)
   {  if(!prefs.network.restrictimages || Issamehost(url,cop->referer))
      {  if(prefs.network.loadimg==LOADIMG_ALL)
         {  Auload(url,AUMLF_IMAGE|flag,cop->referer,NULL,cop->frame);
         }
         else if(prefs.network.loadimg==LOADIMG_MAPS)
         {  if(cop->usemap || (cop->flags&CPYF_ISMAP))
            {  Auload(url,AUMLF_IMAGE|flag,cop->referer,NULL,cop->frame);
            }
            else
            {  Auload(url,AUMLF_IFINMEM|PROXY(cop),cop->referer,NULL,cop->frame);
            }
         }
         else
         {  Auload(url,AUMLF_IFINMEM|PROXY(cop),cop->referer,NULL,cop->frame);
         }
      }
   }
   else if(cop->flags&CPYF_BACKGROUND)
   {  if(!prefs.network.restrictimages || Issamehost(url,cop->referer))
      {  if(prefs.browser.docolors)
         {  Auload(url,AUMLF_IMAGE|flag,cop->referer,NULL,cop->frame);
         }
         else
         {  Auload(url,AUMLF_IFINMEM|PROXY(cop),cop->referer,NULL,cop->frame);
         }
      }
   }
   else if(cop->flags&CPYF_BGSOUND)
   {  if(prefs.browser.dobgsound)
      {  Auload(url,AUMLF_IMAGE|flag,cop->referer,NULL,cop->frame);
      }
   }
   else if(cop->flags&CPYF_MAPDOCUMENT)
   {  Auload(url,flag,cop->referer,NULL,cop->frame);
   }
   cop->flags&=~CPYF_INITIAL;
}

/*------------------------------------------------------------------------*/

static long Measurecopy(struct Copy *cop,struct Ammeasure *amm)
{  BOOL ready=Agetattr((struct Aobject *)cop->driver,AOCDV_Ready);
   void *icon;
   long dw,dh,realw;
   dw=2*Extraw(cop);
   dh=2*Extrah(cop);
   if(ready && !(cop->flags&CPYF_ERROR))
   {  /* Use the driver's dimensions, add border and gutter */
      AmethodA((struct Aobject *)cop->driver,(struct Amessage *)amm);
      cop->fld.elt.aow=MAX(cop->width,cop->driver->aow)+dw;
      cop->fld.elt.aoh=MAX(cop->height,cop->driver->aoh)+dh;
      if(amm->ammr)
      {  amm->ammr->width+=cop->fld.elt.aow-cop->driver->aow;
         amm->ammr->minwidth+=cop->fld.elt.aow-cop->driver->aow;
         if(cop->fld.elt.eltflags&ELTF_NOBR)
         {  amm->ammr->minwidth+=amm->addwidth;
            amm->ammr->addwidth=amm->ammr->minwidth;
         }
      }
   }
   else
   {  /* If width and height given, use it */
      if(cop->width && cop->height)
      {  cop->fld.elt.aow=cop->width;
         cop->fld.elt.aoh=cop->height;
      }
      /* If alternate text given, use its width */
      else if(cop->fld.elt.textpos && amm->text)
      {  SetFont(mrp,cop->fld.elt.font);
         SetSoftStyle(mrp,cop->fld.elt.style,0x0f);
         Textlengthext(mrp,amm->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length,&realw);
         cop->fld.elt.aow=realw+4;
         cop->fld.elt.aoh=cop->fld.elt.font->tf_YSize+4;
      }
      /* If there is an icon, use its size */
      else if(icon=Copyicon(cop))
      {  cop->fld.elt.aow=Getvalue(icon,BITMAP_Width);
         cop->fld.elt.aoh=Getvalue(icon,BITMAP_Height);
      }
      else
      {  cop->fld.elt.aow=32;
         cop->fld.elt.aoh=32;
      }
      cop->fld.elt.aow+=dw;
      cop->fld.elt.aoh+=dh;
      AmethodasA(AOTP_FIELD,(struct Aobject *)cop,(struct Amessage *)amm);
   }
   return 0;
}

static long Layoutcopy(struct Copy *cop,struct Amlayout *aml)
{  long result=0;
   Setdisplayed(cop,FALSE);
   if(cop->driver)
   {  /* First calculate pixel dimensions from percentage */
      if(cop->pwidth>=0)
      {  cop->width=(aml->width-aml->startx)*cop->pwidth/100;
      }
      if(cop->pheight>=0 && !(aml->flags&AMLF_INTABLE))
      {  cop->height=aml->height*cop->pheight/100;
      }
      /* Pass the resulting (or unchanged) pixel dimensions to driver */
      Asetattrs((struct Aobject *)cop->driver,
         AOCDV_Width,cop->width,
         AOCDV_Height,cop->height,
         TAG_END);
      if(Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR))
      {  /* Use driver dimensions, add border and gutter */
         result=AmethodA((struct Aobject *)cop->driver,(struct Amessage *)aml);
         cop->fld.elt.aow=MAX(cop->width,cop->driver->aow)+2*Extraw(cop);
         cop->fld.elt.aoh=MAX(cop->height,cop->driver->aoh)+2*Extrah(cop);
         /* Return value for plugins is undefined, really only keep AMLR_FHAGAIN. */
         if(cop->driver->object.objecttype!=AOTP_DOCUMENT) result=0;
      }
   }
   result|=AmethodasA(AOTP_FIELD,(struct Aobject *)cop,(struct Amessage *)aml);
   if(cop->driver)
   {  cop->driver->aox=cop->fld.elt.aox;
   }
   return result;
}

static long Aligncopy(struct Copy *cop,struct Amalign *ama)
{  long result=AmethodasA(AOTP_FIELD,(struct Aobject *)cop,(struct Amessage *)ama);
   if(cop->driver && !(cop->flags&CPYF_ERROR))
   {  cop->driver->aox=cop->fld.elt.aox;
      cop->driver->aoy=cop->fld.elt.aoy;
      /* If driver smaller then suggested, center it */
      if(cop->driver->aow<cop->width)
      {  cop->driver->aox+=(cop->width-cop->driver->aow)/2;
      }
      if(cop->driver->aoh<cop->height)
      {  cop->driver->aoy+=(cop->height-cop->driver->aoh)/2;
      }
      /* Add border and gutter to driver's position */
      cop->driver->aox+=Extraw(cop);
      cop->driver->aoy+=Extrah(cop);
   }
   return result;
}

static long Movecopy(struct Copy *cop,struct Ammove *amm)
{  long result;
   result=AmethodasA(AOTP_FIELD,(struct Aobject *)cop,(struct Amessage *)amm);
   if(cop->driver && !(cop->flags&CPYF_ERROR)) AmethodA((struct Aobject *)cop->driver,(struct Amessage *)amm);
   return result;
}

/* Only complement the highlight section. Pass a valid Coords. */
static void Highlightcopy(struct Copy *cop,struct Coords *coo)
{  long x,y,w,h,len,histart=0,hiend=0;
   BOOL highlight;
   struct TextExtent te={0};
   x=cop->fld.elt.aox+Extraw(cop)+coo->dx;
   y=cop->fld.elt.aoy+Extrah(cop)+coo->dy;
   w=cop->fld.elt.aow-2*Extraw(cop);
   h=cop->fld.elt.aoh-2*Extrah(cop);
   SetFont(coo->rp,cop->fld.elt.font);
   SetSoftStyle(coo->rp,cop->fld.elt.style,0x0f);
   len=TextFit(coo->rp,cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length,&te,NULL,1,w,h);
   x+=(w-te.te_Width)/2;
   y+=(h-te.te_Height)/2;
   if(len>0)
   {  if(cop->hilength)
      {  highlight=TRUE;
         if(cop->histart<=cop->fld.elt.textpos)
         {  histart=0;
         }
         else if(cop->histart<cop->fld.elt.textpos+len)
         {  histart=Textlength(coo->rp,cop->text->buffer+cop->fld.elt.textpos,
               cop->histart-cop->fld.elt.textpos);
         }
         else highlight=FALSE;
         hiend=Textlength(coo->rp,cop->text->buffer+cop->fld.elt.textpos,
            cop->histart+MIN(len,cop->hilength)-cop->fld.elt.textpos);
      }
      else
      {  highlight=FALSE;
      }
      if(highlight)
      {  SetDrMd(coo->rp,COMPLEMENT);
         RectFill(coo->rp,x+histart,y,x+hiend-1,y+coo->rp->TxHeight-1);
         SetDrMd(coo->rp,JAM1);
      }
   }
}

static long Rendercopy(struct Copy *cop,struct Amrender *amr)
{  struct Coords *coo;
   void *icon;
   struct BitMap *bitmap;
   UBYTE *mask;
   long x,y,w,h,bw,bh,len,histart=0,hiend=0;
   short pen;
   BOOL highlight;
   struct TextExtent te={0};
   Setdisplayed(cop,TRUE);
   if(!Agetattr((struct Aobject *)cop,AOELT_Visible)) return 0;
   if(cop->driver && Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR))
   {  AmethodA((struct Aobject *)cop->driver,(struct Amessage *)amr);
      if(cop->border)
      {  coo=Clipcoords(cop->fld.elt.cframe,amr->coords);
         if(coo && coo->rp)
         {  Drawframe(cop,coo,amr,FALSE);
         }
         Unclipcoords(coo);
      }
   }
   else
   {  coo=Clipcoords(cop->fld.elt.cframe,amr->coords);
      if(coo && coo->rp)
      {  if(!(amr->flags&(AMRF_UPDATESELECTED|AMRF_UPDATENORMAL)))
         {  if(amr->flags&AMRF_CLEAR)
            {  /* If embedded, clear only our area. Else clear everything. */
               if(cop->flags&CPYF_EMBEDDED)
               {  Erasebg(cop->fld.elt.cframe,coo,
                     MAX(amr->rect.minx,cop->fld.elt.aox),
                     MAX(amr->rect.miny,cop->fld.elt.aoy),
                     MIN(amr->rect.maxx,cop->fld.elt.aox+cop->fld.elt.aow-1),
                     MIN(amr->rect.maxy,cop->fld.elt.aoy+cop->fld.elt.aoh-1));
               }
               else
               {
                  Erasebg
                  (
                      cop->fld.elt.cframe,
                      coo,
                      amr->rect.minx,
                      amr->rect.miny,
                      amr->rect.maxx,
                      amr->rect.maxy
                  );
               }
            }
            x=cop->fld.elt.aox+Extraw(cop)+coo->dx;
            y=cop->fld.elt.aoy+Extrah(cop)+coo->dy;
            w=cop->fld.elt.aow-2*Extraw(cop);
            h=cop->fld.elt.aoh-2*Extrah(cop);
            if(cop->fld.elt.textpos && cop->text)
            {  SetFont(coo->rp,cop->fld.elt.font);
               SetSoftStyle(coo->rp,cop->fld.elt.style,0x0f);
               len=TextFit(coo->rp,cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length,&te,NULL,1,w,h);
               if(prefs.browser.docolors && cop->color && cop->color->pen>=0) pen=cop->color->pen;
               else pen=coo->textcolor;
               SetAPen(coo->rp,pen);
               if(len>0)
               {  if(cop->hilength)
                  {  highlight=TRUE;
                     if(cop->histart<=cop->fld.elt.textpos)
                     {  histart=0;
                     }
                     else if(cop->histart<cop->fld.elt.textpos+len)
                     {  histart=Textlength(coo->rp,cop->text->buffer+cop->fld.elt.textpos,
                           cop->histart-cop->fld.elt.textpos);
                     }
                     else highlight=FALSE;
                     hiend=Textlength(coo->rp,cop->text->buffer+cop->fld.elt.textpos,
                        cop->histart+MIN(len,cop->hilength)-cop->fld.elt.textpos);
                  }
                  else
                  {  highlight=FALSE;
                  }
                  if(highlight && !(amr->flags&AMRF_CLEAR))
                  {  Erasebg(cop->fld.elt.cframe,coo,
                        MAX(amr->rect.minx,cop->fld.elt.aox),
                        MAX(amr->rect.miny,cop->fld.elt.aoy),
                        MIN(amr->rect.maxx,cop->fld.elt.aox+cop->fld.elt.aow-1),
                        MIN(amr->rect.maxy,cop->fld.elt.aoy+cop->fld.elt.aoh-1));
                  }
                  x+=(w-te.te_Width)/2;
                  y+=(h-te.te_Height)/2;
                  Move(coo->rp,x,y+coo->rp->TxBaseline);
                  Text(coo->rp,cop->text->buffer+cop->fld.elt.textpos,len);
                  if(cop->fld.elt.style&FSF_STRIKE)
                  {  Move(coo->rp,x,y+coo->rp->TxHeight/2);
                     Draw(coo->rp,x+te.te_Width,y+coo->rp->TxHeight/2);
                  }
                  if(highlight && !(amr->flags&AMRF_CLEARHL))
                  {  SetDrMd(coo->rp,COMPLEMENT);
                     RectFill(coo->rp,x+histart,y,x+hiend-1,y+coo->rp->TxHeight-1);
                     SetDrMd(coo->rp,JAM1);
                  }
               }
            }
            else if(icon=Copyicon(cop))
            {  bitmap=(struct BitMap *)Getvalue(icon,BITMAP_BitMap);
               mask=(UBYTE *)Getvalue(icon,BITMAP_MaskPlane);
               bw=Getvalue(icon,BITMAP_Width);
               bh=Getvalue(icon,BITMAP_Height);
               if(w>bw) /* Center icon in space */
               {  x+=(w-bw)/2;
                  w=bw;
               }
               if(h>bh)
               {  y+=(h-bh)/2;
                  h=bh;
               }
               if(mask)
               {  BltMaskBitMapRastPort(bitmap,0,0,coo->rp,x,y,w,h,0xe0,mask);
               }
               else
               {  BltBitMapRastPort(bitmap,0,0,coo->rp,x,y,w,h,0xc0);
               }
            }
         }
         Drawframe(cop,coo,amr,TRUE);
      }
      Unclipcoords(coo);
   }
   return 0;
}

static long Setcopy(struct Copy *cop,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   void *newsource=NULL;
   UBYTE *defaulttype=NULL;
   UBYTE *pname=NULL,*ptype=NULL,*pvalue=NULL,*pvaluetype=NULL;
   BOOL super=FALSE,initload=FALSE,nodisplay=FALSE,param=FALSE,changed=FALSE,
      dispose=FALSE;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOCPY_Url:
            newsource=(void *)Agetattr((void *)tag->ti_Data,AOURL_Source);
            cop->flags|=CPYF_NEWSOURCE;
            cop->url=(void *)tag->ti_Data;
            if(cop->fld.win && (cop->flags&CPYF_INITIAL)) initload=TRUE;
            else if(cop->flags&CPYF_JSIMAGE) initload=TRUE;
            break;
         case AOCPY_Source:
            newsource=(void *)tag->ti_Data;
            cop->flags|=CPYF_NEWSOURCE;
            cop->flags&=~(CPYF_ERROR|CPYF_EOF);
            if(cop->driver)
            {  Adisposeobject((struct Aobject *)cop->driver);
               cop->driver=NULL;
               if(newsource)
               {  changed=Setchangedcopy(cop);
                  Changedlayout();
               }
            }
            break;
         case AOCPY_Driver:
            if(cop->driver)
            {  Adisposeobject((struct Aobject *)cop->driver);
               if((cop->flags&CPYF_BACKGROUND) && prefs.browser.docolors)
               {  /* Let parent know that previous background object is gone */
                  Asetattrs(cop->parent,AOBJ_Changedchild,(Tag)cop,TAG_END);
               }
            }
            if(tag->ti_Data
            && (cop->flags&(CPYF_EMBEDDED|CPYF_BACKGROUND|CPYF_BGSOUND))
            && ((struct Copydriver *)tag->ti_Data)->object.objecttype==AOTP_DOCUMENT)
            {  /* NEVER use a document as an embedded or background object */
               cop->flags|=CPYF_ERROR;
               cop->driver=NULL;
            }
            else
            {  if(tag->ti_Data)
               {  cop->driver=(struct Copydriver *)tag->ti_Data;
                  cop->flags|=CPYF_NEWDRIVER|CPYF_JSONLOAD;
                  Asetattrs((struct Aobject *)cop->driver,
                     AOCDV_Width,cop->width,
                     AOCDV_Height,cop->height,
                     AOCDV_Soundloop,cop->soundloop,
                     AOCDV_Displayed,(cop->flags&CPYF_DISPLAYED) &&
                        !(cop->flags&(CPYF_BACKGROUND|CPYF_BGSOUND|CPYF_ERROR)),
                     AOCDV_Reloadverify,(cop->flags&CPYF_RELOADVERIFY),
                     AOCDV_Mapdocument,(cop->flags&CPYF_MAPDOCUMENT),
                     AOCDV_Marginwidth,cop->mwidth,
                     AOCDV_Marginheight,cop->mheight,
                     ISEMPTY(&cop->params)?TAG_IGNORE:AOCDV_Objectparams,(Tag)cop->params.first,
                     AOBJ_Cframe,(cop->flags&CPYF_ERROR)?0:(Tag)cop->fld.elt.cframe,
                     AOBJ_Frame,(cop->flags&CPYF_ERROR)?0:(Tag)cop->frame,
                     AOBJ_Window,(cop->flags&CPYF_ERROR)?0:(Tag)cop->fld.win,
                     AOBJ_Winhis,(Tag)cop->whis,
                     AOBJ_Nobackground,BOOLVAL(cop->flags&CPYF_NOBACKGROUND),
                     TAG_END);
                  cop->flags&=~CPYF_RELOADVERIFY;  /* One verify is enough ;) */
                  if(!(cop->flags&(CPYF_EMBEDDED|CPYF_BACKGROUND)))
                  {  /* Re-position at fragment after fast cache reload */
                     Asetattrs(cop->frame,AOFRM_Reposfragment,TRUE,TAG_END);
                  }
               }
               else
               {  if(cop->driver)
                  {  changed=Setchangedcopy(cop);
                     Changedlayout();
                     cop->driver=NULL;
                  }
               }
            }
            break;
         case AOBJ_Layoutparent:
            cop->parent=(void *)tag->ti_Data;
            break;
         case AOCPY_Embedded:
            if(tag->ti_Data) cop->flags|=CPYF_EMBEDDED;
            else cop->flags&=~CPYF_EMBEDDED;
            break;
         case AOCPY_Background:
            if(tag->ti_Data) cop->flags|=CPYF_BACKGROUND;
            else cop->flags&=~CPYF_BACKGROUND;
            break;
         case AOCPY_Border:
            cop->border=tag->ti_Data;
            cop->flags|=CPYF_BORDERSET;
            break;
         case AOCPY_Hspace:
            cop->hspace=tag->ti_Data;
            break;
         case AOCPY_Vspace:
            cop->vspace=tag->ti_Data;
            break;
         case AOCPY_Mwidth:
            cop->mwidth=tag->ti_Data;
            break;
         case AOCPY_Mheight:
            cop->mheight=tag->ti_Data;
            break;
         case AOCPY_Width:
            cop->width=tag->ti_Data;
            break;
         case AOCPY_Height:
            cop->height=tag->ti_Data;
            break;
         case AOCPY_Percentwidth:
            cop->pwidth=tag->ti_Data;
            break;
         case AOCPY_Percentheight:
            cop->pheight=tag->ti_Data;
            break;
         case AOCPY_Usemap:
            cop->usemap=(void *)tag->ti_Data;
            break;
         case AOCPY_Ismap:
            if(tag->ti_Data) cop->flags|=CPYF_ISMAP;
            else cop->flags&=~CPYF_ISMAP;
            break;
         case AOCPY_Text:
            cop->text=(struct Buffer *)tag->ti_Data;
            break;
         case AOCPY_Referer:
            cop->referer=(void *)tag->ti_Data;
            cop->flags|=CPYF_REFERER;
            break;
         case AOCPY_Error:
            if(cop->flags&(CPYF_EMBEDDED|CPYF_BACKGROUND|CPYF_BGSOUND))
            {  if(tag->ti_Data)
               {  cop->flags|=CPYF_ERROR;
                  if(cop->driver)
                  {  Asetattrs((struct Aobject *)cop->driver,
                        AOCDV_Displayed,FALSE,
                        AOBJ_Frame,0,
                        AOBJ_Window,0,
                        TAG_END);
                  }
                  if(cop->flags&CPYF_DISPLAYED)
                  {  /* Change ourselves to the error icon */
                     changed=Setchangedcopy(cop);
                     Changedlayout();
                  }
               }
               else cop->flags&=~CPYF_ERROR;
            }
            if(cop->onerror)
            {
                Jkeepobject(cop->jobject,TRUE);
                Runjavascript(cop->frame?cop->frame:cop->jsframe,cop->onerror,&cop->jobject);
                Jkeepobject(cop->jobject,FALSE);
            }
            break;
         case AOCPY_Defaulttype:
            defaulttype=(UBYTE *)tag->ti_Data;
            break;
         case AOCPY_Soundloop:
            cop->soundloop=(long)tag->ti_Data;
            cop->flags|=CPYF_BGSOUND;
            cop->flags&=~(CPYF_EMBEDDED|CPYF_BACKGROUND);
            break;
         case AOCPY_Reloadverify:
            SETFLAG(cop->flags,CPYF_RELOADVERIFY,tag->ti_Data);
            break;
         case AOCPY_Info:
            cop->info=(void *)tag->ti_Data;
            if(cop->info)
            {  Asetattrs(cop->info,
                  AOINF_Target,(Tag)cop,
                  AOINF_Windowkey,Agetattr(cop->fld.win,AOWIN_Key),
                  TAG_END);
            }
            break;
         case AOCPY_Nodisplay:
            nodisplay=BOOLVAL(tag->ti_Data);
            break;
         case AOCPY_Mapdocument:
            SETFLAG(cop->flags,CPYF_MAPDOCUMENT,tag->ti_Data);
            break;
         case AOCPY_Objectready:
            SETFLAG(cop->flags,CPYF_OBJECTDEF,!tag->ti_Data);
            if(tag->ti_Data)
            {  if(cop->fld.win && (cop->flags&CPYF_INITIAL)) initload=TRUE;
            }
            else
            {  initload=FALSE;
            }
            break;
         case AOCPY_Paramname:
            pname=(UBYTE *)tag->ti_Data;
            param=TRUE;
            break;
         case AOCPY_Paramtype:
            ptype=(UBYTE *)tag->ti_Data;
            param=TRUE;
            break;
         case AOCPY_Paramvalue:
            pvalue=(UBYTE *)tag->ti_Data;
            param=TRUE;
            break;
         case AOCPY_Paramvaluetype:
            pvaluetype=(UBYTE *)tag->ti_Data;
            param=TRUE;
            break;
         case AOCPY_Trueimage:
            SETFLAG(cop->flags,CPYF_TRUEIMAGE,tag->ti_Data);
            break;
         case AOCPY_Name:
            if(cop->name) FREE(cop->name);
            cop->name=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOCPY_Onload:
            if(cop->onload) FREE(cop->onload);
            cop->onload=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOCPY_Onerror:
            if(cop->onerror) FREE(cop->onerror);
            cop->onerror=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOCPY_Onabort:
            if(cop->onabort) FREE(cop->onabort);
            cop->onabort=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOCPY_Onclick:
            if(cop->onclick) FREE(cop->onclick);
            cop->onclick=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOCPY_Onmouseover:
            if(cop->onmouseover) FREE(cop->onmouseover);
            cop->onmouseover=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOCPY_Onmouseout:
            if(cop->onmouseout) FREE(cop->onmouseout);
            cop->onmouseout=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOCPY_Onimgload:
            if(tag->ti_Data && cop->onload &&
               ((cop->frame && cop->flags&CPYF_JSONLOAD) || cop->flags&CPYF_JSIMAGE))
            {  if(!(cop->flags&CPYF_JSONLOADQ)) Queuesetmsg(cop,CQID_ONLOAD);
               cop->flags&=~CPYF_JSONLOAD;
               cop->flags|=CPYF_JSONLOADQ;
            }
            break;
         case AOCPY_Onimganim:
            if(tag->ti_Data && cop->onload && cop->frame
            && cop->flags&CPYF_DISPLAYED)
            {  if(!(cop->flags&CPYF_JSONLOADQ)) Queuesetmsg(cop,CQID_ONLOAD);
               cop->flags|=CPYF_JSONLOADQ;
            }
            break;
         case AOCPY_Jform:
            cop->jform=(void *)tag->ti_Data;
            break;
         case AOCPY_Infochanged:
            Queuesetmsg(cop,CQID_INFOCHANGED);
            break;
         case AOCPY_Deferdispose:
            Queuesetmsg(cop,CQID_DEFERDISPOSE);
            Asetattrs((struct Aobject *)cop,
               AOBJ_Window,0,
               AOBJ_Cframe,0,
               AOBJ_Frame,0,
               AOBJ_Layoutparent,0,
               TAG_END);
            break;
         case AOBJ_Queueid:
            switch(tag->ti_Data)
            {  case CQID_ONLOAD:
                  if(cop->onload && (cop->flags&CPYF_JSONLOADQ))
                  {  cop->flags&=~CPYF_JSONLOADQ;
                     Runjavascript(cop->frame?cop->frame:cop->jsframe,cop->onload,&cop->jobject);
                  }
                  break;
               case CQID_INFOCHANGED:
                  if(cop->info)
                  {  Asetattrs(cop->info,
                        AOINF_Target,(Tag)cop,
                        AOINF_Windowkey,Agetattr(cop->fld.win,AOWIN_Key),
                        TAG_END);
                  }
                  break;
               case CQID_DEFERDISPOSE:
                  dispose=TRUE;
                  break;
            }
            break;
         case AOBJ_Frame:
            if(tag->ti_Data)
            {  if(!cop->frame) cop->flags|=CPYF_JSONLOAD;
            }
            else cop->flags&=~CPYF_JSONLOAD;
            cop->frame=(void *)tag->ti_Data;
            if(!cop->frame && cop->info)
            {  cop->info=NULL;
            }
            break;
         case AOBJ_Cframe:
            cop->hilength=0;
            super=TRUE;
            break;
         case AOBJ_Window:
            if(tag->ti_Data && !cop->fld.win)
            {  Asetattrs(cop->source,AOSRC_Displayed,TRUE,TAG_END);
            }
            else if(!tag->ti_Data && cop->fld.win)
            {  Asetattrs(cop->fld.win,AOWIN_Goinactive,(Tag)cop,TAG_END);
               Asetattrs(cop->source,AOSRC_Displayed,FALSE,TAG_END);
               if(cop->popup)
               {  Adisposeobject(cop->popup);
                  cop->popup=NULL;
               }
               Freejcopy(cop);
            }
            cop->fld.win=(void *)tag->ti_Data;
            if(cop->fld.win && (cop->flags&CPYF_INITIAL) && !(cop->flags&CPYF_OBJECTDEF))
            {  initload=TRUE;
            }
            super=TRUE;
            break;
         case AOBJ_Winhis:
            cop->whis=(void *)tag->ti_Data;
            break;
         case AOBJ_Changedchild:
            if((void *)tag->ti_Data==cop->driver)
            {  /* Don't refresh if background and no background is shown */
               if(!(cop->flags&CPYF_BACKGROUND) || prefs.browser.docolors)
               {  changed=Setchangedcopy(cop);
                  if(changed) Changedlayout();
               }
            }
            break;
         case AOBJ_Nobackground:
            SETFLAG(cop->flags,CPYF_NOBACKGROUND,tag->ti_Data);
            break;
         case AOELT_Color:
            cop->color=(struct Colorinfo *)tag->ti_Data;
            break;
         case AOURL_Clientpull:
            if(cop->clientpull) FREE(cop->clientpull);
            cop->clientpull=Dupstr((UBYTE *)tag->ti_Data,-1);
            if(cop->clientpull && (cop->flags&CPYF_EOF))
            {

               Asetattrs(cop->frame,AOURL_Clientpull,(Tag)cop->clientpull,TAG_END);
            }
            break;
         case AOURL_Eof:
            if(tag->ti_Data)
            {  cop->flags|=CPYF_EOF;
               if(cop->clientpull)
               {

                   Asetattrs(cop->frame,AOURL_Clientpull,(Tag)cop->clientpull,TAG_END);
               }
            }
            break;
         case AOINF_Inquire:
            Asetattrs(cop->source,AOINF_Inquire,tag->ti_Data,TAG_END);
            /* Will also be forwarded to driver */
            break;
         case AOPUP_Inquire:
            Popupinquire(cop,(void *)tag->ti_Data);
            break;
         case AOPUP_Command:
            Popupselectcopy(cop,(UBYTE *)tag->ti_Data);
            break;
         case AOELT_Link:
            if(tag->ti_Data && !(cop->flags&CPYF_BORDERSET))
            {  cop->border=1;
            }
            super=TRUE;
            break;
         default:
            super=TRUE;
      }
   }
   if(super) Amethodas(AOTP_FIELD,cop,AOM_SET,(Tag)ams->tags);
   if(cop->frame && !(cop->flags&CPYF_REFERER))
   {  cop->referer=(void *)Agetattr(cop->frame,AOFRM_Url);
   }
   if(cop->flags&CPYF_NEWSOURCE)
   {  cop->flags&=~CPYF_NEWSOURCE;
      if(cop->source)
      {  if(cop->fld.win || (cop->flags&CPYF_JSIMAGE))
         {  Asetattrs(cop->source,AOSRC_Displayed,FALSE,TAG_END);
         }
         Aremchild(cop->source,(struct Aobject *)cop,0);
      }
      if(cop->driver)
      {  Adisposeobject((struct Aobject *)cop->driver);
         cop->driver=NULL;
         cop->flags|=CPYF_NEWDRIVER;
      }
      cop->source=newsource;
      if(cop->source)
      {  Aaddchild(cop->source,(struct Aobject *)cop,0);
         if(cop->fld.win || (cop->flags&CPYF_JSIMAGE))
         {  Asetattrs(cop->source,AOSRC_Displayed,TRUE,TAG_END);
         }
      }
   }
   if(cop->driver) Amethod((struct Aobject *)cop->driver,AOM_SET,(Tag)ams->tags);
   if(cop->flags&CPYF_NEWDRIVER)
   {  cop->flags&=~CPYF_NEWDRIVER;
      if(!(cop->flags&(CPYF_EMBEDDED|CPYF_BACKGROUND|CPYF_BGSOUND))
      && !Agetattr((struct Aobject *)cop->driver,AOCDV_Undisplayed))
      {  Asetattrs((struct Aobject *)cop->parent,AOFRM_Displaycopy,(Tag)cop,TAG_END);
      }
   }
   if(defaulttype) Asetattrs(cop->source,AOSRC_Defaulttype,(Tag)defaulttype,TAG_END);
   if(param) Addparam(cop,pname,pvalue,pvaluetype,ptype);
   if(initload) Initialloadcopy(cop);
   if(changed)
   {  Changedcopy(cop);
      cop->flags&=~CPYF_CHANGEDCOPY;
   }
   if(dispose)
   {  Adisposeobject((struct Aobject *)cop);
   }
   else if(nodisplay)
   {  /* This might dispose us: */
      Asetattrs(cop->parent,AOFRM_Cancelcopy,(Tag)cop,TAG_END);
   }
   return 0;
}

static struct Copy *Newcopy(struct Amset *ams)
{  struct Copy *cop;
   if(cop=Allocobject(AOTP_COPY,sizeof(struct Copy),ams))
   {  NEWLIST(&cop->params);
      cop->border=0;
      cop->flags=CPYF_INITIAL;
      cop->pwidth=cop->pheight=-1;
      Setcopy(cop,ams);
   }
   return cop;
}

static long Getcopy(struct Copy *cop,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   BOOL super=FALSE;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOCPY_Source:
            PUTATTR(tag,cop->source);
            break;
         case AOCPY_Url:
            if(cop->url)
            {  PUTATTR(tag,cop->url);
            }
            else
            {  PUTATTR(tag,Agetattr(cop->source,AOSRC_Url));
            }
            break;
         case AOCPY_Embedded:
            PUTATTR(tag,BOOLVAL(cop->flags&CPYF_EMBEDDED));
            break;
         case AOCPY_Background:
            PUTATTR(tag,BOOLVAL(cop->flags&CPYF_BACKGROUND));
            break;
         case AOCPY_Driver:
            PUTATTR(tag,cop->driver);
            break;
         case AOCPY_Hlayout:
            PUTATTR(tag,Agetattr((struct Aobject *)cop->driver,AOCDV_Hlayout));
            break;
         case AOCPY_Vlayout:
            PUTATTR(tag,Agetattr((struct Aobject *)cop->driver,AOCDV_Vlayout));
            break;
         case AOCPY_Referer:
            PUTATTR(tag,cop->referer);
            break;
         case AOCPY_Portnumber:
            PUTATTR(tag,Arexxportnumber(Agetattr(cop->fld.win,AOWIN_Key)));
            break;
         case AOBJ_Clipdrag:
            PUTATTR(tag,
               cop->fld.elt.textpos && (cop->flags&CPYF_EMBEDDED) &&
               !(cop->driver && Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR)));
            break;
         case AOBJ_Secure:
            PUTATTR(tag,Agetattr(cop->source,AOBJ_Secure));
            break;
         case AOBJ_Isframeset:
            PUTATTR(tag,Agetattr((struct Aobject *)cop->driver,AOBJ_Isframeset));
            break;
         default:
            super=TRUE;
      }
   }
   if(super) AmethodasA(AOTP_FIELD,(struct Aobject *)cop,(struct Amessage *)ams);
   if(cop->driver) AmethodA((struct Aobject *)cop->driver,(struct Amessage *)ams);
   return 0;
}

static long Hittestcopy(struct Copy *cop,struct Amhittest *amh)
{  long result=0;
   struct Coords *coo,coords={0};
   short target;  /* 1=ours, 2=ours with map, 3=driver, 4=superclass (link),
                   * 5=ours with unloaded form map */
   long x,y,w,h;
   void *url,*maparea;
   UBYTE *mapurl;
   long ready=0,shapes=0,extendhit=0;
   BOOL wasmaphit=BOOLVAL(cop->flags&CPYF_MAPHIT);
   BOOL popupchanged=
      BOOLVAL(cop->flags&CPYF_POPUPHIT)!=BOOLVAL(amh->flags&(AMHF_DOWNLOAD|AMHF_POPUP));
   Agetattrs((struct Aobject *)cop->driver,
      AOCDV_Ready,(Tag)&ready,
      AOCDV_Shapes,(Tag)&shapes,
      AOCDV_Extendhit,(Tag)&extendhit,
      TAG_END);
   if(!(coo=amh->coords))
   {  Framecoords(cop->fld.elt.cframe,&coords);
      coo=&coords;
   }
   x=amh->xco-coo->dx-cop->fld.elt.aox-Extraw(cop);
   y=amh->yco-coo->dy-cop->fld.elt.aoy-Extrah(cop);
   w=cop->fld.elt.aow-2*Extraw(cop);
   h=cop->fld.elt.aoh-2*Extrah(cop);
   cop->flags&=~(CPYF_MAPHIT|CPYF_POPUPHIT);
   maparea=cop->maparea;
   cop->maparea=NULL;
   if(Agetattr((struct Aobject *)cop,AOELT_Visible)
   && ((x>=0 && x<w && y>=0 && y<h) || extendhit || (cop->flags&CPYF_BACKGROUND)))
   {  if(ready && !(cop->flags&CPYF_ERROR))
      {  if(cop->fld.form)
         {  /* This is a form field, construct status msg */
            target=2;
            result|=AMHR_POPUP;
         }
         else if(shapes && !(cop->flags&CPYF_BACKGROUND))
         {  /* Our driver can handle input */
            target=3;
         }
         else
         {  result|=AMHR_POPUP;
            if(amh->flags&(AMHF_DOWNLOAD|AMHF_POPUP))
            {  /* Save or popup for our "image" */
               target=1;
               cop->flags|=CPYF_POPUPHIT;
            }
            else if(cop->usemap || (cop->flags&CPYF_ISMAP))
            {  /* Construct map url */
               target=2;
            }
            else
            {  /* Nothing special, let superclass look for link */
               target=4;
            }
         }
      }
      else  /* Unloaded icon */
      {  result|=AMHR_POPUP;
         if(amh->flags&(AMHF_DOWNLOAD|AMHF_POPUP))
         {  /* Download or popup, don't discriminate */
            target=1;
         }
         else if(!(cop->flags&CPYF_BACKGROUND))
         {  /* If we have a link then upper left half is for superclass or
             * it's our form submit, otherwise it's ours */
            if(cop->fld.form && x*h+y*w<w*h) target=5;
            else if(cop->fld.elt.link && x*h+y*w<w*h) target=4;
            else target=1;
         }
         else
         {  /* Unloaded background, no download or popup */
            target=0;
         }
      }
      switch(target)
      {  case 1:  /* Our hit */
            if(amh->oldobject==(struct Aobject *)cop && !wasmaphit && !popupchanged) result|=AMHR_OLDHIT;
            else
            {  result|=AMHR_NEWHIT;
               if(amh->amhr)
               {  if(cop->lasturl) FREE(cop->lasturl);
                  cop->lasturl=NULL;
                  amh->amhr->object=cop;
                  amh->amhr->jonmouse=cop;
                  url=(void *)Agetattr(cop->source,AOSRC_Url);
                  amh->amhr->text=Dupstr((UBYTE *)Agetattr(url,AOURL_Url),-1);
                  if(cop->fld.elt.textpos && cop->text)
                  {  amh->amhr->tooltip=Dupstr(cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length);
                  }
               }
            }
            break;
         case 2:  /* Our hit with map coordinates */
            /* Getmapurl() sets cop->maparea to the actual area: */
            Getmapurl(cop,amh->coords,amh->xco,amh->yco,&mapurl,NULL);
            if(mapurl)
            {  cop->flags|=CPYF_MAPHIT;
               if(amh->oldobject==(struct Aobject *)cop && cop->lasturl && STREQUAL(mapurl,cop->lasturl)
               && maparea==cop->maparea)
               {  result|=AMHR_OLDHIT;
                  FREE(mapurl);
               }
               else
               {  if(cop->lasturl) FREE(cop->lasturl);
                  cop->lasturl=Dupstr(mapurl,-1);
                  result|=AMHR_NEWHIT;
                  if(amh->amhr)
                  {  amh->amhr->object=cop;
                     amh->amhr->jonmouse=cop;
                     amh->amhr->text=mapurl;
                     if(cop->usemap)
                     {  amh->amhr->jonmouse=cop->maparea;
                     }
                     else
                     {  /* ISMAP, link will want JONMOUSE */
                        amh->amhr->jonmouse=cop->fld.elt.link;
                     }
                     amh->amhr->tooltip=Dupstr((UBYTE *)Agetattr(cop->maparea,AOARA_Tooltip),-1);
                     if(!amh->amhr->tooltip && cop->fld.elt.textpos && cop->text)
                     {  amh->amhr->tooltip=Dupstr(cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length);
                     }
                     if(prefs.browser.handpointer) amh->amhr->ptrtype=APTR_HAND;
                  }
                  else FREE(mapurl);
               }
            }
            else
            {  /* It's our hit but no action */
               if(amh->oldobject==(struct Aobject *)cop && !wasmaphit && !popupchanged) result|=AMHR_OLDHIT;
               else result|=AMHR_NEWHIT;
               if(amh->amhr)
               {  amh->amhr->object=cop;
                  amh->amhr->jonmouse=cop;
                  if(cop->fld.elt.textpos && cop->text)
                  {  amh->amhr->tooltip=Dupstr(cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length);
                  }
               }
            }
            break;
         case 3:  /* Forward to driver */
            result=AmethodA((struct Aobject *)cop->driver,(struct Amessage *)amh);
            if(amh->amhr && !amh->amhr->tooltip && cop->fld.elt.textpos && cop->text && amh->oldobject!=(struct Aobject *)cop)
            {  if(!(result&AMHR_RESULTMASK)) result|=AMHR_NEWHIT;
               amh->amhr->tooltip=Dupstr(cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length);
            }
            break;
         case 4:  /* Pass to superclass */
            if(!(amh->flags & AMHF_NOLINK))
            {
                result|=AmethodasA(AOTP_FIELD,(struct Aobject *)cop,(struct Amessage *)amh);
            }
            if(amh->amhr)
            {
                if(!(result&AMHR_RESULTMASK))
                {
                    if(amh->oldobject==(struct Aobject *)cop && !popupchanged) result|=AMHR_OLDHIT;
                    else result|=AMHR_NEWHIT;
                    amh->amhr->object=cop;
                    amh->amhr->jonmouse=cop;
                }
                if(!amh->amhr->tooltip && cop->fld.elt.textpos && cop->text)
                {
                    amh->amhr->tooltip=Dupstr(cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length);
                }
            }
            break;
         case 5:
            url=(void *)Agetattr(cop->fld.form,AOFOR_Action);
            if(mapurl=Dupstr((UBYTE *)Agetattr(url,AOURL_Url),-1))
            {  cop->flags|=CPYF_MAPHIT;
               if(amh->oldobject==(struct Aobject *)cop && cop->lasturl && STREQUAL(mapurl,cop->lasturl))
               {  result|=AMHR_OLDHIT;
                  FREE(mapurl);
               }
               else
               {  if(cop->lasturl) FREE(cop->lasturl);
                  cop->lasturl=Dupstr(mapurl,-1);
                  result|=AMHR_NEWHIT;
                  if(amh->amhr)
                  {  amh->amhr->object=cop;
                     amh->amhr->jonmouse=cop;
                     amh->amhr->text=mapurl;
                     if(cop->fld.elt.textpos && cop->text)
                     {  amh->amhr->tooltip=Dupstr(cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length);
                     }
                  }
                  else FREE(mapurl);
               }
            }
            break;
      }
   }
   return result;
}

static long Goactivecopy(struct Copy *cop,struct Amgoactive *amg)
{  if(!(cop->flags&CPYF_BACKGROUND))
   {  Arender((struct Aobject *)cop,NULL,0,0,AMRMAX,AMRMAX,AMRF_UPDATESELECTED,NULL);
   }
   return AMR_ACTIVE;
}

static long Handleinputcopy(struct Copy *cop,struct Aminput *ami)
{  long result=AMR_NOCARE,htresult;
   struct Coords coords={0};
   long x,y;
   void *url;
   UBYTE *mapurl=NULL,*fragment=NULL,*targetname=NULL;
   if(ami->imsg)
   {  switch(ami->imsg->Class)
      {  case IDCMP_MOUSEMOVE:
         case IDCMP_RAWKEY:
            result=AMR_REUSE;
            Framecoords(cop->fld.elt.cframe,&coords);
            if(cop->flags&CPYF_MAPHIT)
            {  /* Started as a point into a map. See if it is still ours, and not changed
                * to download/popup, if so return REUSE. */
               if(!(ami->flags&(AMHF_DOWNLOAD|AMHF_POPUP)))
               {  if(Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR))
                  {  Getmapurl(cop,&coords,ami->imsg->MouseX,ami->imsg->MouseY,&mapurl,NULL);
                  }
                  else if(cop->fld.form)
                  {  htresult=Ahittest((struct Aobject *)cop,NULL,ami->imsg->MouseX,ami->imsg->MouseY,
                        ami->flags,cop,NULL) & AMHR_RESULTMASK;
                     if(htresult==AMHR_OLDHIT)
                     {  mapurl=Dupstr(cop->lasturl,-1);
                     }
                     else if(cop->lasturl)
                     {  FREE(cop->lasturl);
                        cop->lasturl=NULL;
                     }
                  }
                  if(mapurl)
                  {  result=AMR_ACTIVE;
                     if(!(cop->lasturl && STREQUAL(mapurl,cop->lasturl)))
                     {  if(cop->lasturl) FREE(cop->lasturl);
                        cop->lasturl=Dupstr(mapurl,-1);
                        if(ami->amir)
                        {  ami->amir->text=mapurl;
                        }
                     }
                     else FREE(mapurl);
                  }
               }
               else if(cop->lasturl)
               {  FREE(cop->lasturl);
                  cop->lasturl=NULL;
               }
            }
            else
            {  htresult=Ahittest((struct Aobject *)cop,NULL,ami->imsg->MouseX,ami->imsg->MouseY,
                  ami->flags,cop,NULL) & AMHR_RESULTMASK;
               if(htresult==AMHR_OLDHIT)
               {  result=AMR_ACTIVE;
               }
            }
            break;
         case IDCMP_MOUSEBUTTONS:
            if(ami->flags&AMHF_POPUPREL)
            {  cop->popup=Anewobject(AOTP_POPUP,
                  AOPUP_Target,(Tag)cop,
                  AOPUP_Target,(Tag)cop->fld.elt.link,
                  (cop->flags&CPYF_BACKGROUND)?AOPUP_Target:TAG_IGNORE,(Tag)cop->frame,
                  AOPUP_Left,ami->imsg->MouseX,
                  AOPUP_Top,ami->imsg->MouseY,
                  AOPUP_Window,(Tag)cop->fld.win,
                  TAG_END);
            }
            else if(ami->imsg->Code==SELECTUP)
            {
               /* handle our onclick event */
               if(cop->onclick)
               {
                   Runjavascript(cop->frame?cop->frame:cop->jsframe,cop->onclick,&cop->jobject);
               }
               if(cop->flags&CPYF_MAPHIT)
               {  Framecoords(cop->fld.elt.cframe,&coords);
                  if(cop->fld.form)
                  {  if(Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR))
                     {  if(Getmapcoords(cop,&coords,ami->imsg->MouseX,ami->imsg->MouseY,&x,&y))
                        {  Amethod(cop->fld.form,AFO_SUBMIT,(Tag)cop,x,y);
                        }
                     }
                     else
                     {  Amethod(cop->fld.form,AFO_SUBMIT,(Tag)cop,0,0);
                     }
                  }
                  else
                  {  Getmapurl(cop,&coords,ami->imsg->MouseX,ami->imsg->MouseY,
                        &mapurl,&targetname);
                     /* If there is an url, make sure to run the onclick handler.
                      * but only attempt to follow the link if the mapurl is not empty. */
                     if(mapurl
                     && (!cop->maparea || Areaonclick(cop->maparea)))
                     {  if(*mapurl)
                        {  fragment=Fragmentpart(mapurl);
                           url=Findurl("",mapurl,0);
                           if(url)
                           {  void *win=NULL,*target;
                              UBYTE *frameid=NULL;
                              target=Targetframe(cop->frame,targetname);
                              Agetattrs(target,
                                 AOBJ_Window,(Tag)&win,
                                 AOFRM_Id,(Tag)&frameid,
                                 TAG_END);
                              Inputwindoc(win,url,fragment,frameid);
                           }
                           FREE(mapurl);
                        }
                     }
                  }
               }
               else
               {  url=(void *)Agetattr(cop->source,AOSRC_Url);
                  if(ami->flags&AMHF_DOWNLOAD)
                  {  if(url) Auload(url,AUMLF_DOWNLOAD|PROXY(cop),cop->referer,NULL,cop->frame);
                  }
                  else
                  {  /* Set defaulttype to image because it might have been an image
                      * link before that used the image viewer */
                     Asetattrs(cop->source,AOSRC_Defaulttype,(Tag)"IMAGE/X-UNKNOWN",TAG_END);
                     Auload(url,AUMLF_IMAGE|((cop->flags&CPYF_ERROR)?AUMLF_RELOAD:0)|PROXY(cop),
                        cop->referer,NULL,cop->frame);
                  }
               }
               result=AMR_NOREUSE;
            }
            break;
      }
   }
   return result;
}

static long Goinactivecopy(struct Copy *cop)
{  if(!(cop->flags&CPYF_BACKGROUND))
   {  Arender((struct Aobject *)cop,NULL,0,0,AMRMAX,AMRMAX,AMRF_UPDATENORMAL,NULL);
   }
   if(cop->lasturl)
   {  FREE(cop->lasturl);
      cop->lasturl=NULL;
   }
   return 0;
}

static long Notifycopy(struct Copy *cop,struct Amnotify *amn)
{  long result=0;
   BOOL forward=TRUE;
   if(amn->nmsg->method==ACM_LOAD)
   {  Loadcopy(cop,(struct Acmload *)amn->nmsg);
   }
   else if(amn->nmsg->method==AOM_GETREXX)
   {  struct Amgetrexx *amg=(struct Amgetrexx *)amn->nmsg;
      if(amg->info==AMGRI_IMAGES && (!cop->driver || cop->driver->object.objecttype!=AOTP_DOCUMENT))
      {  UBYTE *type,*buf=NULL;
         if(cop->flags&CPYF_BGSOUND) type="BGSOUND";
         else if(cop->flags&CPYF_BACKGROUND) type="BACKGROUND";
         else if(cop->usemap) type="USEMAP";
         else if(cop->flags&CPYF_ISMAP) type="ISMAP";
         else type="IMAGE";
         amg->index++;
         Setstemvar(amg->ac,amg->stem,amg->index,"URL",
            (UBYTE *)Agetattr(cop->url,AOURL_Url));
         Setstemvar(amg->ac,amg->stem,amg->index,"TYPE",type);
         if(cop->fld.elt.textpos && cop->text)
         {  buf=Dupstr(cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length);
         }
         Setstemvar(amg->ac,amg->stem,amg->index,"ALT",buf?buf:NULLSTRING);
         if(buf) FREE(buf);
      }
      else if(amg->info==AMGRI_INFO)
      {  AmethodA(cop->source,(struct Amessage *)amn);
         /* and forward to driver */
      }
   }
   else if(amn->nmsg->method==AOM_SET)
   {  struct Amset *ams=(struct Amset *)amn->nmsg;
      struct TagItem *tag,*tstate=ams->tags;
      forward=FALSE;
      while(tag=NextTagItem(&tstate))
      {  switch(tag->ti_Tag)
         {  case AOBJ_Bgchanged:
               if(cop->driver) Asetattrs((struct Aobject *)cop->driver,AOCDV_Bgchanged,tag->ti_Data,TAG_END);
               break;
            default:
               forward=TRUE;
         }
      }
   }
   if(forward && cop->driver)
   {  result=AmethodA((struct Aobject *)cop->driver,(struct Amessage *)amn);
   }
   return result;
}

static long Searchposcopy(struct Copy *cop,struct Amsearch *ams)
{  if(cop->driver && Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR))
   {  AmethodA((struct Aobject *)cop->driver,(struct Amessage *)ams);
   }
   else if((cop->flags&CPYF_EMBEDDED) && cop->fld.elt.textpos)
   {  if(ams->top<cop->fld.elt.aoy+cop->fld.elt.aoh)
      {  ams->pos=cop->fld.elt.textpos;
         return 1;
      }
   }
   return 0;
}

static long Searchsetcopy(struct Copy *cop,struct Amsearch *ams)
{  long result=0;
   BOOL render=FALSE;
   if(cop->driver && Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR))
   {  result=AmethodA((struct Aobject *)cop->driver,(struct Amessage *)ams);
   }
   else if((cop->flags&CPYF_EMBEDDED) && cop->fld.elt.textpos)
   {  if(ams->pos<cop->fld.elt.textpos+cop->fld.elt.length)
      {  ams->left=cop->fld.elt.aox;
         ams->top=cop->fld.elt.aoy;
         render=!(cop->driver && Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR));
         if(ams->flags&AMSF_HIGHLIGHT)
         {  cop->histart=ams->pos;
            cop->hilength=ams->length;
            if(render) Arender((struct Aobject *)cop,NULL,0,0,AMRMAX,AMRMAX,0,ams->text);
         }
         else if(ams->flags&AMSF_UNHIGHLIGHT)
         {  if(render) Arender((struct Aobject *)cop,NULL,0,0,AMRMAX,AMRMAX,AMRF_CLEARHL,ams->text);
            cop->hilength=0;
         }
         if(ams->pos+ams->length<=cop->fld.elt.textpos+cop->fld.elt.length) result=1;
      }
   }
   return result;
}

static long Dragtestcopy(struct Copy *cop,struct Amdragtest *amd)
{  long result=0;
   struct TextExtent te={0};
   long x,y,w,cx,len,hi;
   if(cop->driver && Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR))
   {  result=AmethodA((struct Aobject *)cop->driver,(struct Amessage *)amd);
   }
   else if((cop->flags&CPYF_EMBEDDED) && cop->fld.elt.textpos)
   {  x=amd->xco-amd->coords->dx;
      y=amd->yco-amd->coords->dy;
      cx=cop->fld.elt.aox+Extraw(cop);
      w=cop->fld.elt.aow-2*Extraw(cop);
      if((cop->fld.elt.aoy<=y && cop->fld.elt.aoy+cop->fld.elt.aoh>y && cx+w>x) || cop->fld.elt.aoy>y)
      {  result=AMDR_HIT;
         if(amd->amdr)
         {  amd->amdr->object=cop;
            /* Object position is buffer position of character hit */
            if(x<cx || y<cop->fld.elt.aoy)
            {  amd->amdr->objpos=cop->fld.elt.textpos;
            }
            else
            {  /* First determine centering offset of text */
               SetFont(mrp,cop->fld.elt.font);
               SetSoftStyle(mrp,cop->fld.elt.style,0x0f);
               len=TextFit(mrp,cop->text->buffer+cop->fld.elt.textpos,cop->fld.elt.length,&te,NULL,
                  1,w,cop->fld.elt.aoh-2*Extrah(cop));
               if(len>0)
               {  cx+=(w-te.te_Width)/2;
                  SetSoftStyle(mrp,0,0x0f);  /* No style because italics extension */
                  hi=TextFit(mrp,cop->text->buffer+cop->fld.elt.textpos,len-1,&te,NULL,1,x-cx,cop->fld.elt.aoh);
                  amd->amdr->objpos=cop->fld.elt.textpos+hi;
               }
               else
               {  amd->amdr->objpos=cop->fld.elt.textpos;
               }
            }
         }
      }
   }
   return result;
}

/* Returns new state */
static UWORD Draglimits(struct Copy *cop,void *startobject,ULONG startobjpos,
   void *endobject,ULONG endobjpos,UWORD state,long *startp,long *lengthp,BOOL *act)
{  long s=0,l=0;
   if(cop==startobject)
   {  switch(state)
      {  case AMDS_BEFORE:
            state=AMDS_RENDER;
            *act=TRUE;
            if(cop==endobject)
            {  state=AMDS_AFTER;
               if(endobjpos>=startobjpos)
               {  s=startobjpos;
                  l=endobjpos-s+1;
               }
               else
               {  s=endobjpos;
                  l=startobjpos-s+1;
               }
            }
            else
            {  s=startobjpos;
               l=cop->fld.elt.textpos+cop->fld.elt.length-s;
            }
            break;
         case AMDS_REVERSE:
            state=AMDS_AFTER;
            *act=TRUE;
            s=cop->fld.elt.textpos;
            l=startobjpos-cop->fld.elt.textpos+1;
            break;
      }
   }
   else if(cop==endobject)
   {  switch(state)
      {  case AMDS_BEFORE:
            state=AMDS_REVERSE;
            *act=TRUE;
            s=endobjpos;
            l=cop->fld.elt.textpos+cop->fld.elt.length-s;
            break;
         case AMDS_RENDER:
            state=AMDS_AFTER;
            *act=TRUE;
            s=cop->fld.elt.textpos;
            l=endobjpos-s+1;
            break;
      }
   }
   else
   {  switch(state)
      {  case AMDS_BEFORE:
            s=0;
            l=0;
            break;
         case AMDS_RENDER:
         case AMDS_REVERSE:
            *act=TRUE;
            s=cop->fld.elt.textpos;
            l=cop->fld.elt.length;
            break;
         case AMDS_AFTER:
            s=0;
            l=0;
            if(!cop->histart)
            {  state=AMDS_DONE;
            }
            break;
      }
   }
   *startp=s;
   *lengthp=l;
   return state;
}

static long Dragrendercopy(struct Copy *cop,struct Amdragrender *amd)
{  long result=0;
   struct Coords *coo;
   long s,l;
   BOOL act;
   if(cop->driver && Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR))
   {  result=AmethodA((struct Aobject *)cop->driver,(struct Amessage *)amd);
   }
   else if((cop->flags&CPYF_EMBEDDED) && cop->fld.elt.textpos)
   {  amd->state=Draglimits(cop,amd->startobject,amd->startobjpos,
         amd->endobject,amd->endobjpos,amd->state,&s,&l,&act);
      if(cop->histart!=s || cop->hilength!=l)
      {  coo=Clipcoords(cop->fld.elt.cframe,amd->coords);
         if(coo)
         {  Highlightcopy(cop,coo);
            cop->histart=s;
            cop->hilength=l;
            Highlightcopy(cop,coo);
            Unclipcoords(coo);
         }
      }
   }
   return result;
}

static long Dragcopycopy(struct Copy *cop,struct Amdragcopy *amd)
{  long result=0;
   long s,l;
   short i,n;
   BOOL act=FALSE;
   if(cop->driver && Agetattr((struct Aobject *)cop->driver,AOCDV_Ready) && !(cop->flags&CPYF_ERROR)
   && !(cop->flags&CPYF_EMBEDDED))
   {  result=AmethodA((struct Aobject *)cop->driver,(struct Amessage *)amd);
   }
   else
   {  amd->state=Draglimits(cop,amd->startobject,amd->startobjpos,
         amd->endobject,amd->endobjpos,amd->state,&s,&l,&act);
      if((cop->fld.elt.textpos && l) || ((cop->fld.elt.halign&HALIGN_BULLET) && act))
      {  if(cop->fld.elt.leftindent
         && (!amd->clip->length || amd->clip->buffer[amd->clip->length-1]=='\n'))
         {  n=cop->fld.elt.leftindent;
            if(cop->fld.elt.halign&HALIGN_BULLET) n--;
            for(i=0;i<n;i++)
            {  Addtobuffer(amd->clip,"    ",4);
            }
         }
         if(cop->fld.elt.textpos && l)
         {  if(cop->fld.elt.halign&HALIGN_BULLET)
            {  Addtobuffer(amd->clip,"  ",2);
            }
            Addtobuffer(amd->clip,cop->text->buffer+s,l);
         }
         else
         {  Addtobuffer(amd->clip,"  * ",4);
         }
      }
   }
   return result;
}

static void Disposecopy(struct Copy *cop)
{  void *p;
   if(cop->popup) Adisposeobject(cop->popup);
   if(cop->fld.win || (cop->flags&CPYF_JSIMAGE))
   {  Asetattrs(cop->source,AOSRC_Displayed,FALSE,TAG_END);
   }
   if(cop->fld.win) Asetattrs(cop->fld.win,AOWIN_Goinactive,(Tag)cop,TAG_END);
   if(cop->driver) Adisposeobject((struct Aobject *)cop->driver);
   if(cop->source) Aremchild(cop->source,(struct Aobject *)cop,0);
   if(cop->lasturl) FREE(cop->lasturl);
   if(cop->clientpull) FREE(cop->clientpull);
   if(cop->name) FREE(cop->name);
   while(p=REMHEAD(&cop->params)) Disposeparam(p);
   if(cop->onload) FREE(cop->onload);
   if(cop->onerror) FREE(cop->onerror);
   if(cop->onabort) FREE(cop->onabort);
   if(cop->onclick) FREE(cop->onclick);
   if(cop->onmouseover) FREE(cop->onmouseover);
   if(cop->onmouseout) FREE(cop->onmouseout);
   Freejcopy(cop);
   Queuesetmsg(cop,0);
   Amethodas(AOTP_FIELD,cop,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Copy_Dispatcher,
struct Copy *,cop,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newcopy((struct Amset *)amsg);
         break;
      case AOM_SET:
      case AOM_UPDATE:
         result=Setcopy(cop,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getcopy(cop,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measurecopy(cop,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutcopy(cop,(struct Amlayout *)amsg);
         break;
      case AOM_ALIGN:
         result=Aligncopy(cop,(struct Amalign *)amsg);
         break;
      case AOM_MOVE:
         result=Movecopy(cop,(struct Ammove *)amsg);
         break;
      case AOM_RENDER:
         result=Rendercopy(cop,(struct Amrender *)amsg);
         break;
      case AOM_HITTEST:
         result=Hittestcopy(cop,(struct Amhittest *)amsg);
         break;
      case AOM_GOACTIVE:
         result=Goactivecopy(cop,(struct Amgoactive *)amsg);
         break;
      case AOM_HANDLEINPUT:
         result=Handleinputcopy(cop,(struct Aminput *)amsg);
         break;
      case AOM_GOINACTIVE:
         result=Goinactivecopy(cop);
         break;
      case AOM_NOTIFY:
         result=Notifycopy(cop,(struct Amnotify *)amsg);
         break;
      case AOM_SEARCHPOS:
         result=Searchposcopy(cop,(struct Amsearch *)amsg);
         break;
      case AOM_SEARCHSET:
         result=Searchsetcopy(cop,(struct Amsearch *)amsg);
         break;
      case AOM_DRAGTEST:
         result=Dragtestcopy(cop,(struct Amdragtest *)amsg);
         break;
      case AOM_DRAGRENDER:
         result=Dragrendercopy(cop,(struct Amdragrender *)amsg);
         break;
      case AOM_DRAGCOPY:
         result=Dragcopycopy(cop,(struct Amdragcopy *)amsg);
         break;
      case AOM_JSETUP:
         result=Jsetupcopy(cop,(struct Amjsetup *)amsg);
         break;
      case AOM_JONMOUSE:
         result=Jonmousecopy(cop,(struct Amjonmouse *)amsg);
         break;
      case AOM_DISPOSE:
         Disposecopy(cop);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         result=AmethodasA(AOTP_FIELD,(struct Aobject *)cop,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installcopy(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_COPY,(Tag)Copy_Dispatcher)) return FALSE;
   return TRUE;
}

long Anotifycload(struct Aobject *ao,ULONG flags)
{  struct Acmload acml={{0}};
   struct Amnotify amn={{0}};
   acml.amsg.method=ACM_LOAD;
   acml.flags=flags;
   amn.amsg.method=AOM_NOTIFY;
   amn.nmsg=(struct Amessage *)&acml;
   return (long)AmethodA(ao,(struct Amessage *)&amn);
}
