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

/* parse.c aweb html markup parse */

#include "aweb.h"
#include "html.h"
#include "application.h"
#include "docprivate.h"
#include "jslib.h"
#include "window.h"

#define MAXATTRS 40
static struct Tagattr tagattr[MAXATTRS];

static LIST(Tagattr) attrs;
static short nextattr;

struct Tagdes
{  UBYTE *name;
   UWORD type;
   BOOL container;
};

static struct Tagdes tags[]=
{  {"A",        MARKUP_A,            TRUE},
   {"ADDRESS",  MARKUP_ADDRESS,      TRUE},
   {"AREA",     MARKUP_AREA,         FALSE},
   {"B",        MARKUP_B,            TRUE},
   {"BASE",     MARKUP_BASE,         FALSE},
   {"BASEFONT", MARKUP_BASEFONT,     FALSE},
   {"BGSOUND",  MARKUP_BGSOUND,      FALSE},
   {"BIG",      MARKUP_BIG,          TRUE},
   {"BLINK",    MARKUP_BLINK,        TRUE},
   {"BLOCKQUOTE",MARKUP_BLOCKQUOTE,  TRUE},
   {"BODY",     MARKUP_BODY,         TRUE},
   {"BQ",       MARKUP_BLOCKQUOTE,   TRUE},
   {"BR",       MARKUP_BR,           FALSE},
   {"BUTTON",   MARKUP_BUTTON,       TRUE},
   {"CAPTION",  MARKUP_CAPTION,      TRUE},
   {"CENTER",   MARKUP_CENTER,       TRUE},
   {"CITE",     MARKUP_CITE,         TRUE},
   {"CODE",     MARKUP_CODE,         TRUE},
   {"COL",      MARKUP_COL,          FALSE},
   {"COLGROUP", MARKUP_COLGROUP,     TRUE},
   {"DD",       MARKUP_DD,           FALSE},
   {"DEL",      MARKUP_DEL,          TRUE},
   {"DFN",      MARKUP_DFN,          TRUE},
   {"DIR",      MARKUP_DIR,          TRUE},
   {"DIV",      MARKUP_DIV,          TRUE},
   {"DL",       MARKUP_DL,           TRUE},
   {"DT",       MARKUP_DT,           FALSE},
   {"EM",       MARKUP_EM,           TRUE},
   {"EMBED",    MARKUP_EMBED,        FALSE},
   {"FONT",     MARKUP_FONT,         TRUE},
   {"FORM",     MARKUP_FORM,         TRUE},
   {"FRAME",    MARKUP_FRAME,        FALSE},
   {"FRAMESET", MARKUP_FRAMESET,     TRUE},
   {"H1",       MARKUP_H1,           TRUE},
   {"H2",       MARKUP_H2,           TRUE},
   {"H3",       MARKUP_H3,           TRUE},
   {"H4",       MARKUP_H4,           TRUE},
   {"H5",       MARKUP_H5,           TRUE},
   {"H6",       MARKUP_H6,           TRUE},
   {"HEAD",     MARKUP_HEAD,         TRUE},
   {"HR",       MARKUP_HR,           FALSE},
   {"HTML",     MARKUP_HTML,         TRUE},
   {"I",        MARKUP_I,            TRUE},
   {"IFRAME",   MARKUP_IFRAME,       TRUE},
   {"IMAGE",    MARKUP_IMG,          FALSE},
   {"IMG",      MARKUP_IMG,          FALSE},
   {"INPUT",    MARKUP_INPUT,        FALSE},
   {"INS",      MARKUP_INS,          TRUE},
   {"ISINDEX",  MARKUP_ISINDEX,      FALSE},
   {"KBD",      MARKUP_KBD,          TRUE},
   {"LI",       MARKUP_LI,           FALSE},
   {"LINK",     MARKUP_LINK,         FALSE},
   {"LISTING",  MARKUP_LISTING,      TRUE},
   {"MAP",      MARKUP_MAP,          TRUE},
   {"MENU",     MARKUP_MENU,         TRUE},
   {"META",     MARKUP_META,         FALSE},
   {"NOBR",     MARKUP_NOBR,         TRUE},
   {"NOFRAME",  MARKUP_NOFRAMES,     TRUE},
   {"NOFRAMES", MARKUP_NOFRAMES,     TRUE},
   {"NOSCRIPT", MARKUP_NOSCRIPT,     TRUE},
   {"OBJECT",   MARKUP_OBJECT,       TRUE},
   {"OL",       MARKUP_OL,           TRUE},
   {"OPTION",   MARKUP_OPTION,       FALSE},
   {"P",        MARKUP_P,            TRUE},
   {"PARAM",    MARKUP_PARAM,        FALSE},
   {"PRE",      MARKUP_PRE,          TRUE},
   {"S",        MARKUP_STRIKE,       TRUE},
   {"SAMP",     MARKUP_SAMP,         TRUE},
   {"SCRIPT",   MARKUP_SCRIPT,       TRUE},
   {"SELECT",   MARKUP_SELECT,       TRUE},
   {"SMALL",    MARKUP_SMALL,        TRUE},
   {"STRIKE",   MARKUP_STRIKE,       TRUE},
   {"STRONG",   MARKUP_STRONG,       TRUE},
   {"STYLE",    MARKUP_STYLE,        TRUE},
   {"SUB",      MARKUP_SUB,          TRUE},
   {"SUP",      MARKUP_SUP,          TRUE},
   {"TABLE",    MARKUP_TABLE,        TRUE},
   {"TBODY",    MARKUP_TBODY,        TRUE},
   {"TD",       MARKUP_TD,           TRUE},
   {"TEXTAREA", MARKUP_TEXTAREA,     TRUE},
   {"TFOOT",    MARKUP_TFOOT,        TRUE},
   {"TH",       MARKUP_TH,           TRUE},
   {"THEAD",    MARKUP_THEAD,        TRUE},
   {"TITLE",    MARKUP_TITLE,        TRUE},
   {"TR",       MARKUP_TR,           TRUE},
   {"TT",       MARKUP_TT,           TRUE},
   {"U",        MARKUP_U,            TRUE},
   {"UL",       MARKUP_UL,           TRUE},
   {"VAR",      MARKUP_VAR,          TRUE},
   {"WBR",      MARKUP_WBR,          FALSE},
   {"XMP",      MARKUP_XMP,          TRUE},
};
#define NRTAGS (sizeof(tags)/sizeof(struct Tagdes))

