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

/* body.c - AWeb html document body object */

#include "aweb.h"
#include "body.h"
#include "element.h"
#include "docprivate.h"
#include "frame.h"
#include "window.h"
#include "copy.h"


#include <graphics/rpattr.h>
#include <proto/utility.h>
#include <proto/graphics.h>
#include <proto/layers.h>

/*------------------------------------------------------------------------*/
/* Childs (elements) are added dynamically, and a child can change later.
 * While LAYOUTing, the start of each horizontal line is remembered.
 * MEASURE with the CHANGED flag set will start at the beginning of the
 * last line.
 * LAYOUT with the CHANGED flag set will start at the beginning of the
 * last line that has no (floating object) margins and is not in the middle
 * of a multiline element.
 * RENDER with the CHANGED flag set will start at the beginning of the
 * last line, or if also the CLEAR flag is set with the last line that has
 * no (floating object) margins. Since LAYOUT will add lines it remembers
 * the y position of the first line added, and RENDER will start looking at
 * this line instead of the last line in the list.
 * Without CHANGED flag these methods will start at the first element.
 *
 * When AOBJ_Changedchild is set, the child is looked up and all remembered
 * lines after this child are removed. This ensures the next CHANGED methods
 * will start at or before the line containing the changed child.
 */

/*------------------------------------------------------------------------*/

#define BDYF_SUB           0x0001   /* subscript mode */
#define BDYF_SUP           0x0002   /* superscript mode */
#define BDYF_CHANGEDCHILD  0x0004   /* we have a changed or added child */
#define BDYF_NOBR          0x0008   /* nobreak mode */
#define BDYF_NOBACKGROUND  0x0010   /* no backgrounds */
#define BDYF_FORCEBGCOLOR  0x0020   /* use bgcolor even is bg is off */
#define BDYF_LAYOUTREADY   0x0040   /* layout of all changed childs is ready */

struct Bodybuild
{  LIST(Fontinfo) font;       /* font stack */
   LIST(Listinfo) list;       /* Listinfo stack */
   UWORD hardstyle;          /* current hard style */
   short divalign,align;
   short fonttype;            /* current hard normal/fixed font type */
   short bqindent;            /* Current blockquote indent level */
   void *link;                /* current hyperlink */
};

/* Pushfont() "which" flags */
#define FONTW_STYLE     0x0001
#define FONTW_ABSSIZE   0x0002
#define FONTW_RELSIZE   0x0004
#define FONTW_COLOR     0x0008
#define FONTW_FACE      0x0010

struct Fontinfo               /* stacked character font info */
{  NODE(Fontinfo);            /* stack node */
   short type;                /* normal or fixed */
   short size;                /* size */
   UWORD flags;              /* see below */
   UWORD style;              /* resulting style */
   struct Colorinfo *color;   /* color for this font or NULL */
   UBYTE *face;               /* face name (reference) for this font or NULL */
   UBYTE *facestring;         /* dynamic string */
};

#define FONTF_RELSIZE   0x0001   /* size is relative */
#define FONTF_BASE      0x0002   /* can serve as base font size */
#define FONTF_COLOR     0x0004   /* color was explicitly set */
#define FONTF_FACE      0x0008   /* face was expliticly set */

struct Line
{  NODE(Line);
   struct Element *child;     /* First child on this line */
   long y;                    /* Smallest y coordinate on this line */
   long w;                    /* Width of this line */
   UWORD flags;
};

#define LINEF_MORE      0x0001   /* Line starts in the middle of a multiline element */
#define LINEF_MARGIN    0x0002   /* Line has floating margins */

#define LEFTINDENT(bd) ((ISEMPTY(&(bd)->bld->list)?0:\
   (bd)->bld->list.first->indent)+(bd)->bld->bqindent)
#define RIGHTINDENT(bd) ((bd)->bld->bqindent)

static struct Listinfo nolist={0};

#define INDENT(x) ((x)*40)

struct Margin                 /* Floating margin info */
{  NODE(Margin);
   long y;                    /* First ypos below margin */
   long eaten;                /* Current margin eaten from total */
   short indent;              /* List indentation level this object was placed at */
};

#define MARGIN_LEFT     0x0001
#define MARGIN_RIGHT    0x0002
#define MARGIN_ALL      0x0003   /* Only used in Findclearmargin() */

struct Openfont               /* Font opened by us */
{  NODE(Openfont);
   struct TextFont *font;     /* The font */
};

/*------------------------------------------------------------------------*/

/* Check if we already opened this font. If not, open it.
 * We must open all fonts we use ourselves, in case the preferences change
 * and the dynamic Fontinfo structure is disposed of and the font closed. */
struct Openfont *Addopenfont(struct Body *bd,struct TextFont *font)
{  struct Openfont *of;
   for(of=bd->openfonts.first;of->next;of=of->next)
   {  if(of->font==font)
      {  return of;
      }
   }
   if(of=ALLOCSTRUCT(Openfont,1,MEMF_CLEAR))
   {  struct TextAttr ta={0};
      ta.ta_Name=font->tf_Message.mn_Node.ln_Name;
      ta.ta_YSize=font->tf_YSize;
      of->font=OpenFont(&ta);
      ADDTAIL(&bd->openfonts,of);
   }
   return of;
}

/* Close this font and dispose structure */
void Freeopenfont(struct Openfont *of)
{  if(of)
   {  if(of->font) CloseFont(of->font);
      FREE(of);
   }
}

/*------------------------------------------------------------------------*/

/* Push a fontinfo on the stack. */
static BOOL Pushfont(struct Body *bd,short style,short size,struct Colorinfo *ci,
   UBYTE *face,UWORD which)
{  struct Fontinfo *fi=PALLOCSTRUCT(Fontinfo,1,MEMF_CLEAR,bd->pool);
   struct Styleprefs *sp;
   if(fi)
   {  ADDHEAD(&bd->bld->font,fi);
      if(which&FONTW_STYLE)
      {  sp=&prefs.browser.styles[style];
         fi->type=sp->fonttype;
         fi->size=sp->fontsize;
         fi->flags=sp->relsize?FONTF_RELSIZE:FONTF_BASE;
         fi->style=sp->style;
      }
      else
      {  fi->type=fi->next->type;
         fi->style=fi->next->style;
         if(which&FONTW_ABSSIZE)
         {  fi->size=size;
            fi->flags=0;
         }
         else if(which&FONTW_RELSIZE)
         {  fi->size=size;
            fi->flags=FONTF_RELSIZE;
         }
         else
         {  fi->size=fi->next->size;
            fi->flags=fi->next->flags&~FONTF_FACE;
         }
      }
      if(which&FONTW_COLOR)
      {  fi->color=ci;
         fi->flags|=FONTF_COLOR;
      }
      else if(fi->next->next)
      {  fi->color=fi->next->color;
      }
      if(which&FONTW_FACE)
      {  fi->facestring=Dupstr(face,-1);
         fi->face=fi->facestring;
         fi->flags|=FONTF_FACE;
      }
      else if(fi->next->next)
      {  fi->face=fi->next->face;
      }
//printf("pushfont color=%08x\n",fi->color);
   }
   return (BOOL)(fi!=NULL);
}

/* Pop a fontinfo from the stack */
static void Popfont(struct Body *bd)
{  struct Fontinfo *fi=bd->bld->font.first;
   if(fi->next->next)   /* leave at least one on stack */
   {  REMOVE(fi);
      if(fi->facestring) FREE(fi->facestring);
      FREE(fi);
//printf("popfont color=%08x\n",bd->bld->font.first->color);
   }
}

