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

/* hotmanager.c - AWeb hotlist manager task */

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include "aweblib.h"
#include "task.h"
#include "hotlist.h"
#include "hotlisttask.h"
#include "libraries/awebclib.h"

#include <intuition/intuition.h>
#include <reaction/reaction.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>

enum HOTLIST_GADGET_IDS
{  GID_GLIST=1,GID_GUP,GID_GDOWN,GID_GSORT,GID_GALL,GID_PARENT,GID_ADDE,GID_ADDG,
   GID_ID,GID_MVIN,GID_MVOUT,GID_MOVE,GID_DEL,GID_WHERE,GID_SHOW,
   GID_URLS,GID_REVERT,GID_SAVE,
   GID_LTYPE,GID_LSORT,GID_LLIST,GID_LADD,GID_FIND,GID_PATTERN,
   GID_NAME,GID_URL,GID_FSTR,GID_NEXT,GID_PSTR,
};

enum PAGE_NUMBERS
{  PG_EDIT=0,PG_FIND,PG_PATTERN,
};

static struct List glist,llist;
static struct List ltypelist,lsortlist;
static struct Gadget *toplayout;
static struct Gadget *gnamegad,*glistgad,*gupgad,*gdowngad,*gsortgad,*gparentgad,
   *gaddegad,*gaddggad;
static struct Gadget *idgad,*mvingad,*mvoutgad,*movegad,*delgad,*wheregad,*showgad,*urlsgad;
static struct Gadget *lnamegad,*ltypegad,*lsortgad,*llistgad,*findgad,*patgad;
static struct Gadget *pagelayout,*pagegad,*namegad,*urlgad,*fstrgad,*pstrgad;

static struct Image *leftarrow,*rightarrow;

static UBYTE *ltypelabels[4],*lsortlabels[4];

static BOOL locked=FALSE;

enum LIST_IDS { LIST_NONE,LIST_GROUPS,LIST_LIST };
static short listid;
static BOOL urls;

enum GLIST_TYPES { GTP_ALL,GTP_GROUP,GTP_WHERE };
static short gtype;
static long gselect;
static struct Hotitem *ggroup;

enum LLIST_TYPES { LTP_REST,LTP_ALL,LTP_GROUPS };
enum LLIST_SORTS { LSRT_DATE,LSRT_TITLE,LSRT_URL };
static short ltype,lsort;
static long lselect;
static BOOL wasfind;

static struct Screen *screen;
static struct DrawInfo *dri;
static void *winobj;
static struct Window *window;
static ULONG winsigmask;

static struct Hook grenderhook,lrenderhook,idcmphook;

static BOOL keycontrol,keyshift;

static struct Image *showimg,*hideimg;

static struct DrawList showdata[]=
{  { DLST_LINE,0,0,998,0,1 },
   { DLST_LINE,998,0,998,998,1 },
   { DLST_LINE,998,998,0,998,1 },
   { DLST_LINE,0,998,0,0,1 },
   { DLST_LINE,149,499,799,499,1 },
   { DLST_LINE,499,149,499,799,1 },
   { DLST_END },
};
static struct DrawList hidedata[]=
{  { DLST_LINE,0,0,998,0,1 },
   { DLST_LINE,998,0,998,998,1 },
   { DLST_LINE,998,998,0,998,1 },
   { DLST_LINE,0,998,0,0,1 },
   { DLST_LINE,149,499,799,499,1 },
   { DLST_END },
};

/*-----------------------------------------------------------------------*/

VARARGS68K_DECLARE(static void  Setgadgetattrs(struct Gadget *gad,struct Window *win,struct Requester *req,...))
{
   VA_LIST va;
   struct TagItem *tags;
   VA_STARTLIN(va,req);
   tags = (struct TagItem *)VA_GETLIN(va,struct TagItem *);
   if(SetGadgetAttrsA(gad,win,req,tags) && win) RefreshGList(gad,win,req,1);
   VA_END(va);
}

VARARGS68K_DECLARE(static void Setpagegadgetattrs(struct Gadget *gad,void *page,struct Window *win,
   struct Requester *req,...))
{
   VA_LIST va;
   struct TagItem *tags;
   VA_STARTLIN(va,req);
   tags = (struct TagItem *)VA_GETLIN(va, struct TagItem *);
   if(SetPageGadgetAttrsA(gad,page,win,req,tags) && win) RefreshGList(gad,win,req,1);
   VA_END(va);
}

VARARGS68K_DECLARE(static void Editlbnode(struct Gadget *gad,struct Window *win,struct Requester *req,
   struct Node *node,...))
{
   VA_LIST va;
   struct TagItem * tags;
   VA_STARTLIN(va,node);
   tags = (struct TagItem *)VA_GETLIN(va, struct TagItem *);

   DoGadgetMethod(gad,win,req,LBM_EDITNODE,NULL,node,tags);
   VA_END(va);

}

static long Getvalue(void *gad,ULONG tag)
{  long value=0;
   GetAttr(tag,gad,(ULONG *)&value);
   return value;
}

static long Getlbnvalue(struct Node *node,ULONG tag)
{  long value=0;
   if(node) GetListBrowserNodeAttrs(node,tag,(ULONG *)&value,TAG_END);
   return value;
}

static struct Node *Getnode(struct List *list,long n)
{  struct Node *node=list->lh_Head;
   while(node->ln_Succ && n)
   {  node=node->ln_Succ;
      n--;
   }
   if(node->ln_Succ) return node;
   else return NULL;
}

static long Nodenumber(struct List *list,struct Node *node)
{  struct Node *n;
   long nr=0;
   for(n=list->lh_Head;n->ln_Succ;n=n->ln_Succ)
   {  if(n==node) return nr;
      nr++;
   }
   return -1;
}

static struct Node *Selectednode(struct List *list)
{  struct Node *node;
   for(node=list->lh_Head;node->ln_Succ;node=node->ln_Succ)
   {  if(Getlbnvalue(node,LBNA_Selected)) return node;
   }
   return NULL;
}

static long Selectednodenr(struct List *list)
{  struct Node *node;
   long nr=0;
   for(node=list->lh_Head;node->ln_Succ;node=node->ln_Succ)
   {  if(Getlbnvalue(node,LBNA_Selected)) return nr;
      nr++;
   }
   return -1;
}

static void Unselectall(struct List *list)
{  struct Node *node;
   for(node=list->lh_Head;node->ln_Succ;node=node->ln_Succ)
   {  SetListBrowserNodeAttrs(node,LBNA_Selected,FALSE,TAG_END);
   }
}

static void Makechooserlist(struct List *list,UBYTE **labels)
{  struct Node *node;
   short i;
   for(i=0;labels[i];i++)
   {  if(node=AllocChooserNode(
         CNA_Text,labels[i],
         TAG_END))
         AddTail(list,node);
   }
}

static void Freechooserlist(struct List *list)
{  struct Node *node;
   if(list->lh_Head)
   {  while(node=RemHead(list)) FreeChooserNode(node);
   }
}

#define Enablegad(g,e) Setgadgetattrs(g,window,NULL,GA_Disabled,!(e),TAG_END)
#define Enablepgad(g,e) Setgadgetattrs(g,pagegad,window,NULL,GA_Disabled,!(e),TAG_END)

static UBYTE *Strstri(UBYTE *str,UBYTE *sub)
{  long l=strlen(sub);
   UBYTE *end=str+strlen(str)-l;
   for(;str<=end;str++)
   {  if(STRNIEQUAL(str,sub,l)) return str;
   }
   return NULL;
}

/*-----------------------------------------------------------------------*/

/* Insert a Hotbase in the list */
static void Inserthotbase(LIST(Hotbase) *list,struct Hotbase *hb)
{  struct Hotbase *a;
   for(a=list->first;a->next;a=a->next)
   {  if(stricmp(a->title,hb->title)>0) break;
   }
   INSERT(list,hb,a->prev);
}

/* Find a duplicate Hotbase in the list, but don't create a new one */
static struct Hotbase *Findhotbase(LIST(Hotbase) *list,struct Hotbase *hb1)
{  struct Hotbase *hb;
   for(hb=list->first;hb->next;hb=hb->next)
   {  if(hb!=hb1 && STREQUAL(hb->title,hb1->title) && STREQUAL(hb->url,hb1->url)) return hb;
   }
   return NULL;
}

/* Create a new base, don't insert yet into list */
static struct Hotbase *Newhotbase(UBYTE *title,UBYTE *url)
{  struct Hotbase *hb;
   UBYTE *duptitle,*dupurl;
   if((duptitle=Dupstr(title,-1))
   && (dupurl=Dupstr(url,-1))
   && (hb=ALLOCSTRUCT(Hotbase,1,MEMF_PUBLIC|MEMF_CLEAR)))
   {  hb->nodetype=HNT_BASE;
      hb->title=duptitle;
      hb->url=dupurl;
      hb->date=Today();
      hb->lnode=AllocListBrowserNode(1,
         LBNA_UserData,hb,
         LBNCA_CopyText,TRUE,
         LBNCA_Text,hb->title,
         TAG_END);
   }
   else
   {  if(duptitle) FREE(duptitle);
      if(dupurl) FREE(dupurl);
      hb=NULL;
   }
   return hb;
}

/* Check if group is owner of hi */
static BOOL Isownerof(struct Hotitem *hi,struct Hotitem *group)
{  while(hi)
   {  if(hi==group) return TRUE;
      hi=hi->owner;
   }
   return FALSE;
}

/*-----------------------------------------------------------------------*/

/* Allocate nodes for every entity */
static void Allocitemnodes1(struct Hotitem *hi)
{  if(hi->type!=HITEM_ENTRY)
   {  hi->lnode=AllocListBrowserNode(1,
         LBNA_UserData,hi,
         LBNA_Flags,LBFLG_CUSTOMPENS,
         LBNCA_CopyText,TRUE,
         LBNCA_Text,hi->name,
         LBNCA_FGPen,dri->dri_Pens[HIGHLIGHTTEXTPEN],
         TAG_END);
   }
   hi->showchildren=LBFLG_SHOWCHILDREN;
   hi->gnode=AllocListBrowserNode(1,
      LBNA_UserData,hi,
      LBNA_Flags,hi->type==HITEM_ENTRY?0:LBFLG_CUSTOMPENS|hi->showchildren,
      LBNCA_CopyText,TRUE,
      LBNCA_Text,hi->type==HITEM_ENTRY?hi->base->title:hi->name,
      LBNCA_FGPen,dri->dri_Pens[HIGHLIGHTTEXTPEN],
      TAG_END);
}
static void Allocitemnodes(LIST(Hotitem) *list)
{  struct Hotitem *hi;
   for(hi=list->first;hi->next;hi=hi->next)
   {  Allocitemnodes1(hi);
      if(hi->type!=HITEM_ENTRY)
      {  Allocitemnodes(&hi->subitems);
      }
   }
}
static void Allocallnodes(struct Hotwindow *how)
{  struct Hotbase *hb;
   for(hb=how->hotbase->first;hb->next;hb=hb->next)
   {  hb->lnode=AllocListBrowserNode(1,
         LBNA_UserData,hb,
         LBNCA_CopyText,TRUE,
         LBNCA_Text,hb->title,
         TAG_END);
   }
   Allocitemnodes(how->hotlist);
}

