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

/* jfifsource.c - AWeb jfif plugin sourcedriver */

#include "pluginlib.h"
#include "awebjfif.h"
#include <stdlib.h>

#define FILE void
#undef GLOBAL

#include "jpeglib.h"
#include "jerror.h"
#include <string.h>
#include <setjmp.h>
#include "ezlists.h"
#include <math.h>
#include "libraries/awebsupport.h"
#include "libraries/awebclib.h"

#include <cybergraphx/cybergraphics.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <datatypes/pictureclass.h>
#undef NO_INLINE_STDARG
#include <proto/awebsupport.h>
#define NO_INLINE_STDARG
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/cybergraphics.h>

/*--------------------------------------------------------------------*/
/* General data structures                                            */
/*--------------------------------------------------------------------*/

/* A struct Datablock holds one block of data. Once it has been read
 * from the list, the data can be accessed without semaphore protection
 * because it will never change (but the ln_Succ pointer will!). */
struct Datablock
{  NODE(Datablock);
   UBYTE *data;                     /* The data */
   long length;                     /* Length of the data */
};

/* The object instance data for the source driver */
struct Jfifsource
{  struct Sourcedriver sourcedriver;/* Or superclass object instance */
   struct Aobject *source;          /* The AOTP_SOURCE object we belong to */
   struct Aobject *task;            /* Our decoder task */
   struct BitMap *bitmap;           /* The bitmap for the image */
   long width,height;               /* Bitmap dimensions */
   long ready;                      /* Last complete row in bitmap */
   long memory;                     /* Memory in use */
   LIST(Datablock) data;            /* A linked list of data blocks */
   struct SignalSemaphore sema;     /* Protect the common data */
   USHORT flags;                    /* See below */
   short progress;                  /* Progressive display mode */
   short ncolors;                   /* Number of colors requested */
   short dither;                    /* Dither mode requested */
   long maxmem;                     /* Max memory to use */
   UBYTE pen[256];                  /* Jpeg object's colormap mapped to these pens */
   UBYTE allocated[256];            /* This pen is actually allocated */
   struct BitMap *friendbitmap;     /* Friend bitmap */
   struct ColorMap *colormap;       /* The colormap to obtain pens from */
   short debug;                     /* Debug Level */
};

/* Jfifsource flags: */
#define JFIFSF_EOF         0x0001   /* EOF was received */
#define JFIFSF_DISPLAYED   0x0002   /* This image is being displayed */
#define JFIFSF_MEMORY      0x0004   /* Our owner knows our memory size */
#define JFIFSF_READY       0x0008   /* Decoding is ready */
#define JFIFSF_GRAYSCALE   0x0010   /* Grayscale requested */
#define JFIFSF_LOWPRI      0x0020   /* Run decoder at low priority */
#define JFIFSF_MULTI       0x0040   /* Do progressive JPEGs progressively */

/*--------------------------------------------------------------------*/
/* The decoder subtask                                                */
/*--------------------------------------------------------------------*/

/* This structure holds vital data for the decoding subprocess */
struct Decoder
{  struct Datablock *current;       /* The current datablock */
   struct Jfifsource *source;        /* Points back to our Jfifsource */
   /* Output fields: */
   struct BitMap *bitmap;           /* Bitmap to be filled */
   long width,height,depth;         /* Bitmap dimensions */
   USHORT flags;                    /* See below */
   short progress;                  /* Progress counter */
   struct RastPort rp;              /* Temporary for pixelline8 functions */
   struct RastPort temprp;          /* Temporary for pixelline8 functions */
   UBYTE *chunky;                   /* Row of chunky pixels */
};

/* Decoder flags: */
#define DECOF_STOP         0x0001   /* The decoder process should stop */
#define DECOF_EOF          0x0002   /* No more data */
#define DECOF_MAPPED       0x0004   /* Colormap is used */
#define DECOF_CYBERMAP     0x0010   /* Bitmap is CyberGfx */
#define DECOF_CYBERDEEP    0x0020   /* Bitmap is CyberGfx >8 bits */

/*--------------------------------------------------------------------*/
/* Read from the input stream                                         */
/*--------------------------------------------------------------------*/

