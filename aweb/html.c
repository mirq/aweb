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

/* html.c - AWeb document html engine */

#include "aweb.h"
#include "application.h"
#include "source.h"
#include "html.h"
#include "colours.h"
#include "docprivate.h"
#include "body.h"
#include "frameset.h"
#include "text.h"
#include "break.h"
#include "ruler.h"
#include "link.h"
#include "frame.h"
#include "copy.h"
#include "bullet.h"
#include "table.h"
#include "map.h"
#include "area.h"
#include "form.h"
#include "field.h"
#include "button.h"
#include "checkbox.h"
#include "radio.h"
#include "input.h"
#include "select.h"
#include "textarea.h"
#include "filefield.h"
#include "url.h"
#include "window.h"
#include "jslib.h"

struct Number  /* number or percentage */
{  long n;     /* value */
   short type; /* see below */
};

#define NUMBER_NONE     0
#define NUMBER_NUMBER   1
#define NUMBER_PERCENT  2
#define NUMBER_SIGNED   3
#define NUMBER_RELATIVE 4


#define ATTR(doc,ta)          ((doc)->args.buffer+(ta)->valuepos)
#define ATTREQUAL(doc,ta,v)   STRIEQUAL(ATTR(doc,ta),v)
#define CONDTAG(tag,v)        (((v)>=0)?(tag):TAG_IGNORE),(v)

#define Wantbreak(doc,n) ((doc->wantbreak<(n))?doc->wantbreak=(n):0)

#define STRICT (doc->htmlmode==HTML_STRICT)

static struct Tagattr dummyta={0};

#define DINGBATPATH  "file:///AWebPath:Images/"

#ifdef DEVELOPER
extern BOOL charsetdebug;
#endif
/*---------------------------------------------------------------------*/

/* Add text to text buffer. If buffer is too small, and we are somewhere
   in the source buffer, compute expected text buffer size as:
      (source size) * (text size) / (source position)
   assuming that the ratio source/text will be constant. */
static BOOL Addtotextbuf(struct Document *doc,UBYTE *text,long len)
{  long size;
   if(doc->text.length+len>doc->text.size)
   {  struct Buffer *src=&doc->source->buf;
      if(doc->text.size && doc->srcpos)
      {  size=((doc->text.size<<8)/doc->srcpos)*(src->size>>8);
      }
      else
      {  size=src->size/2;
      }
      if(!Expandbuffer(&doc->text,size-doc->text.length)) return FALSE;
   }
   return Addtobuffer(&doc->text,text,len);
}

/* Make sure that the last position in the text buffer is a space */
static BOOL Ensuresp(struct Document *doc)
{  return (BOOL)(doc->text.buffer[doc->text.length-1]==' '
      || Addtotextbuf(doc," ",1));
}

/* Make sure that the last position in the text buffer is not a space */
static BOOL Ensurenosp(struct Document *doc)
{  return (BOOL)(doc->text.buffer[doc->text.length-1]!=' '
      || Addtotextbuf(doc,".",1));
}

/* Make sure that document has a child, create body if it is still empty */
static BOOL Ensurebody(struct Document *doc)
{  if(doc->doctype==DOCTP_NONE)
   {  if(!(doc->body=Anewobject(AOTP_BODY,
         AOBJ_Pool,(Tag)doc->pool,
         AOBJ_Frame,(Tag)doc->frame,
         AOBJ_Cframe,(Tag)doc->frame,
         AOBJ_Window,(Tag)doc->win,
         AOBJ_Layoutparent,(Tag)doc,
         AOBJ_Nobackground,BOOLVAL(doc->dflags&DDF_NOBACKGROUND),
         AOBDY_Leftmargin,doc->hmargin,
         AOBDY_Topmargin,doc->vmargin,
         TAG_END)))
      {  return FALSE;
      }
      doc->doctype=DOCTP_BODY;
   }
   return TRUE;
}

/* Return the current body (could be button or table member) */
static void *Docbody(struct Document *doc)
{  void *body=NULL;
   if(doc->button) body=(void *)Agetattr(doc->button,AOBUT_Body);
   if(!body && !ISEMPTY(&doc->tables))
   {  body=(void *)Agetattr(doc->tables.first->table,AOTAB_Body);
   }
   if(!body) body=doc->body;
   return body;
}

/* Return the current body (could be button or table member) but don't create a new one */
static void *Docbodync(struct Document *doc)
{  void *body=NULL;
   if(doc->button) body=(void *)Agetattr(doc->button,AOBUT_Body);
   if(!body && !ISEMPTY(&doc->tables))
   {  body=(void *)Agetattr(doc->tables.first->table,AOTAB_Bodync);
   }
   if(!body) body=doc->body;
   return body;
}

/* Create as much line break elements as needed to fulfill (wantbreak) */
static BOOL Solvebreaks(struct Document *doc)
{  void *br;
   for(doc->wantbreak-=doc->gotbreak;doc->wantbreak>0;doc->wantbreak--)
   {  if(!(br=Anewobject(AOTP_BREAK,
         AOBJ_Pool,(Tag)doc->pool,
         AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
         TAG_END))) return FALSE;
      Aaddchild(Docbody(doc),br,0);
      doc->gotbreak++;
   }
   doc->pflags&=~DPF_BULLET;
   return TRUE;
}

/* Add element to document contents. If no contents yet, create a body. */
static BOOL Addelement(struct Document *doc,void *elt)
{  void *body;
   if(!Ensurebody(doc))
   {  Adisposeobject(elt);
      return FALSE;
   }
   if(doc->doctype==DOCTP_BODY)
   {  if(Agetattr(elt,AOELT_Visible))
      {  Solvebreaks(doc);
         doc->gotbreak=0;
         doc->wantbreak=0;
         body=Docbody(doc);
      }
      else
      {  body=Docbodync(doc);
      }
      if(body)
      {  Aaddchild(body,elt,0);
      }
      else
      {  Adisposeobject(elt);
         return FALSE;
      }
   }
   else if(doc->doctype==DOCTP_FRAMESET && doc->framesets.first->next)
   {  Aaddchild(doc->framesets.first->frameset,elt,0);
   }
   else
   {  Adisposeobject(elt);
      return FALSE;
   }
   return TRUE;
}

/* Check for ID attribute. Scan ta list both ways since this function can be
 * called before or after the list was scanned in the tag function itself */
static void Checkid(struct Document *doc,struct Tagattr *tap)
{  struct Tagattr *ta;
   UBYTE *name=NULL;
   if(doc->doctype==DOCTP_BODY && tap)
   {  for(ta=tap->prev;ta && ta->prev && !name;ta=ta->prev)
      {  if(ta->attr==TAGATTR_ID) name=ATTR(doc,ta);
      }
      for(ta=tap;ta && ta->next && !name;ta=ta->next)
      {  if(ta->attr==TAGATTR_ID) name=ATTR(doc,ta);
      }
      if(name)
      {  void *elt;
         struct Fragment *frag;
         Solvebreaks(doc);
         if(elt=Anewobject(AOTP_NAME,
            AOBJ_Pool,(Tag)doc->pool,
            TAG_END))
         {  Addelement(doc,elt);
            if(frag=PALLOCSTRUCT(Fragment,1,0,doc->pool))
            {  frag->name=Dupstr(name,-1);
               frag->elt=elt;
               ADDTAIL(&doc->fragments,frag);
            }
         }
      }
   }
}

/* Remember a new nested table */
static BOOL Pushtable(struct Document *doc,void *table)
{  struct Tableref *tref;
   if(!(tref=PALLOCSTRUCT(Tableref,1,MEMF_CLEAR,doc->pool))) return FALSE;
   tref->table=table;
   ADDHEAD(&doc->tables,tref);
   return TRUE;
}

/* Forget inner nested table */
static void Poptable(struct Document *doc)
{  struct Tableref *tref;
   if(tref=(struct Tableref *)REMHEAD(&doc->tables)) FREE(tref);
}

/* Remember a new nested frameset */
static BOOL Pushframeset(struct Document *doc,void *frameset)
{  struct Framesetref *fref;
   if(!(fref=PALLOCSTRUCT(Framesetref,1,MEMF_CLEAR,doc->pool))) return FALSE;
   fref->frameset=frameset;
   ADDHEAD(&doc->framesets,fref);
   return TRUE;
}

/* Forget inner nested frameset but leave at least one. */
static void Popframeset(struct Document *doc)
{  struct Framesetref *fref=doc->framesets.first;
   if(fref->next && fref->next->next)
   {  Asetattrs(fref->frameset,AOFRS_Endframeset,TRUE,TAG_END);
      REMOVE(fref);
      FREE(fref);
   }
   else
   {  doc->pflags|=DPF_FRAMESETEND;
   }
}

/* Scan (p) for numeric value, possibly signed or percentage or relative. */
static long Getnumber(struct Number *num,UBYTE *p)
{  char c='\0';
   int m = 0;
   BOOL sign=FALSE;
   num->n=0;
   num->type=NUMBER_NONE;
   while(isspace(*p)) p++;
   sign=(*p=='+' || *p=='-');
   if(*p=='*')
   {  num->n=1;
      c='*';
      m=1;
   }
   else
   {  m=sscanf(p," %ld%c",&num->n,&c);
   }
   if(m>0)
   {  if(c=='%') num->type=NUMBER_PERCENT;
      else if(c=='*') num->type=NUMBER_RELATIVE;
      else if(sign) num->type=NUMBER_SIGNED;
      else num->type=NUMBER_NUMBER;
   }
   if(num->type!=NUMBER_SIGNED && num->n<0) num->n=0;
   return num->n;
}

/* Scan (p) for positive (>=0) numeric value */
static long Getposnumber(UBYTE *p)
{  struct Number num = {0};
   Getnumber(&num,p);
   if(num.type==NUMBER_NUMBER) return num.n;
   else return -1;
}

/* Scan (p) for hexadecimal number and put result in (ptr).
 * Parse things like ff ff ff or ff,0,ff or even ff,fff,ff right.
 * Convert a 3-digit number to 6 digits: abc -> aabbcc. */
static void Scanhexnumber(UBYTE *p,ULONG *ptr,BOOL strict)
{  long n=0;
   long part[3]={0,0,0};
   short i,l=0;
   UBYTE c;
   for(i=0;i<3;i++)
   {  while(*p && (isspace(*p) || ispunct(*p))) p++;  /* skip separators */
      if(!*p) break;
      c=*p;
      if(!strict && (c=='o' || c=='O')) c='0';
      if(!isxdigit(c)) return;
      while(isxdigit(c))               /* get partial number */
      {  part[i]=part[i]*0x10+(c<='9'?(c-'0'):(toupper(c)-'A'+10));
         c=*++p;
         if(!strict && (c=='o' || c=='O')) c='0';
         l++;
      }
   }
   if(i<3)
   {  /* one part, first part is number */
      if(l==3)
      {  /* 3-digit number, convert to 6 digits */
         n=((part[0]&0xf00)<<8)|((part[0]&0x0f0)<<4)|(part[0]&0x00f);
         n=n|(n<<4);
      }
      else n=part[0];
   }
   else n=((part[0]&0xff)<<16)|((part[1]&0xff)<<8)|(part[2]&0xff);
   *ptr=n;
}

/* Scan (p) for a color name or a hexadecimal color number,
 * and return the resulting RRGGBB value in (ptr) */
static void Gethexcolor(struct Document *doc,UBYTE *p,ULONG *ptr)
{  BOOL gotone=FALSE;
   if(*p=='#') Scanhexnumber(p+1,ptr,STRICT);
   else
   {  short a=0,b=NR_COLORNAMES,m;
      long c;
      UBYTE buf[32];
      UBYTE *q;
      for(q=p,m=0;*q && m<31;q++)
      {  if(!isspace(*q)) buf[m++]=*q;
      }
      buf[m]='\0';
      while(a<=b)
      {  m=(a+b)/2;
         c=stricmp(colornames[m].name,buf);
         if(c==0)
         {  if(!STRICT || colornames[m].strict)
            {  *ptr=colornames[m].color;
               gotone=TRUE;
            }
            break;
         }
         if(c<0) a=m+1;
         else b=m-1;
      }
      if(!gotone && !STRICT && *p) Scanhexnumber(p,ptr,STRICT);
   }
}

/* Scan (p) for a color designator, find a matching Colorinfo and store it
 * in (cip) */
BOOL Setbodycolor(struct Document *doc,struct Colorinfo **cip,UBYTE *p)
{  ULONG color=(ULONG)~0;
   Gethexcolor(doc,p,&color);
   if(color!=(ULONG)~0)
   {  if(!(*cip=Finddoccolor(doc,color))) return FALSE;
   }
   return TRUE;
}

/* Get horizontal alignment value */
static short Gethalign(UBYTE *p)
{  short align=-1;
   if(STRIEQUAL(p,"LEFT")) align=HALIGN_LEFT;
   else if(STRIEQUAL(p,"CENTER")) align=HALIGN_CENTER;
   else if(STRIEQUAL(p,"RIGHT")) align=HALIGN_RIGHT;
   return align;
}

/* Get vertictal alignment value */
static short Getvalign(UBYTE *p,BOOL strict)
{  short align=-1;
   if(STRIEQUAL(p,"TOP")) align=VALIGN_TOP;
   else if(STRIEQUAL(p,"MIDDLE")) align=VALIGN_MIDDLE;
   else if(STRIEQUAL(p,"BOTTOM")) align=VALIGN_BOTTOM;
   else if(!strict)
   {  if(STRIEQUAL(p,"TEXTTOP")) align=VALIGN_TOP;
      else if(STRIEQUAL(p,"ABSMIDDLE")) align=VALIGN_MIDDLE;
      else if(STRIEQUAL(p,"BASELINE")) align=VALIGN_BOTTOM;
      else if(STRIEQUAL(p,"ABSBOTTOM")) align=VALIGN_BOTTOM;
   }
   return align;
}

/* Get floating alignment value */
static short Getflalign(UBYTE *p)
{  short align=-1;
   if(STRIEQUAL(p,"LEFT")) align=HALIGN_FLOATLEFT;
   else if(STRIEQUAL(p,"RIGHT")) align=HALIGN_FLOATRIGHT;
   return align;
}

/* Get a boolean value */
static long Getbool(UBYTE *p,long defval)
{  switch(toupper(*p))
   {  case 'Y':
      case 'T':
      case '1':
         defval=TRUE;
         break;
      case 'N':
      case 'F':
      case '0':
         defval=FALSE;
         break;
   }
   return defval;
}

/* Convert (n) into roman number in (buf). Returns number of characters */
static long Makeroman(UBYTE *buf,long n,BOOL lower)
{  char *p=buf;
   char letter[]={ 'M','D','C','L','X','V','I' };
   long m=1000,i=0;
   n%=10000;
   while(n)
   {  if(i>0)
      {  if(n>=9*m)
         {  *p++=letter[i];
            *p++=letter[i-2];
            n-=9*m;
         }
         else
         {  if(n>=4*m && n<5*m)
            {  *p++=letter[i];
               n+=m;
            }
            if(n>=5*m)
            {  *p++=letter[i-1];
               n-=5*m;
            }
         }
      }
      while(n>=m)
      {  *p++=letter[i];
         n-=m;
      }
      i+=2;
      m/=10;
   }
   *p++=' ';
   *p='\0';
   if(lower)
   {  for(p=buf;*p;p++) *p=tolower(*p);
   }

   return (unsigned long)p - (unsigned long)buf;
}

/* Find the map descriptor with this name, or create a new one */
static void *Findmap(struct Document *doc,UBYTE *name)
{  struct Aobject *map;
   for(map=doc->maps.first;map->next;map=map->next)
   {  if(STRIEQUAL((UBYTE *)Agetattr(map,AOMAP_Name),name))
      {  return map;
      }
   }
   map=Anewobject(AOTP_MAP,
      AOBJ_Pool,(Tag)doc->pool,
      AOMAP_Name,(Tag)name,
      TAG_END);
   if(map) ADDTAIL(&doc->maps,map);
   return map;
}

/* Create the map descriptor for this external map definition */
static void *Externalmap(struct Document *doc,UBYTE *urlname)
{  void *url,*copy,*referer;
   void *map=NULL;
   UBYTE *mapname=Fragmentpart(urlname);
   if(mapname && *mapname)
   {  if(url=Findurl(doc->base,urlname,0))
      {  referer=(void *)Agetattr(doc->source->source,AOSRC_Url);
         if(url==referer)
         {  map=Findmap(doc,mapname);
         }
         else
         {  copy=Anewobject(AOTP_COPY,
               AOBJ_Pool,(Tag)doc->pool,
               AOCPY_Url,(Tag)url,
               AOCPY_Referer,(Tag)referer,
               AOCPY_Defaulttype,(Tag)"text/html",
               AOCPY_Reloadverify,(doc->pflags&DPF_RELOADVERIFY),
               AOCPY_Mapdocument,TRUE,
               TAG_END);
            if(copy)
            {  map=Anewobject(AOTP_MAP,
                  AOBJ_Pool,(Tag)doc->pool,
                  AOBJ_Window,(Tag)doc->win,
                  AOMAP_Name,(Tag)"",
                  AOMAP_Extcopy,(Tag)copy,
                  AOMAP_Extname,(Tag)mapname,
                  TAG_END);
               if(map)
               {  ADDTAIL(&doc->maps,map);
               }
               else
               {  Adisposeobject(copy);
               }
            }
         }
      }
   }
   return map;
}

/* Create the background image with this url */
static void *Backgroundimg(struct Document *doc,UBYTE *urlname)
{  void *url,*referer,*elt=NULL;
   struct Bgimage *bgi;
   if(!(url=Findurl(doc->base,urlname,0))) return NULL;
   for(bgi=doc->bgimages.first;bgi->next;bgi=bgi->next)
   {  if(bgi->url==url) return bgi->copy;
   }
   referer=(void *)Agetattr(doc->source->source,AOSRC_Url);
   if(url==referer) return NULL; /* Catch <BODY BACKGROUND=""> */
   elt=Anewobject(AOTP_COPY,
      AOBJ_Pool,(Tag)doc->pool,
      AOBJ_Layoutparent,(Tag)doc->body,
      AOBJ_Frame,(Tag)doc->frame,
      AOBJ_Cframe,(Tag)doc->frame,
      AOBJ_Window,(Tag)doc->win,
      AOCPY_Url,(Tag)url,
      AOCPY_Background,TRUE,
      AOCPY_Referer,(Tag)referer,
      AOCPY_Defaulttype,(Tag)"image/x-unknown",
      AOCPY_Reloadverify,(doc->pflags&DPF_RELOADVERIFY),
      TAG_END);
   if(bgi=ALLOCSTRUCT(Bgimage,1,MEMF_CLEAR))
   {  bgi->url=url;
      bgi->copy=elt;
      NEWLIST(&bgi->bgusers);
      ADDTAIL(&doc->bgimages,bgi);
   }
   return elt;
}

/* Add an infotext */
static void Addinfotext(struct Document *doc,UBYTE *name,UBYTE *content,void *link)
{  struct Infotext *it;
   long len=0;
   if(name) len+=strlen(name)+2;
   if(content) len+=strlen(content);
   if(it=ALLOCSTRUCT(Infotext,1,0))
   {  if(it->text=ALLOCTYPE(UBYTE,len+1,MEMF_CLEAR))
      {  if(name)
         {  strcpy(it->text,name);
            strcat(it->text,": ");
         }
         if(content)
         {  strcat(it->text,content);
         }
         it->link=link;
         ADDTAIL(&doc->infotexts,it);
      }
      else
      {  FREE(it);
      }
   }
}

