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

/* awebcharset.c -  awebcharset.awebplugin source */


#include "pluginlib.h"
#include "awebcharset.h"
#include "ezlists.h"

#include <libraries/awebsupport.h>
#include <libraries/awebmodule.h>
#include <exec/memory.h>
#include <exec/semaphores.h>

#undef NO_INLINE_STDARG
#define USE_INLINE_STDARG
#include <proto/awebsupport.h>
#include <proto/codesets.h>
#include <proto/dos.h>
#define NO_INLINE_STDARG

#include <proto/exec.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define CATCOMP_NUMBERS
#include "locale.h"

/* Some global variables for the Info Requester */
#define MAXNAMELEN 42
UBYTE destname[MAXNAMELEN];
UBYTE scrname[MAXNAMELEN];
UBYTE headername[MAXNAMELEN];
UBYTE metaname[MAXNAMELEN];
UBYTE defaultname[MAXNAMELEN];
UBYTE *url="None";
BOOL initdefault = FALSE;
BOOL usedefault = FALSE;

#define UTF8DEFAULT "UTF-8"
#define VALIDBYTES 4 /* maximal Number of Bytes for a valid UTF-8 sequence */

struct Memnode
{
   NODE(Memnode);
   UBYTE *mem;
};

static LIST(Memnode) memlist;
static struct SignalSemaphore *memlistsema;

/* Add a memory pointer to the memlist */
static void Addmem(void *mem)
{
   struct Memnode *mn;

   ObtainSemaphore(memlistsema);
   if(mn=ALLOCSTRUCT(Memnode,1,MEMF_CLEAR))
   {
      mn->mem=mem;
      ADDTAIL(&memlist,mn);
   }
   ReleaseSemaphore(memlistsema);
}

/* Remove a memory pointer from the memlist */
static void Remmem(void *mem)
{
   struct Memnode *mn;

   ObtainSemaphore(memlistsema);
   for(mn=memlist.first;mn->next;mn=mn->next)
   {  if(mn->mem==mem)
      {
         REMOVE(mn);
         FREE(mn);
         break;
      }
   }
   ReleaseSemaphore(memlistsema);
}

/* Get the Systems default codeset name and store it */
static void Initdefault()
{
   struct codeset *cs;

   if((cs = CodesetsFindA(NULL, NULL)) && cs->name)
   {
      strncpy(destname,cs->name,strlen(cs->name));
      strncpy(defaultname,cs->name,strlen(cs->name));
   }
   initdefault = TRUE;
   usedefault = TRUE;
}

/* Get a ENV Variable */
/* Return FALSE if the Variable is Off or 0 */
/* If the Variable doesn't exists then return the default Value */
BOOL Getcharsetenv(STRPTR varname, BOOL defaultval)
{
   char varbuf[5]={0};
   BOOL result=TRUE;
   long len;

   if(DOSBase)
   {
      len = GetVar(varname,varbuf,sizeof(varbuf),GVF_GLOBAL_ONLY);
      if(len == -1)
      {
         result=defaultval;
      }
      else if(!strnicmp(varbuf,"Off",3) || !strnicmp(varbuf,"0",1))
      {
         result=FALSE;
      }
   }
   return result;
}

