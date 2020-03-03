#include <stdio.h>
#include <string.h>
#include "alist.h"
#include "font.h"

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


