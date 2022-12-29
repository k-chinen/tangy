#include <stdio.h>
#include <stdlib.h>

#ifndef FF_SERIF
#define FF_SERIF (1)
#endif
#ifndef FF_SANSERIF
#define FF_SANSERIF (2)
#endif

static int _cwhinit=0;
static int _RMcw[256];
static int _RMch[256];
static int _SFcw[256];
static int _SFch[256];

int
cwhinit()
{
    int i;

    /* default */
    for(i=0;i<256;i++) {
        _RMcw[i] =  50; _RMch[i] =  50; 
        _SFcw[i] =  56; _SFch[i] =  50; 
    }

    /* roman - times-roman */
    _RMcw[' '] = 25;

    _RMcw['A'] = 72; _RMcw['B'] = 67; _RMcw['C'] = 67;
    _RMcw['D'] = 72; _RMcw['E'] = 61; _RMcw['F'] = 55;
    _RMcw['G'] = 72; _RMcw['H'] = 72; _RMcw['I'] = 33;
    _RMcw['J'] = 38; _RMcw['K'] = 72; _RMcw['L'] = 61;
    _RMcw['M'] = 89; _RMcw['N'] = 72; _RMcw['O'] = 72;
    _RMcw['P'] = 56; _RMcw['Q'] = 72; _RMcw['R'] = 67;
    _RMcw['S'] = 56; _RMcw['T'] = 61; _RMcw['U'] = 72;
    _RMcw['V'] = 72; _RMcw['W'] = 94; _RMcw['X'] = 72;
    _RMcw['Y'] = 72; _RMcw['Z'] = 61;

    _RMcw['a'] = 44; _RMcw['b'] = 50; _RMcw['c'] = 44;
    _RMcw['d'] = 50; _RMcw['e'] = 44; _RMcw['f'] = 33;
    _RMcw['g'] = 50; _RMcw['h'] = 50; _RMcw['i'] = 28;
    _RMcw['j'] = 28; _RMcw['k'] = 50; _RMcw['l'] = 28;
    _RMcw['m'] = 78; _RMcw['n'] = 50; _RMcw['o'] = 50;
    _RMcw['p'] = 50; _RMcw['q'] = 50; _RMcw['r'] = 33;
    _RMcw['s'] = 39; _RMcw['t'] = 28; _RMcw['u'] = 50;
    _RMcw['v'] = 50; _RMcw['w'] = 72; _RMcw['x'] = 50;
    _RMcw['Y'] = 50; _RMcw['Z'] = 44;

    /* sanserif - helvetica */
    _SFcw[' '] = 28;

    _SFcw['A'] = 67; _SFcw['B'] = 67; _SFcw['C'] = 72;
    _SFcw['D'] = 72; _SFcw['E'] = 67; _SFcw['F'] = 61;
    _SFcw['G'] = 78; _SFcw['H'] = 72; _SFcw['I'] = 28;
    _SFcw['J'] = 50; _SFcw['K'] = 67; _SFcw['L'] = 56;
    _SFcw['M'] = 83; _SFcw['N'] = 72; _SFcw['O'] = 78;
    _SFcw['P'] = 67; _SFcw['Q'] = 78; _SFcw['R'] = 72;
    _SFcw['S'] = 67; _SFcw['T'] = 61; _SFcw['U'] = 72;
    _SFcw['V'] = 67; _SFcw['W'] = 94; _SFcw['X'] = 67;
    _SFcw['Y'] = 67; _SFcw['Z'] = 61;

    _SFcw['a'] = 56; _SFcw['b'] = 56; _SFcw['c'] = 50;
    _SFcw['d'] = 56; _SFcw['e'] = 56; _SFcw['f'] = 28;
    _SFcw['g'] = 56; _SFcw['h'] = 56; _SFcw['i'] = 22;
    _SFcw['j'] = 22; _SFcw['k'] = 50; _SFcw['l'] = 22;
    _SFcw['m'] = 83; _SFcw['n'] = 56; _SFcw['o'] = 56;
    _SFcw['p'] = 56; _SFcw['q'] = 56; _SFcw['r'] = 33;
    _SFcw['s'] = 50; _SFcw['t'] = 28; _SFcw['u'] = 56;
    _SFcw['v'] = 50; _SFcw['w'] = 72; _SFcw['x'] = 50;
    _SFcw['Y'] = 50; _SFcw['Z'] = 50;


    _cwhinit = 1;
    return 0;
}

/*
 * estimate string width and height accoring to fontname 
 * NOTE:
 *   - this code expect ONLY ASCII.
 *   - this code care ONLY Times-Roman and Helvetica.
 *   - this code DOES NOT care height.
 *   - this code returns percent.
 *      c.f., "open sesami" returns 491x50.  So it means 491x50%.
 *      if you use 10points font, the width is about 49.1points.
 *   - this code inclue some error.
 *      you should care error. put margin or something.
 */
int
est_strwh(char *xfn, int xftype, char *ss, int *rwd, int *rht)
{
    int   rv;
    char *fn;
    char *p;
    int   i;
    int   wsum, hmax;
    int  *cw, *ch;

    if(!_cwhinit) {
        cwhinit();
    }

    fn = xfn;
    if(!xfn) { fn = "default"; }
#if 0
    fprintf(stderr, "%s: fn '%s' xftype %d ss '%s'\n",
        __func__, fn, xftype, ss);
#endif

    switch(xftype) {
    case FF_SANSERIF:   cw = _SFcw; ch = _SFch;     break;
    default:
    case FF_SERIF:      cw = _RMcw; ch = _RMch;     break;
    }

    wsum = 0;
    hmax = -1;
    i = 0;
    p = ss;
    while(*p) {
        wsum += cw[*p];
        if(ch[*p] > hmax) hmax = ch[*p];
#if 0
        fprintf(stderr, "  %2d: '%c' %3d/%-9d; %3d/%-9d\n",
            i, *p, cw[*p], wsum, ch[*p], hmax);
#endif
        p++;
        i++;
    }

    *rwd = wsum;
    *rht = hmax;

#if 0
    fprintf(stderr, "%s: rv %d; wd %d ht %d\n", __func__, rv, *rwd, *rht);
#endif

    return rv;
}

#ifdef STRBB_MAIN
int main(int argc, char *argv[]) {
    int wd, ht;
    int ik;
    if(argc<=1) {
        printf("usage: %s type string\n", argv[0]);
        printf("type: serif=%d senserif=%d\n", FF_SERIF, FF_SANSERIF);
        exit(0);
    }
    ik = est_strwh("", atoi(argv[1]), argv[2], &wd, &ht);
}
#endif /* STRBB_MAIN */
