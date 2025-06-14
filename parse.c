#include <stdio.h>

#include "alist.h"
#include "varray.h"

#include "word.h"
#include "seg.h"
#include "obj.h"
#include "chas.h"
#include "notefile.h"
#include "a.h"
#include "gv.h"

#include "color.h"


int
__sdumpNZ(FILE *fp, char *msg, char *s, int n, int zb)
{
    int c;
    fprintf(fp, "%s: |", msg);
    c = 0;
    while(c<n) {
        if(*s<' '||*s==127) {
#if 0
            fprintf(fp, "<%02hhX>", (unsigned char)*s);
#endif
            fprintf(fp, "<%x%x>", ((unsigned char)*s)/16, ((unsigned char)*s)%16);
            if(zb>0 && *s=='\0') {
                break;
            }
        }
        else {  
            fprintf(fp, "%c", *s);
        }
        s++;
        c++;
    }
    fprintf(fp, "|\n");
    fflush(fp);
    return 0;
}

int
sdumpN(FILE *fp, char *msg, char *s, int n)
{
    return __sdumpNZ(fp, msg, s, n, 0);
}

int
sdumpNZ(FILE *fp, char *msg, char *s, int n)
{
    return __sdumpNZ(fp, msg, s, n, 1);
}

int
sdump(FILE *fp, char *msg, char *s)
{
    fprintf(fp, "%s: |", msg);
    while(*s) {
        if(*s=='\n') {
            fprintf(fp, "<%02X>", *s);
        }
        else {  
            fprintf(fp, "%c", *s);
        }
        s++;
    }
    fprintf(fp, "|\n");
    fflush(fp);
    return 0;
}

int
dellastcr(char *bf)
{
    char *p;
    p = bf;
    while(*p) {
        if(*p=='\n'||*p=='\r') {
            *p = '\0';
            break;
        }
        p++;
    }
    return 0;
}

int
trimdoublequote(char *s)
{
    char *os;
    os = s;
#if 0
    sdumpNZ(stderr, "b", os, 16);
#endif
    if(*s=='"') {
#if 0
        sdumpNZ(stderr, "0s", s, 16);
#endif
        memmove(s, s+1, strlen(s+1)+1);
#if 0
        sdumpNZ(stderr, "1s", s, 16);
#endif
        while(*s) {
            if(*s=='"') {
                *s = '\0';
                break;
            }
            s++;
        }
    }
    *s = '\0';
#if 0
    sdumpNZ(stderr, "a", os, 16);
#endif
    return 0;
}


int
delfirstchars(char *bf, int w)
{
    char *p;
    char *q;

#if 0
fEcho(stderr, "w %d\n", w);
  sdump(stderr, "delfirstchars b ", bf);
#endif

    p = bf+w;
    q = bf;
    while(*p) {
        *q++ = *p++;
    }
    *q = '\0';

#if 0
  sdump(stderr, "delfirstchars a ", bf);
#endif
    return 0;
}

int
parsedimen(char *src)
{
    extern int objunit;
    char  *p, *q;
    char   mstr[BUFSIZ];
    double dm;
    int    m;

    p = src;
    q = mstr;

    while(*p &&
        ((*p>='0'&& *p<='9') || (*p=='.') || (*p=='-'))) {
        *q++ = *p++;
    }
    *q = '\0';

    if(mstr[0]) {
        dm = atof(mstr);
    }
    else {
        /* empty means 1 unit */
        m = (int)objunit;
        goto skip_m;
    }

#if 0
    printf("  p '%s'\n", p);
#endif

    /*
     * 1u       objunit
     * 1inch    72bp    7200
     * 1cm      28.34bp 2834
     * 1mm      2.834bp  283.4
     */
    
    if(strcasecmp(p,"u")==0) {
        dm *= objunit;
    }
    else
    if(strcasecmp(p,"inch")==0) {
        dm *= 7200;
    }
    else
    if(strcasecmp(p,"pica")==0) {
        dm *= 1200;
    }
    else
    if(strcasecmp(p,"cm")==0) {
        dm *= 2834;
    }
    else
    if(strcasecmp(p,"mm")==0) {
        dm *= 283.4;
    }

    m = (int)(dm);

skip_m:
#if 0
    printf("    src '%s' -> m %d\n", src, m);
#endif
    
    return m;
}

int
parsergb(char *src, int *xr, int *xg, int *xb)
{
    int   rv;
    char *hexstr="0123456789ABCDEF";
    char *p;
    char  m;
    char *u;
    int  yr, yg, yb;

    rv = -1;
    p = src;

    if(*p>='a'&&*p<='f') { m = *p - 32; }
    else { m = *p; }

    yr = -1;
    u = hexstr;
    while(*u) {
        if(m==*u) {
            yr = u - hexstr;
            break;
        }
        u++;
    }

    p++;
    if(!*p) {
        goto out;
    }

    if(*p>='a'&&*p<='f') { m = *p - 32; }
    else { m = *p; }

    yg = -1;
    u = hexstr;
    while(*u) {
        if(m==*u) {
            yg = u - hexstr;
            break;
        }
        u++;
    }

    p++;
    if(!*p) {
        goto out;
    }

    if(*p>='a'&&*p<='f') { m = *p - 32; }
    else { m = *p; }

    yb = -1;
    u = hexstr;
    while(*u) {
        if(m==*u) {
            yb = u - hexstr;
            break;
        }
        u++;
    }

out:
    if(yr>=0) { *xr = yr; }
    if(yg>=0) { *xg = yg; }
    if(yb>=0) { *xb = yb; }

    if(yr>=0 && yg>=0 && yb>=0) { rv = 0; }

#if 0
    fprintf(stderr, "%s: src |%s| %d/%d/%d r %d g %d b %d ; rv %d\n", 
            __func__, src, yr, yg, yb, *xr, *xg, *xb, rv);
#endif

    return rv;   
}

int
parsecolor(pallet_t *ar, char *src)
{
    int   rv = -1;
    int   m = 0;
    int   maydec, mayhex;
    char *p;
    char *q;
    int   c;
    char  buf[BUFSIZ];
    color_t *cpos;

    maydec = mayhex = 0;
    cpos = NULL;

    p = src;
    while(*p && (*p>='0' && *p<='9')) {
        p++;
    }
    if(*p=='\0') {
        maydec = 1;
    
        if(maydec) {
            rv = atoi(src);
            m = 1;
            goto out;
        }
    }

    p = src;
    q = buf;
    c = 0;
    while(*p && c<BUFSIZ && ((*p>='0' && *p<='9') || (*p>='a' && *p<='f') || (*p>='A' && *p<='F'))) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
#if 0
        fprintf(stderr, "buf |%s| c %d\n", buf, c);
#endif
    if((*p=='h' ||*p=='H') && *(p+1)=='\0') {
        mayhex = 1;
        m += 10;
#if 0
        fprintf(stderr, "hex buf |%s| c %d\n", buf, c);
#endif
        if(c==3) {
            int vr, vg, vb;
            int ik;

            vr = vg = vb = 9999;
            ik = parsergb(src, &vr, &vg, &vb);
#if 0
            cpos = pallet_findwrgb(ar, vr, vg, vb);
#endif
            cpos = pallet_findaddwrgb(ar, vr, vg, vb);
            if(cpos) {
#if 0
                fprintf(stderr, "FOUND such color; addr %p\n", cpos);
#endif
                rv = cpos->num;
                m += 100;
            }
            else {
                fprintf(stderr, "NOT found such color\n");
                m += 1000;
            }
        }
        else {
            fprintf(stderr, "ignore length. expected 3 RGB letters\n");
        }
    }
    else {
        char *ref;

        if(*src=='-') {
            rv = -1;
#if 0
            fprintf(stderr,
                "explicit negative color; no-draw or transparent\n");
#endif
            goto out2;
        }

        ref = src;
        if(*src=='"') { 
            char *p;
            char *q;

            /* trim " */
            p = src+1;
            q = buf;
            c = 0;
            while(*p&& c<BUFSIZ && *p!='"') {
                *q++ = *p++;
                c++;
            }
            *q = '\0';
            ref = buf;
        }
#if 0
        fprintf(stderr, "it('%s') is name of color, maybe\n", ref);
#endif
        cpos = pallet_findwname(ar, ref);
        if(cpos) {
            rv = cpos->num;
            m += 10000;
        }
    }

out:
#if 0
    fprintf(stderr, "%s: src |%s| maydec %d mayhex %d ; m %d rv %d\n",
        __func__, src, maydec, mayhex, m, rv);
#endif
    if(rv<0) {
        Error("unknown or undefined color '%s'\n", src);
        fflush(stdout);
    }
out2:

    return rv;
}




