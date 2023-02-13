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

/* url.c aweb url object */

#include "aweb.h"
#include "url.h"
#include "frame.h"
#include "link.h"
#include "source.h"
#include "cache.h"
#include "fetch.h"
#include "window.h"
#include <proto/utility.h>


struct Child
{  NODE(Child);
   struct Aobject *object;
   ULONG relation;
};

static UBYTE absurl[4096];

#define NRLISTS 64
#define HASHMASK  (NRLISTS-1)

static LIST(Url) urls[NRLISTS];
static long lastpostnr=0;
static long loadnr=0;
static long dupchecklist;

#define UQID_DUPCHECK   1

static struct Url *emptyurl;

static struct TagItem mapwindowtags[]=
{  {AOURL_Status,       AOWIN_Status},
   {AOURL_Contentlength,AOWIN_Total},
   {AOURL_Datatotal,    AOWIN_Read},
   {TAG_END}
};

/*---------------------------------------------------------------------*/

/* This field is tested against to find beginning of fragemtn while building
 * absurl (is single-threaded anyway). For news: URLs, it is set to '\0',
 * for other URLs it is set to '#'. This is because news: urls can have
 * a # in their message ID and the URL shouldn't be cut off there. */
static UBYTE fraghash='#';

static long Urlschemelength(UBYTE *ptr)
{  UBYTE *p=ptr;
   if(*p==':') return 0;
   while(*p && isalnum(*p) || *p=='+' || *p=='.' || *p=='-') p++;
   if(*p==':') return p-ptr+1;
   else return 0;
}

static long Urlhostlength(UBYTE *ptr)
{  UBYTE *p=ptr;
   if(p[0]=='/' && p[1]=='/')
   {  p+=2;
      while(*p && *p!='/' && *p!=fraghash && p!='?') p++;
      if(*p=='/' || *p=='\0' || *p=='?') return p-ptr;
   }
   return 0;
}

static long Urlpathlength(UBYTE *ptr)
{  UBYTE *p=ptr;
   while(*p && *p!=';' && *p!='?' && *p!=fraghash) p++;
   return p-ptr;
}

static long Urlpathseglength(UBYTE *ptr)
{  UBYTE *p=ptr;
   while(*p && *p!='/' && *p!=';' && *p!='?' && *p!=fraghash) p++;
   if(*p=='/') return p-ptr+1;
   else return p-ptr;
}

static long Urlparamlength(UBYTE *ptr)
{  UBYTE *p=ptr;
   if(*p!=';') return 0;
   while(*p && *p!='?' && *p!=fraghash) p++;
   return p-ptr;
}

static long Urlquerylength(UBYTE *ptr)
{  UBYTE *p=ptr;
   if(*p!='?') return 0;
   while(*p && *p!=fraghash) p++;
   return p-ptr;
}

static long Urllength(UBYTE *ptr)
{  UBYTE *p=ptr;
   while(*p && *p!=fraghash) p++;
   return p-ptr;
}

static BOOL Removelastseg(UBYTE *ptr,BOOL localhost)
{  UBYTE *p=ptr+strlen(ptr)-1;
   if(p<ptr) return FALSE;
   if(*p=='/') p--;
   while(p>=ptr && *p!='/' && !(localhost && *p==':')) p--;
   p[1]='\0';
   return TRUE;
}

static void Urlcat(UBYTE *a,UBYTE *b,long len)
{  long al=strlen(a);
   if(al+len>4095) len=4095-al;
   if(len>0) strncpy(a+al,b,len);
   a[al+len]='\0';
}

static void Buildabsurl(UBYTE *base,UBYTE *rel)
{  long l,m;
   UBYTE *pathp;
   BOOL usebaseparam=FALSE,localhost=FALSE,hadhost=FALSE;
   BOOL isabs=FALSE;
   memset(absurl,0,sizeof(absurl));
   /* remove leading spaces */
   while(*base==' ') base++;
   while(*rel==' ') rel++;
   /* if base is empty, use relurl */
   if(!*base)
   {  fraghash=(STRNIEQUAL(rel,"NEWS:",5))?'\0':'#';
      Urlcat(absurl,rel,Urllength(rel));
      return;
   }
   /* if rel starts with scheme != base scheme, it's absurl */
   /* if rel starts with "x-aweb:", it's absurl */
   if(l=Urlschemelength(rel))
   {  if(!STRNIEQUAL(rel,"x-aweb:",7) && STRNIEQUAL(rel,base,l)) rel+=l;
      else
      {  fraghash=(STRNIEQUAL(rel,"NEWS:",5))?'\0':'#';
         Urlcat(absurl,rel,Urllength(rel));
         return;
      }
   }
   /* inherit the scheme of base */
   l=Urlschemelength(base);
   if(l) Urlcat(absurl,base,l);
   base+=l;
   /* if rel starts with host, use all of it */
   if(Urlhostlength(rel))
   {  fraghash=(STRNIEQUAL(rel,"NEWS:",5))?'\0':'#';
      Urlcat(absurl,rel,Urllength(rel));
      return;
   }
   /* inherit host of base */
   l=Urlhostlength(base);
   if(l)
   {  Urlcat(absurl,base,l);
      hadhost=TRUE;
   }
   localhost=STRNIEQUAL(absurl,"FILE:",5);
   fraghash=(STRNIEQUAL(absurl,"NEWS:",5))?'\0':'#';
   base+=l;
   if(*base=='/') base++;
   /* if rel starts with slash, it's absurl path */
   if(*rel=='/')
   {  /* In strict mode, just copy the remaining path. */
      if(prefs.browser.htmlmode==HTML_STRICT)
      {  Urlcat(absurl,rel,Urllength(rel));
         return;
      }
      /* In tolerant modes, go through the ./ and ../ logic below.
       * Don't inherit the base's path though. */
   }
   /* use base path for a start if there was a host */
   /* add an initial slash unless ths URL is of the form:
      "x-aweb:token" where the token is interpreted as part of the path.
      DO add a slash in case of "x-aweb://token" */
   if(STRNIEQUAL(absurl,"X-AWEB:",7))
   {  if(absurl[7]=='/') Urlcat(absurl,"/",1);
   }
   else if(hadhost) Urlcat(absurl,"/",1);
   pathp=absurl+strlen(absurl); /* pathp is start of path after "/" */
   l=Urlpathlength(base);
   if(*rel=='/')
   {  /* Absolute path, skip the slash */
      rel++;
      isabs = TRUE;
   }
   else
   {  /* Copy the bases path */
      if(l) Urlcat(absurl,base,l);
   }
   base+=l;
   /* if rel path is not empty, merge it into absurl */
   if(Urlpathlength(rel))
   {  if(absurl[strlen(absurl)-1]!='/') Removelastseg(pathp,localhost);
      while(l=Urlpathseglength(rel))
      {  /* remove "./" or ".(eol)" */
         if((l==2 && STRNEQUAL(rel,"./",2))
         || (l==1 && STREQUAL(rel,".")))
         {  /* remove "." */
            rel+=l;
         }
         else /* remove "../" or "..(eol)" */
            if((l==3 && STRNEQUAL(rel,"../",3))
         || (l==2 && STREQUAL(rel,"..")))
         {  /* remove ".." and last segment, or add ".." */
            if(Removelastseg(pathp,localhost)) rel+=l;
            else
            {  /* Only add extra ../ in strict mode */
               if(prefs.browser.htmlmode==HTML_STRICT)
               {  Urlcat(absurl,"../",3);
                  pathp+=3;
               }
               rel+=l;
            }
         }
         else
         {  /* copy segment */
            Urlcat(absurl,rel,l);
            rel+=l;
         }
      }
   }
   else if(!isabs) usebaseparam=TRUE;

   /* add rel's parameters and query */

   l=Urlparamlength(rel);
   m=Urlparamlength(base);
   if(l)
   {  Urlcat(absurl,rel,l);
      usebaseparam=FALSE;
   }
   else if(m && usebaseparam) Urlcat(absurl,base,m);

   rel+=l;
   base+=m;
   l=Urlquerylength(rel);
   m=Urlquerylength(base);
   if(l) Urlcat(absurl,rel,l);
   else if(m && usebaseparam) Urlcat(absurl,base,m);

}