struct Attrdes
{  UBYTE *name;
   UWORD type;
};

static struct Attrdes tagattrs[]=
{  {"ACTION",            TAGATTR_ACTION},
   {"ALIGN",             TAGATTR_ALIGN},
   {"ALINK",             TAGATTR_ALINK},
   {"ALT",               TAGATTR_ALT},
   {"BACKGROUND",        TAGATTR_BACKGROUND},
   {"BGCOLOR",           TAGATTR_BGCOLOR},
   {"BORDER",            TAGATTR_BORDER},
   {"BORDERCOLOR",       TAGATTR_BORDERCOLOR},
   {"BORDERCOLORDARK",   TAGATTR_BORDERCOLORDARK},
   {"BORDERCOLORLIGHT",  TAGATTR_BORDERCOLORLIGHT},
   {"CELLPADDING",       TAGATTR_CELLPADDING},
   {"CELLSPACING",       TAGATTR_CELLSPACING},
   {"CHECKED",           TAGATTR_CHECKED},
   {"CLASSID",           TAGATTR_CLASSID},
   {"CLEAR",             TAGATTR_CLEAR},
   {"CODEBASE",          TAGATTR_CODEBASE},
   {"CODETYPE",          TAGATTR_CODETYPE},
   {"COLOR",             TAGATTR_COLOR},
   {"COLS",              TAGATTR_COLS},
   {"COLSPAN",           TAGATTR_COLSPAN},
   {"CONTENT",           TAGATTR_CONTENT},
   {"CONTINUE",          TAGATTR_CONTINUE},
   {"COORDS",            TAGATTR_COORDS},
   {"DATA",              TAGATTR_DATA},
   {"DECLARE",           TAGATTR_DECLARE},
   {"DINGBAT",           TAGATTR_DINGBAT},
   {"ENCTYPE",           TAGATTR_ENCTYPE},
   {"FACE",              TAGATTR_FACE},
   {"FRAME",             TAGATTR_FRAME},
   {"FRAMEBORDER",       TAGATTR_FRAMEBORDER},
   {"FRAMESPACING",      TAGATTR_FRAMESPACING},
   {"HEIGHT",            TAGATTR_HEIGHT},
   {"HIDDEN",            TAGATTR_HIDDEN},
   {"HREF",              TAGATTR_HREF},
   {"HSPACE",            TAGATTR_HSPACE},
   {"HTTP-EQUIV",        TAGATTR_HTTP_EQUIV},
   {"ID",                TAGATTR_ID},
   {"ISMAP",             TAGATTR_ISMAP},
   {"LANGUAGE",          TAGATTR_LANGUAGE},
   {"LEFTMARGIN",        TAGATTR_LEFTMARGIN},
   {"LINK",              TAGATTR_LINK},
   {"LOOP",              TAGATTR_LOOP},
   {"MARGINHEIGHT",      TAGATTR_MARGINHEIGHT},
   {"MARGINWIDTH",       TAGATTR_MARGINWIDTH},
   {"MAXLENGTH",         TAGATTR_MAXLENGTH},
   {"METHOD",            TAGATTR_METHOD},
   {"MULTIPLE",          TAGATTR_MULTIPLE},
   {"NAME",              TAGATTR_NAME},
   {"NOHREF",            TAGATTR_NOHREF},
   {"NORESIZE",          TAGATTR_NORESIZE},
   {"NOSHADE",           TAGATTR_NOSHADE},
   {"NOWRAP",            TAGATTR_NOWRAP},
   {"ONABORT",           TAGATTR_ONABORT},
   {"ONBLUR",            TAGATTR_ONBLUR},
   {"ONCHANGE",          TAGATTR_ONCHANGE},
   {"ONCLICK",           TAGATTR_ONCLICK},
   {"ONERROR",           TAGATTR_ONERROR},
   {"ONFOCUS",           TAGATTR_ONFOCUS},
   {"ONLOAD",            TAGATTR_ONLOAD},
   {"ONMOUSEOUT",        TAGATTR_ONMOUSEOUT},
   {"ONMOUSEOVER",       TAGATTR_ONMOUSEOVER},
   {"ONRESET",           TAGATTR_ONRESET},
   {"ONSELECT",          TAGATTR_ONSELECT},
   {"ONSUBMIT",          TAGATTR_ONSUBMIT},
   {"ONUNLOAD",          TAGATTR_ONUNLOAD},
   {"PLAIN",             TAGATTR_PLAIN},
   {"PROMPT",            TAGATTR_PROMPT},
   {"REL",               TAGATTR_REL},
   {"ROWS",              TAGATTR_ROWS},
   {"ROWSPAN",           TAGATTR_ROWSPAN},
   {"RULES",             TAGATTR_RULES},
   {"SCROLLING",         TAGATTR_SCROLLING},
   {"SELECTED",          TAGATTR_SELECTED},
   {"SEQNUM",            TAGATTR_SEQNUM},
   {"SHAPE",             TAGATTR_SHAPE},
   {"SHAPES",            TAGATTR_SHAPES},
   {"SIZE",              TAGATTR_SIZE},
   {"SKIP",              TAGATTR_SKIP},
   {"SPAN",              TAGATTR_SPAN},
   {"SRC",               TAGATTR_SRC},
   {"STANDBY",           TAGATTR_STANDBY},
   {"START",             TAGATTR_START},
   {"TARGET",            TAGATTR_TARGET},
   {"TEXT",              TAGATTR_TEXT},
   {"TITLE",             TAGATTR_TITLE},
   {"TOPMARGIN",         TAGATTR_TOPMARGIN},
   {"TYPE",              TAGATTR_TYPE},
   {"USEMAP",            TAGATTR_USEMAP},
   {"VALIGN",            TAGATTR_VALIGN},
   {"VALUE",             TAGATTR_VALUE},
   {"VALUETYPE",         TAGATTR_VALUETYPE},
   {"VLINK",             TAGATTR_VLINK},
   {"VSPACE",            TAGATTR_VSPACE},
   {"WIDTH",             TAGATTR_WIDTH},
};
#define NRATTRS   (sizeof(tagattrs)/sizeof(struct Attrdes))

