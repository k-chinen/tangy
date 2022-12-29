#include <stdio.h>

#include "varray.h"
#include "word.h"

#define CEcho   printf
#define PASS    printf("%s:%d\n", __FILE__, __LINE__);

#include "sstr.h"

#include "color.h"
#include "gv.h"
#include "obj.h"
#include "font.h"
#include "tx.h"
#include "gstr.h"
#include "bez.h"
#include "xcur.h"

#include "put.h"


int
jj(int zx, int zy, int degth, int ir, int iw,
    int *ax, int *ay, int *bx, int *by)
{
    double th;
    double ph;
    double r;
    double w;

    r = ir;
    w = iw;
    th = (M_PI*degth)/180.0;

    ph = th+M_PI/2;
    *ax = (int)(zx + r*cos(th) + w/2 *cos(ph));
    *ay = (int)(zy + r*sin(th) + w/2 *sin(ph));

    *bx = (int)(zx + r*cos(th) - w/2 *cos(ph));
    *by = (int)(zy + r*sin(th) - w/2 *sin(ph));

#if 0
    fprintf(stderr, "%s: %d,%d %d(%f) %d %d; %f %d,%d %d,%d\n",
        __func__, zx, zy, degth, th, ir, iw, ph, *ax, *ay, *bx, *by);
#endif

    return 0;
}

int
_Y_pbpoint(ns *xns, int xdir, ob *u, int *rpx, int *rpy, int *rag)
{
    int     valid=0;
    int     ik;

    double  tdir;
    double  px, py, a;
    int     ux, uy, vx, vy;
    int     tx1, ty1, tx2, ty2;
    double  mu, mv;
    double  pag, nag;
    int     pgap, bgap;
    char    astr[BUFSIZ];
    int     qsx, qsy, qex, qey;
    varray_t *sar;
    

    if(u->type==CMD_BCURVE||u->type==CMD_BCURVESELF) {
        if(u->type==CMD_BCURVE) {
            ik = solve_curve_points(u, xns,
                &mu, &mv, &ux, &uy, &tx1, &ty1, &tx2, &ty2, &vx, &vy);
        }
        else
        if(u->type==CMD_BCURVESELF) {
            ik = solve_curveself_points(u, xns,
                &mu, &mv, &ux, &uy, &tx1, &ty1, &tx2, &ty2, &vx, &vy);
        }
        else {
            abort();
        }

        Echo("%% %s: %d ik %d\n", __func__, __LINE__, ik);
        Echo("%% mu %f mv %f ux,uy %d,%d tx1,ty1 %d,%d tx2,ty2 %d,%d vx,vy %d,%d\n", mu, mv, ux, uy, tx1, ty1, tx2, ty2, vx, vy);



        ik = _bez_posdir(&px, &py, &a, 0.5,
                ux, uy, tx1, ty1, tx2, ty2, vx, vy);

        Echo("%% %s: %d ik %d\n", __func__, __LINE__, ik);

        tdir = a/rf;
        pag = tdir-90;
        nag = tdir+90;
        Echo("%% a %f, tdir %f, pag %f, nag %f\n", a, tdir, pag, nag);
    }
    else {
        sar = u->cob.segar;
        if(!sar) {
#if 1
            Echo("%% %s: oid %d no segar\n", __func__, u->oid);
#endif
            return -2;
        }
        if(sar->use!=1) {
#if 1
            Echo("%% %s: oid %d many points (%d)\n",
                __func__, u->oid, sar->use);
#endif
            if(u->cob.originalshape==0) {
                return -3;
            }
        }

        if(u->cob.originalshape==0) {
            px = (u->gsx+u->gex)/2;
            py = (u->gsy+u->gey)/2;
            Echo("%% oid %d px %f py %f\n", u->oid, px, py);
        }
        else {
            find_from(u, &qsx, &qsy);
            find_to_last(u, &qex, &qey);

            Echo("%% oid %d ox %d oy %d\n", u->oid, u->ox, u->oy);
            Echo("%% oid %d gsx %d gsy %d\n", u->oid, u->gsx, u->gsy);
            Echo("%% oid %d gex %d gey %d\n", u->oid, u->gex, u->gey);
            Echo("%% oid %d sx %d sy %d\n", u->oid, qsx, qsy);
            Echo("%% oid %d ex %d ey %d\n", u->oid, qex, qey);

            px = u->gsx+(qsx+qex)/2;
            py = u->gsy+(qsy+qey)/2;

            tdir = (180.0*atan2(qey-qsy,qex-qsx)/M_PI);
        }

        pag = tdir-90;
        nag = tdir+90;
        Echo("%% pag %f nag %f\n", pag, nag);
    }
    Echo("%% px %.2f py %.2f tdir %.2f pag %.2f nag %.2f\n",
        px, py, tdir, pag, nag);

    return 0;
}