/*---------------------------------------------------------------------*/

/* Compute hash from name */
static short Hash(UBYTE *name)
{  UBYTE hash=0,*p;
   if(name)
   {  for(p=name;*p;p++) hash^=*p;
   }
   return (short)(hash&HASHMASK);
}

/* Dispose object and clear pointer */
static void Clearobject(struct Aobject **p)
{  if(*p) Adisposeobject(*p);
   *p=NULL;
}

/* Solve movedto references */
static struct Url *Finalurl(struct Url *url)
{  short n=0;
   while(++n<100 && url->movedto && !(url->flags&URLF_VOLATILE)
   && ((url->flags&URLF_WASDUP) || (url->cache && !Agetattr(url->cache,AOCAC_Expired))))
   {  url=url->movedto;
   }
   return url;
}

/* Idem but don't require a cache object. Don't know why it is there in the
 * above function, but I don't want to try out what terrible things might
 * happen if I remove the test. But we need it without in one occation.
 * Very ugly solution but since this is the last upgrade anyway... */
static struct Url *Finalurl2(struct Url *url)
{  short n=0;
   while(++n<100 && url->movedto && !(url->flags&URLF_VOLATILE)
   && (!url->cache || !Agetattr(url->cache,AOCAC_Expired)))
   {  url=url->movedto;
   }
   return url;
}

/* Check for movedto loop */
static BOOL Movedtoloop(struct Url *url)
{  struct Url *u=url;
   short n=0;
   while(++n<100 && u->movedto)
   {  u=u->movedto;
      if(url==u) break;
   }
   return (BOOL)(url->movedto && u==url);
}

/* Permanently moved URLs should return their new address */
static UBYTE *Urllinkaddress(struct Url *url)
{  short n=0;
   while(url->movedto && !(url->flags&(URLF_TEMPMOVED|URLF_VOLATILE)))
   {  url=url->movedto;
      if(++n>99) break;
   }
   return url->url?url->url:NULLSTRING;
}
/* temporary ones should not */

static UBYTE *Urladdress(struct Url *url)
{  short n=0;
   while(url->movedto && !(url->flags&(URLF_VOLATILE)))
   {  url=url->movedto;
      if(++n>99) break;
   }
   return url->url?url->url:NULLSTRING;
}


/* Returns old or new SOURCE object for this URL. If URL is moved, return a source
 * for the new location. */
static void *Urlsource(struct Url *url)
{  url=Finalurl(url);
   if(!url->source)
   {  url->source=Anewobject(AOTP_SOURCE,
         AOSRC_Url,(Tag)url,
         TAG_END);
   }
   return url->source;
}

/* Passes these tags to all childs in this relation */
VARARGS68K_DECLARE(static  void Notifychilds(struct Url *url,ULONG relation,...))
{  struct Child *ch,*next;
   struct TagItem *tags;
   VA_LIST va;
   VA_STARTLIN(va,relation);
   tags = (struct TagItem *)VA_GETLIN(va,struct TagItem *);

   for(ch=url->childs.first;ch->next;ch=next)
   {  next=ch->next; /* Window might remove itself */
      if(ch->relation==relation)
      {
          Asetattrs(ch->object,TAG_MORE,(Tag)tags,TAG_DONE);
      }
   }
}

/* Replace the old source object by a new one. If newsource==NULL then create new one. */
static void Replacesource(struct Url *url,void *newsource)
{  void *oldsource;
   if(!newsource)
   {  newsource=Anewobject(AOTP_SOURCE,
         AOSRC_Url,(Tag)url,
         TAG_END);
   }
   if(newsource)
   {  oldsource=url->source;
      url->source=newsource;
      if(oldsource)
      {  /* Now move all COPYs from the old source to the new one.
          * New drivers will be generated by source object. */
         Asetattrs(oldsource,
            AOSRC_Movetourl,(Tag)url,
            TAG_END);
         Adisposeobject(oldsource);
      }
   }
}

/* Mark URL as visited, and notify links. */
static void Urlvisited(struct Url *url)
{  if(!(url->flags&URLF_VISITED))
   {  url->flags|=URLF_VISITED;
      Notifychilds(url,AOREL_URL_LINK,
         AOURL_Visited,TRUE,
         TAG_END);
   }
}

/* Send the source a message that re-posting isn't performed */
static void Norepost(struct Url *url)
{  static UBYTE flushedmsg[]="<html><head><title>%s</title></head>\n<body>\n<h1>%s</h1>\n"
      "<b>%s</b>\n<p>%s\n<p>%s\n</body></html>\n";
   UBYTE *head,*msg1,*msg2,*buffer;
   long len;
   head=AWEBSTR(MSG_EPART_FLUSHED_HEAD);
   msg1=AWEBSTR(MSG_EPART_FLUSHED_MSG);
   msg2=AWEBSTR(MSG_EPART_FLUSHED_NORELOAD);
   len=strlen(flushedmsg)+2*strlen(head)+strlen(url->url)+strlen(msg1)+strlen(msg2);
   if(buffer=ALLOCTYPE(UBYTE,len+16,0))
   {  len=sprintf(buffer,flushedmsg,head,head,url->url,msg1,msg2);
      Asrcupdatetags(url->source,NULL,
         AOURL_Error,TRUE,
         AOURL_Data,(Tag)buffer,
         AOURL_Datalength,len,
         AOURL_Eof,TRUE,
         TAG_END);
      FREE(buffer);
   }
}

/* Determine whether a fast response is allowed for this url */
/* Returns TRUE if afast response *is* allowed               */

static BOOL Fastresponse(struct Url *url)
{
    BOOL fast = TRUE;
    if(url->cache)
    {
        ULONG time;
        if(url->flags&URLF_DEXFETCH)
        {
            fast = FALSE;
        }
        else if(!(time = Agetattr(url->cache,AOCAC_Lastmodified)))
        {
            fast = prefs.network.fastresponse_dyn;
        }
        else
        {
            fast = prefs.network.fastresponse_stat;
        }
    }
    return fast;
}

short Caverify(struct Url *url)
{
    short caverify = CAVERIFY_ALWAYS;
    ULONG time;
    if(url->cache)
    {
        if(!(time = Agetattr(url->cache,AOCAC_Lastmodified)))
        {
            caverify = prefs.network.caverify_dyn;
        }
        else
        {
            caverify = prefs.network.caverify_stat;
        }

    }
    return caverify;
}