/* don't calculate any UTF-8 value, just compare and replace */
/* returns 0 if no buffer is available */
static long Replace(struct Filterdata *fd, STRPTR data, long datalen)
{
   int i=0;
   ULONG len=0;
   UBYTE *bufptr, *end=NULL, *rep=NULL;

   /* look if here is already a buffer */
   if(!fd->buf)
   {
      fd->buf=Allocmem(datalen,MEMF_CLEAR);
      if(fd->buf)
      {
         Addmem(fd->buf);
         fd->bufsize=datalen;
      }
   }
   else
   {
      /* if the old buffer is to small then free the old and alloc a new buffer */
      if(fd->bufsize < datalen)
      {
         Freemem(fd->buf);
         Remmem(fd->buf);
         fd->buf=Allocmem(datalen,MEMF_CLEAR);
         if(fd->buf) 
         {
            Addmem(fd->buf);
            fd->bufsize=datalen;
         }
      }
   }
   if(fd->buf)
   {
      bufptr=fd->buf;
      end=data+datalen;
      datalen=0;

      while(data<end)
      {
         rep=NULL;
         if(data<end-2 && *data==0xE2 && *(data+1)==0x80)
         {
            switch(*(data+2))
            {
               case 0x90:  /* U+2010 -> - (HYPHEN) */
               case 0x91:  /* U+2011 -> - (NON-BREAKING HYPHEN) */
                  rep="-";
                  break;
               case 0x92:  /* U+2012 -> -- (FIGURE DASH) */
               case 0x93:  /* U+2013 -> -- (EN DASH) */
                  rep="--";
                  break;
               case 0x94:  /* U+2014 -> --- (EM DASH) */
               case 0x95:  /* U+2015 -> --- (HORIZONTAL BAR) */
                  rep="---";
                  break;
               case 0x98:  /* U+2018 -> ` (LEFT SINGLE QUOTATION MARK) */
                  rep="`";
                  break;
               case 0x9A:  /* U+201A -> , (SINGLE LOW-9 QUOTATION MARK) */
                  rep=",";
                  break;
               case 0x9B:  /* U+201B -> U+00B4 (SINGLE HIGH-REVERSED-9 QUOTATION MARK) */
                  rep="\xC2\xB4"; /* == U+00B4 (ACUTE ACCENT) */
                  break;
               case 0x9C:  /* U+201C -> '' (LEFT DOUBLE QUOTATION MARK) */
                  rep="''";
                  break;
               case 0x9D:  /* U+201D -> " (RIGHT DOUBLE QUOTATION MARK) */
               case 0x9F:  /* U+201F -> " (DOUBLE HIGH-REVERSED-9 QUOTATION MARK) */
                  rep="\"";
                  break;
               case 0x9E:  /* U+201E -> ,, (DOUBLE LOW-9 QUOTATION MARK) */
                  rep=",,";
                  break;
               case 0xA0:  /* U+2020 -> + (DAGGER) */
               case 0xA1:  /* U+2021 -> + (DOUBLE DAGGER) */
                  rep="+";
                  break;
               case 0xA2:  /* U+2022 -> U+00B7 (BULLET) */
               case 0xA7:  /* U+2027 -> U+00B7 (HYPHENATION POINT) */
                  rep="\xC2\xB7"; /* == U+00B7 (MIDDLE DOT) */
                  break;
               case 0xA5:  /* U+2025 -> .. (TWO DOT LEADER)*/
                  rep="..";
                  break;
               case 0xA6:  /* U+2026 -> ... (HORIZONTAL ELLIPSIS) */
                  rep="...";
                  break;
               case 0xB9:  /* U+2039 -> < (SINGLE LEFT-POINTING ANGLE QUOTATION MARK) */
                  rep="<";
                  break;
               case 0xBA:  /* U+203A -> !! (SINGLE RIGHT-POINTING ANGLE QUOTATION MARK) */
                  rep=">";
                  break;
               case 0xBB:  /* U+203B -> U+00D7 (REFERENCE MARK) */
                  rep="\xc3\x97"; /* == U+00D7 (MULTIPLICATION SIGN) */
                  break;
               case 0xBC:  /* U+203C -> !! (DOUBLE EXCLAMATION MARK) */
                  rep="!!";
                  break;
            }
         }
         if(data<end-2 && *data==0xE2 && *(data+1)==0x82 && *(data+2)==0xAC)
         {
            /* the Euro Sign (U+20AC) replace with U+00A4 (CURRENCY SIGN) */
            rep="\xC2\xA4";
            len=2;
         }
         if(data<end-2 && *data==0xE2 && *(data+1)==0x86)
         {
            switch(*(data+2))
            {
               case 0x90:  /* U+2190 -> <- (LEFTWARDS ARROW) */
                  rep="<-";
                  break;
               case 0x92:  /* U+2192 -> -> (RIGHTWARDS ARROW) */
                  rep="->";
                  break;
            }
         }
         if(rep)
         {
            for(i=0;*rep;i++)
            {
               *bufptr++=*rep++;
            }
            datalen+=i;
            data+=3;
         }
         else
         {
            *bufptr++=*data++;
            datalen++;
         }
      }
      return datalen;
   }
   return 0;
}

