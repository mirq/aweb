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

/* cache.c - AWeb cache object */

#include "aweb.h"
#include "cache.h"
#include "asyncio.h"
#include "url.h"
#include "window.h"
#include "libraries/awebarexx.h"
#include <libraries/locale.h>
#include <proto/utility.h>

/*------------------------------------------------------------------------*/

#include "caprivate.h"

CacheList_t cache;

static long cachenr = 0;

#define NAMESIZE  256

static UBYTE cachename[NAMESIZE];
static UBYTE *awcuname;
static long cachelock;
static BOOL initializing, exitting;
static long nradded = 0;
static long sizeadded = 0;

#define NRCHKPT   100    /* Nr of files to add before saving AWCR again */
#define CHKAFTER  102400 /* Nr of bytes to add before flushing excess */

struct SignalSemaphore cachesema;

long    cadisksize;

static void    Deletecache(struct Cache *cac);
static UBYTE * Makename(UBYTE * file, UBYTE * ext);


/// READ_RET definition
/*
 * Various read functions defined below return
 * values of type READ_RET which may take the values
 * specified.
 */
enum ReadReturnTypes
{
   RRT_OKAY,
   RRT_EOF,
   RRT_ERROR
};
typedef int READ_RET;

/// Read/Write Big-Endian
/*
 * These functions read and write 16 and 32 bit number in big-endian
 * format to an async file. No need to change either the reading
 * or writing process if changed to a little-endian architecture.
 */
static BOOL Write32bit(struct AsyncFile * fh, ULONG val)
{
   if(-1 == WriteCharAsync(fh,(val >> 24) & 0xff))
      return FALSE;
   if(-1 == WriteCharAsync(fh,(val >> 16) & 0xff))
      return FALSE;
   if(-1 == WriteCharAsync(fh,(val >> 8) & 0xff))
      return FALSE;
   if(-1 == WriteCharAsync(fh, val & 0xff))
      return FALSE;

   return TRUE;
}

static READ_RET Read32Bit(struct AsyncFile * fh, ULONG * val)
{
   LONG     r;

   r = ReadCharAsync(fh);
   if(-1 == r)
      return RRT_EOF;
   *val =(r & 0xff) << 24;

   r = ReadCharAsync(fh);
   if(-1 == r)
      return RRT_ERROR;
   *val |=(r & 0xff) << 16;

   r = ReadCharAsync(fh);
   if(-1 == r)
      return RRT_ERROR;
   *val |=(r & 0xff) << 8;

   r = ReadCharAsync(fh);
   if(-1 == r)
      return RRT_ERROR;
   *val |= r & 0xff;

   return RRT_OKAY;
}

/* 16bit are unused
static BOOL Write16bit(struct AsyncFile * fh, UWORD val)
{
        if(-1 == WriteCharAsync(fh,(val >> 8) & 0xff))
                return FALSE;
        if(-1 == WriteCharAsync(fh, val & 0xff))
                return FALSE;

        return TRUE;
}

static READ_RET Read16Bit(struct AsyncFile * fh, UWORD * val)
{
        LONG     r;

        r = ReadCharAsync(fh);
        if(-1 == r)
                return RRT_EOF;
        *val =(r & 0xff) << 8;

        r = ReadCharAsync(fh);
        if(-1 == r)
                return RRT_ERROR;
        *val |= r & 0xff;

        return RRT_OKAY;
}
*/

static BOOL Write8bit(struct AsyncFile * fh, UBYTE val)
{
   if(-1 == WriteCharAsync(fh, val & 0xff))
      return FALSE;

   return TRUE;
}

static READ_RET Read8Bit(struct AsyncFile * fh, UBYTE * val)
{
   LONG     r;

   r = ReadCharAsync(fh);
   if(-1 == r)
      return RRT_EOF;
   *val = r & 0xff;

   return RRT_OKAY;
}

/// Read/Write String
#define READBUF_LEN             32

static READ_RET Readstring(struct AsyncFile * fh, STRPTR * str)
{
   char    read_buf[READBUF_LEN+1];
   LONG    read_len;

   STRPTR  str_swap;
   ULONG           str_len = 0;


   *str = ALLOCTYPE(UBYTE, 1, 0);
   if(NULL == *str)
      return RRT_ERROR;
   **str = '\0';

   do
   {
      if(NULL == FGetsLenAsync(fh, read_buf, READBUF_LEN, &read_len))
      {
         FREE(*str);
         if(0 == IoErr())
            return RRT_EOF;
         return RRT_ERROR;
      }

      str_len += read_len + 1;
      str_swap = ALLOCTYPE(UBYTE, str_len, 0);
      if(NULL == str_swap)
      {
         FREE(*str);
         return RRT_ERROR;
      }
      strcpy(str_swap, *str);
      if('\n' == read_buf[read_len-1])
         strncat(str_swap, read_buf, read_len-1); /* without the newline */
      else
         strncat(str_swap, read_buf, read_len);
      FREE(*str);
      *str = str_swap;
   }
   while('\n' != read_buf[read_len-1]);

   return RRT_OKAY;
}

static BOOL Writestring(struct AsyncFile * fh, STRPTR str)
{
   if(-1 == WriteLineAsync(fh, str))
      return FALSE;
   if(-1 == WriteCharAsync(fh, '\n'))
      return FALSE;

   return TRUE;
}

/// Reading/Writing Header & Entry Structures
#define MAGICLABEL            "AWCR"
#define MAGICLABEL_LEN          4
#define CACHEVERSION            6

struct Creghdr   /* Cache registration header */
{
   UBYTE   label[MAGICLABEL_LEN+1]; /* 'AWCR' */
   ULONG   version;                 /* CACHEVERSION */
   ULONG   lastnr;                  /* last nr used */
};

struct Cregentry /* Cache registration entry version 3 */
{
   ULONG   nr;        /* nr of file */
   ULONG   date;      /* date stamp */
   ULONG   expires;   /* expiry date stamp */
   ULONG   cachedate; /* cache date */
   ULONG   size;      /* disk size */
   UBYTE   type;      /* cache object type */

   /* string entries newline terminated */
   STRPTR  mimetype;
   STRPTR  charset;
   STRPTR  etag;
   STRPTR  url;

   /* if type == COTYPE_MOVED */
   STRPTR  movedto;
};

