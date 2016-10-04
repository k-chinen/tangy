/*
 * Copyright (C) 2002-2009 StarBED Project All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * alist.c - association list
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alist.h"

int
assoc_f(apair_t *al, char *xname)
{
    int i;
    apair_t *ret;

    ret = NULL;
    i = 0;
    while(al[i].name) {
        if(strcasecmp(al[i].name, xname)==0) {
            ret = &al[i];
            break;
        }
        i++;
    }

    if(ret)
        return ret->value;
    else
        return -1;
}

int
assoc_l(apair_t *al, char *xname)
{
    int i;
    apair_t *ret;

    ret = NULL;
    i = 0;
    while(al[i].name) {
        if(strcasecmp(al[i].name, xname)==0) {
            ret = &al[i];
        }
        i++;
    }

    if(ret)
        return ret->value;
    else
        return -1;
}


/* first match */
void*
xrassoc_f(apair_t *al, int xval)
{
    int i;
    apair_t *ret;

    ret = NULL;
    i = 0;
    while(al[i].name) {
        if(al[i].value == xval) {
            ret = &al[i];
            break;
        }
        i++;
    }

    if(ret)
        return ret->name;
    else
        return NULL;
}

/* last match */
void*
xrassoc_l(apair_t *al, int xval)
{
    int i;
    apair_t *ret;

    ret = NULL;
    i = 0;
    while(al[i].name) {
        if(al[i].value == xval) {
            ret = &al[i];
        }
        i++;
    }

    if(ret)
        return ret->name;
    else
        return NULL;
}

static char *ignore_msg = "*ignore*";

char*
rassoc(apair_t *al, int xval)
{
    char *r;
    r = (char*)xrassoc_f(al, xval);
    if(r)
        return r;
    else
        return ignore_msg; 
}



int
al_count(apair_t *ax)
{
    int i;

    i = 0;
    while(ax[i].name) {
        i++;
    }

    return i;
}

apair_t*
al_dup(apair_t *ax)
{
    int an;
    int nn;
    int i;
    apair_t *rx;

    an = al_count(ax);
    nn = an+1;

#if 0
Trace("ax %p, an %d, nn %d\n", ax, an, nn);
#endif
    rx = (apair_t*)malloc(sizeof(apair_t)*nn);
    if(rx==NULL) {
        return NULL;
    }

    for(i=0;i<an;i++) {
        rx[i].name  = strdup(ax[i].name);
        rx[i].value = ax[i].value;
    }
    rx[an].name  = NULL;
    rx[an].value = -1;

    return rx;
}

