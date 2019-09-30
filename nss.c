
#include <stdio.h>

#include "alist.h"
#include "obj.h"
#include "gv.h"

#ifndef NSS_SLOT_LEN
#define NSS_SLOT_LEN (100)
#endif


int nss_n=-1;
ns* nss_slot[NSS_SLOT_LEN];

int
nss_init()
{
    int i;
    for(i=0;i<NSS_SLOT_LEN;i++) {
        nss_slot[i] = NULL;
    }
    nss_n = 0;
    return 0;
}

int
nss_dump()
{
    int i;
    for(i=0;i<nss_n;i++) {
        printf("\t%3d: %p\n", i, nss_slot[i]);
    }
    fflush(stdout);
    return 0;
}

int
nss_push(ns* x)
{
P;
    if(nss_n<0) {
        nss_init();
    }
    if(nss_n>=NSS_SLOT_LEN-1) {
        return -1;
    }
    Echo("  nss push x %p\n", x);
    nss_slot[nss_n] = x;
    nss_n++;
    return 0;
}

ns*
nss_pop()
{
    ns* r;
P;
    if(nss_n<=0) {
        Echo("ERROR chunk stack is empty\n");
        return NULL;
    }
    nss_n--;
    r = nss_slot[nss_n];
    Echo("  nss pop  r %p\n", r);
    return r;
}

int
nss_verify()
{
    extern int nss_n;
    if(nss_n<0) {
        Info("nss does not used\n");
        return 0;
    }
    else 
    if(nss_n == 0) {
        Info("nss maybe used and safe\n");
        return 0;
    }
    else {
        Warn("nss maybe used and ignore value (nss_n %d) check ]\n", nss_n);
        return 1;
    }
    return 0;
}

