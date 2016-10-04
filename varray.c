#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "varray.h"


#define Error   printf


#ifndef VARRAY_ILEN
#define VARRAY_ILEN (8)
#endif

int varray_ilen=VARRAY_ILEN;

int
varray_clear(varray_t *ar)
{
    int ilen;

    ilen = varray_ilen;

    ar->opt = VARRAY_OPT_EXPANDDOUBLE;

    memset(ar, 0, sizeof(*ar));
    ar->len   = 0;
    ar->slot  = (void**)malloc(sizeof(void*)*ilen);
    if(ar->slot) {
        ar->len   = ilen;
    }
    ar->use   = 0;
    memset(ar->slot, 0, sizeof(void*)*ilen);
    ar->userint1  = 0;
    ar->uservoid1 = NULL;

    ar->sprintfunc   = NULL;
#if 0
    ar->purgefunc    = NULL;
#endif
    ar->mempurgefunc = NULL;

    return 0;
}

varray_t*
varray_newraw()
{
    varray_t *r;

    r = (varray_t*) malloc(sizeof(varray_t));

    return r;
}

varray_t*
varray_new()
{
    varray_t *r;
    int ik;

    r = varray_newraw();
    if(r) {
        ik = varray_clear(r);   
    }

    return r;
}

varray_t*
varray_clone(varray_t* s)
{
    varray_t* d;
    int i;
    void** newslot;

    d = varray_newraw();
    if(!d) {
        return NULL;
    }
    memcpy(d, s, sizeof(varray_t));

    newslot = (void**)malloc(sizeof(void*)*s->len);
    memset(newslot, 0, sizeof(void*)*s->len);
    d->slot = newslot;

    for(i=0;i<s->use;i++) {
        d->slot[i] = s->slot[i];
    }
    
    return d;
}

int
varray_entrysprintfunc(varray_t *ar, int (*f)(char*,int,void*,int))
{
    if(!ar) {
        Error("null array\n");
        return -1;
    }
    if(!f) {
        Error("null function\n");
        return -1;
    }
    ar->sprintfunc = f;
    
    return 0;
}

int
varray_entrymempurgefunc(varray_t *ar, int (*f)(void*))
{
    if(!ar) {
        Error("null array\n");
        return -1;
    }
    if(!f) {
        Error("null function\n");
        return -1;
    }
    ar->mempurgefunc = f;
    
    return 0;
}

int
varray_expand(varray_t *ar)
{
    void**newslot;
    int newlen;
    int i;
    void**oldslot;
    int oldlen;

#if 0
printf("%s: called\n", __func__);
#endif
    if(!ar) {
        Error("null array\n");
        return -1;
    }
    if(ar->opt & VARRAY_OPT_EXPANDSQUARE) {
        /* XXX */
        newlen = ar->len*ar->len;
    }
    else
    if(ar->opt & VARRAY_OPT_EXPANDTHREESECONDS) {
        newlen = 3*ar->len/2;
    }
    else {
        newlen = ar->len*2;
    }
    if(newlen<ar->len) {
        Error("new length smaller than current\n");
        return -1;
    }

    oldslot = ar->slot;
    oldlen  = ar->len;

    newslot = (void**)malloc(sizeof(void*)*newlen);
    memset(newslot, 0, sizeof(void*)*newlen);

    ar->slot = newslot;
    ar->len  = newlen;

    for(i=0;i<ar->use;i++) {
        newslot[i] = oldslot[i];
    }
    free(oldslot);

#if 0
printf("  oldlen %d newlen %d\n", oldlen, newlen);
#endif
    return 0;
}


int
varray_push(varray_t *ar, void *nv)
{
    int nt;
    int ik;

    if(!ar) {
        Error("null array\n");
        return -1;
    }

    nt = 0;
    while(ar->use>=ar->len && nt<3) {
        ik = varray_expand(ar);
        if(ik) {
            Error("no slot for new value\n");
            return -1;
        }
        nt++;
    }

    ar->slot[ar->use] = nv;
    ar->use++;

    return 0;
}

void*
varray_pop(varray_t *ar)
{
    void *rv;
    if(!ar) {
        Error("null array\n");
        return NULL;
    }
    if(ar->use<=0) {
        Error("no entry in array\n");
        return NULL;
    }

    rv = ar->slot[ar->use-1];
    ar->use--;

    return rv;
}

void*
varray_top(varray_t *ar)
{
    void *rv;
    if(!ar) {
        Error("null array\n");
        return NULL;
    }
    if(ar->use>=0) {
        Error("no entry in array\n");
        return NULL;
    }

    rv = ar->slot[ar->use];
    ar->slot[ar->use] = NULL;

    return rv;
}

void*
varray_findlast(varray_t *ar)
{
    if(!ar) {
        Error("null array\n");
        return NULL;
    }
    if(ar->use<1) {
        Error("no member\n");
        return NULL;
    }
    return ar->slot[ar->use-1];
}

int
varray_member(varray_t *ar)
{
    if(!ar) {
        Error("null array\n");
        return -1;
    }
    return ar->use;
}

int
varray_fdump(FILE *fp, varray_t *ar)
{
    int i;

    if(!ar) {
        return -1;
    }
    fprintf(fp, "varray %p; %d/%d\n", ar, ar->use, ar->len);
    for(i=0;i<ar->len;i++) {
        fprintf(fp, "%4d: ", i);
        if(i<ar->use) {
            fprintf(fp, "%p\n", ar->slot[i]);
        }
        else {
            fprintf(fp, "---\n");
        }
    }
    return 0;
}

int
varray_fprint(FILE *fp, varray_t *ar)
{
    int i;
    int ik;
    char tmp[BUFSIZ];

    if(!ar) {
        return -1;
    }
    if(!ar->sprintfunc) {
        Error("sprintfunc is not registered\n");
    }

    fprintf(fp, "varray %p; %d/%d slot %p\n",
        ar, ar->use, ar->len, ar->slot);
    for(i=0;i<ar->use;i++) {
        if(!ar->sprintfunc) {
            strcpy(tmp, "---");
        }
        else {
            ik = ar->sprintfunc(tmp, BUFSIZ, ar->slot[i], 0);
        }
        fprintf(fp, "%4d: %p %s\n", i, ar->slot[i], tmp);
    }
    return 0;
}


#ifdef VARRAY_STANDALONE

int
myvarraysprintf(char *dst, int dlen, void* v, int opt)
{
    if(opt<=0) {
        sprintf(dst, "%8d", *((int*)v));
    }
    else {
        sprintf(dst, "%*d", opt, *((int*)v));
    }
    return 0;
}

main()
{
    varray_t *a;
    varray_t *b;
    int   i;
    void *x;
    int  *v;
    int   ik;

    a = varray_new();

    varray_entrysprintfunc(a, myvarraysprintf);

    for(i=0;i<13;i++) {
        v = (int*)malloc(sizeof(int));
        *v = rand()%32+1;
        x = (void*)v;
        ik = varray_push(a, x);
        if(ik) {
            break;
        }
    }

    varray_fprint(stdout, a);
    i = varray_member(a);
    printf("%d members are exist\n", i);

    x = varray_pop(a);
    printf("poped %p\n", x);

    varray_fprint(stdout, a);
    i = varray_member(a);
    printf("%d members are exist\n", i);

    b = varray_clone(a);
    varray_fprint(stdout, b);


}

#endif

