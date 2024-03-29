#include <stdio.h>

#include "alist.h"
#include "word.h"

#include "obj.h"
#include "gv.h"
#include "seg.h"
#include "chas.h"
#include "notefile.h"
#include "font.h"
#include "tx.h"
#include "a.h"
#if 0
#endif
#include "xns.h"
#include "put.h"
#include "bez.h"



int
__solve_dir(ns *xns, ob *u, varray_t *opar,
    int X, int *ang)
{
    int    i;
    segop *e;
    int    ik;

    Echo("%s: enter\n", __func__);

    for(i=0;i<opar->use;i++) {
        e = (segop*)opar->slot[i];
        if(!e)
            continue;
        if(!e->val || !e->val[0]) {
            continue;
        }
        switch(e->cmd) {
        case OA_DIR:
            *ang = atoi(e->val);
            break;
        }
    }

    Echo("%s: leave\n", __func__);

    return 0;
}


int
_extract_chchain(varray_t *la, ns *xns, ob *oa)
{
    ch *ca;
    ch *pca;

#if 0
    Echo("%s: xns %p oa %p\n", __func__, xns, oa);
#endif

    if(oa) {
        varray_entrysprintfunc(la, ch_sprintfoid);

        ca = oa->behas;

        while(1) {
            varray_push(la, ca);
            pca = ca->parent;
#if 0
            Echo("  ca %p pca %p\n", ca, pca);
#endif
            if(pca==NULL) {
                break;
            }
#if 0
            Echo("         ca %p %d\n",  ca,  ca->qob->oid);
            Echo("        pca %p %d\n", pca, pca->qob->oid);
#endif
            ca = pca;
        }

#if 0
        varray_fprint(stdout, la);
#endif

    }

    return 0;
}

int
_maxcommon(ns *xns, ob *oa, ob *ob)
{
    varray_t *la, *lb;
    int       i;
    ch       *cmch;
    ch       *xa, *xb;

    Echo("%s: xns %p oa %p ob %p\n", __func__, xns, oa, ob);

    la = lb = NULL;
    cmch = NULL;

    if(oa) {
        la = varray_new();
        varray_entrysprintfunc(la, ch_sprintfoid);
        _extract_chchain(la, xns, oa);
        varray_reverse(la);
    }
    if(ob) {
        lb = varray_new();
        varray_entrysprintfunc(lb, ch_sprintfoid);
        _extract_chchain(lb, xns, ob);
        varray_reverse(lb);
    }

    Echo("    la ");
    for(i=0;i<la->use;i++) { Echo(" %d", ((ch*)la->slot[i])->qob->oid); }
    Echo("\n");

    Echo("    lb ");
    for(i=0;i<lb->use;i++) { Echo(" %d", ((ch*)lb->slot[i])->qob->oid); }
    Echo("\n");

    Echo("    use a %d b %d\n", la->use, lb->use);
    i = 0;
    while(1) {
        if(i>=la->use) {
            break;
        }
        if(i>=lb->use) {
            break;
        }
        xa = la->slot[i];
        xb = lb->slot[i];
        Echo("      i %d | a addr %p oid %d | b addr %p oid %d\n",
            i, xa, xa->qob->oid, xb, xb->qob->oid);
        if(xa->qob->oid != xb->qob->oid) {
            break;
        }
        cmch = xa;
        
        i++;
    }

    Echo(" cmch %p %d\n", cmch, cmch->qob->oid);
    return cmch->qob->oid;
}


