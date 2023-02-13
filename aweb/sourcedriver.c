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

/* sourcedriver.c - AWeb general sourcedriver object superclass */

#include "aweb.h"
#include "sourcedriver.h"
#include <proto/utility.h>

/*----------------------------------------------------------------------*/

static void Disposesourcedriver(struct Sourcedriver *sdv)
{  Amethodas(AOTP_OBJECT,sdv,AOM_DISPOSE);
}

USRFUNC_H2
(
static long  , Sourcedriver_Dispatcher,
struct Sourcedriver *,sdv,A0,
struct Amessage *,amsg,A1
)
{
    USRFUNC_INIT
  long result=0;
   switch(amsg->method)
   {  case AOM_DISPOSE:
         Disposesourcedriver(sdv);
         break;
      case AOM_DEINSTALL:
         break;
      default:
         break;
   }
   return result;

    USRFUNC_EXIT
}

/*----------------------------------------------------------------------*/

BOOL Installsourcedriver(void)
{  if(!Amethod(NULL,AOM_INSTALL,AOTP_SOURCEDRIVER,(Tag)Sourcedriver_Dispatcher)) return FALSE;
   return TRUE;
}
