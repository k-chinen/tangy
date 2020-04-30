/*
 * tx.c - text entry data structure and its procedures
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>
#include "varray.h"

#include "tx.h"

extern char *def_fontspec;

int tx_trace = 0;

#define Echo    if(tx_trace)printf
#define INTRACE (tx_trace>0)

#define Error   printf("ERROR %s:%s ", __FILE__, __func__);fflush(stdout);printf
#define Warn    printf("WARNING %s:%s ", __FILE__, __func__);fflush(stdout);printf


int
txe_sprint(char *dst, int dlen, void *rv, int dmy)
{
    txe *v;
    v = (txe*)rv;
    sprintf(dst, "<%c%c'%s'>",
        v->ct, v->st, v->st==TXE_CONST ? v->cs : v->vs);
    return 0;
}

txe*
txe_new(int xnt, char *xns)
{
    txe *ne;
    int  xnsl;

    ne = (txe*)malloc(sizeof(txe));
    if(!ne) {
        return NULL;
    }

    memset(ne, 0, sizeof(txe));
    ne->ct = xnt;

    xnsl = strlen(xns);
    if(xnsl < TXELEN -1) {
        ne->st = TXE_CONST;
        strcpy(ne->cs, xns);
    }
    else {
        ne->st = TXE_VARIABLE;
        ne->vs = strdup(xns);
    }

    return ne;
}

int
txe_parse1(varray_t *ar, char *fs)
{
    char *p, *q;
    char  token[BUFSIZ];
    txe  *ne;
    int   cin;

Echo("target '%s'\n", fs);

    ne  = NULL;
    p   = fs;
    cin = 0;
    q = token;
    while(*p) {
#if 0
*q = '\0';
Echo("\t'%s'\n", token);
#endif
        if(*p=='\\') {
            p++;
            if(*p) {
                switch(*p) {
                case 'n':
                case 'r':
                    p++;            /* skip */
                    break;
                default:
                    *q++ = *p++;    /* add */
                }
            }
            continue;
        }
        if(*p==TXE_ESCC) {
            ne = NULL;
            *q = '\0';
            if(token[0]) {
                if(cin<=0) {
Echo(" data %d '%s'\n", cin, token);
                    ne = txe_new(TXE_DATA, token);
                    cin++;
                }
                else
                if(cin>0) {
Echo(" cmd  %d '%s'\n", cin, token);
                    ne = txe_new(TXE_CMD, token);
                    cin--;
                }
                if(ne) {
                    varray_push(ar, ne);
                }
            }
            else {
                if(cin<=0) {
                    cin++;
                }
                else
                if(cin>0) {
                    cin--;
                }
            }
            q = token;

            p++;
            continue;
        }
        *q++ = *p++;
    }
    *q = '\0';

            ne = NULL;
            if(token[0]) {
                if(cin<=0) {
Echo(" data %d '%s'\n", cin, token);
                    ne = txe_new(TXE_DATA, token);
                }
                else {
Echo(" cmd  %d '%s'\n", cin, token);
                    ne = txe_new(TXE_CMD, token);
                }
                if(ne) {
                    varray_push(ar, ne);
                }
            }

    return 0;
}

static
int
dump(FILE *fp, char *pre, char *xx, int len)
{
    unsigned char *p;
    int i;

    fprintf(fp, "%s", pre);
    p = (unsigned char*)xx;
    i = 0;
    while(i<len) {
        fprintf(fp, " %02x", *p);
        p++;
        i++;
    }
    fprintf(fp, "\n");
    fflush(fp);

    return 0;
}