int   lineno=-1;
int   cmdno=-1;


int
gotlabel(char *lab, int lsize, char *line)
{
    char *p, *q;
    char  tmp[BUFSIZ];
    int   c;
    int   found;

    found = 0;
    q = tmp;
    *q = '\0';

    p = line;
    while(*p && (*p==' '||*p=='\t')) {
        p++;
    }

    c = 0;
    while(*p && (isalnum(*p)||*p=='_') && c<lsize-1) {
        *q++ = *p++;
    }
    *q = '\0';
    if(*p==':') {
        found++;
    }

    if(found) {
        strcpy(lab, tmp);
#if 0
        Echo("label '%s'\n", lab);
#endif
    }

    return found;
}

char*
skiplabel(char *lab, int lsize, char *line)
{
    char *s;
    char *p, *q;
    char  tmp[BUFSIZ];
    int   c;
    int   found;

    found = 0;
    s = NULL;
    q = tmp;
    *q = '\0';

    p = line;
    while(*p && (*p==' '||*p=='\t')) {
        p++;
    }

    c = 0;
    s = p;
    while(*p && (isalnum(*p)||*p=='_') && c<lsize-1) {
        *q++ = *p++;
    }
    *q = '\0';
    if(*p==':') {
        found++;
        p++;
    }

    if(found) {
        strcpy(lab, tmp);
#if 0
        Echo("  label '%s'\n", lab);
#endif
        return p;
    }

    return s;
}

char*
parsetype(int *rot, char *line)
{
    char *p;
    int   ot;
    char  word[BUFSIZ];

    ot = -1;
    
    p = line;
    p = draw_wordW(p, word, BUFSIZ);

    ot = assoc(cmd_ial, word);

    if(ot<0) {
        Error("falldown unknown object '%s' [line %d]\n", word, lineno);
        exit(3);
    }

    *rot = ot;

    return p;
}

char*
parse_segop(char *sin, int dt, ob *nob)
{
    char   *p;
    char    tmp[BUFSIZ];
    int     v;
    double  dv;
    int     co;
    char    vstr[BUFSIZ];
    char   *f;
    char   *m;
    segop  *newop;
    char   *r;
    int     sop;
    int     top;

#if 0
    Echo("%s: sin '%s' dt %d nob %p\n", __func__, sin, dt, nob);
#endif
    r = NULL;
#if 0
    Echo("%s: dt %d\n", __func__, dt);
    Echo("sin '%s'\n", sin);
#endif

    co = 0;
    p = sin;
    tmp[0] = '\0';
    vstr[0] = '\0';

    if(dt==OA_THEN) {
    }
    else
    if(dt==OA_CLOSE) {
    }
    else {
        p = draw_wordW(p, tmp, BUFSIZ);

        sop = assoc(objattr_ial, tmp);
        if(sop>=0) {
Info("tmp '%s' seems not value\n", tmp);
            tmp[0] = '\0';
        }
        else {
#if 0
Info("segop tmp '%s'\n", tmp);
#endif
            co = 1;

            strcpy(vstr, tmp);

            if(dt==OA_ARC||dt==OA_ARCN) {
                /* one more arg */

                p = draw_wordW(p, tmp, BUFSIZ);

                top = assoc(objattr_ial, tmp);
                if(sop>=0) {
#if 0
Warn("tmp '%s' seems not value\n", tmp);
#endif
                    tmp[0] = '\0';
                }
                else {
                    co = 1;

                    strcat(vstr, SEG_SEPS);
                    strcat(vstr, tmp);
                }
            }
            else
            if(dt==OA_RCURVETO) {
                /* 5 more args */
                int cc;

                cc = 0;
                while(cc<5) {
                    p = draw_wordW(p, tmp, BUFSIZ);

                    top = assoc(objattr_ial, tmp);
                    if(sop>=0) {
#if 0
Warn("tmp '%s' seems not value\n", tmp);
#endif
                        tmp[0] = '\0';
                        break;
                    }
                    else {
                        co = 1;

                        Echo("cc %d tmp '%s'\n", cc, tmp);
                        strcat(vstr, SEG_SEPS);
                        strcat(vstr, tmp);
                    }
                    cc++;
                }
            }


        }

    }

    newop = (segop*)malloc(sizeof(segop));
    if(!newop) {
E;
        Error("%s: no memory\n", __func__);
    }
    memset(newop, 0, sizeof(segop));
    newop->cmd = dt;
    newop->val = strdup(vstr);

#if 1
    if(dt==OA_FROM && *vstr) {
P;
        nob->cob.hasfrom++;
        nob->cob.originalshape++;
    }
    if(dt==OA_TO   && *vstr) {
P;
        nob->cob.hasto++;
        nob->cob.originalshape++;
    }
#endif

#if 0
    if(dt==OA_FROM && *vstr) {
P;
        nob->cob.afrom = strdup(vstr);
    }
    if(dt==OA_TO   && *vstr) {
P;
        nob->cob.ato   = strdup(vstr);
    }
#endif

#if 0
    if(dt==OA_CLOSE) {
        segop *dmyop;
        dmyop = (segop*)malloc(sizeof(segop));
        if(!dmyop) {
E;
            Error("%s: no memory\n", __func__);
        }
        memset(dmyop, 0, sizeof(segop));
        dmyop->cmd = OA_THEN;
        dmyop->val = strdup(vstr);
        varray_push(nob->cob.segopar, (void*)dmyop);
#if 0
Echo("  push dmy %d val '%s'\n", dmyop->cmd, dmyop->val);
#endif
#if 1
Echo("  push dmy '%s'(%d) val '%s'\n",
        rassoc(objattr_ial, dmyop->cmd), dmyop->cmd, dmyop->val);
#endif
    }
#endif

#if 0
Echo("  push cmd %d val '%s'\n", newop->cmd, newop->val);
#endif
#if 0
Echo("  push cmd '%s'(%d) val '%s'\n",
        rassoc(objattr_ial, newop->cmd), newop->cmd, newop->val);
#endif
    varray_push(nob->cob.segopar, (void*)newop);

#if 0
    fprintf(stdout, "segopar");
    varray_fprint(stdout, nob->cob.segopar);
#endif
#if 0
    fprintf(stdout, "segopar");
    varray_fprintv(stdout, nob->cob.segopar);
#endif

    nob->cob.originalshape++;
#if 1
    Echo("original %d oid %d\n",
        nob->cob.originalshape, nob->oid);
#endif

    if(co) {
        r = p;
    }
    else {
        r = NULL;
    }

#if 1
    Echo("%s: r %p\n", __func__, r);
#endif

    return r;
}


