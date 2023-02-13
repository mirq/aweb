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

/* filterexample.h
 *
 * Example AWeb plugin module - project header file
 *
 * This file contains some general definitions shared between
 * the various source files.
 *
 */

#include <libraries/awebsupport.h>

/* Base pointers of libraries needed */
extern struct ExecBase *SysBase;         /* Defined in startup.c */
extern struct Library *AwebSupportBase;

/* Structure as our filter user data */
struct Filterdata
{
   BOOL first;                /* The first block is to be processed */
   BOOL on;                   /* Filter is on */
};
