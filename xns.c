
#include "word.h"
#include "obj.h"
#include "chas.h"
#include "xns.h"
#include "seg.h"
#include "gv.h"

/*
#define P   fprintf(stderr, "PASS %s:%d\n", __FILE__, __LINE__); fflush(stderr);
*/


#define _ns_find_obj(a1,a2) _ns_find_objP(a1,a2,NULL,NULL)

ob*
_ns_find_objP(ns* xns, char *xname, int* xx, int* xy)
{
    char head[BUFSIZ];
    char rest[BUFSIZ];
    int i;
    ob* r;
    int ik;
    int mx, my;

#if 1
    Echo("  %s: xns %p, xname '%s' xx %p xy %p\n",
        __func__, xns, xname, xx, xy);
    if(xx && xy) {
        Echo("  *xx,*xy = %d,%d\n", *xx, *xy);
    }
#endif
#if 0
    ns_dump(xns);
#endif

    if(xx && xy) {
        mx = *xx;
        my = *xy;
    }
    else {
        mx = my = 0;
    }

    r = NULL;
    if(!xns) {
        goto out;
    }

#if 0
    Echo("  xname '%s'\n", xname);
#endif
    ik = splitdot(head, BUFSIZ, rest, BUFSIZ, xname);
#if 0
    Echo("  head  '%s' rest '%s'\n", head, rest);
#endif

    for(i=0;i<xns->nch;i++) {
#if 0
        Echo("     %2d '%s' vs '%s'\n", i, head, xns->chn[i]);
#endif
        if(strcasecmp(xns->chn[i], head)==0) {
P;
#if 1
            Echo("       FOUND\n");
            Echo("         chn '%s'\n", xns->chn[i]);
            Echo("         cht %d\n",   xns->cht[i]);
            Echo("         chv %p\n",   xns->chv[i]);
#endif

            ob *mm;
            mm = xns->chv[i];
#if 1
            if(mm) {
                Echo(
                  " mm %p ox,oy %d,%d x,y %d,%d gx,gy %d,%d\n",
                    mm, mm->cox, mm->coy, mm->cx, mm->cy,
                    mm->cgx, mm->cgy);
            }
#endif


            if(xns->cht[i]==CMD_OBJ) {
P;
                if(rest[0]=='\0') {
P;
                    r = xns->chv[i];
        Echo("b mx,my %d,%d\n", mx, my);
                    mx += r->cx;
                    my += r->cy;
        Echo("a mx,my %d,%d\n", mx, my);
                    break;
                }
            }
            if(xns->cht[i]==CMD_NAMESPACE) {
                if(rest[0]=='\0') {
P;
                    r = xns->chv[i];
        Echo("b mx,my %d,%d\n", mx, my);
                    mx += r->cx;
                    my += r->cy;
        Echo("a mx,my %d,%d\n", mx, my);
                }
                else {
P;

        Echo("b mx,my %d,%d\n", mx, my);
                    if(mm) {
                        mx += mm->cox + mm->cx;
                        my += mm->coy + mm->cy;
                    }
        Echo("a mx,my %d,%d\n", mx, my);
                    r = _ns_find_objP(xns->chc[i], rest, &mx, &my);
                }
                break;
            }
#if 0
            else {
                break;
            }
#endif
        }
        /* next */
#if 0
        if(r) break;
#endif
    }

out:
#if 1
    Echo("%s:  xname '%s' ; r %p mx,my %d,%d\n", __func__, xname, r, mx, my);
    Echo("  mx,my %d,%d\n", mx, my);
#endif
    if(xx) *xx = mx;
    if(xy) *xy = my;
    return r;
}

ob*
ns_find_obj(ns* xns, char *xname)
{
    ob* r;

#if 0
    Echo("%s:    xns %p, xname '%s'\n", __func__, xns, xname);
#endif
    r = _ns_find_obj(xns, xname);
#if 0
    if(r) {
        Echo("r   %p oid %d type %d\n", r, r->oid, r->type);
    }
    else {
        Echo("r   %p\n", r);
    }
#endif
    return r;
}

int
ch_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    ch *xch;

    xch = (ch*)xv;
    ik = sprintf(dst, "ch-'%s'",
            xch->name);

    return ik;
}

