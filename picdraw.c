#define PD(ux,uy) \
 printf("+ line from (%d,%d) to (%d,%d) to (%d,%d) to (%d,%d) to (%d,%d)\n", \
        ux-3,uy,ux,uy+3,ux+3,uy,ux,uy-3,ux-3,uy)
#define PC(ux,uy) \
 printf("+ line from (%d,%d) to (%d,%d)\n" \
        "+ line from (%d,%d) to (%d,%d)\n", \
        ux-3,uy-3,ux+3,uy+3, ux-3,uy+3,ux+3,uy-3);
#define PB(q) \
 printf("+ line from (%d,%d) to (%d,%d) to (%d,%d) to (%d,%d) to (%d,%d) dashed # PB\n", \
    (q)->clx,(q)->cby,(q)->clx,(q)->cty,(q)->crx,(q)->cty,(q)->crx,(q)->cby, \
    (q)->clx,(q)->cby);
#define PQ(q) \
    {   int zx, zy; \
        zx = ((q)->clx+(q)->crx)/2; zy = ((q)->cty+(q)->cby)/2; \
        printf("+ circle at (%d,%d) rad 3.5\n", zx, zy); \
        printf("+ circle at (%d,%d) rad 3\n", zx, zy); }
#define PQQ(q) \
    {   int zx, zy; \
        zx = ((q)->clx+(q)->crx)/2; zy = ((q)->cty+(q)->cby)/2; \
        printf("+ circle at (%d,%d) rad 1.2\n", zx, zy); \
        printf("+ circle at (%d,%d) rad 0.7 fill 1\n", zx, zy); }


int
picdraw_arrowhead(int atype, int xdir, int x, int y)
{
    int dx, dy;
    int r;
#if 0
    r = u->cht/2;
#endif
    r = def_arrowsize;

    dx =  (int)(r*cos((xdir+180+def_arrowangle/2)*rf));
    dy =  (int)(r*sin((xdir+180+def_arrowangle/2)*rf));
    printf("+   line from (%d,%d) to (%d,%d) thick 1.5\n",
        x, y, x+dx, y+dy);

    dx =  (int)(r*cos((xdir+180-def_arrowangle/2)*rf));
    dy =  (int)(r*sin((xdir+180-def_arrowangle/2)*rf));
    printf("+   line from (%d,%d) to (%d,%d) thick 1.5\n",
        x, y, x+dx, y+dy);

    return 0;
}

