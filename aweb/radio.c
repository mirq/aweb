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

/* radio.c - AWeb html document radio form field element object */

#include "aweb.h"
#include "radio.h"
#include "form.h"
#include "application.h"
#include "jslib.h"
#include <reaction/reaction.h>
#include <intuition/imageclass.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

struct Radio
{  struct Field field;
   UWORD rflags;
   UBYTE *onclick;
   UBYTE *onfocus;
   UBYTE *onblur;
};

#define RADF_CHECKED    0x0001   /* Radio is in selected state */
#define RADF_INITIAL    0x0002   /* Radio was initially selected */

static void *radiobutton;
static long radiobuttonw,radiobuttonh;

static void *sizeradio;          /* Dummy single object to catch pres changes */

/*------------------------------------------------------------------------*/

/* Determine the size */
static void Radiosize(void)
{  short type=prefs.browser.styles[STYLE_NORMAL].fonttype;
   short size=prefs.browser.styles[STYLE_NORMAL].fontsize;
   struct TextFont *font=prefs.browser.font[type][size-1].font;
   radiobuttonh=font->tf_Baseline;
   radiobuttonw=(radiobuttonh*6+4)/5;
   SetAttrs(radiobutton,
      IA_Width,radiobuttonw,
      IA_Height,radiobuttonh,
      TAG_END);
}

/*------------------------------------------------------------------------*/

/* Perform the actions for clicking the radiobutton. First select, then run
   the onclick script and set back. */
static void Clickradio(struct Radio *rad,BOOL testjs)
{  struct Radio *oldrad=NULL;
   if(!(rad->rflags&RADF_CHECKED))
   {  if(testjs)
      {  Agetattrs(rad->field.form,AOFOR_Radioname,(Tag)rad->field.name,AOFOR_Radioselected,(Tag)&oldrad,TAG_END);
      }
      rad->rflags|=RADF_CHECKED;
      Arender((struct Aobject *)rad,NULL,0,0,AMRMAX,AMRMAX,0,NULL);
      Amethod(rad->field.form,AFO_RADIOSELECT,(Tag)rad,(Tag)rad->field.name);
   }
   else oldrad=rad;
   if(testjs && !Runjavascriptwith(rad->field.elt.cframe,awebonclick,&rad->field.jobject,rad->field.form))
   {  if(oldrad)
      {  if(oldrad!=rad) Clickradio(oldrad,FALSE);
      }
      else
      {  rad->rflags&=~RADF_CHECKED;
         Arender((struct Aobject *)rad,NULL,0,0,AMRMAX,AMRMAX,0,NULL);
      }
   }
}

/*------------------------------------------------------------------------*/

/* Get or set value property (JS) */
static BOOL Propertyvalue(struct Varhookdata *vd)
{  BOOL result=FALSE;
   struct Radio *rad=vd->hookdata;
   UBYTE *value;
   if(rad)
   {  switch(vd->code)
      {  case VHC_SET:
            if(value=Jtostring(vd->jc,vd->value))
            {  if(rad->field.value) FREE(rad->field.value);
               rad->field.value=Dupstr(value,-1);
            }
            result=TRUE;
            break;
         case VHC_GET:
            Jasgstring(vd->jc,vd->value,rad->field.value);
            result=TRUE;
            break;
      }
   }
   return result;
}

/* Get or set checked property (JS) */
static BOOL Propertychecked(struct Varhookdata *vd)
{  BOOL result=FALSE;
   struct Radio *rad=vd->hookdata;
   BOOL checked;
   if(rad)
   {  switch(vd->code)
      {  case VHC_SET:
            checked=Jtoboolean(vd->jc,vd->value);
            SETFLAG(rad->rflags,RADF_CHECKED,checked);
            Arender((struct Aobject *)rad,NULL,0,0,AMRMAX,AMRMAX,0,NULL);
            if(checked) Amethod(rad->field.form,AFO_RADIOSELECT,(Tag)rad,(Tag)rad->field.name);
            result=TRUE;
            break;
         case VHC_GET:
            Jasgboolean(vd->jc,vd->value,BOOLVAL(rad->rflags&RADF_CHECKED));
            result=TRUE;
            break;
      }
   }
   return result;
}

/* Javascript methods */
static void Methodclick(struct Jcontext *jc)
{  struct Radio *rad=Jointernal(Jthis(jc));
   if(rad) Clickradio(rad,FALSE);
}

static void Methodfocus(struct Jcontext *jc)
{
}

static void Methodblur(struct Jcontext *jc)
{
}

