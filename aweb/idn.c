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

/** @file idn.c
 * AWeb IDNA related functions.
 */

#include "aweb.h"
#include "awebprotos.h"
#include "idn.h"
#include <proto/dos.h>

/*
 * Punycode (RFC 3492)
 */

/* These functions are taken from punycode.c from RFC 3492 written
 * by Adam M. Costello and were "slightly" adapted to AWeb's specific
 * needs.
 */

/* static declarations */

static ULONG decode_digit(ULONG cp);

static UBYTE encode_digit(ULONG d, int flag);

/*
static UBYTE encode_basic(ULONG bcp, int flag);
*/

/* Other RFC 3490 stuff */

/** Convert buffer area to ACE.
 * The specified buffer area is converted to ACE, i.e. punycode with ACE-prefix
 * ("xn--"). The passed end specification is dynamically changed so that after
 * any conversion *end still contains the offset to the next label delimiter
 * (either '.' or '\0').
 *
 * @param buf pointer to dynamic buffer.
 * @param start start offset from buffer.
 * @param end pointer to end offset of buffer.
 *
 * @return boolean if conversion was successful.
 */

static BOOL Toascii(struct Buffer *buf, ULONG start, ULONG *end);

/** Convert buffer area to plain Latin-1 encoding.
 * The specified buffer area is decoded from ACE encoding to plain Latin-1.
 * If any character outside the Latin-1 codepage appears encoded or any
 * other error occurs, the original buffer content is restored, if the
 * restoring fails, the buffer area is cut. The end offset is dynamically
 * adapted.
 *
 * @param buf Pointer to dynamic buffer.
 * @param start Start offset.
 * @param end Address of end offset variable.
 *
 * @return boolean if conversion was successful.
 */

static BOOL Tolatin(struct Buffer *buf, ULONG start, ULONG *end);

/** Apply a subset of RFC 3491 (NAMEPREP).
 * Apply a subset of the STRINGPREP profile defined in RFC 3491 to the
 * string defined in buf->buffer and with the given area. As of this
 * writing AWeb only supports Latin-1 as character set, so we only apply
 * the necessary operations for this specific subset. These are:
 *  - replace 'ß' with 'ss'
 *  -  0xad with nothing.
 *  - lowercase all alpha characters.
 *
 * @param buf Dynamic buffer carrying the string to normalize.
 * @param start Start offset in buffer.
 * @param end Pointer to end offset in buffer.
 *
 * @return true if the preparation was successful.
 */

static BOOL tiny_nameprep(struct Buffer* buf, ULONG start, ULONG *end);

/** Check for nonascii characters.
 * Checks for nonascii characters in the string range specified
 * by the start and the length.
 *
 * @param start pointer to start of string range.
 * @param len length of the range to check.
 *
 * @return true if at least one character > 0x7f was found.
 */

static BOOL Noascii(UBYTE *start, ULONG len);

/** Encode a buffer area to punycode.
 * Encodes the given buffer area to punycode. The buffer is expanded if
 * necessary and the end offset changed accordingly.
 *
 * @param buf pointer to dynamic buffer.
 * @param start start offset.
 * @param end pointer to end offset.
 *
 * @return Status of conversion.
 */

static enum punycode_status punycode_encode(struct Buffer *buf,
                                     ULONG start,
                                     ULONG *end
                                     );

/** Decode a buffer area from punycode.
 * Decodes the given area from punycode. The buffer is expanded if necessary
 * and the end offset changed accordingly.
 *
 * @param buf pointer to dynamic buffer.
 * @param start start offset.
 * @param end pointer to end offset.
 *
 * @return status of conversion.
 */

static enum punycode_status punycode_decode(struct Buffer *buf,
                                     ULONG start,
                                     ULONG *end
                                     );

/* The original uses those values as parameters for maximal flexibility.
 * We leave it although our purpose is very limited. Job for future
 * cleanup.
 */

enum { base = 36, tmin = 1, tmax = 26, skew = 38, damp = 700,
       initial_bias = 72, initial_n = 0x80, delimiter = 0x2D };

/* decode_digit(cp) returns the numeric value of a basic code */
/* point (for use in representing integers) in the range 0 to */
/* base-1, or base if cp is does not represent a value.       */

static ULONG decode_digit(ULONG cp)
{
  return  cp - 48 < 10 ? cp - 22 :  cp - 65 < 26 ? cp - 65 :
          cp - 97 < 26 ? cp - 97 :  base;
}

/* encode_digit(d,flag) returns the basic code point whose value      */
/* (when used for representing integers) is d, which needs to be in   */
/* the range 0 to base-1.  The lowercase form is used unless flag is  */
/* nonzero, in which case the uppercase form is used.  The behavior   */
/* is undefined if flag is nonzero and digit d has no uppercase form. */

