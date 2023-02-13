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

/* frameset.c - AWeb html document frameset object */

#include "aweb.h"
#include "frameset.h"
#include "element.h"
#include "window.h"
#include "application.h"
#include <proto/utility.h>
#include <proto/graphics.h>

/*------------------------------------------------------------------------*/

struct Frameset
{  struct Aobject object;
   long aox,aoy,aow,aoh;
   void *frame;
   void *pool;
   LIST(Child) childs;
   long nrchilds;
   short spacing;
   short border;
   UWORD flags;
   long ldimension;                 /* Last width or height layed out against. */
   void *win;
   void *whis;
   long minsize,maxsize,cursize;    /* Minimum, maximum and current coordinate of resizing */
   long grab;                       /* Value to add to mouse pos to get current resize pos */
   struct Child *sizechild;         /* First resizing child */
   UBYTE *subcols;                  /* Colspec to pass to subframe if FRSF_BOTH */
   long subnrcols;                  /* Nr of elements to add to one subframeset */
   long subleft;                    /* Nr of elements left to add to subframeset */
   struct Child *subchild;          /* Current subframeset child to add to. */
};

#define FRSF_ROWS          0x0001   /* Vertical set, else horizontal. */
#define FRSF_BORDERSET     0x0004   /* Border was set explicitly */
#define FRSF_SPACINGSET    0x0008   /* Spacing was set explicitly */
#define FRSF_NOBACKGROUND  0x0010   /* Use white to clear bg */
#define FRSF_END           0x0020   /* Definition is complete */
#define FRSF_BOTH          0x0040   /* Both ROWS and COLS were specified */
#define FRSF_SENSIBLE      0x0080   /* Sensible frame */
#define FRSF_RESET         0x0100   /* Do layout even if dims unchanged */

struct Child
{  NODE(Child);
   struct Aobject *object;          /* Another FRAMESET or a FRAME */
   long rawsize;                    /* Suggested size in pixels, perentage or relative */
   long size;                       /* Idem, but pixel size corrected for borders */
   UWORD type;                     /* Type of units */
   long dim;                        /* Computed dimension */
   long mindim;                     /* Minimum size in given dimension */
   long minw,minh;                  /* Minimum needed by child */
   long width,height;               /* Computed width and height */
   long x,y;                        /* Frameset relative coordinates */
};

#define FCHT_PIXELS     1
#define FCHT_PERCENT    2
#define FCHT_RELATIVE   3

/*------------------------------------------------------------------------*/

/* Forward this message to all childs */
static void Broadcast(struct Frameset *frs,struct Amessage *amsg)
{  struct Child *ch;
   for(ch=frs->childs.first;ch->next;ch=ch->next)
   {  if(ch->object) AmethodA(ch->object,amsg);
   }
}

/* Parse COLS or ROWS spec */
static void Parsespec(struct Frameset *frs,UBYTE *spec)
{  UBYTE *p=spec;
   long n,nrc;
   struct Child *ch;
   nrc=0;
   while(*p)
   {  while(*p && *p!='*' && !isdigit(*p)) p++;
      if(!*p) break;
      n=0;
      while(*p && isdigit(*p))
      {  n=10*n+*p-'0';
         p++;
      }
      if(n==0) n=1;
      if(ch=PALLOCSTRUCT(Child,1,MEMF_CLEAR,frs->pool))
      {  ADDTAIL(&frs->childs,ch);
         if(*p=='*')
         {  ch->type=FCHT_RELATIVE;
         }
         else if(*p=='%')
         {  ch->type=FCHT_PERCENT;
            if(n>100) n=100;
         }
         else
         {  ch->type=FCHT_PIXELS;
         }
         ch->rawsize=n;
         nrc++;
      }
      if(*p) p++;
   }
// if(nrc>1) frs->flags|=FRSF_SENSIBLE;
   if(nrc>0) frs->flags|=FRSF_SENSIBLE;
}

/* Get number of items in a spec */
static long Nritemsinspec(UBYTE *spec)
{  UBYTE *p=spec;
   long n=0;
   while(*p)
   {  while(*p && *p!='*' && !isdigit(*p)) p++;
      if(!*p) break;
      n++;
      while(*p && isdigit(*p)) p++;
      if(*p=='*') p++;
   }
   return n;
}

/* Find the first true child */
static struct Child *Firstchild(struct Frameset *frs)
{  struct Child *ch;
   for(ch=frs->childs.first;ch->next && !ch->object;ch=ch->next);
   if(!ch->next) ch=NULL;
   return ch;
}

