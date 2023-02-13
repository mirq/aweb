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

/* jdate.c - AWeb js internal Date object */

#include <libraries/locale.h>
#include <dos/dos.h>
#include <time.h>
#include <math.h>

#include "platform_specific.h"
#include "awebjs.h"
#include "jprotos.h"

struct Date             /* Used as internal object value */
{  double date;         /* ms since 1-1-1970, now stored as GMT / UTC for easier consistency with ECMA spec */
};



struct Brokentime
{  struct tm tm;
   int tm_millis;
};
/*-----------------------------------------------------------------------*/
/* Replacements for gmtime and mktime;  functions */
/* Based in part on libnix originals but adapted to use double accept negatives and */
/* and be thread safe */

static const char monthtable[] = { 31,29,31,30,31,30,31,31,30,31,30 };

double round(double n)
{
    if (n > 0.0)
    {
        n = floor(n);
    }
    else
    if (n < 0.0)
    {
        n = ceil(n);
    }
    return n;
}

static double amktime(struct Brokentime *bt)

{
#if defined(__amigaos4__) & defined(__CLIB2__)
    double time = NAN;
#else
    double time = 0;
#endif
    int year,leapyear=0,i;

    if(bt)
    {
        year = bt->tm.tm_year + 1899;
        time = (double)((year/400)*146097);
        year %=400;
        if(year==399)
            leapyear++;
        time += (double)((year/100)*36524);
        year %=100;
        if(year == 99)
            leapyear--;
        time += (double)((year/4)*1461);
        year %=4;
        if(year==3)
            leapyear++;
        time += (double)(year*365 - 719162 + bt->tm.tm_mday-1);
        for (i=0;i<bt->tm.tm_mon;i++)
        {
            time += (double)monthtable[i];
        }
        if(!leapyear && bt->tm.tm_mon > 1)
            time -= 1.0;

        /* time should now conatin days since 1970 */

        time *=24.0;
        time += (double)bt->tm.tm_hour;
        time *=60.0;
        time += bt->tm.tm_min;
        time *=60.0;
        time += (double)bt->tm.tm_sec;
        time *=1000.0;
        time += (double)bt->tm_millis;
    }
    return time;
}


static struct Brokentime *agmtime(double *t, struct Brokentime *bt)
{
    double time = *t/1000.0;
    int leapday = 0, leapyear = 0,i;
    int days;

    bt->tm_millis = *t - time * 1000.0;
    bt->tm.tm_sec = (int)fmod(time,60);
    if(bt->tm.tm_sec < 0)
    {
        bt->tm.tm_sec += 60;
        time -= 60.0;
    }
    time = round(time/60.0);
    bt->tm.tm_min = (int)fmod(time,60.0);
    if(bt->tm.tm_min < 0)
    {
        bt->tm.tm_min += 60;
        time -= 60.0;
    }
    time = round(time/60.0);
    bt->tm.tm_hour = (int)fmod(time,24.0);
    if(bt->tm.tm_hour < 0)
    {
        bt->tm.tm_hour += 24;
        time -= 24.0;
    }
    time = round(time/24.0) + 719162.0;

    bt->tm.tm_wday = (int)fmod((time + 1.0),7);

    /* time now contains days since 1.1.1 */
    /* max range of days will fit in int so convert to int to avoid == errors on double */

    days = (int)time;


    /*146097 is num days in 400 years (97 are leaps 1/4 less the three 00s 400 itself is a leap) */
    bt->tm.tm_year = days/146097 * 400 -1899;
    days %= 146097;
    if(days > 145731)
    {
        leapyear ++; /* day is one of the 400th*/
        if(days == 146096)
        {
            days--;
            leapday++;
        }

    }
    bt->tm.tm_year += days/36524*100;
    days %= 36524;
    if(days>=36159)
    {
        leapyear--;
    }
    bt->tm.tm_year += days/1461*4;
    days %= 1461;
    if(days >= 1095)
    {
        leapyear ++;
        if(days==1460)
        {
            days--;
            leapday++;
        }
    }
    bt->tm.tm_year += days/365;