int
parse_aheads(char *name, int *m, int *fa, int *ca, int *ba, int acshortdash)
{
    if(acshortdash) {
    if(strcasecmp(name, "-")==0)     {
        *m  = AR_NONE;
        *fa = AH_NONE;
        *ca = AR_NONE;
        *ba = AH_NONE;
        goto out;
    }
    }
    if(strcasecmp(name, "--")==0)     {
        *m  = AR_NONE;
        *fa = AH_NONE;
        *ca = AR_NONE;
        *ba = AH_NONE;
        goto out;
    }
    if(strcasecmp(name, "->")==0)     {
        *m  = AR_FORE;
        *fa = AH_NORMAL;
        *ca = AR_NONE;
        *ba = AH_NONE;
        goto out;
    }
    if(strcasecmp(name, "<-")==0)     {
        *m  = AR_BACK;
        *fa = AH_NONE;
        *ca = AR_NONE;
        *ba = AH_NORMAL;
        goto out;
    }
    if(strcasecmp(name, "<->")==0)     {
        *m  = AR_BOTH;
        *fa = AH_NORMAL;
        *ca = AR_NONE;
        *ba = AH_NORMAL;
        goto out;
    }
    if(strcasecmp(name, "->-")==0)     {
        *m  = AR_CENT;
        *fa = AH_NONE;
        *ca = AH_NORMAL;
        *ba = AH_NONE;
        goto out;
    }
    if(strcasecmp(name, "-<-")==0)     {
        *m  = AR_CENT;
        *fa = AH_NONE;
        *ca = AH_REVNORMAL;
        *ba = AH_NONE;
        goto out;
    }

    return 0;
    
out:
    return 1;
}

char *
parseobjattr(ob *rob, char *src)
{
    char *p;
    char  name[BUFSIZ];
    char  value[BUFSIZ];
    char  tmp[BUFSIZ];
    char *q;
    int   x;
    double dv;
    int   oak;
    sstr *ns;
    int   k1;
    int   v1;
    int   uc;
    int   gc;
    char *s;


    if(!src || *src=='\0') {
        return NULL;
    }

    uc = 0;
    gc = 0;
    
    k1 = -1;
    ns = NULL;
    p = src;

    if((*p>='0'&&*p<='9')||*p=='-') {
        q = tmp;
        if(*(p+1)>='0'&&*(p+1)<='9') {
            /* nothing */
        }
        else {
            goto skip_number;
        }
        while(*p && ((*p>='0'&&*p<='9')||*p=='-')) {
            *q++ = *p++;
        }
        *q = '\0';
        rob->cob.iarg1 = atoi(tmp); 
#if 0
Echo("iarg1 %d as '%s'\n", rob->cob.iarg1, tmp);
#endif
        goto out;
    }
skip_number:

    if(*p=='"') {
        q = tmp;
        p++;
        while(*p && (*p!='"')) {
            *q++ = *p++;
        }
        *q = '\0';
        p++;

        if(rob->cob.carg1) {
            rob->cob.carg2 = strdup(tmp);   
#if 0
            Echo("carg2 '%s' as '%s'\n", rob->cob.carg2, tmp);
#endif
        }
        else {
            rob->cob.carg1 = strdup(tmp);   
#if 0
            Echo("carg1 '%s' as '%s'\n", rob->cob.carg1, tmp);
#endif
        }

        /*
         * although tmp is empty, the string is be add to array.
         * because script may expect empty line in the place.
         */
#if 0
fprintf(stderr, "tmp <%s>\n", tmp);
#endif
        ns = (sstr*) malloc(sizeof(sstr));
        if(!ns) {
            Error("%s: no memory\n", __func__);
        }
        else {
            ns->ssval = strdup(tmp);
            ns->ssopt = 0;
            varray_push(rob->cob.ssar, ns);
#if 0
            varray_fprint(stdout, rob->cob.ssar);
#endif
        }

        goto out;
    }

    p = draw_wordW(p, name, BUFSIZ);
    if(name[0]=='\0') {
        return NULL;
    }

    s = &name[0];
    if(*s==':') {
        char pstr[BUFSIZ];
        int  pn;
        int  ik;

        strcpy(pstr, s+1);
#if 0
        Echo("  pstr '%s'\n", pstr);
#endif
        if(!pstr[0]) {
            goto skip_note;
        }
    
        pn = assoc(pos_ial, pstr);
        if(pn<0) {
            goto skip_note;
        }

        Echo("   pn %d\n", pn);

        p = skipwhite(p);

        if(*p=='"') {
            p = draw_wordDQ(p, value, BUFSIZ);
        }
        else {
            p = draw_wordW(p, value, BUFSIZ);
        }

        
        if(pn==PO_KEY && value[0]) {
            ik = nb_regist(value, rob);
        }

#if 0
        Echo("   value '%s'\n", value);
#endif

        rob->cob.note[pn] = strdup(value);

#if 1
        Echo("THEN pn %3d 0%03o %3xH %-4s '%s' %d\n",
            pn, pn, pn, rassoc(pos_ial, pn), 
            rob->cob.note[pn], (int)strlen(rob->cob.note[pn]));
#endif
        
    }
skip_note:


    parse_aheads(name,
        &rob->cob.arrowheadpart,
        &rob->cob.arrowforeheadtype,
        &rob->cob.arrowcentheadtype,
        &rob->cob.arrowbackheadtype, 0);

    oak = assoc(objattr_ial, name);
    if(oak<0) {
#if 0
        Echo("WARNING unknown attribue for object '%s' [line %d]\n", name, lineno);
#endif
        oak = assoc(pos_ial, name);
        if(oak<0) {
            Echo("WARNING unknown attribue for object/position '%s' [line %d]\n", name, lineno);
        }
    }
#if 0
    Echo("oak %d\n", oak);
#endif
#if 1
    if(oak==OA_FROM||oak==OA_TO) {
        Echo("from/to oak %d '%s'\n", oak, name);
    }
#endif


    /* integer setting */
#define ONSET(x,y) \
    if(oak==(x)) { \
        rob->cob.y = 1; \
        uc++; \
    }
#define ISET(x,y) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        rob->cob.y = parsedimen(value); \
        uc++; \
    }

#define ISET2(x,y1,y2) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        rob->cob.y1 = parsedimen(value); \
        rob->cob.y2 = parsedimen(value); \
        uc++; \
    }

#define RSET(x,y1) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        rob->cob.y1 = atof(value); \
        uc++; \
    }

#define RSET2(x,y1,y2) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        rob->cob.y1 = atof(value); \
        rob->cob.y2 = atof(value); \
        uc++; \
    }

