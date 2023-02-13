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

/* docprivate.h - AWeb document drivers private */

#include "copydriver.h"
#include "html.h"

/*--- Document source driver ---*/

struct Docsource
{  struct Aobject object;
   void *source;              /* Our source object */
   struct Buffer buf;         /* Source buffer */
   UWORD flags;
   void *editor;
   struct Document *spare;    /* A spare document copy. */
};

#define DOSF_HTML       0x0001   /* text/html type, if clear then text/plain */
#define DOSF_EOF        0x0002   /* EOF was reached on input */
#define DOSF_INREL      0x0004   /* Child in APP_USE_BROWSER relationship */
#define DOSF_JSOPEN     0x0008   /* This is a nonclosed JS generated document */
#define DOSF_SCRIPTJS   0x0010   /* Default script type is javascript */

/*--- Private tags ---*/

#define AODOS_Dummy        AOBJ_DUMMYTAG(AOTP_DOCSOURCE)

#define AODOS_Spare        (AODOS_Dummy+1)   /* SET,GET */
   /* (struct Document *) Spare document. If SET to nonnull, existing spare is
    * disposed. If set to NULL, object is left intact but pointer is cleared. */

/*--- Document extension source driver ---*/

struct Docext
{  struct Aobject object;
   void *source;              /* Our source object */
   void *url;                 /* URL of this docext. */
   struct Buffer buf;         /* Source buffer */
   UWORD flags;
};

#define DOXF_EOF        0x0001   /* EOF was reached on input */
#define DOXF_ERROR      0x0002   /* Extension was in error */

/*--- Document copy driver ---*/

struct Document
{  struct Copydriver cdv;
   void *pool;                /* memory pool */
   void *copy;                /* our copy object */
   struct Docsource *source;  /* Source driver */
   void *frame;               /* Our own true FRAME object */

   struct Buffer text;        /* displayable text */
   struct Buffer args;        /* tag arguments, reused */
   struct Buffer jsrc;        /* Javascript source */
   struct Buffer jout;        /* Javascript output */
   long srcpos;               /* first nonprocessed position in source buffer */
   long joutpos;              /* idem, in javascript output buffer */
   long jsrcline;             /* Starting source line # for javascript */
   short htmlmode;            /* strict, tolerant, compatible */
   ULONG pflags;              /* DPF_ parse flags */
   short pmode;               /* parse mode */
   UBYTE *charset;            /* document's character set */
   long hmargin,vmargin;      /* Default body margins */
   LIST(Tableref) tables;     /* stack of nested tables */
   LIST(Frameref) frames;     /* All frames in this document level, used to speed
                               * up AOFRM_Updatecopy notify's. */
   LIST(Framesetref) framesets;  /* stack of nested framesets */
   short charcount;           /* character count on line since <PRE> for tab support */
   UBYTE frameseqnr;          /* Frame sequence number */
   void *select;              /* Current active select form field */
   void *textarea;            /* Current active textarea form field */
   void *button;              /* Current active <BUTTON> type button */
   short gotbreak;            /* Number of line break elements recently added */
   short wantbreak;           /* Number of line breaks wanted */
   LIST(Bgimage) bgimages;    /* All bg images used */
   void *currentobject;       /* COPY object to set PARAM for. */
   short objectnest;          /* Number of nested <OBJECT> tags whose contents are skipped.
                               * If ==1, <PARAM> elements are read. */

   short doctype;             /* DOCTP_ see below */
   void *body;                /* contents: body, frameset */
   UBYTE *base;               /* base URL */
   UBYTE *target;             /* base target frame */
   long titlepos;             /* text buffer position of title */
   UWORD dflags;              /* DDF_ display flags */
   struct Colorinfo *bgcolor, /* Colors to use or NULL */
      *textcolor,*linkcolor,*vlinkcolor,*alinkcolor;
   void *bgimage;             /* Background image to use or NULL */
   struct Aobject *bgalign;   /* Object to align background to align to */
   void *bgsound;             /* Background sound object */
   UBYTE *clientpull;         /* Clientpull string */

