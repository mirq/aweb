/**********************************************************************
 *
 * This file is part of the AWeb-II distribution
 *
 * Copyright (C) 2004 The AWeb Development Team
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

/** @file idn.h
 * AWeb IDNA related functions.
 */

#ifndef _AWEB_IDN_H
#define _AWEB_IDN_H

#include <limits.h>

enum punycode_status {
  punycode_success,     /**< Conversion successful. */
  punycode_bad_input,   /**< Input is invalid.                       */
  punycode_big_output,  /**< Output would exceed the space provided. */
  punycode_overflow     /**< Input needs wider integers to process.  */
};


/** basic(cp) tests whether cp is a basic code point: */
#define basic(cp) ((ULONG)(cp) < 0x80)

/** delim(cp) tests whether cp is a delimiter: */
#define delim(cp) ((cp) == delimiter)

enum IDN_MODE{
    IDN_ENCODE, /**< encode to ACE */
    IDN_DECODE  /**< decode from ACE */
    };


/** Handle IDNA conversion.
 * This function converts the hostname according to the rules defined in RFC
 * 3490 (IDNA). If mode is IDN_ENCODE all host labels are encoded as punycode
 * if necessary. If mode is IDN_DECODE any found ACE labels in punycode encoding
 * are decoded to plain Latin-1. If any of these steps fail an empty string is
 * returned.
 *
 * @param hostname Hostname to encode, labels separated by dots.
 * @param mode Operation mode of conversion routine.
 *
 * @return converted string. empty string ('\0') on failure.
 */

UBYTE *Dorfc3490(UBYTE *hostname, enum IDN_MODE mode);

#endif /* _AWEB_IDN_H */