int
__solve_fandt(ns *xns, ob *u, varray_t *opar,
    int X, int *sx, int *sy, int *ex, int *ey)
{
    int    rv;
    int    i;
    segop *e;
    int    ik;
    ob    *obf, *obt;

    Echo("%s: enter oid %d\n", __func__, u->oid);
    Echo("%s: 0 sx,y %d,%d ex,y %d,%d\n", __func__, *sx, *sy, *ex, *ey);

    rv  = 0;

    obf = obt = NULL;

#if 1
    *sx = u->gsx;
    *sy = u->gsy;
    *ex = u->gex;
    *ey = u->gey;

    Echo("%s: 1 sx,y %d,%d ex,y %d,%d\n", __func__, *sx, *sy, *ex, *ey);
#endif

    for(i=0;i<opar->use;i++) {
        e = (segop*)opar->slot[i];
        if(!e)
            continue;
        if(!e->val || !e->val[0]) {
            continue;
        }
        switch(e->cmd) {
        case OA_FROM:
            ik = _ns_find_objposP(xns, u, e->val, X, sx, sy);
            Echo("  FROM ik %d\n", ik);
            if(ik<0) {
                printf("ERROR not found label-'%s' as FROM\n", e->val);
                exit(11);
            }
#if 1
            {
            int dx, dy;
            obf = _ns_find_objP(xns, e->val, &dx, &dy);
            }
#endif
            rv += 10;
            break;
        case OA_TO:
            ik = _ns_find_objposP(xns, u, e->val, X, ex, ey);
            Echo("  TO   ik %d\n", ik);
            if(ik<0) {
                printf("ERROR not found label-'%s' as TO\n", e->val);
                exit(11);
            }
#if 1
            {
            int dx, dy;
            obt = _ns_find_objP(xns, e->val, &dx, &dy);
            }
#endif
            rv += 100;
            break;
        }
    }

#if 1
    Echo(" obf %p obt %p\n", obf, obt);
    if(obf && obt) {
        _maxcommon(xns, obf, obt);
    }
#endif

    Echo("%s: 9 sx,y %d,%d ex,y %d,%d\n", __func__, *sx, *sy, *ex, *ey);
    Echo("%s: leave\n", __func__);

    return rv;
}



int
solve_curveself_points(ob *xu, ns *xns,
    double *pmu, double *pmv,
    int *p1x, int *p1y,
    int *p2x, int *p2y,
    int *p3x, int *p3y,
    int *p4x, int *p4y)
{
    int    aw, ah;
    int    x1, x2, y1, y2;
    int    cx, cy, tx, ty;
    int    xxth;
    double th;
    double ph;
    double mu, mv;
    double d;
    double r;
    double q;
    int    c1;
    int    c2;
    int    ux, uy, vx, vy;
    int    px, py, qx, qy;
    qbb_t bez_bb;

    ph = xu->cob.bulge*rf;
    c1 = c2 = xu->cob.backchop;

P;
#if 1
Echo("%s: enter\n", __func__);
Echo("%s: enter segopar %p\n", __func__, xu->cob.segopar);
Echo("%s: ph %f (%f) c1 %d c2 %d\n",
    __func__, ph, ph/rf, c1, c2);
#endif

    __solve_fandt(xns, xu, xu->cob.segopar, 1, &x1, &y1, &x2, &y2);
    x2 = x1;
    y2 = y1;

Echo("%s: ? FROM %d,%d TO %d,%d\n", __func__, x1, y1, x2, y2);

    cx = (x1+x2)/2;
    cy = (y1+y2)/2;
Echo(" cx,y %d,%d\n", cx, cy);

    d  = xu->cob.rad*2;

    r  = d/2;
    q  = r/cos(ph);
Echo(" d %f r %f q %f\n", d, r, q);

    __solve_dir(xns, xu, xu->cob.segopar, 1, &xxth);
    th = xxth*rf;

    /*
     *             + px,py
     *            / \
     *           /   +
     *    ux,uy +     \
     *         /   ____+ qx,qy
     *        / __+ vx,vy
     * x1,y1 +--
     */

    mu = th + ph;
    mv = th - ph;

    tx = x1+d*cos(th);
    ty = y1+d*sin(th);
    px = x1+q*cos(mu);
    py = y1+q*sin(mu);
    qx = x1+q*cos(mv);
    qy = y1+q*sin(mv);
    if(c1>0) {
        ux = x1+c1*cos(mu);
        uy = y1+c1*sin(mu);
    }
    else {
        ux = x1;
        uy = y1;
    }
    if(c2>0) {
        vx = x2+c2*cos(mv);
        vy = y2+c2*sin(mv);
    }
    else {
        vx = x2;
        vy = y2;
    }

Echo(" th %.3f (%.1f) ph %.3f (%.1f) mu %.3f (%.1f) mv %.3f (%.1f)\n",
th, th/rf, ph, ph/rf, mu, mu/rf, mv, mv/rf);

#if 0
Echo(" ux,y %d,%d tx,y %d,%d vx,y %d,%d\n",
ux, uy, tx, ty, vx, vy);
#endif
Echo(" ux,y %d,%d px,y %d,%d qx,y %d,%d vx,y %d,%d\n",
ux, uy, px, py, qx, qy, vx, vy);

#if 0
    _bez_mark(&bez_bb, ux, uy, px, py, qx, qy, vx, vy);
    qbb_fprint(stdout, &bez_bb);

    *_lx = bez_bb.lx;
    *_by = bez_bb.by;
    *_rx = bez_bb.rx;
    *_ty = bez_bb.ty;
#endif

    *pmu = mu;
    *pmv = mv;
    *p1x = ux;
    *p1y = uy;
    *p2x = px;
    *p2y = py;
    *p3x = qx;
    *p3y = qy;
    *p4x = vx;
    *p4y = vy;

    return 0;
}