int
_X_pbpoint(int xdir, int da, ob *u, int *px, int *py)
{
    int    rv;
    double th;
    double ph;
    int    fi;
    int    rr;
    int    kx, ky;
/*
 *     \  2
 *   +--\------+
 *   |   \     |
 * 3 |    x    | 1
 *   |         |
 *   +---------+
 *        4
 *
 *       1 r
 *   fi  2 u
 *       3 l
 *       4 d
 */

    rv = 0;
    fi = 0;
    ph = dirnormalize_positive(xdir + da);
    th = atan2(u->ht/2, u->wd/2)/rf;
    rr = sqrt( (u->ht/2)*(u->ht/2) + (u->wd/2)*(u->wd/2) );

#if 0
    fprintf(stderr, "%s: oid %d; wd x ht %d x %d, glx,rx,by,ty %d,%d,%d,%d\n",
        __func__, u->oid, u->wd, u->ht, u->glx, u->grx, u->gby, u->gty);
    fprintf(stderr, "%s: oid %d; wd x ht %d x %d, lx,rx,by,ty %d,%d,%d,%d\n",
        __func__, u->oid, u->wd, u->ht, u->lx, u->rx, u->by, u->ty);
#endif

#if 0
    kx = ( u->glx + u->grx ) / 2;
    ky = ( u->gby + u->gty ) / 2;
#endif

    if(0<=ph && ph<=th)                 { fi = 1; }
    else if(th<=ph && ph<=180-th)       { fi = 2; }
    else if(180-th<=ph && ph<=180+th)   { fi = 3; }
    else if(180+th<=ph && ph<=360-th)   { fi = 4; }
    else if(360-th<=ph)                 { fi = 1; }
    
    switch(fi) {

#if 1
    case 1: /* right side */
            rv++; *px = u->wd/2;  *py = tan(ph*rf)*(u->wd/2);
            break;
    case 3: /* left side */
            rv++; *px = -u->wd/2; *py = tan(ph*rf)*(u->wd/2);
            break;
    case 2: /* upper side */
            rv++; *py = u->ht/2;  *px = (u->ht/2)/tan(ph*rf);
            break;
    case 4: /* bottom sude */
            rv++; *py = -u->ht/2; *px = (u->ht/2)/tan(ph*rf);
            break;
#endif

#if 0
    case 1: /* right side */
            rv++; *px = u->grx; *py = ky + tan(ph*rf)*(*px-kx);
            break;
    case 3: /* left side */
            rv++; *px = u->glx; *py = ky + tan(ph*rf)*(*px-kx);
            break;
    case 2: /* upper side */
            rv++; *py = u->gty; *px = (*py-ky)/tan(ph*rf)+kx;
            break;
    case 4: /* bottom sude */
            rv++; *py = u->gby; *px = (*py-ky)/tan(ph*rf)+kx;
            break;
#endif


    default:
#if 0
        Error("normal line of oid %d does not touch bbox; ph %.2f th %.2f\n",
                u->oid, ph, th);
#endif
            Error("miss cross point oid %d, ph %.2f th %.2f\n",
                u->oid, ph, th);
            break;
    }

#if 0
    fprintf(stderr, "%s: th %.1f, ph %.1f, fi %d, rr %d, kx %d, ky %d; px %d py %d ; rv %d\n",
        __func__, th, ph, fi, rr, kx, ky, *px, *py, rv);
#endif

    return rv;
}


int
_est_portproject(int inag, ob *u, int px, int py,
    int *ax, int *ay, int *bx, int *by,
    int *cx, int *cy, int *dx, int *dy)
{
    double  nag;
    int     ik;
    int     pgap;
    int     psw;
    int     dmyh;

    nag  = (int)inag;

    pgap = def_pbstrgap;
    pgap += u->cob.outlinethick/2;

    if(u->cob.portstr) {
        ik = est_strwh("", FF_SERIF, u->cob.portstr, &psw, &dmyh);
    }
    else {
        psw = 0;
    }
#if 0
    psw = psw*def_textheight/100;
#endif
    psw = (psw+25)*def_textheight/100; /* add 1/4 char margin */

    ik = jj(px, py, nag, pgap, def_textheight,
            ax, ay, bx, by);

    ik = jj(px, py, nag, pgap+psw,   def_textheight,
            cx, cy, dx, dy);

    return 0;
}