    days = days % 365 + leapday;
    if(days < 0)
    {
        days +=365;
    }

    bt->tm.tm_yday = days;
    if(!leapyear && days >= 31 + 28)
    {
        days ++;
    }
    for(i=0;i<11;i++)
    {
        if(days < monthtable[i])
        {
            break;
        }
        else
        {
            days -= monthtable[i];
        }
    }
    bt->tm.tm_mon = i;
    bt->tm.tm_mday = days +1;
    bt->tm.tm_isdst = -1; /* we don't use this at the moment, wtach out for future */
    return bt;

}


/*-----------------------------------------------------------------------*/

/* SAS/C system function replacement to avoid ENV access and bogus local
 * time offsets */
#ifdef __SASC
void __tzset(void)
{  __tzstn[0]='\0';
   __tzname[0]=__tzstn;
   __timezone=0;
   __tzdtn[0]='\0';
   __tzname[1]=__tzdtn;
   __daylight=0;
}
#endif
/*-----------------------------------------------------------------------*/

/* Find the numeric value of Nth argument */
static double Argument(struct Jcontext *jc,long n)
{  struct Variable *var;
   for(var=jc->functions.first->local.first;n && var->next;var=var->next,n--);
   if(var->next)
   {  Tonumber(&var->val,jc);
      if(var->val.attr==VNA_VALID)
      {  return var->val.value.nvalue;
      }
   }
   return 0;
}

static int Numargs(struct Jcontext *jc)
{
    struct Variable *var;
    int n;
    for(n = 0, var=jc->functions.first->local.first;var->next;var=var->next,n++);
    return n;
}

/* Get a pointer to broken-down time from (this) */
static void Gettime(struct Jcontext *jc,struct Brokentime *bt)
{  struct Jobject *jo=jc->jthis;
   if(jo && jo->internal)
   {
      agmtime( &(((struct Date *)jo->internal)->date),bt);
   }
}

static void Getlocaltime(struct Jcontext *jc,struct Brokentime *bt)
{  struct Jobject *jo=jc->jthis;
   double lt;
   if(jo && jo->internal)
   {
      lt = ((struct Date *)jo->internal)->date - 1000.0 * (double)(locale->loc_GMTOffset*60);

      agmtime( &lt ,bt);
   }
}



/* Set (this) to new date */
static void Settime(struct Jcontext *jc,struct Brokentime *bt)
{  struct Jobject *jo=jc->jthis;
   if(jo && jo->internal)
   {
      Asgnumber(RETVAL(jc),VNA_VALID,(((struct Date *)jo->internal)->date = amktime(bt)));
   }
}

static void Setlocaltime(struct Jcontext *jc,struct Brokentime *bt)
{  struct Jobject *jo=jc->jthis;
   if(jo && jo->internal)
   {
      Asgnumber(RETVAL(jc),VNA_VALID,(((struct Date *)jo->internal)->date = amktime(bt) + 1000.0 * (double)(locale->loc_GMTOffset*60)));
   }
}