/* Find the next true child */
static struct Child *Nextchild(struct Child *ch)
{  for(ch=ch->next;ch->next && !ch->object;ch=ch->next);
   if(!ch->next) ch=NULL;
   return ch;
}

/* Return pointer type if this coordinates would allow resizing these two childs;
 * default(==0) is not allowed. */
static UWORD Resizehit(struct Frameset *frs,long x,long y,struct Child *ch1,struct Child *ch2)
{  UWORD ptrtype=APTR_DEFAULT;
   if(ch1 && ch1->object && ch2 && ch2->object)
   {  if(frs->flags&FRSF_ROWS)
      {  if(y>=ch1->y+ch1->height-2 && y<=ch2->y+1
            && Agetattr(ch1->object,AOFRM_Resizebottom)
            && Agetattr(ch2->object,AOFRM_Resizetop))
         {  ptrtype=APTR_RESIZEVERT;
         }
      }
      else
      {  if(x>=ch1->x+ch1->width-2 && x<=ch2->x+1
            && Agetattr(ch1->object,AOFRM_Resizeright)
            && Agetattr(ch2->object,AOFRM_Resizeleft))
         {  ptrtype=APTR_RESIZEHOR;
         }
      }
   }
   return ptrtype;
}

/* Check if frameset can be resized at this edge */
static BOOL Resizefirst(struct Frameset *frs,ULONG tag)
{  struct Child *ch=Firstchild(frs);
   return (BOOL)(ch && Agetattr(ch->object,tag));
}

static BOOL Resizelast(struct Frameset *frs,ULONG tag)
{  struct Child *ch;
   for(ch=frs->childs.last;ch->prev && !ch->object;ch=ch->prev);
   return (BOOL)(ch->prev && Agetattr(ch->object,tag));
}

static BOOL Resizeall(struct Frameset *frs,ULONG tag)
{  struct Child *ch;
   BOOL resize=TRUE;
   for(ch=Firstchild(frs);resize && ch;ch=Nextchild(ch))
   {  resize=Agetattr(ch->object,tag);
   }
   return resize;
}

/* Render sizing line */
static void Rendersize(struct Frameset *frs,struct Coords *coo)
{  coo=Clipcoords(frs->frame,coo);
   if(coo && coo->rp)
   {  SetDrMd(coo->rp,COMPLEMENT);
      if(frs->flags&FRSF_ROWS)
      {  Move(coo->rp,frs->aox+coo->dx,frs->aoy+frs->cursize+coo->dy);
         Draw(coo->rp,frs->aox+coo->dx+frs->aow-1,frs->aoy+frs->cursize+coo->dy);
      }
      else
      {  Move(coo->rp,frs->aox+frs->cursize+coo->dx,frs->aoy+coo->dy);
         Draw(coo->rp,frs->aox+frs->cursize+coo->dx,frs->aoy+coo->dy+frs->aoh-1);
      }
      SetDrMd(coo->rp,JAM1);
   }
   Unclipcoords(coo);
}

/* Perform the resizing of the two resized childs */
static void Doresizechilds(struct Frameset *frs)
{  struct Child *ch1,*ch2;
   long oldcur;
   if((ch1=frs->sizechild) && (ch2=Nextchild(ch1)))
   {  if(frs->flags&FRSF_ROWS)
      {  oldcur=(ch1->y+ch1->height+ch2->y)/2;
         ch1->height+=frs->cursize-oldcur;
         ch2->height-=frs->cursize-oldcur;
         ch2->y+=frs->cursize-oldcur;
         ch1->dim=ch1->height;
         ch2->dim=ch2->height;
      }
      else
      {  oldcur=(ch1->x+ch1->width+ch2->x)/2;
         ch1->width+=frs->cursize-oldcur;
         ch2->width-=frs->cursize-oldcur;
         ch2->x+=frs->cursize-oldcur;
         ch1->dim=ch1->width;
         ch2->dim=ch2->width;
      }
      if(frs->cursize!=oldcur)
      {  Rendersize(frs,NULL);
         Alayout(ch1->object,ch1->width,ch1->height,AMLF_FORCE,NULL,0,NULL);
         Aalign(ch1->object,frs->aox+ch1->x,frs->aoy+ch1->y,0,0);
         Alayout(ch2->object,ch2->width,ch2->height,AMLF_FORCE,NULL,0,NULL);
         Aalign(ch2->object,frs->aox+ch2->x,frs->aoy+ch2->y,0,0);
         frs->sizechild=NULL;
         Arender((struct Aobject *)frs,NULL,0,0,AMRMAX,AMRMAX,AMRF_CLEAR,NULL);
      }
   }
}

