/*
 * Copyright (C) 2002-2015 StarBED Project All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* modified by k-chinen at 2022 Mar 18 */

/*
 * Various data Dictionary 
 */
/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define Error   printf
#ifndef QS
#define QS(x)   (((x)==NULL)?"*null*":(x[0]=='\0'?"*empty*":(x)))
#endif

#include "alist.h"
#include "vdict.h"

apair_t vdictmode_al[] = {
    {"openhash",        VDICT_MODE_OHASH},
    {"sortedarray",     VDICT_MODE_SARRAY},
    {"normalarray",     VDICT_MODE_NARRAY},
    {NULL,  -1},
};

static
int
def_keycmp(const void *ap, const void *bp)
{
    char *a, *b;
    a = ((vdict_cell*)ap)->key;
    b = ((vdict_cell*)bp)->key;
    if(!a)
        return 1;
    if(!b)
        return -1;
    return strcmp(a,b);
}

static
int
def_hash(char *src, int radix, int seed)
{
    unsigned char *p;
    int c;
    unsigned int sum;
    sum = 1027+seed;
    c = 1;
    p = (unsigned char *)src;
    while(*p) {
        sum <<= 1;
        sum += *p * c;
        c += 2;
        p++;
    }
    return sum%radix;
}

int
vdict_clear(vdict_t *dict)
{
    int i;

    memset(dict, 0, sizeof(vdict_t));

    dict->mode = VDICT_MODE_OHASH;
    dict->opt  = VDICT_OPT_EXPANDALLOW;

    dict->usage_hiwater = 80; /* 80% */

    dict->len  = VDICT_SLOTBASELEN;
    dict->slot = (vdict_cell*)malloc(sizeof(vdict_cell)*dict->len);
    dict->use  = 0;
    for(i=0;i<dict->len;i++) {
        dict->slot[i].key = NULL;
    }

    dict->hashfunc   = def_hash;
    dict->purgefunc  = NULL;
    dict->printfunc  = NULL;
    dict->keycmpfunc = def_keycmp;

    dict->ctime    = time(NULL);

    dict->userint1  = 0;
    dict->userint2  = 0;
    dict->userint3  = 0;
    dict->uservoid1 = NULL;
    
    return 0;
}



vdict_t*
vdict_new()
{
    return (vdict_t*)malloc(sizeof(vdict_t));
}

vdict_t*
vdict_newclear()
{
    vdict_t *ret;

    ret = vdict_new();
    if(ret) {
        vdict_clear(ret);
    }
    return ret;
}

int
vdict_setusagehiwater(vdict_t *dict, int xsp)
{
    if(xsp>100) {
        fprintf(stderr, "%s: usage_hiwater must be less euqal 100.\n",
            __func__);
    }
    else {
        dict->usage_hiwater = xsp;
    }
    return 0;
}

/* current = new */
int
vdict_setopt(vdict_t *dict, int ov)
{
    dict->opt = ov;
    return 0;
}
/* current = current | new */
int
vdict_setopt_orpos(vdict_t *dict, int ov)
{
    int old;
    old = dict->opt;
    dict->opt = dict->opt | ov;
#if 0
    fprintf(stderr, "%s:\n", __func__);
    fprintf(stderr, "  cur 0x%08x\n", old);
    fprintf(stderr, "  new 0x%08x\n", ov);
    fprintf(stderr, "  ans 0x%08x\n", dict->opt);
#endif
    return 0;
}
/* current = current & !new */
int
vdict_setopt_andneg(vdict_t *dict, int ov)
{
    int old;
    old = dict->opt;
    dict->opt = dict->opt & ~ov;
#if 0
    fprintf(stderr, "%s:\n", __func__);
    fprintf(stderr, "  cur 0x%08x\n", old);
    fprintf(stderr, "  new 0x%08x\n", ov);
    fprintf(stderr, "  ne' 0x%08x\n", ~ov);
    fprintf(stderr, "  ans 0x%08x\n", dict->opt);
#endif
    return 0;
}

int
vdict_getopt(vdict_t *dict, int *ov)
{
    *ov = dict->opt;
    return 0;
}

int
vdict_entrypurgefunc(vdict_t *dict, int (*subf)(void*))
{
    dict->purgefunc = subf;
    return 0;
}