static UBYTE months[12][4]=
{  "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

/* Get next token from date string */
static UBYTE *Getdatetoken(UBYTE *p,UBYTE *token)
{  while(*p && !isalnum(*p)) p++;
   while(*p && isalnum(*p)) *token++=*p++;
   *token='\0';
   return p;
}

/* Scan date, return GMT date value */
double Scandate(UBYTE *buf)
{  UBYTE *p;
   UBYTE token[16];
   struct Brokentime bt={0};
   BOOL ok,first=TRUE,gmt=FALSE;
   short i;
   double time;
   short offset=0;
   p=buf;
   for(;;)
   {  p=Getdatetoken(p,token);
      if(!*token) break;
      ok=FALSE;
      for(i=0;i<12;i++)
      {  if(STRNIEQUAL(token,months[i],3))
         {  bt.tm.tm_mon=i;
            ok=TRUE;
            break;
         }
      }
      if(ok) continue;
      if(*p==':')
      {  sscanf(token,"%hd",&i);
         bt.tm.tm_hour=i;
         p=Getdatetoken(p,token);
         sscanf(token,"%hd",&i);
         bt.tm.tm_min=i;
         p=Getdatetoken(p,token);
         sscanf(token,"%hd",&i);
         bt.tm.tm_sec=i;
         continue;
      }
      if(sscanf(token,"%hd",&i))
      {  if(first)
         {  bt.tm.tm_mday=i;
            first=FALSE;
         }
         else
         {  if(i<70) i+=2000;          /* e.g. 01 -> 2001 */
            else if(i<1970) i+=1900;   /* e.g. 96 -> 1996 */
                                       /* else: e.g. 1996 */
            bt.tm.tm_year=i-1900;
         }
         continue;
      }
      if(STRIEQUAL(token,"GMT"))
      {  gmt=TRUE;
         sscanf(p,"%hd",&offset);
         break;
      }
   }
   time=amktime(&bt);

   if(offset)
   {  time-= 1000.0 * 3600.0 * (double)(offset/100);
      time-= 1000.0 * 60.0 * (double)(offset%100);
   }
   else if(!gmt)
   {
       time += 1000.0 * (double)(locale->loc_GMTOffset*60);
   }
   return time;
}


/* call type exception if not date */

BOOL isthisdate(struct Jcontext *jc,struct Jobject *jo)
{
    if(jo && jo->internal && jo->type==OBJT_DATE)
    {
        return TRUE;
    }
    Runtimeerror(jc,NTE_TYPE,jc->elt,"Date method called on incompatable object type");
    return FALSE;
}
/*-----------------------------------------------------------------------*/

/* Convert (jthis) to string */
static void Datetostring(struct Jcontext *jc)
{  struct Brokentime bt;
   UBYTE buffer[64];
   if(isthisdate(jc,jc->jthis))
   {
       Getlocaltime(jc,&bt);
       if(!strftime(buffer,63,"%a, %d %b %Y %H:%M:%S",&bt.tm)) *buffer='\0';
       Asgstring(RETVAL(jc),buffer,jc->pool);
   }
}

/* Convert to GMT string */
static void Datetogmtstring(struct Jcontext *jc)
{  UBYTE buffer[64];
   struct Jobject *jo=jc->jthis;
   struct Brokentime bt = {0};
   double time;
   if(isthisdate(jc,jo))
   {
       time=((struct Date *)jo->internal)->date;
       agmtime(&time,&bt);
       if(!strftime(buffer,63,"%a, %d %b %Y %H:%M:%S GMT",&bt.tm)) *buffer='\0';
       Asgstring(RETVAL(jc),buffer,jc->pool);
   }
}

static void Datetodatestring(struct Jcontext *jc)
{  struct Brokentime bt;
   UBYTE buffer[64];
   if(isthisdate(jc,jc->jthis))
   {
       Getlocaltime(jc,&bt);
       if(!strftime(buffer,63,"%a, %d %b %Y",&bt.tm)) *buffer='\0';
       Asgstring(RETVAL(jc),buffer,jc->pool);
   }
}
static void Datetotimestring(struct Jcontext *jc)
{  struct Brokentime bt;
   UBYTE buffer[64];
   if(isthisdate(jc,jc->jthis))
   {
      Getlocaltime(jc,&bt);
      if(!strftime(buffer,63,"%H:%M:%S",&bt.tm)) *buffer='\0';
      Asgstring(RETVAL(jc),buffer,jc->pool);
   }
}


static void Datevalueof(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   double d;
   if(isthisdate(jc,jc->jthis))
   {
      d=((struct Date *)jo->internal)->date;
      Asgnumber(RETVAL(jc),VNA_VALID,d);
   }
}

/* Locale hook. (hook)->h_Data points at next buffer position. */
static void __saveds USRFUNC_H3
(
 , Lputchar,
 struct Hook *,       hook,     A0,
 void *, dummy,         A2,
 UBYTE,                c,        A1
)
{
     USRFUNC_INIT

   UBYTE *p=hook->h_Data;
   *p++=c;
   hook->h_Data=p;

     USRFUNC_EXIT
}

/* Convert to locale string */
static void Datetolocalestring(struct Jcontext *jc)
{  struct Hook hook;
   UBYTE buffer[64];
   struct Jobject *jo=jc->jthis;
   if(isthisdate(jc,jo))
   {
       double d;
       struct DateStamp ds={0};
       long t;
       hook.h_Entry=(HOOKFUNC)Lputchar;
       hook.h_Data=buffer;
       d=((struct Date *)jo->internal)->date;
       t=(long)(d/1000);
       /* JS dates are from 1970, locale dates from 1978. 8 years = 8 * 365 + 2 = 2922 */
       ds.ds_Days=t/86400-2922;
       t=t%86400;
       ds.ds_Minute=t/60;
       t=t%60;
       ds.ds_Tick=t*TICKS_PER_SECOND;
       FormatDate(locale,locale->loc_ShortDateTimeFormat,&ds,&hook);
       Asgstring(RETVAL(jc),buffer,jc->pool);
   }
}

static void Datetolocaledatestring(struct Jcontext *jc)
{  struct Hook hook;
   UBYTE buffer[64];
   struct Jobject *jo=jc->jthis;
   if(isthisdate(jc,jo))
   {
       double d;
       struct DateStamp ds={0};
       long t;
       hook.h_Entry=(HOOKFUNC)Lputchar;
       hook.h_Data=buffer;
       d=((struct Date *)jo->internal)->date;
       t=(long)(d/1000);
       /* JS dates are from 1970, locale dates from 1978. 8 years = 8 * 365 + 2 = 2922 */
       ds.ds_Days=t/86400-2922;
       t=t%86400;
       ds.ds_Minute=t/60;
       t=t%60;
       ds.ds_Tick=t*TICKS_PER_SECOND;
       FormatDate(locale,locale->loc_ShortDateFormat,&ds,&hook);
       Asgstring(RETVAL(jc),buffer,jc->pool);
   }
}

static void Datetolocaletimestring(struct Jcontext *jc)
{  struct Hook hook;
   UBYTE buffer[64];
   struct Jobject *jo=jc->jthis;
   if(isthisdate(jc,jc->jthis))
   {
       double d;
       struct DateStamp ds={0};
       long t;
       hook.h_Entry=(HOOKFUNC)Lputchar;
       hook.h_Data=buffer;
       d=((struct Date *)jo->internal)->date;
       t=(long)(d/1000);
       /* JS dates are from 1970, locale dates from 1978. 8 years = 8 * 365 + 2 = 2922 */
       ds.ds_Days=t/86400-2922;
       t=t%86400;
       ds.ds_Minute=t/60;
       t=t%60;
       ds.ds_Tick=t*TICKS_PER_SECOND;
       FormatDate(locale,locale->loc_ShortTimeFormat,&ds,&hook);
       Asgstring(RETVAL(jc),buffer,jc->pool);
   }
}

static void Datesetyear(struct Jcontext *jc)
{
   int args = Numargs(jc);
   int y;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
       Getlocaltime(jc,&bt);
       if(args > 0)
       {
           y = (int)Argument(jc,0);
           if (y <= 99) y += 1900;
           y-=1900;
           bt.tm.tm_year=y;
       }
       Setlocaltime(jc,&bt);
   }
}

