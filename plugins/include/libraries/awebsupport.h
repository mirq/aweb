#ifndef LIBRARIES_AWEBSUPPORT_H
#define LIBRARIES_AWEBSUPPORT_H

/* libraries/awebsupport.h
 *
 */

#include <exec/types.h>
#include <exec/ports.h>

/*--- common data structures ---*/

struct Arect               /* a rectangle */
{  LONG minx,miny;
   LONG maxx,maxy;
};

struct Coords              /* Rendering context */
{  LONG dx,dy;             /* Add to frame coordinates to get RastPort coordinates */
   LONG minx,miny,maxx,maxy;  /* Resulting max visible limits as RastPort coords */
   APTR win;
   struct RastPort *rp;    /* The window's RastPort */
   struct DrawInfo *dri;   /* The current DrawInfo */
   SHORT bgcolor,textcolor,linkcolor,vlinkcolor,alinkcolor;
                           /* Default pen numbers for current frame */
   APTR bgimage;           /* Background image for current frame */
   APTR bgalign;           /* Object to align background image to */
};

struct Plugininfo
{  ULONG sourcedriver;
   ULONG copydriver;
};

struct Pluginquery
{  long structsize;        /* Set to struct size by caller */
   BOOL command;           /* Set to TRUE if plugin supports Commandplugin() */
   BOOL filter;            /* Set to TRUE if plugin supports Filterplugin() */
};

struct Plugincommand
{  long structsize;        /* Set to struct size by caller */
   STRPTR command;         /* Set to command by caller, modifications
                            * by plugin allowed. */
   STRPTR result;          /* Result allocated by plugin, freed by caller.
                            * Leave NULL if no result returned. */
   long rc;                /* Return code. 0 for success, nonzero for
                            * warning or failure. */
};

struct Pluginfilter        /* All fields except userdata are read-only! */
{  long structsize;        /* Set to struct size by caller */
   void *handle;           /* Use in Setfiltertype() and Writefilter() */
   STRPTR data;            /* Block of data or NULL */
   long length;            /* Length of data */
   BOOL eof;               /* TRUE if EOF was reached */
   STRPTR contenttype;     /* MIME content-type of data */
   STRPTR url;             /* URL of this file */
   void *userdata;         /* For plugin private use. Delete when eof is set! */
   STRPTR encoding;        /* Encoding (character set) of text based data */
};

/*---- Root object ----*/

struct Aobject
{  APTR next;
   APTR prev;
   SHORT objecttype;
};

/*---- Object types ----*/

#define AOTP_OBJECT        0x00
#define AOTP_APPLICATION   0x01
#define AOTP_URL           0x02
#define AOTP_SOURCE        0x05
#define AOTP_COPY          0x06
#define AOTP_TASK          0x07
#define AOTP_FILE          0x11
#define AOTP_TIMER         0x14
#define AOTP_SOURCEDRIVER  0x71
#define AOTP_COPYDRIVER    0x72
#define AOTP_PLUGIN        0x81  /* Use this as offset for private tags */

/*---- General object tags ----*/

#define AOBJ_DUMMYTAG(type) (TAG_USER+0x23EB0000+((type)<<8))

#define AOBJ_Dummy         AOBJ_DUMMYTAG(0)

#define AOBJ_Left          (AOBJ_Dummy+1)
#define AOBJ_Top           (AOBJ_Dummy+2)
#define AOBJ_Width         (AOBJ_Dummy+3)
#define AOBJ_Height        (AOBJ_Dummy+4)
#define AOBJ_Cframe        (AOBJ_Dummy+5)
#define AOBJ_Window        (AOBJ_Dummy+6)
#define AOBJ_Target        (AOBJ_Dummy+8)
#define AOBJ_Map           (AOBJ_Dummy+9)
#define AOBJ_Layoutparent  (AOBJ_Dummy+10)
#define AOBJ_Changedchild  (AOBJ_Dummy+11)
#define AOBJ_Popupinfo     (AOBJ_Dummy+13)
#define AOBJ_Popupselect   (AOBJ_Dummy+14)
#define AOBJ_Application   (AOBJ_Dummy+15)