/* Free nodes for every entity */
static void Freeitemnodes(LIST(Hotitem) *list)
{  struct Hotitem *hi;
   for(hi=list->first;hi->next;hi=hi->next)
   {  if(hi->lnode) FreeListBrowserNode(hi->lnode);
      if(hi->gnode) FreeListBrowserNode(hi->gnode);
      if(hi->type!=HITEM_ENTRY)
      {  Freeitemnodes(&hi->subitems);
      }
   }
}
static void Freeallnodes(struct Hotwindow *how)
{  struct Hotbase *hb;
   for(hb=how->hotbase->first;hb->next;hb=hb->next)
   {  if(hb->lnode) FreeListBrowserNode(hb->lnode);
   }
   Freeitemnodes(how->hotlist);
}

/* Delete this base */
static void Deletebase(struct Hotbase *hb)
{  REMOVE(hb);
   if(hb->title) FREE(hb->title);
   if(hb->url) FREE(hb->url);
   if(hb->lnode) FreeListBrowserNode(hb->lnode);
   FREE(hb);
}

/* Delete all items or referencing this base */
static void Deleteitems(LIST(Hotitem) *list,struct Hotbase *hb,BOOL basetoo)
{  struct Hotitem *hi;
   for(hi=list->first;hi->next;hi=hi->next)
   {  if(hi->type!=HITEM_ENTRY)
      {  Deleteitems(&hi->subitems,hb,basetoo);
      }
      if(!hb || hi->base==hb)
      {  REMOVE(hi);
         if(hi->name) FREE(hi->name);
         if(hi->lnode) FreeListBrowserNode(hi->lnode);
         if(hi->gnode) FreeListBrowserNode(hi->gnode);
         if(hi->base)
         {  hi->base->used--;
            if(basetoo && !hi->base->used)
            {  Deletebase(hi->base);
            }
         }
         FREE(hi);
      }
   }
}

/* Delete this item and its subitems */
static void Deleteitem(struct Hotitem *hi)
{  if(hi->type!=HITEM_ENTRY)
   {  Deleteitems(&hi->subitems,NULL,TRUE);
   }
   REMOVE(hi);
   if(hi->name) FREE(hi->name);
   if(hi->lnode) FreeListBrowserNode(hi->lnode);
   if(hi->gnode) FreeListBrowserNode(hi->gnode);
   if(hi->base)
   {  if(!--hi->base->used) Deletebase(hi->base);
   }
   FREE(hi);
}

/* Update all items to use new hotbase if nonnull, and update gnode text */
static void Updateitems(LIST(Hotitem) *list,struct Hotbase *hb,struct Hotbase *newhb)
{  struct Hotitem *hi;
   for(hi=list->first;hi->next;hi=hi->next)
   {  if(hi->type==HITEM_ENTRY)
      {  if(hi->gnode && hi->base==hb)
         {  if(newhb) hi->base=newhb;
            SetListBrowserNodeAttrs(hi->gnode,
               LBNCA_CopyText,TRUE,
               LBNCA_Text,hi->base->title,
               TAG_END);
         }
      }
      else
      {  Updateitems(&hi->subitems,hb,newhb);
      }
   }
}

/*-----------------------------------------------------------------------*/

/* Update gadgets as result of a list select */
static void Setgadgets(struct Hotwindow *how)
{  long gnum=Getvalue(glistgad,LISTBROWSER_NumSelected);
   long lnum=Getvalue(llistgad,LISTBROWSER_NumSelected);
   BOOL enable;
   UBYTE *name=NULL,*url=NULL;
   struct Hotitem *hi;
   struct Hotbase *hb;
   if(gtype==GTP_ALL) name=AWEBSTR(MSG_HOTL_TITLE_ALLGROUPS);
   else if(gtype==GTP_WHERE) name=AWEBSTR(MSG_HOTL_TITLE_WHERE);
   else if(ggroup) name=ggroup->name;
   Setgadgetattrs(gnamegad,window,NULL,
      GA_Text,name,
      BUTTON_BackgroundPen,(listid==LIST_GROUPS)?dri->dri_Pens[TEXTPEN]:~0,
      BUTTON_TextPen,(listid==LIST_GROUPS)?dri->dri_Pens[BACKGROUNDPEN]:~0,
      TAG_END);
   if(ltype==LTP_REST) name=AWEBSTR(MSG_HOTL_TITLE_REST);
   else if(ltype==LTP_ALL) name=AWEBSTR(MSG_HOTL_TITLE_ALL);
   else name=AWEBSTR(MSG_HOTL_TITLE_ROOTGROUPS);
   Setgadgetattrs(lnamegad,window,NULL,
      GA_Text,name,
      BUTTON_BackgroundPen,(listid==LIST_LIST)?dri->dri_Pens[TEXTPEN]:~0,
      BUTTON_TextPen,(listid==LIST_LIST)?dri->dri_Pens[BACKGROUNDPEN]:~0,
      TAG_END);
   Enablegad(mvingad,lnum>0 && gtype==GTP_GROUP);
   Enablegad(mvoutgad,gtype==GTP_GROUP && gnum>0);
   Enablegad(movegad,lnum>0 && gtype==GTP_GROUP);
   Enablegad(delgad,(listid==LIST_GROUPS && gnum>0) || (listid==LIST_LIST && lnum>0));
   enable=FALSE;
   if(lnum==1 && ltype!=LTP_GROUPS)
   {  hb=(struct Hotbase *)Getlbnvalue(Selectednode(&llist),LBNA_UserData);
      if(hb->used) enable=TRUE;
   }
   Enablegad(wheregad,enable);
   enable=FALSE;
   if(listid==LIST_GROUPS)
   {  if(gtype==GTP_GROUP && gnum==1)
      {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(Selectednode(&glist),LBNA_UserData);
         if(hi->type==HITEM_ENTRY) enable=TRUE;
      }
   }
   else
   {  if(ltype!=LTP_GROUPS && lnum==1) enable=TRUE;
   }
   Enablegad(showgad,enable);
   name=url=NULL;
   if(listid==LIST_GROUPS)
   {  if(gnum==1)
      {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(Selectednode(&glist),LBNA_UserData);
         if(hi)
         {  if(hi->type==HITEM_ENTRY)
            {  name=hi->base->title;
               url=hi->base->url;
            }
            else
            {  name=hi->name;
            }
         }
      }
   }
   else
   {  if(lnum==1)
      {  if(ltype==LTP_GROUPS)
         {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(Selectednode(&llist),LBNA_UserData);
            if(hi)
            {  name=hi->name;
            }
         }
         else
         {  struct Hotbase *hb=(struct Hotbase *)Getlbnvalue(Selectednode(&llist),LBNA_UserData);
            if(hb)
            {  name=hb->title;
               url=hb->url;
            }
         }
      }
   }
   Setpagegadgetattrs(namegad,pagegad,window,NULL,
      GA_Disabled,!name,
      STRINGA_TextVal,name?name:NULLSTRING,
      STRINGA_DispPos,0,
      TAG_END);
   Setpagegadgetattrs(urlgad,pagegad,window,NULL,
      GA_Disabled,!url,
      STRINGA_TextVal,url?url:NULLSTRING,
      STRINGA_DispPos,0,
      TAG_END);
   if(gnum==1 && gtype!=GTP_WHERE)
   {  hi=(struct Hotitem *)Getlbnvalue(Selectednode(&glist),LBNA_UserData);
      if(gtype==GTP_ALL) enable=!hi->owner;
      else enable=TRUE;
   }
   else enable=FALSE;
   Enablegad(gupgad,enable && hi->prev->prev);
   Enablegad(gdowngad,enable && hi->next->next);
   Enablegad(gaddegad,gtype==GTP_GROUP);
   Enablegad(gparentgad,gtype==GTP_GROUP);
}

static void Setlistid(struct Hotwindow *how,short id)
{  if(listid!=id)
   {  listid=id;
      Setgadgetattrs(idgad,window,NULL,
         GA_Image,listid==LIST_LIST?rightarrow:leftarrow,
/*
         BUTTON_AutoButton,listid==LIST_LIST?BAG_RTARROW:BAG_LFARROW,
*/
         TAG_END);
      RefreshGList(idgad,window,NULL,1);
   }
   Setgadgets(how);
}

/*-----------------------------------------------------------------------*/

/* Sort comparison function, sort on URL. */
static int Sortnodesurl(struct Node **na,struct Node **nb)
{  struct Hotbase *hba,*hbb;
   int c=0;
   hba=(struct Hotbase *)Getlbnvalue(*na,LBNA_UserData);
   hbb=(struct Hotbase *)Getlbnvalue(*nb,LBNA_UserData);
   if(hba && hbb && hba->url && hbb->url) c=stricmp(hba->url,hbb->url);
   return c;
}

/* Sort comparison function, sort on date. */
static int Sortnodesdate(struct Node **na,struct Node **nb)
{  struct Hotbase *hba,*hbb;
   int c=0;
   hba=(struct Hotbase *)Getlbnvalue(*na,LBNA_UserData);
   hbb=(struct Hotbase *)Getlbnvalue(*nb,LBNA_UserData);
   if(hba && hbb)
   {  if(hba->date!=hbb->date) c=hbb->date-hba->date;
      else if(hba->title && hbb->title) c=stricmp(hba->title,hbb->title);
   }
   return c;
}

/* Create the right-hand list */
static void Makellist(struct Hotwindow *how)
{  struct Hotbase *hb;
   struct Hotitem *hi;
   struct Node *node;
   short i,n;
   ObtainSemaphore(how->hotsema);
   NewList(&llist);
   if(ltype==LTP_GROUPS)
   {  for(hi=how->hotlist->first;hi->next;hi=hi->next)
      {  if(hi->lnode) AddTail(&llist,hi->lnode);
      }
   }
   else
   {  n=0;
      for(hb=how->hotbase->first;hb->next;hb=hb->next)
      {  if(!hb->lnode)
         {  /* Give newly added base a node */
            hb->lnode=AllocListBrowserNode(1,
               LBNA_UserData,hb,
               LBNCA_CopyText,TRUE,
               LBNCA_Text,hb->title,
               TAG_END);
         }
         if(ltype==LTP_ALL || !hb->used)
         {  if(hb->lnode)
            {  SetListBrowserNodeAttrs(hb->lnode,
                  LBNCA_RenderHook,&lrenderhook,
                  LBNCA_HookHeight,dri->dri_Font->tf_YSize*(urls?2:1),
                  TAG_END);
               AddTail(&llist,hb->lnode);
               n++;
            }
         }
      }
      if(lsort!=LSRT_TITLE)
      {  struct Node **nodes;
         if(nodes=ALLOCTYPE(struct Node *,n,MEMF_CLEAR))
         {  for(i=0;i<n;i++)
            {  if(node=RemHead(&llist)) nodes[i]=node;
            }
            qsort(nodes,n,sizeof(struct Node *),lsort==LSRT_URL?Sortnodesurl:Sortnodesdate);
            for(i=0;i<n;i++)
            {  if(nodes[i]) AddTail(&llist,nodes[i]);
            }
            FREE(nodes);
         }
      }
   }
   ReleaseSemaphore(how->hotsema);
   wasfind=FALSE;
}

