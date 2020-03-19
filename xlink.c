int xlink_trace = 0;

#define XEcho       if(xlink_trace)printf
#define Trace       if(xlink_trace)printf
#define XINTRACE    (xlink_trace>0)

#define XMAP_CHILDS (".")
#define XMAP_RNGS   ("-")
#define XMAP_SEQS   (",")
#define XMAP_SEPS   (";")
#define XMAP_SDS    (":")

#define XMAP_CHILD  ('.')
#define XMAP_RNG    ('-')
#define XMAP_SEQ    (',')
#define XMAP_SEP    (';')
#define XMAP_SD     (':')

typedef struct _mob mob;

struct _mob {
    ob* body;
    int sn;
    int si;
    int sc;
    int dn;
    int di;
    int dc;
};

mob*
mob_new()
{
    mob *rv;
    
    rv = (mob*)malloc(sizeof(mob));
    if(!rv) {
        printf("no memory\n");
        return NULL;
    }
    memset(rv, 0, sizeof(mob));
    rv->sn =  0;
    rv->si = -1;
    rv->sc =  0;
    rv->dn =  0;
    rv->di = -1;
    rv->dc =  0;

    return rv;
}

mob*
mob_new_wob(ob* xx)
{
    mob *rv;
    rv = mob_new();
    if(rv) {
        rv->body = xx;
    }
    return rv;
}

int
mob_sprintf(char* dst, int dlen, void* xv)
{
    int   ik;
    mob*  qmob;
    ob*   qob;

    qmob = (mob*)xv;
    qob = qmob->body;
    if(dlen>20) {
        ik = sprintf(dst, "%3d %3d/%-3d %3d/%-3d",
            qob->oid, qmob->si, qmob->sn, qmob->di, qmob->dn);
    }
    else {
        return -1;
    }

    return 0;
}

int
QLcount(char *src, int sep)
{
    register char *p;
    register int c;
    c = 0;
    p = src;
    if(*p && *p!=(char)sep) {
        c++;
    }
    while(*p) {
        if(*p==(char)sep) {
            if(*(p+1)) {
                c++;
            }
        }
        p++;
    }
#if 0
    Trace("QLcount: |%s| '%c' -> %d\n", src, sep, c);
#endif
    return c;
}

int
QLfind(char *src, char *name, int sep)
{
    char  token[BUFSIZ];
    char *p;

    p = src;
    while(p = draw_word(p, token, BUFSIZ, sep)) {
        if(!token[0]) {
            break;
        }
        if(strcmp(token, name)==0) {
            return 1;
        }
    }

    return 0;
}



char*
QLfindpos(char *src, char *name, int sep)
{
    char  token[BUFSIZ];
    char *p;
    char *q;

    p = src;
    q = NULL;
    while(1) {
        q = p;
        p = draw_word(p, token, BUFSIZ, sep);
        if(!token[0]) {
            break;
        }
        if(strcmp(token, name)==0) {
            return q;
        }
    }

    return NULL; 
}


int
QLadd(char *dst, int dlen, char *xit, int sep)
{
    char *p;
    int   c;
    int   w;
    char *s;
    char *r;

    w = strlen(xit);
    p = dst;
    r = NULL;
    c = 0;
    while(*p) {
        r = p;
        p++;
        c++;
    }
    if(c>=dlen) {
        return 1;
    }
    if(c+w+1>=dlen) {
        return 2;
    }

    if(c>0) {
        if(r && *r==(char)sep) {
#if 0
            Trace("last of list already setted sep, skip\n");
#endif
        }
        else {
#if 0
            Trace("set sep in last of list\n");
#endif
            *p++ = (char)sep;
        }
    }

    s = xit;
    while(*s) {
        *p++ = *s++;
    }
    *p++ = (char)sep;
    *p = '\0';

    return 0;
}

int
QLuniqadd(char *dst, int dlen, char *xit, int sep)
{
    char *p;
    int   c;
    int   w;
    char *s;
    char *r;

    if(QLfind(dst, xit, sep)==1) {
        return 0;
    }

    return QLadd(dst, dlen, xit, sep);
}



static int
cmpfield(const void *x, const void *y)
{
    char *xp = *(char**)x;
    char *yp = *(char**)y;
#if 0
    printf("%s vs %s\n", *(char**)x, *(char**)y);
#endif
    return strcmp(xp,yp);
}

static int
cmpfieldN(const void *x, const void *y)
{
    char *xp = *(char**)x;
    char *yp = *(char**)y;
    int  xv, yv;
#if 0
    printf("%s vs %s\n", *(char**)x, *(char**)y);
#endif
    xv = atoi(xp);
    yv = atoi(yp);
    return xv-yv;
}



