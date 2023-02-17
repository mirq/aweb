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

/* text.c - AWeb html document text element object */

#include "aweb.h"
#include "text.h"
#include "link.h"
#include "application.h"

#include <graphics/gfxmacros.h>

#include <proto/graphics.h>
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

struct Text
{  struct Element elt;
   void *pool;
   struct Tsection *sections,*lastsection;
   long height;            /* Text (font) height */
   struct Colorinfo *color;
   struct TextFont *supfont;
   long histart,hilength;  /* Highlighted section. */
   UWORD flags;
   struct Buffer *text;
};

#define TXTF_BLINK      0x0001   /* Blinking text */
#define TXTF_BLINKING   0x0002   /* Is child in WANT_BLINK relationship */
#define TXTF_BLINKON    0x0004   /* Visible blink phase (or not blinking) */

/* A Tsection is a section of this Text's text. Text elemens can be split
 * up into several sections when they don't fit */
struct Tsection
{  struct Tsection *next;  /* Single-linked list to save memory */
   long x,y,w,h;           /* Hitbox of section */
   long textpos,length;    /* Displayed text */
   UWORD flags;
};

#define TSF_ALIGNED     0x0001   /* This section is aligned */
#define TSF_SPACE       0x0002   /* Section ends in space, not counted in length */
#define TSF_CONT        0x0004   /* This section continues on from the previous with no line break */

#define SHY             0xad     /* Soft hyphen */
#define NBSP            0xa0     /* Non Breaking Space */
/*------------------------------------------------------------------------*/

/* Clear all sections, or only non-aligned sections */
static void Clearsections(struct Text *tx,BOOL all)
{  struct Tsection *ts,*next;
   tx->lastsection=NULL;
   for(ts=tx->sections;ts;ts=next)
   {  next=ts->next;
      if(!(ts->flags&TSF_ALIGNED)) all=TRUE;
      if(all) FREE(ts);
      else tx->lastsection=ts;
   }
   if(tx->lastsection) tx->lastsection->next=NULL;
   else tx->sections=NULL;
}

/* Create a new section */
static struct Tsection *Newsection(struct Text *tx)
{  struct Tsection *ts=PALLOCSTRUCT(Tsection,1,MEMF_CLEAR,tx->pool);
   if(ts)
   {  if(tx->lastsection) tx->lastsection->next=ts;
      else tx->sections=ts;
      tx->lastsection=ts;
   }
   return ts;
}

/* Return next text position after last section */
static long Nextpos(struct Text *tx)
{  struct Tsection *ts=tx->lastsection;
   long pos;
   if(ts)
   {  pos=ts->textpos+ts->length;
      if(ts->flags&TSF_SPACE) pos++;
   }
   else pos=tx->elt.textpos;
   return pos;
}

/* Check if point falls within text sections */
static BOOL Pointintext(struct Text *tx,long x,long y)
{  struct Tsection *ts;
   for(ts=tx->sections;ts;ts=ts->next)
   {  if((ts->flags&TSF_ALIGNED)
       && x>=ts->x && x<ts->x+ts->w && y>=ts->y && y<ts->y+ts->h)
      {  return TRUE;
      }
   }
   return FALSE;
}

/* Only complement this section (start,end). Pass a valid and clipped Coords. */
static void Highlighttext(struct Text *tx,struct Coords *coo,long start,long end)
{  struct Tsection *ts;
   long histart,hiend;  /* Pixels */
   SetFont(coo->rp,tx->elt.font);
   SetSoftStyle(coo->rp,0,0x0f);
   SetDrMd(coo->rp,COMPLEMENT);
   for(ts=tx->sections;ts;ts=ts->next)
   {  if(ts->flags&TSF_ALIGNED)
      {  if(start<ts->textpos+ts->length && end>=ts->textpos)
         {  if(start<=ts->textpos)
            {  histart=0;
            }
            else
            {  histart=Textlength(coo->rp,tx->text->buffer+ts->textpos,start-ts->textpos);
            }
            if(end+1>=ts->textpos+ts->length)
            {  hiend=ts->w;
            }
            else
            {  hiend=Textlength(coo->rp,tx->text->buffer+ts->textpos,end+1-ts->textpos);
            }
            RectFill(coo->rp,
               ts->x+histart+coo->dx,ts->y+coo->dy,
               ts->x+hiend-1+coo->dx,ts->y+tx->height-1+coo->dy);
         }
      }
   }
   SetDrMd(coo->rp,JAM1);
}

