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

/* pngsource.c - AWeb png plugin sourcedriver */

#include "pluginlib.h"
#include "awebpng.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ezlists.h>
#include <libraries/awebsupport.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <datatypes/pictureclass.h>
#undef NO_INLINE_STDARG
#include <proto/awebsupport.h>
#define NO_INLINE_STDARG
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/Picasso96API.h>

#ifndef __ABLIT_H__
typedef union
{
    uint32 color;
    struct
    {
        uint8 r, g, b, a;
    } pixel;
} PixelRGBA32_t;
#endif

#include "png.h"

#include "libraries/awebclib.h"
/* Alpha values >= THRESHOLD are opaque, < THRESHOLD are transparent */
/* As suggested by the PNG docs, treat only alpha 0 as transparent */
#define THRESHOLD 0x01

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
struct Pngsource
{  struct Sourcedriver sourcedriver;/* Or superclass object instance */
   struct Aobject *source;          /* The AOTP_SOURCE object we belong to */
   struct Aobject *task;            /* Our decoder task */
   struct BitMap *bitmap;           /* The bitmap for the image */
   UBYTE *mask;                     /* Transparent mask for the image */
   BOOL  alpha;                     /* We have an alpha channel */
   long width,height;               /* Bitmap dimensions */
   long ready;                      /* Last complete row in bitmap */
   long memory;                     /* Memory in use */
   LIST(Datablock) data;            /* A linked list of data blocks */
   struct SignalSemaphore sema;     /* Protect the common data */
   USHORT flags;                    /* See below */
   short progress;                  /* Progressive display mode */
   float gamma;                     /* Screen gamma correction */
   UBYTE map[256];                  /* Palette entries mapped to these pens */
   UBYTE mapped[256];               /* This palette entry has been mapped */
   UBYTE allocated[256];            /* This screen pen (not palette entry) was allocated */
   struct BitMap *friendbitmap;     /* Friend bitmap */
   struct ColorMap *colormap;       /* The colormap to obtain pens from */
};

/* Pngsource flags: */
#define PNGSF_EOF          0x0001   /* EOF was received */
#define PNGSF_DISPLAYED    0x0002   /* This image is being displayed */
#define PNGSF_MEMORY       0x0004   /* Our owner knows our memory size */
#define PNGSF_READY        0x0010   /* Decoding is ready */
#define PNGSF_LOWPRI       0x0020   /* Run decoder at low priority */
#define PNGSF_MULTI        0x0040   /* Use multipass for interlaced pngs */

/*--------------------------------------------------------------------*/
/* The decoder subtask                                                */
/*--------------------------------------------------------------------*/

/* This structure holds vital data for the decoding subprocess */
struct Decoder
{  struct Datablock *current;       /* The current datablock */
   long currentbyte;                /* The current byte in the current datablock */
   struct Pngsource *source;        /* Points back to our Pngsource */
   /* Output fields: */
   struct BitMap *bitmap;           /* Bitmap to be filled */
   UBYTE *mask;                     /* Transparent mask to be built */
   long width,height,depth;         /* Bitmap dimensions */
   long maskw;                      /* Width of transparent mask */
   USHORT flags;                    /* See below */
   short progress;                  /* Progress counter */
   UBYTE transpcolor;               /* Transparent color if transparent */
   struct RastPort rp;              /* Temporary for pixelline8 functions */
   struct RastPort temprp;          /* Temporary for pixelline8 functions */
   UBYTE *chunky;                   /* Row of chunky pixels */
};

/* Decoder flags: */
#define DECOF_STOP         0x0001   /* The decoder process should stop */
#define DECOF_EOF          0x0002   /* No more data */
#define DECOF_TRANSPARENT  0x0004   /* Transparent png */
#define DECOF_INTERLACED   0x0008   /* Interlaced png */
//#define DECOF_CYBERMAP     0x0010   /* Bitmap is CyberGfx (really p96 on os4)*/
//#define DECOF_CYBERDEEP    0x0020   /* Bitmap is CyberGfx >8 bits */