int
vdict_entryprintfunc(vdict_t *dict, int (*subf)(char *,int,void*))
{
    dict->printfunc = subf;
    return 0;
}

int
vdict_entryhashfunc(vdict_t *dict, int (*subf)(char *,int,int))
{
    dict->hashfunc = subf;
    return 0;
}

int
vdict_entryhashseed(vdict_t *dict, int xseed)
{
    dict->hashseed = xseed;
    return 0;
}

int
vdict_fprinttable(vdict_t *dict, FILE *fp)
{
    int i;
    int ck;
    int c;
    char tmp[BUFSIZ];

    if(!dict) return -1;

    c = 0;
    for(i=0;i<dict->len;i++){
        if(!dict->slot[i].key) {
            continue;
        }
        if(!dict->slot[i].key[0]) {
            continue;
        }
        ck = dict->printfunc(tmp, BUFSIZ, dict->slot[i].value);
        if(ck) {
            strcpy(tmp, "*error*");
        }
        else
        if(!tmp[0]) {
            strcpy(tmp, "*empty*");
        }
        fprintf(fp, "%d %d %s %s\n", i, c, dict->slot[i].key, tmp);
        c++;
    }
    fflush(fp);
    return 0;
}

int
vdict_printtable(vdict_t *dict)
{
    vdict_fprinttable(dict, stdout);
    return 0;
}

int
vdict_printtable_TeX(vdict_t *dict)
{
    int i;
    int c;
    int ck;
    char tmp[BUFSIZ];

    if(!dict) return -1;

    printf("\\documentclass[a4j]{jarticle}\n");
    printf("\\begin{document}\n");

    printf("\\begin{tabular}{rrl}\n");
    printf("\\multicolumn{1}{c}{count} &\n");
    printf("\\multicolumn{1}{c}{pos} &\n");
    printf("\\multicolumn{1}{c}{key} & \n");
    printf("\\multicolumn{1}{c}{value} \\\\\n");
    c = 0;
    for(i=0;i<dict->use;i++){
        ck = dict->printfunc(tmp, BUFSIZ, dict->slot[i].value);
        if(ck) {
            strcpy(tmp, "*error*");
        }
        else
        if(!tmp[0]) {
            strcpy(tmp, "*empty*");
        }
        printf("%d & %d & %s & %s \\\\\n",
            c, i, dict->slot[i].key, tmp);
        c++;
    }
    printf("\\end{tabular}\n");

    printf("\\end{document}\n");
    return 0;
}


int
vdict_fshow_head(vdict_t *dict, FILE *fp)
{
    if(!dict) return -1;

    fprintf(fp, "dict %p %d %d/%d %s 0x%08x; u %d,%d,%d %p\n",
        dict, (int)dict->ctime, dict->use, dict->len, 
        rassoc(vdictmode_al, dict->mode), dict->opt,
        dict->userint1, dict->userint2, dict->userint3, dict->uservoid1);
    fflush(fp);
    return 0;
}
int
vdict_show_head(vdict_t *dict)
{
    return vdict_fshow_head(dict, stdout);
}

static
int
_vdict_fshow(vdict_t *dict, FILE *fp, char *xkey)
{
    int  i;
    int  ck;
    int  c;
    int  h;
    char tmp[BUFSIZ];
    char line[BUFSIZ];

    if(!dict) return -1;
#if 0
    fprintf(stderr, "===\n");
#else
    fprintf(stderr, "#");
#endif
    vdict_fshow_head(dict, fp);

    c = 0;
    for(i=0;i<dict->len;i++){
        if(!dict->slot[i].key) {
            continue;
        }
        if(!dict->slot[i].key[0]) {
            continue;
        }

        if(xkey && strcmp(dict->slot[i].key, xkey)!=0) {
            continue;
        }

        line[0] = '\0';
        sprintf(tmp, "%5d ", i);
        strcat(line, tmp);
    
        if(dict->opt & VDICT_OPT_PRINTADDR) {
            sprintf(tmp, "0x%p ", &dict->slot[i]);
            strcat(line, tmp);
        }
    
        if(dict->opt & VDICT_OPT_PRINTHASH) {
            h  = dict->hashfunc(dict->slot[i].key, dict->len, dict->hashseed);
            sprintf(tmp, "%5d ", h);
            strcat(line, tmp);
        }

        {
            sprintf(tmp, "%-16s", dict->slot[i].key);
            strcat(line, tmp);
        }
    
        if(dict->opt & VDICT_OPT_PRINTADDR) {
            sprintf(tmp, "0x%p ", dict->slot[i].value);
            strcat(line, tmp);
        }

        if(dict->slot[i].value) {
            ck = dict->printfunc(tmp, BUFSIZ, dict->slot[i].value);
            if(ck) {
                strcpy(tmp, "*error*");
            }
            else
            if(!tmp[0]) {
                strcpy(tmp, "*empty*");
            }
        }
        else {
            strcpy(tmp, "*null*");
        }
        strcat(line, tmp);

        strcat(line, "\n");

        fputs(line, fp);

        c++;
    }
#if 0
    fprintf(fp, "--\n%d/%d items\n", c, dict->len);
#endif
    if(xkey) {
        if(c==0) {
            fprintf(fp, "*not-found* %s\n", QS(xkey));
        }
    }
    fflush(fp);
    return 0;
}