/* Initiate the load of this url.
 * If it is a reload, kill everything running and do a plain fetch.
 * Else use this decision table:
 *
 *                1  2   3  4 5  6  7  8
 * Download req   YY NNN NN N NN NN NN NN cdl
 * Document ext   -- NNN NN N NN NN NN YY cdex
 * History req    -- YYY NN N NN NN NN -- chis
 * Ifinmem req    -- --- YY N NN NN NN -- ciim
 * Validate       -- --- -- - NN YY YY -- cval
 * Fast response  -- --- -- - -- NN YY -- cfr
 * Source+driver  -- YNN YN N YN YN YN -- csrc
 * Cache          NY -YN -- N -Y -Y -Y NY ccac
 *
 * Use source     .. X.. XX . X. .. X. .. asrc
 * Save source    XX ... .. . .. .. .. .. assrc
 * Docext source  .. ... .. . .. .. .. XX adsrc
 * New source     .. .XX .. X .X .X .X .. ansrc
 * Rfetch cache   .X .X. .. . .X .. .X .X arf
 * Fetch          X. ..X .. X .. .. .. X. af
 * Vfetch         .. ... .. . .. XX XX .. avf
 *
 */
static long Loadurl(struct Url *url,struct Aumload *auml)
{  long retval=0;
   BOOL nocache=FALSE,expired;
   ULONG veridate=0;
   ULONG expiredate=0;
   BOOL cdl,cdex,chis,ciim,cval,cfr,csrc=FALSE,ccac;
   BOOL asrc=FALSE,assrc=FALSE,adsrc=FALSE,ansrc=FALSE,arf=FALSE,af=FALSE,avf=FALSE;
   BOOL awin=FALSE;  /* Notify window in case of hi-jack ongoing fetch */
   void *window=(void *)Agetattr(auml->frame,AOBJ_Window);
   BYTE *etag = NULL;

   if(auml->flags&(AUMLF_RELOAD|AUMLF_CHANNEL))
   {  /* Break any ongoing fetches */
      Clearobject(&url->fetch);
      Clearobject(&url->rfetch);
      Clearobject(&url->vfetch);
      Clearobject(&url->vcache);
      Clearobject(&url->vsource);
      Clearobject(&url->dsource);
      Clearobject(&url->ocache);
      url->ocache=url->cache;
      url->cache=NULL;
      nocache=TRUE;
      af=TRUE;
      if(auml->flags&AUMLF_DOWNLOAD) assrc=TRUE;
      else if(auml->flags&AUMLF_DOCEXT) adsrc=TRUE;
      else ansrc=TRUE;
      url->movedto=NULL;
      url->flags&=~URLF_TEMPMOVED;
   }
   else /* No reload */
   {
      /* If we are temporarily moved reset our movedto url now */
      if(url->flags&URLF_TEMPMOVED)
      {
          /* For some reason, not even remotely understood we need to clear the cache */
          /* of the previous destination for correct operation. INVESTIGATE */

          Clearobject(&url->movedto->cache);
          url->movedto=NULL;
          url->flags&=~URLF_TEMPMOVED;
      }
      /* Find the resulting (movedto) url. */
      url=Finalurl(url);
      /* Only fetch if no fetch is going on */
      if(!url->fetch && !url->rfetch && !url->vfetch)
      {  /* Set the condition flags */
         cdl=BOOLVAL(auml->flags&AUMLF_DOWNLOAD);
         cdex=BOOLVAL(auml->flags&AUMLF_DOCEXT);
         chis=BOOLVAL(auml->flags&AUMLF_HISTORY);
         ciim=BOOLVAL(auml->flags&AUMLF_IFINMEM);
         expired=(url->flags&URLF_VOLATILE) || Agetattr(url->cache,AOCAC_Expired);

         // Possible Cache improvement:
         // Even if date is expired - we could still use an existing ETAG
         // to verify the cached version of the file.
         // Exiting ETAG overrules
         //
         //if(Agetattr(url->cache,AOCAC_Etag)>0) hasetag=TRUE;

         if(url->cache)
         {
             expiredate=Agetattr(url->cache,AOCAC_Expires);
         }

         cval=(auml->flags&AUMLF_VERIFY) ||     /* Validate if forced by reload etc */
            ((!(expiredate && !expired)) &&     /* Don't validate if it has an expirey date and hasn't expired */
            (Caverify(url)==CAVERIFY_ALWAYS) || /* Otherwise determine from prefa etc */

            (!(url->flags&URLF_VERIFIED) && Caverify(url)!=CAVERIFY_NEVER)
            );
         cfr=Fastresponse(url);

         csrc=url->source && Agetattr(url->source,AOSRC_Driver);
         ccac=url->cache && !expired;
         /* If not history, don't use expired or volatile cache or source */
         // if(!chis && expired && !hasetag)
         if(!chis && expired)
         {  csrc=FALSE;
            ccac=FALSE;
            Clearobject(&url->cache);
         }
         /* Infer the action(s) */
         if(cdl)                          /* Column 1 */
         {  assrc=TRUE;
            if(ccac) arf=TRUE;
            else af=TRUE;
         }
         else if(cdex)                    /* Column 8 */
         {  adsrc=TRUE;
            if(ccac) arf=TRUE;
            else af=TRUE;
         }
         else if(chis)                    /* Column 2 */
         {  if(csrc) asrc=TRUE;
            else
            {  ansrc=TRUE;
               if(ccac) arf=TRUE;
               else af=TRUE;
            }
         }
         else if(ciim)                    /* Column 3 */
         {  asrc=TRUE;
         }
         else if(!csrc && !ccac)          /* Column 4 */
         {  ansrc=TRUE;
            af=TRUE;
         }
         else if(!cval)                   /* Column 5 */
         {  if(csrc) asrc=TRUE;
            else
            {  ansrc=TRUE;
               arf=TRUE;
            }
         }
         else if(!cfr)                    /* Column 6 */
         {  if(!csrc) ansrc=TRUE;
            avf=TRUE;
         }
         else                             /* Column 7 */
         {  if(csrc) asrc=TRUE;
            else
            {  ansrc=TRUE;
               arf=TRUE;
            }
            avf=TRUE;
         }
      }
      else
      {  asrc=TRUE;
         awin=TRUE;
      }
   }
   /* Don't verify POST responses. Always use memory cached copy when available. */
   if(avf && url->postnr && url->flags&(URLF_VISITED))
   {  avf=FALSE;
      if(csrc) asrc=TRUE;
   }
   if(!url->fetch && !url->rfetch && !url->vfetch)
   {  Clearobject(&url->ssource);
   }
   if(assrc && !url->ssource)
   {  url->ssource=Anewobject(AOTP_SOURCE,
         AOSRC_Url,(Tag)url,
         AOSRC_Saveas,TRUE,
         AOSRC_Noicon,BOOLVAL(auml->flags&AUMLF_NOICON),
         TAG_END);
   }
   if(adsrc && !url->dsource)
   {  url->dsource=Anewobject(AOTP_SOURCE,
         AOSRC_Url,(Tag)url,
         AOSRC_Docext,TRUE,
         TAG_END);
   }
   if(ansrc)
   {  Replacesource(url,NULL);
   }
   if(arf)
   {  url->rfetch=Anewobject(AOTP_FETCH,
         AOFCH_Url,(Tag)url,
         AOFCH_Cache,TRUE,
         AOFCH_Name,Agetattr(url->cache,AOCAC_Name),
         AOFCH_Imagefetch,BOOLVAL(auml->flags&AUMLF_IMAGE),
         AOFCH_Windowkey,Agetattr(window,AOWIN_Key),
         TAG_END);
      Asetattrs(url->cache,
         AOCAC_Touched,TRUE,
         AOCAC_Sendinfo,(Tag)url->rfetch,
         TAG_END);
      if(!assrc && !prefs.network.ignoremime)
      {  Asrcupdatetags(url->source,url->rfetch,
            AOURL_Contenttype,Agetattr(url->cache,AOCAC_Contenttype),
            AOURL_Charset,Agetattr(url->cache,AOCAC_Charset),
            TAG_END);
      }
   }
   if(af)
   {  if(url->postnr && url->flags&(URLF_VISITED))
      {  Norepost(url);
      }
      else
      {  if(!(url->fetch=Anewobject(AOTP_FETCH,
            AOFCH_Url,(Tag)url,
            AOFCH_Name,(Tag)url->url,
            AOFCH_Nocache,nocache,
            AOFCH_Referer,auml->referer?(Tag)((struct Url *)auml->referer)->url:0,
            (auml->flags&AUMLF_MULTIPART)?AOFCH_Multipartdata:AOFCH_Postmsg,
               url->postnr?(Tag)auml->postmsg:0,
            AOFCH_Windowkey,Agetattr(window,AOWIN_Key),
            AOFCH_Noproxy,BOOLVAL(auml->flags&AUMLF_NOPROXY),
            AOFCH_Loadflags,auml->flags,
            AOFCH_Imagefetch,BOOLVAL(auml->flags&AUMLF_IMAGE),
            AOFCH_Commands,Agetattr(window,AOWIN_Commands),
            AOFCH_Jframe,(Tag)auml->frame,
            AOFCH_Formwarn,BOOLVAL(auml->flags&AUMLF_FORMWARN),
            AOFCH_Channel,BOOLVAL(auml->flags&AUMLF_CHANNEL),
            TAG_END)))
         {  Notifychilds(url,AOREL_URL_WINDOW,AOWIN_Activeurl,NULL,TAG_END);
            Inputallwindows(FALSE);
         }
         if(url->fetch && (url->flags&URLF_CACHEABLE) && !assrc)
         {  url->cache=Anewobject(AOTP_CACHE,
               AOCAC_Url,(Tag)url,
               TAG_END);
         }
         if(auml->flags&AUMLF_CHANNEL)
         {  retval=Agetattr(url->fetch,AOFCH_Channelid);
         }
      }
   }
   if(avf)
   {  if(url->cache){
        veridate=Agetattr(url->cache,AOCAC_Lastmodified);
        etag=(UBYTE *)Agetattr(url->cache,AOCAC_Etag);
        //   printf("=> etag %s\n\n",etag);
      }
      else
      {  /* For local files, use date from source. */
         if(url->source &&
            (STRNIEQUAL(url->url,"FILE://LOCALHOST/",17) ||
             STRNIEQUAL(url->url,"FILE:///",8)))
         {  veridate=Agetattr(url->source,AOSRC_Lastmodified);
         }
      }
      url->vfetch=Anewobject(AOTP_FETCH,
         AOFCH_Url,(Tag)url,
         AOFCH_Name,(Tag)url->url,
         AOFCH_Ifmodifiedsince,veridate,
         AOFCH_Etag,(Tag)etag,
         AOFCH_Referer,auml->referer?(Tag)((struct Url *)auml->referer)->url:0,
         AOFCH_Noproxy,BOOLVAL(auml->flags&AUMLF_NOPROXY),
         AOFCH_Loadflags,auml->flags,
         AOFCH_Imagefetch,BOOLVAL(auml->flags&AUMLF_IMAGE),
         AOFCH_Windowkey,Agetattr(window,AOWIN_Key),
         AOFCH_Formwarn,BOOLVAL(auml->flags&AUMLF_FORMWARN),
         TAG_END);
      if(url->vfetch)
      {  if(url->flags&URLF_CACHEABLE)
         {  url->vcache=Anewobject(AOTP_CACHE,
               AOCAC_Url,(Tag)url,
               TAG_END);
         }
         url->vsource=Anewobject(AOTP_SOURCE,
            AOSRC_Url,(Tag)url,
            TAG_END);
      }
   }
   if((af || avf) && !assrc)
   {  url->loadnr=++loadnr;
   }
   if(af || arf)
   {  SETFLAG(url->flags,URLF_DEXFETCH,adsrc);
   }
   if(arf || af || avf || awin)
   {  Inputallwindows(TRUE);
   }
   if(asrc)
   {  Asetattrs(url->source,AOSRC_Usedriver,TRUE,TAG_END);
   }

   return retval;
}