struct Jfifsourcemgr
{  struct jpeg_source_mgr pub;
   struct Decoder *decoder;
};

/* Initialize source */
METHODDEF(void) Jfifsourceinit(j_decompress_ptr cinfo)
{
}

/* Fill the input buffer.
 * First check for any waiting messages. If the task should stop then
 * raise a fatal error.
 * Then return the next available data block, or wait for the next one if
 * none is available.
 */
METHODDEF(boolean) Jfiffillbuffer(j_decompress_ptr cinfo)
{  static JOCTET eoimarker[2]={ 0xff, JPEG_EOI };
   struct Jfifsourcemgr *src=(struct Jfifsourcemgr *)cinfo->src;
   struct Decoder *decoder=src->decoder;
   struct Taskmsg *tm;
   struct TagItem *tag,*tstate;
   struct Datablock *db;
   BOOL wait;
   for(;;)
   {  wait=FALSE;
      while(!(decoder->flags&DECOF_STOP) && (tm=Gettaskmsg()))
      {  if(tm->amsg && tm->amsg->method==AOM_SET)
         {  tstate=((struct Amset *)tm->amsg)->tags;
            while(tag=NextTagItem(&tstate))
            {  switch(tag->ti_Tag)
               {  case AOTSK_Stop:
                     if(tag->ti_Data) decoder->flags|=DECOF_STOP;
                     break;
                  case AOJFIF_Data:
                     /* Ignore these now */
                     break;
               }
            }
         }
         Replytaskmsg(tm);
      }
      /* Only continue if we shouldn't stop */
      if(decoder->flags&DECOF_STOP)
      {  ERREXIT(cinfo,JERR_INPUT_EOF);
      }
      ObtainSemaphore(&decoder->source->sema);
      if(decoder->current)
      {  db=decoder->current->next;
      }
      else
      {  db=decoder->source->data.first;
      }
      if(db->next)
      {  /* We have a valid block */
         decoder->current=db;
         src->pub.next_input_byte=db->data;
         src->pub.bytes_in_buffer=db->length;
      }
      else if(decoder->source->flags&JFIFSF_EOF)
      {  /* EOF reached, fake an EOI marker */
         decoder->flags|=DECOF_EOF;
         src->pub.next_input_byte=eoimarker;
         src->pub.bytes_in_buffer=2;
      }
      else
      {  /* No more blocks; wait for next block */
         wait=TRUE;
      }
      ReleaseSemaphore(&decoder->source->sema);
      if(!wait)
      {  break;
      }
      Waittask(0);
   }
   return TRUE;
}

/* Skip data */
METHODDEF(void) Jfifskipdata(j_decompress_ptr cinfo,long nbytes)
{  struct Jfifsourcemgr *src=(struct Jfifsourcemgr *)cinfo->src;
   if(nbytes>0)
   {  while(nbytes>src->pub.bytes_in_buffer)
      {  nbytes-=src->pub.bytes_in_buffer;
         Jfiffillbuffer(cinfo);
      }
      src->pub.next_input_byte+=nbytes;
      src->pub.bytes_in_buffer-=nbytes;
   }
}

/* Terminate the source */
METHODDEF(void) Jfifsourceterm(j_decompress_ptr cinfo)
{
}

/*--------------------------------------------------------------------*/
/* Error manager                                                      */
/*--------------------------------------------------------------------*/

struct Jfiferrormgr
{  struct jpeg_error_mgr pub;
   jmp_buf setjmpbuf;
};

/* Error manager, long jumps back to main to do the necessary cleanup */
METHODDEF(void) Jfiferrorexit(j_common_ptr cinfo)
{  struct Jfiferrormgr *jerr=(struct Jfiferrormgr *)cinfo->err;
   longjmp(jerr->setjmpbuf,1);
}

/* Override because exit() is called from a function in jerror.c but
 * we override that function with our own one above. The reference to
 * exit() is still in the included object module and causes linker errors. */

#if 0
/* provided in libmoduleinit.a */
void exit(int rc)
{
}
#endif

METHODDEF(void)
Jfifoutput_message(j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */
  (*cinfo->err->format_message) (cinfo, buffer);

  /* Send it to stdout, adding a newline */

  Aprintf("%s\n", buffer);

}
METHODDEF(void)
Jfifoutput_dummy(j_common_ptr cinfo)
{
}