int
vdict_fshow(vdict_t *dict, FILE *fp)
{
    return _vdict_fshow(dict, fp, NULL);
}

int
vdict_fshow_wkey(vdict_t *dict, FILE *fp, char *xkey)
{
    return _vdict_fshow(dict, fp, xkey);
}



int
Xvdict_fshow(vdict_t *dict, FILE *fp)
{
    int  i;
    int  ck;
    int  c;
    int  h;
    char tmp[BUFSIZ];
    char line[BUFSIZ];

    if(!dict) return -1;
#if 0
    fprintf(stderr, "===\n");
#else
    fprintf(stderr, "#");
#endif
    vdict_fshow_head(dict, fp);

    c = 0;
    for(i=0;i<dict->len;i++){
        if(!dict->slot[i].key) {
            continue;
        }
        if(!dict->slot[i].key[0]) {
            continue;
        }

        line[0] = '\0';
        sprintf(tmp, "%5d ", i);
        strcat(line, tmp);
    
        if(dict->opt & VDICT_OPT_PRINTADDR) {
            sprintf(tmp, "0x%p ", &dict->slot[i]);
            strcat(line, tmp);
        }
    
        if(dict->opt & VDICT_OPT_PRINTHASH) {
            h  = dict->hashfunc(dict->slot[i].key, dict->len, dict->hashseed);
            sprintf(tmp, "%5d ", h);
            strcat(line, tmp);
        }

        {
            sprintf(tmp, "%-16s", dict->slot[i].key);
            strcat(line, tmp);
        }
    
        if(dict->opt & VDICT_OPT_PRINTADDR) {
            sprintf(tmp, "0x%p ", dict->slot[i].value);
            strcat(line, tmp);
        }

        ck = dict->printfunc(tmp, BUFSIZ, dict->slot[i].value);
        if(ck) {
            strcpy(tmp, "*error*");
        }
        else
        if(!tmp[0]) {
            strcpy(tmp, "*empty*");
        }
        strcat(line, tmp);

        strcat(line, "\n");

        fputs(line, fp);

        c++;
    }
#if 0
    fprintf(fp, "--\n%d/%d items\n", c, dict->len);
#endif
    fflush(fp);
    return 0;
}

int
vdict_show(vdict_t *dict)
{
    return vdict_fshow(dict, stdout);
}

int
vdict_show_wkey(vdict_t *dict, char *xkey)
{
    return vdict_fshow_wkey(dict, stdout, xkey);
}


vdict_cell*
vdict_findpos_wkey(vdict_t *dict, char *xkey)
{
    int pos;
    vdict_cell *ppos, *p, ref;
    int i;

    if(!dict) return NULL;

    ppos = NULL;    

    if(dict->mode==VDICT_MODE_OHASH) {

        i = 0;
        pos = dict->hashfunc(xkey, dict->len, dict->hashseed);
        while(i<dict->len+2) {
            p = &dict->slot[pos];
            if(p->key && strcmp(p->key, xkey)==0) {
                ppos = p;
                break;
            }
            pos = (pos+1)%dict->len;
            i++;
        }
#if 0
        fprintf(stderr, "dict %p; key '%s' -> ret %p, i %d/%d\n",
            dict, xkey, ppos, i, dict->len);
#endif
    }
    else
    if(dict->mode==VDICT_MODE_SARRAY) {
        ref.key = xkey;
        ppos = bsearch(&ref, dict->slot, dict->use,
                sizeof(dict->slot[0]), dict->keycmpfunc);
    }
    else
    if(dict->mode==VDICT_MODE_NARRAY) {
        for(i=0;i<dict->use;i++) {
            p = &dict->slot[i];
            if(p->key && strcmp(p->key, xkey)==0) {
                ppos = p;
                break;
            }
        }
    }
    else {
        fprintf(stderr, "unknown mode dictionary\n");
    }

    return ppos;
}