/*--------------------------------------------------------------------*/
/* Read from the input stream                                         */
/*--------------------------------------------------------------------*/

/* Read the next byte. First check any waiting messages, if the task
 * should stop then set the DECOF_STOP flag.
 * If end of current block reached, skip to next block. If no more
 * blocks available, wait until a new block is added or eof is
 * reached. */
static UBYTE Readbyte(struct Decoder *decoder)
{  struct Taskmsg *tm;
   struct TagItem *tag,*tstate;
   struct Datablock *db;
   BOOL wait;
   UBYTE retval=0;


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
                  case AOPNG_Data:
                     /* Ignore these now */
                     break;
               }
            }
         }
         Replytaskmsg(tm);
      }
      /* Only continue if we shouldn't stop */
      if(decoder->flags&DECOF_STOP) break;
      if(decoder->current)
      {  if(++decoder->currentbyte>=decoder->current->length)
         {  /* End of block reached */
            ObtainSemaphore(&decoder->source->sema);
            db=decoder->current->next;
            if(db->next)
            {  decoder->current=db;
               decoder->currentbyte=0;
            }
            else if(decoder->source->flags&PNGSF_EOF)
            {  decoder->flags|=DECOF_EOF;
            }
            else
            {  /* No more blocks; wait for next block */
               wait=TRUE;
            }
            ReleaseSemaphore(&decoder->source->sema);
         }
      }
      else
      {  /* No current block yet */
         ObtainSemaphore(&decoder->source->sema);
         db=decoder->source->data.first;
         if(db->next)
         {  decoder->current=db;
            decoder->currentbyte=0;
         }
         else if(decoder->source->flags&PNGSF_EOF)
         {  decoder->flags|=DECOF_EOF;
         }
         else
         {  /* No block yet; wait for next block */
            wait=TRUE;
         }
         ReleaseSemaphore(&decoder->source->sema);
      }
      if(!wait)
      {  if(!(decoder->flags&DECOF_EOF))
         {  retval=decoder->current->data[decoder->currentbyte];
         }
         break;
      }
      Waittask(0);
   }
   return retval;
}

/* Read or skip a number of bytes. Returns TRUE if ok, FALSE of EOF reached before
 * end of block, or task should stop. If block is passed as NULL, data is skipped. */
static BOOL Readblock(struct Decoder *decoder,UBYTE *block,long length)
{  UBYTE c;

   while(length && !(decoder->flags&(DECOF_STOP|DECOF_EOF)))
   {  c=Readbyte(decoder);
      if(block) *block++=c;
      length--;
   }
   return (BOOL)!(decoder->flags&(DECOF_STOP|DECOF_EOF));
}

static void Readpngdata(png_structp png,png_bytep data,png_size_t length)
{  struct Decoder *decoder=(struct Decoder *)png_get_io_ptr(png);
   if(!Readblock(decoder,data,length))
   {  png_error(png,"Read Error");
   }
}

/*--------------------------------------------------------------------*/
/* Parse the PNG file and build bitmap                                */
/*--------------------------------------------------------------------*/

#define RGB(x) (((x)<<24)|((x)<<16)|((x)<<8)|(x))

