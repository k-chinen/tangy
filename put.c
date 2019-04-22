
#include <stdio.h>

#include "alist.h"
#include "obj.h"
#include "gv.h"
#include "seg.h"
#include "chas.h"
#include "notefile.h"
#include "a.h"

#include "xcur.h"

int xatoi(char *src);
char* draw_word(char *src, char *dst, int wlen, int sep);


#if 0
char debuglog[BUFSIZ*10]="";
#endif

#ifndef QLIMIT
#define QLIMIT  (0.01)
#endif



/* normalize to -180..180 */
int
dirnormalize(int xxdir)
{
    int dir;

    dir = (xxdir+360+180)%360-180;

    return dir;
}

int
rewindcenter(ob *u, int qdir, int *rx, int *ry)
{
    int dir;
    int m;
    
    dir = dirnormalize(qdir);
    
    switch(dir) {
    case -180:      *rx += u->wd/2;         break;
    case  -90:      *ry += u->ht/2;         break;
    case    0:      *rx -= u->wd/2;         break;
    case   90:      *ry -= u->ht/2;         break;
    case  180:      *rx += u->wd/2;         break;
    default:
        return -1;
    }

    return 0;
}

int
_obj_shift(ob *u, int dx, int dy)
{
/*
PK;
*/
    u->x    += dx;      u->y    += dy;
    u->sx   += dx;      u->sy   += dy;
    u->ex   += dx;      u->ey   += dy;
    u->lx   += dx;      u->by   += dy;
    u->rx   += dx;      u->ty   += dy;

    return 0;
}

int
applywith(ob *u, char *xpos, int *rx, int *ry)
{
    char *ypos;
    int pos;

    if(!xpos || !*xpos) {
        return -1;
    }
    if(!u) {
        return -1;
    }

Echo("%s: b rx,ry %d,%d\n", __func__, *rx, *ry);
    if(*xpos=='.') {
        ypos = xpos+1;
    }
    else {
        ypos = xpos;
    }

    pos = assoc(pos_ial, ypos);
    switch(pos) {
    case PO_CENTER:                                         break;
    case PO_NORTH:      *ry -= u->ht/2;                     break;
    case PO_SOUTH:      *ry += u->ht/2;                     break;
    case PO_EAST:       *rx -= u->wd/2;                     break;
    case PO_WEST:       *rx += u->wd/2;                     break;
    case PO_NORTHEAST:  *rx -= u->wd/2; *ry -= u->ht/2;     break;
    case PO_NORTHWEST:  *rx += u->wd/2; *ry -= u->ht/2;     break;
    case PO_SOUTHEAST:  *rx -= u->wd/2; *ry += u->ht/2;     break;
    case PO_SOUTHWEST:  *rx += u->wd/2; *ry += u->ht/2;     break;
    default:    
        Error("ignore position '%s'\n", xpos);
        return -1;
    }

Echo("%s: a rx,ry %d,%d\n", __func__, *rx, *ry);
    
    return 0;
}


#if 1

#define MARK(m,x,y) \
    Echo("MARK oid %d line %d %s x %6d, y %6d\n", u->oid, __LINE__, m, x, y); \
    if(x<_lx)  _lx = x; if(x>_rx)  _rx = x; \
    if(y<_by)  _by = y; if(y>_ty)  _ty = y; 

#else

#define MARK(m,x,y) \
    if(x<_lx)  _lx = x; if(x>_rx)  _rx = x; \
    if(y<_by)  _by = y; if(y>_ty)  _ty = y; 

#endif


/*
            hasfrom = find_from(u, &isx, &isy);
*/
int
find_from(ob *xob, int *risx, int *risy)
{
    varray_t *segar;
    int i;
    seg *e;
    int r;

    r = 0;

    segar = xob->cob.segar;
    if(!segar) {
        return -1;
    }

    for(i=0;i<segar->use;i++) {
        e = segar->slot[i];
        if(e->ftflag & COORD_FROM) {
            r = 1;
            *risx = e->x1;
            *risy = e->y1;
            break;
        }
    }


    return r;
}

int
find_to_first(ob *xob, int *riex, int *riey)
{
    varray_t *segar;
    int i;
    seg *e;
    int r;

    r = 0;

    segar = xob->cob.segar;
    if(!segar) {
        return -1;
    }

    for(i=0;i<segar->use;i++) {
        e = segar->slot[i];
        if(e->ftflag & COORD_TO) {
            r = 1;
            *riex = e->x1;
            *riey = e->y1;
            break;
        }
    }


    return r;
}

int
find_to_last(ob *xob, int *riex, int *riey)
{
    varray_t *segar;
    int i;
    seg *e;
    int r;

    r = 0;

    segar = xob->cob.segar;
    if(!segar) {
        return -1;
    }

    for(i=0;i<segar->use;i++) {
        e = segar->slot[i];
        if(e->ftflag & COORD_TO) {
            r = 1;
            *riex = e->x1;
            *riey = e->y1;
        }
    }


    return r;
}