/* Update the right-hand list, returns first selected node nr */
static long Updatellist(struct Hotwindow *how)
{  long nr;
   Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   Makellist(how);
   nr=Selectednodenr(&llist);
   Setgadgetattrs(llistgad,window,NULL,
      LISTBROWSER_Labels,&llist,
      LISTBROWSER_MakeVisible,MAX(nr,0),
      TAG_END);
/*
   if(nr>=0) Setgadgetattrs(llistgad,NULL,NULL,LISTBROWSER_MakeVisible,nr,TAG_END);
   RefreshGList(llistgad,window,NULL,1);
*/
   return nr;
}

/* Process double-click in right-hand list */
static void Doubleclickl(struct Hotwindow *how)
{  struct Hotbase *hb;
   if(ltype!=LTP_GROUPS)
   {  hb=(struct Hotbase *)Getlbnvalue(Selectednode(&llist),LBNA_UserData);
      if(hb)
      {  Updatetaskattrs(AOHOT_Follow,hb->url,TAG_END);
      }
   }
}

/* Add an entry in right-hand list */
static void Ladd(struct Hotwindow *how)
{  struct Hotbase *hb;
   if(hb=Newhotbase(AWEBSTR(MSG_HOTL_DEFAULT_TITLE),""))
   {  Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      Inserthotbase(how->hotbase,hb);
      if(ltype==LTP_GROUPS)
      {  Setgadgetattrs(ltypegad,window,NULL,CHOOSER_Active,ltype=LTP_REST,TAG_END);
      }
      Makellist(how);
      Unselectall(&llist);
      SetListBrowserNodeAttrs(hb->lnode,LBNA_Selected,TRUE,TAG_END);
      lselect=Nodenumber(&llist,hb->lnode);
      Setgadgetattrs(llistgad,window,NULL,
         LISTBROWSER_Labels,&llist,
         LISTBROWSER_MakeVisible,lselect,
         TAG_END);
      if(Getvalue(pagegad,PAGE_Current)!=PG_EDIT)
      {  Setgadgetattrs(findgad,window,NULL,GA_Selected,FALSE,TAG_END);
         Setgadgetattrs(patgad,window,NULL,GA_Selected,FALSE,TAG_END);
         SetGadgetAttrs(pagegad,window,NULL,PAGE_Current,PG_EDIT,TAG_END);
         RethinkLayout(pagelayout,window,NULL,TRUE);
      }
      Setlistid(how,LIST_LIST);
      ActivateLayoutGadget(toplayout,window,NULL,(ULONG) namegad);
      Updatetaskattrs(AOTSK_Async,TRUE,AOHOT_Changed,TRUE,TAG_END);
   }
}

/* Find (next) entry in right-hand list */
static void Selectfind(struct Hotwindow *how)
{  struct Node *node;
   UBYTE *string;
   string=(UBYTE *)Getvalue(fstrgad,STRINGA_TextVal);
   if(string && *string)
   {  Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      if(wasfind && (node=Selectednode(&llist)))
      {  SetListBrowserNodeAttrs(node,LBNA_Selected,FALSE,TAG_END);
         node=node->ln_Succ;
      }
      else
      {  Unselectall(&llist);
         node=llist.lh_Head;
      }
      for(;node->ln_Succ;node=node->ln_Succ)
      {  if(ltype==LTP_GROUPS)
         {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(node,LBNA_UserData);
            if(hi->name && Strstri(hi->name,string))
            {  SetListBrowserNodeAttrs(node,LBNA_Selected,TRUE,TAG_END);
               break;
            }
         }
         else
         {  struct Hotbase *hb=(struct Hotbase *)Getlbnvalue(node,LBNA_UserData);
            if((hb->title && Strstri(hb->title,string))
            || (urls && hb->url && Strstri(hb->url,string)))
            {  SetListBrowserNodeAttrs(node,LBNA_Selected,TRUE,TAG_END);
               break;
            }
         }
      }
      lselect=Selectednodenr(&llist);
      Setgadgetattrs(llistgad,window,NULL,
         LISTBROWSER_Labels,&llist,
         LISTBROWSER_MakeVisible,MAX(lselect,0),
         TAG_END);
      Setlistid(how,LIST_LIST);
      wasfind=TRUE;
   }
}

/* Select right-hand list nodes by pattern */
static void Selectpattern(struct Hotwindow *how)
{  struct Node *node;
   UBYTE *pattern,*parsed,*p;
   long len;
   short wild;
   BOOL noscheme=FALSE,select;
   pattern=(UBYTE *)Getvalue(pstrgad,STRINGA_TextVal);
   if(pattern && *pattern)
   {  len=2*strlen(pattern)+4;
      if(parsed=ALLOCTYPE(UBYTE,len,0))
      {  if((wild=ParsePatternNoCase(pattern,parsed,len))>=0)
         {  if(wild && !strstr(pattern,"://")) noscheme=TRUE;
            Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
            for(node=llist.lh_Head;node->ln_Succ;node=node->ln_Succ)
            {  select=FALSE;
               if(ltype==LTP_GROUPS)
               {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(node,LBNA_UserData);
                  if(hi->name)
                  {  if(wild) select|=BOOLVAL(MatchPatternNoCase(parsed,hi->name));
                     else select|=BOOLVAL(Strstri(hi->name,pattern));
                  }
               }
               else
               {  struct Hotbase *hb=(struct Hotbase *)Getlbnvalue(node,LBNA_UserData);
                  if(hb->title)
                  {  if(wild) select|=BOOLVAL(MatchPatternNoCase(parsed,hb->title));
                     else select|=BOOLVAL(Strstri(hb->title,pattern));
                  }
                  if(urls && hb->url)
                  {  if(wild)
                     {  if(noscheme)
                        {  p=strstr(hb->url,"://");
                           if(p) p+=3;
                           else p=hb->url;
                        }
                        else
                        p=hb->url;
                        select|=BOOLVAL(MatchPatternNoCase(parsed,p));
                     }
                     else select|=BOOLVAL(Strstri(hb->url,pattern));
                  }
               }
               SetListBrowserNodeAttrs(node,LBNA_Selected,select,TAG_END);
            }
            lselect=Selectednodenr(&llist);
            Setgadgetattrs(llistgad,window,NULL,
               LISTBROWSER_Labels,&llist,
               LISTBROWSER_MakeVisible,MAX(lselect,0),
               TAG_END);
            Setlistid(how,LIST_LIST);
            wasfind=FALSE;
         }
         FREE(parsed);
      }
   }
}

/*-----------------------------------------------------------------------*/

/* Create the left-hand list, all groups */
static void Addgrouptoglist(LIST(Hotitem) *list,short level)
{  struct Hotitem *hi;
   for(hi=list->first;hi->next;hi=hi->next)
   {  if(hi->type!=HITEM_ENTRY)
      {  if(hi->gnode)
         {  SetListBrowserNodeAttrs(hi->gnode,
               LBNA_Generation,level,
               LBNA_Flags,LBFLG_CUSTOMPENS|LBFLG_HASCHILDREN|hi->showchildren,
               TAG_END);
            AddTail(&glist,hi->gnode);
         }
         Addgrouptoglist(&hi->subitems,level+1);
         if(glist.lh_TailPred==hi->gnode)
         {  /* No subitems added */
            SetListBrowserNodeAttrs(hi->gnode,
               LBNA_Flags,LBFLG_CUSTOMPENS,
               TAG_END);
         }
         else if(!hi->showchildren)
         {  HideListBrowserNodeChildren(hi->gnode);
         }
      }
   }
}

/* Create the left-hand list */
static void Makeglist(struct Hotwindow *how)
{  ObtainSemaphore(how->hotsema);
   NewList(&glist);
   if(gtype==GTP_ALL)
   {  Addgrouptoglist(how->hotlist,1);
      ggroup=NULL;
   }
   else if(ggroup)
   {  struct Hotitem *hi;
      for(hi=ggroup->subitems.first;hi->next;hi=hi->next)
      {  if(hi->gnode)
         {  SetListBrowserNodeAttrs(hi->gnode,
               LBNA_Flags,(hi->type==HITEM_ENTRY?0:LBFLG_CUSTOMPENS),
               hi->type==HITEM_ENTRY?LBNCA_RenderHook:TAG_IGNORE,&grenderhook,
               LBNCA_HookHeight,dri->dri_Font->tf_YSize*(urls?2:1),
               TAG_END);
            AddTail(&glist,hi->gnode);
         }
      }
   }
   ReleaseSemaphore(how->hotsema);
}

/* Set the left-hand list to show this group */
static void Setglist(struct Hotwindow *how,struct Hotitem *group)
{  if(group) gtype=GTP_GROUP;
   else gtype=GTP_ALL;
   ggroup=group;
   Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   Unselectall(&glist);
   Makeglist(how);
   gselect=Selectednodenr(&glist);
   Setgadgetattrs(glistgad,window,NULL,
      LISTBROWSER_Hierarchical,gtype==GTP_ALL,
      LISTBROWSER_Labels,&glist,
      LISTBROWSER_MakeVisible,gselect,
      TAG_END);
   Setlistid(how,LIST_GROUPS);
}

/* Update the left-hand list */
static void Updateglist(struct Hotwindow *how)
{  long nr;
   Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   Makeglist(how);
   nr=Selectednodenr(&glist);
   Setgadgetattrs(glistgad,NULL,NULL,LISTBROWSER_Labels,&glist,TAG_END);
   if(nr>=0) Setgadgetattrs(glistgad,NULL,NULL,LISTBROWSER_MakeVisible,nr,TAG_END);
   RefreshGList(glistgad,window,NULL,1);

}

/* Remember to show or hide children */
static void Showgchildren(long nr,BOOL show)
{  struct Node *node,*selnode;
   struct Hotitem *hi;
   selnode=Getnode(&glist,nr);
   hi=(struct Hotitem *)Getlbnvalue(selnode,LBNA_UserData);
   if(hi)
   {  if(show) hi->showchildren=LBFLG_SHOWCHILDREN;
      else hi->showchildren=0;
      Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      for(node=glist.lh_Head;node->ln_Succ;node=node->ln_Succ)
      {  if(node!=selnode)
         {  SetListBrowserNodeAttrs(node,LBNA_Selected,FALSE,TAG_END);
         }
      }
      Setgadgetattrs(glistgad,NULL,NULL,LISTBROWSER_Labels,&glist,TAG_END);
      gselect=nr;
   }
}

/* Process double-click in left-hand list */
static void Doubleclickg(struct Hotwindow *how)
{  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(Selectednode(&glist),LBNA_UserData);
   if(hi)
   {  if(hi->type==HITEM_ENTRY)
      {  if(hi->base)
         {  Updatetaskattrs(AOHOT_Follow,hi->base->url,TAG_END);
         }
      }
      else
      {  Setglist(how,hi);
      }
   }
}

