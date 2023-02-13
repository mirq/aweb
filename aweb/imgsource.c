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

/* imgsource.c - AWeb image source interpreter object */

#include "aweb.h"
#include "source.h"
#include "sourcedriver.h"
#include "copy.h"
#include "url.h"
#include "file.h"
#include "imgprivate.h"
#include "application.h"
#include "task.h"
#include "cache.h"
#include "fetch.h"
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
//#include <datatypes/pictureclassExt.h> /* 24-bit picture class v43 */
#include <dos/dos.h>

#include <proto/datatypes.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>

#ifdef DEVELOPER
extern BOOL usetemp;
#else
#define usetemp FALSE
#endif

/*------------------------------------------------------------------------*/

struct Imagetask
{  struct Screen *screen;              /* Screen to use for remapping */
   struct SignalSemaphore screensema;  /* Protects the screen pointer */
   struct SignalSemaphore *procsema;   /* Only one task can process */
};

static struct Imagetask imagetask;

#define AOIMS_Dummy        AOBJ_DUMMYTAG(AOTP_IMGSOURCE)

#define AOIMS_Dtobject     (AOIMS_Dummy+1)
#define AOIMS_Bitmap       (AOIMS_Dummy+2)
#define AOIMS_Mask         (AOIMS_Dummy+3)
#define AOIMS_Width        (AOIMS_Dummy+4)
#define AOIMS_Height       (AOIMS_Dummy+5)
#define AOIMS_Memsize      (AOIMS_Dummy+6)
#define AOIMS_Ourmask      (AOIMS_Dummy+7)
#define AOIMS_Depth        (AOIMS_Dummy+8)

/*------------------------------------------------------------------------*/

struct Imgprocess
{  struct Imgsource *ims;
   struct Screen *screen;
   void *dto;
   struct BitMap *bitmap;
   UBYTE *mask;
   long width,height,depth;
   BOOL ourmask;
   long memsize;
};

/* Determine if this is a transparent gif */
static long Transparentgif(struct Imgprocess *imp)
{  long fh;
   UBYTE buf[16];
   long xptcolor=-1;
   long colortabsize,pos;
   if(fh=Open(imp->ims->filename,MODE_OLDFILE))
   {  if(Read(fh,buf,16)==16 && STRNEQUAL(buf,"GIF89",5))
      {  if(buf[0x0a]&0x80) colortabsize=3*(1<<((buf[0x0a]&0x07)+1));
         else colortabsize=0;
         Seek(fh,pos=0x0d+colortabsize,OFFSET_BEGINNING);
         for(;;)
         {  if(Read(fh,buf,7)==7)
            {  if(buf[0]==0x21 && buf[1]==0xf9 && (buf[3]&0x01))
               {  xptcolor=buf[6];
                  break;
               }
               if(buf[0]==0x21)
               {  pos+=2; // 3+buf[2];
                  do
                  {  Seek(fh,pos,OFFSET_BEGINNING);
                     if(Read(fh,buf,1)!=1) buf[0]=0;
                     pos+=1+buf[0];
                  } while(buf[0]);
               }
               else break;
            }
         }
      }
      Close(fh);
   }
   return xptcolor;
}

/* Make a transparent mask if the dt didn't create it */
static void Makegifmask(struct Imgprocess *imp,long xptcolor)
{  struct BitMap *bmap;
   short bmwidth; /* width of remapped bitmap = width of mask */
   UBYTE *p,*q;
   short d,h,w,c;
   void *object;
   struct gpLayout gpl={0};
   ULONG flags;
   if(object=NewDTObject(imp->ims->filename,
         DTA_SourceType,DTST_FILE,
         DTA_GroupID,GID_PICTURE,
         PDTA_Remap,FALSE,
         PDTA_DestMode,PMODE_V42,
         PDTA_UseFriendBitMap,TRUE,
         OBP_Precision,PRECISION_IMAGE,
         TAG_END))
   {  gpl.MethodID=DTM_PROCLAYOUT;
      gpl.gpl_GInfo=NULL;
      gpl.gpl_Initial=TRUE;
      if(DoMethodA(object,(Msg)&gpl)
      && GetDTAttrs(object,
         PDTA_DestBitMap,&bmap,
         TAG_END)
      && bmap)
      {  flags=GetBitMapAttr(imp->bitmap,BMA_FLAGS);
         bmwidth=imp->bitmap->BytesPerRow;
         if(flags&BMF_INTERLEAVED) bmwidth/=imp->bitmap->Depth;
         flags=GetBitMapAttr(bmap,BMA_FLAGS);
         if(flags&BMF_STANDARD)
         {  if(imp->mask=ALLOCTYPE(UBYTE,imp->bitmap->BytesPerRow*bmap->Rows,
               MEMF_CHIP|MEMF_CLEAR))
            {  imp->memsize+=imp->bitmap->BytesPerRow*bmap->Rows;
               for(d=0;d<bmap->Depth;d++)
               {  if(xptcolor&(1<<d)) c=1;
                  else c=0;
                  for(h=0;h<bmap->Rows;h++)
                  {  p=bmap->Planes[d]+h*bmap->BytesPerRow;
                     q=imp->mask+h*imp->bitmap->BytesPerRow;
                     for(w=0;w<bmwidth;w++)
                     {  if(c) *q++|=~*p++;
                        else *q++|=*p++;
                     }
                  }
               }
               imp->ourmask=TRUE;
            }
         }
      }
      DisposeDTObject(object);
   }
}