#define COTYPE_MOVED       1  /* used in AWCR */
#define COTYPE_TEMPMOVED   2
#define COTYPE_DEL         3  /* used in AWCU */
#define MAX_COTYPE         3

static void FreeCRegStruct(struct Cregentry *cre)
{
   if(cre->mimetype)
      FREE(cre->mimetype);

   if(cre->charset)
      FREE(cre->charset);

   if(cre->etag)
      FREE(cre->etag);

   if(cre->url)
      FREE(cre->url);

   if(cre->movedto)
      FREE(cre->movedto);

   cre->mimetype = NULL;
   cre->charset = NULL;
   cre->etag = NULL;
   cre->url = NULL;
   cre->movedto = NULL;
}

static READ_RET ReadCRegStruct_v6(struct AsyncFile *fh, struct Cregentry *cre)
{
   READ_RET        ret;

   /* read nr of file */
   ret = Read32Bit(fh, &cre->nr);
   if(RRT_OKAY != ret)
      return ret;

   /* read date stamp */
   ret = Read32Bit(fh, &cre->date);
   if(RRT_OKAY != ret)
      return RRT_ERROR;

   /* read expiry date stamp */
   ret = Read32Bit(fh, &cre->expires);
   if(RRT_OKAY != ret)
      return RRT_ERROR;

   /* read cache date */
   ret = Read32Bit(fh, &cre->cachedate);
   if(RRT_OKAY != ret)
      return RRT_ERROR;

   /* read size */
   ret = Read32Bit(fh, &cre->size);
   if(RRT_OKAY != ret)
      return RRT_ERROR;

   /* read type */
   ret = Read8Bit(fh, &cre->type);
   if(RRT_OKAY != ret)
      return RRT_ERROR;

   /* read mimetype */
   ret = Readstring(fh, &cre->mimetype);
   if(RRT_OKAY != ret)
      return RRT_ERROR;

   ret = Readstring(fh, &cre->charset);
   if(RRT_OKAY != ret)
      return RRT_ERROR;

   ret = Readstring(fh, &cre->etag);
   if(RRT_OKAY != ret)
      return RRT_ERROR;

   /* read url */
   ret = Readstring(fh, &cre->url);
   if(RRT_OKAY != ret)
      return RRT_ERROR;

   /* read moved to url if appropriate */
   if(COTYPE_MOVED == cre->type || COTYPE_TEMPMOVED == cre->type)
   {
      ret = Readstring(fh, &cre->movedto);
      if(RRT_OKAY != ret)
         return RRT_ERROR;
   }
   else
      cre->movedto = NULL;

   return RRT_OKAY;
}
/*
static READ_RET ReadCRegStruct_v4(struct AsyncFile      *fh, struct Cregentry *cre)
{
        READ_RET        ret;

        // read nr of file
        ret = Read32Bit(fh, &cre->nr);
        if(RRT_OKAY != ret)
                return ret;

        // read date stamp
        ret = Read32Bit(fh, &cre->date);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read expiry date stamp
        ret = Read32Bit(fh, &cre->expires);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read cache date
        ret = Read32Bit(fh, &cre->cachedate);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read size
        ret = Read32Bit(fh, &cre->size);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read type
        ret = Read8Bit(fh, &cre->type);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read mimetype
        ret = Readstring(fh, &cre->mimetype);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read url
        ret = Readstring(fh, &cre->url);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read moved to url if appropriate
        if(COTYPE_MOVED == cre->type || COTYPE_TEMPMOVED == cre->type)
        {
                ret = Readstring(fh, &cre->movedto);
                if(RRT_OKAY != ret)
                        return RRT_ERROR;
        }
        else
                cre->movedto = NULL;

        return RRT_OKAY;
}

static READ_RET ReadCRegStruct_v3(struct AsyncFile      *fh, struct Cregentry *cre)
{
        READ_RET        ret;

        UWORD                   type;

        UBYTE                   mimetype[32];
        int                             i;
        UWORD                   url_size;
        STRPTR          buff, p;

        // read nr of file
        ret = Read32Bit(fh, &cre->nr);
        if(RRT_OKAY != ret)
                return ret;

        // read date stamp
        ret = Read32Bit(fh, &cre->date);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read expiry date stamp
        ret = Read32Bit(fh, &cre->expires);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read cache date
        ret = Read32Bit(fh, &cre->cachedate);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read size
        ret = Read32Bit(fh, &cre->size);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read type
        ret = Read16Bit(fh, &type);
        if(RRT_OKAY != ret)
                return RRT_ERROR;
        cre->type = type & 0xff;

        // read url size
        ret = Read16Bit(fh, &url_size);
        if(RRT_OKAY != ret)
                return RRT_ERROR;

        // read mimetype
        p = mimetype;
        i = 0;
        do
        {
                if(1 != ReadAsync(fh, p, 1))
                        return RRT_ERROR;
        }
        while(*p++ && ++ i < 32); // loop until nullbyte read

        if(i >= 32)
                return RRT_ERROR;

        cre->mimetype = ALLOCTYPE(UBYTE, i + 1, 0);
        if(NULL == cre->mimetype)
                return RRT_ERROR;
        strcpy(cre->mimetype, mimetype);

        // read urls
        buff = ALLOCTYPE(UBYTE, url_size+1, 0);
        if(NULL == buff)
                return RRT_ERROR;

        if(url_size != ReadAsync(fh, buff, url_size))
        {
                FREE(buff);
                return RRT_ERROR;
        }

        // copy url
        cre->url = ALLOCTYPE(UBYTE, strlen(buff)+1, 0);
        if(NULL == cre->url)
        {
                FREE(buff);
                return RRT_ERROR;
        }
        strcpy(cre->url, buff);

        // copy moved to type if necessary
        if(COTYPE_MOVED == cre->type || COTYPE_TEMPMOVED == cre->type)
        {
                p = buff + strlen(buff) + 1;
                cre->movedto = ALLOCTYPE(UBYTE, strlen(p)+1, 0);
                if(NULL == cre->movedto)
                {
                        FREE(buff);
                        return RRT_ERROR;
                }
                strcpy(cre->movedto, p);
        }
        else
                cre->movedto = NULL;

        FREE(buff);

        return RRT_OKAY;
}
*/
static BOOL WriteCRegStruct(struct AsyncFile * fh, struct Cregentry * cre)
{
   if(FALSE == Write32bit(fh, cre->nr))
      return FALSE;

   if(FALSE == Write32bit(fh, cre->date))
      return FALSE;

   if(FALSE == Write32bit(fh, cre->expires))
      return FALSE;

   if(FALSE == Write32bit(fh, cre->cachedate))
      return FALSE;

   if(FALSE == Write32bit(fh, cre->size))
      return FALSE;

   if(FALSE == Write8bit(fh, cre->type))
      return FALSE;

   if(FALSE == Writestring(fh, cre->mimetype))
      return FALSE;

   if(FALSE == Writestring(fh, cre->charset))
      return FALSE;

   if(FALSE == Writestring(fh, cre->etag))
      return FALSE;


   if(FALSE == Writestring(fh, cre->url))
      return FALSE;

   if(COTYPE_MOVED == cre->type || COTYPE_TEMPMOVED == cre->type)
   {
      if(FALSE == Writestring(fh, cre->movedto))
         return FALSE;
   }

   return TRUE;
}