static void Datesetutcyear(struct Jcontext *jc)
{
   int args = Numargs(jc);
   int y;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   if(args > 0)
   {
       y = (int)Argument(jc,0);
       if (y <= 99) y += 1900;
       y-=1900;
       bt.tm.tm_year=y;
   }
   Settime(jc,&bt);
   }
}



static void Datesetfullyear(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   int y;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);
   if(args > 0)
   {
       y = (int)Argument(jc,0);
       /*if(y>1900)*/ y-=1900;  /* looks a dubious thing to me surley y shouls always br -= 1900, lets try it*/
       bt.tm.tm_year=y;
   }
   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm.tm_mon=(int)n;
   }

   if(args > 2)
   {
       n=Argument(jc,2);
       bt.tm.tm_mday=(int)n;
   }
   Setlocaltime(jc,&bt);
   }
}

static void Datesetutcfullyear(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   int y;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   if(args > 0)
   {
       y = (int)Argument(jc,0);
       /*if(y>1900)*/ y-=1900;  /* looks a dubious thing to me surley y shouls always br -= 1900, lets try it*/
       bt.tm.tm_year=y;
   }
   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm.tm_mon=(int)n;
   }

   if(args > 2)
   {
       n=Argument(jc,2);
       bt.tm.tm_mday=(int)n;
   }
   Settime(jc,&bt);
   }
}