/* convert the data and write the data back to AWeb */
static void Convertandwrite(STRPTR data, long datalen, struct Filterdata *fd, void *handle)
{
   ULONG stringlen=0;
   STRPTR dst;

   dst = CodesetsConvertStr(CSA_SourceCodeset, (ULONG)fd->srcCodeset,
                            CSA_DestCodeset,   (ULONG)fd->dstCodeset,
                            CSA_Source,        (ULONG) data,
                            CSA_SourceLen,     datalen,
                            CSA_DestLenPtr,    (ULONG)&stringlen,
                            TAG_DONE);

   Writefilter(handle,dst,stringlen);
   CodesetsFreeA(dst, NULL);
}

/* Actions to be performed when our library has been loaded */
ULONG Initpluginlib(struct AwebCharsetBase *base)
{
   /* Initialise platform specific environment */
   if (!AwebModuleInit()) return FALSE;

   /* Return nonzero if success */
   return TRUE;
}

/* Actions to be performed when our library is about to be expunged */
void Expungepluginlib(struct AwebCharsetBase *base)
{
   /* Cleanup platform specifics */

   /* free all outstanding memory allocations */
   struct Memnode *mn;

   if(memlistsema)
   {
      ObtainSemaphore(memlistsema);
      while(mn=(struct Memnode *)REMHEAD(&memlist))
      {
         if(mn->mem)
         {
            Freemem(mn->mem);
            mn->mem=NULL;
         }
         FREE(mn);
      }
      ReleaseSemaphore(memlistsema);
      FREE(memlistsema);
      memlistsema=NULL;
   }

   AwebModuleExit();
}

/* The first library function */
LIBFUNC_H1
(
   __saveds ULONG  , Initplugin,
   struct Plugininfo *,pi,A0,
   PLUGIN_TYPE, PLUGIN_NAME
)
{
   LIBFUNC_INIT
   
   /* Return zero class IDs to indicate we're a filter plugin */
   pi->sourcedriver=0;
   pi->copydriver=0;

   if(!(memlistsema=ALLOCSTRUCT(SignalSemaphore, 1, 0)))
   {
      return 0;
   }
   InitSemaphore(memlistsema);
   NEWLIST(&memlist);

   /* Get the Systems default codeset name */
   if(!initdefault)
   {
      Initdefault();
   }
   /* Return nonzero (success) */
   return 1;

   LIBFUNC_EXIT
}

/* Query function. Handle in a safe way. There is generally no need to
 * change this code, it is generated from the #defines in pluginlib.h */

#define ISSAFE(s,f) (s->structsize>=((long)&s->f-(long)s+sizeof(s->f)))

LIBFUNC_H1
(
   __saveds void  , Queryplugin,
   struct Pluginquery *,pq,A0,
   PLUGIN_TYPE, PLUGIN_NAME

)
{
   LIBFUNC_INIT

#ifdef PLUGIN_COMMANDPLUGIN
   if(ISSAFE(pq,command)) pq->command=TRUE;
#endif
#ifdef PLUGIN_FILTERPLUGIN
   if(ISSAFE(pq,filter)) pq->filter=TRUE;
#endif

   LIBFUNC_EXIT
}

