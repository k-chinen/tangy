char debuglog[BUFSIZ*10]="";

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

printf("%s: b rx,ry %d,%d\n", __func__, *rx, *ry);
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

printf("%s: a rx,ry %d,%d\n", __func__, *rx, *ry);
    
    return 0;
}


#if 1

#define MARK(m,x,y) \
    printf("MARK %d %s x %6d, y %6d\n", __LINE__, m, x, y); \
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
est_seg(ns* xns, varray_t *opar, varray_t *segar,
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

#if 0
printf("%s:\n", __func__);
    varray_fprint(stdout, opar);
#endif

    isset_final = 0;

    lldir = *zdir;
    ldir = *zdir;

    _rx = _ty = -(INT_MAX-1);
    _lx = _by = INT_MAX;

        x = y = 0;
        lx = ly = 0;
#if 1
        MARK("a", x, y);
#endif

    c = 0; /* count of putted commands w/o then */
    jc = 0;
    for(i=0;i<opar->use;i++) {
        mstr[0] = '\0';
        dm = 0;
        m = 0;
        qc = 0;
        actf = 0;

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
    r = (seg*)malloc(sizeof(seg)); \
    memset(r, 0, sizeof(seg)); \
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
printf("  m %d\n", m);
#endif

        if(e->cmd==OA_ARC||e->cmd==OA_ARCN||e->cmd==OA_THEN||
            e->cmd==OA_JOIN||e->cmd==OA_SKIP) {
            actf = 1;
        }

        /* to reduce process default is skip */
        if(actf) {
flush_que:
            if(c>0) {
                MARK("f", x, y);
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
                            r = (seg*)malloc(sizeof(seg));
                            memset(r, 0, sizeof(seg));
                            r->coordtype = REL_COORD;
                            r->ptype = OA_FORWARD;
                            r->x1 = x - lx;
                            r->y1 = y - ly;
                            varray_push(segar, r);

#if 1
                            ldir = atan2(y-ly, x-lx)/rf;
printf("        y %d x %d -> ldir %.2f\n", y-ly, x-lx, ldir);
#endif

#if 0
printf("        y %d x %d -> ldir %.2f\n", y-ly, x-lx, ldir);
#endif

                            r = NULL;
                            lx = x;
                            ly = y;
                            c=0;
                            jc=0;
            }
#endif


            ik = ns_find_objpos(xns, e->val, &mx, &my);
            if(ik==0) {
P;
#if 0
                MARK("e", mx, my);
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
printf("mark final position %d,%d\n", mx, my);
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

        case OA_RIGHT:      x += m; c++; ldir =    0; break;
        case OA_LEFT:       x -= m; c++; ldir =  180; break;
        case OA_UP:         y += m; c++; ldir =   90; break;
        case OA_DOWN:       y -= m; c++; ldir =  -90; break;

        case OA_JOIN:
                            jc += 1;
                            c++;
                            FREG(OA_JOIN, 1, REL_COORD, 0, 0, 0, 0, 0);
                            MARK("J", x, y);
                            break;

        case OA_SKIP:
                            jc += 1000;
                            x += m*cos(ldir*rf);
                            y += m*sin(ldir*rf);
                            c++;
                            FREG(OA_SKIP, 1000, REL_COORD, 0, x-lx, y-ly, 0, 0);
                            MARK("S", x, y);
                            break;
    
        case OA_FORWARD:    x += m*cos(ldir*rf);
                            y += m*sin(ldir*rf);
                            c++;
                            break;
        
        case OA_ARC:
fprintf(stderr, " arc  ldir %7.2f rad %d an %d\n", ldir, rad, an);
printf("  ldir %7.2f rad %d an %d\n", ldir, rad, an);
                            MARK("cB", x, y);
                            arcx = x + rad*cos((ldir+90)*rf);
                            arcy = y + rad*sin((ldir+90)*rf);

printf("  arc ldir %f .. %f\n", ldir-90, ldir-90+an);
    
                            /* mark peak points */
                            {
                                int a;
                                int tx, ty;
                                for(a=ldir-90;a<=ldir-90+an;a++) {
                                    if(a==ldir-90||a==ldir-90+an||a%90==0) {
                                        tx = arcx+rad*cos((a)*rf);
                                        ty = arcy+rad*sin((a)*rf);
                                        printf(" a %4d ", a);
                                        MARK("cM", tx, ty);
                                    }
                                }
                            }

                            x = arcx+rad*cos((ldir-90+an)*rf);
                            y = arcy+rad*sin((ldir-90+an)*rf);

printf("  arcx,y %d,%d x,y %d,%d\n", arcx, arcy, x, y);
                            MARK("cE", x, y);


                            FREG(OA_ARC, jc, REL_COORD, 0, 0, 0, rad, an);

                            ldir += an;
printf("        y %d x %d -> ldir %.2f\n", y-ly, x-lx, ldir);

                            r = NULL;
                            lx = x;
                            ly = y;
                            c=0;
                            jc=0;

                            break;

        case OA_ARCN:
fprintf(stderr, " arcn ldir %7.2f rad %d an %d\n", ldir, rad, an);
printf("  ldir %7.2f rad %d an %d\n", ldir, rad, an);
                            arcx = x + rad*cos((ldir-90)*rf);
                            arcy = y + rad*sin((ldir-90)*rf);
                            x = arcx+rad*cos((ldir+90)*rf);
                            y = arcy+rad*sin((ldir+90)*rf);

printf("  arcx,y %d,%d x,y %d,%d\n", arcx, arcy, x, y);
                            MARK("cB", x, y);

printf("  arcn ldir %f .. %f\n", ldir, ldir-an);
                            /* mark peak points */
                            {
                                int a;
                                int tx, ty;
                                for(a=ldir+90;a>=ldir+90-an;a--) {
                                    if(a==ldir+90||a==ldir+90-an||a%90==0) {
                                        tx = arcx+rad*cos((a)*rf);
                                        ty = arcy+rad*sin((a)*rf);
                                        printf(" a %4d ", a);
                                        MARK("cM", tx, ty);
                                    }
                                }
                            }

                            x = arcx+rad*cos((ldir+90-an)*rf);
                            y = arcy+rad*sin((ldir+90-an)*rf);

printf("  arcx,y %d,%d x,y %d,%d\n", arcx, arcy, x, y);
                            MARK("cE", x, y);

                            FREG(OA_ARCN, jc, REL_COORD, 0, 0, 0, rad, an);

                            ldir -= an;
printf("        y %d x %d -> ldir %.2f\n", y-ly, x-lx, ldir);

                            r = NULL;
                            lx = x;
                            ly = y;
                            c=0;
                            jc=0;

                            break;

    
        case OA_DIR:        ldir = m;   break;

        case OA_INCDIR:
        case OA_LTURN:
                            ldir += m;  break;
        case OA_DECDIR:
        case OA_RTURN:
                            ldir -= m;  break;

        case OA_THEN:   
            /* nothing */
                            break;
        }
#if 0
printf("    %d: cmd %d val '%s' : mstr '%s' dm %.2f m %d : x,y %d,%d ldir %.2f\n",
        i, e->cmd, e->val, mstr, dm, m, x, y, ldir);
#endif
        MARK("e", x, y);
    }
        MARK("z", x, y);

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

#if 1
    varray_fprint(stdout, segar);
#endif

#if 0
printf("    ldir %.2f\n", ldir);
#endif
    if(!kp) {
#if 0
printf("    *zdir %d -> %.2f\n", *zdir, ldir);
#endif
        *zdir = (int)ldir;
    }
    if(isset_final<=0) {
printf("set final as last position\n");
        *rfx = lx;
        *rfy = ly;
    }
    else {
printf("set final as specified position\n");
    }

    fprintf(stderr, "lldir %.1f ldir %.1f\n", lldir, ldir);

#if 0
printf("%s: %d %d %d %d\n", __func__, *rlx, *rby, *rrx, *rty);
#endif
    return rv;
}

#if 0
int
Xest_seg(varray_t *ar, int *rlx, int *rby, int *rrx, int *rty)
{
    int x, y;
    int h, v;
    int _lx, _by, _rx, _ty;
    int i;
    seg *e;

printf("%s:\n", __func__);

    _rx = _ty = -(INT_MAX-1);
    _lx = _by = INT_MAX;

        x = y = 0;
        MARK("z", x, y);
    for(i=0;i<ar->use;i++) {
        e = (seg*)ar->slot[i];
        h = e->x1;
        v = e->y1;
        x += h;
        y += v;
printf("    %d: h,v %d,%d x,y %d %d\n", i, h, v, x, y);
        MARK("e", x, y);
    }

    *rlx = _lx;
    *rby = _by;
    *rrx = _rx;
    *rty = _ty;

printf("%s: %d %d %d %d\n", __func__, *rlx, *rby, *rrx, *rty);
    return 0;
}

#endif


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
    printf("%s: lob %p oid %d pos %d\n", __func__, lob, lob->oid, pos);

    printf("  lob attributes\n");
    printf("    lx,by,rx,ty %d,%d,%d,%d\n",
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
    printf("  ret %d,%d\n", *nx, *ny);
    return 0;   
}



#define WO      {wd = objunit*3/2;  ht = objunit; }
#define RO      {wd = objunit;      ht = objunit; }
#define NO      {wd = objunit/2;    ht = objunit; }
#define NNO     {wd = objunit/4;    ht = objunit; }
#define ZZ      {wd = 0;            ht = 0;       }


int
putobj(ob *u, ns *xns, int *gdir)
{
    int    wd, ht;
    int    dir;
    int    re;


#if 1
    printf("%s: oid %d\n", __func__, u->oid);
#endif
#if 1
printf("\toid %d u b wd %d ht %d solved? %d\n",
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
        goto apply;
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

 /*
  * WO  1.5 x 1.0   wide
  * RO  1.0 x 1.0   regular
  * NO  0.5 x 1.0   narrow
  */
 

    wd = ht = 0;

    switch(u->type) {

    case CMD_CHUNKOBJATTR:  ZZ; break;

    case CMD_LPAREN:    NNO;    break;
    case CMD_RPAREN:    NNO;    break;
    case CMD_LBRACKET:  NNO;    break;
    case CMD_RBRACKET:  NNO;    break;
    case CMD_LBRACE:    NNO;    break;
    case CMD_RBRACE:    NNO;    break;

    case CMD_MOVE:      WO;     break;

    case CMD_DMY1:      WO;     break;
    case CMD_DMY2:      WO;     break;
    case CMD_PAPER:     WO;     break;
    case CMD_CLOUD:     WO;     break;
    case CMD_DRUM:      WO;     break;
    case CMD_BOX:       WO;     break;
    case CMD_DOTS:      WO;     u->cob.sepcurdir = dir; break;
    case CMD_CIRCLE:    RO;     break;
    case CMD_ELLIPSE:   WO;     break;
    case CMD_POLYGON:   RO;     break;

    case CMD_PING:
    case CMD_PINGPONG:  WO;     break;

    case CMD_SCATTER: 
    case CMD_GATHER: 
            RO;     
            break;

    case CMD_PLINE:
    case CMD_SEP:     
            NO;
printf("SEP oid %d dir %d\n", u->oid ,dir);
            u->cob.sepcurdir = dir;
            break;

    case CMD_CLINE:
    case CMD_LINE:     
    case CMD_ARROW:
    case CMD_WLINE:
    case CMD_WARROW:
    case CMD_BARROW:
    case CMD_LINK: 
        if(u->cob.originalshape) {
            int ik;
            int lx, by, rx, ty, fx, fy;

            ik = est_seg(xns, u->cob.segopar, u->cob.segar,
                    u->cob.keepdir, gdir, &lx, &by, &rx, &ty, &fx, &fy);

#if 1
printf("\tseg bb (%d %d %d %d) fxy %d,%d\n", lx, by, rx, ty, fx, fy);
#endif

#if 1
#endif
            u->clx = lx;
            u->cby = by;
            u->crx = rx;
            u->cty = ty;


            wd = rx - lx;
            ht = ty - by;

#if 1
printf("\tseg originalshape 1 wd %d ht %d fx,fy %d,%d\n", wd, ht, fx, fy);
#endif

            u->fx = fx;
            u->fy = fy;

            if(u->type==CMD_CLINE) {
            }
#if 1
                u->ox = -(lx+rx)/2;
                u->oy = -(ty+by)/2;
#endif
#if 0
                u->jx = -(lx+rx)/2;
                u->jy = -(ty+by)/2;
#endif

            re = 1;
        }
        else {
            WO;     
        }
        break;
    default:
        if(ISDRAWABLE(u->type)) {
            printf("WARNING: sorry the object(oid %d) has no size\n",
                u->oid);
        }
        break;
    }

apply:

#if 1
printf("\t  2 wd %d ht %d\n", wd, ht);
#endif
    if(u->wd<0) u->wd = wd;
    if(u->ht<0) u->ht = ht;

    u->sizesolved++;
#if 0
printf("obj oid %d solved\n", u->oid);
#endif

#if 1
printf("\toid %d u a wd %d ht %d solved? %d\n",
    u->oid, u->wd, u->ht, u->sizesolved);
#endif

#if 0
    printf("%s: oid %d dir %d wxh %dx%d; %d %d %d %d\n",
        __func__, u->oid, dir, u->wd, u->ht,
        u->lx, u->by, u->rx, u->y);
#endif
#if 1
    printf("%s: oid %d dir %d wxh %dx%d; xy %d,%d\n",
        __func__, u->oid, dir, u->wd, u->ht,
        u->x, u->y);
#endif

out:
    return re;
}

int
catobj(ob *u, int xxdir, int *x, int *y, int *fx, int *fy, ns *xns)
{
    int dx, dy;

    if(!u) {
E;
        return -1;
    }

    Echo("%s: oid %d\n", __func__, u->oid);

#if 1
    printf("%s: u %p oid %d xxdir %d *xy %d,%d *fxy %d,%d xns %p\n",
            __func__, u, u->oid, xxdir, *x, *y, *fx, *fy, xns);
    printf("    wd %d ht %d (%d %d %d %d)\n",
            u->wd, u->ht, u->lx, u->by, u->rx, u->ty);
    printf("    ox,oy %d,%d\n", u->ox, u->oy);
    printf("    jx,jy %d,%d\n", u->jx, u->jy);
#endif

    u->csx = *x;
    u->csy = *y;

    u->cx = *x - u->jx;
    u->cy = *y - u->jy;

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

    printf("%s: oid %d (%d %d %d %d)\n",
        __func__, u->oid, u->clx, u->cby, u->crx, u->cty);

    u->cex = *x+*fx;
    u->cey = *y+*fy;
    *x = u->cex;
    *y = u->cey;

#if 0
    printf("%s: u %p oid %d x,y %d,%d sx,sy %d,%d ex,ey %d,%d\n",
            __func__, u, u->oid,
            u->cx, u->cy, u->csx, u->csy, u->cex, u->cey);
#endif
#if 1
    printf("%s: a oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, u->oid,
        u->cx, u->cy, u->csx, u->csy, u->cex, u->cey, u->ox, u->coy);
#endif

    return 0;
}

int
fitobj(ob *u, int xxdir, int *x, int *y, ns *xns)
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

Echo("%s: oid %d\n", __func__, u->oid);

#if 1
    printf("%s: u %p oid %d xxdir %d *xy %d,%d *fxy --,-- xns %p\n",
            __func__, u, u->oid, xxdir, *x, *y, xns);
    printf("    wd %d ht %d (%d %d %d %d)\n",
            u->wd, u->ht, u->lx, u->by, u->rx, u->ty);
    printf("    ox,oy %d,%d\n", u->ox, u->oy);
    printf("    jx,jy %d,%d\n", u->jx, u->jy);
#endif


    /* normalization dir */
    dir = (xxdir+360+180)%360-180;

#if 0
printf("xxdir %d dir %d\n", xxdir, dir);
#endif

#if 0
    printf("%s: oid %d dir %d wxh %dx%d x,y %d,%d\n",
        __func__, u->oid, dir, u->wd, u->ht, *x, *y);
    printf("%s: b oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
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
    printf("th %.3f (rad %.3f) <- %dx%d oid %d v.s. dir %d\n",
        th, th*rf, wd, ht, u->oid, dir);
#endif

    /* atan2 returns -180 to +180 */
    if((dir>th&&dir<180-th)|| (dir>-180+th&&dir<-th)) {
        f1 = 'H';
#if 0
        printf("%s: over,  fit to horizontal\n", __func__);
#endif
        qy = sin(dir*rf);
        q  = qy*qy;

        if(q<QLIMIT) {
            f3 = '0';
#if 0
printf("qy %f q %f v.s. QLIMIT %f\n", qy, q, QLIMIT);
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
        printf("%s: under, fit to vertical\n", __func__);
#endif
        qx = cos(dir*rf);
        q  = qx*qx;

        if(q<QLIMIT) {
            f3 = '0';
#if 0
printf("qx %f q %f v.s. QLIMIT %f\n", qx, q, QLIMIT);
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
    printf("dx,dy %f,%f f %c\n", dx, dy, f);
#endif
#if 0
    printf("oid %-3d th %.3f dir %4d f %c%c%c q %.3f,%.3f q %.3f dx,dy %.3f,%.3f\n",
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
    printf("  oldx,y %d,%d\n", oldx, oldy);
#endif
#if 1
    printf("%s: oid %d (%d %d %d %d)\n",
        __func__, u->oid, u->clx, u->cby, u->crx, u->cty);
    printf("%s: a oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, u->oid,
        u->cx, u->cy, u->csx, u->csy, u->cex, u->cey, u->ox, u->coy);
#endif

#define QQXY(vx,vy) \
    if(vx<-INT_MAX/2||vx>INT_MAX/2) { \
        printf("ERROR oid %d ignore x %s:%d\n", u->oid, __FILE__, __LINE__); \
    } \
    if(vy<-INT_MAX/2||vy>INT_MAX/2) { \
        printf("ERROR oid %d ignore y %s:%d\n", u->oid, __FILE__, __LINE__); \
    } 

    QQXY(u->cx,u->cy);
    QQXY(u->cox,u->coy);
    QQXY(u->csx,u->csy);
    QQXY(u->cex,u->cey);

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
    if(u->type==CMD_DIR)    { *xdir =   u->cob.iarg1; g++; }

    if(u->type==CMD_UNIT)   { objunit = u->cob.iarg1; recalcsizeparam(); g++; }

    if(u->type==CMD_INCDIR) { *xdir +=  u->cob.iarg1; g++; }
    if(u->type==CMD_DECDIR) { *xdir -=  u->cob.iarg1; g++; }

    if(u->type==CMD_LTURN)  { *xdir +=   90; g++; }
    if(u->type==CMD_RTURN)  { *xdir -=   90; g++; }
    if(u->type==CMD_DOWN)   { *xdir =   -90; g++; }
    if(u->type==CMD_RIGHT)  { *xdir =     0; g++; }
    if(u->type==CMD_UP)     { *xdir =    90; g++; }
    if(u->type==CMD_LEFT)   { *xdir =   180; g++; }

#if 0
    printf("%s: oid %d type %d: g %d; dir %d -> %d\n",
        __func__, u->oid, u->type, g, odir, *xdir);
#endif
    if(g && _t_) {
#if 0
        printf("%s: oid %d type %d: g %d; dir %d -> %d\n",
            __func__, u->oid, u->type, g, odir, *xdir);
#endif
    }

    return g;
}

int
bumpH(int x1, int y1, int x2, int y2, int cx, int cy, int cdir,
    int *rgx, int *rgy)
{
    int r;
    double a, b;
    double c, d;
    double gx, gy;

    printf("%s: (%d %d %d %d) vs (%d,%d;%d)\n",
        __func__, x1, y1, x2, y2, cx, cy, cdir);

    r = -1;

    c = tan(cdir*rf);
    d = (double)cy - c * (double)cx;

    printf("  c  %9.3f d  %9.3f\n", c, d);

    gy = y1;
    gx = (y1-d)/c;

    printf("  gx %9.3f gy %9.3f\n", gx, gy);

    if(((gx>=x1 && gx<=x2) || (gx>=x2 && gx<=x1)) &&
       ((gy>=y1 && gy<=y2) || (gy>=y2 && gy<=y1))) {
        printf("  bumped INSIDE\n");
        *rgx = (int)gx;
        *rgy = (int)gy;
        r = 1;
    }
    else {
        printf("  bumped outside\n");
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

    printf("%s: (%d %d %d %d) vs (%d,%d;%d)\n",
        __func__, x1, y1, x2, y2, cx, cy, cdir);

    r = -1;

    c = tan(cdir*rf);
    d = (double)cy - c * (double)cx;

    printf("  c  %9.3f d  %9.3f\n", c, d);

    gx = x1;
    gy = c*x1+d;

    printf("  gx %9.3f gy %9.3f\n", gx, gy);

    if(((gx>=x1 && gx<=x2) || (gx>=x2 && gx<=x1)) &&
       ((gy>=y1 && gy<=y2) || (gy>=y2 && gy<=y1))) {
        printf("  bumped INSIDE\n");
        *rgx = (int)gx;
        *rgy = (int)gy;
        r = 1;
    }
    else {
        printf("  bumped outside\n");
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

    printf("%s: (%d %d %d %d) vs (%d,%d;%d)\n",
        __func__, x1, y1, x2, y2, cx, cy, cdir);

    r = -1;

    a = ((double)y2-(double)y1)/((double)x2-(double)x1);
    b = (double)y1 - a * (double)x1;

    printf("  a  %9.3f b  %9.3f\n", a, b);

    c = tan(cdir*rf);
    d = (double)cy - c * (double)cx;

    printf("  c  %9.3f d  %9.3f\n", c, d);

    gx = (d-b)/(a-c);
    gy = a*gx+b;

    printf("  gx %9.3f gy %9.3f\n", gx, gy);

    if(((gx>=x1 && gx<=x2) || (gx>=x2 && gx<=x1)) &&
       ((gy>=y1 && gy<=y2) || (gy>=y2 && gy<=y1))) {
        printf("  bumped INSIDE\n");
        *rgx = (int)gx;
        *rgy = (int)gy;
        r = 1;
    }
    else {
        printf("  bumped outside\n");
        r = 0;
    }

    return r;
}

int
expand_paren(int bx, int by, int w, int h, int ox, int oy, ob* u)
{

printf("%s: bx,by %d,%d w,h %d,%d ox,oy %d,%d\n",
        __func__, bx, by, w, h, ox, oy);
printf("  vs oid %d cx,cy %d,%d w,h %d,%d noexpand? %d\n",
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
        if(c>=2) printf("ERROR too many bump point\n"); \
        mx[c] = qx-ox; my[c] = qy-oy; c++; \
    }


printf("%s: bx,by %d,%d w,h %d,%d ox,oy %d,%d\n",
        __func__, bx, by, w, h, ox, oy);
printf("  vs oid %d cx,cy %d,%d\n", u->oid, u->cx, u->cy);

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

    printf("  c %d\n", c);

    u->cob.sepx1 = mx[0]; u->cob.sepy1 = my[0];
    u->cob.sepx2 = mx[1]; u->cob.sepy2 = my[1];

    printf("  oid %-4d 0 mx,my %d,%d\n", u->oid, mx[0], my[0]);
    printf("           1 mx,my %d,%d\n",         mx[1], my[1]);

#undef A

    return 0;

}


int
putchunk(ob *xch, int *x, int *y, ns *xns)
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
    printf("%s: oid %d x,y %d,%d\n", __func__, xch->oid, *x, *y);
    printf("%s: b oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid,
        xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey, xch->ox, xch->coy);
#endif

    if(xch->sizesolved) {
        printf("oid %d already solved\n", xch->oid);
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

    printf("goto AT '%s'\n", u->cob.aat);

                r = ns_find_objpos(curns, u->cob.aat, &dx, &dy);
    printf("  ns_find_objpos '%s' ret %d\n", u->cob.aat, r);
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
printf("  oid %d back %d,%d\n", u->oid, nx, ny);
                continue;
            }
        }
        if(u->type==CMD_HBACK) {
            if(c>1) {
                nx = (lx+nx)/2;
                ny = (ly+ny)/2;
printf("  oid %d hback %d,%d\n", u->oid, nx, ny);
                continue;
            }
        }
        if(u->type==CMD_SHORE) {
            ob* fob;

            fob = NULL;

printf("u %p oid %d iarg1 %d\n", u, u->oid, u->cob.iarg1);
            if(u->cob.ato) {    
                    fob = ns_find_obj(curns, u->cob.ato);
            }
            else {
                if(lob) {
                    fob = lob;
                }
            }

            if(fob) {
printf("fob %p\n", fob);
                ik = takelastobjpos(fob, u->cob.iarg1, u->cob.keepdir,
                        &nx, &ny, &xch->cch.dir);
                continue;
            }
        }

        if(u->type==CMD_SAVE) {
#if 0
printf("  oid %d b save %d,%d\n", u->oid, nx, ny);
printf("  oid %d a save %d,%d\n", u->oid, bs.x, bs.y);
#endif
            cha_copy(&bs, &xch->cch);
printf("  oid %d save %d,%d\n", u->oid, bs.x, bs.y);

            bsc++;
            continue;
        }
        if(u->type==CMD_RESTORE) {
            if(bsc<=0) {
                printf("ERROR no saved attributes\n");
                continue;
            }
            
            cha_copy(&xch->cch, &bs);
            nx = xch->cch.x;
            ny = xch->cch.y;

printf("  oid %d restore to %d,%d\n", u->oid, nx, ny);
            bsc--;
            continue;
        }

        if(u->type==CMD_PUSH) {
            chas_push(&regst, &xch->cch);
printf("  oid %d push  %d,%d %d\n", u->oid, bs.x, bs.y, bs.dir);
printf("  oid %d push  %d,%d %d\n", u->oid, 
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

printf("  oid %d pop   %d,%d %d\n", u->oid, nx, ny, xch->cch.dir);
            continue;
        }
        if(u->type==CMD_AGAIN) {
            ch tmp;

            chas_top(&regst, &tmp);

            cha_copy(&xch->cch, &tmp);
            nx = tmp.x;
            ny = tmp.y;

printf("  oid %d again %d,%d %d\n", u->oid, nx, ny, xch->cch.dir);
            continue;
        }

        oldx = nx;
        oldy = ny;


#define C(d,s) \
            (d)->wd = (s)->wd; \
            (d)->ht = (s)->ht; \
            (d)->x  = (s)->x;  \
            (d)->y  = (s)->y;  \
            (d)->ox = (s)->ox; \
            (d)->oy = (s)->oy; \
            (d)->lx = (s)->lx; \
            (d)->rx = (s)->rx; \
            (d)->ty = (s)->ty; \
            (d)->by = (s)->by; 


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
printf("  last pos %d,%d dir %d\n", lx, ly, ldir);
#endif

        if(ISCHUNK(u->type)) {
#if 0
printf("chunk oid %d -> chunk oid %d start\n", xch->oid, u->oid);
#endif
            ik = putchunk(u, &nx, &ny, curns);
#if 0
printf("chunk oid %d -> chunk oid %d ; ik %d\n", xch->oid, u->oid, ik);
#endif

        }
        else {
#if 0
printf("chunk oid %d -> obj oid %d\n", xch->oid, u->oid);
#endif
            ik = putobj(u, curns, &xch->cch.dir);
#if 1
printf("chunk oid %d -> obj oid %d ; ik %d\n", xch->oid, u->oid, ik);
#endif

        }

#if 0
printf("  fx,fy %d,%d dir %d; ik %d\n",
    u->fx, u->fy, xch->cch.dir, ik);
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

#if 0
        if(u->cob.afrom) {
            int r;
            int dx, dy;

printf("found FROM '%s'\n", u->cob.afrom);

            r = ns_find_objpos(curns, u->cob.afrom, &dx, &dy);
printf("  ns_find_objpos '%s' ret %d; %d,%d\n", u->cob.afrom, r, dx, dy);
            if(r==0) {
                nx = dx; ny = dy;
                adjrewind++;

                u->pst += 1;
            }
            else {
                E;
            }
        }
#endif
#if 0
        if(u->cob.ato) {
            int r;
            int dx, dy;

printf("found TO '%s'\n", u->cob.ato);

            r = ns_find_objpos(curns, u->cob.ato, &qx, &qy);
printf("  ns_find_objpos '%s' ret %d; %d,%d\n", u->cob.ato, r, dx, dy);
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

printf("found AT '%s'\n", u->cob.aat);

            r = ns_find_objpos(curns, u->cob.aat, &dx, &dy);
printf("  ns_find_objpos '%s' ret %d; %d,%d\n", u->cob.aat, r, dx, dy);
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

printf("found WITH '%s'\n", u->cob.awith);

            dx = nx; dy = ny;
            r = applywith(u, u->cob.awith, &dx, &dy);
printf("  applywith '%s' ret %d; %d,%d\n", u->cob.awith, r, dx, dy);
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

printf("  rewindcenter dir %d ret %d; %d,%d\n", xch->cch.dir, r, dx, dy);
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


    if(uniqpoint && ISGLUE(u->type)) {
            int gfx, gfy;
            int isx, isy;
            int hasfrom;

            gfx = fx;
            gfy = fy;

            hasfrom = -1;
            hasfrom = find_from(u, &isx, &isy);

            if(hasfrom==1) {
                printf("oid %d hasfrom %d\n", u->oid, hasfrom);
                u->pst += 10000;
                nx = isx;
                ny = isy;
            }

P;
            catobj(u, ldir, &nx, &ny, &gfx, &gfy, curns);
    }
    else {
    

        if(pointjoint && u) {
            int gfx, gfy;

#if 0
printf("  pointjoint %d to lfx,lfy %d,%d ldir %d\n",
    pointjoint, lfx, lfy, ldir);
#endif

                u->pst += 30000;

            gfx = lfx;
            gfy = lfy;
P;
            fitobj(u, xch->cch.dir, &nx, &ny, curns);
#if 0
#endif
        }
        else {
                u->pst += 40000;

P;
            fitobj(u, xch->cch.dir, &nx, &ny, curns);
        }

    }

#if 0
printf("\tnx,ny = %d,%d\n", nx, ny);
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
printf("  oid %d move %d,%d to %d,%d\n", u->oid, oldx, oldy, nx, ny);
#endif
    }

#if 1
    xch->ex = nx;
    xch->ey = ny;
#endif

#if 0
    printf("%s: m oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid,
        xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey, xch->ox, xch->coy);
#endif

#if 0
printf("BB\n");
printf("xch ox,oy %d, %d\n", xch->ox, xch->oy);
#endif

    if(xch->cch.nch>0) {

#if 0
        for(i=0;i<xch->cch.nch;i++) {
            u = (ob*)xch->cch.ch[i];
            printf("%4d: oid %d: %5d %5d -> %5d %5d\n",
                i, u->oid, u->sx, u->sy, u->ex, u->ey);
        }
            printf("here: oid %d: %5d %5d -> %5d %5d\n",
                xch->oid, xch->sx, xch->sy, xch->ex, xch->ey);
#endif

        maxx = -(INT_MAX-1);
        minx = INT_MAX;
        maxy = -(INT_MAX-1);
        miny = INT_MAX;

#if 1
        printf("\t           : oid %d\n", xch->oid);
        printf("\t # : oid: i:     x     y    ox    oy :    lx    by    rx    ty\n");

#endif
        v = 0;
        for(i=0;i<xch->cch.nch;i++) {
            u = (ob*)xch->cch.ch[i];
#if 1
            printf("\t%3d: %3d: %d: %5d %5d %5d %5d : %5d %5d %5d %5d\n",
                i, u->oid, u->ignore, 
                u->cx, u->cy,
                u->cox, u->coy,
                u->clx, u->cby, u->crx, u->cty);
#endif
            if(u->ignore) {
#if 0
    printf("oid %d ignored\n", u->oid);
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

            if(u->type==CMD_LINE||u->type==CMD_CLINE) {
            }
                printf("OUT oid %-3d (%6d %6d %6d %6d) %6d x %-6d ; %6d x %-6d\n",
                    u->oid,
                    u->clx, u->cby, u->crx, u->cty,
                    u->crx-u->clx, u->cty-u->cby,
                    u->cwd, u->cht);

#if 1
            if(u->clx<minx) { minx = u->clx; }
            if(u->crx<minx) { minx = u->crx; }
            if(u->clx>maxx) { maxx = u->clx; }
            if(u->crx>maxx) { maxx = u->crx; }
            if(u->cby<miny) { miny = u->cby; }
            if(u->cty<miny) { miny = u->cty; }
            if(u->cby>maxy) { maxy = u->cby; }
            if(u->cty>maxy) { maxy = u->cty; }
#endif

        }

#if 1
        printf(
        "\tsemi       : %5s %5s %5s %5s : %5d %5d %5d %5d (%d/%d)\n",
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


#if 0
    printf("%s: n oid %d nx,ny %d,%d\n", __func__, xch->oid, nx, ny);
    printf("  lx,ly %d,%d\n", lx, ly);
    printf("  wxh %dx%d\n", xch->wd, xch->ht);
#endif

    zx = minx + xch->wd/2;
    zy = miny + xch->ht/2;

#if 0
    printf("  zx,zy %d,%d\n", zx, zy);
#endif

    xch->ox = -zx;
    xch->oy = -zy;

#if 1
    printf("  new ox,oy %d,%d\n", xch->ox, xch->oy);

    printf("\ttotal %3d  : %5d %5d %5d %5d : %5d %5d %5d %5d (%d/%d) w,h %d,%d\n",
        xch->oid,
        xch->x, xch->y, xch->ox, xch->oy,
        xch->lx, xch->by, xch->rx, xch->ty, v, c, xch->wd, xch->ht);
#endif


    xch->sizesolved++;

#if 0
printf("chunk oid %d solved\n", xch->oid);

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
            printf("SEP oid %d %3d: oid %d xy %d,%d curdir %d\n",
                xch->oid, i, u->oid, u->cx, u->cy, u->cob.sepcurdir);

            ik = expand_sep(xch->cx, xch->cy, xch->cwd, xch->cht,
                    xch->ox, xch->oy, u);
        case CMD_LPAREN:
        case CMD_RPAREN:
        case CMD_LBRACKET:
        case CMD_RBRACKET:
        case CMD_LBRACE:
        case CMD_RBRACE:
            printf("PAREN oid %d %3d: oid %d xy %d,%d curdir %d\n",
                xch->oid, i, u->oid, u->cx, u->cy, u->cob.sepcurdir);

            ik = expand_paren(xch->cx, xch->cy, xch->cwd, xch->cht,
                    xch->ox, xch->oy, u);
            break;
        }
    }

out:

#if 1
    printf("%s: a oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid,
        xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey,
            xch->ox, xch->coy);
#endif

    return 0;
}

int
put(ob *xch, int *x, int *y, ns *xns)
{
    int ik;

P;
    ik = putchunk(xch, x, y, xns);
P;
#if 0
    ik = fitobj(xch, def_dir, x, y, xns);
#endif
    ik = fitobj(xch, xch->cch.dir, x, y, xns);
    return ik;
}