static
int
ins_kanji_shift(char *ds, int dlen, char *ss)
{
    iconv_t cq;

    int     ik;
    char   *is;
    char   *os;
    size_t  ilen;
    size_t  olen;

    char   *ms;
    int     mlen;

    unsigned char   *p;
    unsigned char   *q;
    unsigned char   *u;
    

    mlen = strlen(ss)*4+1;
    ms = (char*)alloca(mlen);
    if(!ms) {
        fprintf(stderr, "no memory (mlen %d)\n", mlen);
        return -1;
    }
    memset(ms, 0, mlen);


    cq = iconv_open("EUC-JP", "UTF-8");
    if(cq == (iconv_t)-1) {
        printf("fail iconv_open (%d)\n", errno);
        return -1;
    }

    is   = (char*)ss;
    os   = (char*)ms;
    ilen = (size_t)strlen(ss);
    olen = (size_t)mlen;
#if 1
    /* padding default font spec and trancate lengths */
    if(def_fontspec && *def_fontspec) {
        int w;
        sprintf(ms, "|%s|", def_fontspec);
        w    = strlen(ms);
        os   = ms+w;
        olen = mlen-w;
    }
#endif

    Echo("ilen %d\n", (int)ilen);
    Echo("olen %d\n", (int)mlen);

    ik = iconv(cq, &is, &ilen, &os, &olen);

    Echo("ik %d iconv\n", ik);

    Echo("ilen %d\n", (int)ilen);
    Echo("olen %d\n", (int)mlen);

    iconv_close(cq);

    if(INTRACE) {
        dump(stderr, "ms", ms, strlen(ms));
    }

    p = (unsigned char*)ms;
    q = (unsigned char*)ds;

    while(*p) {
        if(*p>=0x80) {
            u = p;
            while(*u && *u>=0x80) {
                u++;
            }
#if 0
fprintf(stderr, "p %p %02x\n", p, *p);
fprintf(stderr, "u %p %02x\n", u, *u);
#endif
            *q++ = '|'; 
            *q++ = 'K'; 
            *q++ = 'A'; 
            *q++ = 'N'; 
            *q++ = 'J'; 
            *q++ = 'I'; 
            *q++ = '|'; 
            while(p<u) {
                *q++ = *p++;
            }
            *q++ = '|'; 
            *q++ = 'A'; 
            *q++ = 'S'; 
            *q++ = 'C'; 
            *q++ = 'I'; 
            *q++ = 'I'; 
            *q++ = '|'; 
        }
        else {
            *q++ = *p++;
        }
    }
    *q = '\0';

    if(INTRACE) {
        dump(stderr, "ds", ds, strlen(ds));
    }
    
    Echo("ss '%s'\n", ss);
    Echo("ms '%s'\n", ms);
    Echo("ds '%s'\n", ds);

    return 0;
}

int
txe_parse(varray_t *ar, char *fs)
{
    int ik;
    char *ts;
    int   il;
    int   tl;

    il = strlen(fs);
    tl = (il/2)*14+1;
    ts = (char*)alloca(tl);
    if(!ts) {
        fprintf(stderr, "no memory (il %d, tl %d)\n", il, tl);
        return -1;
    }

    memset(ts, 0, tl);

    Echo("fs '%s'\n", fs);
    Echo("ts '%s'\n", ts);

    ik = ins_kanji_shift(ts, tl, fs);

    Echo("fs '%s'\n", fs);
    Echo("ts '%s'\n", ts);
    
    ik = txe_parse1(ar, ts);

    return ik;
}

int
txe_extract(char *dst, int dlen, txe *te)
{
    dst[0] = '\0';
    if(te->ct==TXE_DATA) {
        if(te->st==TXE_CONST) {
            if(strlen(te->cs)<dlen-1) {
                strcat(dst, te->cs);
            }
            else {
                return -1;
            }
        }
        else {
            if(strlen(te->vs)<dlen-1) {
                strcat(dst, te->vs);
            }
            else {
                return -1;
            }
        }
    }

    return 0;
}

int
txear_extract(char *dst, int dlen, varray_t *ar)
{
    int   j;
    int   w;
    char  qs[BUFSIZ];
    txe  *te;

    dst[0] = '\0';
    w = 0;
    for(j=0;j<ar->use;j++) {
        te = ar->slot[j];
        qs[0] = '\0';
        if(te->ct==TXE_DATA) {
            if(te->st==TXE_CONST) {
#if 0
                psescape(qs, BUFSIZ, te->cs);
#endif
                strcat(qs, te->cs);
            }
            else {
#if 0
                psescape(qs, BUFSIZ, te->vs);
#endif
                strcat(qs, te->vs);
            }
        }
        strcat(dst, qs);
        w = strlen(dst);
    }

    return 0;
}




int
txe_release(varray_t *ar)
{
    int  i;
    txe *e;

    for(i=0;i<ar->use;i++) {
        e = (txe*)ar->slot[i];
        if(!e) {
            continue;
        }
        /* for txe */
        if(e->vs) {
            free(e->vs);
        }
        free(e);
    }

    /* for varray_t */
    free(ar->slot);
    free(ar);

    return 0;
}


#ifdef TX_STANDALONE

/*
 * compile like following:
 *      % cc -DTXELEN=33 -DTX_STANDALONE -DEcho=printf tx.c varray.c
 */

varray_t *tq;

int
main()
{
    tq = varray_new();

    if(!tq) {
        exit(7);
    }

    varray_entrysprintfunc(tq, txe_sprint);

    varray_fprint(stdout, tq);

#if 0
    txe_parse(tq, "ground|special|");
    txe_parse(tq, "|large|a\\|b|small|XYZ");
    txe_parse(tq, "|first|any||foo||dummy|end|");
#endif
#if 0
    txe_parse(tq, "ABC\xe8\xb5\xa4\xe9\x9d\x92");
#endif
    txe_parse(tq, "ABC\xe8\xb5\xa4\xe9\x9d\x92XYZ");

    varray_fprint(stdout, tq);

}

#endif /* TX_STANDALONE */