/*---- Methods ----*/

#define AOM_DUMMYTAG(type) ((type)*100)
#define AOM_DUMMY          AOM_DUMMYTAG(0)
#define AOM_NEW            (AOM_DUMMY+1)
#define AOM_SET            (AOM_DUMMY+2)
#define AOM_GET            (AOM_DUMMY+3)
#define AOM_DISPOSE        (AOM_DUMMY+4)
#define AOM_RENDER         (AOM_DUMMY+5)
#define AOM_HITTEST        (AOM_DUMMY+6)
#define AOM_GOACTIVE       (AOM_DUMMY+7)
#define AOM_HANDLEINPUT    (AOM_DUMMY+8)
#define AOM_GOINACTIVE     (AOM_DUMMY+9)
#define AOM_UPDATE         (AOM_DUMMY+10)
#define AOM_MEASURE        (AOM_DUMMY+11)
#define AOM_LAYOUT         (AOM_DUMMY+12)
#define AOM_SRCUPDATE      (AOM_DUMMY+14)
#define AOM_ADDCHILD       (AOM_DUMMY+15)
#define AOM_REMCHILD       (AOM_DUMMY+16)
#define AOM_NOTIFY         (AOM_DUMMY+17)
#define AOM_INSTALL        (AOM_DUMMY+98)

/*---- Relationships ----*/

#define AOREL_DUMMYTAG(type)  ((type)<<16)

/*---- Activation codes ----*/

#define AMR_ACTIVE         1
#define AMR_REUSE          2
#define AMR_NOREUSE        3
#define AMR_CHANGED        4
#define AMR_NOCARE         5
#define AMR_DEFER          6

/*---- Messages ----*/

struct Amessage
{  ULONG method;
};

struct Amset
{  struct Amessage amsg;
   struct TagItem *tags;   /* Attributes to set or get */
};

struct Ammeasure
{  struct Amessage amsg;
   LONG width,height;      /* Target width and height */
   LONG addwidth;
   USHORT flags;
   APTR text;
   struct Ammresult *ammr;
};

struct Ammresult
{  LONG width;             /* Nominal (maximum) width of object */
   LONG minwidth;          /* Minimum needed for this object */
   LONG minheight;
   LONG addwidth;
   BOOL newline;
};

struct Amlayout
{  struct Amessage amsg;
   LONG width,height;      /* Target width and height to layout within */
   USHORT flags;
   APTR text;
   LONG startx;            /* Start on this position within width */
   struct Amlresult *amlr;
};

struct Amrender
{  struct Amessage amsg;
   struct Coords *coords;  /* Rastport, coordinate offsets to use. If present,
                            * rastport is already clipped. If not present, then
                            * object should call Clipcoords() to obtain one. */
   struct Arect rect;      /* Frame relative coordinates of portion to render. */
   USHORT flags;
   APTR text;
};

#define AMRF_CLEAR            0x0001   /* Clear background before rendering */
#define AMRF_UPDATESELECTED   0x0004   /* Update yourself to selected state */
#define AMRF_UPDATENORMAL     0x0008   /* Update yourself to normal state */

#define AMRMAX          0x3fffffff     /* Use as reasonable maximum render limit */

struct Amhittest
{  struct Amessage amsg;
   struct Coords *coords;  /* Coordinate offsets */
   LONG xco,yco;           /* Rastport coordinates to check. Offset these with
                            * the (coords) values */
   USHORT flags;           /* Special flags, see below */
   struct Aobject *oldobject;
                           /* Previously hit object, checked again after mouse move. */
   struct Amhresult *amhr;
};

/* hittest flags */
#define AMHF_DOWNLOAD   0x0001   /* Download qualifier is active */
#define AMHF_POPUP      0x0002   /* Popup menu qualifier is active */
#define AMHF_DEFER      0x0004   /* Deferred action for AOM_HANDLEINPUT */

