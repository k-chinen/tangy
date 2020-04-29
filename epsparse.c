#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "word.h"
#include "qbb.h"

static int _epsparse_verbose = 0;

int
xlstrcmp(char *b, char *x)
{
    int w;
    int r;

    w = strlen(x);
    r = strncmp(b, x, w);
    return r;
}


int
epsparse_fname(FILE *ifp, qbb_t *qbb)
{
    int    r;
    int    lno;
    char   line[BUFSIZ];
    char   token[BUFSIZ];
    char  *p;
    int    findps;
    int    findeps;
    int    findbb;
    int    finddbb;
    int    lx, by, rx, ty;
    double dlx, dby, drx, dty;

    r = -1;
    lno = 0;
    
    findps  = 0;
    findeps = 0;
    findbb  = 0;
    finddbb = 0;

    while(fgets(line, BUFSIZ, ifp)) {
        lno++;
        if(lno>100) {
            if(_epsparse_verbose) {
                fprintf(stderr, "; giveup %d lines\n", lno);
            }
            break;
        }
        if(_epsparse_verbose) {
            fprintf(stderr, "; %3d: %s", lno, line);
        }
        chomp(line);
#if 0
        fprintf(stderr, ";|%s|\n", line);
#endif
        p = line;
        p = skipwhite(p);
        if(*p=='#'||*p=='\0') {
            if(_epsparse_verbose) {
                fprintf(stderr, "; *skip*\n");
            }
            continue;
        }

        /*
        %!PS-Adobe-3.0 EPSF-3.0
        %%BoundingBox: 0 0 1136 136
        %%HiResBoundingBox: 0.000000 0.000000 1312.000000 652.000000
        */
        if(xlstrcmp(p, "%!PS-Adobe")==0) {
            if(_epsparse_verbose) {
                fprintf(stderr, "= PS\n");
            }
            findps = 1;
            if(strstr(p, "EPSF")) {
                if(_epsparse_verbose) {
                    fprintf(stderr, "== EPS\n");
                }
                findeps = 1;
            }
        }

int _i;
#define SI(mm)  {\
                u = skipwhite(u); \
                if(_epsparse_verbose) { \
                    fprintf(stderr, "%d '%s'\n", _i, u); \
                } \
                u = draw_word(u, token, BUFSIZ, ' '); \
                if(!token[0]) break; \
                mm = atoi(token); \
                _i++; \
                findbb++; \
            }

#define SD(mm)  {\
                u = skipwhite(u); \
                if(_epsparse_verbose) { \
                    fprintf(stderr, "%d '%s'\n", _i, u); \
                } \
                u = draw_word(u, token, BUFSIZ, ' '); \
                if(!token[0]) break; \
                mm = atof(token); \
                _i++; \
                finddbb++; \
            }


        if(xlstrcmp(p, "%%BoundingBox:")==0) {
            char *u;
            _i = 0;
            findbb = 0;
            u = p;
            u = skipwhite(u);
            u = draw_word(u, token, BUFSIZ, ' ');
            SI(lx);
            SI(by);
            SI(rx);
            SI(ty);
        }
        if(xlstrcmp(p, "%%HiResBoundingBox:")==0) {
            char *u;
            _i = 0;
            finddbb = 0;
            u = p;
            u = skipwhite(u);
            u = draw_word(u, token, BUFSIZ, ' ');
            SD(dlx);
            SD(dby);
            SD(drx);
            SD(dty);
        }

    }

    if(_epsparse_verbose) {
        fprintf(stderr, "= bb  %d | %d %d %d %d\n",
            findbb, lx, by, rx, ty);
        fprintf(stderr, "= dbb %d | %f %f %f %f\n",
            finddbb, dlx, dby, drx, dty);
    }

    if(findbb) {
        qbb_setbb(qbb, lx, by, rx, ty);
    }
    else
    if(finddbb) {
        qbb_setbb(qbb, (int)dlx, (int)dby, (int)drx, (int)dty);
    }
    if(findps && findeps) {
            r = 0;
    }

out:
    return r;
}

int
epsparse_fp(char *fname, qbb_t *qbb)
{
    FILE *fp;

    fp = fopen(fname,"r");
    if(fp) {
        return epsparse_fname(fp, qbb);
    }
    else {
        return -2;
    }
}

#ifdef    TEST_EPSPARSE

int
main(int argc, char *argv[])
{
    int    ik;
    qbb_t *qbb;
    char  *fname = NULL;

    qbb = qbb_new();
    if(!qbb) {
        fprintf(stderr, "no memory\n");
        exit(3);
    }

    if(argc>=2) {
        fname = strdup(argv[1]);
    }
    if(_epsparse_verbose) {
        fprintf(stderr, "argc %d fname |%s|\n", argc, fname);
    }

    if(fname) {
        ik = epsparse_fp(fname, qbb);
    }
    else {
        ik = epsparse_fname(stdin, qbb);
    }

#if 0
    fprintf(stderr, "ik %d: %d %d %d %d\n",
        ik, qbb->lx, qbb->by, qbb->rx, qbb->ty);
#endif

    if(ik==0) {
        exit(0);
    }
    if(ik<0) {
        exit(1);
    }
    if(ik>0) {
        exit(2);
    }

}
#endif /* TEST_EPSPARSE */

