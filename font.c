#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alist.h"
#include "font.h"
#include "word.h"

double akratio = 0.95;

apair_t fm_ial[] = {
    {"none",    FM_NONE},
    {"ascii",   FM_ASCII},
    {"kanji",   FM_KANJI},
    {"all",     FM_ALL},
    {NULL,      -1},
};

apair_t fh_ial[] = {
    {"none",    FH_NONE},
    {"normal",  FH_NORMAL},
    {"large",   FH_LARGE},
    {"huge",    FH_HUGE},
    {"small",   FH_SMALL},
    {"tiny",    FH_TINY},
    {NULL,      -1},
};

apair_t fh_val_ial[] = {
    {"1.0",     FH_NONE},
    {"1.0",     FH_NORMAL},
    {"1.414",   FH_LARGE},
    {"2.0",     FH_HUGE},
    {"0.707",   FH_SMALL},
    {"0.5",     FH_TINY},
    {NULL,      -1},
};

apair_t ff_ial[] = {
    {"none",        FF_NONE},
    {"serif",       FF_SERIF},
    {"sanserif",    FF_SANSERIF},
    {"italic",      FF_ITALIC},
    {"type",        FF_TYPE},
    {"all",         FF_ALL},
    {NULL,          -1},
};

apair_t fa_ial[] = {
    {"none",        FA_NONE},
    {"fontname",    FA_FONTNAME},
    {"name",        FA_FONTNAME},
    {"encode",      FA_ENCODE},
    {"scale",       FA_SCALE},
    {NULL,          -1},
};

int
wipe_Xal(apair_t *ls, char *xn)
{
    int i;
    int q;

    i = 0;
    while(ls[i].name) {
        if(ls[i].value>=0) {
            ls[i].name = strdup(xn);
            q++;
        }
        i++;
    }

    return q;
}

int
swap_Xal(apair_t *ls, int xv, char *xn)
{
    int   i;
    int   q=0;

    if(xv<0 || !xn) {
        return -1;
    }

    i = 0;
    while(ls[i].name) {
        if(ls[i].value==xv) {
            ls[i].name = strdup(xn);
            q++;
            break;
        }
        i++;
    }

    if(q) {
        return q;
    }
    else {
        return 0;
    }
}

extern int QLadd(char *dst, int dlen, char *xit, int sep);
char *ext_fontnamelist=NULL;
char *ext_fontfilelist=NULL;

int
parse_type42_fontname(char *filename, char *fontname, int fontnamelen)
{
    int   r;
    FILE *fp;
    char  line[BUFSIZ];
    char *p;
    char *q;
    int   c;

    r = -1;

    fp = fopen(filename, "r");
    if(!fp) {
        fprintf(stderr, "fail open '%s'\n", filename);
        goto out;
    }

    while(fgets(line, BUFSIZ, fp)) {
        chomp(line);
        p = line;
        p = skipwhite(p);
        if(strncmp(p, "/FontName", 9)==0) {
            p += 9;
            p = skipwhite(p);
#if 0
            fprintf(stderr, "p |%s|\n", p);
#endif
            if(*p=='/') {
                p++;
                q = fontname;
                c = 0;
                while(*p && *p!=' ' && c<fontnamelen) {
                    *q++ = *p++;
                    c++;
                }
                *q = '\0';
                r = 0;
            }
#if 0
            fprintf(stderr, "fontname |%s|\n", fontname);
#endif
        }
    }
    
    fclose(fp);
    
out:
    return r;
}

int
al_fprint(FILE *fp, apair_t *pl)
{
    int i;

#if 1
    fprintf(stderr, "%s: %p %p\n", __func__, fp, pl);
#endif

    i = 0;
    while(pl[i].name) {
        fprintf(fp, "%3d: %s %d\n", i, pl[i].name, pl[i].value);
        i++;
    }

    return 0;
}



/*
 * renewal of fonts
 */

