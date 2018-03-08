/*
 * tangy second trial since 2016 Aug 
 * K.Chinen
 *
 */

#ifndef _GV_H_
#define _GV_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "alist.h"
#include "varray.h"
#include "color.h"

#include "qbb.h"
#include "geom.h"

#ifndef MAX
#define MAX(a,b)    ((a)>(b) ? (a) : (b))
#endif

#if 0
double rf = M_PI/180.0;
int _t_ = 1;
int _p_ = 0;
#endif

extern double rf;
extern int _t_;
extern int _p_;

#define INTRACE (_t_>0)
#define P   if(_p_){printf("PASS %s:%d;%s\n", __FILE__, __LINE__, __func__); fflush(stdout); }
#define E   printf("%s:%d;%s probably ERROR\n", __FILE__, __LINE__, __func__); fflush(stdout);


#define Error   printf("ERROR %s:%s ", __FILE__, __func__);fflush(stdout);printf
#define Warn    printf("WARNING %s:%s ", __FILE__, __func__);fflush(stdout);printf
#define Info    if(_t_>=2)printf("INFO %s:%s ", __FILE__, __func__),fflush(stdout),printf
#define Echo    if(_t_)printf




#define CMD_UNKNOWN     (0)
#define CMD_NOP         (1)
#define CMD_COMMENT     (2)
#define CMD_EXIT        (3)
#define CMD_SPACE       (6)
#define CMD_MOVE        (7)
#define CMD_SHORE       (8)
#define CMD_GOTO        (9)

#define CMD_BOX         (20)
#define CMD_DOTS        (21)
#define CMD_CIRCLE      (31)
#define CMD_ELLIPSE     (32)
#define CMD_POLYGON     (43)
#define CMD_CLINE       (44)
#define CMD_DRUM        (50)
#define CMD_PAPER       (51)
#define CMD_CLOUD       (52)

#define CMD_LINE        (101)
#define CMD_ARROW       (103)
#define CMD_WLINE       (111)
#define CMD_WARROW      (113)
#define CMD_BARROW      (121)
#define CMD_PLINE       (130)

#define CMD_LINK        (300)
#define CMD_SEP         (302)

#define CMD_PING        (401)
#define CMD_PINGPONG    (402)

#define CMD_CURVE       (501)
#define CMD_CURVESELF   (502)

#define CMD_UNIT        (801)
#define CMD_COLOR       (802)

#define CMD_DIR         (1000)
#define CMD_INCDIR      (1001)
#define CMD_DECDIR      (1002)
#define CMD_LTURN       (1003)
#define CMD_RTURN       (1004)
#define CMD_RIGHT       (1011)
#define CMD_DOWN        (1012)
#define CMD_LEFT        (1013)
#define CMD_UP          (1014)

#define CMD_SAVE        (2001)
#define CMD_RESTORE     (2002)
#define CMD_PUSH        (2011)
#define CMD_POP         (2012)
#define CMD_AGAIN       (2013)
#define CMD_BACK        (2021)
#define CMD_HBACK       (2022)

#define CMD_GATHER      (3007)
#define CMD_SCATTER     (3008)

#define CMD_LPAREN      (4001)
#define CMD_RPAREN      (4002)
#define CMD_LBRACKET    (4003)
#define CMD_RBRACKET    (4004)
#define CMD_LBRACE      (4005)
#define CMD_RBRACE      (4006)

#define CMD_DMY1        (9001)
#define CMD_DMY2        (9002)
#define CMD_DMY3        (9003)

#define CMD_ALIAS       (9007)
#define CMD_NOTEFILE    (9009)

#define CMD_CHUNK               (10000)
#define CMD_NAMESPACE           (10001)
#define CMD_OBJ                 (10003)
#define CMD_CHUNKOBJATTR        (10005)
#define CMD_CHUNKCHILDATTR      (10006)

#define CMD_RULER               (20001)