int
ch_sprintfoid(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    ch *xch;

    xch = (ch*)xv;
    ik = sprintf(dst, "ch(oid %d)-'%s'",
            xch->qob->oid, xch->name);

    return ik;
}


int
revch(ob *x, ch* ref, int *rx, int *ry)
{
    ch  *qch;
    int  r;
    ch  *c;
    int  i;
    varray_t *chpath;
    ob  *u;
    int  mx, my;
    char hep[BUFSIZ];
    char hdp[BUFSIZ];
    char mp[BUFSIZ];

    Echo("%s: enter x %p ref %p START\n", __func__, x, ref);
    if(x) {
        Echo("  x->oid %d\n", x->oid);
    }

    hep[0] = '\0';
    hdp[0] = '\0';

    mx = my = 0;
    qch = NULL;
    r = -3;
    chpath = NULL;
    
    if(!x) {
        r = -1;
        goto out;
    }

#if 1
    mx = x->cx;
    my = x->cy;
#endif

    if(x->behas) {
        chpath = varray_new();
        varray_entrysprintfunc(chpath, ch_sprintf);

        c = x->behas;
        Echo("c %p\n", c);
        while(c) {
            Echo("  c %p parent %p root %p\n", c, c->parent, c->root);
            u = c->qob;
            if(u) {
                Echo("    qob %p oid %d\n", u, u->oid);
                sprintf(mp, "%d,", u->oid);
                strcat(hep, mp);
            }

            varray_push(chpath, (void*)c);

            c = c->parent;
            Echo("c %p\n", c);
        }

#if 0
        varray_dump(chpath);
#endif
#if 0
        varray_fprint(stdout, chpath);
#endif
        Echo("hep |%s|\n", hep);

        Echo("chpath\n");
        for(i=0;i<chpath->use;i++) {
            c = (ch*)chpath->slot[i];
            Echo("  %2d: %p qob %p\n", i, c, c->qob);

            sprintf(mp, "%d,", c->qob->oid);
            strcat(hdp, mp);

            u = c->qob;
            if(u) {
                Echo("    u %p; oid %d ox,oy %d,%d x,y %d,%d\n",
                    u, u->oid, u->cox, u->coy, u->cx, u->cy);
                mx += u->cox + u->cx;
                my += u->coy + u->cy;
            }

            Echo("    mx,my %d,%d\n", mx, my);
            fflush(stdout);

            if(c==ref) {
                r = 1;
                qch = c;
                break;
            }

        }

        Echo("hdp |%s|\n", hdp);

        varray_del(chpath);     
    }
    
out:
    if(r==1) {
        *rx = mx;
        *ry = my;
    }
    if(x) {
        Echo("%s: out1 x %p oid %d; r %d rx,ry %d,%d\n",
            __func__, x, x->oid, r, *rx, *ry);
    }
    else {
        Echo("%s: out2 x %p; r %d rx,ry %d,%d\n", __func__, x, r, *rx, *ry);
    }
    fflush(stdout);

    return r;
}

#define LPOS    (1)
#define PPOS    (2)

