
#include <stdio.h>
#include <math.h>

#include "qbb.h"

int
_bez_pos(double *x, double *y, double t,
double x1, double y1, double x2, double y2,
double x3, double y3, double x4, double y4)
{
    double tp;
    tp = 1.0 - t;
    *x = t*t*t*x4 + 3 *t*t*tp*x3 + 3 *t*tp*tp*x2 + tp*tp*tp*x1;
    *y = t*t*t*y4 + 3 *t*t*tp*y3 + 3 *t*tp*tp*y2 + tp*tp*tp*y1;
    return 0;
}


int
_bez_posdir(double *x, double *y, double *aa, double t,
double x1, double y1, double x2, double y2,
double x3, double y3, double x4, double y4)
{
    double a1, a2;
    double a1a, a2a;
    double a;
    double dx, dy;
    double dt;
    double lx, ly;
    double rx, ry;
    double da;

    dt = 0.01;

    _bez_pos(&lx, &ly, t-dt,
        (double)x1, (double)y1, (double)x2, (double)y2,
        (double)x3, (double)y3, (double)x4, (double)y4);

    _bez_pos(&rx, &ry, t,
        (double)x1, (double)y1, (double)x2, (double)y2,
        (double)x3, (double)y3, (double)x4, (double)y4);

    dx = rx - lx;
    dy = ry - ly;
    a1 = atan2(dy, dx);

    lx = rx;
    ly = ry;

    _bez_pos(&rx, &ry, t+dt,
        (double)x1, (double)y1, (double)x2, (double)y2,
        (double)x3, (double)y3, (double)x4, (double)y4);

    dx = rx - lx;
    dy = ry - ly;
    a2 = atan2(dy, dx);

    /*
     * atan2 returns (-M_PI, M_PI).
     * sometime, average of straight value by atan2 makes strange.
     * for example: a1 = -3.12, a2 = 3.12, (a1+a2)/2 = 0.0
     * then, here add M_PI*2 if value is negative.
     */

    a1a = a1;
    a2a = a2;
    da  = a2 - a1;
    if(da<-M_PI || da>M_PI) {
        a1 += M_PI*2;
    }
    a = (a1+a2)/2.0;

    *x  = lx;
    *y  = ly;
    *aa = a;

#if 0
    fprintf(stderr, "%s: t %4.2f a1 %7.4f (%7.4f) a2 %7.4f (%7.4f) -> da %7.4f a %7.4f\n",
        __func__, t, a1, a1a, a2, a2a, da, a);
    fprintf(stderr, "%s: t %5.2f aa a1 %7.4f a2 %7.4f -> a %7.4f\n",
        __func__, t, a1, a2, a);
#endif

    return 0;
}


int
_bez_mark(qbb_t *qb, int x1, int y1, int x2, int y2,
    int x3, int y3, int x4, int y4)
{
    int x, y;
    double t;
    double rx, ry;
    double lx, ly;
    double dx, dy;
    double s;
    double d;
    double sumd, maxd;
    
    s = 1.0/50.0;
    sumd = 0.0;
    maxd = -1;
    
    lx = rx = (double)x1;
    ly = ry = (double)y1;

    qbb_reset(qb);
    qbb_mark(qb, x1, y1);

    for(t=0;t<=1.0;t+=s) {

        _bez_pos(&rx, &ry, t,
            (double)x1, (double)y1, (double)x2, (double)y2,
            (double)x3, (double)y3, (double)x4, (double)y4);

        d  = sqrt((rx-lx)*(rx-lx)+(ry-ly)*(ry-ly));
        if(d>maxd) maxd = d;
        sumd += d;

        x = (int)rx;
        y = (int)ry;

        qbb_mark(qb, x, y);

        lx = rx;
        ly = ry;
    }

    qbb_mark(qb, x4, y4);

    return 0;
}