int
_QLsortuniq(char *src, int sep, int numeric)
{
    char    token[BUFSIZ];
    char   *p;
    char   *u;
    int     n;
    int     w,ow;
    int     i;
    char   *dst=NULL;
    char  **s=NULL;
    int     ck;

#if 0
    trace_sman = 1;
#endif

#if 0
    Trace("sortuniq src %p sep '%c' numeric? %d\n", src, sep, numeric);
#endif

    if(src==NULL) {
        return -1;
    }

    w = strlen(src);
#if 0
    Trace("w %d\n", w);
#endif

    if(w<=0) {
        goto out;
    }

    ow = w+2;   /* sometimes dst requires additional area for separators */
#if 0
    Trace("ow %d\n", ow);
#endif

#if 1
    dst = (char*)malloc(sizeof(char*)*ow);
    if(dst==NULL) {
        Error("_QLsortuniq: no memory");
        goto nomemory;
    }
#endif
#if 0
    dst = strdup(src);
    if(dst==NULL) {
        Error("_QLsortuniq: no memory");
        goto nomemory;
    }
#endif

#if 1
    /* trim last sep, empty field */
    while(src[w-1]==sep) {
#if 0
        Trace("lastchar '%c'\n", src[w-1]);
#endif
        src[w-1]='\0';
        w--;
    }
#endif

    n = QLcount(src, sep);
#if 0
    Trace("n %d\n", n);
#endif
    if(n<=1) {
        goto out;
    }


    s = (char**)malloc(sizeof(char*)*n);
    if(s==NULL) {
        Error("_QLsortuniq: no memory");
        goto nomemory;
    }
#if 1
    memset(s, 0, sizeof(char*)*n);
#endif


    p = src;
    i = 0;
    while(u = p, p = draw_word(p, token, BUFSIZ, sep)) {
        if(!token[0]) {
#if 1
            if(i<n) {
                Trace("trim n as %d\n", i);
                n = i;
            }
#endif
            if(*p=='\0') {
                break;
            }
            else {
                /* skip empty field */
                continue;
            }
        }
        s[i] = u;
        if(u!=src) {
            *(u-1) = '\0';
        }
        i++;
    }

    /* */
#if 0
    for(i=0;i<n;i++) {
        Trace("u %3d: %s\n", i, s[i]);
    }
#endif
#if 0
    for(i=MAX(0,n-3);i<n;i++) {
        Trace("u %3d: %s\n", i, s[i]);
    }
#endif

    if(numeric) {
#if 0
        Trace("numeric\n");
#endif
        qsort(s,n,sizeof(char*),cmpfieldN);
    }
    else {
#if 0
        Trace("no-numeric\n");
#endif
        qsort(s,n,sizeof(char*),cmpfield);
    }

    /* */
#if 0
    for(i=0;i<n;i++) {
        Trace("v %3d: %s\n", i, s[i]);
    }
#endif
#if 0
    for(i=MAX(0,n-3);i<n;i++) {
        Trace("v %3d: %s\n", i, s[i]);
    }
#endif
    dst[0] = '\0';
    for(i=0;i<n;i++) {
        ck = QLuniqadd(dst, ow, s[i], sep);
#if 0
        if(i>=n-3) {
            Trace("dst '%s' (%d)\n", dst, ck);
        }
#endif
    }
    strcpy(src, dst);

    goto fine;

nomemory:

fine:
    if(dst) free(dst);
    if(s) free(s);

out:
    return 0;
}


int
expand_tree(char *dst, int dlen, vdict_t *dict, char *prefix, ob *xta)
{
    ob   *pe;
    int   ik;
    int   i;
    char  tmp[BUFSIZ];
    int   u;

#if 0
    XEcho("%s: dst %p dlen %d prefix '%s' ob %p %s(%d)\n",
        __func__, dst, dlen, prefix, (char*)xta,
        rassoc(cmd_ial, xta->type), xta->type);
#endif

    if(xta && ISCHUNK(xta->type)) {
#if 0
        XEcho("  chunk   oid %d\n", xta->oid);
#endif
        u = 0;
        for(i=0;i<xta->cch.nch;i++) {
            pe = (ob*)xta->cch.ch[i];
#if 0
        XEcho("          oid %d %s(%d)\n",
            pe->oid, rassoc(cmd_ial, pe->type), pe->type);
#endif
            if(!EXVISIBLE(pe->type) && !ISCHUNK(pe->type)) {
                continue;
            }
            u++;
            sprintf(tmp, "%s%d", prefix, u);

            if(ISCHUNK(pe->type)) {
                strcat(tmp, XMAP_CHILDS);
                ik = expand_tree(dst, dlen, dict, tmp, pe);
            }
            else {
                mob* ppe;
#if 0
XEcho("call add '%s'\n", tmp);
#endif
#if 0
                vdict_add(dict, tmp, pe);
#endif
                ppe = mob_new_wob(pe);
                if(ppe) {
                    vdict_add(dict, tmp, ppe);
                }
#if 0
XEcho("  use/len %d/%d\n", dict->use, dict->len);
fflush(stdout);
#endif

                strcat(tmp, XMAP_SEPS);
                strcat(dst, tmp);
                
            }
        }
    }
    else {
#if 0
        XEcho("    elem  oid %d\n", xta->oid);
#endif
        if(EXVISIBLE(xta->type)||ISCHUNK(xta->type)) {
#if 0
            XEcho("       O  oid %d\n", xta->oid);
#endif
            sprintf(tmp, "%s0", prefix);
            strcat(dst, tmp);
        }
        else {
        }
    }

    return 0;
}


