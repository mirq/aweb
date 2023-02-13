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

/* print.h - AWeb print process and print.aweblib definitions */

#ifndef AWEB_PRINT_H
#define AWEB_PRINT_H

#include "object.h"

struct Print
{  struct Aobject object;
   void *winobj;           /* ClassAct parameter window object */
   struct Window *window;  /* Its window */
   struct Gadget *bggad,*scalegad,*ffgad,*centergad,*layoutgad;
   struct List layoutlist;
   ULONG screenmode;
   void *prwin;            /* Off-screen print window object */
   void *whis;
   long width,height;      /* Current window dimensions */
   long docwidth;          /* Current document width */
   long scrwidth;          /* Current screen width */
   long printwidth;        /* Width to print against */
   long totalheight;       /* Total height to print */
   long top;               /* Current top position */
   struct RastPort *rp;
   struct ColorMap *cmap;
   struct MsgPort *ioport;
   struct IODRPReq *ioreq;
   short scale;
   long printheight;       /* nominal nr of rows to print in 1 go (including 2 overlap) */
   long numstrips;         /* number of strips to print before FF, or zero */
   long numprinted;        /* number of strips printed on this page */
   void *progressreq;
   struct Arexxcmd *wait;  /* Reply this when finished */
   UWORD flags;
   long debugfile;
};

#define PRTF_PRINTING      0x0001   /* Busy printing, otherwise params requester */
#define PRTF_FORMFEED      0x0002   /* Formfeed requested */
#define PRTF_CENTER        0x0004   /* Centering requested */
#define PRTF_BACKGROUND    0x0008   /* Print backgrounds */
#define PRTF_NOOPTIONS     0x0010   /* Skip the options window */
#define PRTF_DEBUG         0x0100   /* Create debugging file */

/* Maximum height for print window */
#define PRINTWINH    128

#include <proto/awebprint.h>

#endif