/* Only process certain tagtypes for a map definition document */
static UWORD Mapdoctag(UWORD tagtype)
{  switch(tagtype&MARKUP_MASK)
   {  case MARKUP_MAP:
      case MARKUP_AREA:
      case MARKUP_STYLE:
      case MARKUP_SCRIPT:
         break;
      default:
         tagtype=0;
   }
   return tagtype;
}

/*--- header contents -------------------------------------------------*/

/*** <TITLE> ***/
static BOOL Dotitle(struct Document *doc)
{  if(!(Ensuresp(doc))) return FALSE;
   doc->titlepos=doc->text.length;
   doc->pmode=DPM_TITLE;
   return TRUE;
}

/*** text within title ***/
static BOOL Dotitleadd(struct Document *doc,struct Tagattr *ta)
{  return Addtotextbuf(doc,ATTR(doc,ta),ta->length);
}

/*** </TITLE> ***/
static BOOL Dotitleend(struct Document *doc)
{  UBYTE *p;
   doc->pmode=DPM_BODY;
   if(!Addtotextbuf(doc,"\0 ",2)) return FALSE;
   for(p=doc->text.buffer+doc->titlepos;*p;p++)
   {  if(!isspace(*p))
      {  doc->dflags|=DDF_TITLEVALID;
         break;
      }
   }
   return TRUE;
}

/*** <BASE> ***/
static BOOL Dobase(struct Document *doc,struct Tagattr *ta)
{  UBYTE *base=NULL,*target=NULL;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_HREF:
            if(STRICT) base=Dupstr(ATTR(doc,ta),-1);
            else base=Makeabsurl(doc->base,ATTR(doc,ta));
            if(base)
            {  if(doc->base) FREE(doc->base);
               doc->base=base;
            }
            break;
         case TAGATTR_TARGET:
            if(target=Dupstr(ATTR(doc,ta),-1))
            {  if(doc->target) FREE(doc->target);
               doc->target=target;
            }
            break;
      }
   }
   return TRUE;
}

/*** <LINK> ***/
static BOOL Dolink(struct Document *doc,struct Tagattr *ta)
{  UBYTE *rel=NULL,*title=NULL,*href;
   void *url=NULL;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_REL:
            rel=ATTR(doc,ta);
            break;
         case TAGATTR_HREF:
            href=ATTR(doc,ta);
            url=Findurl(doc->base,href,0);
            break;
         case TAGATTR_TITLE:
            title=ATTR(doc,ta);
            break;
      }
   }
   if(url)
   {  Addinfotext(doc,title?NULL:rel,title?title:(UBYTE *)Agetattr(url,AOURL_Url),url);
      Asetattrs(doc->copy,AOCPY_Infochanged,TRUE,TAG_END);
   }
   return TRUE;
}

/*** <META> ***/
static BOOL Dometa(struct Document *doc,struct Tagattr *ta)
{  UBYTE *httpequiv=NULL,*content=NULL,*name=NULL;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_HTTP_EQUIV:
            httpequiv=ATTR(doc,ta);
            break;
         case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_CONTENT:
            content=ATTR(doc,ta);
            break;
      }
   }
   if((httpequiv && content && STRIEQUAL(httpequiv,"REFRESH"))
   || (name && content && STRIEQUAL(name,"REFRESH")))
   {  if(doc->clientpull) FREE(doc->clientpull);
      doc->clientpull=Dupstr(content,-1);
      Asetattrs(doc->copy,AOURL_Clientpull,(Tag)content,TAG_END);
   }
   if(httpequiv && content && STRIEQUAL(httpequiv,"CONTENT-SCRIPT-TYPE"))
   {  SETFLAG(doc->pflags,DPF_SCRIPTJS,STRIEQUAL(content,"TEXT/JAVASCRIPT"));
   }
   if(httpequiv && content && STRIEQUAL(httpequiv,"CONTENT-TYPE"))
   {  UBYTE *p,*q;
      for(p=content;*p && *p!=';';p++);
      for(;*p && !STRNIEQUAL(p,"CHARSET=",8);p++);
      if(*p)
      {  for(p+=8;*p && isspace(*p);p++);
         if(*p=='"')
         {  p++;
            for(q=p;*q && *q!='"';q++);
            *q='\0';
         }
         else
         {  for(q=p;*q && !isspace(*q);q++);
            *q='\0';
         }
         SETFLAG(doc->dflags,DDF_FOREIGN,(*p && !STRIEQUAL(p,"ISO-8859-1")));
         if (*p)
         {
            doc->charset=Findcharset(p);
#ifdef DEVELOPER
            if (charsetdebug) printf("html.c/Dometa(): character set specification found: %s\n",p);
#endif
         }
      }
   }
   if(httpequiv) name=httpequiv;
   if(name && content)
   {  Addinfotext(doc,name,content,NULL);
      Asetattrs(doc->copy,AOCPY_Infochanged,TRUE,TAG_END);
   }
   return TRUE;
}

/*--- JavaScript ------------------------------------------------------*/

/*** <SCRIPT> ***/
static BOOL Doscript(struct Document *doc,struct Tagattr *ta)
{  UBYTE *language;
   UBYTE *src=NULL;
   BOOL isjs=FALSE;
   if(doc->pflags&DPF_SCRIPTJS) language="JavaScript";
   else language="";
   doc->pflags &=~DPF_QSCRIPT;
   doc->pflags &=~DPF_DQSCRIPT;

   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_LANGUAGE:
            language=ATTR(doc,ta);
            break;
         case TAGATTR_TYPE:
            if(STRIEQUAL(ATTR(doc,ta),"text/javascript")) language="JavaScript";
            else language="unknown";
            break;
         case TAGATTR_SRC:
            src=ATTR(doc,ta);
            break;
      }
   }
   if(prefs.browser.dojs)
   {  if(STRNIEQUAL(language,"LiveScript",10)) isjs=TRUE;
      else if(STRNIEQUAL(language,"JavaScript",10))
      {
         /*
         if(prefs.browser.dojs==DOJS_ALL) isjs=TRUE;
         else if(language[10]=='1')
         {  if(language[11]=='.')
            {  if(language[12]=='0' || language[12]=='1') isjs=TRUE;
            }
         }
         else if(!language[10]) isjs=TRUE;
         */
         /* new fastidious mode runs all javascript */
         isjs=TRUE;
      }
   }
   if(isjs)
   {  if(src)
      {  void *url;
         UBYTE *extsrc;
         url=Findurl(doc->base,src,0);
         if(extsrc=Finddocext(doc,url,
            (doc->pflags&DPF_RELOADVERIFY) && !(doc->pflags&DPF_NORLDOCEXT)))
         {  if(extsrc==(UBYTE *)~0)
            {  /* External source is in error, use element contents. */
               Freebuffer(&doc->jsrc);
               doc->pmode=DPM_BODY;
               doc->pflags|=DPF_JSCRIPT;
               doc->dflags|=DDF_NOSPARE;
               doc->jsrcline=Docslinenrfrompos(doc->source,doc->srcpos);
            }
            else
            {  /* Add external source to jsource buffer, skip element contents, and
                * execute the script when </SCRIPT> tag is encountered. */
               Addtobuffer(&doc->jsrc,extsrc,-1);
               doc->pmode=DPM_SCRIPT;
               doc->pflags|=DPF_JSCRIPT;
               doc->pflags&=~DPF_NORLDOCEXT;
               doc->jsrcline=-1; /* 1 will be added when passed to JS library */
            }
         }
         else
         {  /* External source not yet available. */
            doc->pflags|=DPF_SUSPEND|DPF_NORLDOCEXT;
            doc->jsrcline=Docslinenrfrompos(doc->source,doc->srcpos);
         }
      }
      else
      {  Freebuffer(&doc->jsrc);
         doc->pflags|=DPF_JSCRIPT;
         doc->dflags|=DDF_NOSPARE;
         doc->jsrcline=Docslinenrfrompos(doc->source,doc->srcpos);
      }
   }
   else
   {  doc->pmode=DPM_SCRIPT;
   }
   return TRUE;
}

/*** JavaScript source ***/
static BOOL Dojsource(struct Document *doc,struct Tagattr *ta)
{  if(ta->length>0)
   if(!Addtobuffer(&doc->jsrc,ATTR(doc,ta),ta->length)) return FALSE;
   ta=ta->next;
   if(ta && ta->next && ta->attr==TAGATTR_BR)
   {  if(!Addtobuffer(&doc->jsrc,"\n",1)) return FALSE;
   }
   return TRUE;
}

/*** </SCRIPT> ***/
/* Only called while processing JavaScript, not other scripts */
static BOOL Doscriptend(struct Document *doc)
{  long joutpos;
   struct Buffer jout;
   UBYTE *src;
   BOOL jrun,jparse;
   doc->pflags &=~DPF_QSCRIPT;
   doc->pflags &=~DPF_DQSCRIPT;

   if(doc->pflags&DPF_JSCRIPT)
   {  Addtobuffer(&doc->jsrc,"\0",1);
      src=Dupstr(doc->jsrc.buffer,-1);
      Freebuffer(&doc->jsrc);
      doc->pflags&=~DPF_JSCRIPT;

      /* Make stacked copy of current JS output, since the script may
       * document.write("<SCRIPT>") etc... */
      jout=doc->jout;
      joutpos=doc->joutpos;
      memset(&doc->jout,0,sizeof(doc->jout));
      doc->joutpos=0;
      jrun=BOOLVAL(doc->pflags&DPF_JRUN);
      doc->pflags|=DPF_JRUN;
      Docjexecute(doc,src);
      if(!jrun) doc->pflags&=~DPF_JRUN;

      /* Parse the last bit */
      jparse=BOOLVAL(doc->pflags&DPF_JPARSE);
      doc->pflags|=DPF_JPARSE;
      Parsehtml(doc,&doc->jout,TRUE,&doc->joutpos);
      if(!jparse) doc->pflags&=~DPF_JPARSE;
      if(src) FREE(src);

      /* Restore JS output */
      Freebuffer(&doc->jout);
      doc->jout=jout;
      doc->joutpos=joutpos;

      doc->pflags&=~DPF_BADSCRIPT;
   }
   else
   {
      doc->pflags|=DPF_BADSCRIPT;
   }
   return TRUE;
}

/*** <NOSCRIPT> ***/
static BOOL Donoscript(struct Document *doc,struct Tagattr *ta)
{  if(!((prefs.browser.dojs==DOJS_OFF)||(doc->pflags&DPF_BADSCRIPT)))
   {  doc->pmode=DPM_NOSCRIPT;
   }
   return TRUE;
}

/*** </NOSCRIPT> ***/
static BOOL Donoscriptend(struct Document *doc)
{  if(doc->doctype==DOCTP_FRAMESET) doc->pmode=DPM_FRAMESET;
   else doc->pmode=DPM_BODY;
//   doc->pflags|=DPF_BADSCRIPT;
   return TRUE;
}

/*--- body ------------------------------------------------------------*/

/*** <BODY> ***/
static BOOL Dobody(struct Document *doc,struct Tagattr *ta)
{  BOOL gotbg=FALSE,gotcolor=FALSE;
   if(STRICT && doc->doctype!=DOCTP_NONE) return TRUE;
   if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype!=DOCTP_BODY) return TRUE;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_BGCOLOR:
            if(!Setbodycolor(doc,&doc->bgcolor,ATTR(doc,ta))) return FALSE;
            gotbg=TRUE;
            break;
         case TAGATTR_TEXT:
            if(!Setbodycolor(doc,&doc->textcolor,ATTR(doc,ta))) return FALSE;
            gotcolor=TRUE;
            break;
         case TAGATTR_LINK:
            if(!Setbodycolor(doc,&doc->linkcolor,ATTR(doc,ta))) return FALSE;
            gotcolor=TRUE;
            break;
         case TAGATTR_VLINK:
            if(!Setbodycolor(doc,&doc->vlinkcolor,ATTR(doc,ta))) return FALSE;
            gotcolor=TRUE;
            break;
         case TAGATTR_ALINK:
            if(!Setbodycolor(doc,&doc->alinkcolor,ATTR(doc,ta))) return FALSE;
            gotcolor=TRUE;
            break;
         case TAGATTR_BACKGROUND:
            doc->bgimage=Backgroundimg(doc,ATTR(doc,ta));
            break;
         case TAGATTR_LEFTMARGIN:
            if(!STRICT)
            {  doc->hmargin=Getposnumber(ATTR(doc,ta));
               doc->dflags|=DDF_HMARGINSET;
               Asetattrs(doc->body,
                  AOBDY_Leftmargin,doc->hmargin,
                  TAG_END);
            }
            break;
         case TAGATTR_TOPMARGIN:
            if(!STRICT)
            {  doc->vmargin=Getposnumber(ATTR(doc,ta));
               doc->dflags|=DDF_VMARGINSET;
               Asetattrs(doc->body,
                  AOBDY_Topmargin,doc->vmargin,
                  TAG_END);
            }
            break;
         case TAGATTR_ONLOAD:
            if(doc->onload) FREE(doc->onload);
            doc->onload=Dupstr(ATTR(doc,ta),-1);
            if(doc->jobject)
            {
                /* Jobject allready exists so we must add our handler to
                 * it belatedly.
                 * Usually means there was <script> in the head */

                 struct Jcontext *jc = (struct Jcontext *)Agetattr(Aweb(),AOAPP_Jcontext);
                 Jaddeventhandler(jc,((struct Frame *)doc->frame)->jobject,"onload",doc->onload);

            }
            break;
         case TAGATTR_ONUNLOAD:
            if(doc->onunload) FREE(doc->onunload);
            doc->onunload=Dupstr(ATTR(doc,ta),-1);
            if(doc->jobject)
            {
                 struct Jcontext *jc = (struct Jcontext *)Agetattr(Aweb(),AOAPP_Jcontext);
                 Jaddeventhandler(jc,((struct Frame *)doc->frame)->jobject,"onunload",doc->onunload);

            }

            break;
         case TAGATTR_ONFOCUS:
            if(doc->onfocus) FREE(doc->onfocus);
            doc->onfocus=Dupstr(ATTR(doc,ta),-1);
            if(doc->frame) Asetattrs(doc->frame,AOFRM_Onfocus,(Tag)doc->onfocus,TAG_END);
            break;
         case TAGATTR_ONBLUR:
            if(doc->onblur) FREE(doc->onblur);
            doc->onblur=Dupstr(ATTR(doc,ta),-1);
            if(doc->frame) Asetattrs(doc->frame,AOFRM_Onblur,(Tag)doc->onblur,TAG_END);
            break;
      }
   }
   if(gotcolor && !gotbg)
   {  /* Set bg to white if no bg is defined but other colors are. */
      Setbodycolor(doc,&doc->bgcolor,"#ffffff");
   }
   /* Register colours with our frame */
   if(doc->win && doc->frame)
   {  Registerdoccolors(doc);
   }
   Checkid(doc,ta);
   return TRUE;
}

/*--- text, line breaks -----------------------------------------------*/

/*** <BLINK> ***/
static BOOL Doblink(struct Document *doc,struct Tagattr *ta)
{  if(!STRICT)
   {  doc->pflags|=DPF_BLINK;
   }
   Checkid(doc,ta);
   return TRUE;
}

/*** </BLINK> ***/
static BOOL Doblinkend(struct Document *doc)
{  if(!STRICT)
   {  doc->pflags&=~DPF_BLINK;
   }
   return TRUE;
}

/*** <BR> ***/
static BOOL Dobr(struct Document *doc,struct Tagattr *ta)
{  void *elt;
   BOOL clrleft=FALSE,clrright=FALSE;
   short gotbreak;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_CLEAR:
            if(STRIEQUAL(ATTR(doc,ta),"LEFT")) clrleft=TRUE;
            else if(STRIEQUAL(ATTR(doc,ta),"RIGHT")) clrright=TRUE;
            else if(STRIEQUAL(ATTR(doc,ta),"ALL")) clrleft=clrright=TRUE;
            break;
      }
   }
   if(!(elt=Anewobject(AOTP_BREAK,
      AOBJ_Pool,(Tag)doc->pool,
      AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
      AOBRK_Clearleft,clrleft,
      AOBRK_Clearright,clrright,
      TAG_END))) return FALSE;
   if(doc->wantbreak>0) doc->wantbreak--;
   gotbreak=doc->gotbreak+doc->wantbreak;
   if(!Addelement(doc,elt)) return FALSE;
   doc->gotbreak=gotbreak+1;
   if(!Ensuresp(doc)) return FALSE;
   Checkid(doc,ta);
   return TRUE;
}

/*** <CENTER> ***/
static BOOL Docenter(struct Document *doc,struct Tagattr *ta)
{  Wantbreak(doc,1);
   if(!Ensurebody(doc)) return FALSE;
   doc->centeractive=TRUE;
   Asetattrs(Docbodync(doc),AOBDY_Divalign,HALIGN_CENTER,TAG_END);
   Checkid(doc,ta);
   return TRUE;
}

/*** </CENTER> ***/
static BOOL Docenterend(struct Document *doc)
{  Wantbreak(doc,1);
   Asetattrs(Docbodync(doc),AOBDY_Divalign,-1,TAG_END);
   doc->centeractive=FALSE;
   if(!Ensuresp(doc)) return FALSE;
   return TRUE;
}

/*** <DIV> ***/
static BOOL Dodiv(struct Document *doc,struct Tagattr *ta)
{  short align=-1;
   Wantbreak(doc,1);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_ALIGN:
            align=Gethalign(ATTR(doc,ta));
            break;
      }
   }
   if(!Ensurebody(doc)) return FALSE;
   Asetattrs(Docbodync(doc),AOBDY_Divalign,align,TAG_END);
   Checkid(doc,ta);
   return TRUE;
}

/*** </DIV> ***/
static BOOL Dodivend(struct Document *doc)
{  Wantbreak(doc,1);
   Asetattrs(Docbodync(doc),AOBDY_Divalign,doc->centeractive?HALIGN_CENTER:-1,TAG_END);
   if(!Ensuresp(doc)) return FALSE;
   return TRUE;
}

/*** <NOBR> ***/
static BOOL Donobr(struct Document *doc,struct Tagattr *ta)
{  if(!STRICT)
   {  if(!Ensurebody(doc)) return FALSE;
      Asetattrs(Docbody(doc),AOBDY_Nobr,TRUE,TAG_END);
      Checkid(doc,ta);
   }
   return TRUE;
}

/*** </NOBR> ***/
static BOOL Donobrend(struct Document *doc)
{  if(!STRICT)
   {  Asetattrs(Docbody(doc),AOBDY_Nobr,FALSE,TAG_END);
   }
   return TRUE;
}

/*** <P> ***/
static BOOL Dopara(struct Document *doc,struct Tagattr *ta)
{  short align=-1;
   Wantbreak(doc,2);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_ALIGN:
            align=Gethalign(ATTR(doc,ta));
            break;
      }
   }
   if(!Ensurebody(doc)) return FALSE;
   Asetattrs(Docbodync(doc),AOBDY_Align,align,TAG_END);
   if(!Ensuresp(doc)) return FALSE;
   Checkid(doc,ta);
   return TRUE;
}

/*** </P> ***/
static BOOL Doparaend(struct Document *doc)
{  Wantbreak(doc,2);
   Asetattrs(Docbodync(doc),AOBDY_Align,-1,TAG_END);
   if(!Ensuresp(doc)) return FALSE;
   return TRUE;
}