/* Sets basefont size. Just patch the size in the bottom fontinfo. */
static void Setbasefont(struct Body *bd,short size,BOOL rel)
{  short bsize=bd->bld->font.last->size;
   if(rel) bsize+=size;
   else bsize=size;
   if(bsize<1) bsize=1;
   if(bsize>7) bsize=7;
   bd->bld->font.last->size=bsize;
}

/* Sets basefont color. Patch in bottom fontinfo and in higher ones that didn't
 * set the color themselves */
static void Setbasecolor(struct Body *bd,struct Colorinfo *ci)
{  struct Fontinfo *fi;
   bd->bld->font.last->color=ci;
   for(fi=bd->bld->font.last->prev;fi->prev;fi=fi->prev)
   {  if(fi->flags&FONTF_COLOR) break;
      fi->color=ci;
   }
}

/* Sets basefont face. Patch in bottom fontinfo and in higher ones that didn't
 * set the face themselves */
static void Setbaseface(struct Body *bd,UBYTE *face)
{  struct Fontinfo *fi,*base;
   base=bd->bld->font.last;
   if(base->facestring) FREE(base->facestring);
   base->facestring=Dupstr(face,-1);
   base->face=base->facestring;
   for(fi=bd->bld->font.last->prev;fi->prev;fi=fi->prev)
   {  if(fi->flags&FONTF_FACE) break;
      fi->face=base->facestring;
   }
}

/* Gets current fontprefs */
static struct Fontprefs *Getfontprefs(struct Body *bd,struct Fontinfo *fi,UWORD *style)
{  short fonttype,fontsize;
   UBYTE *face=fi->face;
   fonttype=bd->bld->fonttype || fi->type;
   fontsize=fi->size;
   if(fi->flags&FONTF_RELSIZE)
   {  for(fi=fi->next;fi->next && (fi->flags&FONTF_RELSIZE);fi=fi->next);
      if(fi->next)
      {  fonttype=fonttype || fi->type;
         fontsize+=fi->size;
         if(style) *style|=fi->style;
      }
   }
   if(fontsize<1) fontsize=1;
   if(fontsize>7) fontsize=7;
   return Matchfont(face,fontsize-1,fonttype);
}

/* Gets current font attributes */
static void Getcurrentfont(struct Body *bd,struct Fontprefs **fp,UWORD *style,
   struct Colorinfo **ci)
{  *style=bd->bld->hardstyle|bd->bld->font.first->style;
   *fp=Getfontprefs(bd,bd->bld->font.first,style);
   *ci=bd->bld->font.first->color;
}

/*------------------------------------------------------------------------*/

/* Add a new listinfo */
static void Addlist(struct Body *bd,struct Listinfo *lis)
{  struct Listinfo *li=PALLOCSTRUCT(Listinfo,1,MEMF_CLEAR,bd->pool);
   if(li)
   {  li->type=lis->type;
      li->bullettype=lis->bullettype;
      if(li->bullettype==BDBT_IMAGE)
      {  if(lis->bulletsrc) li->bulletsrc=Dupstr(lis->bulletsrc,-1);
      }
      li->bulletnr=lis->bulletnr;
      if(ISEMPTY(&bd->bld->list))
      {  li->level=1;
      }
      else
      {  li->level=bd->bld->list.first->level+1;
      }
      li->indent=li->level;
      if(li->bullettype==BDBT_DEFAULT)
      {  switch(li->type)
         {  case BDLT_UL:
               li->bullettype=(li->level-1)%6+1;
               break;
            case BDLT_OL:
               li->bullettype=BDBT_NUMBER;
               break;
         }
      }
      ADDHEAD(&bd->bld->list,li);
   }
}

/* Remove the top listinfo */
static void Remlist(struct Body *bd)
{  struct Listinfo *li=(struct Listinfo *)REMHEAD(&bd->bld->list);
   if(li)
   {  if(li->bulletsrc) FREE(li->bulletsrc);
      FREE(li);
   }
}

/* Set or unset <DT> in top <DL> list */
static void Setdterm(struct Body *bd,BOOL set)
{  struct Listinfo *li=bd->bld->list.first;
   if(li->next && li->type==BDLT_DL)
   {  if(set)
      {  li->indent=li->level-1;
      }
      else
      {  li->indent=li->level;
      }
   }
}

/*------------------------------------------------------------------------*/

/* Dispose the Bodybuild */
static void Disposebodybuild(struct Body *bd)
{  struct Fontinfo *fi;
   if(bd->bld)
   {  while(fi=(struct Fontinfo *)REMHEAD(&bd->bld->font))
      {  if(fi->facestring) FREE(fi->facestring);
         FREE(fi);
      }
      while(!ISEMPTY(&bd->bld->list)) Remlist(bd);
      FREE(bd->bld);
      bd->bld=NULL;
   }
}

/* Create a Bodybuild */
static BOOL Newbodybuild(struct Body *bd)
{  if(bd->bld) Disposebodybuild(bd);
   if(bd->bld=PALLOCSTRUCT(Bodybuild,1,MEMF_CLEAR,bd->pool))
   {  NEWLIST(&bd->bld->font);
      NEWLIST(&bd->bld->list);
   }
   return BOOLVAL(bd->bld);
}

/*------------------------------------------------------------------------*/

/* Add a line index. */
static struct Line *Addline(struct Body *bd,struct Element *child,long y,long w,
   BOOL more,BOOL margin)
{  struct Line *line;
   if(line=PALLOCSTRUCT(Line,1,MEMF_CLEAR,bd->pool))
   {  line->y=y;
      line->w=w;
      line->child=child;
      if(more) line->flags|=LINEF_MORE;
      if(margin) line->flags|=LINEF_MARGIN;
      ADDTAIL(&bd->lines,line);
   }
   return line;
}

/* Remove all lines below this y. Remove all lines for this y except the last one. */
static void Removelinesbelow(struct Body *bd,long y)
{  struct Line *line,*prev;
   for(line=bd->lines.last;line->prev && line->y>y;line=prev)
   {  prev=line->prev;
      REMOVE(line);
      FREE(line);
   }
   for(;line->prev && line->y==y;line=prev)
   {  prev=line->prev;
      if(prev->prev && prev->y==y)
      {  REMOVE(line);
         FREE(line);
      }
   }
}

/* Remove all lines from behind with one of these flags set */
static void Removelinesflags(struct Body *bd,UWORD flags)
{  struct Line *line,*prev;
   for(line=bd->lines.last;line->prev && (line->flags&flags);line=prev)
   {  prev=line->prev;
      REMOVE(line);
      FREE(line);
   }
}

/* Find first child from last line, or first child if there are no lines. */
static struct Element *Findchild(struct Body *bd)
{  if(bd->lines.last->prev) return bd->lines.last->child;
   else return bd->contents.first;
}

/* Remember the first changed child. */
static void Setchchild(struct Body *bd,struct Element *child)
{  struct Element *ch;
   if(bd->chchild)
   {  for(ch=bd->chchild->object.prev;ch->object.prev;ch=ch->object.prev)
      {  if(ch==child)
         {  bd->chchild=child;
            break;
         }
      }
   }
   else
   {  bd->chchild=child;
   }
}

/* Find last line before this Y, with these flags unset.
 * Start from end because for long bodies it's faster and for shorter bodies
 * it doesn't differ much. */
static struct Line *Findlinebefore(struct Body *bd,long y,UWORD flags)
{  struct Line *line;
   for(line=bd->lines.last;line->prev;line=line->prev)
   {  if(line->y<=y && !(line->flags&flags)) return line;
   }
   return NULL;
}

/* Find the maximum width of all existing lines */
static long Lineswidth(struct Body *bd)
{  struct Line *line;
   long w=0;
   for(line=bd->lines.first;line->next;line=line->next)
   {  if(line->w>w) w=line->w;
   }
   return w;
}

