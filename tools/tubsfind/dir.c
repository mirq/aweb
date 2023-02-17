#include "dir.h"

#ifdef __AMIGA

#include <stdlib.h>
#include <errno.h>

#include <proto/dos.h>
#include <dos/dos.h>

#include "ioerr2errno.h"

struct _DIR
{
    struct dirent         de;
    struct FileInfoBlock *fib;
    BPTR                  lock;
};

DIR *opendir(const char *name)
{
    BPTR lock = Lock(name, SHARED_LOCK);
    
    if (lock)
    {
        DIR *dir = malloc(sizeof(DIR));
         
        if (dir)
        {
            dir->lock = lock;
            dir->fib  = AllocDosObject(DOS_FIB, TAG_DONE);
            
            if (dir->fib)
            {
                if (Examine(dir->lock, dir->fib))
                {
                    if (dir->fib->fib_DirEntryType >= 0)
                        return dir;
                    else
                        errno = ENOTDIR;
                }
                    
                FreeDosObject(DOS_FIB, dir->fib);
            }
            
            free(dir);
        }
    }
    
    if (errno == 0)
        errno = IoErr2errno(IoErr());
        
    return NULL;
}

void closedir(DIR *dir)
{
    FreeDosObject(DOS_FIB, dir->fib);
    UnLock(dir->lock);
    free(dir);
}

struct dirent *readdir(DIR *dir)
{
    if (!ExNext(dir->lock, dir->fib))
    {
        const int err = IoErr();
        
        if (err != ERROR_NO_MORE_ENTRIES)
            errno = IoErr2errno(err);
        else
            errno = 0;
            
        return NULL;
    }
        
    dir->de.d_name = dir->fib->fib_FileName;
    
    return &dir->de;
}

DIR *cd(DIR *dir)
{
    static DIR *cur = NULL;
           DIR *old;
           
    if (!cur)
        cur = malloc(sizeof(DIR));
    
    old = cur;
    
    if (cur)
    {
        cur->lock = CurrentDir(dir->lock);
        
        cur = dir;
    }
   
    return old;
}

#else

#include <unistd.h>

DIR *cd(DIR *dir)
{
    static DIR *cur = NULL;
           DIR *old;
           
    if (!cur)
        cur = opendir(".");
    
    old = cur;
    
    if (cur)
    {
        int fd = dirfd(dir);
        
        if (fd == -1 || fchdir(fd) == -1)
            return NULL;
        
        cur = dir;
    }
   
    return old;
}

#endif /* __AMIGA */