struct Chardes
{  UBYTE *name;
   UWORD ch;
};

static struct Chardes chars[]=
{  {"AElig", 198},
   {"Aacute",193},
   {"Acirc", 194},
   {"Agrave",192},
   {"Aring", 197},
   {"Atilde",195},
   {"Auml",  196},
   {"Ccedil",199},
   {"Dagger",8225},
   {"ETH",   208},
   {"Eacute",201},
   {"Ecirc", 202},
   {"Egrave",200},
   {"Euml",  203},
   {"Iacute",205},
   {"Icirc", 206},
   {"Igrave",204},
   {"Iuml",  207},
   {"Ntilde",209},
   {"OElig", 338},
   {"Oacute",211},
   {"Ocirc", 212},
   {"Ograve",210},
   {"Oslash",216},
   {"Otilde",213},
   {"Ouml",  214},
   {"Prime", 8243},
   {"Scaron",352},
   {"THORN", 222},
   {"Uacute",218},
   {"Ucirc", 219},
   {"Ugrave",217},
   {"Uuml",  220},
   {"Yacute",221},
   {"Yuml",  376},
   {"aacute",225},
   {"acirc", 226},
   {"acute", 180},
   {"aelig", 230},
   {"agrave",224},
   {"amp",   38},
   {"aring", 229},
   {"atilde",227},
   {"auml",  228},
   {"bdquo", 8222},
   {"brvbar",166},
   {"bull",  8226},
   {"ccedil",231},
   {"cedil", 184},
   {"cent",  162},
   {"circ",  710},
   {"copy",  169},
   {"curren",164},
   {"dagger",8224},
   {"deg",   176},
   {"divide",247},
   {"eacute",233},
   {"ecirc", 234},
   {"egrave",232},
   {"empty", 8709},
   {"emsp",  8195},
   {"ensp",  8194},
   {"eth",   240},
   {"euml",  235},
   {"euro",  8364},
   {"fnof",  402},
   {"frac12",189},
   {"frac14",188},
   {"frac34",190},
   {"frasl", 8260},
   {"ge",    8805},
   {"gt",    62},
   {"hellip",8230},
   {"iacute",237},
   {"icirc", 238},
   {"iexcl", 161},
   {"igrave",236},
   {"iquest",191},
   {"iuml",  239},
   {"lang",  9001},
   {"laquo", 171},
   {"ldquo", 8220},
   {"le",    8804},
   {"lowast",8727},
   {"loz",   9674},
   {"lsaquo",8249},
   {"lsquo", 8216},
   {"lt",    60},
   {"macr",  175},
   {"mdash", 8212},
   {"micro", 181},
   {"middot",183},
   {"minus", 8722},
   {"nbsp",  160},
   {"ndash", 8211},
   {"not",   172},
   {"ntilde",241},
   {"oacute",243},
   {"ocirc", 244},
   {"oelig", 339},
   {"ograve",242},
   {"oline", 8254},
   {"ordf",  170},
   {"ordm",  186},
   {"oslash",248},
   {"otilde",245},
   {"ouml",  246},
   {"para",  182},
   {"permil",8240},
   {"plusmn",177},
   {"pound", 163},
   {"prime", 8242},
   {"quot",  34},
   {"rang",  9002},
   {"raquo", 187},
   {"rdquo", 8221},
   {"reg",   174},
   {"rsaquo",8250},
   {"rsquo", 8217},
   {"sbquo", 8218},
   {"scaron",353},
   {"sdot",  8901},
   {"sect",  167},
   {"shy",   173},
   {"sim",   8764},
   {"sup1",  185},
   {"sup2",  178},
   {"sup3",  179},
   {"szlig", 223},
   {"thinsp",8201},
   {"thorn", 254},
   {"tilde", 732},
   {"times", 215},
   {"trade", 8482},
   {"uacute",250},
   {"ucirc", 251},
   {"ugrave",249},
   {"uml",   168},
   {"uuml",  252},
   {"yacute",253},
   {"yen",   165},
   {"yuml",  255},
   {"zwj",   8205},
   {"zwnj",  8204},
};
#define NRCHARS (sizeof(chars)/sizeof(struct Chardes))

static UBYTE *icons[]=
{  "archive",
   "audio",
   "binary.document",
   "binhex.document",
   "calculator",
   "caution",
   "cd.i",
   "cd.rom",
   "clock",
   "compressed.document",
   "disk.drive",
   "diskette",
   "display",
   "document",
   "fax",
   "filing.cabinet",
   "film",
   "fixed.disk",
   "folder",
   "form",
   "ftp",
   "glossary",
   "gopher",
   "home",
   "html",
   "image",
   "index",
   "keyboard",
   "mail",
   "mail.in",
   "mail.out",
   "map",
   "mouse",
   "network",
   "new",
   "next",
   "notebook",
   "parent",
   "play.fast.forward",
   "play.fast.reverse",
   "play.pause",
   "play.start",
   "play.stop",
   "previous",
   "printer",
   "sadsmiley",
   "smiley",
   "stop",
   "summary",
   "telephone",
   "telnet",
   "text.document",
   "tn3270",
   "toc",
   "trash",
   "unknown.document",
   "uuencoded.document",
   "work",
   "www",
};
#define NRICONS (sizeof(icons)/sizeof(UBYTE *))

static struct Tagdes *Findtag(UBYTE *name)
{  short a=0,b=NRTAGS-1,m;
   long c;
   while(a<=b)
   {  m=(a+b)/2;
      c=stricmp(tags[m].name,name);
      if(c==0) return &tags[m];
      if(c<0) a=m+1;
      else b=m-1;
   }
   return NULL;
}

static struct Attrdes *Findattr(UBYTE *name)
{  short a=0,b=NRATTRS-1,m;
   long c;
   while(a<=b)
   {  m=(a+b)/2;
      c=stricmp(tagattrs[m].name,name);
      if(c==0) return &tagattrs[m];
      if(c<0) a=m+1;
      else b=m-1;
   }
   return NULL;
}

static struct Chardes *Findchar(UBYTE *name)
{  short a=0,b=NRCHARS-1,m;
   long c;
   while(a<=b)
   {  m=(a+b)/2;
      c=strncmp(chars[m].name,name,strlen(chars[m].name));
      if(c==0) {
                        return &chars[m];
                }
      if(c<0) a=m+1;
      else b=m-1;
   }
   return NULL;
}