int
est_seg(ns* xns, ob *u, varray_t *opar, varray_t *segar,
    int kp, int *zdir, int *rlx, int *rby, int *rrx, int *rty,
    int *rfx, int *rfy)
{
    int     x, y;
    int     _lx, _by, _rx, _ty;
    int     i;
    segop  *e;
    seg    *r;
    int     m;
    double  dm;
    char    mstr[BUFSIZ];
    char   *p, *q;
    int     lx, ly;
    double  ldir;
    double  lldir;
    
    int     mx, my;
    int     ik;
    int     rad, an;

    int     rv;
    int     qc;
    int     actf;
    int     arcx, arcy;

    int     c;
    int     jc;

    int     isset_final;

    int     isx, isy, iex, iey; /* from and to */

    rv = 0;

Echo("%s:\n", __func__);
#if 0
    varray_fprint(stdout, opar);
#endif

    isset_final = 0;

    lldir = *zdir;
    ldir = *zdir;

    _rx = _ty = -(INT_MAX-1);
    _lx = _by = INT_MAX;

        x = lx = 0;
        y = ly = 0;
#if 0
        MARK("a0", x, y);
#endif

#if 1
    int adjbyfrom=0;
    for(i=0;i<opar->use;i++) {
        e = (segop*)opar->slot[i];
        if(!e)
            continue;
        switch(e->cmd) {
        case OA_FROM:       
#if 1
P;
            ik = _ns_find_objposP(xns, u, e->val, 1, &mx, &my);
#endif
            if(ik==0) {
P;
                x = lx = mx;
                y = ly = my;
#if 1
                MARK("aF", mx, my);
#endif
            }
            else {
                Error("not found object/position '%s'\n", e->val);
            }
            break;
        }
    }
#endif
#if 1
        MARK("a ", x, y);
#endif


    c = 0; /* count of putted commands w/o then */
    jc = 0;
    for(i=0;i<opar->use;i++) {
        mstr[0] = '\0';
        dm   = 0;
        m    = 0;
        qc   = 0;
        actf = 0;
        rad  = 0;
        an   = 0;

        e = (segop*)opar->slot[i];
        if(!e)
            continue;
        if(!e->val || !e->val[0]) {
P;
            /* empty means 1 unit */
            m = (int)objunit;
            goto skip_m;
        }

        if(e->val[0]) {
            m = xatoi(e->val);
        }
        else {
            m = (int)objunit;
        }

        if(e->cmd==OA_ARC || e->cmd==OA_ARCN) {
            char tmp[BUFSIZ];
            p = e->val;
            p = draw_word(p, tmp, BUFSIZ, SEG_SEP);
            if(tmp[0]) {
                rad = xatoi(tmp);
                p = draw_word(p, tmp, BUFSIZ, SEG_SEP);
                if(tmp[0]) {
                    an  = xatoi(tmp);
                }
                else {
                    an  = 90;
                }
            }
            else {
                rad = objunit;
            }
        }

skip_m:


#define FREG(qpt,qjt,qct,qft,qx,qy,qra,qan) \
    r = seg_new(); \
    if(!r) {} \
    r->ptype    = qpt; \
    r->jtype    = qjt; \
    r->coordtype = qct; \
    r->ftflag   = qft;  \
    r->x1       = qx;   \
    r->y1       = qy;   \
    r->rad      = qra; \
    r->ang      = qan; \
    varray_push(segar, r); \
    c = 0; \
    jc = 0;


#if 0
Echo("  m %d\n", m);
#endif

        if(e->cmd==OA_ARC||e->cmd==OA_ARCN||e->cmd==OA_THEN||
            e->cmd==OA_JOIN||e->cmd==OA_SKIP) {
            actf = 1;
        }

        /* to reduce process default is skip */
        if(actf) {
flush_que:
            if(c>0) {
                MARK("f ", x, y);
                FREG(OA_FORWARD, jc, REL_COORD, 0, x-lx, y-ly, 0, 0);
                ldir = atan2(y-ly, x-lx)/rf;
                lx = x;
                ly = y;
            }
        }

        switch(e->cmd) {
        default:
            Warn("unknown sub-command %d\n", e->cmd);
            break;


        case OA_FROM:       
        case OA_TO:     
#if 1
            if(c>0) {
                            r = seg_new();
                            r->coordtype = REL_COORD;
                            r->ptype = OA_FORWARD;
                            r->x1 = x - lx;
                            r->y1 = y - ly;
                            varray_push(segar, r);

#if 1
                            ldir = atan2(y-ly, x-lx)/rf;
Echo("        y %d x %d -> ldir %.2f\n", y-ly, x-lx, ldir);
#endif

#if 0
Echo("        y %d x %d -> ldir %.2f\n", y-ly, x-lx, ldir);
#endif

                            r = NULL;
                            lx = x;
                            ly = y;
                            c=0;
                            jc=0;
            }
#endif


#if 0
            ik = ns_find_objpos(xns, e->val, &mx, &my);
            ik = ns_find_objposG(xns, e->val, &mx, &my);
            ik = ns_find_objposXG(xns, e->val, &mx, &my);
#endif
#if 0
            ik = ns_find_objposX(xns, e->val, &mx, &my);
#endif
#if 1
P;
            ik = _ns_find_objposP(xns, u, e->val, 1, &mx, &my);
#endif
            if(ik==0) {
P;
#if 1
                if(e->cmd==OA_FROM) {
                    MARK("fP", mx, my);
                }
                if(e->cmd==OA_TO) {
                    MARK("tP", mx, my);
                }
#endif
            }
            else {
                Error("not found object/position '%s'\n", e->val);
                break;
            }

P;
            if(e->cmd==OA_FROM) {
                isx = mx;
                isy = my;
            }
P;
            if(e->cmd==OA_TO) {
                iex = mx;
                iey = my;
            }

#if 1
P;
            if(e->cmd==OA_TO) {
Echo("mark final position %d,%d\n", mx, my);
                *rfx = mx;
                *rfy = my;
                isset_final++;
            }
            

P;
            if(e->cmd==OA_FROM) {
                FREG(OA_FORWARD, jc, ABS_COORD, COORD_FROM, mx, my, rad, an);
                rv |= COORD_FROM;
            }
            else
            if(e->cmd==OA_TO) {
                FREG(OA_FORWARD, jc, ABS_COORD, COORD_TO, mx, my, rad, an);
                rv |= COORD_TO;
            }
            else {
                E;
            }
            
#endif

            break;

        case OA_CLOSE:  
            FREG(OA_CLOSE, jc, REL_COORD, 0, 0, 0, 0, 0);
            break;

        case OA_RIGHT:      x += m; c++; ldir =    0; break;
        case OA_LEFT:       x -= m; c++; ldir =  180; break;
        case OA_UP:         y += m; c++; ldir =   90; break;
        case OA_DOWN:       y -= m; c++; ldir =  -90; break;

#if 0
        case OA_DIR:        ldir =  m; break;
        case OA_INCDIR:     ldir += m; break;
        case OA_DECDIR:     ldir -= m; break;
#endif
#if 1
        case OA_DIR:        ldir = m;
                            FREG(OA_DIR, jc, REL_COORD, 0, 0, 0, 0, m);
                            break;

        case OA_INCDIR:
        case OA_LTURN:
                            ldir += m;
                            FREG(OA_INCDIR, jc, REL_COORD, 0, 0, 0, 0, m);
                            break;
        case OA_DECDIR:
        case OA_RTURN:
                            ldir -= m;
                            FREG(OA_INCDIR, jc, REL_COORD, 0, 0, 0, 0, -m);
                            break;
#endif

        case OA_JOIN:
                            jc += 1;
                            c++;
                            FREG(OA_JOIN, 1, REL_COORD, 0, 0, 0, 0, 0);
                            MARK("J ", x, y);
                            break;

        case OA_SKIP:
                            jc += 1000;
                            x += m*cos(ldir*rf);
                            y += m*sin(ldir*rf);
                            c++;
                            FREG(OA_SKIP, 1000, REL_COORD, 0, x-lx, y-ly, 0, 0);
                            MARK("S ", x, y);
                            break;
    
        case OA_FORWARD:    x += m*cos(ldir*rf);
                            y += m*sin(ldir*rf);
                            c++;
                            break;
        
        case OA_ARC:
Echo("  ldir %7.2f rad %d an %d\n", ldir, rad, an);
                            MARK("cB", x, y);
                            arcx = x + rad*cos((ldir+90)*rf);
                            arcy = y + rad*sin((ldir+90)*rf);

Echo("  arc ldir %f .. %f\n", ldir-90, ldir-90+an);
    
                            /* mark peak points */
                            {
                                int a;
                                int tx, ty;
                                for(a=ldir-90;a<=ldir-90+an;a++) {
                                    if(a==ldir-90||a==ldir-90+an||a%90==0) {
                                        tx = arcx+rad*cos((a)*rf);
                                        ty = arcy+rad*sin((a)*rf);
                                        Echo(" a %4d ", a);
                                        if(a==ldir-90) {
                                            Echo(" first  ");
                                        }
                                        if(a==ldir-90+an) {
                                            Echo(" last   ");
                                        }
                                        if(a%90==0) {
                                            Echo(" middle ");
                                        }
                                        MARK("cM", tx, ty);
                                    }
                                }
                            }

                            x = arcx+rad*cos((ldir-90+an)*rf);
                            y = arcy+rad*sin((ldir-90+an)*rf);

Echo("  arcx,y %d,%d x,y %d,%d\n", arcx, arcy, x, y);
                            MARK("cE", x, y);


                            FREG(OA_ARC, jc, REL_COORD, 0, 0, 0, rad, an);

                            ldir += an;
Echo("        y %d x %d -> ldir %.2f\n", y-ly, x-lx, ldir);

                            r = NULL;
                            lx = x;
                            ly = y;
                            c=0;
                            jc=0;

                            break;

        case OA_ARCN:
Echo("  ldir %7.2f rad %d an %d\n", ldir, rad, an);
                            MARK("nB", x, y);

                            arcx = x + rad*cos((ldir-90)*rf);
                            arcy = y + rad*sin((ldir-90)*rf);
#if 0
                            x = arcx+rad*cos((ldir+90)*rf);
                            y = arcy+rad*sin((ldir+90)*rf);

Echo("  arcx,y %d,%d x,y %d,%d\n", arcx, arcy, x, y);
                            MARK("nB", x, y);
#endif

Echo("  arcn ldir %f .. %f\n", ldir+90, ldir+90-an);
                            /* mark peak points */
                            {
                                int a;
                                int tx, ty;
                                for(a=ldir+90;a>=ldir+90-an;a--) {
                                    if(a==ldir+90||a==ldir+90-an||a%90==0) {
                                        tx = arcx+rad*cos((a)*rf);
                                        ty = arcy+rad*sin((a)*rf);
                                        Echo(" a %4d ", a);
                                        if(a==ldir+90) {
                                            Echo(" first  ");
                                        }
                                        if(a==ldir+90-an) {
                                            Echo(" last   ");
                                        }
                                        if(a%90==0) {
                                            Echo(" middle ");
                                        }
                                        MARK("nM", tx, ty);
                                    }
                                }
                            }

                            x = arcx+rad*cos((ldir+90-an)*rf);
                            y = arcy+rad*sin((ldir+90-an)*rf);

Echo("  arcx,y %d,%d x,y %d,%d\n", arcx, arcy, x, y);
                            MARK("nE", x, y);

                            FREG(OA_ARCN, jc, REL_COORD, 0, 0, 0, rad, an);

                            ldir -= an;
Echo("        y %d x %d -> ldir %.2f\n", y-ly, x-lx, ldir);

                            r = NULL;
                            lx = x;
                            ly = y;
                            c=0;
                            jc=0;

                            break;

    

        case OA_THEN:   
            /* nothing */
                            break;
        }
#if 0
Echo("    %d: cmd %d val '%s' : mstr '%s' dm %.2f m %d : x,y %d,%d ldir %.2f\n",
        i, e->cmd, e->val, mstr, dm, m, x, y, ldir);
#endif
        MARK("e ", x, y);
    }
        MARK("z ", x, y);

    Echo("MARK oid %d (%d %d %d %d)\n",
        u->oid, _lx, _by, _rx, _ty);


#if 0
    if(rv & COORD_FROM) {
        *rlx = _lx + isx;
        *rby = _by + isy;
        *rrx = _rx + isx;
        *rty = _ty + isy;
    }
    else {
        *rlx = _lx;
        *rby = _by;
        *rrx = _rx;
        *rty = _ty;
    }
#endif

    {
        *rlx = _lx;
        *rby = _by;
        *rrx = _rx;
        *rty = _ty;
    }

    Echo("MARK*oid %d (%d %d %d %d)\n",
        u->oid, *rlx, *rby, *rrx, *rty);

#if 0
    Echo("opar\n");
    varray_fprint(stdout, opar);
    Echo("segar\n");
    varray_fprint(stdout, segar);
#endif

#if 0
Echo("    ldir %.2f\n", ldir);
#endif
    if(!kp) {
#if 0
Echo("    *zdir %d -> %.2f\n", *zdir, ldir);
#endif
        *zdir = (int)ldir;
    }
    if(isset_final<=0) {
Echo("set final as last position\n");
        *rfx = lx;
        *rfy = ly;
    }
    else {
Echo("set final as specified position\n");
    }

    if(INTRACE) {
        printf("opar\n");
        varray_fprintv(stdout, opar);
        printf("segar\n");
        varray_fprintv(stdout, segar);
    }

#if 1
Echo("%s: oid %d %d %d %d %d\n", __func__, u->oid, *rlx, *rby, *rrx, *rty);
#endif
    return rv;
}

/* XXX */
int
est_simpleseg(ns* xns, ob *u, varray_t *opar, varray_t *segar,
    int kp, int *zdir, int *rlx, int *rby, int *rrx, int *rty,
    int *rfx, int *rfy)
{
    int     rv;
    qbb_t   sbb;
    int     r;
    int     ddir;
    int     isx, isy, iex, iey; /* from and to */
    int     bm, bmx, bmy;   /* beam and its offset for size */

    rv = 0;

Echo("%s:\n", __func__);
    ddir = dirnormalize(*zdir);
Echo("  zdir %4d ; ddir %4d\n", *zdir, ddir);

    qbb_reset(&sbb);

    if(u->cob.length) {
        r = u->cob.length;
    }
    else {
        /* default value */
#if 0
        switch(u->type) {
        case CMD_WLINE:
        case CMD_WARROW:
            r = objunit*3/2;
            break;
        default:
            r = objunit;
            break;
        }
#endif
            r = objunit*3/2;
    }
Echo("  u->cob.length %d r %d\n", u->cob.length, r);


    isx = 0;
    isy = 0;
    iex = r*cos(ddir*rf);
    iey = r*sin(ddir*rf);
Echo("  isx,y %d,%d iex,y %d,%d\n", isx, isy, iex, iey);

    switch(u->type) {
    case CMD_WLINE:
    case CMD_WARROW:
        if(u->cob.arrowforeheadtype>0 || u->cob.arrowcentheadtype>0
            || u->cob.arrowbackheadtype>0) {
            bm = objunit;
        }
        else {
            bm = objunit/2;
        }
        break;
    default:
        if(u->cob.arrowforeheadtype>0 || u->cob.arrowcentheadtype>0
            || u->cob.arrowbackheadtype>0) {
            bm = objunit/8;
        }
        else {
            bm = objunit/20;
        }
    }

    bmx = (bm/2)*cos((ddir+90)*rf);
    bmy = (bm/2)*sin((ddir+90)*rf);
Echo("  bm %d bmx,y %d,%d\n", bm, bmx, bmy);

    qbb_mark(&sbb, isx, isy);
    qbb_mark(&sbb, isx-bmx, isy+bmy);
    qbb_mark(&sbb, isx+bmx, isy-bmy);
    qbb_mark(&sbb, iex, iey); 
    qbb_mark(&sbb, iex-bmx, iey+bmy);
    qbb_mark(&sbb, iex+bmx, iey-bmy);

    *rfx = iex;
    *rfy = iey;

    *rlx = sbb.lx;
    *rby = sbb.by;
    *rrx = sbb.rx;
    *rty = sbb.ty;

#if 1
Echo("%s: oid %d %d %d %d %d\n", __func__, u->oid, *rlx, *rby, *rrx, *rty);
#endif
    return rv;

#undef FREG
}

