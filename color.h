
#ifndef _COLOR_H_

#ifndef COLORNAME_MAX
#define COLORNAME_MAX   (16)
#endif
#if 0
#endif

typedef struct {
    int  num;
    char name[COLORNAME_MAX];
    int  kval;
    int  rval;
    int  gval;
    int  bval;
    int  usecount;
} color_t;

typedef varray_t pallet_t;

int pallet_sprint(char *dst, int dlen, void* xv, int opt);
pallet_t* pallet_new();
int pallet_fprint(FILE *fp, pallet_t *ar);
int pallet_add(pallet_t *ar, int xn, int xk, int xr, int xg, int xb);
int hstrtoi(char *s);
color_t* pallet_addrgb(pallet_t *ar, int xn, int, int, int);
color_t* pallet_addrgbstr(pallet_t *ar, int xn, char *xrgbstr);
color_t* pallet_addnamedcolor(pallet_t *ar, char *xname, int, int, int);
color_t* pallet_addnamedneg(pallet_t *ar, char *xname);
color_t* pallet_findwnum(pallet_t *ar, int xn);
color_t* pallet_findwrgb(pallet_t *ar, int, int, int);
color_t* pallet_findaddwrgb(pallet_t *ar, int, int, int);
color_t* pallet_findwname(pallet_t *ar, char *);
int set_default_pallet(pallet_t* ar);
pallet_t* new_default_pallet();

#define _COLOR_H_
#endif/*_COLOR_H_*/