#define LADD(x,y) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        /* fprintf(stderr, "value  |%s|\n", value); */\
        if(rob->cob.y==NULL) { \
            rob->cob.y = strdup(value); \
        } \
        else { \
            char atmp[BUFSIZ]; \
            strcpy(atmp, rob->cob.y); \
            strcat(atmp, DECO_SEPS); \
            strcat(atmp, value); \
            free(rob->cob.y); \
            rob->cob.y = strdup(atmp); \
        } \
        /*fprintf(stderr, "result |%s|\n", rob->cob.y);*/ \
        uc++; \
    }

#define SADD(x,y) \
    if(oak==(x)) { \
        /* fprintf(stderr, "call draw_wordW <%s>\n", p); */ \
        p = draw_wordW(p, value, BUFSIZ); \
        /* fprintf(stderr, "value  |%s|\n", value); */ \
        if(rob->cob.y==NULL) { \
            rob->cob.y = strdup(value); \
        } \
        else { \
            char atmp[BUFSIZ]; \
            strcpy(atmp, rob->cob.y); \
            strcat(atmp, MAP_SEPS); \
            strcat(atmp, value); \
            free(rob->cob.y); \
            rob->cob.y = strdup(atmp); \
        } \
        /*fprintf(stderr, "result |%s|\n", rob->cob.y);*/ \
        uc++; \
    }

#define XSADD(x,y) \
    if(oak==(x)) { \
        /* fprintf(stderr, "call draw_wordDQ <%s>\n", p); */ \
        p = draw_wordDQ(p, value, BUFSIZ); \
        /* fprintf(stderr, "value  |%s|\n", value); */ \
        if(rob->cob.y==NULL) { \
            rob->cob.y = strdup(value); \
        } \
        else { \
            char atmp[BUFSIZ]; \
            strcpy(atmp, rob->cob.y); \
            strcat(atmp, MAP_SEPS); \
            strcat(atmp, value); \
            free(rob->cob.y); \
            rob->cob.y = strdup(atmp); \
        } \
        /*fprintf(stderr, "result |%s|\n", rob->cob.y);*/ \
        uc++; \
    }

    /* integer setting */
#define OISET(x,y) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        rob->y = parsedimen(value); \
        uc++; \
    }

    /* real setting */
#define GFSET(x,y) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        y = atof(value); \
        if(0) fprintf(stderr, "%s:%d GFSET %d '%s'->%.3f \n", \
                __FILE__, __LINE__, x, value, (double)y); \
        gc++; \
        uc++; \
    }

#define AISET(N,L,P)    \
    if(oak==(N)) {  \
        p = draw_wordW(p, value, BUFSIZ); \
        x = assoc((L), value);  \
        if(x>=0) {  \
            rob->cob.P = x; \
        }   \
        else { \
            Error("ignore value '%s' [line %d]\n", value, lineno); \
        } \
        uc++;   \
    }

#define AISETN(N,L,P)   \
    if(oak==(N)) {  \
        p = draw_wordW(p, value, BUFSIZ); \
        x = assoc((L), value);  \
        if(x>=0) {  \
            rob->cob.P = x; \
        }   \
        else { \
            rob->cob.P = atoi(value);   \
        } \
        uc++;   \
    }

#define AISETX(N,L,P,D) \
    if(oak==(N)) {  \
        p = draw_wordW(p, value, BUFSIZ); \
        x = assoc((L), value);  \
        if(x>=0) {  \
            rob->cob.P = x; \
        }   \
        else {  \
            rob->cob.P = (D);   \
        }   \
        uc++;   \
    }

    /* skip . (dot) */
#define AISETXD(N,L,P,D) \
    if(oak==(N)) {  \
        char *sv; \
        p = draw_wordW(p, value, BUFSIZ); \
        sv = value; \
        if(sv[0]=='.') { sv++; } \
        x = assoc((L), sv);  \
        if(x>=0) {  \
            rob->cob.P = x; \
        }   \
        else {  \
            rob->cob.P = (D);   \
        }   \
        uc++;   \
    }

#define AISETC(N,P) \
    if(oak==(N)) {  \
        p = draw_wordW(p, value, BUFSIZ); \
        /* fprintf(stderr, "value  |%s|\n", value); */ \
        x = parsecolor(pallet, value); \
        rob->cob.P = x; \
        /* fprintf(stderr, "setted |%d|\n", rob->cob.P); */ \
        uc++;   \
    }

P;
    uc = 0;
    AISETC(OA_LINECOLOR,    outlinecolor);
    ISET(OA_LINETHICK,      outlinethick);
    AISETN(OA_LINETYPE,     linetype_ial, outlinetype);

    ISET(OA_WLINETHICK,     wlinethick);

    AISETC(OA_FILLCOLOR,    fillcolor);
    AISETX(OA_FILLHATCH,    hatchtype_ial, fillhatch, HT_XCROSSED);
    ISET(OA_FILLTHICK,      fillthick);
    ISET(OA_FILLPITCH,      fillpitch);

    AISETC(OA_BACKCOLOR,    backcolor);
    AISETX(OA_BACKHATCH, hatchtype_ial, backhatch, HT_XCROSSED);
    ISET(OA_BACKTHICK,      backthick);
    ISET(OA_BACKPITCH,      backpitch);
#if 0
    ISET(OA_HATCHTHICK,     hatchthick);
    ISET(OA_HATCHPITCH,     hatchpitch);
#endif
    ISET2(OA_HATCHTHICK,    fillthick, backthick);
    ISET2(OA_HATCHPITCH,    fillpitch, backpitch);

    AISETC(OA_TEXTCOLOR,    textcolor);
    AISETC(OA_TEXTBGCOLOR,  textbgcolor);
    ISET(OA_TEXTROTATE,     textrotate);
    AISETXD(OA_TEXTPOSITION,  pos_ial, textposition, PO_CENTER);
    ISET(OA_TEXTHOFFSET,    texthoffset);
    ISET(OA_TEXTVOFFSET,    textvoffset);

    ONSET(OA_HOLLOW,        hollow);
    ONSET(OA_SHADOW,        shadow);

    if(oak==OA_NOSLIT) {    rob->cob.slittype = OA_NOSLIT; };
    if(oak==OA_HSLIT)  {    rob->cob.slittype = OA_HSLIT;  };
    if(oak==OA_VSLIT)  {    rob->cob.slittype = OA_VSLIT;  };
    if(oak==OA_HWSLIT) {    rob->cob.slittype = OA_HWSLIT; };
    if(oak==OA_VWSLIT) {    rob->cob.slittype = OA_VWSLIT; };
    ISET(OA_SLITPOS,        slitpos);
    ISET(OA_SLITTHICK,      slitthick);

    AISET(OA_LINKSTYLE, ls_ial, linkstyle);
    SADD(OA_LINKMAP,        linkmap);

    ISET(OA_RAD,            rad);
    ISET(OA_LENGTH,         length);
    ISET(OA_PEAK,           polypeak);
    ISET(OA_PEAKROTATE,     polyrotate);
    ONSET(OA_CONCAVE,       concave);
    ISET(OA_IMARGIN,        imargin);
    ISET(OA_GIMARGIN,       gimargin);
    ISET(OA_ROTATE,         rotateval);

    ISET(OA_PIESTART,       piestart);
    ISET(OA_PIEEND,         pieend);
    ONSET(OA_OUTLINEONLY,   outlineonly);

    LADD(OA_DECO,           deco);
    AISETC(OA_DECOCOLOR,    decocolor);

    AISET(OA_LANEORDER,     lo_ial, laneorder);
    ISET(OA_LANENUM,        lanenum);
    ISET(OA_LANEGAPV,       lanegapv);
    ISET(OA_LANEGAPH,       lanegaph);
    ISET2(OA_LANEGAP,       lanegapv, lanegaph);

    XSADD(OA_PORT,          portstr);
    ISET(OA_PORTROTATE,     portrotate);
    ISET(OA_PORTOFFSET,     portoffset);
    XSADD(OA_STARBOARD,     boardstr);
    ISET(OA_BOARDROTATE,    boardrotate);
    ISET(OA_BOARDOFFSET,    boardoffset);

    SADD(OA_BGSHAPE,        bgshape);

    OISET(OA_WIDTH,         wd);
    OISET(OA_HEIGHT,        ht);

    ISET(OA_CRANKPOS,       crankpos);

    AISETN(OA_AUXLINETYPE,  auxlinetype_ial, auxlinetype);
    OISET(OA_AUXLINEDISTANCE,      cauxlinedistance);
    LADD(OA_AUXLINEOPT,     auxlineopt);
    

    ISET2(OA_CHOP,          forechop, backchop);
    ISET(OA_FORECHOP,       forechop);
    ISET(OA_BACKCHOP,       backchop);
    ISET(OA_BULGE,          bulge);
    ONSET(OA_ARROWEVERY,    arrowevery);

    XSADD(OA_FILE,          filestr);
    RSET2(OA_FILESCALEXY,   filescalex, filescaley);
    RSET(OA_FILESCALEX,     filescalex);
    RSET(OA_FILESCALEY,     filescaley);

    GFSET(OA_TEXTHEIGHTFACTOR,      textheightfactor);
    GFSET(OA_ARROWSIZEFACTOR,       arrowsizefactor);
    GFSET(OA_LINETHICKFACTOR,       linethickfactor);
    GFSET(OA_LINEDECOTHICKFACTOR,   linedecothickfactor);
    GFSET(OA_LINEDECOPITCHFACTOR,   linedecopitchfactor);
    GFSET(OA_HATCHTHICKFACTOR,      hatchthickfactor);
    GFSET(OA_HATCHPITCHFACTOR,      hatchpitchfactor);
    GFSET(OA_NOTEOSEPFACTOR,        noteosepfactor);
    GFSET(OA_NOTEISEPFACTOR,        noteisepfactor);