int
Yest_simpleseg(ns* xns, ob *u, varray_t *opar, varray_t *segar,
    int kp, int *zdir, int *rlx, int *rby, int *rrx, int *rty,
    int *rfx, int *rfy)
{
    int     x, y;
    int     _lx, _by, _rx, _ty;
    int     i;
    segop  *e;
    int     m;
    double  dm;
    char    mstr[BUFSIZ];
    char   *p, *q;
    int     lx, ly;
    double  ldir;
    double  lldir;
    int     r;
    
    int     mx, my;
    int     ik;
    int     rad, an;

    int     rv;
    int     qc;
    int     actf;
    int     arcx, arcy;

    int     c;
    int     jc;

    int     isset_final;

    int     isx, isy, iex, iey; /* from and to */

    int     Uwd, Uht;
    int     ddir;

    rv = 0;

Echo("%s:\n", __func__);
    ddir = dirnormalize(*zdir);
Echo("  zdir %4d ; ddir %4d\n", *zdir, ddir);

    if(u->cob.length) {
        r = u->cob.length;
    }
    else {
        /* default value */
#if 0
        switch(u->type) {
        case CMD_WLINE:
        case CMD_WARROW:
            r = objunit*3/2;
            break;
        default:
            r = objunit;
            break;
        }
#endif
            r = objunit*3/2;
    }
Echo("  u->cob.length %d r %d\n", u->cob.length, r);

    if(ddir==270) {
        *rlx = 0;
        *rby = 0;
        *rrx = 0;
        *rty = -r;
    }
    else if(ddir==180) {
        *rlx = 0;
        *rby = 0;
        *rrx = -r;
        *rty = 0;
    }
    else if(ddir==90) {
        *rlx = 0;
        *rby = 0;
        *rrx = 0;
        *rty = r;
    }
    else if(ddir==0) {
        *rlx = 0;
        *rby = 0;
        *rrx = r;
        *rty = 0;
    }
    else {
        *rlx = 0;
        *rby = 0;
        *rrx = r*cos(ddir*rf);
        *rty = r*sin(ddir*rf);
    }
    *rfx = *rrx;
    *rfy = *rty;

#if 1
Echo("%s: oid %d %d %d %d %d\n", __func__, u->oid, *rlx, *rby, *rrx, *rty);
#endif
    return rv;

#undef FREG
}


int
Xest_simpleseg(ns* xns, ob *u, varray_t *opar, varray_t *segar,
    int kp, int *zdir, int *rlx, int *rby, int *rrx, int *rty,
    int *rfx, int *rfy)
{
    int     x, y;
    int     _lx, _by, _rx, _ty;
    int     i;
    segop  *e;
    seg    *r;
    int     m;
    double  dm;
    char    mstr[BUFSIZ];
    char   *p, *q;
    int     lx, ly;
    double  ldir;
    double  lldir;
    
    int     mx, my;
    int     ik;
    int     rad, an;

    int     rv;
    int     qc;
    int     actf;
    int     arcx, arcy;

    int     c;
    int     jc;

    int     isset_final;

    int     isx, isy, iex, iey; /* from and to */

    int     Uwd, Uht;
    int     ddir;

    rv = 0;

Echo("%s:\n", __func__);
    ddir = (*zdir+360)%360;
Echo("  zdir %4d ; ddir %4d\n", *zdir, ddir);

    Uwd = objunit*3/2;
    Uht = objunit;
    

    if(ddir==270) {
        *rlx = 0;
        *rby = 0;
        *rrx = 0;
        *rty = -Uht;
        *rfx = 0;
        *rfy = -Uht;
    }
    else if(ddir==180) {
        *rlx = 0;
        *rby = 0;
        *rrx = -Uwd;
        *rty = 0;
        *rfx = -Uwd;
        *rfy = 0;
    }
    else if(ddir==90) {
        *rlx = 0;
        *rby = 0;
        *rrx = 0;
        *rty = Uht;
        *rfx = 0;
        *rfy = Uht;
    }
    else if(ddir==0) {
        *rlx = 0;
        *rby = 0;
        *rrx = Uwd;
        *rty = 0;
        *rfx = Uwd;
        *rfy = 0;
    }
    else {
        *rlx = 0;
        *rby = 0;
        *rrx = objunit*cos(ddir*rf);
        *rty = objunit*sin(ddir*rf);

        *rfx = *rrx;
        *rfy = *rty;
    }

#if 1
Echo("%s: oid %d %d %d %d %d\n", __func__, u->oid, *rlx, *rby, *rrx, *rty);
#endif
    return rv;

#undef FREG
}


#undef MARK

/*
 *
 */
int
takelastobjpos(ob *lob, int pos, int kp, int *nx, int *ny, int *dir)
{
    if(!lob) {
        return -1;
    }
    Echo("%s: lob %p oid %d pos %d\n", __func__, lob, lob->oid, pos);

    Echo("  lob attributes\n");
    Echo("    lx,by,rx,ty %d,%d,%d,%d\n",
        lob->lx, lob->by, lob->rx, lob->ty);

    switch(pos) {
    case PO_NORTH:  
        *nx = lob->cx; *ny = lob->ty; if(!kp) { *dir =   90; } break;
    case PO_NORTHEAST:
        *nx = lob->rx; *ny = lob->ty; if(!kp) { *dir =   45; } break;
    case PO_EAST:
        *nx = lob->rx; *ny = lob->cy; if(!kp) { *dir =    0; } break;
    case PO_SOUTHEAST:
        *nx = lob->rx; *ny = lob->by; if(!kp) { *dir =  -45; } break;
    case PO_SOUTH:
        *nx = lob->cx; *ny = lob->by; if(!kp) { *dir =  -90; } break;
    case PO_SOUTHWEST:
        *nx = lob->lx; *ny = lob->by; if(!kp) { *dir = -135; } break;
    case PO_WEST:
        *nx = lob->lx; *ny = lob->cy; if(!kp) { *dir =  180; } break;
    case PO_NORTHWEST:
        *nx = lob->lx; *ny = lob->ty; if(!kp) { *dir =  135; } break;

    case PO_START:
        *nx = lob->csx;
        *ny = lob->csy;
        break;
    case PO_END:
        *nx = lob->csx;
        *ny = lob->csy;
        break;

    case PO_CENTER:
    default:
        *nx = lob->cx;
        *ny = lob->cy;
        break;
    }
    Echo("  ret %d,%d\n", *nx, *ny);
    return 0;   
}

int
OP_GDUMP(const char *pre, char *mid, ob *u)
{
    if(!u)
        return -1;

    Echo("%s: %s oid %d = = = = =\n", pre, mid, u->oid);
#if 0
    Echo("  L x,y %6d,%6d sx,sy %6d,%6d ex,ey %6d,%6d lbrt %6d,%6d,%6d,%6d\n",
        u->cx, u->cy, u->csx, u->csy, u->ex, u->ey,
        u->clx, u->cby, u->crx, u->cty);
#endif
    Echo("  G x,y %6d,%6d sx,sy %6d,%6d ex,ey %6d,%6d lbrt %6d,%6d,%6d,%6d\n",
        u->cgx, u->cgy, u->cgsx, u->cgsy, u->gex, u->gey,
        u->cglx, u->cgby, u->cgrx, u->cgty);
    Echo("\n");

    return 0;
}

int
OP_LDUMP(const char *pre, char *mid, ob *u)
{
    if(!u)
        return -1;

    Echo("%s: %s oid %d = = = = =\n", pre, mid, u->oid);
    Echo("  L x,y %6d,%6d sx,sy %6d,%6d ex,ey %6d,%6d lbrt %6d,%6d,%6d,%6d\n",
        u->cx, u->cy, u->csx, u->csy, u->ex, u->ey,
        u->clx, u->cby, u->crx, u->cty);
#if 0
    Echo("  G x,y %6d,%6d sx,sy %6d,%6d ex,ey %6d,%6d lbrt %6d,%6d,%6d,%6d\n",
        u->cgx, u->cgy, u->cgsx, u->cgsy, u->gex, u->gey,
        u->cglx, u->cgby, u->cgrx, u->cgty);
#endif
    Echo("\n");

    return 0;
}

int
insboxpath(varray_t *xar, int xwd, int xht)
{
    fprintf(stdout, "%s: xar %p xwd %d xht %d\n", __func__, xar, xwd, xht);
#if 0
    fprintf(stdout, "b ");
    varray_fprint(stdout, xar);


    fprintf(stdout, "a ");
    varray_fprint(stdout, xar);
#endif
    return 0;
}


 /*
  * name    size        description
  *-----
  * WO      1.5  x 1.0  wide
  * RO      1.0  x 1.0  square
  * NO      0.5  x 1.0  narrow
  * NNO     0.25 x 1.0  narrow narrow
  * VO      0    x 1.0  vartical line
  * HO      1.0  x 0    horizontal line
  * ZO      0    x 0    zero
  * SO      0.1  x 0.1  small
  *
  */
 

#define WO      {wd = objunit*3/2;  ht = objunit;   }
#define RO      {wd = objunit;      ht = objunit;   }
#define NO      {wd = objunit/2;    ht = objunit;   }
#define NNO     {wd = objunit/4;    ht = objunit;   }
#define VO      {wd = 0;            ht = objunit;   }
#define HO      {wd = objunit       ht = 0;         }
#define ZZ      {wd = 0;            ht = 0;         }
#define SO      {wd = objunit/5;    ht = objunit/5; }

#if 0
#include "xcur.c"
#endif

