/**********************************************************************
 *
 * This file is part of the AWeb-II distribution
 *
 * Copyright (C) 2009 Frank (Opi) Weber
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

/* builddate.c - Print out a datestring used for the version strings */
/* in the format '"DD.MM.YYYY"' */

#include <time.h>
#include <stdio.h>

int main(void)
{
   time_t t;
   struct tm *mytime;

   time(&t);
   mytime = localtime(&t);
   printf("\"%d.%d.%04d\"\n", mytime->tm_mday, mytime->tm_mon+1, mytime->tm_year+1900);
   return 0;
}
