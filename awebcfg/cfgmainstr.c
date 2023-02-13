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

/* cfgmainstr.c - AWebCfg main locale */

#define NOCFGLOCALE
#define NOLOCALE
#include "awebcfg.h"

#define CATCOMP_ARRAY
#include "locale.h"

UBYTE *Getmainstr(ULONG msg)
{  long i;
   for(i=0;i<sizeof(CatCompArray);i++)
   {  if(CatCompArray[i].cca_ID==msg)
      {  return GetCatalogStr(maincatalog,msg,(CONST STRPTR)CatCompArray[i].cca_Str);
      }
   }
   return "";
}