int
tgyfont_fdump(FILE *fp, tgyfont_t *tf, char *pmsg)
{
    char *qname = "*none*";
    char *qenc  = "*none*";

    if(pmsg) {
        fprintf(fp, "%s ", pmsg);
    }

    if(tf->fname)   { qname = tf->fname; }
    if(tf->fencode) { qenc  = tf->fencode; }

    fprintf(fp, "%p ", tf);
    fprintf(fp, "%2d:%-5s %2d:%-8s %-12s %-12s %5.2f %p\n",
        tf->fmode, 
        rassoc(fm_ial, tf->fmode),
        tf->fface, 
        rassoc(ff_ial, tf->fface), 
        qname, qenc,
        tf->fscale,
        (void*)tf->fcq);
    fflush(fp);

    return 0;
}



int
tgyfont_fprint(FILE *fp, tgyfont_t *tf, char *pmsg)
{
    char *qname = "*none*";
    char *qenc  = "*none*";

    if(pmsg) {
        fprintf(fp, "%s ", pmsg);
    }

    if(tf->fname)   { qname = tf->fname; }
    if(tf->fencode) { qenc  = tf->fencode; }

#if 0
    fprintf(fp, "%p ", tf);
#endif
#if 0
    fprintf(fp, "<%d %d %p %p %p> ",
        tf->fmode, tf->fface, tf->fname, tf->fencode, (void*)tf->fcq);
#endif

#if 0
    fprintf(fp, "%2d:%-5s %2d:%-8s %-12s %-12s %p\n",
        tf->fmode, 
        rassoc(fm_ial, tf->fmode),
        tf->fface, 
        rassoc(ff_ial, tf->fface), 
        qname, qenc, (void*)tf->fcq);
#endif

#if 1
    fprintf(fp, "%-5s %-8s %-12s %-12s %5.2f\n",
        rassoc(fm_ial, tf->fmode),
        rassoc(ff_ial, tf->fface), 
        qname, qenc, tf->fscale);
#endif

    fflush(fp);

    return 0;
}


int tgyfontset_use = -1;
int tgyfontset_max = -1;
tgyfont_t *tgyfontset = NULL;

int
tgyfontset_fprint(FILE *fp, char *pmsg)
{
    int i;
    tgyfont_t *ctf;

    if(pmsg && *pmsg) {
        fprintf(fp, "%s\n", pmsg);
    }
    fprintf(fp, "fontset use/max %d/%d default-max %d\n",
        tgyfontset_use, tgyfontset_max, TGYFONTSET_N);
    for(i=0;i<tgyfontset_use;i++) {
        ctf = &tgyfontset[i];
        fprintf(fp, "%2d: ", i);
        tgyfont_fprint(fp, ctf, NULL);
    }
    return 0;
}

int
tgyfontset_fdump(FILE *fp, char *pmsg)
{
    int i;
    tgyfont_t *ctf;

    if(pmsg && *pmsg) {
        fprintf(fp, "%s\n", pmsg);
    }
    fprintf(fp, "fontset-dump use/max %d/%d default-max %d\n",
        tgyfontset_use, tgyfontset_max, TGYFONTSET_N);
    for(i=0;i<tgyfontset_max;i++) {
        ctf = &tgyfontset[i];
        fprintf(fp, "%2d: ", i);
        tgyfont_fdump(fp, ctf, NULL);
    }
    return 0;
}


int
tgyfontset_put(int xpos, int xmode, int xface, char *xname, char *xenc)
{
    tgyfont_t *tf;

    if(xpos>=tgyfontset_max) {
        printf("ERROR %s invalid position %d\n", __func__, xpos);
        return -1;
    }
    
    tf = &tgyfontset[xpos];
    tf->fmode   = xmode;
    tf->fface   = xface;
#if 1
    tf->fname   = strdup(xname);
    tf->fencode = strdup(xenc);
#endif
#if 1
    tf->fname   = xname;
    tf->fencode = xenc;
#endif
    tf->fcq     = (void*)0;

    return 0;
}

int
tgyfontset_add(int xmode, int xface, char *xname, char *xenc)
{
    int ik;
    if(tgyfontset_use>=tgyfontset_max) {
        printf("ERROR no space to newfont\n");
        return -1;
    }
    ik = tgyfontset_put(tgyfontset_use, xmode, xface, xname, xenc);
    if(ik==0) {
        tgyfontset_use++;
    }
    return ik;
}

