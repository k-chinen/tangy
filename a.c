/*
 * tangy second trial since 2016 Aug 
 * K.Chinen
 *
 */
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

#ifndef MAX
#define MAX(a,b)    ((a)>(b) ? (a) : (b))
#endif

double rf = M_PI/180.0;

int _t_ = 1;
int _p_ = 0;
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

#define PO_NOL              (021)
#define PO_NOC              (022)
#define PO_NOR              (023)
#define PO_NIL              (024)
#define PO_NIC              (025)
#define PO_NIR              (026)

#define PO_EOL              (031)
#define PO_EOC              (032)
#define PO_EOR              (033)
#define PO_EIL              (034)
#define PO_EIC              (035)
#define PO_EIR              (036)

#define PO_SOL              (041)
#define PO_SOC              (042)
#define PO_SOR              (043)
#define PO_SIL              (044)
#define PO_SIC              (045)
#define PO_SIR              (046)

#define PO_WOL              (051)
#define PO_WOC              (052)
#define PO_WOR              (053)
#define PO_WIL              (054)
#define PO_WIC              (055)
#define PO_WIR              (056)

#define PO_MAX              (057)

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


#define SJ_CENTER           (0)
#define SJ_LEFT             (1)
#define SJ_RIGHT            (2)
#define SJ_FIT              (3)

apair_t sj_ial[] = {
    {"center",  SJ_CENTER},
    {"left",    SJ_LEFT},
    {"right",   SJ_RIGHT},
    {"fit",     SJ_FIT},
    {NULL,      -1},
};

#define LS_DIRECT            (0)
#define LS_SQUARE           (10)
#define LS_SQUAREDOT        (11)
#define LS_MAN              (20)
#define LS_ARC              (30)

apair_t ls_ial[] = {
    {"direct",      LS_DIRECT},
    {"square",      LS_SQUARE},
    {"squaredot",   LS_SQUAREDOT},
    {"man",         LS_MAN},
    {"arc",         LS_ARC},
    {NULL,          -1},
};


#define LO_NWR              (0)
#define LO_NWD              (1)
#define LO_SWU              (2)
#define LO_SWR              (3)
#define LO_NEL              (4)
#define LO_SEL              (4)

apair_t lo_ial[] = {
    {"nwd",         LO_NWD},
    {"nwr",         LO_NWR},
    {"swu",         LO_SWU},
    {"swr",         LO_SWU},
    {"nel",         LO_NEL},
    {NULL,          -1},
};


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


#ifndef CH_OBJ_LEN
#define CH_OBJ_LEN (100)
#endif

#ifndef OBJUNIT
#define OBJUNIT (10000)
#endif

int objunit = OBJUNIT;

extern int epsoutmargin;

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

#if 0
char *def_fontname      = "Times-Roman";
#endif
extern char *def_fontname;
extern int   def_markcolor;


pallet_t *pallet = NULL;


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

char*
skipwhite(char *p)
{
    while(*p==' ' ||*p=='\t') {
        p++;
    }
    return p;
}

char*
chomp(char *line)
{
    char *p;

    if(line==NULL) {
        return NULL;
    }

    p = line;
    while(*p) {
        if(*p=='\r' && *(p+1)=='\n' && *(p+2)=='\0') {
            *p = '\0';
            break;
        }
        if(*p=='\n' && *(p+1)=='\0') {
            *p = '\0';
            break;
        }
        p++;
    }

    return line;
}

int
dellastcharif(char *src, int xch)
{
    char *p, *q;

    q = NULL;
    p = src;
    while(*p) {
        q = p;
        p++;
    }

#if 0
    printf("%s: ", __func__);
    printf(" b src |%s| -> ", src);
#endif
    if(q && *q==xch) *q='\0';
#if 0
    printf(" a src |%s|\n", src);
#endif
    return 0;
}

/* double quoted string */
char*
draw_wordDQ(char *src, char *dst, int wlen)
{
    register char *p, *q;
    int c;

    p = src;
    q = dst;
    c = 0;
    if(*p!='"') {
        return NULL;
    }
    p++;
    while(*p&&c<wlen&&(*p!='"')) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    if(*p=='"') {
        p++;
    }

    return p;
}


char*
draw_wordW(char *src, char *dst, int wlen)
{
    register char *p, *q;
    int c;

    p = src;
    q = dst;
    c = 0;
    while(*p&&c<wlen&&(*p!=' ' && *p!='\t' && *p!='\n')) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    while(*p==' '||*p=='\t'|| *p=='\n') {
        p++;
    }

    return p;
}

char*
draw_word(char *src, char *dst, int wlen, int sep)
{
    register char *p, *q;
    int c;

    p = src;
    q = dst;
    c = 0;
    while(*p&&c<wlen&&(unsigned char)*p!=(unsigned char)sep) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    if((unsigned char)*p==(unsigned char)sep) {
        p++;
    }

    return p;
}

int
confirm_objattr(struct obattr xoa) 
{
    if(xoa.fillhatch==HT_SOLID) {
        if(xoa.fillcolor<0) {
            xoa.fillcolor = def_color;
        }
    }
    if(xoa.fillcolor>=0) {
        if(xoa.textbgcolor<0) {
            xoa.textbgcolor = def_bgcolor;
        }
    }
            xoa.textbgcolor = def_bgcolor;

    return 0;
}

int
confirm_attr(ob *xo)
{
    struct obattr *xoa;
    xoa = &xo->vob;

#if 0
    if(xoa->ato && xoa->afrom) {
        xo->floated = 1;
    }
#endif
#if 1
    if(xoa->hasto && xoa->hasfrom) {
        xo->floated = 1;
    }
#endif

    return 0;
}

int
dump_objattr(struct obattr xo) 
{
    printf("  outlinecolor     %d\n", xo.outlinecolor);
    printf("  outlinetype      %d\n", xo.outlinetype);
    printf("  outlinethick     %d\n", xo.outlinethick);
#if 0
    printf("  outlinethickmode %d\n", xo.outlinethickmode);
#endif
    printf("  fillcolor        %d\n", xo.fillcolor);
    printf("  fillhatch        %d\n", xo.fillhatch);
    printf("    hatchthick     %d\n", xo.hatchthick);
    printf("    hatchpitch     %d\n", xo.hatchpitch);
#if 0
    printf("  filllevel        %d\n", xo.filllevel);
    printf("  imargin          %d\n", xo.imargin);
#endif
    printf("  polypeak         %d\n", xo.polypeak);
    printf("  rotateval        %d\n", xo.rotateval);
    printf("  rad              %d\n", xo.rad);
    return 0;
}

int
cha_fprint(FILE *fp, char *msg, ch *tg)
{
    fprintf(fp, "%s dir %d thick(%d) %d scale %d x,y %d,%d\n",
        msg, tg->dir, tg->thickmode, tg->thick, tg->scale, tg->x, tg->y);
    fflush(fp);
    return 0;
}

