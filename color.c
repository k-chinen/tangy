
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
    sprintf(dst, "(%3d:%X%X%X:%s)<%5d>",
        v->num, v->rval, v->gval, v->bval, v->name, v->usecount);
#if 0
    sprintf(dst, "(%3d:%X%X%X)<%5d>",
        v->num, v->rval, v->gval, v->bval, v->usecount);
#endif

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

color_t*
pallet_addrgb(pallet_t *ar, int xn, int vr, int vg, int vb)
{
    color_t *nc;

    nc = (color_t*)malloc(sizeof(color_t));
    if(!nc) {
        printf("%s: ERROR no memory\n", __func__);
        return NULL;
    }
    memset(nc, 0, sizeof(color_t));

    nc->num  = xn;
    nc->kval = -1;
    nc->rval = vr;
    nc->gval = vg;
    nc->bval = vb;

    varray_push(ar, (void*)nc);

    return nc;
}



color_t*
pallet_addneg(pallet_t *ar, int xn)
{
    color_t *nc;

    nc = (color_t*)malloc(sizeof(color_t));
    if(!nc) {
        printf("%s: ERROR no memory\n", __func__);
        return NULL;
    }
    memset(nc, 0, sizeof(color_t));

    nc->num  = xn;
    nc->kval = -1;
    nc->rval = -1;
    nc->gval = -1;
    nc->bval = -1;

    varray_push(ar, (void*)nc);

    return nc;
}


color_t*
pallet_addrgbstr(pallet_t *ar, int xn, char *xrgbstr)
{
    color_t *nc;
    char *p;

    p = xrgbstr;

    nc = (color_t*)malloc(sizeof(color_t));
    if(!nc) {
        printf("%s: ERROR no memory\n", __func__);
        return NULL;
    }
    memset(nc, 0, sizeof(color_t));

    nc->num  = xn;
    nc->kval = -1;
    nc->rval = hstrtoi(p); p++;
    nc->gval = hstrtoi(p); p++;
    nc->bval = hstrtoi(p); p++;

    varray_push(ar, (void*)nc);

    return nc;
}

int
pallet_maxcnum(pallet_t *ar)
{
    color_t *c;
    int i;
    int cmax;
    
    cmax = -1;

    for(i=0;i<ar->use;i++) {
        c = (color_t*)ar->slot[i];
        if(c->num>cmax) {
            cmax = c->num;
        }
    }

    return cmax;
}

color_t*
pallet_findwnum(pallet_t *ar, int xn)
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

color_t*
pallet_findwname(pallet_t *ar, char *xname)
{
    color_t *pos;
    color_t *c;
    int i;
    
    pos = NULL;

    for(i=0;i<ar->use;i++) {
        c = (color_t*)ar->slot[i];
        if(c->name[0] && strcasecmp(c->name, xname)==0) {
            pos = c;
        }
    }

    return pos;
}

color_t*
pallet_findwrgb(pallet_t *ar, int xr, int xg, int xb)
{
    color_t *pos;
    color_t *c;
    int i;
    
    pos = NULL;

    for(i=0;i<ar->use;i++) {
        c = (color_t*)ar->slot[i];
        if(c->rval==xr && c->gval==xg && c->bval==xb) {
            pos = c;
        }
    }

    return pos;
}

color_t*
pallet_findaddwrgb(pallet_t *ar, int xr, int xg, int xb)
{
    color_t *cpos;
    int xx;

    cpos = pallet_findwrgb(ar, xr, xg, xb);
    if(cpos==NULL) {
        xx = pallet_maxcnum(ar);
        xx++;
        pallet_addrgb(ar, xx, xr, xg, xb);
    }
    cpos = pallet_findwrgb(ar, xr, xg, xb);

    return cpos;
}

color_t*
pallet_addnamedcolor(pallet_t* ar, char *xname, int xr, int xg, int xb)
{
    color_t *cpos;
    int xx;

    xx = pallet_maxcnum(ar);
    xx++;
    cpos = pallet_addrgb(ar, xx, xr, xg, xb);
    if(cpos) {
        strncpy(cpos->name, xname, COLORNAME_MAX-1);
        cpos->name[COLORNAME_MAX-1] = '\0';
    }

    return cpos;
}

color_t*
pallet_addnamedneg(pallet_t* ar, char *xname)
{
    color_t *cpos;

    cpos = pallet_addneg(ar, -1);
    if(cpos) {
        strncpy(cpos->name, xname, COLORNAME_MAX-1);
        cpos->name[COLORNAME_MAX-1] = '\0';
    }

    return cpos;
}

/* only gradiation of basic 8 colors, not includes others c.f. orange */

/* only gradiation of basic 8 colors, not includes others c.f. orange */
/* 5 leveles in 8 colors(R,G,b) -> 8x5 = 40 colors */
int
g2_3d(pallet_t* ar, int st)
{
    int  i;
    int  r, g, b, y;
    char val[BUFSIZ];
    for(i=0;i<40;i++) {
        r = i & 0x01;
        g = (i & 0x02)>>1;
        b = (i & 0x04)>>2;
        y = 15 - i/8*3;
        sprintf(val, "%03x", r*y*256 +  g*y*16 + b*y);
#if 0
        printf("%3d: %3d r/g/b/y %02x %02x %02x %02x |%s|\n",
            i+st, i, r, g, b, y, val);
#endif
        pallet_addrgbstr(ar,  i+st, val);
    }

    return 0;
}

