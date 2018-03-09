#ifndef _TANGY_LTYPE_
#define _TANGY_LTYPE_

#define LT_SOLID            (0)
#define LT_DASHED           (1)
#define LT_DOTTED           (2)
#define LT_CHAINED          (4)
#define LT_DOUBLECHAINED    (5)
#define LT_WAVED            (7)
#define LT_ZIGZAG           (8)
#define LT_CIRCLE           (9)
#define LT_WCIRCLE          (10)
#define LT_TRIANGLE         (11)
#define LT_MOUNTAIN         (12)
#if 0
#define LT_CUTTED           (13)
#define LT_ARROWCENTERED    (14)
#endif
#define LT_LMUST            (21)
#define LT_RMUST            (22)
#define LT_DOUBLED          (100)
    /* DBL and DBR are pair. do not use single */
#define LT_DBR              (101)
#define LT_DBL              (102)

extern apair_t linetype_ial[];

#endif/*_TANGY_LTYPE_*/