static void Methodtostring(struct Jcontext *jc)
{  struct Radio *rad=Jointernal(Jthis(jc));
   struct Buffer buf={0};
   if(rad)
   {  Addtagstr(&buf,"<input",ATSF_NONE,0);
      Addtagstr(&buf,"type",ATSF_STRING,"radio");
      if(rad->field.name) Addtagstr(&buf,"name",ATSF_STRING,rad->field.name);
      if(rad->field.value) Addtagstr(&buf,"value",ATSF_STRING,rad->field.value);
      if(rad->rflags&RADF_INITIAL) Addtagstr(&buf,"checked",ATSF_NONE,0);
      Addtobuffer(&buf,">",2);
      Jasgstring(jc,NULL,buf.buffer);
      Freebuffer(&buf);
   }
}

/*------------------------------------------------------------------------*/

static long Measureradio(struct Radio *rad,struct Ammeasure *amm)
{  rad->field.elt.aow=radiobuttonw;
   rad->field.elt.aoh=radiobuttonh;
   AmethodasA(AOTP_FIELD,(struct Aobject *)rad,(struct Amessage *)amm);
   return 0;
}

static long Renderradio(struct Radio *rad,struct Amrender *amr)
{  struct Coords *coo,coords={0};
   BOOL clip=FALSE;
   ULONG clipkey=0;
   struct RastPort *rp;
   struct DrawInfo *drinfo=(struct DrawInfo *)Agetattr(Aweb(),AOAPP_Drawinfo);
   UWORD state=IDS_NORMAL;
   if(rad->rflags&RADF_CHECKED) state=IDS_SELECTED;
   if(rad->field.elt.eltflags&ELTF_ALIGNED)
   {  if(!(coo=amr->coords))
      {  Framecoords(rad->field.elt.cframe,&coords);
         coo=&coords;
         clip=TRUE;
      }
      if(coo->rp)
      {  rp=coo->rp;
         if(clip) clipkey=Clipto(rp,coo->minx,coo->miny,coo->maxx,coo->maxy);
         DrawImageState(rp,radiobutton,rad->field.elt.aox+coo->dx,rad->field.elt.aoy+coo->dy,state,drinfo);
         if(clip) Unclipto(clipkey);
      }
   }
   return 0;
}

static long Setradio(struct Radio *rad,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=Amethodas(AOTP_FIELD,rad,AOM_SET,(Tag)ams->tags);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AORAD_Checked:
            if(tag->ti_Data) rad->rflags|=RADF_CHECKED;
            else rad->rflags&=~RADF_CHECKED;
            break;
         case AOFLD_Reset:
            if(rad->rflags&RADF_INITIAL) rad->rflags|=RADF_CHECKED;
            else rad->rflags&=~RADF_CHECKED;
            Arender((struct Aobject *)rad,NULL,0,0,AMRMAX,AMRMAX,0,NULL);
            break;
         case AOFLD_Onclick:
            if(rad->onclick) FREE(rad->onclick);
            rad->onclick=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOFLD_Onfocus:
            if(rad->onfocus) FREE(rad->onfocus);
            rad->onfocus=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOFLD_Onblur:
            if(rad->onblur) FREE(rad->onblur);
            rad->onblur=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOAPP_Browsersettings:
            Radiosize();
            break;
      }
   }
   if(!rad->field.value)
   {  rad->field.value=Dupstr("On",-1);
   }
   return result;
}

static long Getradio(struct Radio *rad,struct Amset *ams)
{  long result;
   struct TagItem *tag,*tstate=ams->tags;
   result=AmethodasA(AOTP_FIELD,(struct Aobject *)rad,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AORAD_Checked:
            PUTATTR(tag,BOOLVAL(rad->rflags&RADF_CHECKED));
            break;
         case AOFLD_Name:
            /* NULLify name and value set by superclass if unselected */
            if(!(rad->rflags&RADF_CHECKED))
            {  PUTATTR(tag,NULL);
            }
            break;
         case AOFLD_Value:
            if(!(rad->rflags&RADF_CHECKED))
            {  PUTATTR(tag,NULL);
            }
            break;
      }
   }
   return result;
}

static struct Radio *Newradio(struct Amset *ams)
{  struct Radio *rad;
   if(rad=Allocobject(AOTP_RADIO,sizeof(struct Radio),ams))
   {  Setradio(rad,ams);
      if(rad->rflags&RADF_CHECKED) rad->rflags|=RADF_INITIAL;
   }
   return rad;
}