/* Perform special operations */
static long Specialurl(struct Url *url,struct Aumspecial *aums)
{  switch(aums->type)
   {  case AUMST_SAVESOURCE:
         url=Finalurl(url);
         Asetattrs(url->source,AOSRC_Savesource,TRUE,TAG_END);
         break;
      case AUMST_VIEWSOURCE:
         url=Finalurl(url);
         Asetattrs(url->source,AOSRC_Viewsource,TRUE,TAG_END);
         break;
      case AUMST_CANCELFETCH:
         url=Finalurl(url);
         if(url->fetch) Asetattrs(url->fetch,AOFCH_Cancel,TRUE,TAG_END);
         if(url->rfetch) Asetattrs(url->rfetch,AOFCH_Cancel,TRUE,TAG_END);
         if(url->vfetch) Asetattrs(url->vfetch,AOFCH_Cancel,TRUE,TAG_END);
         break;
      case AUMST_DELETECACHE:
         Clearobject(&url->cache);
         break;
      case AUMST_EDITSOURCE:
         url=Finalurl(url);
         Asetattrs(url->source,AOSRC_Editsource,TRUE,TAG_END);
         break;
      case AUMST_FLUSHSOURCE:
         Asetattrs(url->source,AOSRC_Flush,TRUE,TAG_END);
         Doupdateframes();
         break;
   }
   return 0;
}

/* Find the content type of this object */
static UBYTE *Urlcontenttype(struct Url *url)
{  UBYTE *type=NULL;
   if(url->cache && !prefs.network.ignoremime)
   {  type=(UBYTE *)Agetattr(url->cache,AOCAC_Contenttype);
   }
   if(!type) type=(UBYTE *)Agetattr(url->source,AOURL_Contenttype);
   if(!type) type=Mimetypefromext(url->url);
   return type;
}

/* Find the character set of this object */
static UBYTE *Urlcharset(struct Url *url)
{  UBYTE *charset=NULL;
   if(url->cache && !prefs.network.ignoremime)
   {  charset=(UBYTE *)Agetattr(url->cache,AOCAC_Charset);
   }
   if(!charset) charset=(UBYTE *)Agetattr(url->source,AOSRC_Charset);
   return charset;
}

/* Find the Etag of this object */
static UBYTE *Urletag(struct Url *url)
{  UBYTE *etag=NULL;
   etag=(UBYTE *)Agetattr(url->source,AOSRC_Etag);
   return etag;
}