static UBYTE encode_digit(ULONG d, int flag)
{
  return d + 22 + 75 * (d < 26) - ((flag != 0) << 5);
  /*  0..25 map to ASCII a..z or A..Z */
  /* 26..35 map to ASCII 0..9         */
}

/* encode_basic(bcp,flag) forces a basic code point to lowercase */
/* if flag is zero, uppercase if flag is nonzero, and returns    */
/* the resulting code point.  The code point is unchanged if it  */
/* is caseless.  The behavior is undefined if bcp is not a basic */
/* code point.                                                   */

/*
static UBYTE encode_basic(ULONG bcp, int flag)
{
  bcp -= (bcp - 97 < 26) << 5;
  return bcp + ((!flag && (bcp - 65 < 26)) << 5);
}
*/

/*** Bias adaptation function ***/

static ULONG adapt(
  ULONG delta, ULONG numpoints, int firsttime )
{
  ULONG k;

  delta = firsttime ? delta / damp : delta >> 1;
  /* delta >> 1 is a faster way of doing delta / 2 */
  delta += delta / numpoints;

  for (k = 0;  delta > ((base - tmin) * tmax) / 2;  k += base) {
    delta /= base - tmin;
  }

  return k + (base - tmin + 1) * delta / (delta + skew);
}

        
static enum punycode_status punycode_encode(
  struct Buffer *buf,
  ULONG start,
  ULONG *end )
 /*
  ULONG input_length,
  const UBYTE input[],
  const unsigned char case_flags[],
  ULONG *output_length,
  UBYTE output[] )
*/
{
  ULONG n, delta, h, b, out, max_out, bias, m, q, k, t, maxint;
  UBYTE output[64], *input;
  ULONG input_length, j;

  /* first handle the API conversion */
  input_length = *end - start;
  input = buf->buffer + start;
  
  /* Initialize the state: */

  maxint = (ULONG)-1;
  n = initial_n;
  delta = out = 0;
  max_out = 63;
  bias = initial_bias;

  /* Handle the basic code points: */
  for (j = 0;  j < input_length;  ++j) {
    if (basic(input[j])) {
      if (max_out - out < 2) return punycode_big_output;
      output[out++] = input[j]; /* no case_flags for AWeb */
    }
    /* else if (input[j] < n) return punycode_bad_input; */
    /* (not needed for Punycode with unsigned code points) */
  }

  h = b = out;

  /* h is the number of code points that have been handled, b is the  */
  /* number of basic code points, and out is the number of characters */
  /* that have been output.                                           */

  if (b > 0) output[out++] = delimiter;

  /* Main encoding loop: */

  while (h < input_length) {
    /* All non-basic code points < n have been     */
    /* handled already.  Find the next larger one: */

    for (m = maxint, j = 0;  j < input_length;  ++j) {
      /* if (basic(input[j])) continue; */
      /* (not needed for Punycode) */
      if (input[j] >= n && input[j] < m) m = input[j];
    }

    /* Increase delta enough to advance the decoder's    */
    /* <n,i> state to <m,0>, but guard against overflow: */

    if (m - n > (maxint - delta) / (h + 1)) return punycode_overflow;
    delta += (m - n) * (h + 1);
    n = m;

    for (j = 0;  j < input_length;  ++j) {
      /* Punycode does not need to check whether input[j] is basic: */
      if (input[j] < n /* || basic(input[j]) */ ) {
        if (++delta == 0) return punycode_overflow;
      }

      if (input[j] == n) {
        /* Represent delta as a generalized variable-length integer: */

        for (q = delta, k = base;  ;  k += base) {
          if (out >= max_out) return punycode_big_output;
          t = k <= bias /* + tmin */ ? tmin :     /* +tmin not needed */
              k >= bias + tmax ? tmax : k - bias;
          if (q < t) break;
          output[out++] = encode_digit(t + (q - t) % (base - t), 0);
          q = (q - t) / (base - t);
        }

        output[out++] = encode_digit(q, 0); /* no case flags for AWeb */
        bias = adapt(delta, h + 1, h == b);
        delta = 0;
        ++h;
      }
    }

    ++delta, ++n;
  }
  /* backwards API wrapper */
  /* delete input label */
  Deleteinbuffer(buf, start, input_length);
  *end -= input_length;
  /* reinsert the converted label */
  if (!Insertinbuffer(buf, output, out, start)) {
    return punycode_big_output;
  }
  *end += out;
  return punycode_success;
}

static enum punycode_status punycode_decode(
    struct Buffer *buf,
    ULONG start,
    ULONG *end)