static READ_RET ReadCHdrStruct(struct AsyncFile * fh, struct Creghdr * crh)
{
   LONG readlen; /* amount of data actually read by ReadAsync() */

   /* read magic label */
   readlen = ReadAsync(fh, &crh->label, MAGICLABEL_LEN);
   if(MAGICLABEL_LEN != readlen)
      return RRT_ERROR;
   crh->label[MAGICLABEL_LEN] = '\0';

   /* read version number */
   if(RRT_OKAY != Read32Bit(fh, &crh->version))
      return RRT_ERROR;

   /* read in last number entry */
   if(RRT_OKAY != Read32Bit(fh, &crh->lastnr))
      return RRT_ERROR;

   return RRT_OKAY;
}

static BOOL WriteCHdrStruct(struct AsyncFile * fh, struct Creghdr * crh)
{
   LONG writelen; /* amount of data actually written by WriteAsync() */

   /* write magic label */
   writelen = WriteAsync(fh, &crh->label, MAGICLABEL_LEN);
   if(MAGICLABEL_LEN != writelen)
      return FALSE;

   /* write version number */
   if(FALSE == Write32bit(fh, crh->version))
      return FALSE;

   /* write last number entry */
   if(FALSE == Write32bit(fh, crh->lastnr))
      return FALSE;

   return TRUE;
}

/// Read/Write Cache Register Entries
static BOOL Readcachereg(UBYTE * name, long lock)
{
   struct AsyncFile   *fh;

   struct Creghdr     crh = {{0}};
   struct Cregentry   cre = {0};

   READ_RET(*ReadCRegStruct)(struct AsyncFile *fh, struct Cregentry *cre) = ReadCRegStruct_v6;

   UBYTE   name_buf[20];
   STRPTR  name_ext;
   STRPTR  filename;

   void   *url;

   struct Cache *cac;


#ifndef LOCALONLY
   fh = OpenAsync(name, MODE_READ, FILEBLOCKSIZE);
   if(NULL == fh)
      return FALSE;

   if(RRT_OKAY != ReadCHdrStruct(fh, &crh))
   {
      CloseAsync(fh);
      return FALSE;
   }

   /* check for header validity */
   if(0 != strncmp(crh.label, MAGICLABEL, MAGICLABEL_LEN))
   {
      CloseAsync(fh);
      return FALSE;
   }

   if(CACHEVERSION != crh.version)
   {
      /* if previous version is supported, change which readCRegStruct function we're using */
      /* if(3 == crh.version)
             ReadCRegStruct = ReadCRegStruct_v3;
         else
         {*/
         CloseAsync(fh);
         return FALSE;
      /* }*/
   }

   cachenr = crh.lastnr;

   for(;;)
   {
      READ_RET        ret;

      ret = ReadCRegStruct(fh, &cre);
      if(RRT_OKAY != ret)
      {
         CloseAsync(fh);
         FreeCRegStruct(&cre);
         if(RRT_ERROR == ret)
            return FALSE;
         return TRUE;
      }

      /* check validity of entry type */
      if(cre.type > MAX_COTYPE) //was: if(cre.type < 0 || cre.type > MAX_COTYPE)
      {
         CloseAsync(fh);
         FreeCRegStruct(&cre);
         return FALSE;
      }

      if(cre.type == COTYPE_DEL)
      {
         url = Findurl("", cre.url, 0);
         if(NULL == url)
         {
            CloseAsync(fh);
            FreeCRegStruct(&cre);
            return FALSE;
         }

         cac =(struct Cache *) Agetattr(url, AOURL_Cache);
         if(cac && cac->nr == cre.nr)
            Auspecial(url, AUMST_DELETECACHE);
      }
      else
      {
         url = Anewobject(AOTP_URL, AOURL_Url,(Tag)cre.url, TAG_END);
         if(NULL == url)
         {
            CloseAsync(fh);
            FreeCRegStruct(&cre);
            return FALSE;
         }

         name_ext = Urlfileext(cre.url);
         if(!name_ext && Isxbm(cre.mimetype))
            name_ext = Dupstr("xbm", 3);
         sprintf(name_buf, "AWCD%02lX/%08lX", cre.nr & 0x3f, cre.nr);
         filename = Makename(name_buf, name_ext);
         if(name_ext)
            FREE(name_ext);
         if(cre.expires && cre.expires <= Today())
         {
            if(filename)
            {
               DeleteFile(filename);
               FREE(filename);
            }
         }
         else
         {
            cac = Anewobject(AOTP_CACHE, AOCAC_Url,(Tag)url,
                              AOCAC_Number, cre.nr,
                              AOCAC_Cachedate, cre.cachedate,
                              TAG_END);
            if(NULL == cac)
            {
               CloseAsync(fh);
               FreeCRegStruct(&cre);
               return FALSE;
            }

            cac->name = filename;
            cac->date = cre.date;
            cac->expires = cre.expires;
            strcpy(cac->mimetype, cre.mimetype);
            strcpy(cac->charset, cre.charset);
            strcpy(cac->etag, cre.etag);
            cac->disksize = cre.size;
            cadisksize += cac->disksize;
            Asetattrs(url, AOURL_Cache,(Tag)cac, AOURL_Visited, TRUE, TAG_END);

            if(cre.type == COTYPE_MOVED)
               Asetattrs(url, AOURL_Movedto,(Tag)cre.movedto, TAG_END);
            else if(cre.type == COTYPE_TEMPMOVED)
               Asetattrs(url, AOURL_Tempmovedto,(Tag)cre.movedto, TAG_END);

            if(cre.nr > cachenr)
               cachenr = cre.nr;
         }
      }

      FreeCRegStruct(&cre);
   }
#endif /* LOCALONLY */

   return TRUE;
}