int
picdraw_linearrow(int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int x2, y2;

    printf("+ # linearrow head %d %d %d\n", 
        xu->cob.arrowheadpart,
        xu->cob.arrowbackheadtype,
        xu->cob.arrowbackheadtype);

    x1 = xox+xu->csx;
    y1 = xoy+xu->csy;
    x2 = xox+xu->cex;
    y2 = xoy+xu->cey;

    pf = NULL;
    if(xu->cafrom) {
        pf = ns_find_obj(xns, xu->cafrom);
        printf("%s: pf %p\n", __func__, pf);
        if(!pf) {
            goto to_phase;
        }
        if(pf->cfixed) {
            printf("  from %d,%d\n", pf->gx, pf->gy);
            x1 = pf->gx;
            y1 = pf->gy;
        }
    }

to_phase:

    pt = NULL;
    if(xu->cafrom) {
        pt = ns_find_obj(xns, xu->cato);
        printf("%s: pt %p\n", __func__, pt);
        if(!pt) {
            goto apply;
        }
        if(pt->cfixed) {
            printf("  to %d,%d\n", pt->gx, pt->gy);
            x2 = pt->gx;
            y2 = pt->gy;
        }
    }

apply:
    printf("%s: from (%d,%d) to (%d,%d)\n", __func__, x1, y1, x2, y2);
#if 0
    printf("+ line from (%d,%d) to (%d,%d) # line\n", x1, y1, x2, y2);
#endif
    printf("+ line from (%d,%d) to (%d,%d) <-> thick 2 # line\n", x1, y1, x2, y2);


#if 0
    if(xu->cob.arrowforeheadtype>0) {
#endif
    if(xu->cob.arrowheadpart & AR_FORE) {
        int xdir;
        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        picdraw_arrowhead(xu->cob.arrowforeheadtype, xdir, x2, y2);
    }
#if 0
    if(xu->cob.arrowbackheadtype>0) {
#endif
    if(xu->cob.arrowheadpart & AR_BACK) {
        int xdir;
        xdir = (int)(atan2((y1-y2),(x1-x2))/rf);
        picdraw_arrowhead(xu->cob.arrowbackheadtype, xdir, x1, y1);
    }

out:
    return 0;
}

        
int
picdraw_line(int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int x2, y2;

    printf("+ # linearrow\n");

    x1 = xox+xu->csx;
    y1 = xoy+xu->csy;
    x2 = xox+xu->cex;
    y2 = xoy+xu->cey;

    pf = NULL;
    if(xu->cafrom) {
        pf = ns_find_obj(xns, xu->cafrom);
        printf("%s: pf %p\n", __func__, pf);
        if(!pf) {
            goto to_phase;
        }
        if(pf->cfixed) {
            printf("  from %d,%d\n", pf->gx, pf->gy);
            x1 = pf->gx;
            y1 = pf->gy;
        }
    }

to_phase:

    pt = NULL;
    if(xu->cafrom) {
        pt = ns_find_obj(xns, xu->cato);
        printf("%s: pt %p\n", __func__, pt);
        if(!pt) {
            goto apply;
        }
        if(pt->cfixed) {
            printf("  to %d,%d\n", pt->gx, pt->gy);
            x2 = pt->gx;
            y2 = pt->gy;
        }
    }

apply:
    printf("%s: from (%d,%d) to (%d,%d)\n", __func__, x1, y1, x2, y2);
    printf("+ line from (%d,%d) to (%d,%d) # line\n", x1, y1, x2, y2);

out:
    return 0;
}

int
picdrawobj(ob *u, int *xdir, int ox, int oy, ns *xns)
{
    int wd, ht;
    int g;

    printf("%s: oid %d xdir %d ox,oy %d,%d\n",
        __func__, u->oid, *xdir, ox, oy);
    
    wd = u->crx-u->clx;
    ht = u->cty-u->cby;

    printf("%s: b oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, u->oid,
        u->cx, u->cy, u->csx, u->csy, u->cex, u->cey, u->ox, u->coy);

    printf("+ # oid %d, type %d START\n", u->oid, u->type);

#if 0
    PB(u);
    PQQ(u);
#endif
    PD(ox+u->csx, oy+u->csy);
#if 0
#endif
    printf("+ line from (%d,%d) to (%d,%d) thick 0.1 # sxy->exy\n",
        ox+u->csx, oy+u->csy, ox+u->cex, oy+u->cey);

    g = eval_dir(u, xdir);
    if(g>0) {
        goto out;
    }

    if(u->type==CMD_LINE) {
        printf("+   line from (%d,%d) to (%d,%d) thick 1.5 \"%d\" \"\" # line\n",
            ox+u->csx, oy+u->csy, ox+u->cex, oy+u->cey, u->oid);
        picdraw_linearrow(ox, oy, u, xns);
    }
    if(u->type==CMD_ARROW) {
        picdraw_linearrow(ox, oy, u, xns);
#if 0
        int dx, dy;
        int r;
        printf("+   line from (%d,%d) to (%d,%d) thick 1.5 \"%d\" \"\" # arrow\n",
            ox+u->csx, oy+u->csy, ox+u->cex, oy+u->cey, u->oid);
#if 0
        r = u->cht/2;
#endif
        r = def_arrowsize;

        dx =  (int)(r*cos((*xdir+180+def_arrowangle/2)*rf));
        dy =  (int)(r*sin((*xdir+180+def_arrowangle/2)*rf));
        printf("+   line from (%d,%d) to (%d,%d) thick 1.5\n",
            ox+u->cex, oy+u->cey, ox+u->cex+dx, oy+u->cey+dy);

        dx =  (int)(r*cos((*xdir+180-def_arrowangle/2)*rf));
        dy =  (int)(r*sin((*xdir+180-def_arrowangle/2)*rf));
        printf("+   line from (%d,%d) to (%d,%d) thick 1.5\n",
            ox+u->cex, oy+u->cey, ox+u->cex+dx, oy+u->cey+dy);
#endif
    }
    if(u->type==CMD_PLINE) {
        int dx, dy;
        int r;
        if(u->cht < u->cwd) { r = u->cht; } else { r = u->cwd; }
        r = r/2;

        dx = (int)(r*cos((*xdir+90)*rf));
        dy = (int)(r*sin((*xdir+90)*rf));
printf("+ # dir %d; dx,dy %d,%d\n", *xdir, dx, dy);
        printf("+   line from (%d,%d) to (%d,%d) thick 0.1\n",
            ox+u->clx, oy+u->cy, ox+u->crx, oy+u->cy);
        printf("+   line from (%d,%d) to (%d,%d) thick 0.1\n",
            ox+u->csx, oy+u->csy, ox+u->cex, oy+u->cey);
        printf("+   line from (%d,%d) to (%d,%d) thick 1.5 \"%d   \"# pline\n",
            ox+u->cx-dx, oy+u->cy-dy, ox+u->cx+dx, oy+u->cy+dy, u->oid);

    }
    if(u->type==CMD_MOVE) {
        printf("+   # empty as \"move\"\n");
        printf("+   box at (%d,%d) width %d height %d \"%d\" invis\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, u->oid);
    }

    if(u->type==CMD_DMY1) {
        printf("+   box at (%d,%d) width %d height %d rad %d \"%d\"\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, wd/8, u->oid);
    }
    if(u->type==CMD_DMY2) {
        printf("+   box at (%d,%d) width %d height %d rad %d \"%d\"\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, wd/8, u->oid);
    }

    if(u->type==CMD_BOX) {
        printf("+   box at (%d,%d) width %d height %d rad %d \"%d\"\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, wd/8, u->oid);
        printf("+   box at (%d,%d) width %d height %d rad %d thickness %d \"%d\"\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, wd/8, u->cob.outlinethick, u->oid);
    }
    if(u->type==CMD_CIRCLE) {
        printf("+   circle at (%d,%d) rad %d \"%d\"\n",
            ox+u->cx, oy+u->cy, u->cwd/2, u->oid);
    }
    if(u->type==CMD_ELLIPSE) {
        printf("+   ellipse at (%d,%d) width %d height %d \"%d\"\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, u->oid);
    }
    if(u->type==CMD_POLYGON) {
#if 0
        printf("+   box at (%d,%d) width %d height %d \"%p\"\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, u);
#endif
        printf("+   line from (%d,%d) to (%d,%d) to (%d,%d) to (%d,%d)\n",
            ox+u->clx, oy+u->cby, ox+(u->clx+u->crx)/2, oy+u->cty,
            ox+u->crx, oy+u->cby, ox+u->clx, oy+u->cby);
    }
    printf("+ # oid %d, type %d DONE\n", u->oid, u->type);

out:
    printf("%s: a oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, u->oid,
        u->cx, u->cy, u->csx, u->csy, u->cex, u->cey, u->ox, u->coy);

    return 0;
}


int
picdrawchunk(ob *xch, int gox, int goy, ns *xns)
{
    int ik;
    int i;
    ob* u;

    printf("%s: oid %d x,y %d,%d\n", __func__, xch->oid, gox, goy);

    printf("%s:   oid %d wxh %dx%d o %d,%d\n",
        __func__, xch->oid, xch->wd, xch->ht, xch->ox, xch->oy);
    printf("%s: b oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid,
 xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey, xch->ox, xch->coy);

    printf("+ ##### oid %d START\n", xch->oid);

    cha_reset(&xch->cch);

    PB(xch);
#if 0
#endif
    PQ(xch);

#if 1
    printf("%p (%d,%d)->(%d,%d) (%d,%d) (%d,%d)-(%d,%d)\n",
        xch,
        xch->csx, xch->csy, xch->cex, xch->cey,
        xch->cx, xch->cy, xch->clx, xch->cby, xch->crx, xch->cty);
    printf("%p+(%d,%d) (%d,%d)->(%d,%d) (%d,%d) (%d,%d)-(%d,%d)\n",
        xch, gox, goy,
        gox+xch->csx, goy+xch->csy, gox+xch->cex, goy+xch->cey,
        gox+xch->cx, goy+xch->cy, gox+xch->clx, goy+xch->cby, gox+xch->crx, goy+xch->cty);
#endif

    
#if 0
    printf("+ arrow from (%d,%d) to (%d,%d)\n",
        gox+xch->csx, goy+xch->csy,
        gox+xch->cex, goy+xch->cey);
    printf("+ circle at (%d,%d) rad %d dashed\n",
        gox+xch->cx, goy+xch->cy, xch->cmr);
    printf("+ line from (%d,%d) to (%d,%d) to (%d,%d) to (%d,%d) to (%d,%d) dashed\n",
        gox+xch->clx, gox+xch->cby, gox+xch->clx, gox+xch->cty,
        gox+xch->crx, gox+xch->cty, gox+xch->crx, gox+xch->cby,
        gox+xch->clx, gox+xch->cby);
#endif

#if 0
    printf("+ box at (%d,%d) width 5 height 5 thick 2\n",
        xch->cox, xch->coy);
#endif
#if 0
    PC(xch->cox, xch->coy);
#endif
    PC(xch->cx, xch->cy);

    printf("+ circle at (%d,%d) rad 2 fill 1\n",
        xch->csx, xch->csy);
    printf("+ circle at (%d,%d) rad 2 fill 0\n",
        xch->cex, xch->cey);
#if 0
#endif

#if 0
    printf("+ circle at (%d,%d) rad %d dashed\n",
        gox+xch->cox+xch->cx, xch->coy+goy+xch->cy, xch->cmr);
    printf("+ circle at (%d,%d) rad %d dashed\n",
        xch->cox+xch->cx, xch->coy+xch->cy, xch->cmr);
#endif

    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*)xch->cch.ch[i];
        if(u->type==CMD_CHUNK) {
#if 0
            ik = picdrawchunk(u, xch->x+xch->ox, xch->y+xch->oy, xns);
#endif
            ik = picdrawchunk(u,
                    xch->x+xch->ox+u->ox,
                    xch->y+xch->oy+u->oy, xns);
        }
        else {
            ik = picdrawobj(u, &xch->cch.dir, xch->x+xch->ox, xch->y+xch->oy, xns);
        }
    }

    printf("+ ##### oid %d DONE\n", xch->oid);

    printf("%s: a oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid, 
        xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey, xch->ox, xch->coy);

    return 0;
}


int
picdraw(ob *xch, int x, int y, ns *xns)
{
    int ik;
    int i;

    for(i=0;i<20;i++) {
        printf("+ \n");
    }
   
printf( "+ .PS 4\n");

    if(debuglog[0]) {
        printf("%s\n", debuglog);
    }

    ik = picdrawchunk(xch, x, y, xns);

printf( "+ .PE\n");

    return ik;
}

#undef PD
#undef PC
#undef PB
#undef PQ
#undef PQQ
