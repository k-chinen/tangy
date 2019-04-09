#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "obj.h"
#include "gv.h"
#include "seg.h"

apair_t coord_ial[] = {
    {"nul",     NUL_COORD},
    {"rel",     REL_COORD},
    {"abs",     ABS_COORD},
    {NULL,      -1},
};

apair_t coord_isal[] = {
    {"n",   NUL_COORD},
    {"R",   REL_COORD},
    {"A",   ABS_COORD},
    {NULL,      -1},
};

apair_t ftcoord_ial[] = {
    {"nul ",    NUL_COORD},
    {"from",    COORD_FROM},
    {"to  ",    COORD_TO},
    {NULL,      -1},
};

apair_t ftcoord_isal[] = {
    {"n",       NUL_COORD},
    {"F",       COORD_FROM},
    {"T",       COORD_TO},
    {NULL,      -1},
};




seg*
seg_new()
{
    seg* na;
    na = (seg*)malloc(sizeof(seg)); 
    if(!na) {
        Error("%s: no memory for seg\n", __func__);
        return NULL;
    }
    memset(na, 0, sizeof(seg)); 
    return na;
}

int
try_regsegclose(varray_t *segar)
{
    int  ik;
    seg *e;

#if 0
    Echo("%s:\n", __func__);
    Echo("b use %d\n", segar->use);
#endif

    e = seg_new();
    if(!e) {
        return -1;
    }
    e->ptype        = OA_CLOSE;
    e->coordtype    = REL_COORD;
    ik = varray_push(segar, e);

out:
#if 0
    Echo("ik %d\n", ik);
    Echo("a use %d\n", segar->use);
#endif

    return 0;
}

int
try_regline(varray_t *segar, int x1, int y1, int x2, int y2)
{
    seg *e;

#if 0
    Echo("%s: x1,y1 %d,%d x2,y2 %d,%d\n", __func__, x1, y1, x2, y2);
    Echo("b use %d\n", segar->use);
#endif
P;
    if(segar->use>0) {
        goto out;
    }
    e = seg_new();
    if(!e) {
        return -1;
    }
    e->ptype        = OA_LINE;
    e->coordtype    = REL_COORD;
    e->x1           = x1;
    e->y1           = y1;
    e->x2           = x2;
    e->y2           = y2;
    varray_push(segar, e);

out:

    return 0;
}


int
try_regsegmoveto(varray_t *segar, int x1, int y1)
{
    int  ik;
    seg *e;

    Echo("%s: x1,y1 %d,%d\n", __func__, x1, y1);

    e = seg_new();
    if(!e) {
        return -1;
    }
    e->ptype        = OA_MOVETO;
    e->coordtype    = REL_COORD;
    e->x1           = x1;
    e->y1           = y1;
    ik = varray_push(segar, e);

out:

    return 0;
}

int
try_regsegrmoveto(varray_t *segar, int x1, int y1)
{
    seg *e;

    Echo("%s: x1,y1 %d,%d\n", __func__, x1, y1);

    e = seg_new();
    if(!e) {
        return -1;
    }
    e->ptype        = OA_RMOVETO;
    e->coordtype    = REL_COORD;
    e->x1           = x1;
    e->y1           = y1;
    varray_push(segar, e);

out:

    return 0;
}

int
try_regseglineto(varray_t *segar, int x1, int y1)
{
    seg *e;

    Echo("%s: x1,y1 %d,%d\n", __func__, x1, y1);

    e = seg_new();
    if(!e) {
        return -1;
    }
    e->ptype        = OA_LINETO;
    e->coordtype    = REL_COORD;
    e->x1           = x1;
    e->y1           = y1;
    varray_push(segar, e);

out:

    return 0;
}

int
try_regsegrlineto(varray_t *segar, int x1, int y1)
{
    seg *e;

    Echo("%s: x1,y1 %d,%d\n", __func__, x1, y1);

    e = seg_new();
    if(!e) {
        return -1;
    }
    e->ptype        = OA_RLINETO;
    e->coordtype    = REL_COORD;
    e->x1           = x1;
    e->y1           = y1;
    varray_push(segar, e);

out:

    return 0;
}

int
try_regsegforward(varray_t *segar, int x1, int y1)
{
    seg *e;

    Echo("%s: x1,y1 %d,%d\n", __func__, x1, y1);

    e = seg_new();
    if(!e) {
        return -1;
    }
    e->ptype        = OA_FORWARD;
    e->coordtype    = REL_COORD;
    e->x1           = x1;
    e->y1           = y1;
    varray_push(segar, e);

out:

    return 0;
}



int
try_regsegarc(varray_t *segar, int rad, int ang)
{
    seg *e;

    Echo("%s: rad,ang %d,%d\n", __func__, rad, ang);

    e = seg_new();
    if(!e) {
        return -1;
    }
    e->ptype        = OA_ARC;
    e->coordtype    = REL_COORD;
    e->rad          = rad;
    e->ang          = ang;
    varray_push(segar, e);

out:

    return 0;
}

int
try_regsegarcn(varray_t *segar, int rad, int ang)
{
    seg *e;

    Echo("%s: rad,ang %d,%d\n", __func__, rad, ang);

    e = seg_new();
    if(!e) {
        return -1;
    }
    e->ptype        = OA_ARCN;
    e->coordtype    = REL_COORD;
    e->rad          = rad;
    e->ang          = ang;
    varray_push(segar, e);

out:

    return 0;
}