/* Move this url. */
static void Moveurl(struct Url *url,UBYTE *newurl,BOOL temp,BOOL seeother,void *fetch)
{  ULONG loadflags=0;
   UBYTE *referer=NULL,*postmsg=NULL;
   struct Multipartdata *mpd=NULL;
   Agetattrs(fetch,
      AOFCH_Referer,(Tag)&referer,
      seeother?TAG_IGNORE:AOFCH_Postmsg,(Tag)&postmsg,
      seeother?TAG_IGNORE:AOFCH_Multipartdata,(Tag)&mpd,
      AOFCH_Loadflags,(Tag)&loadflags,
      TAG_END);
   if(prefs.browser.htmlmode==HTML_COMPATIBLE) postmsg=NULL;

#if 0
   /* If the old URL doesn't have a cache object attached any more, there
    * must have been a Pragma: no-cache header. Make the url volatile. */
   if(!url->cache) url->flags|=URLF_VOLATILE;
#endif

   /* If the new URL contains a query part, it's probably meant to be
    * retrieved with a GET request. */
   if(postmsg && strchr(newurl,'?')) postmsg=NULL;
   if(url->movedto=Findurl(url->url,newurl,postmsg?-1:0))
   {  if(!Movedtoloop(url))
      {  if(temp) url->flags|=URLF_TEMPMOVED;
         else url->flags&=~URLF_TEMPMOVED;
         if(url->source)
         {  Asetattrs(url->source,
               AOSRC_Movetourl,(Tag)url->movedto,
               TAG_END);
            Adisposeobject(url->source);
            url->source=NULL;
         }
         Notifychilds(url,AOREL_URL_WINDOW,AOWIN_Activeurl,url->movedto,TAG_END);
         /* In case of multipart data, the loadflags are already set ok. Just use
          * mpd instead of postmsg */
         Auload(url->movedto,loadflags,referer?Findurl("",referer,0):NULL,
            mpd?(UBYTE *)mpd:postmsg,NULL);
         if(url->ssource)
         {  Asetattrs(url->ssource,
               AOSRC_Movetourl,(Tag)url->movedto,
               TAG_END);
            Adisposeobject(url->ssource);
            url->ssource=NULL;
         }
         if(url->dsource)
         {  Asetattrs(url->dsource,
               AOSRC_Movetourl,(Tag)url->movedto,
               TAG_END);
            Adisposeobject(url->dsource);
            url->dsource=NULL;
         }
      }
   }
   Changedlayout();
}

/* Process verify input */
static void Srcupdatevfetch(struct Url *url,struct Amsrcupdate *ams)
{  struct TagItem *wtags,*tag,*tstate=ams->tags;
   UBYTE *referer;
   ULONG loadflags=0;
   BOOL header=FALSE;
   while(url->vfetch && (tag=NextTagItem(&tstate)))
   {  switch(tag->ti_Tag)
      {  case AOURL_Data:
         case AOURL_Movedto:
         case AOURL_Tempmovedto:
            /* Modified data on a verify. Use it and kill any rfetch. */
            if(url->rfetch)
            {  Notifychilds(url,AOREL_URL_WINDOW,AOWIN_Status,NULL,TAG_END);
               Clearobject(&url->rfetch);
            }
            if(url->cache)
            {  Adisposeobject(url->cache);
            }
            url->fetch=url->vfetch;
            url->cache=url->vcache;
            Replacesource(url,url->vsource);
            url->vfetch=NULL;
            url->vcache=NULL;
            url->vsource=NULL;
            url->flags|=URLF_VERIFIED;
            break;
         case AOURL_Error:
            /* Don't use an error validation response. */
            if(!tag->ti_Data) break;
            Adisposeobject(url->vfetch);
            /* Will NOT return to us with AOURL_Terminate set
             * so fall through: */
         case AOURL_Terminate:
            /* Not modified. Dispose vcache, start reload if slow verify. */
            url->vfetch=NULL;
            url->flags|=URLF_VERIFIED;
            Clearobject(&url->vcache);
            Clearobject(&url->vsource);
            if(!url->rfetch)
            {  Notifychilds(url,AOREL_URL_WINDOW,AOWIN_Activeurl,NULL,TAG_END);
               Inputallwindows(FALSE);
            }
            else
            {  Notifychilds(url,AOREL_URL_WINDOW,AOWIN_Status,NULL,TAG_END);
            }
            if(!Fastresponse(url))
            {  /* Let load logic decide to use source or cache copy.
                * Pass HISTORY flag so it won't start another verify */
               Agetattrs(ams->fetch,
                  AOFCH_Referer,(Tag)&referer,
                  AOFCH_Loadflags,(Tag)&loadflags,
                  TAG_END);
               loadflags&=AUMLF_NOPROXY;
               Auload(url,AUMLF_HISTORY|loadflags,
                  referer?Findurl("",referer,0):NULL,NULL,NULL);
               Changedlayout();
            }
            break;
         case AOURL_Lastmodified:
            Asetattrs(url->vsource,AOSRC_Lastmodified,tag->ti_Data,TAG_END);
            break;
         case AOURL_Header:
            header=TRUE;
            break;
      }
   }
   if(!url->rfetch)
   {  if(wtags=CloneTagItems(ams->tags))
      {  MapTags(wtags,mapwindowtags,MAP_REMOVE_NOT_FOUND);
         Notifychilds(url,AOREL_URL_WINDOW,TAG_MORE,wtags);
         FreeTagItems(wtags);
      }
   }
   if(url->vcache)
   {  AmethodA(url->vcache,(struct Amessage *)ams);
   }
   if(header)
   {  AmethodA(url->vsource,(struct Amessage *)ams);
   }
}

/* Update from normal fetch or cache reload. Only forward to cache if (cache) is TRUE. */
static void Srcupdatefetch(struct Url *url,struct Amsrcupdate *ams,BOOL cache)
{  struct TagItem *wtags,*tag,*tstate=ams->tags;
   BOOL terminate=FALSE;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOURL_Movedto:
            Moveurl(url,(UBYTE *)tag->ti_Data,FALSE,FALSE,ams->fetch);
            Urlvisited(url);
            break;
         case AOURL_Tempmovedto:
            Moveurl(url,(UBYTE *)tag->ti_Data,TRUE,FALSE,ams->fetch);
            Urlvisited(url);
            break;
         case AOURL_Seeother:
            Moveurl(url,(UBYTE *)tag->ti_Data,TRUE,TRUE,ams->fetch);
            Urlvisited(url);
            break;
         case AOURL_Terminate:
            terminate=TRUE;
            url->flags|=URLF_VERIFIED;
            Clearobject(&url->ocache);
            break;
         case AOURL_Error:
            if(tag->ti_Data && cache)
            {  /* Don't cache error response */
               Clearobject(&url->cache);
               /* Restore old cache if it exists */
               url->cache=url->ocache;
               url->ocache=NULL;
            }
            break;
         case AOURL_Data:
            Urlvisited(url);
            break;
         case AOURL_Reload:
            Clearobject(&url->cache);
            if(url->ssource)
            {  Adisposeobject(url->ssource);
               url->ssource=Anewobject(AOTP_SOURCE,
                  AOSRC_Url, (Tag)url,
                  AOSRC_Saveas,TRUE,
                  TAG_END);
            }
            else if(url->dsource)
            {  Adisposeobject(url->dsource);
               url->dsource=Anewobject(AOTP_SOURCE,
                  AOSRC_Url,(Tag)url,
                  AOSRC_Docext,TRUE,
                  TAG_END);
            }
            else
            {  Replacesource(url,NULL);
            }
            if(url->flags&URLF_CACHEABLE)
            {  url->cache=Anewobject(AOTP_CACHE,AOCAC_Url,(Tag)url,TAG_END);
            }
            break;
         case AOURL_Nocache:
            if(tag->ti_Data)
            {  Clearobject(&url->cache);
            }
            break;
         case AOURL_Flushsource:
            if(tag->ti_Data && url->source)
            {
                Asetattrs(url->source,AOSRC_Flush,TRUE,TAG_END);
                Doupdateframes();
            }
            break;
      }
   }
   if(wtags=CloneTagItems(ams->tags))
   {  MapTags(wtags,mapwindowtags,MAP_REMOVE_NOT_FOUND);
      Notifychilds(url,AOREL_URL_WINDOW,TAG_MORE,wtags);
      FreeTagItems(wtags);
   }
   if(terminate)
   {  url->fetch=NULL;
      url->rfetch=NULL;
      if(!url->vfetch)
      {  Notifychilds(url,AOREL_URL_WINDOW,AOWIN_Activeurl,NULL,TAG_END);
         Inputallwindows(FALSE);
      }
      else
      {  Notifychilds(url,AOREL_URL_WINDOW,AOWIN_Status,NULL,TAG_END);
      }
   }

   /* Update our source; create a new one if none exists, except if we are moved */
   if(url->ssource)
   {  AmethodA(url->ssource,(struct Amessage *)ams);
   }
   if(url->dsource)
   {  AmethodA(url->dsource,(struct Amessage *)ams);
   }
   if(!url->ssource)
   {  /* Update our cache too */
      if(cache && url->cache)
      {  AmethodA(url->cache,(struct Amessage *)ams);
      }
   }
   if(!url->ssource && !(url->flags&URLF_DEXFETCH))
   {
      if(url->source || !url->movedto)
      {  AmethodA(Urlsource(url),(struct Amessage *)ams);
      }
      Changedlayout();
   }
}

