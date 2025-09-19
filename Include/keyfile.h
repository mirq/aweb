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

/* keyfile.h - AWeb keyfile prototypes */

/*-----------------------------------------------------------------------*/

#ifndef AWEB_KEYFILE_H
#define AWEB_KEYFILE_H

#ifndef CPU
#define CPU "unknown cpu"
#endif

#define BETARELEASENR "167APL"
#define BETALIBREVISION 167

#define FULLRELEASE "3.5.092"
#define FULLLIBVERSION 3
#define FULLLIBREVISION 5

/*-----------------------------------------------------------------------*/

                           /* Select type of release: */
// #define BETAKEYFILE        /* Beta testers version, needs aweb.betakey */
// #define DEMOVERSION        /* Demo version, no key but limited features */
#define COMMERCIAL         /* Commercial version, no key */
// #define OSVERSION          /* Special Edition; implies DEMOVERSION, NETDEMO, NEED35 */

// #define DEVELOPER       /* no initial about requester */
// #define LOCALONLY       /* no network access */
// #define LIMITED         /* limited version. Use /support/LimitDemo to insert string */
// #define NEED35          /* needs OS 3.5 or higher */

/* Set these to the correct values when you create your own browser */
#define WWWADDRESS   "http://aweb.sunsite.dk"
#define APLADDRESS   "file://localhost/AWebPath:apl.txt"

/*-----------------------------------------------------------------------*/
/* Special settings, normally not set here */

// #define POPABOUT        /* is default for BETAVERSION unless DEVELOPER */
// #define NOKEYFILE       /* set if DEMOVERSION or COMMERCIAL. Don't set here */

// #define DEFAULTCFG      /* Path in ENV[ARC]: for default config */

// #define LOCALDEMO       /* local-only no-net demo */
// #define NETDEMO         /* network capable demo */
// #define NOAREXXPORTS    /* no ARexx ports */

// #define AWEBLIBVERSION  /* numeric library version */
// #define AWEBLIBREVISION /* numeric library revision */
// #define AWEBLIBVSTRING  /* library version string */

/*-----------------------------------------------------------------------*/

#define BETARELEASE "0." BETARELEASENR

#ifdef BETAKEYFILE
 #ifndef BETAVERSION
  #define BETAVERSION
 #endif
#endif

#ifdef BETAVERSION
 #ifndef DEVELOPER
  #ifndef POPABOUT
   #define POPABOUT
  #endif
 #endif
#endif

#ifndef BETAVERSION
 #ifndef DEMOVERSION
  #ifndef OSVERSION
   #define COMMERCIAL
  #endif
 #endif
#endif

#ifdef DEMOVERSION
 #define NOKEYFILE
#endif
#ifdef COMMERCIAL
 #define NOKEYFILE
#endif
#ifdef OSVERSION
 #define NOKEYFILE
#endif

#ifdef DEMOVERSION
 #ifdef LOCALONLY
  #define LOCALDEMO
 #else
  #define NETDEMO
 #endif
 #define NOAREXXPORTS
#endif

#ifdef OSVERSION
 #define DEMOVERSION
 #define NETDEMO
 #define NEED35
#endif

#ifdef BETAVERSION
 #define AWEBVERSION BETARELEASE
 #define RELEASECLASS "beta"
 #define AWEBLIBVERSION 0
 #define AWEBLIBREVISION BETALIBREVISION
 #define DEFAULTCFG "AWeb3"
#else
 #define AWEBVERSION FULLRELEASE
 #ifdef DEMOVERSION
  #ifdef OSVERSION
   #undef AWEBVERSION
   #define AWEBVERSION FULLRELEASE "." BETARELEASENR
   #define RELEASECLASS "SE"
   #define DEFAULTCFG "AWeb3SE"
  #else
   #ifdef LOCALONLY
    #define RELEASECLASS "LOCALDEMO"
   #else
    #define RELEASECLASS "DEMO"
   #endif
   #define DEFAULTCFG "AWeb3DEMO"
  #endif
 #else
  #ifdef NEED35
   #define RELEASECLASS "UFO"
  #else
   #define RELEASECLASS ""
  #endif
  #define DEFAULTCFG "AWeb3"
 #endif
 #define AWEBLIBVERSION FULLLIBVERSION
 #define AWEBLIBREVISION FULLLIBREVISION
#endif

#define AWEBLIBVSTRING AWEBVERSION RELEASECLASS CPU

#endif /* !AWEB_KEYFILE_H */