/*------------------------------------------------------------------------*/

/* Add a margin. */
static void Addmargin(struct Body *bd,long y,long eaten,short indent,UWORD side)
{  struct Margin *newm=PALLOCSTRUCT(Margin,1,MEMF_CLEAR,bd->pool);
   struct Margin *m,*nextm;
   LIST(Margin) *list;
   if(side==MARGIN_LEFT) list=(APTR)&bd->leftmargins;
   else list=(APTR)&bd->rightmargins;
   if(newm)
   {  /* Compute new margins starting from current */
      newm->eaten=eaten;
      m=list->first;
      if(m->next) newm->eaten+=m->eaten;
      newm->y=y;
      newm->indent=indent;
      /* Insert new margin in list sorted on ending y position.
       * Remove earlier ending margins on this side. */
      for(m=list->first;m->next;m=nextm)
      {  nextm=m->next;
         if(m->y>newm->y) break;
         REMOVE(m);
         FREE(m);
      }
      INSERT(list,newm,m->prev);
   }
}

/* Find the current margins. Remove any earlier ended margins. */
static void Currentmargin(struct Body *bd,long y,long *left,long *right)
{  struct Margin *m,*nextm;
   *left=*right=0;
   for(m=bd->leftmargins.first;m->next;m=nextm)
   {  nextm=m->next;
      if(m->y<=y)
      {  REMOVE(m);
         FREE(m);
      }
      else
      {  *left=m->eaten;
         break;
      }
   }
   for(m=bd->rightmargins.first;m->next;m=nextm)
   {  nextm=m->next;
      if(m->y<=y)
      {  REMOVE(m);
         FREE(m);
      }
      else
      {  *right=m->eaten;
         break;
      }
   }
}

/* Find the y position where eaten is not more than a given value */
static long Findmargin(struct Body *bd,long y,long eaten)
{  struct Margin *lm,*rm;
   long leaten,reaten,fy=y;
   /* Balance both lists */
   lm=bd->leftmargins.first;
   rm=bd->rightmargins.first;
   while(lm->next || rm->next)
   {  leaten=(lm->next)?lm->eaten:0;
      reaten=(rm->next)?rm->eaten:0;

      if(leaten+reaten<=eaten) break;
      /* Set fy to where the first one (left or right) ends */
      if(lm->next)
      {  fy=lm->y;
         if(rm->next) fy=MIN(fy,rm->y);
      }
      else if(rm->next) fy=rm->y;
      /* Advance list that ends on this fy (could be both) */
      if(lm->next && lm->y==fy) lm=lm->next;
      if(rm->next && rm->y==fy) rm=rm->next;
   }
   return MAX(y,fy);
}

/* Find the y position where margin is clear */
static long Findclearmargin(struct Body *bd,long y,UWORD side)
{  if((side&MARGIN_LEFT) && !ISEMPTY(&bd->leftmargins))
   {  y=MAX(y,bd->leftmargins.last->y);
   }
   if((side&MARGIN_RIGHT) && !ISEMPTY(&bd->rightmargins))
   {  y=MAX(y,bd->rightmargins.last->y);
   }
   return y;
}

/* Find the y position where no left margins for this or higher indent levels exist */
static long Findindentmargin(struct Body *bd,long y,short indent)
{  struct Margin *m;
   for(m=bd->leftmargins.first;m->next;m=m->next)
   {  if(m->indent<indent) break;
      if(m->y>y) y=m->y;
   }
   return y;
}

/*------------------------------------------------------------------------*/

static long Measurebody(struct Body *bd,struct Ammeasure *amm)
{  struct Element *child,*ch;
   struct Ammresult ammr;
   long w=0,totalw=0,totalminw=0,addwidth=0,indent=0,halign,left,right,minw,width;
   ULONG flags;
   ammr.newline=TRUE;
   /* If AMMF_CHANGED, do measure only if we have a changed child */
   if(amm->flags&AMMF_CHANGED)
   {  if(!(bd->flags&BDYF_CHANGEDCHILD)) return 0;
      child=Findchild(bd);
   }
   else
   {  child=bd->contents.first;
   }
   for(;child->object.next;child=child->object.next)
   {  if(ammr.newline)
      {  /* Get left and right indent levels (valid for entire line)
          * and if it is a bullet (valid for first child only).
          * First find next visible child and start measuring from there. */
         for(ch=child;ch->object.next && !Agetattr((struct Aobject *)ch,AOELT_Visible);ch=ch->object.next);
         if(!ch->object.next) ch=child;
         Agetattrs((struct Aobject *)ch,
            AOELT_Halign,(Tag)&halign,
            AOELT_Leftindent,(Tag)&left,
            AOELT_Rightindent,(Tag)&right,
            TAG_END);
         indent=INDENT(left)+INDENT(right);
         w=indent;
         addwidth=0;
         child=ch;
      }
      else halign=0;
      ammr.width=ammr.minwidth=ammr.addwidth=0;
      ammr.newline=FALSE;
      flags=amm->flags;
      if(child==bd->chchild) flags|=AMMF_CHANGED;
      Ameasure((struct Aobject *)child,amm->width-2*bd->hmargin,amm->height-2*bd->vmargin,
         addwidth,flags,amm->text,&ammr);
      if(halign&HALIGN_BULLET)
      {  width=MAX(0,ammr.width-indent);
         minw=MAX(0,ammr.minwidth-indent)+indent;
      }
      else
      {  width=ammr.width;
         minw=ammr.minwidth+indent;
      }
      w+=width;
      if(w>totalw) totalw=w;
      if(minw>totalminw) totalminw=minw;
      addwidth=ammr.addwidth;
   }
   if(amm->ammr)
   {  amm->ammr->width=totalw+2*bd->hmargin;
      {  amm->ammr->minwidth=totalminw+2*bd->hmargin;
      }
      amm->ammr->newline=TRUE;
   }
   return 0;
}

