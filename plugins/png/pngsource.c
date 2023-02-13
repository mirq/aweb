#if defined(__amigaos4__)
#include "__pngsourceos4.c"
#elif defined(__MORPHOS__)
#include "__pngsourcemos.c"
#else
#include "__pngsource68k.c"
#endif