int
_ns_find_objposP(ns *xns, ob *b, char *xname, int pmode, int *rx, int *ry)
{
    char  yname[BUFSIZ];
    char  token[BUFSIZ];
    char *p, *q;
    ob   *u;
    int   pos;
    int   r;
    int   ux, uy;
P;
#if 1
        Echo("%s: b %p xname '%s' pmode %d\n",
            __func__, p, xname, pmode);
        if(b) {
            Echo("  b->behas %p\n", b->behas);
        }
#endif

    r = -1;
    yname[0] = '\0';

    q = NULL;
    p = xname;
    while(*p) {
        p = draw_word(p, token, BUFSIZ, '.');
        if((token[0]>='A'&&token[0]<='Z')||token[0]=='_') {
            strcat(yname, token);
            strcat(yname, ".");
            q = p;
        }
        else {
            break;
        }
    }


#if 0
        Echo("  xname '%s' 1\n", xname);
#endif
#if 0
        Echo("  yname '%s' 1\n", yname);
#endif
    dellastcharif(yname, '.');
#if 0
        Echo("  yname '%s' 2\n", yname);
    if(q) {
        Echo("  q     '%s'\n", q);
    }
    if(p) {
        Echo("  p     '%s'\n", p);
    }
#endif

    ux = uy = 0;

    u = _ns_find_objP(xns, yname, &ux, &uy);
    if(u) {
#if 0
        Echo("  u %p gx,gy = %d,%d solved? %d drawed? %d finalized? %d\n",
            u, u->gx, u->gy, u->sizesolved, u->drawed, u->finalized);
        Echo("  u %p ux,uy = %d, %d\n", u, ux, uy);
#endif

#if 1
        Echo("  u behas %p oid %d vs b behas %p oid %d\n",
            u->behas, u->oid, b->behas, b->oid);
#if 0
        if(u->behas != b->behas) 
#endif
        {
            int ik;
            ik = revch(u, b->behas, &ux, &uy);
        }
#endif

        if(q && *q) {
            pos = assoc(pos_ial, q);
#if 0
            Echo("  pos %d q '%s'\n", pos, q);
#endif
            if(pos<0) {
                printf("ERROR ignore position '%s' of '%s'\n", q, yname);
                pos = PO_CENTER;
            }
        }
        else {
            pos = PO_CENTER;
        }
#if 0
        Echo("  pos %d\n", pos);
#endif
        int x, y;

        if(pmode==PPOS) { x = u->cgx;   y = u->cgy; }
        else            { x = u->cx;    y = u->cy; }

#if 0
        Echo("  original x,y = %d,%d ; %s\n", x, y, __func__);
        Echo("     guess x,y = %d,%d\n", ux, uy);
#endif
        *rx = ux;
        *ry = uy;

        switch(pos) {
        case PO_CENTER:    *rx += 0;          *ry += 0;         break;
        case PO_NORTH:     *rx += 0;          *ry += u->cht/2;  break;
        case PO_NORTHEAST: *rx += u->cwd/2;   *ry += u->cht/2;  break;
        case PO_EAST:      *rx += u->cwd/2;   *ry += 0;         break;
        case PO_SOUTHEAST: *rx += u->cwd/2;   *ry += -u->cht/2; break;
        case PO_SOUTH:     *rx += 0;          *ry += -u->cht/2; break;
        case PO_SOUTHWEST: *rx += -u->cwd/2;  *ry += -u->cht/2; break;
        case PO_WEST:      *rx += -u->cwd/2;  *ry += 0;         break;
        case PO_NORTHWEST: *rx += -u->cwd/2;  *ry += u->cht/2;  break;
        default:
            printf("ERROR ignore position '%s' of '%s' -\n", q, yname);
        }

#if 0
        Echo("  cooked  rx,y = %d,%d\n", *rx, *ry);
#endif

        r = 0;
    }
    else {
    }

P;
    Echo("%s: xname %-16s pmode %d; r %d x,y %d,%d\n",
        __func__, xname, pmode, r, *rx, *ry);

    return r;
}


int
_ns_find_objpos(ns *xns, char *xname, int *rx, int *ry, int pmode)
{
    char  yname[BUFSIZ];
    char  token[BUFSIZ];
    char *p, *q;
    ob   *u;
    int   pos;
    int   r;

#if 1
        Echo("%s: xname '%s' pmode %d\n", __func__, xname, pmode);
#endif

    r = -1;
    yname[0] = '\0';

    q = NULL;
    p = xname;
    while(*p) {
        q = p;
        p = draw_word(p, token, BUFSIZ, '.');
        if((token[0]>='A'&&token[0]<='Z')||token[0]=='_') {
            strcat(yname, token);
            strcat(yname, ".");
        }
        else {
            break;
        }
    }


#if 1
        Echo("  yname '%s' 1\n", yname);
#endif
    dellastcharif(yname, '.');
#if 1
        Echo("  yname '%s' 2\n", yname);
    if(q) {
        Echo("  q     '%s'\n", q);
    }
    if(p) {
        Echo("  p     '%s'\n", p);
    }
#endif

    u = ns_find_obj(xns, yname);
    if(u) {
#if 1
        Echo("  u %p gx,gy = %d,%d solved? %d drawed? %d finalized? %d\n",
            u, u->gx, u->gy, u->sizesolved, u->drawed, u->finalized);
#endif

        if(q) {
            pos = assoc(pos_ial, q);
#if 1
            Echo("  pos %d q '%s'\n", pos, q);
#endif
        }
        else {
            pos = PO_CENTER;
        }
#if 1
        Echo("  pos %d\n", pos);
#endif
        int x, y;

        if(pmode==PPOS) { x = u->cgx;   y = u->cgy; }
        else            { x = u->cx;    y = u->cy; }

#if 1
        Echo("  original x,y = %d,%d ; %s\n", x, y, __func__);
#endif

        switch(pos) {
        case PO_CENTER:    *rx = x;          *ry = y;           break;
        case PO_NORTH:     *rx = x;          *ry = y+u->cht/2;  break;
        case PO_NORTHEAST: *rx = x+u->cwd/2; *ry = y+u->cht/2;  break;
        case PO_EAST:      *rx = x+u->cwd/2; *ry = y;           break;
        case PO_SOUTHEAST: *rx = x+u->cwd/2; *ry = y-u->cht/2;  break;
        case PO_SOUTH:     *rx = x;          *ry = y-u->cht/2;  break;
        case PO_SOUTHWEST: *rx = x-u->cwd/2; *ry = y-u->cht/2;  break;
        case PO_WEST:      *rx = x-u->cwd/2; *ry = y;           break;
        case PO_NORTHWEST: *rx = x-u->cwd/2; *ry = y+u->cht/2;  break;
        default:
            printf("ERROR ignore position '%s' of '%s'\n", p, xname);
        }

#if 1
        Echo("  cooked  rx,y = %d,%d\n", *rx, *ry);
#endif

        r = 0;
    }
    else {
    }

P;
    Echo("%s: xname %-16s pmode %d; r %d x,y %d,%d\n",
        __func__, xname, pmode, r, *rx, *ry);

    return r;
}


