/*
 * tangy second trial since 2016 Aug 
 * K.Chinen
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "alist.h"
#include "varray.h"
#include "color.h"

#include "qbb.h"
#include "geom.h"
#include "obj.h"
#include "gv.h"
#include "chas.h"
#include "notefile.h"
#include "font.h"
#include "seg.h"
#include "epsdraw.h"
#include "a.h"

#ifndef MAX
#define MAX(a,b)    ((a)>(b) ? (a) : (b))
#endif

int
confirm_objattr(struct obattr *xoa) 
{
#if 0
    if(xoa.fillhatch==HT_SOLID) {
        if(xoa.fillcolor<0) {
            xoa.fillcolor = def_color;
        }
    }
    if(xoa.fillcolor>=0) {
        if(xoa.textbgcolor<0) {
            xoa.textbgcolor = def_bgcolor;
        }
    }
            xoa.textbgcolor = def_bgcolor;

#endif

    if(xoa->shadow) {
        if(xoa->backhatch==HT_NONE) {
            xoa->backcolor = def_bgcolor;
            xoa->backhatch = HT_SOLID;
        }
    }

    return 0;
}

int
confirm_attr(ob *xo)
{
    struct obattr *xoa;

    double bp2ip;
    double ip2bp;

    /* XXX */
    ip2bp = 0.01;
    bp2ip = 1.0/ip2bp;

    xoa = &xo->vob;

#if 0
    if(xoa->ato && xoa->afrom) {
        xo->floated = 1;
    }
#endif
#if 1
    if(xoa->hasto || xoa->hasfrom) {
        xo->floated = 1;
    }
#endif

#if 1
    if(xo->type==CMD_AUXLINE) {
        if(xo->cauxlinetype == ALT_UNKNOWN) {
            xo->cauxlinetype          = ALT_ARROW;
            xo->cob.arrowheadpart     = AR_FORE;
            xo->cob.arrowforeheadtype = AH_NORMAL;
        }
    }
