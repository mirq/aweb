#if defined(__amigaos4__)
#include <proto/utility.h>


int stricmp(const char *s1,const char *s2)
{
  return IUtility->Stricmp((STRPTR)s1,(STRPTR)s2);
}

#endif