/*** text ***/
static BOOL Dotext(struct Document *doc,struct Tagattr *ta)
{  void *elt;
   if(ta->length>0)
   {  if(!(elt=Anewobject(AOTP_TEXT,
         AOBJ_Pool,(Tag)doc->pool,
         AOELT_Textpos,doc->text.length,
         AOELT_Textlength,ta->length,
         AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
         AOTXT_Blink,doc->pflags&DPF_BLINK,
         AOTXT_Text,(Tag)&doc->text,
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      if(!Addtotextbuf(doc,ATTR(doc,ta),ta->length)) return FALSE;
   }
   ta=ta->next;
   if(ta && ta->next && ta->attr==TAGATTR_BR)
   {  Wantbreak(doc,1);
      if(!Solvebreaks(doc)) return FALSE;
      doc->gotbreak=0;
   }
   return TRUE;
}

/*** <WBR> ***/
static BOOL Dowbr(struct Document *doc,struct Tagattr *ta)
{  void *elt;
   if(!STRICT)
   {  if(!(elt=Anewobject(AOTP_BREAK,
         AOBJ_Pool,(Tag)doc->pool,
         AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
         AOBRK_Wbr,TRUE,
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      Checkid(doc,ta);
   }
   return TRUE;
}

/*--- preformatted ----------------------------------------------------*/

/*** <PRE> ***/
static BOOL Dopre(struct Document *doc,struct Tagattr *ta)
{  if(!Ensurebody(doc)) return FALSE;
   Wantbreak(doc,1);
   doc->pflags|=DPF_PREFORMAT;
   Asetattrs(Docbody(doc),
      AOBDY_Style,STYLE_PRE,
      AOBDY_Fixedfont,TRUE,
      TAG_END);
   doc->charcount=0;
   Checkid(doc,ta);
   return TRUE;
}

/*** </PRE> ***/
static BOOL Dopreend(struct Document *doc)
{  doc->pflags&=~(DPF_PREFORMAT|DPF_LISTING|DPF_XMP);
   Asetattrs(Docbody(doc),
      AOBDY_Fixedfont,FALSE,
      AOBDY_Fontend,TRUE,
      TAG_END);
   if(doc->charcount) Wantbreak(doc,1);
   if(!Ensuresp(doc)) return FALSE;
   return TRUE;
}

/*** <LISTING> ***/
static BOOL Dolisting(struct Document *doc,struct Tagattr *ta)
{  if(!Dopre(doc,ta)) return FALSE;
   doc->pflags|=DPF_LISTING;
   return TRUE;
}

/*** <XMP> ***/
static BOOL Doxmp(struct Document *doc,struct Tagattr *ta)
{  if(!Dopre(doc,ta)) return FALSE;
   doc->pflags|=DPF_XMP;
   return TRUE;
}

/*--- hard styles -----------------------------------------------------*/

/*** <B> <I> <U> <STRIKE> </B> </I> </U> </STRIKE> ***/
/* Set or unset child's style if it is a body. */
static BOOL Dohardstyle(struct Document *doc,struct Tagattr *ta,UWORD style,BOOL set)
{  if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         (set?AOBDY_Sethardstyle:AOBDY_Unsethardstyle),style,
         TAG_END);
   }
   Checkid(doc,ta);
   return TRUE;
}

/*** <TT> </TT> ***/
static BOOL Dott(struct Document *doc,struct Tagattr *ta,BOOL set)
{  if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),AOBDY_Fixedfont,set,TAG_END);
   }
   Checkid(doc,ta);
   return TRUE;
}

/*--- logical styles --------------------------------------------------*/

/*** <CITE> <CODE> <DFN> <EM> <KBD> <SAMP> <STRONG> <VAR> <BIG> <SMALL> ***/
static BOOL Dostyle(struct Document *doc,struct Tagattr *ta,UWORD style)
{  if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),AOBDY_Style,style,TAG_END);
   }
   Checkid(doc,ta);
   return TRUE;
}

/*** </CITE> </CODE> </DFN> </EM> </KBD> </SAMP> </STRONG> </VAR> </BIG> </SMALL> ***/
static BOOL Dostyleend(struct Document *doc)
{  if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),AOBDY_Fontend,TRUE,TAG_END);
   }
   return TRUE;
}

/*** <ADDRESS> ***/
static BOOL Doaddress(struct Document *doc,struct Tagattr *ta)
{  if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         AOBDY_Style,STYLE_ADDRESS,
         TAG_END);
   }
   Checkid(doc,ta);
   Wantbreak(doc,1);
   return TRUE;
}

/*** </ADDRESS> ***/
static BOOL Doaddressend(struct Document *doc)
{  if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         AOBDY_Fontend,TRUE,
         TAG_END);
   }
   Wantbreak(doc,1);
   if(!Ensuresp(doc)) return FALSE;
   return TRUE;
}

/*** <BLOCKQUOTE> ***/
static BOOL Doblockquote(struct Document *doc,struct Tagattr *ta)
{  if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         AOBDY_Style,STYLE_BLOCKQUOTE,
         AOBDY_Blockquote,TRUE,
         TAG_END);
   }
   Checkid(doc,ta);
   Wantbreak(doc,1);
   return TRUE;
}

/*** </BLOCKQUOTE> ***/
static BOOL Doblockquoteend(struct Document *doc)
{  if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         AOBDY_Fontend,TRUE,
         AOBDY_Blockquote,FALSE,
         TAG_END);
   }
   Wantbreak(doc,1);
   if(!Ensuresp(doc)) return FALSE;
   return TRUE;
}

/*** <Hn> ***/
static BOOL Doheading(struct Document *doc,short level,struct Tagattr *ta)
{  short align=-1;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_ALIGN:
            align=Gethalign(ATTR(doc,ta));
            break;
      }
   }
   if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Wantbreak(doc,2);
      if(!Solvebreaks(doc)) return FALSE;
      Asetattrs(Docbody(doc),
         AOBDY_Style,STYLE_H1+level,
         AOBDY_Align,align,
         TAG_END);
   }
   if(!Ensuresp(doc)) return FALSE;
   Checkid(doc,ta);
   return TRUE;
}

/*** </Hn> ***/
static BOOL Doheadingend(struct Document *doc)
{  if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         AOBDY_Fontend,TRUE,
         AOBDY_Align,-1,
         TAG_END);
   }
   if(!Ensuresp(doc)) return FALSE;
   Wantbreak(doc,2);
   return TRUE;
}

/*** <SUB> ***/
static BOOL Dosub(struct Document *doc,struct Tagattr *ta)
{  if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         AOBDY_Style,STYLE_SUB,
         AOBDY_Subscript,TRUE,
         TAG_END);
   }
   Checkid(doc,ta);
   return TRUE;
}

/*** </SUB> ***/
static BOOL Dosubend(struct Document *doc)
{  if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         AOBDY_Fontend,TRUE,
         AOBDY_Subscript,FALSE,
         TAG_END);
   }
   return TRUE;
}

/*** <SUP> ***/
static BOOL Dosup(struct Document *doc,struct Tagattr *ta)
{  if(!Ensurebody(doc)) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         AOBDY_Style,STYLE_SUP,
         AOBDY_Superscript,TRUE,
         TAG_END);
   }
   Checkid(doc,ta);
   return TRUE;
}

/*** </SUP> ***/
static BOOL Dosupend(struct Document *doc)
{  if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),
         AOBDY_Fontend,TRUE,
         AOBDY_Superscript,FALSE,
         TAG_END);
   }
   return TRUE;
}

/*** <INS> ***/
static BOOL Doins(struct Document *doc, struct Tagattr *ta)
{
  struct Colorinfo *ci;

  if(!Ensurebody(doc)) {
    return FALSE;
  }
  /* choose red colour. Might be better configurable */
  ci = Finddoccolor(doc,0xff0000L);
  Asetattrs(Docbody(doc),
            AOBDY_Sethardstyle,FSF_UNDERLINED,
            AOBDY_Fontcolor,(Tag)ci,
            TAG_END);
  Checkid(doc, ta);
  return TRUE;
}

/*** </INS> ***/
static BOOL Doinsend(struct Document *doc)
{
  if(doc->doctype == DOCTP_BODY) {
        Asetattrs(Docbody(doc),
                  AOBDY_Unsethardstyle,FSF_UNDERLINED,
                  AOBDY_Fontend,TRUE,
                  TAG_END);
  }
  return TRUE;
}

/*** <DEL> ***/
static BOOL Dodel(struct Document *doc, struct Tagattr *ta)
{
  struct Colorinfo *ci;

  if(!Ensurebody(doc)) {
        return FALSE;
  }
  /* this one is grey. make it configurable, too! */
  ci = Finddoccolor(doc,0xccccccL);
  Asetattrs(Docbody(doc),
            AOBDY_Sethardstyle,FSF_STRIKE,
            AOBDY_Fontcolor,(Tag)ci,
            TAG_END);
  Checkid(doc, ta);
  return TRUE;
}

/*** </DEL> ***/
static BOOL Dodelend(struct Document *doc)
{
  if(doc->doctype == DOCTP_BODY) {
        Asetattrs(Docbody(doc),
                  AOBDY_Unsethardstyle, FSF_STRIKE,
                  AOBDY_Fontend, TRUE,
                  TAG_END);
  }
  return TRUE;
}

/*--- font ------------------------------------------------------------*/

/*** <BASEFONT> ***/
static BOOL Dobasefont(struct Document *doc,struct Tagattr *ta)
{  short size=0;
   struct Number num;
   ULONG sizetag=TAG_IGNORE;
   ULONG colorrgb=~0;
   struct Colorinfo *ci=NULL;
   UBYTE *face=NULL;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_SIZE:
            size=Getnumber(&num,ATTR(doc,ta));
            if(num.type==NUMBER_SIGNED) sizetag=AOBDY_Basefontrel;
            else sizetag=AOBDY_Basefont;
            break;
         case TAGATTR_COLOR:
            Gethexcolor(doc,ATTR(doc,ta),&colorrgb);
            break;
         case TAGATTR_FACE:
            face=ATTR(doc,ta);
            break;
      }
   }
   if(size || colorrgb!=~0 || face)
   {  if(!Ensurebody(doc)) return FALSE;
      if(doc->doctype==DOCTP_BODY)
      {  if(colorrgb!=~0)
         {  if(!(ci=Finddoccolor(doc,colorrgb))) return FALSE;
         }
         Asetattrs(Docbody(doc),
            sizetag,size,
            (ci?AOBDY_Basecolor:TAG_IGNORE),(Tag)ci,
            (face?AOBDY_Baseface:TAG_IGNORE),(Tag)face,
            TAG_END);
      }
   }
   Checkid(doc,ta);
   return TRUE;
}

/*** <FONT> ***/
static BOOL Dofont(struct Document *doc,struct Tagattr *ta)
{  short size=0;
   ULONG sizetag=TAG_IGNORE;
   struct Number num;
   ULONG colorrgb=~0;
   struct Colorinfo *ci=NULL;
   UBYTE *face=NULL;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_SIZE:
            size=Getnumber(&num,ATTR(doc,ta));
            if(num.type==NUMBER_SIGNED) sizetag=AOBDY_Fontsizerel;
            else sizetag=AOBDY_Fontsize;
            break;
         case TAGATTR_COLOR:
            Gethexcolor(doc,ATTR(doc,ta),&colorrgb);
            break;
         case TAGATTR_FACE:
            face=ATTR(doc,ta);
            break;
      }
   }
   if(sizetag!=TAG_IGNORE || colorrgb!=(ULONG)~0 || face)
   {  if(!Ensurebody(doc)) return FALSE;
      if(!Solvebreaks(doc)) return FALSE;
      if(doc->doctype==DOCTP_BODY)
      {  if(colorrgb!=~0)
         {  if(!(ci=Finddoccolor(doc,colorrgb))) return FALSE;
         }
         Asetattrs(Docbody(doc),
            sizetag,size,
            (ci?AOBDY_Fontcolor:TAG_IGNORE),(Tag)ci,
            (face?AOBDY_Fontface:TAG_IGNORE),(Tag)face,
            TAG_END);
      }
   }
   Checkid(doc,ta);
   return TRUE;
}

/*** </FONT> ***/
static BOOL Dofontend(struct Document *doc)
{  if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbody(doc),AOBDY_Fontend,TRUE,TAG_END);
   }
   return TRUE;
}

/*--- ruler -----------------------------------------------------------*/