/*--------------------------------------------------------------------*/
/* Image processing                                                   */
/*--------------------------------------------------------------------*/

#define RGB(x) (((x)<<24)|((x)<<16)|((x)<<8)|(x))

/* Decompress the image and build the bitmap */
static BOOL Decompress(struct Decoder *decoder)
{  struct jpeg_decompress_struct cinfo={0};
   struct Jfiferrormgr jerr={0};
   struct Jfifsourcemgr jsrc={0};
   JSAMPARRAY buffer=NULL;
   long rowsize,fromrow,row,x;
   short depth,pen,map;
   UBYTE r,g,b;
   BOOL error=FALSE;
   BOOL done=FALSE;
   cinfo.err=jpeg_std_error((struct jpeg_error_mgr *)&jerr);
   jerr.pub.error_exit=Jfiferrorexit;
   if (decoder->source->debug)
   {
       jerr.pub.output_message = Jfifoutput_message;
   }
   else
   {
       jerr.pub.output_message = Jfifoutput_dummy;
   }
   if(!setjmp(jerr.setjmpbuf))
   {  /* Here we are after setting the long jump */
      jpeg_create_decompress(&cinfo);
      cinfo.mem->max_memory_to_use=decoder->source->maxmem*1024;
      cinfo.src=(struct jpeg_source_mgr *)&jsrc;
      jsrc.pub.init_source=Jfifsourceinit;
      jsrc.pub.fill_input_buffer=Jfiffillbuffer;
      jsrc.pub.skip_input_data=Jfifskipdata;
      jsrc.pub.resync_to_restart=jpeg_resync_to_restart;
      jsrc.pub.term_source=Jfifsourceterm;
      jsrc.pub.bytes_in_buffer=0;
      jsrc.pub.next_input_byte=NULL;
      jsrc.decoder=decoder;
      jpeg_read_header(&cinfo,TRUE);

      decoder->width=cinfo.image_width;
      decoder->height=cinfo.image_height;

      /* Now we know the image dimensions, allocate a bitmap. */
      if(CyberGfxBase)
      {  depth=GetBitMapAttr(decoder->source->friendbitmap,BMA_DEPTH);
         decoder->bitmap=AllocBitMap(decoder->width,decoder->height,depth,
            BMF_MINPLANES|BMF_CLEAR,decoder->source->friendbitmap);
         if (decoder->bitmap)
                        {
                 if(GetCyberMapAttr(decoder->bitmap,CYBRMATTR_ISCYBERGFX))
              {  decoder->flags|=DECOF_CYBERMAP;
              if(depth>8)
                   {  decoder->flags|=DECOF_CYBERDEEP;
                }
                }
         }
      }
      else
      {  decoder->bitmap=AllocBitMap(decoder->width,decoder->height,8,BMF_CLEAR,NULL);
         depth=8;
      }
      if(!decoder->bitmap)
                {       error=TRUE;
                }
      else
      {  /* Save our bitmap and dimensions. */
              ObtainSemaphore(&decoder->source->sema);
              decoder->source->bitmap=decoder->bitmap;
              decoder->source->width=decoder->width;
              decoder->source->height=decoder->height;
              ReleaseSemaphore(&decoder->source->sema);

              Updatetaskattrs(
                 AOJFIF_Memory,decoder->width*decoder->height*depth/8,
                 TAG_END);

              InitRastPort(&decoder->rp);
              decoder->rp.BitMap=decoder->bitmap;
              if(decoder->flags&DECOF_CYBERMAP)
              {  error=!(decoder->chunky=AllocVec(decoder->width*3,MEMF_PUBLIC));
              }
              else
              {  /* The colour mapping process needs a temporary RastPort plus BitMap
                  * for the PixelLine8 functions. */
                 InitRastPort(&decoder->temprp);
                 error=!(decoder->temprp.BitMap=AllocBitMap(
                       8*(((decoder->width+15)>>4)<<1),1,8,0,decoder->bitmap))
                    || !(decoder->chunky=AllocVec(((decoder->width+15)>>4)<<4,MEMF_PUBLIC));
              }

              if(cinfo.jpeg_color_space==JCS_GRAYSCALE)
              {  decoder->source->flags|=JFIFSF_GRAYSCALE;
              }

              if(!(decoder->flags&DECOF_CYBERDEEP) || decoder->source->ncolors
              || (decoder->source->flags&JFIFSF_GRAYSCALE))
              {  decoder->flags|=DECOF_MAPPED;
              }

              cinfo.dct_method=JDCT_IFAST;
              cinfo.do_fancy_upsampling=FALSE;
              cinfo.do_block_smoothing=FALSE;
              cinfo.out_color_space=(decoder->source->flags&JFIFSF_GRAYSCALE)?JCS_GRAYSCALE:JCS_RGB;
              if(decoder->flags&DECOF_MAPPED)
              {  cinfo.quantize_colors=TRUE;
                 cinfo.desired_number_of_colors=decoder->source->ncolors?decoder->source->ncolors:256;
                 cinfo.dither_mode=decoder->source->dither;
                 cinfo.two_pass_quantize=FALSE;
              }
              if(decoder->source->flags&JFIFSF_MULTI)
              {  cinfo.buffered_image=TRUE;
              }
              jpeg_start_decompress(&cinfo);

              rowsize=cinfo.output_width*cinfo.output_components;
              /* If allocation fails, the error exit is taken from the library */
              buffer=cinfo.mem->alloc_sarray((j_common_ptr)&cinfo,JPOOL_IMAGE,rowsize,1);

              done=FALSE;
      }
      while(!error && !done)
      {  fromrow=0;
         if(decoder->source->flags&JFIFSF_MULTI)
         {  jpeg_start_output(&cinfo,cinfo.input_scan_number);
         }
         while(cinfo.output_scanline<cinfo.output_height)
         {  jpeg_read_scanlines(&cinfo,buffer,1);
            row=cinfo.output_scanline-1;
            if(decoder->flags&DECOF_MAPPED)
            {  for(x=0;x<decoder->width;x++)
               {  pen=buffer[0][x];
                  if(decoder->source->allocated[pen])
                  {  map=decoder->source->pen[pen];
                  }
                  else
                  {  r=cinfo.colormap[0][pen];
                     if(cinfo.out_color_components>2)
                     {  g=cinfo.colormap[1][pen];
                        b=cinfo.colormap[2][pen];
                     }
                     else
                     {  g=b=r;
                     }
                     map=ObtainBestPen(decoder->source->colormap,
                        RGB(r),RGB(g),RGB(b),
                        OBP_Precision,PRECISION_IMAGE,
                        TAG_END);
                     decoder->source->pen[pen]=map;
                     decoder->source->allocated[pen]=TRUE;
                  }
                  decoder->chunky[x]=map;
               }
               if(decoder->flags&DECOF_CYBERMAP)
               {  WritePixelArray(decoder->chunky,0,0,decoder->width,
                     &decoder->rp,0,row,decoder->width,1,RECTFMT_LUT8);
               }
               else
               {  WritePixelLine8(&decoder->rp,0,row,decoder->width,decoder->chunky,
                  &decoder->temprp);
               }
            }
            else
            {  WritePixelArray(buffer[0],0,0,rowsize,
                  &decoder->rp,0,row,decoder->width,1,RECTFMT_RGB);
            }

            if(++decoder->progress==decoder->source->progress || row==decoder->height-1)
            {  Updatetaskattrs(
                  AOJFIF_Readyfrom,fromrow,
                  AOJFIF_Readyto,row,
                  TAG_END);
               decoder->progress=0;
               fromrow=row+1;
            }
         }
         if(decoder->source->flags&JFIFSF_MULTI)
         {  jpeg_finish_output(&cinfo);
         }
         done=(!(decoder->source->flags&JFIFSF_MULTI) || jpeg_input_complete(&cinfo));
      }
      if(!(decoder->flags&DECOF_STOP))
      {  jpeg_finish_decompress(&cinfo);
      }
   }
   else
   {  /* We longjumped to here in case of fatal error */
      error=TRUE;
   }
   Updatetaskattrs(
      AOJFIF_Readyfrom,decoder->height-1,
      AOJFIF_Readyto,decoder->height-1,
      AOJFIF_Imgready,TRUE,
      TAG_END);
   jpeg_destroy_decompress(&cinfo);
   if(decoder->chunky) FreeVec(decoder->chunky);
   if(decoder->temprp.BitMap) FreeBitMap(decoder->temprp.BitMap);
   return (BOOL)!error;
}

