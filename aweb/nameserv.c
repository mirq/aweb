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

/* nameserv.c - aweb name cache */

#include "aweb.h"
#include <netdb.h>

#include "awebtcp.h"
#include "idn.h"

#include <proto/exec.h>

struct Hostname
{  NODE(Hostname);
   struct hostent hent;
   UBYTE *hostname;        /* official host name */
   UBYTE *name;            /* requested name */
   UBYTE *addrp;           /* points to addr */
   UBYTE *nullp;           /* terminates list */
   UBYTE addr[4];          /* internet address */
};

static LIST(Hostname) names;
static BOOL inited;

static struct SignalSemaphore namesema;

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/

BOOL Initnameserv(void)
{  InitSemaphore(&namesema);
   NEWLIST(&names);
   inited=TRUE;
   return TRUE;
}

void Freenameserv(void)
{  struct Hostname *hn;
   if(inited)
   {  while(hn=(struct Hostname *)REMHEAD(&names))
      {  if(hn->hostname) FREE(hn->hostname);
         if(hn->name) FREE(hn->name);
         FREE(hn);
      }
   }
}

struct hostent *Lookup(UBYTE *name,struct Library *base)
{  struct Hostname *hn;
   struct hostent *hent=NULL;
   short i;

   /* IDNA encoding */
   name = Dorfc3490(name, IDN_ENCODE);
   ObtainSemaphore(&namesema);
   for(hn=names.first;hn->next;hn=hn->next)
   {  if(STRIEQUAL(hn->name,name))
      {  hent=&hn->hent;
      }
   }
   ReleaseSemaphore(&namesema);
   if(hent) {
      FREE(name);
      return hent;
   }
   if(hent=a_gethostbyname(name,base))
   {
      if(hn=ALLOCSTRUCT(Hostname,1,MEMF_CLEAR))
      {  if((hn->hostname=Dupstr(hent->h_name,-1))
         && (hn->name=Dupstr(name,-1)))
         {
            hn->hent=*hent;
            hn->hent.h_name=hn->hostname;
            for(i=0;i<4;i++) hn->addr[i]=(*hent->h_addr_list)[i];
            hn->addrp=hn->addr;
       /* UBYTE** assigned to char** mmmh... */
            hn->hent.h_addr_list=(char **)&hn->addrp;
            hent=&hn->hent;
            ObtainSemaphore(&namesema);
            ADDTAIL(&names,hn);
            ReleaseSemaphore(&namesema);
         }
         else
         {  if(hn->hostname) FREE(hn->hostname);
            if(hn->name) FREE(hn->name);
            FREE(hn);
         }
      }
   }
   FREE(name);
   return hent;
}