/* Move one entry in lh list up or down */
static void Moveglist(struct Hotwindow *how,long d)
{  struct Hotitem *hi,*hiafter;
   void *list;
   hi=(struct Hotitem *)Getlbnvalue(Selectednode(&glist),LBNA_UserData);
   if(hi && (gtype==GTP_GROUP || (gtype==GTP_ALL && !hi->owner)))
   {  Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      if(d>0) hiafter=hi->next;
      else hiafter=hi->prev->prev;
      if(hiafter && hiafter->next)
      {  REMOVE(hi);
         if(hi->owner) list=&hi->owner->subitems;
         else list=how->hotlist;
         INSERT(list,hi,hiafter);
         if(d<0) gselect--;
         else gselect++;
      }
      Makeglist(how);
      Setgadgetattrs(glistgad,window,NULL,
         LISTBROWSER_Labels,&glist,
         LISTBROWSER_MakeVisible,gselect,
         TAG_END);
      Setlistid(how,LIST_GROUPS);
      Updatetaskattrs(AOTSK_Async,TRUE,AOHOT_Changed,TRUE,TAG_END);
   }
}

/* Sort comparison function. */
static int Sorthitem(struct Hotitem **ha,struct Hotitem **hb)
{  UBYTE *pa,*pb;
   if((*ha)->type==HITEM_ENTRY) pa=(*ha)->base->title;
   else pa=(*ha)->name;
   if((*hb)->type==HITEM_ENTRY) pb=(*hb)->base->title;
   else pb=(*hb)->name;
   return stricmp(pa,pb);
}

/* Sort the left-hand list */
static void Sortglist(struct Hotwindow *how)
{  long ng=0,ne=0,ig,ie;
   struct Hotitem *hi;
   struct Hotitem **hia;
   LIST(Hotitem) *list=NULL;
   struct Node *node=Getnode(&glist,gselect);
   if(gtype==GTP_GROUP && ggroup) list=&ggroup->subitems;
   else list=how->hotlist;
   if(list && gtype!=GTP_WHERE)
   {  Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      for(hi=list->first;hi->next;hi=hi->next)
      {  if(hi->type==HITEM_ENTRY) ne++;
         else ng++;
      }
      if((ne+ng) && (hia=ALLOCTYPE(struct Hotitem *,ne+ng,0)))
      {  ig=0,ie=0;
         while(hi=REMHEAD(list))
         {  if(hi->type==HITEM_ENTRY)
            {  hia[ng+ie]=hi;
               ie++;
            }
            else
            {  hia[ig]=hi;
               ig++;
            }
         }
         if(ng) qsort(hia,ng,sizeof(struct Hotitem *),Sorthitem);
         if(ne) qsort(hia+ng,ne,sizeof(struct Hotitem *),Sorthitem);
         for(ie=0;ie<ng+ne;ie++)
         {  ADDTAIL(list,hia[ie]);
         }
         FREE(hia);
      }
      Makeglist(how);
      gselect=Nodenumber(&glist,node);
      Setgadgetattrs(glistgad,window,NULL,
         LISTBROWSER_Labels,&glist,
         LISTBROWSER_MakeVisible,gselect,
         TAG_END);
      Setlistid(how,LIST_GROUPS);
      Updatetaskattrs(AOTSK_Async,TRUE,AOHOT_Changed,TRUE,TAG_END);
   }
}

/* Add a new group */
static void Gaddg(struct Hotwindow *how)
{  struct Hotitem *hi;
   LIST(Hotitem) *list;
   if(gtype==GTP_ALL) list=how->hotlist;
   else if(gtype==GTP_GROUP && ggroup) list=&ggroup->subitems;
   else return;
   if(hi=ALLOCSTRUCT(Hotitem,1,MEMF_PUBLIC|MEMF_CLEAR))
   {  hi->nodetype=HNT_ITEM;
      hi->owner=(gtype==GTP_ALL)?NULL:ggroup;
      hi->type=HITEM_GROUP;
      hi->name=Dupstr(AWEBSTR(MSG_HOTL_DEFAULT_GROUP),-1);
      NEWLIST(&hi->subitems);
      Allocitemnodes1(hi);
      ADDTAIL(list,hi);
      Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      Makeglist(how);
      Unselectall(&glist);
      if(hi->gnode)
      {  SetListBrowserNodeAttrs(hi->gnode,LBNA_Selected,TRUE,TAG_END);
      }
      gselect=Nodenumber(&glist,hi->gnode);
      Setgadgetattrs(glistgad,window,NULL,
         LISTBROWSER_Labels,&glist,
         LISTBROWSER_MakeVisible,gselect,
         TAG_END);
      if(gtype==GTP_ALL && ltype==LTP_GROUPS)
      {  Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
         Makellist(how);
         Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,&llist,TAG_END);
      }
      if(Getvalue(pagegad,PAGE_Current)!=PG_EDIT)
      {  Setgadgetattrs(findgad,window,NULL,GA_Selected,FALSE,TAG_END);
         Setgadgetattrs(patgad,window,NULL,GA_Selected,FALSE,TAG_END);
         SetGadgetAttrs(pagegad,window,NULL,PAGE_Current,PG_EDIT,TAG_END);
         RethinkLayout(pagelayout,window,NULL,TRUE);
      }
      Setlistid(how,LIST_GROUPS);
      ActivateLayoutGadget(toplayout,window,NULL,(ULONG) namegad);
      Updatetaskattrs(AOTSK_Async,TRUE,AOHOT_Changed,TRUE,TAG_END);
   }
}

/* Add a new entry */
static void Gadde(struct Hotwindow *how)
{  struct Hotitem *hi;
   struct Hotbase *hb;
   if(gtype==GTP_GROUP && ggroup)
   {  if(hb=Newhotbase(AWEBSTR(MSG_HOTL_DEFAULT_TITLE),""))
      {  Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
         Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
         Inserthotbase(how->hotbase,hb);
         if(hi=ALLOCSTRUCT(Hotitem,1,MEMF_PUBLIC|MEMF_CLEAR))
         {  hi->nodetype=HNT_ITEM;
            ADDTAIL(&ggroup->subitems,hi);
            hi->owner=ggroup;
            hi->type=HITEM_ENTRY;
            hi->base=hb;
            hb->used++;
            NEWLIST(&hi->subitems);
            Allocitemnodes1(hi);
            Makellist(how);
            Makeglist(how);
            Unselectall(&glist);
            if(hi->gnode)
            {  SetListBrowserNodeAttrs(hi->gnode,LBNA_Selected,TRUE,TAG_END);
            }
         }
         gselect=Nodenumber(&glist,hb->lnode);
         Setgadgetattrs(glistgad,window,NULL,
            LISTBROWSER_Labels,&glist,
            LISTBROWSER_MakeVisible,gselect,
            TAG_END);
         Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,&llist,TAG_END);
         if(Getvalue(pagegad,PAGE_Current)!=PG_EDIT)
         {  Setgadgetattrs(findgad,window,NULL,GA_Selected,FALSE,TAG_END);
            Setgadgetattrs(patgad,window,NULL,GA_Selected,FALSE,TAG_END);
            SetGadgetAttrs(pagegad,window,NULL,PAGE_Current,PG_EDIT,TAG_END);
            RethinkLayout(pagelayout,window,NULL,TRUE);
         }
         Setlistid(how,LIST_GROUPS);
         ActivateLayoutGadget(toplayout,window,NULL,(ULONG) namegad);
         Updatetaskattrs(AOTSK_Async,TRUE,AOHOT_Changed,TRUE,TAG_END);
      }
   }
}

/*-----------------------------------------------------------------------*/

/* Change name of hotitem */
static void Newitemname(struct Hotitem *hi,UBYTE *name)
{  if(name=Dupstr(name,-1))
   {  if(hi->name) FREE(hi->name);
      hi->name=name;
   }
   if(hi->lnode)
   {  SetListBrowserNodeAttrs(hi->lnode,
         LBNCA_CopyText,TRUE,
         LBNCA_Text,hi->name,
         TAG_END);
   }
   if(hi->gnode)
   {  SetListBrowserNodeAttrs(hi->gnode,
         LBNCA_CopyText,TRUE,
         LBNCA_Text,hi->name,
         TAG_END);
   }
}

/* Change name or url of hotbase. If it clashes with existing base, merge the two
 * and return the new hotbase. */
static struct Hotbase *Newbasename(struct Hotwindow *how,
   struct Hotbase *hb,UBYTE *name,BOOL url)
{  struct Hotbase *hbm;
   UBYTE **sp;
   if(url) sp=&hb->url;
   else sp=&hb->title;
   if(name=Dupstr(name,-1))
   {  if(*sp) FREE(*sp);
      *sp=name;
   }
   if(hbm=Findhotbase(how->hotbase,hb))
   {  hbm->used+=hb->used;
      Deletebase(hb);
   }
   else
   {  if(!url)
      {  REMOVE(hb);
         Inserthotbase(how->hotbase,hb);
         if(hb->lnode)
         {  SetListBrowserNodeAttrs(hb->lnode,
               LBNCA_CopyText,TRUE,
               LBNCA_Text,hb->title,
               TAG_END);
         }
      }
   }
   return hbm;
}

/* Handle a name or url change */
static void Setname(struct Hotwindow *how,struct Gadget *gad,BOOL url)
{  struct Node *node;
   UBYTE *name=(UBYTE *)Getvalue(gad,STRINGA_TextVal);
   if(listid==LIST_LIST)
   {  node=Selectednode(&llist);
      if(ltype==LTP_GROUPS)
      {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(node,LBNA_UserData);
         if(hi)
         {  Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
            Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
            Newitemname(hi,name);
            Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,&llist,TAG_END);
            Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,&glist,TAG_END);
         }
      }
      else
      {  struct Hotbase *hb=(struct Hotbase *)Getlbnvalue(node,LBNA_UserData);
         struct Hotbase *hbm;
         if(hb)
         {  Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
            Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
            hbm=Newbasename(how,hb,name,url);
            if(!url || lsort==LSRT_URL || hbm) Makellist(how);
            lselect=Nodenumber(&llist,hb->lnode);
            Setgadgetattrs(llistgad,window,NULL,
               LISTBROWSER_Labels,&llist,
               LISTBROWSER_MakeVisible,lselect,
               TAG_END);
            Updateitems(how->hotlist,hb,hbm);
            Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,&glist,TAG_END);
         }
      }
   }
   else
   {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(Selectednode(&glist),LBNA_UserData);
      BOOL ltoo;
      if(hi)
      {  if(hi->type==HITEM_ENTRY)
         {  struct Hotbase *hbm;
            Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
            Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
            if(hi->base->used>1)
            {  struct Hotbase *hb;
               if(hb=Newhotbase(hi->base->title,hi->base->url))
               {  Inserthotbase(how->hotbase,hb);
                  hb->used++;
                  hbm=Newbasename(how,hb,name,url);
                  if(hbm) hb=hbm;
                  hi->base=hb;
               }
            }
            else
            {  hbm=Newbasename(how,hi->base,name,url);
               if(hbm) hi->base=hbm;
            }
            if(hi->gnode)
            {  SetListBrowserNodeAttrs(hi->gnode,
                  LBNCA_CopyText,TRUE,
                  LBNCA_Text,hi->base->title,
                  TAG_END);
            }
            Makellist(how);
            Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,&llist,TAG_END);
            Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,&glist,TAG_END);
         }
         else
         {  ltoo=(ltype==LTP_GROUPS) && (gtype==GTP_ALL);
            if(ltoo) Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
            Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
            Newitemname(hi,name);
            if(ltoo) Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,&llist,TAG_END);
            Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,&glist,TAG_END);
         }
      }
   }
   Updatetaskattrs(AOTSK_Async,TRUE,AOHOT_Changed,TRUE,TAG_END);
}