static UBYTE *Findicon(UBYTE *name)
{  short a=0,b=NRICONS-1,m;
   long c;
   while(a<=b)
   {  m=(a+b)/2;
      c=strcmp(icons[m],name);
      if(c==0) return icons[m];
      if(c<0) a=m+1;
      else b=m-1;
   }
   return NULL;
}

static struct Tagattr *Nextattr(struct Document *doc)
{  struct Tagattr *ta;
   if(nextattr==MAXATTRS) REMOVE(&tagattr[--nextattr]);
   ta=&tagattr[nextattr++];
   ADDTAIL(&attrs,ta);
   ta->attr=0;
   ta->valuepos=doc->args.length;
   ta->length=0;
   return ta;
}

/* Remove newlines, and all spaces surrounding newlines, from attribute value */
static void Removenl(struct Buffer *buf,struct Tagattr *ta)
{  UBYTE *p=buf->buffer+ta->valuepos;
   UBYTE *q,*s;
   UBYTE *end=p+ta->length;
   BOOL skipsp=FALSE;
   q=p;     /* destination */
   s=NULL;  /* first space written to destination */
   while(p<end)
   {  if(Isspace(*p))
      {  if(!s) s=q;
         if(*p=='\n')
         {  skipsp=TRUE;
            q=s;
         }
         if(!skipsp) *q++=*p;
      }
      else
      {  *q++=*p;
         s=NULL;
         skipsp=FALSE;
      }
      p++;
   }
   *q='\0';
   ta->length=q-(buf->buffer+ta->valuepos);
}