/* Create a datatype object and the mask */
static BOOL Makeobject(struct Imgprocess *imp)
{  BOOL result=FALSE;
   struct gpLayout gpl={0};
   ULONG flags;
   void *maskplane=NULL;
   long xptcolor=Transparentgif(imp);
   if(imp->dto=NewDTObject(imp->ims->filename,
         DTA_SourceType,DTST_FILE,
         DTA_GroupID,GID_PICTURE,
         PDTA_Remap,TRUE,
         PDTA_Screen,imp->screen,
         PDTA_FreeSourceBitMap,TRUE,
         PDTA_DestMode,PMODE_V43,    /* was 43 */
         PDTA_UseFriendBitMap,TRUE,
         OBP_Precision,PRECISION_IMAGE,
         TAG_END))
   {  gpl.MethodID=DTM_PROCLAYOUT;
      gpl.gpl_GInfo=NULL;
      gpl.gpl_Initial=TRUE;
      if(DoMethodA(imp->dto,(Msg)&gpl)
      && GetDTAttrs(imp->dto,
         DTA_NominalHoriz,&imp->width,
         DTA_NominalVert,&imp->height,
         PDTA_DestBitMap,&imp->bitmap,
         PDTA_MaskPlane,&maskplane,
         TAG_END)
      && imp->bitmap)
      {  imp->depth=GetBitMapAttr(imp->bitmap,BMA_DEPTH);
         imp->memsize=imp->width*imp->height*imp->depth/8;
         flags=GetBitMapAttr(imp->bitmap,BMA_FLAGS);
         if(xptcolor>=0) /* we think our gif is transparent */
         { if(maskplane)
            {
               if(flags&BMF_STANDARD)
               {
                   /*make our own mask because the datatype one is suspect*/
                   Makegifmask(imp,xptcolor);
               }
               else
               {
                   /* oh well bitmaps in wrong format, just have totrust the datatype (rtg?) */
                   imp->mask=maskplane;
                   imp->memsize*=2;
               }

            }
         }
         result=TRUE;
      }
   }
   return result;
}

/* Image processing subtask */
static void Imagetask(struct Imgsource *ims)
{
   struct Imgprocess imp={0};
   imp.ims=ims;
   if(!imagetask.procsema || Obtaintasksemaphore(imagetask.procsema))
   {  ObtainSemaphore(&imagetask.screensema);
      imp.screen=imagetask.screen;
      ReleaseSemaphore(&imagetask.screensema);
      if(imp.screen)
      {  Makeobject(&imp);
         Updatetaskattrs(
            AOIMS_Dtobject,imp.dto,
            AOIMS_Bitmap,imp.bitmap,
            AOIMS_Mask,imp.mask,
            AOIMS_Width,imp.width,
            AOIMS_Height,imp.height,
            AOIMS_Depth,imp.depth,
            AOIMS_Memsize,imp.memsize,
            AOIMS_Ourmask,imp.ourmask,
            TAG_END);
      }
      if(imagetask.procsema) ReleaseSemaphore(imagetask.procsema);
   }
}

/*------------------------------------------------------------------------*/