/* Read image parameters and data */
static BOOL Parsepngimage(struct Decoder *decoder)
{  png_structp png=NULL;
   png_infop pnginfo=NULL;
   png_uint_32 width,height;
   int bitdepth,colortype,interlace,npalette,ntrans = 0;
   double gamma;
   png_color *palette=NULL;
   png_bytep trans=NULL;
   png_byte *buffer=NULL;
   png_byte **rows=NULL;
   long rowbytes,depth,npass=0,pass,x,fromrow,y;
   short pen,map;
   UBYTE r,g,b;
   UBYTE *trow;
   BOOL error=FALSE,transpixel;

   decoder->chunky = NULL;
   decoder->temprp.BitMap = NULL;

   if((png = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL) )
      && (pnginfo=png_create_info_struct(png)))
   {  if(!setjmp(png->jmpbuf))
      {  /* Normal fallthrough */
         png_set_read_fn(png,decoder,Readpngdata);
         png_read_info(png,pnginfo);
         png_get_IHDR(png,pnginfo,&width,&height,&bitdepth,&colortype,
                      &interlace,NULL,NULL);
         decoder->width=width;
         decoder->height=height;

         if(colortype==PNG_COLOR_TYPE_GRAY && bitdepth<8)
         {  png_set_expand(png);
         }
         if(png_get_valid(png,pnginfo,PNG_INFO_tRNS))
         {  if(colortype==PNG_COLOR_TYPE_PALETTE)
            {  png_get_tRNS(png,pnginfo,&trans,&ntrans,NULL);
            }
            else
            {  /* Expand grayscale or color to alpha channel */
               png_set_expand(png);
            }
            decoder->flags|=DECOF_TRANSPARENT;
         }

         if(bitdepth==16)
         {  png_set_strip_16(png);
         }
         if(bitdepth<8)
         {  png_set_packing(png);
         }
         if(!png_get_gAMA(png,pnginfo,&gamma))
         {  gamma=0.50;
         }
         png_set_gamma(png,decoder->source->gamma,gamma);
         npass=png_set_interlace_handling(png);

         png_read_update_info(png,pnginfo);
         rowbytes=png_get_rowbytes(png,pnginfo);
         colortype=png_get_color_type(png,pnginfo);
         bitdepth=png_get_bit_depth(png,pnginfo);
         if(colortype&PNG_COLOR_MASK_ALPHA) decoder->flags|=DECOF_TRANSPARENT;

         /* Hereafter comes the image data. Allocate a bitmap first.
          * Always allocate a bitmap of depth >=8. Even if our source has less
          * planes, the colours may be remapped to pen numbers up to 255. */
         /* If we have a deep screen use 32 bit else if < 8 use 8 */
         /* if using 8 use a transparent mask otherwise use alpha */

         depth = GetBitMapAttr( decoder->source->friendbitmap, BMA_DEPTH);
         if(depth > 8)
         {
             depth = 32;
             decoder->bitmap=p96AllocBitMap(decoder->width,decoder->height,depth,
                                            BMF_USERPRIVATE|BMF_CLEAR,NULL,RGBFB_R8G8B8A8);
         }
         else
         {
            // 8 bit chunky clut bitmap
             decoder->bitmap=p96AllocBitMap(decoder->width,decoder->height,8,
                                            BMF_USERPRIVATE|BMF_CLEAR,NULL,RGBFB_CLUT);

         }

         if(!decoder->bitmap) return FALSE;
         /* if we are on a screen shallower than 8 we are going to use */
         /* a transparent mask if > 8 bitmap with alpha channel */

         if(decoder->flags&DECOF_TRANSPARENT && (depth <= 8))
         {
            decoder->maskw = GetBitMapAttr( decoder->bitmap, BMA_WIDTH ) >> 3;
            decoder->mask=(UBYTE *)AllocVec(decoder->maskw*decoder->height, MEMF_CLEAR );
         }

         /* Save our bitmap and dimensions. */
         ObtainSemaphore(&decoder->source->sema);
         decoder->source->bitmap = decoder->bitmap;
         decoder->source->mask   = decoder->mask;
         if ((decoder->flags & DECOF_TRANSPARENT) && (decoder->mask == NULL))
         {
            decoder->source->alpha = TRUE;
         }
         else
         {
            decoder->source->alpha = FALSE;
         }
         decoder->source->width  = decoder->width;
         decoder->source->height = decoder->height;
         ReleaseSemaphore(&decoder->source->sema);

         Updatetaskattrs(
            AOPNG_Memory,decoder->width*decoder->height*depth/8+
               (decoder->mask?(decoder->maskw*decoder->height/8):0),
            TAG_END);

         /* For normal images, we just need 1 row of data. For interlaced images,
          * we need to remember the entire image because we can't reconstruct
          * the row data from the previous pass just from the bitmap. */
         if(npass==1)
         {  if(!(buffer=AllocVec(rowbytes,MEMF_CLEAR))) longjmp(png->jmpbuf,1);
            rows=&buffer;
         }
         else
         {  /* For transparent interlaced palette images, find a palette entry
             * that is transparent. Initialize the rows with that value so
             * early passes won't obscure areas that become transparent in a
             * later pass. If there is no such entry - well, then the image
             * isn't transparent after all and we need not bother. */
            map=0;
            if(colortype==PNG_COLOR_TYPE_PALETTE && trans)
            {  for(pen=0;pen<ntrans;pen++)
               {  if(trans[pen]<THRESHOLD)
                  {  map=pen;
                     break;
                  }
               }
            }
            if(!(rows=AllocVec(decoder->height*sizeof(png_byte *),MEMF_CLEAR)))
            {  longjmp(png->jmpbuf,1);
            }
            for(y=0;y<decoder->height;y++)
            {  if(!(rows[y]=AllocVec(rowbytes, MEMF_CLEAR )))
               {  longjmp(png->jmpbuf,1);
               }
               if(map) memset(rows[y],map,rowbytes);
            }
         }

         if(colortype==PNG_COLOR_TYPE_PALETTE)
         {  png_get_PLTE(png,pnginfo,&palette,&npalette);
         }

         for(pass=0;pass<npass;pass++)
         {  struct RenderInfo ri;
            fromrow=0;
            for(y=0;y<decoder->height;y++)
            {
               /* For normal images use a fixed buffer. For interlaced images,
                * use a separate buffer for each row. */
               if(npass>1)
               {  buffer=rows[y];
               }

               /* Use 'sparkle' effect for transparent interlaced images */
               if(npass==1 || (decoder->flags&DECOF_TRANSPARENT))
               {  png_read_rows(png,&buffer,NULL,1);
               }
               else
               {  png_read_rows(png,NULL,&buffer,1);
               }

               int32 lock = p96LockBitMap( decoder->bitmap, &ri, sizeof(struct RenderInfo) );
               if (lock)
               {
                     if ( ri.RGBFormat == RGBFB_R8G8B8A8 )
                     {
                         uint32 pitch = y * ri.BytesPerRow / sizeof(PixelRGBA32_t); // pixel = 4 bytes, but we use a index below, scale down by 4 bytes
                         PixelRGBA32_t *pixel = (PixelRGBA32_t*)ri.Memory;
                         for(x=0;x<decoder->width;x++)
                         {
                            transpixel=FALSE;
                            if(decoder->mask)
                            {
                                trow = decoder->mask+y*decoder->maskw;
                            }
                            switch(colortype)
                            {
                            case PNG_COLOR_TYPE_GRAY:
                                pixel[x+pitch].pixel.r = buffer[x];
                                pixel[x+pitch].pixel.g = buffer[x];
                                pixel[x+pitch].pixel.b = buffer[x];
                                pixel[x+pitch].pixel.a = 0xff;
                                break;
                            case PNG_COLOR_TYPE_GRAY_ALPHA:
                                pixel[x+pitch].pixel.r = buffer[2*x];
                                pixel[x+pitch].pixel.g = buffer[2*x];
                                pixel[x+pitch].pixel.b = buffer[2*x];
                                pixel[x+pitch].pixel.a = buffer[2*x+1];
                                transpixel=(buffer[2*x+1]<THRESHOLD);
                                break;
                            case PNG_COLOR_TYPE_PALETTE:
                                pen = buffer[x];
                                pixel[x+pitch].pixel.r = palette[pen].red;
                                pixel[x+pitch].pixel.g = palette[pen].green;
                                pixel[x+pitch].pixel.b = palette[pen].blue;
// this seems to be buggy with gcc4:  transpixel=(trans && pen<ntrans && trans[pen]<THRESHOLD);
                                if( trans && pen < ntrans)
                                {
                                    transpixel = trans[pen]<THRESHOLD ;
                                    pixel[x+pitch].pixel.a=trans[pen];
                                }
                                else
                                {
                                    pixel[x+pitch].pixel.a=255;
                                }
                                break;
                            case PNG_COLOR_TYPE_RGB:
                                // we access the 4th byte illegaly, but we only read from it. That should be save
                                pixel[x+pitch].color = *(uint32*)(buffer+3*x) | 0x000000ff;
                                break;
                            case PNG_COLOR_TYPE_RGB_ALPHA:
                                pixel[x+pitch].color = *(uint32*)(buffer+4*x);
                                transpixel=(pixel[x+pitch].pixel.a<THRESHOLD);
                                break;
                            }
                            if(decoder->mask && !transpixel)
                            {  trow[x>>3]|=0x80>>(x&0x7);
                            }
                        }
                    }
                    else if ( ri.RGBFormat == RGBFB_CLUT )
                    {
                        // 8 bit platette mode
                        uint32 pitch = (uint32)ri.BytesPerRow;
                        uint8 *pixel = (uint8*)ri.Memory;

                        for(x=0; pixel && x<decoder->width;x++)
                        {
                         map=-1;
                         pen=-1;
                         transpixel=FALSE;
                         if(decoder->mask)
                         {  trow=decoder->mask+y*decoder->maskw;
                         }
                         switch(colortype)
                         {  case PNG_COLOR_TYPE_GRAY:
                               // See if a pen is allocated for this shade
                               pen=buffer[x];
                               if(decoder->source->mapped[pen])
                               {  map=decoder->source->map[pen];
                               }
                               else
                               {  r=g=b=pen;
                               }
                               break;
                            case PNG_COLOR_TYPE_GRAY_ALPHA:
                               // See if a pen is allocated for this shade
                               pen=buffer[2*x];
                               if(decoder->source->mapped[pen])
                               {  map=decoder->source->map[pen];
                               }
                               else
                               {  r=g=b=pen;
                               }
                               transpixel=(buffer[2*x+1]<THRESHOLD);
                               break;
                            case PNG_COLOR_TYPE_PALETTE:
                               pen=buffer[x];
                               if(decoder->source->mapped[pen])
                               {  map=decoder->source->map[pen];
                               }
                               else
                               {  r=palette[pen].red;
                                  g=palette[pen].green;
                                  b=palette[pen].blue;
                               }
                               if (trans && pen < ntrans )
                               {
                                   transpixel = ( trans[pen] < THRESHOLD );
                               }
                               break;
                            case PNG_COLOR_TYPE_RGB:
                               r=buffer[3*x  ];
                               g=buffer[3*x+1];
                               b=buffer[3*x+2];
                               break;
                            case PNG_COLOR_TYPE_RGB_ALPHA:
                               r=buffer[4*x  ];
                               g=buffer[4*x+1];
                               b=buffer[4*x+2];
                               transpixel=(buffer[4*x+3]<THRESHOLD);
                               break;
                         }
                         if(map<0)
                         {  map = ObtainBestPen(decoder->source->colormap,
                                              RGB(r),RGB(g),RGB(b),TAG_END);
                            if(decoder->source->allocated[map]) {
                               ReleasePen(decoder->source->colormap,map);
                            } else {
                               decoder->source->allocated[map]=TRUE;
                            }
                            if(pen>=0)
                            {  decoder->source->map[pen]=map;
                               decoder->source->mapped[pen]=TRUE;
                            }
                         }
                         /*decoder->chunky[x]=map;*/
                         pixel[x+y*pitch] = map;
                         if(decoder->mask && !transpixel)
                         {  trow[x>>3]|=0x80>>(x&0x7);
                         }
                      }
                    }
                    p96UnlockBitMap( decoder->bitmap, lock );
               }

               if((decoder->source->flags & PNGSF_MULTI) && (++decoder->progress==decoder->source->progress || y==decoder->height-1))
               {
                   Updatetaskattrs(
                     AOPNG_Readyfrom,fromrow,
                     AOPNG_Readyto,y,
                 //    AOPNG_Imgready,pass==npass-1 && y==decoder->height-1,
                     TAG_END);
                  decoder->progress=0;
                  fromrow=y+1;
               }
               if(pass == npass -1 && y == decoder->height - 1)
               {
                   Updatetaskattrs(
                     AOPNG_Readyfrom,y,
                     AOPNG_Readyto,y,
                     AOPNG_Imgready,TRUE,
                     TAG_END);

               }
            }
         }
      }
      else
      {  /* Error jumps to here */
         error=TRUE;
      }
   }
   png_destroy_read_struct(&png,&pnginfo,NULL);
   if(npass==1)
   {  if(buffer) FreeVec(buffer);
   }
   else
   {  if(rows)
      {  for(y=0;y<decoder->height;y++)
         {  if(rows[y]) FreeVec(rows[y]);
         }
         FreeVec(rows);
      }
   }
   return (BOOL)!error;
}