/*------------------------------------------------------------------------*/

static long Measureframeset(struct Frameset *frs,struct Ammeasure *amm)
{  struct Child *ch;
   long minw=0,minh=0;
   struct Ammresult ammr;

   if(!(frs->flags&FRSF_END)) return 0;

   for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
   {  ammr.width=ammr.minwidth=ammr.minheight=0;
      Ameasure(ch->object,amm->width,amm->height,0,amm->flags|AMMF_MINHEIGHT,NULL,&ammr);
      ch->minw=ammr.minwidth;
      ch->minh=ammr.minheight;
      if(frs->flags&FRSF_ROWS)
      {  minh+=ch->minh;
         if(ch->minw>minw) minw=ch->minw;
      }
      else
      {  minw+=ch->minw;
         if(ch->minh>minh) minh=ch->minh;
      }
   }
   if(frs->flags&FRSF_ROWS)
   {  minh+=(frs->nrchilds-1)*frs->spacing;
   }
   else
   {  minw+=(frs->nrchilds-1)*frs->spacing;
   }
   if(amm->ammr)
   {  amm->ammr->width=amm->width;
      amm->ammr->minwidth=minw;
      amm->ammr->minheight=minh;
   }
   return 0;
}

static long Layoutframeset(struct Frameset *frs,struct Amlayout *aml)
{  struct Child *ch;
   long available;      /* Available room in given dimension */
   long totpixel=0;     /* Total desired size of pixel sized childs */
   long totpixsize=0;   /* Total wanted size of pixel sized childs */
   long minpixel=0;     /* Total minimum size of pixel sized childs */
   long minperc=0;      /* Total minimum size of percentage sized childs */
   long minrel=0;       /* Total minimum size of relative sized childs */
   long totalprc=0;     /* Total percentage used */
   long totalrel=0;     /* Total number of relative units */
   long relprc=0;       /* Percentage left for all relative sized combined */
   long reldim=0;       /* Size left for all relative sized combined */
   long minother=0;     /* Minimum size in other direction */
   long d,x,y;
   BOOL done;

   if(!(frs->flags&FRSF_END)) return 0;

   if(frs->flags&FRSF_ROWS) available=aml->height;
   else available=aml->width;

   /* Only do the layout magic if the dimension to layout against has changed. */
   if(available==frs->ldimension && !(frs->flags&FRSF_RESET))
   {  /* Compute minother */
      for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
      {  if(frs->flags&FRSF_ROWS)
         {  d=ch->minw;
         }
         else
         {  d=ch->minh;
         }
         if(d>minother) minother=d;
      }
   }
   else
   {  frs->ldimension=available;
      available-=(frs->nrchilds-1)*frs->spacing;
      frs->flags&=~FRSF_RESET;

      /* Compute totals and set minimum dimension */
      for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
      {  if(frs->flags&FRSF_ROWS)
         {  ch->mindim=ch->minh;
            d=ch->minw;
         }
         else
         {  ch->mindim=ch->minw;
            d=ch->minh;
         }
         ch->size=ch->rawsize;
         switch(ch->type)
         {  case FCHT_PIXELS:
               ch->size+=2*Agetattr(ch->object,AOFRM_Border);
               totpixel+=MAX(ch->size,ch->mindim);
               totpixsize+=ch->size;
               minpixel+=ch->mindim;
               break;
            case FCHT_PERCENT:
               minperc+=ch->mindim;
               totalprc+=ch->size;
               break;
            case FCHT_RELATIVE:
               minrel+=ch->mindim;
               totalrel+=ch->size;
               break;
         }
         ch->dim=0;
         if(d>minother) minother=d;
      }
      /* If total minimum greater or equal available space, give everything
       * its minimum size and we are done. */
      if(minpixel+minperc+minrel>=available)
      {  for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
         {  ch->dim=ch->mindim;
         }
      }
      else
      {  /* If not enough room for pixel sized plus minimum of others,
          * give others their minimum size and size pixel sized evenly
          * over available space. */
         if(totpixel && (totpixel+minperc+minrel>=available))
         {  available-=minperc+minrel;
            for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
            {  if(ch->type==FCHT_PIXELS)
               {  d=MAX(ch->size,ch->mindim);
                  ch->dim=available*d/totpixel;
                  available-=ch->dim;
                  totpixel-=d;
               }
               else
               {  ch->dim=ch->mindim;
               }
            }
         }
         else
         {  /* There is enough room for the pixel sized to give them their
             * wanted size.
             * If there are no percentage or relative sized, blow the pixel
             * sized up to fill the room, relative to their true wanted
             * size.
             */
            if(!totalprc && !totalrel)
            {  /* While there are childs that would get a size less than their
                * minimum, give them their minimum. */
               do
               {  done=TRUE;
                  for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
                  {  if(!ch->dim
                     && (available*ch->size/totpixsize<ch->mindim))
                     {  ch->dim=ch->mindim;
                        available-=ch->dim;
                        totpixsize-=ch->size;
                        done=FALSE;
                        break;
                     }
                  }
               } while(!done);

               /* Then blow up the others in the remaining room */
               for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
               {  if(!ch->dim)
                  {  ch->dim=ch->size*available/totpixsize;
                     available-=ch->dim;
                     totpixsize-=ch->size;
                  }
               }
            }
            else
            {  /* Give pixel sized their minimum or wanted size. */
               for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
               {  if(ch->type==FCHT_PIXELS)
                  {  ch->dim=MAX(ch->mindim,ch->size);
                     available-=ch->dim;
                  }
               }

               /* If there are relative sized childs, assign their total percentage
                * the remainder of the percentage. */
               if(totalrel)
               {  relprc=MAX(100-totalprc,0);
                  totalprc+=relprc;
               }

               /* While there are unsized percentage sized (including the combined
                * relative sized) that would get a size less than their minimum,
                * give them their minimum size. */
               if(totalrel && (available*relprc/totalprc<minrel))
               {  reldim=minrel;
                  available-=minrel;
                  totalprc-=relprc;
               }
               do
               {  done=TRUE;
                  for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
                  {  if(ch->type==FCHT_PERCENT && !ch->dim
                     && (available*ch->size/totalprc<ch->mindim))
                     {  ch->dim=ch->mindim;
                        available-=ch->mindim;
                        totalprc-=ch->size;
                        done=FALSE;
                        break;
                     }
                  }
               } while(!done);

               /* Now we know that all percentage sized (including combined relative
                * sized) can get their desired percentage. */
               if(totalrel && !reldim)
               {  reldim=available*relprc/totalprc;
                  available-=reldim;
                  totalprc-=relprc;
               }
               for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
               {  if(ch->type==FCHT_PERCENT && !ch->dim)
                  {  ch->dim=available*ch->size/totalprc;
                     available-=ch->dim;
                     totalprc-=ch->size;
                  }
               }

               /* Only the relative sized are left to do.
                * While there are unsized that would get a size less than their
                * minimum, give them their minimum size. */
               do
               {  done=TRUE;
                  for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
                  {  if(ch->type==FCHT_RELATIVE && !ch->dim
                     && (reldim*ch->size/totalrel<ch->mindim))
                     {  ch->dim=ch->mindim;
                        reldim-=ch->mindim;
                        totalrel-=ch->size;
                        done=FALSE;
                        break;
                     }
                  }
               } while(!done);

               /* Now there is enough room for the other relative sized. */
               for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
               {  if(ch->type==FCHT_RELATIVE && !ch->dim)
                  {  ch->dim=reldim*ch->size/totalrel;
                     reldim-=ch->dim;
                     totalrel-=ch->size;
                  }
               }
            }
         }
      }
   }

   /* Give all childs their resulting sizes, and compute the total
    * resulting size for this frameset. */
   if(frs->flags&FRSF_ROWS)
   {  frs->aow=MAX(minother,aml->width);
      frs->aoh=0;
   }
   else
   {  frs->aow=0;
      frs->aoh=MAX(minother,aml->height);
   }
   for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
   {  if(frs->flags&FRSF_ROWS)
      {  ch->width=frs->aow;
         ch->height=ch->dim;
         frs->aoh+=ch->height;
      }
      else
      {  ch->width=ch->dim;
         ch->height=frs->aoh;
         frs->aow+=ch->width;
      }
   }
   if(frs->flags&FRSF_ROWS)
   {  if(frs->aoh) frs->aoh+=(frs->nrchilds-1)*frs->spacing;
      else frs->aoh=aml->height;
   }
   else
   {  if(frs->aow) frs->aow+=(frs->nrchilds-1)*frs->spacing;
      else frs->aow=aml->width;
   }

   /* Now layout our childs and set their width and height. Then ALIGN them
    * to their frameset relative location. Our parent will MOVE or ALIGN us further. */
   frs->aox=frs->aoy=0;
   x=y=0;
   for(ch=Firstchild(frs);ch;ch=Nextchild(ch))
   {  Alayout(ch->object,ch->width,ch->height,AMLF_FORCE,NULL,0,NULL);
      Aalign(ch->object,x,y,0,0);
      ch->x=x;
      ch->y=y;
      if(frs->flags&FRSF_ROWS)
      {  y+=ch->height+frs->spacing;
      }
      else
      {  x+=ch->width+frs->spacing;
      }
   }
   return 0;
}