tgyfont_t*
tgyfontset_find(int xmode, int xface, void *opt)
{
    tgyfont_t* rv;
    tgyfont_t* ctf;
    int pos;
    int i;

    rv = NULL;
    pos = -1;

    for(i=0;i<tgyfontset_use;i++) {
        ctf = &tgyfontset[i];
        if(ctf->fmode==xmode && ctf->fface==xface) {
            pos = i;
            rv = ctf;
            break;  /* first match */
        }
    }
    
    return rv;
}

char*
tgyfont_resolv_fontname(int xmode, int xface)
{
    tgyfont_t* tf;
    tf = tgyfontset_find(xmode, xface, NULL);
    if(tf) {
        return tf->fname;
    }
    else {
        return NULL;
    }
}

char*
tgyfont_resolv_encode(int xmode, int xface)
{
    tgyfont_t* tf;
    tf = tgyfontset_find(xmode, xface, NULL);
    if(tf) {
        return tf->fencode;
    }
    else {
        return NULL;
    }
}

double
tgyfont_resolv_scale(int xmode, int xface)
{
    tgyfont_t* tf;
    tf = tgyfontset_find(xmode, xface, NULL);
    if(tf) {
        return tf->fscale;
    }
    else {
        return (double)-1;;
    }
}

int
tgyfont_wipe_fontnameMF(int xmode, int xface, char *xval)
{
    tgyfont_t* ctf;
    int i;
    int c;

    for(i=0;i<tgyfontset_use;i++) {
        ctf = &tgyfontset[i];
        if(ctf->fmode==xmode) {
            if(xface==FF_ALL || ctf->fface==xface) {
                ctf->fname = strdup(xval);
                c++;
            }
        }
    }
    
    return c;
}

int
tgyfont_wipe_encodeMF(int xmode, int xface, char *xval)
{
    tgyfont_t* ctf;
    int i;
    int c;

    for(i=0;i<tgyfontset_use;i++) {
        ctf = &tgyfontset[i];
        if(ctf->fmode==xmode) {
            if(xface==FF_ALL || ctf->fface==xface) {
                ctf->fencode = strdup(xval);
                c++;
            }
        }
    }
    
    return c;
}

int
tgyfont_wipe_scaleMF(int xmode, int xface, double xval)
{
    tgyfont_t* ctf;
    int i;
    int c;

    for(i=0;i<tgyfontset_use;i++) {
        ctf = &tgyfontset[i];
        if(ctf->fmode==xmode) {
            if(xface==FF_ALL || ctf->fface==xface) {
                ctf->fscale = xval;
                c++;
            }
        }
    }
    
    return c;
}

int
tgyfont_wipe_fontnameM(int xmode, char *xval)
{
    return tgyfont_wipe_fontnameMF(xmode, FF_ALL, xval);
}

int
tgyfont_wipe_encodeM(int xmode, char *xval)
{
    return tgyfont_wipe_encodeMF(xmode, FF_ALL, xval);
}

int
tgyfont_wipe_scaleM(int xmode, double xval)
{
    return tgyfont_wipe_scaleMF(xmode, FF_ALL, xval);
}