#if 0
    if(strcasecmp(name, "linetype")==0)        {
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(linetype_ial, value);
        if(x>=0) {
            rob->cob.outlinetype = x;
        }
        else {
            Echo("WARNING strange linetype '%s' [line %d]\n", value, lineno);
            rob->cob.outlinetype = atoi(value);
        }
        uc++;
    }
    else
#endif
    /***
     ***
     ***/
    if(strcasecmp(name, "at")==0)     {
        rob->hasrel = 1;
        p = draw_wordW(p, value, BUFSIZ);
        rob->cob.aat = strdup(value);
        uc++;
    }
    else if(strcasecmp(name, "with")==0)     {
        rob->hasrel = 1;
        p = draw_wordW(p, value, BUFSIZ);
        rob->cob.awith = strdup(value);
        uc++;
    }
#if 0
    else if(strcasecmp(name, "from")==0)     {
        rob->hasrel = 1;
        p = draw_wordW(p, value, BUFSIZ);
        rob->cob.afrom = strdup(value);
        uc++;
        rob->cob.originalshape++;
    }
    else if(strcasecmp(name, "to")==0)     {
        rob->hasrel = 1;
        p = draw_wordW(p, value, BUFSIZ);
        rob->cob.ato   = strdup(value);
        uc++;
        rob->cob.originalshape++;
    }
#endif
    /***
     *** arrow head
     ***/
    else if(strcasecmp(name, "forehead")==0) {
        rob->cob.arrowheadpart |= AR_FORE;
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(arrowhead_ial, value);
        if(x>=0) {
            rob->cob.arrowforeheadtype = x;
        }
        else {
            rob->cob.arrowforeheadtype = atoi(value);
        }
        uc++;
    }
    else if(strcasecmp(name, "backhead")==0) {
        rob->cob.arrowheadpart |= AR_BACK;
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(arrowhead_ial, value);
        if(x>=0) {
            rob->cob.arrowbackheadtype = x;
        }
        else {
            rob->cob.arrowbackheadtype = atoi(value);
        }
        uc++;
    }

    else if(strcasecmp(name, "centhead")==0) {
        rob->cob.arrowheadpart |= AR_CENT;
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(arrowhead_ial, value);
        if(x>=0) {
            rob->cob.arrowcentheadtype = x;
        }
        else {
            rob->cob.arrowcentheadtype = atoi(value);
        }
        uc++;
    }

    else if(strcasecmp(name, "bothhead")==0) {
        rob->cob.arrowheadpart |= AR_BOTH;
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(arrowhead_ial, value);
        if(x>=0) {
            rob->cob.arrowforeheadtype = x;
            rob->cob.arrowbackheadtype = x;
        }
        else {
            rob->cob.arrowforeheadtype = atoi(value);
            rob->cob.arrowbackheadtype = atoi(value);
        }
        uc++;
    }

    else if(strcasecmp(name, "allhead")==0) {
        rob->cob.arrowheadpart |= AR_ALL;
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(arrowhead_ial, value);
        if(x>=0) {
            rob->cob.arrowforeheadtype = x;
            rob->cob.arrowcentheadtype = x;
            rob->cob.arrowbackheadtype = x;
        }
        else {
            rob->cob.arrowforeheadtype = atoi(value);
            rob->cob.arrowcentheadtype = atoi(value);
            rob->cob.arrowbackheadtype = atoi(value);
        }
        uc++;
    }

    else if(strcasecmp(name, "centheadpos")==0) {
        rob->cob.arrowheadpart |= AR_CENT;
        p = draw_wordW(p, value, BUFSIZ);
        dv = atof(value);
        rob->cob.arrowcentheadpos = dv;
        uc++;
    }

    else if((strcasecmp(name, "right")==0)  ||
            (strcasecmp(name, "left")==0)   ||
            (strcasecmp(name, "up")==0)     ||
            (strcasecmp(name, "down")==0)   ||

            (strcasecmp(name, "from")==0)   ||
            (strcasecmp(name, "to")==0)     ||

            (strcasecmp(name, "forward")==0)||
            (strcasecmp(name, "incdir")==0) ||
            (strcasecmp(name, "decdir")==0) ||
            (strcasecmp(name, "lturn")==0)  ||
            (strcasecmp(name, "rturn")==0)  ||
            (strcasecmp(name, "dir")==0)    ||
            (strcasecmp(name, "arc")==0)    ||
            (strcasecmp(name, "arcn")==0)   ||
            (strcasecmp(name, "curve")==0)  ||
            (strcasecmp(name, "join")==0)   ||
            (strcasecmp(name, "skip")==0)   ||
#if 1
            (strcasecmp(name, "markcir")==0)    ||
            (strcasecmp(name, "markbox")==0)    ||
            (strcasecmp(name, "markxss")==0)    ||
            (strcasecmp(name, "markcross")==0)  ||
            (strcasecmp(name, "markpls")==0)    ||
            (strcasecmp(name, "markplus")==0)   ||
#endif

            (strcasecmp(name, "close")==0)  ||
            (strcasecmp(name, "then")==0)   ) {
        char *j;
        int   dt;
        dt = assoc(objattr_ial, name);
#if 1
Info("sub-command '%s' %d\n", name, dt);
#endif
        j = parse_segop(p, dt, rob);
        if(j) {
#if 1
Info("sub-command '%s' argument '%s'\n", name, p);
#endif
            p = j;

        }
        else {
#if 1
Info("sub-command '%s' argument skip\n", name);
#endif
        }
        uc++;
    }
    else if((k1=assoc(pos_ial, name))>=0) {
        rob->cob.iarg1 = k1;        
        uc++;
    }
    else if(oak==OA_KEEPDIR) {
        rob->cob.keepdir = 1;
        uc++;
    }
    else if(oak==OA_MARKPITCH) {
        rob->cob.markpitch = 1;
        uc++;
    }
    else if(oak==OA_MARKNODE) {
        rob->cob.marknode = 1;
        uc++;
    }
    else if(oak==OA_MARKBB) {
        rob->cob.markbb = 1;
        uc++;
    }
    else if(oak==OA_MARKGUIDE) {
        rob->cob.markguide = 1;
        uc++;
    }
    else if(oak==OA_MARKPATH) {
        rob->cob.markpath = 1;
        uc++;
    }
    else if(oak==OA_MARKGUIDE) {
        rob->cob.markguide = 1;
        uc++;
    }