vdict_cell*
vdict_findpos_wcharval(vdict_t *dict, char *xval)
{
    int  pos;
    vdict_cell *ppos, *p, ref;
    int  i;
    int  ck;
    char tmp[BUFSIZ];

#if 0
    fprintf(stderr, "enter %s\n", __func__);
    fprintf(stderr, "dict %p, xval %s\n", dict, QS(xval));
#endif

    if(!dict) return NULL;
    if(!dict->printfunc) {
        Error("no print function");
        return NULL;
    }

    ppos = NULL;    

    /* do not care the mode of dict */
    for(i=0;i<dict->len;i++) {
        p = &dict->slot[i];
        if(p->key) {
            ck = dict->printfunc(tmp, BUFSIZ, p->value);
#if 0
fprintf(stderr, "cmp %3d+ %s vs %s <ck %d>\n",
    i, QS(xval), QS(tmp), ck, p->key);
#endif
            if(strcmp(tmp, xval)==0) {
                ppos = p;
                break;
            }
        }
    }

    return ppos;
}


int
vdict_expand(vdict_t *dict)
{
    vdict_cell *newslot;
    int newlen;
    int i;
    vdict_cell *oldslot;
    int oldlen;

    if(dict->opt & VDICT_OPT_EXPANDSQUARE) {
        /* XXX */
        newlen = dict->len*dict->len;
    }
    else
    if(dict->opt & VDICT_OPT_EXPANDTHREESECONDS) {
        newlen = 3*dict->len/2;
    }
    else {
        newlen = dict->len*2;
    }

#if 0
    fprintf(stderr, "%s: use %d, len %d -> newlen %d\n",
        __func__, dict->use, dict->len, newlen);
#endif

    newslot = (vdict_cell*)malloc(sizeof(vdict_cell)*newlen);
    for(i=0;i<newlen;i++) {
        newslot[i].key = NULL;
    }
    oldslot = dict->slot;
    oldlen  = dict->len;
    dict->slot = newslot;
    dict->len  = newlen;
    dict->use  = 0;

    for(i=0;i<oldlen;i++) {
        if(oldslot[i].key && oldslot[i].key[0]) 
            vdict_add(dict, oldslot[i].key, oldslot[i].value);
    }
    free(oldslot);

    return 0;
}



int _ohash_new = 0;
int _ohash_reuse = 0;