struct Amhresult
{  struct Aobject *object; /* The object hit, should be ready to receive AOM_GOACTIVE. */
   STRPTR text;            /* Status gadget text. */
};

/* hittest return values: */
#define AMHR_NOHIT      0
#define AMHR_NEWHIT     1  /* Coordinates fall within new object */
#define AMHR_OLDHIT     2  /* Coordinates still fall within old object */


struct Amgoactive
{  struct Amessage amsg;
   struct IntuiMessage *imsg;
                           /* The IntuiMessage that caused activation. The
                            * MouseX/Y coordinates in this message are RastPort
                            * coordinates and have to be corrected with Coords
                            * offsets (you have to request a Coords yourself). */
   USHORT flags;           /* AMHF_DOWNLOAD or AMHF_POPUP qualifier */
};

struct Aminput
{  struct Amessage amsg;
   struct IntuiMessage *imsg;
                           /* The IntuiMessage to be handled. */
   USHORT flags;           /* AMHF_DOWNLOAD or AMHF_POPUP qualifier */
   struct Amiresult *amir;
};

struct Amiresult
{  STRPTR text;            /* New status gadget text. */
   struct Aobject *newobject;
                           /* If not NULL, this object wants to become the active one. */
};

struct Amsrcupdate
{  struct Amessage amsg;
   APTR fetch;
   struct TagItem *tags;   /* See url.h */
};

struct Amadd
{  struct Amessage amsg;
   struct Aobject *child;  /* child object to add or remove */
   ULONG relation;         /* parent child relation */
};

struct Amnotify
{  struct Amessage amsg;
   struct Amessage *nmsg;  /* The message to process and forward */
};

struct Aminstall
{  struct Amessage amsg;
   LONG objecttype;
   LONG (*dispatcher)(struct Aobject *,struct Amessage *);
};

/*--- attributes used in AOM_SRCUPDATE messages ---*/

#define AOURL_Dummy        AOBJ_DUMMYTAG(AOTP_URL)
#define AOURL_Contenttype  (AOURL_Dummy+101)
#define AOURL_Contentlength (AOURL_Dummy+102)
#define AOURL_Data         (AOURL_Dummy+107)
#define AOURL_Datalength   (AOURL_Dummy+108)
#define AOURL_Eof          (AOURL_Dummy+110)
#define AOURL_Error        (AOURL_Dummy+115)

/*--- AOTP_APPLICATION attributes ---*/

#define AOAPP_Dummy        AOBJ_DUMMYTAG(AOTP_APPLICATION)
#define AOAPP_Programname  (AOAPP_Dummy+1)
#define AOAPP_Screen       (AOAPP_Dummy+2)
#define AOAPP_Screenname   (AOAPP_Dummy+3)
#define AOAPP_Colormap     (AOAPP_Dummy+4)
#define AOAPP_Drawinfo     (AOAPP_Dummy+5)
#define AOAPP_Screenfont   (AOAPP_Dummy+6)
#define AOAPP_Systemfont   (AOAPP_Dummy+7)
#define AOAPP_Screenwidth  (AOAPP_Dummy+8)
#define AOAPP_Screenheight (AOAPP_Dummy+9)
#define AOAPP_Screendepth  (AOAPP_Dummy+10)
#define AOAPP_Screenvalid  (AOAPP_Dummy+12)
#define AOAPP_Visualinfo   (AOAPP_Dummy+14)
#define AOAPP_Browsebgpen  (AOAPP_Dummy+15)
#define AOAPP_Textpen      (AOAPP_Dummy+16)
#define AOAPP_Linkpen      (AOAPP_Dummy+17)
#define AOAPP_Vlinkpen     (AOAPP_Dummy+18)
#define AOAPP_Alinkpen     (AOAPP_Dummy+19)
#define AOAPP_Whitepen     (AOAPP_Dummy+29)
#define AOAPP_Blackpen     (AOAPP_Dummy+30)
#define AOAPP_Semaphore    (AOAPP_Dummy+31)

