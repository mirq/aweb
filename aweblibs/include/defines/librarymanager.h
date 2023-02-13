/* Automatically generated header! Do not edit! */

#ifndef _INLINE_LIBRARYMANAGER_H
#define _INLINE_LIBRARYMANAGER_H

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef LIBRARYMANAGER_BASE_NAME
#define LIBRARYMANAGER_BASE_NAME LibraryManagerBase
#endif /* !LIBRARYMANAGER_BASE_NAME */

#define LibOpen(___version) __LibOpen_WB(LIBRARYMANAGER_BASE_NAME, ___version)
#define __LibOpen_WB(___base, ___version) \
   AROS_LC1(struct Library *, LibOpen, \
   AROS_LCA(ULONG, (___version), D0), \
   struct Library *, (___base), 1, Librarymanager)

#define LibClose() __LibClose_WB(LIBRARYMANAGER_BASE_NAME)
#define __LibClose_WB(___base) \
   AROS_LC0(BPTR, LibClose, \
   struct Library *, (___base), 2, Librarymanager)

#define LibExpunge() __LibExpunge_WB(LIBRARYMANAGER_BASE_NAME)
#define __LibExpunge_WB(___base) \
   AROS_LC0(BPTR, LibExpunge, \
   struct Library *, (___base), 3, Librarymanager)

#endif /* !_INLINE_LIBRARYMANAGER_H */