/* If text is followed by other text with the same style, use the smaller width,
 * else use the larger realw */
static long Realwidth(struct Text *tx,long width,long realw)
{  struct Element *next=tx->elt.object.next;
   if(next->object.next && next->object.objecttype==AOTP_TEXT
   && (next->style&(FSF_BOLD|FSF_ITALIC))==(tx->elt.style&(FSF_BOLD|FSF_ITALIC)))
   {  realw=width;
   }
   return realw;
}

/*------------------------------------------------------------------------*/

static long Measuretext(struct Text *tx,struct Ammeasure *amm)
{  long w,width,realw;
   UBYTE *p,*q,*end;
   SetFont(mrp,tx->elt.font);
   SetSoftStyle(mrp,tx->elt.style,0x0f);
   p=amm->text->buffer+tx->elt.textpos;
   width=Textlengthext(mrp,p,tx->elt.length,&realw);
   if(realw!=width) width=Realwidth(tx,width,realw);
   /* Set aow to ensure correct alignment of text bullets: */
   tx->elt.aow=width;
   tx->height=tx->elt.font->tf_YSize;
   if(amm->ammr)
   {  amm->ammr->width=width;
      /* Compute minimum width = width of longest word */
      if(amm->flags&AMMF_NOWRAP)
      {  amm->ammr->minwidth=amm->addwidth+width;
         amm->ammr->addwidth=amm->ammr->minwidth;
      }
      else if(tx->elt.eltflags&ELTF_NOBR)
      {  amm->ammr->minwidth=amm->addwidth+width;
         amm->ammr->addwidth=amm->ammr->minwidth;
      }
      else
      {  end=p+tx->elt.length;
         w=amm->addwidth;
         while(p<end)
         {  for(q=p;q<end && *q!=' ' && *q!=SHY;q++);
            width=Textlengthext(mrp,p,q-p+((q<end && *q==SHY)?1:0),&realw);
            if(width!=realw && q>=end) width=Realwidth(tx,width,realw);
            w+=width;
            if(w>amm->ammr->minwidth) amm->ammr->minwidth=w;
            if(q>=end) amm->ammr->addwidth=w;
            else amm->ammr->addwidth=0;
            p=q+1;
            w=0;
         }
      }
   }
   return 0;
}

