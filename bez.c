
#include <stdio.h>
#include <math.h>

#include "alist.h"
#include "qbb.h"

double
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
_bez_mark(qbb_t *qb, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
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

    return 0;
}