#endif
    
    if(xo->type==CMD_POLYGON) {
        if(xoa->polypeak<3) {
            int nv;
#if 0
fprintf(stderr, "polygon peak %d\n", xoa->polypeak);
#endif
            nv = 3;
            Info("SET oid %d polygon peak %d -> %d\n",
                xo->oid, xoa->polypeak, nv);
            xoa->polypeak = nv;
        }
    }
    
    if(xo->type==CMD_GEAR) {
        if(xoa->polypeak<4) {
            int nv;
#if 0
fprintf(stderr, "polygon peak %d\n", xoa->polypeak);
#endif
            nv = 32;
            Info("SET oid %d polygon peak %d -> %d\n",
                xo->oid, xoa->polypeak, nv);
            xoa->polypeak = nv;
        }
    }

    if(xo->type==CMD_OBJLOAD) {
        if(xoa->filestr) {
            int    ik;
            qbb_t *qbb;
            qbb_t *ibb;
            int    w0, h0;
            int    w, h;
extern int epsparse_fp(char *fname, qbb_t *qbb);

            qbb = qbb_new();
            if(!qbb) {
                fprintf(stderr, "no memory\n");
                return -1;
            }
            ik = epsparse_fp(xoa->filestr, qbb);
#if 0
            fprintf(stderr, " ik %d: %d %d %d %d\n",
                ik, qbb->lx, qbb->by, qbb->rx, qbb->ty);
#endif

            if(ik) {
                fprintf(stderr, "cannot parse file '%s'\n", xoa->filestr);
                return -1;
            }

#if 0
            fprintf(stderr, "---\n");
            fprintf(stderr, " 0 oid %-3d wd %6d ht %6d scale %7.3f,%7.3f\n",
                xo->oid, xo->wd, xo->ht, xoa->filescalex, xoa->filescaley);
#endif

            ibb = qbb_new();
            if(!ibb) {
                fprintf(stderr, "no memory\n");
                return -1;
            }
            qbb_setbb(ibb, (int)(qbb->lx*bp2ip), (int)(qbb->by*bp2ip),
                (int)(qbb->rx*bp2ip), (int)(qbb->ty*bp2ip));

            w0 = ibb->rx - ibb->lx;
            h0 = ibb->ty - ibb->by;

            if(xo->wd>0) {
                if(xo->ht>0) {
                    xoa->filescalex = ((double)(xo->wd - xoa->imargin*2))/w0;
                    xoa->filescaley = ((double)(xo->ht - xoa->imargin*2))/h0;
                }
                else {
                    xoa->filescalex = ((double)(xo->wd - xoa->imargin*2))/w0;
                    xoa->filescaley = xoa->filescalex;
                    xo->ht = (int)((double)h0*xoa->filescalex) + xoa->imargin*2;
                }
            }
            else
            if(xo->ht>0) {
                    xoa->filescaley = ((double)(xo->ht - xoa->imargin*2))/h0;
                    xoa->filescalex = xoa->filescaley;
                    xo->wd = (int)((double)w0*xoa->filescaley) + xoa->imargin*2;
            }
            else {
#define RDELTA  (0.00001)
                if(fabs(xoa->filescalex)>RDELTA) {
                    if(fabs(xoa->filescaley)>RDELTA) {
                        xo->wd = xoa->filescalex*w0 + xoa->imargin*2;
                        xo->ht = xoa->filescaley*h0 + xoa->imargin*2;
                    }
                    else {
                        xo->wd = xoa->filescalex*w0 + xoa->imargin*2;
                        xo->ht = xoa->filescalex*h0 + xoa->imargin*2;
                        xoa->filescaley = xoa->filescalex;
                    }
                }
                else
                if(fabs(xoa->filescaley)>RDELTA) {
                        xo->wd = xoa->filescaley*w0 + xoa->imargin*2;
                        xo->ht = xoa->filescaley*h0 + xoa->imargin*2;
                        xoa->filescalex = xoa->filescaley;
                }
                else {
                    xo->wd = w0 + xoa->imargin*2;
                    xo->ht = h0 + xoa->imargin*2;
                    xoa->filescalex = 1.0;
                    xoa->filescaley = 1.0;
                }
            }

#if 0
            fprintf(stderr, " 1 oid %-3d wd %6d ht %6d scale %7.3f,%7.3f\n",
                xo->oid, xo->wd, xo->ht, xoa->filescalex, xoa->filescaley);
#endif

            free(ibb);
            free(qbb);
        }
    }

    return 0;
}

int
dump_objattr(struct obattr xo) 
{
    printf("  outlinecolor     %d\n", xo.outlinecolor);
    printf("  outlinetype      %d\n", xo.outlinetype);
    printf("  outlinethick     %d\n", xo.outlinethick);
#if 0
    printf("  outlinethickmode %d\n", xo.outlinethickmode);
#endif
    printf("  fillcolor        %d\n", xo.fillcolor);
    printf("  fillhatch        %d\n", xo.fillhatch);
    printf("  fillthick        %d\n", xo.fillthick);
    printf("  fillpitch        %d\n", xo.fillpitch);
    printf("  backcolor        %d\n", xo.backcolor);
    printf("  backhatch        %d\n", xo.backhatch);
    printf("  backthick        %d\n", xo.backthick);
    printf("  backpitch        %d\n", xo.backpitch);
#if 0
    printf("    hatchthick     %d\n", xo.hatchthick);
    printf("    hatchpitch     %d\n", xo.hatchpitch);
#endif
#if 0
    printf("  filllevel        %d\n", xo.filllevel);
    printf("  imargin          %d\n", xo.imargin);
#endif
    printf("  polypeak         %d\n", xo.polypeak);
    printf("  rotateval        %d\n", xo.rotateval);
    printf("  rad              %d\n", xo.rad);
    return 0;
}

int
cha_fprint(FILE *fp, char *msg, ch *tg)
{
    fprintf(fp, "%s dir %d thick(%d) %d scale %d x,y %d,%d\n",
        msg, tg->dir, tg->thickmode, tg->thick, tg->scale, tg->x, tg->y);
    fflush(fp);
    return 0;
}

