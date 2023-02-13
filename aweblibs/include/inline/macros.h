#include_next <inline/macros.h>

#ifndef LP5A4FP
#define LP5A4FP(offs, rt, name, t1, v1, r1, t2, v2, r2, t3, v3, r3, t4, v4, r4, t5, v5, r5, bt, bn, fpt)   \
({                                                                                                         \
    typedef fpt;                                                                                           \
    LP5A4((offs), rt, name, t1, (v1), r1, t2, (v2), r2, t3, (v3), r3, t4, (v4), r4, t5, (v5), r5, bt, bn); \
})
#endif

#ifndef LP7A4A5
#define LP7A4A5(offs, rt, name, t1, v1, r1, t2, v2, r2, t3, v3, r3, t4, v4, r4, t5, v5, r5, t6, v6, r6, \
                                t7, v7, r7, bt, bn)                                                     \
({                                                                                                      \
    t1 _##name##_v1 = (v1);                                                                             \
    t2 _##name##_v2 = (v2);                                                                             \
    t3 _##name##_v3 = (v3);                                                                             \
    t4 _##name##_v4 = (v4);                                                                             \
    t5 _##name##_v5 = (v5);                                                                             \
    t6 _##name##_v6 = (v6);                                                                             \
    t7 _##name##_v7 = (v7);                                                                             \
    ({                                                                                                  \
        register char * _##name##__bn __asm("a6") = (char *) (bn);                                      \
                                                                                                   \
        typedef rt _##name##_t(char * __asm("a6"), t1 __asm(#r1), t2 __asm(#r2), t3 __asm(#r3), t4 __asm(#r4),        \
            t5 __asm(#r5), t6 __asm(#r6), t7 __asm(#r7));                                             \
   ((_##name##_t *)(_##name##__bn - 336))(_##name##__bn, _##name##_v1, _##name##_v2, _##name##_v3, _##name##_v4,   \
                                         _##name##_v5, _##name##_v6, _##name##_v7);                \
    });                                                                                                 \
})
#endif
/*
        (((rt (*)(char * __asm("a6"), t1 __asm(#r1), t2 __asm(#r2), t3 __asm(#r3), t4 __asm(#r4),        \
            t5 __asm(#r5), t6 __asm(#r6), t7 __asm(#r7))))                                         \
        (_##name##__bn - 336))(_##name##__bn, _##name##_v1, _##name##_v2, _##name##_v3, _##name##_v4,   \
                                         _##name##_v5, _##name##_v6, _##name##_v7);                \
    });                                                                                                 \
*/