int
cha_reset(ch *xch)
{
    if(!xch) {
        printf("ERROR no chunk\n");
        return -1;
    }
    xch->dir        = def_dir;
    xch->thickmode  = def_linethickmode;
    xch->thick      = def_linethick;
    xch->scale      = def_scale;
    xch->x          = 0;
    xch->y          = 0;

    return 0;
}

int
cha_copy(ch *dst, ch *src)
{
    dst->dir        = src->dir;
    dst->thickmode  = src->thickmode;
    dst->thick      = src->thick;
    dst->scale      = src->scale;
    dst->x          = src->x;
    dst->y          = src->y;

#if 0
    cha_fprint(stdout, "src", src);
    cha_fprint(stdout, "dst", dst);
#endif

    return 0;
}



int dumptabstop=2;
int dumplabel=26;

int
_ns_dump(ns* s, int w)
{
    int i;
    int u;
#define W for(u=0;u<w;u++) printf(" ");
    if(!s) {
        return -1;
    }

    W;
    printf("namespace %p name '%s' root %p parent %p num %d\n",
        s, s->name, s->root, s->parent, s->nch);
    for(i=0;i<s->nch;i++) {
        W;
        printf("%3d: '%s' type %d ", i, s->chn[i], s->cht[i]);
        if(s->cht[i]==CMD_NAMESPACE) {
            W;
            printf("namespace object %p oid %d\n", s->chv[i], s->chv[i]->oid);
            _ns_dump(s->chc[i], w+dumptabstop);
        }
        else {
            printf("object %p oid %d\n", s->chv[i], s->chv[i]->oid);
        }
    }

    return 0;
}

int
ns_dump(ns* s)
{
    int r;
    printf("***\n");
    r = _ns_dump(s, 0);
    printf("---\n");
    return r;
}

int
_ob_dump(ob* s, int w)
{
    int i;
    int u;

#define W for(u=0;u<w;u++) printf(" ");


    printf("%3d:%c ", s->oid, ISCHUNK(s->type)? 'C': '_');

    W;
    printf("%s ", rassoc(cmd_ial, s->type));

    if(ISCHUNK(s->type)) 
    {
        if(s->cch.name[0]) {
            printf("%p name '%s' root %p parent %p num %d\n",
                s, s->cch.name,
                s->cch.root, s->cch.parent, s->cch.nch);
        }
        else {
            printf("%p root %p parent %p num %d\n",
                s, 
                s->cch.root, s->cch.parent, s->cch.nch);
        }
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_dump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }
    else {
        printf("%p %d type %d\n", s, s->ignore, s->type);
    }

    fflush(stdout);
    return 0;
#undef W
}

int
ob_dump(ob* s)
{
    int r;
    printf("=== DUMP\n");
    r = _ob_dump(s, 0);
    printf("---\n");
    return r;
}



int
_ob_gdump(ob* s, int w)
{
    int i;
    int u;
    char tm[BUFSIZ];
    int rest;

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    strcpy(tm, rassoc(cmd_ial, s->type));

    printf("%3d: ", s->oid);

    W;
    printf("%-*s %6d,%-6d %6dx%-6d %6d,%-6d %6d,%-6d %6d,%-6d\n",
        rest,
        tm,
        s->ox, s->oy, s->wd, s->ht,
        s->x, s->y, s->sx, s->sy, s->ex, s->ey);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_gdump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }


    fflush(stdout);
    return 0;
#undef W
}

int
ob_gdump(ob* s)
{
    int r;
    printf("=== GDUMP\n");
    printf("oid: type                          oxy           wxh           xy            sxy           exy\n");

    r = _ob_gdump(s, 0);
    printf("---\n");
    return r;
}





int
_ob_adump(ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    printf("%3d:", s->oid);
    W;
    printf("%-*s %6d,%-6d %6dx%-6d %d %6d,%-6d %6d,%-6d %6d,%-6d\n",
        rest, tm,
        s->ox, s->oy, s->wd, s->ht,
        s->fixed, s->gx, s->gy, s->gsx, s->gsy, s->gex, s->gey);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_adump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    fflush(stdout);
    return 0;
#undef W
}

int
ob_adump(ob* s)
{
    int r;
    printf("=== ADUMP\n");
    printf(
"oid:type                         ox,oy          wxh      f     gx,y        gsx,sy         gex,ey\n");


    r = _ob_adump(s, 0);
    printf("---\n");
    return r;
}





int
_ob_bldump(ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    printf("%3d: ", s->oid);
    W;
    printf("%-*s %d %d: %6d,%-6d : %6d %6d %6d %6d\n",
        rest, tm,
        s->hasrel, s->fixed,
        s->x, s->y, s->lx, s->by, s->rx, s->ty);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_bldump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    fflush(stdout);
    return 0;
#undef W
}

int
_ob_bgdump(ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    printf("%3d: ", s->oid);
    W;
    printf("%-*s %d %d: %6d,%-6d : %6d %6d %6d %6d\n",
        rest, tm,
        s->hasrel, s->fixed,
        s->gx, s->gy, s->glx, s->gby, s->grx, s->gty);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_bgdump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    fflush(stdout);
    return 0;
#undef W
}


int
_ob_cndump(ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    printf("%3d: ", s->oid);
    W;
#if 0
    printf("%-*s %d %d: %5d,%-5d : %5d %5d %5d %5d\n",
        rest, tm,
        s->hasrel, s->fixed,
        s->gx, s->gy, s->glx, s->gby, s->grx, s->gty);
#endif
    printf("%-*s %p\n", rest, tm, s->cch.qns);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_cndump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    fflush(stdout);
    return 0;
#undef W
}

int
ob_bldump(ob* s)
{
    int r;
    printf("=== BLDUMP\n");
    printf(
"oid: type                     r f:      x,y      :   lx     by     rx     ty\n");

    r = _ob_bldump(s, 0);
    printf("---\n");
    return r;
}

int
ob_bgdump(ob* s)
{
    int r;
    printf("=== BGDUMP\n");
    printf(
"oid: type                     r f:     gx,gy     :   glx    gby    grx    gty\n");

    r = _ob_bgdump(s, 0);
    printf("---\n");
    return r;
}


int
ob_cndump(ob*s )
{
    int r;
    printf("=== CNDUMP\n");
    printf(
"oid: type                     ns\n");

    r = _ob_cndump(s, 0);
    printf("---\n");
    return r;
}


