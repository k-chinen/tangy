/*
 * tangy second trial since 2016 Aug 
 * K.Chinen
 *
 */
/* Tangy's object and related definitions */

#ifndef _TANGY_OBJ_H_
#define _TANGY_OBJ_H_

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

#include "ltype.h"
#include "ahead.h"
#include "hatch.h"



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
#define CMD_POINT       (35)
#define CMD_POLYGON     (43)
#define CMD_ULINE       (44)
#define CMD_DRUM        (50)
#define CMD_PAPER       (51)
#define CMD_CARD        (52)
#define CMD_DIAMOND     (53)
#define CMD_HOUSE       (54)
#define CMD_CLOUD       (60)

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

#define CMD_XCURVE       (501)
#define CMD_XCURVESELF   (502)
#define CMD_BCURVE      (511)
#define CMD_BCURVESELF  (512)

#define CMD_GEAR        (600)


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
#define CMD_THRU        (3009)

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

#define CMD_TRACEON             (30001)
#define CMD_TRACEOFF            (30002)

extern apair_t cmd_ial[];


#define ISCHUNK(x)  ((x)==CMD_CHUNK)
#define ISGLUE(x)   \
    ((x)==CMD_LINE||(x)==CMD_LINK||(x)==CMD_ARROW||\
     (x)==CMD_WLINE||(x)==CMD_WARROW||\
     (x)==CMD_BCURVE||(x)==CMD_BCURVESELF||\
     (x)==CMD_XCURVE||(x)==CMD_XCURVESELF||\
     (x)==CMD_PING||(x)==CMD_PINGPONG|| \
     (x)==CMD_BARROW||(x)==CMD_PLINE \
    )
#define ISATOM(x)   \
    ((x)==CMD_BOX||(x)==CMD_CIRCLE||(x)==CMD_ELLIPSE||(x)==CMD_DRUM|| \
     (x)==CMD_DOTS||(x)==CMD_ULINE||(x)==CMD_PAPER||(x)==CMD_CLOUD|| \
     (x)==CMD_CARD||(x)==CMD_HOUSE||(x)==CMD_DIAMOND|| \
     (x)==CMD_POLYGON||(x)==CMD_POINT||(x)==CMD_GEAR|| \
     (x)==CMD_SEP||(x)==CMD_LPAREN||(x)==CMD_RPAREN|| \
     (x)==CMD_LBRACKET||(x)==CMD_RBRACKET|| \
     (x)==CMD_LBRACE||(x)==CMD_RBRACE \
    )
#define HASBODY(x)  \
    (ISGLUE(x)||ISATOM(x)||(x)==CMD_NOP) 
#define MAYEXPAND(x)  \
    ((x)==CMD_SEP||(x)==CMD_PLINE|| \
     (x)==CMD_SCATTER||(x)==CMD_GATHER||(x)==CMD_THRU|| \
     (x)==CMD_LPAREN||(x)==CMD_RPAREN|| \
     (x)==CMD_LBRACKET||(x)==CMD_RBRACKET|| \
     (x)==CMD_LBRACE||(x)==CMD_RBRACE \
    )

#define EXVISIBLE(x) \
((x)==CMD_BOX||(x)==CMD_CIRCLE||(x)==CMD_POINT||(x)==CMD_ELLIPSE||\
 (x)==CMD_POLYGON||(x)==CMD_ULINE||(x)==CMD_DRUM||(x)==CMD_PAPER||\
 (x)==CMD_CARD||(x)==CMD_HOUSE||(x)==CMD_DIAMOND|| \
 (x)==CMD_CLOUD||(x)==CMD_GEAR||\
 (x)==CMD_LINE||(x)==CMD_ARROW||(x)==CMD_WLINE||(x)==CMD_WARROW||\
 (x)==CMD_BARROW||(x)==CMD_PLINE||(x)==CMD_LINK||\
 (x)==CMD_PING||(x)==CMD_PINGPONG||(x)==CMD_WLINE||(x)==CMD_WARROW||\
 (x)==CMD_XCURVE||(x)==CMD_XCURVESELF||(x)==CMD_BCURVE||(x)==CMD_BCURVESELF)

#define VISIBLE(x) \
(EXVISIBLE(x)||(x)==CMD_DOTS||(x)==CMD_SEP)


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


#define SJ_CENTER           (0)
#define SJ_LEFT             (1)
#define SJ_RIGHT            (2)
#define SJ_FIT              (3)

extern apair_t sj_ial[];

