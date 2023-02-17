/**********************************************************************

   This file is part of the AWeb-II distribution

   Copyright (C) 2002 Yvon Rozijn
   Copyright (C) 2002-2003 The AWeb Developement Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the AWeb Public License as included in this
   distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   AWeb Public License for more details.

   $Id: module_call.c,v 1.2 2009/06/15 17:05:23 opi Exp $

   Desc: call list of functions in ascending or descending priority

***********************************************************************/


#include "module.h"

/* CallFunctionList() calls all the functions  in Ascending or                */
/* Descending order of priority                                               */
/* This function uses a similar algorithm to the standard libnix startup code */
/* to sort through the priorities but re-implemented in C                     */


void CallFunctionList( struct FunctionList *funcs, int direction)
{
    unsigned char current_pri;
    unsigned char running_pri;
    int not_done = 0;

    current_pri = (direction == ASCENDING ? 0 : 255 );
    running_pri = (direction == ASCENDING ? 255 : 0 );

    do
    {
        int i;
        not_done = 0;
        for(i=0;i < funcs->fl_Size / 2; i++)
        {

            /* execute function if at relavent priority */

            if (funcs->fl_Funcs[i].fli_Pri == current_pri)
            {

                funcs->fl_Funcs[i].fli_Func();

            }

            if (direction == ASCENDING)
            {

                if (
                     (funcs->fl_Funcs[i].fli_Pri < running_pri) &&
                     (funcs->fl_Funcs[i].fli_Pri > current_pri) )
                {
                    running_pri = funcs->fl_Funcs[i].fli_Pri;
                    not_done |= 1;
                }

            }
            else
            {

                if (
                     (funcs->fl_Funcs[i].fli_Pri > running_pri) &&
                     (funcs->fl_Funcs[i].fli_Pri < current_pri) )
                {
                    running_pri = funcs->fl_Funcs[i].fli_Pri;
                    not_done |= 1;
                }

            }

        }

        current_pri = running_pri;
        running_pri = (direction == ASCENDING ? 255 : 0 );

    } while (not_done);


}