/* Main subtask process. */

__saveds static  void Decodetask(struct Pngsource *is)
{
   struct Decoder decoderdata,*decoder=&decoderdata;
   struct Task *task=FindTask(NULL);

   memset(&decoderdata,0,sizeof(decoderdata));
   decoder->source=is;
   if(decoder->source->flags&PNGSF_LOWPRI)
   {  SetTaskPri(task,task->tc_Node.ln_Pri-1);
   }

   if(!Parsepngimage(decoder)) goto err;
   return;

err:
   Updatetaskattrs(AOPNG_Error,TRUE,TAG_END);
}

/*--------------------------------------------------------------------*/
/* Main task functions                                                */
/*--------------------------------------------------------------------*/

/* Save all our source data blocks in this AOTP_FILE object */
static void Savesource(struct Pngsource *ps,struct Aobject *file)
{  struct Datablock *db;
   for(db=ps->data.first;db->next;db=db->next)
   {  Asetattrs(file,
         AOFIL_Data,db->data,
         AOFIL_Datalength,db->length,
         TAG_END);
   }
}

/* Start the decoder task. Only start it if the screen is
 * currenty valid. */
static void Startdecoder(struct Pngsource *ps)
{  struct Screen *screen=NULL;
   if(Agetattr(Aweb(),AOAPP_Screenvalid))
   {  Agetattrs(Aweb(),
         AOAPP_Screen,&screen,
         AOAPP_Colormap,&ps->colormap,
         TAG_END);
      if(screen) ps->friendbitmap=screen->RastPort.BitMap;
      if(ps->task=Anewobject(AOTP_TASK,
         AOTSK_Entry,Decodetask,
         AOTSK_Name,"AWebPng decoder",
         AOTSK_Userdata,ps,
         AOTSK_Stacksize,128000,
         AOBJ_Target,ps,
         TAG_END))
      {  Asetattrs(ps->task,AOTSK_Start,TRUE,TAG_END);
      }
   }
}