/* Write an entry to the cache registration file */
static void Writeregentry(void *fh, struct Cache *cac, BOOL del)
{
#ifndef LOCALONLY
   struct Cregentry cre = {0};
   UBYTE  *movedto = NULL;

   cre.nr = cac->nr;
   cre.date = cac->date;
   cre.expires = cac->expires;
   cre.cachedate = cac->cachedate;
   cre.size = cac->disksize;
   cre.type = 0;

   if(del)
      cre.type = COTYPE_DEL;
   else if(movedto =(UBYTE *) Agetattr(cac->url, AOURL_Movedto))
   {
      cre.type = COTYPE_MOVED;
   }
   else if(movedto =(UBYTE *) Agetattr(cac->url, AOURL_Tempmovedto))
   {
      cre.type = COTYPE_TEMPMOVED;
   }

   cre.mimetype = cac->mimetype;
   cre.charset = cac->charset;
   cre.etag = cac->etag;
   cre.url =(UBYTE *) Agetattr(cac->url, AOURL_Realurl);
   cre.movedto = movedto;

   WriteCRegStruct(fh, &cre);

#endif
}

/* Create a full name from filename and optional extension. Dynamic string. */
static UBYTE * Makename(UBYTE * file, UBYTE * ext)
{
   long    len = strlen(cachename) + strlen(file) + 10;
   UBYTE  *name;

   if(ext)
      len += strlen(ext) + 1;
   if(name = ALLOCTYPE(UBYTE, len, 0))
   {
      strcpy(name, cachename);
      AddPart(name, file, len);
      if(ext && *ext)
      {
         strcat(name, ".");
         strcat(name, ext);
      }
   }
   return name;
}

static void Createdirectories(void)
{
#ifndef LOCALONLY
   short   i;
   UBYTE   name[NAMESIZE + 16];
   UBYTE  *p;
   long    lock;

   strcpy(name, cachename);
   AddPart(name, "AWCD", NAMESIZE + 14);
   p = name + strlen(name);
   for(i = 0; i <= 0x3f; i++)
   {
      sprintf(p, "%02X", i);
      if(lock = CreateDir(name))
         UnLock(lock);
   }
#endif
}

/* Create an empty cache registration */
static void Createlog(UBYTE * name)
{
#ifndef LOCALONLY
   void   *fh;
   struct Creghdr crh = {{0}};

   if(fh = OpenAsync(name, MODE_WRITE, FILEBLOCKSIZE))
   {
      strncpy(crh.label, "AWCR", 4);
      crh.version = CACHEVERSION;
      crh.lastnr = cachenr;
      WriteCHdrStruct(fh, &crh);
      CloseAsync(fh);
   }
#endif
}

static void Savecachereg(BOOL nodelete)
{
#ifndef LOCALONLY
   UBYTE  *name;
   void   *fh;
   struct Creghdr crh = {{0}};
   struct Cache *cac, *ucac;
   BOOL   ok = FALSE;

   if(name = Makename("AWCR", NULL))
{
      if(fh = OpenAsync(name, MODE_WRITE, FILEBLOCKSIZE))
      {
         strncpy(crh.label, "AWCR", 4);
         crh.version = CACHEVERSION;
         crh.lastnr = cachenr;
         WriteCHdrStruct(fh, &crh);
         for(cac = cache.first; cac->object.next; cac = cac->object.next)
         {
            ucac =(struct Cache *) Agetattr(cac->url, AOURL_Cache);
            if(cac == ucac)
            {
               Writeregentry(fh, cac, FALSE);
               if(nodelete)
                  cac->flags |= CACF_NODELETE;
            }
         }
         ok = TRUE;
         if(CloseAsync(fh) >= 0)
         {
            DeleteFile(awcuname);
         }
         else
         {
            DeleteFile(name);
         }
      }
      FREE(name);
   }
   if(nodelete && !ok)
   {
      for(cac = cache.first; cac->object.next; cac = cac->object.next)
      {
         ucac =(struct Cache *) Agetattr(cac->url, AOURL_Cache);
         if(cac == ucac)
         {
            cac->flags |= CACF_NODELETE;
         }
      }
   }
#endif
}

/* Add an entry to the registration */
static void Addregentry(struct Cache *cac, BOOL del)
{
#ifndef LOCALONLY
   void   *fh;

   if(!exitting)
   {
      if(fh = OpenAsync(awcuname, MODE_APPEND, FILEBLOCKSIZE))
      {
         Writeregentry(fh, cac, del);
         CloseAsync(fh);
      }
      if(++nradded > NRCHKPT)
      {
         UBYTE  *awcrname = Makename("AWCR", NULL);

         if(awcrname)
         {
            Savecachereg(FALSE);
            Rename(awcrname, awcuname);
            FREE(awcrname);
            nradded = 0;
         }
      }
   }
#endif
}

/*------------------------------------------------------------------------*/

/* Create file and write entry in registration. */
static void Opencacfile(struct Cache *cac)
{
   UBYTE  *urlname =(UBYTE *) Agetattr(cac->url, AOURL_Url);
   UBYTE  *ext = Urlfileext(urlname);
   UBYTE   buf[20];

   sprintf(buf, "AWCD%02lX/%08lX", cac->nr & 0x3f, cac->nr);
   cac->name = Makename(buf, ext);
   if(ext)
      FREE(ext);
   cac->fh = OpenAsync(cac->name, MODE_WRITE, FILEBLOCKSIZE);
}

/* Delete the associated file, and write entry in registration. */
static void Deletecache(struct Cache *cac)
{
   DeleteFile(cac->name);
   if(!initializing)
   {  /* Don't add a DEL entry when we are reading in the registration */
      Addregentry(cac, TRUE);
   }
}

