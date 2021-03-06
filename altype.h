#ifndef _TANGY_ALTYPE_
#define _TANGY_ALTYPE_

#define ALT_UNKNOWN         (0)
#define ALT_ARROW           (1)
#define ALT_LINE            (2)
#define ALT_NONE            (3)
#define ALT_PAREN           (10)
#define ALT_BRACKET         (11)
#define ALT_BRACE           (12)

#define ALO_NLINE           (0x01)
#define ALO_BLINEWRAP       (0x02)
#define ALO_BASEPOINT       (0x10)

extern apair_t auxlinetype_ial[];
extern apair_t auxlineopt_ial[];

#endif/*_TANGY_ALTYPE_*/