/* Check for duplicate URL in the same list. Then queue a check for the next
 * url in this list, or for the first url in the next list. */
static void Dupcheck(struct Url *url)
{  struct Url *u;
   for(u=url->object.next;u->object.next;u=u->object.next)
   {  if(STRIEQUAL(url->url,u->url) && url->postnr==u->postnr)
      {  Clearobject(&u->cache);
         u->movedto=url;
         u->flags&=~URLF_TEMPMOVED;
         u->flags|=URLF_WASDUP;
      }
   }
   u=url->object.next;
   if(!u->object.next)
   {  for(dupchecklist++;dupchecklist<NRLISTS;dupchecklist++)
      {  if(!ISEMPTY(&urls[dupchecklist]))
         {  u=urls[dupchecklist].first;
            break;
         }
      }
   }
   if(u->object.next) Queuesetmsg(u,UQID_DUPCHECK);
}

/*---------------------------------------------------------------------*/

static long Seturl(struct Url *url,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOURL_Url:
            if(!url->url) url->url=Dupstr((UBYTE *)tag->ti_Data,-1);
            break;
         case AOURL_Postnr:
            url->postnr=tag->ti_Data;
            break;
         case AOURL_Cacheable:
            if(tag->ti_Data) url->flags|=URLF_CACHEABLE;
            else url->flags&=~URLF_CACHEABLE;
            break;
         case AOURL_Movedto:  /* Set by cache when reading in registration */
            url->movedto=Findurl("",(UBYTE *)tag->ti_Data,0);
            url->flags&=~URLF_TEMPMOVED;
            break;
         case AOURL_Tempmovedto:  /* Set by cache when reading in registration */
            url->movedto=Findurl("",(UBYTE *)tag->ti_Data,0);
            url->flags|=URLF_TEMPMOVED;
            break;
         case AOURL_Cache:
            if((url->flags&URLF_CACHEABLE) && !url->cache)
            {  url->cache=(void *)tag->ti_Data;
            }
            break;
         case AOURL_Visited:
            if(tag->ti_Data) url->flags|=URLF_VISITED;
            else url->flags&=~URLF_VISITED;
            break;
         case AOURL_Cachefetch:
            if(!tag->ti_Data && !url->rfetch)
            {  Clearobject(&url->cache);
            }
            break;
         case AOURL_Volatile:
            SETFLAG(url->flags,URLF_VOLATILE,tag->ti_Data);
            break;
         case AOBJ_Queueid:
            if(tag->ti_Data==UQID_DUPCHECK)
            {  Dupcheck(url);
            }
            break;
      }
   }
   if(url->postnr)
   {  if(url->postnr<0) url->postnr=++lastpostnr;
      url->flags&=~URLF_CACHEABLE;
   }
   if(!(url->flags&URLF_CACHEABLE))
   {  Clearobject(&url->cache);
   }
   return 0;
}

static long Geturl(struct Url *url,struct Amset *ams)
{  struct TagItem *tag,*tstate=ams->tags;
   while(tag=NextTagItem(&tstate))
   {  switch(tag->ti_Tag)
      {  case AOURL_Url:
            PUTATTR(tag,Urladdress(url));
            break;
         case AOURL_Linkurl:
            PUTATTR(tag,Urllinkaddress(url));
            break;
         case AOURL_Postnr:
            PUTATTR(tag,url->postnr);
            break;
         case AOURL_Visited:
            PUTATTR(tag,BOOLVAL(url->flags&URLF_VISITED));
            break;
         case AOURL_Source:
            PUTATTR(tag,Urlsource(url));
            break;
         case AOURL_Contenttype:
            PUTATTR(tag,Urlcontenttype(url));
            break;
         case AOURL_Cacheable:
            PUTATTR(tag,BOOLVAL(url->flags&URLF_CACHEABLE));
            break;
         case AOURL_Movedto:
            if(url->movedto && !(url->flags&URLF_TEMPMOVED))
            {  PUTATTR(tag,url->movedto->url);
            }
            break;
         case AOURL_Tempmovedto:
            if(url->movedto && (url->flags&URLF_TEMPMOVED))
            {  PUTATTR(tag,url->movedto->url);
            }
            break;
         case AOURL_Realurl:
            PUTATTR(tag,url->url);
            break;
         case AOURL_Loadnr:
            PUTATTR(tag,url->loadnr);
            break;
         case AOURL_Cache:
            PUTATTR(tag,url->cache);
            break;
         case AOURL_Isinmem:
            PUTATTR(tag,url->source && Agetattr(url->source,AOSRC_Driver));
            break;
         case AOURL_Sourcevalid:
            /* If no cache, it's not expired so it's valid. */
            /* Why the (!url->fetch)?? Why wouldn't a source be valid when
             * a fetch is still going on? Either it is the new source, which
             * will be valid, or it is still the old one, which will replace
             * itself when new data arrives. Note this attr is only gotten in
             * frame.c:Setnewwinhis()
             * Leaving the !url->fetch in causes loops when location is set
             * in a script while the fetch is still going on. */
            PUTATTR(tag,/**!url->fetch &&**/ !Agetattr(url->cache,AOCAC_Expired));
            break;
         case AOURL_Saveassource:
            PUTATTR(tag,url->ssource);
            break;
         case AOURL_Input:
            {  struct Url *u=Finalurl(url);
               PUTATTR(tag,u->fetch || u->rfetch || u->vfetch);
            }
            break;
         case AOURL_Docextsource:
            PUTATTR(tag,url->dsource);
            break;
         case AOURL_Finalurl:
            PUTATTR(tag,Finalurl(url)->url);
            break;
         case AOURL_Finalurlptr:
            PUTATTR(tag,Finalurl2(url));
            break;
         case AOURL_Charset:
            PUTATTR(tag,Urlcharset(url));
            break;
         case AOURL_Etag:
            PUTATTR(tag,Urletag(url));
            break;
      }
   }
   return 0;
}

static struct Url *Newurl(struct Amset *ams)
{  struct Url *url=Allocobject(AOTP_URL,sizeof(struct Url),ams);
   if(url)
   {  NEWLIST(&url->childs);
      url->flags|=URLF_CACHEABLE;
      Seturl(url,ams);
      ADDHEAD(&urls[Hash(url->url)],url);
   }
   return url;
}