static long Layoutbody(struct Body *bd,struct Amlayout *amlp)
{  struct Element *child,*ch,*endch=0,*nextchild=0,*floatchild;
   struct Amlayout aml=*amlp;
   struct Amlresult amlr;
   struct Amalign ama;
   struct Line *line;
   long y,newy,endx;
   long above,below,toph,bottomh;
   long halign,left,right,need;
   long margleft,margright;
   long result=0;
   UWORD nextflags=0,clrmargin=0;
   BOOL align=FALSE,wasmore,bullet,first=TRUE,softnl=FALSE;
   bd->aox=bd->aoy=0;
   /* If AMLF_CHANGED, do layout only if we have a changed child */
   if((amlp->flags&AMLF_CHANGED) && !(bd->flags&BDYF_CHANGEDCHILD)) return 0;
   /* If AMLF_CHANGED, remove all lines from the end with MORE or MARGIN set,
    * else remove all lines so we start from top. */
   if(amlp->flags&AMLF_CHANGED)
   {  Removelinesflags(bd,LINEF_MORE|LINEF_MARGIN);
   }
   else
   {  Removelinesbelow(bd,-1);
   }
   /* If there is a last line, use its Y as starting Y. Remove the line, a new
    * one will be added in the process. */
   if(bd->lines.last->prev)
   {  line=bd->lines.last;
      y=bd->rendery=line->y;
      child=line->child;
      /* If a FITHEIGHT layout is requested, check if we are not already too high */
      if((amlp->flags&AMLF_FITHEIGHT) && y>amlp->height)
      {  bd->aoh=y;
         return 0;
      }
      REMOVE(line);
      FREE(line);
   }
   else
   {  y=bd->vmargin;
      bd->rendery=0;
      child=bd->contents.first;
   }
   aml.amlr=&amlr;
   bd->aow=Lineswidth(bd);
   /* if floatchild is nonnull, set AMLF_RETRY until this child has been layed out. */
   floatchild=NULL;
   for(ch=child;ch->object.next;ch=ch->object.next)
   {  Asetattrs((struct Aobject *)ch,AOELT_Resetlayout,TRUE,TAG_END);
   }
   while(child && child->object.next)
   {  /* New line. Get horizontal align and indents from first visible child. */
      for(ch=child;ch->object.next && !Agetattr((struct Aobject *)ch,AOELT_Visible);ch=ch->object.next);
      if(!ch->object.next) ch=child;
      Agetattrs((struct Aobject *)ch,
         AOELT_Halign,(Tag)&halign,
         AOELT_Leftindent,(Tag)&left,
         AOELT_Rightindent,(Tag)&right,
         TAG_END);
      bullet=BOOLVAL(halign&HALIGN_BULLET);
      above=below=toph=bottomh=0;
      if(bullet)
      {  /* Find correct margin for this bullet.
          * Don't mess with Y if it is a retry of a bullet, bullet is already correctly
          * placed then. */
         if(!(nextflags&AMLF_RETRY))
         {  y=Findindentmargin(bd,y,left);
         }
      }
      else
      {  /* Find correct margin for this level of text. That is, find a place
          * where all left margins of higher levels are clear. */
         y=Findindentmargin(bd,y,left+1);
      }
      Currentmargin(bd,y,&margleft,&margright);
      aml.startx=bd->hmargin+margleft+INDENT(left);
      aml.width=amlp->width-bd->hmargin-margright-INDENT(right);
      aml.flags=nextflags;
      wasmore=BOOLVAL(aml.flags&AMLF_MORE);
      endx=0;
      if(bullet)
      {  /* Place bullet right-aligned with real startx minus 8 pixels.
          * Note: (ch) is first visible child */
         aml.startx=MAX(bd->hmargin+margleft,aml.startx-8-ch->aow);
      }
      aml.flags|=AMLF_FIRST;
      /* (child) is first element on line, (ch) is running element */
      for(ch=child;ch->object.next;ch=ch->object.next)
      {  memset(&amlr,0,sizeof(amlr));
         if((amlp->flags&AMLF_CHANGED) && ch==bd->chchild) aml.flags|=AMLF_CHANGED;
         if(floatchild) aml.flags|=AMLF_RETRY;
         if(amlp->flags&AMLF_INTABLE) aml.flags|=AMLF_INTABLE;
         AmethodA((struct Aobject *)ch,(struct Amessage *)&aml);
#ifdef BETAVERSION
if(SetSignal(0,0)&SIGBREAKF_CTRL_C) return 0;
#endif
         if(softnl && amlr.result==AMLR_NEWLINE && amlr.endx==aml.startx)
         {  /* Hard line break immediately after a soft newline, ignore */
            amlr.result=AMLR_OK;
         }
         else if(amlr.result&AMLRF_OK)
         {  above=MAX(above,amlr.above);
            below=MAX(below,amlr.below);
            toph=MAX(toph,amlr.toph);
            bottomh=MAX(bottomh,amlr.bottomh);
            endx=amlr.endx;
         }
         if(ch==floatchild)
         {  aml.flags&=~AMLF_RETRY;
            floatchild=NULL;
         }
         if(amlr.result!=AMLR_FBREAK)
         {  if(amlr.result!=AMLR_OK) break;
            if(amlr.endx>aml.width) break;
         }
         if(amlr.endx>aml.startx)
         {  aml.startx=amlr.endx;
            aml.flags&=~AMLF_FIRST;
         }
         aml.flags&=~(AMLF_MORE);
         softnl=FALSE;
         if(bullet && Agetattr((struct Aobject *)ch,AOELT_Visible))
         {  /* Restore startx after bullet, but allow long bullets */
            aml.startx=MAX(aml.startx,bd->hmargin+margleft+INDENT(left));
            bullet=FALSE;
         }
      }
      while(amlr.result==AMLR_BREAK && ch!=child)
      {  ch=ch->object.prev;
         aml.flags=AMLF_BREAK;
         AmethodA((struct Aobject *)ch,(struct Amessage *)&aml);
         endx=amlr.endx;
      }
      switch(amlr.result)
      {  case AMLR_OK:
         case AMLR_FBREAK:
            /* (ch) fits because it's last before BREAK, or (ch) is past the end. */
            nextflags=0;
            if(ch->object.next)
            {  nextchild=ch->object.next;
               endch=ch->object.next;
            }
            else
            {  nextchild=ch;
               endch=ch;
            }
            align=TRUE;
            softnl=TRUE;
            break;
         case AMLR_NEWLINE:
            /* (ch) fits because it's last before newline */
            nextflags=0;
            nextchild=ch->object.next;
            endch=ch->object.next;
            align=TRUE;
            break;
         case AMLR_NLCLRLEFT:
         case AMLR_NLCLRRIGHT:
         case AMLR_NLCLRALL:
            /* (ch) fits because it's last before newline */
            nextflags=0;
            nextchild=ch->object.next;
            endch=ch->object.next;
            align=TRUE;
            if(amlr.result&AMLRF_CLRLEFT) clrmargin|=MARGIN_LEFT;
            if(amlr.result&AMLRF_CLRRIGHT) clrmargin|=MARGIN_RIGHT;
            break;
         case AMLR_MORE:
            /* (ch) fits but wants another try */
            nextflags=AMLF_MORE;
            nextchild=ch;
            endch=ch->object.next;
            align=TRUE;
            break;

         case AMLR_NOFIT:
         case AMLR_BREAK:
            /* (ch) doesn't fit.
             * If it is the first one, search for space and try again. If no space
             * found, try again forced.
             * If (ch) is not the first one, break before this one.
             * Set MORE flag just in case */
            if(ch==child)
            {  /* we need: amlr.endx - original startx + indentations.
                * (let compiler optimize -&+ INDENT(left)) */
               need=amlr.endx-(bd->hmargin+margleft+INDENT(left))
                  +INDENT(left)+INDENT(right)+2*bd->hmargin;
               newy=Findmargin(bd,y,amlp->width-need);
               if(newy>y) nextflags=0;
               else nextflags=AMLF_FORCE;
               y=newy;
               nextflags|=AMLF_MORE;
               nextchild=child;
               align=FALSE;
            }
            else
            {  nextflags=AMLF_MORE;
               nextchild=ch;
               endch=ch;
               align=TRUE;
               softnl=TRUE;
            }
            break;
         case AMLR_FLOATING:
            /* Align (ch) element now, and RETRY the others from start. Add a
             * new margin. */
            halign=Agetattr((struct Aobject *)ch,AOELT_Halign)&HALIGN_FLOATRIGHT;
            ama.amsg.method=AOM_ALIGN;
            if(halign==HALIGN_FLOATRIGHT)
            {  ama.dx=MAX(amlp->width-bd->hmargin-margright-INDENT(right)-ch->aox-ch->aow,
                  -ch->aox);
            }
            else
            {  ama.dx=bd->hmargin+margleft+INDENT(left)-ch->aox;
            }
            ama.y=y;
            AmethodA((struct Aobject *)ch,(struct Amessage *)&ama);
            bd->aow=MAX(bd->aow,ch->aox+ch->aow+bd->hmargin);
            nextflags=AMLF_RETRY|AMLF_MORE;
            nextchild=child;
            align=FALSE;
            floatchild=ch;
            Addmargin(bd,ch->aoy+ch->aoh,ch->aow,left,
               (halign==HALIGN_FLOATRIGHT)?MARGIN_RIGHT:MARGIN_LEFT);
            break;
         case AMLR_CONT:
            /* element wants to continue on the same line */
            nextflags=AMLF_MORE|AMLF_FORCE;
            nextchild=ch;
            endch=ch->object.next;
            align=FALSE;
            break;
      }
      if(align)
      {  bottomh=MAX(bottomh,above+below);
         ama.amsg.method=AOM_ALIGN;
         switch(halign&0x0f)
         {  case HALIGN_LEFT:
               ama.dx=0;
               break;
            case HALIGN_CENTER:
               if(endx<aml.width) ama.dx=(aml.width-endx)/2;
               else ama.dx=0;
               break;
            case HALIGN_RIGHT:
               if(endx<aml.width) ama.dx=aml.width-endx;
               else ama.dx=0;
               break;
         }
         ama.y=y;
         ama.baseline=bottomh-below-1;
         ama.height=MAX(toph,bottomh);
         for(ch=child;ch!=endch;ch=ch->object.next)
         {  AmethodA((struct Aobject *)ch,(struct Amessage *)&ama);
         }
         bd->aow=MAX(bd->aow,endx+bd->hmargin+ama.dx);
         /* Add a new line index */
         Addline(bd,child,y,endx+bd->hmargin,
            wasmore,!(ISEMPTY(&bd->leftmargins) && ISEMPTY(&bd->rightmargins)));
         /* Remember space above the baseline for first line */
         if(first)
         {  if(amlp->amlr)
            {  amlp->amlr->above=above;
            }
            first=FALSE;
         }
         y+=ama.height;
         if(clrmargin)
         {  y=Findclearmargin(bd,y,clrmargin);
            clrmargin=0;
         }
         if((amlp->flags&AMLF_FITHEIGHT) && y>amlp->height)
         {  bd->aoh=y;
            Removelinesbelow(bd,bd->rendery);
            result=AMLR_FHAGAIN;
            break;
         }
      }
      child=nextchild;
   }
   /* Include all pending floating objects in body height */
   bd->aoh=Findmargin(bd,y,0)+bd->vmargin;
   /* Clear all pending margins, as MORE starts before the margins anyway. */
   Currentmargin(bd,bd->aoh,&margleft,&margright);
   bd->flags|=BDYF_LAYOUTREADY;
   return result;
}

