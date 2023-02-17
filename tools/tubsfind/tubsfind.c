#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#include "dir.h"

#include <unistd.h>

static char *program_name;

void fatal(const char *msg, int ___errno)
{
    if (!___errno)
        exit(EXIT_SUCCESS);

    if (msg)
        fprintf(stderr, "%s: %s: %s\n", program_name, msg, strerror(___errno));
    else
        fprintf(stderr, "%s: %s\n", program_name, strerror(___errno));

    exit(20);
}

typedef struct _Node
{
    struct _Node *next;
    char         *name;
    size_t        namelen;
} Node;

#define Node(n)     ((Node *)(n))
#define next(n)     (*(typeof(n)*)&Node(n)->next)
#define name(n)     (Node(n)->name)
#define namelen(n)  (Node(n)->namelen)

typedef struct _TNode
{
    Node           node;
    struct _TNode *parent;
    struct _TNode *children;
    time_t         mtime;
    int            outofdate:1;
    int            ignore:1;
    int            is_file:1;
}  TNode;

#define TNode(n)     ((TNode *)(n))
#define parent(n)    (*(typeof(n)*)&TNode(n)->parent)
#define children(n)  (*(typeof(n)*)&TNode(n)->children)
#define mtime(n)     (TNode(n)->mtime)
#define outofdate(n) (errno = 0, (TNode(n)->outofdate ? 1 : __outofdate(n)))
#define ignore(n)    (TNode(n)->ignore)
#define is_file(n)   (TNode(n)->is_file)

/* Given a TNode, return a Node containing the full path name leading to the TNode itself.  */
static Node *fname_1(TNode *n, size_t prevlen)
{
    Node *path;

    if (!n)
    {
        static Node path;

        namelen(&path) = 0;
        name(&path)    = realloc(name(&path), prevlen + 1);
        if (!name(&path))
            fatal(NULL, ENOMEM);

        name(&path)[prevlen] = '\0';

        return &path;
    }

    path = fname_1(parent(n), prevlen + namelen(n) + 1);

    memcpy(name(path) + namelen(path), name(n), namelen(n));
    namelen(path) += namelen(n);

    if (namelen(n) > 0 && name(n)[namelen(n)-1] != ':')
    {
        name(path)[namelen(path)]  = '/';
        namelen(path)             += 1;
    }

    return path;
}

/* Given a TNode, construct the full path name leading to it.  */
static char *fname(TNode *n)
{
    Node *n2 = fname_1(n, 0);

    if (is_file(n)) name(n2)[namelen(n2)-1] = '\0';

    return name(n2);
}

#define dname(n) fname(TNode(n))

Node *new_Node(size_t nodesize, const char *name, size_t namelen)
{
    Node *n;

    if (nodesize < sizeof(Node)) nodesize = sizeof(Node);

    n = malloc(nodesize);
    if (!n)
        fatal(NULL, ENOMEM);

    if (namelen == 0 && name != NULL)
        namelen = strlen(name);

    n->name = malloc(namelen+1);
    if (!n->name)
        fatal(NULL, ENOMEM);

    if (name) memcpy(n->name, name, namelen);
    n->name[namelen] = '\0';

    n->namelen = namelen;
    n->next    = NULL;

    return n;
}

TNode *new_TNode(size_t nodesize, const char *name, size_t namelen, TNode *parent, time_t mtime, int outofdate, int is_file)
{
    TNode *n;

    if (nodesize < sizeof(TNode)) nodesize = sizeof(TNode);

    n = TNode(new_Node(sizeof(TNode), name, namelen));

    n->parent    = parent;
    n->children  = NULL;
    n->mtime     = mtime;
    n->outofdate = outofdate;
    n->ignore    = 0;
    n->is_file   = is_file;

    return n;
}

void dump_tree(TNode *t, int level)
{
    if (t)
    {
        int i;
        TNode *c;

        for (i = 0; i < level; ++i)
            printf("   +");

        puts(name(t));

        for (c = children(t); c; c = next(c))
            dump_tree(c, level+1);
    }
}

void delete_Node(Node **n)
{
    Node *n2 = *n;

    *n = (*n)->next;
    free(n2->name);
    free(n2);
}

void delete_TNode(TNode **n)
{
    TNode **c;

    c = &children(*n);
    while (*c)
    {
        delete_TNode(c);
    }

    delete_Node((Node **)n);
}