/* Set the file's comment */
static void Setcomment(struct Cache *cac)
{
   UBYTE  *urlname, *comment;
   long    len;

   if(urlname =(UBYTE *) Agetattr(cac->url, AOURL_Url))
   {
      len = strlen(urlname);
      if(comment = Dupstr(urlname, MIN(80, len)))
      {
         SetComment(cac->name, comment);
         FREE(comment);
      }
   }
}

/*------------------------------------------------------------------------*/

/* Delete all files beyond limit, last touched first */
static void Flushexcess(void)
{
   struct Cache *cac, *next;
   long    max = prefs.network.cadisksize * 1024;

   ObtainSemaphore(&cachesema);
   for(cac = cache.first; cac->object.next && cadisksize > max; cac = next)
   {
      next = cac->object.next;
      Auspecial(cac->url, AUMST_DELETECACHE);
   }
   ReleaseSemaphore(&cachesema);
   sizeadded = 0;
   }

/* Delete all files or all files for this type and/or pattern */
static void Flushtype(UWORD type, UBYTE * pattern)
{
   struct Cache *cac, *next;
   long    len, max = 0, count = 0;

   UBYTE  *parsepat = NULL,
          *url,
          *p,
          *buf,
          *msg = AWEBSTR(MSG_FIXCACHE_DELETEPROGRESS);
   long    l = strlen(msg)+sizeof(long);

   BOOL    scheme = FALSE, sel = FALSE;
   short   maxok;
   void   *preq = NULL;

   if(pattern)
   {
      len = 2 * strlen(pattern) + 4;
      parsepat = ALLOCTYPE(UBYTE, len, 0);
      if(parsepat)
      {
         if(ParsePatternNoCase(pattern, parsepat, len) < 0)
         {
            FREE(parsepat);
            parsepat = NULL;
         }
      }
      scheme = BOOLVAL(strstr(pattern, "://"));
   }
   Busypointer(TRUE);
   ObtainSemaphore(&cachesema);
   if(buf=ALLOCTYPE(UBYTE,l,0))
   {
      for(maxok = 0; maxok <= 1; maxok++)
      {
         if(maxok && max)
         {
            sprintf(buf,msg,max);
            preq = Openprogressreq(buf);
         }
         for(cac = cache.first; cac->object.next; cac = next)
         {
            next = cac->object.next;
            /*
            * If documents, delete "text/..", if images delete not "text/.."
            */
            if(type)
            {
               sel =(type == CACFT_DOCUMENTS) == BOOLVAL(STRNIEQUAL(cac->mimetype, "TEXT/", 5));
            }
            else
               sel = TRUE;
            if(sel && parsepat)
            {
               url =(UBYTE *) Agetattr(cac->url, AOURL_Url);
               if(url)
               {
                  if(!scheme)
                  {
                     p = strstr(url, "://");
                     if(p)
                        p += 3;
                     else
                        p = url;
                  }
                  else
                     p = url;
                  sel = MatchPatternNoCase(parsepat, p);
               }
            }
            if(sel)
            {
               if(!maxok)
               {
                  max++;
               }
               else
               {
                  if(preq)
                  {
                     count++;
                     Setprogressreq(preq, count, max);
                     if(Checkprogressreq(preq))
                     {
                        goto err;
                     }
                  }
                  Auspecial(cac->url, AUMST_DELETECACHE);
               }
            }
         }
      }
err:
      if(preq) Closeprogressreq(preq);
      FREE(buf);
   }
   ReleaseSemaphore(&cachesema);
   Busypointer(FALSE);
   if(parsepat)
      FREE(parsepat);
}

/* Send constructed HTTP headers */
static void Sendinfo(struct Cache *cac, void *fetch)
{
   UBYTE   buf[64];
   UBYTE   datebuf[32];

   Asrcupdatetags(cac->url, fetch, AOURL_Fromcache, TRUE, TAG_END);
   if(*cac->mimetype)
   {
      if(*cac->charset)
      {
#ifdef CHARSET_DEBUG
         printf("cache.c/Sendinfo(): sending charset information: %s for url %s\n",cac->charset, cac->url->url);
#endif
         sprintf(buf, "Content-Type: %s; charset=%s", cac->mimetype, cac->charset);
      }
      else
         sprintf(buf, "Content-Type: %s", cac->mimetype);
      Asrcupdatetags(cac->url, fetch, AOURL_Header,(Tag)buf, TAG_END);
   }
   sprintf(buf, "Content-Length: %ld", cac->disksize);
   Asrcupdatetags(cac->url, fetch, AOURL_Header,(Tag)buf, TAG_END);
   if(cac->date)
   {
      Makedate(cac->date, datebuf);
      sprintf(buf, "Last-Modified: %s", datebuf);
      Asrcupdatetags(cac->url, fetch, AOURL_Header,(Tag)buf, TAG_END);
   }
   if(cac->expires)
   {
      Makedate(cac->expires, datebuf);
      sprintf(buf, "Expires: %s", datebuf);
      Asrcupdatetags(cac->url, fetch, AOURL_Header,(Tag)buf, TAG_END);
   }
}

/*------------------------------------------------------------------------*/

#ifndef LOCALONLY
struct Cafix
{
   NODE(Cafix);
   struct Cache *cac;
   UBYTE   name[40]; /* including 'AWCD##/' */
   short   directory;
};

struct Cafixdel
{
   NODE(Cafixdel);
   UBYTE   name[40];
};

static struct Cafix * Newcafix(struct Cache *cac)
{
   struct Cafix *cf = ALLOCSTRUCT(Cafix, 1, MEMF_CLEAR);

   if(cf)
   {
      cf->cac = cac;
      if(cac->name)
      {
         strncpy(cf->name, Cfnameshort(cac->name), sizeof(cf->name));
         cf->directory = cac->nr & 0x3f;
      }
   }
   return cf;
}

static struct Cafixdel * Newcafixdel(UBYTE * name)
{
   struct Cafixdel *cd = ALLOCSTRUCT(Cafixdel, 1, MEMF_CLEAR);

   if(cd)
   {
      strncpy(cd->name, name, 39);
   }
   return cd;
}

static BOOL Makecafixlist(void * alist)
{
   LIST(Cafix) * list=alist;
   struct Cafix *cf;
   struct Cache *cac;

   for(cac = cache.first; cac->object.next; cac = cac->object.next)
   {
      if(cac->name)
      {
         if(cf = Newcafix(cac))
            ADDTAIL(list, cf);
         else
            return FALSE;
      }
   }
   return TRUE;
}