static struct RastPort *Alloctemprp(struct RastPort *rp)

{

   struct Layer_Info *li;
   struct Layer *l;
   struct BitMap *tempbm;
   ULONG bmwidth, bmheight, bmdepth;

   bmwidth  = GetBitMapAttr(rp->BitMap,BMA_WIDTH);
   bmheight = GetBitMapAttr(rp->BitMap,BMA_HEIGHT);
   bmdepth  = GetBitMapAttr(rp->BitMap,BMA_DEPTH);

   if(tempbm = AllocBitMap(bmwidth,bmheight,bmdepth,BMF_MINPLANES,rp->BitMap))
   {
       if (rp && rp->Layer)
       {
          if ((li = NewLayerInfo()) != NULL)
          {
             if ((l = CreateUpfrontHookLayer(li,
                                             tempbm,
                                             rp->Layer->bounds.MinX,
                                             rp->Layer->bounds.MinY,
                                             rp->Layer->bounds.MaxX,
                                             rp->Layer->bounds.MaxY,
                                             LAYERSIMPLE,
                                             LAYERS_NOBACKFILL,
                                             NULL)) != NULL)
             {
                InstallLayerHook(l,(struct Hook *)rp->Layer->BackFill);
                return (l->rp);
             }
             DisposeLayerInfo(li);
          }
       }
       FreeBitMap(tempbm);
   }
   return (NULL);
}

static void Freetemprp(struct RastPort *rp)
{
    struct Layer_Info *li;
    struct BitMap *bm = NULL;
    if(rp)
    {
        bm = rp->BitMap;
    }
    if (rp && rp->Layer)
    {
       li = rp->Layer->LayerInfo;
       InstallLayerInfoHook(li,LAYERS_NOBACKFILL);
       DeleteLayer(0,rp->Layer);
       DisposeLayerInfo(li);
    }
    if(bm)
    {
        FreeBitMap(bm);
    }
}


static long Renderbody(struct Body *bd,struct Amrender *amr)
{  struct Coords *coo;
   struct Element *child;
   struct Line *line;
   UWORD flags=amr->flags&~AMRF_CLEAR;
   short bgcolor;
   void *bgimage;
   struct Aobject *bgalign;


   struct RastPort *temprp = NULL;
   struct RastPort *coorp;


   long y;

   /* prevent multiple rendering by putting the value of bgupdate in to bd->bgupdate */

   bd->bgupdate = bgupdate;

   /* If AMRF_CHANGED, start with first child from rendery, but only if something
    * was changed.
    * Otherwise render everything. */
   child=bd->contents.first;
   y=0;
   if(amr->flags&AMRF_CHANGED)
   {  if(!(bd->flags&BDYF_CHANGEDCHILD)) return 0;
      if(amr->flags&AMRF_CLEAR) line=Findlinebefore(bd,bd->rendery,LINEF_MARGIN);
      else line=Findlinebefore(bd,bd->rendery,0);
      if(line)
      {  child=line->child;
         y=line->y;
      }
   }
   if((coo=Clipcoords(bd,amr->coords)) && coo->rp)
   {  bgcolor=coo->bgcolor;
      bgimage=coo->bgimage;
      bgalign=coo->bgalign;


      coorp = coo->rp;

      /* Try to set up temp rastport unless notemprast is set */

      if ( (!(amr->flags&AMRF_NOTEMPRAST)) && prefs.browser.temprp )
      {



          /*if we can allocate our temp bitmap replace the coo->rp,otherwise go with the original */

          if(temprp = Alloctemprp(coorp))
          {
             temprp->DrawMode = coorp->DrawMode;
             temprp->FgPen = coorp->FgPen;
             coo->rp = temprp;
          }
      }

      if(prefs.browser.docolors && (bd->bgcolor>=0 || bd->bgimage)
      && !(bd->flags&BDYF_NOBACKGROUND))
      {  if(bd->bgcolor>=0)
         {  coo->bgcolor=bd->bgcolor;
         }
         coo->bgimage=bd->bgimage;
         coo->bgalign=bd->bgalign;
      }

      if(!prefs.browser.docolors && (bd->flags&BDYF_FORCEBGCOLOR) && bd->bgcolor>=0)
      {  coo->bgcolor=bd->bgcolor;
      }
      /* Clear background only if CLEAR requested,
       * or if CLEARBG requested and this body has a different background
       * i.e. it is a table cell body, so frame!=cframe. */
      if((amr->flags&AMRF_CLEAR)
      || ((amr->flags&AMRF_CLEARBG) && (bd->bgimage || bd->bgcolor>=0) &&
            (bd->frame!=bd->cframe) && !(bd->flags&BDYF_NOBACKGROUND)))
      {  if(amr->flags&AMRF_CHANGED)
         {  while(child->object.next && !Agetattr((struct Aobject *)child,AOELT_Visible)) child=child->object.next;
            if(child->object.next) y+=Agetattr((struct Aobject *)child,AOELT_Incrementaly);
         }

         Erasebg(bd->cframe,coo,amr->rect.minx,MAX(y,amr->rect.miny),amr->rect.maxx,amr->rect.maxy);
         flags|=AMRF_CLEARBG;
      }

      flags |= AMRF_NOTEMPRAST;

      for(;child->object.next;child=child->object.next)
      {  if(child->aox<=amr->rect.maxx && child->aox+child->aow>amr->rect.minx
         && child->aoy<=amr->rect.maxy && child->aoy+child->aoh>amr->rect.miny)

         /* ensure our children don't set a temp rast port with AMRF_NOTEMPRAST*/

         {  Arender((struct Aobject *)child,coo,amr->rect.minx,amr->rect.miny,amr->rect.maxx,amr->rect.maxy,(ULONG)flags,amr->text);
         }
      }

      /* if were using our own rastport and bitmap */

      if (temprp)
      {

         /* only blit back if y < amr.rect.maxy */
         /* ie not if all "rendering" was below screen */

         if (y <= amr->rect.maxy)
         {
            ClipBlit(temprp,coo->dx + amr->rect.minx, coo->dy + MAX(y,amr->rect.miny),coorp,
                             coo->dx + amr->rect.minx,coo->dy+MAX(y,amr->rect.miny), amr->rect.maxx-amr->rect.minx + 1, amr->rect.maxy-MAX(y,amr->rect.miny)+1,0xc0);
         }
         Freetemprp(temprp);
      }

      coo->bgcolor=bgcolor;
      coo->bgimage=bgimage;
      coo->bgalign=bgalign;

      coo->rp = coorp;

      Unclipcoords(coo);
      if(bd->flags&BDYF_LAYOUTREADY)
      {  bd->flags&=~BDYF_CHANGEDCHILD;
      }
      bd->chchild=NULL;
   }
   return 0;
}