static
int
_vdict_add(vdict_t *dict, char *xkey, void* xvalue, int allowswap)
{
    int pos;
    int ck;
    int i;

    if(!dict) {
        Error("null dict");
        return -1;
    }
    if(!xkey || !xkey[0]) {
        Error("null or empty key");
        return -1;
    }

    if(dict->opt & VDICT_OPT_EXPANDALLOW) {
#if 0
        fprintf(stderr, "use %d, len %d, hiwater %d; len*hiwater/100 %d\n",
            dict->use, dict->len, dict->usage_hiwater,
            dict->len*dict->usage_hiwater/100);
#endif

        if(dict->use >= dict->len*dict->usage_hiwater/100 - 1) {
            vdict_expand(dict);
        }
    }
    
    if(dict->use+1>=dict->len) {
        fprintf(stderr, "%s: no more slot\n", __func__);
        return -1;
    }
    

    if(dict->mode == VDICT_MODE_OHASH) {

        pos = dict->hashfunc(xkey, dict->len, dict->hashseed);
        while( dict->slot[pos].key &&
            dict->slot[pos].key[0]) {
            if(dict->slot[pos].key[0]==xkey[0]
                && strcmp(dict->slot[pos].key, xkey)==0) {
#if 0
                fprintf(stderr, "already entried '%s'\n", xkey);
#endif
                if(allowswap) {
                    if(dict->purgefunc) {
                        ck = (dict->purgefunc)(dict->slot[pos].value);
                        dict->slot[pos].value = NULL;
                    }
                    goto found;
                }
                else {
                    fprintf(stderr, "found same %s; swap is not allowd\n", xkey);
                    return 1;
                }
            }
            pos = (pos+1)%dict->len;
        }
        if(!dict->slot[pos].key) {
            dict->use++;
            _ohash_new++;
        }
        else {
            free(dict->slot[pos].key);
            _ohash_reuse++;
        }

newentry:
        dict->slot[pos].key = strdup(xkey);
found:
        dict->slot[pos].value = xvalue;

    }
    else
    if(dict->mode == VDICT_MODE_SARRAY ||
            dict->mode == VDICT_MODE_NARRAY) {
        pos = -1;
        for(i=0;i<dict->len;i++) {
            if(dict->slot[i].key && strcmp(dict->slot[i].key, xkey)==0) {
                pos = i;
                break;
            }
        }

        if(pos>=0) {
            /* already exist entry */
            if(allowswap) {
                if(dict->purgefunc) {
                    ck = (dict->purgefunc)(dict->slot[pos].value);
                    dict->slot[pos].value = NULL;
                }
                goto found;
            }
            else {
                fprintf(stderr, "found same %s; swap is not allowd\n", xkey);
                return 1;
            }
            dict->slot[pos].value = xvalue;
        }
        else {
            /* new entry */
            dict->slot[dict->use].key = strdup(xkey);
            dict->slot[dict->use].value = xvalue;
            dict->use++;
            if(dict->mode == VDICT_MODE_SARRAY) {
                qsort(dict->slot, dict->use,
                    sizeof(dict->slot[0]), dict->keycmpfunc);
            }
        }
    }

out:
    return 0;
}

int
vdict_add(vdict_t *dict, char *xkey, void* xvalue)
{
    return _vdict_add(dict, xkey, xvalue, 0);
}
int
vdict_addorswap(vdict_t *dict, char *xkey, void* xvalue)
{
    return _vdict_add(dict, xkey, xvalue, 1);
}

int
vdict_del(vdict_t *dict, char *xkey)
{
    vdict_cell *pos;
    int ck;

    pos = vdict_findpos(dict, xkey);
    if(pos) {
        pos->key[0] = '\0';
        if(dict->purgefunc) {
            ck = (dict->purgefunc)(pos->value);
            pos->value = NULL;
        }
        return 0;
    }
    return 1;
}

int
vdict_sortbykey(vdict_t *dict)
{
    qsort(dict->slot, dict->len,
        sizeof(dict->slot[0]), dict->keycmpfunc);
    dict->mode = VDICT_MODE_SARRAY;
    return 0;
}

int
vdict_sortbyvalue(vdict_t *dict, int (*cmp)(const void*,const void*))
{
#if 0
    qsort(dict->slot, dict->use,
        sizeof(dict->slot[0]), cmp);
#endif
    qsort(dict->slot, dict->len,
        sizeof(dict->slot[0]), cmp);
    return 0;
}


int vdict_setuserint1(vdict_t *dict, int uv)
{
    dict->userint1 = uv;
    return 0;
}
int vdict_setuserint2(vdict_t *dict, int uv)
{
    dict->userint2 = uv;
    return 0;
}
int vdict_setuserint3(vdict_t *dict, int uv)
{
    dict->userint3 = uv;
    return 0;
}
int vdict_getuserint1(vdict_t *dict, int *uv)
{
    *uv = dict->userint1;
    return 0;
}
int vdict_getuserint2(vdict_t *dict, int *uv)
{
    *uv = dict->userint1;
    return 0;
}
int vdict_getuserint3(vdict_t *dict, int *uv)
{
    *uv = dict->userint1;
    return 0;
}

int vdict_setuservoid1(vdict_t *dict, void *uv)
{
    dict->uservoid1 = uv;
    return 0;
}
int vdict_getuservoid1(vdict_t *dict, void **uv)
{
    *uv = dict->uservoid1;
    return 0;
}


#ifdef STANDALONE