static long Alignframeset(struct Frameset *frs,struct Amalign *ama)
{  struct Ammove amm={{0}};
   frs->aox+=ama->dx;
   frs->aoy=ama->y;
   amm.amsg.method=AOM_MOVE;
   amm.dx=ama->dx;
   amm.dy=ama->y;
   Broadcast(frs,(struct Amessage *)&amm);
   return 0;
}

static long Moveframeset(struct Frameset *frs,struct Ammove *amm)
{  frs->aox+=amm->dx;
   frs->aoy+=amm->dy;
   Broadcast(frs,(struct Amessage *)amm);
   return 0;
}

static long Renderframeset(struct Frameset *frs,struct Amrender *amr)
{  struct Coords *coo;
   struct Amrender amrf=*amr;
   long x1,y1,x2,y2;

   if(!(frs->flags&FRSF_END)) return 0;

   if(amr->flags&AMRF_CLEAR)
   {  if((coo=Clipcoords(frs->frame,amr->coords)) && coo->rp)
      {  SetAPen(coo->rp,
            (frs->flags&FRSF_NOBACKGROUND)?
               Agetattr(Aweb(),AOAPP_Whitepen):coo->dri->dri_Pens[BACKGROUNDPEN]);
         x1=MAX(amr->rect.minx,frs->aox)+coo->dx;
         y1=MAX(amr->rect.miny,frs->aoy)+coo->dy;
         x2=MIN(amr->rect.maxx,frs->aox+frs->aow-1)+coo->dx;
         y2=MIN(amr->rect.maxy,frs->aoy+frs->aoh-1)+coo->dy;
         if(x1<=x2 && y1<=y2)
         {  RectFill(coo->rp,x1,y1,x2,y2);
         }
/*
         RectFill(coo->rp,frs->aox+coo->dx,frs->aoy+coo->dy,
            frs->aox+frs->aow+coo->dx-1,frs->aoy+frs->aoh+coo->dy-1);
*/
      }
      Unclipcoords(coo);
      amrf.flags&=~AMRF_CLEAR;
   }
   Broadcast(frs,(struct Amessage *)&amrf);
   return 0;
}