/* 3 leveles in 3 axis (R,G,B) -> 3**3 = 27 colors */
int
l3_3d(pallet_t* ar, int st)
{
    int  i;
    int  r, g, b, y;
    char val[BUFSIZ];

    for(i=0;i<27;i++) {
        r = i/9;
        g = (i/3)%3;
        b = i%3;

#define AV(x)   {if(x==2) x = 15; if(x==1) x = 8; if(x==0) x = 0; }
        AV(r);
        AV(g)
        AV(b)
#undef  AV

        sprintf(val, "%03x", r*256 +  g*16 + b);
#if 0
        printf("%3d %3d r/g/b/y %02x %02x %02x %02x |%s|\n",
            i, i+st, r, g, b, y, val);
#endif
        pallet_addrgbstr(ar,  i+st, val);
    }

    return 0;
}

/* 4 leveles in 3 axis (R,G,B) -> 4**3 = 64 colors */
int
l4_3d(pallet_t* ar, int st)
{
    int  i;
    int  r, g, b, y;
    char val[BUFSIZ];

    for(i=0;i<4*4*4;i++) {
        r = i/16;
        g = (i/4)%4;
        b = i%4;

#define AV(x)   \
    {if(x==3) x = 15; if(x==2) x = 10; if(x==1) x = 5; if(x==0) x = 0; }
        AV(r);
        AV(g)
        AV(b)
#undef  AV

        sprintf(val, "%03x", r*256 +  g*16 + b);
#if 0
        printf("%3d %3d r/g/b/y %02x %02x %02x %02x |%s|\n",
            i, i+st, r, g, b, y, val);
#endif
        pallet_addrgbstr(ar,  i+st, val);
    }

    return 0;
}

/* 5 leveles in 3 axis (R,G,B) -> 5**3 = 125 colors */
int
l5_3d(pallet_t* ar, int st)
{
    int  i;
    int  r, g, b, y;
    char val[BUFSIZ];

    for(i=0;i<5*5*5;i++) {
        r = i/25;
        g = (i/5)%5;
        b = i%5;

#define AV(x)   \
    {if(x==4) x = 15; if(x==3) x = 11; \
     if(x==2) x = 7;  if(x==1) x = 4; if(x==0) x = 0; }
        AV(r);
        AV(g)
        AV(b)
#undef  AV

        sprintf(val, "%03x", r*256 +  g*16 + b);
#if 0
        printf("%3d %3d r/g/b/y %02x %02x %02x %02x |%s|\n",
            i, i+st, r, g, b, y, val);
#endif
        pallet_addrgbstr(ar,  i+st, val);
    }

    return 0;
}

/* bright and dark in 3 axis (R,G,B) */
int
set_old16_pallet(pallet_t* ar)
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

#if 0
    pallet_addrgbstr(ar,  8, "888");
    pallet_addrgbstr(ar,  8, "666");
#endif
    pallet_addrgbstr(ar,  8, "333");
    pallet_addrgbstr(ar,  9, "c00");
    pallet_addrgbstr(ar, 10, "0c0");
    pallet_addrgbstr(ar, 11, "cc0");
    pallet_addrgbstr(ar, 12, "00c");
    pallet_addrgbstr(ar, 13, "c0c");
    pallet_addrgbstr(ar, 14, "0cc");
    pallet_addrgbstr(ar, 15, "ccc");
#if 0
    pallet_addrgbstr(ar, 15, "aaa");
#endif

    return 0;
}


int
set_old48_pallet(pallet_t* ar)
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

    pallet_addrgbstr(ar,  8, "111");
    pallet_addrgbstr(ar,  9, "fcc");
    pallet_addrgbstr(ar, 10, "cfc");
    pallet_addrgbstr(ar, 11, "ffc");
    pallet_addrgbstr(ar, 12, "ccf");
    pallet_addrgbstr(ar, 13, "fcf");
    pallet_addrgbstr(ar, 14, "cff");
    pallet_addrgbstr(ar, 15, "eee");

    pallet_addrgbstr(ar, 16, "333");
    pallet_addrgbstr(ar, 17, "f66");
    pallet_addrgbstr(ar, 18, "6f6");
    pallet_addrgbstr(ar, 19, "ff6");
    pallet_addrgbstr(ar, 20, "66f");
    pallet_addrgbstr(ar, 21, "f6f");
    pallet_addrgbstr(ar, 22, "6ff");
    pallet_addrgbstr(ar, 23, "ccc");

    pallet_addrgbstr(ar, 24, "666");
    pallet_addrgbstr(ar, 25, "c00");
    pallet_addrgbstr(ar, 26, "0c0");
    pallet_addrgbstr(ar, 27, "cc0");
    pallet_addrgbstr(ar, 28, "00c");
    pallet_addrgbstr(ar, 29, "c0c");
    pallet_addrgbstr(ar, 30, "0cc");
    pallet_addrgbstr(ar, 31, "aaa");

    pallet_addrgbstr(ar, 32, "999");
    pallet_addrgbstr(ar, 33, "900");
    pallet_addrgbstr(ar, 34, "090");
    pallet_addrgbstr(ar, 35, "990");
    pallet_addrgbstr(ar, 36, "009");
    pallet_addrgbstr(ar, 37, "909");
    pallet_addrgbstr(ar, 38, "099");
    pallet_addrgbstr(ar, 39, "777");

    pallet_addrgbstr(ar, 40, "bbb");
    pallet_addrgbstr(ar, 41, "600");
    pallet_addrgbstr(ar, 42, "060");
    pallet_addrgbstr(ar, 43, "660");
    pallet_addrgbstr(ar, 44, "006");
    pallet_addrgbstr(ar, 45, "606");
    pallet_addrgbstr(ar, 46, "066");
    pallet_addrgbstr(ar, 47, "444");

    pallet_addrgbstr(ar, 50, "fd0");

    return 0;
}