int
cha_reset(ch *xch)
{
    if(!xch) {
        printf("ERROR no chunk\n");
        return -1;
    }
    xch->dir        = def_dir;
    xch->thickmode  = def_linethickmode;
    xch->thick      = def_linethick;
    xch->scale      = def_scale;
    xch->x          = 0;
    xch->y          = 0;

    return 0;
}

int
cha_copy(ch *dst, ch *src)
{
    dst->dir        = src->dir;
    dst->thickmode  = src->thickmode;
    dst->thick      = src->thick;
    dst->scale      = src->scale;
    dst->x          = src->x;
    dst->y          = src->y;

#if 0
    cha_fprint(stdout, "src", src);
    cha_fprint(stdout, "dst", dst);
#endif

    return 0;
}



int dumptabstop=2;
int dumplabel=26;

int
_ns_dump(ns* s, int w)
{
    int i;
    int u;
#define W for(u=0;u<w;u++) printf(" ");
    if(!s) {
        return -1;
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
            _ns_dump(s->chc[i], w+dumptabstop);
        }
        else {
            printf("object %p oid %d\n", s->chv[i], s->chv[i]->oid);
        }
    }

    return 0;
}

int
ns_dump(ns* s)
{
    int r;
    printf("***\n");
    r = _ns_dump(s, 0);
    printf("---\n");
    return r;
}

int
_ob_dump(ob* s, int w)
{
    int i;
    int u;

#define W for(u=0;u<w;u++) printf(" ");


    printf("%3d:%c ", s->oid, ISCHUNK(s->type)? 'C': '_');

    W;
    printf("%s ", rassoc(cmd_ial, s->type));

    if(ISCHUNK(s->type)) 
    {
        if(s->cch.name[0]) {
            printf("%p name '%s' root %p parent %p num %d\n",
                s, s->cch.name,
                s->cch.root, s->cch.parent, s->cch.nch);
        }
        else {
            printf("%p root %p parent %p num %d\n",
                s, 
                s->cch.root, s->cch.parent, s->cch.nch);
        }
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_dump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }
    else {
        printf("%p %d type %d\n", s, s->ignore, s->type);
    }

    fflush(stdout);
    return 0;
#undef W
}

int
ob_dump(ob* s)
{
    int r;
    printf("=== DUMP\n");
    r = _ob_dump(s, 0);
    printf("---\n");
    return r;
}



int
_ob_gdump(ob* s, int w)
{
    int i;
    int u;
    char tm[BUFSIZ];
    int rest;

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    strcpy(tm, rassoc(cmd_ial, s->type));

    printf("%3d: ", s->oid);

    W;
    printf("%-*s %6d,%-6d %6dx%-6d %6d,%-6d %6d,%-6d %6d,%-6d\n",
        rest,
        tm,
        s->ox, s->oy, s->wd, s->ht,
        s->x, s->y, s->sx, s->sy, s->ex, s->ey);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_gdump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }


    fflush(stdout);
    return 0;
#undef W
}

int
ob_gdump(ob* s)
{
    int r;
    printf("=== GDUMP\n");
    printf("oid: type                          oxy           wxh           xy            sxy           exy\n");

    r = _ob_gdump(s, 0);
    printf("---\n");
    return r;
}





int
_ob_adump(ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    printf("%3d: ", s->oid);
    W;
    printf("%-*s %6d,%-6d %6dx%-6d %d %d %6d,%-6d %6d,%-6d %6d,%-6d\n",
        rest, tm,
        s->ox, s->oy, s->wd, s->ht,
        s->fixed, 
        s->cob.originalshape,
        s->gx, s->gy, s->gsx, s->gsy, s->gex, s->gey);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_adump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    fflush(stdout);
    return 0;
#undef W
}

int
ob_adump(ob* s)
{
    int r;
    printf("=== ADUMP\n");
    printf(
"oid: type                         ox,oy          wxh      f o     gx,y        gsx,sy         gex,ey\n");


    r = _ob_adump(s, 0);
    printf("---\n");
    return r;
}