   LIST(Colorinfo) colors;    /* colors used in this document */
   LIST(Aobject) links;       /* links used in this document */
   LIST(Aobject) maps;        /* map descriptors used in this document */
   LIST(Aobject) forms;       /* forms used in this document */
   LIST(Fragment) fragments;  /* fragment names used in this document */
   LIST(Infotext) infotexts;  /* meta and link texts */
   void *win;                 /* window in which document is displayed, or NULL */
   struct Jobject *jobject;   /* JS object */
   struct Jobject *jforms;    /* JS forms array */
   struct Jobject *jlinks;    /* JS links array */
   struct Jobject *jimages;   /* JS images array */
   struct Jobject *janchors;  /* JS anchors array */
   struct Jobject *japplets;  /* JS applets array */
   struct Jobject *jembeds;   /* JS embeds array */
   UBYTE *onload;             /* JS onLoad event handler */
   UBYTE *onunload;           /* JS onUnload event handler */
   UBYTE *onfocus;
   UBYTE *onblur;
   UBYTE *jdomain;            /* JS domain property */
   BOOL  centeractive;        /* Center Tag is active */
};

#define DPF_PREFORMAT      0x00000001  /* doing PRE */
#define DPF_RELOADVERIFY   0x00000002  /* make all embedded objects forced verify */
#define DPF_BLINK          0x00000004  /* doing BLINK */
#define DPF_OBJECTSHAPES   0x00000008  /* doing OBJECT with SHAPES */
#define DPF_XMP            0x00000010  /* doing XMP */
#define DPF_LISTING        0x00000020  /* doing LISTING */
#define DPF_SKIPNEWLINE    0x00000040  /* next newline in source should be skipped */
#define DPF_BULLET         0x00000080  /* last element was list bullet */
#define DPF_JSCRIPT        0x00000100  /* doing <SCRIPT> for JavaScript */
#define DPF_JPARSE         0x00000200  /* parsing JavaScript output, ignore EOF */
#define DPF_JRUN           0x00000400  /* running JavaScript while parsing */
#define DPF_FORM           0x00000800  /* form is active (forms.last) */
#define DPF_FRAMESETEND    0x00001000  /* top level frameset is closed */
#define DPF_SCRIPTJS       0x00002000  /* default script type is Javascript */
#define DPF_BADSCRIPT      0x00004000  /* a script was in a language not understood by us */
#define DPF_SUSPEND        0x00008000  /* waiting for extension, suspend parsing */
#define DPF_NORLDOCEXT     0x00010000  /* don't reload the next extension */
#define DPF_QSCRIPT        0x00020000  /* ignoring a quoted section of script*/
#define DPF_DQSCRIPT       0x00040000  /* ignoring a quoted section of script*/

#define DPM_BODY           0        /* parsing normal body contents */
#define DPM_TITLE          1        /* parsing <TITLE> */
#define DPM_STYLE          2        /* skipping <STYLE> */
#define DPM_SCRIPT         3        /* skipping <SCRIPT> */
#define DPM_MAP            4        /* doing <MAP> */
#define DPM_OPTION         5        /* doing <OPTION> */
#define DPM_TEXTAREA       6        /* doing <TEXTAREA> */
#define DPM_FRAMESET       7        /* doing <FRAMESET>s */
#define DPM_NOFRAMES       8        /* skipping <NOFRAMES> */
#define DPM_OBJECT         9        /* skipping one or more <OBJECT> contents */
#define DPM_NOSCRIPT       10       /* skipping <NOSCRIPT> */
#define DPM_IFRAME         11       /* skipping <IFRAME> */

/* If DPF_JSCRIPT is set when in DPM_SCRIPT mode, the <SCRIPT> element is
 * skipped but the </SCRIPT> tag should run the script anyway (uses a
 * script soirce from document extension). */