/*
  ULONG input_length,
  const char input[],
  ULONG *output_length,
  ULONG output[],
  unsigned char case_flags[] )
*/
{
  ULONG n, out, i, max_out, bias,
                 b, j, in, oldi, w, k, digit, t;
  UBYTE *input, output[64];
  ULONG input_length, maxint;
                  
  /* API wrapper */
  input_length = *end - start;
  input = buf->buffer + start;

  /* Initialize the state: */

  n = initial_n;
  out = i = 0;
  max_out = 63;
  bias = initial_bias;
  maxint = (ULONG)-1;

  /* Handle the basic code points:  Let b be the number of input code */
  /* points before the last delimiter, or 0 if there is none, then    */
  /* copy the first b code points to the output.                      */

  for (b = j = 0;  j < input_length;  ++j) if (delim(input[j])) b = j;
  if (b > max_out) return punycode_big_output;

  for (j = 0;  j < b;  ++j) {
  /* if (case_flags) case_flags[out] = flagged(input[j]); */
    if (!basic(input[j])) return punycode_bad_input;
    output[out++] = input[j];
  }

  /* Main decoding loop:  Start just after the last delimiter if any  */
  /* basic code points were copied; start at the beginning otherwise. */

  for (in = b > 0 ? b + 1 : 0;  in < input_length;  ++out) {

    /* in is the index of the next character to be consumed, and */
    /* out is the number of code points in the output array.     */

    /* Decode a generalized variable-length integer into delta,  */
    /* which gets added to i.  The overflow checking is easier   */
    /* if we increase i as we go, then subtract off its starting */
    /* value at the end to obtain delta.                         */

    for (oldi = i, w = 1, k = base;  ;  k += base) {
      if (in >= input_length) return punycode_bad_input;
      digit = decode_digit(input[in++]);
      if (digit >= base) return punycode_bad_input;
      if (digit > (maxint - i) / w) return punycode_overflow;
      i += digit * w;
      t = k <= bias /* + tmin */ ? tmin :     /* +tmin not needed */
          k >= bias + tmax ? tmax : k - bias;
      if (digit < t) break;
      if (w > maxint / (base - t)) return punycode_overflow;
      w *= (base - t);
    }

    bias = adapt(i - oldi, out + 1, oldi == 0);

    /* i was supposed to wrap around from out+1 to 0,   */
    /* incrementing n each time, so we'll fix that now: */

    if (i / (out + 1) > maxint - n) return punycode_overflow;
    n += i / (out + 1);
    i %= (out + 1);

    /* Insert n at position i of the output: */

    /* not needed for Punycode: */
    /* if (decode_digit(n) <= base) return punycode_invalid_input; */
    if (out >= max_out) return punycode_big_output;

    /* no case flags in AWeb */
//    if (case_flags) {
//      memmove(case_flags + i + 1, case_flags + i, out - i);
//      /* Case of last character determines uppercase flag: */
//      case_flags[i] = flagged(input[in - 1]);
//    }
    /* we have our own API for this */
/*    memmove(output + i + 1, output + i, (out - i) * sizeof *output);
    output[i++] = n; */
    if (!Insertinbuffer(buf, (UBYTE *)&n, 1UL, i++)) {
       *end = start + out;
       return punycode_big_output;
    }
  }

  *end = start + out;
  return punycode_success;
}

static BOOL tiny_nameprep(struct Buffer* buf, ULONG start, ULONG *end)
{
  ULONG bpos;
  UBYTE ch;
  
  bpos = start;
  while(bpos < *end) {
    ch = buf->buffer[bpos];
    if (Isalpha(ch)) {
      if (ch != (UBYTE)'ß') {
        /* RFC 3454, table B.2 */
        /* ConvToLower() would be nice, but C= went belly up... */
        buf->buffer[bpos] |= 0x20;
      } else {
        /* Unicode NFKC for 'ß' */
        if (!Insertinbuffer(buf, "s", 1UL, bpos)) {
          /* This should happen at the start, when no change has happened
           * yet */
          return FALSE;
        }
        bpos++, ++*end;
        buf->buffer[bpos] = 's';
      }
    } else {
      if (ch == 0xad) {
        /* RFC 3454, table B.1 */
        Deleteinbuffer(buf, bpos, 1UL);
        bpos--, --*end;
      }
    }
    bpos++;
  }
  return TRUE;
}

static BOOL Noascii(UBYTE *start, ULONG num)
{
  while (num-- > 0) {
    if (*start > 0x7f) {
      return TRUE;
    }
    start++;
  }
  return FALSE;
}