int
putobj(ob *u, ns *xns, int *gdir)
{
    int    wd, ht;
    int    dir;
    int    re;

#if 1
    if(u) {
        Echo("%s: oid %d type %s(%d)\n",
            __func__, u->oid,
            rassoc(cmd_ial, u->type),
             u->type);
    }
    else {
        Echo("%s: null object\n", __func__);
    }
#endif
#if 1
Echo("\toid %d u b wd %d ht %d solved? %d\n",
    u->oid, u->wd, u->ht, u->sizesolved);
#endif

    dir = *gdir;
    re = 0;

#if 0
    if(u->hasrel) {
P;
        wd = 0;
        ht = 0;
        goto apply;
    }
#endif
    if(u->floated) {
P;
        wd = 0;
        ht = 0;
#if 0
        goto apply;
#endif
    }

    if(u->sizesolved) {
        goto out;
    }


/*
 * object are calculated by rectangle.
 * if rotate is on, the rectangele will be expand.
 * 
 *   -----
 *   | /\|
 *   |/ /|
 *   |\/ |
 *   -----
 */

    wd = ht = 0;

    switch(u->type) {
    case CMD_TRACEON:
        _p_ = 1;
        _t_ ++;
        break;
    case CMD_TRACEOFF:
        _p_ = 0;
        _t_ --;
        break;

    case CMD_NOP:
        break;

    case CMD_CHUNKOBJATTR:  ZZ; break;

    case CMD_LPAREN:    NNO;    break;
    case CMD_RPAREN:    NNO;    break;
    case CMD_LBRACKET:  NNO;    break;
    case CMD_RBRACKET:  NNO;    break;
    case CMD_LBRACE:    NNO;    break;
    case CMD_RBRACE:    NNO;    break;

    case CMD_MOVE:      WO;     break;

    case CMD_PAPER:     WO;     break;
    case CMD_CARD:      WO;     break;
    case CMD_DIAMOND:   WO;     break;
    case CMD_HOUSE:     WO;     break;
    case CMD_CLOUD:     WO;     break;
    case CMD_DRUM:      WO;     break;

    case CMD_BOX:       WO;     break;
    case CMD_CIRCLE:    RO;     break;
    case CMD_POINT:     SO;     break;
    case CMD_ELLIPSE:   WO;     break;
    case CMD_POLYGON:   RO;     break;

#if 0
    case CMD_DMY1:      WO;     break;
    case CMD_DMY2:      RO;     break;
    case CMD_DMY3:      WO;     break;
#endif

    case CMD_DOTS:      WO;     u->cob.sepcurdir = dir; break;

    case CMD_PING:
    case CMD_PINGPONG:  WO;     break;

    case CMD_SCATTER: 
    case CMD_GATHER: 
    case CMD_THRU: 
            RO;     
            break;

    case CMD_PLINE:
            VO;
Echo("PLINE oid %d dir %d\n", u->oid ,dir);
            u->cob.sepcurdir = dir;
            break;
    case CMD_SEP:     
            NNO;
Echo("SEP oid %d dir %d\n", u->oid ,dir);
            u->cob.sepcurdir = dir;
            break;

    case CMD_XCURVE:
    case CMD_XCURVESELF:
    case CMD_BCURVE:
    case CMD_BCURVESELF:
        if(u->cob.originalshape) {
        }
        else {
            WO;     
            break;
        }
            {
            int ik;
            int _sx, _sy, _ex, _ey;
            int _lx, _by, _rx, _ty, fx, fy;
            if(u->type==CMD_XCURVE || u->type==CMD_BCURVE) {
                ik = MARK_bcurveX(u, xns,
                        &_sx, &_sy, &_ex, &_ey, &_lx, &_by, &_rx, &_ty);
            }
            else
            if(u->type==CMD_XCURVESELF || u->type==CMD_BCURVESELF) {
                ik = MARK_bcurveself(u, xns,
#if 0
                        &_sx, &_sy, &_ex, &_ey, &_lx, &_by, &_rx, &_ty);
#endif
                        &_lx, &_by, &_rx, &_ty);
            }
            else {
                break;
            }
P;
#if 1
Echo("\tcurve original oid %d sx,y %d,%d ex,y %d,%d bb (%d %d %d %d) fxy %d,%d\n",
        u->oid, _sx, _sy, _ex, _ey, _lx, _by, _rx, _ty, fx, fy);
#endif
            u->csx = _sx;
            u->csy = _sy;
            u->cex = _ex;
            u->cey = _ey;

            u->clx = _lx;
            u->cby = _by;
            u->crx = _rx;
            u->cty = _ty;

            wd = _rx - _lx;
            ht = _ty - _by;

#if 0
            u->fx = wd;
            u->fy = ht;
#endif
#if 0
            u->fx = fx - lx;
            u->fy = fy - by;
#endif
#if 0
            u->ox = -wd/2;
            u->oy = -ht/2;
#endif

            re = 1;
            }
            break;

    case CMD_ULINE:
        {
            int ik;
            int lx, by, rx, ty, fx, fy;

            if(u->cob.originalshape) {
                ik = est_seg(xns, u, u->cob.segopar, u->cob.segar,
                    u->cob.keepdir, gdir, &lx, &by, &rx, &ty, &fx, &fy);
            }
            else {
                ik = est_simpleseg(xns, u, u->cob.segopar, u->cob.segar,
                    u->cob.keepdir, gdir, &lx, &by, &rx, &ty, &fx, &fy);
            }
            wd = rx - lx;
            ht = ty - by;
Echo("ULINEs oid %d %d,%d,%d,%d %dx%d\n", u->oid, lx, by, rx, ty, wd, ht);
            u->ox = -wd/2-lx;
            u->oy = -ht/2-by;
Echo("ULINEs oid %d u->ox %d, u->oy %d\n", u->oid, u->ox, u->oy);
#if 0
#endif
        }
        break;

    case CMD_LINE:     
    case CMD_ARROW:
    case CMD_WLINE:
    case CMD_WARROW:
    case CMD_BARROW:
    case CMD_LINK: 
        {
            int ik;
            int lx, by, rx, ty, fx, fy;

            if(u->cob.originalshape) {
                ik = est_seg(xns, u, u->cob.segopar, u->cob.segar,
                    u->cob.keepdir, gdir, &lx, &by, &rx, &ty, &fx, &fy);
            }
            else {
                ik = est_simpleseg(xns, u, u->cob.segopar, u->cob.segar,
                    u->cob.keepdir, gdir, &lx, &by, &rx, &ty, &fx, &fy);
            }

#if 1
Echo("\tseg original oid %d bb (%d %d %d %d) fxy %d,%d\n",
        u->oid, lx, by, rx, ty, fx, fy);
#endif

            u->clx = lx;
            u->cby = by;
            u->crx = rx;
            u->cty = ty;

            wd = rx - lx;
            ht = ty - by;

#if 1
Echo("\tseg original 1 wd %d ht %d\n", wd, ht);
#endif

#if 1
            u->fx = fx;
            u->fy = fy;
#endif
#if 0
            u->fx = fx - lx;
            u->fy = fy - by;
#endif

#if 1
            u->ox = -wd/2;
            u->oy = -ht/2;
#endif
#if 0
            u->ox = -(lx+rx)/2;
            u->oy = -(ty+by)/2;
#endif
            if(u->type==CMD_ULINE) {
            }
            else {
#if 0
                u->x = -(lx+rx)/2;
                u->y = -(ty+by)/2;
#endif
            }

#if 1
Echo("\tseg original 1 u; ox,oy %d,%d fx,fy %d,%d\n",
    u->ox, u->oy, u->fx, u->fy);
#endif

            re = 1;
        }
        break;
    default:
        if(ISATOM(u->type)) {
            Warn("WARNING: sorry the object(oid %d) has no size\n",
                u->oid);
        }
        else {
            Warn("unsupported type '%s'(%d)\n", 
                rassoc(cmd_ial, u->type), u->type);
        }
        break;
    }

apply:

#if 1
Echo("\t  2 u wd %d ht %d\n", u->wd, u->ht);
Echo("\t  2   wd %d ht %d\n", wd, ht);
#endif
    if(u->wd<0) u->wd = wd;
    if(u->ht<0) u->ht = ht;
#if 1
Echo("\t  3 u wd %d ht %d\n", u->wd, u->ht);
#endif

#if 1
    /*** FIX ***/
    if(u->type==CMD_CIRCLE || u->type==CMD_POLYGON) {
        if(u->cob.rad>0) {
            u->wd = u->cob.rad*2;
            u->ht = u->cob.rad*2;
        }
    }
    if(u->type==CMD_POINT) {
        if(u->cob.rad>0) {
            u->wd = u->cob.rad*2;
            u->ht = u->cob.rad*2;
        }
    }
#endif

    u->sizesolved++;
#if 0
Echo("obj oid %d solved\n", u->oid);
#endif

#if 0
    switch(u->type) {
    case CMD_DMY1:
        insboxpath(u->cob.segar, u->wd, u->ht);
        break;
    }
#endif

#if 1
Echo("\toid %d u a wd %d ht %d solved? %d\n",
    u->oid, u->wd, u->ht, u->sizesolved);
#endif

#if 0
    Echo("%s: oid %d dir %d wxh %dx%d; %d %d %d %d\n",
        __func__, u->oid, dir, u->wd, u->ht,
        u->lx, u->by, u->rx, u->y);
#endif
#if 1
    Echo("%s: oid %d dir %d wxh %dx%d; xy %d,%d; re %d\n",
        __func__, u->oid, dir, u->wd, u->ht,
        u->x, u->y, re);
#endif

#if 1
    OP_LDUMP(__func__, "a", u);
#endif

out:
    return re;
}



int
fitobj_LBRT(ob *u, int xxdir, int *x, int *y, int *fx, int *fy, ns *xns)
{
    int dx, dy;

    if(!u) {
E;
        return -1;
    }

#if 0
    Echo("%s: oid %d START\n", __func__, u->oid);
#endif
Echo("%s: oid %d START dir %d x,y %d,%d fx,y %d,%d\n",
    __func__, u->oid, xxdir, *x, *y, *fx, *fy);

#if 1
    Echo("%s: b %p oid %-3d xxdir %-4d *xy %6d,%-6d *fxy %6d,%-6d xns %p\n",
            __func__, u, u->oid, xxdir, *x, *y, *fx, *fy, xns);
    Echo("    wd %d ht %d (%d %d %d %d)\n",
            u->wd, u->ht, u->lx, u->by, u->rx, u->ty);
    Echo("    ox,oy %d,%d\n", u->ox, u->oy);
#endif

#if 1
    OP_LDUMP(__func__, "b", u);
#endif

    u->csx = *x;
    u->csy = *y;

#if 1
    u->cx = *x - u->ox;
    u->cy = *y - u->oy;
#else
    u->cx = *x + u->ox;
    u->cy = *y + u->oy;
#endif

#if 1 
    u->clx = *x + u->clx;
    u->cby = *y + u->cby;
    u->crx = *x + u->crx;
    u->cty = *y + u->cty;
#endif
#if 0
    u->clx = u->cx + u->clx;
    u->cby = u->cy + u->cby;
    u->crx = u->cx + u->crx;
    u->cty = u->cy + u->cty;
#endif
#if 0
    u->clx = u->cx + u->clx;
    u->cby = u->cy + u->cby;
    u->crx = u->cx + u->crx;
    u->cty = u->cy + u->cty;
#endif

    Echo("%s: oid %d (%d %d %d %d)\n",
        __func__, u->oid, u->clx, u->cby, u->crx, u->cty);

#if 0
    u->cex = *fx - *x;
    u->cey = *fy - *y;
#endif
#if 0
    u->cex = *fx;
    u->cey = *fy;
#endif
#if 1
    u->cex = *x+*fx;
    u->cey = *y+*fy;
#endif
#if 0
    u->cex = *x+u->wd;
    u->cey = *y+u->ht;
#endif
    *x = u->cex;
    *y = u->cey;

#if 0
    int ik;
    ik = try_regline(u->cob.segar, u->csx, u->csy, u->cex, u->cey);
#endif

#if 0
    Echo("%s: u %p oid %d x,y %d,%d sx,sy %d,%d ex,ey %d,%d\n",
            __func__, u, u->oid,
            u->cx, u->cy, u->csx, u->csy, u->cex, u->cey);
#endif
#if 1
    Echo("%s: a oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, u->oid,
        u->cx, u->cy, u->csx, u->csy, u->cex, u->cey, u->ox, u->coy);
#endif

#if 1
    OP_LDUMP(__func__, "a", u);
#endif
#if 1
    Echo("%s: a %p oid %-3d xxdir %-4d *xy %6d,%-6d *fxy %6d,%-6d xns %p\n",
            __func__, u, u->oid, xxdir, *x, *y, *fx, *fy, xns);
#endif

    return 0;
}

