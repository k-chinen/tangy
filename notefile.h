/*** notefile.c ***/
#ifndef _NOTEFILE_H_
#define _NOTEFILE_H_

/* note binding */
typedef struct {
    char *nbkey;
    ob   *nbtarget;
} nb_rec;

/* notefile */
typedef struct {
    char *fn;
    int   loadcount;
} nf_rec;

extern varray_t *gnotebindlist;
extern varray_t *gnotefilelist;

nb_rec *nb_new();
nb_rec *nb_newpair(char *xk, ob *xt);
int nb_sprintf(char *dst, int dlen, void *xv, int opt);
ob *nb_find_ob(char *xkey);
nf_rec *nf_new();
nf_rec *nf_newfile(char *xfn);
int nf_sprintf(char *dst, int dlen, void *xv, int opt);
int nf_load(FILE *fp);
int nf_loadfn(char *fn);
int nf_registfilename(char *fn);
int nb_regist(char *xkey, ob *xob);
int notefile_load();
int notefile_setup();

#endif