int
_printobjlist(FILE *fp, char *pre, ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) fprintf(fp, " ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    fprintf(fp, "%s%3d: ", pre, s->oid);
    W;
#if 0
    fprintf(fp, "%-*s %d %d: %5d,%-5d : %5d %5d %5d %5d\n",
        rest, tm,
        s->hasrel, s->fixed,
        s->gx, s->gy, s->glx, s->gby, s->grx, s->gty);
#endif

    fprintf(fp, "%-*s\n",
        rest, tm);

    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _printobjlist(fp, pre, (ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    return 0;
#undef W
}

int
printobjlist(FILE *fp, char *pre, ob* s)
{
    int r;
    fprintf(fp, "%sobject list\n", pre);
#if 0
    fprintf(fp,
"%soid: type                     r f:    gx,gy    :   glx   gby   grx   gty\n", pre);
#endif

    fprintf(fp, "%soid: type\n", pre);

    r = _printobjlist(fp, pre, s, 0);
    fprintf(fp, "%s\n", pre);
    fflush(fp);
    return r;
}





ns*
newnamespace()
{
    ns* r;
    r = malloc(sizeof(ns));
    if(r) {
        memset(r, 0, sizeof(ns));
    }
    r->root = r;
    return r;
}

ns*
newnamespace_child(ns*pns)
{
    ns* r;
    r = malloc(sizeof(ns));
    if(r) {
        memset(r, 0, sizeof(ns));
    }
    r->root = pns->root;
    r->parent = pns;
    return r;
}

int hide_serial=1;

int
ss_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    sstr *sv;

    sv = (sstr*)xv;
    ik = sprintf(dst, "<'%s' %x>", sv->ssval, sv->ssopt);

    return ik;
}

int
segop_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    segop *sv;

    sv = (segop*)xv;
    ik = sprintf(dst, "<%s(%d) '%s'>",
            rassoc(objattr_ial, sv->cmd), sv->cmd, sv->val);

    return ik;
}

int
seg_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    seg* s;

    s = (seg*)xv;
    ik = sprintf(dst, "(%s(%d),%s(%d),%d,%d;%d,%d,%d,%d;%d,%d)",
        rassoc(objattr_ial, s->ptype), s->ptype,
        s->jtype>0 ? "JOIN" : "-",
        s->jtype,
        s->coordtype, s->ftflag,
        s->x1, s->y1, s->x2, s->y2,
        s->rad, s->ang);

    return 0;
}

ob*
newobj()
{
    ob* r;
#if 0
P;
#endif
    r = malloc(sizeof(ob));
    if(!r) {
        printf("ERROR no memory\n");
        return NULL;
    }
    memset(r, 0, sizeof(ob));
    r->oid = hide_serial++;

    r->cob.outlinecolor = def_fgcolor;
    r->cob.textcolor    = def_fgcolor;
    r->cob.textbgcolor  = def_bgcolor;
    r->cob.fillcolor    = def_bgcolor;

    r->cob.rad      = -1;
    
    r->cob.ssar     = varray_new();
    r->cob.segopar  = varray_new();
    r->cob.segar    = varray_new();
    varray_entrysprintfunc(r->cob.ssar, ss_sprintf);
    varray_entrysprintfunc(r->cob.segopar, segop_sprintf);
    varray_entrysprintfunc(r->cob.segar, seg_sprintf);

    qbb_reset(&r->localbb);
    qbb_reset(&r->globalbb);

    return r;
}

ob*
cloneobj(ob* oo)
{
    ob* r;
#if 0
P;
#endif
    r = malloc(sizeof(ob));
    if(!r) {
        printf("ERROR no memory\n");
        return NULL;
    }
    memcpy(r, oo, sizeof(ob));
    r->oid = hide_serial++;

    return r;
}

ob*
newbox()
{
    ob* r;

    r = newobj();
    if(r) {
        r->type = CMD_BOX;
    }
    return r;
}

ob*
newchunk()
{
    int  i;
    ob*  r;
    ch*  rx;
#if 0
P;
#endif
    r  = newobj();
    if(!r) {
        return NULL;
    }

    rx = (ch*)malloc(sizeof(ch));
    if(!rx) {
        printf("ERROR no memory\n");
        free(r);
        return NULL;
    }

    memset(rx, 0, sizeof(ch));
    rx->nch = 0;
    for(i=0;i<CH_OBJ_LEN;i++) {
        rx->ch[i] = NULL;
    }

    r->type = CMD_CHUNK;
    r->cch  = *rx;
    r->cch.root = &r->cch;

    free(rx);
    rx = NULL;

#if 0
    printf("%s:%d r %p\n", __func__, __LINE__, r);
#endif
    return r;
}

ob*
newchunk_child(ob* pch)
{
    ob* nch;
    nch = newchunk();
    if(!nch) {
        return NULL;
    }

    sprintf(nch->cch.name, "copy of %s", pch->cch.name);
#if 0
    nch->cch.dir   = pch->cch.dir;
    nch->cch.scale = pch->cch.scale;
#endif
    cha_copy(&nch->cch, &pch->cch);
    nch->cch.root   = pch->cch.root;
    nch->cch.parent = &pch->cch;
#if 0
    nch->cch.qob    = pch;
#endif
#if 1
    nch->cch.qob    = nch;
#endif

    return nch;
}


#include "chs.c"
#include "nss.c"
#include "chas.c"

int
chadd(ob* xch, ob* xob)
{
#if 0
    printf("%s:%d xch %p xob %p\n", __func__, __LINE__, xch, xob);
#endif

    if(!xch) {
        printf("%s: ERROR NULL chunk\n", __func__);
        return -1;
    }
#if 0
    if(xch->type!=CMD_CHUNK) 
#endif
    if(!ISCHUNK(xch->type))
    {
        printf("%s: ERROR not chunk oid %d\n", __func__, xch->type);
        return -1;
    }
    if(!xob) {
        printf("%s: ERROR NULL object\n", __func__);
        return -1;
    }

#if 0
    printf("%s:%d xch %d xob %d\n",
        __func__, __LINE__, xch->oid, xob->oid);
#endif

#if 0
    printf("nch %3d/%d \n", xch->cch.nch, CH_OBJ_LEN);
#endif
    if(xch->cch.nch>=CH_OBJ_LEN-1) {
        Error("no slot in chunk\n");
    }

    xch->cch.ch[xch->cch.nch] = (struct obattr*)xob;
    xch->cch.nch++;

#if 1
    xob->behas = &xch->cch;
#endif

    return 0;
}

int
chdel(ob* xch, ob* xob)
{
    ob* stored;
P;
#if 0
    printf("%s:%d xch %p xob %p\n", __func__, __LINE__, xch, xob);
#endif

    if(!xch) {
        printf("%s: ERROR NULL chunk\n", __func__);
        return -1;
    }
#if 0
    if(xch->type!=CMD_CHUNK) 
#endif
    if(!ISCHUNK(xch->type))
    {
        printf("%s: ERROR not chunk oid %d\n", __func__, xch->type);
        return -1;
    }
    if(!xob) {
        printf("%s: ERROR NULL object\n", __func__);
        return -1;
    }
    
    stored = NULL;

    if(xch->cch.nch<=0) {
        printf("ERROR no member chunk\n");
    }

    stored = (ob*) xch->cch.ch[xch->cch.nch-1];
    xch->cch.ch[xch->cch.nch-1] = NULL;
    xch->cch.nch--;

    if(stored!=xob) {
        printf("ERROR not equal stored %p xob %p\n", stored, xob);
    }
    else {
        Echo("\tequal stored %p xob %p\n", stored, xob);
    }

    return 0;
}