int
ns_find_objpos(ns *xns, char *xname, int *rx, int *ry)
{
    return _ns_find_objpos(xns, xname, rx, ry, LPOS);
}

int
ns_find_objposG(ns *xns, char *xname, int *rx, int *ry)
{
    return _ns_find_objpos(xns, xname, rx, ry, PPOS);
}

ob*
_ns_find_objX(ns* xns, char *xname, int *ux, int *uy)
{
    char head[BUFSIZ];
    char rest[BUFSIZ];
    int i;
    ob* r;
    int ik;
    int cx, cy;

#if 1
    Echo("  %s: xns %p, xname '%s'\n", __func__, xns, xname);
#endif
#if 0
    ns_dump(xns);
#endif

    *ux = -99999;
    *uy = -99999;

    r = NULL;
    if(!xns) {
        goto out;
    }

#if 0
    Echo("  xname '%s'\n", xname);
#endif
    ik = splitdot(head, BUFSIZ, rest, BUFSIZ, xname);
#if 0
    Echo("  head  '%s' rest '%s'\n", head, rest);
#endif

    for(i=0;i<xns->nch;i++) {
#if 0
        Echo("     %2d '%s' vs '%s'\n", i, head, xns->chn[i]);
#endif
        if(strcasecmp(xns->chn[i], head)==0) {
P;
#if 1
            Echo("       FOUND\n");
            Echo("         chn '%s'\n", xns->chn[i]);
            Echo("         cht %d\n",   xns->cht[i]);
            Echo("         chv %p\n",   xns->chv[i]);
#endif
            if(xns->cht[i]==CMD_OBJ) {
P;
                if(rest[0]=='\0') {
P;
                    r = xns->chv[i];
                    *ux = r->gx;
                    *uy = r->gy;
                    break;
                }
            }
            if(xns->cht[i]==CMD_NAMESPACE) {
                if(rest[0]=='\0') {
P;
                    r = xns->chv[i];
#if 1
#endif
                }
                else {
P;
                    r = _ns_find_objX(xns->chc[i], rest, &cx, &cy);

#if 1
                    Echo("  nest %d,%d and %d,%d\n", r->ox, r->oy, cx, cy);
#endif

                    *ux = r->ox + cx;
                    *uy = r->oy + cy;
                }
                break;
            }
#if 0
            else {
                break;
            }
#endif
        }
        /* next */
#if 0
        if(r) break;
#endif
    }

out:
#if 1
    Echo("%s: r %p; %d,%d\n", __func__, r, *ux, *uy);
#endif
    return r;
}

