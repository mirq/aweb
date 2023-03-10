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

/* pluginlib.h
 *
 * AWeb plugin module project dependent header file
 *
 * It is included in startup.h to import project specific
 * constants.
 *
 * In this file, the following macros must be defined:
 *
 * PLUGIN_VERSION       Version of the module to be generated
 * PLUGIN_REVISION      Revision of the module to be generated
 * PLUGIN_LIBNAME       Name of the module, must match the name
 *                      used to open the library
 * PLUGIN_LIBID         Lib ID string of the module
 * PLUGIN_SIZE          Size in bytes of the actual library node
 *                      structure
 *
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include "platform_specific.h"

#define PLUGIN_VERSION     1
#define PLUGIN_REVISION    14
#define PLUGIN_LIBNAME     "awebpng.awebplugin"
#define PLUGIN_LIBID       "$VER: awebpng 1.14 (" __AMIGADATE__ ") " CPU
#define PLUGIN_SIZE        sizeof(struct AwebPngBase)

struct AwebPngBase
{  struct Library node;
   ULONG sourcedriver;
   ULONG copydriver;
};