int
expand_full(char *dst, int dlen, char *bseq, char *fseq)
{
    char *p, *q;
    int   i, j;
    char  bn[BUFSIZ], fn[BUFSIZ];
    char  fblk[BUFSIZ];
    char  tmp[BUFSIZ];

    XEcho("%s: '%s' x '%s'\n", __func__, bseq, fseq);

    p = bseq;
    i = 0;
    while(p) {
        p = draw_word(p, bn, BUFSIZ, XMAP_SEP);
        if(!bn[0]) {
            break;
        }
        i++;

        fblk[0] = '\0';
        
        q = fseq;
        j = 0;
        while(q) {
            q = draw_word(q, fn, BUFSIZ, XMAP_SEP);
            if(!fn[0]) {
                break;
            }
            j++;

            strcat(fblk, fn);
            strcat(fblk, XMAP_SEQS);
            
            XEcho("        %d,%d %s %s\n", i, j, bn, fn);
        }
            sprintf(tmp, "%s:%s", bn, fblk);
            strcat(dst, tmp);
            strcat(dst, XMAP_SEPS);
    }

    XEcho("        %s: dst |%s|\n", __func__, dst);

    return 0;
}

int
parse_range(char *src, int *rb, int *re)
{
    char *p, *q;
    int   i;
    char  bb[BUFSIZ];
    char  eb[BUFSIZ];

    *rb = *re = -1;

    p = src;

    q = bb;
    i = 0;
    while(*p && i<BUFSIZ) {
        if(*p>='0' && *p<='9') {
            *q++ = *p++;
        }
        else {
            break;
        }
    }

    *rb = atoi(bb);

    if(!*p) {
        *re = *rb;
        goto out;
    }

    if(*p!=XMAP_RNG) {
        XEcho("ignore char, RNG expected place\n");
        return -1;
    }

    p++; /* skip XMAP_RNG */

    q = eb;
    i = 0;
    while(*p && i<BUFSIZ) {
        if(*p>='0' && *p<='9') {
            *q++ = *p++;
        }
        else {
            break;
        }
    }

    *re = atoi(eb);

out:
    XEcho("%s: rb %d re %d\n", __func__, *rb, *re);

    return 0;
}

int
expand_bfpat(char *dst, int dlen, vdict_t *bdict, vdict_t *fdict,
    char *xpat)
{
    char *p;
    char  base;
    char  token[BUFSIZ];
    char  list[BUFSIZ];
    char  tmp[BUFSIZ];
    int   rb, re;
    int   ik;
    int   e;

    vdict_t    *d;
    vdict_cell *c;
    int i;

    XEcho("%s: xpat |%s|\n", __func__, xpat);

    p = xpat;
    while(p && ((*p>='A' && *p<='Z') || (*p>='a' && *p<='z'))) {
        base = *p;
        p++;
    }

    XEcho("  base %c\n", base);

    while(p) {
        p = draw_word(p, token, BUFSIZ, XMAP_SEQ);
        if(!token[0]) {
            break;
        }
        if(token[0]=='*') {
            goto all;
        }
        XEcho("    token |%s|\n", token);
        if(index(token, XMAP_RNG)) {
            ik = parse_range(token, &rb, &re);
            for(e=rb; e<=re; e++) {
                sprintf(tmp, "%c%d", base, e);
                strcat(dst, tmp);
                strcat(dst, XMAP_SEPS);
            }
        }
        else {
            sprintf(tmp, "%c%s", base, token);
            strcat(dst, tmp);
            strcat(dst, XMAP_SEPS);
        }
    }
    goto out;

all:

    if(base=='f') { d = fdict; }
    else
    if(base=='b') { d = bdict; }
    else {
        goto out;
    }
        
    vdict_fshow(d, stdout);
    for(i=0;i<d->len;i++) {
        c = &d->slot[i];
        if(c->key && c->key[0]) {
            strcat(dst, c->key);
            strcat(dst, XMAP_SEPS);
        }
    }
#if 0
fprintf(stderr, "dst 0 |%s|\n", dst);
#endif
    _QLsortuniq(dst, XMAP_SEP, 0);
#if 0
fprintf(stderr, "dst 1 |%s|\n", dst);
#endif
#if 0
    _QLsortuniq(dst, XMAP_SEP, 1);
fprintf(stderr, "dst 2 |%s|\n", dst);
#endif

    goto out;

out:

    fflush(stdout);
    XEcho("     %s: dst '%s'\n", __func__, dst);

    return 0;
}
    

