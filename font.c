#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alist.h"
#include "font.h"
#include "word.h"

double akratio = 0.95;

apair_t fm_ial[] = {
    {"ascii",   FM_ASCII},
    {"kanji",   FM_KANJI},
    {NULL,      -1},
};

apair_t fh_ial[] = {
    {"normal",  FH_NORMAL},
    {"large",   FH_LARGE},
    {"huge",    FH_HUGE},
    {"small",   FH_SMALL},
    {"tiny",    FH_TINY},
    {NULL,      -1},
};

apair_t fh_act_ial[] = {
    {"1.0",     FH_NORMAL},
    {"1.414",   FH_LARGE},
    {"2.0",     FH_HUGE},
    {"0.707",   FH_SMALL},
    {"0.5",     FH_TINY},
    {NULL,      -1},
};

apair_t ff_ial[] = {
    {"serif",       FF_SERIF},
    {"sanserif",    FF_SANSERIF},
    {"italic",      FF_ITALIC},
    {"type",        FF_TYPE},
    {NULL,      -1},
};

apair_t ff_act_ial[] = {
    {"Times-Roman",     FF_SERIF},
    {"Helvetica",       FF_SANSERIF},
    {"Times-Italic",    FF_ITALIC},
    {"Courier",         FF_TYPE},
    {NULL,      -1},
};

apair_t ff_actk_ial[] = {
    {"IPAMincho-H",     FF_SERIF},
    {"IPAGothic-H",     FF_SANSERIF},
    {"IPAMincho-H",     FF_ITALIC},
    {"IPAGothic-H",     FF_TYPE},
    {NULL,      -1},
};

int
swap_font(int xv, char *xn)
{
#if 0
    char *bfn;
    char *afn;
#endif
    int   i;
    int   q=0;

    if(xv<0 || !xn) {
        return -1;
    }

#if 0
    bfn  = rassoc(ff_act_ial, xv);
    fprintf(stderr, "bfn |%s|\n", bfn);
#endif

    i = 0;
    while(ff_act_ial[i].name) {
        if(ff_act_ial[i].value==xv) {
            ff_act_ial[i].name = strdup(xn);
            q++;
            break;
        }
        i++;
    }

#if 0
    afn  = rassoc(ff_act_ial, FF_SERIF);
    fprintf(stderr, "afn |%s|\n", afn);
#endif

    if(q) {
        return q;
    }
    else {
        return 0;
    }
}