int
set_default_pallet(pallet_t* ar)
{
    if(!ar) {
        return -1;
    }

    pallet_addrgbstr(ar,   0, "000");
    pallet_addrgbstr(ar, 999, "fff");

#if 0
    set_old48_pallet(ar);
#endif
#if 1
    set_old16_pallet(ar);
    l4_3d(ar, 16);

    pallet_addrgbstr(ar, 80, "fd0"); /* gold */

    return 0;
#endif

#if 0
    pallet_addrgbstr(ar,  0, "000");
    pallet_addrgbstr(ar,  1, "f00");
    pallet_addrgbstr(ar,  2, "0f0");
    pallet_addrgbstr(ar,  3, "ff0");
    pallet_addrgbstr(ar,  4, "00f");
    pallet_addrgbstr(ar,  5, "f0f");
    pallet_addrgbstr(ar,  6, "0ff");
    pallet_addrgbstr(ar,  7, "fff");

    pallet_addrgbstr(ar,  8, "111");
    pallet_addrgbstr(ar,  9, "fcc");
    pallet_addrgbstr(ar, 10, "cfc");
    pallet_addrgbstr(ar, 11, "ffc");
    pallet_addrgbstr(ar, 12, "ccf");
    pallet_addrgbstr(ar, 13, "fcf");
    pallet_addrgbstr(ar, 14, "cff");
    pallet_addrgbstr(ar, 15, "eee");

    pallet_addrgbstr(ar, 16, "333");
    pallet_addrgbstr(ar, 17, "f66");
    pallet_addrgbstr(ar, 18, "6f6");
    pallet_addrgbstr(ar, 19, "ff6");
    pallet_addrgbstr(ar, 20, "66f");
    pallet_addrgbstr(ar, 21, "f6f");
    pallet_addrgbstr(ar, 22, "6ff");
    pallet_addrgbstr(ar, 23, "ccc");

    pallet_addrgbstr(ar, 24, "666");
    pallet_addrgbstr(ar, 25, "c00");
    pallet_addrgbstr(ar, 26, "0c0");
    pallet_addrgbstr(ar, 27, "cc0");
    pallet_addrgbstr(ar, 28, "00c");
    pallet_addrgbstr(ar, 29, "c0c");
    pallet_addrgbstr(ar, 30, "0cc");
    pallet_addrgbstr(ar, 31, "aaa");

    pallet_addrgbstr(ar, 32, "999");
    pallet_addrgbstr(ar, 33, "900");
    pallet_addrgbstr(ar, 34, "090");
    pallet_addrgbstr(ar, 35, "990");
    pallet_addrgbstr(ar, 36, "009");
    pallet_addrgbstr(ar, 37, "909");
    pallet_addrgbstr(ar, 38, "099");
    pallet_addrgbstr(ar, 39, "777");

    pallet_addrgbstr(ar, 40, "bbb");
    pallet_addrgbstr(ar, 41, "600");
    pallet_addrgbstr(ar, 42, "060");
    pallet_addrgbstr(ar, 43, "660");
    pallet_addrgbstr(ar, 44, "006");
    pallet_addrgbstr(ar, 45, "606");
    pallet_addrgbstr(ar, 46, "066");
    pallet_addrgbstr(ar, 47, "444");

    pallet_addrgbstr(ar, 50, "fd0");

#endif

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
        y = pallet_findwnum(x, q);
        if(y) {
            printf("%2d %3d -> %X%X%X\n", i, q, y->rval, y->gval, y->bval);
        }
    }
    
 #if 0
    pallet_addrgbstr(x,  0, "aaa");
    pallet_fprint(stdout, x);

    for(i=1;i<argc;i++) {
        q = atoi(argv[i]);
        y = pallet_findwnum(x, q);
        if(y) {
            printf("%2d %3d -> %X%X%X\n", i, q, y->rval, y->gval, y->bval);
        }
    }
 #endif
}
#endif