extern apair_t cmd_ial[];
#if 0
apair_t cmd_ial[] = {
    {"unknown",         CMD_UNKNOWN},
    {"exit",            CMD_EXIT},
    {"nop",             CMD_NOP},
    {"comment",         CMD_COMMENT},
    {"unit",            CMD_UNIT},
    {"color",           CMD_COLOR},
    {"move",            CMD_MOVE},
    {"shore",           CMD_SHORE},
    {"goto",            CMD_GOTO},
    {"box",             CMD_BOX},
    {"dots",            CMD_DOTS},
    {"drum",            CMD_DRUM},
    {"paper",           CMD_PAPER},
    {"cloud",           CMD_CLOUD},
#if 0
    {"cylinder",        CMD_DRUM},
#endif
    {"circle",          CMD_CIRCLE},
    {"ellipse",         CMD_ELLIPSE},
    {"polygon",         CMD_POLYGON},
    {"cline",           CMD_CLINE},
    {"line",            CMD_LINE},
    {"arrow",           CMD_ARROW},
    {"wline",           CMD_WLINE},
    {"warrow",          CMD_WARROW},
    {"barrow",          CMD_BARROW},
    {"pline",           CMD_PLINE},
    {"link",            CMD_LINK},
    {"sep",             CMD_SEP},
    {"ping",            CMD_PING},
    {"pingpong",        CMD_PINGPONG},
    {"curve",           CMD_CURVE},
    {"curveself",       CMD_CURVESELF},
    {"dir",             CMD_DIR},
    {"incdir",          CMD_INCDIR},
    {"decdir",          CMD_DECDIR},
    {"lturn",           CMD_LTURN},
    {"rturn",           CMD_RTURN},
    {"right",           CMD_RIGHT},
    {"down",            CMD_DOWN},
    {"left",            CMD_LEFT},
    {"up",              CMD_UP},
    {"save",            CMD_SAVE},
    {"restore",         CMD_RESTORE},
    {"push",            CMD_PUSH},
    {"pop",             CMD_POP},
    {"again",           CMD_AGAIN},
    {"back",            CMD_BACK},
    {"hback",           CMD_HBACK},
#if 0
    {"dmy1",            CMD_DMY1},
    {"dmy2",            CMD_DMY2},
    {"dmy3",            CMD_DMY3},
#endif
    {"lparen",          CMD_LPAREN},
    {"rparen",          CMD_RPAREN},
    {"\"(\"",           CMD_LPAREN},
    {"\")\"",           CMD_RPAREN},
    {"lbracket",        CMD_LBRACKET},
    {"rbracket",        CMD_RBRACKET},
    {"\"[\"",           CMD_LBRACKET},
    {"\"]\"",           CMD_RBRACKET},
    {"lbrace",          CMD_LBRACE},
    {"rbrace",          CMD_RBRACE},
    {"\"{\"",           CMD_LBRACE},
    {"\"}\"",           CMD_RBRACE},
    {"chunk",           CMD_CHUNK},
    {"[",               CMD_CHUNK},
    {".",               CMD_CHUNKOBJATTR},
    {"*",               CMD_CHUNKCHILDATTR},
    {"alias",           CMD_ALIAS},
    {"notefile",        CMD_NOTEFILE},
    {"namespace",       CMD_NAMESPACE},
    {"object",          CMD_OBJ},
    {"ruler",           CMD_RULER},
    {"gather",          CMD_GATHER},
    {"scatter",         CMD_SCATTER},
    {NULL,              -1},
};
#endif


#if 0
#define ISCHUNK(x)  ((x)==CMD_CHUNK||(x)==CMD_FORK||(x)==CMD_BRANCH)
#endif

#define ISCHUNK(x)  ((x)==CMD_CHUNK)
#define ISGLUE(x)   \
    ((x)==CMD_LINE||(x)==CMD_LINK||(x)==CMD_ARROW||\
     (x)==CMD_WLINE||(x)==CMD_WARROW|(x)==CMD_PING||(x)==CMD_PINGPONG|| \
     (x)==CMD_BARROW||(x)==CMD_PLINE \
    )
#define ISATOM(x)   \
    ((x)==CMD_BOX||(x)==CMD_CIRCLE||(x)==CMD_ELLIPSE||(x)==CMD_DRUM|| \
     (x)==CMD_DOTS||(x)==CMD_CLINE||(x)==CMD_PAPER||(x)==CMD_CLOUD|| \
     (x)==CMD_POLYGON|| \
     (x)==CMD_DMY1|| (x)==CMD_DMY2|| (x)==CMD_DMY3|| \
     (x)==CMD_SEP||(x)==CMD_LPAREN||(x)==CMD_RPAREN|| \
     (x)==CMD_LBRACKET||(x)==CMD_RBRACKET|| \
     (x)==CMD_LBRACE||(x)==CMD_RBRACE \
    )
#define HASBODY(x)  \
    (ISGLUE(x)||ISATOM(x)||(x)==CMD_NOP) 