static long Setframeset(struct Frameset *frs,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   BOOL setframe=FALSE,setwin=FALSE,setwhis=FALSE,setjscancel=FALSE;
   struct Child *ch;
   UBYTE *rows=NULL,*cols=NULL;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Left:
            frs->aox=tag->ti_Data;
            break;
         case AOBJ_Top:
            frs->aoy=tag->ti_Data;
            break;
         case AOBJ_Width:
            frs->aow=tag->ti_Data;
            break;
         case AOBJ_Height:
            frs->aoh=tag->ti_Data;
            break;
         case AOBJ_Frame:
            frs->frame=(void *)tag->ti_Data;
            setframe=TRUE;
            break;
         case AOBJ_Window:
            if(!tag->ti_Data && frs->win)
            {  Asetattrs(frs->win,AOWIN_Goinactive,(Tag)frs,TAG_END);
            }
            frs->win=(void *)tag->ti_Data;
            setwin=TRUE;
            break;
         case AOBJ_Pool:
            frs->pool=(void *)tag->ti_Data;
            break;
         case AOFRS_Cols:
            cols=(UBYTE *)tag->ti_Data;
            break;
         case AOFRS_Rows:
            rows=(UBYTE *)tag->ti_Data;
            break;
         case AOFRM_Border:
            frs->border=tag->ti_Data;
            frs->flags|=FRSF_BORDERSET;
            break;
         case AOFRM_Defaultborder:
            if(!(frs->flags&FRSF_BORDERSET))
            {  frs->border=tag->ti_Data;
            }
            break;
         case AOFRS_Spacing:
            frs->spacing=tag->ti_Data;
            frs->flags|=FRSF_SPACINGSET;
            break;
         case AOFRS_Defaultspacing:
            if(!(frs->flags&FRSF_SPACINGSET))
            {  frs->spacing=tag->ti_Data;
            }
            break;
         case AOFRS_Endframeset:
            frs->flags|=FRSF_END;
            if(frs->subchild)
            {  Asetattrs(frs->subchild->object,AOFRS_Endframeset,TRUE,TAG_END);
            }
            break;
         case AOBJ_Winhis:
            frs->whis=(void *)tag->ti_Data;
            setwhis=TRUE;
            break;
         case AOBJ_Nobackground:
            SETFLAG(frs->flags,FRSF_NOBACKGROUND,tag->ti_Data);
            break;
         case AOBJ_Jscancel:
            setjscancel|=tag->ti_Data;
            break;
         case AOFRM_Prepreset:
            SETFLAG(frs->flags,FRSF_RESET,tag->ti_Data);
            break;
      }
   }
   if(setframe || setwin || setwhis || setjscancel)
   {  for(ch=frs->childs.first;ch->next;ch=ch->next)
      {  if(ch->object)
         {  Asetattrs(ch->object,
               setframe?AOBJ_Frame:TAG_IGNORE,(Tag)frs->frame,
               setwin?AOBJ_Window:TAG_IGNORE,(Tag)frs->win,
               setwhis?AOBJ_Winhis:TAG_IGNORE,(Tag)frs->whis,
               setjscancel?AOBJ_Jscancel:TAG_IGNORE,TRUE,
               TAG_END);
         }
      }
   }
   if(rows)
   {  frs->flags|=FRSF_ROWS;
      Parsespec(frs,rows);
      if(cols)
      {  frs->flags|=FRSF_BOTH;
         frs->subcols=Dupstr(cols,-1);
         frs->subnrcols=Nritemsinspec(cols);
         if(frs->subnrcols>1) frs->flags|=FRSF_SENSIBLE;
      }
   }
   else if(cols)
   {  frs->flags&=~FRSF_ROWS;
      Parsespec(frs,cols);
   }
   return 0;
}