int
_ns_find_objposX(ns *xns, char *xname, int *rx, int *ry, int pmode)
{
    char  yname[BUFSIZ];
    char  token[BUFSIZ];
    char *p, *q;
    ob   *u;
    int   pos;
    int   r;

    int   gx, gy;

#if 1
        Echo("%s: xname '%s' pmode %d\n", __func__, xname, pmode);
#endif

    r = -1;
    yname[0] = '\0';

    q = NULL;
    p = xname;
    while(*p) {
        q = p;
        p = draw_word(p, token, BUFSIZ, '.');
        if((token[0]>='A'&&token[0]<='Z')||token[0]=='_') {
            strcat(yname, token);
            strcat(yname, ".");
        }
        else {
            break;
        }
    }


#if 1
        Echo("  yname '%s' 1\n", yname);
#endif
    dellastcharif(yname, '.');
#if 1
        Echo("  yname '%s' 2\n", yname);
    if(q) {
        Echo("  q     '%s'\n", q);
    }
    if(p) {
        Echo("  p     '%s'\n", p);
    }
#endif


#if 1

#endif


    u = _ns_find_objX(xns, yname, &gx, &gy);
    if(u) {
#if 1
        Echo("  xname '%s'\n", xname);
#endif
#if 1
        Echo("  u %p gx,y %d,%d\n", u, gx, gy);
#endif

        if(q) {
            pos = assoc(pos_ial, q);
#if 1
            Echo("  pos %d q '%s'\n", pos, q);
#endif
        }
        else {
            pos = PO_CENTER;
        }
#if 1
        Echo("  pos %d\n", pos);
#endif
        int x, y;

#if 1
        Echo("  guess    x,y = %d,%d\n", gx, gy);
#endif

        if(pmode==PPOS) { x = u->cgx;   y = u->cgy; }
        else            { x = u->cx;    y = u->cy; }

#if 1
        Echo("  original x,y = %d,%d ; %s\n", x, y, __func__);
#endif

        switch(pos) {
        case PO_CENTER:    *rx = x;          *ry = y;           break;
        case PO_NORTH:     *rx = x;          *ry = y+u->cht/2;  break;
        case PO_NORTHEAST: *rx = x+u->cwd/2; *ry = y+u->cht/2;  break;
        case PO_EAST:      *rx = x+u->cwd/2; *ry = y;           break;
        case PO_SOUTHEAST: *rx = x+u->cwd/2; *ry = y-u->cht/2;  break;
        case PO_SOUTH:     *rx = x;          *ry = y-u->cht/2;  break;
        case PO_SOUTHWEST: *rx = x-u->cwd/2; *ry = y-u->cht/2;  break;
        case PO_WEST:      *rx = x-u->cwd/2; *ry = y;           break;
        case PO_NORTHWEST: *rx = x-u->cwd/2; *ry = y+u->cht/2;  break;
        default:
            printf("ERROR ignore position '%s' of '%s'\n", p, xname);
        }

#if 1
        Echo("  cooked  rx,y = %d,%d\n", *rx, *ry);
#endif

        r = 0;
    }
    else {
    }

P;
    Echo("%s: xname %-16s pmode %d; r %d x,y %d,%d\n",
        __func__, xname, pmode, r, *rx, *ry);

    return r;
}


int
ns_find_objposX(ns *xns, char *xname, int *rx, int *ry)
{
    return _ns_find_objposX(xns, xname, rx, ry, LPOS);
}

int
ns_find_objposXG(ns *xns, char *xname, int *rx, int *ry)
{
    return _ns_find_objposX(xns, xname, rx, ry, PPOS);
}


char*
_ns_find_name(ns* s, ob* xob, int w)
{
    int i;
    int u;
    char *q;
#define W for(u=0;u<w;u++) printf(" ");
    if(!s) {
        return NULL;
    }

    W;
    printf("namespace %p name '%s' root %p parent %p num %d\n",
        s, s->name, s->root, s->parent, s->nch);
    for(i=0;i<s->nch;i++) {
        W;
        printf("%3d: '%s' type %d ", i, s->chn[i], s->cht[i]);
        if(s->cht[i]==CMD_NAMESPACE) {
            W;
            printf("namespace object %p oid %d\n", s->chv[i], s->chv[i]->oid);
            q = _ns_find_name(s->chc[i], xob, w+dumptabstop);
            if(q) {
                return q;
            }
        }
        else {
            printf("object %p oid %d\n", s->chv[i], s->chv[i]->oid);
            if(s->chv[i]==xob) {
                return s->chn[i];
            }
        }
    }

    return NULL;
}