int
expand_sdpat(char *dst, int dlen, vdict_t *bdict, vdict_t *fdict,
    char *xpatlist)
{
    char *p, *q;
    char  epat[BUFSIZ];
    char *u;
    char  spat[BUFSIZ];
    char  dpat[BUFSIZ];
    char  sseq[BUFSIZ];
    char  dseq[BUFSIZ];
    int   i;
    int   ik;
    int   xls;

    XEcho("%s: xpatlist |%s|\n", __func__, xpatlist);

    p = xpatlist;
    i = 0;
    while(p) {
        p = draw_word(p, epat, BUFSIZ, XMAP_SEP);
        if(!epat[0]) break;

        xls = assoc(ls_ial, epat);
        if(xls>=0) {
            XEcho("  xpatlist '%s' -> xls %d\n", epat, xls);
            strcat(dst, epat);
            strcat(dst, XMAP_SEPS);
            continue;
        }

        i++;
        XEcho("  %d: epat |%s|\n", i, epat);    
            u = epat;
            u = draw_word(u, spat, BUFSIZ, XMAP_SD);
        XEcho("      spat |%s|\n", spat);   
            u = draw_word(u, dpat, BUFSIZ, XMAP_SD);
        XEcho("      dpat |%s|\n", dpat);   

        memset(sseq, 0x55, sizeof(sseq));
        sseq[0] = '\0';
        expand_bfpat(sseq, BUFSIZ, bdict, fdict, spat);
        XEcho("      sseq |%s|\n", sseq);

        memset(dseq, 0xaa, sizeof(dseq));
        dseq[0] = '\0';
        expand_bfpat(dseq, BUFSIZ, bdict, fdict, dpat);
        XEcho("      dseq |%s|\n", dseq);

        ik = expand_full(dst, dlen, sseq, dseq);
    }

    XEcho("sdpat xpatlist |%s|\n  dst |%s|\n", xpatlist, dst);

    return 0;
}



/*
 * basically this code expect gather; multi sources and single destination.
 */