/* Main subtask process. */
__saveds static  void Decodetask(struct Jfifsource *is)
{
   struct Decoder decoderdata,*decoder=&decoderdata;
   struct Task *task=FindTask(NULL);

   memset(&decoderdata,0,sizeof(decoderdata));
   decoder->source=is;
   if(decoder->source->flags&JFIFSF_LOWPRI)
   {  SetTaskPri(task, max( -128, task->tc_Node.ln_Pri - 1));  /* olsen: keep this in range */
   }

   if(!Decompress(decoder)) goto err;
   return;

err:
   Updatetaskattrs(AOJFIF_Error,TRUE,TAG_END);
}

/*--------------------------------------------------------------------*/
/* Main task functions                                                */
/*--------------------------------------------------------------------*/

/* Save all our source data blocks in this AOTP_FILE object */
static void Savesource(struct Jfifsource *js,struct Aobject *file)
{  struct Datablock *db;
   for(db=js->data.first;db->next;db=db->next)
   {  Asetattrs(file,
         AOFIL_Data,db->data,
         AOFIL_Datalength,db->length,
         TAG_END);
   }
}

/* Start the decoder task. Only start it if the screen is
 * currenty valid. */
static void Startdecoder(struct Jfifsource *js)
{  struct Screen *screen=NULL;
   if(Agetattr(Aweb(),AOAPP_Screenvalid))
   {  Agetattrs(Aweb(),
         AOAPP_Screen,&screen,
         AOAPP_Colormap,&js->colormap,
         TAG_END);
      if(screen) js->friendbitmap=screen->RastPort.BitMap;
      if(js->task=Anewobject(AOTP_TASK,
         AOTSK_Entry,Decodetask,
         AOTSK_Name,"AWebJFIF decoder",
         AOTSK_Userdata,js,
         AOTSK_Stacksize,32000,
         AOBJ_Target,js,
         TAG_END))
      {  Asetattrs(js->task,AOTSK_Start,TRUE,TAG_END);
      }
   }
}