/* Move rh-side selected entries into group */
static void Movein(struct Hotwindow *how,BOOL remove)
{  struct Node *node;
   struct Hotitem *hi;
   struct Hotbase *hb;
   if(gtype==GTP_GROUP && ggroup)
   {  Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      Unselectall(&glist);
      for(node=llist.lh_Head;node->ln_Succ;node=node->ln_Succ)
      {  if(Getlbnvalue(node,LBNA_Selected))
         {  if(ltype==LTP_GROUPS)
            {  hi=(struct Hotitem *)Getlbnvalue(node,LBNA_UserData);
               if(!Isownerof(ggroup,hi))
               {  REMOVE(hi);
                  ADDTAIL(&ggroup->subitems,hi);
                  hi->owner=ggroup;
                  if(hi->gnode)
                  {  SetListBrowserNodeAttrs(hi->gnode,LBNA_Selected,TRUE,TAG_END);
                  }
               }
            }
            else
            {  hb=(struct Hotbase *)Getlbnvalue(node,LBNA_UserData);
               if(remove)
               {  if(hb->used)
                  {  /* Delete all items where hb is used, but don't delete hb */
                     Deleteitems(how->hotlist,hb,FALSE);
                  }
               }
               hi=ALLOCSTRUCT(Hotitem,1,MEMF_PUBLIC|MEMF_CLEAR);
               if(hi)
               {  hi->nodetype=HNT_ITEM;
                  ADDTAIL(&ggroup->subitems,hi);
                  hi->owner=ggroup;
                  hi->type=HITEM_ENTRY;
                  hi->base=hb;
                  hb->used++;
                  NEWLIST(&hi->subitems);
                  Allocitemnodes1(hi);
                  if(hi->gnode)
                  {  SetListBrowserNodeAttrs(hi->gnode,LBNA_Selected,TRUE,TAG_END);
                  }
               }
            }
         }
      }
      Makeglist(how);
      Makellist(how);
      gselect=Selectednodenr(&glist);
      Setgadgetattrs(glistgad,window,NULL,
         LISTBROWSER_Labels,&glist,
         LISTBROWSER_MakeVisible,gselect,
         TAG_END);
      Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,&llist,TAG_END);
      Setlistid(how,LIST_GROUPS);
      Updatetaskattrs(AOTSK_Async,TRUE,AOHOT_Changed,TRUE,TAG_END);
   }
}

/* Move lh-side selected entries out of group */
static void Moveout(struct Hotwindow *how)
{  struct Node *node,*next;
   struct Hotitem *hi;
   struct Hotbase *hb;
   if(gtype==GTP_GROUP && ggroup)
   {  Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
      Unselectall(&llist);
      for(node=glist.lh_Head;node->ln_Succ;node=next)
      {  next=node->ln_Succ;
         if(Getlbnvalue(node,LBNA_Selected))
         {  hi=(struct Hotitem *)Getlbnvalue(node,LBNA_UserData);
            if(hi->type==HITEM_ENTRY)
            {  hb=hi->base;
               REMOVE(hi);
               if(hi->name) FREE(hi->name);
               if(hi->gnode) FreeListBrowserNode(hi->gnode);
               if(hi->lnode) FreeListBrowserNode(hi->lnode);
               FREE(hi);
               hb->used--;
               if(hb->lnode)
               {  SetListBrowserNodeAttrs(hb->lnode,LBNA_Selected,TRUE,TAG_END);
               }
            }
            else
            {  REMOVE(hi);
               ADDTAIL(how->hotlist,hi);
               hi->owner=NULL;
               if(hi->lnode)
               {  SetListBrowserNodeAttrs(hi->lnode,LBNA_Selected,TRUE,TAG_END);
               }
            }
         }
      }
      Makeglist(how);
      Makellist(how);
      lselect=Selectednodenr(&llist);
      Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,&glist,TAG_END);
      Setgadgetattrs(llistgad,window,NULL,
         LISTBROWSER_Labels,&llist,
         LISTBROWSER_MakeVisible,lselect,
         TAG_END);
      Setlistid(how,LIST_LIST);
      Updatetaskattrs(AOTSK_Async,TRUE,AOHOT_Changed,TRUE,TAG_END);
   }
}

/* Delete selected entries */
static void Deleteentries(struct Hotwindow *how)
{  struct Node *node,*next;
   Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
   if(listid==LIST_GROUPS)
   {  /* In case of the ALL list, deleting a node will also delete the
       * subnodes that come next in the list...
       * Therefore the list is traversed backwards. */
      for(node=glist.lh_TailPred;node->ln_Pred;node=next)
      {  next=node->ln_Pred;
         if(Getlbnvalue(node,LBNA_Selected))
         {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(node,LBNA_UserData);
            Deleteitem(hi);
         }
      }
      gselect=-1;
   }
   else
   {  for(node=llist.lh_Head;node->ln_Succ;node=next)
      {  next=node->ln_Succ;
         if(Getlbnvalue(node,LBNA_Selected))
         {  if(ltype==LTP_GROUPS)
            {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(node,LBNA_UserData);
               if(gtype==GTP_GROUP && Isownerof(ggroup,hi))
               {  ggroup=NULL;
                  gtype=GTP_ALL;
               }
               Deleteitem(hi);
            }
            else
            {  struct Hotbase *hb=(struct Hotbase *)Getlbnvalue(node,LBNA_UserData);
               if(hb->used) Deleteitems(how->hotlist,hb,TRUE); /* Last one deletes base too */
               else Deletebase(hb);
            }
         }
      }
      lselect=-1;
   }
   Makeglist(how);
   Makellist(how);
   Setgadgetattrs(glistgad,window,NULL,
      LISTBROWSER_Hierarchical,gtype==GTP_ALL,
      LISTBROWSER_Labels,&glist,
      TAG_END);
   Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,&llist,TAG_END);
   Setgadgets(how);
   Updatetaskattrs(AOTSK_Async,TRUE,AOHOT_Changed,TRUE,TAG_END);
}

/* Add group to glist if list contains this base, unselect all items of
 * group added except the matchine one, return nr of items added */
static long Additemswhere(struct Hotitem *group,LIST(Hotitem) *list,struct Hotbase *hb)
{  long n=0;
   struct Hotitem *hi,*match=NULL;
   for(hi=list->first;hi->next;hi=hi->next)
   {  if(hi->type==HITEM_ENTRY)
      {  if(hi->base==hb && group && group->gnode)
         {  match=hi;
            break;
         }
      }
   }
   if(match)
   {  SetListBrowserNodeAttrs(group->gnode,
         LBNA_Selected,FALSE,
         TAG_END);
      AddTail(&glist,group->gnode);
      n++;
      for(hi=list->first;hi->next;hi=hi->next)
      {  SetListBrowserNodeAttrs(hi->gnode,
            LBNA_Selected,hi==match,
            TAG_END);
      }
   }
   for(hi=list->first;hi->next;hi=hi->next)
   {  if(hi->type!=HITEM_ENTRY)
      {  n+=Additemswhere(hi,&hi->subitems,hb);
      }
   }
   return n;
}

/* Find group(s) containing this entry */
static void Where(struct Hotwindow *how)
{  struct Hotbase *hb;
   long n;
   if(ltype!=LTP_GROUPS)
   {  hb=(struct Hotbase *)Getlbnvalue(Selectednode(&llist),LBNA_UserData);
      if(hb)
      {  Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,~0,TAG_END);
         gselect=-1;
         NewList(&glist);
         n=Additemswhere(NULL,how->hotlist,hb);
         if(n==0)
         {  Setglist(how,NULL);
         }
         else if(n==1)
         {  Setglist(how,(struct Hotitem *)Getlbnvalue(glist.lh_Head,LBNA_UserData));
            gselect=Selectednodenr(&glist);
         }
         else
         {  gtype=GTP_WHERE;
            Setgadgets(how);
         }
         Setgadgetattrs(glistgad,window,NULL,
            LISTBROWSER_Labels,&glist,
            LISTBROWSER_Hierarchical,gtype==GTP_ALL,
            LISTBROWSER_MakeVisible,MAX(gselect,0),
            TAG_END);
      }
   }
}

/* Show selected entry */
static void Show(struct Hotwindow *how)
{  struct Hotbase *hb=NULL;
   if(listid==LIST_GROUPS)
   {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(Selectednode(&glist),LBNA_UserData);
      if(hi && hi->type==HITEM_ENTRY) hb=hi->base;
   }
   else if(ltype!=LTP_GROUPS)
   {  hb=(struct Hotbase *)Getlbnvalue(Selectednode(&llist),LBNA_UserData);
   }
   if(hb)
   {  Updatetaskattrs(AOHOT_Follow,hb->url,TAG_END);
   }
}

/* Determine the next visible node */
static long Nextlistnode(struct List *list,long selected)
{  struct Node *node=Getnode(list,selected);
   long nr=selected;
   ULONG flags;
   ULONG gen1;
   if(!node) return 0;
   flags=Getlbnvalue(node,LBNA_Flags);
   gen1=Getlbnvalue(node,LBNA_Generation);
   if((flags&LBFLG_HASCHILDREN) && !(flags&LBFLG_SHOWCHILDREN))
   {  for(node=node->ln_Succ;node->ln_Succ;node=node->ln_Succ)
      {  nr++;
         if(Getlbnvalue(node,LBNA_Generation)<=gen1) break;
      }
   }
   else
   {  node=node->ln_Succ;
      nr++;
   }
   if(node->ln_Succ) return nr;
   else return selected;
}

/* Determine the previous visible node */
static long Prevlistnode(struct List *list,long selected)
{  struct Node *node=Getnode(list,selected);
   long nra=selected,nrb;
   ULONG flags;
   ULONG gen1,gen2,gen3;
   if(!node) return -1;
   gen1=Getlbnvalue(node,LBNA_Generation);
   node=node->ln_Pred;
   if(!node->ln_Pred) return selected;
   nra--;
   gen2=Getlbnvalue(node,LBNA_Generation);
   if(gen2<=gen1) return nra;
   /* Previous node is member of other group. Check if it is visible */
   /* nra will be number of visible node, nrb = number of node looked at */
   /* gen1=level of original; gen3=group level to look for */
   nrb=nra;
   gen3=gen2-1;
   for(node=node->ln_Pred;node->ln_Pred;node=node->ln_Pred)
   {  nrb--;
      flags=Getlbnvalue(node,LBNA_Flags);
      if(flags&LBFLG_HASCHILDREN)                  /* got a group */
      {  gen2=Getlbnvalue(node,LBNA_Generation);
         if(gen2==gen1)       /* same level as original */
         {  if(flags&LBFLG_SHOWCHILDREN) return nra; /* group is open */
            else return nrb;  /* group is closed, return group level */
         }
         /* got a group. Is it the level we're looking for? */
         if(gen2==gen3)
         {  if(!(flags&LBFLG_SHOWCHILDREN))  /* this group is closed */
            {  nra=nrb;       /* return at most this group, not its member */
            }
            gen3--;           /* look out for parent level */
         }
      }
   }
   return selected;
}