int
tgyfont_edit(int cat, char *opseq)
{
    int   r;
    char *p;
    char *u;
    char *w;
    char *m;
    char *e;
    char *g;
    char  pair[BUFSIZ];
    char  key[BUFSIZ];
    char  val[BUFSIZ];

    char  mstr[BUFSIZ], fstr[BUFSIZ], astr[BUFSIZ];

    char  func[BUFSIZ];
    char  fontfilename[BUFSIZ];
    char  fontname[BUFSIZ];
    int   fm;
    int   ff;
    int   fa;
    int   ik;

    r = -1;
#if 0
    fprintf(stderr, "%s: cat %d opseq |%s|\n", __func__, cat , opseq);
#endif

#if 0
    tgyfontset_fprint(stderr, "before edit");
#endif

    if(!ext_fontnamelist)  {
        ext_fontnamelist = malloc(sizeof(char)*BUFSIZ);
        if(!ext_fontnamelist) {
            fprintf(stderr, "no memory for ext_fontnamelist\n");
            goto out;
        }
        memset(ext_fontnamelist, 0, BUFSIZ);
    }

    if(!ext_fontfilelist)  {
        ext_fontfilelist = malloc(sizeof(char)*BUFSIZ);
        if(!ext_fontfilelist) {
            fprintf(stderr, "no memory for ext_fontfilelist\n");
            goto out;
        }
        memset(ext_fontfilelist, 0, BUFSIZ);
    }

    /*
     *  opseq := pair[;pair]
     *  pair := fm[.ff][.fa]=val1,val2
     *            period   comma
     */

    p = opseq;
    while(*p) {
        p = skipwhite(p);
        p = draw_word(p, pair, BUFSIZ, ';');
        if(!pair[0]) {
            break;
        }
#if 0
        fprintf(stderr, "  pair |%s|\n", pair);
#endif
    
        u = pair;
        u = skipwhite(u);
        u = draw_word(u, key, BUFSIZ, '=');
        strcpy(val, u);

#if 0
        fprintf(stderr, "  key  |%s|\n", key);
        fprintf(stderr, "  val  |%s|\n", val);
#endif

        if(strcasecmp(key, "default")==0) {
            def_fontspec = strdup(val);
            continue;
        }

        
        fm = -1;
        ff = -1;
        fa = FA_FONTNAME;

        w = key;
        w = draw_word(w, mstr, BUFSIZ, '.');
        w = draw_word(w, fstr, BUFSIZ, '.');
        strcpy(astr, w);

#if 0
        fprintf(stderr, "    mstr |%s|\n", mstr);
        fprintf(stderr, "    fstr |%s|\n", fstr);
        fprintf(stderr, "    astr |%s|\n", astr);
#endif

        if(astr[0]) {
            fm = assoc(fm_ial, mstr);
            ff = assoc(ff_ial, fstr);
            fa = assoc(fa_ial, astr);
        }
        else
        if(fstr[0]) {
            fm = assoc(fm_ial, mstr);
            ff = assoc(ff_ial, fstr);
        }
        else {
            int _m, _f;
            _m = assoc(fm_ial, mstr);
            _f = assoc(ff_ial, mstr);
            if(_m>=0) {
                fm = _m;
                ff = FF_ALL;
            }
            else
            if(_f>=0) {
                fm = FM_ALL;
                ff = _f;
            }
        }
        if(ff<0) {
            ff = FF_SERIF;
        }

#if 0
        fprintf(stderr, "    fm/ff/fa = %d/%d/%d\n", fm, ff, fa);
#endif


#if 0
        fprintf(stderr, "  val* |%s|\n", val);
#endif

        if(fa==FA_SCALE) {
            double sv;
            sv = atof(val);
            if(fm==FM_ALL) {
                ik = tgyfont_wipe_scaleMF(FM_ASCII, ff, sv);
                ik = tgyfont_wipe_scaleMF(FM_KANJI, ff, sv);
            }
            else {
                ik = tgyfont_wipe_scaleMF(fm, ff, sv);
            }
        }
        else
        if(fa==FA_ENCODE) {
            if(fm==FM_ALL) {
                ik = tgyfont_wipe_encodeMF(FM_ASCII, ff, val);
                ik = tgyfont_wipe_encodeMF(FM_KANJI, ff, val);
            }
            else {
                ik = tgyfont_wipe_encodeMF(fm, ff, val);
            }
        }
        else
        if(fa==FA_FONTNAME) {
            if(fm==FM_ALL) {
                ik = tgyfont_wipe_fontnameMF(FM_ASCII, ff, val);
                ik = tgyfont_wipe_fontnameMF(FM_KANJI, ff, val);
            }
            else {
                ik = tgyfont_wipe_fontnameMF(fm, ff, val);
            }
        }
        else {
            /* nothing */
        }


        {
            m = val;
            m = skipwhite(m);
            if(strncasecmp(m, "file(", 5)==0) {
                e = m+5;
                e = skipwhite(e);       
                g = fontfilename;
                while(*e && *e!=')') {
                    *g++ = *e++;
                }
                *g = '\0';
                ik = parse_type42_fontname(fontfilename, fontname, BUFSIZ);
                if(ik==0) {
                    if(fontname[0]) {
                        strcpy(val, fontname);

                        if(ext_fontnamelist)  {
                            QLadd(ext_fontnamelist, BUFSIZ, fontname, ',');
                        }
                        if(ext_fontfilelist)  {
                            QLadd(ext_fontfilelist, BUFSIZ, fontfilename, ',');
                        }
                    }
                }
            }

        }

    }

#if 0
    tgyfontset_fprint(stderr, "after  edit");

    if(ext_fontnamelist) {
        fprintf(stderr, "ext_fontnamelist |%s|\n", ext_fontnamelist);
    }
    if(ext_fontfilelist) {
        fprintf(stderr, "ext_fontfilelist |%s|\n", ext_fontfilelist);
    }
#endif
#if 0
    fprintf(stderr, "def_fontspec |%s|\n", def_fontspec);
#endif
    
    r = 0;

out:
    return r;
}