#if 0
    else if(oak==OA_HOLLOW) {
        rob->cob.hollow = 1;
        uc++;
    }
#endif
    else if(oak==OA_ARROWEVERY) {
        rob->cob.arrowevery = 1;
        uc++;
    }
    else if(oak==OA_ARROWEDGE) {
        rob->cob.arrowevery = 0;
        uc++;
    }
    else if(oak==OA_NOEXPAND) {
        rob->cob.noexpand = 1;
        uc++;
    }

    if(uc==0) {
        Echo("WARNING unknown object attribute '%s' [line %d]\n", name, lineno);
    }

    if(gc>0) {
        recalcsizeparam();
    }


#if 0
Echo("k1 %d\n", k1);
#endif

#if 0
P;
    fprintf(stdout, "segopar\n");
    varray_fprintv(stdout, rob->cob.segopar);
#endif

out:
    return p;
}


ob*
parseobj(char *p)
{
    int ot;
    ob* nob;
    char *u;
    char *oldp;

    oldp = p;

#if 1
    if(!*p) {
        return NULL;
    }
#endif

    u = parsetype(&ot, p);
    if(ot==-1) {
        Echo("ERROR unknow syntax/command or object\n");
        return NULL;
    }
    nob = newobj();
    if(!nob) {
        Echo("ERROR no memory\n");
        return NULL;
    }

    nob->type = ot;

    /* common init */
    nob->cob.outlinecolor   = def_fgcolor;
    nob->cob.outlinethick   = def_linethick;
    nob->cob.fillcolor      = def_fgcolor;
    nob->cob.fillhatch      = HT_NONE;
    nob->cob.fillpitch      = def_hatchpitch;
    nob->cob.fillthick      = def_hatchthick;
    nob->cob.backcolor      = def_bgcolor;
    nob->cob.backhatch      = HT_NONE;
    nob->cob.backpitch      = def_hatchpitch;
    nob->cob.backthick      = def_hatchthick;
#if 0
    nob->cob.hatchpitch     = def_hatchpitch;
    nob->cob.hatchthick     = def_hatchthick;
#endif

    nob->cob.slittype       = OA_NOSLIT;
    nob->cob.slitpos        = 70;
    nob->cob.slitthick      = 10;

    nob->cob.textcolor      = def_fgcolor;
    nob->cob.textbgcolor    = def_bgcolor;
    nob->cob.textposition   = PO_CENTER;
    nob->cob.texthoffset    = 0;
    nob->cob.textvoffset    = 0;
    nob->cob.textrotate     = 0;
    nob->cob.decocolor      = -1;

    nob->cob.arrowevery     = 0;

    nob->cob.crankpos       = 50;

    nob->cob.wlinethick     = def_wlinethick;
    
    /* for automatic, set negative size as unsolved */
    nob->wd                 = -1;
    nob->ht                 = -1;
    nob->sizesolved         = 0;

    /* if need special procees, do that */
    switch(ot) {
    case CMD_CHUNK:
    case CMD_CHUNKOBJATTR:
Echo("CHECK CHUNK attr\n");
        nob->cob.fillhatch          = HT_NONE;
        nob->cob.fillcolor          = -1;
        nob->cob.outlinethick       = -1;
        break;
    case CMD_SAVE:
    case CMD_RESTORE:
    case CMD_PUSH:
    case CMD_POP:
    case CMD_AGAIN:
    case CMD_BACK:
    case CMD_HBACK:
        nob->invisible = 1;
        break;

#if 0
    case CMD_AUXLINE:
        nob->cob.auxlinetype       = ALT_ARROW;
#endif
    case CMD_ARROW:
    case CMD_WARROW:
        nob->cob.arrowheadpart     = AR_FORE;
        nob->cob.arrowforeheadtype = AH_NORMAL;
        break;

    case CMD_XCURVE:
    case CMD_XCURVESELF:
    case CMD_BCURVE:
    case CMD_BCURVESELF:
        nob->cob.bulge              = 20;
        if(ot==CMD_XCURVESELF||
            ot==CMD_BCURVESELF) {
            nob->cob.arrowcentheadpos   = 0.98;
            nob->cob.rad = (objunit*3)/2;
        }
        else {
            nob->cob.arrowcentheadpos   = 0.5;
        }
        break;

    case CMD_THUNDER:
        nob->cob.bulge              = 10;
        break;

    case CMD_LINE:
        nob->cob.keepdir = 0;
        break;
    case CMD_ULINE:
        nob->cob.keepdir = 1;
        break;

    case CMD_DIR:
    case CMD_INCDIR:
    case CMD_DECDIR:
    case CMD_LTURN:
    case CMD_RTURN:
    case CMD_DOWN:
    case CMD_RIGHT:
    case CMD_UP:
    case CMD_LEFT:
        nob->invisible = 1;
        break;

    case CMD_NOTEFILE:
        nob->invisible = 1;
        break;

    case CMD_TRACEON:
    case CMD_TRACEOFF:
        nob->invisible = 1;
        break;

    default:
        break;
    }

    do {
        while(*u && (*u==' '||*u=='\t')) u++;

        if(*u=='#') {
            break;
        }
    
        u = parseobjattr(nob, u);
#if 0
Echo("u %p\n", u);
fflush(stdout);
#endif
        if(!u) break;
        if(*u=='\0') break;
    } while(1);

#if 0
    if(nob->cob.segopar && nob->cob.segopar->use>0) {
P;
        fprintf(stdout, "segopar\n");
        varray_fprintv(stdout, nob->cob.segopar);
    }
#endif


    if(nob->type==CMD_NOTEFILE) {
        if(nob->cob.carg1) {
            Echo("got note filename |%s|\n", nob->cob.carg1);
            nf_registfilename(nob->cob.carg1);
        }
        else {
            fprintf(stderr, "no filename\n");
        }
    }


#define PV(x) if((x) && (x)->use) varray_fprint(stdout, (x));

#if 0
    dump_objattr(nob->cob);
#endif

    confirm_objattr(&nob->cob);
    confirm_attr(nob);

#if 0
    dump_objattr(nob->cob);
#endif

#if 0
    Echo("; %s\n", oldp);
#endif
#if 0
    PV(nob->cob.ssar);
#endif
#if 0
    PV(nob->cob.segopar);
#endif

    if(nob->cob.originalshape) {
        segop *ent;
        ent = (segop*)varray_findlast(nob->cob.segopar);    
        if(ent) {
            if(ent->cmd!=OA_THEN) {
                /* add one more entry 'THEN' */
                ent = (segop*)malloc(sizeof(segop));
                if(ent) {
                    ent->cmd = OA_THEN;
                    ent->val = strdup("");
                    varray_push(nob->cob.segopar, ent);
                }
                else {
                    Error("%s: no memory\n", __func__);
                }
            }
        }
    }
#if 0
    PV(nob->cob.segopar);
#endif

#if 1
    /* XXX */
    if(nob->type==CMD_UNIT && nob->cob.iarg1>0) {
        objunit = nob->cob.iarg1; recalcsizeparam();
    }
#endif

    return nob;
}


