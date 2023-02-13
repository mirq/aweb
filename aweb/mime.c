/*****************************s*****************************************
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

/* mime.c aweb mime types */

#include "aweb.h"

struct Mime
{  NODE(Mime);
   UBYTE mimetype[32];
   UWORD driver;
   UBYTE *cmd;
   UBYTE *args;
   LIST(Mimextens) extensions;
};

struct Mimextens
{  NODE(Mimextens);
   UBYTE ext[16];
};

static LIST(Mime) mimes;
static BOOL inited;

/*-----------------------------------------------------------------------*/

static void Freemimetype(struct Mime *m)
{  struct Mimextens *me;
   while(me=(struct Mimextens *)REMHEAD(&m->extensions)) FREE(me);
   if(m->cmd) FREE(m->cmd);
   if(m->args) FREE(m->args);
   FREE(m);
}

static UBYTE *Getextension(UBYTE *url)
{  static UBYTE extbuf[16];
   UBYTE *path,*ext,*end;
   if(!url) return NULL;
   path=url;
   end=path;
   while(*end && *end!=';' && *end!='?' && *end!='#') end++;
   ext=end-1;
   while(ext>path && *ext!='.') ext--;
   if(end-ext>15) return NULL;
   memmove(extbuf,ext+1,end-ext-1);
   extbuf[end-ext-1]='\0';
   return extbuf;
}

static void Defaultmimes(void)
{  UBYTE exts[16];
   strcpy(exts,"htm html");
   Addmimetype("TEXT/HTML",exts,MDRIVER_INTERNAL,NULL,NULL);
   strcpy(exts,"txt");
   Addmimetype("TEXT/PLAIN",exts,MDRIVER_INTERNAL,NULL,NULL);
}

/*-----------------------------------------------------------------------*/

BOOL Initmime(void)
{  NEWLIST(&mimes);
   inited=TRUE;
   Defaultmimes();
   return TRUE;
}

void Freemime(void)
{  struct Mime *m;
   if(inited)
   {  while(m=(struct Mime *)REMHEAD(&mimes)) Freemimetype(m);
   }
}

void Reinitmime(void)
{  struct Mime *m;
   while(m=(struct Mime *)REMHEAD(&mimes)) Freemimetype(m);
   Defaultmimes();
}

void Addmimetype(UBYTE *type,UBYTE *exts,UWORD driver,UBYTE *cmd,UBYTE *args)
{  struct Mime *m;
   struct Mimextens *me;
   UBYTE *p,*q;
   BOOL ok=TRUE;
   for(m=mimes.first;m->next;m=m->next)
   {
       if(STRIEQUAL(m->mimetype,type))
      { // REMOVE(m);
        // Freemimetype(m);
         break;
      }
   }
   if(m=ALLOCSTRUCT(Mime,1,MEMF_CLEAR))
   {  NEWLIST(&m->extensions);
      strncpy(m->mimetype,type,31);
      for(p=m->mimetype;*p;p++) *p=toupper(*p);
      if(cmd && *cmd && !(m->cmd=Dupstr(cmd,-1))) ok=FALSE;
      if(args && *args && !(m->args=Dupstr(args,-1))) ok=FALSE;
      m->driver=driver;
      p=exts;
      while(ok && *p)
      {  if(*p==' ' || *p==',') p++;
         else
         {  for(q=p;*q && *q!=' ' && *q!=',';q++);
            if(me=ALLOCSTRUCT(Mimextens,1,MEMF_CLEAR))
            {  strncpy(me->ext,p,MIN(15,q-p));
               ADDTAIL(&m->extensions,me);
            }
            else ok=FALSE;
            if(*q) q++;      /* skip q to next, or leave at eol */
            p=q;
         }
      }
      if(ok) ADDTAIL(&mimes,m);
      else Freemimetype(m);
   }
}