int
nsaddobj(ns* xns, ob* xob, char *label)
{
#if 0
P;
#endif
    xns->chn[xns->nch] = strdup(label);
    xns->cht[xns->nch] = CMD_OBJ;
    xns->chv[xns->nch] = xob;
    xns->chc[xns->nch] = NULL;
    xns->nch++;
    return 0;
}

int
nsaddns(ns* xns, ns* xob, char *label)
{
#if 0
P;
#endif
    xns->chn[xns->nch] = strdup(label);
    xns->cht[xns->nch] = CMD_NAMESPACE;
    xns->chv[xns->nch] = NULL;
    xns->chc[xns->nch] = xob;
    xns->nch++;
    return 0;
}

int
nsaddnsobj(ns* xns, ns* nns, ob* xob, char *label)
{
#if 0
P;
#endif
    xns->chn[xns->nch] = strdup(label);
    xns->cht[xns->nch] = CMD_NAMESPACE;
    xns->chv[xns->nch] = xob;
    xns->chc[xns->nch] = nns;
    xns->nch++;
    return 0;
}


int
splitdot(char *h, int hlen, char *r, int rlen, char *full)
{
    char *p, *q;
    int  c;

    p = full;
    q = h;
    c = 0;
    while(*p && *p!='.' && c<hlen) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    if(*p=='.') {
        p++;
        strcpy(r, p);
    }
    else {
        r[0] = '\0';
    }

    return 0;
}

#define _ns_find_obj(a1,a2) _ns_find_objP(a1,a2,NULL,NULL)

#if 0
ob*
_ns_find_obj(ns* xns, char *xname)
#endif
ob*
_ns_find_objP(ns* xns, char *xname, int* xx, int* xy)
{
    char head[BUFSIZ];
    char rest[BUFSIZ];
    int i;
    ob* r;
    int ik;
    int mx, my;

#if 1
    Echo("  %s: xns %p, xname '%s' xx %p xy %p\n",
        __func__, xns, xname, xx, xy);
    if(xx && xy) {
        Echo("  *xx,*xy = %d,%d\n", *xx, *xy);
    }
#endif
#if 0
    ns_dump(xns);
#endif

    if(xx && xy) {
        mx = *xx;
        my = *xy;
    }
    else {
        mx = my = 0;
    }

    r = NULL;
    if(!xns) {
        goto out;
    }

#if 0
    Echo("  xname '%s'\n", xname);
#endif
    ik = splitdot(head, BUFSIZ, rest, BUFSIZ, xname);
#if 0
    Echo("  head  '%s' rest '%s'\n", head, rest);
#endif

    for(i=0;i<xns->nch;i++) {
#if 0
        Echo("     %2d '%s' vs '%s'\n", i, head, xns->chn[i]);
#endif
        if(strcasecmp(xns->chn[i], head)==0) {
P;
#if 1
            Echo("       FOUND\n");
            Echo("         chn '%s'\n", xns->chn[i]);
            Echo("         cht %d\n",   xns->cht[i]);
            Echo("         chv %p\n",   xns->chv[i]);
#endif

            ob *mm;
            mm = xns->chv[i];
#if 1
            if(mm) {
                Echo(
                  " mm %p ox,oy %d,%d x,y %d,%d gx,gy %d,%d\n",
                    mm, mm->cox, mm->coy, mm->cx, mm->cy,
                    mm->cgx, mm->cgy);
            }
#endif


            if(xns->cht[i]==CMD_OBJ) {
P;
                if(rest[0]=='\0') {
P;
                    r = xns->chv[i];
        Echo("b mx,my %d,%d\n", mx, my);
                    mx += r->cx;
                    my += r->cy;
        Echo("a mx,my %d,%d\n", mx, my);
                    break;
                }
            }
            if(xns->cht[i]==CMD_NAMESPACE) {
                if(rest[0]=='\0') {
P;
                    r = xns->chv[i];
        Echo("b mx,my %d,%d\n", mx, my);
                    mx += r->cx;
                    my += r->cy;
        Echo("a mx,my %d,%d\n", mx, my);
                }
                else {
P;

        Echo("b mx,my %d,%d\n", mx, my);
                    if(mm) {
                        mx += mm->cox + mm->cx;
                        my += mm->coy + mm->cy;
                    }
        Echo("a mx,my %d,%d\n", mx, my);
                    r = _ns_find_objP(xns->chc[i], rest, &mx, &my);
                }
                break;
            }
#if 0
            else {
                break;
            }
#endif
        }
        /* next */
#if 0
        if(r) break;
#endif
    }

out:
#if 1
    Echo("%s:  xname '%s' ; r %p mx,my %d,%d\n", __func__, xname, r, mx, my);
    Echo("  mx,my %d,%d\n", mx, my);
#endif
    if(xx) *xx = mx;
    if(xy) *xy = my;
    return r;
}

ob*
ns_find_obj(ns* xns, char *xname)
{
    ob* r;

#if 0
    Echo("%s:    xns %p, xname '%s'\n", __func__, xns, xname);
#endif
    r = _ns_find_obj(xns, xname);
#if 0
    if(r) {
        Echo("r   %p oid %d type %d\n", r, r->oid, r->type);
    }
    else {
        Echo("r   %p\n", r);
    }
#endif
    return r;
}

int
ch_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    ch *xch;

    xch = (ch*)xv;
    ik = sprintf(dst, "ch-'%s'",
            xch->name);

    return ik;
}


int
revch(ob *x, ch* ref, int *rx, int *ry)
{
    ch  *qch;
    int  r;
    ch  *c;
    int  i;
    varray_t *chpath;
    ob  *u;
    int  mx, my;

    Echo("%s: x %p ref %p START\n", __func__, x, ref);
    if(x) {
        Echo("  x->oid %d\n", x->oid);
    }

    mx = my = 0;
    qch = NULL;
    r = -3;
    chpath = NULL;
    
    if(!x) {
        r = -1;
        goto out;
    }

#if 1
    mx = x->cx;
    my = x->cy;
#endif

    if(x->behas) {
        chpath = varray_new();
        varray_entrysprintfunc(chpath, ch_sprintf);

        c = x->behas;
        Echo("c %p\n", c);
        while(c) {
            Echo("  c %p parent %p root %p\n", c, c->parent, c->root);
            u = c->qob;
            if(u) {
                Echo("    qob %p oid %d\n", u, u->oid);
            }

            varray_push(chpath, (void*)c);

            c = c->parent;
            Echo("c %p\n", c);
        }

#if 0
        varray_dump(chpath);
#endif
#if 0
        varray_fprint(stdout, chpath);
#endif

        Echo("chpath\n");
        for(i=0;i<chpath->use;i++) {
            c = (ch*)chpath->slot[i];
            Echo("  %2d: %p qob %p\n", i, c, c->qob);

            u = c->qob;
            if(u) {
                Echo("    u %p; oid %d ox,oy %d,%d x,y %d,%d\n",
                    u, u->oid, u->cox, u->coy, u->cx, u->cy);
                mx += u->cox + u->cx;
                my += u->coy + u->cy;
            }

            Echo("    mx,my %d,%d\n", mx, my);

            if(c==ref) {
                r = 1;
                qch = c;
                break;
            }
        }

    }
    
out:
    if(r==1) {
        *rx = mx;
        *ry = my;
    }
    if(x) {
        Echo("%s: x %p oid %d; r %d rx,ry %d,%d\n",
            __func__, x, x->oid, r, *rx, *ry);
    }
    else {
        Echo("%s: x %p; r %d rx,ry %d,%d\n", __func__, x, r, *rx, *ry);
    }
    fflush(stdout);

    return r;
}