static void Translate(struct Document *doc,struct Buffer *buf,struct Tagattr *ta,
   BOOL isattr)
{  LONG index=ta->valuepos;
   LONG endIndex=index + ta->length;
   UBYTE *r, *chartable;
   UBYTE ebuf[12];
   ULONG n;
   BOOL valid;
   BOOL strict=(doc->htmlmode==HTML_STRICT),lf=(doc->pmode==DPM_TEXTAREA);
   BOOL compatible = (doc->htmlmode==HTML_COMPATIBLE);

   /* check if the windows charset is forced if so use it */
   /* may want to think about forcing frames instead of windows */

   chartable=(UBYTE *)Agetattr(doc->win,AOWIN_Chartable);
   if(!chartable)
   {
       chartable=doc->charset;
   }

   while (index  < endIndex)
   {  n=buf->buffer[index];
      if (buf->buffer[index]=='&')
      {  ULONG entityIndex=index+1;
         if (entityIndex >= endIndex) break;
         if (entityIndex + 1 < endIndex && buf->buffer[entityIndex] == '#')
         {  n = 0;
            entityIndex++;
            if (isdigit(buf->buffer[entityIndex])) {
               while (entityIndex <= endIndex && isdigit(buf->buffer[entityIndex])) {
                  n = 10 * n + (buf->buffer[entityIndex] - '0');
                  entityIndex++;
               }
            } else if (toupper(buf->buffer[entityIndex]) == 'X') {
               entityIndex++;
               while (entityIndex <= endIndex && isxdigit(buf->buffer[entityIndex])) {
                  char c = buf->buffer[entityIndex];
                  n = 16 * n + (c <= '9' ? c - '0' : toupper(c) - 'A' + 10);
                  entityIndex++;
               }
            }

            /* Blindly replace the entity by the character value.
             * Validity check and translation is done below.
             * First remember the exact source in case we don't know the character. */
            if (entityIndex <= endIndex)
            {  short l;
               if (buf->buffer[entityIndex] != ';') entityIndex--;
               l = entityIndex - index + 1;
               if(l>11) l=11;
               strncpy(ebuf, buf->buffer + index,l);
               ebuf[l]='\0';
            } else {
               entityIndex = endIndex;
            }
            Deleteinbuffer(buf, index, entityIndex - index);
            ta->length -= entityIndex - index;
            endIndex -= entityIndex - index;

            buf->buffer[index] = (UBYTE)n;

         }
         else if(isattr && buf->buffer[entityIndex] == '{')
         {  /* JavaScript expression */
            ULONG jIndex;
            UBYTE *s;
            entityIndex++;
            for(jIndex = entityIndex; jIndex < endIndex && buf->buffer[jIndex] == '}'; jIndex++);
            if(s=Dupstr(buf->buffer + entityIndex, jIndex - entityIndex))
            {  struct Jcontext *jc;
               struct Jvar *jv;
               UBYTE *result;
               ULONG l;
               doc->dflags|=DDF_NOSPARE;
               Runjavascript(doc->frame,s,NULL);
               jc=(struct Jcontext *)Agetattr(Aweb(),AOAPP_Jcontext);
               if(jc && (jv=Jgetreturnvalue(jc)))
               {  result=Jtostring(jc,jv);
               }
               else
               {  result="";
               }
               jIndex++;
               if(jIndex < endIndex && buf->buffer[jIndex] == ';') jIndex++;
               l=strlen(result);
               Deleteinbuffer(buf, index, jIndex - index);
               if (Insertinbuffer(buf, result, l, index)) {
                  ta->length = ta->length - (jIndex - index) + l;
                  endIndex = endIndex - (jIndex - index) + l;
                  index = index + l - 1;
               } else {
                  /* something went wrong, we deleted but we couldn't insert */
                  ta->length -=  jIndex - index;
                  endIndex -= jIndex - index;
               }
               FREE(s);
            }
            /* Below we will set buf->buffer[index] = n, so we make that a no-op */
            n = buf->buffer[index];
         }
         else
         {  UBYTE name[8];
            struct Chardes *cd;
            short i;
            for(i=0;
               entityIndex < endIndex && i < 7 &&
                  (isalnum(buf->buffer[entityIndex])
                  || buf->buffer[entityIndex]=='.'
                  || buf->buffer[entityIndex]=='-');
               entityIndex++,i++) name[i] = buf->buffer[entityIndex];
            name[i]='\0';
            n = buf->buffer[index];    /* in case we don't find a replacement */
            if(cd=Findchar(name))
            {
               /* Allow unterminated names when compatible; when strict or tolerant name
                * must be terminated by tag or non-alphanumeric and match entirely.
                */

               if( (compatible || ((entityIndex >= endIndex || !isalnum(buf->buffer[entityIndex])) && STREQUAL(cd->name,name) ) ))

               {  entityIndex = index + 1 + strlen(cd->name); /* +1 because of & */
                  if(entityIndex < endIndex)
                  {  if(buf->buffer[entityIndex] != ';') entityIndex--;
                     memmove(buf->buffer + index, buf->buffer + entityIndex,
                           endIndex + 1 - entityIndex);
                  }
                  else entityIndex--;
                  ta->length -= (entityIndex - index);
                  endIndex -= (entityIndex - index);
                  n=cd->ch;
                  buf->buffer[index]=cd->ch;
               }
            }

         }
      }
      /* Translate document to local character set.
       * (n) contains character (possibly Unicode) */
      r=NULL;
/*      if(!strict && !(doc->dflags&DDF_FOREIGN) && n>=128 && n<=159)
      {
                        switch(n)
         {  case 130:n=(UBYTE)',';break;
            case 131:n=(UBYTE)'f';break;
            case 132:n=(UBYTE)'"';break;
            case 133:r="...";break;
            case 134:n=(UBYTE)'+';break;
            case 135:n=(UBYTE)'+';break;
            case 136:n=(UBYTE)'^';break;
            case 137:r="°/..";break;
            case 138:n=(UBYTE)'S';break;   // S hacek
            case 139:n=(UBYTE)'{';break;
            case 140:r="OE";break;
            case 145:n=(UBYTE)'`';break;
            case 146:n=(UBYTE)'\'';break;
            case 147:n=(UBYTE)'"';break;
            case 148:n=(UBYTE)'"';break;
            case 149:n=(UBYTE)'·';break;
            case 150:n=(UBYTE)'-';break;
            case 151:n=(UBYTE)'-';break;
            case 152:n=(UBYTE)'~';break;
            case 153:r="TM";break;
            case 154:n=(UBYTE)'s';break;   // s hacek
            case 155:n=(UBYTE)'}';break;
            case 156:r="oe";break;
            case 159:n=(UBYTE)'Y';break;   // Y trema
         }
      }
      else
*/
      /* This piece is left as is yet, i'll return to Unicode stuff later - Pavel Fedin */
      if(n>255)
      {
                        switch(n)
         {  case 338:r="OE";break;
            case 339:r="oe";break;
            case 352:n=(UBYTE)'S';break;
            case 353:n=(UBYTE)'s';break;
            case 376:n=(UBYTE)'Y';break;
            case 402:n=(UBYTE)'f';break;
            case 710:n=(UBYTE)'°';break;
            case 732:n=(UBYTE)'~';break;
            case 8194:n=(UBYTE)' ';break;
            case 8195:n=(UBYTE)' ';break;
            case 8201:n=(UBYTE)' ';break;
            case 8204:n=(UBYTE)' ';break;
            case 8205:n=(UBYTE)' ';break;
            case 8211:n=(UBYTE)'-';break;
            case 8212:n=(UBYTE)'-';break;
            case 8216:n=(UBYTE)'`';break;
            case 8217:n=(UBYTE)'\'';break;
            case 8218:n=(UBYTE)'"';break;
            case 8220:n=(UBYTE)'"';break;
            case 8221:n=(UBYTE)'"';break;
            case 8222:n=(UBYTE)'"';break;
            case 8224:n=(UBYTE)'+';break;
            case 8225:n=(UBYTE)'+';break;
            case 8226:n=(UBYTE)'·';break;
            case 8230:r="...";break;
            case 8240:r="°/..";break;
            case 8242:n=(UBYTE)'\'';break;
            case 8243:n=(UBYTE)'"';break;
            case 8249:n=(UBYTE)'<';break;
            case 8250:n=(UBYTE)'>';break;
            case 8254:n=(UBYTE)'¯';break;
            case 8260:n=(UBYTE)'/';break;
            case 8364:n=(UBYTE)164;break;
            case 8482:r="TM";break;
            case 8709:n=(UBYTE)'ø';break;
            case 8722:n=(UBYTE)'-';break;
            case 8727:n=(UBYTE)'*';break;
            case 8764:n=(UBYTE)'~';break;
            case 8804:r="<=";break;
            case 8805:r=">=";break;
            case 8901:n=(UBYTE)'·';break;
            case 9001:n=(UBYTE)'<';break;
            case 9002:n=(UBYTE)'>';break;
            case 9674:n=(UBYTE)'¤';break;
            default:
               r=ebuf;
         }
      }
      else
         n=chartable[n];
      if(r)
      {
         ULONG l = strlen(r);


                        Deleteinbuffer(buf,index,1);
         if (Insertinbuffer(buf,r,l,index)) {
            ta->length=ta->length+l-1;
            endIndex=endIndex+l-1;
            index=index+l-1;
         } else {
            /* something went wrong, we deleted but we couldn't insert */
            ta->length = ta->length - 1;
            endIndex = endIndex - 1;
            index = index - 1;
         }
      }

      /* replace invalid number by space if compatible */
      valid=(n>=32 && n<=126) || (n>=160 && n<=255) || (lf && n==10);
      if((valid || doc->htmlmode==HTML_COMPATIBLE) && !r)
      {  if(!valid) n=32;
         buf->buffer[index]=n;
      }
      else if(n==0 && !strict)
      {  buf->buffer[index]=' ';
      }
      else if(n==9)
      {  buf->buffer[index]=' ';
      }
      index++;
   }

}