UBYTE *Mimetypefromext(UBYTE *name)
{  UBYTE *ext;
   if(ext=Getextension(name))
   {  struct Mime *m;
      struct Mimextens *me;
      for(m=mimes.first;m->next;m=m->next)
      {  for(me=m->extensions.first;me->next;me=me->next)
         {  if(STRIEQUAL(me->ext,ext))
            {  return m->mimetype;
            }
         }
      }
   }
   return NULL;
}

BOOL Checkmimetype(UBYTE *data,long length,UBYTE *type)
{  UBYTE *p,*end;
   BOOL ok=TRUE;
   if(STRNIEQUAL(type,"TEXT/",5))
   {
      int ilegal = 0;
      p=data;
      end=p+length;
      /* Ignore leading nullbytes */
      while(p<end && !*p) p++;
      while(p<end)
      {
          if(!((Isprint(*p) || Isspace(*p))))
          {
            ilegal++;
          }
          p++;
      }
      if(ilegal > 2) /* allow 1 or two stray unprintables */
      {
        ok = FALSE;
      }
   }
   else if(length>0 && STRNIEQUAL(type,"IMAGE/",6))
   {  static UBYTE gif87asig[]={ 'G','I','F','8','7','a' };
      static UBYTE gif89asig[]={ 'G','I','F','8','9','a' };
      static UBYTE pngsig[]={ 0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a };
      static UBYTE jpegsig[]={ 0xff,0xd8 };
      short l;
      if(STRIEQUAL(type+6,"GIF"))
      {  l=MIN(length,sizeof(gif87asig));
         ok=!memcmp(data,gif87asig,l) || !memcmp(data,gif89asig,l);
      }
      else if(STRIEQUAL(type+6,"JPEG"))
      {  l=MIN(length,sizeof(jpegsig));
         ok=!memcmp(data,jpegsig,l);
      }
      else if(STRIEQUAL(type+6,"PNG") || STRIEQUAL(type+6,"X-PNG"))
      {  l=MIN(length,sizeof(pngsig));
         ok=!memcmp(data,pngsig,l);
      }
   }
   return ok;
}

UBYTE *Mimetypefromdata(UBYTE *data,long length,UBYTE *deftype)
{  UBYTE *p,*end;
   UBYTE *type=deftype?deftype:(UBYTE *)"X-UNKNOWN/X-UNKNOWN";
   if(data && length)
   {  if(STRNIEQUAL(deftype,"TEXT/",5))
      {  p=data;
         end=data+length;
         while(p<end && !*p) p++;
         while(p<end-3 && isspace(*p)) p++;
         if(p<=end-4 && STRNIEQUAL(p,"<!--",4))
         {  type="TEXT/HTML";
         }
         else if(p<=end-5 && STRNIEQUAL(p,"<HTML",5))
         {  type="TEXT/HTML";
         }

         else if(p<=end-10 && STRNIEQUAL(p,"<!DOCTYPE",9) && isspace(p[9]))
         {  p+=10;
            while(p<end && isspace(*p)) p++;
            if(p<=end-5 && STRNIEQUAL(p,"HTML ",5))
            {  type="TEXT/HTML";
            }
            else
            if(p<=end-5 && STRNIEQUAL(p,"HTML>",5))
            {  type="TEXT/HTML";
            }
         }
         else if(p<end-5 && STRNIEQUAL(p,"<?xml",5))
         {
            type="TEXT/HTML";
         }
         else if(Checkmimetype(data,length,"TEXT/PLAIN"))
         {  type="TEXT/PLAIN";
         }
         else
         {  type="APPLICATION/OCTET-STREAM";
         }
      }
   }
   return type;
}