int
fitobj_wdht(ob *u, int xxdir, int *x, int *y, ns *xns)
{
    int oldx, oldy;
    double dx, dy;
    double q, qx, qy;
    int r;
    int hh;
    double th;
    int    wd, ht;
    char   f1, f2, f3;
    int dir;

    if(!u) {
        return -1;
    }

Echo("%s: oid %d START dir %d x,y %d,%d\n", __func__, u->oid, xxdir, *x, *y);

#if 1
    Echo("%s: b %p oid %-3d xxdir %-4d *xy %6d,%-6d *fxy ------,------ xns %p\n",
            __func__, u, u->oid, xxdir, *x, *y, xns);
    Echo("    wd %d ht %d (%d %d %d %d)\n",
            u->wd, u->ht, u->lx, u->by, u->rx, u->ty);
    Echo("    ox,oy %d,%d\n", u->ox, u->oy);
#endif

#if 1
    OP_LDUMP(__func__, "b", u);
#endif


    /* normalization dir */
    dir = (xxdir+360+180)%360-180;

#if 0
Echo("xxdir %d dir %d\n", xxdir, dir);
#endif

#if 0
    Echo("%s: oid %d dir %d wxh %dx%d x,y %d,%d\n",
        __func__, u->oid, dir, u->wd, u->ht, *x, *y);
    Echo("%s: b oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, u->oid,
        u->cx, u->cy, u->csx, u->csy, u->cex, u->cey, u->ox, u->coy);
#endif

    f1 = '.';
    f2 = '.';
    f3 = '.';

    dx = 0;
    dy = 0;

    qx = 0;
    qy = 0;
    q  = -1;

    oldx = *x;
    oldy = *y;

    wd = u->wd;
    ht = u->ht;

    th = atan2(ht, wd);
    th = th/rf;

#if 0
    Echo("th %.3f (rad %.3f) <- %dx%d oid %d v.s. dir %d\n",
        th, th*rf, wd, ht, u->oid, dir);
#endif

    /* atan2 returns -180 to +180 */
    if((dir>th&&dir<180-th)|| (dir>-180+th&&dir<-th)) {
        f1 = 'H';
#if 0
        Echo("%s: over,  fit to horizontal\n", __func__);
#endif
        qy = sin(dir*rf);
        q  = qy*qy;

        if(q<QLIMIT) {
            f3 = '0';
#if 0
Echo("qy %f q %f v.s. QLIMIT %f\n", qy, q, QLIMIT);
#endif
            dx = 0;
        }
        else {
#if 0
            dx = (tan(dir*rf)*(ht/2));
#endif
            dx = (tan((90-dir)*rf)*(ht/2));
        }
        if(qy>0) {
            f2 = '+';
            dy = ht/2;
        }
        else {
            f2 = '-';
            dy = -ht/2;
            dx = -dx;
        }

    }
    else {
        f1 = 'V';
#if 0
        Echo("%s: under, fit to vertical\n", __func__);
#endif
        qx = cos(dir*rf);
        q  = qx*qx;

        if(q<QLIMIT) {
            f3 = '0';
#if 0
Echo("qx %f q %f v.s. QLIMIT %f\n", qx, q, QLIMIT);
#endif
            dy = 0;
        }
        else {
            dy = (tan(dir*rf)*(wd/2));
        }

        if(qx>0) {
            f2 = '+';
            dx = wd/2;
        }
        else {
            f2 = '-';
            dx = -wd/2;
            dy = -dy;
        }

    }
#if 0
    Echo("dx,dy %f,%f f %c\n", dx, dy, f);
#endif
#if 0
    Echo("oid %-3d th %.3f dir %4d f %c%c%c q %.3f,%.3f q %.3f dx,dy %.3f,%.3f\n",
        u->oid, th, dir, f1, f2, f3, qx, qy, q, dx, dy);
#endif

    u->cx = *x+dx;
    u->cy = *y+dy;

    u->clx = u->cx - wd/2;
    u->crx = u->cx + wd/2;
    u->cby = u->cy - ht/2;
    u->cty = u->cy + ht/2;

    u->csx = *x;
    u->csy = *y;

    *x += dx*2;
    *y += dy*2;

    u->cex = *x;
    u->cey = *y;

#if 0
    int ik;
    ik = try_regline(u->cob.segar, u->csx, u->csy, u->cex, u->cey);
#endif


#if 0
    Echo("  oldx,y %d,%d\n", oldx, oldy);
#endif
#if 1
    Echo("%s: oid %d (%d %d %d %d)\n",
        __func__, u->oid, u->clx, u->cby, u->crx, u->cty);
    Echo("%s: a oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, u->oid,
        u->cx, u->cy, u->csx, u->csy, u->cex, u->cey, u->ox, u->coy);
#endif

#define QQXY(vx,vy) \
    if(vx<-INT_MAX/2||vx>INT_MAX/2) { \
        Echo("ERROR oid %d ignore x %s:%d\n", u->oid, __FILE__, __LINE__); \
    } \
    if(vy<-INT_MAX/2||vy>INT_MAX/2) { \
        Echo("ERROR oid %d ignore y %s:%d\n", u->oid, __FILE__, __LINE__); \
    } 

    QQXY(u->cx,u->cy);
    QQXY(u->cox,u->coy);
    QQXY(u->csx,u->csy);
    QQXY(u->cex,u->cey);

#if 1
    OP_LDUMP(__func__, "a", u);
#endif
#if 1
    Echo("%s: a %p oid %-3d xxdir %-4d *xy %6d,%-6d *fxy ------,------ xns %p\n",
            __func__, u, u->oid, xxdir, *x, *y, xns);
#endif

    return 0;
}



int
eval_dir(ob *u, int *xdir)
{
    int g;
    int odir;

    g = 0;
    odir = *xdir;

    /* XXX */
    if(u->type==CMD_UNIT)   { objunit = u->cob.iarg1; recalcsizeparam(); g++; }

    if(u->type==CMD_DIR)    { *xdir =   u->cob.iarg1; g++; }
    if(u->type==CMD_INCDIR) { *xdir +=  u->cob.iarg1; g++; }
    if(u->type==CMD_DECDIR) { *xdir -=  u->cob.iarg1; g++; }

    if(u->type==CMD_LTURN)  { *xdir +=   90; g++; }
    if(u->type==CMD_RTURN)  { *xdir -=   90; g++; }
    if(u->type==CMD_DOWN)   { *xdir =   -90; g++; }
    if(u->type==CMD_RIGHT)  { *xdir =     0; g++; }
    if(u->type==CMD_UP)     { *xdir =    90; g++; }
    if(u->type==CMD_LEFT)   { *xdir =   180; g++; }

    *xdir = dirnormalize(*xdir);

#if 0
    Echo("%s: oid %d type %d: g %d; dir %d -> %d\n",
        __func__, u->oid, u->type, g, odir, *xdir);
#endif
    if(g && _t_) {
#if 0
        Echo("%s: oid %d type %d: g %d; dir %d -> %d\n",
            __func__, u->oid, u->type, g, odir, *xdir);
#endif
    }

    return g;
}

static
int
isinside(int x1, int x2, int y1, int y2, double gx, double gy)
{
    int r=0;
    if(((gx>=x1 && gx<=x2) || (gx>=x2 && gx<=x1)) &&
       ((gy>=y1 && gy<=y2) || (gy>=y2 && gy<=y1))) {
        r = 1;
    }
    return r;
}

static
int
isinside2(int x1, int x2, int y1, int y2, double gx, double gy)
{
    int r=0;
    int tx, ty;
    tx = (int)gx;
    ty = (int)gy;
    if(((tx>=x1 && tx<=x2) || (tx>=x2 && tx<=x1)) &&
       ((ty>=y1 && ty<=y2) || (ty>=y2 && ty<=y1))) {
        r = 1;
    }
    return r;
}

int
bumpH(int x1, int y1, int x2, int y2, int cx, int cy, int cdir,
    int *rgx, int *rgy)
{
    int r;
    double a, b;
    double c, d;
    double gx, gy;
    int ik, ik2;

    Echo("%s: (%d %d %d %d) vs (%d,%d;%d)\n",
        __func__, x1, y1, x2, y2, cx, cy, cdir);

    r = -1;

    c = tan(cdir*rf);
    d = (double)cy - c * (double)cx;

    Echo("  c  %9.3f d  %9.3f\n", c, d);

    gy = y1;
    gx = (y1-d)/c;

    Echo("  gx %9.3f gy %9.3f\n", gx, gy);

    Echo("  x1,gx,x2 %d,%f,%d\n", x1, gx, x2);
    Echo("  y1,gy,y2 %d,%f,%d\n", y1, gy, y2);
    Echo("  q1 %d, q2 %d, q3 %d, q4 %d\n",
        (gx>=x1 && gx<=x2),
        (gx>=x2 && gx<=x1),
        (gy>=y1 && gy<=y2),
        (gy>=y2 && gy<=y1));

    ik = isinside(x1, x2, y1, y2, gx, gy);
    Echo("  ik %d\n", ik);
    ik = isinside2(x1, x2, y1, y2, gx, gy);
    Echo("  ik %d\n", ik);

    if(ik2) {
        Echo("  bumped INSIDE\n");
        *rgx = (int)gx;
        *rgy = (int)gy;
        r = 1;
    }
    else {
        Echo("  bumped outside\n");
        r = 0;
    }

    return r;
}


int
bumpV(int x1, int y1, int x2, int y2, int cx, int cy, int cdir,
    int *rgx, int *rgy)
{
    int r;
    double a, b;
    double c, d;
    double gx, gy;
    int ik, ik2;

    Echo("%s: (%d %d %d %d) vs (%d,%d;%d)\n",
        __func__, x1, y1, x2, y2, cx, cy, cdir);

    r = -1;

    c = tan(cdir*rf);
    d = (double)cy - c * (double)cx;

    Echo("  c  %9.3f d  %9.3f\n", c, d);

    gx = x1;
    gy = c*x1+d;

    Echo("  gx %9.3f gy %9.3f\n", gx, gy);

    Echo("  x1,gx,x2 %d,%f,%d\n", x1, gx, x2);
    Echo("  y1,gy,y2 %d,%f,%d\n", y1, gy, y2);
    Echo("  q1 %d, q2 %d, q3 %d, q4 %d\n",
        (gx>=x1 && gx<=x2),
        (gx>=x2 && gx<=x1),
        (gy>=y1 && gy<=y2),
        (gy>=y2 && gy<=y1));

    ik = isinside(x1, x2, y1, y2, gx, gy);
    Echo("  ik %d\n", ik);
    ik = isinside2(x1, x2, y1, y2, gx, gy);
    Echo("  ik %d\n", ik);

    if(ik2) {
        Echo("  bumped INSIDE\n");
        *rgx = (int)gx;
        *rgy = (int)gy;
        r = 1;
    }
    else {
        Echo("  bumped outside\n");
        r = 0;
    }

    return r;
}