Node **find_Node(Node **list, const char *name, size_t namelen)
{
    while (*list)
    {
        if (name && namelen(*list) == namelen && strncmp(name(*list), name, namelen) == 0)
            break;

        list = &next(*list);
    }

    return list;
}

TNode *add_TNode(TNode *tree, const char *name, size_t namelen, time_t mtime, int outofdate, int is_file)
{
    TNode **p = (TNode **)find_Node((Node **)&children(tree), name, namelen);

    if (*p) return *p;

    return *p = new_TNode(0, name, namelen, tree, mtime, outofdate, is_file);
}

int __outofdate(TNode *n)
{
    struct stat sb;

    return n->outofdate = lstat(name(n), &sb) != -1 && (((is_file(n) && S_ISREG(sb.st_mode)) ||
                          S_ISDIR(sb.st_mode)) || ((errno = ENOENT), 0)) &&
                          (sb.st_mtime > mtime(n)) && ((mtime(n) = sb.st_mtime), 1);
}

void scan(DIR *dir, TNode *n)
{
    struct dirent *de;
    struct stat sb;

    while ((de = readdir (dir)))
    {
        #ifndef _D_EXACT_NAMLEN
        #    define _D_EXACT_NAMLEN(de) (strlen((de)->d_name))
        #endif

        size_t len = _D_EXACT_NAMLEN(de);

        if (de->d_name[0] == '.' && (len == 1 || (de->d_name[1] == '.' && len == 2)))
            continue;

        if (lstat(de->d_name, &sb) == -1)
        {
            if (errno != 0 && errno != ENOENT && errno != EPERM)
                fatal(de->d_name, errno);

            continue;
        }

        if (!S_ISREG(sb.st_mode) && !S_ISDIR(sb.st_mode))
            continue;

        if (S_ISREG(sb.st_mode) && (len != 13 || strncmp(de->d_name, "Makefile.tubs", 13) != 0))
            continue;

        add_TNode(n, de->d_name, len, sb.st_mtime, 1, S_ISREG(sb.st_mode));
    }
}



void update(TNode **tree, Node **files, TNode *except)
{
    DIR *dir = NULL, *old;
    TNode **c, *restore_except =  NULL;

    int outofdate;

    c = (TNode **)find_Node((Node **)&except, "%", 1);
    if (*c)
    {
        restore_except = except;
        except         = children(*c);
    }


    while (except)
    {
        c = (TNode **)find_Node((Node **)&except, name(*tree), namelen(*tree));
        if (*c)
        {
            if (!children(*c))
            {
                mtime(*tree) = 0;

                return;
            }

            except = children(*c);
            break;
        }
        else
        {
            except         = restore_except;
            restore_except = NULL;
        }
    }

    if (is_file(*tree))
    {
        if (outofdate(*tree) || errno == 0)
        {
            Node *next = *files;

            *files = new_Node(0, fname(*tree), 0);
            next(*files) = next;
        }
        else
            goto err;

        return;
    }

    dir = opendir(name(*tree));
    if (!dir)
        goto err;

    outofdate = outofdate(*tree);
    if (!outofdate && errno != 0)
        goto err;

    if ((old = cd(dir)) ==  NULL)
        fatal(NULL, errno);

    if (outofdate)
        scan(dir, *tree);

    c = &children(*tree);
    while (*c)
    {
        TNode *oldc = *c;

        if (!ignore(*c))
            update(c, files, except);

        if (*c == oldc)
            c = &next(*c);
    }

    if (cd(old) == NULL)
        fatal(NULL, errno);

    closedir(dir);

    return;

err:
    if (errno == ENOENT)
    {
        fprintf(stderr, "%s: Warning: %s disappeared\n", program_name, dname(*tree));

        delete_TNode(tree);
    }
    else
    {
        ignore(*tree) = 1;
        fprintf(stderr, "%s: Warning: ignoring %s: %s\n", program_name, dname(*tree), strerror(errno));
    }

    if (dir) closedir(dir);

    return;
}

static size_t find_slash_or_null(const char *ptr)
{
    const char *start = ptr;

    while (*ptr && *ptr != '/') ptr++;

    return (size_t)((ptrdiff_t)ptr - (ptrdiff_t)start);
}