static void Datesetmonth(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);

   if(args > 0)
   {
       n=Argument(jc,0);
       bt.tm.tm_mon=(int)n;
   }

   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm.tm_mday=(int)n;
   }
   Setlocaltime(jc,&bt);
   }
}

static void Datesetutcmonth(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);

   if(args > 0)
   {
       n=Argument(jc,0);
       bt.tm.tm_mon=(int)n;
   }

   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm.tm_mday=(int)n;
   }
   Settime(jc,&bt);
   }
}

static void Datesetutcdate(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   if(args > 0)
   {
       n=Argument(jc,0);
       bt.tm.tm_mday=(int)n;
   }

   Settime(jc,&bt);
   }
}


static void Datesetdate(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);
   if(args > 0)
   {
       n=Argument(jc,0);
       bt.tm.tm_mday=(int)n;
   }
   Setlocaltime(jc,&bt);
   }
}

static void Datesethours(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);

   if(args > 0)
   {
       n=Argument(jc,0);
       bt.tm.tm_hour=(int)n;
   }
   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm.tm_min=(int)n;
   }
   if(args > 2)
   {
       n=Argument(jc,2);
       bt.tm.tm_sec=(int)n;
   }
   if(args > 3)
   {
       n=Argument(jc,3);
       bt.tm_millis=(int)n;
   }
   Setlocaltime(jc,&bt);
   }
}

static void Datesetutchours(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);

   if(args > 0)
   {
       n=Argument(jc,0);
       bt.tm.tm_hour=(int)n;
   }
   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm.tm_min=(int)n;
   }
   if(args > 2)
   {
       n=Argument(jc,2);
       bt.tm.tm_sec=(int)n;
   }
   if(args > 3)
   {
       n=Argument(jc,3);
       bt.tm_millis=(int)n;
   }
   Settime(jc,&bt);
   }
}


static void Datesetminutes(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);
   if(args > 0)
   {
       n=Argument(jc,0);
       bt.tm.tm_min=(int)n;
   }
   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm.tm_sec=(int)n;
   }
   if(args > 2)
   {
       n=Argument(jc,2);
       bt.tm_millis=(int)n;
   }
   Setlocaltime(jc,&bt);
   }
}

static void Datesetutcminutes(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   if(args > 0)
   {
       n=Argument(jc,0);
       bt.tm.tm_min=(int)n;
   }
   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm.tm_sec=(int)n;
   }
   if(args > 2)
   {
       n=Argument(jc,2);
       bt.tm_millis=(int)n;
   }
   Settime(jc,&bt);
   }
}


static void Datesetseconds(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);

   if(args > 0)
   {
       n=Argument(jc,1);
       bt.tm.tm_sec=(int)n;
   }
   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm_millis=(int)n;
   }
   Setlocaltime(jc,&bt);
   }
}