/* Relations from AOTP_APPLICATION */

#define AOREL_APP_DUMMY          AOREL_DUMMYTAG(AOTP_APPLICATION)
#define AOREL_APP_USE_SCREEN     (AOREL_APP_DUMMY+1)

/*--- AOTP_FILE attributes ---*/

#define AOFIL_Dummy        AOBJ_DUMMYTAG(AOTP_FILE)
#define AOFIL_Name         (AOFIL_Dummy+1)
#define AOFIL_Extension    (AOFIL_Dummy+2)
#define AOFIL_Data         (AOFIL_Dummy+3)
#define AOFIL_Datalength   (AOFIL_Dummy+4)
#define AOFIL_Eof          (AOFIL_Dummy+5)
#define AOFIL_Delete       (AOFIL_Dummy+6)
#define AOFIL_Icontype     (AOFIL_Dummy+7)
#define AOFIL_Comment      (AOFIL_Dummy+8)
#define AOFIL_Append       (AOFIL_Dummy+9)
#define AOFIL_Copyfile     (AOFIL_Dummy+10)
#define AOFIL_Stringdata   (AOFIL_Dummy+11)
#define AOFIL_Filesize     (AOFIL_Dummy+12)
#define AOFIL_Datestamp    (AOFIL_Dummy+13)
#define AOFIL_Pipe         (AOFIL_Dummy+14)

/* file icon types */

#define FILEICON_NONE      0  /* no icon */
#define FILEICON_TEXT      1  /* project icon with AWeb as default tool */
#define FILEICON_DATA      2  /* project icon with no default tool */

/*--- AOTP_TIMER attributes ---*/

#define AOTIM_Dummy        AOBJ_DUMMYTAG(AOTP_TIMER)
#define AOTIM_Waitseconds  (AOTIM_Dummy+1)
#define AOTIM_Waitmicros   (AOTIM_Dummy+2)
#define AOTIM_Seconds      (AOTIM_Dummy+3)
#define AOTIM_Micros       (AOTIM_Dummy+4)
#define AOTIM_Ready        (AOTIM_Dummy+5)

/*--- AOTP_TASK attributes ---*/

#define AOTSK_Dummy        AOBJ_DUMMYTAG(AOTP_TASK)
#define AOTSK_Entry        (AOTSK_Dummy+1)
#define AOTSK_Userdata     (AOTSK_Dummy+2)
#define AOTSK_Stacksize    (AOTSK_Dummy+3)
#define AOTSK_Name         (AOTSK_Dummy+4)
#define AOTSK_Start        (AOTSK_Dummy+5)
#define AOTSK_Stop         (AOTSK_Dummy+6)
#define AOTSK_Suspend      (AOTSK_Dummy+7)
#define AOTSK_Async        (AOTSK_Dummy+8)
#define AOTSK_Replied      (AOTSK_Dummy+9)
#define AOTSK_Message      (AOTSK_Dummy+10)

/* structures */

struct Taskmsg
{  struct Message execmsg;       /* The EXEC message header */
   struct Amessage *amsg;        /* The AWeb OO-message */
   LONG result;                  /* Return value of the method */
};

/*--- AOTP_SOURCE attributes ---*/

#define AOSRC_Dummy        AOBJ_DUMMYTAG(AOTP_SOURCE)
#define AOSRC_Memory       (AOSRC_Dummy+10)

/*--- AOTP_SOURCE relationships ---*/

#define AOREL_SRC_DUMMY    AOREL_DUMMYTAG(AOTP_SOURCE)
#define AOREL_SRC_COPY     (AOREL_SRC_DUMMY+1)

/*--- AOTP_SOURCEDRIVER datastructure ---*/

struct Sourcedriver
{  struct Aobject object;     /* Object header */
   APTR extension;            /* Private extension data */
};

/*--- AOTP_SOURCEDRIVER attributes ---*/

