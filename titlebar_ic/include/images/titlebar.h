
#ifndef IMAGES_TITLEBAR_H
#define IMAGES_TITLEBAR_H

/* $VER: titlebar.h 40.18 (5.5.2021) */

/************************************************************/
/* Public definitions for the "titlebar image" BOOPSI class */
/************************************************************/

/* This macro can be used to compute the correct position for a gadget   */
/* to be placed into the titlebar. "tbi" is a pointer to a "tbiclass"    */
/* instance and "num" is the number of gadgets (zoom, depth...) that     */
/* will be at the right side of the new gadget. For instance, if your    */
/* window has both a zoom gadget and a depth gadget, you can compute     */
/* the position of a new titlebar gadget with TBI_RELPOS(tbi,2).         */
/* If there's instead only a depth gadget, you'll use TBI_RELPOS(tbi,1). */
/*                                                                       */
/* Note: the new gadget MUST have the GFLG_RELRIGHT flag set.            */
/*                                                                       */
/* Also note: don't use this macro under AmigaOS 4.x! Instead, pass the  */
/* gadget { GA_Titlebar, TRUE } or set GTYP_TBARGADGET in GadgetType.    */

#define TBI_RELPOS(tbi,num) (1 - ((1 + (num)) * \
                             (((struct Image *)tbi)->Width - 1)))

/* Attributes defined by the "tbiclass" image class */

#define TBIA_Dummy       (TAG_USER + 0x0B0000)
#define TBIA_ContentsBox (TBIA_Dummy + 0x0001)  /* Get inner size (V40.12) */
#define TBIA_FullFrame   (TBIA_Dummy + 0x0002)  /* For VP & OS4.x (V40.17) */

/* Types of titlebar gadget images available */

#define TBI_POPUPIMAGE    (101)
#define TBI_MUIIMAGE      (102)
#define TBI_SNAPSHOTIMAGE (103)
#define TBI_ICONIFYIMAGE  (104)
#define TBI_PADLOCKIMAGE  (105)
#define TBI_TBFRAMEIMAGE  (106)

/* Don't use these obsolete definitions under Intuition V47 or higher! */

#ifndef ICONIFYIMAGE
#define POPUPIMAGE    TBI_POPUPIMAGE
#define MUIIMAGE      TBI_MUIIMAGE
#define SNAPSHOTIMAGE TBI_SNAPSHOTIMAGE
#define ICONIFYIMAGE  TBI_ICONIFYIMAGE
#define PADLOCKIMAGE  TBI_PADLOCKIMAGE
#define TBFRAMEIMAGE  TBI_TBFRAMEIMAGE
#endif

#endif /* IMAGES_TITLEBAR_H */