static long Layouttext(struct Text *tx,struct Amlayout *aml)
{  short result;
   long pos,width,length,realw;
   struct TextExtent te;
   UBYTE *p,*q;
   struct Tsection *ts,*tsa;
   BOOL space=FALSE;
   BOOL cont=FALSE;
   if(aml->flags&AMLF_BREAK)
   {  /* Shorten last section to the break position.
       * If no break found, remove the section completely. */
      ts=tx->lastsection;
      if(ts)
      {  p=aml->text->buffer+ts->textpos;
         q=p+ts->length-1;
         if(ts->flags&TSF_SPACE) q++;
         /* Now q points to the last character in this section.
          * Search for the last space or soft hyphen. */
         while(q>p && *q!=' ' && *q!=SHY) q--;
         if(*q==' ' && !(tx->elt.eltflags&ELTF_NOBR))
         {  /* Space found. Shorten this section, but include the space. */
            ts->flags|=TSF_SPACE;  /* Don't include space in length */
            ts->length=q-p;
            SetFont(mrp,tx->elt.font);
            SetSoftStyle(mrp,tx->elt.style,0x0f);
            ts->w=Textlength(mrp,p,ts->length);
            /* See if this section (plus the trailing space) fits */
            if(ts->textpos+ts->length+1<tx->elt.textpos+tx->elt.length) result=AMLR_MORE;
            else result=AMLR_OK;
            if(aml->amlr) aml->amlr->endx=ts->x+ts->w;
         }
         else if(*q==SHY && !(tx->elt.eltflags&ELTF_NOBR))
         {  /* Soft hyphen found. Shorten this section and include the SHY. */
            ts->length=q-p+1;
            SetFont(mrp,tx->elt.font);
            SetSoftStyle(mrp,tx->elt.style,0x0f);
            ts->w=Textlength(mrp,p,ts->length);
            /* See if this section fits */
            if(ts->textpos+ts->length<tx->elt.textpos+tx->elt.length) result=AMLR_MORE;
            else result=AMLR_OK;
            if(aml->amlr) aml->amlr->endx=ts->x+ts->w;
         }
         else
         {  /* No break found. Remove section. */
            FREE(ts);
            if(tx->sections==ts)
            {  tx->sections=tx->lastsection=NULL;
            }
            else
            {  for(tsa=tx->sections;tsa->next!=ts;tsa=tsa->next);
               tsa->next=NULL;
               tx->lastsection=tsa;
            }
            result=AMLR_BREAK;
         }
      }
      else result=AMLR_BREAK;
   }
   else
   {  /* First set endx, may be overwritten later. */
      if(aml->amlr) aml->amlr->endx=aml->startx;
      /* If not MORE, it's a new layout. Clear all sections.
       * If RETRY, clear only non-aligned sections. */
      if(!(aml->flags&AMLF_MORE)) Clearsections(tx,TRUE);
      else if(aml->flags&AMLF_RETRY) Clearsections(tx,FALSE);
      /* Find the current starting position */
      pos=Nextpos(tx);
      length=tx->elt.length-(pos-tx->elt.textpos);
      p=aml->text->buffer+pos;
      if(length>0)
      {  SetFont(mrp,tx->elt.font);
         SetSoftStyle(mrp,tx->elt.style,0x0f);
         width=Textlengthext(mrp,p,length,&realw);
         if(realw!=width) width=Realwidth(tx,width,realw);
      }
      else
      {  width=0;
      }
      tx->height=tx->elt.font->tf_YSize;
      if(aml->startx+width<=aml->width)
      {  /* Everything fits */
         result=AMLR_OK;
      }
      else if(tx->elt.eltflags&ELTF_NOBR)
      {  /* Text doesn't fit, but don't line break. */
         if(aml->amlr)
         {  aml->amlr->endx=aml->startx+width;
         }
         if(aml->flags&AMLF_FORCE)
         {
             result=AMLR_FBREAK;
         }
         else
         {  result=AMLR_BREAK;
            length=0;   /* Prevent creation of section. */
         }
      }
      else
      {  /* Text doesn't fit. Try to find a break position within target width. */
         length=TextFit(mrp,p,length,&te,NULL,1,aml->width-aml->startx,tx->height);
         q=p+length;
         /* now q points to first character not to fit.
          * Search for last space or soft hyphen. Accept a space in the last character
          * but not a SHY. */
         if(q>p && *q!=' ') q--; /* Skip over any SHY at the end */
         while(q>p && *q!=' ' && *q!=SHY) q--;
         if(*q==' ')
         {  /* Space found. Break the text after the space but don't include
             * the space in the section. If the space found is the last character,
             * then we fit after all. */
            if(q==aml->text->buffer+pos+tx->elt.length-1)
            {  result=AMLR_OK;
            }
            else
            {  result=AMLR_MORE;
            }
            length=q-p;
            width=Textlength(mrp,p,length);
            space=TRUE;
         }
         else if(*q==SHY)
         {  /* Soft hyphen found (not the last character). Break the text after the SHY. */
            result=AMLR_MORE;
            length=q-p+1;
            width=Textlength(mrp,p,length);
            space=FALSE;
         }
         else
         {  /* No space or SHY found. Find a break position after the target width. */
            UBYTE *r=aml->text->buffer+tx->elt.textpos+tx->elt.length-1; /* r = last char of text */
            q=p+length;
            while(q<r && *q!=' ' && *q!=SHY) q++;
            /* Now q points either to a space, to a SHY before the last character,
             * or to the last character. Break here */
            if(*q==' ') space=TRUE;
            else q++;   /* include SHY or last character */

            width=Textlength(mrp,p,q - p);
            if(width > 6000)
            {
                /* Width is unacceptable (beyond system limits) , have to break earlier */
                struct TextExtent te = {0};
                long part;
                part = TextFit(mrp,p,strlen(p),&te,NULL,1,6000,6000);
                q = p + part;
                width=Textlength(mrp,p,q - p);

                cont = TRUE;
            }
            length=q-p;

            /* If FORCE, use this text. Else set the resulting endx to show our needs. */
            if(aml->flags&AMLF_FORCE)
            {  if(q<=r)
               {
                   if(cont /* &&  tx->lastsection && tx->lastsection->flags & TSF_CONT */) result = AMLR_CONT;
                   else result=AMLR_MORE;
               }
               else
               {
                   result=AMLR_FBREAK;
               }
            }
            else
            {  length=0;
               space=FALSE;
               result=AMLR_BREAK;
               if(aml->amlr)
               {  aml->amlr->endx=aml->startx+width;
               }
            }
         }
      }
      /* Now create a section for the fitting text. */
      if(length>0 || space)
      {
         struct Tsection *ls = tx->lastsection;
         if(ts=Newsection(tx))
         {
            if(ls && ls->flags & TSF_CONT)
            {
                ts->x=ls->x + ls->w;
                if(!cont)
                result = AMLR_FBREAK;
            }
            else
            {
                ts->x=aml->startx;
            }
            /* ts->y will be set when aligned */
            ts->w=width;
            ts->h=tx->height;
            ts->textpos=p-aml->text->buffer;
            ts->length=length;
            if(space) ts->flags|=TSF_SPACE;
            if(cont) ts->flags |= TSF_CONT;
            if(aml->amlr)
            {  aml->amlr->endx=ts->x+ts->w;
               switch(tx->elt.valign)
               {  case VALIGN_SUB:  /* align middle of font with baseline */
                     aml->amlr->above=tx->elt.font->tf_YSize/2;
                     aml->amlr->below=tx->elt.font->tf_YSize-aml->amlr->above;
                     break;
                  case VALIGN_SUP:  /* align 1/3 of font with top of supfont */
                     if(tx->supfont)
                     {  aml->amlr->above=tx->supfont->tf_Baseline +
                           tx->elt.font->tf_YSize/3;
                        aml->amlr->below=tx->supfont->tf_YSize-aml->amlr->above;
                        break;
                     }
                     /* else fall through: */
                  default:
                     aml->amlr->above=tx->elt.font->tf_Baseline+1;
                     aml->amlr->below=tx->elt.font->tf_YSize-aml->amlr->above;
               }
            }
         }
         else
         {  /* No more memory, avoid loop (document is shown distorted anyway) */
            result=AMLR_OK;
         }
      }
   }
   if(aml->amlr) aml->amlr->result=result;
   return 0;
}
#if 0
static long Aligntext(struct Text *tx,struct Amalign *ama)
{  struct Tsection *ts;
   /* Align the last section (previous sections should have been aligned already) */
   ts=tx->lastsection;
   if(ts && !(ts->flags&TSF_ALIGNED))
   {  ts->x+=ama->dx;
      switch(tx->elt.valign)
      {  case VALIGN_SUB:
            ts->y=ama->y+ama->baseline-tx->elt.font->tf_Baseline/2;
            break;
         case VALIGN_SUP:
            if(tx->supfont)
            {  ts->y=ama->y+ama->baseline-tx->supfont->tf_Baseline -
                  tx->elt.font->tf_YSize/3;
               break;
            }
            /* else fall through: */
         default:
            ts->y=ama->y+ama->baseline-tx->elt.font->tf_Baseline;
      }
      ts->flags|=TSF_ALIGNED;
      /* Make sure tx->aox/y/w/h is outer box */
      if(ts==tx->sections)
      {  /* first section */
         tx->elt.aox=ts->x;
         tx->elt.aoy=ts->y;
         tx->elt.aow=ts->w;
         tx->elt.aoh=ts->h;
      }
      else
      {  if(ts->x<tx->elt.aox)
         {  tx->elt.aow+=tx->elt.aox-ts->x;
            tx->elt.aox=ts->x;
         }
         if(ts->y<tx->elt.aoy)
         {  tx->elt.aoh+=tx->elt.aoy-ts->y;
            tx->elt.aoy=ts->y;
         }
         if(ts->x+ts->w>tx->elt.aox+tx->elt.aow) tx->elt.aow=ts->x+ts->w-tx->elt.aox;
         if(ts->y+ts->h>tx->elt.aoy+tx->elt.aoh) tx->elt.aoh=ts->y+ts->h-tx->elt.aoy;
      }
   }
   return 0;
}
#endif

