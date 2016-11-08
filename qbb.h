/*
 * structure for bounding box
 */

#ifndef _QBB_H_
#define _QBB_H_

typedef struct {
    int qbbid;
    int cc;
    int lx, by, rx, ty;
} qbb_t;

int qbb_reset(qbb_t *qb);
qbb_t *qbb_new();
int qbb_fprint(FILE *fp, qbb_t *qb);
int qbb_mark(qbb_t *qb, int tx, int ty);
int qbb_getcenter(qbb_t *qb, int *rx, int *ry);
int qbb_getsize(qbb_t *qb, int *rw, int *rh);

#endif/*_QBB_H_*/