static void Deletedir(UBYTE * name)
{
   __aligned struct FileInfoBlock fib = { 0 };
   long    lock, oldcd;

   LIST(Cafixdel) dellist;
   struct Cafixdel *cd;

   NEWLIST(&dellist);
   if(lock = Lock(name, SHARED_LOCK))
   {
      oldcd = CurrentDir(lock);
      if(Examine(lock, &fib))
      {
         while(ExNext(lock, &fib))
         {
            if(fib.fib_DirEntryType > 0)
            {
               Deletedir(fib.fib_FileName);
            }
            if(cd = Newcafixdel(fib.fib_FileName))
               ADDTAIL(&dellist, cd);
         }
      }
      while(cd =(struct Cafixdel *)REMHEAD(&dellist))
      {
         DeleteFile(cd->name);
         FREE(cd);
      }
      CurrentDir(oldcd);
      UnLock(lock);
   }
}

static BOOL Fixroot(void *preq)
{
   __aligned struct FileInfoBlock fib = {0};
   long    oldcd = CurrentDir(cachelock);

   LIST(Cafixdel) dellist;
   struct Cafixdel *cd;
   UBYTE  *p;
   BOOL    ok = FALSE;

   NEWLIST(&dellist);
   if(Examine(cachelock, &fib) && fib.fib_DirEntryType > 0)
   {
      while(ExNext(cachelock, &fib))
      {
         if(fib.fib_DirEntryType < 0) /* plain file */
         {
            if(!STREQUAL(fib.fib_FileName, "AWCU") && !STREQUAL(fib.fib_FileName, "AWCK"))
            {
               if(cd = Newcafixdel(fib.fib_FileName))
                  ADDTAIL(&dellist, cd);
            }
         }
         else if(fib.fib_DirEntryType > 0) /* directory */
         {
            p = fib.fib_FileName;
            if(!(STRNEQUAL(p, "AWCD", 4) && p[4] >= '0' && p[4] <= '3' && isxdigit(p[5]) && p[6] == '\0'))
            {
               Deletedir(fib.fib_FileName);
               if(cd = Newcafixdel(fib.fib_FileName))
                  ADDTAIL(&dellist, cd);
            }
         }
         if(Checkprogressreq(preq))
            goto err;
      }
   }
   while(cd =(struct Cafixdel *)REMHEAD(&dellist))
   {
      DeleteFile(cd->name);
      FREE(cd);
   }
   ok = TRUE;
err:
   while(cd =(struct Cafixdel *)REMHEAD(&dellist))
      FREE(cd);
   CurrentDir(oldcd);
   return ok;
}

static BOOL Fixcachedir(void *preq, void  *alist, short dirnum)
{
   LIST(Cafix) * list=alist;
   struct Cafix *cf;
   __aligned struct FileInfoBlock fib = { 0 };
   UBYTE   name[8];
   long    oldcd = CurrentDir(cachelock);
   long    dirlock;

   LIST(Cafixdel) dellist;
   struct Cafixdel *cd;
   BOOL    ok = FALSE, fileok;

   NEWLIST(&dellist);
   sprintf(name, "AWCD%02X", dirnum);
   if(dirlock = Lock(name, SHARED_LOCK))
   {
      CurrentDir(dirlock);
      if(Examine(dirlock, &fib))
      {
         while(ExNext(dirlock, &fib))
         {
            if(fib.fib_DirEntryType < 0) /* plain file */
            {
               fileok = FALSE;
               for(cf = list->first; cf->next; cf = cf->next)
               {
                  if(cf->directory == dirnum && STREQUAL(fib.fib_FileName, cf->name + 7)) /* skip 'AWCDxx/' */
                  {
                     REMOVE(cf);
                     FREE(cf);
                     fileok = TRUE;
                     break;
                  }
               }
               if(!fileok)
               {
                  if(cd = Newcafixdel(fib.fib_FileName))
                     ADDTAIL(&dellist, cd);
               }
            }
            else if(fib.fib_DirEntryType > 0) /* directory */
            {
               Deletedir(fib.fib_FileName);
               if(cd = Newcafixdel(fib.fib_FileName))
                  ADDTAIL(&dellist, cd);
            }
            if(Checkprogressreq(preq))
               goto err;
         }
      }
      while(cd =(struct Cafixdel *)REMHEAD(&dellist))
      {
         DeleteFile(cd->name);
         FREE(cd);
      }
      ok = TRUE;
err:
      while(cd =(struct Cafixdel *)REMHEAD(&dellist))
         FREE(cd);
      UnLock(dirlock);
   }
   else
   {
      if(dirlock = CreateDir(name))
         UnLock(dirlock);
      ok = TRUE;
   }
   CurrentDir(oldcd);
   return ok;
}

static void Fixcachereg(void * alist)
{
   LIST(Cafix) *list=alist;
   struct Cafix *cf;

   while(cf =(struct Cafix *)REMHEAD(list))
   {
      Auspecial(cf->cac->url, AUMST_DELETECACHE);
      FREE(cf);
   }
}

static void Dofixcache(short code, void *data)
{
   void   *preq;
   struct Cafix *cf;
   short   i;
   UBYTE  *awcrname = Makename("AWCR", NULL);

   if(code && awcrname)
   {
      LIST(Cafix) list;
      NEWLIST(&list);
      Busypointer(TRUE);
      ObtainSemaphore(&cachesema);
      if(preq = Openprogressreq(AWEBSTR(MSG_FIXCACHE_PROGRESS)))
      {
         if(!Makecafixlist(&list))
            goto err;
         if(Checkprogressreq(preq))
            goto err;
         Setprogressreq(preq, 1, 67);
         if(!Fixroot(preq))
            goto err;
         Setprogressreq(preq, 2, 67);
         for(i = 0; i < 64; i++)
         {
            if(!Fixcachedir(preq, &list, i))
               goto err;
            Setprogressreq(preq, i + 3, 67);
         }
         Fixcachereg(&list);
         Savecachereg(FALSE);
         Rename(awcrname, awcuname);
         Setprogressreq(preq, 67, 67);
err:
         while(cf =(struct Cafix *)REMHEAD(&list))
            FREE(cf);
         Closeprogressreq(preq);
      }
      ReleaseSemaphore(&cachesema);
      Busypointer(FALSE);
   }
   if(data)
      FREE(data);
   if(awcrname)
      FREE(awcrname);
}
#endif /* !LOCALONLY */