/*** <HR> ***/
static BOOL Dohr(struct Document *doc,struct Tagattr *ta)
{  short width=0;
   ULONG wtag=TAG_IGNORE;
   BOOL noshade=FALSE;
   long size=-1;
   short align=-1;
   ULONG colorrgb=(ULONG)~0;
   struct Colorinfo *color=NULL;
   struct Number num;
   void *elt;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_WIDTH:
            width=Getnumber(&num,ATTR(doc,ta));
            wtag=AORUL_Width;
            if(width<=0) width=100;
            else if(num.type!=NUMBER_PERCENT)
            {  wtag=AORUL_Pixelwidth;
            }
            break;
         case TAGATTR_SIZE:
            size=Getnumber(&num,ATTR(doc,ta));
            break;
         case TAGATTR_NOSHADE:
            noshade=TRUE;
            break;
         case TAGATTR_ALIGN:
            align=Gethalign(ATTR(doc,ta));
            break;
         case TAGATTR_COLOR:
            if(!STRICT)
            {  Gethexcolor(doc,ATTR(doc,ta),&colorrgb);
            }
            break;
      }
   }
   Wantbreak(doc,1);
   Asetattrs(Docbody(doc),AOBDY_Align,-1,TAG_END);
   if(colorrgb!=(ULONG)~0)
   {  if(!(color=Finddoccolor(doc,colorrgb))) return FALSE;
   }
   if(!(elt=Anewobject(AOTP_RULER,
      AOBJ_Pool,(Tag)doc->pool,
      wtag,width,
      CONDTAG(AORUL_Size,size),
      AORUL_Noshade,noshade,
      AORUL_Color,(Tag)color,
      CONDTAG(AOELT_Halign,align),
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   if(!Ensuresp(doc)) return FALSE;
   doc->gotbreak=2;
   return TRUE;
}

/*--- anchor ----------------------------------------------------------*/

/*** <A> ***/
static BOOL Doanchor(struct Document *doc,struct Tagattr *ta)
{  UBYTE *href=NULL,*name=NULL,*title=NULL;
   UBYTE *target=doc->target;
   UBYTE *onclick=NULL,*onmouseover=NULL,*onmouseout=NULL;
   BOOL post=FALSE,targetset=FALSE;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_HREF:
            href=ATTR(doc,ta);
            break;
         case TAGATTR_NAME:
         case TAGATTR_ID:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_TARGET:
            target=ATTR(doc,ta);
            targetset=TRUE;
            break;
         case TAGATTR_TITLE:
            title=ATTR(doc,ta);
            break;
         case TAGATTR_ONCLICK:
            onclick=ATTR(doc,ta);
            break;
         case TAGATTR_ONMOUSEOVER:
            onmouseover=ATTR(doc,ta);
            break;
         case TAGATTR_ONMOUSEOUT:
            onmouseout=ATTR(doc,ta);
            break;
         case TAGATTR_METHOD:
            if(!STRICT && STRIEQUAL(ATTR(doc,ta),"POST"))
            {  post=TRUE;
            }
            break;
      }
   }
   if(!Ensurebody(doc)) return FALSE;
   if((href || onclick || onmouseover || onmouseout) && doc->doctype==DOCTP_BODY)
   {  void *link;
      struct Url *url=NULL;
      UBYTE *f=NULL;
      if(href)
      {  f=Fragmentpart(href);
         /* If it is a link to a fragment in this doc use its URL (can't use Findurl
          * because of postnr) */
         if(*href=='#')
         {  url=(struct Url *)Agetattr(doc->source->source,AOSRC_Url);
         }
         else
         {  url=Findurl(doc->base,href,0);
         }
         if(!targetset && Mailnewsurl(href))
         {  target="_top";
         }
      }
      if(!(link=Anewobject(AOTP_LINK,
         AOBJ_Pool,(Tag)doc->pool,
         AOBJ_Frame,(Tag)doc->frame,
         AOBJ_Cframe,(Tag)doc->frame,
         AOBJ_Window,(Tag)doc->win,
         AOLNK_Url,(Tag)url,
         AOLNK_Fragment,(Tag)f,
         AOLNK_Target,(Tag)target,
         AOLNK_Text,(Tag)&doc->text,
         AOLNK_Title,(Tag)title,
         AOLNK_Onclick,(Tag)onclick,
         AOLNK_Onmouseover,(Tag)onmouseover,
         AOLNK_Onmouseout,(Tag)onmouseout,
         AOLNK_Post,post,
         TAG_END))) return FALSE;
      ADDTAIL(&doc->links,link);
      Asetattrs(Docbody(doc),AOBDY_Link,(Tag)link,TAG_END);
   }
   if(name && doc->doctype==DOCTP_BODY)
   {  void *elt;
      struct Fragment *frag;
      if(!Solvebreaks(doc)) return FALSE;
      if(!(elt=Anewobject(AOTP_NAME,
         AOBJ_Pool,(Tag)doc->pool,
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      if(!(frag=PALLOCSTRUCT(Fragment,1,0,doc->pool))) return FALSE;
      frag->name=Dupstr(name,-1);
      frag->elt=elt;
      ADDTAIL(&doc->fragments,frag);
   }
   return TRUE;
}

/*** </A> ***/
static BOOL Doanchorend(struct Document *doc)
{  if(doc->doctype==DOCTP_BODY)
   {  Asetattrs(Docbodync(doc),AOBDY_Link,(Tag)NULL,TAG_END);
   }
   return TRUE;
}

/*--- frames ----------------------------------------------------------*/

/* Add a FRAME or IFRAME */
static BOOL Addframe(struct Document *doc,struct Tagattr *ta,BOOL iframe)
{  short align=-1,flalign=-1;
   long width=0, height=0;
   ULONG wtag=TAG_IGNORE,htag=TAG_IGNORE;
   long mwidth=-1,mheight=-1;
   BOOL scrolling=TRUE,resize=TRUE;
   long border=-1;
   UBYTE *src=NULL,*name=NULL,*p;
   struct Url *url;
   void *elt;
   struct Number num;
   struct Frameref *fref;
   Checkid(doc,ta);
   if(iframe)
   {
      width=50;
      wtag=AOFRM_Width;
      height=50;
      htag=AOFRM_Height;
   }
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_ALIGN:
            align=Getvalign(ATTR(doc,ta),STRICT);
            flalign=Getflalign(ATTR(doc,ta));
            break;
         case TAGATTR_SRC:
            src=ATTR(doc,ta);
            break;
         case TAGATTR_WIDTH:
            width=Getnumber(&num,ATTR(doc,ta));
            wtag=AOFRM_Width;

            if(width <= (iframe?-1:0)) width=100;
            else if(num.type!=NUMBER_PERCENT)
            {  wtag=AOFRM_Pixelwidth;
            }
            break;
         case TAGATTR_HEIGHT:
            height=Getnumber(&num,ATTR(doc,ta));
            htag=AOFRM_Height;
            if(height<= (iframe?-1:0)) height=100;
            else if(num.type!=NUMBER_PERCENT)
            {  htag=AOFRM_Pixelheight;
            }
            break;
         case TAGATTR_MARGINWIDTH:
            mwidth=Getnumber(&num,ATTR(doc,ta));
            break;
         case TAGATTR_MARGINHEIGHT:
            mheight=Getnumber(&num,ATTR(doc,ta));
            break;
         case TAGATTR_SCROLLING:
            scrolling=Getbool(ATTR(doc,ta),scrolling);
            break;
         case TAGATTR_FRAMEBORDER:
         case TAGATTR_BORDER:
            p=ATTR(doc,ta);
            if(toupper(*p)=='N') border=0;
            else if(isdigit(*p)) border=Getnumber(&num,p);
            else border=2;
            break;
         case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_NORESIZE:
            resize=FALSE;
            break;
      }
   }
   if(!iframe)
   {  width=height=100;
      wtag=AOFRM_Width;
      htag=AOFRM_Height;
   }
   if(!src) src="x-nil:";
   if(!(url=Findurl(doc->base,src,0))) return FALSE;
   if(!(elt=Anewobject(AOTP_FRAME,
      AOBJ_Pool,(Tag)doc->pool,
      CONDTAG(AOELT_Valign,align),
      CONDTAG(AOELT_Floating,flalign),
      AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
      AOFRM_Url,(Tag)url,
      AOFRM_Seqnr,++doc->frameseqnr,
      AOBJ_Winhis,Agetattr(doc->frame,AOBJ_Winhis),
      wtag,width,
      htag,height,
      CONDTAG(AOFRM_Marginwidth,mwidth),
      CONDTAG(AOFRM_Marginheight,mheight),
      AOFRM_Scrolling,scrolling,
      AOFRM_Resize,resize,
      CONDTAG(AOFRM_Border,border),
      AOFRM_Name,(Tag)name,
      AOFRM_Inline,iframe,
      AOFRM_Reloadverify,BOOLVAL(doc->pflags&DPF_RELOADVERIFY),
      AOBJ_Nobackground,BOOLVAL(doc->dflags&DDF_NOBACKGROUND),
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   if(!Ensurenosp(doc)) return FALSE;
   if(!(fref=ALLOCSTRUCT(Frameref,1,MEMF_CLEAR))) return FALSE;
   fref->frame=elt;
   ADDTAIL(&doc->frames,fref);
   return TRUE;
}

/*** <FRAME> ***/
static BOOL Doframe(struct Document *doc,struct Tagattr *ta)
{  BOOL result=TRUE;
   if(prefs.browser.doframes)
   {  if(doc->doctype==DOCTP_FRAMESET && !(doc->pflags&DPF_FRAMESETEND))
      {  result=Addframe(doc,ta,FALSE);
      }
/*
      else if(!STRICT)
      {  result=Addframe(doc,ta,TRUE);
      }
*/
   }
   return result;
}

/*** <IFRAME> ***/
static BOOL Doiframe(struct Document *doc,struct Tagattr *ta)
{  BOOL result=TRUE;
   if(prefs.browser.doframes)
   {  if(!Ensurebody(doc)) return FALSE;
      result=Addframe(doc,ta,TRUE);
      doc->pmode=DPM_IFRAME;
   }
   return result;
}

/*** <FRAMESET> ***/
static BOOL Doframeset(struct Document *doc,struct Tagattr *ta)
{  UBYTE *rows=NULL,*cols=NULL,*p;
   long border=-1;
   long spacing=-1;
   struct Number num;
   void *frameset;
   if(prefs.browser.doframes)
   {  for(;ta->next;ta=ta->next)
      {  switch(ta->attr)
         {  case TAGATTR_COLS:
               cols=ATTR(doc,ta);
               break;
            case TAGATTR_ROWS:
               rows=ATTR(doc,ta);
               break;
            case TAGATTR_FRAMEBORDER:
            case TAGATTR_BORDER:
               p=ATTR(doc,ta);
               if(toupper(*p)=='N') border=0;
               else if(isdigit(*p)) border=Getnumber(&num,p);
               else border=2;
               break;
            case TAGATTR_FRAMESPACING:
               spacing=Getnumber(&num,ATTR(doc,ta));
               break;
         }
      }
      if(rows == NULL && cols == NULL)
      {
        rows="*";
      }
      if(doc->body && doc->doctype!=DOCTP_FRAMESET)
      {  Adisposeobject(doc->body);
         doc->body=NULL;
         doc->doctype=DOCTP_NONE;
      }
      if(doc->doctype!=DOCTP_BODY && !(doc->pflags&DPF_FRAMESETEND))
      {  if(!(frameset=Anewobject(AOTP_FRAMESET,
            AOBJ_Pool,(Tag)doc->pool,
            AOFRS_Cols,(Tag)cols,
            AOFRS_Rows,(Tag)rows,
            CONDTAG(AOFRM_Border,border),
            CONDTAG(AOFRS_Spacing,spacing),
            AOBJ_Frame,(Tag)doc->frame,
            AOBJ_Window,(Tag)doc->win,
            AOBJ_Nobackground,BOOLVAL(doc->dflags&DDF_NOBACKGROUND),
            TAG_END))) return FALSE;
         if(Agetattr(frameset,AOFRS_Sensible))
         {  if(doc->doctype==DOCTP_NONE)
            {  doc->doctype=DOCTP_FRAMESET;
               doc->pmode=DPM_FRAMESET;
               doc->body=frameset;
            }
            else if(doc->framesets.first->next)
            {  Aaddchild(doc->framesets.first->frameset,frameset,0);
            }
            else
            {  Adisposeobject(frameset);
               return FALSE;
            }
         }
         else
         {  Adisposeobject(frameset);
            return FALSE;
         }
         Pushframeset(doc,frameset);
      }
   }
   return TRUE;
}

/*** </FRAMESET> ***/
static BOOL Doframesetend(struct Document *doc)
{  if(prefs.browser.doframes)
   {  if(doc->doctype==DOCTP_FRAMESET)
      {  Popframeset(doc);
      }
   }
   return TRUE;
}

/*--- images ----------------------------------------------------------*/

/*** <AREA> ***/
static BOOL Doarea(struct Document *doc,struct Tagattr *ta,BOOL validarea)
{  UWORD shape=AREASHAPE_RECTANGLE;
   UBYTE *href=NULL;
   UBYTE *coords=NULL;
   struct Url *url=NULL;
   UBYTE *fragment=NULL;
   UBYTE *target=doc->target;
   long alt=-1,altlen=-1;
   UBYTE *onclick=NULL,*onmouseover=NULL,*onmouseout=NULL;
   void *area;
   BOOL targetset=FALSE;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_SHAPE:
            if(STRNIEQUAL(ATTR(doc,ta),"RECT",4)) shape=AREASHAPE_RECTANGLE;
            else if(STRNIEQUAL(ATTR(doc,ta),"CIRC",4)) shape=AREASHAPE_CIRCLE;
            else if(STRNIEQUAL(ATTR(doc,ta),"POLY",4)) shape=AREASHAPE_POLYGON;
            else if(STRIEQUAL(ATTR(doc,ta),"DEFAULT")) shape=AREASHAPE_DEFAULT;
            validarea=TRUE;
            break;
         case TAGATTR_COORDS:
            coords=ATTR(doc,ta);
            validarea=TRUE;
            break;
         case TAGATTR_HREF:
            href=ATTR(doc,ta);
            break;
         case TAGATTR_NOHREF:
            href=NULL;
            break;
         case TAGATTR_TARGET:
            target=ATTR(doc,ta);
            targetset=TRUE;
            break;
         case TAGATTR_ALT:
            if(ta->length)
            {  alt=doc->text.length;
               altlen=ta->length;
               if(!(Addtotextbuf(doc,ATTR(doc,ta),ta->length))) return FALSE;
            }
            break;
         case TAGATTR_ONCLICK:
            onclick=ATTR(doc,ta);
            break;
         case TAGATTR_ONMOUSEOVER:
            onmouseover=ATTR(doc,ta);
            break;
         case TAGATTR_ONMOUSEOUT:
            onmouseout=ATTR(doc,ta);
            break;
      }
   }
   if(doc->maps.last->prev && validarea)
   {  if(href)
      {  fragment=Fragmentpart(href);
         if(!(url=Findurl(doc->base,href,0))) return FALSE;
         if(!targetset && Mailnewsurl(href))
         {  target="_top";
         }
      }
      if(!(area=Anewobject(AOTP_AREA,
         AOBJ_Pool,(Tag)doc->pool,
         AOBJ_Frame,(Tag)doc->frame,
         AOBJ_Cframe,(Tag)doc->frame,
         AOBJ_Window,(Tag)doc->win,
         AOLNK_Url,(Tag)url,
         AOLNK_Fragment,(Tag)fragment,
         AOLNK_Target,(Tag)target,
         AOLNK_Text,(Tag)&doc->text,
         AOLNK_Onclick,(Tag)onclick,
         AOLNK_Onmouseover,(Tag)onmouseover,
         AOLNK_Onmouseout,(Tag)onmouseout,
         AOARA_Shape,shape,
         AOARA_Coords,(Tag)coords,
         CONDTAG(AOARA_Textpos,alt),
         CONDTAG(AOARA_Textlength,altlen),
         TAG_END))) return FALSE;
      ADDTAIL(&doc->links,area);
      Asetattrs(doc->maps.last,
         AOMAP_Area,(Tag)area,
         TAG_END);
   }
   return TRUE;
}

/*** <IMG> ***/
static BOOL Doimg(struct Document *doc,struct Tagattr *ta)
{  short border=-1,width=-1,height=-1,hspace=-1,vspace=-1;
   short align=-1,flalign=-1;
   ULONG wtag=TAG_IGNORE,htag=TAG_IGNORE;
   long alt=-1,altlen=-1;
   void *usemap=NULL;
   struct Number num;
   UBYTE *src=NULL,*mapname,*name=NULL;
   UBYTE *onload=NULL,*onerror=NULL,*onabort=NULL;
   UBYTE *onclick=NULL,*onmouseout=NULL,*onmouseover=NULL;
   BOOL ismap=FALSE,wasspace=FALSE,srcvalid = TRUE;
   void *elt,*url,*referer,*jform=NULL;

   if(!prefs.browser.imgborder) border=0;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_SRC:
            src=ATTR(doc,ta);

            break;
         case TAGATTR_ALIGN:
            align=Getvalign(ATTR(doc,ta),STRICT);
            flalign=Getflalign(ATTR(doc,ta));
            break;
         case TAGATTR_ALT:
            if(ta->length)
            {  wasspace=(doc->text.buffer[doc->text.length-1]==' ');
               alt=doc->text.length;
               altlen=ta->length;
               if(!(Addtotextbuf(doc,ATTR(doc,ta),ta->length))) return FALSE;
            }
            break;
         case TAGATTR_ISMAP:
            ismap=TRUE;
            break;
         case TAGATTR_USEMAP:
            mapname=ATTR(doc,ta);
            if(mapname[0]=='#')
            {  usemap=Findmap(doc,mapname+1);
            }
            else
            {  usemap=Externalmap(doc,mapname);
            }
            break;
         case TAGATTR_BORDER:
            border=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_WIDTH:
            width=Getnumber(&num,ATTR(doc,ta));
            if(width<=0) width=0;
            if(num.type==NUMBER_PERCENT)
            {  wtag=AOCPY_Percentwidth;
            }
            else
            {  wtag=AOCPY_Width;
            }
            break;
         case TAGATTR_HEIGHT:
            height=Getnumber(&num,ATTR(doc,ta));
            if(height<=0) height=0;
            if(num.type==NUMBER_PERCENT)
            {  htag=AOCPY_Percentheight;
            }
            else
            {  htag=AOCPY_Height;
            }
            break;
         case TAGATTR_HSPACE:
            hspace=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_VSPACE:
            vspace=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_ONLOAD:
            onload=ATTR(doc,ta);
            break;
         case TAGATTR_ONERROR:
            onerror=ATTR(doc,ta);
            break;
         case TAGATTR_ONABORT:
            onabort=ATTR(doc,ta);
            break;
         case TAGATTR_ONCLICK:
            onclick=ATTR(doc,ta);
            break;
         case TAGATTR_ONMOUSEOVER:
            onmouseover=ATTR(doc,ta);
            break;
         case TAGATTR_ONMOUSEOUT:
            onmouseout=ATTR(doc,ta);
            break;
      }
   }

   /* if src is vvalid and not "" */
   /* we need to reject "<whitespace>" as Findurl strips leading spaces from */
   /* a url */
   if(src)
   {
      char *p = src;
      do
      {
         if((*p) == '\0')
         {
            srcvalid = FALSE;
            break;
         }
      }
      while( isspace(*p++));
   }
   if(src && (srcvalid == TRUE))
   {  if(name && doc->pflags&DPF_FORM) jform=doc->forms.last;
      if(!(url=Findurl(doc->base,src,0))) return FALSE;
      referer=(void *)Agetattr(doc->source->source,AOSRC_Url);
      if(!Ensurebody(doc)) return FALSE;
      if(!(elt=Anewobject(AOTP_COPY,
         AOBJ_Pool,(Tag)doc->pool,
         CONDTAG(AOELT_Valign,align),
         CONDTAG(AOELT_Floating,flalign),
         CONDTAG(AOELT_Textpos,alt),
         CONDTAG(AOELT_Textlength,altlen),
         AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
         AOCPY_Url,(Tag)url,
         AOCPY_Embedded,TRUE,
         AOCPY_Usemap,(Tag)usemap,
         AOCPY_Ismap,ismap,
         AOCPY_Text,(Tag)&doc->text,
         AOCPY_Referer,(Tag)referer,
         AOCPY_Defaulttype,(Tag)"image/x-unknown",
         AOCPY_Reloadverify,(doc->pflags&DPF_RELOADVERIFY),
         AOCPY_Trueimage,TRUE,
         AOCPY_Name,(Tag)name,
         AOCPY_Onload,(Tag)onload,
         AOCPY_Onerror,(Tag)onerror,
         AOCPY_Onabort,(Tag)onabort,
         AOCPY_Onclick,(Tag)onclick,
         AOCPY_Onmouseover,(Tag)onmouseover,
         AOCPY_Onmouseout,(Tag)onmouseout,
         AOCPY_Jform,(Tag)jform,
         CONDTAG(AOCPY_Border,border),
         wtag,width,
         htag,height,
         CONDTAG(AOCPY_Hspace,hspace),
         CONDTAG(AOCPY_Vspace,vspace),
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      if(flalign<0)
      {  if(!Ensurenosp(doc)) return FALSE;
      }
      else if(wasspace)
      {  if(!Ensuresp(doc)) return FALSE;
      }
   }
   return TRUE;
}

/*** &dingbat; ***/
static BOOL Doicon(struct Document *doc,struct Tagattr *ta)
{  UBYTE name[sizeof(DINGBATPATH)+40]="";
   UBYTE *src=NULL;
   struct Url *url;
   void *elt;
   long alt,altlen;
   /* No loop through ta - we *know* that it is the source. Its valuepos
    * points to the name as a string, not as offset in args buffer. */
   src=(UBYTE *)ta->valuepos;
   alt=doc->text.length;
   altlen=strlen(src);
   if(!Addtotextbuf(doc,src,strlen(src))) return FALSE;
   strcpy(name,DINGBATPATH);
   strncat(name,src,32);
   if(!(url=Findurl("",name,0))) return FALSE;
   if(!Ensurebody(doc)) return FALSE;
   if(!(elt=Anewobject(AOTP_COPY,
      AOBJ_Pool,(Tag)doc->pool,
      AOELT_Textpos,alt,
      AOELT_Textlength,altlen,
      AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
      AOCPY_Url,(Tag)url,
      AOCPY_Embedded,TRUE,
      AOCPY_Text,(Tag)&doc->text,
      AOCPY_Defaulttype,(Tag)"image/x-unknown",
      AOCPY_Reloadverify,(doc->pflags&DPF_RELOADVERIFY),
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   Asetattrs(elt,AOBJ_Layoutparent,(Tag)doc->body,TAG_END);
   return TRUE;
}

/*** <MAP> ***/
static BOOL Domap(struct Document *doc,struct Tagattr *ta)
{  void *map;
   UBYTE *name=NULL;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
      }
   }
   if(name)
   {  if(!(map=Findmap(doc,name))) return FALSE;
      /* make sure this map is at the end of the list for Doarea() */
      REMOVE(map);
      ADDTAIL(&doc->maps,map);
      doc->pmode=DPM_MAP;
   }
   return TRUE;
}

/*--- lists -----------------------------------------------------------*/

/*** <DD> ***/
static BOOL Dodd(struct Document *doc,struct Tagattr *ta)
{  Wantbreak(doc,1);
   Asetattrs(Docbody(doc),
      AOBDY_Align,-1,
      AOBDY_Dterm,FALSE,
      TAG_END);
   Checkid(doc,ta);
   return TRUE;
}

/*** <DL> ***/
static BOOL Dodl(struct Document *doc,struct Tagattr *ta)
{  struct Listinfo li={0},*nestli;
   li.type=BDLT_DL;
   if(!Ensurebody(doc)) return FALSE;
   /* Add extra space if it is the outer list */
   nestli=(struct Listinfo *)Agetattr(Docbody(doc),AOBDY_List);
   Wantbreak(doc,nestli->next?1:2);
   Asetattrs(Docbody(doc),
      AOBDY_Align,-1,
      AOBDY_List,(Tag)&li,
      TAG_END);
   Checkid(doc,ta);
   return TRUE;
}

/*** </DL> ***/
static BOOL Dodlend(struct Document *doc)
{  struct Listinfo *nestli;
   Asetattrs(Docbody(doc),
      AOBDY_Align,-1,
      AOBDY_List,(Tag)NULL,
      TAG_END);
   /* Add extra space if it is the outer list */
   nestli=(struct Listinfo *)Agetattr(Docbody(doc),AOBDY_List);
   doc->wantbreak=nestli->next?1:2;
   return TRUE;
}

/*** <DT> ***/
static BOOL Dodt(struct Document *doc,struct Tagattr *ta)
{  Wantbreak(doc,1);
   Asetattrs(Docbody(doc),
      AOBDY_Align,-1,
      AOBDY_Dterm,TRUE,
      TAG_END);
   Checkid(doc,ta);
   return TRUE;
}

/*** <OL> ***/
static BOOL Dool(struct Document *doc,struct Tagattr *ta)
{  struct Listinfo li={0},*nestli;
   li.type=BDLT_OL;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_CONTINUE:
            break;
         case TAGATTR_SEQNUM:
            if(!STRICT) li.bulletnr=Getposnumber(ATTR(doc,ta))-1;
            break;
         case TAGATTR_START:
            li.bulletnr=Getposnumber(ATTR(doc,ta))-1;
            if(li.bulletnr<-1) li.bulletnr=-1;
            break;
         case TAGATTR_TYPE:
            switch(*ATTR(doc,ta))
            {  case 'A':   li.bullettype=BDBT_ALPHA;break;
               case 'a':   li.bullettype=BDBT_ALPHALOW;break;
               case 'I':   li.bullettype=BDBT_ROMAN;break;
               case 'i':   li.bullettype=BDBT_ROMANLOW;break;
            }
            break;
      }
   }
   if(!Ensurebody(doc)) return FALSE;
   /* Add extra space if it is the outer list */
   nestli=(struct Listinfo *)Agetattr(Docbody(doc),AOBDY_List);
   Wantbreak(doc,nestli->next?1:2);
   Asetattrs(Docbody(doc),
      AOBDY_Align,-1,
      AOBDY_List,(Tag)&li,
      TAG_END);
   Checkid(doc,ta);
   return TRUE;
}

/*** </OL> ***/
static BOOL Doolend(struct Document *doc)
{  struct Listinfo *nestli;
   Asetattrs(Docbody(doc),
      AOBDY_Align,-1,
      AOBDY_List,(Tag)NULL,
      TAG_END);
   doc->gotbreak=0;
   /* Add extra space if it is the outer list */
   nestli=(struct Listinfo *)Agetattr(Docbody(doc),AOBDY_List);
   doc->wantbreak=nestli->next?1:2;
   return TRUE;
}

/*** <UL> ***/
static BOOL Doul(struct Document *doc,struct Tagattr *ta)
{  UBYTE dingbat[sizeof(DINGBATPATH)+40]="";
   struct Listinfo li={0},*nestli;
   li.type=BDLT_UL;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_TYPE:
            if(STRNIEQUAL(ATTR(doc,ta),"DISC",4)) li.bullettype=BDBT_DISC;
            else if(STRNIEQUAL(ATTR(doc,ta),"CIRC",4)) li.bullettype=BDBT_CIRCLE;
            else if(STRNIEQUAL(ATTR(doc,ta),"SQUA",4)) li.bullettype=BDBT_SQUARE;
            break;
         case TAGATTR_SRC:
            li.bulletsrc=ATTR(doc,ta);
            li.bullettype=BDBT_IMAGE;
            break;
         case TAGATTR_DINGBAT:
            strcpy(dingbat,DINGBATPATH);
            strncat(dingbat,ATTR(doc,ta),32);
            li.bulletsrc=dingbat;
            li.bullettype=BDBT_IMAGE;
         break;
      }
   }
   if(!Ensurebody(doc)) return FALSE;
   /* Add extra space if it is the outer list */
   nestli=(struct Listinfo *)Agetattr(Docbody(doc),AOBDY_List);
   Wantbreak(doc,nestli->next?1:2);
   Asetattrs(Docbody(doc),
      AOBDY_Align,-1,
      AOBDY_List,(Tag)&li,
      TAG_END);
   Checkid(doc,ta);
   return TRUE;
}