static long Aligntext(struct Text *tx,struct Amalign *ama)
{  struct Tsection *ts;
   /* Align any section not yet aligned */
   for(ts=tx->sections;ts;ts=ts->next)
   {
       if(!(ts->flags&TSF_ALIGNED))
       {  ts->x+=ama->dx;
          switch(tx->elt.valign)
          {  case VALIGN_SUB:
                ts->y=ama->y+ama->baseline-tx->elt.font->tf_Baseline/2;
                break;
             case VALIGN_SUP:
                if(tx->supfont)
                {  ts->y=ama->y+ama->baseline-tx->supfont->tf_Baseline -
                      tx->elt.font->tf_YSize/3;
                   break;
                }
                /* else fall through: */
             default:
                ts->y=ama->y+ama->baseline-tx->elt.font->tf_Baseline;
          }
          ts->flags|=TSF_ALIGNED;
          /* Make sure tx->aox/y/w/h is outer box */
          if(ts==tx->sections)
          {  /* first section */
             tx->elt.aox=ts->x;
             tx->elt.aoy=ts->y;
             tx->elt.aow=ts->w;
             tx->elt.aoh=ts->h;
          }
          else
          {  if(ts->x<tx->elt.aox)
             {  tx->elt.aow+=tx->elt.aox-ts->x;
                tx->elt.aox=ts->x;
             }
             if(ts->y<tx->elt.aoy)
             {  tx->elt.aoh+=tx->elt.aoy-ts->y;
                tx->elt.aoy=ts->y;
             }
             if(ts->x+ts->w>tx->elt.aox+tx->elt.aow) tx->elt.aow=ts->x+ts->w-tx->elt.aox;
             if(ts->y+ts->h>tx->elt.aoy+tx->elt.aoh) tx->elt.aoh=ts->y+ts->h-tx->elt.aoy;
          }
       }
   }
   return 0;
}