static long Setbody(struct Body *bd,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   UWORD fontw=0;
   short fontstyle=0,fontsize=0;
   struct Colorinfo *fontcolor=NULL;
   UBYTE *fontface=NULL;
   BOOL setframe=FALSE,setwin=FALSE,setwhis=FALSE;
   void *whis=0;
   struct Element *child;

   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Left:
            bd->aox=tag->ti_Data;
            break;
         case AOBJ_Top:
            bd->aoy=tag->ti_Data;
            break;
         case AOBJ_Width:
            bd->aow=tag->ti_Data;
            break;
         case AOBJ_Height:
            bd->aoh=tag->ti_Data;
            break;
         case AOBJ_Cframe:
            bd->cframe=(void *)tag->ti_Data;
            break;
         case AOBJ_Frame:
            /* AOBJ_Frame is set/cleared when in/out display.
             * Forward it to children, with ourself as Cframe */
            bd->frame=(void *)tag->ti_Data;
            setframe=TRUE;
            /* Let layout and render start from top */
            Removelinesbelow(bd,-1);
            bd->rendery=0;
            bd->flags|=BDYF_CHANGEDCHILD;
            bd->flags&=~BDYF_LAYOUTREADY;
            break;
         case AOBJ_Window:
            bd->win=(void *)tag->ti_Data;
            setwin=TRUE;
            break;
         case AOBJ_Winhis:
            whis=(void *)tag->ti_Data;
            setwhis=TRUE;
            break;
         case AOBJ_Nobackground:
            SETFLAG(bd->flags,BDYF_NOBACKGROUND,tag->ti_Data);
            break;

         case AOBJ_Changedbgimage:
            if (bd->parent && tag->ti_Data)
            {
                Asetattrs(bd->parent,AOBJ_Changedbgimage,tag->ti_Data,TAG_END);
            }
            break;

         case AOBDY_Sethardstyle:
            if(bd->bld) bd->bld->hardstyle|=tag->ti_Data;
            break;
         case AOBDY_Unsethardstyle:
            if(bd->bld) bd->bld->hardstyle&=~tag->ti_Data;
            break;
         case AOBDY_Align:
            if(bd->bld)
            {  if((short)tag->ti_Data<0)
               {  bd->bld->align=bd->bld->divalign;
               }
               else
               {  bd->bld->align=tag->ti_Data;
               }
            }
            break;
         case AOBDY_Divalign:
            if(bd->bld)
            {  if((short)tag->ti_Data<0)
               {  bd->bld->align=bd->bld->divalign=HALIGN_LEFT;
               }
               else
               {  bd->bld->align=bd->bld->divalign=tag->ti_Data;
               }
            }
            break;
         case AOBJ_Pool:
            bd->pool=(void *)tag->ti_Data;
            break;
         case AOBDY_Style:
            fontstyle=tag->ti_Data;
            fontw|=FONTW_STYLE;
            break;
         case AOBDY_Fixedfont:
            if(bd->bld) bd->bld->fonttype=BOOLVAL(tag->ti_Data);
            break;
         case AOBDY_Fontsize:
            fontsize=tag->ti_Data;
            fontw|=FONTW_ABSSIZE;
            break;
         case AOBDY_Fontsizerel:
            fontsize=tag->ti_Data;
            fontw|=FONTW_RELSIZE;
            break;
         case AOBDY_Fontcolor:
            fontcolor=(struct Colorinfo *)tag->ti_Data;
            fontw|=FONTW_COLOR;
            break;
         case AOBDY_Fontface:
            fontface=(UBYTE *)tag->ti_Data;
            fontw|=FONTW_FACE;
            break;
         case AOBDY_Fontend:
            Popfont(bd);
            break;
         case AOBDY_Basefont:
            Setbasefont(bd,tag->ti_Data,FALSE);
            break;
         case AOBDY_Basefontrel:
            Setbasefont(bd,tag->ti_Data,TRUE);
            break;
         case AOBDY_Basecolor:
            Setbasecolor(bd,(struct Colorinfo *)tag->ti_Data);
            break;
         case AOBDY_Baseface:
            Setbaseface(bd,(UBYTE *)tag->ti_Data);
            break;
         case AOBDY_Subscript:
            if(tag->ti_Data) bd->flags|=BDYF_SUB;
            else bd->flags&=~BDYF_SUB;
            break;
         case AOBDY_Superscript:
            if(tag->ti_Data) bd->flags|=BDYF_SUP;
            else bd->flags&=~BDYF_SUP;
            break;
         case AOBDY_Link:
            if(bd->bld) bd->bld->link=(void *)tag->ti_Data;
            break;
         case AOBJ_Layoutparent:
            bd->parent=(void *)tag->ti_Data;
            break;
         case AOBJ_Changedchild:
            child=(struct Element *)tag->ti_Data;
            Removelinesbelow(bd,child->aoy);
            Setchchild(bd,child);
            bd->flags|=BDYF_CHANGEDCHILD;
            bd->flags&=~BDYF_LAYOUTREADY;
            Asetattrs(bd->parent,AOBJ_Changedchild,(Tag)bd,TAG_END);
            Changedlayout();
            break;
         case AOBDY_Blockquote:
            if(bd->bld)
            {  if(tag->ti_Data)
               {  bd->bld->bqindent++;
               }
               else if(bd->bld->bqindent)
               {  bd->bld->bqindent--;
               }
            }
            break;
         case AOBDY_List:
            if(bd->bld)
            {  if(tag->ti_Data)
               {  Addlist(bd,(struct Listinfo *)tag->ti_Data);
               }
               else
               {  Remlist(bd);
               }
            }
            break;
         case AOBDY_Dterm:
            Setdterm(bd,BOOLVAL(tag->ti_Data));
            break;
         case AOBDY_Leftmargin:
            bd->hmargin=MAX(0,(short)tag->ti_Data);
            break;
         case AOBDY_Topmargin:
            bd->vmargin=MAX(0,(short)tag->ti_Data);
            break;
         case AOBDY_Bgcolor:
            bd->bgcolor=(short)tag->ti_Data;
            Removelinesbelow(bd,-1);
            bd->flags|=BDYF_CHANGEDCHILD;
            bd->flags&=~(BDYF_FORCEBGCOLOR|BDYF_LAYOUTREADY);
            break;
         case AOBDY_Forcebgcolor:
            bd->bgcolor=(short)tag->ti_Data;
            Removelinesbelow(bd,-1);
            bd->flags|=BDYF_CHANGEDCHILD|BDYF_FORCEBGCOLOR;
            bd->flags&=~BDYF_LAYOUTREADY;
            break;
         case AOBDY_Bgimage:
            bd->bgimage=(void *)tag->ti_Data;

            /* FIXME: maybe this block should be a function */

            if(bd->bgimage)
            {
                void *framecopy;
                struct Document *doc;
                struct Bguser   *bgu;
                struct Bgimage  *bgi;

                /* get the document we belong too ... */
                /* but abort if we sometimes don't have a frame */
                /* NOTE: at sometime investigate just why this can happen */

                if(bd->frame)
                {
                    framecopy = (void *)Agetattr((struct Aobject*)bd->frame,AOFRM_Copy);
                    doc  = (struct Document *)Agetattr((struct Aobject*)framecopy,AOCPY_Driver);
                    for(bgi=doc->bgimages.first;bgi->next;bgi=bgi->next)
                    {
                        if((void *)tag->ti_Data == bgi->copy)
                        {
                            if((bgu = ALLOCSTRUCT(Bguser,1,MEMF_CLEAR)))
                            {
                                bgu->user = bd;
                                ADDTAIL(&bgi->bgusers,bgu);
                            }
                            break;
                        }
                    }
                }


            }
            /* question: why should changing bground require a relayout etc? */

            Removelinesbelow(bd,-1);
            bd->flags|=BDYF_CHANGEDCHILD;
            bd->flags&=~BDYF_LAYOUTREADY;
            break;
         case AOBDY_Bgalign:
            bd->bgalign=(struct Aobject *)tag->ti_Data;
            /* investigate what else may be necessry here*/
            break;
         case AOBDY_Nobr:
            SETFLAG(bd->flags,BDYF_NOBR,tag->ti_Data);
            break;
         case AOBDY_End:
            Disposebodybuild(bd);
            break;
         case AOBDY_Tcell:
            bd->tcell = (void *)tag->ti_Data;
            break;
      }
   }
   if(fontw) Pushfont(bd,fontstyle,fontsize,fontcolor,fontface,fontw);
   if(setframe || setwin || setwhis)
   {  for(child=bd->contents.first;child->object.next;child=child->object.next)
      {  Asetattrs((struct Aobject *)child,
            setframe?AOBJ_Cframe:TAG_IGNORE,bd->frame?(Tag)bd:(Tag)NULL,
            setframe?AOBJ_Frame:TAG_IGNORE,(Tag)bd->frame,
            setwin?AOBJ_Window:TAG_IGNORE,(Tag)bd->win,
            setwhis?AOBJ_Winhis:TAG_IGNORE,(Tag)whis,
            TAG_END);
      }
   }
   return 0;
}

