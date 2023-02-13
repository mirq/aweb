#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
/*#define CATCOMP_BLOCK*/
#define CATCOMP_ARRAY
/* #define CATCOMP_CODE  */
#define __USE_INLINE__
#include "locale.h"
#include <proto/locale.h>


UBYTE *GetString(struct LocaleInfo *li, LONG stringNum)
{
    int i;
    for(i=0;i<(sizeof(CatCompArray)/sizeof(struct CatCompArrayType));i++)
    {
        if ( CatCompArray[i].cca_ID == stringNum )
        {
            return GetCatalogStr ( li->li_Catalog, stringNum, (CONST STRPTR) CatCompArray[i].cca_Str );
        }
    }
    return "";
}
