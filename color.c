
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "varray.h"
#include "color.h"


#define PASS    printf("%s:%d:%s PASS\n", __FILE__,__LINE__,__func__);\
                fflush(stdout);

int
pallet_sprint(char *dst, int dlen, void* xv, int opt)
{
    color_t* v;

    v = (color_t*)xv;
    sprintf(dst, "(%3d:%X%X%X:%5d)",
        v->num, v->rval, v->gval, v->bval, v->usecount);

    return 0;
}

pallet_t*
pallet_new()
{
    pallet_t *r;
    int i;

    r = varray_new();
    if(!r) {
        printf("%s: ERROR no memory\n", __func__);
        return NULL;
    }

    varray_entrysprintfunc(r, pallet_sprint);

    return r;
}

int
pallet_fprint(FILE *fp, pallet_t *ar)
{
    return varray_fprint(fp, ar);
}

int
pallet_add(pallet_t *ar, int xn, int xk, int xr, int xg, int xb)
{
    color_t *nc;

    nc = (color_t*)malloc(sizeof(color_t));
    if(!nc) {
        printf("%s: ERROR no memory\n", __func__);
        return -1;
    }
    memset(nc, 0, sizeof(color_t));

    nc->num  = xn;
    nc->kval = xk;
    nc->rval = xr;
    nc->gval = xg;
    nc->bval = xb;

    varray_push(ar, (void*)nc);

    return 0;
}

int
hstrtoi(char *s)
{
    int d;

    if(*s>='0'&&*s<='9') {
        d = *s - '0';
    }
    else
    if(*s>='A'&&*s<='F') {
        d = *s - 'A' + 10;
    }
    else
    if(*s>='a'&&*s<='f') {
        d = *s - 'a' + 10;
    }
    else {
        d = -1;
    }
#if 0
    printf(" *s |%c| -> d %d\n", *s, d);
#endif
    return d;
}

int
pallet_addrgbstr(pallet_t *ar, int xn, char *xrgbstr)
{
    color_t *nc;
    char *p;

    p = xrgbstr;

    nc = (color_t*)malloc(sizeof(color_t));
    if(!nc) {
        printf("%s: ERROR no memory\n", __func__);
        return -1;
    }
    memset(nc, 0, sizeof(color_t));

    nc->num  = xn;
    nc->kval = -1;
    nc->rval = hstrtoi(p); p++;
    nc->gval = hstrtoi(p); p++;
    nc->bval = hstrtoi(p); p++;

    varray_push(ar, (void*)nc);


    return 0;
}


color_t*
pallet_find(pallet_t *ar, int xn)
{
    color_t *pos;
    color_t *c;
    int i;
    
    pos = NULL;

    for(i=0;i<ar->use;i++) {
        c = (color_t*)ar->slot[i];
        if(c->num==xn) {
            pos = c;
        }
    }

    return pos;
}



int
set_default_pallet(pallet_t* ar)
{
    if(!ar) {
        return -1;
    }

    pallet_addrgbstr(ar,  0, "000");
    pallet_addrgbstr(ar,  1, "f00");
    pallet_addrgbstr(ar,  2, "0f0");
    pallet_addrgbstr(ar,  3, "ff0");
    pallet_addrgbstr(ar,  4, "00f");
    pallet_addrgbstr(ar,  5, "f0f");
    pallet_addrgbstr(ar,  6, "0ff");
    pallet_addrgbstr(ar,  7, "fff");

    pallet_addrgbstr(ar,  8, "666");
    pallet_addrgbstr(ar,  9, "f66");
    pallet_addrgbstr(ar, 10, "6f6");
    pallet_addrgbstr(ar, 11, "ff6");
    pallet_addrgbstr(ar, 12, "66f");
    pallet_addrgbstr(ar, 13, "f6f");
    pallet_addrgbstr(ar, 14, "6ff");
    pallet_addrgbstr(ar, 15, "fff");

    pallet_addrgbstr(ar, 16, "000");
    pallet_addrgbstr(ar, 17, "600");
    pallet_addrgbstr(ar, 18, "060");
    pallet_addrgbstr(ar, 19, "660");
    pallet_addrgbstr(ar, 20, "006");
    pallet_addrgbstr(ar, 21, "606");
    pallet_addrgbstr(ar, 22, "066");
    pallet_addrgbstr(ar, 23, "666");

    return 0;
}

pallet_t*
new_default_pallet()
{
    pallet_t *np;

    np = pallet_new();
    if(np) {
        set_default_pallet(np);
    }

    return np;
}


#ifdef COLOR_STANDALONE
int
main(int argc, char *argv[])
{
    pallet_t *x;
    color_t *y;
    int q;
    int i;

    x = new_default_pallet();

    pallet_fprint(stdout, x);

    for(i=1;i<argc;i++) {
        q = atoi(argv[i]);
        y = pallet_find(x, q);
        if(y) {
            printf("%2d %3d -> %X%X%X\n", i, q, y->rval, y->gval, y->bval);
        }
    }
    
 #if 0
    pallet_addrgbstr(x,  0, "aaa");
    pallet_fprint(stdout, x);

    for(i=1;i<argc;i++) {
        q = atoi(argv[i]);
        y = pallet_find(x, q);
        if(y) {
            printf("%2d %3d -> %X%X%X\n", i, q, y->rval, y->gval, y->bval);
        }
    }
 #endif
}
#endif


