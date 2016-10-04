
#ifndef _COLOR_H_

typedef struct {
    int num;
    int kval;
    int rval;
    int gval;
    int bval;
    int usecount;
} color_t;

typedef varray_t pallet_t;

int pallet_sprint(char *dst, int dlen, void* xv, int opt);
pallet_t* pallet_new();
int pallet_fprint(FILE *fp, pallet_t *ar);
int pallet_add(pallet_t *ar, int xn, int xk, int xr, int xg, int xb);
int hstrtoi(char *s);
int pallet_addrgbstr(pallet_t *ar, int xn, char *xrgbstr);
color_t* pallet_find(pallet_t *ar, int xn);
int set_default_pallet(pallet_t* ar);
pallet_t* new_default_pallet();

#define _COLOR_H_
#endif/*_COLOR_H_*/
