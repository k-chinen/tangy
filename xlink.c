int xlink_trace = 1;

#define XEcho   if(xlink_trace)printf

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
    ik = sprintf(dst, "%3d %3d/%-3d %3d/%-3d",
        qob->oid, qmob->si, qmob->sn, qmob->di, qmob->dn);

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

#if 0
    dst[0] = '\0';
#endif

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
            
            XEcho(" %d,%d %s %s\n", i, j, bn, fn);
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

    XEcho("%s: xpat |%s|\n", __func__, xpat);

#if 0
    dst[0] = '\0';
#endif

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
 {
    vdict_t    *d;
    vdict_cell *c;
    int i;

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
    fflush(stdout);
 }

    goto out;

out:

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

#if 0
    dst[0] = '\0';
#endif

    p = xpatlist;
    i = 0;
    while(p) {
        p = draw_word(p, epat, BUFSIZ, XMAP_SEP);
        if(!epat[0]) break;

        xls = assoc(ls_ial, epat);
        if(xls>=0) {
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

#if 0
        dst[0] = '\0';
#endif
        ik = expand_full(dst, dlen, sseq, dseq);
    }

    XEcho("sdpat xpatlist |%s|\n  dst |%s|\n", xpatlist, dst);

    return 0;
}

int
epsdraw_xlink(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns)
{
    ob *pf, *pb;
    ob *fs[FBSLOT_LEN];
    ob *bs[FBSLOT_LEN];
    ob *pe, *se;
    int ik;
    int i, j;
    int u, v;
    int am;
    int cm;
    varray_t *tmpar;

    int bxmax, fxmin;

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

    char btree[BUFSIZ];
    char ftree[BUFSIZ];
    vdict_t *bdict;
    vdict_t *fdict;
    char rmap[BUFSIZ];
    char cmap[BUFSIZ];

 {

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
    vdict_fshow(bdict, stdout);

    ftree[0] = '\0';
    ik = expand_tree(ftree, BUFSIZ, fdict, "f", pf);
    XEcho("ftree |%s|\n", ftree);
    vdict_fshow(fdict, stdout);

 }
 
 {
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

 }

 {
    char *p;
    char  mseg[BUFSIZ];
    char *u;
    char  src[BUFSIZ];
    char  mdst[BUFSIZ];
    int   i;
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
#if 0
    int q;
#endif

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

    int bstyle;
    int xstyle;
    int cstyle;
    int xls;

    /* count the number of links per object */

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
        XEcho("mseg  |%s|\n", mseg);

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

        {
            fprintf(fp, "%% PBGRX,GTY\n");
            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d %d 0 360 arc\n",
                pb->grx, pb->gty, objunit/20);
            fprintf(fp, "  fill\n");
            fprintf(fp, "grestore\n");
        }

        {
            fprintf(fp, "%% PFGLX,GBY\n");
            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d %d 0 360 arc\n",
                pf->glx, pf->gby, objunit/20);
            fprintf(fp, "  fill\n");
            fprintf(fp, "grestore\n");
        }

    sd_gap = pf->glx - pb->grx ;
    sd_gappitch = ((double)sd_gap)/(num_s+1);

    XEcho("num_s %d pb grx %d pf glx %d; sd_gap %d sd_gappitch %f\n",
        num_s, pb->grx, pf->glx,
        sd_gap, sd_gappitch);

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


        if(pse->si<0) {
            pse->si = 0;
        }
        else {
            pse->si++;
        }

        mx = pb->grx + (int)((double)(is+1)*sd_gappitch);
    

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
            if(pse->si==-1) {
                pse->si = 0;
            }
            else {
                pse->si++;
            }

            fprintf(fp, "%% thru part %d - %d\n", se->oid, de->oid);

            fprintf(fp, "gsave\n");
            fprintf(fp, "  0.01 setlinewidth\n");
            fprintf(fp, "  1 0 0 setrgbcolor\n");
            fprintf(fp, "  newpath\n");
            fprintf(fp, "  %d %d moveto\n", se->gx, se->gy);
            fprintf(fp, "  %d %d lineto\n", de->gx, de->gy);
            fprintf(fp, "  stroke\n");
            fprintf(fp, "grestore\n");

            {

                dxp = (((double)de->ty-de->by))/(pde->dn+1);
                d0x = de->glx;
                d0y = de->gty;

                dcx = d0x;
                dcy = de->gy;

                fprintf(fp, "gsave\n");

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


                sx = scx;
                sy = scy;

                dx = d0x;
                dy = d0y-dxp*(pde->di+1);

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


XEcho("is %d si %d di %d\n", is, pse->si, pde->di);
                six = s0x;
                siy = s0y-sxp*(pse->si);

                dix = d0x;
                diy = d0y-dxp*(pde->di+1);

    XEcho("cstyle %d %x\n", cstyle, cstyle);
    switch(cstyle) {
    case LS_DIRECT:
                /*
                 * direct lines
                 */
                changecolor(fp, xu->cob.outlinecolor);
                changethick(fp, xu->cob.outlinethick);

                fprintf(fp, "  %d %d moveto\n", six, siy);
                fprintf(fp, "  %d %d lineto\n", dix, diy);
                fprintf(fp, "  stroke\n");
        break;

    default:
    case LS_SQUARE:
                /*
                 * tree shape; thunk is thick, banches are thin.
                 * 
                 */
                changecolor(fp, xu->cob.outlinecolor);
                changethick(fp, xu->cob.outlinethick*pse->sc);

                fprintf(fp, "  %d %d moveto\n", sx, sy);
                fprintf(fp, "  %d %d lineto\n", mx, sy);
                fprintf(fp, "  stroke\n");

                changethick(fp, xu->cob.outlinethick);
                fprintf(fp, "  %d %d moveto\n", mx, sy);
                fprintf(fp, "  %d %d lineto\n", mx, dy);
                fprintf(fp, "  %d %d lineto\n", dx, dy);
                fprintf(fp, "  stroke\n");

                fprintf(fp, "  %d %d moveto\n", sx, sy);
                fprintf(fp, "  %d %d rmoveto\n", 
                    xu->cob.outlinethick*8, xu->cob.outlinethick*8);
                fprintf(fp, "  (dummy) show\n");

    }


                fprintf(fp, "grestore\n");
            
            }



            if(xu->cob.linkstyle) {
            }

        }

        is++;
    }


 }
    



 if(0) 
 {
    u = 0;
    for(i=0;i<pb->cch.nch;i++) {
        pe = (ob*)pb->cch.ch[i];
        if(EXVISIBLE(pe->type)||ISCHUNK(pe->type)) {
        }
        else {
            continue;
        }
        u++;
        bs[u] = pe;
        XEcho("i,u %2d %2d: %p oid %d %d\n", i, u, pe, pe->oid, pe->type);
        if(pe->gx + pe->cwd/2 > bxmax) {
            bxmax = pe->gx + pe->cwd/2;
        }
    }

    v = 0;
    for(j=0;j<pf->cch.nch;j++) {
        se = (ob*)pf->cch.ch[j];
        if(EXVISIBLE(se->type)||ISCHUNK(se->type)) {
        }
        else {
            continue;
        }
        v++;
        fs[v] = se;
        XEcho("j,v %2d %2d: %p oid %d %d\n", j, v, se, se->oid, se->type);
        if(se->gx - se->cwd/2 < fxmin) {
            fxmin = se->gx - se->cwd/2;
        }
    }
    XEcho(" u %d v %d\n", u, v);
    XEcho(" bxmax %d fxmin %d\n", bxmax, fxmin);
    fflush(stdout);


    for(i=0;i<u;i++) {
        if(!bs[i]) {
            XEcho("b %3d: %p\n", i, bs[i]);
        }
        else {
            XEcho("b %3d: %p oid %d type %d\n",
                i, bs[i], bs[i]->oid, bs[i]->type);
        }
    }
    for(j=0;j<v;j++) {
        if(!fs[j]) {
            XEcho("f %3d: %p\n", j, fs[j]);
        }
        else {
            XEcho("f %3d: %p oid %d type %d\n",
                j, fs[j], fs[j]->oid, fs[j]->type);
        }
    }

    if(xu->cob.markguide)
    {
    int x1,y1;
    int fht;
    fprintf(fp, "      gsave\n");

    /* TEMP */
    fht = def_textheight;
    fprintf(fp, "      /%s findfont %d scalefont setfont %% thru backport\n",
        def_fontname, fht);

    for(i=0;i<=u;i++) {
        pe = bs[i];
        if(!pe) {
            continue;
        }
        x1 = pe->gx+pe->cwd/2;
        y1 = pe->gy;
        fprintf(fp, "      %d %d %d (%d) rrshow\n",
            x1, y1, 0, i);
    }

    for(j=0;j<=v;j++) {
        pe = fs[j];
        if(!pe) {
            continue;
        }
        x1 = pe->gx-pe->cwd/2;
        y1 = pe->gy;
        fprintf(fp, "      %d %d %d (%d) lrshow\n",
            x1, y1, 0, j);
    }

    fprintf(fp, "      grestore\n");
    }


    if(xu->cob.linkmap) {
        XEcho("%s: oid %d linkmap '%s'\n",
            __func__, xu->oid, xu->cob.linkmap);
    }
    else {
        XEcho("%s: oid %d no linkmap\n",
            __func__, xu->oid);
    }
    
  {
    char *p;
    char token[BUFSIZ];
    char cmap[BUFSIZ];
    int  ss;
    int  so, sn, eo, en, at;
    int  af, ac, ab;
    int  ik;
    int  cs;

    cmap[0] = '\0';

    if(xu->cob.linkmap) {
        strcpy(cmap, xu->cob.linkmap);
    }
    else {
        char w[BUFSIZ];
        int m, h;
        m = (u<v ? u : v); /* MIN */
        
        strcpy(cmap, "solid");
        for(h=0;h<=m;h++) {
            sprintf(w, ",b%d:f%d", h, h);
            strcat(cmap, w);
        }
    }
    XEcho(" oid %d cmap '%s'\n", xu->oid, cmap);
    p = cmap;
    ss = xu->cob.outlinetype;
    at = xu->cob.arrowheadpart;
    af = xu->cob.arrowforeheadtype;
    ac = xu->cob.arrowcentheadtype;
    ab = xu->cob.arrowbackheadtype;
                XEcho("g ss %d at %d af %d ac %d ab %d\n",
                    ss, at, af, ac, ab);

#if 0
    ob** par;
    par = (ob**)alloca(sizeof(ob*)*(u+v));
#endif

    int npar;
    clink_t *par;
    int i;

    par = (clink_t*)alloca(sizeof(clink_t)*(u+v));
    if(par) {
        memset(par, 0, sizeof(clink_t)*(u+v));
    }

    npar = 0;
    for(i=0;i<u+v;i++) {
        par[i].prop.cob.outlinetype     = xu->cob.outlinetype;
        par[i].prop.cob.arrowheadpart   = xu->cob.arrowheadpart;
        par[i].prop.cob.arrowforeheadtype = xu->cob.arrowforeheadtype;
        par[i].prop.cob.arrowcentheadtype = xu->cob.arrowcentheadtype;
        par[i].prop.cob.arrowbackheadtype = xu->cob.arrowbackheadtype;
    }   

    if(INTRACE) {
        fprintf(stdout, "npar %d\n", npar);
        clink_fprint(stdout, par, u+v);
        clink_fprint(stdout, par, npar);
    }

    while(*p) {
        p = draw_word(p, token, BUFSIZ, XMAP_SEP);
        XEcho("token '%s'\n", token);
        if(!token[0]) {
            continue;
        }
        ik = solve_se(token,
                &so, &sn, &eo, &en, &ss, &at, &af, &ac, &ab);    
        XEcho("  ik %d; so %d sn %d eo %d en %d; ss %d at %d af %d ac %d ab %d\n",
            ik, so, sn, eo, en, ss, at, af, ac, ab);
        fflush(stdout);

        if(ik==1) {
            int x1, y1, x2, y2, x3, y3, x4, y4;
            if(so==AR_BACK) { pe = bs[sn]; } else { pe = fs[sn]; }
            if(eo==AR_BACK) { se = bs[en]; } else { se = fs[en]; }

            if(pe && se) {
                if(so==AR_BACK) {
                    x1 = pe->gx + pe->cwd/2;
                    y1 = pe->gy;
                    x3 = bxmax + am;
                    y3 = pe->gy;
                }
                else {
                    x1 = pe->gx - pe->cwd/2;
                    y1 = pe->gy;
                    x3 = fxmin - am;
                    y3 = pe->gy;
                }
                if(eo==AR_FORE) {
                    x2 = se->gx - se->cwd/2;
                    y2 = se->gy;
                    x4 = fxmin - am;
                    y4 = se->gy;
                }
                else {
                    x2 = se->gx + se->cwd/2;
                    y2 = se->gy;
                    x4 = bxmax + am;
                    y4 = se->gy;
                }


                XEcho("b ss %d at %d af %d ac %d ab %d\n",
                    ss, at, af, ac, ab);
                if(at<0) {  
                    ss = xu->cob.outlinetype;
                    at = xu->cob.arrowheadpart;
                    af = xu->cob.arrowforeheadtype;
                    ac = xu->cob.arrowcentheadtype;
                    ab = xu->cob.arrowbackheadtype;
                }
                XEcho("a ss %d at %d af %d ac %d ab %d\n",
                    ss, at, af, ac, ab);

#if 0
                fprintf(fp, "newpath %d %d moveto %d %d %d 0 360 arc fill\n",
                    x3, y3, x3, y3, objunit/20);
                fprintf(fp, "newpath %d %d moveto %d %d %d 0 360 arc fill\n",
                    x4, y4, x4, y4, objunit/20);
#endif

{
                tmpar = varray_new();
                varray_entrysprintfunc(tmpar, seg_sprintf);

#if 0
                varray_fprintv(stdout, tmpar);
#endif

                path_regsegmoveto(tmpar, x1, y1);
                path_regseglineto(tmpar, x3, y3);
                if(x3==x4) {
                    if(so==AR_BACK) {
                        path_regsegcurveto(tmpar, x3+am, y3, x4+am, y4, x4, y4);
                    }
                    else {
                        path_regsegcurveto(tmpar, x3-am, y3, x4-am, y4, x4, y4);
                    }
                }
                else {
                    if(so==AR_BACK) {
                        path_regsegcurveto(tmpar, x3+cm, y3, x4-cm, y4, x4, y4);
                    }
                    else {
                        path_regsegcurveto(tmpar, x3-cm, y3, x4+cm, y4, x4, y4);
                    }
                }
                path_regseglineto(tmpar, x2, y2);

                if(tmpar->use>0) {
                    /*** NOTE offset is cared already. do not applay twice */
fprintf(fp, "%% thruX token '%s'\n", token);
                    __drawpath_LT(fp, 0, 0, 0, xu, xns, tmpar);
                }

    
}


                if(npar<u+v) {
XEcho("ADD\n");
                    par[npar].so = so;
                    par[npar].sn = sn;
                    par[npar].eo = eo;
                    par[npar].en = en;
                    par[npar].x1 = x1;
                    par[npar].y1 = y1;
                    par[npar].x2 = x2;
                    par[npar].y2 = y2;
                    par[npar].x3 = x3;
                    par[npar].y3 = y3;
                    par[npar].x4 = x4;
                    par[npar].y4 = y4;

#if 1
                    par[npar].prop.cob.outlinetype = ss;
                    par[npar].prop.cob.arrowheadpart = at;
                    par[npar].prop.cob.arrowforeheadtype = af;
                    par[npar].prop.cob.arrowcentheadtype = ac;
                    par[npar].prop.cob.arrowbackheadtype = ab;
#endif

#if 0
                    par[npar].prop.cob.outlinetype = xu->cob.outlinetype;
                    par[npar].prop.cob.arrowheadpart = xu->cob.arrowheadpart;
                    par[npar].prop.cob.arrowforeheadtype = xu->cob.arrowforeheadtype;
                    par[npar].prop.cob.arrowcentheadtype = xu->cob.arrowcentheadtype;
                    par[npar].prop.cob.arrowbackheadtype = xu->cob.arrowbackheadtype;
#endif


                    npar++;

    if(INTRACE) {
        fprintf(stdout, "npar %d\n", npar);
        clink_fprint(stdout, par, u+v);
        clink_fprint(stdout, par, npar);
    }
                }
            }
            else {
                Error("ignore position in '%s'; recognized objects back %d fore %d\n",
                    token, u, v);
            }
        }
        if(ik==2) {
#if 0
            cs = ss;
#endif
        }
    }


    if(INTRACE) {
        fprintf(stdout, "npar %d\n", npar);
        clink_fprint(stdout, par, u+v);
        clink_fprint(stdout, par, npar);
    }

  }

 }

out:
    return 0;
}