#define LPOS    (1)
#define PPOS    (2)

int
_ns_find_objposP(ns *xns, ob *b, char *xname, int pmode, int *rx, int *ry)
{
    char  yname[BUFSIZ];
    char  token[BUFSIZ];
    char *p, *q;
    ob   *u;
    int   pos;
    int   r;
    int   ux, uy;
P;
#if 1
        Echo("%s: b %p xname '%s' pmode %d\n",
            __func__, p, xname, pmode);
        if(b) {
            Echo("  b->behas %p\n", b->behas);
        }
#endif

    r = -1;
    yname[0] = '\0';

    q = NULL;
    p = xname;
    while(*p) {
        q = p;
        p = draw_word(p, token, BUFSIZ, '.');
        if((token[0]>='A'&&token[0]<='Z')||token[0]=='_') {
            strcat(yname, token);
            strcat(yname, ".");
        }
        else {
            break;
        }
    }


#if 1
        Echo("  yname '%s' 1\n", yname);
#endif
    dellastcharif(yname, '.');
#if 1
        Echo("  yname '%s' 2\n", yname);
    if(q) {
        Echo("  q     '%s'\n", q);
    }
    if(p) {
        Echo("  p     '%s'\n", p);
    }
#endif

    ux = uy = 0;

    u = _ns_find_objP(xns, yname, &ux, &uy);
    if(u) {
#if 1
        Echo("  u %p gx,gy = %d,%d solved? %d drawed? %d finalized? %d\n",
            u, u->gx, u->gy, u->sizesolved, u->drawed, u->finalized);
        Echo("  u %p ux,uy = %d, %d\n", u, ux, uy);
#endif

#if 1
        {
            int ik;
            ik = revch(u, b->behas, &ux, &uy);
        }
#endif

        if(q) {
            pos = assoc(pos_ial, q);
#if 1
            Echo("  pos %d q '%s'\n", pos, q);
#endif
        }
        else {
            pos = PO_CENTER;
        }
#if 1
        Echo("  pos %d\n", pos);
#endif
        int x, y;

        if(pmode==PPOS) { x = u->cgx;   y = u->cgy; }
        else            { x = u->cx;    y = u->cy; }

#if 1
        Echo("  original x,y = %d,%d ; %s\n", x, y, __func__);
        Echo("     guess x,y = %d,%d\n", ux, uy);
#endif
        *rx = ux;
        *ry = uy;

        switch(pos) {
        case PO_CENTER:    *rx += 0;          *ry += 0;           break;
        case PO_NORTH:     *rx += 0;          *ry += u->cht/2;  break;
        case PO_NORTHEAST: *rx += u->cwd/2;   *ry += u->cht/2;  break;
        case PO_EAST:      *rx += u->cwd/2;   *ry += 0;           break;
        case PO_SOUTHEAST: *rx += u->cwd/2;   *ry += -u->cht/2;  break;
        case PO_SOUTH:     *rx += 0;          *ry += -u->cht/2;  break;
        case PO_SOUTHWEST: *rx += -u->cwd/2;  *ry += -u->cht/2;  break;
        case PO_WEST:      *rx += -u->cwd/2;  *ry += 0;           break;
        case PO_NORTHWEST: *rx += -u->cwd/2;  *ry += u->cht/2;  break;
        default:
            printf("ERROR ignore position '%s' of '%s'\n", p, xname);
        }

#if 1
        Echo("  cooked  rx,y = %d,%d\n", *rx, *ry);
#endif

        r = 0;
    }
    else {
    }

P;
    Echo("%s: xname %-16s pmode %d; r %d x,y %d,%d\n",
        __func__, xname, pmode, r, *rx, *ry);

    return r;
}


int
_ns_find_objpos(ns *xns, char *xname, int *rx, int *ry, int pmode)
{
    char  yname[BUFSIZ];
    char  token[BUFSIZ];
    char *p, *q;
    ob   *u;
    int   pos;
    int   r;

#if 1
        Echo("%s: xname '%s' pmode %d\n", __func__, xname, pmode);
#endif

    r = -1;
    yname[0] = '\0';

    q = NULL;
    p = xname;
    while(*p) {
        q = p;
        p = draw_word(p, token, BUFSIZ, '.');
        if((token[0]>='A'&&token[0]<='Z')||token[0]=='_') {
            strcat(yname, token);
            strcat(yname, ".");
        }
        else {
            break;
        }
    }


#if 1
        Echo("  yname '%s' 1\n", yname);
#endif
    dellastcharif(yname, '.');
#if 1
        Echo("  yname '%s' 2\n", yname);
    if(q) {
        Echo("  q     '%s'\n", q);
    }
    if(p) {
        Echo("  p     '%s'\n", p);
    }
#endif

    u = ns_find_obj(xns, yname);
    if(u) {
#if 1
        Echo("  u %p gx,gy = %d,%d solved? %d drawed? %d finalized? %d\n",
            u, u->gx, u->gy, u->sizesolved, u->drawed, u->finalized);
#endif

        if(q) {
            pos = assoc(pos_ial, q);
#if 1
            Echo("  pos %d q '%s'\n", pos, q);
#endif
        }
        else {
            pos = PO_CENTER;
        }
#if 1
        Echo("  pos %d\n", pos);
#endif
        int x, y;

        if(pmode==PPOS) { x = u->cgx;   y = u->cgy; }
        else            { x = u->cx;    y = u->cy; }

#if 1
        Echo("  original x,y = %d,%d ; %s\n", x, y, __func__);
#endif

        switch(pos) {
        case PO_CENTER:    *rx = x;          *ry = y;           break;
        case PO_NORTH:     *rx = x;          *ry = y+u->cht/2;  break;
        case PO_NORTHEAST: *rx = x+u->cwd/2; *ry = y+u->cht/2;  break;
        case PO_EAST:      *rx = x+u->cwd/2; *ry = y;           break;
        case PO_SOUTHEAST: *rx = x+u->cwd/2; *ry = y-u->cht/2;  break;
        case PO_SOUTH:     *rx = x;          *ry = y-u->cht/2;  break;
        case PO_SOUTHWEST: *rx = x-u->cwd/2; *ry = y-u->cht/2;  break;
        case PO_WEST:      *rx = x-u->cwd/2; *ry = y;           break;
        case PO_NORTHWEST: *rx = x-u->cwd/2; *ry = y+u->cht/2;  break;
        default:
            printf("ERROR ignore position '%s' of '%s'\n", p, xname);
        }

#if 1
        Echo("  cooked  rx,y = %d,%d\n", *rx, *ry);
#endif

        r = 0;
    }
    else {
    }

P;
    Echo("%s: xname %-16s pmode %d; r %d x,y %d,%d\n",
        __func__, xname, pmode, r, *rx, *ry);

    return r;
}


