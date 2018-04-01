#ifndef _TANGY_XCUR_H_
#define _TANGY_XCUR_H_
int __solve_dir(ns *xns, ob *u, varray_t *opar, int X, int *ang);
int __solve_fandt(ns *xns, ob *u, varray_t *opar, int X, int *sx, int *sy, int *ex, int *ey);
int solve_curveself_points(ob *xu, ns *xns, double *pmu, double *pmv, int *p1x, int *p1y, int *p2x, int *p2y, int *p3x, int *p3y, int *p4x, int *p4y);
int solve_curve_points(ob *xu, ns *xns, double *pmu, double *pmv, int *p1x, int *p1y, int *p2x, int *p2y, int *p3x, int *p3y, int *p4x, int *p4y);
int MARK_bcurve(    ob *xu, ns *xns, int *_lx, int *_by, int *_rx, int *_ty);
int MARK_bcurveself(ob *xu, ns *xns, int *_lx, int *_by, int *_rx, int *_ty);
int MARK_bcurveX(    ob *xu, ns *xns, int *_sx, int *_sy, int *_ex, int *_ey, int *_lx, int *_by, int *_rx, int *_ty);
int MARK_bcurveselfX(ob *xu, ns *xns, int *_sx, int *_sy, int *_ex, int *_ey, int *_lx, int *_by, int *_rx, int *_ty);
#endif/*_TANGY_XCUR_H_*/
