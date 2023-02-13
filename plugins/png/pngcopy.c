#if defined(__amigaos4__)
#include "__pngcopyos4.c"
#elif defined(__MORPHOS__)
#include "__pngcopymos.c"
#else
#include "__pngcopy68k.c"
#endif