int
ns_find_objpos(ns *xns, char *xname, int *rx, int *ry)
{
    return _ns_find_objpos(xns, xname, rx, ry, LPOS);
}

int
ns_find_objposG(ns *xns, char *xname, int *rx, int *ry)
{
    return _ns_find_objpos(xns, xname, rx, ry, PPOS);
}

ob*
_ns_find_objX(ns* xns, char *xname, int *ux, int *uy)
{
    char head[BUFSIZ];
    char rest[BUFSIZ];
    int i;
    ob* r;
    int ik;
    int cx, cy;

#if 1
    Echo("  %s: xns %p, xname '%s'\n", __func__, xns, xname);
#endif
#if 0
    ns_dump(xns);
#endif

    *ux = -99999;
    *uy = -99999;

    r = NULL;
    if(!xns) {
        goto out;
    }

#if 0
    Echo("  xname '%s'\n", xname);
#endif
    ik = splitdot(head, BUFSIZ, rest, BUFSIZ, xname);
#if 0
    Echo("  head  '%s' rest '%s'\n", head, rest);
#endif

    for(i=0;i<xns->nch;i++) {
#if 0
        Echo("     %2d '%s' vs '%s'\n", i, head, xns->chn[i]);
#endif
        if(strcasecmp(xns->chn[i], head)==0) {
P;
#if 1
            Echo("       FOUND\n");
            Echo("         chn '%s'\n", xns->chn[i]);
            Echo("         cht %d\n",   xns->cht[i]);
            Echo("         chv %p\n",   xns->chv[i]);
#endif
            if(xns->cht[i]==CMD_OBJ) {
P;
                if(rest[0]=='\0') {
P;
                    r = xns->chv[i];
                    *ux = r->gx;
                    *uy = r->gy;
                    break;
                }
            }
            if(xns->cht[i]==CMD_NAMESPACE) {
                if(rest[0]=='\0') {
P;
                    r = xns->chv[i];
#if 1
#endif
                }
                else {
P;
                    r = _ns_find_objX(xns->chc[i], rest, &cx, &cy);

#if 1
                    Echo("  nest %d,%d and %d,%d\n", r->ox, r->oy, cx, cy);
#endif

                    *ux = r->ox + cx;
                    *uy = r->oy + cy;
                }
                break;
            }
#if 0
            else {
                break;
            }
#endif
        }
        /* next */
#if 0
        if(r) break;
#endif
    }

out:
#if 1
    Echo("%s: r %p; %d,%d\n", __func__, r, *ux, *uy);
#endif
    return r;
}

int
_ns_find_objposX(ns *xns, char *xname, int *rx, int *ry, int pmode)
{
    char  yname[BUFSIZ];
    char  token[BUFSIZ];
    char *p, *q;
    ob   *u;
    int   pos;
    int   r;

    int   gx, gy;

#if 1
        Echo("%s: xname '%s' pmode %d\n", __func__, xname, pmode);
#endif

    r = -1;
    yname[0] = '\0';

    q = NULL;
    p = xname;
    while(*p) {
        q = p;
        p = draw_word(p, token, BUFSIZ, '.');
        if((token[0]>='A'&&token[0]<='Z')||token[0]=='_') {
            strcat(yname, token);
            strcat(yname, ".");
        }
        else {
            break;
        }
    }


#if 1
        Echo("  yname '%s' 1\n", yname);
#endif
    dellastcharif(yname, '.');
#if 1
        Echo("  yname '%s' 2\n", yname);
    if(q) {
        Echo("  q     '%s'\n", q);
    }
    if(p) {
        Echo("  p     '%s'\n", p);
    }
#endif


#if 1

#endif


    u = _ns_find_objX(xns, yname, &gx, &gy);
    if(u) {
#if 1
        Echo("  xname '%s'\n", xname);
#endif
#if 1
        Echo("  u %p gx,y %d,%d\n", u, gx, gy);
#endif

        if(q) {
            pos = assoc(pos_ial, q);
#if 1
            Echo("  pos %d q '%s'\n", pos, q);
#endif
        }
        else {
            pos = PO_CENTER;
        }
#if 1
        Echo("  pos %d\n", pos);
#endif
        int x, y;

#if 1
        Echo("  guess    x,y = %d,%d\n", gx, gy);
#endif

        if(pmode==PPOS) { x = u->cgx;   y = u->cgy; }
        else            { x = u->cx;    y = u->cy; }

#if 1
        Echo("  original x,y = %d,%d ; %s\n", x, y, __func__);
#endif

        switch(pos) {
        case PO_CENTER:    *rx = x;          *ry = y;           break;
        case PO_NORTH:     *rx = x;          *ry = y+u->cht/2;  break;
        case PO_NORTHEAST: *rx = x+u->cwd/2; *ry = y+u->cht/2;  break;
        case PO_EAST:      *rx = x+u->cwd/2; *ry = y;           break;
        case PO_SOUTHEAST: *rx = x+u->cwd/2; *ry = y-u->cht/2;  break;
        case PO_SOUTH:     *rx = x;          *ry = y-u->cht/2;  break;
        case PO_SOUTHWEST: *rx = x-u->cwd/2; *ry = y-u->cht/2;  break;
        case PO_WEST:      *rx = x-u->cwd/2; *ry = y;           break;
        case PO_NORTHWEST: *rx = x-u->cwd/2; *ry = y+u->cht/2;  break;
        default:
            printf("ERROR ignore position '%s' of '%s'\n", p, xname);
        }

#if 1
        Echo("  cooked  rx,y = %d,%d\n", *rx, *ry);
#endif

        r = 0;
    }
    else {
    }

P;
    Echo("%s: xname %-16s pmode %d; r %d x,y %d,%d\n",
        __func__, xname, pmode, r, *rx, *ry);

    return r;
}


int
ns_find_objposX(ns *xns, char *xname, int *rx, int *ry)
{
    return _ns_find_objposX(xns, xname, rx, ry, LPOS);
}