static BOOL Toascii(struct Buffer *buf, ULONG start, ULONG *end)
{
  BOOL noascii = FALSE;
  enum punycode_status ps;
  
  noascii = Noascii(buf->buffer + start, *end - start);
  if (noascii && !tiny_nameprep(buf, start, end)) {
    return FALSE; /* NAMEPREP failed */
  }
  /* we might have normalized to ASCII-only */
  noascii = Noascii(buf->buffer + start, *end - start);
//printf("after NAMEPREP start %d; end %d\n",start,*end);
  if (noascii) {
    if (!STRNIEQUAL(buf->buffer + start, "xn--", 4)) {
      ps = punycode_encode(buf, start, end);
//    printf("after punycode start %d;  end %d\n",start,*end);
      if (ps != punycode_success) {
//      PutStr("Punycode unsuccessful\n");
        return FALSE;
      }
      if (!Insertinbuffer(buf, "xn--", 4, start)) {
//      PutStr("Insertbuffer() failed\n");
        return FALSE;
      }
      *end += 4;
    }
  }
//printf("pointer difference: %d\n", *end - start);
  return *end - start > 63 ? FALSE : TRUE;
}

static BOOL Tolatin(struct Buffer *buf, ULONG start, ULONG *end)
{
  ULONG cur, et;
  BOOL noascii = FALSE;
  UBYTE *clabel, *dlabel;
  enum punycode_status ps;
  
  for (cur = ++start; cur <*end; cur++) {
    if (buf->buffer[cur] > 0x7f) {
      noascii = TRUE;
    break;
    }
  }
  if (noascii && !tiny_nameprep(buf, start, end)) {
    return TRUE; /* Tounicode() never fails, so does Tolatin() */
  }
  if (STRNIEQUAL(buf->buffer + start, "xn--", 4)) {
    /* copy of encoded label */
    clabel = Dupstr(buf->buffer + start, *end - start);
    if (!clabel) {
      return TRUE; /* out of memory, but who cares */
    }
    Deleteinbuffer(buf, start, 4UL); /* delete prefix */
    *end -= 4;
    ps = punycode_decode(buf, start, end);
    /* copy of decoded label */
    et = *end - start;
    dlabel = Dupstr(buf->buffer + start, et);
    if (dlabel && Toascii(buf, start, end)) {
      if (STRNIEQUAL(buf->buffer + start, dlabel, *end - start)) {
        Deleteinbuffer(buf, start, *end - start);
        if (Insertinbuffer(buf, dlabel, strlen(dlabel), start)) {
          *end = start + et;
          FREE(clabel);
          FREE(dlabel);
          return TRUE; /* this is the exitpoint if all went well */
        }
      }
    }
    Deleteinbuffer(buf, start, *end - start);
    FREE(dlabel);
    if (Insertinbuffer(buf, clabel, strlen(clabel), start)) {
      *end = start + strlen(clabel);
      FREE(clabel);
      return TRUE;
    }
    FREE(clabel);
    /* CANNOT REACH! FIXME! If the program ever arrives here, some task
     * has been eating memory aggressively. The only way around this is to
     * operate on a copy of the original buffer so that the original
     * stays unchanged. For now the buffer is returned shortened.
     */
  }
  return TRUE;
}

UBYTE *Dorfc3490(UBYTE *hostname, enum IDN_MODE mode)
{
  UBYTE *daddr, *rv;
  struct Buffer *buf;
  ULONG sl, el;
  BOOL tret=FALSE;

  /* our copy of the hostname */
  buf = ALLOCSTRUCT(Buffer,1,0);
  if (!Addtobuffer(buf, hostname,-1)) {
    if (buf) {
      FREE(buf);
    }
    return NULL;
  }
//printf("Dorfc3490: %s\n",buf->buffer);
  /* split into labels. to avoid expensive copying we adapt our routines
   * to handle start- and endoffset. Pointers are not possible because
   * the buffer routines could realloc memory */
  sl = 0;
  while (buf->buffer && (daddr = strchr(buf->buffer + sl, '.'))) {
    el = daddr - buf->buffer;
    /* use address of endoffset because it might change during the
     * transformation, our label might get longer or shorter. */
    switch (mode) {
      case IDN_ENCODE:
        tret = Toascii(buf, sl, &el);
//      printf("encoded: %s\n",buf->buffer);
        break;
      case IDN_DECODE:
        tret = Tolatin(buf, sl, &el);
        break;
    }
    if (!tret) {
      /* something failed */
//    PutStr("something went wrong...\n");
      FREE(buf->buffer);
      buf->buffer = NULL;
      break;
    }
    sl = el+1;
  }
  rv = Dupstr(buf->buffer,-1); /* becomes NULL if an error occured above */
  Freebuffer(buf);
  FREE(buf);
  return rv;
}
