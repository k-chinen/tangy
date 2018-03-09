
#include <stdio.h>
#include "obj.h"
#include "notefile.h"
#include "chas.h"
#include "seg.h"

#include "a.h"
#include "gv.h"


#ifndef CHS_SLOT_LEN
#define CHS_SLOT_LEN (100)
#endif


int chs_n=-1;
ob* chs_slot[CHS_SLOT_LEN];

int
chs_init()
{
    int i;
    for(i=0;i<CHS_SLOT_LEN;i++) {
        chs_slot[i] = NULL;
    }
    chs_n = 0;
    return 0;
}

int
chs_dump()
{
    int i;
    for(i=0;i<chs_n;i++) {
        printf("\t%3d: %p\n", i, chs_slot[i]);
    }
    fflush(stdout);
    return 0;
}

int
chs_push(ob* x)
{
    if(chs_n<0) {
        chs_init();
    }
    if(chs_n>=CHS_SLOT_LEN-1) {
        return -1;
    }
    Echo("  chs push x %p\n", x);
    chs_slot[chs_n] = x;
    chs_n++;
    return 0;
}

ob*
chs_pop()
{
    ob* r;
    if(chs_n<=0) {
        Echo("ERROR chunk stack is empty\n");
        return NULL;
    }
    chs_n--;
    r = chs_slot[chs_n];
    Echo("  chs pop  r %p\n", r);
    return r;
}