static long Srcupdateurl(struct Url *url,struct Amsrcupdate *ams)
{  long result=0;
   Busypointer(TRUE);
   if(ams->fetch==url->vfetch)
   {  Srcupdatevfetch(url,ams);
   }
   if(ams->fetch==url->fetch)
   {  Srcupdatefetch(url,ams,TRUE);
   }
   if(ams->fetch==url->rfetch)
   {  Srcupdatefetch(url,ams,FALSE);
   }
   Busypointer(FALSE);
   return result;
}

static long Addchildurl(struct Url *url,struct Amadd *ama)
{  struct Child *ch=ALLOCSTRUCT(Child,1,MEMF_CLEAR);
   if(ch)
   {  ch->object=ama->child;
      ch->relation=ama->relation;
      ADDTAIL(&url->childs,ch);
   }
   return 0;
}

static long Remchildurl(struct Url *url,struct Amadd *ama)
{  struct Child *ch;
   for(ch=url->childs.first;ch->next;ch=ch->next)
   {  if(ch->object==ama->child && ch->relation==ama->relation)
      {  REMOVE(ch);
         FREE(ch);
         break;
      }
   }
   return 0;
}

static void Disposeurl(struct Url *url)
{  struct Child *ch;
   while(ch=(struct Child *)REMHEAD(&url->childs))
   {  switch(ch->relation)
      {  case AOREL_URL_LINK:
            Asetattrs(ch->object,AOLNK_Url,(Tag)NULL,TAG_END);
            break;
         case AOREL_URL_WINDOW:
            Asetattrs(ch->object,AOWIN_Activeurl, 0 ,TAG_END);
            break;
      }
      FREE(ch);
   }
   Clearobject(&url->fetch);
   Clearobject(&url->rfetch);
   Clearobject(&url->vfetch);
   Clearobject(&url->source);
   Clearobject(&url->vsource);
   Clearobject(&url->ssource);
   Clearobject(&url->dsource);
   /* Cache objects will be disposed of by cache */
   if(url->url) FREE(url->url);
   Amethodas(AOTP_OBJECT,url,AOM_DISPOSE);
}

static void Deinstall(void)
{  short i;
   void *p;
   for(i=0;i<NRLISTS;i++)
   {  if(urls[i].first)
      {  while(p=REMHEAD(&urls[i])) Disposeurl(p);
      }
   }
}

