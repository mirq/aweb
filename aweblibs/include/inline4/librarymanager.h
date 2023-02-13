/* Automatically generated header! Do not edit! */

#ifndef _INLINE_LIBRARYMANAGER_H
#define _INLINE_LIBRARYMANAGER_H

#define LibOpen(___version) __LibOpen_WB(ILibraryManager, ___version)
#define __LibOpen_WB(___base, ___version) \
   (((struct LibraryManagerIFace *)(___base))->LibOpen)((___version))

#define LibClose() __LibClose_WB(ILibraryManager)
#define __LibClose_WB(___base) \
   (((struct LibraryManagerIFace *)(___base))->LibClose)()

#define LibExpunge() __LibExpunge_WB(ILibraryManager)
#define __LibExpunge_WB(___base) \
   (((struct LibraryManagerIFace *)(___base))->LibExpunge)()

#endif /* !_INLINE_LIBRARYMANAGER_H */