static long Rendertext(struct Text *tx,struct Amrender *amr)
{  struct Coords *coo,coords={0};
   BOOL clip=FALSE;
   ULONG clipkey=0;
   struct RastPort *rp;
   struct Tsection *ts;
   short pen;
   UWORD pattern=0;
   BOOL clearpattern=FALSE,highlight,blinkoff;
   long histart=0,hiend=0;
   if(!(coo=amr->coords))
   {  Framecoords(tx->elt.cframe,&coords);
      coo=&coords;
      clip=TRUE;
   }
   if(coo->rp)
   {  rp=coo->rp;
      if(clip) clipkey=Clipto(rp,coo->minx,coo->miny,coo->maxx,coo->maxy);
      SetFont(rp,tx->elt.font);
      SetSoftStyle(rp,tx->elt.style,0x0f);
      if(tx->elt.link)
      {  if(Agetattr(tx->elt.link,AOLNK_Visited))
         {  pen=coo->vlinkcolor;
            if(prefs.browser.ullink)
            {  pattern=0xcccc;
               clearpattern=TRUE;
            }
         }
         else
         {  pen=coo->linkcolor;
            if(prefs.browser.ullink) pattern=0xffff;
         }
         if(Agetattr(tx->elt.link,AOLNK_Selected))
         {  pen=coo->alinkcolor;
         }
      }
      else pen=coo->textcolor;
      if(prefs.browser.docolors && tx->color && tx->color->pen>=0) pen=tx->color->pen;
      SetAPen(rp,pen);
      blinkoff=(tx->flags&TXTF_BLINKING) && !(tx->flags&TXTF_BLINKON);
      for(ts=tx->sections;ts;ts=ts->next)
      {  if(ts->flags&TSF_ALIGNED)
         {
            if((ts->x <= amr->rect.maxx) && (ts->x + ts->w > amr->rect.minx) &&
               (ts->y <= amr->rect.maxy) && (ts->y + ts->h > amr->rect.miny))
            {
                highlight=FALSE;
                if(tx->hilength
                && tx->histart<ts->textpos+ts->length && tx->histart+tx->hilength>ts->textpos)
                {  if(tx->histart<=ts->textpos)
                   {  histart=0;
                   }
                   else
                   {  histart=Textlength(rp,tx->text->buffer+ts->textpos,tx->histart-ts->textpos);
                   }
                   if(tx->histart+tx->hilength>=ts->textpos+ts->length)
                   {  hiend=ts->w;
                   }
                   else
                   {  hiend=Textlength(rp,tx->text->buffer+ts->textpos,
                         tx->histart+tx->hilength-ts->textpos);
                   }
                   highlight=TRUE;
                }
                /* Clear background before highlighting */
                if(highlight && !(amr->flags&AMRF_CLEAR))
                {  Erasebg(tx->elt.cframe,coo,ts->x+histart,ts->y,ts->x+hiend-1,ts->y+tx->height-1);
                }
                /* Clear background before dashed underlining */
                if(clearpattern)
                {  Erasebg(tx->elt.cframe,coo,ts->x,ts->y+tx->height-1,ts->x+ts->w-1,ts->y+tx->height-1);
                }
                /* Clear background in blink off phase or draghighlight */
                if(blinkoff || (amr->flags&AMRF_CLEAR))
                {  Erasebg(tx->elt.cframe,coo,ts->x,ts->y,ts->x+ts->w-1,ts->y+tx->height-1);
                }
                if(!blinkoff)
                {
                   Move(rp,ts->x+coo->dx,ts->y+tx->elt.font->tf_Baseline+coo->dy);
                   Text(rp,tx->text->buffer+ts->textpos,ts->length);
                   if(tx->elt.style&FSF_STRIKE)
                   {  Move(rp,ts->x+coo->dx,ts->y+tx->elt.font->tf_YSize/2+coo->dy);
                      Draw(rp,ts->x+ts->w+coo->dx-1,ts->y+tx->elt.font->tf_YSize/2+coo->dy);
                   }
                   if(pattern)
                   {  SetDrPt(rp,pattern);
                      Move(rp,ts->x+coo->dx,ts->y+coo->dy+tx->height-1);
                      Draw(rp,ts->x+coo->dx+ts->w-1,ts->y+coo->dy+tx->height-1);
                      SetDrPt(rp,0xffff);
                   }
                }
                if(highlight && !(amr->flags&AMRF_CLEARHL))
                {  SetDrMd(rp,COMPLEMENT);
                   RectFill(rp,
                      ts->x+histart+coo->dx,ts->y+coo->dy,
                      ts->x+hiend-1+coo->dx,ts->y+tx->height-1+coo->dy);
                   SetDrMd(rp,JAM1);
                }
            }
         }
      }
      if(clip) Unclipto(clipkey);
   }
   return 0;
}