int
bump(int x1, int y1, int x2, int y2, int cx, int cy, int cdir,
    int *rgx, int *rgy)
{
    int r;
    double a, b;
    double c, d;
    double gx, gy;

    Echo("%s: (%d %d %d %d) vs (%d,%d;%d)\n",
        __func__, x1, y1, x2, y2, cx, cy, cdir);

    r = -1;

    a = ((double)y2-(double)y1)/((double)x2-(double)x1);
    b = (double)y1 - a * (double)x1;

    Echo("  a  %9.3f b  %9.3f\n", a, b);

    c = tan(cdir*rf);
    d = (double)cy - c * (double)cx;

    Echo("  c  %9.3f d  %9.3f\n", c, d);

    gx = (d-b)/(a-c);
    gy = a*gx+b;

    Echo("  gx %9.3f gy %9.3f\n", gx, gy);

    if(((gx>=x1 && gx<=x2) || (gx>=x2 && gx<=x1)) &&
       ((gy>=y1 && gy<=y2) || (gy>=y2 && gy<=y1))) {
        Echo("  bumped INSIDE\n");
        *rgx = (int)gx;
        *rgy = (int)gy;
        r = 1;
    }
    else {
        Echo("  bumped outside\n");
        r = 0;
    }

    return r;
}

int
expand_paren(int bx, int by, int w, int h, int ox, int oy, ob* u)
{

Echo("%s: bx,by %d,%d w,h %d,%d ox,oy %d,%d\n",
        __func__, bx, by, w, h, ox, oy);
Echo("  vs oid %d cx,cy %d,%d w,h %d,%d noexpand? %d\n",
        u->oid, u->cx, u->cy, u->cwd, u->cht, u->cnoexpand);

    if(u->cnoexpand) {
    }

    u->cht = h;

    return 0;
}

int
expand_sep(int bx, int by, int w, int h, int ox, int oy, ob* u)
{
    int x1, y1, x2, y2;
    int c;
    int qx, qy;
    int ik;
    int mx[4], my[4];

#define A   \
    if(ik) { \
        if(c>=2) Echo("ERROR too many bump point\n"); \
        mx[c] = qx-ox; my[c] = qy-oy; c++; \
    }


Echo("%s: bx,by %d,%d w,h %d,%d ox,oy %d,%d\n",
        __func__, bx, by, w, h, ox, oy);
Echo("  vs oid %d cx,cy %d,%d\n", u->oid, u->cx, u->cy);

    c = 0;

    /* top hline */
    ik = bumpH(bx-w/2, by+h/2, bx+w/2, by+h/2,
            u->cx+ox, u->cy+oy, u->cob.sepcurdir+90, &qx, &qy);
    A;

    /* botom hline */
    ik = bumpH(bx-w/2, by-h/2, bx+w/2, by-h/2,
            u->cx+ox, u->cy+oy, u->cob.sepcurdir+90, &qx, &qy);
    A;

    /* left vline */
    ik = bumpV(bx-w/2, by-h/2, bx-w/2, by+h/2,
            u->cx+ox, u->cy+oy, u->cob.sepcurdir+90, &qx, &qy);
    A;

    /* right vline */
    ik = bumpV(bx+w/2, by-h/2, bx+w/2, by+h/2,
            u->cx+ox, u->cy+oy, u->cob.sepcurdir+90, &qx, &qy);
    A;

    Echo("  c %d\n", c);

    u->cob.sepx1 = mx[0]; u->cob.sepy1 = my[0];
    u->cob.sepx2 = mx[1]; u->cob.sepy2 = my[1];

    Echo("  oid %-4d 0 mx,my %d,%d\n", u->oid, mx[0], my[0]);
    Echo("           1 mx,my %d,%d\n",         mx[1], my[1]);

#undef A

    return 0;

}


int putchunk(ob *xch, int *x, int *y, ns *xns);

int
_putchunk_lane(ob *xch, int *rx, int *ry, ns *xns)
{
    int ik;
    int i;
    ob* u;
    int v;
    int minx, maxx;
    int miny, maxy;
    int c, maxc;
    int oldx, oldy;
    int lx, ly;
    int nx, ny;
    int qx, qy, hasqxy;
    int zx, zy;
    int g;
    ob* lob;
    int fx, fy;
    int lfx, lfy;
    ns *curns;

    int x, y;
    int xc, yc;

    int   ldir;

    int pointjoint;
    int uniqpoint;
    
    chas  regst;
    ch    bs;
    int   bsc=0;

    int   adjrewind=0;

    int maxw, maxh;
    int f, l;
    int gf, gl;
    int tdir;
    int gaph;
    int gapv;
    int maxf;

#if 0
P;
#endif
#if 0
    Echo("%s: oid %d x,y %d,%d\n", __func__, xch->oid, *rx, *ry);
    Echo("%s: b oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid,
        xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey, xch->ox, xch->coy);
#endif

#if 0
    Echo("%s: lanenum %d\n", __func__, xch->cch.lanenum);
#endif
    Echo("%s: lanenum %d\n", __func__, xch->cob.lanenum);

    if(xch->sizesolved) {
        Echo("oid %d already solved\n", xch->oid);
        return 0;
    }

    chas_init(&regst);
    cha_reset(&xch->cch);

    if(xch->cch.qns) {
        curns = xch->cch.qns;
    }

    gapv = xch->cob.lanegapv;
    gaph = xch->cob.lanegaph;


    maxw = -1;
    maxh = -1;
    maxf = -1;

    maxc = -1;

    c = 0;
    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*) xch->cch.ch[i];
        if(ISCHUNK(u->type) || HASBODY(u->type)) {
        }
        else {
            continue;
        }

        if(ISCHUNK(u->type)) {
            ik = putchunk(u, &nx, &ny, curns);
        }
        else {
#if 0
            ik = putobj(u, curns, &xch->cch.dir);
#endif
            tdir = 0;
            ik = putobj(u, curns, &tdir);
        }

        c++;

        if(u->wd > maxw) maxw = u->wd;
        if(u->ht > maxh) maxh = u->ht;
        if(c > maxc) maxc = c;      
    }


#if 0
    fprintf(stdout, "maxw %d maxh %d\n", maxw, maxh);
#endif


    xc = -1;
    yc = -1;
    if(maxc<xch->cob.lanenum) {
        xc = (maxc%xch->cob.lanenum);
    }
    else {
        xc = xch->cob.lanenum;
    }
    yc = ((maxc+xch->cob.lanenum-1)/xch->cob.lanenum);



    c = 0;
    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*) xch->cch.ch[i];

        if(ISCHUNK(u->type) || HASBODY(u->type)) {
        }
        else {
            continue;
        }

 #define MMD    (0)
 #define MMU    (1)
 #define MMR    (0)
 #define MML    (1)

        f = c / xch->cob.lanenum;
        l = c % xch->cob.lanenum;

        x = c % xch->cob.lanenum;
        y = c / xch->cob.lanenum;

        if(xch->cob.laneorder==LO_NWR) {
            u->cx = (maxw+gaph)*(x+1)  - maxw/2;
            u->cy = (maxh+gapv)*(yc-y) - maxh/2;
        }
        else
        if(xch->cob.laneorder==LO_NWD) {
            u->cx = (maxw+gaph)*(y+1)  - maxw/2;
            u->cy = (maxh+gapv)*(xc-x) - maxh/2;
        }
#if 0
        else
        if(xch->cob.laneorder==LO_SWU) {
            u->cx = (maxw+gaph)*(f+1) - maxw/2;
            u->cy = (maxh+gapv)*(l+1) - maxh/2;
        }
        else 
        if(xch->cob.laneorder==LO_SWR) {
            u->cx = (maxw+gaph)*(l+1) - maxw/2;
            u->cy = (maxh+gapv)*(f+1) - maxh/2;
        }
        else
        if(xch->cob.laneorder==LO_SWR) {
            u->cx = (maxw+gaph)*(l+1) - maxw/2;
            u->cy = (maxh+gapv)*(f+1) - maxh/2;
        }
        else
        if(xch->cob.laneorder==LO_SEL) {
            u->cx = (maxw+gaph)*(xch->cob.lanenum-l) - maxw/2;
            u->cy = (maxh+gapv)*(f+1) - maxh/2;
        }
#endif
        else {
            Error("not support laneorder %s (%d) yet\n",
                rassoc(lo_ial, xch->cob.laneorder), xch->cob.laneorder);
        }
    

        u->csx = u->cx - maxw/2;
        u->csy = u->cy;
        u->cex = u->cx + maxw/2;
        u->cey = u->cy;
        u->clx = u->cx - maxw/2;
        u->crx = u->cx + maxw/2;
        u->cby = u->cy - maxh/2;
        u->cty = u->cy + maxh/2;

        if(f>maxf) maxf = f;

        c++;
    }

    
    switch(xch->cob.laneorder) {
    case LO_SWU:
    case LO_NWD:
        xch->wd = yc*maxw+(yc+1)*gaph;
        xch->ht = xc*maxh+(xc+1)*gapv;
        break;
    default:
    case LO_NWR:
        xch->wd = xc*maxw+(xc+1)*gaph;
        xch->ht = yc*maxh+(yc+1)*gapv;
        break;
    }


    xch->cx = xch->wd/2;
    xch->cy = xch->ht/2;
    xch->csx = 0;
    xch->csy = xch->ht/2;
    xch->cex = xch->wd;
    xch->cey = xch->ht/2;
    xch->clx = 0;
    xch->crx = xch->wd;
    xch->cby = 0;
    xch->cty = xch->ht;
    xch->ox  = -xch->wd/2;
    xch->oy  = -xch->ht/2;


    return 0;
}