/* Notify our related Jfifcopy objects that the bitmap has gone.
 * Delete the bitmap, and release all obtained pens. */
static void Releaseimage(struct Jfifsource *js)
{  short i;
   Anotifyset(js->source,AOJFIF_Bitmap,NULL,TAG_END);
   if(js->bitmap)
   {  FreeBitMap(js->bitmap);
      js->bitmap=NULL;
   }
   for(i=0;i<256;i++)
   {  if(js->allocated[i])
      {  ReleasePen(js->colormap,js->pen[i]);
         js->allocated[i]=FALSE;
      }
   }
   js->colormap=NULL;
   js->flags&=~JFIFSF_READY;
   js->memory=0;
   Asetattrs(js->source,AOSRC_Memory,0,TAG_END);
}

/* Delete all source data */
static void Releasedata(struct Jfifsource *js)
{  struct Datablock *db;
   while(db=REMHEAD(&js->data))
   {  if(db->data) FreeVec(db->data);
      FreeVec(db);
   }
}

/*--------------------------------------------------------------------*/
/* Plugin sourcedriver object dispatcher functions                    */
/*--------------------------------------------------------------------*/

static ULONG Setsource(struct Jfifsource *js,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   UBYTE *arg,*p;
   Amethodas(AOTP_SOURCEDRIVER,(struct Aobject *)js,AOM_SET,amset->tags);
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            js->source=(struct Aobject *)tag->ti_Data;
            break;
         case AOSDV_Savesource:
            Savesource(js,(struct Aobject *)tag->ti_Data);
            break;
         case AOSDV_Displayed:
            if(tag->ti_Data)
            {  /* We are becoming displayed. Start our decoder task if
                * we have data but no bitmap yet, and task is not yet
                * running. */
               js->flags|=JFIFSF_DISPLAYED;
               if(js->data.first->next && !js->bitmap && !js->task)
               {  Startdecoder(js);
               }
            }
            else
            {  /* We are not displayed anywhere now. Leave the bitmap
                * intact so we don't need to decode it again when we
                * are becoming displayed again. */
               js->flags&=~JFIFSF_DISPLAYED;
            }
            break;
         case AOAPP_Screenvalid:
            if(tag->ti_Data)
            {  if(js->data.first->next && (js->flags&JFIFSF_DISPLAYED)
               && !js->task)
               {  Startdecoder(js);
               }
            }
            else
            {  if(js->task)
               {  Adisposeobject(js->task);
                  js->task=NULL;
               }
               Releaseimage(js);
            }
            break;
         case AOSDV_Arguments:
            arg=(UBYTE *)tag->ti_Data;
            if(arg)
            {  for(p=arg;*p;p++)
               {  if(!strnicmp(p,"PROGRESS=",9))
                  {  js->progress=atoi(p+9);
                     p+=9;
                  }
                  /* Not publicly supported but leave it for testing */
                  else if(!strnicmp(p,"COLORS=",7))
                  {  js->ncolors=atoi(p+7);
                     p+=7;
                  }
                  else if(!strnicmp(p,"DITHER=",7))
                  {  switch(atoi(p+7))
                     {  case 0:js->dither=JDITHER_NONE;break;
                        case 1:js->dither=JDITHER_ORDERED;break;
                        case 2:js->dither=JDITHER_FS;break;
                     }
                     p+=7;
                  }
                  else if(!strnicmp(p,"GRAYSCALE",9))
                  {  js->flags|=JFIFSF_GRAYSCALE;
                     p+=9;
                  }
                  else if(!strnicmp(p,"MAXMEM=",7))
                  {  js->maxmem=atoi(p+7);
                     if(js->maxmem<1) js->maxmem=1;
                     p+=7;
                  }
                  else if(!strnicmp(p,"LOWPRI",6))
                  {  js->flags|=JFIFSF_LOWPRI;
                     p+=6;
                  }
                  else if(!strnicmp(p,"MULTIPASS=",10))
                  {  if(atoi(p+10)) js->flags|=JFIFSF_MULTI;
                     else js->flags&=~JFIFSF_MULTI;
                     p+=10;
                  }
                  else if(!strnicmp(p,"DEBUG=",6))
                  {  js->debug = atoi(p+6);
                     p+=6;
                  }

               }
            }
            break;
      }
   }
   return 0;
}

