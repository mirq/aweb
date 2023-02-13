/*
    Copyright © 1995-2004, The AROS Development Team. All rights reserved.
    $Id: ioerr2errno.c,v 1.1 2004/10/22 14:00:56 falemagn Exp $

    Desc: errno internals
    Lang: english
*/

#ifdef __AMIGA 

#include <dos/dos.h>
#include <dos/dosasl.h>

#include <errno.h>

int IoErr2errno (int ioerr)
{
    switch (ioerr)
    {
	case 0:
	    return 0;

	case ERROR_OBJECT_WRONG_TYPE:
	    return EINVAL;

	case ERROR_NO_FREE_STORE:
	    return ENOMEM;

        case ERROR_DEVICE_NOT_MOUNTED:
	case ERROR_OBJECT_NOT_FOUND:
	    return ENOENT;

	case ERROR_OBJECT_EXISTS:
	    return EEXIST;

	case ERROR_BUFFER_OVERFLOW:
	    return ENOBUFS;

	case ERROR_BREAK:
	    return EINTR;

	case ERROR_FILE_NOT_OBJECT:
	case ERROR_NOT_EXECUTABLE:
	    return ENOEXEC;

	case ERROR_OBJECT_IN_USE:
	    return EBUSY;

	case ERROR_DIR_NOT_FOUND:
	    return ENOTDIR;
    }

    return 10000+ioerr;
} /* IoErr2errno */

#endif
