
#ifndef _TANGY_SEG_H_
#define _TANGY_SEG_H_

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


seg *seg_new();
int try_regsegclose(varray_t *segar);
int try_regline(varray_t *segar, int x1, int y1, int x2, int y2);
int try_regsegmove(varray_t *segar, int x1, int y1);
int try_regsegrmove(varray_t *segar, int x1, int y1);
int try_regsegrline(varray_t *segar, int x1, int y1);
int try_regsegforward(varray_t *segar, int x1, int y1);
int try_regsegarc(varray_t *segar, int rad, int ang);
int try_regsegarcn(varray_t *segar, int rad, int ang);

#endif/*_TANGY_SEG_H_*/
