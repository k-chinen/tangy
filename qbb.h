/*
 * structure for bounding box
 */

#ifndef _QBB_H_
#define _QBB_H_

#define QBB_M_L     (0x07)
#define QBB_M_B     (0x04)
#define QBB_M_R     (0x02)
#define QBB_M_T     (0x01)
#define QBB_M_ALL   (0x0f)

#define QBB_M_U     (0x01)
#define QBB_M_M     (0x02)
#define QBB_M_O     (0x04)

typedef struct {
    int qbbid;
    int cc;
    int lx, by, rx, ty;
} qbb_t;

int qbb_reset(qbb_t *qb);
qbb_t *qbb_new();
int qbb_setbb(qbb_t *qb, int x1, int y1, int x2, int y2);
int qbb_shift(qbb_t *qb, int mx, int my);
int qbb_fprint(FILE *fp, qbb_t *qb);
int qbb_mark(qbb_t *qb, int tx, int ty);
int qbb_getcenter(qbb_t *qb, int *rx, int *ry);
int qbb_getsize(qbb_t *qb, int *rw, int *rh);

#endif/*_QBB_H_*/