static struct Frameset *Newframeset(struct Amset *ams)
{  struct Frameset *frs;
   if(frs=Allocobject(AOTP_FRAMESET,sizeof(struct Frameset),ams))
   {  NEWLIST(&frs->childs);
      frs->spacing=2;
      frs->border=2;
      Setframeset(frs,ams);
   }
   return frs;
}

static long Getframeset(struct Frameset *frs,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Left:
            PUTATTR(tag,frs->aox);
            break;
         case AOBJ_Top:
            PUTATTR(tag,frs->aoy);
            break;
         case AOBJ_Width:
            PUTATTR(tag,frs->aow);
            break;
         case AOBJ_Height:
            PUTATTR(tag,frs->aoh);
            break;
         case AOBJ_Frame:
            PUTATTR(tag,frs->frame);
            break;
         case AOFRM_Resizetop:
            PUTATTR(tag,(frs->flags&FRSF_ROWS)?
               Resizefirst(frs,tag->ti_Tag):Resizeall(frs,tag->ti_Tag));
            break;
         case AOFRM_Resizebottom:
            PUTATTR(tag,(frs->flags&FRSF_ROWS)?
               Resizelast(frs,tag->ti_Tag):Resizeall(frs,tag->ti_Tag));
            break;
         case AOFRM_Resizeleft:
            PUTATTR(tag,(frs->flags&FRSF_ROWS)?
               Resizeall(frs,tag->ti_Tag):Resizefirst(frs,tag->ti_Tag));
            break;
         case AOFRM_Resizeright:
            PUTATTR(tag,(frs->flags&FRSF_ROWS)?
               Resizeall(frs,tag->ti_Tag):Resizelast(frs,tag->ti_Tag));
            break;
         case AOFRM_Border:
            PUTATTR(tag,0);
            break;
         case AOFRS_Sensible:
            PUTATTR(tag,BOOLVAL(frs->flags&FRSF_SENSIBLE));
            break;
      }
   }
   return 0;
}

static long Notifyframeset(struct Frameset *frs,struct Amnotify *amn)
{  AmethodA((struct Aobject *)frs,amn->nmsg);
   Broadcast(frs,(struct Amessage *)amn);
   return 0;
}

