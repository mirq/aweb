#ifndef _DIR_H
#define _DIR_H

#ifdef __AMIGA

struct dirent
{
    char *d_name;
};

typedef struct _DIR DIR;

extern DIR *opendir(const char *);
extern struct dirent *readdir(DIR *);
extern void closedir(DIR *);

#else
#   include <dirent.h>
#endif

extern DIR *cd(DIR *);

#endif /* !_DIR_H */