#define DDF_TITLEVALID     0x0001   /* titlepos is valid */
#define DDF_DISPTITLE      0x0002   /* change title when appropriate */
#define DDF_DONE           0x0004   /* eof reached, don't parse any more */
#define DDF_NEWBGSOUND     0x0008   /* we have a new background sound */
#define DDF_ISSPARE        0x0010   /* this is a spare copy, when disposed do a real
                                     * dispose */
#define DDF_NOBACKGROUND   0x0020   /* Don't use colours */
#define DDF_PLAYBGSOUND    0x0040   /* Background sound should play */
#define DDF_MAPDOCUMENT    0x0080   /* Document for MAP definitions only */
#define DDF_NOSPARE        0x0100   /* Document must not be kept in spare (uses JS) */
#define DDF_HMARGINSET     0x0200   /* hmargin was set by HTML */
#define DDF_VMARGINSET     0x0400   /* vmargin was set by HTML */
#define DDF_FOREIGN        0x0800   /* Data uses foreign character set */

#define DOCTP_NONE         0        /* no contents yet */
#define DOCTP_BODY         1        /* contents is body, add elements */
#define DOCTP_FRAMESET     2        /* contents is frameset, ignore elements */

/* Tablerefs form a stack of nested tables */
struct Tableref
{  NODE(Tableref);
   void *table;
};

/* Framerefs form a list of all frames in this document level */
struct Frameref
{  NODE(Frameref);
   void *frame;
};

/* framesetrefs form a stack of netsed framesets */
struct Framesetref
{  NODE(Framesetref);
   void *frameset;
};

/* Fragment pointer */
struct Fragment
{  NODE(Fragment);
   UBYTE *name;               /* This fragment's name */
   struct Element *elt;       /* The related NAME element */
};

/* a user of a given background image */
struct Bguser
{
   NODE(Bguser);
   void *user;
};

/* All bgimages used in document or tables */
struct Bgimage
{  NODE(Bgimage);
   void *url;                 /* Url for this image */
   void *copy;                /* Image copy object */
   LIST(Bguser) bgusers;          /* List of objects using this bgimage */
};

/* META or LINK text */
struct Infotext
{  NODE(Infotext);
   UBYTE *text;
   void *link;                /* URL object linked to */
};

/*--- Private tags: ---*/

#define AODOC_Dummy        AOBJ_DUMMYTAG(AOTP_DOCUMENT)

#define AODOC_Srcupdate    (AODOC_Dummy+101)
   /* (BOOL) Source has been updated */

#define AODOC_Reload       (AODOC_Dummy+102)
   /* (BOOL) A reload has started. Dispose copy contents. */

#define AODOC_Docextready  (AODOC_Dummy+103)
   /* (void *) A document extension was ready for this URL. The waiting document
    * has already been removed from the docext list. */

/*--- prototypes ---*/

/* from parse.c: */

extern BOOL Parsehtml(struct Document *doc,struct Buffer *src,BOOL eof,long *srcpos);
extern BOOL Parseplain(struct Document *doc,struct Buffer *src,BOOL eof,long *srcpos);

/* from html.c: */

extern BOOL Setbodycolor(struct Document *doc,struct Colorinfo **cip,UBYTE *p);
extern BOOL Processhtml(struct Document *doc,UWORD tagtype,struct Tagattr *ta, BOOL selfclosing);

/* from document.c: */

extern struct Colorinfo *Finddoccolor(struct Document *doc,ULONG rgb);
extern void Registerdoccolors(struct Document *doc);

/* from docjs.c: */

extern long Jsetupdocument(struct Document *doc,struct Amjsetup *amj);
extern void Docjexecute(struct Document *doc,UBYTE *source);
extern void Freejdoc(struct Document *doc);

extern void Initdocjs(void);
extern void Exitdocjs(void);

/* from docext.c: */

extern UBYTE *Finddocext(struct Document *doc,void *url,BOOL reload);
extern void Remwaitingdoc(struct Document *doc);

/* from docsource.c: */

extern long Docslinenrfrompos(struct Docsource *dos,long pos);