/* pure - no xu and xns */
int
solve_curve_points_pure(
    double xbulge, int c1, int c2,
    double *pmu, double *pmv,
    int *p1x, int *p1y,
    int *p2x, int *p2y,
    int *p3x, int *p3y,
    int *p4x, int *p4y)
{
    int    x1, x2, y1, y2;
    int    cx, cy, tx, ty;
    double th;
    double ph;
    double mu, mv;
    double d;
    double r;
    double q;
    int    ux, uy, vx, vy;

    ph = xbulge*rf;

P;
Echo("%s: enter\n", __func__);
Echo("%s: ph %f (%f) c1 %d c2 %d\n",
    __func__, ph, ph/rf, c1, c2);
#if 0
#endif

    x1 = *p1x;
    y1 = *p1y;
    x2 = *p4x;
    y2 = *p4y;

    cx = (x1+x2)/2;
    cy = (y1+y2)/2;
Echo(" cx,y %d,%d\n", cx, cy);


    d  = SQRT_2DD_I2D(x1,y1,x2,y2);
    r  = d/2;
    q  = r/cos(ph);
Echo(" d %f r %f q %f\n", d, r, q);

    th = (double)(atan2((y2-y1),(x2-x1)));

    /*
     *             + tx,ty
     *            / \
     *           /   + vx,vy
     *    ux,uy +     \
     *         /:ph ___+ x2,y2
     *        / :__+ cx,cy
     * x1,y1 +-- :th
     *
     *      mu = th + ph
     */

    mu = th + ph;
    mv = th + M_PI - ph;

    tx = x1+q*cos(mu);
    ty = y1+q*sin(mu);
#if 0
    printf("curve x1,y1 %d,%d x2,y2 %d,%d -> gx,y %d,%d\n",
        x1, y1, x2, y2, xu->gx, xu->gy);
#endif
    if(c1>0) {
        ux = x1+c1*cos(mu);
        uy = y1+c1*sin(mu);
    }
    else {
        ux = x1;
        uy = y1;
    }
    if(c2>0) {
        vx = x2+c2*cos(mv);
        vy = y2+c2*sin(mv);
    }
    else {
        vx = x2;
        vy = y2;
    }

Echo(" th %.3f (%.1f) ph %.3f (%.1f) mu %.3f (%.1f) mv %.3f (%.1f)\n",
th, th/rf, ph, ph/rf, mu, mu/rf, mv, mv/rf);

Echo(" ux,y %d,%d tx,y %d,%d vx,y %d,%d\n",
ux, uy, tx, ty, vx, vy);

    *pmu = mu;
    *pmv = mv;
    *p1x = ux;
    *p1y = uy;
    *p2x = tx;
    *p2y = ty;
    *p3x = tx;
    *p3y = ty;
    *p4x = vx;
    *p4y = vy;

    return 0;
}