/* Selected another entry because of cursor keys */
static void Moveselected(struct Hotwindow *how,short d,BOOL shift)
{  struct List *list;
   struct Gadget *gad;
   long *select;
   BOOL update=FALSE,sel;
   struct Node *node,*selnode;
   if(listid==LIST_GROUPS)
   {  list=&glist;
      gad=glistgad;
      select=&gselect;
   }
   else
   {  list=&llist;
      gad=llistgad;
      select=&lselect;
      wasfind=FALSE;
   }
   if(listid==LIST_GROUPS && gtype==GTP_ALL)
   {  if(d<0) *select=Prevlistnode(list,*select);
      else *select=Nextlistnode(list,*select);
      update=TRUE;
      selnode=Getnode(list,*select);
   }
   else
   {  selnode=Getnode(list,*select);
      if(selnode && (!selnode->ln_Succ || !selnode->ln_Pred)) selnode=NULL;
      if(d<0)
      {  if(selnode && selnode->ln_Pred->ln_Pred)
         {  selnode=selnode->ln_Pred;
            (*select)--;
            update=TRUE;
         }
         else if(list->lh_TailPred->ln_Pred)
         {  selnode=list->lh_TailPred;
            *select=Getvalue(gad,LISTBROWSER_TotalNodes)-1;
            update=TRUE;
         }
      }
      else if(d>0)
      {  if(selnode && selnode->ln_Succ->ln_Succ)
         {  selnode=selnode->ln_Succ;
            (*select)++;
            update=TRUE;
         }
         else if(list->lh_Head->ln_Succ)
         {  selnode=list->lh_Head;
            *select=0;
            update=TRUE;
         }
      }
   }
   if(update)
   {  for(node=list->lh_Head;node->ln_Succ;node=node->ln_Succ)
      {  sel=Getlbnvalue(node,LBNA_Selected);
         if((!shift && sel && node!=selnode)
         || (shift && sel && node==selnode))
         {  Editlbnode(gad,window,NULL,node,LBNA_Selected,FALSE,TAG_END);
         }
         else if(node==selnode && !sel)
         {  Editlbnode(gad,window,NULL,node,LBNA_Selected,TRUE,TAG_END);
         }
      }
      Setgadgetattrs(gad,window,NULL,LISTBROWSER_MakeVisible,*select,TAG_END);
      Setgadgets(how);
   }
   else
   {  *select=-1;
   }
}

/* Dispose the gadget lists */
static void Disposelist(struct Hotwindow *how)
{  Setgadgetattrs(llistgad,window,NULL,LISTBROWSER_Labels,NULL,TAG_END);
   Setgadgetattrs(glistgad,window,NULL,LISTBROWSER_Labels,NULL,TAG_END);
   Freeallnodes(how);
   Setgadgets(how);
   Setgadgetattrs(toplayout,window,NULL,GA_ReadOnly,TRUE,TAG_END);
}

/* Build new gadget lists */
static void Newhotlist(struct Hotwindow *how)
{  NewList(&glist);
   NewList(&llist);
   listid=LIST_LIST;
   gtype=GTP_ALL;
   gselect=-1;
   ggroup=NULL;
   lselect=-1;
   urls=FALSE;
   wasfind=FALSE;
   Allocallnodes(how);
   Makeglist(how);
   Makellist(how);
   Setgadgetattrs(llistgad,window,NULL,
      LISTBROWSER_Labels,&llist,
      LISTBROWSER_Top,0,
      TAG_END);
   Setgadgetattrs(glistgad,window,NULL,
      LISTBROWSER_Labels,&glist,
      LISTBROWSER_Hierarchical,TRUE,
      LISTBROWSER_Top,0,
      TAG_END);
   Setgadgets(how);
   Setgadgetattrs(toplayout,window,NULL,GA_ReadOnly,FALSE,TAG_END);
}

/*-----------------------------------------------------------------------*/

/* Erase any part of "old" which is not covered by "nw" */
static void Fillouter(struct RastPort *rp,struct Rectangle *old,struct Rectangle *nw)
{   if(old->MinX<nw->MinX) RectFill(rp,old->MinX,old->MinY,nw->MinX-1,old->MaxY);
    if(old->MaxX>nw->MaxX) RectFill(rp,nw->MaxX+1,old->MinY,old->MaxX,old->MaxY);
    if(old->MaxY>nw->MaxY) RectFill(rp,old->MinX,nw->MaxY+1,old->MaxX,old->MaxY);
    if(old->MinY<nw->MinY) RectFill(rp,old->MinX,old->MinY,old->MaxX,nw->MinY-1);
}

/* Hook called to render entries with title and url */
DECLARE_HOOK
(
static ULONG __saveds   , Renderhook,
struct Hook *,hook,A0,
struct Node *,node,A2,
struct LBDrawMsg *,msg,A1

)
{
    USRFUNC_INIT
  ULONG result=LBCB_UNKNOWN;
   struct RastPort *rp=msg->lbdm_RastPort;
   struct Rectangle rect;
   struct TextExtent te;
   short fit,slack,apen,bpen,x,y;
   struct Hotbase *hb;
   UWORD used=0;
   if(msg->lbdm_MethodID==LB_DRAW)
   {  if(hook->h_Data==(APTR)LIST_GROUPS)
      {  struct Hotitem *hi=(struct Hotitem *)Getlbnvalue(node,LBNA_UserData);
         hb=hi->base;
      }
      else
      {  hb=(struct Hotbase *)Getlbnvalue(node,LBNA_UserData);
         if(!hb->used) used=FSF_ITALIC;
      }
      if(msg->lbdm_State==LBR_NORMAL)
      {  apen=msg->lbdm_DrawInfo->dri_Pens[TEXTPEN];
         bpen=msg->lbdm_DrawInfo->dri_Pens[BACKGROUNDPEN];
      }
      else
      {  apen=msg->lbdm_DrawInfo->dri_Pens[FILLTEXTPEN];
         bpen=msg->lbdm_DrawInfo->dri_Pens[FILLPEN];
      }

      SetABPenDrMd(rp,apen,bpen,JAM2);
      SetSoftStyle(rp,(urls?FSF_BOLD:0)|used,0x0f);
      rect=msg->lbdm_Bounds;
      if(urls) rect.MaxY=(rect.MinY+rect.MaxY)/2;
      fit=TextFit(rp,hb->title,strlen(hb->title),&te,NULL,1,
         rect.MaxX-rect.MinX,rect.MaxY-rect.MinY+1);
      slack=(rect.MaxY-rect.MinY)-(te.te_Extent.MaxY-te.te_Extent.MinY);
      x=rect.MinX-te.te_Extent.MinX+1;
      y=rect.MinY-te.te_Extent.MinY+(slack+1)/2;
      Move(rp,x,y);
      Text(rp,hb->title,fit);
      te.te_Extent.MinX+=x;
      te.te_Extent.MinY+=y;
      te.te_Extent.MaxX+=x;
      te.te_Extent.MaxY+=y;
      SetAPen(rp,bpen);
      Fillouter(rp,&rect,&te.te_Extent);

      if(urls)
      {  SetABPenDrMd(rp,apen,bpen,JAM2);
         SetSoftStyle(rp,used,0x0f);
         rect=msg->lbdm_Bounds;
         if(urls) rect.MinY=(rect.MinY+rect.MaxY)/2+1;
         rect.MinX+=12;
         fit=TextFit(rp,hb->url,strlen(hb->url),&te,NULL,1,
            rect.MaxX-rect.MinX,rect.MaxY-rect.MinY+1);
         slack=(rect.MaxY-rect.MinY)-(te.te_Extent.MaxY-te.te_Extent.MinY);
         x=rect.MinX-te.te_Extent.MinX+1;
         y=rect.MinY-te.te_Extent.MinY+(slack+1)/2;
         Move(rp,x,y);
         Text(rp,hb->url,fit);
         te.te_Extent.MinX+=x;
         te.te_Extent.MinY+=y;
         te.te_Extent.MaxX+=x;
         te.te_Extent.MaxY+=y;
         SetAPen(rp,bpen);
         Fillouter(rp,&rect,&te.te_Extent);
      }

      SetSoftStyle(rp,0,0x0f);
      result=LBCB_OK;
   }
   return result;

    USRFUNC_EXIT
}

