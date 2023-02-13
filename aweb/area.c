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

/* area.c - AWeb html area object */

#include "aweb.h"
#include "area.h"
#include "element.h"
#include "frame.h"
#include "popup.h"
#include "url.h"
#include "window.h"
#include "jslib.h"
#include <proto/graphics.h>
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

/* Parse the coordinates string */
static void Setcoords(struct Area *area,UWORD shape,UBYTE *coords)
{  long nrcoords,i,n;
   UBYTE *p;
   BOOL ok=FALSE;
   for(p=coords,nrcoords=0;p && *p;)
   {  while(*p && !isdigit(*p)) p++;
      if(*p)
      {  nrcoords++;
         while(*p && isdigit(*p)) p++;
      }
   }
   switch(shape)
   {  case AREASHAPE_DEFAULT:
         ok=TRUE;
         nrcoords=0;
         break;
      case AREASHAPE_RECTANGLE:
         ok=(nrcoords>=4);
         nrcoords=4;
         break;
      case AREASHAPE_CIRCLE:
         ok=(nrcoords>=3);
         nrcoords=3;
         break;
      case AREASHAPE_POLYGON:
         ok=(nrcoords>=6);
         nrcoords&=~1;  /* make even */
         break;
   }
   if(ok)
   {  if(area->coords)
      {  FREE(area->coords);
         area->coords=NULL;
      }
      area->shape=shape;
      area->nrcoords=nrcoords;
      if(nrcoords)
      {  if(area->coords=PALLOCTYPE(long,nrcoords,MEMF_CLEAR,area->link.pool))
         {  for(i=0,p=coords;i<nrcoords;i++)
            {  n=0;
               while(!isdigit(*p)) p++;
               while(isdigit(*p))
               {  n=10*n+(*p-'0');
                  p++;
               }
               area->coords[i]=n;
            }
         }
      }
   }
}

/*------------------------------------------------------------------------*/

static long Setarea(struct Area *area,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   UWORD newshape=0;
   UBYTE *newcoords=NULL;
   result=Amethodas(AOTP_LINK,area,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOARA_Shape:
            newshape=tag->ti_Data;
            break;
         case AOARA_Coords:
            newcoords=(UBYTE *)tag->ti_Data;
            break;
         case AOARA_Textpos:
            area->textpos=tag->ti_Data;
            break;
         case AOARA_Textlength:
            area->textlen=tag->ti_Data;
            break;
      }
   }
   if(newshape && newcoords)
   {  Setcoords(area,newshape,newcoords);
   }
   return result;
}

static void Disposearea(struct Area *area)
{  if(area->coords) FREE(area->coords);
   if(area->tooltip) FREE(area->tooltip);
   Amethodas(AOTP_LINK,area,AOM_DISPOSE);
}

static struct Area *Newarea(struct Amset *ams)
{  struct Area *area;
   if(area=Allocobject(AOTP_AREA,sizeof(struct Area),ams))
   {  NEWLIST(&area->link.components);
      Setarea(area,ams);
   }
   return area;
}

static long Getarea(struct Area *area,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=AmethodasA(AOTP_LINK,(struct Aobject *)area,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOARA_Shape:
            PUTATTR(tag,area->shape);
            break;
         case AOARA_Tooltip:
            if(area->link.text && area->textpos>0 && area->textlen>0 && !area->tooltip)
            {  area->tooltip=Dupstr(area->link.text->buffer+area->textpos,area->textlen);
            }
            PUTATTR(tag,area->tooltip);
            break;
      }
   }
   return result;
}

/*
static long Hittestarea(struct Area *area,struct Amhittest *amh)
{  long result;
   result=AmethodasA(AOTP_LINK,area,amh);
   if((result&AMHR_NEWHIT) && amh->amhr && amh->amhr->object==area
   && area->text && area->textpos>0 && area->textlen>0)
   {  amh->amhr->tooltip=Dupstr(area->text->buffer+area->textpos,area->textlen);
   }
   return result;
}
*/

USRFUNC_H2
(
static long  , Area_Dispatcher,
struct Area *,area,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newarea((struct Amset *)amsg);
         break;
      case AOM_SET:
      case AOM_UPDATE:
         result=Setarea(area,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getarea(area,(struct Amset *)amsg);
         break;
/*
      case AOM_HITTEST:
         result=Hittestarea(area,(struct Amhittest *)amsg);
         break;
*/
      case AOM_DISPOSE:
         Disposearea(area);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         result=AmethodasA(AOTP_LINK,(struct Aobject *)area,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installarea(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_AREA,(Tag)Area_Dispatcher)) return FALSE;
   return TRUE;
}

BOOL Areahit(struct Area *area,long x,long y)
{  BOOL found=FALSE;
   long *co=area->coords;
   long rx,ry;
   switch(area->shape)
   {  case AREASHAPE_RECTANGLE:
         found=(x>=co[0] && y>=co[1] && x<=co[2] && y<=co[3]);
         break;
      case AREASHAPE_CIRCLE:
         rx=x-co[0];
         ry=y-co[1];
         found=(rx*rx+ry*ry<=co[2]*co[2]);
         break;
      case AREASHAPE_POLYGON:
         {  long nint,f1,f2,a,b,c,d,det,i,j;
            nint=0;
            for(i=0;i<area->nrcoords;i+=2)   /* for all coordinates */
            {  j=i+2;                        /* next coordinate */
               if(j>=area->nrcoords) j=0;
               /* 0<f1<det and 0<f2<det if (x,y)-(-1,-1) intersects segment */
               a=-1-x;
               b=co[i]-co[j];
               c=-1-y;
               d=co[i+1]-co[j+1];
               det=a*d-b*c;
               f1= d*(co[i]-x)-b*(co[i+1]-y);
               f2=-c*(co[i]-x)+a*(co[i+1]-y);
               if(det<0)
               {  det=-det;
                  f1=-f1;
                  f2=-f2;
               }
               if(0<f1 && f1<det && 0<f2 && f2<det) nint++;
            }
            /* if nint odd, point is inside polygon */
            if(nint&1) found=TRUE;
         }
         break;
      case AREASHAPE_DEFAULT:
         found=TRUE;
         break;
   }
   return found;
}

BOOL Areaonclick(struct Area *area)
{  return Runjavascript(area->link.cframe,area->link.onclick,&area->link.jobject);
}
