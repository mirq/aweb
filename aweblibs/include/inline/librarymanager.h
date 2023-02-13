/* Automatically generated header! Do not edit! */

#ifndef _INLINE_LIBRARYMANAGER_H
#define _INLINE_LIBRARYMANAGER_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef LIBRARYMANAGER_BASE_NAME
#define LIBRARYMANAGER_BASE_NAME LibraryManagerBase
#endif /* !LIBRARYMANAGER_BASE_NAME */

#define LibOpen(___version) __LibOpen_WB(LIBRARYMANAGER_BASE_NAME, ___version)
#define __LibOpen_WB(___base, ___version) \
   LP1(0x6, struct Library *, LibOpen , ULONG, ___version, d0, ,(___base)\
)

#define LibClose() __LibClose_WB(LIBRARYMANAGER_BASE_NAME)
#define __LibClose_WB(___base) \
   LP0(0xc, BPTR, LibClose , ,(___base)\
)

#define LibExpunge() __LibExpunge_WB(LIBRARYMANAGER_BASE_NAME)
#define __LibExpunge_WB(___base) \
   LP0(0x12, BPTR, LibExpunge , ,(___base)\
)

#endif /* !_INLINE_LIBRARYMANAGER_H */
