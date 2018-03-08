#include "chas.h"

#if 0
typedef struct {
    int use;
    int max;
    ch* slot;
} chas;

#ifndef DEF_CHAS_ILEN
#define DEF_CHAS_ILEN   (32)
#endif

#endif

int def_chas_ilen=DEF_CHAS_ILEN;

int
chas_initN(chas *st, int n)
{
    int i;

    if(!st) {
        return -1;
    }

    st->slot = (ch*)malloc(sizeof(ch)*n);
    if(!st->slot) {
        return -1;
    }
    st->use  = 0;
    st->max  = n;

    memset(st->slot, 0, sizeof(ch)*n);
    
    return 0;
}

int
chas_init(chas *st)
{
    return chas_initN(st, def_chas_ilen);
}

int
chas_push(chas *st, ch *src)
{
    if(!st) {
        return -1;
    }
    if(st->use>=st->max) {
        printf("%s: ERROR overun %d/%d\n", __func__, st->use, st->max);
        return -1;
    }
    
    cha_copy(&st->slot[st->use], src);
    st->use++;

    return 0;
}

int
chas_pop(chas *st, ch *dst)
{
    if(!st) {
        return -1;
    }
    if(st->use<=0) {
        printf("%s: ERROR underrun %d/%d\n", __func__, st->use, st->max);
    }
    st->use--;
    cha_copy(dst, &st->slot[st->use]);
    return 0;
}

int
chas_top(chas *st, ch *dst)
{
    if(!st) {
        return -1;
    }
    if(st->use<=0) {
        printf("%s: ERROR underrun %d/%d\n", __func__, st->use, st->max);
    }
    cha_copy(dst, &st->slot[st->use-1]);
    return 0;
}