DECLARE_HOOK
(
static long __saveds   , Idcmphook,
struct Hook *,hook,A0,
APTR, dummy, A2,
struct IntuiMessage *,msg,A1
)
{
    USRFUNC_INIT
  switch(msg->Class)
   {  case IDCMP_RAWKEY:
         keycontrol=keyshift=FALSE;
         if(msg->Qualifier&(IEQUALIFIER_CONTROL)) keycontrol=TRUE;
         else if(msg->Qualifier&(IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) keyshift=TRUE;
         break;
      case IDCMP_CHANGEWINDOW:
         Updatetaskattrs(AOHOT_Dimx,window->LeftEdge,
            AOHOT_Dimy,window->TopEdge,
            AOHOT_Dimw,window->Width-window->BorderLeft-window->BorderRight,
            AOHOT_Dimh,window->Height-window->BorderTop-window->BorderBottom,
            TAG_END);
         break;
   }
   return 0;

    USRFUNC_EXIT
}

/*-----------------------------------------------------------------------*/

static void Buildhotlistwindow(struct Hotwindow *how)
{  short imgsize;
   NewList(&glist);
   NewList(&llist);
   NewList(&ltypelist);
   NewList(&lsortlist);
   ltypelabels[0]=AWEBSTR(MSG_HOTL_LTYPE_REST);
   ltypelabels[1]=AWEBSTR(MSG_HOTL_LTYPE_ALL);
   ltypelabels[2]=AWEBSTR(MSG_HOTL_LTYPE_GROUPS);
   ltypelabels[3]=NULL;
   lsortlabels[0]=AWEBSTR(MSG_HOTL_LSORT_DATE);
   lsortlabels[1]=AWEBSTR(MSG_HOTL_LSORT_TITLE);
   lsortlabels[2]=AWEBSTR(MSG_HOTL_LSORT_URL);
   lsortlabels[3]=NULL;
   Makechooserlist(&ltypelist,ltypelabels);
   Makechooserlist(&lsortlist,lsortlabels);
   grenderhook.h_Entry=(HOOKFUNC) GIVEME_HOOKENTRY(Renderhook);
   grenderhook.h_Data=(APTR)LIST_GROUPS;
   lrenderhook.h_Entry=(HOOKFUNC) GIVEME_HOOKENTRY(Renderhook);
   lrenderhook.h_Data=(APTR)LIST_LIST;
   listid=LIST_GROUPS;
   gtype=GTP_ALL;
   gselect=-1;
   ggroup=NULL;
   ltype=LTP_REST;
   lsort=LSRT_DATE;
   lselect=-1;
   urls=FALSE;
   wasfind=FALSE;
   idcmphook.h_Entry=(HOOKFUNC) GIVEME_HOOKENTRY(Idcmphook);
   idcmphook.h_Data=how;
   if((screen=LockPubScreen(how->screenname))
   && (dri=GetScreenDrawInfo(screen))
   && (leftarrow=LabelObject,
         LABEL_DrawInfo,dri,
         LABEL_Justification,LJ_LEFT,
         LABEL_DisposeImage,TRUE,
         LABEL_Image,GlyphObject,
            GLYPH_Glyph,GLYPH_LEFTARROW,
            GA_Width,dri->dri_Font->tf_YSize,
            GA_Height,dri->dri_Font->tf_YSize,
         End,
      End)
   && (rightarrow=LabelObject,
         LABEL_DrawInfo,dri,
         LABEL_Justification,LJ_RIGHT,
         LABEL_DisposeImage,TRUE,
         LABEL_Image,GlyphObject,
            GLYPH_Glyph,GLYPH_RIGHTARROW,
            GA_Width,dri->dri_Font->tf_YSize,
            GA_Height,dri->dri_Font->tf_YSize,
         End,
      End)
   )
   {  imgsize=((dri->dri_Font->tf_YSize+1)&~1)-1;
      showimg=DrawListObject,
         IA_Width,imgsize,
         IA_Height,MAX(7,imgsize-2),
         DRAWLIST_Directives,showdata,
         DRAWLIST_RefWidth,999,
         DRAWLIST_RefHeight,999,
      End;
      hideimg=DrawListObject,
         IA_Width,imgsize,
         IA_Height,MAX(7,imgsize-2),
         DRAWLIST_Directives,hidedata,
         DRAWLIST_RefWidth,999,
         DRAWLIST_RefHeight,999,
      End;
      Allocallnodes(how);
      Makeglist(how);
      Makellist(how);
      if(!how->w)
      {  how->x=screen->Width/4;
         how->y=screen->Height/4;
         how->w=screen->Width/2;
         how->h=screen->Height/2;
      }
      winobj=WindowObject,
         WA_Title,AWEBSTR(MSG_HOTL_TITLE_MANAGER),
         WA_Left,how->x,
         WA_Top,how->y,
         WA_InnerWidth,how->w,
         WA_InnerHeight,how->h,
         WA_SizeGadget,TRUE,
         WA_DepthGadget,TRUE,
         WA_DragBar,TRUE,
         WA_CloseGadget,TRUE,
         WA_Activate,TRUE,
         WA_AutoAdjust,TRUE,
         WA_SimpleRefresh,TRUE,
         WA_PubScreen,screen,
         WA_IDCMP,IDCMP_RAWKEY,
         WINDOW_IDCMPHook,&idcmphook,
         WINDOW_IDCMPHookBits,IDCMP_RAWKEY|IDCMP_CHANGEWINDOW,
         WINDOW_Layout,toplayout=VLayoutObject,
            LAYOUT_SpaceOuter,TRUE,
            LAYOUT_DeferLayout,TRUE,
            StartMember,HLayoutObject,
               StartMember,VLayoutObject,                /* Left column */
                  StartMember,gnamegad=ButtonObject,
                     GA_ReadOnly,TRUE,
                     GA_Text,"",
                  EndMember,
                  CHILD_WeightedHeight,0,
                  StartMember,glistgad=ListBrowserObject,
                     GA_ID,GID_GLIST,
                     GA_RelVerify,TRUE,
                     LISTBROWSER_Labels,&glist,
                     LISTBROWSER_ShowSelected,TRUE,
                     LISTBROWSER_Hierarchical,gtype==GTP_ALL,
                     LISTBROWSER_MultiSelect,TRUE,
                     LISTBROWSER_ShowImage,showimg,
                     LISTBROWSER_HideImage,hideimg,
                  EndMember,
                  StartMember,HLayoutObject,
                     StartMember,VLayoutObject,
                        StartMember,HLayoutObject,
                           LAYOUT_SpaceInner,FALSE,
                           StartMember,gupgad=ButtonObject,
                              GA_ID,GID_GUP,
                              GA_RelVerify,TRUE,
                              BUTTON_AutoButton,BAG_UPARROW,
                           EndMember,
                           CHILD_WeightedWidth,0,
                           StartMember,gdowngad=ButtonObject,
                              GA_ID,GID_GDOWN,
                              GA_RelVerify,TRUE,
                              BUTTON_AutoButton,BAG_DNARROW,
                           EndMember,
                           CHILD_WeightedWidth,0,
                           StartMember,gsortgad=ButtonObject,
                              GA_ID,GID_GSORT,
                              GA_RelVerify,TRUE,
                              GA_Text,AWEBSTR(MSG_HOTL_SORT),
                           EndMember,
                        EndMember,
                        StartMember,HLayoutObject,
                           StartMember,ButtonObject,
                              GA_ID,GID_GALL,
                              GA_RelVerify,TRUE,
                              GA_Text,AWEBSTR(MSG_HOTL_ALL),
                           EndMember,
                           StartMember,gparentgad=ButtonObject,
                              GA_ID,GID_PARENT,
                              GA_RelVerify,TRUE,
                              GA_Text,AWEBSTR(MSG_HOTL_PARENT),
                           EndMember,
                        EndMember,
                     EndMember,
                     CHILD_WeightedWidth,67,
                     StartMember,VLayoutObject,
                        StartMember,gaddegad=ButtonObject,
                           GA_ID,GID_ADDE,
                           GA_RelVerify,TRUE,
                           GA_Text,AWEBSTR(MSG_HOTL_ADDENTRY),
                        EndMember,
                        StartMember,gaddggad=ButtonObject,
                           GA_ID,GID_ADDG,
                           GA_RelVerify,TRUE,
                           GA_Text,AWEBSTR(MSG_HOTL_ADDGROUP2),
                        EndMember,
                     EndMember,
                     CHILD_WeightedWidth,33,
                  EndMember,
                  CHILD_WeightedHeight,0,
               EndMember,
               StartMember,VLayoutObject,                /* Center column */
                  StartMember,VLayoutObject,
                     LAYOUT_EvenSize,TRUE,
                     StartMember,idgad=ButtonObject,
                        GA_ID,GID_ID,
                        GA_RelVerify,TRUE,
                        GA_Image,leftarrow,
                     EndMember,
                     StartMember,mvingad=ButtonObject,
                        GA_ID,GID_MVIN,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_HOTL_MOVEIN),
                     EndMember,
                     StartMember,movegad=ButtonObject,
                        GA_ID,GID_MOVE,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_HOTL_MOVE),
                     EndMember,
                     StartMember,mvoutgad=ButtonObject,
                        GA_ID,GID_MVOUT,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_HOTL_MOVEOUT),
                     EndMember,
                     StartMember,delgad=ButtonObject,
                        GA_ID,GID_DEL,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_HOTL_DEL),
                     EndMember,
                     StartMember,wheregad=ButtonObject,
                        GA_ID,GID_WHERE,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_HOTL_WHERE),
                     EndMember,
                     StartMember,showgad=ButtonObject,
                        GA_ID,GID_SHOW,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_HOTL_SHOW),
                     EndMember,
                  EndMember,
                  CHILD_WeightedHeight,0,
                  StartMember,VLayoutObject,
                     StartMember,HLayoutObject,
                        StartMember,urlsgad=CheckBoxObject,
                           GA_ID,GID_URLS,
                           GA_RelVerify,TRUE,
                           GA_Text,AWEBSTR(MSG_HOTL_URLS),
                           GA_Selected,urls,
                        EndMember,
                        CHILD_WeightedWidth,0,
                     EndMember,
                     StartMember,ButtonObject,
                        GA_ID,GID_REVERT,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_HOTL_REVERT),
                     EndMember,
                     StartMember,ButtonObject,
                        GA_ID,GID_SAVE,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_HOTL_SAVE),
                     EndMember,
                  EndMember,
                  CHILD_WeightedHeight,0,
               EndMember,
               CHILD_WeightedWidth,0,
               StartMember,VLayoutObject,                /* Right column */
                  StartMember,lnamegad=ButtonObject,
                     GA_ReadOnly,TRUE,
                     GA_Text,"",
                  EndMember,
                  CHILD_WeightedHeight,0,
                  StartMember,llistgad=ListBrowserObject,
                     GA_ID,GID_LLIST,
                     GA_RelVerify,TRUE,
                     LISTBROWSER_Labels,&llist,
                     LISTBROWSER_ShowSelected,TRUE,
                     LISTBROWSER_MultiSelect,TRUE,
                  EndMember,
                  StartMember,HLayoutObject,
                     StartMember,VLayoutObject,
                        StartMember,ButtonObject,
                           GA_ID,GID_LADD,
                           GA_RelVerify,TRUE,
                           GA_Text,AWEBSTR(MSG_HOTL_LADD),
                        EndMember,
                        StartMember,HLayoutObject,
                           LAYOUT_SpaceInner,FALSE,
                           StartMember,findgad=ButtonObject,
                              GA_ID,GID_FIND,
                              GA_RelVerify,TRUE,
                              GA_Text,AWEBSTR(MSG_HOTL_FIND),
                              BUTTON_PushButton,TRUE,
                           EndMember,
                           StartMember,patgad=ButtonObject,
                              GA_ID,GID_PATTERN,
                              GA_RelVerify,TRUE,
                              GA_Text,AWEBSTR(MSG_HOTL_PATTERN),
                              BUTTON_PushButton,TRUE,
                           EndMember,
                        EndMember,
                     EndMember,
                     StartMember,VLayoutObject,
                        StartMember,ltypegad=ChooserObject,
                           GA_ID,GID_LTYPE,
                           GA_RelVerify,TRUE,
                           CHOOSER_PopUp,TRUE,
                           CHOOSER_Labels,&ltypelist,
                           CHOOSER_Active,ltype,
                        EndMember,
                        MemberLabel(AWEBSTR(MSG_HOTL_LTYPE)),
                        StartMember,lsortgad=ChooserObject,
                           GA_ID,GID_LSORT,
                           GA_RelVerify,TRUE,
                           CHOOSER_PopUp,TRUE,
                           CHOOSER_Labels,&lsortlist,
                           CHOOSER_Active,lsort,
                        EndMember,
                        MemberLabel(AWEBSTR(MSG_HOTL_LSORT)),
                     EndMember,
                  EndMember,
                  CHILD_WeightedHeight,0,
               EndMember,
            EndMember,
            StartMember,pagelayout=HLayoutObject,
               StartMember,pagegad=PageObject,
                  PAGE_Add,HLayoutObject,
                     StartMember,namegad=StringObject,
                        GA_ID,GID_NAME,
                        GA_RelVerify,TRUE,
                        GA_TabCycle,TRUE,
                        STRINGA_TextVal,"",
                        STRINGA_MaxChars,127,
                     EndMember,
                     MemberLabel(AWEBSTR(MSG_HOTL_NAME)),
                     StartMember,urlgad=StringObject,
                        GA_ID,GID_URL,
                        GA_RelVerify,TRUE,
                        GA_TabCycle,TRUE,
                        STRINGA_TextVal,"",
                        STRINGA_MaxChars,127,
                     EndMember,
                     MemberLabel(AWEBSTR(MSG_HOTL_URL)),
                  EndMember,
                  PAGE_Add,HLayoutObject,
                     StartMember,SpaceObject,
                     EndMember,
                     CHILD_WeightedWidth,50,
                     StartMember,fstrgad=StringObject,
                        GA_ID,GID_FSTR,
                        GA_RelVerify,TRUE,
                        STRINGA_TextVal,"",
                        STRINGA_MaxChars,127,
                     EndMember,
                     CHILD_WeightedWidth,40,
                     StartMember,ButtonObject,
                        GA_ID,GID_NEXT,
                        GA_RelVerify,TRUE,
                        GA_Text,AWEBSTR(MSG_HOTL_NEXT),
                     EndMember,
                     CHILD_WeightedWidth,10,
                     CHILD_WeightedHeight,0,
                  EndMember,
                  PAGE_Add,HLayoutObject,
                     StartMember,SpaceObject,
                     EndMember,
                     StartMember,pstrgad=StringObject,
                        GA_ID,GID_PSTR,
                        GA_RelVerify,TRUE,
                        STRINGA_TextVal,"",
                        STRINGA_MaxChars,127,
                     EndMember,
                  EndMember,
               EndMember,
            EndMember,
            CHILD_WeightedHeight,0,
         EndMember,
      EndWindow;
      if(winobj)
      {  if(window=RA_OpenWindow(winobj))
         {  GetAttr(WINDOW_SigMask,winobj,&winsigmask);
            Setgadgets(how);
         }
      }
   }
}

