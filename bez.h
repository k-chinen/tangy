#ifndef _TANGY_BEZ_H_
#define _TANGY_BEZ_H_

double
_bez_pos(double *x, double *y, double t,
double x1, double y1, double x2, double y2,
double x3, double y3, double x4, double y4);

int
_bez_mark(qbb_t *qb, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

#endif/*_TANGY_BEZ_H_*/
