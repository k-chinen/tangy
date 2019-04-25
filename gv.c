/*
 * tangy second trial since 2016 Aug 
 * K.Chinen
 *
 */

#include "obj.h"
#include "gv.h"

double rf = M_PI/180.0;

int _t_ = 0;
int _p_ = 0;


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
    {"point",           CMD_POINT},
    {"dots",            CMD_DOTS},
    {"drum",            CMD_DRUM},
    {"paper",           CMD_PAPER},
    {"card",            CMD_CARD},
    {"diamond",         CMD_DIAMOND},
    {"house",           CMD_HOUSE},
    {"cloud",           CMD_CLOUD},
#if 0
    {"cylinder",        CMD_DRUM},
#endif
    {"circle",          CMD_CIRCLE},
    {"ellipse",         CMD_ELLIPSE},
    {"polygon",         CMD_POLYGON},
    {"uline",           CMD_ULINE},
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
    {"xcurve",          CMD_XCURVE},
    {"xcurveself",      CMD_XCURVESELF},
    {"curve",           CMD_BCURVE},
    {"curveself",       CMD_BCURVESELF},
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
    {"dmy1",            CMD_DMY1},
#if 0
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
    {"thru",            CMD_THRU},
    {"traceon",         CMD_TRACEON},
    {"traceoff",        CMD_TRACEOFF},
    {NULL,              -1},
};


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

apair_t sj_ial[] = {
    {"center",  SJ_CENTER},
    {"left",    SJ_LEFT},
    {"right",   SJ_RIGHT},
    {"fit",     SJ_FIT},
    {NULL,      -1},
};

apair_t ls_ial[] = {
    {"direct",      LS_DIRECT  },
    {"direct1",     LS_DIRECT | LS_FOCUS},
    {"square",      LS_SQUARE},
    {"square1",     LS_SQUARE | LS_FOCUS},
    {"arc",         LS_ARC},
    {"arc1",        LS_ARC    | LS_FOCUS},
    {"straight",    LS_STRAIGHT},
    {"straight1",   LS_STRAIGHT | LS_FOCUS},
    {"man",         LS_MAN},
    {NULL,          -1},
};

apair_t lo_ial[] = {
    {"nwd",         LO_NWD},
    {"nwr",         LO_NWR},
    {"swu",         LO_SWU},
    {"swr",         LO_SWU},
    {"nel",         LO_NEL},
    {NULL,          -1},
};

apair_t objattr_ial[] = {

    {"linecolor",           OA_LINECOLOR},
    {"linetype",            OA_LINETYPE},
    {"linethick",           OA_LINETHICK},
    {"linedecothick",       OA_LINEDECOTHICK},
    {"wlinethick",          OA_WLINETHICK},
    {"backcolor",           OA_BACKCOLOR},
    {"backhatch",           OA_BACKHATCH},
    {"fillcolor",           OA_FILLCOLOR},
    {"fillhatch",           OA_FILLHATCH},
    {"hatchthick",          OA_HATCHTHICK},
    {"hatchpitch",          OA_HATCHPITCH},
    {"textcolor",           OA_TEXTCOLOR},
    {"textbgcolor",         OA_TEXTBGCOLOR},

    {"linkstyle",           OA_LINKSTYLE},
    {"linkmap",             OA_LINKMAP},

    {"width",               OA_WIDTH},
    {"height",              OA_HEIGHT},
    {"depth",               OA_DEPTH},

    {"imargin",             OA_IMARGIN},
    {"gimargin",            OA_GIMARGIN},
    {"peak",                OA_PEAK},
    {"peakrotate",          OA_PEAKROTATE},
    {"concave",             OA_CONCAVE},
    {"textrotate",          OA_TEXTROTATE},
    {"rotate",              OA_ROTATE},
    {"rad",                 OA_RAD},
    {"length",              OA_LENGTH},
    {"deco",                OA_DECO},
    {"decocolor",           OA_DECOCOLOR},

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
    {"moveto",              OA_MOVETO},
    {"rmoveto",             OA_RMOVETO},
    {"lineto",              OA_LINETO},
    {"rlineto",             OA_RLINETO},
    {"rcurveto",            OA_RCURVETO},
    {"curveto",             OA_CURVETO},
    {"curve",               OA_RCURVETO},
    {"close",               OA_CLOSE},

    {"forehead",            OA_FOREHEAD},
    {"centhead",            OA_CENTHEAD},
    {"backhead",            OA_BACKHEAD},
    {"bothhead",            OA_BOTHHEAD},
    {"centheadpos",         OA_CENTHEADPOS},
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
    {"markguide",           OA_MARKGUIDE},

    {"hollow",              OA_HOLLOW},
    {"shadow",              OA_SHADOW},
    
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
    {"linedecothickfactor", OA_LINEDECOTHICKFACTOR},
    {"linedecopitchfactor", OA_LINEDECOPITCHFACTOR},
    {"wlinethickfactor",    OA_WLINETHICKFACTOR},
    {"hatchthickfactor",    OA_HATCHTHICKFACTOR},
    {"hatchpitchfactor",    OA_HATCHPITCHFACTOR},
    {"noteosepfactor",      OA_NOTEOSEPFACTOR},
    {"noteisepfactor",      OA_NOTEISEPFACTOR},

    {"bwcir",               OA_BWCIR},
    {"bwhcir",              OA_BWHCIR},
    {"bwrcir",              OA_BWRCIR},

    {"bwbox",               OA_BWBOX},
    {"bwxss",               OA_BWXSS},
    {"bwpls",               OA_BWPLS},

    {NULL,                  -1},
};

#if 0
apair_t coord_ial[] = {
    {"nul",     NUL_COORD},
    {"rel",     REL_COORD},
    {"abs",     ABS_COORD},
    {NULL,      -1},
};

apair_t coord_isal[] = {
    {"N",   NUL_COORD},
    {"R",   REL_COORD},
    {"A",   ABS_COORD},
    {NULL,      -1},
};

apair_t ftcoord_ial[] = {
    {"nul ",    NUL_COORD},
    {"from",    COORD_FROM},
    {"to  ",    COORD_TO},
    {NULL,      -1},
};

apair_t ftcoord_isal[] = {
    {"N",       NUL_COORD},
    {"F",       COORD_FROM},
    {"T",       COORD_TO},
    {NULL,      -1},
};
#endif


int objunit = OBJUNIT;

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

double def_hollowratio  = 0.8;
double def_shadowgray   = 0.5;

int def_dir             =   0;
int def_scale           =   1;
int def_color           =   0;
int def_fgcolor         =   0;
int def_bgcolor         =   7;
int def_backhatch       =   HT_SOLID;

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

char debuglog[BUFSIZ*10]="";