#define AOSDV_Dummy        AOBJ_DUMMYTAG(AOTP_SOURCEDRIVER)
#define AOSDV_Source       (AOSDV_Dummy+1)
#define AOSDV_Arguments    (AOSDV_Dummy+3)
#define AOSDV_Saveable     (AOSDV_Dummy+4)
#define AOSDV_Viewable     (AOSDV_Dummy+5)
#define AOSDV_Savesource   (AOSDV_Dummy+6)
#define AOSDV_Displayed    (AOSDV_Dummy+7)
#define AOSDV_Volatile     (AOSDV_Dummy+8)
#define AOSDV_Getsource    (AOSDV_Dummy+9)
#define AOSDV_Getable      (AOSDV_Dummy+10)
#define AOSDV_Editsource   (AOSDV_Dummy+11)

/*--- AOTP_COPYDRIVER datastructures ---*/

struct Copydriver
{  struct Aobject object;     /* Object header */
   APTR extension;            /* Private extension data */
   LONG aox,aoy;              /* Left,top position */
   LONG aow,aoh;              /* Width, height */
   struct Aobject *cframe;    /* Current context frame */
};

struct Objectparam            /* General purpose object parameter */
{  struct MinNode node;
   STRPTR name;               /* Parameter name */
   STRPTR value;              /* Parameter value */
   STRPTR valuetype;          /* Type of value: "DATA", "REF", "URL" or "OBJECT" */
   STRPTR type;               /* MIME type of parameter, only for REF or URL. */
};

/*--- AOTP_COPYDRIVER attributes ---*/

#define AOCDV_Dummy        AOBJ_DUMMYTAG(AOTP_COPYDRIVER)
#define AOCDV_Copy         (AOCDV_Dummy+1)
#define AOCDV_Title        (AOCDV_Dummy+2)
#define AOCDV_Sourcedriver (AOCDV_Dummy+5)
#define AOCDV_Ready        (AOCDV_Dummy+6)
#define AOCDV_Shapes       (AOCDV_Dummy+7)
#define AOCDV_Width        (AOCDV_Dummy+8)
#define AOCDV_Height       (AOCDV_Dummy+9)
#define AOCDV_Imagebitmap  (AOCDV_Dummy+10)
#define AOCDV_Imagemask    (AOCDV_Dummy+11)
#define AOCDV_Imagewidth   (AOCDV_Dummy+12)
#define AOCDV_Imageheight  (AOCDV_Dummy+13)
#define AOCDV_Hlayout      (AOCDV_Dummy+14)
#define AOCDV_Vlayout      (AOCDV_Dummy+15)
#define AOCDV_Undisplayed  (AOCDV_Dummy+17)
#define AOCDV_Soundloop    (AOCDV_Dummy+18)
#define AOCDV_Playsound    (AOCDV_Dummy+19)
#define AOCDV_Displayed    (AOCDV_Dummy+20)
#define AOCDV_Bgchanged    (AOCDV_Dummy+21)
#define AOCDV_Objectparams (AOCDV_Dummy+24)
#define AOCDV_Alpha        (AOCDV_Dummy+27)

/*--- Useful macros ---*/

/* Store an attribute value in the memory location that tag->ti_Data
 * points to */
#define PUTATTR(tag,attr) *((ULONG *)(tag)->ti_Data)=(ULONG)(attr)

/*=== AWeb 3.1 additions ===*/

/*--- AOTP_COPY attributes ---*/

#define AOCPY_Dummy        AOBJ_DUMMYTAG(AOTP_COPY)
#define AOCPY_Onimgload    (AOCPY_Dummy+36)
#define AOCPY_Onimganim    (AOCPY_Dummy+37)


/*=== AWeb 3.5 additions ===*/

/*--- AOTP_COPY attributes ---*/

#define AOCPY_Portnumber   (AOCPY_Dummy+41)

/* function typedefs */

typedef void Subtaskfunction(APTR userdata);
typedef void Repliedfunction(struct Aobject *task,struct Taskmsg *msg);


#endif