int
ns_find_objposXG(ns *xns, char *xname, int *rx, int *ry)
{
    return _ns_find_objposX(xns, xname, rx, ry, PPOS);
}


char*
_ns_find_name(ns* s, ob* xob, int w)
{
    int i;
    int u;
    char *q;
#define W for(u=0;u<w;u++) printf(" ");
    if(!s) {
        return NULL;
    }

    W;
    printf("namespace %p name '%s' root %p parent %p num %d\n",
        s, s->name, s->root, s->parent, s->nch);
    for(i=0;i<s->nch;i++) {
        W;
        printf("%3d: '%s' type %d ", i, s->chn[i], s->cht[i]);
        if(s->cht[i]==CMD_NAMESPACE) {
            W;
            printf("namespace object %p oid %d\n", s->chv[i], s->chv[i]->oid);
            q = _ns_find_name(s->chc[i], xob, w+dumptabstop);
            if(q) {
                return q;
            }
        }
        else {
            printf("object %p oid %d\n", s->chv[i], s->chv[i]->oid);
            if(s->chv[i]==xob) {
                return s->chn[i];
            }
        }
    }

    return NULL;
}


int
recalcsizeparam()
{

    if(objunit<=0) {
        Error("sorry unit size is zero or negative.\n");
        exit(7);
    }

#define V(x)    printf("%-20s %8d\n", #x, x);
    
    def_arrowsize       = arrowsizefactor       * objunit;
    def_warrowsize      = warrowsizefactor      * objunit;
    def_barrowgap       = barrowgapfactor       * objunit;

    def_linethick       = linethickfactor       * objunit;
    def_wlinethick      = wlinethickfactor      * objunit;
    def_linedecothick   = linedecothickfactor   * objunit;
    def_linedecopitch   = linedecopitchfactor   * objunit;
    def_mustsize        = mustsizefactor        * objunit;
#if 1
    if(def_linethick<=0) {
        def_linethick = 1;
    }
#endif
#if 1
    if(def_wlinethick<=0) {
        def_wlinethick = 1;
    }
#endif


    def_textheight      = textheightfactor      * objunit;
    def_hatchpitch      = hatchpitchfactor      * objunit;
    def_hatchthick      = hatchthickfactor      * objunit;
#if 1
    if(def_hatchthick<=0) {
        def_hatchthick = 1;
    }
#endif
    def_noteosep        = noteosepfactor        * objunit;
    def_noteisep        = noteisepfactor        * objunit;
    def_marknoderad     = marknoderadfactor     * objunit;
    def_markbbthick     = markbbthickfactor     * objunit;

#if 0
    V(objunit);
    V(def_arrowsize);
    V(def_linethick);
    V(def_wlinethick);
    V(def_linedecothick);
    V(def_linedecopitch);
    V(def_barrowgap);
    V(def_textheight);
    V(def_hatchpitch);
    V(def_hatchthick);
#endif
#if 0
    V(def_textdecent);
#endif

    return 0;
}

#include "notefile.c"

#include "seg.c"

#include "parse.c"
#ifdef DO_FORKCHK
#include "forkchk.c"
#endif
#include "put.c"
#ifdef DO_LINKCHK
#include "linkchk.c"
#endif
#include "finalize.c"
#include "picdraw.c"
#include "epsdraw.c"

int     nodraw = 0;
char   *outfile = "out.eps";
int     canvaswd = (int)( 8.27 * 72);
int     canvasht = (int)(11.69 * 72);
int     canvasrt = 0;
double  canvassc = 0.01;

int
print_usage()
{
    printf("tangy - picture generator according to special language\n");
    printf("usage: tangy [option]\n");
    printf("option:     ### bp is bigpoint = 1/72inch (c.f. 1cm=28.34bp)\n");
    printf("    -h      print this message\n");
    printf("    -m      print hints for language\n");
    printf("    -V      print version\n");
    printf("    -P      print parameters\n");
    printf("    -o file set output file (current '%s')\n",
            outfile);
    printf("    -u num  set unit by 0.01bp (current %d)\n",
            objunit);
    printf("    -M num  set EPS outside margin by bp (current %d)\n",
            epsoutmargin);
    printf("    -g      draw grid\n");
    printf("    -G num  set grid pitch in bp (current %d)\n", 
            def_gridpitch);
    printf("    -R num  set grid range; how many times of pitch (current %d)\n",
        def_gridrange);
    printf("    -b      draw boundingbox of objects\n");
    printf("    -i      draw object IDs\n");
    printf("    -l      print object list for debug\n");
    printf("    -c      print color list for debug\n");
    printf("    -F font set default font (current '%s')\n", def_fontname);
    printf("following itmes are reserved for future. do not use.\n");
    printf("    -v      verbose mode (current %d)\n", _t_);
    printf("    -q      quiet mode\n");
    printf("    -p      passing trace mode (current %d)\n", _p_);
    printf("   *-d      draft mode\n");
    printf("   *-L      draw labels\n");
    printf("   *-D      debug mode\n");
    printf("   *-r      draw ruler\n");
    printf("   *-s num  set scale\n");
    printf("   *-n      no draw\n");
    printf("   *-N file set notefile\n");

    return 0;
}


int
_print_alistmemkeys(char *pre, int ind, int wmax, apair_t *ap)
{
    int i;
    int j;
    apair_t *cp;
    int w;
    int l;

    printf("%s\n", pre);

    cp = ap;
    i  = 0;
    w  = ind;
            for(j=0;j<ind;j++) printf(" ");
    while(1) {
        if(!cp->name) {
            break;
        }
        l = strlen(cp->name)+1;

        if(w+l<wmax) {
            printf("%s ", cp->name);
            w += l;
        }
        else {
            printf("\n");
            for(j=0;j<ind;j++) printf(" ");
            printf("%s ", cp->name);
            w = ind + l;
        }
        
        cp++;
        i++;
    }


    printf("\n");
    
    return 0;
}

int
print_alistmemkeys(char *pre, apair_t *ap)
{
    return _print_alistmemkeys(pre, 4, 72, ap);
}

int
print_hints()
{
    printf("You may find your forget keyword...\n");
    print_alistmemkeys("object/command names:", cmd_ial);
    print_alistmemkeys("object attributes:", objattr_ial);
    print_alistmemkeys("line type:", linetype_ial);
    print_alistmemkeys("arrowhead type:", arrowhead_ial);
    print_alistmemkeys("hatching type:", hatchtype_ial);
    print_alistmemkeys("font type:", ff_ial);
    print_alistmemkeys("font size:", fh_ial);

    return 0;
}

int
print_version()
{
    printf("tangy version 2.031"
#ifdef GITCHASH
    " " GITCHASH
#endif
    "\n");
    return 0;
}

