/**********************************************************************
 *
 * This file is part of the AWeb-II distribution
 *
 * Copyright (C) 2002 Yvon Rozijn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the AWeb Public License as included in this
 * distribution.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * AWeb Public License for more details.
 *
 **********************************************************************/

// easy lists

#ifndef EZLISTS_H
#define EZLISTS_H

#include <exec/lists.h>

#define LIST(name) \
struct\
{ struct name *first;\
  struct name *tail;\
  struct name *last;\
}

#define NODE(name) \
struct name *next;\
struct name *prev

#define FULLNODE(name) \
struct name *next;\
struct name *prev;\
unsigned char ln_Type;\
char ln_Pri;\
char *ln_Name

/* We're going to define a bunch of macros ourselves, so undef them all to avoid clashes with
   the ones the system header might have defined already.  */
#undef ADDHEAD
#undef ADDTAIL
#undef REMOVE
#undef REMHEAD
#undef REMTAIL
#undef NEWLIST
#undef INSERT
#undef ISEMPTY
#undef ENQUEUE
#ifndef __amigaos4__
#undef IsListEmpty
#endif
/* The following macros are taken from the AROS' <exec/lists.h> header file. */

#ifndef __amigaos4__

#define IsListEmpty(l) \
        ( (((struct List *)l)->lh_TailPred) == (struct Node *)(l) )

#define NEWLIST(_l)                                     \
do                                                      \
{                                                       \
    struct List *__aros_list_tmp = (struct List *)(_l), \
                *l = __aros_list_tmp;                   \
                                                        \
    l->lh_TailPred = (struct Node *)l;                  \
    l->lh_Tail     = 0;                                 \
    l->lh_Head     = (struct Node *)&l->lh_Tail;        \
} while (0)

#define ADDHEAD(_l,_n)                                  \
do                                                      \
{                                                       \
    struct Node *__aros_node_tmp = (struct Node *)(_n), \
                *n = __aros_node_tmp;                   \
    struct List *__aros_list_tmp = (struct List *)(_l), \
                *l = __aros_list_tmp;                   \
                                                        \
    n->ln_Succ          = l->lh_Head;                   \
    n->ln_Pred          = (struct Node *)&l->lh_Head;   \
    l->lh_Head->ln_Pred = n;                            \
    l->lh_Head          = n;                            \
} while (0)

#define ADDTAIL(_l,_n)                                    \
do                                                        \
{                                                         \
    struct Node *__aros_node_tmp = (struct Node *)(_n),   \
                *n = __aros_node_tmp;                     \
    struct List *__aros_list_tmp = (struct List *)(_l),   \
                *l = __aros_list_tmp;                     \
                                                          \
    n->ln_Succ              = (struct Node *)&l->lh_Tail; \
    n->ln_Pred              = l->lh_TailPred;             \
    l->lh_TailPred->ln_Succ = n;                          \
    l->lh_TailPred          = n;                          \
} while (0)

#define REMOVE(_n)                                      \
({                                                      \
    struct Node *__aros_node_tmp = (struct Node *)(_n), \
                *n = __aros_node_tmp;                   \
                                                        \
    n->ln_Pred->ln_Succ = n->ln_Succ;                   \
    n->ln_Succ->ln_Pred = n->ln_Pred;                   \
                                                        \
    n;                                                  \
})

#define GetHead(_l)                                     \
({                                                      \
    struct List *__aros_list_tmp = (struct List *)(_l), \
                *l = __aros_list_tmp;                   \
                                                        \
   l->lh_Head->ln_Succ ? l->lh_Head : (struct Node *)0; \
})

#define GetTail(_l)                                              \
({                                                               \
    struct List *__aros_list_tmp = (struct List *)(_l),          \
                *l = __aros_list_tmp;                            \
                                                                 \
    l->lh_TailPred->ln_Pred ? l->lh_TailPred : (struct Node *)0; \
})

#define GetSucc(_n)                                      \
({                                                       \
    struct Node *__aros_node_tmp = (struct Node *)(_n),  \
                *n = __aros_node_tmp;                    \
                                                         \
    n->ln_Succ->ln_Succ ? n->ln_Succ : (struct Node *)0; \
})

#define GetPred(_n)                                      \
({                                                       \
    struct Node *__aros_node_tmp = (struct Node *)(_n),  \
                *n = __aros_node_tmp;                    \
                                                         \
    n->ln_Pred->ln_Pred ? n->ln_Pred : (struct Node *)0; \
})

#define REMHEAD(_l)                                               \
({                                                                \
    struct List *__aros_list_tmp = (struct List *)(_l),           \
                *l = __aros_list_tmp;                             \
                                                                  \
     l->lh_Head->ln_Succ ? REMOVE(l->lh_Head) : (struct Node *)0; \
})

#define REMTAIL(_l)                                                      \
({                                                                       \
    struct List *__aros_list_tmp = (struct List *)(_l),                  \
                *l = __aros_list_tmp;                                    \
                                                                         \
    l->lh_TailPred->ln_Pred ? REMOVE(l->lh_TailPred) : (struct Node *)0; \
})

#define ForeachNode(l,n)                       \
for                                            \
(                                              \
    n=(void *)(((struct List *)(l))->lh_Head); \
    ((struct Node *)(n))->ln_Succ;             \
    n=(void *)(((struct Node *)(n))->ln_Succ)  \
)

#define ForeachNodeSafe(l,n,n2)                 \
for                                             \
(                                               \
    n=(void *)(((struct List *)(l))->lh_Head);  \
    (n2=(void *)((struct Node *)(n))->ln_Succ); \
    n=(void *)n2                                \
)

#define SetNodeName(node,name)   \
    (((struct Node *)(node))->ln_Name = (char *)(name))
#define GetNodeName(node)        \
    (((struct Node *)(node))->ln_Name)

#define ListLength(list,count)     \
do {                               \
    struct Node * __n;             \
    count = 0;                     \
    ForeachNode (list,__n) count ++; \
} while (0)

#define INSERT(l,n,a) Insert((struct List *)(l),(struct Node *)(n),(struct Node *)(a))
#define ISEMPTY(l)    IsListEmpty( (struct List *) l)
#define ENQUEUE(l,n)  Enqueue((struct List *)(l),(struct Node *)(n))

#else /* __amigaos4__ */


#define ADDHEAD(l,n) AddHead((struct List *)(l),(struct Node *)(n))
#define ADDTAIL(l,n) AddTail((struct List *)(l),(struct Node *)(n))
#define REMOVE(n)  Remove((struct Node *)(n))
#define REMHEAD(l) (void *)RemHead((struct List *)(l))
#define REMTAIL(l) (void *)RemTail((struct List *)(l))
#define NEWLIST(l) NewList((struct List *)(l))
#define INSERT(l,n,a) Insert((struct List *)(l),(struct Node *)(n),(struct Node *)(a))
#define ISEMPTY(l) IsListEmpty( (struct List *) l)


/* only use full nodes for this macro!! */
#define ENQUEUE(l,n) Enqueue((struct List *)(l),(struct Node *)(n))

#endif /* __amigaos4__ */

#endif