static struct Jfifsource *Newsource(struct Amset *amset)
{  struct Jfifsource *js;
   if(js=Allocobject(AwebPluginBase->sourcedriver,sizeof(struct Jfifsource),amset))
   {  InitSemaphore(&js->sema);
      NEWLIST(&js->data);
      Aaddchild(Aweb(),(struct Aobject *)js,AOREL_APP_USE_SCREEN);
      js->progress=4;
      js->ncolors=0;
      js->dither=JDITHER_FS;
      js->maxmem=1024;
      js->flags|=JFIFSF_MULTI;
      Setsource(js,amset);
      if(js->ncolors!=0 && js->ncolors<4)
      {  if(js->flags&JFIFSF_GRAYSCALE) js->ncolors=4;
         else js->ncolors=8;
      }
      if(js->ncolors>256) js->ncolors=256;
   }
   return js;
}

static ULONG Getsource(struct Jfifsource *js,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   AmethodasA(AOTP_SOURCEDRIVER,(struct Aobject *)js,(struct Amessage *)amset);
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            PUTATTR(tag,js->source);
            break;
         case AOSDV_Saveable:
            PUTATTR(tag,(js->flags&JFIFSF_EOF)?TRUE:FALSE);
            break;
      }
   }
   return 0;
}

static ULONG Srcupdatesource(struct Jfifsource *js,struct Amsrcupdate *amsrcupdate)
{  struct TagItem *tag,*tstate;
   UBYTE *data=NULL;
   long datalength=0;
   BOOL eof=FALSE;
   AmethodasA(AOTP_SOURCEDRIVER,(struct Aobject *)js,(struct Amessage *)amsrcupdate);
   tstate=amsrcupdate->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOURL_Data:
            data=(UBYTE *)tag->ti_Data;
            break;
         case AOURL_Datalength:
            datalength=tag->ti_Data;
            break;
         case AOURL_Eof:
            if(tag->ti_Data)
            {  eof=TRUE;
               js->flags|=JFIFSF_EOF;
            }
            break;
      }
   }
   if(data && datalength)
   {  struct Datablock *db;
      if(db=AllocVec(sizeof(struct Datablock),MEMF_PUBLIC|MEMF_CLEAR))
      {  if(db->data=AllocVec(datalength,MEMF_PUBLIC))
         {  memmove(db->data,data,datalength);
            db->length=datalength;
            ObtainSemaphore(&js->sema);
            ADDTAIL(&js->data,db);
            ReleaseSemaphore(&js->sema);
         }
         else
         {  FreeVec(db);
         }
      }
      if(!js->task)
      {  Startdecoder(js);
      }
   }
   if((data && datalength) || eof)
   {  if(js->task)
      {  Asetattrsasync(js->task,AOJFIF_Data,TRUE,TAG_END);
      }
   }
   return 0;
}

