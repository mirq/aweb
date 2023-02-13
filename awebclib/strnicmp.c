#if defined(__amigaos4__)

#include <proto/utility.h>

int strnicmp(const char *s1,const char *s2,long len)
{
  return IUtility->Strnicmp((STRPTR)s1,(STRPTR)s2,(LONG)len);
}

#endif