static long Hittestradio(struct Radio *rad,struct Amhittest *amh)
{  long result;
   if(amh->oldobject==(struct Aobject *)rad)
   {  result=AMHR_OLDHIT;
   }
   else
   {  result=AMHR_NEWHIT;
      if(amh->amhr)
      {  amh->amhr->object=rad;
      }
   }
   return result;
}

static long Goactiveradio(struct Radio *rad,struct Amgoactive *amg)
{  Clickradio(rad,rad->onclick || AWebJSBase);
   return AMR_CHANGED;
}

static long Dragtestradio(struct Radio *rad,struct Amdragtest *amd)
{  return AMDR_STOP;
}

static long Jsetupradio(struct Radio *rad,struct Amjsetup *amj)
{  struct Jvar *jv;
   AmethodasA(AOTP_FIELD,(struct Aobject *)rad,(struct Amessage *)amj);
   if(rad->field.jobject)
   {  if(jv=Jproperty(amj->jc,rad->field.jobject,"value"))
      {  Setjproperty(jv,Propertyvalue,rad);
      }
      if(jv=Jproperty(amj->jc,rad->field.jobject,"type"))
      {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
         Jasgstring(amj->jc,jv,"radio");
      }
      if(jv=Jproperty(amj->jc,rad->field.jobject,"defaultChecked"))
      {  Setjproperty(jv,JPROPHOOK_READONLY,NULL);
         Jasgboolean(amj->jc,jv,BOOLVAL(rad->rflags&RADF_INITIAL));
      }
      if(jv=Jproperty(amj->jc,rad->field.jobject,"checked"))
      {  Setjproperty(jv,Propertychecked,rad);
      }
      Addjfunction(amj->jc,rad->field.jobject,"click",Methodclick,NULL);
      Addjfunction(amj->jc,rad->field.jobject,"focus",Methodfocus,NULL);
      Addjfunction(amj->jc,rad->field.jobject,"blur",Methodblur,NULL);
      Addjfunction(amj->jc,rad->field.jobject,"toString",Methodtostring,NULL);
      Jaddeventhandler(amj->jc,rad->field.jobject,"onfocus",rad->onfocus);
      Jaddeventhandler(amj->jc,rad->field.jobject,"onblur",rad->onblur);
      Jaddeventhandler(amj->jc,rad->field.jobject,"onclick",rad->onclick);
   }
   return 0;
}

static void Disposeradio(struct Radio *rad)
{  if(rad->onblur) FREE(rad->onblur);
   if(rad->onclick) FREE(rad->onclick);
   if(rad->onfocus) FREE(rad->onfocus);
   Amethodas(AOTP_FIELD,rad,AOM_DISPOSE);
}

static void Deinstallradio(void)
{  if(sizeradio)
   {  Aremchild(Aweb(),sizeradio,AOREL_APP_USE_BROWSER);
      Adisposeobject(sizeradio);
   }
   if(radiobutton) DisposeObject(radiobutton);
}

USRFUNC_H2
(
static long  , Radio_Dispatcher,
struct Radio *,rad,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newradio((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setradio(rad,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getradio(rad,(struct Amset *)amsg);
         break;
      case AOM_MEASURE:
         result=Measureradio(rad,(struct Ammeasure *)amsg);
         break;
      case AOM_RENDER:
         result=Renderradio(rad,(struct Amrender *)amsg);
         break;
      case AOM_HITTEST:
         result=Hittestradio(rad,(struct Amhittest *)amsg);
         break;
      case AOM_GOACTIVE:
         result=Goactiveradio(rad,(struct Amgoactive *)amsg);
         break;
      case AOM_DRAGTEST:
         result=Dragtestradio(rad,(struct Amdragtest *)amsg);
         break;
      case AOM_JSETUP:
         result=Jsetupradio(rad,(struct Amjsetup *)amsg);
         break;
      case AOM_DISPOSE:
         Disposeradio(rad);
         break;
      case AOM_DEINSTALL:
         Deinstallradio();
         break;
      default:
         result=AmethodasA(AOTP_FIELD,(struct Aobject *)rad,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installradio(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_RADIO,(Tag)Radio_Dispatcher)) return FALSE;
   if(!(radiobutton=GlyphObject,
      GLYPH_Glyph,GLYPH_RADIOBUTTON,
      End)) return FALSE;
   return TRUE;
}

BOOL Initradio(void)
{  Radiosize();
   if(!(sizeradio=Anewobject(AOTP_RADIO,TAG_END))) return FALSE;
   Aaddchild(Aweb(),sizeradio,AOREL_APP_USE_BROWSER);
   return TRUE;
}
