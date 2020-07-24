
#ifndef _TANGY_SEG_H_
#define _TANGY_SEG_H_

#define SEG_SEPC         (',')
#define SEG_SEPS        (",")

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
    int x1, y1, x2, y2, x3, y3, x4, y4;
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

extern apair_t coord_ial[];
extern apair_t coord_isal[];
extern apair_t ftcoord_ial[];
extern apair_t ftcoord_isal[];

seg *seg_new();
int path_regline(varray_t *segar, int x1, int y1, int x2, int y2);
int path_regsegmoveto(varray_t *segar, int x1, int y1);
int path_regsegrmoveto(varray_t *segar, int x1, int y1);
int path_regseglineto(varray_t *segar, int x1, int y1);
int path_regsegrlineto(varray_t *segar, int x1, int y1);
int path_regsegforward(varray_t *segar, int d);
int path_regsegarc(varray_t *segar, int rad, int ang);
int path_regsegarcn(varray_t *segar, int rad, int ang);
int path_regsegcurveto(varray_t *segar, int x1, int y1, int x2, int y2, int x3, int y3);
int path_regsegrcurveto(varray_t *segar, int x1, int y1, int x2, int y2, int x3, int y3);
int path_regsegclose(varray_t *segar);
int path_regsegbwcir(varray_t *segar, int, int);
int path_regsegdir(varray_t *segar, int);

#define SEG_VISIBLE(x) \
    ((x)==OA_LINETO||(x)==OA_RLINETO||(x)==OA_LINE||\
     (x)==OA_CURVETO||(x)==OA_RCURVETO||\
     (x)==OA_RIGHT||(x)==OA_LEFT||(x)==OA_UP||(x)==OA_DOWN||\
     (x)==OA_FORWARD||(x)==OA_ARC||(x)==OA_ARCN||(x)==OA_JOIN)

#define TMP_SEG_VISIBLE(x) \
    ((x)==OA_LINETO||(x)==OA_RLINETO||(x)==OA_LINE||\
     (x)==OA_CURVETO||(x)==OA_RCURVETO||\
     (x)==OA_RIGHT||(x)==OA_LEFT||(x)==OA_UP||(x)==OA_DOWN||\
     (x)==OA_ARC||(x)==OA_ARCN||(x)==OA_JOIN)

int path_firstvisible(varray_t *segar);
int path_middlevisible(varray_t *segar);
int path_lastvisible(varray_t *segar);

#endif/*_TANGY_SEG_H_*/
