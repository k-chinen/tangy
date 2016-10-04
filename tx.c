
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "varray.h"


#ifndef TXELEN  
#define TXELEN      (32)
#endif

#define TXE_CMD         ('o')
#define TXE_DATA        ('d')
#define TXE_CONST       ('c')
#define TXE_VARIABLE    ('v')


typedef struct {
    int   ct;
    int   st;
    char  cs[TXELEN];
    char *vs;
} txe;


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
txe_parse(varray_t *ar, char *fs)
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
#if 0
            *q++ = *p++;
#endif
            p++;
            if(*p) {
                *q++ = *p++;
            }
            continue;
        }
        if(*p=='|') {
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

    txe_parse(tq, "ground|special|");
    txe_parse(tq, "|large|a\\|b|small|XYZ");
    txe_parse(tq, "|first|any||foo||dummy|end|");

    varray_fprint(stdout, tq);

}

#endif /* TX_STANDALONE */