#define MAYEXPAND(x)  \
    ((x)==CMD_SEP||(x)==CMD_LPAREN||(x)==CMD_RPAREN|| \
     (x)==CMD_LBRACKET||(x)==CMD_RBRACKET|| \
     (x)==CMD_LBRACE||(x)==CMD_RBRACE \
    )


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

#if 0
apair_t linetype_ial[] = {
    {"solid",               LT_SOLID},
    {"dashed",              LT_DASHED},
    {"dotted",              LT_DOTTED},
    {"chained",             LT_CHAINED},
    {"doublechained",       LT_DOUBLECHAINED},
    {"doubled",             LT_DOUBLED},
    {"waved",               LT_WAVED},
    {"zigzag",              LT_ZIGZAG},
    {"circle",              LT_CIRCLE},
    {"triangle",            LT_TRIANGLE},
    {"mountain",            LT_MOUNTAIN},
    {"lmust",               LT_LMUST},
    {"rmust",               LT_RMUST},
#if 0
    {"cutted",              LT_CUTTED},
    {"arrowcentered",       LT_ARROWCENTERED},
#endif
    {NULL,                  -1},
};
#endif

#define HT_NONE             (0)
#define HT_SLASHED          (1)
#define HT_BACKSLASHED      (2)
#define HT_HLINED           (3)
#define HT_VLINED           (4)
#define HT_CROSSED          (5)
#define HT_XCROSSED         (6)
#define HT_SOLID            (10)
#define HT_DOTTED           (13)
#define HT_SPARSEDOTTED     (14)
#define HT_VZIGZAG          (15)
#define HT_HZIGZAG          (16)
#define HT_CHECKED          (17)
#define HT_BIGCHECKED       (18)

extern apair_t hatchtype_ial[];
#if 0
apair_t hatchtype_ial[] = {
    {"none",                HT_NONE},
    {"solid",               HT_SOLID},
    {"slashed",             HT_SLASHED},
    {"backslashed",         HT_BACKSLASHED},
    {"hlined",              HT_HLINED},
    {"vlined",              HT_VLINED},
    {"crossed",             HT_CROSSED},
    {"xcrossed",            HT_XCROSSED},
    {"dotted",              HT_DOTTED},
    {"sparsedotted",        HT_SPARSEDOTTED},
    {"vzigzag",             HT_VZIGZAG},
    {"hzigzag",             HT_HZIGZAG},
    {"checked",             HT_CHECKED},
    {"bigchecked",          HT_BIGCHECKED},
    {NULL,                  -1},
};
#endif

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

#if 0
apair_t arrowhead_ial[] = {
    {"none",                AH_NONE},
    {"normal",              AH_NORMAL},
    {"wire",                AH_WIRE},
    {"arrow3",              AH_ARROW3},
    {"arrow4",              AH_ARROW4},
    {"wnormal",             AH_WNORMAL},
    {"double",              AH_DOUBLE},
    {"diamond",             AH_DIAMOND},
    {"circle",              AH_CIRCLE},
    {"ship",                AH_SHIP},
    {"wship",               AH_WSHIP},
    {"wdiamond",            AH_WDIAMOND},
    {"wcircle",             AH_WCIRCLE},
    {"revnormal",           AH_REVNORMAL},
    {"revwire",             AH_REVWIRE},
    {NULL,                  -1},
};
#endif

#define PO_CENTER           (0)
#define PO_NORTH            (1)
#define PO_NORTHEAST        (2)
#define PO_EAST             (3)
#define PO_SOUTHEAST        (4)
#define PO_SOUTH            (5)
#define PO_SOUTHWEST        (6)
#define PO_WEST             (7)
#define PO_NORTHWEST        (8)
#define PO_START            (10)
#define PO_END              (11)

#define PO_KEY              (12)

#define PO_CIL              (13)
#define PO_CIC              (14)
#define PO_CIR              (15)

#define PO_NOL              (21)
#define PO_NOC              (22)
#define PO_NOR              (23)
#define PO_NBL              (24)
#define PO_NBC              (25)
#define PO_NBR              (26)
#define PO_NIL              (27)
#define PO_NIC              (28)
#define PO_NIR              (29)

#define PO_EOL              (31)
#define PO_EOC              (32)
#define PO_EOR              (33)
#define PO_EIL              (34)
#define PO_EIC              (35)
#define PO_EIR              (36)

#define PO_SOL              (41)
#define PO_SOC              (42)
#define PO_SOR              (43)
#define PO_SIL              (44)
#define PO_SIC              (45)
#define PO_SIR              (46)