int
_est_boardproject(int inag, ob *u, int px, int py,
    int *ax, int *ay, int *bx, int *by,
    int *cx, int *cy, int *dx, int *dy)
{
    double  nag;
    int     ik;
    int     bgap;
    int     bsw;
    int     dmyh;

    nag  = (int)inag;

    bgap = def_pbstrgap;
    bgap += u->cob.outlinethick/2;

    if(u->cob.boardstr) {
        ik = est_strwh("", FF_SERIF, u->cob.boardstr, &bsw, &dmyh);
    }
    else {
        bsw = 0;
    }

#if 0
    bsw = bsw*def_textheight/100;
#endif
    bsw = (bsw+25)*def_textheight/100; /* add 1/4 char margin */

    ik = jj(px, py, nag, bgap, def_textheight,
            ax, ay, bx, by);

    ik = jj(px, py, nag, bgap+bsw,   def_textheight,
            cx, cy, dx, dy);

    return 0;
}


int
est_portboard4c(ns *xns, int gggtdir, ob *u, int xside,
    int *ax, int *ay, int *bx, int *by,
    int *cx, int *cy, int *dx, int *dy)
{
    int     valid=0;
    int     ik;
    
    int     inag = -999;

    int     gap;
    int     swid;
    int     dmyh;

    double  tdir;
    double  px, py, a;
    int     ipx, ipy;
    int     ux, uy, vx, vy;
    int     tx1, ty1, tx2, ty2;
    double  mu, mv;
    double  pag, nag;
    char    astr[BUFSIZ];
    int     qsx, qsy, qex, qey;
    varray_t *sar;
    int     old_t_;

#if 0
    old_t_ = _t_;
    _t_ = 1;
#endif

#if 0
    fflush(stdout);
    Echo("%s: dir %d u %p oid %d type %d glue? %d\n",
        __func__, gggtdir, u, u->oid, u->type, ISGLUE(u->type));
#endif

    if(xside==OA_PORT&& !(u->cob.portstr)) {
        Error("%s: null string in port of oid %d\n", __func__, u->oid);
        return -1;
    }
    if(xside==OA_BOARD&& !(u->cob.boardstr)) {
        Error("%s: null string in board of oid %d\n", __func__, u->oid);
        return -1;
    }

    if(!ISGLUE(u->type)) 
        goto not_glue;

its_glue:
    if(u->type==CMD_BCURVE||u->type==CMD_BCURVESELF) {
        if(u->type==CMD_BCURVE) {
            ik = solve_curve_points(u, xns,
                &mu, &mv, &ux, &uy, &tx1, &ty1, &tx2, &ty2, &vx, &vy);
        }
        else
        if(u->type==CMD_BCURVESELF) {
            ik = solve_curveself_points(u, xns,
                &mu, &mv, &ux, &uy, &tx1, &ty1, &tx2, &ty2, &vx, &vy);
        }
        else {
            abort();
        }

        Echo("%% %s: %d ik %d\n", __func__, __LINE__, ik);
        Echo("%% mu %f mv %f ux,uy %d,%d tx1,ty1 %d,%d tx2,ty2 %d,%d vx,vy %d,%d\n", mu, mv, ux, uy, tx1, ty1, tx2, ty2, vx, vy);


        ik = _bez_posdir(&px, &py, &a, 0.5,
                ux, uy, tx1, ty1, tx2, ty2, vx, vy);

        Echo("%% px,py %f,%f a %f\n", px, py, a);

#if 1
 {
        int x1, y1, x2, y2;
        int zx, zy;
        __solve_fandt(xns, u, u->cob.segopar, 1, &x1, &y1, &x2, &y2);
        zx = (x1+x2)/2;
        zy = (y1+y2)/2;
            Echo("%% zx,zy %d,%d\n", zx, zy);
        px = px - zx;
        py = py - zy;
            Echo("%% px,py %f,%f a %f again\n", px, py, a);
 }
#endif


        Echo("%% %s: %d ik %d\n", __func__, __LINE__, ik);

        tdir = a/rf;
        pag = tdir-90;
        nag = tdir+90;
        Echo("%% a %f, tdir %f, pag %f, nag %f\n", a, tdir, pag, nag);
        u->cob._dbginfo |= (1 << (xside-OA_PORT)*4);
    }
    else {
        sar = u->cob.segar;
        if(!sar) {
#if 1
            Echo("%% %s: oid %d no segar\n", __func__, u->oid);
#endif
            return -2;
        }
        Echo("%% %s: oid %d sar %p use %d original? %d\n",
            __func__, u->oid, sar, sar->use, u->cob.originalshape);
#if 0
        if(sar->use!=1) {
            Echo("%% %s: oid %d many points (%d)\n",
                __func__, u->oid, sar->use);
            if(u->cob.originalshape==0) {
                return -3;
            }
        }
#endif

        if(u->cob.originalshape==0) {
            tdir = gggtdir;
#if 0
            px = (u->gsx+u->gex)/2;
            py = (u->gsy+u->gey)/2;
#endif
            px = (u->sx+u->ex)/2;
            py = (u->sy+u->ey)/2;
            Echo("%% oid %d px %f py %f\n", u->oid, px, py);
            u->cob._dbginfo |= (4 << (xside-OA_PORT)*4);
        }
        else {
            find_from(u, &qsx, &qsy);
            find_to_last(u, &qex, &qey);

            Echo("%% oid %d ox %d oy %d\n", u->oid, u->ox, u->oy);
            Echo("%% oid %d gsx %d gsy %d\n", u->oid, u->gsx, u->gsy);
            Echo("%% oid %d gex %d gey %d\n", u->oid, u->gex, u->gey);
            Echo("%% oid %d from sx %d sy %d\n", u->oid, qsx, qsy);
            Echo("%% oid %d to   ex %d ey %d\n", u->oid, qex, qey);

#if 0
            px = u->gsx+(qsx+qex)/2;
            py = u->gsy+(qsy+qey)/2;
#endif
#if 0
            px = (qsx+qex)/2;
            py = (qsy+qey)/2;
#endif
#if 1
            px = py = 0;
#endif

            tdir = (180.0*atan2(qey-qsy,qex-qsx)/M_PI);
            u->cob._dbginfo |= (5 << (xside-OA_PORT)*4);
        }

        pag = tdir-90;
        nag = tdir+90;
        Echo("%% pag %f nag %f\n", pag, nag);
    }
    goto posfix;


not_glue:
    tdir = gggtdir;
    ik = _X_pbpoint(gggtdir, 90, u, &ipx, &ipy);
    fflush(stdout);
    fprintf(stdout,"_X_pbpoint ik %d\n", ik);
    px = ipx;
    py = ipy;
    if(xside==OA_PORT)  { u->cob.portzx = ipx;  u->cob.portzy = ipy;  }
    if(xside==OA_BOARD) { u->cob.boardzx = ipx; u->cob.boardzy = ipy; }
    u->cob._dbginfo |= (3 << (xside-OA_PORT)*4);

    goto posfix;

posfix:

    Echo("%% posfix oid %d, px %.2f py %.2f tdir %.2f pag %.2f nag %.2f\n",
        u->oid, px, py, tdir, pag, nag);


    gap = def_pbstrgap;
    gap += u->cob.outlinethick/2;
    swid = 0;

    if(xside==OA_PORT) {
        gap += u->cob.portoffset;
        ik = est_strwh("", FF_SERIF, u->cob.portstr, &swid, &dmyh);
        Echo(" '%s' -> swid %d\n", u->cob.portstr, swid);
    }
    if(xside==OA_BOARD) {
        gap += u->cob.boardoffset;
        ik = est_strwh("", FF_SERIF, u->cob.boardstr, &swid, &dmyh);
        Echo(" '%s' -> swid %d\n", u->cob.boardstr, swid);
    }


#if 0
    swid = swid*def_textheight/100;
#endif
    swid = (swid+25)*def_textheight/100; /* add 1/4 char margin */

    ik = jj(px, py, nag, gap, def_textheight,
            ax, ay, bx, by);

    ik = jj(px, py, nag, gap+swid,   def_textheight,
            cx, cy, dx, dy);

    Echo("%s: out oid %d; ik %d dbg %02x nag %.2f %d,%d %d,%d %d,%d %d,%d\n",
        __func__, u->oid,
        ik, u->cob._dbginfo, nag, *ax, *ay, *bx, *by, *cx, *cy, *dx, *dy);

#if 0
    _t_ = old_t_;
#endif

    return 0;
}