/*** </UL> ***/
static BOOL Doulend(struct Document *doc)
{  struct Listinfo *nestli;
   Asetattrs(Docbody(doc),
      AOBDY_Align,-1,
      AOBDY_List,(Tag)NULL,
      TAG_END);
   doc->gotbreak=0;
   /* Add extra space if it is the outer list */
   nestli=(struct Listinfo *)Agetattr(Docbody(doc),AOBDY_List);
   doc->wantbreak=nestli->next?1:2;
   return TRUE;
}

/*** <LI> ***/
static BOOL Doli(struct Document *doc,struct Tagattr *ta)
{  UBYTE dingbat[sizeof(DINGBATPATH)+40]="";
   struct Listinfo *li;
   UBYTE buf[32];
   long length;
   UBYTE *src=NULL;
   void *elt=NULL,*url,*referer;
   short btype;
   if(doc->pflags&DPF_BULLET) doc->gotbreak=0;
   Wantbreak(doc,1);
   Asetattrs(Docbody(doc),AOBDY_Align,-1,TAG_END);
   li=(struct Listinfo *)Agetattr(Docbody(doc),AOBDY_List);
   if(!li || !li->type)
   {  if(!Doul(doc,&dummyta)) return FALSE;
      li=(struct Listinfo *)Agetattr(Docbody(doc),AOBDY_List);
   }
   if(li && li->type==BDLT_UL)
   {  btype=li->bullettype;
      src=li->bulletsrc;
      for(;ta->next;ta=ta->next)
      {  switch(ta->attr)
         {  case TAGATTR_TYPE:
               if(STRNIEQUAL(ATTR(doc,ta),"DISC",4)) li->bullettype=BDBT_DISC;
               else if(STRNIEQUAL(ATTR(doc,ta),"CIRC",4)) li->bullettype=BDBT_CIRCLE;
               else if(STRNIEQUAL(ATTR(doc,ta),"SQUA",4)) li->bullettype=BDBT_SQUARE;
               btype=li->bullettype;
               break;
            case TAGATTR_SRC:
               src=ATTR(doc,ta);
               btype=BDBT_IMAGE;
               break;
            case TAGATTR_DINGBAT:
               strcpy(dingbat,DINGBATPATH);
               strncat(dingbat,ATTR(doc,ta),32);
               src=dingbat;
               btype=BDBT_IMAGE;
               break;
         }
      }
      switch(btype)
      {  case BDBT_DISC:
         case BDBT_CIRCLE:
         case BDBT_SQUARE:
         case BDBT_DIAMOND:
         case BDBT_SOLIDDIA:
         case BDBT_RECTANGLE:
            if(!(elt=Anewobject(AOTP_BULLET,
               AOBJ_Pool,(Tag)doc->pool,
               AOBUL_Type,btype,
               AOELT_Bullet,TRUE,
               AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
               TAG_END))) return FALSE;
            break;
         case BDBT_IMAGE:
            referer=(void *)Agetattr(doc->source->source,AOSRC_Url);
            if(!(url=Findurl(doc->base,src,0))) return FALSE;
            if(!(elt=Anewobject(AOTP_COPY,
               AOBJ_Pool,(Tag)doc->pool,
               AOELT_Bullet,TRUE,
               AOCPY_Url,(Tag)url,
               AOCPY_Embedded,TRUE,
               AOCPY_Text,(Tag)&doc->text,
               AOCPY_Referer,(Tag)referer,
               AOCPY_Defaulttype,(Tag)"image/x-unknown",
               AOCPY_Reloadverify,(doc->pflags&DPF_RELOADVERIFY),
               TAG_END))) return FALSE;
            break;
      }
      if(elt)
      {  if(!Addelement(doc,elt)) return FALSE;
         Asetattrs(elt,AOBJ_Layoutparent,(Tag)doc->body,TAG_END);
      }
      doc->gotbreak=2;
      doc->pflags|=DPF_BULLET;
   }
   else if(li && li->type==BDLT_OL)
   {  for(;ta->next;ta=ta->next)
      {  switch(ta->attr)
         {  case TAGATTR_SKIP:
               if(!STRICT)
               {  short n;
                  sscanf(ATTR(doc,ta)," %hd",&n);
                  li->bulletnr+=n;
               }
               break;
            case TAGATTR_VALUE:
               if(sscanf(ATTR(doc,ta)," %ld",&li->bulletnr) > 0) li->bulletnr--;
               break;
            case TAGATTR_TYPE:
               switch(*ATTR(doc,ta))
               {  case 'A':   li->bullettype=BDBT_ALPHA;break;
                  case 'a':   li->bullettype=BDBT_ALPHALOW;break;
                  case 'I':   li->bullettype=BDBT_ROMAN;break;
                  case 'i':   li->bullettype=BDBT_ROMANLOW;break;
               }
               break;
         }
      }
      switch(li->bullettype)
      {  case BDBT_ALPHA:
            length=sprintf(buf,"%c ",'A'+(char)((li->bulletnr++)%26));
            break;
         case BDBT_ALPHALOW:
            length=sprintf(buf,"%c ",'a'+(char)((li->bulletnr++)%26));
            break;
         case BDBT_ROMAN:
            length=Makeroman(buf,++li->bulletnr,FALSE);
            break;
         case BDBT_ROMANLOW:
            length=Makeroman(buf,++li->bulletnr,TRUE);
            break;
         default:
            length=sprintf(buf,"%ld. ",++li->bulletnr);
            break;
      }
      if(!(elt=Anewobject(AOTP_TEXT,
         AOBJ_Pool,(Tag)doc->pool,
         AOELT_Textpos,doc->text.length,
         AOELT_Textlength,length,
         AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
         AOELT_Bullet,TRUE,
         AOTXT_Blink,doc->pflags&DPF_BLINK,
         AOTXT_Text,(Tag)&doc->text,
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      if(!Addtotextbuf(doc,buf,length)) return FALSE;
      doc->gotbreak=2;
      doc->pflags|=DPF_BULLET;
   }
   Checkid(doc,ta);
   return TRUE;
}

/*--- tables ----------------------------------------------------------*/

/*** <TABLE> ***/
static BOOL Dotable(struct Document *doc,struct Tagattr *ta)
{  short border=-1,cellspacing=-1,cellpadding=-1,width=-1;
   short align=-1,flalign=-1;
   short frame=-1,rules=-1;
   ULONG wtag=TAG_IGNORE;
   struct Colorinfo *bgcolor=NULL,*bordercolor=NULL,*borderdark=NULL,*borderlight=NULL;
   struct Number num;
   void *elt,*bgimg=NULL;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_ALIGN:
            flalign=Getflalign(ATTR(doc,ta));
            if(STRIEQUAL(ATTR(doc,ta),"CENTER")) align=HALIGN_CENTER;
            break;
         case TAGATTR_BORDER:
            if(ta->length)
            {  border=Getposnumber(ATTR(doc,ta));
            }
            else border=1;
            if(frame<0) frame=border?TABFRM_ALL:TABFRM_NONE;
            if(rules<0) rules=border?TABRUL_ALL:TABRUL_NONE;
            break;
         case TAGATTR_WIDTH:
            width=Getnumber(&num,ATTR(doc,ta));
            wtag=AOTAB_Percentwidth;
            if(width<=0) width=100;
            else if(num.type!=NUMBER_PERCENT)
            {  wtag=AOTAB_Pixelwidth;
            }
            break;
         case TAGATTR_CELLPADDING:
            cellpadding=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_CELLSPACING:
            cellspacing=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_BACKGROUND:
            if(!STRICT) bgimg=Backgroundimg(doc,ATTR(doc,ta));
            break;
         case TAGATTR_BGCOLOR:
            if(!Setbodycolor(doc,&bgcolor,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_BORDERCOLOR:
            if(!STRICT && !Setbodycolor(doc,&bordercolor,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_BORDERCOLORDARK:
            if(!STRICT && !Setbodycolor(doc,&borderdark,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_BORDERCOLORLIGHT:
            if(!STRICT && !Setbodycolor(doc,&borderlight,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_FRAME:
            if(STRIEQUAL(ATTR(doc,ta),"VOID")) frame=TABFRM_NONE;
            else if(STRIEQUAL(ATTR(doc,ta),"ABOVE")) frame=TABFRM_ABOVE;
            else if(STRIEQUAL(ATTR(doc,ta),"BELOW")) frame=TABFRM_BELOW;
            else if(STRIEQUAL(ATTR(doc,ta),"HSIDES")) frame=TABFRM_HSIDES;
            else if(STRIEQUAL(ATTR(doc,ta),"LHS")) frame=TABFRM_LEFT;
            else if(STRIEQUAL(ATTR(doc,ta),"RHS")) frame=TABFRM_RIGHT;
            else if(STRIEQUAL(ATTR(doc,ta),"VSIDES")) frame=TABFRM_VSIDES;
            else if(STRIEQUAL(ATTR(doc,ta),"BOX")) frame=TABFRM_ALL;
            else if(STRIEQUAL(ATTR(doc,ta),"BORDER")) frame=TABFRM_ALL;
            else frame=TABFRM_ALL;
            if(border<0) border=frame?1:0;
            break;
         case TAGATTR_RULES:
            if(STRIEQUAL(ATTR(doc,ta),"NONE")) rules=TABRUL_NONE;
            else if(STRIEQUAL(ATTR(doc,ta),"GROUPS")) rules=TABRUL_GROUPS;
            else if(STRIEQUAL(ATTR(doc,ta),"ROWS")) rules=TABRUL_ROWS;
            else if(STRIEQUAL(ATTR(doc,ta),"COLS")) rules=TABRUL_COLS;
            else if(STRIEQUAL(ATTR(doc,ta),"ALL")) rules=TABRUL_ALL;
            else rules=TABRUL_ALL;
            break;
      }
   }
   if(!(Ensurebody(doc))) return FALSE;
   if(doc->doctype==DOCTP_BODY)
   {  if(flalign<0) Wantbreak(doc,1);
      Asetattrs(Docbody(doc),AOBDY_Align,-1,TAG_END);
      if(!(elt=Anewobject(AOTP_TABLE,
         AOBJ_Pool,(Tag)doc->pool,
         AOBJ_Nobackground,BOOLVAL(doc->dflags&DDF_NOBACKGROUND),
         CONDTAG(AOELT_Halign,align),
         CONDTAG(AOELT_Floating,flalign),
         AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
         CONDTAG(AOTAB_Border,border),
         CONDTAG(AOTAB_Tabframe,frame),
         CONDTAG(AOTAB_Rules,rules),
         CONDTAG(AOTAB_Cellpadding,cellpadding),
         CONDTAG(AOTAB_Cellspacing,cellspacing),
         AOTAB_Bgimage,(Tag)bgimg,
         AOTAB_Bgcolor,(Tag)bgcolor,
         AOTAB_Bordercolor,(Tag)bordercolor,
         AOTAB_Borderdark,(Tag)borderdark,
         AOTAB_Borderlight,(Tag)borderlight,
         wtag,width,
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      if(!Pushtable(doc,elt)) return FALSE;
      if(!Ensuresp(doc)) return FALSE;
   }
   doc->pflags&=~DPF_BLINK;
   return TRUE;
}

/*** </TABLE> ***/
static BOOL Dotableend(struct Document *doc)
{  UWORD flalign=0;
   if(!ISEMPTY(&doc->tables))
   {  flalign=Agetattr(doc->tables.first->table,AOELT_Floating);
      Asetattrs(doc->tables.first->table,
         AOTAB_Vspacing,doc->gotbreak,
         AOTAB_Endtable,TRUE,
         TAG_END);
      Poptable(doc);
      doc->gotbreak=0;
      if(!Ensuresp(doc)) return FALSE;
      if(!flalign)
      {  Wantbreak(doc,1);
         Solvebreaks(doc);
      }
      if(Agetattr(Docbodync(doc),AOBDY_Style)!=STYLE_PRE) doc->pflags&=~DPF_PREFORMAT;
   }
   doc->pflags&=~DPF_BLINK;
   return TRUE;
}

/*** <CAPTION> ***/
static BOOL Docaption(struct Document *doc,struct Tagattr *ta)
{  short align=-1,halign=-1,a;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_VALIGN:
            if(STRICT) break;
            /* else fall trhough: */
         case TAGATTR_ALIGN:
            a=Getvalign(ATTR(doc,ta),STRICT);
            if(a>=0)
            {  align=a;
            }
            else if(!STRICT)
            {  halign=Gethalign(ATTR(doc,ta));
            }
            break;
      }
   }
   if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         AOTAB_Vspacing,doc->gotbreak,
         AOTAB_Caption,TRUE,
         CONDTAG(AOTAB_Valign,align),
         CONDTAG(AOTAB_Halign,halign),
         TAG_END);
      doc->gotbreak=2;
      if(Agetattr(Docbodync(doc),AOBDY_Style)!=STYLE_PRE) doc->pflags&=~DPF_PREFORMAT;
   }
   if(!Ensuresp(doc)) return FALSE;
   doc->pflags&=~DPF_BLINK;
   Checkid(doc,ta);
   return TRUE;
}

/*** </CAPTION> ***/
static BOOL Docaptionend(struct Document *doc)
{  if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         AOTAB_Vspacing,doc->gotbreak,
         AOTAB_Caption,FALSE,
         TAG_END);
      doc->gotbreak=0;
      doc->wantbreak=0;
      if(Agetattr(Docbodync(doc),AOBDY_Style)!=STYLE_PRE) doc->pflags&=~DPF_PREFORMAT;
   }
   if(!Ensuresp(doc)) return FALSE;
   doc->pflags&=~DPF_BLINK;
   return TRUE;
}

/*** <COLGROUP>, <COL> ***/
static BOOL Docolgrouporcol(struct Document *doc,struct Tagattr *ta,ULONG tagtype)
{  short halign=-1,valign=-1;
   short width=-1;
   ULONG wtag=TAG_IGNORE;
   short span=1;
   struct Number num;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_SPAN:
            span=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_WIDTH:
            width=Getnumber(&num,ATTR(doc,ta));
            if(width<=0) width=0;
            if(num.type==NUMBER_PERCENT)
            {  wtag=AOTAB_Percentwidth;
            }
            else if(num.type==NUMBER_RELATIVE)
            {  wtag=AOTAB_Relwidth;
            }
            else
            {  wtag=AOTAB_Pixelwidth;
            }
            break;
         case TAGATTR_ALIGN:
            halign=Gethalign(ATTR(doc,ta));
            break;
         case TAGATTR_VALIGN:
            valign=Getvalign(ATTR(doc,ta),STRICT);
            if(STRIEQUAL(ATTR(doc,ta),"BASELINE")) valign=VALIGN_BASELINE;
            break;
      }
   }
   if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         tagtype==MARKUP_COLGROUP?AOTAB_Colgroup:AOTAB_Column,TRUE,
         AOTAB_Colspan,span,
         CONDTAG(AOTAB_Halign,halign),
         CONDTAG(AOTAB_Valign,valign),
         wtag,width,
         TAG_END);
   }
   return TRUE;
}

/*** </COLGROUP> ***/
static BOOL Docolgroupend(struct Document *doc)
{  if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         AOTAB_Colgroup,FALSE,
         TAG_END);
   }
   return TRUE;
}

/*** <THEAD>, <TFOOT>, <TBODY> ***/
static BOOL Dorowgroup(struct Document *doc,struct Tagattr *ta,ULONG tagtype)
{  short halign=-1,valign=-1;
   ULONG gtag=TAG_END;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_ALIGN:
            halign=Gethalign(ATTR(doc,ta));
            break;
         case TAGATTR_VALIGN:
            valign=Getvalign(ATTR(doc,ta),STRICT);
            if(STRIEQUAL(ATTR(doc,ta),"BASELINE")) valign=VALIGN_BASELINE;
            break;
      }
   }
   switch(tagtype)
   {  case MARKUP_THEAD:
         gtag=AOTAB_Thead;
         break;
      case MARKUP_TFOOT:
         gtag=AOTAB_Tfoot;
         break;
      case MARKUP_TBODY:
         gtag=AOTAB_Tbody;
         break;
   }
   if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         gtag,TRUE,
         CONDTAG(AOTAB_Halign,halign),
         CONDTAG(AOTAB_Valign,valign),
         TAG_END);
   }
   return TRUE;
}

/*** </THEAD>, </TFOOT>, </TBODY> ***/
static BOOL Dorowgroupend(struct Document *doc,ULONG tagtype)
{  ULONG gtag=TAG_END;
   switch(tagtype)
   {  case MARKUP_THEAD:
         gtag=AOTAB_Thead;
         break;
      case MARKUP_TFOOT:
         gtag=AOTAB_Tfoot;
         break;
      case MARKUP_TBODY:
         gtag=AOTAB_Tbody;
         break;
   }
   if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         gtag,FALSE,
         TAG_END);
   }
   return TRUE;
}

/*** <TR> ***/
static BOOL Dotr(struct Document *doc,struct Tagattr *ta)
{  short halign=-1,valign=-1;
   struct Colorinfo *bgcolor=NULL,*bordercolor=NULL,*borderdark=NULL,*borderlight=NULL;
   void *bgimg=NULL;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_ALIGN:
            halign=Gethalign(ATTR(doc,ta));
            break;
         case TAGATTR_VALIGN:
            valign=Getvalign(ATTR(doc,ta),STRICT);
            if(STRIEQUAL(ATTR(doc,ta),"BASELINE")) valign=VALIGN_BASELINE;
            break;
         case TAGATTR_BACKGROUND:
            if(!STRICT) bgimg=Backgroundimg(doc,ATTR(doc,ta));
            break;
         case TAGATTR_BGCOLOR:
            if(!Setbodycolor(doc,&bgcolor,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_BORDERCOLOR:
            if(!STRICT && !Setbodycolor(doc,&bordercolor,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_BORDERCOLORDARK:
            if(!STRICT && !Setbodycolor(doc,&borderdark,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_BORDERCOLORLIGHT:
            if(!STRICT && !Setbodycolor(doc,&borderlight,ATTR(doc,ta))) return FALSE;
            break;
      }
   }
   if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         AOTAB_Vspacing,doc->gotbreak,
         AOTAB_Row,TRUE,
         CONDTAG(AOTAB_Halign,halign),
         CONDTAG(AOTAB_Valign,valign),
         AOTAB_Bgimage,(Tag)bgimg,
         AOTAB_Bgcolor,(Tag)bgcolor,
         AOTAB_Bordercolor,(Tag)bordercolor,
         AOTAB_Borderdark,(Tag)borderdark,
         AOTAB_Borderlight,(Tag)borderlight,
         TAG_END);
      doc->gotbreak=2;
      doc->wantbreak=0;
      if(Agetattr(Docbodync(doc),AOBDY_Style)!=STYLE_PRE) doc->pflags&=~DPF_PREFORMAT;
   }
   if(!Ensuresp(doc)) return FALSE;
   doc->pflags&=~DPF_BLINK;
   return TRUE;
}

/*** </TR> ***/
static BOOL Dotrend(struct Document *doc)
{  if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         AOTAB_Vspacing,doc->gotbreak,
         AOTAB_Row,FALSE,
         TAG_END);
      doc->gotbreak=0;
      doc->wantbreak=0;
      if(Agetattr(Docbodync(doc),AOBDY_Style)!=STYLE_PRE) doc->pflags&=~DPF_PREFORMAT;
   }
   if(!Ensuresp(doc)) return FALSE;
   doc->pflags&=~DPF_BLINK;
   return TRUE;
}