static long Settext(struct Text *tx,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=Amethodas(AOTP_ELEMENT,tx,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Pool:
            tx->pool=(void *)tag->ti_Data;
            break;
         case AOELT_Color:
            tx->color=(struct Colorinfo *)tag->ti_Data;
            break;
         case AOELT_Supfont:
            tx->supfont=(struct TextFont *)tag->ti_Data;
            break;
         case AOELT_Resetlayout:
            Clearsections(tx,TRUE);
            break;
         case AOBJ_Cframe:
            if(tag->ti_Data)
            {  if(tx->flags&TXTF_BLINK)
               {  Aaddchild(Aweb(),(struct Aobject *)tx,AOREL_APP_WANT_BLINK);
                  tx->flags|=TXTF_BLINKING;
               }
            }
            else
            {  /* Clear highlighting if becoming undisplayed */
               tx->hilength=0;
               if(tx->flags&TXTF_BLINKING)
               {  Aremchild(Aweb(),(struct Aobject *)tx,AOREL_APP_WANT_BLINK);
                  tx->flags&=~TXTF_BLINKING;
               }
            }
            break;
         case AOTXT_Blink:
            SETFLAG(tx->flags,TXTF_BLINK,tag->ti_Data);
            break;
         case AOTXT_Text:
            tx->text=(struct Buffer *)tag->ti_Data;
            break;
         case AOAPP_Blink:
            SETFLAG(tx->flags,TXTF_BLINKON,tag->ti_Data);
            Arender((struct Aobject *)tx,NULL,0,0,AMRMAX,AMRMAX,0,tx->text);
            break;
      }
   }
   return result;
}

static struct Text *Newtext(struct Amset *ams)
{  struct Text *tx;
   if(tx=Allocobject(AOTP_TEXT,sizeof(struct Text),ams))
   {  tx->flags|=TXTF_BLINKON;
      Settext(tx,ams);
   }
   return tx;
}

static long Gettext(struct Text *tx,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=AmethodasA(AOTP_ELEMENT,(struct Aobject *)tx,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Clipdrag:
            PUTATTR(tag,TRUE);
            break;
      }
   }
   return result;
}

static long Hittesttext(struct Text *tx,struct Amhittest *amh)
{  long result=0;
   struct Coords *coo,coords={0};
   if(!(coo=amh->coords))
   {  Framecoords(tx->elt.cframe,&coords);
      coo=&coords;
   }
   if(Pointintext(tx,amh->xco-coo->dx,amh->yco-coo->dy))
   {  result=AmethodasA(AOTP_ELEMENT,(struct Aobject *)tx,(struct Amessage *)amh);
   }
   return result;
}

