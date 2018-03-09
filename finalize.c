
#include <stdio.h>

#include "alist.h"
#include "obj.h"
#include "gv.h"
#include "seg.h"
#include "chas.h"
#include "notefile.h"
#include "a.h"



#define MARK(m,x,y) \
    Echo("MARK finalize oid %d line %d %s x %4d, y %4d\n", \
        u->oid, __LINE__, m, x, y); \
    if(x<_lx) { _lx = x; } if(x>_rx) { _rx = x; }\
    if(y<_by) { _by = y; } if(y>_ty) { _ty = y; } 

int
finalizeobj(ob *u, int *xdir, int ox, int oy, ns *xns)
{
    int wd, ht;
    int g;
    ob* np;
    int nx, ny;
    int _lx, _rx, _ty, _by;
P;
    if(u->finalized) {
Echo("%s: oid %d finalized skip\n", __func__, u->oid);
        return -1;
    }
Echo("%s: oid %d hasrel %d ox,oy %d,%d\n", __func__, u->oid, u->hasrel, ox, oy);

    _rx = _ty = -(INT_MAX-1);
    _lx = _by = INT_MAX;

#define XXPN(n) \
    if(u->c##n) { \
        np = ns_find_obj(xns, u->c##n); \
        if(np) { \
            if(!np->fixed) Echo("found but not fixed\n"); \
            nx = np->gx; \
            ny = np->gy; \
            Echo("  found " #n " as (%d,%d)\n", nx, ny); \
            MARK("objxy", nx, ny) \
        } \
        else { \
            Echo("ERROR not found as '" #n "' '%s'\n", u->c##n); \
        } \
    }

#define PN(n) \
    if(u->c##n) { \
        int ik;\
        ik = ns_find_objpos(xns, u->c##n, &nx, &ny); \
        if(np) { \
            Echo("  found " #n " as (%d,%d)\n", nx, ny); \
            MARK("objxy", nx, ny) \
        } \
        else { \
            Echo("ERROR not found as '" #n "' '%s'\n", u->c##n); \
        } \
    }

    PN(afrom);
    PN(ato);
    PN(awith);
    PN(aat);
#if 0
#endif

    g = eval_dir(u, xdir);
    if(g>0) {
        goto out;
    }

    Echo("obj oid %d shift %d,%d\n",  u->oid, ox, oy);

    u->cfixed = 1;
    u->cgx  = ox+u->cx;     u->cgy  = oy+u->cy;
    u->cgsx = ox+u->csx;    u->cgsy = oy+u->csy;
    u->cgex = ox+u->cex;    u->cgey = oy+u->cey;

    u->cob.gsepx1 = ox + u->cob.sepx1;
    u->cob.gsepy1 = oy + u->cob.sepy1;
    u->cob.gsepx2 = ox + u->cob.sepx2;
    u->cob.gsepy2 = oy + u->cob.sepy2;

    Echo("    c  %5d %5d : %5d %5d -> %5d %5d\n",
            u->cx, u->cy, u->csx, u->csy, u->cex, u->cey);
    Echo("    cg %5d %5d : %5d %5d -> %5d %5d\n",
            u->cgx, u->cgy, u->cgsx, u->cgsy, u->cgex, u->cgey);

    Echo("= oid %d type %d x,y %d,%d: gx,gy %d,%d\n",
        u->oid, u->type, u->cx, u->cy, u->cgx, u->cgy);

#if 0
    Echo("obb oid %-4d s1 %6d %6d %6d %6d\n",
        u->oid, _lx, _by, _rx, _ty);
#endif

#if 0
    MARK("objcx ",  ox+u->cx,       oy+u->cy);
    MARK("objcsx",  ox+u->csx,      oy+u->cey);
    MARK("objcex",  ox+u->cex,      oy+u->cey);
    MARK("objcLB",  ox+u->cx-u->wd/2, oy+u->cy-u->ht/2);
    MARK("objcRT",  ox+u->cx+u->wd/2, oy+u->cy+u->ht/2);
#endif

#if 1
    MARK("objcgx ", u->cgx,       u->cgy);
    MARK("objcgsx", u->cgsx,      u->cgey);
    MARK("objcgex", u->cgex,      u->cgey);
    MARK("objcLB",  ox+u->cx-u->wd/2, oy+u->cy-u->ht/2);
    MARK("objcRT",  ox+u->cx+u->wd/2, oy+u->cy+u->ht/2);
#endif

    Echo("obb oid %-4d s2 %6d %6d %6d %6d ; %6d x %6d\n",
        u->oid, _lx, _by, _rx, _ty, (_rx-_lx), (_ty-_by));

#if 0
#include <assert.h>
assert(_lx<=_rx);
assert(_by<=_ty);
#endif

    u->glx = _lx;
    u->gby = _by;
    u->grx = _rx;
    u->gty = _ty;

#if 1
    u->gx  = (_lx+_rx)/2;
    u->gy  = (_by+_ty)/2;
#endif

#if 0
    Echo("obb oid %d s3 %d %d: %d %d %d %d\n",
        u->oid, u->gx, u->gy, u->glx, u->gby, u->grx, u->gty);
#endif

    Echo("%s: oid %d finalized %d %d: %d %d %d %d\n",
        __func__, u->oid, u->gx, u->gy, u->glx, u->gby, u->grx, u->gty);
    u->finalized++;

out:
    return 0;
}


int
finalizechunk(ob *xch, int gox, int goy, ns *xns)
{
    int ik;
    int i;
    ob* u;
    int _lx, _rx, _ty, _by;
    ns  *curns;

    if(xch->finalized) {
        Echo("%s: oid %d finalized skip\n", __func__, xch->oid);
        return -1;
    }
P;
    if(xch->cch.nch<=0) {
        Echo("%s: oid %d has no member\n", __func__, xch->oid);

        xch->glx = xch->grx = xch->gx  = gox;
        xch->gby = xch->gty = xch->gy  = goy;

        goto success;
    }

    _rx = _ty = -(INT_MAX-1);
    _lx = _by = INT_MAX;

    cha_reset(&xch->cch);

    if(xch->cch.qns) {
        curns = xch->cch.qns;
    }
    else {
        curns = xns;
    }

    xch->cfixed = 1;
    xch->cgx  = gox+xch->cx;    xch->cgy  = goy+xch->cy;
    xch->cgsx = gox+xch->csx;   xch->cgsy = goy+xch->csy;
    xch->cgex = gox+xch->cex;   xch->cgey = goy+xch->cey;

Echo("chunk shift %d,%d oid %d\n", gox, goy, xch->oid);

    Echo("= oid %d type %d x,y %d,%d: gx,gy %d,%d\n",
        xch->oid, xch->type, xch->cx, xch->cy, xch->cgx, xch->cgy);
#if 0
    MARK("chlb", gox+xch->lx, goy+xch->by);
    MARK("chrt", gox+xch->rx, goy+xch->ty);

    Echo("cb oid %d m: %d %d %d %d\n",
        xch->oid, _lx, _by, _rx, _ty);
#endif

    
    if(xch->cch.qns) 
        curns = xch->cch.qns;
    else
        curns = xns;

    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*)xch->cch.ch[i];
        if(ISCHUNK(u->type)) {
#if 0
            ik = finalizechunk(u,
                    xch->x+xch->ox+u->ox,
                    xch->y+xch->oy+u->oy, xns);
#endif
            ik = finalizechunk(u,
                    gox+xch->ox+xch->x,
                    goy+xch->oy+xch->y, curns);

#if 1
            MARK("c-clb", u->glx, u->gby);
            MARK("c-crt", u->grx, u->gty);
#endif
        }
        else {
#if 1
            if(u->ignore) {
                continue;
            }
#endif

#if 0
            ik = finalizeobj(u, &xch->cch.dir,
                    xch->x+xch->ox, xch->y+xch->oy, xns);
#endif
            ik = finalizeobj(u, &xch->cch.dir,
                    gox+xch->ox+xch->x,
                    goy+xch->oy+xch->y, curns);

#if 1
            MARK("c-objlb", u->glx, u->gby);
            MARK("c-objrt", u->grx, u->gty);
#endif
        }
            Echo("cbb oid %d-%d oid %d: %d %d %d %d\n",
                xch->oid, i, u->oid, _lx, _by, _rx, _ty);
    }

    Echo("cbb oid %d-e: -: %d %d %d %d\n",
        xch->oid, _lx, _by, _rx, _ty);

    xch->glx = _lx;
    xch->grx = _rx;
    xch->gby = _by;
    xch->gty = _ty;
#if 0
    u->gx  = (_lx+_rx)/2;
    u->gy  = (_by+_ty)/2;
#endif
#if 1
    xch->gx  = (_lx+_rx)/2;
    xch->gy  = (_by+_ty)/2;
#endif

success:
    Echo("%s: oid %d finalized %d %d: %d %d %d %d\n",
        __func__, xch->oid,
        xch->gx, xch->gy, xch->glx, xch->gby, xch->grx, xch->gty);
    xch->finalized++;

out:
    return 0;
}

int
finalize(ob *xch, int x, int y, ns *xns)
{
    int ik;
    ik = finalizechunk(xch, x, y, xns);
    return ik;
}

#undef MARK