static void Disposebody(struct Body *bd)
{  void *p;
   while(p=REMHEAD(&bd->contents)) Adisposeobject(p);
   while(p=REMHEAD(&bd->lines)) FREE(p);
   while(p=REMHEAD(&bd->leftmargins)) FREE(p);
   while(p=REMHEAD(&bd->rightmargins)) FREE(p);
   while(p=REMHEAD(&bd->openfonts)) Freeopenfont(p);
   if(bd->bld) Disposebodybuild(bd);
   Amethodas(AOTP_OBJECT,bd,AOM_DISPOSE);
}

static struct Body *Newbody(struct Amset *ams)
{  struct Body *bd;
   if(bd=Allocobject(AOTP_BODY,sizeof(struct Body),ams))
   {  NEWLIST(&bd->contents);
      NEWLIST(&bd->lines);
      NEWLIST(&bd->leftmargins);
      NEWLIST(&bd->rightmargins);
      NEWLIST(&bd->openfonts);
      if(Newbodybuild(bd))
      {  Pushfont(bd,STYLE_NORMAL,0,NULL,NULL,FONTW_STYLE);
         bd->bgcolor=-1;
         Setbody(bd,ams);
      }
      else
      {  Disposebody(bd);
         bd=NULL;
      }
   }
   return bd;
}

static long Getbody(struct Body *bd,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Left:
            PUTATTR(tag,bd->aox);
            break;
         case AOBJ_Top:
            PUTATTR(tag,bd->aoy);
            break;
         case AOBJ_Width:
            PUTATTR(tag,bd->aow);
            break;
         case AOBJ_Height:
            PUTATTR(tag,bd->aoh);
            break;
         case AOBJ_Cframe:
            PUTATTR(tag,bd->cframe);
            break;
         case AOBJ_Frame:
            PUTATTR(tag,bd->frame);
            break;
         case AOBJ_Layoutparent:
            PUTATTR(tag,bd->parent);
            break;
         case AOBDY_List:
            if(!bd->bld || ISEMPTY(&bd->bld->list))
            {  PUTATTR(tag,&nolist);
            }
            else
            {  PUTATTR(tag,bd->bld->list.first);
            }
            break;
         case AOBDY_Style:
            PUTATTR(tag,bd->bld->hardstyle|bd->bld->font.first->style);
            break;
         case AOBDY_Tcell:
            PUTATTR(tag,bd->tcell);
            break;
         case AOBDY_Bgupdate:
            PUTATTR(tag,bd->bgupdate);
            break;
      }
   }
   return 0;
}

static long Hittestbody(struct Body *bd,struct Amhittest *amh)
{  long result=0;
   struct Coords *coo,coords={0};
   struct Element *child;
   long x,y;
   if(!(coo=amh->coords))
   {  Framecoords(bd->cframe,&coords);
      coo=&coords;
   }
   if(coo->win)
   {  x=amh->xco-coo->dx;
      y=amh->yco-coo->dy;
      for(child=bd->contents.first;child->object.next;child=child->object.next)
      {  if(y>=child->aoy && y<child->aoy+child->aoh
         && x>=child->aox && x<child->aox+child->aow)
         {  result=Ahittest((struct Aobject *)child,coo,amh->xco,amh->yco,amh->flags,amh->oldobject,amh->amhr);
            if(result) break;
         }
      }
   }
   return result;
}

static long Addchild(struct Body *bd,struct Amadd *ama)
{  struct Fontprefs *fp,*supfp=NULL;
   struct Openfont *of,*supof=NULL;
   UWORD style;
   struct Colorinfo *ci;
   short valign=-1;
   if(bd->bld && ama->child)
   {  Getcurrentfont(bd,&fp,&style,&ci);
      of=Addopenfont(bd,fp->font);
      if(bd->flags&BDYF_SUB) valign=VALIGN_SUB;
      else if(bd->flags&BDYF_SUP)
      {  valign=VALIGN_SUP;
         supfp=Getfontprefs(bd,bd->bld->font.first->next,NULL);
         supof=Addopenfont(bd,supfp->font);
      }
      Asetattrs(ama->child,
         AOELT_Link,(Tag)bd->bld->link,
         AOELT_Font,(Tag)of->font,
         AOELT_Style,style,
         AOELT_Defhalign,bd->bld->align,
         AOELT_Color,(Tag)ci,
         AOELT_Leftindent,LEFTINDENT(bd),
         AOELT_Rightindent,RIGHTINDENT(bd),
         AOELT_Nobr,BOOLVAL(bd->flags&BDYF_NOBR),
         AOBJ_Frame,(Tag)bd->frame,
         AOBJ_Cframe,bd->frame?(Tag)bd:(Tag)NULL,
         AOBJ_Window,(Tag)bd->win,
         (valign>=0)?AOELT_Valign:TAG_IGNORE,valign,
         supof?AOELT_Supfont:TAG_IGNORE,supof?(Tag)supof->font:(Tag)NULL,
         AOBJ_Layoutparent,(Tag)bd,
         AOBJ_Nobackground,BOOLVAL(bd->flags&BDYF_NOBACKGROUND),
         TAG_END);

      ADDTAIL(&bd->contents,ama->child);
      /* Set changed child flag so layout CHANGED will do something. */
      bd->flags|=BDYF_CHANGEDCHILD;
      bd->flags&=~BDYF_LAYOUTREADY;
   }

   return 0;
}