/* Create a new file object if data isn't stored in cache */
static void *Newfile(struct Imgsource *ims,struct Amsrcupdate *ams)
{  void *url,*cache;
   UBYTE *urlname,*ext=NULL,*ctype;
   void *file=NULL;
   url=(void *)Agetattr(ims->source,AOSRC_Url);
   cache=(void *)Agetattr(url,AOURL_Cache);
   urlname=(UBYTE *)Agetattr(url,AOURL_Url);
   if(cache && !usetemp)
   {  ims->filename=(UBYTE *)Agetattr(cache,AOCAC_Name);
      ims->flags|=IMSF_CACHEFILE;
      if(ams) Asetattrs(ams->fetch,AOFCH_Cancellocal,TRUE,TAG_END);
   }
   else if(urlname && (ims->filename=Urllocalfilename(urlname)) && !usetemp)
   {  ims->flags|=IMSF_CACHEFILE;
      if(ams) Asetattrs(ams->fetch,AOFCH_Cancellocal,TRUE,TAG_END);
   }
   else
   {  if(urlname)
      {  ext=Urlfileext(urlname);
      }
      if(!ext)
      {  if(ctype=(UBYTE *)Agetattr(url,AOURL_Contenttype))
         {  if(Isxbm(ctype)) ext=Dupstr("xbm",3);
         }
      }
      file=Anewobject(AOTP_FILE,
         AOFIL_Extension,(Tag)ext,
         TAG_END);
      if(ext) FREE(ext);
      ims->flags&=~IMSF_CACHEFILE;
   }
   return file;
}

/* Dispose the datatype object and related stuff */
static void Disposedto(struct Imgsource *ims)
{  if(ims->dto)
   {  DisposeDTObject(ims->dto);
      ims->dto=NULL;
   }
   if(ims->mask && (ims->flags&IMSF_OURMASK))
   {  FREE(ims->mask);
   }
   ims->bitmap=NULL;
   ims->mask=NULL;
   ims->width=0;
   ims->height=0;
   ims->depth=0;
   ims->flags&=~IMSF_OURMASK;
   Asetattrs(ims->source,AOSRC_Memory,0,TAG_END);
}

/* Start processing of image */
static void Startprocessimg(struct Imgsource *ims)
{  ObtainSemaphore(&imagetask.screensema);
   if(!imagetask.screen)
   {  imagetask.screen=(struct Screen *)Agetattr(Aweb(),AOAPP_Screen);
   }
   ReleaseSemaphore(&imagetask.screensema);
   if(!imagetask.procsema)
   {  imagetask.procsema=(struct SignalSemaphore *)Agetattr(Aweb(),AOAPP_Semaphore);
   }
   if(ims->task=Anewobject(AOTP_TASK,
      AOTSK_Entry,(Tag)Imagetask,
      AOTSK_Name,(Tag)"AWebIP",
      AOTSK_Userdata,(Tag)ims,
      AOBJ_Target,(Tag)ims,
      TAG_END))
   {  Asetattrs(ims->task,AOTSK_Start,TRUE,TAG_END);
   }
}

/*------------------------------------------------------------------------*/

static long Setimgsource(struct Imgsource *ims,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=Amethodas(AOTP_OBJECT,ims,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            ims->source=(void *)tag->ti_Data;
            break;
         case AOSDV_Savesource:
            if(ims->filename)
            {  Asetattrs((void *)tag->ti_Data,
                  AOFIL_Copyfile,(Tag)ims->filename,
                  TAG_END);
            }
            break;
         case AOAPP_Screenvalid:
            if(tag->ti_Data)
            {  /* Start processing if we have got all data and are displayed */
               if((ims->flags&IMSF_EOF) && !ims->task
               && Agetattr(ims->source,AOSRC_Displayed))
               {  Startprocessimg(ims);
               }
            }
            else
            {  ObtainSemaphore(&imagetask.screensema);
               imagetask.screen=NULL;
               ReleaseSemaphore(&imagetask.screensema);
               if(ims->task)
               {  Adisposeobject(ims->task);
                  ims->task=NULL;
               }
               Disposedto(ims);
               Anotifyset(ims->source,AOIMP_Srcupdate,TRUE,TAG_END);
            }
            break;
         case AOSDV_Displayed:
            /* If becoming displayed and no bitmap and screen valid, process. */
            if(tag->ti_Data && (ims->flags&IMSF_EOF) && !ims->task
            && Agetattr(Aweb(),AOAPP_Screenvalid))
            {  Startprocessimg(ims);
            }
            break;
      }
   }
   return result;
}

static struct Imgsource *Newimgsource(struct Amset *ams)
{  struct Imgsource *ims;
   if(ims=Allocobject(AOTP_IMGSOURCE,sizeof(struct Imgsource),ams))
   {  Aaddchild(Aweb(),(struct Aobject *)ims,AOREL_APP_USE_SCREEN);
      Setimgsource(ims,ams);
   }
   return ims;
}

static long Getimgsource(struct Imgsource *ims,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=AmethodasA(AOTP_OBJECT,(struct Aobject *)ims,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            PUTATTR(tag,ims->source);
            break;
         case AOSDV_Saveable:
            PUTATTR(tag,BOOLVAL(ims->filename));
            break;
      }
   }
   return result;
}

