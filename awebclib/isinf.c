#if defined(__amigaos4__)

#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/timer.h>

#include <proto/dos.h>


//void timer(long clock[2])
//{
//   __ITimer->GetSysTime((struct timeval *)&clock[0]);
//   struct DateStamp ds;
//   IDOS->DateStamp(&ds);
//   clock[0]=(ds.ds_Days*24*60+ds.ds_Minute)*60+ds.ds_Tick/TICKS_PER_SECOND;
//   clock[1]=(ds.ds_Tick%TICKS_PER_SECOND)*(1000000/TICKS_PER_SECOND);
//}

#if defined(__amigaos4__)
#define __USE_OLD_TIMEVAL__
#include <sys/time.h>

void timer(long clock[2])
{
    struct timeval tv;
    struct timezone tzp;
    gettimeofday(&tv,&tzp);
    clock[0] = tv.tv_secs - 60 * tzp.tz_minuteswest;
    clock[1] = tv.tv_usec;
}

#endif


/*
int isinf(double d)
{
   uint32 *i = (uint32 *)&d;
   int exp = (i[0] & 0x7ff00000) >> 20;

   if ((exp == 0x7ff) && !(i[0] & 0xf0000) && !i[1])
   {
      return TRUE;
   } else {
      return FALSE;
   }
}

int isnan(double d)
{
   uint32 *i = (uint32 *)&d;
   int exp = (i[0] & 0x7ff00000) >> 20;

   if ((exp == 0x7ff) && (i[0] & 0xf0000 || i[1]))
   {
      return TRUE;
   } else {
      return FALSE;
   }
}

*/
#endif