/*------------------------------------------------------------------------*/

static long Setcache(struct Cache *cac, struct Amset *ams)
{
   struct TagItem *tag, *tstate = ams->tags;

   while(tag = NextTagItem(&tstate))
   {
      switch(tag->ti_Tag)
      {
         case AOCAC_Url:
            if(!cac->url)
               cac->url =(void *) tag->ti_Data;
            break;
         case AOCAC_Name:
            if(!cac->name)
               cac->name =(UBYTE *) tag->ti_Data;
            break;
         case AOCAC_Number:
            if(!cac->nr)
               cac->nr = tag->ti_Data;
            break;
         case AOCAC_Cachedate:
            if(!cac->cachedate)
               cac->cachedate = tag->ti_Data;
            break;
         case AOCAC_Touched:
            if(tag->ti_Data)
            {
               ObtainSemaphore(&cachesema);
               REMOVE(cac);
               ADDTAIL(&cache, cac);
               ReleaseSemaphore(&cachesema);
            }
            break;
         case AOCAC_Sendinfo:
            Sendinfo(cac,(void *) tag->ti_Data);
            break;
      }
   }
   return 0;
}

static struct Cache * Newcache(struct Amset *ams)
{
   struct Cache *cac;
   if(cac = Allocobject(AOTP_CACHE, sizeof(struct Cache), ams))
   {
      Setcache(cac, ams);
      if(!cac->nr)
         cac->nr = ++cachenr;
      if(!cac->cachedate)
         cac->cachedate = Today();
      ObtainSemaphore(&cachesema);
      ADDTAIL(&cache, cac);
      ReleaseSemaphore(&cachesema);
   }
   return cac;
}

static long Getcache(struct Cache *cac, struct Amset *ams)
{
   struct TagItem *tag,
            *tstate = ams->tags;

   while(tag = NextTagItem(&tstate))
   {
      switch(tag->ti_Tag)
      {
         case AOCAC_Name:
            PUTATTR(tag, cac->name);
            break;
         case AOCAC_Url:
            PUTATTR(tag, cac->url);
            break;
         case AOCAC_Number:
            PUTATTR(tag, cac->nr);
            break;
         case AOCAC_Cachedate:
            PUTATTR(tag, cac->cachedate);
            break;
         case AOCAC_Lastmodified:
            PUTATTR(tag, cac->date);
            break;
         case AOCAC_Contenttype:
            PUTATTR(tag, *cac->mimetype ? cac->mimetype : NULL);
            break;
         case AOCAC_Expired:
            PUTATTR(tag, cac->expires && cac->expires <= Today());
            break;
         case AOCAC_Expires:
            PUTATTR(tag, cac->expires);
            break;
         case AOCAC_Charset:
            PUTATTR(tag, cac->charset);
            break;
         case AOCAC_Etag:
            PUTATTR(tag, cac->etag);
            break;

      }
   }
   return 0;
}

static long Srcupdatecache(struct Cache *cac, struct Amsrcupdate *ams)
{
   struct TagItem *tag,
            *tstate = ams->tags;
   UBYTE  *data = NULL;
   long    length = 0;
   BOOL    eof = FALSE;

   while(tag = NextTagItem(&tstate))
   {
      switch(tag->ti_Tag)
      {
         case AOURL_Contenttype:
            strncpy(cac->mimetype,(UBYTE *) tag->ti_Data, 31);
            break;
         case AOURL_Data:
            data =(UBYTE *) tag->ti_Data;
            break;
         case AOURL_Datalength:
            length = tag->ti_Data;
            break;
         case AOURL_Eof:
            if(tag->ti_Data)
               eof = TRUE;
            break;
         case AOURL_Lastmodified:
            cac->date = tag->ti_Data;
            break;
         case AOURL_Expires:
            cac->expires = tag->ti_Data;
            break;
         case AOURL_Terminate:
            eof = TRUE;
            break;
         case AOURL_Charset:
            strncpy(cac->charset,(UBYTE *) tag->ti_Data, 31);
            break;
         case AOURL_Etag:
            strncpy(cac->etag,(UBYTE *) tag->ti_Data, 63);
            break;

      }
   }
   if(data || eof)
   {
      if(!cac->name)
         Opencacfile(cac);
   }
   if(data)
   {
      if(length && cac->fh)
      {
         WriteAsync(cac->fh, data, length);
         cac->disksize += length;
         cadisksize += length;
         sizeadded += length;
         if(sizeadded > CHKAFTER)
            Flushexcess();
      }
   }
   if(eof)
   {
      if(cac->fh)
      {
         CloseAsync(cac->fh);
         Setcomment(cac);
         cac->fh = NULL;
         Addregentry(cac, FALSE);
         Addcabrobject(cac);
         Flushexcess();
      }
   }
   return 0;
}

static void Disposecache(struct Cache *cac)
{
   ObtainSemaphore(&cachesema);
   REMOVE(cac);
   ReleaseSemaphore(&cachesema);
   cadisksize -= cac->disksize;
   Remcabrobject(cac);
   if(cac->fh)
      CloseAsync(cac->fh);
   if(cac->name)
   {
      if(!(cac->flags & CACF_NODELETE))
         Deletecache(cac);
      FREE(cac->name);
   }
   Amethodas(AOTP_OBJECT, cac, AOM_DISPOSE);
}

static void Deinstallcache(void)
{
   while(cache.first->object.next)
      Adisposeobject((struct Aobject *)cache.first);
   if(awcuname)
      FREE(awcuname);
   if(cachelock)
      UnLock(cachelock);
}

USRFUNC_H2
(
   static long  , Cache_Dispatcher,
   struct Cache *,cac,A0,
   struct Amessage *,amsg,A1
)
{
   USRFUNC_INIT

   long    result = 0;

   switch(amsg->method)
   {
      case AOM_NEW:
         result =(long) Newcache((struct Amset *) amsg);
         break;
      case AOM_SET:
         result = Setcache(cac,(struct Amset *) amsg);
         break;
      case AOM_GET:
         result = Getcache(cac,(struct Amset *) amsg);
         break;
      case AOM_SRCUPDATE:
         result = Srcupdatecache(cac,(struct Amsrcupdate *) amsg);
         break;
      case AOM_DISPOSE:
         Disposecache(cac);
         break;
      case AOM_DEINSTALL:
         Deinstallcache();
         break;
   }
   return result;

   USRFUNC_EXIT
}