#define PO_WOL              (51)
#define PO_WOC              (52)
#define PO_WOR              (53)
#define PO_WIL              (54)
#define PO_WIC              (55)
#define PO_WIR              (56)

#define PO_MAX              (57)

extern apair_t pos_ial[];

#if 0
apair_t pos_ial[] = {

    {"c",                   PO_CENTER},
    {"n",                   PO_NORTH},
    {"ne",                  PO_NORTHEAST},
    {"e",                   PO_EAST},
    {"se",                  PO_SOUTHEAST},
    {"s",                   PO_SOUTH},
    {"sw",                  PO_SOUTHWEST},
    {"w",                   PO_WEST},
    {"nw",                  PO_NORTHWEST},

    {"center",              PO_CENTER},
    {"north",               PO_NORTH},
    {"northeast",           PO_NORTHEAST},
    {"east",                PO_EAST},
    {"southeast",           PO_SOUTHEAST},
    {"south",               PO_SOUTH},
    {"southwest",           PO_SOUTHWEST},
    {"west",                PO_WEST},
    {"northwest",           PO_NORTHWEST},

    {"start",               PO_START},
    {"end",                 PO_END},

    {"key",                 PO_KEY},

    {"CIL",                 PO_CIL},
    {"CIC",                 PO_CIC},
    {"CIR",                 PO_CIR},

    {"NOL",                 PO_NOL},
    {"NOC",                 PO_NOC},
    {"NOR",                 PO_NOR},
    {"NBL",                 PO_NBL},
    {"NBC",                 PO_NBC},
    {"NBR",                 PO_NBR},
    {"NIL",                 PO_NIL},
    {"NIC",                 PO_NIC},
    {"NIR",                 PO_NIR},

    {"EOL",                 PO_EOL},
    {"EOC",                 PO_EOC},
    {"EOR",                 PO_EOR},
    {"EIL",                 PO_EIL},
    {"EIC",                 PO_EIC},
    {"EIR",                 PO_EIR},

    {"SOL",                 PO_SOL},
    {"SOC",                 PO_SOC},
    {"SOR",                 PO_SOR},
    {"SIL",                 PO_SIL},
    {"SIC",                 PO_SIC},
    {"SIR",                 PO_SIR},

    {"WOL",                 PO_WOL},
    {"WOC",                 PO_WOC},
    {"WOR",                 PO_WOR},
    {"WIL",                 PO_WIL},
    {"WIC",                 PO_WIC},
    {"WIR",                 PO_WIR},

    {NULL,                  -1},
};
#endif


#define SJ_CENTER           (0)
#define SJ_LEFT             (1)
#define SJ_RIGHT            (2)
#define SJ_FIT              (3)

extern apair_t sj_ial[];
#if 0
apair_t sj_ial[] = {
    {"center",  SJ_CENTER},
    {"left",    SJ_LEFT},
    {"right",   SJ_RIGHT},
    {"fit",     SJ_FIT},
    {NULL,      -1},
};
#endif

#define LS_DIRECT            (0)
#define LS_SQUARE           (10)
#define LS_SQUAREDOT        (11)
#define LS_MAN              (20)
#define LS_ARC              (30)

extern apair_t ls_ial[];
#if 0
apair_t ls_ial[] = {
    {"direct",      LS_DIRECT},
    {"square",      LS_SQUARE},
    {"squaredot",   LS_SQUAREDOT},
    {"man",         LS_MAN},
    {"arc",         LS_ARC},
    {NULL,          -1},
};
#endif


#define LO_NWR              (0)
#define LO_NWD              (1)
#define LO_SWU              (2)
#define LO_SWR              (3)
#define LO_NEL              (4)
#define LO_SEL              (4)

extern apair_t lo_ial[];
#if 0
apair_t lo_ial[] = {
    {"nwd",         LO_NWD},
    {"nwr",         LO_NWR},
    {"swu",         LO_SWU},
    {"swr",         LO_SWU},
    {"nel",         LO_NEL},
    {NULL,          -1},
};
#endif


/*
#define OA_UNKNOWN           (0)
*/
#define OA_LINECOLOR         (1)
#define OA_LINETYPE          (2)
#define OA_LINETHICK         (3)
#define OA_LINEDECOTHICK     (4)
#define OA_LINEDECOPITCH     (5)
#define OA_WLINETHICK        (6)