int
solve_curve_points(ob *xu, ns *xns,
    double *pmu, double *pmv,
    int *p1x, int *p1y,
    int *p2x, int *p2y,
    int *p3x, int *p3y,
    int *p4x, int *p4y)
{
    int    aw, ah;
    int    x1, x2, y1, y2;
    int    cx, cy, tx, ty;
    double th;
    double ph;
    double mu, mv;
    double d;
    double r;
    double q;
    int    c1;
    int    c2;
    int    ux, uy, vx, vy;
    qbb_t bez_bb;

    ph = xu->cob.bulge*rf;
    c1 = xu->cob.backchop;
    c2 = xu->cob.forechop;

P;
#if 0
Echo("%s: enter\n", __func__);
Echo("%s: ph %f (%f) c1 %d c2 %d\n",
    __func__, ph, ph/rf, c1, c2);
#endif

#if 1
    __solve_fandt(xns, xu, xu->cob.segopar, 1, &x1, &y1, &x2, &y2);
#endif

#if 0
    if(!xu->cob.originalshape) {
        x1 = xu->csx;
        y1 = xu->csy;
        x2 = xu->cex;
        y2 = xu->cey;
    }
    else {
        __solve_fandt(xns, xu, xu->cob.segopar, 1, &x1, &y1, &x2, &y2);
    }
#endif


#if 1
Echo("%s: %d - FROM %d,%d TO %d,%d\n", __func__, xu->oid,
    xu->csx, xu->csy, xu->cex, xu->cey);
Echo("%s: %d g FROM %d,%d TO %d,%d\n", __func__, xu->oid,
    xu->gsx, xu->gsy, xu->gex, xu->gey);
#endif
Echo("%s: %d ? FROM %d,%d TO %d,%d\n", __func__, xu->oid,
    x1, y1, x2, y2);

    cx = (x1+x2)/2;
    cy = (y1+y2)/2;
Echo(" cx,y %d,%d\n", cx, cy);


    d  = SQRT_2DD_I2D(x1,y1,x2,y2);
    r  = d/2;
    q  = r/cos(ph);
Echo(" d %f r %f q %f\n", d, r, q);

    th = (double)(atan2((y2-y1),(x2-x1)));

    /*
     *             + tx,ty
     *            / \
     *           /   + vx,vy
     *    ux,uy +     \
     *         /:ph ___+ x2,y2
     *        / :__+ cx,cy
     * x1,y1 +-- :th
     *
     *      mu = th + ph
     */

    mu = th + ph;
    mv = th + M_PI - ph;

    tx = x1+q*cos(mu);
    ty = y1+q*sin(mu);
#if 0
    xu->gx = tx;
    xu->gy = ty;
#endif
#if 0
    printf("curve oid %d x1,y1 %d,%d x2,y2 %d,%d -> gx,y %d,%d\n",
        xu->oid, x1, y1, x2, y2, xu->gx, xu->gy);
#endif
    if(c1>0) {
        ux = x1+c1*cos(mu);
        uy = y1+c1*sin(mu);
    }
    else {
        ux = x1;
        uy = y1;
    }
    if(c2>0) {
        vx = x2+c2*cos(mv);
        vy = y2+c2*sin(mv);
    }
    else {
        vx = x2;
        vy = y2;
    }

Echo(" th %.3f (%.1f) ph %.3f (%.1f) mu %.3f (%.1f) mv %.3f (%.1f)\n",
th, th/rf, ph, ph/rf, mu, mu/rf, mv, mv/rf);

Echo(" ux,y %d,%d tx,y %d,%d vx,y %d,%d\n",
ux, uy, tx, ty, vx, vy);

#if 0
    _bez_mark(&bez_bb, ux, uy, tx, ty, tx, ty, vx, vy);
    qbb_fprint(stdout, &bez_bb);

    *_lx = bez_bb.lx;
    *_by = bez_bb.by;
    *_rx = bez_bb.rx;
    *_ty = bez_bb.ty;
#endif

    *pmu = mu;
    *pmv = mv;
    *p1x = ux;
    *p1y = uy;
    *p2x = tx;
    *p2y = ty;
    *p3x = tx;
    *p3y = ty;
    *p4x = vx;
    *p4y = vy;

    return 0;
}