static long Movebody(struct Body *bd,struct Ammove *amm)
{  struct Element *child;
   bd->aox+=amm->dx;
   bd->aoy+=amm->dy;
   for(child=bd->contents.first;child->object.next;child=child->object.next)
   {  AmethodA((struct Aobject *)child,(struct Amessage *)amm);
   }
   return 0;
}

static long Notifybody(struct Body *bd,struct Amnotify *amn)
{  struct Element *child;
   for(child=bd->contents.first;child->object.next;child=child->object.next)
   {  AmethodA((struct Aobject *)child,(struct Amessage *)amn);
   }
   return 0;
}

static long Searchposbody(struct Body *bd,struct Amsearch *ams)
{  struct Element *child;
   long result=0;
   for(child=bd->contents.first;child->object.next;child=child->object.next)
   {  if(ams->top>=child->aoy && ams->top<child->aoy+child->aoh) break;
   }
   for(;!result && child->object.next;child=child->object.next)
   {  result=AmethodA((struct Aobject *)child,(struct Amessage *)ams);
   }
   return result;
}

static long Searchsetbody(struct Body *bd,struct Amsearch *ams)
{  struct Element *child;
   long result=0;
   for(child=bd->contents.first;!result && child->object.next;child=child->object.next)
   {  result=AmethodA((struct Aobject *)child,(struct Amessage *)ams);
   }
   return result;
}

static long Dragtestbody(struct Body *bd,struct Amdragtest *amd)
{  long result=0;
   struct Element *child;
   long x,y;
   BOOL hit;
   if(amd->coords && amd->coords->win)
   {  x=amd->xco-amd->coords->dx;
      y=amd->yco-amd->coords->dy;
      for(child=bd->contents.first;child->object.next && !result;child=child->object.next)
      {  hit=FALSE;
         if(child->aoy<=y && child->aoy+child->aoh>y)
         {  if(child->object.objecttype==AOTP_BREAK || child->aox+child->aow>x) hit=TRUE;
         }
         else if(child->aoy>y) hit=TRUE;
         if(hit)
         {  result=AmethodA((struct Aobject *)child,(struct Amessage *)amd);
            /* In case of floating right object, the test on x above could pick the
             * wrong child. So if child is floating right, revert AMDR_STOP to AMDR_NOHIT. */
            if((child->halign&HALIGN_FLOATRIGHT)==HALIGN_FLOATRIGHT && result==AMDR_STOP)
            {  result=AMDR_NOHIT;
            }
         }
      }
   }
   return result;
}

static long Dragrenderbody(struct Body *bd,struct Amdragrender *amd)
{  long result=0;
   struct Element *child;
   short bgcolor;
   void *bgimage;
   struct Aobject *bgalign;
   if(amd->coords && amd->coords->win)
   {  bgcolor=amd->coords->bgcolor;
      bgimage=amd->coords->bgimage;
      bgalign=amd->coords->bgalign;
      if(prefs.browser.docolors && (bd->bgcolor>=0 || bd->bgimage))
      {  if(bd->bgcolor>=0)
         {  amd->coords->bgcolor=bd->bgcolor;
         }
         amd->coords->bgimage=bd->bgimage;
         amd->coords->bgalign=bd->bgalign;
      }
      if(!prefs.browser.docolors && (bd->flags&BDYF_FORCEBGCOLOR) && bd->bgcolor>=0)
      {  amd->coords->bgcolor=bd->bgcolor;
      }
      for(child=bd->contents.first;child->object.next && amd->state!=AMDS_DONE;child=child->object.next)
      {  result=AmethodA((struct Aobject *)child,(struct Amessage *)amd);
      }
      amd->coords->bgcolor=bgcolor;
      amd->coords->bgimage=bgimage;
      amd->coords->bgalign=bgalign;
   }
   return result;
}

static long Dragcopybody(struct Body *bd,struct Amdragcopy *amd)
{  long result=0;
   struct Element *child;
   for(child=bd->contents.first;child->object.next && amd->state!=AMDS_DONE;child=child->object.next)
   {  result=AmethodA((struct Aobject *)child,(struct Amessage *)amd);
   }
   return result;
}

static long Jsetupbody(struct Body *bd,struct Amjsetup *amj)
{  struct Element *child;
   for(child=bd->contents.first;child->object.next;child=child->object.next)
   {  AmethodA((struct Aobject *)child,(struct Amessage *)amj);
   }
   return 0;
}

USRFUNC_H2
(
static long  , Body_Dispatcher,
struct Body *,bd,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newbody((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setbody(bd,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getbody(bd,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measurebody(bd,(struct Ammeasure *)amsg);
         break;
      case AOM_LAYOUT:
         result=Layoutbody(bd,(struct Amlayout *)amsg);
         break;
      case AOM_RENDER:
         result=Renderbody(bd,(struct Amrender *)amsg);
         break;
      case AOM_HITTEST:
         result=Hittestbody(bd,(struct Amhittest *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchild(bd,(struct Amadd *)amsg);
         break;
      case AOM_MOVE:
         result=Movebody(bd,(struct Ammove *)amsg);
         break;
      case AOM_NOTIFY:
         result=Notifybody(bd,(struct Amnotify *)amsg);
         break;
      case AOM_SEARCHPOS:
         result=Searchposbody(bd,(struct Amsearch *)amsg);
         break;
      case AOM_SEARCHSET:
         result=Searchsetbody(bd,(struct Amsearch *)amsg);
         break;
      case AOM_DRAGTEST:
         result=Dragtestbody(bd,(struct Amdragtest *)amsg);
         break;
      case AOM_DRAGRENDER:
         result=Dragrenderbody(bd,(struct Amdragrender *)amsg);
         break;
      case AOM_DRAGCOPY:
         result=Dragcopybody(bd,(struct Amdragcopy *)amsg);
         break;
      case AOM_JSETUP:
         result=Jsetupbody(bd,(struct Amjsetup *)amsg);
         break;
      case AOM_DISPOSE:
         Disposebody(bd);
         break;
      case AOM_DEINSTALL:
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installbody(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_BODY,(Tag)Body_Dispatcher)) return FALSE;
   return TRUE;
}

void *Bodyframe(struct Body *bd)
{  if(bd && bd->object.objecttype==AOTP_BODY)
   {  return bd->frame;
   }
   else
   {  return bd;
   }
}

void Bodycoords(struct Body *bd,struct Coords *coo)
{  if(bd->object.objecttype==AOTP_BODY)
   {  Framecoords(bd->cframe,coo);
      if(prefs.browser.docolors && (bd->bgcolor>=0 || bd->bgimage)
      && !(bd->flags&BDYF_NOBACKGROUND))
      {  if(bd->bgcolor>=0)
         {  coo->bgcolor=bd->bgcolor;
         }
         coo->bgimage=bd->bgimage;
         coo->bgalign=bd->bgalign;
      }
      if(!prefs.browser.docolors && (bd->flags&BDYF_FORCEBGCOLOR) && bd->bgcolor>=0)
      {  coo->bgcolor=bd->bgcolor;
      }
   }
   else
   {  Framecoords((struct Frame *)bd,coo);
   }
}