static long Movetext(struct Text *tx,struct Ammove *amm)
{  struct Tsection *ts;
   for(ts=tx->sections;ts;ts=ts->next)
   {  ts->x+=amm->dx;
      ts->y+=amm->dy;
   }
   AmethodasA(AOTP_ELEMENT,(struct Aobject *)tx,(struct Amessage *)amm);
   return 0;
}

static long Searchpostext(struct Text *tx,struct Amsearch *ams)
{  struct Tsection *ts;
   for(ts=tx->sections;ts;ts=ts->next)
   {  if(ams->top<ts->y+ts->h)
      {  ams->pos=ts->textpos;
         return 1;
      }
   }
   return 0;
}

static long Searchsettext(struct Text *tx,struct Amsearch *ams)
{  struct Tsection *ts;
   if(ams->pos<tx->elt.textpos+tx->elt.length)
   {  for(ts=tx->sections;ts;ts=ts->next)
      {  if(ams->pos<ts->textpos+ts->length)
         {  /* Fill in top if it isn't yet set. Break out if no highlight
             * or if highlight ends in this text. Otherwise continue
             * and let successors know the highlight. */
            if(ams->top<0)
            {  ams->top=ts->y;
               if(ams->pos<=ts->textpos)
               {  ams->left=ts->x;
               }
               else
               {  SetFont(mrp,tx->elt.font);
                  SetSoftStyle(mrp,tx->elt.style,0x0f);
                  ams->left=ts->x+
                     Textlength(mrp,ams->text->buffer+ts->textpos,ams->pos-ts->textpos);
               }
            }
            if(ams->flags&AMSF_HIGHLIGHT)
            {  tx->histart=ams->pos;
               tx->hilength=ams->length;
               Arender((struct Aobject *)tx,NULL,0,0,AMRMAX,AMRMAX,0,ams->text);
               if(ams->pos+ams->length<=tx->elt.textpos+tx->elt.length) return 1;
               else break;
            }
            if(ams->flags&AMSF_UNHIGHLIGHT)
            {  Arender((struct Aobject *)tx,NULL,0,0,AMRMAX,AMRMAX,AMRF_CLEARHL,ams->text);
               tx->hilength=0;
               if(ams->pos+ams->length<=tx->elt.textpos+tx->elt.length) return 1;
               else break;
            }
            return 1;
         }
      }
   }
   return 0;
}

static long Dragtesttext(struct Text *tx,struct Amdragtest *amd)
{  struct Tsection *ts;
   struct TextExtent te={0};
   long result=0;
   long x,y;
   x=amd->xco-amd->coords->dx;
   y=amd->yco-amd->coords->dy;
   for(ts=tx->sections;ts;ts=ts->next)
   {  if((ts->y<=y && ts->y+ts->h>y && ts->x+ts->w>x) || ts->y>y)
      {  /* Matching section */
         result=AMDR_HIT;
         if(amd->amdr)
         {  amd->amdr->object=tx;
            /* Object position is buffer position of character hit */
            if(x<ts->x || y<ts->y)
            {  amd->amdr->objpos=ts->textpos;
            }
            else
            {  SetFont(mrp,tx->elt.font);
               SetSoftStyle(mrp,0,0x0f);  /* No style because italics extension */
               amd->amdr->objpos=ts->textpos+TextFit(mrp,
                  tx->text->buffer+ts->textpos,ts->length,&te,NULL,1,x-ts->x,tx->height);
            }
         }
         break;
      }
   }
   return result;
}

/* Returns new state */
static UWORD Draglimits(struct Text *tx,void *startobject,ULONG startobjpos,
   void *endobject,ULONG endobjpos,UWORD state,long *startp,long *lengthp)
{  long s=0,l=0;
   if(tx==startobject)
   {  switch(state)
      {  case AMDS_BEFORE:
            state=AMDS_RENDER;
            if(tx==endobject)
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
               l=tx->elt.textpos+tx->elt.length-s;
            }
            break;
         case AMDS_REVERSE:
            state=AMDS_AFTER;
            s=tx->elt.textpos;
            l=startobjpos-tx->elt.textpos+1;
            break;
      }
   }
   else if(tx==endobject)
   {  switch(state)
      {  case AMDS_BEFORE:
            state=AMDS_REVERSE;
            s=endobjpos;
            l=tx->elt.textpos+tx->elt.length-s;
            break;
         case AMDS_RENDER:
            state=AMDS_AFTER;
            s=tx->elt.textpos;
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
            s=tx->elt.textpos;
            l=tx->elt.length;
            break;
         case AMDS_AFTER:
            s=0;
            l=0;
            if(!tx->histart)
            {  state=AMDS_DONE;
            }
            break;
      }
   }
   *startp=s;
   *lengthp=l;
   return state;
}