int
MARK_bcurveselfX(ob *xu, ns *xns,
    int *_sx, int *_sy, int *_ex, int *_ey,
    int *_lx, int *_by, int *_rx, int *_ty)
{

    int     ux, uy, vx, vy;
    int     px, py, qx, qy;
    double  mu, mv;
    int     ik;
    qbb_t   bez_bb;

    ik = solve_curveself_points(xu, xns,
        &mu, &mv, &ux, &uy, &px, &py, &qx, &qy, &vx, &vy);

    _bez_mark(&bez_bb, ux, uy, px, py, qx, qy, vx, vy);
#if 0
    qbb_fprint(stdout, &bez_bb);
#endif
    
    *_sx = ux;
    *_sy = uy;
    *_ex = vx;
    *_ey = vy;

    *_lx = bez_bb.lx;
    *_by = bez_bb.by;
    *_rx = bez_bb.rx;
    *_ty = bez_bb.ty;

    return 0;
}


int
MARK_bcurveself(ob *xu, ns *xns,
    int *_lx, int *_by, int *_rx, int *_ty)
{

    int     ux, uy, vx, vy;
    int     px, py, qx, qy;
    double  mu, mv;
    int     ik;
    qbb_t   bez_bb;

    ik = solve_curveself_points(xu, xns,
        &mu, &mv, &ux, &uy, &px, &py, &qx, &qy, &vx, &vy);

    _bez_mark(&bez_bb, ux, uy, px, py, qx, qy, vx, vy);
#if 0
    qbb_fprint(stdout, &bez_bb);
#endif

    *_lx = bez_bb.lx;
    *_by = bez_bb.by;
    *_rx = bez_bb.rx;
    *_ty = bez_bb.ty;

    return 0;
}


int
MARK_bcurve(ob *xu, ns *xns,
    int *_lx, int *_by, int *_rx, int *_ty)
{
    int     ux, uy, vx, vy;
    int     tx1, ty1, tx2, ty2;
    double  mu, mv;
    int     ik;
    qbb_t   bez_bb;

    ik = solve_curve_points(xu, xns,
        &mu, &mv, &ux, &uy, &tx1, &ty1, &tx2, &ty2, &vx, &vy);

    _bez_mark(&bez_bb, ux, uy, tx1, ty1, tx2, ty2, vx, vy);
#if 0
    qbb_fprint(stdout, &bez_bb);
#endif

    *_lx = bez_bb.lx;
    *_by = bez_bb.by;
    *_rx = bez_bb.rx;
    *_ty = bez_bb.ty;

    return 0;
}


int
MARK_bcurveX(ob *xu, ns *xns,
    int *_sx, int *_sy, int *_ex, int *_ey,
    int *_lx, int *_by, int *_rx, int *_ty)
{
    int     ux, uy, vx, vy;
    int     tx1, ty1, tx2, ty2;
    double  mu, mv;
    int     ik;
    qbb_t   bez_bb;

    ik = solve_curve_points(xu, xns,
        &mu, &mv, &ux, &uy, &tx1, &ty1, &tx2, &ty2, &vx, &vy);

    _bez_mark(&bez_bb, ux, uy, tx1, ty1, tx2, ty2, vx, vy);
#if 0
    qbb_fprint(stdout, &bez_bb);
#endif
    
    *_sx = ux;
    *_sy = uy;
    *_ex = vx;
    *_ey = vy;

    *_lx = bez_bb.lx;
    *_by = bez_bb.by;
    *_rx = bez_bb.rx;
    *_ty = bez_bb.ty;

    return 0;
}