ULONG Getmimedriver(UBYTE *mimetype,UBYTE *url,UBYTE **name,UBYTE **args)
{  struct Mime *m,*mvw=NULL;
   struct Mime *match = NULL,*extmatch = NULL;
   struct Mime *wildmatch = NULL,*wildextmatch = NULL;
   struct Mime *defmatch = NULL, *wilddefmatch = NULL;
   UBYTE wildtype[32],noxtype[32];
   UBYTE *p;
   ULONG mime=MIMEDRV_NONE;
   UBYTE *ext = Getextension(url);      /* ext will be null if url is null */
   struct Mimextens *me;

   if(mimetype)
   {  strcpy(wildtype,mimetype);
      strcpy(noxtype,mimetype);
      if(p=strchr(mimetype,'/')) strcpy(wildtype+(p+1-mimetype),"*");
      if(p && STRNIEQUAL(p,"/X-",3))
      {  strcpy(noxtype+(p+1-mimetype),p+3);
      }
      /* search for mime type, or remember matching wild subtype */
      for(m=mimes.first;m->next;m=m->next)
      {  if((STRIEQUAL(m->mimetype,mimetype) || STRIEQUAL(m->mimetype,noxtype))
         && m->driver!=MDRIVER_NONE)
         {
            match=m;
                for(me=m->extensions.first;me->next;me=me->next)
                {
                   if(STRIEQUAL(me->ext,"*"))
                   {
                      defmatch = m;
                   }
                }

            if(ext)
            {
                for(me=m->extensions.first;me->next;me=me->next)
                {
                   if(STRIEQUAL(me->ext,ext))
                   {
                      extmatch = m;
                   }
                }

            }
            // break;
         }
         if(!match && STRIEQUAL(m->mimetype,wildtype))
         {
         wildmatch=m;
                for(me=m->extensions.first;me->next;me=me->next)
                {
                   if(STRIEQUAL(me->ext,"*"))
                   {
                      wilddefmatch = m;
                   }
                }

             if(ext)
             {
                for(me=m->extensions.first;me->next;me=me->next)
                {
                   if(STRIEQUAL(me->ext,ext))
                   {
                      wildextmatch = m;
                   }
                }

             }
         }
      }
      if(extmatch)
      {
          mvw = extmatch;
      }
      else if(defmatch)
      {
          mvw = defmatch;
      }
      else if(match)
      {
          mvw = match;
      }
      else if(wildextmatch)
      {
          mvw = wildextmatch;
      }
      else if(wilddefmatch)
      {
          mvw = wilddefmatch;
      }
      else if(wildmatch)
      {
         mvw = wildmatch;
      }

      if(mvw && mvw->driver==MDRIVER_EXTERNAL && mvw->cmd && mvw->args)
      {  mime=MIMEDRV_EXTPROG;
         *name=mvw->cmd;
         *args=mvw->args;
      }
      else if(mvw && mvw->driver==MDRIVER_EXTPIPE && mvw->cmd && mvw->args)
      {  mime=MIMEDRV_EXTPROGPIPE;
         *name=mvw->cmd;
         *args=mvw->args;
      }
      else if(mvw && mvw->driver==MDRIVER_NOFETCH && mvw->cmd && mvw->args)
      {  mime=MIMEDRV_EXTPROGNOFTCH;
         *name=mvw->cmd;
         *args=mvw->args;
      }
      else if(mvw && mvw->driver==MDRIVER_PLUGIN && mvw->cmd)
      {  mime=MIMEDRV_PLUGIN;
         *name=mvw->cmd;
         *args=mvw->args;
      }
      else if(mvw && mvw->driver==MDRIVER_INTERNAL)
      {  if(STRNIEQUAL("TEXT/",mimetype,5)) mime=MIMEDRV_DOCUMENT;
         else if(STRNIEQUAL("IMAGE/",mimetype,6)) mime=MIMEDRV_IMAGE;
         else if(STRNIEQUAL("AUDIO/",mimetype,6)) mime=MIMEDRV_SOUND;
      }
      else if(mvw && mvw->driver==MDRIVER_SAVELOCAL)
      {  mime=MIMEDRV_SAVELOCAL;
      }
      else if(mvw && mvw->driver==MDRIVER_CANCEL)
      {  mime=MIMEDRV_CANCEL;
      }

   }
   return mime;
}

BOOL Isxbm(UBYTE *mimetype)
{  return (BOOL)
      (STRIEQUAL(mimetype,"IMAGE/X-XBITMAP") || STRIEQUAL(mimetype,"IMAGE/XBITMAP"));
}