/*------------------------------------------------------------------------*/

BOOL Installcache(void)
{
   NEWLIST(&cache);
   InitSemaphore(&cachesema);
   if(!Amethod(NULL, AOM_INSTALL, AOTP_CACHE,(Tag)Cache_Dispatcher))
      return FALSE;
   return TRUE;
}

BOOL Initcache(void)
{
   UBYTE *awcr;
   long   lock;
   BOOL   corrupt = FALSE;

   cachelock = Lock(prefs.network.cachepath, SHARED_LOCK);
   if(!cachelock)
      cachelock = Lock("T:", SHARED_LOCK);
   if(cachelock)
      NameFromLock(cachelock, cachename, NAMESIZE);
   if(!(awcuname = Makename("AWCU", NULL)))
      return FALSE;
   if(!(awcr = Makename("AWCR", NULL)))
      return FALSE;
#ifndef LOCALONLY
   initializing = TRUE;
   if(lock = Lock(awcuname, SHARED_LOCK))
   {  /* old cache log exists - rebuild cache */
      corrupt = !Readcachereg(awcuname, lock);
      UnLock(lock);
      Savecachereg(FALSE); /* create new reg file and delete AWCU */
      Rename(awcr, awcuname);
   }
   else
   {
      if(lock = Lock(awcr, SHARED_LOCK))
      {
         corrupt = !Readcachereg(awcr, lock);
         UnLock(lock);
         Rename(awcr, awcuname);
      }
      else
      {
         Createdirectories();
         Createlog(awcuname);
      }
   }
   initializing = FALSE;
   if(corrupt)
   {
      if(Syncrequest(AWEBSTR(MSG_REQUEST_TITLE), AWEBSTR(MSG_FIXCACHE_CORRUPT), AWEBSTR(MSG_FIXCACHE_BUTTONS), 0))
      {
         Dofixcache(1, NULL);
      }
   }
#endif
   FREE(awcr);
   return TRUE;
}

void Exitcache(void)
{
   if(cache.first)
   {
      Savecachereg(TRUE);
   }
   /*
    * Prevent ongoing fetches from creating a bogus AWCU
   */
   exitting = TRUE;
}

UBYTE  * Cachename(void)
{
   return cachename;
}

void Flushcache(UWORD type)
{
   Flushcachepattern(type, NULL);
}

void Flushcachepattern(UWORD type, UBYTE * pattern)
{
   switch(type)
   {
      case CACFT_DOCUMENTS:
      case CACFT_IMAGES:
         Flushtype(type, pattern);
         break;
      case CACFT_EXCESS:
         Flushexcess();
         break;
      case CACFT_ALL:
         Flushtype(0, pattern);
         break;
   }
}

/* Find cachedir/name part of file name */
UBYTE  * Cfnameshort(UBYTE * name)
{
   short   i = 0;
   UBYTE  *p;

   for(p = name + strlen(name) - 1; p > name; p--)
   {
      if(*p == '/' && ++i > 1)
         break;
   }
   if(*p == '/')
      p++;
   return p;
}

void Fixcache(BOOL force)
{
#ifndef LOCALONLY
   UBYTE  *p = haiku ? HAIKU20 : AWEBSTR(MSG_FIXCACHE_WARNING);
   UBYTE  *buf;
   long    len = strlen(p) + strlen(cachename) + 8;

   if(force)
   {
      Dofixcache(TRUE, NULL);
   }
   else
   {
      if(buf = ALLOCTYPE(UBYTE, len, 0))
      {
         sprintf(buf, p, cachename);
         if(!Asyncrequest(AWEBSTR(MSG_REQUEST_TITLE), buf, AWEBSTR(MSG_FIXCACHE_BUTTONS), Dofixcache, NULL))
         {
            FREE(buf);
         }
      }
   }
#endif
}

void Getcachecontents(struct Arexxcmd *ac, UBYTE * stem, UBYTE * pattern)
{
#ifndef LOCALONLY
   UBYTE   buf[32];
   UBYTE  *parsepat = NULL, *url, *p;
   BOOL    scheme=FALSE, sel=FALSE;
   struct  Cache *cac;
   long    i, len;

   if(pattern)
   {
      len = 2 * strlen(pattern) + 4;
      parsepat = ALLOCTYPE(UBYTE, len, 0);
      if(parsepat)
      {
         if(ParsePatternNoCase(pattern, parsepat, len) < 0)
         {
            FREE(parsepat);
            parsepat = NULL;
         }
      }
      scheme = BOOLVAL(strstr(pattern, "://"));
   }
   ObtainSemaphore(&cachesema);
   i = 0;
   for(cac = cache.first; cac->object.next; cac = cac->object.next)
   {
      url =(UBYTE *) Agetattr(cac->url, AOURL_Url);
      if(parsepat)
      {
         if(url)
         {
            if(!scheme)
            {
               p = strstr(url, "://");
               if(p)
                  p += 3;
               else
                  p = url;
            }
            else
               p = url;
            sel = MatchPatternNoCase(parsepat, p);
         }
      }
      if(sel)
      {
         i++;
         Setstemvar(ac, stem, i, "URL", url);
         Setstemvar(ac, stem, i, "TYPE", cac->mimetype);
         Setstemvar(ac, stem, i, "CHARSET", cac->charset);
         sprintf(buf, "%ld", cac->disksize);
         Setstemvar(ac, stem, i, "SIZE", buf);
         sprintf(buf, "%ld", cac->cachedate / 86400);
         Setstemvar(ac, stem, i, "DATE", buf);
         Setstemvar(ac, stem, i, "ETAG", cac->etag);
         Setstemvar(ac, stem, i, "FILE", Cfnameshort(cac->name));
      }
   }
   ReleaseSemaphore(&cachesema);
   sprintf(buf, "%ld", i);
   Setstemvar(ac, stem, 0, NULL, buf);
   if(parsepat)
      FREE(parsepat);
#endif
}