static
int
_drawXlink(varray_t *qar, int xid, int style, int jr,
    int j, int n, int h1,
    int sx, int sy, int ssy, int maxsx,
    int mx, int my,
    int minex, int ex, int ey, int eey, int dsdir)
{

    /*
     *
     *      maxsx,sy 
     * sx,sy  +..+........
     * sx,ssy +--+-----+ .
     *            h1   | . 
     *                 | . minex,sy
     *    maxsx+h1,eey +--------+--+ ex,eey
     *                   +......+..+ ex,ey
     *                mx,my 
     *
     * + basically, (sx,sy)-(mx,sy)-(mx,ey)-(ex,ey) is drown.
     * + according to h1, line is shift.
     */ 

    int rstyle;
    int focus;
    int join;

    rstyle = style & LS_M_TYPE;
    focus  = style & LS_FOCUS;
    join   = style & LS_JOIN;
#if 1
    join   = focus;
#endif


    Echo("%s: ar %p xid %d\n", __func__, qar, xid);

    Echo(
        "%s: xid %d style %3d %3xH rstyle %3d %3xH focus %d join %d j/n %d/%d\n",
        __func__, xid, style, style, rstyle, rstyle, focus, join, j, n);

#if 0
    if(dsdir>0) {
        path_regsegdir(qar, 180);
    }
    if(dsdir<0) {
        path_regsegdir(qar, -180);
    }
#endif

    switch(rstyle) {
    case LS_STRAIGHT:
        if(dsdir>=0) {
            mkpath_1seg(qar, sx, sy, ex, sy);
        }
        else {
            mkpath_1seg(qar, ex, sy, sx, sy);
        }
        break;

    case LS_SQUARE:
XEcho("j/n %d/%d\n", j, n);
XEcho("h1 %d\n", h1);
        /* SQUARE do not care focus ; always separated */
        {
            if(dsdir>=0) {
                mkpath_3seg(qar, sx, ssy, maxsx+h1, ssy, maxsx+h1, eey, ex, eey);
            }
            else {
                mkpath_3seg(qar, ex, eey, maxsx+h1, eey, maxsx+h1, sy, sx, sy);
            }
        }
        break;

    case LS_COMB:
        /* COMB do not care focus ; always concentrated */
        {
            if(dsdir>=0) {
fprintf(stderr, " comb pos\n");
                mkpath_3seg(qar, sx, sy, mx, sy, mx, eey, ex, eey);
fprintf(stderr, "j/n %d/%d\n", j, n);
                if(join && ( j>0 && j<n-1) ) {
fprintf(stderr, "   draw\n");
                    mkpath_addbwcir(qar, mx, eey);
                    mkpath_addbwcir(qar, mx, sy);
                }
            }
            else {
fprintf(stderr, " comb neg\n");
                mkpath_3seg(qar, ex, eey, mx, eey, mx, sy, sx, sy);
                if(join && j!=0 && j!=n-1) {
                    mkpath_addbwcir(qar, mx, sy);
                    mkpath_addbwcir(qar, mx, ey);
                }
            }
        }
        break;

    case LS_CURVE:
        if(focus) {
            if(dsdir>=0) {
                mkpath_segarcseg2(qar, sx, sy, maxsx, sy,
                   mx, eey, ex, eey);
            }
            else {
                mkpath_segarcseg2(qar, ex, ey, mx, ey,
                   maxsx, sy, sx, sy);
            }
#if 0
            if(join) { mkpath_addbwcir(qar, mx, my); }
#endif
        }
        else {
            if(dsdir>=0) {
                mkpath_segarcseg2(qar, sx, ssy, maxsx, ssy, 
                   mx, eey, ex, eey);
            }
            else {
                mkpath_segarcseg2(qar, ex, eey, mx, eey,
                   maxsx, ssy, sx, ssy);
            }
        }
        break;

    case LS_ARC:
        if(focus) {
            if(dsdir>=0) {
                mkpath_segarcseg(qar, sx, sy, maxsx, sy, mx, sy,
                   mx, ey, ex, ey);
            }
            else {
                mkpath_segarcseg(qar, ex, ey, mx, ey, mx, sy,
                   maxsx, sy, sx, sy);
            }
            if(join) { mkpath_addbwcir(qar, mx, my); }
        }
        else {
            if(dsdir>=0) {
                mkpath_segarcseg(qar, sx, sy, maxsx, sy, mx, sy,
                   mx, eey, ex, eey);
            }
            else {
                mkpath_segarcseg(qar, ex, eey, mx, eey, mx, sy,
                   maxsx, sy, sx, sy);
            }
        }
        break;

    default:
        fprintf(stderr, "ignore style <%d;%xH>\n", rstyle, rstyle);

    case LS_NONE:
    case LS_DIRECT:
        if(focus) {
            if(dsdir>=0) {
                mkpath_3seg(qar, sx, sy, maxsx, sy, mx, eey, ex, eey);
            }
            else {
                mkpath_3seg(qar, ex, ey, mx, ey, maxsx, sy, sx, sy);
            }
#if 0
            if(join) { mkpath_addbwcir(qar, mx, my); }
#endif
        }
        else {
#if 0
            mkpath_3seg(qar, sx, ssy, maxsx, ssy, mx, eey, ex, eey);
#endif
            if(dsdir>=0) {
                mkpath_3seg(qar, sx, ssy, maxsx, ssy, mx, eey, ex, eey);
            }
            else {
                mkpath_3seg(qar, ex, eey, mx, eey, maxsx, ssy, sx, ssy);
            }
        }
        break;
    }

out:
    return 0;
}