int
xtest()
{
    tgyfont_t *tf;
    int c;
    int ik;

#if 0
    tf = tgyfontset_find(FM_ASCII, FF_SANSERIF, NULL);

    if(tf) {
        tgyfont_fprint(stderr, tf, "found");
    }
    else {
        fprintf(stderr, "not found\n");
    }
#endif

#if 0
    c = tgyfont_wipe_encodeM(FM_KANJI, "AJAPA");
    c = tgyfont_wipe_encodeMF(FM_KANJI, FF_TYPE, "OOPS");
    tgyfontset_fprint(stderr, "");
#endif

#if 0
#endif
#if 0
    ik = tgyfont_wipe_scaleMF(FM_KANJI, FF_ALL, 0.8);
    ik = tgyfont_wipe_scaleMF(FM_KANJI, FF_TYPE, 0.64);
#endif
#if 0
    {
    char    *dmy;
    double   junk;
    dmy = tgyfont_resolv_fontname(FM_KANJI, FF_TYPE);
    printf("dmy %p %s\n", dmy, dmy);
    dmy = tgyfont_resolv_fontname(FM_KANJI, FF_NONE);
    printf("dmy %p %s\n", dmy, dmy);
    dmy = tgyfont_resolv_fontname(FM_KANJI, FF_ALL);
    printf("dmy %p %s\n", dmy, dmy);
    dmy = tgyfont_resolv_encode(FM_KANJI, FF_SERIF);
    printf("dmy %p %s\n", dmy, dmy);
    junk = tgyfont_resolv_scale(FM_KANJI, FF_SANSERIF);
    printf("junk %5.2f\n", junk);
    }
#endif

    return 0;
}

int
tgyfont_setup(int howmany)
{
    int num;
    int sz;
    int i;

    if(tgyfontset) {
        printf("WARNNING clear fontset\n");
        free(tgyfontset);
        tgyfontset = NULL;
    }

    if(howmany<0) {
        num = TGYFONTSET_N;
    }
    else {
        num = howmany;
    }

    sz = sizeof(tgyfont_t)*num;
#if 0
printf("sz %d\n", sz);
#endif
    tgyfontset = (tgyfont_t*)malloc(sz);
    if(!tgyfontset) {
        printf("ERROR no memory for fontset\n");
        return -1;
    }
    memset(tgyfontset, 0, sz);
    for(i=0;i<num;i++) {
        tgyfontset[i].fscale = 1.0;
    }

    tgyfontset_use = 0;
    tgyfontset_max = num;

#if 0
    tgyfontset_fdump(stderr, "");

    fflush(stdout);
    fflush(stderr);
#endif

    /*
     * default fonts
     */

    tgyfontset_add(FM_ASCII, FF_SERIF,      "Times-Roman",  "ASCII");
    tgyfontset_add(FM_ASCII, FF_SANSERIF,   "Helvetica",    "ASCII");
    tgyfontset_add(FM_ASCII, FF_ITALIC,     "Times-Italic", "ASCII");
    tgyfontset_add(FM_ASCII, FF_TYPE,       "Courier",      "ASCII");

    tgyfontset_add(FM_KANJI, FF_SERIF,      "IPAMincho-H",  "EUC-JP");
    tgyfontset_add(FM_KANJI, FF_SANSERIF,   "IPAGothic-H",  "EUC-JP");
    tgyfontset_add(FM_KANJI, FF_ITALIC,     "IPAMincho-H",  "EUC-JP");
    tgyfontset_add(FM_KANJI, FF_TYPE,       "IPAGothic-H",  "EUC-JP");

#if 0
    fflush(stdout);
    fflush(stderr);

    tgyfontset_fdump(stderr, "");
    tgyfontset_fprint(stderr, "");
#endif

    xtest();
    
    return 0;
}



