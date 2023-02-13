/* jpeg.c */

#include <exec/types.h>
#include <stdio.h>
#include "jpeglib.h"

#include <dos/dos.h>
#include <intuition/intuition.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <string.h>
#include <setjmp.h>

void *IntuitionBase,*GfxBase;

struct my_error_mgr
{  struct jpeg_error_mgr pub;
   jmp_buf setjmpbuf;
};

METHODDEF void my_error_exit(j_common_ptr cinfo)
{  struct my_error_mgr *myerr=(struct my_error_mgr *)cinfo->err;
   longjmp(myerr->setjmpbuf,1);
}

#define RGB(x) (((x)<<24)|((x)<<16)|((x)<<8)|(x))

void main(int argc,char **argv)
{  FILE *f;
   struct jpeg_decompress_struct cinfo;
   struct my_error_mgr jerr;
   JSAMPARRAY buffer;
   long rowsize;
   short i;
   struct Window *win;
   BOOL done=FALSE,abort=FALSE;
   struct IntuiMessage *msg;
   UBYTE allocated[256];
   struct ColorMap *cmap;

   memset(allocated,0,sizeof(allocated));
   if((IntuitionBase=OpenLibrary("intuition.library",0))
   && (GfxBase=OpenLibrary("graphics.library",0)))
   {
      if(argc>1)
      {  if(f=fopen(argv[1],"r"))
         {  if(win=OpenWindowTags(NULL,
               WA_IDCMP,IDCMP_CLOSEWINDOW|IDCMP_VANILLAKEY,
               WA_PubScreen,NULL,
               WA_CloseGadget,TRUE,
               WA_DepthGadget,TRUE,
               WA_DragBar,TRUE,
               WA_Activate,TRUE,
               WA_GimmeZeroZero,TRUE,
               WA_Width,400,
               WA_Height,400,
               TAG_END))
            {  cmap=win->WScreen->ViewPort.ColorMap;

               cinfo.err=jpeg_std_error(&jerr);
               jerr.pub.error_exit=my_error_exit;
               if(!setjmp(jerr.setjmpbuf))
               {  jpeg_create_decompress(&cinfo);
                  jpeg_stdio_src(&cinfo,f);
                  jpeg_read_header(&cinfo,TRUE);
                  printf("Image %dx%d, %d components colorspace %d (1=gray 2=rgb 3=ycbcr 4=cmyk 5=ycck)\n",
                     cinfo.image_width,cinfo.image_height,cinfo.num_components,
                     cinfo.jpeg_color_space);
                  printf("Output color space=%d\n",cinfo.out_color_space);

                  cinfo.quantize_colors=TRUE;
                  cinfo.desired_number_of_colors=16;
                  cinfo.dither_mode=JDITHER_ORDERED;

                  jpeg_start_decompress(&cinfo);

                  rowsize=cinfo.output_width*cinfo.output_components;
                  buffer=cinfo.mem->alloc_sarray((j_common_ptr)&cinfo,JPOOL_IMAGE,rowsize,1);

                  while(cinfo.output_scanline<cinfo.output_height)
                  {  jpeg_read_scanlines(&cinfo,buffer,1);

/*
                     for(i=0;i<cinfo.output_width;i++)
                     {  UBYTE r,g,b;
                        short pen;
                        r=buffer[0][3*i];
                        g=buffer[0][3*i+1];
                        b=buffer[0][3*i+2];
                        pen=ObtainBestPen(cmap,RGB(r),RGB(g),RGB(b),TAG_END);
                        if(allocated[pen]) ReleasePen(cmap,pen);
                        else allocated[pen]=TRUE;
                        SetAPen(win->RPort,pen);
                        WritePixel(win->RPort,i,cinfo.output_scanline-1);
                     }
*/
                     for(i=0;i<cinfo.output_width;i++)
                     {  UBYTE r,g,b;
                        short pen;
                        pen=buffer[0][i];
                        r=cinfo.colormap[0][pen];
                        g=cinfo.colormap[1][pen];
                        b=cinfo.colormap[2][pen];
                        pen=ObtainBestPen(cmap,RGB(r),RGB(g),RGB(b),TAG_END);
                        if(allocated[pen]) ReleasePen(cmap,pen);
                        else allocated[pen]=TRUE;
                        SetAPen(win->RPort,pen);
                        WritePixel(win->RPort,i,cinfo.output_scanline-1);
                     }

                     if(SetSignal(0,0)&SIGBREAKF_CTRL_C)
                     {  abort=TRUE;
                        break;
                     }
                  }

                  if(!abort) jpeg_finish_decompress(&cinfo);

                  jpeg_destroy_decompress(&cinfo);

                  while(!done)
                  {  while(!(msg=(struct IntuiMessage *)GetMsg(win->UserPort)))
                     {  WaitPort(win->UserPort);
                     }
                     switch(msg->Class)
                     {  case IDCMP_CLOSEWINDOW:
                           done=TRUE;
                           break;
                        case IDCMP_VANILLAKEY:
                           if(msg->Code==27) done=TRUE;
                           break;
                     }
                     ReplyMsg(msg);
                  }
               }
               else
               {  cinfo.err->output_message(&cinfo);
               }
               CloseWindow(win);
               for(i=0;i<256;i++)
               {  if(allocated[i]) ReleasePen(cmap,i);
               }

            }
            fclose(f);
         }
         else printf("Can't open %s\n",argv[1]);
      }
   }
   if(IntuitionBase) CloseLibrary(IntuitionBase);
   if(GfxBase) CloseLibrary(GfxBase);
}
