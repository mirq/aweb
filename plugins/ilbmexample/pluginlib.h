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
 * PLUGIN_VERSION   Version of the module to be generated
 * PLUGIN_REVISION   Revision of the module to be generated
 * PLUGIN_LIBNAME   Name of the module, must match the name
 *         used to open the library
 * PLUGIN_LIBID    Lib ID string of the module
 * PLUGIN_SIZE      Size in bytes of the actual library node
 *         structure
 *
 * Optionally the following macros can be #define'd if the plugin
 * supports the feature, or #undef'ed if the plugin doesn't
 * support the feature:
 *
 * PLUGIN_COMMANDPLUGIN If the Commandplugin() function is supported
 * PLUGIN_FILTERPLUGIN   If the Filterplugin() function is supported
 *
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include "platform_specific.h"

#define PLUGIN_VERSION      2
#define PLUGIN_REVISION    0
#define PLUGIN_LIBNAME      "ilbmexample.awebplugin"
#define PLUGIN_LIBID      "ilbmexample 2.0 " __AMIGADATE__
#define PLUGIN_SIZE      sizeof(struct ILBMExampleBase)

struct ILBMExampleBase
{  struct Library node;
   ULONG sourcedriver;
   ULONG copydriver;
};


/* This plugin doesn't support commands, and is not a filter type plugin */
// #define PLUGIN_COMMANDPLUGIN
#undef PLUGIN_COMMANDPLUGIN

// #define PLUGIN_FILTERPLUGIN
#undef PLUGIN_FILTERPLUGIN