/*** <TD>,<TH> ***/
static BOOL Dotd(struct Document *doc,struct Tagattr *ta,BOOL heading)
{  short halign=-1,valign=-1,rowspan=-1,colspan=-1;
   short width=-1,height=-1;
   ULONG wtag=TAG_IGNORE,htag=TAG_IGNORE;
   BOOL nowrap=FALSE;
   struct Number num;
   struct Colorinfo *bgcolor=NULL,*bordercolor=NULL,*borderdark=NULL,*borderlight=NULL;
   void *bgimg=NULL;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_ALIGN:
            halign=Gethalign(ATTR(doc,ta));
            break;
         case TAGATTR_VALIGN:
            valign=Getvalign(ATTR(doc,ta),STRICT);
            if(STRIEQUAL(ATTR(doc,ta),"BASELINE")) valign=VALIGN_BASELINE;
            break;
         case TAGATTR_WIDTH:
            width=Getnumber(&num,ATTR(doc,ta));
            if(width<=0) width=0;
            if(num.type==NUMBER_PERCENT)
            {  wtag=AOTAB_Percentwidth;
            }
            else if(num.type==NUMBER_RELATIVE)
            {  wtag=AOTAB_Relwidth;
            }
            else
            {  wtag=AOTAB_Pixelwidth;
            }
            break;
         case TAGATTR_HEIGHT:
            height=Getnumber(&num,ATTR(doc,ta));
            if(height<0) height=0;
            if(num.type==NUMBER_PERCENT)
            {  htag=AOTAB_Percentheight;
            }
            else
            {  htag=AOTAB_Pixelheight;
            }
            break;
         case TAGATTR_ROWSPAN:
            rowspan=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_COLSPAN:
            colspan=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_NOWRAP:
            nowrap=TRUE;
            break;
         case TAGATTR_BACKGROUND:
            if(!STRICT) bgimg=Backgroundimg(doc,ATTR(doc,ta));
            break;
         case TAGATTR_BGCOLOR:
            if(!Setbodycolor(doc,&bgcolor,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_BORDERCOLOR:
            if(!STRICT && !Setbodycolor(doc,&bordercolor,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_BORDERCOLORDARK:
            if(!STRICT && !Setbodycolor(doc,&borderdark,ATTR(doc,ta))) return FALSE;
            break;
         case TAGATTR_BORDERCOLORLIGHT:
            if(!STRICT && !Setbodycolor(doc,&borderlight,ATTR(doc,ta))) return FALSE;
            break;
      }
   }
   if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         AOTAB_Vspacing,doc->gotbreak,
         (heading?AOTAB_Hcell:AOTAB_Cell),TRUE,
         CONDTAG(AOTAB_Halign,halign),
         CONDTAG(AOTAB_Valign,valign),
         CONDTAG(AOTAB_Rowspan,rowspan),
         CONDTAG(AOTAB_Colspan,colspan),
         wtag,width,
         htag,height,
         AOTAB_Nowrap,nowrap,
         AOTAB_Bgimage,(Tag)bgimg,
         AOTAB_Bgcolor,(Tag)bgcolor,
         AOTAB_Bordercolor,(Tag)bordercolor,
         AOTAB_Borderdark,(Tag)borderdark,
         AOTAB_Borderlight,(Tag)borderlight,
         TAG_END);
      doc->gotbreak=2;
      doc->wantbreak=0;
      if(Agetattr(Docbodync(doc),AOBDY_Style)!=STYLE_PRE) doc->pflags&=~DPF_PREFORMAT;
   }
   if(!Ensuresp(doc)) return FALSE;
   doc->pflags&=~DPF_BLINK;
   Checkid(doc,ta);
   return TRUE;
}

/*** </TD>,</TH> ***/
static BOOL Dotdend(struct Document *doc)
{  if(!ISEMPTY(&doc->tables))
   {  Asetattrs(doc->tables.first->table,
         AOTAB_Vspacing,doc->gotbreak,
         AOTAB_Cell,FALSE,
         TAG_END);
      doc->gotbreak=0;
      doc->wantbreak=0;
      if(Agetattr(Docbodync(doc),AOBDY_Style)!=STYLE_PRE) doc->pflags&=~DPF_PREFORMAT;
   }
   if(!Ensuresp(doc)) return FALSE;
   doc->pflags&=~DPF_BLINK;
   return TRUE;
}

/*--- sound -----------------------------------------------------------*/

/*** <BGSOUND> ***/
static BOOL Dobgsound(struct Document *doc,struct Tagattr *ta)
{  UBYTE *src=NULL;
   long loop=1;
   struct Number num;
   void *url,*referer;
   if(!STRICT)
   {  for(;ta->next;ta=ta->next)
      {  switch(ta->attr)
         {  case TAGATTR_SRC:
               src=ATTR(doc,ta);
               break;
            case TAGATTR_LOOP:
               if(STRNIEQUAL(ATTR(doc,ta),"INF",3)) loop=-1;
               else loop=Getnumber(&num,ATTR(doc,ta));
               break;
         }
      }
      if(src && *src)
      {  if(!(url=Findurl(doc->base,src,0))) return FALSE;
         referer=(void *)Agetattr(doc->source->source,AOSRC_Url);
         if(!(doc->bgsound=Anewobject(AOTP_COPY,
            AOBJ_Pool,(Tag)doc->pool,
            AOBJ_Frame,(Tag)doc->frame,
            AOBJ_Cframe,(Tag)doc->frame,
            AOBJ_Window,(Tag)doc->win,
            AOCPY_Url,(Tag)url,
            AOCPY_Referer,(Tag)referer,
            AOCPY_Defaulttype,(Tag)"audio/x-unknown",
            AOCPY_Soundloop,loop,
            AOCPY_Reloadverify,(doc->pflags&DPF_RELOADVERIFY),
            TAG_END))) return FALSE;
         if(doc->win) Asetattrs(doc->win,AOWIN_Bgsound,TRUE,TAG_END);
      }
   }
   return TRUE;
}

/*--- forms -----------------------------------------------------------*/

/*** <FORM> ***/
static BOOL Doform(struct Document *doc,struct Tagattr *ta)
{  UBYTE *action=doc->base;
   UBYTE *target=doc->target;
   UBYTE *name=NULL;
   UBYTE *onreset=NULL,*onsubmit=NULL;
   UWORD method=FORMTH_GET;
   BOOL multipart=FALSE;
   struct Url *url;
   void *form;
   if(doc->pflags&DPF_FORM) return TRUE;  /* No nested forms */
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_METHOD:
            if(STRIEQUAL(ATTR(doc,ta),"GET")) method=FORMTH_GET;
            else if(STRIEQUAL(ATTR(doc,ta),"POST")) method=FORMTH_POST;
            break;
         case TAGATTR_ACTION:
            action=ATTR(doc,ta);
            break;
         case TAGATTR_TARGET:
            target=ATTR(doc,ta);
            break;
         case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_ONRESET:
            onreset=ATTR(doc,ta);
            break;
         case TAGATTR_ONSUBMIT:
            onsubmit=ATTR(doc,ta);
            break;
         case TAGATTR_ENCTYPE:
            if(STRIEQUAL(ATTR(doc,ta),"MULTIPART/FORM-DATA")) multipart=TRUE;
            break;
      }
   }
   if(!(url=Findurl(doc->base,action,0))) return FALSE;
   if(!(form=Anewobject(AOTP_FORM,
      AOBJ_Pool,(Tag)doc->pool,
      AOBJ_Window,(Tag)doc->win,
      AOBJ_Frame,(Tag)doc->frame,
      AOBJ_Cframe,(Tag)doc->frame,
      AOFOR_Method,method,
      AOFOR_Action,(Tag)url,
      AOFOR_Target,(Tag)target,
      AOFOR_Multipart,multipart,
      AOFOR_Name,(Tag)name,
      AOFOR_Onreset,(Tag)onreset,
      AOFOR_Onsubmit,(Tag)onsubmit,
      AOFOR_Charset,(Tag)doc->charset,
      TAG_END))) return FALSE;
   ADDTAIL(&doc->forms,form);
   doc->pflags|=DPF_FORM;
   Asetattrs(Docbodync(doc),AOBDY_Align,-1,TAG_END);
   Wantbreak(doc,1);
   Checkid(doc,ta);
   return TRUE;
}

/*** </FORM> ***/

static BOOL Dobuttonend(struct Document *doc);

static BOOL Doformend(struct Document *doc)
{
   if(doc->button)
   {
      /* Ouch some designer left a button open! Close it */
      Dobuttonend(doc);
   }
   if(doc->pflags&DPF_FORM)
   {  Asetattrs(doc->forms.last,AOFOR_Complete,TRUE,TAG_END);
      doc->pflags&=~DPF_FORM;
   }
   return TRUE;
}

