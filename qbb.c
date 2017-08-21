/*
 * structure for bounding box
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "qbb.h"

#if 0
typedef struct {
    int qbbid;
    int cc;
    int lx, by, rx, ty;
} qbb_t;
#endif

static int qbb_serial=0;

int
qbb_reset(qbb_t *qb)
{
    qb->rx = qb->ty = -(INT_MAX-1);
    qb->lx = qb->by =   INT_MAX;
    qb->cc = 0;

    return 0;
}

qbb_t*
qbb_new()
{
    qbb_t *rv;
    rv = (qbb_t*)malloc(sizeof(qbb_t));
    if(!rv) {
        return NULL;
    }
    rv->qbbid = ++qbb_serial;
    qbb_reset(rv);

    return rv;
}

int
qbb_isinit(qbb_t *qb)
{
    int rv;
    rv = 0;
    if((qb->rx == -(INT_MAX-1)) && 
        (qb->ty == -(INT_MAX-1)) &&
         (qb->lx == INT_MAX) &&
          (qb->by == INT_MAX)) {
        rv = 1;
    }
    return rv;
}

int
qbb_fprint(FILE *fp, qbb_t *qb)
{
    int c;
    if(!qb) {
        return -1;
    }
    c = qbb_isinit(qb);
    fprintf(fp, "qbb (id %d) %c [%d %d %d %d] %d\n",
        qb->qbbid, c==1 ? 'I' : '-',
        qb->lx, qb->by, qb->rx, qb->ty, qb->cc);
    return 0;
}

int
qbb_mark(qbb_t *qb, int tx, int ty)
{
    if(!qb) {
        return -1;
    }
    if(tx<qb->lx) qb->lx = tx;
    if(tx>qb->rx) qb->rx = tx;
    if(ty<qb->by) qb->by = ty;
    if(ty>qb->ty) qb->ty = ty;

    return 0;
}

int
qbb_getcenter(qbb_t *qb, int *rx, int *ry)
{
    *rx = (qb->rx+qb->lx)/2;
    *ry = (qb->ty+qb->by)/2;
    return 0;
}

int
qbb_getsize(qbb_t *qb, int *rw, int *rh)
{
    *rw = (qb->rx-qb->lx);
    *rh = (qb->ty-qb->by);;
    return 0;
}