int
swap_Xfont(apair_t *ls, int xv, char *xn)
{
#if 0
    char *bfn;
    char *afn;
#endif
    int   i;
    int   q=0;

    if(xv<0 || !xn) {
        return -1;
    }

#if 0
    bfn  = rassoc(ls, xv);
    fprintf(stderr, "bfn |%s|\n", bfn);
#endif

    i = 0;
    while(ls[i].name) {
        if(ls[i].value==xv) {
            ls[i].name = strdup(xn);
            q++;
            break;
        }
        i++;
    }

#if 0
    afn  = rassoc(ls, FF_SERIF);
    fprintf(stderr, "afn |%s|\n", afn);
#endif

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


#define FSLEN   (8)

int
fontset_fprint(FILE *fp, char *pmsg)
{
    int    i;
    int    j;
    char **snl;
    char **afnl;
    char **kfnl;
    char  *sn;
    int    p;

#if 1
    fprintf(stderr, "%s: |%s|\n", __func__, pmsg);
#endif

    snl = (char**)malloc(sizeof(char*)*FSLEN);
    if(!snl) {  fprintf(stderr, "no memory\n"); return -1; }
    afnl = (char**)malloc(sizeof(char*)*FSLEN);
    if(!afnl) { fprintf(stderr, "no memory\n"); return -1; }
    kfnl = (char**)malloc(sizeof(char*)*FSLEN);
    if(!kfnl) { fprintf(stderr, "no memory\n"); return -1; }

    memset(snl,  0, sizeof(char*)*FSLEN);
    memset(afnl, 0, sizeof(char*)*FSLEN);
    memset(kfnl, 0, sizeof(char*)*FSLEN);

#if 0
    for(i=0;i<FSLEN;i++) {
        fprintf(fp, "%2d: %-8s %-32s %-32s\n",
            i,  (snl[i]  ? snl[i]  : "*"),
                (afnl[i] ? afnl[i] : "-"),
                (kfnl[i] ? kfnl[i] : "-"));
    }
#endif
    
    j = 0;
    while(ff_act_ial[j].name) {
        p = ff_act_ial[j].value;
        if(p>=0 && p<FSLEN) {
            snl[p]  = rassoc(ff_ial, ff_act_ial[j].value);
            afnl[p] = ff_act_ial[j].name;
        }
        j++;
    }
    
    j = 0;
    while(ff_actk_ial[j].name) {
        p = ff_actk_ial[j].value;
        if(p>=0 && p<FSLEN) {
            snl[p]  = rassoc(ff_ial, ff_actk_ial[j].value);
            kfnl[p] = ff_actk_ial[j].name;
        }
        j++;
    }

    for(i=0;i<FSLEN;i++) {
        fprintf(fp, "%2d: %-8s %-32s %-32s\n",
            i,  (snl[i]  ? snl[i]  : "*"),
                (afnl[i] ? afnl[i] : "-"),
                (kfnl[i] ? kfnl[i] : "-"));
    }


    free(snl);
    free(afnl);
    free(kfnl);

    return 0;
}

int
font_edit(int cat, char *opseq)
{
    int   r;
    char *p;
    char *u;
    char *m;
    char *e;
    char *g;
    char  pair[BUFSIZ];
    char  key[BUFSIZ];
    char  val[BUFSIZ];
    char  func[BUFSIZ];
    char  fontfilename[BUFSIZ];
    char  fontname[BUFSIZ];
    int   ff;
    int   ik;

    r = -1;
#if 0
    fprintf(stderr, "%s: cat %d opseq |%s|\n", __func__, cat , opseq);
#endif

#if 0
    fontset_fprint(stderr, "before edit");
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
        ff = assoc(ff_ial, key);    

#if 0
        fprintf(stderr, "  key  |%s| ff %d\n", key, ff);
        fprintf(stderr, "  val  |%s|\n", val);
#endif

        if(strcasecmp(key, "default")==0) {
            def_fontspec = strdup(val);
            continue;
        }
        if(ff<0) {
            if(cat==FM_ASCII) {
                ik = swap_Xfont(ff_act_ial, FF_SERIF, key);
                def_fontname = strdup(key);
            }
            if(cat==FM_KANJI) {
                ik = swap_Xfont(ff_actk_ial, FF_SERIF, key);
            }
            continue;
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

#if 0
            fprintf(stderr, "  val* |%s|\n", val);
#endif
            if(cat==FM_ASCII) {
                swap_Xfont(ff_act_ial, ff, val);
            }
            else
            if(cat==FM_KANJI) {
                swap_Xfont(ff_actk_ial, ff, val);
            }
            else {
            }
        }

    }

#if 0
    fontset_fprint(stderr, "after  edit");

    if(ext_fontnamelist) {
        fprintf(stderr, "ext_fontnamelist |%s|\n", ext_fontnamelist);
    }
    if(ext_fontfilelist) {
        fprintf(stderr, "ext_fontfilelist |%s|\n", ext_fontfilelist);
    }
#endif
    
    r = 0;

out:
    return r;
}

char*
resolv_font(int xmode, int xface)
{
    char *rv;
    apair_t *ls;

    rv = NULL;
    if(xmode==FM_ASCII) {
        ls = ff_act_ial;
    }
    else {
        ls = ff_actk_ial;
    }
    rv = rassoc(ls, xface);

#if 0
    Echo("%s: xmode %s(%d) xface %s(%d) -> rv %s\n", __func__,
        rassoc(fm_ial, xmode), xmode,
        rassoc(ff_ial, xface),  xface, rv);
#endif
    return rv;
}
