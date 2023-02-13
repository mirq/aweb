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

/* map.c - AWeb html map descriptor object */

#include "aweb.h"
#include "map.h"
#include "area.h"
#include "document.h"
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

struct Map
{  struct Aobject object;
   void *pool;
   UBYTE *name;
   LIST(Arearef) areas;
   void *defarea;          /* Default AREA, not in list */
   void *extcopy;          /* COPY object for external definition */
   UBYTE *extname;         /* Name of external definition */
};

struct Arearef             /* A hot zone */
{  NODE(Arearef);
   void *area;             /* An AREA object */
};

/*------------------------------------------------------------------------*/

/* Add a new area */
static void Addarea(struct Map *map,void *area)
{  struct Arearef *aref;
   if(Agetattr(area,AOARA_Shape)==AREASHAPE_DEFAULT)
   {  if(!map->defarea) map->defarea=area;
   }
   else
   {  if(aref=PALLOCSTRUCT(Arearef,1,MEMF_CLEAR,map->pool))
      {  aref->area=area;
         ADDTAIL(&map->areas,aref);
      }
   }
}

/*------------------------------------------------------------------------*/

/* Find area related to this point in the map */
static void *Findinmap(struct Map *map,long x,long y)
{  struct Arearef *aref;
   void *area=NULL;
   for(aref=map->areas.first;aref->next;aref=aref->next)
   {  if(Areahit(aref->area,x,y))
      {  area=aref->area;
         break;
      }
   }
   if(!area)   /* not found */
   {  area=map->defarea;
   }
   return area;
}

/*------------------------------------------------------------------------*/

static long Setmap(struct Map *map,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   void *newwin=NULL;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOBJ_Pool:
            map->pool=(void *)tag->ti_Data;
            break;
         case AOBJ_Window:
            if(map->extcopy) Asetattrs(map->extcopy,AOBJ_Window,tag->ti_Data,TAG_END);
            else newwin=(void *)tag->ti_Data;
            break;
         case AOMAP_Name:
            if(map->name) FREE(map->name);
            if(tag->ti_Data)
            {  map->name=Dupstr((UBYTE *)tag->ti_Data,-1);
            }
            else
            {  map->name=NULL;
            }
            break;
         case AOMAP_Area:
            Addarea(map,(void *)tag->ti_Data);
            break;
         case AOMAP_Extcopy:
            if(map->extcopy) Adisposeobject(map->extcopy);
            map->extcopy=(void *)tag->ti_Data;
            if(newwin) Asetattrs(map->extcopy,AOBJ_Window,(Tag)newwin,TAG_END);
            break;
         case AOMAP_Extname:
            if(map->extname) FREE(map->extname);
            map->extname=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
      }
   }
   return 0;
}

static struct Map *Newmap(struct Amset *ams)
{  struct Map *map;
   if(map=Allocobject(AOTP_MAP,sizeof(struct Map),ams))
   {  NEWLIST(&map->areas);
      Setmap(map,ams);
   }
   return map;
}

static long Getmap(struct Map *map,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   long xco=-1,yco=-1;
   void **areap=NULL;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOMAP_Xco:
            xco=tag->ti_Data;
            break;
         case AOMAP_Yco:
            yco=tag->ti_Data;
            break;
         case AOMAP_Area:
            areap=(void **)tag->ti_Data;
            break;
         case AOMAP_Name:
            PUTATTR(tag,map->name);
            break;
      }
   }
   if(areap)
   {  if(map->extcopy && map->extname)
      {  Agetattrs(map->extcopy,
            AODOC_Mapname,(Tag)map->extname,
            TAG_MORE,(Tag)ams->tags);
      }
      else
      {  *areap=Findinmap(map,xco,yco);
      }
   }
   return 0;
}

static void Disposemap(struct Map *map)
{  void *p;
   if(map->name) FREE(map->name);
   while(p=REMHEAD(&map->areas)) FREE(p);
   if(map->extcopy) Adisposeobject(map->extcopy);
   if(map->extname) FREE(map->extname);
   Amethodas(AOTP_OBJECT,map,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Map_Dispatcher,
struct Map *,map,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newmap((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setmap(map,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getmap(map,(struct Amset *)amsg);
         break;
      case AOM_DISPOSE:
         Disposemap(map);
         break;
      case AOM_DEINSTALL:
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installmap(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_MAP,(Tag)Map_Dispatcher)) return FALSE;
   return TRUE;
}