#define OA_FILLCOLOR        (11)
#define OA_FILLHATCH        (12)
#define OA_TEXTCOLOR        (21)
#define OA_TEXTBGCOLOR      (22)
#define OA_HATCHTHICK       (31)
#define OA_HATCHPITCH       (32)

#define OA_WIDTH            (41)
#define OA_HEIGHT           (42)
#define OA_DEPTH            (43)

#define OA_LINKSTYLE        (51)

#define OA_PEAK             (70)
#define OA_PEAKROTATE       (71)
#define OA_RAD              (75)
#define OA_IMARGIN          (77)
#define OA_GIMARGIN         (78)
#define OA_ROTATE           (79)
#define OA_TEXTROTATE       (80)
#define OA_DECO             (90)

#define OA_AT               (100)
#define OA_WITH             (101)
#define OA_FROM             (102)
#define OA_TO               (103)

#define OA_FOREHEAD         (201)
#define OA_CENTHEAD         (202)
#define OA_BACKHEAD         (204)
#define OA_BOTHHEAD         (205)
#define OA_ALLHEAD          (207)
#define OA_ARROWEVERY       (211)
#define OA_ARROWEDGE        (212)

#define OA_CHOP             (221)
#define OA_FORECHOP         (222)
#define OA_BACKCHOP         (223)
#define OA_BULGE            (230)

#define OA_RIGHT            (301)
#define OA_LEFT             (302)
#define OA_UP               (303)
#define OA_DOWN             (304)
#define OA_THEN             (305)

#define OA_FORWARD          (311)
#define OA_INCDIR           (312)
#define OA_DECDIR           (313)
#define OA_LTURN            (314)
#define OA_RTURN            (315)
#define OA_DIR              (316)
#define OA_ARC              (317)
#define OA_ARCN             (318)
#define OA_JOIN             (321)
#define OA_SKIP             (322)

#define OA_MOVE             (350)
#define OA_RMOVE            (351)
#define OA_LINE             (352)
#define OA_RLINE            (353)
#define OA_CLOSE            (354)

#define OA_PORT             (402)
#define OA_STARBOARD        (403)

#define OA_BGSHAPE          (501)

#define OA_KEEPDIR          (701)
#define OA_NOEXPAND         (703)

#define OA_HOLLOW           (601)

#define OA_MARKBB           (711)
#define OA_MARKPATH         (712)
#define OA_MARKNODE         (713)
#define OA_MARKPITCH        (714)

#define OA_LANEORDER        (800)
#define OA_LANENUM          (801)
#define OA_LANEGAP          (802)
#define OA_LANEGAPV         (803)
#define OA_LANEGAPH         (804)

#define OA_TEXTHEIGHTFACTOR         (901)
#define OA_TEXTDECENTFACTOR         (902)
#define OA_TEXTBGMARGINFACTOR       (903)
#define OA_ARROWSIZEFACTOR          (912)
#define OA_WARROWSIZEFACTOR         (913)
#define OA_LINETHICKFACTOR          (923)
#define OA_WLINETHICKFACTOR         (924)
#define OA_LINEDECOTHICKFACTOR      (925)
#define OA_LINEDECOPITCHFACTOR      (926)
#define OA_HATCHTHICKFACTOR         (931)
#define OA_HATCHPITCHFACTOR         (932)
#define OA_NOTEOSEPFACTOR           (941)
#define OA_NOTEISEPFACTOR           (942)


