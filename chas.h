#ifndef _CHAS_H_
#define _CHAS_H_
typedef struct {
    int use;
    int max;
    ch* slot;
} chas;

#ifndef DEF_CHAS_ILEN
#define DEF_CHAS_ILEN   (32)
#endif

extern int def_chas_ilen;

int chas_initN(chas *st, int n);
int chas_init(chas *st);
int chas_push(chas *st, ch *src);
int chas_pop(chas *st, ch *dst);
int chas_top(chas *st, ch *dst);

#endif /* _CHAS_H_ */