void add_path_to_tree(TNode *t, const char *path)
{
    TNode  *parent   = NULL, **l;
    size_t  part_len;

    l = &children(t);

    while ((part_len = find_slash_or_null(path)) != 0)
    {
        l = (TNode **)find_Node((Node **)l, path, part_len);

        if (!*l)
            *l = new_TNode(0, path, part_len, parent, 0, 0, 0);

        parent = *l;
        l = &children(*l);

        if (path[part_len] == '\0')
            break;

        path += part_len + 1;
    }
}

TNode *cache2tree_1(TNode *t, FILE *cache)
{
    size_t namelen;
    time_t mtime;
    char   is_file;
    size_t num_children;

    TNode *n, **c;

    fread(&namelen, sizeof(namelen), 1, cache);
    fread(&mtime, sizeof(mtime), 1, cache);
    fread(&is_file, sizeof(is_file), 1, cache);

    n = new_TNode(sizeof(TNode), NULL, namelen, t, mtime, 0, is_file);

    fread(name(n), namelen, 1, cache);
    fread(&num_children, sizeof(num_children), 1, cache);

    for (c = &children(n); num_children--; c = &next(*c))
        *c = cache2tree_1(n, cache);

    return n;
}

TNode *cache2tree(const char *cachename)
{
    TNode *t;
    FILE *cache;

    if (!cachename)
        return NULL;

    cache = fopen(cachename, "r");

    if  (!cache)
    {
        if (errno != ENOENT)
            fatal(cachename, errno);

        return NULL;
    }

    t = cache2tree_1(NULL, cache);


    fclose(cache);

    return t;
}

void tree2cache_1(TNode *t, FILE *cache)
{
    size_t num_children;

    if (t && !ignore(t))
    {
        TNode *c;

        char is_file = is_file(t);

        fwrite(&namelen(t), sizeof(namelen(t)), 1, cache);
        fwrite(&mtime(t), sizeof(mtime(t)), 1, cache);
        fwrite(&is_file, sizeof(is_file), 1, cache);
        fwrite(name(t), namelen(t), 1, cache);

        num_children = 0;
        for (c = children(t); c; c = next(c))
            ++num_children;
        fwrite(&num_children, sizeof(num_children), 1, cache);

        for (c = children(t); c; c = next(c))
            tree2cache_1(c, cache);
    }
}

void tree2cache(TNode *t, const char *cachename)
{
    FILE *cache;

    if (!cachename)
        return;

    cache = fopen(cachename, "w");

    if  (!cache) fatal(cachename, errno);

    tree2cache_1(t, cache);

    fclose(cache);
}

TNode *new_dirtree(const char *rootname, const char *cachename)
{
    TNode *t = cache2tree(cachename);

    if (t && strcmp(name(t), rootname) != 0)
        delete_TNode(&t);

    if (!t)
        return new_TNode(0, rootname, 0, 0, 0, 0, 0);

    return t;
}

#ifdef __AMIGA
#   define CURDIR ""
#else
#   define CURDIR "."
#endif

int main (int argc, char *argv[])
{
    TNode *root, *except = NULL;
    Node  *files = NULL;
    char *rootname = CURDIR, *cachename = NULL, *outname = NULL;
    int count = 1;

    program_name = argv[0];

    while (count < argc)
    {
        char *opt = argv[count];

        if (opt[0] == '-')
        {
            char *optarg;

            if (opt[1] && opt[2])
                optarg = &opt[2];
            else
                optarg = argv[++count];

            if (optarg == NULL)
                goto badopts;

            switch (opt[1])
            {
                case 'r': rootname     = optarg; break;
                case 'o': outname      = optarg; break;
                case 'c': cachename    = optarg; break;
                case 'e':
                {
                    if (!except)
                        except = new_dirtree(rootname, NULL);

                    add_path_to_tree(except, optarg);
                } break;

                default : goto badopts;
            }
        }
        else
        {
            badopts:

            fprintf(stderr, "Usage: %s [-r rootdir] [-c cachefile] [-o outfile] [-e exceptlist]\n", argv[0]);
            exit(20);
        }

        ++count;
    }

    root = new_dirtree(rootname, cachename);

    update(&root, &files, except);

    {
        FILE *out;

        if (outname)
            out = fopen(outname, "w");
        else
            out = stdout;

        fputs("TUBSFILES := \\\n", out);
        while (files)
        {
            fprintf(out, "%s \\\n", name(files));
            files = next(files);
        };

        if (outname)
            fclose(out);
    }

    tree2cache(root, cachename);

    return 0;
}