static char ibuf[BUFSIZ]="";

int
getcmd(FILE *fp, char *ebuf, int elen)
{
    char  tmp[BUFSIZ];
    char *p, *q;
    char *ck;
    int   ol, nl;
    int   u;


    p = ibuf;
    q = ebuf;
    u = 0;
    while(1) {
        if(!*p) {
            ck = fgets(tmp, BUFSIZ, fp);
            lineno++;
            if(ck==NULL) {
#if 0
                fEcho(stderr, "EOF\n");
#endif
                return 1;
                break;
            }
            else {
                ol = strlen(ibuf);  
                nl = strlen(tmp);
                if(ol+nl+1>=BUFSIZ) {
                    Echo("ERROR too long line [line %d]\n", lineno);
                    exit(7);
                }
                strcat(ibuf, tmp);
                continue;
            }
        }
        if(*p=='\\' && *(p+1)=='\n') {
            p++;
            p++;
#if 0
            sdump(stderr, "k ibuf", ibuf);
            sdump(stderr, "k ebuf", ebuf);
#endif
        }
#if 0
        else
        if(*p=='\\') {
            int nv = -1;
P;
#if 0
            p++;
            *q++ = *p++;
#endif
            sdumpNZ(stderr, "0p", p, 16);
            sdumpNZ(stderr, "0q", q, 16);
            sdumpNZ(stderr, "0e", ebuf, 16);
            switch(*(p+1)) {
            case 'n':   nv = '\n';      break;
            case 'r':   nv = '\r';      break;
            }
            if(nv>0) {
P;
                *q++ = nv;
                p++;
                p++;
            }
            else {
P;
                *q++ = *p++;
            }
            sdumpNZ(stderr, "1p", p, 16);
            sdumpNZ(stderr, "1q", q, 16);
            sdumpNZ(stderr, "1e", ebuf, 16);
        }
#endif
        else
        if(*p=='"') {
            u++;
            *q++ = *p++;
        }
        else {
            if(u%2==0&&*p==';') {
                p++;
                break;
            }
            else
            if(*p=='\n') {
                *q++ = *p++;
                break;
            }
            else {
                *q++ = *p++;
            }
        }
    }
    
    *q = '\0';

#if 0
  sdump(stderr, "b ibuf", ibuf);
  sdump(stderr, "b ebuf", ebuf);
#endif

    delfirstchars(ibuf, p-ibuf);

#if 0
  sdump(stderr, "a ibuf", ibuf);
  sdump(stderr, "a ebuf", ebuf);
#endif

    cmdno++;

    return 0;
}

typedef struct {
    char src[BUFSIZ];
    char dst[BUFSIZ];
} alias_cell;


int
aliasprintf(char *dst, int dlen, void* v, int opt)
{
    alias_cell *m;
    m = (alias_cell*)v;
    sprintf(dst, "(\"%s\"->\"%s\")", m->src, m->dst);
    return 0;
}

int
alias_add(varray_t *ar, char *xsrc, char *xdst)
{
    alias_cell *c;

    c = (alias_cell*)malloc(sizeof(alias_cell));
    if(!c) {
        Error("%s: no memory\n", __func__);
        return -1;
    }
    memset(c, 0, sizeof(alias_cell));

    strcpy(c->src, xsrc);
    strcpy(c->dst, xdst);

    varray_push(ar, c);

    return 0;
}

char*
alias_find(varray_t *ar, char *xsrc)
{
    int i;
    int pos;
    alias_cell *c;

    pos = -1;
    for(i=0;i<ar->use;i++) {
        c = (alias_cell*)ar->slot[i];
        if(c && c->src[0] && strcasecmp(c->src, xsrc)==0) {
            pos = i;
        }
    }

    if(pos>=0) {
        c = (alias_cell*)ar->slot[pos];
        return c->dst;
    }

    return NULL;
}

int
try_alias(varray_t *ar, char *orig, char *ns, int nlen)
{
    char *u;
    char  first[BUFSIZ];
    char  second[BUFSIZ];
    char  third[BUFSIZ];
    char *d;
    char *q;

#if 0
Echo("%s: orig |%s|\n", __func__, orig);
#endif

    u = orig;
    u = draw_wordW(u, first, BUFSIZ);
    if(!first[0]) {
        return 0;
    }

#if 0
Echo("  first '%s'\n", first);
#endif
    if(strcasecmp(first, "alias")==0) {
        while(*u && (*u==' ' || *u=='\t')) {
            u++;
        }
        
        u = draw_wordW(u, second, BUFSIZ);

        while(*u && (*u==' ' || *u=='\t')) {
            u++;
        }

#if 0
Echo(" 1 *u '%c'\n", *u);
Echo(" 2 *u '%c'\n", *u);
Echo(" 3 *u '%c'\n", *u);
#endif
        
        q = third;
        if(*u=='"') {
            u++;
            while(*u && (*u!='"') ) {
                if(*u=='\\') {
                    u++;
                }
                *q++ = *u++;
            }
            *q = '\0';
        }
        else {
            while(*u) {
                *q++ = *u++;
            }
            *q = '\0';
        }
#if 0
Echo("  second'%s'\n", second);
Echo("  third '%s'\n", third);
#endif
        alias_add(ar, second, third);
#if 0
        varray_fprint(stdout, ar);
#endif
        return 2;
    }
    else {

        d = alias_find(ar, first);
        if(d) {
            strcpy(ns, d);
            strcat(ns, " ");
            strcat(ns, u);

#if 1
Echo("%s: ns   |%s|\n", __func__, ns);
#endif
            return 1;
        }

        return 0;
    }

    return -1;
}

int
Xnss_verify()
{
    extern int nss_n;
    if(nss_n<0) {
        Info("nss does not used\n");
        return 0;
    }
    else 
    if(nss_n == 0) {
        Info("nss maybe used and safe\n");
        return 0;
    }
    else {
        Warn("nss maybe used and ignore value (nss_n %d) check ]\n", nss_n);
        return 1;
    }
    return 0;
}

int
Xchs_verify()
{
    extern int chs_n;
    if(chs_n<0) {
        Info("chs does not used\n");
        return 0;
    }
    else 
    if(chs_n == 0) {
        Info("chs maybe used and safe\n");
        return 0;
    }
    else {
        Warn("chs maybe used and ignore value (chs_n %d) check >\n", chs_n);
        return 1;
    }
    return 0;
}