/* The command function.
 *
 * This function understand the follow commands:
 *
 * 'On' and 'Off' command which turn the filter On and Off.
 *
 * 'ReplaceOn' and 'ReplaceOff' which turn the feature
 * to replace some UTF-8 characters On or Off.
 *
 * 'RequestOn' and 'RequestOff' which turn the feature to Show
 * a Requester if the Meta and Header Charset differ On or Off.
 *
 * 'Info' show a requester with some informations.
 *
 * 'System' Set the destination charset back to the system default.
 *
 * If a charset (must be supported from the codeset.library)
 * is given as command then the filter is switched On
 * and this charset is used as destination charset.
 *
 * If any other command is given the plugin return with
 * RC=10 and the default codeset is used.
*/
LIBFUNC_H1
(
   __saveds void  , Commandplugin,
   struct Plugincommand *,pc,A0,
   PLUGIN_TYPE, PLUGIN_NAME

)
{
   LIBFUNC_INIT

   STRPTR *array;
   int i=0;
   BOOL valid=FALSE;
   UBYTE *buf, *infotxt, *filteronoff, *replaceonoff, *requestonoff, *setby;
   long len=0;

   if(CodesetsBase)
   {
      /* be sure the default charsetname is set */
      if(!initdefault)
      {
         Initdefault();
      }
      /* search for supported commands */
      if(!stricmp(pc->command,"Off"))
      {
         SetVar("AWeb3/CharsetFilter","Off",3,GVF_GLOBAL_ONLY);
         valid=TRUE;
      }
      else if(!stricmp(pc->command,"On"))
      {
         SetVar("AWeb3/CharsetFilter","On",2,GVF_GLOBAL_ONLY);
         valid=TRUE;
      }
      else if(!stricmp(pc->command,"ReplaceOff"))
      {
         SetVar("AWeb3/CharsetReplace","Off",3,GVF_GLOBAL_ONLY);
         valid=TRUE;
      }
      else if(!stricmp(pc->command,"ReplaceOn"))
      {
         SetVar("AWeb3/CharsetReplace","On",2,GVF_GLOBAL_ONLY);
         valid=TRUE;
      }
      else if(!stricmp(pc->command,"RequestOff"))
      {
         SetVar("AWeb3/CharsetRequest","Off",3,GVF_GLOBAL_ONLY);
         valid=TRUE;
      }
      else if(!stricmp(pc->command,"RequestOn"))
      {
         SetVar("AWeb3/CharsetRequest","On",2,GVF_GLOBAL_ONLY);
         valid=TRUE;
      }
      else if(!stricmp(pc->command,"Info"))
      {
         filteronoff =Getcharsetenv("AWeb3/CharsetFilter",  TRUE) ?Awebstr(MSG_CHARSET_ON):Awebstr(MSG_CHARSET_OFF);
         replaceonoff=Getcharsetenv("AWeb3/CharsetReplace", TRUE) ?Awebstr(MSG_CHARSET_ON):Awebstr(MSG_CHARSET_OFF);
         requestonoff=Getcharsetenv("AWeb3/CharsetRequest",FALSE) ?Awebstr(MSG_CHARSET_ON):Awebstr(MSG_CHARSET_OFF);
         setby = usedefault ? Awebstr(MSG_CHARSET_SYSTEM):Awebstr(MSG_CHARSET_COMMAND);

         len =strlen(infotxt=Awebstr(MSG_CHARSET_INFOTEXT));
         len+=strlen(filteronoff);
         len+=strlen(replaceonoff);
         len+=strlen(requestonoff);
         len+=strlen(url);
         len+=strlen(headername);
         len+=strlen(metaname);
         len+=strlen(scrname);
         len+=strlen(destname);
         len+=strlen(setby);

         if(buf=Allocmem(len+11,MEMF_CLEAR))
         {
            sprintf(buf,infotxt, filteronoff, replaceonoff ,requestonoff ,url,
                    headername, metaname, scrname, destname, setby);
            Syncrequest(Awebstr(MSG_CHARSET_INFOTITLE),buf,Awebstr(MSG_CHARSET_OK));
            Freemem(buf);
         }
         valid=TRUE;
      }
      else if(!stricmp(pc->command,"System"))
      {
         strncpy(destname,defaultname,sizeof(destname)-1);
         usedefault=TRUE;
         valid=TRUE;
      }
      else
      {
         if((array = CodesetsSupportedA(NULL)))
         {
            for(i=0; array[i] != NULL; i++)
            {
               if(!strnicmp(pc->command,array[i],strlen(array[i])))
               {
                  SetVar("AWeb3/CharsetFilter","On",2,GVF_GLOBAL_ONLY);
                  strncpy(destname,array[i],sizeof(destname)-1);
                  usedefault=FALSE;
                  valid=TRUE;
               }
            }
            CodesetsFreeA(array, NULL);
         }
      }
   }
   if(valid==FALSE)
   {
      /* Invalid command */
      strncpy(destname,defaultname,sizeof(destname)-1);
      usedefault=TRUE;
      pc->rc=10;
   }
   LIBFUNC_EXIT
}