static long Addchildframeset(struct Frameset *frs,struct Amadd *ama)
{  struct Child *ch;
   if(frs->flags&FRSF_BOTH)
   {  if(!frs->subleft)
      {  if(frs->subchild)
         {  Asetattrs(frs->subchild->object,AOFRS_Endframeset,TRUE,TAG_END);
         }
         for(ch=frs->childs.first;ch->next && ch->object;ch=ch->next);
         if(!ch->next)
         {  if(ch=PALLOCSTRUCT(Child,1,MEMF_CLEAR,frs->pool))
            {  ADDTAIL(&frs->childs,ch);
               ch->type=FCHT_RELATIVE;
               ch->rawsize=1;
            }
         }
         if(ch)
         {  ch->object=Anewobject(AOTP_FRAMESET,
               AOBJ_Pool,(Tag)frs->pool,
               AOFRS_Cols,(Tag)frs->subcols,
               AOFRM_Border,frs->border,
               AOFRS_Spacing,frs->spacing,
               AOBJ_Frame,(Tag)frs->frame,
               AOBJ_Window,(Tag)frs->win,
               AOBJ_Nobackground,BOOLVAL(frs->flags&FRSF_NOBACKGROUND),
               TAG_END);
            frs->nrchilds++;
            frs->subchild=ch;
            frs->subleft=frs->subnrcols;
         }
      }
      if(frs->subchild)
      {  AmethodA(frs->subchild->object,(struct Amessage *)ama);
         frs->subleft--;
      }
      else
      {  Adisposeobject(ama->child);
      }
   }
   else
   {  for(ch=frs->childs.first;ch->next && ch->object;ch=ch->next);
      if(!ch->next)
      {  if(ch=PALLOCSTRUCT(Child,1,MEMF_CLEAR,frs->pool))
         {  ADDTAIL(&frs->childs,ch);
            ch->type=FCHT_RELATIVE;
            ch->rawsize=1;
         }
      }
      if(ch)
      {  ch->object=ama->child;
         frs->nrchilds++;
         Asetattrs(ch->object,
            AOBJ_Frame,(Tag)frs->frame,
            AOBJ_Window,(Tag)frs->win,
            AOELT_Valign,VALIGN_TOP,
            AOFRM_Defaultborder,frs->border,
            AOFRS_Defaultspacing,frs->spacing,
            TAG_END);
      }
      else
      {  Adisposeobject(ama->child);
      }
   }
   return 0;
}

static long Hittestframeset(struct Frameset *frs,struct Amhittest *amh)
{  long result=0;
   struct Child *ch,*nextch;
   struct Coords *coo,coords={0};
   long x,y;
   UWORD ptrtype;
   if(!(coo=amh->coords))
   {  Framecoords(frs->frame,&coords);
      coo=&coords;
   }
   x=amh->xco-frs->aox-coo->dx;
   y=amh->yco-frs->aoy-coo->dy;
   for(ch=Firstchild(frs);ch;ch=nextch)
   {  nextch=Nextchild(ch);
      if(nextch && (ptrtype=Resizehit(frs,x,y,ch,nextch)))
      {  if(amh->oldobject==(struct Aobject *)frs)
         {  result=AMHR_OLDHIT;
         }
         else
         {  result=AMHR_NEWHIT;
            if(amh->amhr)
            {  amh->amhr->object=frs;
               amh->amhr->text=Dupstr(AWEBSTR(MSG_AWEB_FRAME_RESIZE),-1);
               amh->amhr->ptrtype=ptrtype;
            }
         }
      }
      else if(x>=ch->x && x<ch->x+ch->width && y>=ch->y && y<ch->y+ch->height)
      {  result=AmethodA(ch->object,(struct Amessage *)amh);
         break;
      }
   }
   return result;
}

static long Goactiveframeset(struct Frameset *frs,struct Amgoactive *amg)
{  long result=AMR_NOREUSE;
   struct Coords *coo;
   long x,y;
   struct Child *ch,*nextch;
   if(amg->imsg && (coo=Clipcoords(frs->frame,NULL)))
   {  x=amg->imsg->MouseX-frs->aox-coo->dx;
      y=amg->imsg->MouseY-frs->aoy-coo->dy;
      for(ch=Firstchild(frs);ch;ch=nextch)
      {  nextch=Nextchild(ch);
         if(nextch && Resizehit(frs,x,y,ch,nextch))
         {  if(frs->flags&FRSF_ROWS)
            {  frs->cursize=(ch->y+ch->height+nextch->y)/2;
               frs->minsize=frs->cursize-ch->height+ch->minh;
               frs->maxsize=frs->cursize+nextch->height-nextch->minh;
               frs->grab=frs->cursize-y;
            }
            else
            {  frs->cursize=(ch->x+ch->width+nextch->x)/2;
               frs->minsize=frs->cursize-ch->width+ch->minw;
               frs->maxsize=frs->cursize+nextch->width-nextch->minw;
               frs->grab=frs->cursize-x;
            }
            frs->sizechild=ch;
            result=AMR_ACTIVE;
            Rendersize(frs,coo);
            break;
         }
      }
      Unclipcoords(coo);
   }
   return result;
}

