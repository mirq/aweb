#ifndef _AWEBJS_REGEXP_H
#define _AWEBJS_REGEXP_H

#include "../regexp/pcre.h"


/* Internal object value */
struct Regexp
{
    UBYTE *pattern;
    UWORD flags;
    pcre  *compiled;
    int   lastIndex;
};


#define REF_GLOBAL 0x0001
#define REF_MULTI  0x0002
#define REF_NOCASE 0x0004

#endif