int
epsdraw_xlink(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns)
{
    ob *pf, *pb;
    ob *fs[FBSLOT_LEN];
    ob *bs[FBSLOT_LEN];
    ob *pe;
    int ik;
    int i, j;
    int am;
    int cm;

    int bxmax, fxmin;

    char btree[BUFSIZ];
    char ftree[BUFSIZ];
    vdict_t *bdict;
    vdict_t *fdict;
    char rmap[BUFSIZ];
    char cmap[BUFSIZ];

    char *p;
    char  mseg[BUFSIZ];
    char *u;
    char  src[BUFSIZ];
    char  mdst[BUFSIZ];
    char *m;
    char  dst[BUFSIZ];
    char  seg[BUFSIZ];
    vdict_cell *ce;
    mob  *pse;
    ob   *se;
    mob  *pde;
    ob   *de;

    int sx, sy;
    int s0x, s0y, six, siy, scx, scy;
    double sxp;
    int maxsx;

    int mx;
    int num_s;
    int num_d;
    int sd_gap;
    double sd_gappitch;
    int is;

    int dx, dy;
    int d0x, d0y, dix, diy, dcx, dcy;
    double dxp;
    int k;
    int mindx;

    int bstyle;
    int xstyle;
    int cstyle;
    int xls;

    int ind;

    varray_t *tmpar;

    qbb_t *stage;


    am = 4*objunit/10;
    cm = 2*objunit/10;
    bxmax = -(INT_MAX-1);
    fxmin = INT_MAX;

    XEcho("%s: oid %d\n", __func__, xu->oid);
P;
    pf = (ob*)xu->cob.linkfore;
    pb = (ob*)xu->cob.linkback;
    fprintf(fp, "%% pf %p, pb %p\n", pf, pb);
    XEcho("%% pf %p, pb %p\n", pf, pb);

    if(!pf || !pb) {
        goto out;
    }
    XEcho("pf %p oid %d %d\n", pf, pf->oid, pf->type);
    XEcho("pb %p oid %d %d\n", pb, pb->oid, pb->type);

    memset(fs, 0, sizeof(fs));
    memset(bs, 0, sizeof(bs));

/***** 
 ***** new code
 *****/


    /*
     * collect the name of previous/success objects
     *      and store to dictionaries.
     */

    bdict = vdict_newclear();
    if(!bdict) {
        Error("nomemory\n");
        goto out;
    }
    vdict_entryprintfunc(bdict, mob_sprintf);

    fdict = vdict_newclear();
    if(!fdict) {
        Error("nomemory\n");
        goto out;
    }
    vdict_entryprintfunc(fdict, mob_sprintf);
    
    btree[0] = '\0';
    ik = expand_tree(btree, BUFSIZ, bdict, "b", pb);
    XEcho("btree |%s|\n", btree);
#if 0
    vdict_fshow(bdict, stdout);
#endif

    ftree[0] = '\0';
    ik = expand_tree(ftree, BUFSIZ, fdict, "f", pf);
    XEcho("ftree |%s|\n", ftree);
#if 0
    vdict_fshow(fdict, stdout);
#endif

 
    rmap[0] = '\0';
    cmap[0] = '\0';

    if(xu->cob.linkmap) {
        strcpy(rmap, xu->cob.linkmap);
    }
    else {
        strcpy(rmap, "full");
    }

    if(strcmp(rmap, "full")==0) {
        int ik;
        ik = expand_full(cmap, BUFSIZ, btree, ftree);
    }
    else {
        int ik;
        ik = expand_sdpat(cmap, BUFSIZ, bdict, fdict, rmap);
    }




    /* count the number of links per object */

    XEcho("dummy si\n");
    XEcho("count links - - -\n");

    num_s = 0;
    num_d = 0;

    bstyle = xu->cob.linkstyle;

    cstyle = bstyle;
    p = cmap;
    while(p) {
        p = draw_word(p, mseg, BUFSIZ, XMAP_SEP);
        if(!mseg[0]) {
            break;
        }
        XEcho("mseg* |%s|\n", mseg);

        xls = assoc(ls_ial, mseg);
        XEcho("  xls %d\n", xls);
        if(xls>=0) {
            xstyle = xls;
            cstyle = xstyle;
            continue;
        }

        u = mseg;
        u = draw_word(u, src, BUFSIZ, XMAP_SD);
        if(!src[0]) {
            break;
        }
        u = draw_word(u, mdst, BUFSIZ, XMAP_SD);
        if(!mdst[0]) {
            break;
        }
        XEcho("mseg  |%s|\n", mseg);
        XEcho(" src  |%s|\n", src);

        ce = vdict_findpos(bdict, src);
        if(!ce) {
            ce = vdict_findpos(fdict, src);
        }
        if(!ce) {
        XEcho("      UNKOWN src\n");
            continue;
        }
        pse = ce->value;
        if(!pse) {
        XEcho("      known but cannot find in dict\n");
            continue;
        }
        se = pse->body;
        if(!se) {
        XEcho("      known but cannot find in dict\n");
            continue;
        }
        XEcho("      oid %d\n", se->oid);

        pse->sn++;


        XEcho(" mdst |%s|\n", mdst);

        m = mdst;
        i = 0;
        while(m) {
            m = draw_word(m, dst, BUFSIZ, XMAP_SEQ);
            if(!dst[0]) {
                break;
            }
            i++;
            
        XEcho("  %3d |%s|\n", i, dst);

            ce = vdict_findpos(bdict, dst);
            if(!ce) {
                ce = vdict_findpos(fdict, dst);
            }
            if(!ce) {
        XEcho("      UNKOWN dst\n");
                continue;
            }
            pde = ce->value;
            if(!pde) {
        XEcho("      known but cannot find in dict\n");
                continue;
            }
            de = pde->body;
            if(!de) {
        XEcho("      known but cannot find in dict\n");
                continue;
            }
        XEcho("      oid %d\n", de->oid);

            pse->sc++;
            pde->dn++;

            num_d++;

        XEcho("              |%s|--|%s|\n", src, dst);
        }

            num_s++;
    }
    

    XEcho("draw links - - -\n");

    stage = qbb_new();
    qbb_mark(stage, pb->grx, pb->gty);
    qbb_mark(stage, pb->grx, pb->gby);
    qbb_mark(stage, pf->glx, pf->gby);
    qbb_mark(stage, pf->glx, pf->gty);

    if(XINTRACE) {
        fprintf(fp, "gsave\n");
        fprintf(fp, "  1 0.7 0.7 setrgbcolor\n");
        fprintf(fp, "  %d %d moveto\n", stage->lx, stage->by);
        fprintf(fp, "  %d %d lineto\n", stage->rx, stage->by);
        fprintf(fp, "  %d %d lineto\n", stage->rx, stage->ty);
        fprintf(fp, "  %d %d lineto\n", stage->lx, stage->ty);
        fprintf(fp, "  closepath\n");
        fprintf(fp, "  stroke\n");
        fprintf(fp, "grestore\n");
    }

    maxsx = pb->grx;
    mindx = pf->glx;
    sd_gap = pf->glx - pb->grx ;
    sd_gappitch = ((double)sd_gap)/(num_s+1);
#if 0
    sd_gappitch = ((double)sd_gap)/(num_s+3);
#endif

    XEcho("num_s %d pb grx %d pf glx %d; sd_gap %d sd_gappitch %f\n",
        num_s, pb->grx, pf->glx,
        sd_gap, sd_gappitch);

    if(XINTRACE) {

        int z;
        int x;
        int y;
        for(z=0;z<=num_s+1;z++) {
            fprintf(fp, "%% z %d\n", z);
            fprintf(fp, "gsave\n");
            fprintf(fp, "  0.8 0.8 0.8 setrgbcolor\n");
            fprintf(fp, "  %d %d %d 0 360 arc\n",
                pb->grx+(int)(sd_gappitch*z), pb->gty, objunit/20);
            fprintf(fp, "  fill\n");
            fprintf(fp, "grestore\n");
        }

    }



    cstyle = bstyle;
    p = cmap;
    is = 0;
    while(p) {
        p = draw_word(p, mseg, BUFSIZ, XMAP_SEP);
        if(!mseg[0]) {
            break;
        }

        xls = assoc(ls_ial, mseg);
        XEcho("  xls %d\n", xls);
        if(xls>=0) {
            xstyle = xls;
            cstyle = xstyle;
            continue;
        }

        tmpar = varray_new();
        varray_entrysprintfunc(tmpar, seg_sprintf);

        u = mseg;
        u = draw_word(u, src, BUFSIZ, XMAP_SD);
        if(!src[0]) {
            break;
        }
        u = draw_word(u, mdst, BUFSIZ, XMAP_SD);
        if(!mdst[0]) {
            break;
        }
        XEcho(" src  |%s|\n", src);

        ce = vdict_findpos(bdict, src);
        if(!ce) {
            ce = vdict_findpos(fdict, src);
        }
        if(!ce) {
        XEcho("      UNKOWN src\n");
            continue;
        }
        pse = ce->value;
        if(!pse) {
        XEcho("      known but cannot find in dict\n");
            continue;
        }
        se = pse->body;
        if(!se) {
        XEcho("      known but cannot find in dict\n");
            continue;
        }
        XEcho("      src oid %d\n", se->oid);

#if 0
XEcho("b si %d\n", pse->si);
        if(pse->si<0) {
            pse->si = 0;
        }
        else {
            pse->si++;
        }
XEcho("a si %d\n", pse->si);
#endif

        mx = pb->grx + (int)((double)(is+1)*sd_gappitch);
#if 0
        mx = pb->grx + (int)((double)(is+2)*sd_gappitch);
#endif
    
        sxp = (((double)se->ty-se->by))/(pse->sc+1);
        s0x = se->grx;
        s0y = se->gty;

        scx = s0x;
        scy = se->gy;

        m = mdst;
        i = 0;
        while(m) {
            m = draw_word(m, dst, BUFSIZ, XMAP_SEQ);
            if(!dst[0]) {
                break;
            }
            i++;
            
            ce = vdict_findpos(bdict, dst);
            if(!ce) {
                ce = vdict_findpos(fdict, dst);
            }
            if(!ce) {
            XEcho("      UNKOWN dst\n");
                continue;
            }
            pde = ce->value;
            if(!pde) {
            XEcho("      known but cannot find in dict\n");
                continue;
            }
            de = pde->body;
            if(!de) {
            XEcho("      known but cannot find in dict\n");
                continue;
            }
            XEcho("      dst oid %d\n", de->oid);


        XEcho("              |%s|--|%s|\n", src, dst);
        XEcho("            n  %3d   %3d\n", pse->sn, pde->dn);
        XEcho("            i  %3d   %3d\n", pse->si, pde->di);
        XEcho("            c  %3d   %3d\n", pse->sc, pde->dc);
            if(pde->di==-1) {
                pde->di = 0;
            }
            else {
                pde->di++;
            }
#if 1
XEcho("b si %d\n", pse->si);
            if(pse->si==-1) {
                pse->si = 0;
                pse->si = 1;
            }
            else {
                pse->si++;
            }
XEcho("a si %d\n", pse->si);
#endif

            fprintf(fp, "%% thru part %d - %d\n", se->oid, de->oid);

            if(XINTRACE) {
                fprintf(fp, "gsave\n");
                fprintf(fp, "  0.01 setlinewidth\n");
                fprintf(fp, "  1 0 0 setrgbcolor\n");
                fprintf(fp, "  newpath\n");
                fprintf(fp, "  %d %d moveto\n", se->gx, se->gy);
                fprintf(fp, "  %d %d lineto\n", de->gx, de->gy);
                fprintf(fp, "  stroke\n");
                fprintf(fp, "grestore\n");
            }

            {

                dxp = (((double)de->ty-de->by))/(pde->dn+1);
                d0x = de->glx;
                d0y = de->gty;

                dcx = d0x;
                dcy = de->gy;

                fprintf(fp, "gsave\n");


                if(XINTRACE) {
                    fprintf(fp, "  0.1 setlinewidth\n");

                    for(k=0;k<pse->sc;k++) {
                        six = s0x;
                        siy = s0y-sxp*(k+1);

                        fprintf(fp, "%% six k %d\n", k);
                        fprintf(fp, "  0.5 0.5 0.5 setrgbcolor\n");
                        fprintf(fp, "  newpath\n");
                        fprintf(fp, "  %d %d %d 0 360 arc\n",
                            six-2*objunit/20, siy, objunit/20);
                        fprintf(fp, "  fill\n");

                    }

                    for(k=0;k<pde->dn;k++) {
                        dix = d0x;
                        diy = d0y-dxp*(k+1);

                        fprintf(fp, "%% dix k %d\n", k);
                        fprintf(fp, "  0.5 0.5 0.5 setrgbcolor\n");
                        fprintf(fp, "  newpath\n");
                        fprintf(fp, "  %d %d %d 0 360 arc\n",
                            dix+2*objunit/20, diy, objunit/20);
                        fprintf(fp, "  fill\n");

                    }

                    fprintf(fp, "  50 setlinewidth\n");
                    fprintf(fp, "  1 0 0 setrgbcolor\n");
                    fprintf(fp, "  %d %d %d 0 360 arc\n",
                        scx-4*objunit/20, scy, objunit/20);
                    fprintf(fp, "  fill\n");

                    fprintf(fp, "  50 setlinewidth\n");
                    fprintf(fp, "  1 0 0 setrgbcolor\n");
                    fprintf(fp, "  %d %d %d 0 360 arc\n",
                        dcx+4*objunit/20, dcy, objunit/20);
                    fprintf(fp, "  fill\n");

                }

                sx = scx;
                sy = scy;

                dx = d0x;
                dy = d0y-dxp*(pde->di+1);

                if(XINTRACE) {
                    fprintf(fp, "  200 setlinewidth\n");
                    fprintf(fp, "  0 1 0 setrgbcolor\n");
                    fprintf(fp, "  %d %d %d 0 360 arc\n",
                        sx, sy, objunit/20);
                    fprintf(fp, "  stroke\n");

                    fprintf(fp, "  200 setlinewidth\n");
                    fprintf(fp, "  0 1 0 setrgbcolor\n");
                    fprintf(fp, "  %d %d %d 0 360 arc\n",
                        dx, dy, objunit/15);
                    fprintf(fp, "  stroke\n");

                    fprintf(fp, "  50 setlinewidth\n");
                    fprintf(fp, "  %d %d moveto\n", sx, sy);
                    fprintf(fp, "  %d %d lineto\n", dx, dy);
                    fprintf(fp, "  stroke\n");
                }


XEcho("is %d si %d di %d\n", is, pse->si, pde->di);
                six = s0x;
                siy = s0y-sxp*(pse->si);

                dix = d0x;
                diy = d0y-dxp*(pde->di+1);

                changecolor(fp, xu->cob.outlinecolor);
                changethick(fp, xu->cob.outlinethick);

                int grd;


                if(scx<dcx) {
                    if(siy>scy) { grd = 1; } else { grd = -1; }
                }
                else {
                    if(siy>scy) { grd = -1; } else { grd = 1; }
                }

                if(grd>0) {
                    ind = (int)((sd_gappitch*pse->si)/pse->sc);
                }
                else {
                    ind = (int)((sd_gappitch*(pse->sc-pse->si))/pse->sc);
                }
XEcho("grd %d; ind %d\n", grd, ind);

                _drawXlink(tmpar, xu->oid, cstyle, xu->cob.outlinethick*2,
                    pse->si-1, pse->sc, ind,
                    scx, scy, siy, maxsx,
                    mx, dcy,
                    mindx, dcx, dcy, diy, dcx-scx);

                if(tmpar->use>0) {
                    /* NOTE offset is cared already. do not apply twice */
                    __drawpath_LT(fp, 0, 0, 0, xu, xns, tmpar);
                }

                fprintf(fp, "grestore\n");
            
            }
        }
        is++;
    }


out:
    return 0;
}