static void Datesetutcseconds(struct Jcontext *jc)
{
   int args = Numargs(jc);
   double n;
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);

   if(args > 0)
   {
       n=Argument(jc,0);
       bt.tm.tm_sec=(int)n;
   }
   if(args > 1)
   {
       n=Argument(jc,1);
       bt.tm_millis=(int)n;
   }
   Settime(jc,&bt);
   }
}


static void Datesetmilliseconds(struct Jcontext *jc)
{
   double n=Argument(jc,0);
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
       Getlocaltime(jc,&bt);
       bt.tm_millis=(int)n;
       Setlocaltime(jc,&bt);
   }
}
static void Datesetutcmilliseconds(struct Jcontext *jc)
{
   double n=Argument(jc,0);
   struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
       Gettime(jc,&bt);
       bt.tm_millis=(int)n;
       Settime(jc,&bt);
   }
}


static void Datesettime(struct Jcontext *jc)
{  double n=Argument(jc,0);
   struct Jobject *jo=jc->jthis;
   if(isthisdate(jc,jo))
   {
    ((struct Date *)jo->internal)->date=n;
   }
}

static void Dategetday(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
       Getlocaltime(jc,&bt);
       Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_wday));
   }
}

static void Dategetutcday(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
       Gettime(jc,&bt);
       Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_wday));
   }
}

static void Dategetyear(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);
   if(bt.tm.tm_year>=100)
   {  bt.tm.tm_year+=1900;
   }
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_year));
   }
}

static void Dategetutcyear(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   if(bt.tm.tm_year>=100)
   {  bt.tm.tm_year+=1900;
   }
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_year));
   }
}


static void Dategetfullyear(struct Jcontext* jc)
{
    struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
    Getlocaltime(jc,&bt);
    bt.tm.tm_year +=1900;
    Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_year));
    }
}

static void Dategetutcfullyear(struct Jcontext* jc)
{
    struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {

    Gettime(jc,&bt);
    bt.tm.tm_year +=1900;
    Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_year));
    }
}


static void Dategetmonth(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {

   Getlocaltime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_mon));
   }
}

static void Dategetutcmonth(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_mon));
   }
}


static void Dategetdate(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_mday));
   }
}

static void Dategetutcdate(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_mday));
   }
}


static void Dategethours(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_hour));
   }
}

static void Dategetutchours(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_hour));
   }
}


static void Dategetminutes(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_min));
   }
}

static void Dategetutcminutes(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_min));
   }
}


static void Dategetseconds(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_sec));
   }
}

static void Dategetutcseconds(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm.tm_sec));
   }
}

static void Dategetmilliseconds(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Getlocaltime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm_millis));
   }
}

static void Dategetutcmilliseconds(struct Jcontext *jc)
{  struct Brokentime bt;
   if(isthisdate(jc,jc->jthis))
   {
   Gettime(jc,&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,(double)(bt.tm_millis));
   }
}



static void Dategettime(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   if(isthisdate(jc,jc->jthis))
   {
      Asgnumber(RETVAL(jc),VNA_VALID,
         ((struct Date *)jo->internal)->date);
   }
}

static void Dategettimezoneoffset(struct Jcontext *jc)
{  long offset=locale->loc_GMTOffset;
   Asgnumber(RETVAL(jc),VNA_VALID,(double)offset);
}

static void Dateparse(struct Jcontext *jc)
{  struct Variable *var;
   double d=0.0;
   var=jc->functions.first->local.first;
   if(var->next)
   {  Tostring(&var->val,jc);
      d=Scandate(var->val.value.svalue);
      d-=60000.0*locale->loc_GMTOffset;
   }
   Asgnumber(RETVAL(jc),VNA_VALID,d);
}