/* Notify our related Pngcopy objects that the bitmap has gone.
 * Delete the bitmap, and release all obtained pens. */
static void Releaseimage(struct Pngsource *ps)
{  short i;


   Anotifyset(ps->source,AOPNG_Bitmap,NULL,TAG_END);
   if(ps->bitmap)
   {  FreeBitMap(ps->bitmap);
      ps->bitmap=NULL;
   }
   if(ps->mask)
   {  FreeVec(ps->mask);
      ps->mask=NULL;
   }
   for(i=0;i<256;i++)
   {  if(ps->allocated[i])
      {  ReleasePen(ps->colormap,i);
         ps->allocated[i]=FALSE;
      }
      ps->mapped[i] = FALSE;
   }


   ps->colormap=NULL;
   ps->flags&=~PNGSF_READY;
   ps->memory=0;
   Asetattrs(ps->source,AOSRC_Memory,0,TAG_END);
}

/* Delete all source data */
static void Releasedata(struct Pngsource *ps)
{  struct Datablock *db;
   while(db=REMHEAD(&ps->data))
   {  if(db->data) FreeVec(db->data);
      FreeVec(db);
   }
}

/*--------------------------------------------------------------------*/
/* Plugin sourcedriver object dispatcher functions                    */
/*--------------------------------------------------------------------*/