static ULONG Updatesource(struct Jfifsource *js,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   BOOL notify=FALSE;
   long readyfrom=0,readyto=-1;
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOJFIF_Readyfrom:
            readyfrom=tag->ti_Data;
            notify=TRUE;
            break;
         case AOJFIF_Readyto:
            readyto=tag->ti_Data;
            if(readyto>js->ready) js->ready=readyto;
            notify=TRUE;
            break;
         case AOJFIF_Imgready:
            if(tag->ti_Data) js->flags|=JFIFSF_READY;
            else js->flags&=~JFIFSF_READY;
            break;
         case AOJFIF_Error:
            break;
         case AOJFIF_Memory:
            js->memory+=tag->ti_Data;
            Asetattrs(js->source,
               AOSRC_Memory,js->memory,
               TAG_END);
      }
   }
   if(notify && js->bitmap)
   {  Anotifyset(js->source,
         AOJFIF_Bitmap,js->bitmap,
         AOJFIF_Width,js->width,
         AOJFIF_Height,js->height,
         AOJFIF_Readyfrom,readyfrom,
         AOJFIF_Readyto,readyto,
         AOJFIF_Imgready,js->flags&JFIFSF_READY,
         TAG_END);
   }
   return 0;
}

static ULONG Addchildsource(struct Jfifsource *js,struct Amadd *amadd)
{  if(amadd->relation==AOREL_SRC_COPY)
   {  if(js->bitmap)
      {  Asetattrs(amadd->child,
            AOJFIF_Bitmap,js->bitmap,
            AOJFIF_Width,js->width,
            AOJFIF_Height,js->height,
            AOJFIF_Readyfrom,0,
            AOJFIF_Readyto,js->ready,
            AOJFIF_Imgready,js->flags&JFIFSF_READY,
            TAG_END);
      }
   }
   return 0;
}

static void Disposesource(struct Jfifsource *js)
{  if(js->task)
   {  Adisposeobject(js->task);
   }
   Releaseimage(js);
   Releasedata(js);
   Aremchild(Aweb(),(struct Aobject *)js,AOREL_APP_USE_SCREEN);
   Amethodas(AOTP_SOURCEDRIVER,(struct Aobject *)js,AOM_DISPOSE);
}

USRFUNC_H2
(
__saveds  ULONG  , Dispatchersource,
struct Jfifsource *,js,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  ULONG result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(ULONG)Newsource((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setsource(js,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getsource(js,(struct Amset *)amsg);
         break;
      case AOM_SRCUPDATE:
         result=Srcupdatesource(js,(struct Amsrcupdate *)amsg);
         break;
      case AOM_UPDATE:
         result=Updatesource(js,(struct Amset *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchildsource(js,(struct Amadd *)amsg);
         break;
      case AOM_DISPOSE:
         Disposesource(js);
         break;
      default:
         result=AmethodasA(AOTP_SOURCEDRIVER,(struct Aobject *)js,amsg);
         break;
   }
   return result;

    USRFUNC_EXIT
}