/* The filter function */
LIBFUNC_H1
(
   __saveds void  , Filterplugin,
   struct Pluginfilter *,pf,A0,
   PLUGIN_TYPE, PLUGIN_NAME
)
{
   LIBFUNC_INIT

   ULONG len=0, datalen=0;
   UBYTE *data=NULL, *ptr=NULL, *end=NULL;
   UBYTE *xmlptr=NULL, *endname=NULL;
   UBYTE *reqtxt=NULL, *reqbut=NULL;
   UBYTE *txtbuf=NULL, *butbuf=NULL;
   UBYTE metacharset[MAXNAMELEN]="", headercharset[MAXNAMELEN]="";
   BOOL validheader=FALSE, validmeta=FALSE;
   BOOL xml=FALSE, activ=FALSE, meta=FALSE, found=FALSE;
   long result=0;
   STRPTR *array;
   int i=0;

   struct Filterdata *fd;

   /* See if there is already a userdata for us.
    * If not, allocate and initialize. */
   fd=pf->userdata;
   if(!fd)
   {
      fd=Allocmem(sizeof(struct Filterdata),MEMF_CLEAR);
      if(!fd) return;

      Addmem(fd);
      pf->userdata=fd;
      fd->first=TRUE;
      fd->on=Getcharsetenv("AWeb3/CharsetFilter",TRUE);
      fd->replace=Getcharsetenv("AWeb3/CharsetReplace",TRUE);
      fd->request=Getcharsetenv("AWeb3/CharsetRequest",FALSE);

   }
   if(pf->data)
   {
      if(CodesetsBase)
      {
         if(fd->first)
         {
            url=pf->url;
            strncpy(fd->destcharset,destname,sizeof(fd->destcharset)-1);

            /* look if here is a header charset */
            if(pf->encoding)
            {
               strncpy(headername,pf->encoding,sizeof(headername)-1);
            }
            /* now search for "<?xml" and meta 'charset=..."' */
            metaname[0] = '\0';
            i=0;
            ptr = pf->data;
            end = pf->data+pf->length;
            if(ptr<end-6 && !strnicmp(ptr,"<?xml",5))
            {
               xml = TRUE;
               ptr+=5;
            }
            /* for the case here is a BOM (3 Byte) we also search after the BOM too */
            if(ptr<end-9 && !strnicmp(ptr+3,"<?xml",5))
            {
               xml = TRUE;
               ptr+=8;
            }
            /* now search for a meta CHARSET definition also if a XML was found */
            while(ptr<end && !found)
            {
               while(ptr<end && isspace(*ptr)) ptr++; /* skip spaces */
               if(ptr<end-1 && *ptr=='<')
               {
                  activ = TRUE;
                  ptr++;
               }
               if(activ)
               {
                  if(ptr<end-1 && *ptr=='>')
                  {
                     activ = FALSE;
                     ptr++;
                  }
                  if(ptr<end-5 && !strnicmp(ptr,"META",4))
                  {
                     meta = TRUE;
                     ptr+=4;
                  }
                  if(ptr<end-6 && !strnicmp(ptr,"/HEAD>",6) || !strnicmp(ptr,"BODY>",5))
                  {
                     break;
                  }
                  if(meta)
                  {
                     if(ptr<end-8 && !strnicmp(ptr,"CHARSET=",8))
                     {
                        found = TRUE;
                        ptr+=7;
                     }
                  }
               }
               if(xml)
               {
                  if(ptr<end-10 && !strnicmp(ptr,"ENCODING=",9))
                  {
                     xmlptr = ptr+=9;
                  }
               }
               ptr++;
            }
            if(!found && xml)
            {
               if(xmlptr)
               {
                  ptr = xmlptr;
                  found = TRUE;
               }
               else
               {
                  strncpy(metaname,UTF8DEFAULT,sizeof(UTF8DEFAULT));
               }
            }
            if(found)
            {
               /* here ptr should point to the meta or xml charsetname */
               while(ptr<end && *ptr == '\"' || isspace(*ptr)) ptr++; /* skip quotes and spaces */
               if(endname = strpbrk(ptr,"\">? \t\r\n\0"))
               {
                  i=0;
                  while(ptr<end && ptr < endname && i<sizeof(metaname)-1)
                  {
                     metaname[i] = *ptr;
                     i++;
                     ptr++;
                  }
                  metaname[i] = '\0';
               }
            }
            /* now look if the charsets are supported from the codeset.library */
            if(*headername || *metaname)
            {
               if((array = CodesetsSupportedA(NULL)))
               {
                  if(*headername)
                  {
                     /* work around for servers which use "utf8" */
                     /* instead of the correct name "utf-8" */
                     if(!strnicmp(headername, "utf8", 4))
                     {
                        strncpy(headername,UTF8DEFAULT,sizeof(UTF8DEFAULT));
                     }
                     for(i=0; array[i] != NULL; i++)
                     {
                        if(!strnicmp(headername,array[i],strlen(array[i])))
                        {
                           validheader = TRUE;
                           strncpy(headercharset,array[i],sizeof(headercharset)-1);
                           strncpy(fd->srccharset,headercharset,sizeof(fd->srccharset)-1);
                           break;
                        }
                     }
                  }
                  if(*metaname)
                  {
                     /* work around for webpages which use "utf8" */
                     /* instead of the correct name "utf-8" */
                     if(!strnicmp(metaname, "utf8", 4))
                     {
                        strncpy(metaname,UTF8DEFAULT,sizeof(UTF8DEFAULT));
                     }
                     for(i=0; array[i] != NULL; i++)
                     {
                        if(!strnicmp(metaname,array[i],strlen(array[i])))
                        {
                           validmeta = TRUE;
                           strncpy(metacharset,array[i],sizeof(metacharset)-1);
                           if(!validheader)
                           {
                              strncpy(fd->srccharset,metacharset,sizeof(fd->srccharset)-1);
                           }
                           break;
                        }
                     }
                  }
                  CodesetsFreeA(array, NULL);
               }
            }
            if(!validheader && !validmeta) /* use default charset */
            {
               strncpy(fd->srccharset,defaultname,strlen(defaultname));
               usedefault=TRUE;
            }
            /* check if the filter is On and we have 2 valid names */
            if(fd->on && validheader && validmeta)
            {
               /* check if both are different and if so then pop up a requester */
               if (strnicmp(headercharset,metaname,strlen(headercharset)))
               {
                  if(fd->request==TRUE)
                  {
                     len =strlen(reqtxt=Awebstr(MSG_CHARSET_DIFFREQTXT));
                     len+=strlen(headercharset);
                     len+=strlen(metaname);
                     if(txtbuf=Allocmem(len+3, MEMF_CLEAR))
                     {
                        sprintf(txtbuf,reqtxt, headercharset, metaname);

                        len =strlen(reqbut=Awebstr(MSG_CHARSET_DIFFREQBUT));
                        len+=strlen(headercharset);
                        len+=strlen(metaname);
                        if(butbuf=Allocmem(len+3, MEMF_CLEAR))
                        {
                           sprintf(butbuf,"%s|%s|%s", headercharset, metaname, reqbut);
                           switch(Syncrequest(Awebstr(MSG_CHARSET_CODESETTITLE), txtbuf, butbuf))
                           {
                              case 0: /* Don't ask again */
                                 fd->on = FALSE;
                                 SetVar("AWeb3/CharsetRequest","Off",3,GVF_GLOBAL_ONLY);
                                 break;
                              case 1: /* Header Charset */
                                 strncpy(fd->srccharset,headercharset,sizeof(fd->srccharset)-1);
                                 break;
                              case 2: /* Meta Charset */
                                 strncpy(fd->srccharset,metacharset,sizeof(fd->srccharset)-1);
                                 break;
                           }
                           Freemem(butbuf);
                        }
                        Freemem(txtbuf);
                     }
                  }
                  else
                  {
                     strncpy(fd->srccharset,headercharset,sizeof(fd->srccharset)-1);
                  }
               }
            }
            strncpy(scrname,fd->srccharset, sizeof(scrname)-1);
            fd->srcCodeset = CodesetsFind(fd->srccharset,
                                          CSA_FallbackToDefault, FALSE,
                                          TAG_DONE);

            fd->dstCodeset = CodesetsFind(fd->destcharset,
                                          CSA_FallbackToDefault, FALSE,
                                          TAG_DONE);
            fd->first=FALSE;
         }
         if(!fd->on)
         {
            /* Don't filter data. Just write the entire block unchanged. */
            Writefilter(pf->handle,pf->data,pf->length);
         }
         else
         {
            data=pf->data;
            datalen=pf->length;
            /* look if here are old Data from last Block */
            if(fd->olddata && datalen >= (fd->totallen-fd->olddatalen))
            {
               memcpy(fd->olddata + fd->olddatalen, data, fd->totallen - fd->olddatalen);
               if(fd->replace && fd->totallen>0 && !strnicmp(scrname,"UTF-8",5))
               {
                  if(result=Replace(fd, fd->olddata, fd->totallen))
                  {
                     Convertandwrite(fd->buf, result, fd, pf->handle);
                  }
                  else
                  {
                     /* Don't filter data. Just write the sequence unchanged. */
                     Writefilter(pf->handle, fd->olddata, fd->totallen);
                  }
               }
               else
               {
                  Convertandwrite(fd->olddata, fd->totallen, fd, pf->handle);
               }

               data+=fd->totallen - fd->olddatalen;
               datalen-=fd->totallen - fd->olddatalen;

               Freemem(fd->olddata);
               Remmem(fd->olddata);
               fd->olddata=0;
               fd->olddatalen=0;
            }
            if(fd->replace && datalen>0 && !strnicmp(scrname,"UTF-8",5))
            {
               if(result=Replace(fd, data, datalen))
               {
                  datalen=result;
                  data=fd->buf;
               }
            }
            if(!strnicmp(fd->srccharset,fd->destcharset,strlen(fd->srccharset)))
            {
               Writefilter(pf->handle,pf->data,pf->length);
            }
            else
            {
               end=data+datalen-1;
               ptr=end;
               if(*ptr > 127 && datalen && !strnicmp(scrname,"UTF-8",5))
               {
                  /* here we are because an UTF-8 sequence could be split into 2 Blocks */
                  /* we must find the last non UTF-8 Byte or an UTF-8 Startbyte in the Block */
                  for(i=1;*ptr && ptr>data && i<=VALIDBYTES;i++)
                  {
                     if(*ptr < 0x80 || *ptr > 0xBF)
                     {
                        break;
                     }
                     ptr--;
                  }
                  if(i<=VALIDBYTES) /* don't process invalid UTF-8 sequences */
                  {
                     /* here ptr should point to the last non UTF-8 Byte or an UTF-8 Startbyte */
                     /* now check how long the UTF-8 sequence is */
                     if(*ptr > 0xF7) /* invalid Data */
                     {
                        len=0;
                     }
                     else if(*ptr < 0xC0)
                     {
                        len=1;
                     }
                     else if(*ptr < 0xE0)
                     {
                        len=2;
                     }
                     else if(*ptr < 0xF0)
                     {
                        len=3;
                     }
                     else
                     {
                        len=4;
                     }
                     /* here i = length of data to remember and len= Number of Bytes of the UTF-8 sequence */
                     /* if both are equal then the last Byte was the also the last byte from the UTF-8 sequence */
                     if(len>1 && i!=len)
                     {
                        if(fd->olddata=Allocmem(5, MEMF_CLEAR))
                        {
                           Addmem(fd->olddata);
                           memcpy(fd->olddata, ptr, i);
                           fd->olddatalen=i;
                           fd->totallen=len;
                           datalen=datalen-i;
                        }
                     }
                  }
               }
               Convertandwrite(data, datalen, fd, pf->handle);
            }
         }
      }
      else
      {
         /* Don't filter data. Just write the entire block unchanged. */
         Writefilter(pf->handle,pf->data,pf->length);
      }
   }
   /* Delete our userdata when eof was reached */
   if(pf->eof)
   {
      if(fd)
      {
         if(fd->olddata)
         {
            /* for the Case we still have olddata then write the data unchanged back to AWeb */
            /* that should never happen but who know... */
            Writefilter(pf->handle, fd->olddata, fd->totallen);
            Freemem(fd->olddata);
            Remmem(fd->olddata);
         }
         if(fd->buf)
         {
            Freemem(fd->buf);
            Remmem(fd->buf);
         }
         Freemem(fd);
         Remmem(fd);
         fd=NULL;
         pf->userdata=NULL;
      }
   }
   LIBFUNC_EXIT
}
