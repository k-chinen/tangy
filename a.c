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
#include "a.h"

#ifndef MAX
#define MAX(a,b)    ((a)>(b) ? (a) : (b))
#endif



char*
skipwhite(char *p)
{
    while(*p==' ' ||*p=='\t') {
        p++;
    }
    return p;
}

char*
chomp(char *line)
{
    char *p;

    if(line==NULL) {
        return NULL;
    }

    p = line;
    while(*p) {
        if(*p=='\r' && *(p+1)=='\n' && *(p+2)=='\0') {
            *p = '\0';
            break;
        }
        if(*p=='\n' && *(p+1)=='\0') {
            *p = '\0';
            break;
        }
        p++;
    }

    return line;
}

int
dellastcharif(char *src, int xch)
{
    char *p, *q;

    q = NULL;
    p = src;
    while(*p) {
        q = p;
        p++;
    }

#if 0
    printf("%s: ", __func__);
    printf(" b src |%s| -> ", src);
#endif
    if(q && *q==xch) *q='\0';
#if 0
    printf(" a src |%s|\n", src);
#endif
    return 0;
}

/* double quoted string */
char*
draw_wordDQ(char *src, char *dst, int wlen)
{
    register char *p, *q;
    int c;

    p = src;
    q = dst;
    c = 0;
    if(*p!='"') {
        return NULL;
    }
    p++;
    while(*p&&c<wlen&&(*p!='"')) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    if(*p=='"') {
        p++;
    }

    return p;
}


char*
draw_wordW(char *src, char *dst, int wlen)
{
    register char *p, *q;
    int c;

    p = src;
    q = dst;
    c = 0;
    while(*p&&c<wlen&&(*p!=' ' && *p!='\t' && *p!='\n')) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    while(*p==' '||*p=='\t'|| *p=='\n') {
        p++;
    }

    return p;
}

char*
draw_word(char *src, char *dst, int wlen, int sep)
{
    register char *p, *q;
    int c;

    p = src;
    q = dst;
    c = 0;
    while(*p&&c<wlen&&(unsigned char)*p!=(unsigned char)sep) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    if((unsigned char)*p==(unsigned char)sep) {
        p++;
    }

    return p;
}

int
confirm_objattr(struct obattr xoa) 
{
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

    return 0;
}

int
confirm_attr(ob *xo)
{
    struct obattr *xoa;
    xoa = &xo->vob;

#if 0
    if(xoa->ato && xoa->afrom) {
        xo->floated = 1;
    }
#endif
#if 1
    if(xoa->hasto && xoa->hasfrom) {
        xo->floated = 1;
    }
#endif

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
    printf("    hatchthick     %d\n", xo.hatchthick);
    printf("    hatchpitch     %d\n", xo.hatchpitch);
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

    printf("%3d:", s->oid);
    W;
    printf("%-*s %6d,%-6d %6dx%-6d %d %6d,%-6d %6d,%-6d %6d,%-6d\n",
        rest, tm,
        s->ox, s->oy, s->wd, s->ht,
        s->fixed, s->gx, s->gy, s->gsx, s->gsy, s->gex, s->gey);

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
"oid:type                         ox,oy          wxh      f     gx,y        gsx,sy         gex,ey\n");


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

int
ss_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    sstr *sv;

    sv = (sstr*)xv;
    ik = sprintf(dst, "<'%s' %x>", sv->ssval, sv->ssopt);

    return ik;
}

int
segop_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    segop *sv;

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

    s = (seg*)xv;
    ik = sprintf(dst, "(%s(%d),%s(%d),%d,%d;%d,%d,%d,%d;%d,%d)",
        rassoc(objattr_ial, s->ptype), s->ptype,
        s->jtype>0 ? "JOIN" : "-",
        s->jtype,
        s->coordtype, s->ftflag,
        s->x1, s->y1, s->x2, s->y2,
        s->rad, s->ang);

    return 0;
}

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


#include "chs.c"
#include "nss.c"
#include "chas.c"

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

#define _ns_find_obj(a1,a2) _ns_find_objP(a1,a2,NULL,NULL)