static ULONG Setsource(struct Pngsource *ps,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   UBYTE *arg,*p;
   Amethodas(AOTP_SOURCEDRIVER,(struct Aobject *)ps,AOM_SET,amset->tags);
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            ps->source=(struct Aobject *)tag->ti_Data;
            break;
         case AOSDV_Savesource:
            Savesource(ps,(struct Aobject *)tag->ti_Data);
            break;
         case AOSDV_Displayed:
            if(tag->ti_Data)
            {  /* We are becoming displayed. Start our decoder task if
                * we have data but no bitmap yet, and task is not yet
                * running. */
               ps->flags|=PNGSF_DISPLAYED;
               if(ps->data.first->next && !ps->bitmap && !ps->task)
               {  Startdecoder(ps);
               }
            }
            else
            {  /* We are not displayed anywhere now. Leave the bitmap
                * intact so we don't need to decode it again when we
                * are becoming displayed again. */
               ps->flags&=~PNGSF_DISPLAYED;
            }
            break;
         case AOAPP_Screenvalid:
            if(tag->ti_Data)
            {  if(ps->data.first->next && (ps->flags&PNGSF_DISPLAYED)
               && !ps->task)
               {  Startdecoder(ps);
               }
            }
            else
            {  if(ps->task)
               {  Adisposeobject(ps->task);
                  ps->task=NULL;
               }
               Releaseimage(ps);
            }
            break;
         case AOSDV_Arguments:
            arg=(UBYTE *)tag->ti_Data;
            if(arg)
            {  for(p=arg;*p;p++)
               {
                  /* If multipass is disabled also disable progressive */
                  /* Otherwise there is no benefit */

                  if(!strnicmp(p,"PROGRESS=",9))
                  {
                     if((ps->flags | PNGSF_MULTI))
                     {
                          ps->progress=atoi(p+9);
                     }
                     else
                     {
                        ps->progress = 0;
                     }
                     p+=9;
                  }
                  else if(!strnicmp(p,"GAMMA=",6))
                  {  ps->gamma=atof(p+6);
                     p+=6;
                  }
                  else if(!strnicmp(p,"MULTIPASS=",10))
                  {
                      if(atoi(p+10))
                      {
                        ps->flags |= PNGSF_MULTI;
                      }
                      else
                      {
                        ps->flags &=~PNGSF_MULTI;
                      }
                      p+=10;
                  }
                  else if(!strnicmp(p,"LOWPRI",6))
                  {  ps->flags|=PNGSF_LOWPRI;
                     p+=6;
                  }
               }
            }
            break;
      }
   }
   return 0;
}