int
_putchunk(ob *xch, int *x, int *y, ns *xns)
{
    int ik;
    int i;
    ob* u;
    int v;
    int minx, maxx;
    int miny, maxy;
    int c;
    int oldx, oldy;
    int lx, ly;
    int nx, ny;
    int qx, qy, hasqxy;
    int zx, zy;
    int g;
    ob* lob;
    int fx, fy;
    int lfx, lfy;
    ns *curns;

    int isx, isy;
    int iex, iey;
    int ckfrom;
    int ckto;

    int   ldir;

    int pointjoint;
    int uniqpoint;
    
    chas  regst;
    ch    bs;
    int   bsc=0;

    int   adjrewind=0;

#if 0
P;
#endif
#if 0
    Echo("%s: oid %d x,y %d,%d\n", __func__, xch->oid, *x, *y);
    Echo("%s: b oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid,
        xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey, xch->ox, xch->coy);
#endif

    if(xch->sizesolved) {
        Echo("oid %d already solved\n", xch->oid);
        return 0;
    }

    chas_init(&regst);
    cha_reset(&xch->cch);

    if(xch->cch.qns) {
        curns = xch->cch.qns;
    }

    oldx = 0;
    oldy = 0;

    nx = 0;
    ny = 0;
#if 1
    xch->sx = nx;
    xch->sy = ny;
#endif

    lob = NULL;
    u   = NULL;

    pointjoint = 0;

    c = 0;
    for(i=0;i<xch->cch.nch;i++) {
        c++;

        lob  = u;
        lx   = oldx;
        ly   = oldy;
        ldir = xch->cch.dir;
        lfx  = fx;
        lfy  = fy;

        u = (ob*) xch->cch.ch[i];

        xch->cch.x = nx;
        xch->cch.y = ny;

#if 1
    /*XXX */
        if(u->type==CMD_GOTO) {
            if(u->cob.aat) {
                int r;
                int dx, dy;

    Echo("goto AT '%s'\n", u->cob.aat);

                r = ns_find_objpos(curns, u->cob.aat, &dx, &dy);
    Echo("  ns_find_objpos  '%s' ret %d\n", u->cob.aat, r);
                if(r==0) {
                    nx = dx; ny = dy;
                }
                continue;
            }
        }
#endif

        if(u->type==CMD_BACK) {
            if(c>1) {
                nx = lx;
                ny = ly;
Echo("  oid %d back %d,%d\n", u->oid, nx, ny);
                continue;
            }
        }
        if(u->type==CMD_HBACK) {
            if(c>1) {
                nx = (lx+nx)/2;
                ny = (ly+ny)/2;
Echo("  oid %d hback %d,%d\n", u->oid, nx, ny);
                continue;
            }
        }
        if(u->type==CMD_SHORE) {
            ob* fob;

            fob = NULL;

Echo("u %p oid %d iarg1 %d\n", u, u->oid, u->cob.iarg1);
            if(u->cob.ato) {    
                    fob = ns_find_obj(curns, u->cob.ato);
            }
            else {
                if(lob) {
                    fob = lob;
                }
            }

            if(fob) {
Echo("fob %p\n", fob);
                ik = takelastobjpos(fob, u->cob.iarg1, u->cob.keepdir,
                        &nx, &ny, &xch->cch.dir);
                continue;
            }
        }

        if(u->type==CMD_SAVE) {
#if 0
Echo("  oid %d b save %d,%d\n", u->oid, nx, ny);
Echo("  oid %d a save %d,%d\n", u->oid, bs.x, bs.y);
#endif
            cha_copy(&bs, &xch->cch);
Echo("  oid %d save %d,%d\n", u->oid, bs.x, bs.y);

            bsc++;
            continue;
        }
        if(u->type==CMD_RESTORE) {
            if(bsc<=0) {
                Echo("ERROR no saved attributes\n");
                continue;
            }
            
            cha_copy(&xch->cch, &bs);
            nx = xch->cch.x;
            ny = xch->cch.y;

Echo("  oid %d restore to %d,%d\n", u->oid, nx, ny);
            bsc--;
            continue;
        }

        if(u->type==CMD_PUSH) {
            chas_push(&regst, &xch->cch);
Echo("  oid %d push  %d,%d %d\n", u->oid, bs.x, bs.y, bs.dir);
Echo("  oid %d push  %d,%d %d\n", u->oid, 
    regst.slot[regst.use-1].x,
    regst.slot[regst.use-1].y,
    regst.slot[regst.use-1].dir);

            continue;
        }
        if(u->type==CMD_POP) {
            ch tmp;

            chas_pop(&regst, &tmp);

            cha_copy(&xch->cch, &tmp);
            nx = tmp.x;
            ny = tmp.y;

Echo("  oid %d pop   %d,%d %d\n", u->oid, nx, ny, xch->cch.dir);
            continue;
        }
        if(u->type==CMD_AGAIN) {
            ch tmp;

            chas_top(&regst, &tmp);

            cha_copy(&xch->cch, &tmp);
            nx = tmp.x;
            ny = tmp.y;

Echo("  oid %d again %d,%d %d\n", u->oid, nx, ny, xch->cch.dir);
            continue;
        }

        oldx = nx;
        oldy = ny;


#define PK(m,d) \
  Echo("PK %s oid %d %d,%d %d,%d %d,%d->%d,%d %dx%d (%d,%d,%d,%d)\n", \
  (m), (d)->oid, \
  (d)->x, (d)->y, (d)->ox, (d)->oy, (d)->sx, (d)->sy, (d)->ex, (d)->ey, \
  (d)->wd, (d)->ht, (d)->lx, (d)->by, (d)->rx, (d)->ty);


#define C(d,s) \
            (d)->wd = (s)->wd; \
            (d)->ht = (s)->ht; \
            (d)->x  = (s)->x;  \
            (d)->y  = (s)->y;  \
            (d)->ox = (s)->ox; \
            (d)->oy = (s)->oy; \
            (d)->lx = (s)->lx; \
            (d)->by = (s)->by; \
            (d)->rx = (s)->rx; \
            (d)->ty = (s)->ty; 

        g = eval_dir(u, &(xch->cch.dir));
        if(g>0) {
            continue;
        }
#if 0
        if(u->hasrel) {
            /* do nothing */
        }
#endif
        if(u->floated) {
            /* do nothing */
        }
        else {
        }

#if 0
Echo("  last pos %d,%d dir %d\n", lx, ly, ldir);
#endif

        if(ISCHUNK(u->type)) {
#if 0
Echo("chunk oid %d -> chunk oid %d start\n", xch->oid, u->oid);
#endif
            ik = putchunk(u, &nx, &ny, curns);
#if 0
Echo("chunk oid %d -> chunk oid %d ; ik %d\n", xch->oid, u->oid, ik);
#endif

        }
        else {
#if 0
Echo("chunk oid %d -> obj oid %d\n", xch->oid, u->oid);
#endif
            ik = putobj(u, curns, &xch->cch.dir);
#if 1
Echo("chunk oid %d -> obj oid %d ; putobj ik %d\n", xch->oid, u->oid, ik);
#endif

        }

#if 1
Echo("  fx,fy %d,%d dir %d; ik %d\n",
    u->fx, u->fy, xch->cch.dir, ik);
#endif
#if 1
Echo("  fx-ox,fy-oy %d,%d dir %d; ik %d\n",
    u->fx-u->ox, u->fy-u->oy, xch->cch.dir, ik);
#endif

        fx = u->fx;
        fy = u->fy;
        
        if(ik) {
            uniqpoint = 1;
        }
        else {
            uniqpoint = 0;
        }

        adjrewind = 0;
        hasqxy = 0;


#define S(x)    ((x)?(x):"*none*")
P;
Echo("POS oid %d from %s to %s at %s\n",
    u->oid, S(u->cob.afrom), S(u->cob.ato), S(u->cob.aat));
#undef S
Echo("POS oid %d from %d to %d\n",
    u->oid, u->cob.hasfrom, u->cob.hasto);

#if 0
        if(u->cob.afrom) {
            int r;
            int dx, dy;

P;
Echo("found FROM '%s'\n", u->cob.afrom);

#if 0
            r = ns_find_objpos(curns, u->cob.afrom, &dx, &dy);
Echo("  ns_find_objpos '%s' ret %d; %d,%d\n", u->cob.afrom, r, dx, dy);
#endif
            r = _ns_find_objposP(curns, u, u->cob.afrom, 1 /*LPOS*/, &dx, &dy);
Echo("  ns_find_objposP '%s' ret %d; %d,%d\n", u->cob.afrom, r, dx, dy);
            if(r==0) {
                nx = dx; ny = dy;
                adjrewind++;

                u->pst += 1;
            }
            else {
                E;
            }
        }
        if(u->cob.ato) {
            int r;
            int dx, dy;
P;
Echo("found TO '%s'\n", u->cob.ato);

#if 0
            r = ns_find_objpos(curns, u->cob.ato, &qx, &qy);
Echo("  ns_find_objpos '%s' ret %d; %d,%d\n", u->cob.ato, r, dx, dy);
#endif
            r = _ns_find_objposP(curns, u, u->cob.ato, 1 /*LPOS*/, &dx, &dy);
Echo("  ns_find_objposP '%s' ret %d; %d,%d\n", u->cob.ato, r, dx, dy);
            if(r==0) {
                qx = dx; qy = dy;
                hasqxy++;
                adjrewind++;

                u->pst += 1;
            }
            else {
                E;
            }
        }
#endif

#if 1
        if(u->cob.aat) {
            int r;
            int dx, dy;

P;
Echo("found AT '%s'\n", u->cob.aat);

#if 0
            r = ns_find_objpos(curns, u->cob.aat, &dx, &dy);
Echo("  ns_find_objpos  '%s' ret %d; %d,%d\n", u->cob.aat, r, dx, dy);
#endif
            r = _ns_find_objposP(curns, u, u->cob.aat, 1 /*LPOS*/, &dx, &dy);
Echo("  ns_find_objposP '%s' ret %d; %d,%d\n", u->cob.aat, r, dx, dy);
            if(r==0) {
                nx = dx; ny = dy;

                adjrewind++;

                u->pst += 10;
            }
            else {
                E;
            }

  #if 1
        if(u->cob.awith) {
            int r;
            int dx, dy;

Echo("found WITH '%s'\n", u->cob.awith);

            dx = nx; dy = ny;
            r = applywith(u, u->cob.awith, &dx, &dy);
Echo("  applywith '%s' ret %d; %d,%d\n", u->cob.awith, r, dx, dy);
            if(r==0) {
                nx = dx; ny = dy;
                adjrewind++;
                u->pst += 100;
            }
            else {
                E;
            }
        }
                
        if(adjrewind>0) {
            int r;
            int dx, dy;

            dx = nx; dy = ny;
            r = rewindcenter(u, xch->cch.dir, &dx, &dy);

Echo("  rewindcenter dir %d ret %d; %d,%d\n", xch->cch.dir, r, dx, dy);
            if(r==0) {
                nx = dx; ny = dy;
                u->pst += 1000;
            }
            else {
                E;
            }
        }

        }
  #endif
#endif

skip_at_with:

    Echo("oid %-3d floated %d originalshape %d uniqpoint %d; pointjoint %d\n",
        u->oid, u->floated, u->cob.originalshape, uniqpoint, pointjoint);


    ckfrom = -1;
    if(u->cob.hasfrom) {
        ckfrom = find_from(u, &isx, &isy);
        Echo("oid %d ckfrom %d\n", u->oid, ckfrom);
        Echo("  isx,isy %d,%d\n", isx, isy);
        u->pst += 10000;
    }

    ckto = -1;
    if(u->cob.hasto) {
        ckto = find_to_last(u, &iex, &iey);
        Echo("oid %d ckto %d\n", u->oid, ckto);
        Echo("  iex,iey %d,%d\n", iex, iey);
        u->pst += 10000;
    }

PK("b",u);
#if 1
    if(u->floated) {    /* fitting is not require */
#if 0
            _obj_shift(u, (u->lx),  (u->by));

            _obj_shift(u, nx - (u->lx),  ny - (u->by));
            _obj_shift(u, u->wd/2, u->ht/2);
#endif
PK("m",u);
#if 0
            fitobj_LBRT(u, ldir, &zx1, &zy1, &zx2, &zy2, curns);
            fitobj_wdht(u, ldir, &zx1, &zy1, curns);
#endif
    }
    else
#endif
    if(uniqpoint) {
        int gfx, gfy;

        gfx = fx;
        gfy = fy;

        if(ckfrom==1) {
            nx = isx;
            ny = isy;
        }

P;
        fitobj_LBRT(u, ldir, &nx, &ny, &gfx, &gfy, curns);
    }
    else if(ISGLUE(u->type)) {
        int gfx, gfy;

        gfx = fx;
        gfy = fy;

        if(ckfrom==1) {
            nx = isx;
            ny = isy;
        }

P;
        fitobj_LBRT(u, ldir, &nx, &ny, &gfx, &gfy, curns);
    }
    else {

        if(pointjoint && u) {
            int gfx, gfy;

#if 0
Echo("  pointjoint %d to lfx,lfy %d,%d ldir %d\n",
    pointjoint, lfx, lfy, ldir);
#endif

                u->pst += 30000;

            gfx = lfx;
            gfy = lfy;
P;
            fitobj_wdht(u, xch->cch.dir, &nx, &ny, curns);
#if 0
#endif
        }
        else {
                u->pst += 40000;

P;
            fitobj_wdht(u, xch->cch.dir, &nx, &ny, curns);
        }

    }
PK("a",u);

#if 0
Echo("\tnx,ny = %d,%d\n", nx, ny);
#endif

        if(uniqpoint) {
            pointjoint = 1;
        }
        else {
            pointjoint = 0;
        }


#if 0
        xch->cch.x = nx;
        xch->cch.x = ny;
#endif

#if 0
Echo("  oid %d move %d,%d to %d,%d\n", u->oid, oldx, oldy, nx, ny);
#endif
    }

#if 1
    xch->ex = nx;
    xch->ey = ny;
#endif

#if 0
    Echo("%s: m oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid,
        xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey, xch->ox, xch->coy);
#endif

#if 0
Echo("BB\n");
Echo("xch ox,oy %d, %d\n", xch->ox, xch->oy);
#endif

    if(xch->cch.nch>0) {

#if 0
        for(i=0;i<xch->cch.nch;i++) {
            u = (ob*)xch->cch.ch[i];
            Echo("%4d: oid %d: %5d %5d -> %5d %5d\n",
                i, u->oid, u->sx, u->sy, u->ex, u->ey);
        }
            Echo("here: oid %d: %5d %5d -> %5d %5d\n",
                xch->oid, xch->sx, xch->sy, xch->ex, xch->ey);
#endif

        maxx = -(INT_MAX-1);
        minx = INT_MAX;
        maxy = -(INT_MAX-1);
        miny = INT_MAX;

#if 1
        Echo("\t           : oid %d\n", xch->oid);
        Echo("\t # : oid: i:      x      y     ox     oy :     lx     by     rx     ty\n");

#endif
        v = 0;
        for(i=0;i<xch->cch.nch;i++) {
            u = (ob*)xch->cch.ch[i];
#if 1
            Echo("\t%3d: %3d: %d: %6d %6d %6d %6d : %6d %6d %6d %6d\n",
                i, u->oid, u->ignore, 
                u->cx, u->cy,
                u->cox, u->coy,
                u->clx, u->cby, u->crx, u->cty);
#endif
            if(u->ignore) {
#if 0
    Echo("oid %d ignored\n", u->oid);
#endif
                continue;
            }
            v++;
#if 0
            if(u->type==CMD_CHUNK) 
#endif
            if(ISCHUNK(u->type)) 
            {
            }
            else {
            }

            if(u->type==CMD_LINE||u->type==CMD_ULINE) {
            }
                Echo("OUT oid %-3d (%6d %6d %6d %6d) %6d x %-6d ; %6d x %-6d\n",
                    u->oid,
                    u->clx, u->cby, u->crx, u->cty,
                    u->crx-u->clx, u->cty-u->cby,
                    u->cwd, u->cht);

#if 1
            if(u->floated) {
                /* nothing */
            }
            else {
                if(u->clx<minx) { minx = u->clx; }
                if(u->crx<minx) { minx = u->crx; }
                if(u->clx>maxx) { maxx = u->clx; }
                if(u->crx>maxx) { maxx = u->crx; }
                if(u->cby<miny) { miny = u->cby; }
                if(u->cty<miny) { miny = u->cty; }
                if(u->cby>maxy) { maxy = u->cby; }
                if(u->cty>maxy) { maxy = u->cty; }
            }
#endif

        }

#if 1
        Echo(
        "\tsemi       : %6s %6s %6s %6s : %6d %6d %6d %6d (%d/%d)\n",
            "", "", 
            "", "",
            minx, miny, maxx, maxy, v, c);
#endif


        xch->clx = minx;
        xch->cby = miny;
        xch->crx = maxx;
        xch->cty = maxy;
        xch->cmr = sqrt((maxx-minx)/2*(maxx-minx)/2+
                        (maxy-miny)/2*(maxy-miny)/2);

    }
    else {
        /* no member */
        xch->clx = 0;
        xch->cby = 0;
        xch->crx = 0;
        xch->cty = 0;
        xch->cmr = 0;
    }


    xch->wd = (maxx - minx);
    xch->ht = (maxy - miny);
    zx = minx + xch->wd/2;
    zy = miny + xch->ht/2;
    xch->ox = -zx;
    xch->oy = -zy;

#if 1
    Echo("%s: n oid %d nx,ny %d,%d\n", __func__, xch->oid, nx, ny);
    Echo("  lx,ly %d,%d\n", lx, ly);

    Echo("  x %d..%d\n", minx, maxx);
    Echo("  y %d..%d\n", miny, maxy);
    Echo("  zx,zy %d,%d\n", zx, zy);
    Echo("  wxh %dx%d\n", xch->wd, xch->ht);
    Echo("  oxy %d,%d\n", xch->ox, xch->oy);
#endif

    /***
     *** check SEP
     ***/
    for(i=0;i<xch->cch.nch;i++) {
        int ik;
        int qx, qy;
        u = (ob*)xch->cch.ch[i];
        if(!MAYEXPAND(u->type)) {
            continue;
        }
        if(u->cnoexpand) {
            continue;
        }
        switch(u->type) {
        case CMD_SEP:
        case CMD_PLINE:
            Echo("SEP/PLINE oid %d %3d: oid %d xy %d,%d curdir %d\n",
                xch->oid, i, u->oid, u->cx, u->cy, u->cob.sepcurdir);

            ik = expand_sep(xch->cx, xch->cy, xch->cwd, xch->cht,
                    xch->ox, xch->oy, u);
            break;
        case CMD_LPAREN:
        case CMD_RPAREN:
        case CMD_LBRACKET:
        case CMD_RBRACKET:
        case CMD_LBRACE:
        case CMD_RBRACE:
            Echo("PAREN oid %d %3d: oid %d xy %d,%d curdir %d\n",
                xch->oid, i, u->oid, u->cx, u->cy, u->cob.sepcurdir);

            ik = expand_paren(xch->cx, xch->cy, xch->cwd, xch->cht,
                    xch->ox, xch->oy, u);
            break;
        }
    }


    xch->wd = (maxx - minx) + xch->cgimargin*2;
    xch->ht = (maxy - miny) + xch->cgimargin*2;
    zx = minx - xch->cgimargin + xch->wd/2;
    zy = miny - xch->cgimargin + xch->ht/2;
    xch->ox = -zx;
    xch->oy = -zy;

        xch->clx = minx-xch->cgimargin;
        xch->cby = miny-xch->cgimargin;
        xch->crx = maxx+xch->cgimargin;
        xch->cty = maxy+xch->cgimargin;


#if 1
    Echo("  gimargin %d\n", xch->cgimargin);
    Echo("  x %d..%d\n", minx, maxx);
    Echo("  y %d..%d\n", miny, maxy);
    Echo("  zx,zy %d,%d\n", zx, zy);
    Echo("  wxh %dx%d\n", xch->wd, xch->ht);
    Echo("  oxy %d,%d\n", xch->ox, xch->oy);
#endif


#if 1
    Echo("  new ox,oy %d,%d\n", xch->ox, xch->oy);

    Echo("\ttotal %3d  : %5d %5d %5d %5d : %5d %5d %5d %5d (%d/%d) w,h %d,%d\n",
        xch->oid,
        xch->x, xch->y, xch->ox, xch->oy,
        xch->lx, xch->by, xch->rx, xch->ty, v, c, xch->wd, xch->ht);
#endif


    xch->sizesolved++;

#if 0
Echo("chunk oid %d solved\n", xch->oid);

#endif




out:

#if 1
    Echo("%s: a oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid,
        xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey,
            xch->ox, xch->coy);
#endif
#if 1
    Echo("%s: oid %d b bb g %7d %7d %7d %7d\n",
            __func__, xch->oid,
            xch->glx, xch->gby, xch->grx, xch->gty);
    Echo("             bb _ %7d %7d %7d %7d\n",
            xch->lx, xch->by, xch->rx, xch->ty);
#endif

    return 0;
}

int
putchunk(ob *xch, int *x, int *y, ns *xns)
{
    int ik;

#if 0
    Echo("%s: oid %d lanenum %d\n", __func__, xch->oid, xch->cch.lanenum);
    if(xch->cch.lanenum>0) 
#endif
    Echo("%s: oid %d dir %d lanenum %d\n",
        __func__, xch->oid, xch->cch.dir, xch->cob.lanenum);
    if(xch->cob.lanenum>0) 
    {
        ik = _putchunk_lane(xch, x, y, xns);
    }
    else {
        ik = _putchunk(xch, x, y, xns);
    }
    return ik;
}

int
put(ob *xch, int *x, int *y, ns *xns)
{
    int ik;

P;
    ik = putchunk(xch, x, y, xns);
P;
#if 0
    ik = fitobj_wdht(xch, def_dir, x, y, xns);
#endif
    ik = fitobj_wdht(xch, xch->cch.dir, x, y, xns);
    return ik;
}