static long Handleinputframeset(struct Frameset *frs,struct Aminput *ami)
{  long result=AMR_NOCARE;
   struct Coords *coo;
   long newc;
   if(ami->imsg)
   {  switch(ami->imsg->Class)
      {  case IDCMP_MOUSEMOVE:
            coo=Clipcoords(frs->frame,NULL);
            if(frs->flags&FRSF_ROWS)
            {  newc=(ami->imsg->MouseY-frs->aoy-coo->dy)-frs->grab;
            }
            else
            {  newc=(ami->imsg->MouseX-frs->aox-coo->dx)-frs->grab;
            }
            if(newc<frs->minsize) newc=frs->minsize;
            if(newc>frs->maxsize) newc=frs->maxsize;
            if(newc!=frs->cursize)
            {  Rendersize(frs,coo);
               frs->cursize=newc;
               Rendersize(frs,coo);
            }
            Unclipcoords(coo);
            result=AMR_ACTIVE;
            break;
         case IDCMP_MOUSEBUTTONS:
            if(ami->imsg->Code==SELECTUP)
            {  Doresizechilds(frs);
               result=AMR_NOREUSE;
            }
            break;
      }
   }
   return result;
}

static long Goinactiveframeset(struct Frameset *frs)
{  if(frs->sizechild)
   {  Rendersize(frs,NULL);
      frs->sizechild=0;
   }
   return 0;
}

static long Dragtestframeset(struct Frameset *frs,struct Amdragtest *amd)
{  long result=0;
   struct Child *ch;
   for(ch=frs->childs.first;ch->next && !result;ch=ch->next)
   {  if(ch->object)
      {  result=AmethodA(ch->object,(struct Amessage *)amd);
      }
   }
   return result;
}

static long Dragrenderframeset(struct Frameset *frs,struct Amdragrender *amd)
{  struct Child *ch;
   for(ch=frs->childs.first;ch->next && amd->state!=AMDS_DONE;ch=ch->next)
   {  if(ch->object)
      {  AmethodA(ch->object,(struct Amessage *)amd);
      }
   }
   return 0;
}

static long Dragcopyframeset(struct Frameset *frs,struct Amdragcopy *amd)
{  struct Child *ch;
   for(ch=frs->childs.first;ch->next && amd->state!=AMDS_DONE;ch=ch->next)
   {  if(ch->object)
      {  AmethodA(ch->object,(struct Amessage *)amd);
      }
   }
   return 0;
}

static long Jsetupframeset(struct Frameset *frs,struct Amjsetup *amj)
{  Broadcast(frs,(struct Amessage *)amj);
   return 0;
}

static void Disposeframeset(struct Frameset *frs)
{  struct Child *ch;
   while(ch=(struct Child *)REMHEAD(&frs->childs))
   {  if(ch->object) Adisposeobject(ch->object);
      FREE(ch);
   }
   if(frs->subcols) FREE(frs->subcols);
   if(frs->win) Asetattrs(frs->win,AOWIN_Goinactive,(Tag)frs,TAG_END);
   Amethodas(AOTP_OBJECT,frs,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Frameset_Dispatcher,
struct Frameset *,frs,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newframeset((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setframeset(frs,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getframeset(frs,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measureframeset(frs,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutframeset(frs,(struct Amlayout *)amsg);
         break;
      case AOM_ALIGN:
         result=Alignframeset(frs,(struct Amalign *)amsg);
         break;
      case AOM_MOVE:
         result=Moveframeset(frs,(struct Ammove *)amsg);
         break;
      case AOM_RENDER:
         result=Renderframeset(frs,(struct Amrender *)amsg);
         break;
      case AOM_NOTIFY:
         result=Notifyframeset(frs,(struct Amnotify *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchildframeset(frs,(struct Amadd *)amsg);
         break;
      case AOM_HITTEST:
         result=Hittestframeset(frs,(struct Amhittest *)amsg);
         break;
      case AOM_GOACTIVE:
         result=Goactiveframeset(frs,(struct Amgoactive *)amsg);
         break;
      case AOM_HANDLEINPUT:
         result=Handleinputframeset(frs,(struct Aminput *)amsg);
         break;
      case AOM_GOINACTIVE:
         result=Goinactiveframeset(frs);
         break;
      case AOM_DRAGTEST:
         result=Dragtestframeset(frs,(struct Amdragtest *)amsg);
         break;
      case AOM_DRAGRENDER:
         result=Dragrenderframeset(frs,(struct Amdragrender *)amsg);
         break;
      case AOM_DRAGCOPY:
         result=Dragcopyframeset(frs,(struct Amdragcopy *)amsg);
         break;
      case AOM_JSETUP:
         result=Jsetupframeset(frs,(struct Amjsetup *)amsg);
         break;
      case AOM_DISPOSE:
         Disposeframeset(frs);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installframeset(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_FRAMESET,(Tag)Frameset_Dispatcher)) return FALSE;
   return TRUE;
}