int
_ob_bldump(ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    printf("%3d: ", s->oid);
    W;
    printf("%-*s %d %d: %6d,%-6d : %6d %6d %6d %6d\n",
        rest, tm,
        s->hasrel, s->fixed,
        s->x, s->y, s->lx, s->by, s->rx, s->ty);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_bldump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    fflush(stdout);
    return 0;
#undef W
}

int
_ob_bgdump(ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    printf("%3d: ", s->oid);
    W;
    printf("%-*s %d %d: %6d,%-6d : %6d %6d %6d %6d\n",
        rest, tm,
        s->hasrel, s->fixed,
        s->gx, s->gy, s->glx, s->gby, s->grx, s->gty);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_bgdump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    fflush(stdout);
    return 0;
#undef W
}


int
_ob_cndump(ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) printf(" ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    printf("%3d: ", s->oid);
    W;
#if 0
    printf("%-*s %d %d: %5d,%-5d : %5d %5d %5d %5d\n",
        rest, tm,
        s->hasrel, s->fixed,
        s->gx, s->gy, s->glx, s->gby, s->grx, s->gty);
#endif
    printf("%-*s %p\n", rest, tm, s->cch.qns);

#if 0
    if(s->type==CMD_CHUNK) 
#endif
    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _ob_cndump((ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    fflush(stdout);
    return 0;
#undef W
}

int
ob_bldump(ob* s)
{
    int r;
    printf("=== BLDUMP\n");
    printf(
"oid: type                     r f:      x,y      :   lx     by     rx     ty\n");

    r = _ob_bldump(s, 0);
    printf("---\n");
    return r;
}

int
ob_bgdump(ob* s)
{
    int r;
    printf("=== BGDUMP\n");
    printf(
"oid: type                     r f:     gx,gy     :   glx    gby    grx    gty\n");

    r = _ob_bgdump(s, 0);
    printf("---\n");
    return r;
}


int
ob_cndump(ob*s )
{
    int r;
    printf("=== CNDUMP\n");
    printf(
"oid: type                     ns\n");

    r = _ob_cndump(s, 0);
    printf("---\n");
    return r;
}


int
_printobjlist(FILE *fp, char *pre, ob* s, int w)
{
    int  i;
    int  u;
    int  rest;
    char tm[BUFSIZ];

#define W for(u=0;u<w;u++) fprintf(fp, " ");

    strcpy(tm, rassoc(cmd_ial, s->type));

    rest = dumplabel - w - dumptabstop;

    fprintf(fp, "%s%3d: ", pre, s->oid);
    W;
#if 0
    fprintf(fp, "%-*s %d %d: %5d,%-5d : %5d %5d %5d %5d\n",
        rest, tm,
        s->hasrel, s->fixed,
        s->gx, s->gy, s->glx, s->gby, s->grx, s->gty);
#endif

    fprintf(fp, "%-*s\n",
        rest, tm);

    if(ISCHUNK(s->type))
    {
        for(i=0;i<s->cch.nch;i++) {
            if(s->cch.ch[i]) {
                _printobjlist(fp, pre, (ob*)s->cch.ch[i], w+dumptabstop);
            }
        }
    }

    return 0;
#undef W
}

int
printobjlist(FILE *fp, char *pre, ob* s)
{
    int r;
    fprintf(fp, "%sobject list\n", pre);
#if 0
    fprintf(fp,
"%soid: type                     r f:    gx,gy    :   glx   gby   grx   gty\n", pre);
#endif

    fprintf(fp, "%soid: type\n", pre);

    r = _printobjlist(fp, pre, s, 0);
    fprintf(fp, "%s\n", pre);
    fflush(fp);
    return r;
}





ns*
newnamespace()
{
    ns* r;
    r = malloc(sizeof(ns));
    if(r) {
        memset(r, 0, sizeof(ns));
    }
    r->root = r;
    return r;
}

ns*
newnamespace_child(ns*pns)
{
    ns* r;
    r = malloc(sizeof(ns));
    if(r) {
        memset(r, 0, sizeof(ns));
    }
    r->root = pns->root;
    r->parent = pns;
    return r;
}

int hide_serial=1;

#if 0
int
ss_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    sstr *sv;

    if(!xv) return -1;
    sv = (sstr*)xv;
    ik = sprintf(dst, "<'%s' %x>", sv->ssval, sv->ssopt);

    return ik;
}
#endif

int
segop_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    segop *sv;

    if(!xv) return -1;
    sv = (segop*)xv;
    ik = sprintf(dst, "<%s(%d) '%s'>",
            rassoc(objattr_ial, sv->cmd), sv->cmd, sv->val);

    return ik;
}

