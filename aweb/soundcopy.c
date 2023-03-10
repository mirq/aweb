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

/* soundcopy.c - AWeb sound copy driver object */

#include "aweb.h"
#include "soundprivate.h"
#include "copydriver.h"
#include "task.h"
#include <datatypes/soundclass.h>
#include <devices/ahi.h>
#include <devices/audio.h>
#include <exec/execbase.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/datatypes.h>
#include <proto/intuition.h>

/*------------------------------------------------------------------------*/

struct Sndcopy
{  struct Copydriver cdv;
   APTR copy;
   struct Sndsource *source;
   LONG loop;                    /* How many times to loop */
   ULONG flags;
   APTR task;
};

#define SNDF_WANTPLAY      0x0001   /* We want to play and wait for source */

/*------------------------------------------------------------------------*/

struct Sndtask
{
   APTR dto;                    /* Sound datatype object */
   ULONG samplength,freq,volume,cycles; /* Sound dto details */
   ULONG period;
   LONG loop;                    /* Nr of times yet to go */
   struct MsgPort *ioport;
   struct AHIRequest *ahireq[2];
   ULONG flags;
   APTR dtsample;

   struct IOAudio *audev;
   UBYTE *sample[2];
   struct IOAudio *audio[2];
   LONG nextpos;
   LONG nextio;
   ULONG sampletype;
};

#define SNTF_PLAYING       0x0002   /* Currently playing through audio device */

#define MAXLENGTH 131072

static void PlaybufAHI(struct Sndtask *st, struct AHIRequest *req, APTR link)
{
   req->ahir_Std.io_Command = CMD_WRITE;
   req->ahir_Std.io_Offset = 0;
   req->ahir_Std.io_Data = st->dtsample;
   req->ahir_Std.io_Length = st->samplength;
   req->ahir_Link = link;
   SendIO((struct IORequest *)req);
}