/*** Add a form button */
static BOOL Addbutton(struct Document *doc,struct Tagattr *ta,BOOL custom)
{  UBYTE *name=NULL,*value=NULL;
   UBYTE *onclick=NULL,*onfocus=NULL,*onblur=NULL;
   UWORD type=0;
   void *elt;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_VALUE:
            value=ATTR(doc,ta);
            break;
         case TAGATTR_TYPE:
            if(STRIEQUAL(ATTR(doc,ta),"SUBMIT")) type=BUTTP_SUBMIT;
            else if(STRIEQUAL(ATTR(doc,ta),"RESET")) type=BUTTP_RESET;
            else if(STRIEQUAL(ATTR(doc,ta),"BUTTON")) type=BUTTP_BUTTON;
            break;
         case TAGATTR_ONCLICK:
            onclick=ATTR(doc,ta);
            break;
         case TAGATTR_ONFOCUS:
            onfocus=ATTR(doc,ta);
            break;
         case TAGATTR_ONBLUR:
            onblur=ATTR(doc,ta);
            break;
      }
   }
   if(!(elt=Anewobject(AOTP_BUTTON,
      AOBJ_Pool,(Tag)doc->pool,
      AOBJ_Frame,(Tag)doc->frame,
      AOBJ_Cframe,(Tag)doc->cdv.cframe,
      AOBJ_Window,(Tag)doc->win,
      AOFLD_Form,doc->pflags&DPF_FORM?(Tag)doc->forms.last:0,
      AOFLD_Name,(Tag)name,
      AOFLD_Value,(Tag)value,
      AOFLD_Onclick,(Tag)onclick,
      AOFLD_Onfocus,(Tag)onfocus,
      AOFLD_Onblur,(Tag)onblur,
      AOBUT_Custom,custom,
      AOBUT_Type,type,
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   if(custom) doc->button=elt;
   if(!Ensurenosp(doc)) return FALSE;
   return TRUE;
}

/*** </BUTTON> ***/
static BOOL Dobuttonend(struct Document *doc)
{  if(doc->button)
   {  Asetattrs(doc->button,AOBUT_Complete,TRUE,TAG_END);
      doc->button=NULL;
      if(Agetattr(Docbody(doc),AOBDY_Style)!=STYLE_PRE) doc->pflags&=~DPF_PREFORMAT;
      Ensurenosp(doc);
   }
   return TRUE;
}

/*** <BUTTON> ***/
static BOOL Dobutton(struct Document *doc,struct Tagattr *ta)
{  //if(doc->pflags&DPF_FORM)
   {  if(doc->button)
      {  Dobuttonend(doc);
      }
      Addbutton(doc,ta,TRUE);
      Ensuresp(doc);
   }
   return TRUE;
}

/*** <INPUT TYPE=SUBMIT,RESET,BUTTON ***/
static BOOL Dofldbutton(struct Document *doc,struct Tagattr *ta)
{  return Addbutton(doc,ta,FALSE);
}

/*** <INPUT TYPE=CHECKBOX> ***/
static BOOL Dofldcheckbox(struct Document *doc,struct Tagattr *ta)
{  UBYTE *name=NULL,*value=NULL;
   UBYTE *onclick=NULL,*onfocus=NULL,*onblur=NULL;
   BOOL checked=FALSE;
   void *elt;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_VALUE:
            value=ATTR(doc,ta);
            break;
         case TAGATTR_CHECKED:
            checked=TRUE;
            break;
         case TAGATTR_ONCLICK:
            onclick=ATTR(doc,ta);
            break;
         case TAGATTR_ONFOCUS:
            onfocus=ATTR(doc,ta);
            break;
         case TAGATTR_ONBLUR:
            onblur=ATTR(doc,ta);
            break;
      }
   }
   if(!(elt=Anewobject(AOTP_CHECKBOX,
      AOBJ_Pool,(Tag)doc->pool,
      AOFLD_Form,doc->pflags&DPF_FORM?(Tag)doc->forms.last:0,
      AOFLD_Name,(Tag)name,
      AOFLD_Value,(Tag)value,
      AOFLD_Onclick,(Tag)onclick,
      AOFLD_Onfocus,(Tag)onfocus,
      AOFLD_Onblur,(Tag)onblur,
      AOCHB_Checked,checked,
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   if(!Ensurenosp(doc)) return FALSE;
   return TRUE;
}

/*** <INPUT TYPE=IMAGE> ***/
static BOOL Dofldimage(struct Document *doc,struct Tagattr *ta)
{  UBYTE *name=NULL;
   short width=-1,height=-1,hspace=-1,vspace=-1;
   short align=-1,flalign=-1;
   long alt=-1,altlen=-1;
   UBYTE *src=NULL;
   void *elt,*url,*referer;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_SRC:
            src=ATTR(doc,ta);
            break;
         case TAGATTR_ALIGN:
            align=Getvalign(ATTR(doc,ta),STRICT);
            flalign=Getflalign(ATTR(doc,ta));
            break;
         case TAGATTR_ALT:
            alt=doc->text.length;
            altlen=ta->length;
            if(!(Addtotextbuf(doc,ATTR(doc,ta),ta->length))) return FALSE;
            break;
         case TAGATTR_WIDTH:
            width=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_HEIGHT:
            height=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_HSPACE:
            hspace=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_VSPACE:
            vspace=Getposnumber(ATTR(doc,ta));
            break;
      }
   }
   if(src)
   {  if(!(url=Findurl(doc->base,src,0))) return FALSE;
      referer=(void *)Agetattr(doc->source->source,AOSRC_Url);
      if(!Ensurebody(doc)) return FALSE;
      if(!(elt=Anewobject(AOTP_COPY,
         AOBJ_Pool,(Tag)doc->pool,
         CONDTAG(AOELT_Valign,align),
         CONDTAG(AOELT_Floating,flalign),
         CONDTAG(AOELT_Textpos,alt),
         CONDTAG(AOELT_Textlength,altlen),
         AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
         AOFLD_Form,doc->pflags&DPF_FORM?(Tag)doc->forms.last:0,
         AOFLD_Name,(Tag)name,
         AOCPY_Url,(Tag)url,
         AOCPY_Embedded,TRUE,
         AOCPY_Text,(Tag)&doc->text,
         AOCPY_Reloadverify,(doc->pflags&DPF_RELOADVERIFY),
         AOCPY_Defaulttype,(Tag)"image/x-unknown",
         CONDTAG(AOCPY_Width,width),
         CONDTAG(AOCPY_Height,height),
         CONDTAG(AOCPY_Hspace,hspace),
         CONDTAG(AOCPY_Vspace,vspace),
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      Asetattrs(elt,AOBJ_Layoutparent,(Tag)doc->body,TAG_END);
      if(!Ensurenosp(doc)) return FALSE;
   }
   return TRUE;
}

/*** <INPUT TYPE=RADIO> ***/
static BOOL Dofldradio(struct Document *doc,struct Tagattr *ta)
{  UBYTE *name=NULL,*value=NULL;
   UBYTE *onclick=NULL,*onfocus=NULL,*onblur=NULL;
   BOOL checked=FALSE;
   void *elt;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_VALUE:
            value=ATTR(doc,ta);
            break;
         case TAGATTR_CHECKED:
            checked=TRUE;
            break;
         case TAGATTR_ONCLICK:
            onclick=ATTR(doc,ta);
            break;
         case TAGATTR_ONFOCUS:
            onfocus=ATTR(doc,ta);
            break;
         case TAGATTR_ONBLUR:
            onblur=ATTR(doc,ta);
            break;
      }
   }
   if(!(elt=Anewobject(AOTP_RADIO,
      AOBJ_Pool,(Tag)doc->pool,
      AOFLD_Form,doc->pflags&DPF_FORM?(Tag)doc->forms.last:0,
      AOFLD_Name,(Tag)name,
      AOFLD_Value,(Tag)value,
      AOFLD_Onclick,(Tag)onclick,
      AOFLD_Onfocus,(Tag)onfocus,
      AOFLD_Onblur,(Tag)onblur,
      AORAD_Checked,checked,
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   if(!Ensurenosp(doc)) return FALSE;
   return TRUE;
}

/*** <INPUT TYPE=HIDDEN> ***/
static BOOL Dofldhidden(struct Document *doc,struct Tagattr *ta)
{  UBYTE *name=NULL,*value="";
   void *elt;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_VALUE:
            value=ATTR(doc,ta);
            break;
      }
   }
   if(!(elt=Anewobject(AOTP_HIDDEN,
      AOBJ_Pool,(Tag)doc->pool,
      AOELT_Visible,FALSE,
      AOFLD_Form,doc->pflags&DPF_FORM?(Tag)doc->forms.last:0,
      AOFLD_Name,(Tag)name,
      AOFLD_Value,(Tag)value,
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   return TRUE;
}

/*** <INPUT TYPE=FILE> ***/
static BOOL Dofldfile(struct Document *doc,struct Tagattr *ta)
{  UBYTE *name=NULL,*value=NULL;
   UBYTE *onchange=NULL,*onfocus=NULL,*onblur=NULL;
   long size=-1;
   void *elt;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_VALUE:
            value=ATTR(doc,ta);
            break;
         case TAGATTR_SIZE:
            size=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_ONCHANGE:
            onchange=ATTR(doc,ta);
            break;
         case TAGATTR_ONFOCUS:
            onfocus=ATTR(doc,ta);
            break;
         case TAGATTR_ONBLUR:
            onblur=ATTR(doc,ta);
            break;
      }
   }
   if(!(elt=Anewobject(AOTP_FILEFIELD,
      AOBJ_Pool,(Tag)doc->pool,
      AOFLD_Form,doc->pflags&DPF_FORM?(Tag)doc->forms.last:0,
      AOFLD_Name,(Tag)name,
      AOFLD_Value,(Tag)value,
      AOFLD_Onchange,(Tag)onchange,
      AOFLD_Onfocus,(Tag)onfocus,
      AOFLD_Onblur,(Tag)onblur,
      CONDTAG(AOFUF_Size,size),
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   if(!Ensurenosp(doc)) return FALSE;
   return TRUE;
}

/*** <INPUT TYPE=TEXT,PASSWORD> ***/
static BOOL Dofldtext(struct Document *doc,struct Tagattr *ta)
{  UBYTE *name=NULL,*value=NULL;
   UBYTE *onchange=NULL,*onfocus=NULL,*onblur=NULL,*onselect=NULL;
   long maxlength=-1,size=-1;
   UWORD type=INPTP_TEXT;
   void *elt;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_TYPE:
            if(STRIEQUAL(ATTR(doc,ta),"PASSWORD")) type=INPTP_PASSWORD;
            else type=INPTP_TEXT;
            break;
         case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_VALUE:
            value=ATTR(doc,ta);
            break;
         case TAGATTR_MAXLENGTH:
            maxlength=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_SIZE:
            size=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_ONCHANGE:
            onchange=ATTR(doc,ta);
            break;
         case TAGATTR_ONFOCUS:
            onfocus=ATTR(doc,ta);
            break;
         case TAGATTR_ONBLUR:
            onblur=ATTR(doc,ta);
            break;
         case TAGATTR_ONSELECT:
            onselect=ATTR(doc,ta);
            break;
      }
   }
   if(!(elt=Anewobject(AOTP_INPUT,
      AOBJ_Pool,(Tag)doc->pool,
      AOFLD_Form,doc->pflags&DPF_FORM?(Tag)doc->forms.last:0,
      AOFLD_Name,(Tag)name,
      AOFLD_Value,(Tag)value,
      AOFLD_Onchange,(Tag)onchange,
      AOFLD_Onfocus,(Tag)onfocus,
      AOFLD_Onblur,(Tag)onblur,
      AOFLD_Onselect,(Tag)onselect,
      AOINP_Type,type,
      CONDTAG(AOINP_Maxlength,maxlength),
      CONDTAG(AOINP_Size,size),
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   if(!Ensurenosp(doc)) return FALSE;
   return TRUE;
}

/*** <INPUT> ***/
static BOOL Doinput(struct Document *doc,struct Tagattr *ta)
{  struct Tagattr *tattrs=ta;
   if(!STRICT || doc->pflags&DPF_FORM)
   {  for(;ta->next;ta=ta->next)
      {  switch(ta->attr)
         {  case TAGATTR_TYPE:
               if(STRIEQUAL(ATTR(doc,ta),"TEXT")) return Dofldtext(doc,tattrs);
               else if(STRIEQUAL(ATTR(doc,ta),"PASSWORD")) return Dofldtext(doc,tattrs);
               else if(STRIEQUAL(ATTR(doc,ta),"SUBMIT")) return Dofldbutton(doc,tattrs);
               else if(STRIEQUAL(ATTR(doc,ta),"RESET")) return Dofldbutton(doc,tattrs);
               else if(STRIEQUAL(ATTR(doc,ta),"BUTTON")) return Dofldbutton(doc,tattrs);
               else if(STRIEQUAL(ATTR(doc,ta),"HIDDEN")) return Dofldhidden(doc,tattrs);
               else if(STRIEQUAL(ATTR(doc,ta),"CHECKBOX")) return Dofldcheckbox(doc,tattrs);
               else if(STRIEQUAL(ATTR(doc,ta),"RADIO")) return Dofldradio(doc,tattrs);
               else if(STRIEQUAL(ATTR(doc,ta),"IMAGE")) return Dofldimage(doc,tattrs);
               else if(STRIEQUAL(ATTR(doc,ta),"FILE")) return Dofldfile(doc,tattrs);
               break;
         }
      }
      return Dofldtext(doc,tattrs);
   }
   return TRUE;
}

/*** <ISINDEX> ***/
static BOOL Doisindex(struct Document *doc,struct Tagattr *ta)
{  UBYTE *prompt=NULL;
   void *elt,*form;
   struct Url *url;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_PROMPT:
            prompt=ATTR(doc,ta);
            break;
      }
   }
   if(!prompt) prompt=AWEBSTR(MSG_AWEB_INDEXPROMPT);
   Wantbreak(doc,2);
   if(!(elt=Anewobject(AOTP_TEXT,
      AOBJ_Pool,(Tag)doc->pool,
      AOELT_Textpos,doc->text.length,
      AOELT_Textlength,strlen(prompt),
      AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
      AOTXT_Blink,doc->pflags&DPF_BLINK,
      AOTXT_Text,(Tag)&doc->text,
      TAG_END))) return FALSE;
   if(!Addelement(doc,elt)) return FALSE;
   if(!Addtotextbuf(doc,prompt,strlen(prompt))) return FALSE;
   if(!(url=Findurl(doc->base,"",0))) return FALSE;
   doc->pflags|=DPF_FORM;
   if(!(form=Anewobject(AOTP_FORM,
      AOBJ_Pool,(Tag)doc->pool,
      AOBJ_Window,(Tag)doc->win,
      AOBJ_Frame,(Tag)doc->frame,
      AOBJ_Cframe,(Tag)doc->frame,
      AOFOR_Method,FORMTH_INDEX,
      AOFOR_Action,(Tag)url,
      AOFOR_Charset,(Tag)doc->charset,
      TAG_END))) return FALSE;
   ADDTAIL(&doc->forms,form);
   if(!Dofldtext(doc,&dummyta)) return FALSE;
   doc->pflags&=~DPF_FORM;
   Wantbreak(doc,2);
   return TRUE;
}

/*** <OPTION> ***/
static BOOL Dooption(struct Document *doc,struct Tagattr *ta)
{  UBYTE *value=NULL;
   BOOL selected=FALSE;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_VALUE:
            value=ATTR(doc,ta);
            break;
         case TAGATTR_SELECTED:
            selected=TRUE;
            break;
      }
   }
   if(doc->select)
   {  Asetattrs(doc->select,
         AOSEL_Option,TRUE,
         AOSEL_Optionvalue,(Tag)value,
         AOSEL_Selected,selected,
         TAG_END);
      doc->pmode=DPM_OPTION;
      if(!Ensuresp(doc)) return FALSE;
   }
   return TRUE;
}

/*** text within <OPTION> ***/
static BOOL Dooptiontext(struct Document *doc,struct Tagattr *ta)
{  if(doc->select)
   {  Asetattrs(doc->select,
         AOSEL_Optiontext,(Tag)ATTR(doc,ta),
         TAG_END);
   }
   return TRUE;
}

/*** <SELECT> ***/
static BOOL Doselect(struct Document *doc,struct Tagattr *ta)
{  UBYTE *name=NULL,*value=NULL;
   UBYTE *onchange=NULL,*onfocus=NULL,*onblur=NULL;
   BOOL multiple=FALSE;
   short size=-1;
   void *elt;
 //  if(doc->pflags&DPF_FORM)
   {  Checkid(doc,ta);
      for(;ta->next;ta=ta->next)
      {  switch(ta->attr)
         {  case TAGATTR_NAME:
               name=ATTR(doc,ta);
               break;
            case TAGATTR_VALUE:
               value=ATTR(doc,ta);
               break;
            case TAGATTR_MULTIPLE:
               multiple=TRUE;
               break;
            case TAGATTR_SIZE:
               size=Getposnumber(ATTR(doc,ta));
               break;
            case TAGATTR_ONCHANGE:
               onchange=ATTR(doc,ta);
               break;
            case TAGATTR_ONFOCUS:
               onfocus=ATTR(doc,ta);
               break;
            case TAGATTR_ONBLUR:
               onblur=ATTR(doc,ta);
               break;
         }
      }
      if(!(elt=Anewobject(AOTP_SELECT,
         AOBJ_Pool,(Tag)doc->pool,
         AOFLD_Form,doc->pflags&DPF_FORM?(Tag)doc->forms.last:0,
         AOFLD_Name,(Tag)name,
         AOFLD_Value,(Tag)value,
         AOFLD_Onchange,(Tag)onchange,
         AOFLD_Onfocus,(Tag)onfocus,
         AOFLD_Onblur,(Tag)onblur,
         CONDTAG(AOSEL_Size,size),
         AOSEL_Multiple,multiple,
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      if(!Ensurenosp(doc)) return FALSE;
      doc->select=elt;
   }
   return TRUE;
}

/*** </SELECT> ***/
static BOOL Doselectend(struct Document *doc)
{  doc->pmode=DPM_BODY; /* clear OPTION mode */
   if(doc->select)
   {  Asetattrs(doc->select,AOSEL_Complete,TRUE,TAG_END);
      doc->select=NULL;
   }
   return TRUE;
}

/*** <TEXTAREA> ***/
static BOOL Dotextarea(struct Document *doc,struct Tagattr *ta)
{  UBYTE *name=NULL;
   UBYTE *onchange=NULL,*onfocus=NULL,*onblur=NULL,*onselect=NULL;
   short cols=-1,rows=-1;
   void *elt;
   if(doc->pflags&DPF_FORM)
   {  Checkid(doc,ta);
      for(;ta->next;ta=ta->next)
      {  switch(ta->attr)
         {  case TAGATTR_NAME:
               name=ATTR(doc,ta);
               break;
            case TAGATTR_COLS:
               cols=Getposnumber(ATTR(doc,ta));
               break;
            case TAGATTR_ROWS:
               rows=Getposnumber(ATTR(doc,ta));
               break;
            case TAGATTR_ONCHANGE:
               onchange=ATTR(doc,ta);
               break;
            case TAGATTR_ONFOCUS:
               onfocus=ATTR(doc,ta);
               break;
            case TAGATTR_ONBLUR:
               onblur=ATTR(doc,ta);
               break;
            case TAGATTR_ONSELECT:
               onselect=ATTR(doc,ta);
               break;
         }
      }
      if(!(elt=Anewobject(AOTP_TEXTAREA,
         AOBJ_Pool,(Tag)doc->pool,
         AOFLD_Form,doc->pflags&DPF_FORM?(Tag)doc->forms.last:0,
         AOFLD_Name,(Tag)name,
         AOFLD_Onchange,(Tag)onchange,
         AOFLD_Onfocus,(Tag)onfocus,
         AOFLD_Onblur,(Tag)onblur,
         AOFLD_Onselect,(Tag)onselect,
         CONDTAG(AOTXA_Cols,cols),
         CONDTAG(AOTXA_Rows,rows),
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      if(!Ensurenosp(doc)) return FALSE;
      doc->pmode=DPM_TEXTAREA;
      doc->textarea=elt;
   }
   return TRUE;
}

/*** </TEXTAREA> ***/
static BOOL Dotextareaend(struct Document *doc)
{  doc->pmode=DPM_BODY; /* clear TEXTAREA mode */
   if(doc->textarea)
   {  Asetattrs(doc->textarea,AOTXA_Complete,TRUE,TAG_END);
      doc->textarea=NULL;
      if(!Ensurenosp(doc)) return FALSE;
   }
   return TRUE;
}

/*** text within <TEXTAREA> ***/
static BOOL Dotextareatext(struct Document *doc,struct Tagattr *ta)
{  if(doc->textarea)
   {  Asetattrs(doc->textarea,
         AOTXA_Text,(Tag)ATTR(doc,ta),
         TAG_END);
   }
   return TRUE;
}

/*--- object ----------------------------------------------------------*/

/*** <OBJECT> ***/
static BOOL Doobject(struct Document *doc,struct Tagattr *ta)
{  short border=-1,width=-1,height=-1,hspace=-1,vspace=-1;
   short align=-1,flalign=-1;
   long alt=-1,altlen=-1;
   void *usemap=NULL;
   UBYTE *data=NULL,*type=NULL,*codebase=NULL,*codetype=NULL,*classid=NULL,*name=NULL;
   UBYTE *mapname,*ttype;
   UBYTE *dummyp;
   UBYTE *onload=NULL,*onerror=NULL,*onabort=NULL;
   UBYTE *onclick=NULL,*onmouseout=NULL,*onmouseover=NULL;
   BOOL ismap=FALSE,declare=FALSE,shapes=TRUE;
   void *elt,*url,*referer;
   Checkid(doc,ta);
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_DATA:
            data=ATTR(doc,ta);
            break;
         case TAGATTR_TYPE:
            type=ATTR(doc,ta);
            break;
         case TAGATTR_CODEBASE:
            codebase=ATTR(doc,ta);
            break;
         case TAGATTR_CODETYPE:
            codetype=ATTR(doc,ta);
            break;
         case TAGATTR_CLASSID:
            classid=ATTR(doc,ta);
            break;
         case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_DECLARE:
            declare=TRUE;
            break;
         case TAGATTR_SHAPES:
            shapes=TRUE;
            /* Create anonymous map */
            usemap=Anewobject(AOTP_MAP,
               AOBJ_Pool,(Tag)doc->pool,
               TAG_END);
            if(usemap) ADDTAIL(&doc->maps,usemap);
            break;
         case TAGATTR_ALIGN:
            align=Getvalign(ATTR(doc,ta),STRICT);
            flalign=Getflalign(ATTR(doc,ta));
            break;
         case TAGATTR_STANDBY:
            if(ta->length)
            {  alt=doc->text.length;
               altlen=ta->length;
               if(!(Addtotextbuf(doc,ATTR(doc,ta),ta->length))) return FALSE;
            }
            break;
         case TAGATTR_ISMAP:
            if(!STRICT) ismap=TRUE;
            break;
         case TAGATTR_USEMAP:
            mapname=ATTR(doc,ta);
            shapes=FALSE;
            if(mapname[0]=='#')
            {  usemap=Findmap(doc,mapname+1);
            }
            else
            {  usemap=Externalmap(doc,mapname);
            }
            break;
         case TAGATTR_BORDER:
            border=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_WIDTH:
            width=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_HEIGHT:
            height=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_HSPACE:
            hspace=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_VSPACE:
            vspace=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_ONLOAD:
            onload=ATTR(doc,ta);
            break;
         case TAGATTR_ONERROR:
            onerror=ATTR(doc,ta);
            break;
         case TAGATTR_ONABORT:
            onabort=ATTR(doc,ta);
            break;
         case TAGATTR_ONCLICK:
            onclick=ATTR(doc,ta);
            break;
         case TAGATTR_ONMOUSEOVER:
            onmouseover=ATTR(doc,ta);
            break;
         case TAGATTR_ONMOUSEOUT:
            onmouseout=ATTR(doc,ta);
            break;

      }
   }
   /* We cannot handle external code yet */
   if(!codebase && !classid && !declare)
   {  if(data)
      {  /* Check if we understand the type */
         if(!(ttype=type))
         {  ttype=Mimetypefromext(data);
         }
         if(ttype && !STRNIEQUAL(ttype,"TEXT/",5)
         && Getmimedriver(ttype,NULL,&dummyp,&dummyp))
         {  if(!(url=Findurl(doc->base,data,0))) return FALSE;
            referer=(void *)Agetattr(doc->source->source,AOSRC_Url);
            if(!Ensurebody(doc)) return FALSE;
            if(!(elt=Anewobject(AOTP_COPY,
               AOBJ_Pool,(Tag)doc->pool,
               CONDTAG(AOELT_Valign,align),
               CONDTAG(AOELT_Floating,flalign),
               CONDTAG(AOELT_Textpos,alt),
               CONDTAG(AOELT_Textlength,altlen),
               AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
               AOCPY_Url,(Tag)url,
               AOCPY_Embedded,TRUE,
               AOCPY_Usemap,(Tag)usemap,
               AOCPY_Ismap,ismap,
               AOCPY_Text,(Tag)&doc->text,
               AOCPY_Referer,(Tag)referer,
               AOCPY_Defaulttype,type?(Tag)type:(Tag)"image/x-unknown",
               AOCPY_Reloadverify,(doc->pflags&DPF_RELOADVERIFY),
               CONDTAG(AOCPY_Border,border),
               CONDTAG(AOCPY_Width,width),
               CONDTAG(AOCPY_Height,height),
               CONDTAG(AOCPY_Hspace,hspace),
               CONDTAG(AOCPY_Vspace,vspace),
               AOCPY_Onload,(Tag)onload,
               AOCPY_Onerror,(Tag)onerror,
               AOCPY_Onabort,(Tag)onabort,
               AOCPY_Onclick,(Tag)onclick,
               AOCPY_Onmouseover,(Tag)onmouseover,
               AOCPY_Onmouseout,(Tag)onmouseout,
               AOCPY_Objectready,FALSE,
               TAG_END))) return FALSE;
            if(!Addelement(doc,elt)) return FALSE;
            Asetattrs(elt,AOBJ_Layoutparent,(Tag)doc->body,TAG_END);
            if(flalign<0)
            {  if(!Ensurenosp(doc)) return FALSE;
            }
            doc->currentobject=elt;
            doc->objectnest++;
            doc->pmode=DPM_OBJECT;
            if(shapes) doc->pflags|=DPF_OBJECTSHAPES;
         }
      }
   }
   return TRUE;
}

/*** </OBJECT> ***/
static BOOL Doobjectend(struct Document *doc)
{  if(doc->objectnest)
   {  doc->objectnest--;
      if(!doc->objectnest)
      {  Asetattrs(doc->currentobject,AOCPY_Objectready,TRUE,TAG_END);
         doc->currentobject=NULL;
         doc->pmode=DPM_BODY;
         doc->pflags&=~DPF_OBJECTSHAPES;
      }
   }
   return TRUE;
}

/*** <PARAM> ***/
static BOOL Doparam(struct Document *doc,struct Tagattr *ta)
{  UBYTE *name=NULL,*type=NULL,*value=NULL,*valuetype=NULL;
   if(doc->objectnest==1)
   {  for(;ta->next;ta=ta->next)
      {  switch(ta->attr)
         {  case TAGATTR_NAME:
               name=ATTR(doc,ta);
               break;
            case TAGATTR_TYPE:
               type=ATTR(doc,ta);
               break;
            case TAGATTR_VALUE:
               value=ATTR(doc,ta);
               break;
            case TAGATTR_VALUETYPE:
               valuetype=ATTR(doc,ta);
               break;
         }
      }
      Asetattrs(doc->currentobject,
         AOCPY_Paramname,(Tag)name,
         AOCPY_Paramtype,(Tag)type,
         AOCPY_Paramvalue,(Tag)value,
         AOCPY_Paramvaluetype,(Tag)valuetype,
         TAG_END);
   }
   return TRUE;
}

/*** <EMBED> ***/
static BOOL Doembed(struct Document *doc,struct Tagattr *ta)
{  struct Tagattr *orgta=ta;
   short width=-1,height=-1;
   UBYTE *src=NULL,*name=NULL,*value;
   void *elt,*url,*referer;
   BOOL hidden=FALSE;
   if(STRICT) return TRUE;
   for(;ta->next;ta=ta->next)
   {  switch(ta->attr)
      {  case TAGATTR_SRC:
            src=ATTR(doc,ta);
            break;
         case TAGATTR_NAME:
            name=ATTR(doc,ta);
            break;
         case TAGATTR_WIDTH:
            width=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_HEIGHT:
            height=Getposnumber(ATTR(doc,ta));
            break;
         case TAGATTR_EMBEDPARAMNAME:
            if(STRIEQUAL(ATTR(doc,ta),"HIDDEN")
            && ta->next->next && ta->next->attr==TAGATTR_EMBEDPARAMVALUE)
            {  if(STRIEQUAL(ATTR(doc,ta->next),"TRUE")
               || STRIEQUAL(ATTR(doc,ta->next),"YES"))
               {  hidden=TRUE;
               }
            }
            break;
      }
   }
   if(src)
   {  if(width==0 && height==0) hidden=TRUE;
      if(!(url=Findurl(doc->base,src,0))) return FALSE;
      referer=(void *)Agetattr(doc->source->source,AOSRC_Url);
      if(!Ensurebody(doc)) return FALSE;
      if(!(elt=Anewobject(AOTP_COPY,
         AOBJ_Pool,(Tag)doc->pool,
         AOELT_Preformat,doc->pflags&DPF_PREFORMAT,
         AOELT_Visible,!hidden,
         AOCPY_Url,(Tag)url,
         AOCPY_Embedded,TRUE,
         AOCPY_Referer,(Tag)referer,
         AOCPY_Defaulttype,(Tag)"image/x-unknown",
         AOCPY_Reloadverify,(doc->pflags&DPF_RELOADVERIFY),
         CONDTAG(AOCPY_Width,width),
         CONDTAG(AOCPY_Height,height),
         AOCPY_Objectready,FALSE,
         TAG_END))) return FALSE;
      if(!Addelement(doc,elt)) return FALSE;
      Asetattrs(elt,AOBJ_Layoutparent,(Tag)doc->body,TAG_END);
      /* See if there are odd named parameters */
      name=value=NULL;
      for(ta=orgta;ta->next;ta=ta->next)
      {  switch(ta->attr)
         {  case TAGATTR_EMBEDPARAMNAME:
               name=ATTR(doc,ta);
               break;
            case TAGATTR_EMBEDPARAMVALUE:
               value=ATTR(doc,ta);
               Asetattrs(elt,
                  AOCPY_Paramname,(Tag)name,
                  AOCPY_Paramvalue,(Tag)value,
                  TAG_END);
               break;
         }
      }
      Asetattrs(elt,AOCPY_Objectready,TRUE,TAG_END);
      if(!Ensurenosp(doc)) return FALSE;
   }
   return TRUE;
}

/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/

/* Clean up, close all open elements, etc. */
static void Doeof(struct Document *doc)
{  if(doc->pmode==DPM_TITLE)
   {  Dotitleend(doc);
   }
   if(doc->button)
   {
      Dobuttonend(doc);
   }
   if(doc->pmode==DPM_OPTION)
   {  Doselectend(doc);
   }
   if(doc->pmode==DPM_TEXTAREA)
   {  Dotextareaend(doc);
   }
   while(!ISEMPTY(&doc->tables))
   {  Dotableend(doc);
   }
   if(doc->pflags&DPF_FORM)
   {  Doformend(doc);
   }
   if(doc->framesets.first->next)
   {  while(doc->framesets.first->next->next)
      {  Popframeset(doc);
      }
      Asetattrs(doc->framesets.first->frameset,AOFRS_Endframeset,TRUE,TAG_END);
   }
   if(doc->doctype==DOCTP_BODY)
   {  /* Add a line break to end the last line and activate trailing <br> */
    //  Dobr(doc,&dummyta);
      Asetattrs(doc->body,AOBDY_End,TRUE,TAG_END);
   }
}

/*---------------------------------------------------------------------*/

BOOL Processhtml(struct Document *doc,UWORD tagtype,struct Tagattr *ta,BOOL selfclosing)
{
   BOOL result=TRUE;
   if(tagtype==MARKUP_EOF && !(doc->pflags&DPF_JPARSE)) Doeof(doc);

   if(doc->dflags&DDF_MAPDOCUMENT)
   {  tagtype=Mapdoctag(tagtype);
   }

   switch(doc->pmode)
   {  case DPM_TITLE:
         switch(tagtype)
         {  case MARKUP_TEXT:
               result=Dotitleadd(doc,ta);
               break;
            case MARKUP_TITLE|MARKUP_END:
               result=Dotitleend(doc);
               break;
            case MARKUP_HEAD|MARKUP_END:
               if(doc->htmlmode==HTML_COMPATIBLE)
               {  result=Dotitleend(doc);
               }
               break;
            case MARKUP_BODY:
               if(doc->htmlmode==HTML_COMPATIBLE)
               {  if(result=Dotitleend(doc))
                  {  result=Dobody(doc,ta);
                  }
               }
               break;
         }
         break;
      case DPM_STYLE:
         switch(tagtype)
         {  case MARKUP_STYLE|MARKUP_END:
               doc->pmode=DPM_BODY;
               break;
            case MARKUP_HEAD|MARKUP_END:
               if(doc->htmlmode==HTML_COMPATIBLE)
               {  doc->pmode=DPM_BODY;
               }
               break;
            case MARKUP_BODY:
               if(doc->htmlmode==HTML_COMPATIBLE)
               {  doc->pmode=DPM_BODY;
                  result=Dobody(doc,ta);
               }
               break;
         }
         break;
      case DPM_SCRIPT:
         switch(tagtype)
         {  case MARKUP_SCRIPT|MARKUP_END:
               doc->pmode=DPM_BODY;
               if(1||doc->pflags&DPF_JSCRIPT)
               {  /* execute external script now */
                  result=Doscriptend(doc);
               }
               break;
            case MARKUP_HEAD|MARKUP_END:
               if(doc->htmlmode==HTML_COMPATIBLE)
               {  doc->pmode=DPM_BODY;
               }
               break;
            case MARKUP_BODY:
               if(doc->htmlmode==HTML_COMPATIBLE)
               {  doc->pmode=DPM_BODY;
                  result=Dobody(doc,ta);
               }
               break;
         }
         break;
      case DPM_MAP:
         switch(tagtype)
         {  case MARKUP_AREA:
               result=Doarea(doc,ta,TRUE);
               break;
            case MARKUP_MAP|MARKUP_END:
               doc->pmode=DPM_BODY;
               break;
         }
         break;
      case DPM_OPTION:
         switch(tagtype)
         {  case MARKUP_TEXT:
               result=Dooptiontext(doc,ta);
               break;
            case MARKUP_OPTION:
               result=Dooption(doc,ta);
               break;
            case MARKUP_SELECT|MARKUP_END:
               result=Doselectend(doc);
               break;
            case MARKUP_FORM|MARKUP_END:
               result=Doselectend(doc)
                     || Doformend(doc);
               break;
         }
         break;
      case DPM_TEXTAREA:
         switch(tagtype)
         {  case MARKUP_TEXT:
               result=Dotextareatext(doc,ta);
               break;
            case MARKUP_TEXTAREA|MARKUP_END:
               result=Dotextareaend(doc);
               break;
            case MARKUP_FORM|MARKUP_END:
               result=Dotextareaend(doc)
                     || Doformend(doc);
               break;
         }
         break;
      case DPM_FRAMESET:
         switch(tagtype)
         {  case MARKUP_FRAME:
               result=Doframe(doc,ta);
               break;
            case MARKUP_FRAMESET:
               result=Doframeset(doc,ta);
               break;
            case MARKUP_FRAMESET|MARKUP_END:
               result=Doframesetend(doc);
               break;
            case MARKUP_NOFRAMES:
               break;
            case MARKUP_NOSCRIPT:
               if(!STRICT)
               {  result=Donoscript(doc,ta);
               }
               break;
         }
         break;
      case DPM_OBJECT:
         switch(tagtype)
         {  case MARKUP_OBJECT:
               doc->objectnest++;
               break;
            case MARKUP_OBJECT|MARKUP_END:
               result=Doobjectend(doc);
               break;
            case MARKUP_PARAM:
               result=Doparam(doc,ta);
               break;
            case MARKUP_A:
               if(doc->pflags&DPF_OBJECTSHAPES)
               {  result=Doarea(doc,ta,FALSE);
               }
               break;
         }
         break;
      case DPM_NOSCRIPT:
         switch(tagtype)
         {  case MARKUP_NOSCRIPT|MARKUP_END:
               result=Donoscriptend(doc);
               break;
         }
         break;
      case DPM_IFRAME:
         switch(tagtype)
         {  case MARKUP_IFRAME|MARKUP_END:
               doc->pmode=DPM_BODY;
               break;
         }
         break;
      default:
         switch(tagtype)
         {
         /*---- header contents ----*/
            case MARKUP_TITLE:
               result=Dotitle(doc);
               break;
            case MARKUP_STYLE:
               doc->pmode=DPM_STYLE;
               break;
            case MARKUP_SCRIPT:
               result=Doscript(doc,ta);
               break;
            case MARKUP_SCRIPT|MARKUP_END:
               result=Doscriptend(doc);
               break;
            case MARKUP_BASE:
               result=Dobase(doc,ta);
               break;
            case MARKUP_LINK:
               result=Dolink(doc,ta);
               break;
            case MARKUP_META:
               result=Dometa(doc,ta);
               break;
         /*---- body ----*/
            case MARKUP_BODY:
               result=Dobody(doc,ta);
               break;
            case MARKUP_NOSCRIPT:
               result=Donoscript(doc,ta);
               break;
         /*---- text and line breaks ----*/
            case MARKUP_BLINK:
               result=Doblink(doc,ta);
               break;
            case MARKUP_BLINK|MARKUP_END:
               result=Doblinkend(doc);
               break;
            case MARKUP_BR:
               result=Dobr(doc,ta);
               break;
            case MARKUP_CENTER:
               result=Docenter(doc,ta);
               break;
            case MARKUP_CENTER|MARKUP_END:
               result=Docenterend(doc);
               break;
            case MARKUP_DIV:
               result=Dodiv(doc,ta);
               break;
            case MARKUP_DIV|MARKUP_END:
               result=Dodivend(doc);
               break;
            case MARKUP_NOBR:
               result=Donobr(doc,ta);
               break;
            case MARKUP_NOBR|MARKUP_END:
               result=Donobrend(doc);
               break;
            case MARKUP_P:
               result=Dopara(doc,ta);
               break;
            case MARKUP_P|MARKUP_END:
               result=Doparaend(doc);
               break;
            case MARKUP_TEXT:
               if(doc->pflags&DPF_JSCRIPT)
               {  result=Dojsource(doc,ta);
               }
               else
               {  result=Dotext(doc,ta);
               }
               break;
            case MARKUP_WBR:
               result=Dowbr(doc,ta);
               break;
         /*---- preformatted ----*/
            case MARKUP_PRE:
               result=Dopre(doc,ta);
               break;
            case MARKUP_LISTING:
               result=Dolisting(doc,ta);
               break;
            case MARKUP_XMP:
               result=Doxmp(doc,ta);
               break;
            case MARKUP_PRE|MARKUP_END:
            case MARKUP_LISTING|MARKUP_END:
            case MARKUP_XMP|MARKUP_END:
               result=Dopreend(doc);
               break;
         /*---- hard styles ----*/
            case MARKUP_B:
               result=Dohardstyle(doc,ta,FSF_BOLD,TRUE);
               break;
            case MARKUP_B|MARKUP_END:
               result=Dohardstyle(doc,ta,FSF_BOLD,FALSE);
               break;
            case MARKUP_I:
               result=Dohardstyle(doc,ta,FSF_ITALIC,TRUE);
               break;
            case MARKUP_I|MARKUP_END:
               result=Dohardstyle(doc,ta,FSF_ITALIC,FALSE);
               break;
            case MARKUP_DEL:
               result=Dodel(doc, ta);
               break;
            case MARKUP_STRIKE:
               result=Dohardstyle(doc,ta,FSF_STRIKE,TRUE);
               break;
            case MARKUP_DEL|MARKUP_END:
               result=Dodelend(doc);
               break;
            case MARKUP_STRIKE|MARKUP_END:
               result=Dohardstyle(doc,ta,FSF_STRIKE,FALSE);
               break;
            case MARKUP_TT:
               result=Dott(doc,ta,TRUE);
               break;
            case MARKUP_TT|MARKUP_END:
               result=Dott(doc,ta,FALSE);
               break;
            case MARKUP_INS:
               result = Doins(doc, ta);
               break;
            case MARKUP_INS|MARKUP_END:
               result = Doinsend(doc);
               break;
            case MARKUP_U:
               result=Dohardstyle(doc,ta,FSF_UNDERLINED,TRUE);
               break;
            case MARKUP_U|MARKUP_END:
               result=Dohardstyle(doc,ta,FSF_UNDERLINED,FALSE);
               break;
         /*---- logical styles ----*/
            case MARKUP_CITE:
            case MARKUP_CODE:
            case MARKUP_DFN:
            case MARKUP_EM:
            case MARKUP_KBD:
            case MARKUP_SAMP:
            case MARKUP_STRONG:
            case MARKUP_VAR:
               result=Dostyle(doc,ta,STYLE_CITE+(tagtype-MARKUP_CITE));
               break;
            case MARKUP_BIG:
               result=Dostyle(doc,ta,STYLE_BIG);
               break;
            case MARKUP_SMALL:
               result=Dostyle(doc,ta,STYLE_SMALL);
               break;
            case MARKUP_CITE|MARKUP_END:
            case MARKUP_CODE|MARKUP_END:
            case MARKUP_DFN|MARKUP_END:
            case MARKUP_EM|MARKUP_END:
            case MARKUP_KBD|MARKUP_END:
            case MARKUP_SAMP|MARKUP_END:
            case MARKUP_STRONG|MARKUP_END:
            case MARKUP_VAR|MARKUP_END:
            case MARKUP_BIG|MARKUP_END:
            case MARKUP_SMALL|MARKUP_END:
               result=Dostyleend(doc);
               break;
            case MARKUP_ADDRESS:
               result=Doaddress(doc,ta);
               break;
            case MARKUP_ADDRESS|MARKUP_END:
               result=Doaddressend(doc);
               break;
            case MARKUP_BLOCKQUOTE:
               result=Doblockquote(doc,ta);
               break;
            case MARKUP_BLOCKQUOTE|MARKUP_END:
               result=Doblockquoteend(doc);
               break;
            case MARKUP_H1:
            case MARKUP_H2:
            case MARKUP_H3:
            case MARKUP_H4:
            case MARKUP_H5:
            case MARKUP_H6:
               result=Doheading(doc,tagtype-MARKUP_H1,ta);
               break;
            case MARKUP_H1|MARKUP_END:
            case MARKUP_H2|MARKUP_END:
            case MARKUP_H3|MARKUP_END:
            case MARKUP_H4|MARKUP_END:
            case MARKUP_H5|MARKUP_END:
            case MARKUP_H6|MARKUP_END:
               result=Doheadingend(doc);
               break;
            case MARKUP_SUB:
               result=Dosub(doc,ta);
               break;
            case MARKUP_SUB|MARKUP_END:
               result=Dosubend(doc);
               break;
            case MARKUP_SUP:
               result=Dosup(doc,ta);
               break;
            case MARKUP_SUP|MARKUP_END:
               result=Dosupend(doc);
               break;
         /*---- font ----*/
            case MARKUP_BASEFONT:
               result=Dobasefont(doc,ta);
               break;
            case MARKUP_FONT:
               result=Dofont(doc,ta);
               break;
            case MARKUP_FONT|MARKUP_END:
               result=Dofontend(doc);
               break;
         /*---- ruler ----*/
            case MARKUP_HR:
               result=Dohr(doc,ta);
               break;
         /*---- anchor ----*/
            case MARKUP_A:
               result=Doanchor(doc,ta);
               break;
            case MARKUP_A|MARKUP_END:
               result=Doanchorend(doc);
               break;
         /*---- frames ----*/
            case MARKUP_FRAME:
               result=Doframe(doc,ta);
               break;
            case MARKUP_FRAMESET:
               result=Doframeset(doc,ta);
               break;
            case MARKUP_IFRAME:
               result=Doiframe(doc,ta);
               break;
         /*---- images ----*/
            case MARKUP_IMG:
               result=Doimg(doc,ta);
               break;
            case MARKUP_ICON:
               result=Doicon(doc,ta);
               break;
            case MARKUP_MAP:
               result=Domap(doc,ta);
               break;
         /*---- lists ----*/
            case MARKUP_DD:
               result=Dodd(doc,ta);
               break;
            case MARKUP_DL:
               result=Dodl(doc,ta);
               break;
            case MARKUP_DL|MARKUP_END:
               result=Dodlend(doc);
               break;
            case MARKUP_DT:
               result=Dodt(doc,ta);
               break;
            case MARKUP_LI:
               result=Doli(doc,ta);
               break;
            case MARKUP_OL:
               result=Dool(doc,ta);
               break;
            case MARKUP_OL|MARKUP_END:
               result=Doolend(doc);
               break;
            case MARKUP_UL:
            case MARKUP_DIR:
            case MARKUP_MENU:
               result=Doul(doc,ta);
               break;
            case MARKUP_UL|MARKUP_END:
            case MARKUP_DIR|MARKUP_END:
            case MARKUP_MENU|MARKUP_END:
               result=Doulend(doc);
               break;
         /*---- tables ----*/
            case MARKUP_TABLE:
               result=Dotable(doc,ta);
               break;
            case MARKUP_TABLE|MARKUP_END:
               result=Dotableend(doc);
               break;
            case MARKUP_COLGROUP:
            case MARKUP_COL:
               result=Docolgrouporcol(doc,ta,tagtype);
               break;
            case MARKUP_COLGROUP|MARKUP_END:
               result=Docolgroupend(doc);
               break;
            case MARKUP_THEAD:
            case MARKUP_TFOOT:
            case MARKUP_TBODY:
               result=Dorowgroup(doc,ta,tagtype);
               break;
            case MARKUP_THEAD|MARKUP_END:
            case MARKUP_TFOOT|MARKUP_END:
            case MARKUP_TBODY|MARKUP_END:
               result=Dorowgroupend(doc,tagtype&~MARKUP_END);
               break;
            case MARKUP_TR:
               result=Dotr(doc,ta);
               break;
            case MARKUP_TR|MARKUP_END:
               result=Dotrend(doc);
               break;
            case MARKUP_TD:
               result=Dotd(doc,ta,FALSE);
               break;
            case MARKUP_TH:
               result=Dotd(doc,ta,TRUE);
               break;
            case MARKUP_TD|MARKUP_END:
            case MARKUP_TH|MARKUP_END:
               result=Dotdend(doc);
               break;
            case MARKUP_CAPTION:
               result=Docaption(doc,ta);
               break;
            case MARKUP_CAPTION|MARKUP_END:
               result=Docaptionend(doc);
               break;
         /*---- sound ----*/
            case MARKUP_BGSOUND:
               result=Dobgsound(doc,ta);
               break;
         /*---- forms ----*/
            case MARKUP_FORM:
               result=Doform(doc,ta);
               break;
            case MARKUP_FORM|MARKUP_END:
               result=Doformend(doc);
               break;
            case MARKUP_BUTTON:
               result=Dobutton(doc,ta);
               break;
            case MARKUP_BUTTON|MARKUP_END:
               result=Dobuttonend(doc);
               break;
            case MARKUP_INPUT:
               result=Doinput(doc,ta);
               break;
            case MARKUP_ISINDEX:
               result=Doisindex(doc,ta);
               break;
            case MARKUP_OPTION:
               result=Dooption(doc,ta);
               break;
            case MARKUP_SELECT:
               result=Doselect(doc,ta);
               break;
            case MARKUP_SELECT|MARKUP_END:
               result=Doselectend(doc);
               break;
            case MARKUP_TEXTAREA:
               result=Dotextarea(doc,ta);
               break;
         /*---- object ----*/
            case MARKUP_OBJECT:
               result=Doobject(doc,ta);
               break;
            case MARKUP_EMBED:
               result=Doembed(doc,ta);
               break;
         /*---- unknown ----*/
            case MARKUP_UNKNOWN:
               Checkid(doc,ta);
               break;

         /*----  ----*/
         /*----  ----*/
         /*----  ----*/
         }
   }

   if (result && selfclosing)
   {
       result = Processhtml(doc,tagtype | MARKUP_END, ta, FALSE);
   }
   return result;
}
