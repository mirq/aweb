

/*  Titlebar.image test (5.5.2021)  */
/*  Written for SAS/C               */
/*  Compile: SC LINK TBITest        */
/*  © 1998-2021 Massimo Tantignone  */
/*  e-mail: tanti@intercom.it       */


#include <dos/dos.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <images/titlebar.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/titlebarimage.h>


/* The library base for the "titlebar.image" class library */

struct Library *TitlebarImageBase;


ULONG main(void)
{
   /* The usual stuff */

   struct Screen *scr;
   struct Window *win;
   struct IntuiMessage *imsg;
   struct DrawInfo *dri;
   ULONG class, code;
   ULONG width = 640, height = 200;
   BOOL done = FALSE;
   Object *tbim, *tbgad = NULL;

   /* Let's try opening the "titlebar.image" class library any way we can */

   TitlebarImageBase = OpenLibrary("titlebar.image",40L);

   if (!TitlebarImageBase)
      TitlebarImageBase = OpenLibrary("Images/titlebar.image",40L);

   if (!TitlebarImageBase)
      TitlebarImageBase = OpenLibrary("Classes/Images/titlebar.image",40L);

   /* Really not found? Then quit (and complain a bit) */

   if (!TitlebarImageBase)
      return (RETURN_FAIL);

   /* Lock the current default public screen */

   if (scr = LockPubScreen(NULL))
   {
      /* Inquire about the real screen size */

      width = scr->Width;
      height = scr->Height;

      /* Get the screen's DrawInfo, it will be useful... */

      if (dri = GetScreenDrawInfo(scr))
      {
         /* Create a TBI_ICONIFYIMAGE instance of the "tbiclass" image class */

         tbim = NewObject(NULL,"tbiclass",SYSIA_Which,TBI_ICONIFYIMAGE,
                                          SYSIA_DrawInfo,dri,
                                          TAG_END);

         if (tbim)
         {
            /* Attempt to create an iconify gadget for the title bar;  */
            /* of course it will use our new "tbiclass" iconify image. */

            /* Here we pass 2 to TBI_RELPOS() because there will be 2  */
            /* gadgets at the right side of our new gadget in the      */
            /* window title bar: the zoom gadget and the depth gadget. */
            /* If we added another gadget to the left of this one,     */
            /* we would pass 3 for it, then 4 for a third, and so on.  */

            tbgad = NewObject(NULL,"buttongclass",
                              GA_RelRight,TBI_RELPOS(tbim,2),
                              GA_Top,0,
                              GA_Width,((struct Image *)tbim)->Width - 1,
                              GA_Height,((struct Image *)tbim)->Height,
                              GA_TopBorder,TRUE,
                              GA_Image,tbim,
                              GA_RelVerify,TRUE,
                              TAG_END);
         }

         /* Open a window on the default public screen */

         if (win = OpenWindowTags(NULL,WA_Left,(width - 400) / 2,
                                       WA_Top,(height - 250) / 2,
                                       WA_Width,400,
                                       WA_Height,250,
                                       WA_CloseGadget,TRUE,
                                       WA_Title,"titlebar.image test",
                                       WA_DragBar,TRUE,
                                       WA_DepthGadget,TRUE,
                                       WA_SizeGadget,TRUE,
                                       WA_Gadgets,tbgad,  /* May be NULL */
                                       WA_Activate,TRUE,
                                       WA_SmartRefresh,TRUE,
                                       WA_IDCMP,IDCMP_CLOSEWINDOW |
                                                IDCMP_REFRESHWINDOW |
                                                IDCMP_GADGETUP,
                                       TAG_END))
         {
            ULONG x, y, t = TBI_POPUPIMAGE;
            struct Image *im;

            /* Show the various image types */

            for (x = 0; x < 3; x++)
            {
               for (y = 0; y < 2; y++)
               {
                  if (im = (struct Image *)NewObject(NULL,"tbiclass",
                                                     SYSIA_Which,t++,
                                                     SYSIA_DrawInfo,dri,
                                                     TAG_END))
                  {
                     DrawImageState(win->RPort,im,
                                    50 + x * 100,50 + y * 100,
                                    IDS_NORMAL,dri);

                     DrawImageState(win->RPort,im,
                                    90 + x * 100,50 + y * 100,
                                    IDS_SELECTED,dri);

                     DrawImageState(win->RPort,im,
                                    50 + x * 100,90 + y * 100,
                                    IDS_INACTIVENORMAL,dri);

                     DrawImageState(win->RPort,im,
                                    90 + x * 100,90 + y * 100,
                                    IDS_INACTIVESELECTED,dri);

                     WaitBlit();

                     DisposeObject(im);
                  }
               }
            }

            /* Put our extra gadget to the front so it can be used */

            /* We passed it also at window opening so that patches */
            /* centering window titles could compute the correct   */
            /* title position right from the start.                */

            if (tbgad)
            {
               RemoveGadget(win,(struct Gadget *)tbgad);
               AddGadget(win,(struct Gadget *)tbgad,0);
            }

            /* Let's handle the events until the window gets closed */

            while (!done)
            {
               Wait(1 << win->UserPort->mp_SigBit);

               while (imsg = (struct IntuiMessage *)GetMsg(win->UserPort))
               {
                  class = imsg->Class;
                  code = imsg->Code;
                  ReplyMsg((struct Message *)imsg);

                  if (class == IDCMP_CLOSEWINDOW)
                  {
                     done = TRUE;
                  }
                  else if (class == IDCMP_GADGETUP)
                  {
                     /* This was a message from our title bar gadget; */
                     /* let's do something to show we received it.    */

                     DisplayBeep(NULL);
                  }
                  else if (class == IDCMP_REFRESHWINDOW)
                  {
                     BeginRefresh(win);
                     EndRefresh(win,TRUE);
                  }
               }
            }

            /* Say good-bye to the window... */

            CloseWindow(win);
         }

         /* ... to the gadget... */

         DisposeObject(tbgad);

         /* ... and to the image */

         DisposeObject(tbim);

         /* Release the DrawInfo structure */

         FreeScreenDrawInfo(scr,dri);
      }

      /* Finally, unlock the screen */

      UnlockPubScreen(NULL,scr);
   }

   /* Close the class library */

   CloseLibrary(TitlebarImageBase);

   /* We did our job, now let's go home :-) */

   return (RETURN_OK);
}