/* inspect text, find icon entities and split up */
static void Lookforicons(struct Document *doc,struct Tagattr *ta)
{  if((doc->pflags&(DPF_PREFORMAT|DPF_JSCRIPT))
   || doc->pmode==DPM_TITLE
   || doc->pmode==DPM_OPTION
   || doc->pmode==DPM_TEXTAREA
   || doc->htmlmode==HTML_STRICT)
   {  Translate(doc,&doc->args,ta,FALSE);
      Processhtml(doc,MARKUP_TEXT,ta,FALSE);
   }
   else
   {  UBYTE name[32],*q;
      short i;
      UBYTE *icon;
      UBYTE *begin=doc->args.buffer+ta->valuepos,*p;
      UBYTE *end=begin+ta->length;
      p=begin;
      while(p<end)
      {  if(*p=='&')
         {  for(i=0,q=p+1;
                  q<end && i<31 && (isalnum(*q) || *q=='.' || *q=='-');
                  i++,q++)
               name[i]=*q;
            name[i]='\0';
            if(icon=Findicon(name))
            {  struct Tagattr tta={0};
               tta.attr=TAGATTR_TEXT;
               tta.valuepos=ta->valuepos;
               tta.length=p-begin;
               Translate(doc,&doc->args,&tta,FALSE);
               Processhtml(doc,MARKUP_TEXT,&tta,FALSE);
               tta.attr=TAGATTR_SRC;
               tta.valuepos=(long)icon;
               Processhtml(doc,MARKUP_ICON,&tta,FALSE);
               if(q<end && *q==';') q++;
               ta->valuepos+=q-begin;
               ta->length-=q-begin;
               begin=p=q;
            }
            else p++;
         }
         else p++;
      }
      Translate(doc,&doc->args,ta,FALSE);
      Processhtml(doc,MARKUP_TEXT,ta,FALSE);
   }
}

/* End of text reached, If eof, notify HTML doc. */
static BOOL Eofandexit(struct Document *doc,BOOL eof)
{  if(eof)
   {  Processhtml(doc,MARKUP_EOF,attrs.first,FALSE);
   }
   return TRUE;
}

/* Parse comment in html mode. Initial p points after initial "<!--"
 * Return new buffer pointer, or NULL when eof */
static int a = 0;
/* Strict HTML:  <!{--comment--wsp}> */

UBYTE *Parsecommentstrict(UBYTE *p,UBYTE *end,BOOL eof)
{
    UBYTE *q;
    UBYTE *r = p;
    a++;
    for(;;)
    {
        while( p<end-1 && *p != '>') p++; /* Skip to next close delimiter > */
        q  = p -1;
        /* back up checking for whitespace and -- */
        while (isspace(*q)  && q > r)
        {
            q--;
        }
        if(q > r && *q == '-')
        {
            q--;
            if(*q == '-') break;
        }
        if (p >= end -1) return NULL;
        p++;
    }
    p++;
    return p;
}

/* Tolerant: Try strict first, but if wsp is something else than wsp,
 * or "---" is found, then redo using    <!--any> */
UBYTE *Parsecommenttolerant(UBYTE *p,UBYTE *end,BOOL eof)
{  UBYTE *savep=p;
   for(;;)
   {  while(p<end-1 && !(p[0]=='-' && p[1]=='-')) p++;   /* Skip to closing -- */
      p+=2;
      if(p<end && *p=='-')
      {  /* "---" */
         savep = p;
         break;
      }
      while(p<end && isspace(*p)) p++;                   /* Skip whitespace */
      if(p<end && *p=='>')
      {  p++;
         return p;
      }
      else if(p>=end-1)
      {  if(!eof) return NULL;
         /* EOF found, retry */
         break;
      }
      else if(p[0]=='-' && p[1]=='-')
      {  /* Still a valid strict comment */
         p+=2;                                           /* Skip next opening -- */
         if(p<end && *p=='-')
         {  /* "---" */
            savep = p;
            break;
         }
      }
      else
      {  /* Whitespace is no whitespace, retry */
         break;
      }
   }
   /* If loop was broken, retry */
   p=savep;
   while(p<end && *p!='>') p++;                          /* Skip to > */
   p++;
   return p;
}

/* Compatible:   <!--comment> */
UBYTE *Parsecommentcompatible(UBYTE *p,UBYTE *end,BOOL eof)
{  while(p<end && !(p[0]=='>')) p++;      /* Skip to > */
   if(p>=end) return NULL;
   p++;
   return p;
}