int
seg_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    seg* s;
    char *mcr="E";
    char *mft="E";

    char tmp1[32];
    char tmp2[32];

    if(!xv) return -1;
    s = (seg*)xv;

    if(!s) {
#if 0
    ik = sprintf(dst, "<%-12.12s,%s(%d),%d,%d;%6d,%6d,%6d,%6d;%6d,%4d>",
#endif
    ik = sprintf(dst, "<%-12s,%-7s,%1s,%1s;%6s,%6s,%6s,%6s,%6s,%6s,%6s,%6s;%6s,%4s>",
            "type", "join", "c", "f",
            "x1", "y1", "x2", "y2", "x3", "y3", "x4", "y4",
            "rad", "ang");
    return 0;
    }

    memset(tmp1, 0, sizeof(tmp1));
    sprintf(tmp1, "%s(%d)", rassoc(objattr_ial, s->ptype), s->ptype);
    memset(tmp2, 0, sizeof(tmp2));
    sprintf(tmp2, "%s(%d)", s->jtype>0 ? "JOIN" : "-", s->jtype);

    mcr = rassoc(coord_isal, s->coordtype);
    mft = rassoc(ftcoord_isal, s->ftflag);

#if 0
    ik = sprintf(dst, "(%s(%d),%s(%d),%d,%d;%d,%d,%d,%d;%d,%d)",
#endif
    ik = sprintf(dst, "<%-12.12s,%-7.7s,%s,%s;%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d;%6d,%4d>",
        tmp1, tmp2,
        mcr, mft,
        s->x1, s->y1, s->x2, s->y2,
        s->x3, s->y3, s->x4, s->y4,
        s->rad, s->ang);

    return 0;
}

#if 0
int
varray_fprintv(FILE *fp, varray_t *ar)
{
    int i;
    int ik;
    char tmp[BUFSIZ];

    if(!ar) {
        return -1;
    }
    if(!ar->sprintfunc) {
        Error("sprintfunc is not registered\n");
    }

    fprintf(fp, "varray %p; %d/%d\n",
        ar, ar->use, ar->len);

    tmp[0] = '\0';
    ik = ar->sprintfunc(tmp, BUFSIZ, NULL, 0);
    if(ik == 0 && tmp[0]) {
        fprintf(fp, "num : %s\n", tmp);
    }

    for(i=0;i<ar->use;i++) {
        if(!ar->sprintfunc) {
            strcpy(tmp, "---");
        }
        else {
            ik = ar->sprintfunc(tmp, BUFSIZ, ar->slot[i], 0);
        }
        fprintf(fp, "%4d: %s\n", i, tmp);
    }
    return 0;
}
#endif

ob*
newobj()
{
    ob* r;
#if 0
P;
#endif
    r = malloc(sizeof(ob));
    if(!r) {
        printf("ERROR no memory\n");
        return NULL;
    }
    memset(r, 0, sizeof(ob));
    r->oid = hide_serial++;

    r->cob.outlinecolor = def_fgcolor;
    r->cob.textcolor    = def_fgcolor;
    r->cob.textbgcolor  = def_bgcolor;
    r->cob.fillcolor    = def_bgcolor;

    r->cob.rad      = -1;
    r->cauxlinedistance = objunit/4;
    r->cauxlinetype = ALT_UNKNOWN;
#if 0
    r->cauxlinetype = ALT_ARROW;
#endif
    
    r->cob.ssar     = varray_new();
    r->cob.segopar  = varray_new();
    r->cob.segar    = varray_new();
    r->cob.seghar   = varray_new();
    varray_entrysprintfunc(r->cob.ssar,     ss_sprintf);
    varray_entrysprintfunc(r->cob.segopar,  segop_sprintf);
    varray_entrysprintfunc(r->cob.segar,    seg_sprintf);
    varray_entrysprintfunc(r->cob.seghar,   seg_sprintf);

    qbb_reset(&r->localbb);
    qbb_reset(&r->globalbb);

    r->cob.filestr = NULL;
    r->cob.filescalex = 0.0;
    r->cob.filescaley = 0.0;

    return r;
}

ob*
cloneobj(ob* oo)
{
    ob* r;
#if 0
P;
#endif
    r = malloc(sizeof(ob));
    if(!r) {
        printf("ERROR no memory\n");
        return NULL;
    }
    memcpy(r, oo, sizeof(ob));
    r->oid = hide_serial++;

    return r;
}

ob*
newbox()
{
    ob* r;

    r = newobj();
    if(r) {
        r->type = CMD_BOX;
    }
    return r;
}

ob*
newchunk()
{
    int  i;
    ob*  r;
    ch*  rx;
#if 0
P;
#endif
    r  = newobj();
    if(!r) {
        return NULL;
    }

    rx = (ch*)malloc(sizeof(ch));
    if(!rx) {
        printf("ERROR no memory\n");
        free(r);
        return NULL;
    }

    memset(rx, 0, sizeof(ch));
    rx->nch = 0;
    for(i=0;i<CH_OBJ_LEN;i++) {
        rx->ch[i] = NULL;
    }

    r->type = CMD_CHUNK;
    r->cch  = *rx;
    r->cch.root = &r->cch;

    free(rx);
    rx = NULL;

#if 0
    printf("%s:%d r %p\n", __func__, __LINE__, r);
#endif
    return r;
}

ob*
newchunk_child(ob* pch)
{
    ob* nch;
    nch = newchunk();
    if(!nch) {
        return NULL;
    }

    sprintf(nch->cch.name, "copy of %s", pch->cch.name);
#if 0
    nch->cch.dir   = pch->cch.dir;
    nch->cch.scale = pch->cch.scale;
#endif
    cha_copy(&nch->cch, &pch->cch);
    nch->cch.root   = pch->cch.root;
    nch->cch.parent = &pch->cch;
#if 0
    nch->cch.qob    = pch;
#endif
#if 1
    nch->cch.qob    = nch;
#endif

    return nch;
}


int
chadd(ob* xch, ob* xob)
{
#if 0
    printf("%s:%d xch %p xob %p\n", __func__, __LINE__, xch, xob);
#endif

    if(!xch) {
        printf("%s: ERROR NULL chunk\n", __func__);
        return -1;
    }
#if 0
    if(xch->type!=CMD_CHUNK) 
#endif
    if(!ISCHUNK(xch->type))
    {
        printf("%s: ERROR not chunk oid %d\n", __func__, xch->type);
        return -1;
    }
    if(!xob) {
        printf("%s: ERROR NULL object\n", __func__);
        return -1;
    }

#if 0
    printf("%s:%d xch %d xob %d\n",
        __func__, __LINE__, xch->oid, xob->oid);
#endif

#if 0
    printf("nch %3d/%d \n", xch->cch.nch, CH_OBJ_LEN);
#endif
    if(xch->cch.nch>=CH_OBJ_LEN-1) {
        Error("no slot in chunk\n");
    }

    xch->cch.ch[xch->cch.nch] = (struct obattr*)xob;
    xch->cch.nch++;

#if 1
    xob->behas = &xch->cch;
#endif

    return 0;
}

int
chdel(ob* xch, ob* xob)
{
    ob* stored;
P;
#if 0
    printf("%s:%d xch %p xob %p\n", __func__, __LINE__, xch, xob);
#endif

    if(!xch) {
        printf("%s: ERROR NULL chunk\n", __func__);
        return -1;
    }
#if 0
    if(xch->type!=CMD_CHUNK) 
#endif
    if(!ISCHUNK(xch->type))
    {
        printf("%s: ERROR not chunk oid %d\n", __func__, xch->type);
        return -1;
    }
    if(!xob) {
        printf("%s: ERROR NULL object\n", __func__);
        return -1;
    }
    
    stored = NULL;

    if(xch->cch.nch<=0) {
        printf("ERROR no member chunk\n");
    }

    stored = (ob*) xch->cch.ch[xch->cch.nch-1];
    xch->cch.ch[xch->cch.nch-1] = NULL;
    xch->cch.nch--;

    if(stored!=xob) {
        printf("ERROR not equal stored %p xob %p\n", stored, xob);
    }
    else {
        Echo("\tequal stored %p xob %p\n", stored, xob);
    }

    return 0;
}

int
nsaddobj(ns* xns, ob* xob, char *label)
{
#if 0
P;
#endif
    xns->chn[xns->nch] = strdup(label);
    xns->cht[xns->nch] = CMD_OBJ;
    xns->chv[xns->nch] = xob;
    xns->chc[xns->nch] = NULL;
    xns->nch++;
    return 0;
}

int
nsaddns(ns* xns, ns* xob, char *label)
{
#if 0
P;
#endif
    xns->chn[xns->nch] = strdup(label);
    xns->cht[xns->nch] = CMD_NAMESPACE;
    xns->chv[xns->nch] = NULL;
    xns->chc[xns->nch] = xob;
    xns->nch++;
    return 0;
}

int
nsaddnsobj(ns* xns, ns* nns, ob* xob, char *label)
{
#if 0
P;
#endif
    xns->chn[xns->nch] = strdup(label);
    xns->cht[xns->nch] = CMD_NAMESPACE;
    xns->chv[xns->nch] = xob;
    xns->chc[xns->nch] = nns;
    xns->nch++;
    return 0;
}


int
splitdot(char *h, int hlen, char *r, int rlen, char *full)
{
    char *p, *q;
    int  c;

    p = full;
    q = h;
    c = 0;
    while(*p && *p!='.' && c<hlen) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    if(*p=='.') {
        p++;
        strcpy(r, p);
    }
    else {
        r[0] = '\0';
    }

    return 0;
}


int
recalcsizeparam()
{
    Echo("%s: enter\n", __func__);

    if(objunit<=0) {
        Error("sorry unit size is zero or negative.\n");
        exit(7);
    }

#define V(x)    printf("%-20s %8d\n", #x, x);
    
    def_arrowsize       = arrowsizefactor       * objunit;
    def_warrowsize      = warrowsizefactor      * objunit;
    def_barrowgap       = barrowgapfactor       * objunit;

    def_linethick       = linethickfactor       * objunit;
    def_wlinethick      = wlinethickfactor      * objunit;
    def_linedecothick   = linedecothickfactor   * objunit;
    def_linedecopitch   = linedecopitchfactor   * objunit;
    def_mustsize        = mustsizefactor        * objunit;
#if 1
    if(def_linethick<=0) {
        def_linethick = 1;
    }
#endif
#if 1
    if(def_wlinethick<=0) {
        def_wlinethick = 1;
    }
#endif


    def_textheight      = textheightfactor      * objunit;
    def_hatchpitch      = hatchpitchfactor      * objunit;
    def_hatchthick      = hatchthickfactor      * objunit;
#if 1
    if(def_hatchthick<=0) {
        def_hatchthick = 1;
    }
#endif
    def_noteosep        = noteosepfactor        * objunit;
    def_noteisep        = noteisepfactor        * objunit;
    def_marknoderad     = marknoderadfactor     * objunit;
    def_markbbthick     = markbbthickfactor     * objunit;

    def_pbstrgap        = pbstrgapfactor        * objunit;

#if 0
    V(objunit);
    V(def_arrowsize);
    V(def_linethick);
    V(def_wlinethick);
    V(def_linedecothick);
    V(def_linedecopitch);
    V(def_barrowgap);
    V(def_textheight);
    V(def_hatchpitch);
    V(def_hatchthick);
#endif
#if 0
    V(def_textdecent);
#endif

    return 0;
}
