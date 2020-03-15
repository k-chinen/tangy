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
 * $Id$
 */

#ifndef _VDICT_H_
#define _VDICT_H_

typedef struct {
    char *key;
    void *value;
} vdict_cell;

typedef struct {
    int   mode;
    time_t ctime;
    int   usage_hiwater;    /* hiwater (by percent)
                             *     call expand function
                             *         if use>=len*usage_hiwater/100
                             */
                            /* NOTE: lowater is nothing */
    int   len;
    int   use;
    vdict_cell *slot;

    /* option for behaviors */
    int   opt;
    int   hashseed;

    int (*hashfunc)(char*, int, int);
    int (*printfunc)(char*, int, void*);
    int (*keycmpfunc)(const void *ap, const void *bp);
    int (*purgefunc)(void*);

    /* user data */
    int   userint1;
    int   userint2;
    int   userint3;
    void *uservoid1;
} vdict_t;

#define VDICT_MODE_OHASH    'o' /* open hash */
#define VDICT_MODE_SARRAY   's' /* sorted array */
#define VDICT_MODE_NARRAY   'n' /* normal(not sorted) array */

#define VDICT_OPT_PRINTADDR             (0x0001)
#define VDICT_OPT_PRINTHASH             (0x0002)

#define VDICT_OPT_EXPANDALLOW           (0x0800)
#define VDICT_OPT_EXPANDSQUARE          (0x0400)    /* x*x      */
#define VDICT_OPT_EXPANDTHREESECONDS    (0x0200)    /* x*1.5    */
#define VDICT_OPT_EXPANDDOUBLE          (0x0100)    /* x*x      */

extern apair_t vdictmode_al[];

#ifndef VDICT_SLOTBASELEN
#define VDICT_SLOTBASELEN     (32)
#endif

int vdict_clear(vdict_t *dict);
vdict_t *vdict_new();
vdict_t *vdict_newclear();

int vdict_entryprintfunc(vdict_t *dict, int(*subf)(char *, int, void *));
int vdict_entrypurgefunc(vdict_t *dict, int(*subf)(void *));
int vdict_entryhashfunc(vdict_t *dict, int(*subf)(char *, int, int));
int vdict_entryhashseed(vdict_t *dict, int);
int vdict_setusagehiwater(vdict_t *dict, int ov);
int vdict_setopt(vdict_t *dict, int ov);
int vdict_setopt_orpos(vdict_t *dict, int ov);
int vdict_setopt_andneg(vdict_t *dict, int ov);
#define vdict_enableopt(d,v)    vdict_setopt_orpos(d, v)
#define vdict_disableopt(d,v)   vdict_setopt_andneg(d, v)

int vdict_fshow(vdict_t *dict, FILE *fp);
int vdict_fshow_wkey(vdict_t *dict, FILE *fp, char *xkey);
int vdict_show(vdict_t *dict);
int vdict_show_wkey(vdict_t *dict, char *xkey);
int vdict_fprinttable(vdict_t *dict, FILE *fp);
int vdict_printtable(vdict_t *dict);
int vdict_printtable_TeX(vdict_t *dict);

#define vdict_findpos(d,k) vdict_findpos_wkey(d,k)
vdict_cell *vdict_findpos_wkey(vdict_t *dict, char *xkey);
vdict_cell *vdict_findpos_wcharval(vdict_t *dict, char *xval);
int vdict_expand(vdict_t *dict);
int vdict_add(vdict_t *dict, char *xkey, void *xvalue);
int vdict_addorswap(vdict_t *dict, char *xkey, void *xvalue);
int vdict_del(vdict_t *dict, char *xkey);

int vdict_setuserint1(vdict_t *dict, int uv);
int vdict_setuserint2(vdict_t *dict, int uv);
int vdict_setuserint3(vdict_t *dict, int uv);
int vdict_getuserint1(vdict_t *dict, int *uv);
int vdict_getuserint2(vdict_t *dict, int *uv);
int vdict_getuserint3(vdict_t *dict, int *uv);

int vdict_setuservoid1(vdict_t *dict, void *uv);
int vdict_getuservoid1(vdict_t *dict, void **uv);


#endif