BOOL Parsehtml(struct Document *doc,struct Buffer *src,BOOL eof,long *srcpos)
{  UBYTE *p=src->buffer+(*srcpos);
   UBYTE *end=src->buffer+src->length;
   UBYTE *q;
   UBYTE buf[32];
   UBYTE quote;
   struct Tagdes *td;
   struct Attrdes *tattr;
   short i;
   struct Tagattr *ta;
   UWORD tagtype;
   BOOL thisisdata=FALSE;  /* looks like tag but is in fact data */
   BOOL removenl=FALSE;    /* Remove newlines from URL values */
   BOOL skipnewline;       /* Current value of the DPF_SKIPNEWLINE flag */
   BOOL selfclosing=FALSE; /* support xml/xhtml style <element /> */
   long oldsrcpos;
   /* Skip leading nullbytes */
   if((*srcpos)==0)
   {  while(p<end && !*p) p++;
      *srcpos=p-src->buffer;
   }
   while(p<end && !(doc->pflags&DPF_SUSPEND))
   {  NEWLIST(&attrs);
      nextattr=0;
      doc->args.length=0;
      skipnewline=BOOLVAL(doc->pflags&DPF_SKIPNEWLINE);

      /* if we are inside textarea dont parse anything */
      if(doc->pmode==DPM_TEXTAREA)
      {
          thisisdata=TRUE;
          if(strnicmp( &p[1],"/textarea",9)==0) thisisdata = FALSE;
      }

      if(p==end-1 && *p=='<' && !thisisdata) return Eofandexit(doc,eof);
      if(p<end-1 && *p=='<' && (isalpha(p[1]) || p[1]=='/' || p[1]=='!' || (p[1]=='?' && p[2]=='x' && p[3]=='m' && p[4]=='l')  ) && !thisisdata)
      {  BOOL endtag=FALSE;
         if(++p>=end) return Eofandexit(doc,eof);
         if(*p=='!' && !(doc->pflags&(DPF_XMP|DPF_LISTING|DPF_JSCRIPT)))
         {  tagtype=0;
            p++;
            if(p>=end-1) return Eofandexit(doc,eof);
            if(p[0]=='-' && p[1]=='-')    /* <!-- */
            {  p+=2;                      /* skip opening -- */
               switch(doc->htmlmode)
               {  case HTML_STRICT:
                     p=Parsecommentstrict(p,end,eof);
                     break;
                  case HTML_TOLERANT:
                     p=Parsecommenttolerant(p,end,eof);
                     break;
                  default:
                     p=Parsecommentcompatible(p,end,eof);
                     break;
               }
               if(!p || p>=end) return Eofandexit(doc,eof);
            }
            else  /* No real comment */
            {  while(p<end && *p!='>') p++;  /* skip up to > */
               if(p>=end) return Eofandexit(doc,eof);
               p++;                          /* skip > */
            }
         }
         else
         {  if(*p=='/')
            {  endtag=TRUE;
               if(++p>=end) return Eofandexit(doc,eof);
            }
            q=buf;
            i=0;
            while(p<end && Issgmlchar(*p) && i<31)
            {  *q++=*p++;
               i++;
            }
            if(p>=end) return Eofandexit(doc,eof);
            *q='\0';
            td=Findtag(buf);
            if(doc->pflags&DPF_XMP)
            {  if(!(endtag && td && td->type==MARKUP_XMP))
               {  thisisdata=TRUE;
                  p=src->buffer+(*srcpos);
                  continue;   /* try again */
               }
            }
            if(doc->pflags&DPF_LISTING)
            {  if(!(endtag && td && td->type==MARKUP_LISTING))
               {  thisisdata=TRUE;
                  p=src->buffer+(*srcpos);
                  continue;   /* try again */
               }
            }
            if(doc->pflags&DPF_JSCRIPT)
            {  if(!(endtag && td && td->type==MARKUP_SCRIPT))
               {  thisisdata=TRUE;
                  p=src->buffer+(*srcpos);
                  continue;   /* try again */
               }
            }
            for(;;)
            {  while(p<end && isspace(*p)) p++;
               if(p>=end) return Eofandexit(doc,eof);
               if(*p=='>') break;
               if((*p=='/') && (*(p+1)=='>') && td && td->container)
               {
                   selfclosing=TRUE;
                   p++;
                   break;
               }

               if(doc->htmlmode!=HTML_STRICT && *p=='<')
               {  p--;  /* Don't skip over '<' yet */
                  break;
               }
               ta=Nextattr(doc);
               i=0;q=buf;
               if(!Issgmlchar(*p)) p++;   /* Skip invalid character to avoid endless loop */
               while(p<end && Issgmlchar(*p) && i<31)
               {  *q++=*p++;
                  i++;
               }
               if(p>=end) return Eofandexit(doc,eof);
               *q='\0';
               tattr=Findattr(buf);
               /* If <EMBED>, only allow valid attributes. Pass others as EMBEDPARAM pairs */
               if(td && td->type==MARKUP_EMBED)
               {  switch(tattr?tattr->type:0)
                  {  case TAGATTR_WIDTH:
                     case TAGATTR_HEIGHT:
                     case TAGATTR_NAME:
                     case TAGATTR_SRC:
                        ta->attr=tattr->type;
                        break;
                     default:
                        ta->attr=TAGATTR_EMBEDPARAMNAME;
                        if(!Addtobuffer(&doc->args,buf,strlen(buf)+1)) return FALSE;
                        ta=Nextattr(doc);
                        ta->attr=TAGATTR_EMBEDPARAMVALUE;
                  }
               }
               else if(tattr) ta->attr=tattr->type;
               while(p<end && isspace(*p)) p++;
               if(p>=end) return Eofandexit(doc,eof);
               if(*p=='=')
               {  p++;
                  while(p<end && isspace(*p)) p++;
                  if(p>=end) return Eofandexit(doc,eof);
                  if(*p=='"' || *p=='\'')
                  {  quote=*p;
                     if(++p>=end) return Eofandexit(doc,eof);
                     q=p;
                     removenl=FALSE;
                     if(doc->htmlmode!=HTML_STRICT)
                     {  switch(ta->attr)
                        {  case TAGATTR_ACTION:
                           case TAGATTR_BACKGROUND:
                           case TAGATTR_DATA:
                           case TAGATTR_HREF:
                           case TAGATTR_SRC:
                           case TAGATTR_USEMAP:
                              removenl=TRUE;
                              break;
                        }
                     }
                     while(q<end && *q!=quote)
                     {  if(doc->htmlmode==HTML_COMPATIBLE)
                        {  /* terminate quoted attribute on '>' */
                           if(*q=='>') break;
                           /* terminate URL on whitespace */
                           if(isspace(*q)
                           && (ta->attr==TAGATTR_HREF
                              || ta->attr==TAGATTR_SRC
                              || ta->attr==TAGATTR_ACTION)) break;
                        }
                        if(*q=='\r' || *q=='\n')
                        {  if(!Addtobuffer(&doc->args,p,q-p)) return FALSE;
                           ta->length+=(q-p);
                           if(*q=='\r')
                           {  if(q>=end-1) return Eofandexit(doc,eof);
                              if(q[1]=='\n') q++;
                           }
                           if(!Addtobuffer(&doc->args,removenl?"\n":" ",1)) return FALSE;
                           ta->length++;
                           p=++q;
                        }
                        else q++;
                     }
                     if(q>=end) return Eofandexit(doc,eof);
                     if(!Addtobuffer(&doc->args,p,q-p)) return FALSE;
                     ta->length+=(q-p);
                     p=q;if(*p==quote) p++;
                  }
                  else
                  {  q=p;
                     while(q<end && !isspace(*q) && *q!='>') q++;
                     if(!Addtobuffer(&doc->args,p,q-p)) return FALSE;
                     ta->length+=(q-p);
                     p=q;
                  }
                  if(!Addtobuffer(&doc->args,"",1)) return FALSE;

                  if(removenl) Removenl(&doc->args,ta);

                  Translate(doc,&doc->args,ta,TRUE);

               }
               else
               {  /* Add nullbyte for empty attribute */
                  if(!Addtobuffer(&doc->args,"",1)) return FALSE;
               }
            }
            p++; /* skip over '>' */

            /* Skip newlines after opening tag */
            if(td && td->container && !endtag && !selfclosing && doc->htmlmode!=HTML_COMPATIBLE)
            {  skipnewline=TRUE;
            }
            if(td) tagtype=td->type;
            else tagtype=MARKUP_UNKNOWN;
            if(endtag) tagtype|=MARKUP_END;
         }
      }
      else
      {  thisisdata=FALSE;
         if(!Addtobuffer(&doc->args,
            doc->text.buffer+doc->text.length-1,1)) return FALSE;
         ta=Nextattr(doc);
         ta->attr=TAGATTR_TEXT;
         tagtype=MARKUP_TEXT;
         while(p<end)
         {  if(isspace(*p))
            {  if((doc->pflags&(DPF_PREFORMAT|DPF_JSCRIPT))
               && doc->pmode!=DPM_OPTION && doc->pmode!=DPM_TEXTAREA)
               {  if(*p=='\r' || *p=='\n')
                  {  if(!skipnewline)
                     {  ta=Nextattr(doc);
                        ta->attr=TAGATTR_BR;
                        if(*p=='\r')
                        {  if(++p>=end) return Eofandexit(doc,eof);
                           if(*p=='\n') p++;
                        }
                        else p++;
                        doc->charcount=0;
                        break; /* exit text loop and process */
                     }
                  }
                  else if(*p=='\t')
                  {  /* Add nbsp to fill up to next multiple of 8 */
                     i=8-(doc->charcount%8);
                     if(!Addtobuffer(&doc->args,"\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0",i))
                        return FALSE;
                     ta->length+=i;
                     doc->charcount+=i;
                     skipnewline=FALSE;
                  }
                  else
                  {  if(!Addtobuffer(&doc->args,"\xa0",1)) return FALSE;
                     ta->length++;
                     doc->charcount++;
                     skipnewline=FALSE;
                  }
               }
               else if(doc->pmode==DPM_TEXTAREA)
               {  if(*p=='\r')
                  {  if(p>=end+1) return Eofandexit(doc,eof);
                     if(p[1]=='\n') p++;
                     if(!skipnewline)
                     {  if(!Addtobuffer(&doc->args,"\n",1)) return FALSE;
                        ta->length++;
                     }
                  }
                  else if(*p=='\n')
                  {  if(!skipnewline)
                     {  if(!Addtobuffer(&doc->args,"\n",1)) return FALSE;
                        ta->length++;
                     }
                  }
                  else
                  {  if(!Addtobuffer(&doc->args," ",1)) return FALSE;
                     ta->length++;
                     skipnewline=FALSE;
                  }
               }
               else
               {  if(doc->args.buffer[doc->args.length-1]!=' ')
                  {  if(!skipnewline || (*p!='\n' && *p!='\r'))
                     {  if(!Addtobuffer(&doc->args," ",1)) return FALSE;
                        ta->length++;
                        skipnewline=FALSE;
                     }
                  }
               }
            }
            else
            {  if(!Addtobuffer(&doc->args,p,1)) return FALSE;
               ta->length++;
               if(doc->pflags&DPF_PREFORMAT) doc->charcount++;
               skipnewline=FALSE;
               if(doc->pflags & DPF_JSCRIPT)
               {
                  if(*p == '"' && !(doc->pflags & DPF_QSCRIPT))
                  {
                    /* toggle quoted script, avoid parsing an end of script </SCRIPT> tag in script expression */
                    doc->pflags ^= DPF_DQSCRIPT;
                  }
                  if(*p == '\'' && !(doc->pflags & DPF_DQSCRIPT))
                  {
                    /* toggle single quoted script, avoid parsing an end of script </SCRIPT> tag in script expression */
                    doc->pflags ^= DPF_QSCRIPT;

                  }

               }
            }
            p++;
            /* Temporarialy suspend skipping quoted javascript (code doesn't work in all cases) */

            if(p<end && *p=='<' /*&& !(doc->pflags&(DPF_QSCRIPT | DPF_DQSCRIPT)) */) break;
         }
         if(p>=end && !eof) return Eofandexit(doc,eof);
         if(!Addtobuffer(&doc->args,"",1)) return FALSE;
      }
      /* Store this flag only here in case it was reset but text not yet processed. */
      SETFLAG(doc->pflags,DPF_SKIPNEWLINE,skipnewline);
      oldsrcpos=*srcpos;
      (*srcpos)=p-src->buffer;   /* Before processhtml bcz buffer size calculation uses it */
      if(tagtype==MARKUP_TEXT && !(doc->pflags&DPF_JSCRIPT))
      {  Lookforicons(doc,attrs.first);
      }
      else
      {  Processhtml(doc,tagtype,attrs.first,selfclosing);
         selfclosing = FALSE;
         if(doc->pflags&DPF_SUSPEND)
         {  /* Resume processing later with this same tag */
            (*srcpos)=oldsrcpos;
            /* Don't pass an EOF to the HTML engine */
            return TRUE;
         }
      }
   }
   return Eofandexit(doc,eof);
}