static struct Pngsource *Newsource(struct Amset *amset)
{  struct Pngsource *ps;
   if(ps=Allocobject(AwebPluginBase->sourcedriver,sizeof(struct Pngsource),amset))
   {  InitSemaphore(&ps->sema);
      NEWLIST(&ps->data);
      Aaddchild(Aweb(),(struct Aobject *)ps,AOREL_APP_USE_SCREEN);
      ps->progress=4;
      ps->gamma=2.0;
      ps->flags |= PNGSF_MULTI;
      Setsource(ps,amset);
   }
   return ps;
}

static ULONG Getsource(struct Pngsource *ps,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   AmethodasA(AOTP_SOURCEDRIVER,(struct Aobject *)ps,(struct Amessage *)amset);
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOSDV_Source:
            PUTATTR(tag,ps->source);
            break;
         case AOSDV_Saveable:
            PUTATTR(tag,(ps->flags&PNGSF_EOF)?TRUE:FALSE);
            break;
      }
   }
   return 0;
}

static ULONG Srcupdatesource(struct Pngsource *ps,struct Amsrcupdate *amsrcupdate)
{  struct TagItem *tag,*tstate;
   UBYTE *data=NULL;
   long datalength=0;
   BOOL eof=FALSE;
   AmethodasA(AOTP_SOURCEDRIVER,(struct Aobject *)ps,(struct Amessage *)amsrcupdate);
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
               ps->flags|=PNGSF_EOF;
            }
            break;
      }
   }
   if(data && datalength)
   {  struct Datablock *db;
      if(db=AllocVec(sizeof(struct Datablock),MEMF_ANY|MEMF_CLEAR))
      {  if(db->data=AllocVec(datalength,MEMF_ANY))
         {  memmove(db->data,data,datalength);
            db->length=datalength;
            ObtainSemaphore(&ps->sema);
            ADDTAIL(&ps->data,db);
            ReleaseSemaphore(&ps->sema);
         }
         else
         {  FreeVec(db);
         }
      }
      if(!ps->task)
      {  Startdecoder(ps);
      }
   }
   if((data && datalength) || eof)
   {  if(ps->task)
      {  Asetattrsasync(ps->task,AOPNG_Data,TRUE,TAG_END);
      }
   }
   return 0;
}