extern apair_t objattr_ial[];
#if 0
apair_t objattr_ial[] = {

    {"linecolor",           OA_LINECOLOR},
    {"linetype",            OA_LINETYPE},
    {"linethick",           OA_LINETHICK},
    {"linedecothick",       OA_LINEDECOTHICK},
    {"wlinethick",          OA_WLINETHICK},
    {"fillcolor",           OA_FILLCOLOR},
    {"fillhatch",           OA_FILLHATCH},
    {"hatchthick",          OA_HATCHTHICK},
    {"hatchpitch",          OA_HATCHPITCH},
    {"textcolor",           OA_TEXTCOLOR},
    {"textbgcolor",         OA_TEXTBGCOLOR},

    {"linkstyle",           OA_LINKSTYLE},

    {"width",               OA_WIDTH},
    {"height",              OA_HEIGHT},
    {"depth",               OA_DEPTH},

    {"imargin",             OA_IMARGIN},
    {"gimargin",            OA_GIMARGIN},
    {"peak",                OA_PEAK},
    {"peakrotate",          OA_PEAKROTATE},
    {"textrotate",          OA_TEXTROTATE},
    {"rotate",              OA_ROTATE},
    {"rad",                 OA_RAD},
    {"deco",                OA_DECO},

    {"at",                  OA_AT},
    {"with",                OA_WITH},
    {"from",                OA_FROM},
    {"to",                  OA_TO},

    {"right",               OA_RIGHT},
    {"left",                OA_LEFT},
    {"up",                  OA_UP},
    {"down",                OA_DOWN},
    {"then",                OA_THEN},

    {"forward",             OA_FORWARD},
    {"incdir",              OA_INCDIR},
    {"decdir",              OA_DECDIR},
    {"lturn",               OA_LTURN},
    {"rturn",               OA_RTURN},
    {"dir",                 OA_DIR},
    {"arc",                 OA_ARC},
    {"arcn",                OA_ARCN},
    {"join",                OA_JOIN},
    {"skip",                OA_SKIP},

    {"line",                OA_LINE},
    {"move",                OA_MOVE},
    {"rmove",               OA_RMOVE},
    {"rline",               OA_RLINE},
    {"close",               OA_CLOSE},

    {"forehead",            OA_FOREHEAD},
    {"centhead",            OA_CENTHEAD},
    {"backhead",            OA_BACKHEAD},
    {"bothhead",            OA_BOTHHEAD},
    {"arrowevery",          OA_ARROWEVERY},
    {"arrowedge",           OA_ARROWEDGE},

    {"chop",                OA_CHOP},
    {"forechop",            OA_FORECHOP},
    {"backchop",            OA_BACKCHOP},

    {"bulge",               OA_BULGE},

    {"port",                OA_PORT},
    {"larboard",            OA_PORT},
    {"starboard",           OA_STARBOARD},
    {"board",               OA_STARBOARD},

    {"bgshape",             OA_BGSHAPE},

    {"keepdir",             OA_KEEPDIR},
    {"noexpand",            OA_NOEXPAND},

    {"markbb",              OA_MARKBB},
    {"markpath",            OA_MARKPATH},
    {"marknode",            OA_MARKNODE},
    {"markpitch",           OA_MARKPITCH},

    {"hollow",              OA_HOLLOW},
    
    {"laneorder",           OA_LANEORDER},
    {"lanenum",             OA_LANENUM},
    {"lanegap",             OA_LANEGAP},
    {"lanegapv",            OA_LANEGAPV},
    {"lanegaph",            OA_LANEGAPH},

    {"textheightfactor",    OA_TEXTHEIGHTFACTOR},
    {"textdecentfactor",    OA_TEXTDECENTFACTOR},
    {"textbgmarginfactor",  OA_TEXTBGMARGINFACTOR},
    {"arrowsizefactor",     OA_ARROWSIZEFACTOR},
    {"warrowsizefactor",    OA_WARROWSIZEFACTOR},
    {"linethickfactor",     OA_LINETHICKFACTOR},
    {"linedecothickfactor",     OA_LINEDECOTHICKFACTOR},
    {"linedecopitchfactor",     OA_LINEDECOPITCHFACTOR},
    {"wlinethickfactor",    OA_WLINETHICKFACTOR},
    {"hatchthickfactor",    OA_HATCHTHICKFACTOR},
    {"hatchpitchfactor",    OA_HATCHPITCHFACTOR},
    {"noteosepfactor",      OA_NOTEOSEPFACTOR},
    {"noteisepfactor",      OA_NOTEISEPFACTOR},

    {NULL,                  -1},
};
#endif


#ifndef CH_OBJ_LEN
#define CH_OBJ_LEN (100)
#endif

#ifndef OBJUNIT
#define OBJUNIT (10000)
#endif

#if 0
int objunit = OBJUNIT;
#endif
extern int objunit;

extern int epsoutmargin;

#if 0
double arrowsizefactor      = 0.30;
double warrowsizefactor     = 0.60;
double linethickfactor      = 0.02;
double linedecothickfactor  = 0.06;
double linedecopitchfactor  = 0.06;
double wlinethickfactor     = 0.20;
double barrowgapfactor      = 0.25;
double mustsizefactor       = 0.10;

double textheightfactor     = 0.30;
#if 0
double textdecentfactor     = 0.25;
double textbgmarginfactor   = 0.20;
#endif
double textdecentfactor     = 0.20;
double textbgmarginfactor   = 0.10;