BOOL Parseplain(struct Document *doc,struct Buffer *src,BOOL eof,long *srcpos)
{  UBYTE *p=src->buffer+(*srcpos);
   UBYTE *end=src->buffer+src->length;
   UBYTE ch;
   struct Tagattr *ta;
   short i;
   if((*srcpos)==0)
   {  while(p<end && !*p) p++;
      *srcpos=p-src->buffer;
      NEWLIST(&attrs);
      Nextattr(doc);
      Processhtml(doc,MARKUP_PRE,attrs.first,FALSE);
   }
   while(p<end)
   {  NEWLIST(&attrs);
      nextattr=0;
      doc->args.length=0;
      ta=Nextattr(doc);
      ta->attr=TAGATTR_TEXT;
      while(p<end && *p!='\r' && *p!='\n')
      {  if(*p=='\t')
         {  i=8-(doc->charcount%8);
            if(!Addtobuffer(&doc->args,"\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0",i))
               return FALSE;
            ta->length+=i;
            doc->charcount+=i;
         }
         else
         {  if(isspace(*p)) ch=(UBYTE)'\xa0';
            else ch=*p;
            if(!Addtobuffer(&doc->args,&ch,1)) return FALSE;
            ta->length++;
            doc->charcount++;
         }
         p++;
      }
      if(p>=end && !eof) break;
      ta=Nextattr(doc);
      ta->attr=TAGATTR_BR;
      if(p<end && *p=='\r')
      {  if(p+1<end)
         {  if(p[1]=='\n') p++;
         }
         else if(!eof) break;
      }
      p++;
      doc->charcount=0;
      (*srcpos)=p-src->buffer;
      Processhtml(doc,MARKUP_TEXT,attrs.first,FALSE);
   }
   return TRUE;
}