int
parse(FILE *fp, ob* ch0, ns *ns0) 
{
    ob*   curch;
    ns*   curns;
    char  line[BUFSIZ];
    char  label[BUFSIZ];
    char *p;
    int   ot;
    int   flag;
    int   do_rotate;
    int   ik;
    int   tmpv;
    char  tmps[BUFSIZ];

    varray_t *al;
    char  rep[BUFSIZ];

    al = varray_new();
    varray_entrysprintfunc(al, aliasprintf);


    curch = ch0;
    curns = ns0;
    ch0->cch.qns = ns0;
#if 1
    ch0->cch.qob = ch0;
#endif

    lineno = 0;
    cmdno  = 0;

    while(1) {
        ik = getcmd(fp, line, BUFSIZ);
#if 1
Echo("getcmd ik %d |%s|\n", ik, line);
#endif
        if(ik) {
#if 0
Echo("getcmd ik %d |%s|\n", ik, line);
#endif
            break;
        }

#if 0
        Echo("\t\t\tcurns %p, curch %p\n", curns, curch);
        Echo("; %s", line);
#endif
    
        dellastcr(line);

#if 1
        Echo(";; %3d %3d; %s\n", cmdno, lineno, line);
#endif
        if(line[0]=='\0') {
            continue;
        }


        label[0] = '\0';
        p = line;
        p = skiplabel(label, BUFSIZ, p);
        while(*p && (*p==' ' || *p=='\t')) {
            p++;
        }
        if(!*p) {
            continue;
        }
        if(*p=='#') {
            continue;
        }
        if(*p=='\r'||*p=='\n') {
            continue;
        }

        /***
         *** try to replace by alias 
         ***/
        ik = try_alias(al, p, rep, BUFSIZ);
        if(ik==1) {
            int d;
            d = p - line;
            strcpy(p, rep);

#if 0
Echo("new line |%s|\n", line);
Echo("  line %p, p %p, p-line %d\n", line, p, (int)((char*)p-(char*)line));
#endif
        }
        if(ik==2) {
            continue;
        }

#if 0
Echo("new line |%s|\n", line);
sdump(stdout, "raw line ", line);
Echo("new p |%s|\n", p);
sdump(stdout, "raw p ", p);
#endif

        if(*p=='\\') {

        }
        else
        if(*p=='@') {
        }
        else
        /***
         ***/
        if(*p=='{') {
            ns* nns;

            nss_push(curns);
            nns = newnamespace_child(curns);
            if(label[0]) {
                nsaddns(curns, nns, label);
            }
            else {
                nsaddns(curns, nns, "nolabel");
            }
            curns = nns;
        }
        else
        if(*p=='}') {
            ns* ons;

            ons = nss_pop();
            curns = ons;
        }
        else
        /***
         ***/
        if(*p=='[') {
            ns* nns;
            ob* nch;
            ob* nob;

            chs_push(curch);
            nch = newchunk_child(curch);
            chadd(curch, nch);
            if(label[0]) {
                nsaddobj(curns, nch, label);
            }
#if 0
#endif
            curch = nch;

            nss_push(curns);
            nns = newnamespace_child(curns);
            if(label[0]) {
#if 0
                nsaddns(curns, nns, label);
#endif
                nsaddnsobj(curns, nns, nch, label);
            }
            else {
                nsaddnsobj(curns, nns, nch, "nolabel");
            }
            curns = nns;

            curch->cch.qns = nns;

        }
        else
        if(*p==']') {
            ns* ons;
            ob* och;

            ons = nss_pop();
            curns = ons;

            och = chs_pop();
            curch = och;
        }
        else
        /***
         ***/
        if(*p=='<') {
            ob* nch;

            chs_push(curch);
            nch = newchunk_child(curch);
            chadd(curch, nch);
            if(label[0]) {
                nsaddobj(curns, nch, label);
            }
            curch = nch;

            curch->cch.qns = curns;
        }
        else
        if(*p=='>') {
            ob* och;

            och = chs_pop();
            curch = och;
        }
#if 1
        else
        if(strncasecmp(p, "namedcolor", 10)==0) {
            char  xcmd[BUFSIZ];
            char  xname[BUFSIZ];
            char  xvalue[BUFSIZ];
            char *k;
            int   ik;
            color_t *cpos;
            int   vr, vg, vb;

            k = p;
            k = draw_wordW(k, xcmd, BUFSIZ);
            k = draw_wordW(k, xname, BUFSIZ);
            k = draw_wordW(k, xvalue, BUFSIZ);
#if 0
            fprintf(stderr, "xcmd |%s| xname |%s| xvalue |%s|\n",
                xcmd, xname, xvalue);
#endif
            
            ik = parsergb(xvalue, &vr, &vg, &vb);
            if(ik>=0) {
                cpos = pallet_addnamedcolor(pallet, xname, vr, vg, vb);
            }
            else {
#if 1
                fprintf(stderr, "ik %d by for namedcolor\n", ik);
#endif
                cpos = pallet_addnamedneg(pallet, xname);
            }
        }
#endif
        else {
    /*****
     *****
     ***** regular object parsing 
     *****
     *****/
            ob* nch;
            ob* och;
            ob* nob;
            ob *curfork;
            int kkh;

            nob = parseobj(p);
            if(!nob) {
Error("not found object '%s'\n", p);
                break;
            }

            chadd(curch, nob);

            if(label[0]) {
                nsaddobj(curns, nob, label);
            }

            if(nob->type==CMD_EXIT) {
                break;
            }

            /*** XXX ***/
            if(nob->type==CMD_CHUNKOBJATTR) {
                Echo("curch oid %d copy from nob oid %d\n",
                    curch->oid, nob->oid);
                curch->vob = nob->vob;
                chdel(curch, nob);

                goto skip_specialforms;
            }

#if 0
            /*** XXX ***/
            if(nob->type==CMD_CHUNKCHILDATTR) {
                Echo("curch oid %d copy from nob oid %d\n",
                    curch->oid, nob->oid);
                curch->vob = nob->vob;
                chdel(curch, nob);

                goto skip_specialforms;
            }
#endif

#if 0
            curfork = NULL;
            if(nob->cob.afrom) {
                curfork = ns_find_obj(ns0, nob->cob.afrom);
                if(!curfork) {
                    Echo("\tcurfork %p\n", curfork);
                    Echo("ERROR not found fork '%s'\n", nob->cob.afrom);
                }
                else {
                    kkh = curfork->cob.forkhasbranch;
                }
            }
#endif

#if 0
    Echo("\tcurfork %p oid %d\n", curfork, curfork->oid);
    curfork->cob.forkbranchmem[curfork->cob.forkhasbranch] = nob;
    curfork->cob.forkhasbranch++;

    kkh = curfork->cob.forkhasbranch;
#endif

skip_specialforms:
            (void)0;
        }
    }
    
#if 1
    ob_dump(ch0);
    ns_dump(ns0);
#endif
#if 0
    extern int nss_n, chs_n;
    printf("nss_n %d\n", nss_n);
    printf("chs_n %d\n", chs_n);
#endif
#if 1
    int vk;
    vk = nss_verify();
    vk = chs_verify();
#endif

    return 0;
}




int
fparse(char *fn, ob* ch0, ns *ns0) 
{
    FILE *fp;
    int   ik;
    
    fp = fopen(fn, "r");
    if(fp==NULL) {
        Error("cannot open file %s\n", fn);
        return -1;
    }
    ik = parse(fp, ch0, ns0);
    fclose(fp);
    return ik;
}