static BOOL Handlehotlistwindow(struct Hotwindow *how)
{  ULONG result,relevent;
   BOOL done=FALSE;
   UWORD click;
   short v;
   while((result=RA_HandleInput(winobj,&click))!=WMHI_LASTMSG)
   {  if(!locked)
      {  ObtainSemaphore(how->hotsema);
         switch(result&WMHI_CLASSMASK)
         {  case WMHI_CLOSEWINDOW:
               done=TRUE;
               break;
            case WMHI_GADGETUP:
               switch(result&WMHI_GADGETMASK)
               {
                  case GID_ID:
                     Setlistid(how,listid==LIST_LIST?LIST_GROUPS:LIST_LIST);
                     break;
                  case GID_MVIN:
                     Movein(how,FALSE);
                     break;
                  case GID_MOVE:
                     Movein(how,TRUE);
                     break;
                  case GID_MVOUT:
                     Moveout(how);
                     break;
                  case GID_DEL:
                     Deleteentries(how);
                     break;
                  case GID_WHERE:
                     Where(how);
                     break;
                  case GID_SHOW:
                     Show(how);
                     break;
                  case GID_URLS:
                     v=Getvalue(urlsgad,GA_Selected);
                     if(v!=urls)
                     {  urls=v;
                        if(gtype==GTP_GROUP) Updateglist(how);
                        if(ltype!=LTP_GROUPS) Updatellist(how);
                     }
                     break;
                  case GID_REVERT:
                     Updatetaskattrs(AOTSK_Async,TRUE,
                        AOHOT_Restore,TRUE,
                        TAG_END);
                     break;
                  case GID_SAVE:
                     ReleaseSemaphore(how->hotsema);
                     Updatetaskattrs(AOHOT_Save,TRUE,TAG_END);
                     ObtainSemaphore(how->hotsema);
                     break;

                  case GID_GLIST:
                     relevent=Getvalue(glistgad,LISTBROWSER_RelEvent);
                     if(relevent&LBRE_SHOWCHILDREN)
                     {  Showgchildren(click,TRUE);
                     }
                     else if(relevent&LBRE_HIDECHILDREN)
                     {  Showgchildren(click,FALSE);
                     }
                     else if((relevent&LBRE_DOUBLECLICK) && click==gselect)
                     {  Doubleclickg(how);
                     }
                     else
                     {  gselect=click;
                     }
                     Setlistid(how,LIST_GROUPS);
                     break;
                  case GID_GALL:
                     Setglist(how,NULL);
                     break;
                  case GID_PARENT:
                     Setglist(how,ggroup?ggroup->owner:NULL);
                     break;
                  case GID_GUP:
                     Moveglist(how,-1);
                     break;
                  case GID_GDOWN:
                     Moveglist(how,1);
                     break;
                  case GID_GSORT:
                     Sortglist(how);
                     break;
                  case GID_ADDG:
                     Gaddg(how);
                     break;
                  case GID_ADDE:
                     Gadde(how);
                     break;

                  case GID_LLIST:
                     relevent=Getvalue(llistgad,LISTBROWSER_RelEvent);
                     if((relevent&LBRE_DOUBLECLICK) && click==lselect)
                     {  Doubleclickl(how);
                     }
                     else
                     {  lselect=click;
                     }
                     Setlistid(how,LIST_LIST);
                     wasfind=FALSE;
                     break;
                  case GID_LADD:
                     Ladd(how);
                     break;
                  case GID_LTYPE:
                     v=Getvalue(ltypegad,CHOOSER_Active);
                     if(v!=ltype)
                     {  ltype=v;
                        lselect=Updatellist(how);
                        Setlistid(how,LIST_LIST);
                     }
                     break;
                  case GID_LSORT:
                     v=Getvalue(lsortgad,CHOOSER_Active);
                     if(v!=lsort)
                     {  lsort=v;
                        lselect=Updatellist(how);
                        Setlistid(how,LIST_LIST);
                     }
                     break;
                  case GID_FIND:
                     if(findgad->Flags&GFLG_SELECTED)
                     {  Setgadgetattrs(patgad,window,NULL,GA_Selected,FALSE,TAG_END);
                        SetGadgetAttrs(pagegad,window,NULL,PAGE_Current,PG_FIND,TAG_END);
                     }
                     else
                     {  SetGadgetAttrs(pagegad,window,NULL,PAGE_Current,PG_EDIT,TAG_END);
                     }
                     RethinkLayout(pagelayout,window,NULL,TRUE);
                     if(findgad->Flags&GFLG_SELECTED)
                     {  ActivateLayoutGadget(toplayout,window,NULL,(ULONG) fstrgad);
                     }
                     break;
                  case GID_PATTERN:
                     if(patgad->Flags&GFLG_SELECTED)
                     {  Setgadgetattrs(findgad,window,NULL,GA_Selected,FALSE,TAG_END);
                        SetGadgetAttrs(pagegad,window,NULL,PAGE_Current,PG_PATTERN,TAG_END);
                     }
                     else
                     {  SetGadgetAttrs(pagegad,window,NULL,PAGE_Current,PG_EDIT,TAG_END);
                     }
                     RethinkLayout(pagelayout,window,NULL,TRUE);
                     if(patgad->Flags&GFLG_SELECTED)
                     {  ActivateLayoutGadget(toplayout,window,NULL,(ULONG) pstrgad);
                     }
                     break;
                  case GID_NAME:
                     Setname(how,namegad,0);
                     break;
                  case GID_URL:
                     Setname(how,urlgad,1);
                     break;
                  case GID_FSTR:
                  case GID_NEXT:
                     Selectfind(how);
                     break;
                  case GID_PSTR:
                     Selectpattern(how);
                     break;
               }
               break;
            case WMHI_RAWKEY:
               switch(result&WMHI_GADGETMASK)
               {  case 0x45:  /* esc */
                     done=TRUE;
                     break;
                  case 0x41:  /* bs */
                     Setglist(how,ggroup?ggroup->owner:NULL);
                     break;
                  case 0x43:  /* num enter */
                  case 0x44:  /* enter */
                     if(listid==LIST_GROUPS) Doubleclickg(how);
                     else Doubleclickl(how);
                     break;
                  case 0x4c:  /* up */
                     if(keycontrol)
                     {  if(listid==LIST_GROUPS) Moveglist(how,-1);
                     }
                     else Moveselected(how,-1,keyshift);
                     break;
                  case 0x4d:  /* down */
                     if(keycontrol)
                     {  if(listid==LIST_GROUPS) Moveglist(how,1);
                     }
                     else Moveselected(how,1,keyshift);
                     break;
                  case 0x4e:  /* right */
                     if(keyshift) Moveout(how);
                     else Setlistid(how,LIST_LIST);
                     break;
                  case 0x4f:  /* left */
                     if(keyshift) Movein(how,FALSE);
                     else Setlistid(how,LIST_GROUPS);
                     break;
               }
               break;
/*
            case WMHI_CHANGEWINDOW:
               Updatetaskattrs(AOHOT_Dimx,window->LeftEdge,
                  AOHOT_Dimy,window->TopEdge,
                  AOHOT_Dimw,window->Width-window->BorderLeft-window->BorderRight,
                  AOHOT_Dimh,window->Height-window->BorderTop-window->BorderBottom,
                  TAG_END);
               break;
*/
         }
         ReleaseSemaphore(how->hotsema);
      }
   }
   return done;
}

static void Closehotlistwindow(struct Hotwindow *how)
{  if(winobj)
   {  if(window)
      {  Updatetaskattrs(AOHOT_Dimx,window->LeftEdge,
            AOHOT_Dimy,window->TopEdge,
            AOHOT_Dimw,window->Width-window->BorderLeft-window->BorderRight,
            AOHOT_Dimh,window->Height-window->BorderTop-window->BorderBottom,
            TAG_END);
      }
      DisposeObject(winobj);
   }
   Freeallnodes(how);
   Freechooserlist(&ltypelist);
   Freechooserlist(&lsortlist);
   if(showimg) DisposeObject(showimg);showimg=NULL;
   if(hideimg) DisposeObject(hideimg);hideimg=NULL;
   if(dri) FreeScreenDrawInfo(screen,dri);dri=NULL;
   if(screen) UnlockPubScreen(NULL,screen);screen=NULL;
   if(leftarrow) DisposeObject(leftarrow);
   if(rightarrow) DisposeObject(rightarrow);
}

__saveds void Hotmgrtask(struct Hotwindow *how)
{
   struct Taskmsg *hm;
   ULONG done=FALSE;
   ULONG getmask;
   struct TagItem *tag,*tstate;
   Buildhotlistwindow(how);
   if(window)
   {  while(!done)
      {  getmask=Waittask(winsigmask);
         while(!done && (hm=Gettaskmsg()))
         {  if(hm->amsg && hm->amsg->method==AOM_SET)
            {  tstate=((struct Amset *)hm->amsg)->tags;
               while(tag=NextTagItem(&tstate))
               {  switch(tag->ti_Tag)
                  {  case AOTSK_Stop:
                        if(tag->ti_Data) done=TRUE;
                        break;
                     case AOHOT_Tofront:
                        WindowToFront(window);
                        ActivateWindow(window);
                        break;
                     case AOHOT_Addentry:
                        ObtainSemaphore(how->hotsema);
                        Updatellist(how);
                        ReleaseSemaphore(how->hotsema);
                        break;
                     case AOHOT_Dispose:
                        ObtainSemaphore(how->hotsema);
                        Disposelist(how);
                        ReleaseSemaphore(how->hotsema);
                        locked=TRUE;
                        break;
                     case AOHOT_Newlist:
                        ObtainSemaphore(how->hotsema);
                        Newhotlist(how);
                        ReleaseSemaphore(how->hotsema);
                        locked=FALSE;
                        break;
                  }
               }
            }
            Replytaskmsg(hm);
         }
         if(!done && (getmask&winsigmask))
         {  done=Handlehotlistwindow(how);
         }
      }
      Closehotlistwindow(how);
   }
   Updatetaskattrs(AOTSK_Async,TRUE,
      AOHOT_Close,TRUE,
      TAG_END);
}