static void PlaysbufAHI(struct Sndtask *st)
{
   LONG loop, pending;

   loop = st->loop - 1;
   pending = 1;

   PlaybufAHI(st, st->ahireq[0], NULL);

   if (loop > 0)
   {
      loop--;
      pending = 2;
      PlaybufAHI(st, st->ahireq[1], st->ahireq[0]);
   }

   for (;;)
   {
      struct AHIRequest *io;

      if (Waittask(1 << st->ioport->mp_SigBit | SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
         break;

      if ((io = (struct AHIRequest *)GetMsg(st->ioport)))
      {
         if (GetMsg(st->ioport))
         {
            if (loop <= 0)
               break;

            pending = 1;
            loop--;

            PlaybufAHI(st, st->ahireq[0], NULL);

            if (loop > 0)
            {
               loop--;
               pending = 2;
               PlaybufAHI(st, st->ahireq[1], st->ahireq[0]);
            }
         }
         else
         {
            pending--;

            if (loop <= 0)
            {
               if (!pending)
                  break;
               else
                  continue;
            }

            pending++;
            loop--;

            PlaybufAHI(st, io, io == st->ahireq[0] ? st->ahireq[1] : st->ahireq[0]);
         }
      }
   }

   if (!CheckIO((struct IORequest *)st->ahireq[0]))
   {
      AbortIO((struct IORequest *)st->ahireq[0]);
      WaitIO((struct IORequest *)st->ahireq[0]);
   }

   if (st->loop > 1 && !CheckIO((struct IORequest *)st->ahireq[1]))
   {
      AbortIO((struct IORequest *)st->ahireq[1]);
      WaitIO((struct IORequest *)st->ahireq[1]);
   }
}

static ULONG AHIPlay(struct Sndtask *st)
{
   ULONG rc = FALSE;

   if ((st->ahireq[0] = (struct AHIRequest *)CreateIORequest(st->ioport,sizeof(struct AHIRequest))))
   {
      if ((st->ahireq[1] = (struct AHIRequest *)CreateIORequest(st->ioport,sizeof(struct AHIRequest))))
      {
         st->ahireq[0]->ahir_Version = 2;

         if (!OpenDevice("ahi.device", 0, (struct IORequest *)st->ahireq[0], 0))
         {
            #if !defined(__MORPHOS__) && !defined(__amigaos4__)
            st->freq = st->period ? 3546897/st->period : 8000;
            #endif

            #if !defined(__MORPHOS__)
            st->ahireq[0]->ahir_Type = AHIST_M8S;
            #else
            st->ahireq[0]->ahir_Type = st->sampletype == SDTST_M8S ? AHIST_M8S : st->sampletype == SDTST_S8S ? AHIST_S8S : st->sampletype == SDTST_M16S ? AHIST_M16S : AHIST_S16S;
            #endif
            st->ahireq[0]->ahir_Frequency = st->freq;
            st->ahireq[0]->ahir_Volume = 0x10000;
            st->ahireq[0]->ahir_Position = 0x8000;

            CopyMem(st->ahireq[0], st->ahireq[1], sizeof(struct AHIRequest));
            PlaysbufAHI(st);
            CloseDevice((struct IORequest *)st->ahireq[0]);

            rc = TRUE;
         }
         DeleteIORequest((struct IORequest *)st->ahireq[1]);
      }
      DeleteIORequest((struct IORequest *)st->ahireq[0]);
   }

   return rc;
}

#if defined(__MORPHOS__)
static ULONG convert_sample(UBYTE *src, UBYTE *dst, ULONG len, ULONG type)
{
   ULONG i, samples;

   switch (type)
   {
      case SDTST_M8S:
         samples = len;
         memcpy(dst, src, len);
         break;

      case SDTST_S8S:
         samples = len / 2;

         for (i = 0; i < len; i += 2)
         {
            *dst = (src[0] + src[1]) / 2;
            dst++;
            src += 2;
         }
         break;

      case SDTST_M16S:
         {
            WORD *s = (WORD *)src;

            samples = len / 4;

            for (i = 0; i < len; i += 4)
            {
               *dst = *s / 2;
               dst++;
               s++;
            }
         }
         break;

      case SDTST_S16S:
         {
            WORD *s = (WORD *)src;

            samples = len / 4;

            for (i = 0; i < len; i += 4)
            {
               *dst = (s[0] + s[1]) / 4;
               dst++;
               s += 2;
            }
         }
         break;
   }

   return samples;
}
#endif

/* Write the next buffer */
static void Startdbuf(struct Sndtask *st,short n)
{
   long len=MIN(MAXLENGTH,st->samplength-st->nextpos);

   if (len)
   {
      ULONG samples;

      #if defined(__MORPHOS__)
      samples = convert_sample(st->dtsample+st->nextpos, st->sample[n], len, st->sampletype);
      #else
      memmove(st->sample[n],st->dtsample+st->nextpos,len);
      samples = len;
      #endif
      *st->audio[n]=*st->audev;
      st->audio[n]->ioa_Request.io_Command=CMD_WRITE;
      st->audio[n]->ioa_Request.io_Flags=ADIOF_PERVOL;
      st->audio[n]->ioa_Data=st->sample[n];
      st->audio[n]->ioa_Length=samples;
      st->audio[n]->ioa_Period=st->period;
      st->audio[n]->ioa_Cycles=1;
      st->audio[n]->ioa_Volume=st->volume;
      BeginIO((struct IORequest *)st->audio[n]);
      st->nextpos+=len;
      if(st->nextpos>=st->samplength)
      {
         st->nextpos=0;
         st->loop--;
      }
   }
}

static void Playdbuf(struct Sndtask *st)
{
   if((st->sample[0]=AllocMem(MAXLENGTH,MEMF_CHIP))
   && (st->sample[1]=AllocMem(MAXLENGTH,MEMF_CHIP))
   && (st->audio[0]=(struct IOAudio *)CreateIORequest(st->ioport,sizeof(struct IOAudio)))
   && (st->audio[1]=(struct IOAudio *)CreateIORequest(st->ioport,sizeof(struct IOAudio))))
   {
      st->nextio=0;
      st->nextpos=0;
      Startdbuf(st,0);
      Startdbuf(st,1);
      while(st->loop)
      {  Waittask(1<<st->ioport->mp_SigBit | SIGBREAKF_CTRL_C);
         if(Checktaskbreak()) break;
         WaitIO((struct IORequest *)st->audio[st->nextio]);   /* Should be this one that's ready */
         Startdbuf(st,st->nextio);
         st->nextio=1-st->nextio;
      }
      /* Now wait until both buffers are finished playing, or we are killed. */
      if(!Checktaskbreak()) Waittask(1<<st->ioport->mp_SigBit | SIGBREAKF_CTRL_C);
      if(Checktaskbreak()) AbortIO((struct IORequest *)st->audio[st->nextio]);
      WaitIO((struct IORequest *)st->audio[st->nextio]);
      st->nextio=1-st->nextio;
      if(!Checktaskbreak()) Waittask(1<<st->ioport->mp_SigBit | SIGBREAKF_CTRL_C);
      if(Checktaskbreak()) AbortIO((struct IORequest *)st->audio[st->nextio]);
      WaitIO((struct IORequest *)st->audio[st->nextio]);
   }
   DeleteIORequest(st->audio[0]);
   DeleteIORequest(st->audio[1]);
   FreeMem(st->sample[0],MAXLENGTH);
   FreeMem(st->sample[1],MAXLENGTH);
}

/* Play the sound using audio.device or AHI */
static void Audioplay(struct Sndcopy *snd,struct Sndtask *st)
{
   GetDTAttrs(st->dto,
      SDTA_Sample,&st->dtsample,
      SDTA_SampleLength,&st->samplength,
      SDTA_Volume,&st->volume,
      SDTA_Cycles,&st->cycles,
      #if defined(__MORPHOS__)
      SDTA_Frequency, &st->freq,
      SDTA_SampleType, &st->sampletype,
      #elif defined(__amigaos4__)
      SDTA_SamplesPerSec, &st->freq,
      #endif
      SDTA_Period,&st->period,
      TAG_END);

   if (st->dtsample)
   {
      if ((st->ioport = CreateMsgPort()))
      {
         ULONG rc;

         #if defined(__MORPHOS__)
         st->samplength *= SDTM_BYTESPERPOINT(st->sampletype);
         #endif

         rc = AHIPlay(st);

         /* MaxLocMem != 0 if we have real chip ram */
         if (!rc && SysBase->MaxLocMem)
         {
            Playdbuf(st);
         }
      }
      DeleteMsgPort(st->ioport);
   }
}

/* Subtask playing the sound */
static void Soundtask(struct Sndcopy *snd)
{
   struct Sndtask st = {0};
   st.dto=NewDTObject(snd->source->filename,
      DTA_SourceType,DTST_FILE,
      DTA_GroupID,GID_SOUND,
      #if defined(__MORPHOS__)
      SDTA_Mode, SDTA_Mode_Extended,
      #endif
      TAG_END);
   if(st.dto)
   {
      st.loop=snd->loop;
      Audioplay(snd,&st);

      DisposeDTObject(st.dto);
      Waittask(0);
   }
}

/*------------------------------------------------------------------------*/

/* Stop playing */
static void Stopsound(struct Sndcopy *snd)
{  if(snd->task)
   {  Adisposeobject(snd->task);
      snd->task=NULL;
   }
   snd->flags&=~SNDF_WANTPLAY;
}

/* Start playing of sound. If no source available yet, set flag and
 * start to play upon receipt of AOSNP_Srcupdate. */
static void Playsound(struct Sndcopy *snd)
{  if(snd->source && snd->source->filename)
   {  snd->flags&=~SNDF_WANTPLAY;
      Stopsound(snd);
      if ((snd->task=Anewobject(AOTP_TASK,
         AOTSK_Entry,(Tag)Soundtask,
         AOTSK_Name,(Tag)"AWeb sound player",
         AOTSK_Userdata,(Tag)snd,
         TAG_END)))
      {  Asetattrs(snd->task,AOTSK_Start,TRUE,TAG_END);
      }
   }
   else
   {  snd->flags|=SNDF_WANTPLAY;
   }
}

/*------------------------------------------------------------------------*/

static long Setsoundcopy(struct Sndcopy *snd,struct Amset *ams)
{  long result=0;
   struct TagItem *tag,*tstate=ams->tags;
   Amethodas(AOTP_COPYDRIVER,snd,AOM_SET,(Tag)ams->tags);
   while ((tag=NextTagItem(&tstate)))
   {  switch(tag->ti_Tag)
      {  case AOCDV_Copy:
            snd->copy=(void *)tag->ti_Data;
            break;
         case AOCDV_Sourcedriver:
            snd->source=(struct Sndsource *)tag->ti_Data;
            break;
         case AOCDV_Soundloop:
            snd->loop=(long)tag->ti_Data;
            if(snd->loop==0) snd->loop=1;
            break;
         case AOSNP_Srcupdate:
            Playsound(snd);
            break;
         case AOBJ_Cframe:
            /* Probably a background sound of a document going in or out of display */
            if(tag->ti_Data)
            {  Playsound(snd);
            }
            else
            {  Stopsound(snd);
            }
            break;
      }
   }
   return result;
}

static struct Sndcopy *Newsoundcopy(struct Amset *ams)
{  struct Sndcopy *snd=Allocobject(AOTP_SOUNDCOPY,sizeof(struct Sndcopy),ams);
   if(snd)
   {  snd->loop=1;
      Setsoundcopy(snd,ams);
   }
   return snd;
}

static long Getsoundcopy(struct Sndcopy *snd,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   long result;
   result=AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)snd,(struct Amessage *)ams);
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOCDV_Undisplayed:
            PUTATTR(tag,TRUE);
            break;
      }
   }
   return result;
}

static void Disposesoundcopy(struct Sndcopy *snd)
{  Stopsound(snd);
   Amethodas(AOTP_COPYDRIVER,snd,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Soundcopy_Dispatcher,
struct Sndcopy *,snd,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newsoundcopy((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Setsoundcopy(snd,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Getsoundcopy(snd,(struct Amset *)amsg);
         break;
      case AOM_DISPOSE:
         Disposesoundcopy(snd);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         result=AmethodasA(AOTP_COPYDRIVER,(struct Aobject *)snd,amsg);
   }
   return result;

    USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installsoundcopy(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_SOUNDCOPY,(Tag)Soundcopy_Dispatcher)) return FALSE;
   return TRUE;
}