double hatchthickfactor     = 0.02;
double hatchpitchfactor     = 0.07;
double marknoderadfactor    = 0.05;
double markbbthickfactor    = 0.02;

double noteosepfactor       = 0.20;
double noteisepfactor       = 0.10;

int draft_mode      = 0;
int skelton_mode    = 0;
int grid_mode       = 0;
int bbox_mode       = 0;
int oidl_mode       = 0;
int namel_mode      = 0;
int ruler_mode      = 0;
int text_mode       = 0;

int def_dir             =   0;
int def_scale           =   1;
int def_color           =   0;
int def_fgcolor         =   0;
int def_bgcolor         =   7;

int def_linethickmode   =   1;
int def_linethick       =   1;
int def_arrowangle      =  30;
int def_arrowsize       =  20;
int def_linedecothick   =   5;
int def_linedecopitch   =   5;
int def_mustsize        =  20;

int def_wlinethick      =   1;
int def_warrowangle     =  60;
int def_warrowsize      =  90;
int def_barrowgap       =  30;

int def_textheight      =  30;

int def_hatchpitch      =   5;
int def_hatchthick      =   1;
int def_marknoderad     =  10;
int def_markbbthick     =  10;

int def_gridpitch       = 1000;
int def_gridrange       =  100;

int def_noteosep        =   5;
int def_noteisep        =   5;
#endif

#if 0
char *def_fontname      = "Times-Roman";
#endif
extern char *def_fontname;
extern int   def_markcolor;


extern double arrowsizefactor;
extern double warrowsizefactor;
extern double linethickfactor;
extern double linedecothickfactor;
extern double linedecopitchfactor;
extern double wlinethickfactor;
extern double barrowgapfactor;
extern double mustsizefactor;
extern double textheightfactor;
extern double textdecentfactor;
extern double textbgmarginfactor;
extern double textdecentfactor;
extern double textbgmarginfactor;
extern double hatchthickfactor;
extern double hatchpitchfactor;
extern double marknoderadfactor;
extern double markbbthickfactor;
extern double noteosepfactor;
extern double noteisepfactor;

extern int draft_mode;
extern int skelton_mode;
extern int grid_mode;
extern int bbox_mode;
extern int oidl_mode;
extern int namel_mode;
extern int ruler_mode;
extern int text_mode;

extern int def_dir;
extern int def_scale;
extern int def_color;
extern int def_fgcolor;
extern int def_bgcolor;
extern int def_linethickmode;
extern int def_linethick;
extern int def_arrowangle;
extern int def_arrowsize;
extern int def_linedecothick;
extern int def_linedecopitch;
extern int def_mustsize;
extern int def_wlinethick;
extern int def_warrowangle;
extern int def_warrowsize;
extern int def_barrowgap;
extern int def_textheight;
extern int def_hatchpitch;
extern int def_hatchthick;
extern int def_marknoderad;
extern int def_markbbthick;
extern int def_gridpitch;
extern int def_gridrange;
extern int def_noteosep;
extern int def_noteisep;
extern char *def_fontname;

#if 0
pallet_t *pallet = NULL;
#endif
extern pallet_t *pallet;


#ifndef MAXBRANCH
#define MAXBRANCH   (8)
#endif

typedef struct {
    char *ssval;
    int   ssopt;
} sstr;

typedef struct {
    int   cmd;
    char *val;
} segop;

#define NUL_COORD   (0)
#define REL_COORD   (1)
#define ABS_COORD   (2)
#define COORD_FROM  (1)
#define COORD_TO    (2)

typedef struct {
    int ptype;
    int jtype;
    int coordtype;
    int ftflag;
    int x1, y1, x2, y2;
    int rad;
    int ang;
    int ltype;
    int lthick;
    int lcolor;
    int ahpart;
    int ahftype;
    int ahctype;
    int ahbtype;
} seg;

struct obattr {
    int   outlinecolor;
    int   outlinetype;
    int   outlinethick;
    int   outlinethickmode;

    int   wlinethick;
    int   forechop;
    int   backchop;
    int   bulge;

    int   arrowheadpart;
    int   arrowforeheadtype;
    int   arrowcentheadtype;
    int   arrowbackheadtype;

    int   fillcolor;
    int   fillhatch;
    int   filllevel;

    int   textcolor;
    int   textbgcolor;

    int   hatchthick;
    int   hatchpitch;

    int   imargin;
    int   gimargin;

    int   polypeak;
    int   polyrotate;
    int   rotateval;
    int   rad;