static long Dragrendertext(struct Text *tx,struct Amdragrender *amd)
{  long s,l,e,olde;
   struct Coords *coo;
   amd->state=Draglimits(tx,amd->startobject,amd->startobjpos,
      amd->endobject,amd->endobjpos,amd->state,&s,&l);
   if(tx->histart!=s || tx->hilength!=l)
   {  coo=Clipcoords(tx->elt.cframe,amd->coords);
      e=s+l-1;
      olde=tx->histart+tx->hilength-1;
      if(!tx->hilength)
      {  /* Only highlight new */
         if(l) Highlighttext(tx,coo,s,e);
      }
      else if(!l)
      {  /* Only clear old */
         if(tx->hilength) Highlighttext(tx,coo,tx->histart,olde);
      }
      else if(olde<s || e<tx->histart)
      {  /* Old and new sections are disjunct */
         Highlighttext(tx,coo,tx->histart,olde);
         Highlighttext(tx,coo,s,e);
      }
      else
      {  /* Overlapping sections */
         if(tx->histart<s)
         {  Highlighttext(tx,coo,tx->histart,s-1);
         }
         else if(s<tx->histart)
         {  Highlighttext(tx,coo,s,tx->histart-1);
         }
         if(olde>e)
         {  Highlighttext(tx,coo,e+1,olde);
         }
         else if(e>olde)
         {  Highlighttext(tx,coo,olde+1,e);
         }
      }
      tx->histart=s;
      tx->hilength=l;
      Unclipcoords(coo);
   }
   return 0;
}

static long Dragcopytext(struct Text *tx,struct Amdragcopy *amd)
{  long s,l;
   short i,n;
   amd->state=Draglimits(tx,amd->startobject,amd->startobjpos,
      amd->endobject,amd->endobjpos,amd->state,&s,&l);
   if(l)
   {  if(tx->elt.leftindent
      && (!amd->clip->length || amd->clip->buffer[amd->clip->length-1]=='\n'))
      {  n=tx->elt.leftindent;
         if(tx->elt.halign&HALIGN_BULLET) n--;
         for(i=0;i<n;i++)
         {  Addtobuffer(amd->clip,"    ",4);
         }
      }
      if(tx->elt.halign&HALIGN_BULLET)
      {  Addtobuffer(amd->clip,"  ",2);
      }
      Addtobuffer(amd->clip,tx->text->buffer+s,l);
   }
   return 0;
}

static void Disposetext(struct Text *tx)
{  Clearsections(tx,TRUE);
   if(tx->flags&TXTF_BLINKING)
   {  Aremchild(Aweb(),(struct Aobject *)tx,AOREL_APP_WANT_BLINK);
   }
   Amethodas(AOTP_ELEMENT,tx,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Text_Dispatcher,
struct Text *,tx,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newtext((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Settext(tx,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Gettext(tx,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measuretext(tx,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layouttext(tx,(struct Amlayout *)amsg);
         break;
      case AOM_ALIGN:
         result=Aligntext(tx,(struct Amalign *)amsg);
         break;
      case AOM_RENDER:
         result=Rendertext(tx,(struct Amrender *)amsg);
         break;
      case AOM_HITTEST:
         result=Hittesttext(tx,(struct Amhittest *)amsg);
         break;
      case AOM_MOVE:
         result=Movetext(tx,(struct Ammove *)amsg);
         break;
      case AOM_SEARCHPOS:
         result=Searchpostext(tx,(struct Amsearch *)amsg);
         break;
      case AOM_SEARCHSET:
         result=Searchsettext(tx,(struct Amsearch *)amsg);
         break;
      case AOM_DRAGTEST:
         result=Dragtesttext(tx,(struct Amdragtest *)amsg);
         break;
      case AOM_DRAGRENDER:
         result=Dragrendertext(tx,(struct Amdragrender *)amsg);
         break;
      case AOM_DRAGCOPY:
         result=Dragcopytext(tx,(struct Amdragcopy *)amsg);
         break;
      case AOM_DISPOSE:
         Disposetext(tx);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         AmethodasA(AOTP_ELEMENT,(struct Aobject *)tx,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installtext(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_TEXT,(Tag)Text_Dispatcher)) return FALSE;
   return TRUE;
}