#define LS_NONE             (0x00)
#define LS_DIRECT           (0x01)
#define LS_SQUARE           (0x02)
#define LS_ARC              (0x04)
#define LS_CURVE            (0x07)
#define LS_STRAIGHT         (0x10)
#define LS_M_TYPE           (0x1f)
#define LS_JOIN             (0x100)
#define LS_FOCUS            (0x200)
#define LS_MAN              (0x1000)

extern apair_t ls_ial[];


#define LO_NWR              (0)
#define LO_NWD              (1)
#define LO_SWU              (2)
#define LO_SWR              (3)
#define LO_NEL              (4)
#define LO_SEL              (4)

extern apair_t lo_ial[];


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
#define OA_BACKCOLOR        (15)
#define OA_BACKHATCH        (16)

#define OA_HATCHTHICK       (21)
#define OA_HATCHPITCH       (22)

#define OA_HOLLOW           (25)
#define OA_SHADOW           (26)

#define OA_TEXTCOLOR        (31)
#define OA_TEXTBGCOLOR      (32)

#define OA_WIDTH            (41)
#define OA_HEIGHT           (42)
#define OA_DEPTH            (43)

#define OA_LINKSTYLE        (51)
#define OA_LINKMAP          (52)

#define OA_PEAK             (70)
#define OA_PEAKROTATE       (71)
#define OA_CONCAVE          (72)
#define OA_RAD              (75)
#define OA_LENGTH           (76)
#define OA_IMARGIN          (77)
#define OA_GIMARGIN         (78)
#define OA_ROTATE           (79)
#define OA_TEXTROTATE       (80)

#define OA_DECO             (90)
#define OA_DECOCOLOR        (91)

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
#define OA_CENTHEADPOS      (215)

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

#define OA_MOVETO           (350)
#define OA_RMOVETO          (351)
#define OA_LINETO           (352)
#define OA_RLINETO          (353)
#define OA_CURVETO          (354)
#define OA_RCURVETO         (355)
#define OA_CLOSE            (359)
#define OA_LINE             (360)


#define OA_PORT             (402)
#define OA_STARBOARD        (403)

#define OA_BGSHAPE          (501)

#define OA_KEEPDIR          (701)
#define OA_NOEXPAND         (703)

#define OA_MARKBB           (711)
#define OA_MARKPATH         (712)
#define OA_MARKNODE         (713)
#define OA_MARKPITCH        (714)
#define OA_MARKGUIDE        (715)

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

#define OA_BWCIR                    (1010)
#define OA_BWHCIR                   (1011)
#define OA_BWRCIR                   (1012)
#define OA_BWBOX                    (1020)
#define OA_BWXSS                    (1030)
#define OA_BWPLS                    (1040)

extern apair_t objattr_ial[];


#ifndef CH_OBJ_LEN
#define CH_OBJ_LEN (100)
#endif

#ifndef OBJUNIT
#define OBJUNIT (10000)
#endif






#ifndef MAXBRANCH
#define MAXBRANCH   (8)
#endif

typedef struct {
    char *ssval;
    int   ssopt;
} sstr;

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
    double arrowcentheadpos;

    int   backcolor;
    int   backhatch;

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
    int   concave;
    int   rotateval;
    int   rad;
    int   length; /* for line family */

    int   sepcurdir;
    int   sepx1, sepy1, sepx2, sepy2;

    int   gsepx1, gsepy1, gsepx2, gsepy2;

    char  *aat;
    char  *awith;
    char  *afrom;
    char  *ato;

    int    hasfrom;
    int    hasto;

    int   originalshape;

    varray_t *ssar;         /* surface string ; text */
    int   textrotate;

    char  *portstr;
    char  *boardstr;

    varray_t *segopar;      /* path segment operation */
    varray_t *segar;        /* path segment outline */
    varray_t *seghar;       /* path segment hollow */

    int    iarg1;
    int    iarg2;
    char  *carg1;
    char  *carg2;

    char  *deco;
    int    decocolor;

    char  *bgshape;

    void  *linkfore;
    void  *linkback;
    int    linkstyle;
    char  *linkmap;

    int    arrowevery;
    int    keepdir;
    int    noexpand;

    int    markbb;
    int    markpath;
    int    marknode;
    int    markpitch;
    int    markguide;
    int    hollow;
    int    shadow;

    int    laneorder;
    int    lanenum;
    int    lanegapv;
    int    lanegaph;

    char   *note[PO_MAX];
}; 

typedef struct _ch {
    char name[BUFSIZ];

    /* registers */
    int  dir;
    int  thickmode;
    int  thick;
    int  scale;
    int  x, y;

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


#endif /* _TANGY_OBJ_H_ */