static long Updateimgsource(struct Imgsource *ims,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOIMS_Dtobject:
            ims->dto=(void *)tag->ti_Data;
            break;
         case AOIMS_Bitmap:
            ims->bitmap=(struct BitMap *)tag->ti_Data;
            break;
         case AOIMS_Mask:
            ims->mask=(UBYTE *)tag->ti_Data;
            break;
         case AOIMS_Width:
            ims->width=tag->ti_Data;
            break;
         case AOIMS_Height:
            ims->height=tag->ti_Data;
            break;
         case AOIMS_Depth:
            ims->depth=tag->ti_Data;
            break;
         case AOIMS_Ourmask:
            SETFLAG(ims->flags,IMSF_OURMASK,tag->ti_Data);
            break;
         case AOIMS_Memsize:
            Asetattrs(ims->source,AOSRC_Memory,tag->ti_Data,TAG_END);
            break;
      }
   }
   Anotifyset(ims->source,AOIMP_Srcupdate,TRUE,TAG_END);
   Changedlayout();
   return 0;
}

static long Srcupdateimgsource(struct Imgsource *ims,struct Amsrcupdate *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   long length=0;
   UBYTE *data=NULL;
   BOOL eof=FALSE;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOURL_Data:
            data=(UBYTE *)tag->ti_Data;
            break;
         case AOURL_Datalength:
            length=tag->ti_Data;
            break;
         case AOURL_Reload:
            ims->flags&=~(IMSF_EOF|IMSF_ERROR);
            if(ims->task)
            {  Adisposeobject(ims->task);
               ims->task=NULL;
               ims->flags&=~IMSF_CACHEFILE;
            }
            break;
         case AOURL_Eof:
            if(tag->ti_Data) eof=TRUE;
            break;
         case AOURL_Error:
            SETFLAG(ims->flags,IMSF_ERROR,tag->ti_Data);
            break;
      }
   }
   if(data && !(ims->flags&IMSF_CACHEFILE))
   {  if(!ims->file) ims->file=Newfile(ims,ams);
      if(ims->file)
      {  Asetattrs(ims->file,
            AOFIL_Data,(Tag)data,
            AOFIL_Datalength,length,
            TAG_END);
      }
   }
   if(eof && ims->file)
   {  Asetattrs(ims->file,AOFIL_Eof,TRUE,TAG_END);
      ims->filename=(UBYTE *)Agetattr(ims->file,AOFIL_Name);
   }
   if(eof && ims->filename && !(ims->flags&IMSF_ERROR))
   {  ims->flags|=IMSF_EOF;
      if(!ims->task && Agetattr(Aweb(),AOAPP_Screenvalid))
      {  ObtainSemaphore(&imagetask.screensema);
         if(!imagetask.screen)
         {  imagetask.screen=(struct Screen *)Agetattr(Aweb(),AOAPP_Screen);
         }
         ReleaseSemaphore(&imagetask.screensema);
         Startprocessimg(ims);
      }
   }
   return 0;
}

static long Addchildimgsource(struct Imgsource *ims,struct Amadd *ama)
{  if(ama->relation==AOREL_SRC_COPY)
   {  if(ims->bitmap)
      {  Asetattrs(ama->child,AOIMP_Srcupdate,TRUE,TAG_END);
      }
   }
   return 0;
}

static void Disposeimgsource(struct Imgsource *ims)
{  if(ims->task) Adisposeobject(ims->task);
   Disposedto(ims);
   Asetattrs(ims->source,AOSRC_Memory,0,TAG_END);
   if(ims->file) Adisposeobject(ims->file);
   Aremchild(Aweb(),(struct Aobject *)ims,AOREL_APP_USE_SCREEN);
   Amethodas(AOTP_OBJECT,ims,AOM_DISPOSE);
}

static void Deinstallimgsource(void)
{
}

USRFUNC_H2
(
static long  , Imgsource_Dispatcher,
struct Imgsource *,ims,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newimgsource((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setimgsource(ims,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getimgsource(ims,(struct Amset *)amsg);
         break;
      case AOM_UPDATE:
         result=Updateimgsource(ims,(struct Amset *)amsg);
         break;
      case AOM_SRCUPDATE:
         result=Srcupdateimgsource(ims,(struct Amsrcupdate *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchildimgsource(ims,(struct Amadd *)amsg);
         break;
      case AOM_DISPOSE:
         Disposeimgsource(ims);
         break;
      case AOM_DEINSTALL:
         Deinstallimgsource();
         break;
      default:
         result=AmethodasA(AOTP_OBJECT,(struct Aobject *)ims,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installimgsource(void)
{  InitSemaphore(&imagetask.screensema);
   if(!Amethod(NULL,AOM_INSTALL,AOTP_IMGSOURCE,(Tag)Imgsource_Dispatcher)) return FALSE;
   return TRUE;
}