static ULONG Updatesource(struct Pngsource *ps,struct Amset *amset)
{  struct TagItem *tag,*tstate;
   BOOL notify=FALSE;
   long readyfrom=0,readyto=-1;
   tstate=amset->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOPNG_Readyfrom:
            readyfrom=tag->ti_Data;
            notify=TRUE;
            break;
         case AOPNG_Readyto:
            readyto=tag->ti_Data;
            if(readyto>ps->ready) ps->ready=readyto;
            notify=TRUE;
            break;
         case AOPNG_Imgready:
            if(tag->ti_Data) ps->flags|=PNGSF_READY;
            else ps->flags&=~PNGSF_READY;
            break;
         case AOPNG_Error:
            break;
         case AOPNG_Memory:
            ps->memory+=tag->ti_Data;
            Asetattrs(ps->source,
               AOSRC_Memory,ps->width*ps->height,
               TAG_END);
      }
   }
   if(notify && ps->bitmap)
   {  Anotifyset(ps->source,
         AOPNG_Bitmap,ps->bitmap,
         AOPNG_Mask,ps->mask,
         AOPNG_Alpha, ps->alpha,
         AOPNG_Width,ps->width,
         AOPNG_Height,ps->height,
         AOPNG_Readyfrom,readyfrom,
         AOPNG_Readyto,readyto,
         AOPNG_Imgready,ps->flags&PNGSF_READY,
         TAG_END);
   }
   return 0;
}

static ULONG Addchildsource(struct Pngsource *ps,struct Amadd *amadd)
{
   if(amadd->relation==AOREL_SRC_COPY)
   {  if(ps->bitmap)
      {  Asetattrs(amadd->child,
            AOPNG_Bitmap,ps->bitmap,
            AOPNG_Mask,ps->mask,
            AOPNG_Width,ps->width,
            AOPNG_Height,ps->height,
            AOPNG_Readyfrom,0,
            AOPNG_Readyto,ps->ready,
            AOPNG_Imgready,ps->flags&PNGSF_READY,
            AOPNG_Alpha, ps->alpha,
            TAG_END);
      }
   }
   return 0;
}

static void Disposesource(struct Pngsource *ps)
{
   if(ps->task)
   {  Adisposeobject(ps->task);
   }
   Releaseimage(ps);
   Releasedata(ps);
   Aremchild(Aweb(),(struct Aobject *)ps,AOREL_APP_USE_SCREEN);
   Amethodas(AOTP_SOURCEDRIVER,(struct Aobject *)ps,AOM_DISPOSE);
}

USRFUNC_H2
(
__saveds  ULONG  , Dispatchersource,
struct Pngsource *,ps,A0,
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
         result=Setsource(ps,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getsource(ps,(struct Amset *)amsg);
         break;
      case AOM_SRCUPDATE:
         result=Srcupdatesource(ps,(struct Amsrcupdate *)amsg);
         break;
      case AOM_UPDATE:
         result=Updatesource(ps,(struct Amset *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchildsource(ps,(struct Amadd *)amsg);
         break;
      case AOM_DISPOSE:
         Disposesource(ps);
         break;
      default:
         result=AmethodasA(AOTP_SOURCEDRIVER,(struct Aobject *)ps,amsg);
         break;
   }
   return result;

    USRFUNC_EXIT
}