    int   sepcurdir;
    int   sepx1, sepy1, sepx2, sepy2;

    int   gsepx1, gsepy1, gsepx2, gsepy2;

    char  *aat;
    char  *awith;
    char  *afrom;
    char  *ato;

    int    hasfrom;
    int    hasto;

#if 0
    void  *forkfocus;
    int    forkhasbranch;
    void  *forkbranchmem[MAXBRANCH];    /* XXX */
    int    forkhasend;
    void  *forkbranchbody;
#endif

    int   originalshape;

    varray_t *ssar;
    int   textrotate;

    char  *portstr;
    char  *boardstr;

    varray_t *segopar;
    varray_t *segar;
    varray_t *seghar;

    int    iarg1;
    int    iarg2;
    char  *carg1;
    char  *carg2;

    char  *deco;

    char  *bgshape;

    void  *linkfore;
    void  *linkback;
    int    linkstyle;

    int    arrowevery;
    int    keepdir;
    int    noexpand;

    int    markbb;
    int    markpath;
    int    marknode;
    int    markpitch;
    int    hollow;

#if 1
    int    laneorder;
    int    lanenum;
    int    lanegapv;
    int    lanegaph;
#endif

#if 1
    char   *note[PO_MAX];
#endif
}; 

typedef struct _ch {
    char name[BUFSIZ];

    /* registers */
    int  dir;
    int  thickmode;
    int  thick;
    int  scale;
    int  x, y;
#if 0
    int  lanenum;
#endif

    /* members */
    struct _ch *root;
    struct _ch *parent;
    struct _ns *qns;

    struct _ob *qob;

    int  nch;
    struct obattr*  ch[CH_OBJ_LEN];
} ch;


typedef struct _ob {
    int  oid;
    int  type;
    int  ignore;
    int  draft;

    int  pst;

    int  layer;

    ch  *behas;

                        /***** LOGICAL *****/
    int  sx, sy;        /* area start */
    int  ex, ey;        /* area end */
    int  ox, oy;        /* offset or pen start */
#if 0
    int  jx, jy;        /* pen start; jo point, sometime be used to draw starting point */
#endif
    int  fx, fy;        /* pen end; final point, sometime be used to draw next object */

    int  x, y;          /* center */
    int  lx, rx;        /* BB lx, rx left right  */
    int  ty, by;        /* BB ty, by top  bottom */
    int  rotate;        /* rotate */
    int  wd, ht;        /* width height */

    int  mr;            /* margin radius */

    qbb_t localbb;

    int  sizesolved;
    int  drawed;
    int  finalized;

    int  fixed;
    int  hasrel;
    int  floated;

                        /***** PHYSICAL *****/
    int  gx, gy;
    int  gsx, gsy;
    int  gex, gey;
    int  glx, grx;
    int  gby, gty;

    qbb_t globalbb;

    ch  vch;
    struct obattr vob;
} ob;

#define cch vch
#define cob vob

#define csx sx
#define csy sy
#define cex ex
#define cey ey
#define cmr mr
#define cox ox
#define coy oy
#define cjx jx
#define cjy jy
#define cfx fx
#define cfy fy

#define cx  x
#define cy  y
#define clx lx
#define crx rx
#define cty ty
#define cby by

#define cht ht
#define cwd wd

#define cfixed  fixed
#define chasrel hasrel
#define cfloated floated
#define cgx     gx
#define cgy     gy
#define cgsx    gsx
#define cgsy    gsy
#define cgex    gex
#define cgey    gey
#define cglx    glx
#define cgby    gby
#define cgrx    grx
#define cgty    gty

#define cafrom  vob.afrom
#define cato    vob.ato
#define cawith  vob.awith
#define caat    vob.aat

#define cimargin     vob.imargin
#define cgimargin    vob.gimargin

#define cnoexpand    vob.noexpand


#ifndef NS_OBJ_LEN
#define NS_OBJ_LEN (100)
#endif

typedef struct _ns {
    int  type;
    char name[BUFSIZ];

    /* members */
    struct _ns *root;
    struct _ns *parent;
    int         nch;
    char       *chn[NS_OBJ_LEN]; /* name */
    int         cht[NS_OBJ_LEN]; /* type */
    ob         *chv[NS_OBJ_LEN]; /* value object */
    struct _ns *chc[NS_OBJ_LEN]; /* child ns */
} ns;


#define SEG_SEP         ','
#define SEG_SEPS        ","

extern char debuglog[];


#endif /* _GV_H_ */