int
print_param()
{
    printf("tangy parameters:\n");

    printf("    %-24s %7s %7s\n", "name", "value", "factor");
    printf("    ---\n");

#define QPV(x)  \
    printf("    %-24s %7d %7.3f\n", #x, def_ ## x, x ## factor);
#define QP(x)  \
    printf("    %-24s %7s %7.3f\n", #x, "", x ## factor);

    printf("  unit-base: multiply by unitsize %d\n", objunit);

    QPV(arrowsize);
    QPV(warrowsize);
    QPV(linethick);
    QPV(linedecothick);
    QPV(linedecopitch);
    QPV(wlinethick);
    QPV(barrowgap);
    QPV(textheight);
    QPV(hatchthick);
    QPV(hatchpitch);
    QPV(marknoderad);
    QPV(markbbthick);
    QPV(noteosep);
    QPV(noteisep);

    printf("  text-base: multiply by text height (dynamic; normal %d)\n",
        def_textheight);

    QP(textdecent);
    QP(textbgmargin);

#undef QPV
#undef QP

    return 0;
}

int
test0()
{
    int i;

    i = 0;
    while(cmd_ial[i].name) {
        printf("%-16s ", cmd_ial[i].name);
        if(ISCHUNK(cmd_ial[i].value)) {
            printf("CHUNK ");
        }
        else {
            printf("-     ");
        }
        if(ISATOM(cmd_ial[i].value)) {
            printf("ATOM ");
        }
        else {
            printf("-    ");
        }
        if(ISGLUE(cmd_ial[i].value)) {
            printf("GLUE ");
        }
        else {
            printf("-    ");
        }
        if(HASBODY(cmd_ial[i].value)) {
            printf("BODY ");
        }
        else {
            printf("-    ");
        }
        if(MAYEXPAND(cmd_ial[i].value)) {
            printf("EXPAND ");
        }
        else {
            printf("-      ");
        }

            printf("\n");

        i++;
    }
    return 0;
}

int
main(int argc, char *argv[])
{
    ob*   ch0;
    ns*   ns0;
    int   flag;
    int   ik;
    int   do_colorlist, do_objlist;
    FILE *fp;
    int x, y;

    do_objlist = 0;
    do_colorlist = 0;
    
    pallet = new_default_pallet();

    while((flag=getopt(argc, argv,
            "0hmVPvqpngbSdiLrtDo:u:G:R:M:F:lcs:"))!=EOF) {
        switch(flag) {
        case '0':
            test0();
            exit(0);
        case 'h':
            print_usage();
            exit(0);
            break;
        case 'm':
            print_hints();
            exit(0);
            break;
        case 'V':
            print_version();
            exit(0);
            break;
        case 'P':
            recalcsizeparam(); /* XXX */
            print_param();
            exit(0);
            break;
        case 'v':
            _t_ ++;
            break;
        case 'q':
            _t_ = 0;
            break;
        case 'p':
            _p_ = 1 - _p_;
            break;
        case 'n':
            nodraw = 1;
            break;
        case 'g':
            grid_mode = 1;
            break;
        case 'b':
            bbox_mode = 1;
            break;
        case 'i':
            oidl_mode = 1;
            break;
        case 'L':
            namel_mode = 1;
            break;
        case 'S':
            skelton_mode = 1;
            break;
        case 'd':
            draft_mode = 1;
            break;
        case 'r':
            ruler_mode = 1;
            break;
        case 't':
            text_mode = 1;
            break;
        case 'D':
            grid_mode = 1;
            bbox_mode = 1;
            draft_mode = 1;
            oidl_mode = 1;
            ruler_mode = 1;
            text_mode = 1;
            break;
            
        case 'o':
            outfile = strdup(optarg);
            break;
        case 'u':
            objunit = atoi(optarg);
            break;
        case 'G':
            def_gridpitch = atoi(optarg);
            break;
        case 'R':
            def_gridrange = atoi(optarg);
            break;
        case 'M':
            epsoutmargin = atoi(optarg);
            break;

        case 'F':
            def_fontname = strdup(optarg);
            break;

#if 0
        case 'M':
            if(strcasecmp(optarg, "a4")==0) {
                /* inch x bp/inch */
                canvaswd = (int)( 8.27 * 72);
                canvasht = (int)(11.69 * 72);
            }
            break;
#endif
        case 'l':
            do_objlist++;
            break;
        case 'c':
            do_colorlist++;
            break;
        case 's':
            canvassc *= atof(optarg);
            break;
        }
    }

#if 0
    if(do_rotate) {
        int x;
        x = canvasht;
        canvasht = canvaswd;
        canvaswd = x;
        canvasrt = 90;
    }
#endif

    notefile_setup();

    ch0   = newchunk();
    ns0   = newnamespace();
    if(ch0) strcpy(ch0->cch.name, "*root*");
    if(ns0) strcpy(ns0->name, "*root*");

    recalcsizeparam();

    ik = parse(stdin, ch0, ns0);
    if(INTRACE) {
        ob_cndump(ch0);
    }

    if(INTRACE) {
        varray_fprint(stdout, gnotefilelist);
        varray_fprint(stdout, gnotebindlist);
    }

    notefile_load();

#if 0
Echo("ch0 oid %d LANE? %d\n", ch0->oid, ch0->cch.lanenum);
#endif
Echo("ch0 oid %d LANE? %d\n", ch0->oid, ch0->cob.lanenum);

#ifdef DO_FORKCHK
    ik = fkchk(ch0, ns0);
P;
#endif

    x = 0;
    y = 0;
P;
    ik = put(ch0, &x, &y, ns0);
    if(INTRACE) {
        ob_gdump(ch0);
    }

#ifdef DO_LINKCHK
P;
    ik = linkchk(ch0, ns0);
    if(INTRACE) {
        ob_gdump(ch0);
    }
#endif

P;
    finalize(ch0, 0, 0, ns0);

    if(INTRACE) {
        ob_adump(ch0);
        ob_bgdump(ch0);
        ob_bldump(ch0);
    }

#if 0
    picdraw(ch0, 0, 0, ns0);
    fflush(stdout);
#endif

#if 0
    printf("ht root-chunk %d vs canvas %d\n", ch0->ht, canvasht);
#endif

    if(ch0->ht<=0 || ch0->wd<=0) {
        Error("figure is empty or negative size\n");
        exit(9);
    }

#if 0
    canvassc = canvasht/ch0->ht;
    canvassc = 0.01;
#endif

P;
    fp = fopen(outfile, "w");
    if(!fp) {
        Error("cannot open output file '%s'\n", outfile);
        goto skip_output;
    }

    epsdraw(fp, canvaswd, canvasht, canvasrt, canvassc,
        ch0, 0, 0, ns0);
    fclose(fp);
    fp = NULL;

skip_output:
    (void) 0;

    if(do_objlist) {
        printobjlist(stdout, "  ", ch0);
    }
    if(do_colorlist) {
        varray_fprint(stdout, pallet);
    }

    exit(0);
}


