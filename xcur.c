static
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

static        
int
__solve_fandt(ns *xns, ob *u, varray_t *opar,
    int X, int *sx, int *sy, int *ex, int *ey)
{
    int    i;
    segop *e;
    int    ik;

    Echo("%s: enter\n", __func__);
    Echo("%s: sx,y %d,%d ex,y %d,%d\n", __func__, *sx, *sy, *ex, *ey);

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
            break;
        case OA_TO:
            ik = _ns_find_objposP(xns, u, e->val, X, ex, ey);
            Echo("  TO   ik %d\n", ik);
            if(ik<0) {
                printf("ERROR not found label-'%s' as TO\n", e->val);
                exit(11);
            }
            break;
        }
    }

    Echo("%s: sx,y %d,%d ex,y %d,%d\n", __func__, *sx, *sy, *ex, *ey);
    Echo("%s: leave\n", __func__);

    return 0;
}

int
EST_curveself(FILE *fp, ob *xu, ns *xns, int *_lx, int *_by, int *_rx, int *_ty)
{
#if 0
    int r;
#endif
    int aw, ah;
    int x1, x2, y1, y2;
    int cx, cy, tx, ty;
    int xxth;
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

    qbb_t  bb;

    ph = xu->cob.bulge*rf;
    c1 = c2 = xu->cob.backchop;

P;
#if 0
Echo("%s: enter\n", __func__);
Echo("%s: ph %f (%f) c1 %d c2 %d\n",
    __func__, ph, ph/rf, c1, c2);
#endif

    if(xu->cob.hasfrom) {
#if 0
        varray_fprint(stdout, xu->cob.segopar);
#endif

        __solve_fandt(xns, xu, xu->cob.segopar, 1, &x1, &y1, &x2, &y2);
        x2 = x1;
        y2 = y1;

Echo("%s: ? FROM %d,%d TO %d,%d\n", __func__,
        x1, y1, x2, y2);

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
         *             + tx,ty
         *            / \
         *           /   + vx,vy
         *    ux,uy +     \
         *         /   ____+ x2,y2
         *        / __+ cx,cy
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

Echo(" ux,y %d,%d px,y %d,%d qx,y %d,%d vx,y %d,%d\n",
    ux, uy, px, py, qx, qy, vx, vy);

        qbb_reset(&bb);
        qbb_mark(&bb, ux, uy);
        qbb_mark(&bb, px, py);
        qbb_mark(&bb, tx, ty);
        qbb_mark(&bb, qx, qy);
        qbb_mark(&bb, vx, vy);
        qbb_mark(&bb, x1, y1);
        qbb_mark(&bb, x2, y2);
        qbb_mark(&bb, cx, cy);

Echo("%s: BB lx %d by %d rx %d ty %d\n", 
    __func__, bb.lx, bb.by, bb.rx, bb.ty);
        *_lx = bb.lx;
        *_by = bb.by;
        *_rx = bb.rx;
        *_ty = bb.ty;

    }
    return 1;
}



int
EST_curve(FILE *fp, ob *xu, ns *xns, int *_lx, int *_by, int *_rx, int *_ty)
{
#if 0
    int r;
#endif
    int aw, ah;
    int x1, x2, y1, y2;
    int cx, cy, tx, ty;
    double th;
    double ph;
    double mu, mv;
    double d;
    double r;
    double q;
    int    c1;
    int    c2;
    int    ux, uy, vx, vy;

    qbb_t  bb;

#if 0
    ph = ((double)M_PI)/4.0;
    c1 = objunit/3;
    c2 = objunit/4;
#endif
    ph = xu->cob.bulge*rf;
    c1 = xu->cob.backchop;
    c2 = xu->cob.forechop;

P;
#if 1
Echo("%s: enter\n", __func__);
Echo("%s: ph %f (%f) c1 %d c2 %d\n",
    __func__, ph, ph/rf, c1, c2);
#endif

    if(xu->cob.hasfrom && xu->cob.hasto) {

        varray_fprint(stdout, xu->cob.segopar);
#if 0
        varray_fprint(stdout, xu->cob.ssar);
        varray_fprint(stdout, xu->cob.segar);
        varray_fprint(stdout, xu->cob.seghar);
#endif

#if 0
        int ck;
        ik = _ns_find_objposP(xns, u, e->val, 1, &mx, &my);
#endif
        __solve_fandt(xns, xu, xu->cob.segopar, 1, &x1, &y1, &x2, &y2);

#if 0
Echo("%s: - FROM %d,%d TO %d,%d\n", __func__,
        xu->csx, xu->csy, xu->cex, xu->cey);
Echo("%s: g FROM %d,%d TO %d,%d\n", __func__,
        xu->gsx, xu->gsy, xu->gex, xu->gey);
#endif
Echo("%s: ? FROM %d,%d TO %d,%d\n", __func__,
        x1, y1, x2, y2);

        cx = (x1+x2)/2;
        cy = (y1+y2)/2;
Echo(" cx,y %d,%d\n", cx, cy);

#if 0
        d  = sqrt((double)(y2-y1)*(y2-y1)+(double)(x2-x1)*(x2-x1));
#endif
        d  = SQRT_I2D(x1,y1,x2,y2);
        r  = d/2;
        q  = r/cos(ph);
Echo(" d %f r %f q %f\n", d, r, q);

        th = (double)(atan2((y2-y1),(x2-x1)));

        /*
         *             + tx,ty
         *            / \
         *           /   + vx,vy
         *    ux,uy +     \
         *         /   ____+ x2,y2
         *        / __+ cx,cy
         * x1,y1 +--
         */

        mu = th + ph;
        mv = th + M_PI - ph;

        tx = x1+q*cos(mu);
        ty = y1+q*sin(mu);
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

        qbb_reset(&bb);
        qbb_mark(&bb, ux, uy);
        qbb_mark(&bb, tx, ty);
        qbb_mark(&bb, vx, vy);
        qbb_mark(&bb, x1, y1);
        qbb_mark(&bb, x2, y2);
        qbb_mark(&bb, cx, cy);

Echo("%s: BB lx %d by %d rx %d ty %d\n", 
    __func__, bb.lx, bb.by, bb.rx, bb.ty);
        *_lx = bb.lx;
        *_by = bb.by;
        *_rx = bb.rx;
        *_ty = bb.ty;

    }

return 0;

}