int
Zsprintf(char *dst, int dlen, void*xsrc)
{
    char *src;

    src = (char*)xsrc;
    sprintf(dst, "%s", src);
    return 0;
}

int
Zfree(void *xstr)
{
    printf("%s:\n", __func__);
    free(xstr);
    return 0;
}

/* focus '0'..'9' */

int
hash2(char *src, int radix, int seed)
{
    unsigned char *p;
    int c;
    int q[7] = {53, 29, 71, 37, 19, 83, 47};
    int i;
    unsigned int sum;
    sum = 1027+seed;
    c = 1;
    i = 0;
    p = (unsigned char *)src;
    while(*p) {
        sum <<= 1;
        sum += (*p - '5') * q[i%7] * c;
        c += 2;
        i++;
        p++;
    }
    return sum%radix;
}

test1()
{
    vdict_t dict;
    time_t  now;
    int   ck;
    char  name[BUFSIZ];
    char  val[BUFSIZ];
    int   i;
    int   j;
    int   elap;
    int   uv1; 
    int   ob_int;

    srand48(getpid());

    ck = vdict_clear(&dict);
    vdict_entryprintfunc(&dict, Zsprintf);
    vdict_entryhashfunc(&dict, hash2);

    vdict_setusagehiwater(&dict, 80);
    vdict_enableopt(&dict, VDICT_OPT_PRINTHASH | VDICT_OPT_PRINTADDR);
#if 0
    vdict_disableopt(&dict, VDICT_OPT_PRINTHASH);
#endif

    /*
     * regist many data
     */
    for(i=0;i<30000;i++)   {
#if 0
        name[0] = 'A' + lrand48()%25;
        name[1] = 'a' + lrand48()%5;
        name[2] = '\0';
#endif
        sprintf(name, "%d/%d", 
            lrand48()%8<7 ? lrand48()%16+1 : lrand48()%32+1,
            lrand48()%60+1);

        val[0] = 'A' + lrand48()%25;
        val[1] = 'a' + lrand48()%25;
        val[2] = '\0';
        ck = vdict_addorswap(&dict, name, strdup(val));
        if(ck)
            break;
    }
    vdict_show(&dict);


    /*
     * sample for user data. user can store 3 variables into a dict.
     * periodical check
     */
    ck  = vdict_setuserint1(&dict, time(NULL));

    ob_int = 20;

    for(i=0;i<20;i++) {
        now = time(NULL);
        ck  = vdict_getuserint1(&dict, &uv1);
        elap = now - uv1;
        printf("now %d; uv %d; elap %3d;\n", now, uv1, elap);
#if 0
        vdict_fshow_head(&dict, stdout);
#endif

        if(elap >= ob_int) {
            printf("*** CHECK ***\n");

            /* reset */
            ck  = vdict_setuserint1(&dict, now);
        }

        j = lrand48()%7+1;
        printf("  sleep %d", j);
        fflush(stdout);
        sleep(j);
        printf("%c[0G%c[1K", 0x1b, 0x1b);
    }

}

test2()
{
    vdict_t dict;
    time_t  now;
    int   ck;
    char  name[BUFSIZ];
    char  val[BUFSIZ];
    int   i;
    int   j;
    int   elap;
    int   uv1; 
    int   ob_int;

    srand48(getpid());

    ck = vdict_clear(&dict);
    vdict_entryprintfunc(&dict, Zsprintf);
    vdict_entrypurgefunc(&dict, Zfree);
    vdict_entryhashfunc(&dict, hash2);

    vdict_setusagehiwater(&dict, 80);
    vdict_enableopt(&dict, VDICT_OPT_PRINTHASH | VDICT_OPT_PRINTADDR);
#if 0
    vdict_disableopt(&dict, VDICT_OPT_PRINTHASH);
#endif

    /*
     * regist many data
     */
    j = 0;
    for(i=0;i<30000;i++)   {
        name[0] = 'A' + lrand48()%10;
        name[1] = '\0';

        val[0] = 'A' + lrand48()%25;
        val[1] = 'a' + lrand48()%25;
        val[2] = '\0';
        ck = vdict_addorswap(&dict, name, strdup(val));
        if(ck) {
            break;
        }
        j++;
    }
    vdict_show(&dict);
    printf("i %d, j %d\n", i, j);

}

main()
{
    test2();
}
#endif