#if 0
ob*
_ns_find_obj(ns* xns, char *xname)
#endif
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
revch(ob *x, ch* ref, int *rx, int *ry)
{
    ch  *qch;
    int  r;
    ch  *c;
    int  i;
    varray_t *chpath;
    ob  *u;
    int  mx, my;

    Echo("%s: x %p ref %p START\n", __func__, x, ref);
    if(x) {
        Echo("  x->oid %d\n", x->oid);
    }

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

        Echo("chpath\n");
        for(i=0;i<chpath->use;i++) {
            c = (ch*)chpath->slot[i];
            Echo("  %2d: %p qob %p\n", i, c, c->qob);

            u = c->qob;
            if(u) {
                Echo("    u %p; oid %d ox,oy %d,%d x,y %d,%d\n",
                    u, u->oid, u->cox, u->coy, u->cx, u->cy);
                mx += u->cox + u->cx;
                my += u->coy + u->cy;
            }

            Echo("    mx,my %d,%d\n", mx, my);

            if(c==ref) {
                r = 1;
                qch = c;
                break;
            }
        }

    }
    
out:
    if(r==1) {
        *rx = mx;
        *ry = my;
    }
    if(x) {
        Echo("%s: x %p oid %d; r %d rx,ry %d,%d\n",
            __func__, x, x->oid, r, *rx, *ry);
    }
    else {
        Echo("%s: x %p; r %d rx,ry %d,%d\n", __func__, x, r, *rx, *ry);
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

    ux = uy = 0;

    u = _ns_find_objP(xns, yname, &ux, &uy);
    if(u) {
#if 1
        Echo("  u %p gx,gy = %d,%d solved? %d drawed? %d finalized? %d\n",
            u, u->gx, u->gy, u->sizesolved, u->drawed, u->finalized);
        Echo("  u %p ux,uy = %d, %d\n", u, ux, uy);
#endif

#if 1
        {
            int ik;
            ik = revch(u, b->behas, &ux, &uy);
        }
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
        Echo("     guess x,y = %d,%d\n", ux, uy);
#endif
        *rx = ux;
        *ry = uy;

        switch(pos) {
        case PO_CENTER:    *rx += 0;          *ry += 0;           break;
        case PO_NORTH:     *rx += 0;          *ry += u->cht/2;  break;
        case PO_NORTHEAST: *rx += u->cwd/2;   *ry += u->cht/2;  break;
        case PO_EAST:      *rx += u->cwd/2;   *ry += 0;           break;
        case PO_SOUTHEAST: *rx += u->cwd/2;   *ry += -u->cht/2;  break;
        case PO_SOUTH:     *rx += 0;          *ry += -u->cht/2;  break;
        case PO_SOUTHWEST: *rx += -u->cwd/2;  *ry += -u->cht/2;  break;
        case PO_WEST:      *rx += -u->cwd/2;  *ry += 0;           break;
        case PO_NORTHWEST: *rx += -u->cwd/2;  *ry += u->cht/2;  break;
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

#include "notefile.c"


#include "parse.c"
#ifdef DO_FORKCHK
#include "forkchk.c"
#endif

#ifdef DO_LINKCHK
#include "linkchk.c"
#endif

#include "picdraw.c"
#if 0
#include "epsdraw.c"
#endif

int     nodraw = 0;
char   *outfile = "out.eps";
int     canvaswd = (int)( 8.27 * 72);
int     canvasht = (int)(11.69 * 72);
int     canvasrt = 0;
double  canvassc = 0.01;

int
print_usage()
{
    printf("tangy - picture generator according to special language\n");
    printf("usage: tangy [option]\n");
    printf("option:     ### bp is bigpoint = 1/72inch (c.f. 1cm=28.34bp)\n");
    printf("    -h      print this message\n");
    printf("    -m      print hints for language\n");
    printf("    -V      print version\n");
    printf("    -P      print parameters\n");
    printf("    -o file set output file (current '%s')\n",
            outfile);
    printf("    -u num  set unit by 0.01bp (current %d)\n",
            objunit);
    printf("    -M num  set EPS outside margin by bp (current %d)\n",
            epsoutmargin);
    printf("    -g      draw grid\n");
    printf("    -G num  set grid pitch in bp (current %d)\n", 
            def_gridpitch);
    printf("    -R num  set grid range; how many times of pitch (current %d)\n",
        def_gridrange);
    printf("    -b      draw boundingbox of objects\n");
    printf("    -i      draw object IDs\n");
    printf("    -l      print object list for debug\n");
    printf("    -c      print color list for debug\n");
    printf("    -F font set default font (current '%s')\n", def_fontname);
    printf("following itmes are reserved for future. do not use.\n");
    printf("    -v      verbose mode (current %d)\n", _t_);
    printf("    -q      quiet mode\n");
    printf("    -p      passing trace mode (current %d)\n", _p_);
    printf("   *-d      draft mode\n");
    printf("   *-L      draw labels\n");
    printf("   *-D      debug mode\n");
    printf("   *-r      draw ruler\n");
    printf("   *-s num  set scale\n");
    printf("   *-n      no draw\n");
    printf("   *-N file set notefile\n");

    return 0;
}


int
_print_alistmemkeys(char *pre, int ind, int wmax, apair_t *ap)
{
    int i;
    int j;
    apair_t *cp;
    int w;
    int l;

    printf("%s\n", pre);

    cp = ap;
    i  = 0;
    w  = ind;
            for(j=0;j<ind;j++) printf(" ");
    while(1) {
        if(!cp->name) {
            break;
        }
        l = strlen(cp->name)+1;

        if(w+l<wmax) {
            printf("%s ", cp->name);
            w += l;
        }
        else {
            printf("\n");
            for(j=0;j<ind;j++) printf(" ");
            printf("%s ", cp->name);
            w = ind + l;
        }
        
        cp++;
        i++;
    }


    printf("\n");
    
    return 0;
}

int
print_alistmemkeys(char *pre, apair_t *ap)
{
    return _print_alistmemkeys(pre, 4, 72, ap);
}

int
print_hints()
{
    printf("You may find your forget keyword...\n");
    print_alistmemkeys("object/command names:", cmd_ial);
    print_alistmemkeys("object attributes:", objattr_ial);
    print_alistmemkeys("line type:", linetype_ial);
    print_alistmemkeys("arrowhead type:", arrowhead_ial);
    print_alistmemkeys("hatching type:", hatchtype_ial);
    print_alistmemkeys("font type:", ff_ial);
    print_alistmemkeys("font size:", fh_ial);

    return 0;
}

int
print_version()
{
    printf("tangy version 2.031"
#ifdef GITCHASH
    " " GITCHASH
#endif
    "\n");
    return 0;
}

int
print_param()
{
    printf("tangy parameters:\n");

    printf("    %-24s %7s %7s\n", "name", "value", "factor");
    printf("    ---\n");

#define QPV(x)  \
    printf("    %-24s %7d %7.3f\n", #x, def_ ## x, x ## factor);
#define QP(x)  \
    printf("    %-24s %7s %7.3f\n", #x, "", x ## factor);

    printf("  unit-base: multiply by unitsize %d\n", objunit);

    QPV(arrowsize);
    QPV(warrowsize);
    QPV(linethick);
    QPV(linedecothick);
    QPV(linedecopitch);
    QPV(wlinethick);
    QPV(barrowgap);
    QPV(textheight);
    QPV(hatchthick);
    QPV(hatchpitch);
    QPV(marknoderad);
    QPV(markbbthick);
    QPV(noteosep);
    QPV(noteisep);

    printf("  text-base: multiply by text height (dynamic; normal %d)\n",
        def_textheight);

    QP(textdecent);
    QP(textbgmargin);

#undef QPV
#undef QP

    return 0;
}

int
test0()
{
    int i;

    i = 0;
    while(cmd_ial[i].name) {
        printf("%-16s ", cmd_ial[i].name);
        if(ISCHUNK(cmd_ial[i].value)) {
            printf("CHUNK ");
        }
        else {
            printf("-     ");
        }
        if(ISATOM(cmd_ial[i].value)) {
            printf("ATOM ");
        }
        else {
            printf("-    ");
        }
        if(ISGLUE(cmd_ial[i].value)) {
            printf("GLUE ");
        }
        else {
            printf("-    ");
        }
        if(HASBODY(cmd_ial[i].value)) {
            printf("BODY ");
        }
        else {
            printf("-    ");
        }
        if(MAYEXPAND(cmd_ial[i].value)) {
            printf("EXPAND ");
        }
        else {
            printf("-      ");
        }

            printf("\n");

        i++;
    }
    return 0;
}

int
main(int argc, char *argv[])
{
    ob*   ch0;
    ns*   ns0;
    int   flag;
    int   ik;
    int   do_colorlist, do_objlist;
    FILE *fp;
    int x, y;

    do_objlist = 0;
    do_colorlist = 0;
    
    pallet = new_default_pallet();

    while((flag=getopt(argc, argv,
            "0hmVPvqpngbSdiLrtDo:u:G:R:M:F:lcs:"))!=EOF) {
        switch(flag) {
        case '0':
            test0();
            exit(0);
        case 'h':
            print_usage();
            exit(0);
            break;
        case 'm':
            print_hints();
            exit(0);
            break;
        case 'V':
            print_version();
            exit(0);
            break;
        case 'P':
            recalcsizeparam(); /* XXX */
            print_param();
            exit(0);
            break;
        case 'v':
            _t_ ++;
            break;
        case 'q':
            _t_ = 0;
            break;
        case 'p':
            _p_ = 1 - _p_;
            break;
        case 'n':
            nodraw = 1;
            break;
        case 'g':
            grid_mode = 1;
            break;
        case 'b':
            bbox_mode = 1;
            break;
        case 'i':
            oidl_mode = 1;
            break;
        case 'L':
            namel_mode = 1;
            break;
        case 'S':
            skelton_mode = 1;
            break;
        case 'd':
            draft_mode = 1;
            break;
        case 'r':
            ruler_mode = 1;
            break;
        case 't':
            text_mode = 1;
            break;
        case 'D':
            grid_mode = 1;
            bbox_mode = 1;
            draft_mode = 1;
            oidl_mode = 1;
            ruler_mode = 1;
            text_mode = 1;
            break;
            
        case 'o':
            outfile = strdup(optarg);
            break;
        case 'u':
            objunit = atoi(optarg);
            break;
        case 'G':
            def_gridpitch = atoi(optarg);
            break;
        case 'R':
            def_gridrange = atoi(optarg);
            break;
        case 'M':
            epsoutmargin = atoi(optarg);
            break;

        case 'F':
            def_fontname = strdup(optarg);
            break;

#if 0
        case 'M':
            if(strcasecmp(optarg, "a4")==0) {
                /* inch x bp/inch */
                canvaswd = (int)( 8.27 * 72);
                canvasht = (int)(11.69 * 72);
            }
            break;
#endif
        case 'l':
            do_objlist++;
            break;
        case 'c':
            do_colorlist++;
            break;
        case 's':
            canvassc *= atof(optarg);
            break;
        }
    }

#if 0
    if(do_rotate) {
        int x;
        x = canvasht;
        canvasht = canvaswd;
        canvaswd = x;
        canvasrt = 90;
    }
#endif

    notefile_setup();

    ch0   = newchunk();
    ns0   = newnamespace();
    if(ch0) strcpy(ch0->cch.name, "*root*");
    if(ns0) strcpy(ns0->name, "*root*");

    recalcsizeparam();

    ik = parse(stdin, ch0, ns0);
    if(INTRACE) {
        ob_cndump(ch0);
    }

    if(INTRACE) {
        varray_fprint(stdout, gnotefilelist);
        varray_fprint(stdout, gnotebindlist);
    }

    notefile_load();

#if 0
Echo("ch0 oid %d LANE? %d\n", ch0->oid, ch0->cch.lanenum);
#endif
Echo("ch0 oid %d LANE? %d\n", ch0->oid, ch0->cob.lanenum);

#ifdef DO_FORKCHK
    ik = fkchk(ch0, ns0);
P;
#endif

    x = 0;
    y = 0;
P;
    ik = put(ch0, &x, &y, ns0);
    if(INTRACE) {
        ob_gdump(ch0);
    }

#ifdef DO_LINKCHK
P;
    ik = linkchk(ch0, ns0);
    if(INTRACE) {
        ob_gdump(ch0);
    }
#endif

P;
    finalize(ch0, 0, 0, ns0);

    if(INTRACE) {
        ob_adump(ch0);
        ob_bgdump(ch0);
        ob_bldump(ch0);
    }

#if 0
    picdraw(ch0, 0, 0, ns0);
    fflush(stdout);
#endif

#if 0
    printf("ht root-chunk %d vs canvas %d\n", ch0->ht, canvasht);
#endif

    if(ch0->ht<=0 || ch0->wd<=0) {
        Error("figure is empty or negative size\n");
        exit(9);
    }

#if 0
    canvassc = canvasht/ch0->ht;
    canvassc = 0.01;
#endif

P;
    fp = fopen(outfile, "w");
    if(!fp) {
        Error("cannot open output file '%s'\n", outfile);
        goto skip_output;
    }

    epsdraw(fp, canvaswd, canvasht, canvasrt, canvassc,
        ch0, 0, 0, ns0);
    fclose(fp);
    fp = NULL;

skip_output:
    (void) 0;

    if(do_objlist) {
        printobjlist(stdout, "  ", ch0);
    }
    if(do_colorlist) {
        varray_fprint(stdout, pallet);
    }

    exit(0);
}