USRFUNC_H2
(
static long  , Url_Dispatcher,
struct Url *,url,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_NEW:
         result=(long)Newurl((struct Amset *)amsg);
         break;
      case AOM_SET:
         result=Seturl(url,(struct Amset *)amsg);
         break;
      case AOM_GET:
         result=Geturl(url,(struct Amset *)amsg);
         break;
      case AOM_SRCUPDATE:
         result=Srcupdateurl(url,(struct Amsrcupdate *)amsg);
         break;
      case AOM_ADDCHILD:
         result=Addchildurl(url,(struct Amadd *)amsg);
         break;
      case AOM_REMCHILD:
         result=Remchildurl(url,(struct Amadd *)amsg);
         break;
      case AOM_DISPOSE:
         Disposeurl(url);
         break;
      case AOM_DEINSTALL:
         Deinstall();
         break;
      case AUM_LOAD:
         result=Loadurl(url,(struct Aumload *)amsg);
         break;
      case AUM_SPECIAL:
         result=Specialurl(url,(struct Aumspecial *)amsg);
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*---------------------------------------------------------------------*/

BOOL Installurl(void)
{  short i;
   for(i=0;i<NRLISTS;i++)
   {  NEWLIST(&urls[i]);
   }
   if(!Amethod(NULL,AOM_INSTALL,AOTP_URL,(Tag)Url_Dispatcher)) return FALSE;
   return TRUE;
}

BOOL Initurl(void)
{
    emptyurl = Anewobject
    (
        AOTP_URL, AOURL_Url,
        (ULONG)"", TAG_END
    );

   if(!emptyurl) return FALSE;
   Replacesource(emptyurl,NULL);
   Asrcupdatetags(emptyurl->source,NULL,
      AOURL_Contenttype,(Tag)"text/plain",
      AOURL_Data,(Tag)"",
      AOURL_Datalength,0,
      TAG_END);
   Asrcupdatetags(emptyurl->source,NULL,
      AOURL_Eof,TRUE,
      AOURL_Terminate,TRUE,
      TAG_END);
   Asetattrs(emptyurl->source,AOSRC_Noflush,TRUE,TAG_END);
   return TRUE;
}

BOOL Initurl2(void)
{  for(dupchecklist=0;dupchecklist<NRLISTS;dupchecklist++)
   {  if(!ISEMPTY(&urls[dupchecklist]))
      {  Queuesetmsg(urls[dupchecklist].first,UQID_DUPCHECK);
         break;
      }
   }
   return TRUE;
}

void *Findurlloadnr(long loadnr)
{  struct Url *url;
   short i;
   for(i=0;i<NRLISTS;i++)
   {  for(url=urls[i].first;url->object.next;url=url->object.next)
      {  if(url->loadnr==loadnr) return url;
      }
   }
   return NULL;
}

UBYTE *Makeabsurl(UBYTE *base,UBYTE *url)
{  Buildabsurl(base?base:NULLSTRING,url);
   return Dupstr(absurl,-1);
}

UBYTE *Urlfilename(UBYTE *url)
{  UBYTE *p,*q;
   for(p=url;*p && *p!=';' && *p!='?' && *p!='#';p++);
   for(q=p-1;q>=url && *q!='/' && *q!=':';q--);
   if(q>url && *(q-1)!='/') return Dupstr(q+1,p-q-1);
   else return Dupstr("",0);
}

UBYTE *Urlfilenamefb(UBYTE *url)
{  UBYTE *p,*q;
   for(p=url;*p && *p!=';' && *p!='?' && *p!='#';p++);
   p--;
   if(p>=url && *p=='/') p--;
   for(q=p;q>=url && *q!='/';q--);
   if(q>url) return Dupstr(q+1,p-q);
   p=strchr(url,':');
   if(p && p[1]) return Dupstr(p+1,strlen(p+1));
   if(!p) p=url+strlen(url);
   return Dupstr(url,p-url);
}

UBYTE *Urlfileext(UBYTE *url)
{  UBYTE *p,*q;
   for(p=url;*p && *p!=':';p++);
   if(p[0]==':' && p[1]=='/' && p[2]=='/')
   {  for(p+=3;*p && *p!='/';p++);
      if(*p)
      {  for(p++;*p && *p!=';' && *p!='?' && *p!='#';p++);
         for(q=p-1;q>=url && *q!='/' && *q!='.';q--);
         if(q>=url && *q=='.') return Dupstr(q+1,p-q-1);
      }
   }
   return NULL;
}

UBYTE *Fragmentpart(UBYTE *url)
{  UBYTE *p;
   /* Don't allow fragments in news: URLs (message ID can contain #) */
   if(!STRNIEQUAL(url,"NEWS:",5))
   {  if(p=strchr(url,'#')) return p+1;
   }
   return NULL;
}

void *Findurl(UBYTE *base,UBYTE *url,long postnr)
{  struct Url *u;
   UBYTE *absurl=Makeabsurl(base,url);
   UBYTE hash=0;
   hash=Hash(absurl);
   for(u=urls[hash].first;u->object.next;u=u->object.next)
   {  if(STREQUAL(u->url,absurl) && u->postnr==postnr)
      {  FREE(absurl);
         return u;
      }
   }
   if(postnr<0) postnr=++lastpostnr;
   else postnr=0;
   u=Anewobject(AOTP_URL,
      AOURL_Url,(Tag)absurl,
      AOURL_Postnr,postnr,
      TAG_END);
   FREE(absurl);
   return u;
}

UBYTE *Fixurlname(UBYTE *name)
{  UBYTE *p,*begin=NULL,*end=NULL,*fixname;
   short scheme=0;   /* 0=unknown, 1=found, 2=not found */
   long len;
   for(p=name;*p;p++)
   {  if(!begin && !isspace(*p)) begin=p;
      if(begin)
      {  if(!isspace(*p)) end=p+1;
         if(!scheme)
         {  if(*p==':') scheme=1;
            else if(*p=='/') scheme=2;
         }
      }
   }
   len=end-begin;
   if(scheme!=1) len+=7;
   if(fixname=ALLOCTYPE(UBYTE,len+1,0))
   {  *p='\0';
      if(scheme!=1) strcat(fixname,"http://");
      strncat(fixname,begin,end-begin);
   }
   return fixname;
}

UBYTE *Urllocalfilename(UBYTE *url)
{  UBYTE *p;
   if(STRNIEQUAL(url,"FILE://",7))
   {  if(p=strchr(url+7,'/'))
      {  return p+1;
      }
   }
   return NULL;
}

UBYTE *Urladdfragment(UBYTE *url,UBYTE *fragment)
{  UBYTE *p;
   long l=0;
   if(url) l=strlen(url);
   if(fragment) l+=strlen(fragment)+1;
   if(p=ALLOCTYPE(UBYTE,l+1,0))
   {  if(url) strcpy(p,url);
      else *p='\0';
      if(fragment)
      {  strcat(p,"#");
         strcat(p,fragment);
      }
   }
   return p;
}

long Auload(void *url,ULONG flags,void *referer,UBYTE *postmsg,void *frame)
{  struct Aumload auml={{0}};
   auml.amsg.method=AUM_LOAD;
   auml.flags=flags;
   auml.referer=referer;
   auml.postmsg=postmsg;
   auml.frame=frame;
   return (long)AmethodA(url,(struct Amessage *)&auml);
}

long Auspecial(void *url,ULONG type)
{  struct Aumspecial aums={{0}};
   aums.amsg.method=AUM_SPECIAL;
   aums.type=type;
   return (long)AmethodA(url,(struct Amessage *)&aums);
}

void Getjspart(struct Url *url,UWORD which,UBYTE **start,long *length)
{

   UBYTE *p;
   *start=NULL;
   *length=0;
   /* Do a sanity check as Commonlocproperty) in framejs.c sometimes call
      this with url = NULL */

   if(url && url->url)
   {
       p = url->url;
       if(STRNIEQUAL(p,"x-jsgenerated:",14))
       {  for(p+=14;*p && *p!='/';p++);
          if(*p) p++;
       }
       if(which==UJP_PROTOCOL) *start=p;
       for(;*p && *p!=':';p++);
       if(which==UJP_PROTOCOL)
       {
          *length=p + 1 - *start;
          return;
       }
       if(*p==':') p++;
       if(p[0]=='/' && p[1]=='/') p+=2;
       if(which==UJP_HOST || which==UJP_HOSTNAME) *start=p;
       for(;*p && *p!='/' && *p!=':' && *p!='?';p++);
       if(which==UJP_HOST)
       {  *length=p-*start;
          return;
       }
       if(*p==':') p++;
       if(which==UJP_PORT) *start=p;
       for(;*p && *p!='/';p++);
       if(which==UJP_PORT || which==UJP_HOSTNAME)
       {  *length=p-*start;
          return;
       }
       if(which==UJP_PATHNAME) *start=p;
       for(;*p && *p!='?' && *p!='#';p++);
       if(which==UJP_PATHNAME)
       {  *length=p-*start;
          return;
       }
       if(which==UJP_SEARCH) *start=p;
       for(;*p && *p!='#';p++);
       if(which==UJP_SEARCH)
       {  *length=p-*start;
          return;
       }

   }
}

void *Repjspart(struct Url *url,UWORD which,UBYTE *part)
{  UBYTE *buf,*p,*q,*start;
   long l=strlen(url->url)+strlen(part)+4;
   void *newurl=url;

   if(url && url->url)
   {
       if(buf=ALLOCTYPE(UBYTE,l,0))
       {  p=url->url;
          q=buf;
          start=q;
          for(;*p && *p!=':';*q++=*p++);
          if(which==UJP_PROTOCOL)
          {  strcpy(start,part);
             if((q = strchr(start,':')) != 0)
             {
                *q ='\0';
             }
             q=start+strlen(start);
          }
          if(*p==':') *q++=*p++;
          if(p[0]=='/' && p[1]=='/')
          {  *q++=*p++;
             *q++=*p++;
          }
          start=q;
          for(;*p && *p!='/' && *p!=':' && *p!='?';*q++=*p++);
          if(which==UJP_HOST)
          {  strcpy(start,part);
             q=start+strlen(start);
          }
          if(*p==':') *q++=*p++;
          if(which==UJP_PORT) start=q;
          for(;*p && *p!='/';*q++=*p++);
          if(which==UJP_HOSTNAME || which==UJP_PORT)
          {  strcpy(start,part);
             q=start+strlen(start);
          }
          start=q;
          for(;*p && *p!='?' && *p!='#';*q++=*p++);
          if(which==UJP_PATHNAME)
          {  if(*part!='/') *start++='/';
             strcpy(start,part);
             q=start+strlen(start);
          }
          start=q;
          for(;*p && *p!='#';*q++=*p++);
          if(which==UJP_SEARCH)
          {  if(*part!='?') *start++='?';
             strcpy(start,part);
             q=start+strlen(start);
          }
          *q='\0';
          newurl=Findurl("",buf,0);
          FREE(buf);
       }
   }
   return newurl;
}

void *Emptyurl(void)
{  return emptyurl;
}

BOOL Mailnewsurl(UBYTE *url)
{  if(STRNIEQUAL(url,"mailto:",7))
   {  return TRUE;
   }
   if(STRNIEQUAL(url,"news:",5) && !strchr(url,'@'))
   {  return TRUE;
   }
   return FALSE;
}

BOOL Issamehost(struct Url *url1,struct Url *url2)
{  char *p,*q;
   if(!url1 || !url2) return FALSE;
   p=url1->url;
   q=url2->url;
   if(!p || !q) return FALSE;
   if(STRNIEQUAL(p,"HTTP://",7)) p+=7;
   else if(STRNIEQUAL(p,"HTTPS://",8)) p+=8;
   else if(STRNIEQUAL(p,"FTP://",6)) p+=6;
   else if(STRNIEQUAL(p,"FILE://",7)) p+=7;
   else return FALSE;
   if(STRNIEQUAL(q,"HTTP://",7)) q+=7;
   else if(STRNIEQUAL(q,"HTTPS://",8)) q+=8;
   else if(STRNIEQUAL(q,"FTP://",6)) q+=6;
   else if(STRNIEQUAL(q,"FILE://",7)) q+=7;
   else return FALSE;
   while(*p && *q && *p!=':' && *p!='/' && *q!=':' && *q!='/' && *p==*q)
   {  p++;
      q++;
   }
   if((!*p || *p==':' || *p=='/') && (!*q || *q==':' || *q=='/')) return TRUE;
   return FALSE;
}