static void Dateutc(struct Jcontext *jc)
{  struct Brokentime bt={0};
   double time;
   bt.tm.tm_year=(int)Argument(jc,0);
   bt.tm.tm_mon=(int)Argument(jc,1);
   bt.tm.tm_mday=(int)Argument(jc,2);
   bt.tm.tm_hour=(int)Argument(jc,3);
   bt.tm.tm_min=(int)Argument(jc,4);
   bt.tm.tm_sec=(int)Argument(jc,5);
   time=amktime(&bt);
   Asgnumber(RETVAL(jc),VNA_VALID,time);
}

/* Dispose a Date object */
static void Destructor(struct Date *d)
{
   FREE(d);
}

/* Make (jthis) a new Date object */
static void Constructor(struct Jcontext *jc)
{  struct Jobject *jo=jc->jthis;
   struct Date *d;
   if(jc->flags&EXF_CONSTRUCT)
   {  if(jo)
      {  if(d=ALLOCSTRUCT(Date,1,0,jc->pool))
         {  struct Variable *arg1;
            jo->internal=d;
            jo->dispose=(Objdisposehookfunc *)Destructor;
            jo->type=OBJT_DATE;
            arg1=jc->functions.first->local.first;
            if(arg1->next && arg1->val.type!=VTP_UNDEFINED)
            {  if(arg1->next->next && arg1->next->val.type!=VTP_UNDEFINED)
               {  struct Brokentime bt={0};
                  bt.tm.tm_year=(int)Argument(jc,0);
                  if(bt.tm.tm_year<70) bt.tm.tm_year+=100;
                  else if(bt.tm.tm_year>=1900) bt.tm.tm_year-=1900;
                  bt.tm.tm_mon=(int)Argument(jc,1);
                  bt.tm.tm_mday=(int)Argument(jc,2);
                  bt.tm.tm_hour=(int)Argument(jc,3);
                  bt.tm.tm_min=(int)Argument(jc,4);
                  bt.tm.tm_sec=(int)Argument(jc,5);
                  Settime(jc,&bt);
               }
               else
               {  /* Only 1 argument */
                  if(arg1->val.type==VTP_STRING)
                  {  d->date=Scandate(arg1->val.value.svalue)-60000.0*locale->loc_GMTOffset;
                  }
                  else
                  {  d->date=Argument(jc,0)-60000.0*locale->loc_GMTOffset;
                  }
               }
            }
            else
            {  d->date=Today();
            }
         }
      }
   }
   else
   {  /* Not called as a constructor; return date string */
      double date=Today();
      struct Brokentime bt ={0};
      UBYTE buffer[64];
      agmtime(&date,&bt);
      if(!strftime(buffer,63,"%a, %d %b %Y %H:%M:%S",&bt.tm)) *buffer='\0';
      Asgstring(RETVAL(jc),buffer,jc->pool);
   }
}

/*-----------------------------------------------------------------------*/

double Today(void)
{  unsigned int clock[2]={ 0, 0 };
   double t;
   timer((long *)clock);
   t=1000.0*clock[0]+clock[1]/1000;
   /* System time is since 1978, convert to 1970 (2 leap years) */
   t+=(8*365+2)*24*60*60*1000.0;
   return t;
}

