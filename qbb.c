/*
 * structure for bounding box
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "qbb.h"

static int qbb_serial=0;

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
    fprintf(fp, "qbb %p (id %d) %c [%d %d %d %d] %d\n",
        qb, qb->qbbid, c==1 ? 'I' : '-',
        qb->lx, qb->by, qb->rx, qb->ty, qb->cc);
    return 0;
}

int
qbb_reset(qbb_t *qb)
{
#if 0
    fprintf(stderr, "%s: b ", __func__);
    if(qb) qbb_fprint(stderr, qb);
#endif

    qb->rx = qb->ty = -(INT_MAX-1);
    qb->lx = qb->by =   INT_MAX;
    qb->cc = 0;

#if 0
    fprintf(stderr, "%s: a ", __func__);
    if(qb) qbb_fprint(stderr, qb);
#endif

    return 0;
}

int
qbb_reset_wid(qbb_t *qb)
{
    if(!qb) {
        return -1;
    }
    qbb_reset(qb);
    qb->qbbid = ++qbb_serial;

#if 0
    fprintf(stderr, "%s: a ", __func__);
    if(qb) qbb_fprint(stderr, qb);
#endif

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
    qbb_reset(rv);
    rv->qbbid = ++qbb_serial;

    return rv;
}

int
qbb_setbb(qbb_t *qb, int x1, int y1, int x2, int y2)
{
    if(!qb) {
        return -1;
    }
    if(x1>x2)   { qb->rx = x1; qb->lx = x2; }
    else        { qb->lx = x1; qb->rx = x2; }
    if(y1>y2)   { qb->ty = y1; qb->by = y2; }
    else        { qb->by = y1; qb->ty = y2; }

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

#if 0
static
int
_umo(int lv, int hv, int av)
{
    if(av<lv)       {   return QBB_M_U; }
    else if(av<=hv) {   return QBB_M_M; }
    else            {   return QBB_M_O; }
}
#endif

#define _umo(lv,hv,av) \
    (((av)<(lv)? QBB_M_U : ((av)<=(hv) ? QBB_M_M : QBB_M_O)))

int
qbb_isinside(qbb_t *q, int cx, int cy)
{
    int mx, my;

    mx = _umo(q->lx, q->rx, cx);
    my = _umo(q->by, q->ty, cy);

    return (mx<<4) | my;
}


int
Xqbb_isinside(qbb_t *q, int cx, int cy)
{
    int mx, my;

    mx = 0;
    if(cx<=q->lx)       { mx |= QBB_M_U; }
    else if(cx<=q->rx)  { mx |= QBB_M_M; }
    else                { mx |= QBB_M_O; }
    my = 0;
    if(cy<=q->by)       { my |= QBB_M_U; }
    else if(cy<=q->ty)  { my |= QBB_M_M; }
    else                { my |= QBB_M_O; }

    return (mx<<4) | my;
}

int
Yqbb_isinside(qbb_t *q, int cx, int cy)
{
    int rr;
    rr = 0;
    if(q->lx<=cx && cx<=q->rx && q->by<=cy && cy<=q->ty) {
        rr = 1;
    }
    return rr;
}


#define QWRAP(u)    ((u)==0x12||(u)==0x14||(u)==0x22||(u)==0x24)

int
ccomp(const void *a, const void *b)
{
    return *((int*)a) - *((int*)b);
}

int
qbb_overwrap(qbb_t *qa, qbb_t *qb, qbb_t *qc)
{
    int rv;
    int m;
    int xm, ym;

    fprintf(stdout, "%s: %d vs %d\n",
        __func__, qa->qbbid, qb->qbbid);

    rv = -1;
    qbb_reset(qc);
        
    m = 0;
    m |= _umo(qa->lx, qa->rx, qb->lx) << 12;
    m |= _umo(qa->lx, qa->rx, qb->rx) <<  8;
    m |= _umo(qa->by, qa->ty, qb->by) <<  4;
    m |= _umo(qa->by, qa->ty, qb->ty);

    xm = (m & 0xff00) >> 8;
    ym = (m & 0x00ff);
    if(QWRAP(xm) && QWRAP(ym)) {
        int xs[4];
        int ys[4];

        fprintf(stdout, "WRAP!\n");

        xs[0] = qa->lx; xs[1] = qa->rx;
        xs[2] = qb->lx; xs[3] = qb->rx;
        qsort(xs, sizeof(int), 4, ccomp);
        qc->lx = xs[1]; qc->rx = xs[2];

        ys[0] = qa->by; ys[1] = qa->ty;
        ys[2] = qb->by; ys[3] = qb->ty;
        qsort(ys, sizeof(int), 4, ccomp);
        qc->by = ys[1]; qc->ty = ys[2];

        rv = 1;
    }
    else {
        rv = 0;
    }

    return rv;
}

#ifdef QBB_TEST

int
boxtestchunk(int sf, int lx, int by, int rx, int ty, int n, qbb_t **qb)
{
    int i;
    int x, y;
    int ik;
    char line[BUFSIZ];
    

    fprintf(stdout, "     ");
        for(x=lx;x<=rx;x++) {
            fprintf(stdout, "%4d ", x);
        }
    fprintf(stdout, "\n");
    for(y=ty;y>=by;y--) {
            fprintf(stdout, "%4d ", y);
        memset(line, ' ', sizeof(line));
        for(x=lx;x<=rx;x++) {
            for(i=0;i<n;i++) {
                ik = qbb_isinside(qb[i], x, y);
                if(ik==0x22) {
                    if(line[x-lx]==' ') {
                        line[x-lx] = '1';
                    }
                    else {
                        line[x-lx]++;
                    }
                }
            }
        }
        line[rx-lx+1] = '\0';
#if 0
        for(x=lx;x<=rx;x++) {
#endif
        for(x=0;x<=rx-lx;x++) {
            fprintf(stdout, "   %c ", line[x]);
        }
#if 0
        fprintf(stdout, "%s\n", line);
#endif
        fprintf(stdout, "\n");
    }

    return 0;
}

int
boxtest(qbb_t *qb, int sf, int lx, int by, int rx, int ty)
{
    int x, y;
    int ik;

    fprintf(stdout, "     ");
        for(x=lx;x<=rx;x++) {
            fprintf(stdout, "%4d ", x);
        }
    fprintf(stdout, "\n");
    for(y=ty;y>=by;y--) {
            fprintf(stdout, "%4d ", y);
        for(x=lx;x<=rx;x++) {
            ik = qbb_isinside(qb, x, y);
            if(sf>0) {
                fprintf(stdout, " %02xH ", ik);
            }
            else {
                fprintf(stdout, " %3s ", (ik==0x22 ? "I" : "."));
            }
        }
        fprintf(stdout, "\n");
    }
    return 0;
}


int
main()
{
    int ik, ik2;
    qbb_t qa, qb, qc;
    qbb_t qo;

    memset(&qa, 0xaa, sizeof(qa));
    memset(&qb, 0xbb, sizeof(qb));
    memset(&qc, 0xcc, sizeof(qc));
    memset(&qo, 0x55, sizeof(qo));

    qbb_reset_wid(&qa);
    qbb_mark(&qa,  5,  7);
    qbb_mark(&qa,  7,  9);

    qbb_reset_wid(&qb);
    qbb_mark(&qb,  8, 10);
    qbb_mark(&qb, 10, 12);

    qbb_reset_wid(&qc);
    qbb_mark(&qc,  6,  8);
    qbb_mark(&qc,  9, 11);

    qbb_fprint(stdout, &qa);
    qbb_fprint(stdout, &qb);
    qbb_fprint(stdout, &qc);

#if 0
    boxtest(&qa, 0, 4, 6, 11, 13);
    boxtest(&qb, 0, 4, 6, 11, 13);
    boxtest(&qc, 0, 4, 6, 11, 13);
#endif
    {
        qbb_t **bch;

        bch = (qbb_t**)malloc(sizeof(qbb_t*)*10);
        bch[0] = &qa;
        bch[1] = &qb;
        bch[2] = &qc;
        bch[3] = &qo;
        boxtestchunk(0, 4, 6, 11, 13, 3, bch);
    }

    qbb_reset_wid(&qo);

    ik = qbb_overwrap(&qa, &qc, &qo);
    fprintf(stdout, "ik overwrap ? %5d %04xH\n", ik, ik);
    if(ik>0) {
        qbb_fprint(stdout, &qo);
    }

    ik = qbb_overwrap(&qa, &qb, &qo);
    fprintf(stdout, "ik overwrap ? %5d %04xH\n", ik, ik);
    if(ik>0) {
        qbb_fprint(stdout, &qo);
    }

    ik = qbb_overwrap(&qb, &qa, &qo);
    fprintf(stdout, "ik overwrap ? %5d %04xH\n", ik, ik);
    if(ik>0) {
        qbb_fprint(stdout, &qo);
    }

    ik = qbb_overwrap(&qb, &qc, &qo);
    fprintf(stdout, "ik overwrap ? %5d %04xH\n", ik, ik);
    if(ik>0) {
        qbb_fprint(stdout, &qo);
    }

}
#endif /* QBB_TEST */
