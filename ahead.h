#ifndef _TANGE_AHEAD_H_
#define _TANGE_AHEAD_H_

#define AR_NONE             (0)
#define AR_FORE             (1)
#define AR_CENT             (2)
#define AR_BACK             (4)
#define AR_BOTH             (5)
#define AR_ALL              (7)

#define AH_NONE             (0)
#define AH_NORMAL           (1)
#define AH_WIRE             (2)
#define AH_ARROW3           (3)
#define AH_ARROW4           (4)
#define AH_ARROW5           (5)
#define AH_ARROW6           (6)
#define AH_WNORMAL          (7)
#define AH_DOUBLE           (9)
#define AH_DIAMOND          (11)
#define AH_CIRCLE           (12)
#define AH_WDIAMOND         (13)
#define AH_WCIRCLE          (14)
#define AH_SHIP             (21)
#define AH_WSHIP            (22)
#define AH_REVNORMAL        (101)
#define AH_REVWIRE          (102)

extern apair_t arrowhead_ial[];

/* some arrow head needs chopping */
#define AH_MUSTCHOP(x) \
    ((x)==AH_NORMAL||(x)==AH_ARROW3||(x)==AH_ARROW5|| \
    (x)==AH_ARROW6||(x)==AH_SHIP)

#endif/*_TANGE_AHEAD_H_*/