void Initdate(struct Jcontext *jc, struct Jobject *jscope)
{  struct Jobject *jo,*f;
   struct Variable *prop;
   if(jo=Internalfunction(jc,"Date",(Internfunc *)Constructor,"arg1","month","day",
      "hours","minutes","seconds",NULL))
   {

      Initconstruct(jc,jo,"Object",jc->object);
      Addprototype(jc,jo,Getprototype(jo->constructor));

      // Addglobalfunction(jc,jo);
      // Keepobject(jo,TRUE);
      if((prop = Addproperty(jscope,"Date")))
      {
           Asgobject(&prop->val,jo);
           prop->flags |= VARF_DONTDELETE;
      }

      if(f=Internalfunction(jc,"parse",(Internfunc *)Dateparse,"dateString",NULL))
      {  Addinternalproperty(jc,jo,f);
      }
      if(f=Internalfunction(jc,"UTC",(Internfunc *)Dateutc,"year","month","day","hrs","min","sec",NULL))
      {  Addinternalproperty(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toString",(Internfunc *)Datetostring,NULL))
      {  Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"toDateString",(Internfunc *)Datetodatestring,NULL))
      {  Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"toTimeString",(Internfunc *)Datetotimestring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toGMTString",(Internfunc *)Datetogmtstring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toUTCString",(Internfunc *)Datetogmtstring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toLocaleString",(Internfunc *)Datetolocalestring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toLocaleDateString",(Internfunc *)Datetolocaledatestring,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"toLocaleTimeString",(Internfunc *)Datetolocaletimestring,NULL))
      {  Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"valueOf",(Internfunc *)Datevalueof,NULL))
      {  Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"setYear",(Internfunc *)Datesetyear,"yearValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setFullYear",(Internfunc *)Datesetfullyear,"yearValue","monthValue","dateValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"setMonth",(Internfunc *)Datesetmonth,"monthValue","dateVlaue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setDate",(Internfunc *)Datesetdate,"dateValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setHours",(Internfunc *)Datesethours,"hoursValue","minutesValue","secondsValue","millisValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setMinutes",(Internfunc *)Datesetminutes,"minutesValue","secondsValue","millisValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setSeconds",(Internfunc *)Datesetseconds,"secondsValue","millisValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setMilliseconds",(Internfunc *)Datesetmilliseconds,"secondsValue","millisValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setUTCYear",(Internfunc *)Datesetutcyear,"yearValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setUTCFullYear",(Internfunc *)Datesetutcfullyear,"yearValue","monthValue","dateValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setUTCMonth",(Internfunc *)Datesetutcmonth,"monthValue","dateVlaue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setUTCDate",(Internfunc *)Datesetutcdate,"dateValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setUTCHours",(Internfunc *)Datesetutchours,"hoursValue","minutesValue","secondsValue","millisValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setUTCMinutes",(Internfunc *)Datesetutcminutes,"minutesValue","secondsValue","millisValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setUTCSeconds",(Internfunc *)Datesetutcseconds,"secondsValue","millisValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setUTCMilliseconds",(Internfunc *)Datesetutcmilliseconds,"secondsValue","millisValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"setTime",(Internfunc *)Datesettime,"timeValue",NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getDay",(Internfunc *)Dategetday,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getYear",(Internfunc *)Dategetyear,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getFullYear",(Internfunc *)Dategetfullyear,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getMonth",(Internfunc *)Dategetmonth,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getDate",(Internfunc *)Dategetdate,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getHours",(Internfunc *)Dategethours,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getMinutes",(Internfunc *)Dategetminutes,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getSeconds",(Internfunc *)Dategetseconds,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getMilliseconds",(Internfunc *)Dategetmilliseconds,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getUTCDay",(Internfunc *)Dategetutcday,NULL))
      {  Addtoprototype(jc,jo,f);
      }

      if(f=Internalfunction(jc,"getUTCYear",(Internfunc *)Dategetutcyear,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getUTCFullYear",(Internfunc *)Dategetutcfullyear,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getUTCMonth",(Internfunc *)Dategetutcmonth,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getUTCDate",(Internfunc *)Dategetutcdate,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getUTCHours",(Internfunc *)Dategetutchours,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getUTCMinutes",(Internfunc *)Dategetutcminutes,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getUTCSeconds",(Internfunc *)Dategetutcseconds,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getUTCMilliseconds",(Internfunc *)Dategetutcmilliseconds,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getTime",(Internfunc *)Dategettime,NULL))
      {  Addtoprototype(jc,jo,f);
      }
      if(f=Internalfunction(jc,"getTimezoneOffset",(Internfunc *)Dategettimezoneoffset,NULL))
      {  Addtoprototype(jc,jo,f);
      }


   }
}
