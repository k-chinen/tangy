/***
 *** EPS output routines
 ***/

#include <stdio.h>

#include "alist.h"
#include "word.h"

#include "obj.h"
#include "gv.h"
#include "seg.h"
#include "chas.h"
#include "notefile.h"
#include "font.h"
#include "tx.h"
#include "a.h"
#include "bez.h"
#include "xns.h"
#include "put.h"
#include "xcur.h"
#include "epsdraw.h"

#include "qbb.h"

#define _umo(lv,hv,av) \
    (((av)<(lv)? QBB_M_U : ((av)<=(hv) ? QBB_M_M : QBB_M_O)))


#ifndef EPSOUTMARGIN
#define EPSOUTMARGIN    (18)    /* 1/4 inch */
#endif

/* outside of scaling */
int epsoutmargin        = EPSOUTMARGIN;

/* inside of scaling */
int epsdraftfontsize    = 10;
int epsdraftgap         =  5;

char *def_fontname      = "Times-Roman";

int   def_markcolor     = 5;
int   def_guide1color    = 4;
int   def_guide2color    = 2;
#define     def_guidecolor  def_guide1color

int debug_clip = 0;

#define PP  fprintf(fp, "%% PASS %s:%d\n", __func__, __LINE__); fflush(fp);

#define SLW_x(fp,x) \
    fprintf(fp, "    %d setlinewidth\n", x);
#define SLW_1x(fp,x) \
    fprintf(fp, "    currentlinewidth %d div setlinewidth\n", x);
#define SLW_x1(fp,x) \
    fprintf(fp, "    currentlinewidth %d mul setlinewidth\n", x);
#define SLW_12(fp) fprintf(fp, "    currentlinewidth 2 div setlinewidth\n");
#define SLW_14(fp) fprintf(fp, "    currentlinewidth 4 div setlinewidth\n");
#define SLW_21(fp) fprintf(fp, "    currentlinewidth 2 mul setlinewidth\n");
#define SLW_41(fp) fprintf(fp, "    currentlinewidth 4 mul setlinewidth\n");


/*
 * Marking
 *      MX  cross
 *      MP  plus
 *      MC  circle
 *      MCF circle fill
 *      MQ  square
 *      MQF square fill
 *      MT  triangle
 *      MTF triangle fill
 */

#define ML(c,x1,y1,x2,y2) \
    fprintf(fp, "    %% ML %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      currentlinewidth 4 div setlinewidth\n"); \
    fprintf(fp, "      newpath %d %d moveto %d %d lineto stroke grestore\n", x1, y1, x2, y2); 

#define MX(c,x,y) \
    fprintf(fp, "    %% MX %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      currentlinewidth 4 div setlinewidth\n"); \
    fprintf(fp, "      newpath %d %d moveto %d %d rlineto\n", x-objunit/40, y-objunit/40, 2*objunit/40, 2*objunit/40); \
    fprintf(fp, "      %d %d moveto %d %d rlineto stroke grestore\n", x-objunit/40, y+objunit/40, 2*objunit/40, -2*objunit/40); 
#define MP(c,x,y) \
    fprintf(fp, "    %% MP %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      currentlinewidth 4 div setlinewidth\n"); \
    fprintf(fp, "      newpath %d %d moveto %d 0 rlineto\n", x-objunit/40, y, 2*objunit/40); \
    fprintf(fp, "      %d %d moveto 0 %d rlineto stroke grestore\n", x, y-objunit/40, 2*objunit/40);
#define MC(c,x,y) \
    fprintf(fp, "    %% MC %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      currentlinewidth 4 div setlinewidth\n"); \
    fprintf(fp, "      newpath %d %d %d 0 360 arc stroke grestore\n", x, y, objunit/40);
#define MCF(c,x,y) \
    fprintf(fp, "    %% MCF %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      currentlinewidth 4 div setlinewidth\n"); \
    fprintf(fp, "      newpath %d %d %d 0 360 arc fill grestore\n", x, y, objunit/40);
#define MQ(c,x,y) \
    fprintf(fp, "    %% MQ %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      currentlinewidth 4 div setlinewidth\n"); \
    fprintf(fp, "      newpath %d %d moveto %d 0 rlineto 0 %d rlineto %d neg 0 rlineto closepath stroke grestore\n", x-objunit/40, y-objunit/40, 2*objunit/40, 2*objunit/40, 2*objunit/40); 
#define MQF(c,x,y) \
    fprintf(fp, "    %% MQF %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      currentlinewidth 4 div setlinewidth\n"); \
    fprintf(fp, "      newpath %d %d moveto %d 0 rlineto 0 %d rlineto %d neg 0 rlineto closepath fill grestore\n", x-objunit/40, y-objunit/40, 2*objunit/40, 2*objunit/40, 2*objunit/40); 
#define MT(c,x,y,a) \
    fprintf(fp, "    %% MT %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      currentlinewidth 4 div setlinewidth\n"); \
    fprintf(fp, "      %d %d translate %d rotate\n", x, y, a); \
    fprintf(fp, "      %d %d translate\n", -2*objunit/40, 0); \
    fprintf(fp, "      0 %d moveto 0 %d lineto %d 0 lineto\n", -objunit/40, objunit/40, objunit/40*2); \
    fprintf(fp, "      closepath stroke\n"); \
    fprintf(fp, "      grestore\n");
#define MTF(c,x,y,a) \
    fprintf(fp, "    %% MTF %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      currentlinewidth 4 div setlinewidth\n"); \
    fprintf(fp, "      %d %d translate %d rotate\n", x, y, a); \
    fprintf(fp, "      %d %d translate\n", -2*objunit/40, 0); \
    fprintf(fp, "      0 %d moveto 0 %d lineto %d 0 lineto\n", -objunit/40, objunit/40, objunit/40*2); \
    fprintf(fp, "      closepath fill\n"); \
    fprintf(fp, "      grestore\n");

double
solve_pitch(int ty)
{
    double rv;

    rv = def_linedecopitch; 

    switch(ty) {
    case LT_LMUST:          rv = def_linedecopitch;   break;
    case LT_RMUST:          rv = def_linedecopitch;   break;
    case LT_DASHED:         rv = def_linedecopitch;     break;
    case LT_DOTTED:         rv = def_linedecopitch/2;   break;
    case LT_CHAINED:        rv = def_linedecopitch/2;   break;
    case LT_DOUBLECHAINED:  rv = def_linedecopitch/2;   break;
    default:
    case LT_CIRCLE:         rv = def_linedecopitch*2;   break;
    }

    Echo("%s: rv %.3f ty %d def_linedecopitch %d\n",
        __func__, rv, ty, def_linedecopitch);

    return rv;
}

int
solve_dashpart(int ltype, int j)
{
    int act;

    act = -1;

    switch(ltype) {
    case LT_DOTTED:
    case LT_DASHED:
            act = 0;
            if(j%2==0) { act = 1; }
            break;
    case LT_CHAINED:
            act = 1;
            if(j%7==4||j%7==6) { act = 0; }
            break;
    case LT_DOUBLECHAINED:
            act = 1;
            if(j%9==4||j%9==6||j%9==8) { act = 0; }
            break;
    default:
            break;
    }
    
    return act;
}

int
psescape(char *dst, int dlen, char* src)
{
    char *p, *q;
    int   c;

    p = src;
    q = dst;
    c = 0;
    while(*p && c<dlen-2) {
        if(*p=='(' && c<dlen-4) {
            p++;
            *q++ = '\\'; *q++ = '0'; *q++ = '5'; *q++ = '0'; c+=4;
        }
        else
        if(*p==')' && c<dlen-4) { p++;
            *q++ = '\\'; *q++ = '0'; *q++ = '5'; *q++ = '1'; c+=4;
        }
        else {
            *q++ = *p++;
            c++;
        }
    }
    *q = '\0';

#if 1
Echo("%s: '%s' <- '%s'\n", __func__, dst, src);
#endif

    return 0;
}

int
changethick(FILE *fp, int lth)
{
    if(lth<0) {
        return 0;
    }
    /*           12345678*/
    fprintf(fp, "        %d setlinewidth\n", lth);

    return 0;
}

int
changecolor(FILE *fp, int cn)
{
    color_t *c;
    char tmp[BUFSIZ];

    if(cn<0) {
        return 0;
    }

    c = pallet_find(pallet, cn);
    if(c) {
        /*           12345678*/
        sprintf(tmp, "       %.2f %.2f %.2f setrgbcolor %% cn %d\n",
            (double) c->rval / 15,
            (double) c->gval / 15,
            (double) c->bval / 15,
            cn);
        c->usecount++;
    }
    else {
        /*           12345678*/
        sprintf(tmp, "       0 setgray %% unresolved color %d\n", cn);
    }
    fputs(tmp, fp);

    return 0;
}


static int
changehot(FILE *fp)
{
    fprintf(fp, "1 0 0 setrgbcolor\n");
    fprintf(fp, "/Courier findfont %d scalefont setfont\n",
        epsdraftfontsize);
    return 0;
}

static int
changeground(FILE *fp)
{
    fprintf(fp, "0.8 1 0.8 setrgbcolor\n");
    fprintf(fp, "/Courier findfont %d scalefont setfont\n",
        epsdraftfontsize);
    return 0;
}

static int
changebbox(FILE *fp)
{
    fprintf(fp, "0.3 1 0.3 setrgbcolor\n");
    fprintf(fp, "/Courier findfont %d scalefont setfont\n",
        epsdraftfontsize);
    return 0;
}

static int
changedraft(FILE *fp)
{
    fprintf(fp, "0 0.8 1 setrgbcolor\n");
    fprintf(fp, "/Courier findfont %d scalefont setfont\n",
        epsdraftfontsize);
    return 0;
}

static int
changetext(FILE *fp)
{
    fprintf(fp, "        1 0.5 0 setrgbcolor %% %s\n", __func__);
#if 0
    fprintf(fp, "    /Courier findfont %d scalefont setfont\n",
        epsdraftfontsize);
#endif
    return 0;
}

static int
changetext2(FILE *fp)
{
    fprintf(fp, "        1 1 0 setrgbcolor %% %s\n", __func__);
#if 0
    fprintf(fp, "    /Courier findfont %d scalefont setfont\n",
        epsdraftfontsize);
#endif
    return 0;
}

static int
changetext3(FILE *fp)
{
    fprintf(fp, "    0.3 0.7 0.7 setrgbcolor\n");
#if 0
    fprintf(fp, "    /Courier findfont %d scalefont setfont\n",
        epsdraftfontsize);
#endif
    return 0;
}

static int
changenormal(FILE *fp)
{
    fprintf(fp, "0 setgray %% as normal color\n");
    return 0;
}


/* centerized rotated rect */
static int
drawCRrectMJ(FILE *fp, int x1, int y1, int wd, int ht, int ro, char *msg, int sj)
{
    fprintf(fp, "    gsave %% CRrect\n");
    fprintf(fp, "      %d %d translate\n", x1, y1);
    fprintf(fp, "      0 0 moveto %d rotate\n", ro);
    fprintf(fp, "      newpath\n");
    fprintf(fp, "      %d %d moveto\n",   -wd/2, -ht/2);
    fprintf(fp, "      %d %d lineto\n",    wd/2, -ht/2);
    fprintf(fp, "      %d %d lineto\n",    wd/2,  ht/2);
    fprintf(fp, "      %d %d lineto\n",   -wd/2,  ht/2);
    fprintf(fp, "      closepath\n");
    fprintf(fp, "      stroke\n");
    if(msg && *msg) {
        if(sj==SJ_CENTER) {
            fprintf(fp, "      %d %d (%s) cshow\n", 0, ht*6/10, msg);
        }
        else
        if(sj==SJ_RIGHT) {
            fprintf(fp, "      %d %d (%s) rshow\n", 0, ht*6/10, msg);
        }
        else {
            /* otherwise,  SJ_LEFT */
            fprintf(fp, "      %d %d (%s) lshow\n", 0, ht*6/10, msg);
        }
    }
    fprintf(fp, "    grestore\n");

    return 0;
}
static int
drawCRrectM(FILE *fp, int x1, int y1, int wd, int ht, int ro, char *msg)
{
    drawCRrectMJ(fp, x1, y1, wd, ht, ro, msg, SJ_CENTER);
    return 0;
}

static int
drawCRrect(FILE *fp, int x1, int y1, int wd, int ht, int ro)
{
    drawCRrectM(fp, x1, y1, wd, ht, ro, NULL); 

    return 0;
}

static int
drawCRrectskel(FILE *fp, int x1, int y1, int wd, int ht, int ro)
{
    int q;

    q = objunit/10;

    fprintf(fp, "  gsave %% CRrectskel\n");
    fprintf(fp, "    %d %d translate\n", x1, y1);
    fprintf(fp, "    0 0 moveto %d rotate\n", ro);
    fprintf(fp, "    newpath\n");

    fprintf(fp, "    %d %d moveto\n",   -wd/2, -ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   -q);
    fprintf(fp, "    %d %d moveto\n",   -wd/2, -ht/2);
    fprintf(fp, "    0 %d rlineto\n",   -q);

    fprintf(fp, "    %d %d moveto\n",   -wd/2, ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   -q);
    fprintf(fp, "    %d %d moveto\n",   -wd/2, ht/2);
    fprintf(fp, "    0 %d rlineto\n",   q);

    fprintf(fp, "    %d %d moveto\n",   wd/2, ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   q);
    fprintf(fp, "    %d %d moveto\n",   wd/2, ht/2);
    fprintf(fp, "    0 %d rlineto\n",   q);

    fprintf(fp, "    %d %d moveto\n",   wd/2, -ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   q);
    fprintf(fp, "    %d %d moveto\n",   wd/2, -ht/2);
    fprintf(fp, "    0 %d rlineto\n",   -q);

    fprintf(fp, "    stroke\n");
    fprintf(fp, "  grestore\n");

    return 0;
}

static int
drawCRrectskel2(FILE *fp, int x1, int y1, int wd, int ht, int ro)
{
    int q;

    q = objunit/10;

    fprintf(fp, "  gsave %% CRrectskel2\n");
    fprintf(fp, "    %d %d translate\n", x1, y1);
    fprintf(fp, "    0 0 moveto %d rotate\n", ro);
    fprintf(fp, "    newpath\n");

    fprintf(fp, "    %d %d moveto\n",   -wd/2+q, -ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   -q);
    fprintf(fp, "    0 %d rlineto\n",   q);

    fprintf(fp, "    %d %d moveto\n",   -wd/2+q, ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   -q);
    fprintf(fp, "    0 %d rlineto\n",   -q);

    fprintf(fp, "    %d %d moveto\n",   wd/2-q, ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   q);
    fprintf(fp, "    0 %d rlineto\n",   -q);

    fprintf(fp, "    %d %d moveto\n",   wd/2-q, -ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   q);
    fprintf(fp, "    0 %d rlineto\n",   q);

    fprintf(fp, "    stroke\n");
    fprintf(fp, "  grestore\n");

    return 0;
}


static int
XdrawCRrectskel2(FILE *fp, int x1, int y1, int wd, int ht, int ro)
{
    int q;

    q = objunit/10;

    fprintf(fp, "  gsave %% XCRrectskel2\n");
    fprintf(fp, "    %d %d translate\n", x1, y1);
    fprintf(fp, "    0 0 moveto %d rotate\n", ro);
    fprintf(fp, "    newpath\n");

    fprintf(fp, "    %d %d moveto\n",   -wd/2, -ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   q);
    fprintf(fp, "    %d %d moveto\n",   -wd/2, -ht/2);
    fprintf(fp, "    0 %d rlineto\n",   q);

    fprintf(fp, "    %d %d moveto\n",   -wd/2, ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   q);
    fprintf(fp, "    %d %d moveto\n",   -wd/2, ht/2);
    fprintf(fp, "    0 %d rlineto\n",   -q);

    fprintf(fp, "    %d %d moveto\n",   wd/2, ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   -q);
    fprintf(fp, "    %d %d moveto\n",   wd/2, ht/2);
    fprintf(fp, "    0 %d rlineto\n",   -q);

    fprintf(fp, "    %d %d moveto\n",   wd/2, -ht/2);
    fprintf(fp, "    %d 0 rlineto\n",   -q);
    fprintf(fp, "    %d %d moveto\n",   wd/2, -ht/2);
    fprintf(fp, "    0 %d rlineto\n",   q);

    fprintf(fp, "    stroke\n");
    fprintf(fp, "  grestore\n");

    return 0;
}


static int
drawCrect(FILE *fp, int x1, int y1, int wd, int ht)
{
    return drawCRrect(fp, x1, y1, wd, ht, 0);
}

/* 4courner rect */
static int
drawrect(FILE *fp, int x1, int y1, int x2, int y2)
{
    fprintf(fp, "  newpath %% rect\n");
    fprintf(fp, "  %d %d moveto\n", x1, y1);
    fprintf(fp, "  %d %d lineto\n", x1, y2);
    fprintf(fp, "  %d %d lineto\n", x2, y2);
    fprintf(fp, "  %d %d lineto\n", x2, y1);
    fprintf(fp, "  closepath\n");
    fprintf(fp, "  stroke\n");

    return 0;
}

static int
drawrectcm(FILE *fp, int x1, int y1, int x2, int y2, char *cm)
{
    fprintf(fp, "  newpath %% rectcm\n");
    fprintf(fp, "  %d %d moveto\n", x1, y1);
    fprintf(fp, "  %d %d lineto\n", x1, y2);
    fprintf(fp, "  %d %d lineto\n", x2, y2);
    fprintf(fp, "  %d %d lineto\n", x2, y1);
    fprintf(fp, "  closepath\n");
    fprintf(fp, "  stroke\n");
    fprintf(fp, "  %d %d moveto\n", x1, y2);
    fprintf(fp, "  0 currentlinewidth 1 mul rmoveto\n");
    fprintf(fp, "  (%s) show\n", cm);

    return 0;
}

static int
Xdrawrectcm(FILE *fp, int x1, int y1, int x2, int y2, char *cm)
{
    x1 -= epsdraftgap;
    y1 -= epsdraftgap;
    x2 += epsdraftgap;
    y2 += epsdraftgap;
    fprintf(fp, "  newpath %% Xrectcm\n");
    fprintf(fp, "  %d %d moveto\n", x1, y1);
    fprintf(fp, "  %d %d lineto\n", x1, y2);
    fprintf(fp, "  %d %d lineto\n", x2, y2);
    fprintf(fp, "  %d %d lineto\n", x2, y1);
    fprintf(fp, "  closepath\n");
    fprintf(fp, "  stroke\n");
    fprintf(fp, "  %d %d moveto\n", x1, y2+epsdraftgap);
    fprintf(fp, "  (%s) show\n", cm);

    return 0;
}

static int
drawCRrectG(FILE *fp, int x1, int y1, int wd, int ht, int ro, int gl)
{
    fprintf(fp, "  gsave\n");
    fprintf(fp, "    %d %d translate\n", x1, y1);
    fprintf(fp, "    0 0 moveto %d rotate\n", ro);

    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                    -wd/2, -ht/2,   0, -gl);
    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                    -wd/2, -ht/2, -gl,   0);

    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                     wd/2, -ht/2,   0, -gl);
    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                     wd/2, -ht/2,  gl,   0);

    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                     wd/2,  ht/2,   0,  gl);
    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                     wd/2,  ht/2,  gl,   0);
    
    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                    -wd/2,  ht/2,   0,  gl);
    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                    -wd/2,  ht/2, -gl,   0);

    fprintf(fp, "  grestore\n");

    return 0;
}


int
epsdraw_bbox_glbrt(FILE *fp, ob *xu)
{
    char msg[BUFSIZ];

    if(INTRACE) {
    fprintf(fp, "  %% bbox guide oid %d with GLBRT (%d %d %d %d) by %s\n",
        xu->oid, xu->glx, xu->gby, xu->grx, xu->gty, __func__);
    }
    fprintf(fp, "  gsave %% for bbox of oid %d by %s\n", xu->oid, __func__);
    changecolor(fp, 5);

#if 0
    drawCrect(fp, (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2,
        xu->grx-xu->glx, xu->gty-xu->gby);
#endif
    sprintf(msg, "oid %d", xu->oid);
    drawCRrectMJ(fp, (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2,
        xu->grx-xu->glx, xu->gty-xu->gby, 0, msg, SJ_RIGHT);

    if(0*xu->cob.rotateval) {
    }
    fprintf(fp, "  grestore %% for bbox\n");

    return 0;
}

int
epsdraw_bbox_glbrtB(FILE *fp, ob *xu)
{
    char msg[BUFSIZ];

    if(INTRACE) {
        fprintf(fp, "  %% bbox guide oid %d with GLBRT (%d %d %d %d) by %s\n",
            xu->oid, xu->glx, xu->gby, xu->grx, xu->gty, __func__);
    }
    fprintf(fp, "  gsave %% for bbox of oid %d by %s\n", xu->oid, __func__);
    fprintf(fp, "    currentlinewidth 40 mul setlinewidth\n");
    changecolor(fp, 2);

#if 0
    drawCrect(fp, (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2,
        xu->grx-xu->glx, xu->gty-xu->gby);
#endif
    sprintf(msg, "oid %d", xu->oid);
    drawCRrectMJ(fp, (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2,
        xu->grx-xu->glx, xu->gty-xu->gby, 0, msg, SJ_RIGHT);

    if(0*xu->cob.rotateval) {
    }
    fprintf(fp, "  grestore %% for bbox\n");

    return 0;
}

int
epsdraw_bbox_glbrtR(FILE *fp, ob *xu)
{
    char msg[BUFSIZ];
    int  br;
extern int _box_path(FILE *fp, int x1, int y1, int aw, int ah, int r, int op);

    if(INTRACE) {
    fprintf(fp, "  %% bbox guide oid %d with GLBRT (%d %d %d %d) by %s\n",
        xu->oid, xu->glx, xu->gby, xu->grx, xu->gty, __func__);
    }
    fprintf(fp, "  gsave %% for bbox of oid %d by %s\n", xu->oid, __func__);
    changecolor(fp, 5);
    changethick(fp, def_markbbthick);

    br = objunit/16;

#if 0
    drawCrect(fp, (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2,
        xu->grx-xu->glx, xu->gty-xu->gby);
#endif
    sprintf(msg, "oid %d", xu->oid);
#if 0
    drawCRrectMJ(fp, (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2,
        xu->grx-xu->glx, xu->gty-xu->gby, 0, msg, SJ_RIGHT);
#endif
#if 0
    _box_path(fp, (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2,
        xu->grx-xu->glx, xu->gty-xu->gby, objunit/8, 1);
#endif
    fprintf(fp, "    %d %d translate\n",
        (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2
        );
#if 0
    _box_path(fp, (xu->glx), (xu->gby)/2,
        xu->grx-xu->glx, xu->gty-xu->gby, objunit/8, 1);
#endif
    _box_path(fp, (xu->glx), (xu->gby)/2,
        xu->grx-xu->glx+br*2, xu->gty-xu->gby+br*2, br, 1);

    if(0*xu->cob.rotateval) {
    }
    fprintf(fp, "  grestore %% for bbox\n");

    return 0;
}

int
epsdraw_bbox_lbrt(FILE *fp, int xox, int xoy, ob *xu)
{
    char msg[BUFSIZ];

    if(INTRACE) {
        fprintf(fp,
            "%% bbox guide oid %d with %d,%d (%d %d %d %d) by %s\n",
            xu->oid, xox, xoy, xu->lx, xu->by, xu->rx, xu->ty, __func__);
    }
    fprintf(fp, "  gsave %% for bbox of oid %d by %s\n", xu->oid, __func__);
    changecolor(fp, 3);
#if 0
    drawCrect(fp, xox+(xu->lx+xu->rx)/2, xoy+(xu->by+xu->ty)/2,
        xu->rx-xu->lx, xu->ty-xu->by);
#endif
    sprintf(msg, "oid %d", xu->oid);
    drawCRrectMJ(fp, (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2,
        xu->grx-xu->glx, xu->gty-xu->gby, 0, msg, SJ_LEFT);

    if(0*xu->cob.rotateval) {
    }
    fprintf(fp, "  grestore %% for bbox\n");

    return 0;
}

int
epsdraw_bbox_lbrtR(FILE *fp, int xox, int xoy, ob *xu)
{
    char msg[BUFSIZ];

    if(INTRACE) {
        fprintf(fp,
            "%% bbox guide oid %d with %d,%d (%d %d %d %d) by %s\n",
            xu->oid, xox, xoy, xu->lx, xu->by, xu->rx, xu->ty, __func__);
    }
    fprintf(fp, "  gsave %% for bbox of oid %d by %s\n", xu->oid, __func__);
    changecolor(fp, 3);
#if 1
    drawCrect(fp, xox+(xu->lx+xu->rx)/2, xoy+(xu->by+xu->ty)/2,
        xu->rx-xu->lx, xu->ty-xu->by);
#endif
#if 0
    sprintf(msg, "oid %d", xu->oid);
    drawCRrectMJ(fp, (xu->glx+xu->grx)/2, (xu->gby+xu->gty)/2,
        xu->grx-xu->glx, xu->gty-xu->gby, 0, msg, SJ_LEFT);
#endif

    if(0*xu->cob.rotateval) {
    }
    fprintf(fp, "  grestore %% for bbox\n");

    return 0;
}


#define epsdraw_bbox    epsdraw_bbox_glbrtR



double
_arrowheadD(FILE *fp, int atype, double xdir, int lc, double x, double y)
{
    double  r;
    double  dx, dy;
    double  r2;

#if 0
    fprintf(fp, "%% arrowhead atype %d xdir %.2f lc %d x,y %.3f,%.3f\n",
        atype, xdir, lc, x, y);
#endif
    fprintf(fp, "gsave %% for AH\n");

    changecolor(fp, lc);

    switch(atype) {
    case AH_DIAMOND:
    case AH_WDIAMOND:
        r = def_arrowsize/3;

        if(atype==AH_WDIAMOND) {
        fprintf(fp,"gsave\n");
        fprintf(fp,"1 setgray\n");
        fprintf(fp, "%.3f %.3f moveto\n",    x,  y+r);
        fprintf(fp, "%.3f %.3f rlineto\n",   r, -r);
        fprintf(fp, "%.3f %.3f rlineto\n",  -r, -r);
        fprintf(fp, "%.3f %.3f rlineto\n",  -r,  r);
        fprintf(fp, "%.3f %.3f rlineto\n",   r,  r);
        fprintf(fp, "fill\n");
        fprintf(fp,"grestore\n");
        }

        fprintf(fp, "%.3f %.3f moveto\n",    x,  y+r);
        fprintf(fp, "%.3f %.3f rlineto\n",   r, -r);
        fprintf(fp, "%.3f %.3f rlineto\n",  -r, -r);
        fprintf(fp, "%.3f %.3f rlineto\n",  -r,  r);
        fprintf(fp, "%.3f %.3f rlineto\n",   r,  r);
        if(atype==AH_WDIAMOND) {
            fprintf(fp, "stroke\n");
        }
        else {
            fprintf(fp, "fill\n");
        }

        break;

    case AH_WCIRCLE:
    case AH_CIRCLE:
        r = def_arrowsize/3;

        if(atype==AH_WCIRCLE) {
        fprintf(fp,"gsave\n");
        fprintf(fp,"1 setgray\n");
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f %.3f 0 360 arc\n", x, y, r);
        fprintf(fp, "fill\n");
        fprintf(fp,"grestore\n");
        }

#if 0
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
#endif
        fprintf(fp, "newpath\n");
        fprintf(fp, "%.3f %.3f %.3f 0 360 arc\n", x, y, r);
        if(atype==AH_WCIRCLE) {
        fprintf(fp, "stroke\n");
        }
        else {
        fprintf(fp, "fill\n");
        }


        break;

    case AH_SHIP:
    case AH_WSHIP:
        {   
        double k;
        double th;
        double d;
        k = objunit/4;
        r = k;
        th = acos(0.5);
        d = (double)((double)r - (double)r*sin(th));
        fprintf(fp, "%% th %.4f; k %.3f d %.3f\n", th, k, d);

        fprintf(fp, "  gsave\n");
        fprintf(fp, "    %.3f %.3f translate\n", x, y);
        fprintf(fp, "    %.2f rotate\n", xdir);

        fprintf(fp, "    %d %d moveto\n", 0, 0);
        fprintf(fp, "    %.3f %.3f %.3f 30 90 arc\n", 0-k+d, 0-k/2, r);
        fprintf(fp, "    %.3f %.3f rlineto\n", -k-d, 0.0);
        fprintf(fp, "    %.3f %.3f rlineto\n", 0.0, -k);
        fprintf(fp, "    %.3f %.3f %.3f -90 -30 arc\n", 0-k+d, 0+k/2, r);
        fprintf(fp, "    closepath\n");

        if(atype==AH_SHIP) {
            fprintf(fp, "    fill\n");
        }
        else {
            fprintf(fp, "    stroke\n");
        }

        fprintf(fp, "  grestore\n");

        }

        break;

    case AH_REVNORMAL:
        xdir = xdir + 180;
        r = def_arrowsize*2;

        dx =  (double)(r*cos((xdir+180)*rf));
        dy =  (double)(r*sin((xdir+180)*rf));
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        r = def_arrowsize;

#if 1
  {
    double dx2, dy2;
        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));

        fprintf(fp, "newpath\n");
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);

        dx2 =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy2 =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%.3f %.3f rlineto\n", dx2-dx, dy2-dy);
        fprintf(fp, "closepath fill\n");
  }
#endif

        break;

    case AH_REVWIRE:
        xdir = xdir + 180;
        r = def_arrowsize*2;

        dx =  (double)(r*cos((xdir+180)*rf));
        dy =  (double)(r*sin((xdir+180)*rf));
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        r = def_arrowsize;

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        break;

    case AH_WIRE:
        r = def_arrowsize;

        fprintf(fp, "gsave\n");
        fprintf(fp, "  0 setlinejoin\n");

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "  %.3f %.3f moveto\n", x+dx, y+dy);
        fprintf(fp, "  %.3f %.3f lineto\n", x, y);

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "  %.3f %.3f lineto\n", x+dx, y+dy);
        fprintf(fp, "  stroke\n");

        fprintf(fp, "grestore\n");

        break;

    case AH_DOUBLE:
      {
        double dx1, dy1, dx2, dy2;

        r = def_arrowsize;

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
        dx1 = dx; dy1 = dy;
        fprintf(fp, "newpath\n");
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        dx2 = dx; dy2 = dy;
        fprintf(fp, "%.3f %.3f lineto\n", x+dx, y+dy);
        fprintf(fp, "closepath fill\n");
    
        epsdraw_arrowhead(fp, AH_NORMAL,
            xdir, lc, x+(dx2+dx1)/2, y+(dy2+dy1)/2);
      }

        break;

    case AH_ARROW3:
        r = def_arrowsize;

        fprintf(fp, "newpath\n");
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);

        dx =  (double)(r*2/3*cos((xdir+180)*rf));
        dy =  (double)(r*2/3*sin((xdir+180)*rf));
        fprintf(fp, "%.3f %.3f lineto\n", x+dx, y+dy);

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%.3f %.3f lineto\n", x+dx, y+dy);

        fprintf(fp, "closepath fill\n");
        break;

    case AH_ARROW4:
     {
        double q;
        double w;
        double mx, my;

        r = def_arrowsize;
        w = r/8;

        SLW_14(fp);
        fprintf(fp, "newpath\n");
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);

        dx =  (double)(r*2/3*cos((xdir+180+def_arrowangle)*rf));
        dy =  (double)(r*2/3*sin((xdir+180+def_arrowangle)*rf));
        mx = dx + w/2*cos((xdir+180+def_arrowangle-90)*rf);
        my = dy + w/2*sin((xdir+180+def_arrowangle-90)*rf);
        fprintf(fp, "%.3f %.3f %.3f %.3f %.3f arcn\n",
            x+mx, y+my, w/2,
            xdir+180+def_arrowangle+90,
            xdir+180+def_arrowangle-90
            );

        q = w/sin(def_arrowangle*rf);
        dx =  (double)(q*cos((xdir+180)*rf));
        dy =  (double)(q*sin((xdir+180)*rf));
        fprintf(fp, "%.3f %.3f lineto\n",  x+dx, y+dy);

        dx =  (double)(r*2/3*cos((xdir+180-def_arrowangle)*rf));
        dy =  (double)(r*2/3*sin((xdir+180-def_arrowangle)*rf));
        mx = dx + w/2*cos((xdir+180-def_arrowangle+90)*rf);
        my = dy + w/2*sin((xdir+180-def_arrowangle+90)*rf);
        fprintf(fp, "%.3f %.3f %.3f %.3f %.3f arcn\n",
            x+mx, y+my, w/2,
            xdir+180-def_arrowangle+90,
            xdir+180-def_arrowangle-90
            );

        fprintf(fp, "closepath fill\n");

    }

        break;

    case AH_ARROW5:
        r = def_arrowsize;

        dx =  (double)(r*cos((xdir+180+def_arrowangle*2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle*2)*rf));
        fprintf(fp, "newpath\n");
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);

        dx =  (double)(r*cos((xdir+180-def_arrowangle*2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle*2)*rf));
        fprintf(fp, "%.3f %.3f lineto\n", x+dx, y+dy);
        fprintf(fp, "closepath fill\n");

        break;

    case AH_ARROW6:
      {
        double dx1, dy1, dx2, dy2, dx3, dy3;
        r = def_arrowsize;

        dx1 =  (double)(r/2*cos((xdir+180+def_arrowangle)*rf));
        dy1 =  (double)(r/2*sin((xdir+180+def_arrowangle)*rf));
        dx2 =  (double)(r/2*cos((xdir+180-def_arrowangle)*rf));
        dy2 =  (double)(r/2*sin((xdir+180-def_arrowangle)*rf));
        dx3 =  (double)(r/2*cos((xdir+180+0)*rf));
        dy3 =  (double)(r/2*sin((xdir+180+0)*rf));

        fprintf(fp, "newpath\n");
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f lineto\n",   x+dx1, y+dy1);
        fprintf(fp, "%.3f %.3f lineto\n",   x+dx1+dx3, y+dy1+dy3);
        fprintf(fp, "%.3f %.3f lineto\n",   x+dx3, y+dy3);
        fprintf(fp, "%.3f %.3f lineto\n",   x+dx2+dx3, y+dy2+dy3);
#if 0
#endif
        fprintf(fp, "%.3f %.3f lineto\n",   x+dx2, y+dy2);
        fprintf(fp, "closepath fill\n");

      }
        break;


    case AH_WNORMAL:
        r = def_arrowsize;

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "newpath\n");
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%.3f %.3f lineto\n", x+dx, y+dy);
        fprintf(fp, "closepath stroke\n");

        break;

    case AH_NORMAL:
    default:
        r = def_arrowsize;

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "newpath\n");
        fprintf(fp, "%.3f %.3f moveto\n",   x,  y);
        fprintf(fp, "%.3f %.3f rlineto\n", dx, dy);

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%.3f %.3f lineto\n", x+dx, y+dy);
        fprintf(fp, "closepath fill\n");

        break;

    }
    fprintf(fp, "grestore %% for AH\n");

    return 0;
}

double
epsdraw_arrowhead(FILE *fp, int atype, int xdir, int lc, int x, int y)
{
    return _arrowheadD(fp, atype, (double)xdir, lc, (double)x, (double)y);
}

int
epsdraw_seglineSEP(FILE *fp, int ltype, int lt, int lc,
    int x1, int y1, int x2, int y2)
{
    int xdir;
    int ll;
    int j;
    int sepw;
    double px, py;
    double nx, ny;
    double ux, uy;
    double cy;

    fprintf(fp, "%% seglineSEP type %d %d,%d-%d,%d\n",
        ltype, x1, y1, x2, y2);

    changecolor(fp, lc);

    switch(ltype) {
    case LT_DOTTED:
    case LT_CHAINED:
    case LT_DOUBLECHAINED:
#if 0
        sepw = def_linedecothick/4;
#endif
        sepw = def_linedecothick/2;
        break;
    case LT_DASHED:
        sepw = def_linedecothick;
        break;
    }


    changethick(fp, lt);

    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

    if((xdir<125&&xdir>45)||(xdir<-45&&xdir>-135)) {
#if 0
        fprintf(fp, "%% Y-base\n");
#endif

        if(y1>y2) {
            int tmp;
#if 0
            fprintf(fp, "%% swap x1,x2 y1,y2\n");
#endif
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
#if 0
            fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
                ltype, x1, y1, x2, y2);
#endif
        }

        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        ll = SQRT_2DD_I2D(x1,y1,x2,y2);
        cy = ((double)y2-y1)/sepw;
        px = (x2-x1)/cy;
        py = (y2-y1)/cy;

        fprintf(fp, "%% ll %d cy %.2f px %.2f py %.2f\n",
            ll, cy, px, py);

        j  = 0;
        ux = x1;
        uy = y1;
        while(uy<y2) {
            ux = x1 + j*px;
            uy = y1 + j*py;
            if(uy>=y2) {
                break;
            }

            nx = ux + px;
            ny = uy + py;
            if(ny>=y2) {
                break;
            }

            {
                int act;
                act = solve_dashpart(ltype, j);
                if(act) {
                    fprintf(fp,
                        "%.2f %.2f moveto %.2f %.2f lineto stroke\n",
                        ux, uy, nx, ny);
                }
            }
            
            j++;
        }

    }
    else {
#if 0
        fprintf(fp, "%% X-base\n");
#endif
        
        if(x1>x2) {
#if 0
            fprintf(fp, "%% swap x1,x2 y1,y2\n");
#endif
            int tmp;
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
#if 0
            fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
                ltype, x1, y1, x2, y2);
#endif
        }

        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        ll = SQRT_2DD_I2D(x1,y1,x2,y2);
        cy = ((double)x2-x1)/sepw;
        px = (x2-x1)/cy;
        py = (y2-y1)/cy;

        fprintf(fp, "%% ll %d cy %.2f px %.2f py %.2f\n",
            ll, cy, px, py);

        j  = 0;
        ux = x1;
        uy = y1;
        while(ux<x2) {
            ux = x1 + j*px;
            uy = y1 + j*py;
            if(ux>=x2) {
                break;
            }

            nx = ux + px;
            ny = uy + py;
            if(nx>=x2) {
                break;
            }

            {
                int act;
                act = solve_dashpart(ltype, j);
                if(act) {
                    fprintf(fp,
                        "%.2f %.2f moveto %.2f %.2f lineto stroke\n",
                        ux, uy, nx, ny);
                }
            }
            
            j++;
        }
    }
out:
    return 0;
}

int
epsdraw_seglineTICK(FILE *fp, int ltype, int lt, int lc,
    int x1, int y1, int x2, int y2)
{
    int xdir;
    int ll;
    int j;
    int sepw;
    double px, py;
    double nx, ny;
    double ux, uy;
    double cy;
    int oxdir;
    double hx, hy;
    double tx, ty;

#if 0
    fprintf(fp, "%% seglineTICK type %d %d,%d-%d,%d\n",
        ltype, x1, y1, x2, y2);
#endif

    changethick(fp, lt);
    changecolor(fp, lc);

#if 0
    if(ltype==LT_DOTTED) {
        sepw = def_linedecothick/4;
    }
    else {
        sepw = def_linedecothick;
    }
#endif
    sepw = def_linedecothick;

    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
    oxdir = xdir;

    if((xdir<125&&xdir>45)||(xdir<-45&&xdir>-135)) {
#if 0
        fprintf(fp, "%% Y-base\n");
#endif

        if(y1>y2) {
            int tmp;
#if 0
            fprintf(fp, "%% swap x1,x2 y1,y2\n");
#endif
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
#if 0
            fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
                ltype, x1, y1, x2, y2);
#endif
        }

        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        ll = SQRT_2DD_I2D(x1,y1,x2,y2);
        cy = ((double)y2-y1)/sepw;
        px = (x2-x1)/cy;
        py = (y2-y1)/cy;

#if 0
        fprintf(fp, "%% ll %d cy %.2f px %.2f py %.2f\n",
            ll, cy, px, py);
#endif

        if(ltype==LT_TRIANGLE || ltype==LT_MOUNTAIN) {
            hx = sepw*cos((oxdir)*rf);
            hy = sepw*sin((oxdir)*rf);
            tx = sepw*cos((oxdir+90)*rf);
            ty = sepw*sin((oxdir+90)*rf);

#if 0
fprintf(fp, "%% hx,hy %.2f,%.2f tx,ty %.2f,%.2f\n", hx, hy, tx, ty);
#endif
        }

        j  = 0;
        ux = x1;
        uy = y1;
        while(uy<y2) {
            ux = x1 + j*px;
            uy = y1 + j*py;
            if(uy>=y2) {
                break;
            }

            nx = ux + px;
            ny = uy + py;
            if(ny>=y2) {
                break;
            }

            if(j%2==0) {
                if(ltype==LT_MOUNTAIN) {
                    fprintf(fp, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto stroke\n",
                        ux-tx, uy-ty, ux+hx, uy+hy, ux+tx, uy+ty);
                }
                else 
                if(ltype==LT_TRIANGLE) {
                    fprintf(fp, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto %.2f %.2f lineto closepath fill\n",
                        ux, uy, ux-tx, uy-ty, ux+hx, uy+hy, ux+tx, uy+ty);
                }
                else 
                if(ltype==LT_WCIRCLE) {
                    fprintf(fp, "newpath %.2f %.2f %.2f 0 360 arc stroke\n",
                        ux, uy, (double)sepw/2);
                }
                else {
                    fprintf(fp, "newpath %.2f %.2f %.2f 0 360 arc fill\n",
                        ux, uy, (double)sepw/2);
                }
            }
            j++;
        }

    }
    else {
#if 0
        fprintf(fp, "%% X-base\n");
#endif
        
        if(x1>x2) {
#if 0
            fprintf(fp, "%% swap x1,x2 y1,y2\n");
#endif
            int tmp;
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
#if 0
            fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
                ltype, x1, y1, x2, y2);
#endif
        }

        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        ll = SQRT_2DD_I2D(x1,y1,x2,y2);
        cy = ((double)x2-x1)/sepw;
        px = (x2-x1)/cy;
        py = (y2-y1)/cy;

        fprintf(fp, "%% ll %d cy %.2f px %.2f py %.2f\n",
            ll, cy, px, py);

        if(ltype==LT_TRIANGLE || ltype==LT_MOUNTAIN) {
            hx = sepw*cos((oxdir)*rf);
            hy = sepw*sin((oxdir)*rf);
            tx = sepw*cos((oxdir+90)*rf);
            ty = sepw*sin((oxdir+90)*rf);
        }

        j  = 0;
        ux = x1;
        uy = y1;
        while(ux<x2) {
            ux = x1 + j*px;
            uy = y1 + j*py;
            if(ux>=x2) {
                break;
            }

            nx = ux + px;
            ny = uy + py;
            if(nx>=x2) {
                break;
            }

            if(j%2==0) {
                if(ltype==LT_MOUNTAIN) {
                    fprintf(fp, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto stroke\n",
                        ux-tx, uy-ty, ux+hx, uy+hy, ux+tx, uy+ty);
                }
                else 
                if(ltype==LT_TRIANGLE) {
                    fprintf(fp, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto %.2f %.2f lineto closepath fill\n",
                        ux, uy, ux-tx, uy-ty, ux+hx, uy+hy, ux+tx, uy+ty);
                }
                else 
                if(ltype==LT_WCIRCLE) {
                    fprintf(fp, "newpath %.2f %.2f %.2f 0 360 arc stroke\n",
                        ux, uy, (double)sepw/2);
                }
                else {
                    fprintf(fp, "newpath %.2f %.2f %.2f 0 360 arc fill\n",
                        ux, uy, (double)sepw/2);
                }
            }

            j++;
        }
    }
out:
    return 0;
}

int
epsdraw_seglineTICK2(FILE *fp, int ltype, int lt, int lc,
    int x1, int y1, int x2, int y2)
{
    int xdir;
    int ll;
    int j;
    int c;
    int sepw;
    double px, py;
    double nx, ny;
    double ux, uy;
    double cy;
    double r;

    double d1x, d1y, d2x, d2y;

    fprintf(fp, "%% seglineTICK2 type %d %d,%d-%d,%d\n",
        ltype, x1, y1, x2, y2);

    changethick(fp, lt);
    changecolor(fp, lc);


#if 0
    if(ltype==LT_DOTTED) {
        sepw = def_linedecothick/4;
    }
    else {
        sepw = def_linedecothick;
    }
#endif
        sepw = def_linedecothick;

    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

    if((xdir<125&&xdir>45)||(xdir<-45&&xdir>-135)) {
#if 0
        fprintf(fp, "%% Y-base\n");
#endif

        if(y1>y2) {
            int tmp;
#if 0
            fprintf(fp, "%% swap x1,x2 y1,y2\n");
#endif
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
#if 0
            fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
                ltype, x1, y1, x2, y2);
#endif
        }

        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        ll = SQRT_2DD_I2D(x1,y1,x2,y2);
        cy = ((double)y2-y1)/sepw;
        px = (x2-x1)/cy;
        py = (y2-y1)/cy;

        r  = SQRT_2D(px, py);

        d1x = sepw*cos((xdir+45)*rf);
        d1y = sepw*sin((xdir+45)*rf);
        d2x = sepw*cos((xdir-45)*rf);
        d2y = sepw*sin((xdir-45)*rf);

        fprintf(fp, "%% ll %d cy %.2f px %.2f py %.2f\n",
            ll, cy, px, py);

            fprintf(fp, "newpath\n");
    if(ltype==LT_ZIGZAG) {
            fprintf(fp, "%d %d moveto\n", x1, y1);
    }


        c  = 0;
        j  = 0;
        ux = x1;
        uy = y1;
        while(uy<y2) {
            ux = x1 + j*px;
            uy = y1 + j*py;
            if(uy>=y2) {
                break;
            }

            nx = ux + px;
            ny = uy + py;
            if(ny>=y2) {
                break;
            }

    if(ltype==LT_WAVED) {
            if(j%4==1) {
                fprintf(fp, "%.2f %.2f %.2f %d %d arcn\n",
                    ux, uy, r, xdir+180, xdir);
                c++;
            }
            if(j%4==3) {
                fprintf(fp, "%.2f %.2f %.2f %d %d arc\n",
                    ux, uy, r, xdir+180, xdir);
                c++;
            }
    }

    if(ltype==LT_ZIGZAG) {
            if(j%2==0) {
                fprintf(fp, "%.2f %.2f lineto\n",
                    ux+d1x, uy+d1y);
                c++;
            }
            if(j%2==1) {
                fprintf(fp, "%.2f %.2f lineto\n",
                    ux+d2x, uy+d2y);
                c++;
            }
    }






            j++;
        }


        if(c) {
            fprintf(fp, "%d %d lineto\n", x2, y2);
            fprintf(fp, "stroke\n");
        }


    }
    else {
#if 0
        fprintf(fp, "%% X-base\n");
#endif
        
        if(x1>x2) {
#if 0
            fprintf(fp, "%% swap x1,x2 y1,y2\n");
#endif
            int tmp;
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
#if 0
            fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
                ltype, x1, y1, x2, y2);
#endif
        }

        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        ll = SQRT_2DD_I2D(x1,y1,x2,y2);
        cy = ((double)x2-x1)/sepw;
        px = (x2-x1)/cy;
        py = (y2-y1)/cy;

        r  = SQRT_2D(px, py);

        d1x = sepw*cos((xdir+45)*rf);
        d1y = sepw*sin((xdir+45)*rf);
        d2x = sepw*cos((xdir-45)*rf);
        d2y = sepw*sin((xdir-45)*rf);

        fprintf(fp, "%% ll %d cy %.2f px %.2f py %.2f\n",
            ll, cy, px, py);

            fprintf(fp, "newpath\n");
    if(ltype==LT_ZIGZAG) {
            fprintf(fp, "%d %d moveto\n", x1, y1);
    }

        c  = 0;
        j  = 0;
        ux = x1;
        uy = y1;
        while(ux<x2) {
            ux = x1 + j*px;
            uy = y1 + j*py;
            if(ux>=x2) {
                break;
            }

            nx = ux + px;
            ny = uy + py;
            if(nx>=x2) {
                break;
            }

    if(ltype==LT_WAVED) {
            if(j%4==1) {
                fprintf(fp, "%.2f %.2f %.2f %d %d arcn\n",
                    ux, uy, r, xdir+180, xdir);
                c++;
            }
            if(j%4==3) {
                fprintf(fp, "%.2f %.2f %.2f %d %d arc\n",
                    ux, uy, r, xdir+180, xdir);
                c++;
            }
    }

    if(ltype==LT_ZIGZAG) {
            if(j%2==0) {
                fprintf(fp, "%.2f %.2f lineto\n",
                    ux+d1x, uy+d1y);
                c++;
            }
            if(j%2==1) {
                fprintf(fp, "%.2f %.2f lineto\n",
                    ux+d2x, uy+d2y);
                c++;
            }
    }

            j++;
        }
        if(c) {
            fprintf(fp, "%d %d lineto\n", x2, y2);
            fprintf(fp, "stroke\n");
        }
    }
out:
    return 0;
}

int
epsdraw_seglineM(FILE *fp, int ltype, int lt, int lc,
    int x1, int y1, int x2, int y2)
{
    int xdir;
    int dx, dy;
    int s1x, s1y;
    int s2x, s2y;
    int e1x, e1y;
    int e2x, e2y;
    int ll;
    double opx, opy;
    double px, py;
    double nx, ny;
#if 0
    double ux, uy;
#endif
    int j;
    double cy;
    int ux, uy;
    int vx, vy;
    double a, b;

    /*
     * x1   ux vx   x2
     *  +----| |-----+
     *
     */

    fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
        ltype, x1, y1, x2, y2);

    changethick(fp, lt);
    changecolor(fp, lc);

    if(x1>x2) {
fprintf(fp, "%% swap x1,x2 y1,y2\n");
        int tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
        ltype, x1, y1, x2, y2);
    }
    else {
fprintf(fp, "%% no swap x1,x2 y1,y2\n");
    }


    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);


    ux = (x1+x2)/2-def_linedecothick/2;
    vx = (x1+x2)/2+def_linedecothick/2;
    a = (y2-y1)/(x2-x1);
    b = y1 - a * x1;
    uy = a*ux+b;
    vy = a*vx+b;

    fprintf(fp, "%% a %.3f b %.3f\n", a, b);

    dx = def_linedecothick*cos((xdir+60)*rf);
    dy = def_linedecothick*sin((xdir+60)*rf);

    Echo("%s: dir %d linedecothick %d dx, dy %d,%d\n",
        __func__, xdir, def_linedecothick, dx, dy);
#if 0
    Echo("%s: sx, sy %d,%d\n", __func__, x1, y1);
    Echo("%s: ex, ey %d,%d\n", __func__, x2, y2);
#endif
    Echo("%s: s->e   %5d,%5d - %5d,%5d\n",
        __func__, x1, y1, x2, y2);


    fprintf(fp, "%% ux,uy %d,%d  vx,vy %d,%d\n", ux, uy, vx, vy);

    fprintf(fp, "%d %d moveto\n", x1, y1);
    fprintf(fp, "%d %d lineto\n", ux, uy);
    fprintf(fp, "%d %d moveto\n", vx, vy);
    fprintf(fp, "%d %d lineto\n", x2, y2);

    fprintf(fp, "%d %d moveto\n", ux, uy);
    fprintf(fp, "%d %d rlineto\n", dx, dy);
    fprintf(fp, "%d %d moveto\n", ux, uy);
    fprintf(fp, "%d %d rlineto\n", -dx, -dy);

    fprintf(fp, "%d %d moveto\n", vx, vy);
    fprintf(fp, "%d %d rlineto\n", dx, dy);
    fprintf(fp, "%d %d moveto\n", vx, vy);
    fprintf(fp, "%d %d rlineto\n", -dx, -dy);

    fprintf(fp, "stroke\n");

out:

    return 0;
}

int
epsdraw_seglineW(FILE *fp, int ltype, int lt, int lc,
    int x1, int y1, int x2, int y2)
{
    int xdir;
    int dx, dy;
    int s1x, s1y;
    int s2x, s2y;
    int e1x, e1y;
    int e2x, e2y;
    int ll;
    double opx, opy;
    double px, py;
    double nx, ny;
    double ux, uy;
    int j;
    double cy;

    fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
        ltype, x1, y1, x2, y2);

    changethick(fp, lt);
    changecolor(fp, lc);

    if(x1>x2) {
fprintf(fp, "%% swap x1,x2 y1,y2\n");
        int tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
        ltype, x1, y1, x2, y2);
    }
    else {
fprintf(fp, "%% no swap x1,x2 y1,y2\n");
    }


    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

    opx = def_linedecothick*cos((xdir+0)*rf);
    opy = def_linedecothick*sin((xdir+0)*rf);

    ll = SQRT_2DD_I2D(x1,y1,x2,y2);
    cy = ((double)x2-x1)/def_linedecothick;
    px = (x2-x1)/cy;
    py = (y2-y1)/cy;
#if 0
#endif

    fprintf(fp, "%% ll %d opx %.2f opy %.2f cy %.2f px %.2f py %.2f\n",
        ll, opx, opy, cy, px, py);

    dx = def_linedecothick*cos((xdir+90)*rf);
    dy = def_linedecothick*sin((xdir+90)*rf);

    Echo("%s: dir %d linedecothick %d dx, dy %d,%d\n",
        __func__, xdir, def_linedecothick, dx, dy);
#if 0
    Echo("%s: sx, sy %d,%d\n", __func__, x1, y1);
    Echo("%s: ex, ey %d,%d\n", __func__, x2, y2);
#endif
    Echo("%s: s->e   %5d,%5d - %5d,%5d\n",
        __func__, x1, y1, x2, y2);

    s1x = x1 + dx;
    s1y = y1 + dy;
    e1x = x2 + dx;
    e1y = y2 + dy;

    Echo("%s: s1->e1 %5d,%5d - %5d,%5d\n",
        __func__, s1x, s1y, e1x, e1y);

    s2x = x1 - dx;
    s2y = y1 - dy;
    e2x = x2 - dx;
    e2y = y2 - dy;
    Echo("%s: s2->e2 %5d,%5d - %5d,%5d\n",
        __func__, s2x, s2y, e2x, e2y);


    fprintf(fp, "%% s1->e1\n");
    fprintf(fp, "%d %d moveto\n", s1x, s1y);
    fprintf(fp, "%d %d lineto\n", e1x, e1y);
    fprintf(fp, "stroke\n");
    fprintf(fp, "%% s2->e2\n");
    fprintf(fp, "%d %d moveto\n", s2x, s2y);
    fprintf(fp, "%d %d lineto\n", e2x, e2y);
    fprintf(fp, "stroke\n");


out:

    return 0;
}

int
epsdraw_segline(FILE *fp, int ltype, int lt, int lc,
    int x1, int y1, int x2, int y2)
{
    int rv;

    rv = 0;
    switch(ltype) {
    case LT_DOTTED:
    case LT_DASHED:
    case LT_CHAINED:
    case LT_DOUBLECHAINED:
        rv = epsdraw_seglineSEP(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_WAVED:
    case LT_ZIGZAG:
        rv = epsdraw_seglineTICK2(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_CIRCLE:
    case LT_WCIRCLE:
    case LT_TRIANGLE:
    case LT_MOUNTAIN:
        rv = epsdraw_seglineTICK(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_DOUBLED:
        rv = epsdraw_seglineW(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
#if 0
    case LT_CUTTED:
        rv = epsdraw_seglineM(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_ARROWCENTERED:
#endif
    case LT_SOLID:
    default:
        changethick(fp, lt);
        changecolor(fp, lc);
        fprintf(fp, "  %% solid\n");
        fprintf(fp, "  %d %d moveto\n", x1, y1);
        fprintf(fp, "  %d %d lineto\n", x2, y2);
        fprintf(fp, "  stroke\n");
#if 0
        if(ltype==LT_ARROWCENTERED) {
            double xdir, r;
            int x3, y3;
            xdir = (int)(atan2(y2-y1, x2-x1)/rf);
            r    = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
            r    = r/2 + def_arrowsize;
#if 0
            x3   = (int)((double)x1 + r*cos(xdir*rf));
            y3   = (int)((double)y1 + r*sin(xdir*rf));
#endif
            x3   = (x1+x2)/2;
            y3   = (y1+y2)/2;
            rv = epsdraw_arrowhead(fp, AH_NORMAL, xdir, lc, x3, y3);
        }
#endif
        break;
    }
    return rv;
}

int
epsdraw_warrowhead(FILE *fp, int atype, int xdir, int lc, int x, int y)
{
    int  dx, dy;
    int  bx, by;

    fprintf(fp, "%% warrowhead %d\n", atype);

    bx = def_warrowsize*cos((xdir+180)*rf);
    by = def_warrowsize*sin((xdir+180)*rf);
    dx =  (int)(def_wlinethick*cos((xdir+90)*rf));
    dy =  (int)(def_wlinethick*sin((xdir+90)*rf));

    fprintf(fp, " %d %d moveto\n", x+bx+dx, y+by+dy);
    fprintf(fp, " %d %d lineto\n", x+bx+dx+dx, y+by+dy+dy);
    fprintf(fp, " %d %d lineto\n", x, y);
    fprintf(fp, " %d %d lineto\n", x+bx-dx-dx, y+by-dy-dy);
    fprintf(fp, " %d %d lineto\n", x+bx-dx, y+by-dy);
    fprintf(fp, " stroke\n");

    return 0;
}

int
epsdraw_segwline_orig(FILE *fp, int wlt, int ltype, int lt, int lc,
    int x1, int y1, int x2, int y2)
{
    int xdir;
    int dx, dy;
    int s1x, s1y;
    int s2x, s2y;
    int e1x, e1y;
    int e2x, e2y;
    int ll;
    double opx, opy;
    double px, py;
    double nx, ny;
    double ux, uy;
    int j;
    double cy;

    fprintf(fp, "%% segwline_orig wlt %d type %d lt %d lc %d %d,%d - %d,%d\n",
        wlt, ltype, lt, lc, x1, y1, x2, y2);

    changethick(fp, lt);
    changecolor(fp, lc);

    if(x1>x2) {
fprintf(fp, "%% swap x1,x2 y1,y2\n");
        int tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
        ltype, x1, y1, x2, y2);
    }
    else {
fprintf(fp, "%% no swap x1,x2 y1,y2\n");
    }


    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

    opx = wlt*cos((xdir+0)*rf);
    opy = wlt*sin((xdir+0)*rf);

    ll = SQRT_2DD_I2D(x1,y1,x2,y2);
    cy = ((double)x2-x1)/def_linedecothick;
    px = (x2-x1)/cy;
    py = (y2-y1)/cy;
#if 0
#endif

    fprintf(fp, "%% ll %d opx %.2f opy %.2f cy %.2f px %.2f py %.2f\n",
        ll, opx, opy, cy, px, py);

    dx = wlt*cos((xdir+90)*rf);
    dy = wlt*sin((xdir+90)*rf);

    Echo("%s: dir %d linedecothick %d dx, dy %d,%d\n",
        __func__, xdir, def_linedecothick, dx, dy);
#if 0
    Echo("%s: sx, sy %d,%d\n", __func__, x1, y1);
    Echo("%s: ex, ey %d,%d\n", __func__, x2, y2);
#endif
    Echo("%s: s->e   %5d,%5d - %5d,%5d\n",
        __func__, x1, y1, x2, y2);

    s1x = x1 + dx;
    s1y = y1 + dy;
    e1x = x2 + dx;
    e1y = y2 + dy;

    Echo("%s: s1->e1 %5d,%5d - %5d,%5d\n",
        __func__, s1x, s1y, e1x, e1y);

    s2x = x1 - dx;
    s2y = y1 - dy;
    e2x = x2 - dx;
    e2y = y2 - dy;
    Echo("%s: s2->e2 %5d,%5d - %5d,%5d\n",
        __func__, s2x, s2y, e2x, e2y);


    fprintf(fp, "%% s1->e1\n");
    fprintf(fp, "%d %d moveto\n", s1x, s1y);
    fprintf(fp, "%d %d lineto\n", e1x, e1y);
    fprintf(fp, "stroke\n");
    fprintf(fp, "%% s2->e2\n");
    fprintf(fp, "%d %d moveto\n", s2x, s2y);
    fprintf(fp, "%d %d lineto\n", e2x, e2y);
    fprintf(fp, "stroke\n");

out:

    return 0;
}


int
epsdraw_segwline(FILE *fp,
    int wlt,
    int ltype, int lt, int lc, int x1, int y1, int x2, int y2)
{
    int r;

    r = epsdraw_segwline_orig(fp, wlt, ltype, lt, lc, x1, y1, x2, y2);

    return r;
}


int
epsdraw_Xseglinearrow(FILE *fp,
    int xox, int xoy, 
    int x1, int y1, int x2, int y2,
    int xltype, int xlt, int xlc, int xahpart, int xahfore, int xahcent, int xahback)
{
    ob* pf;
    ob* pt;
    int r;
    int dx, dy;
    int xdir;

    int x1i, y1i;
    int x2i, y2i;
    int x3, y3;

    /*
     *   x1,y1 x1i,y1i x3,y3 x2i,y2i  x2,y2
     *     <---+---------+------+---->
     */

    Echo("%s: enter\n", __func__);

    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

#if 0
    fprintf(fp, "%% %s\n", __func__);
    fprintf(fp, "%%   xdir               %4d\n", xdir);
    fprintf(fp, "%%   line-type          %4d\n", xltype);
    fprintf(fp, "%%   arrowhead-part     %4d fore-type %4d back-type %4d\n",
            xahpart, xahfore, xahback);
    fflush(fp);
#endif

    if(draft_mode) {
        fprintf(fp, "    gsave %% draft 1\n");
        changedraft(fp);
        fprintf(fp, "      %d %d moveto\n", x1, y1);
        fprintf(fp, "      %d %d lineto\n", x2, y2);
        fprintf(fp, "      stroke\n");
        fprintf(fp, "    grestore\n");
    }


    if(xahpart & AR_BACK) {
        if( (xahback==AH_REVNORMAL)||
            (xahback==AH_REVWIRE)) {
            goto no_backhead;
        }
#if 0
        dx = def_arrowsize*2*cos((xdir)*rf);
        dy = def_arrowsize*2*sin((xdir)*rf);
#endif
        dx = def_arrowsize*cos((xdir)*rf);
        dy = def_arrowsize*sin((xdir)*rf);
        x1i = x1 + dx;
        y1i = y1 + dy;
    }
    else {
no_backhead:
        x1i = x1;
        y1i = y1;
    }

    if((xahpart & AR_FORE)) {
        if((xahfore==AH_REVNORMAL)||
            (xahfore==AH_REVWIRE)) {
            goto no_forehead;
        }
#if 0
        dx = def_arrowsize*2*cos((xdir+180)*rf);
        dy = def_arrowsize*2*sin((xdir+180)*rf);
#endif
        dx = def_arrowsize*cos((xdir+180)*rf);
        dy = def_arrowsize*sin((xdir+180)*rf);
        x2i = x2 + dx;
        y2i = y2 + dy;
    }
    else {
no_forehead:
        x2i = x2;
        y2i = y2;
    }


P;
    /*** MAIN LINE */
    epsdraw_segline(fp, xltype, xlt, xlc, x1i, y1i, x2i, y2i);

    /*** BACK ARROW HEAD */
    if(x1i!=x1 || y1i!=y1) {
        fprintf(fp, "%% back arrow head\n");
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            x1, y1, x1i, y1i);
    }
    if(xahpart & AR_BACK) {
        xdir = (int)(atan2((y1-y2),(x1-x2))/rf);
        epsdraw_arrowhead(fp, xahback, xdir, xlc, x1, y1);
    }

#if 1
    if(xahpart & AR_CENT) {
        x3 = (x1+x2)/2;
        y3 = (y1+y2)/2;
        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        epsdraw_arrowhead(fp, xahcent, xdir, xlc, x3, y3);
    }
#endif

    /*** FORE ARROW HEAD */
    if(x2i!=x2 || y2i!=y2) {
        fprintf(fp, "%% fore arrow head\n");
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            x2i, y2i, x2, y2);
P;
    }
    if(xahpart & AR_FORE) {
        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        epsdraw_arrowhead(fp, xahfore, xdir, xlc, x2, y2);
    }


out:
    return 0;
}



int
epsdraw_Xsegwlinearrow(FILE *fp,
    int xox, int xoy, 
    int x1, int y1, int x2, int y2,
    int xwlt,
    int xltype, int xlt, int xlc, int xahpart, int xahfore, int xahcent, int xahback)
{
    ob* pf;
    ob* pt;
    int r;
    int dx, dy;
    int xdir;

    int x1i, y1i;
    int x2i, y2i;

    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

    /*
     *   x1,y1 x1i,y1i  x2i,y2i  x2,y2
     *     <---+--------------+---->
     */

    if(draft_mode) {
        fprintf(fp, "gsave\n");
        changedraft(fp);
        fprintf(fp, "  %d %d moveto\n", x1, y1);
        fprintf(fp, "  %d %d lineto\n", x2, y2);
        fprintf(fp, "  stroke\n");
        fprintf(fp, "grestore\n");
    }

    fprintf(fp, "%% %s\n", __func__);
    fprintf(fp, "%%   line-type          %d\n", xltype);
    fprintf(fp, "%%   arrowhead-part     %d\n", xahpart);
    fprintf(fp, "%%   arrowforehead-type %d\n", xahfore);
    fprintf(fp, "%%   arrowbackhead-type %d\n", xahback);
    fprintf(fp, "%%   ---\n");
    fprintf(fp, "%%   xdir               %d\n", xdir);
    fflush(fp);


    if(xahpart & AR_BACK) {
        if( (xahback==AH_REVNORMAL)||
            (xahback==AH_REVWIRE)) {
            goto no_backhead;
        }
        dx = def_arrowsize*2*cos((xdir)*rf);
        dy = def_arrowsize*2*sin((xdir)*rf);
        x1i = x1 + dx;
        y1i = y1 + dy;
    }
    else {
no_backhead:
        x1i = x1;
        y1i = y1;
    }

    if((xahpart & AR_FORE)) {
        if((xahfore==AH_REVNORMAL)||
            (xahfore==AH_REVWIRE)) {
            goto no_forehead;
        }
        dx = def_arrowsize*2*cos((xdir+180)*rf);
        dy = def_arrowsize*2*sin((xdir+180)*rf);
        x2i = x2 + dx;
        y2i = y2 + dy;
    }
    else {
no_forehead:
        x2i = x2;
        y2i = y2;
    }



    changethick(fp, xlt);
    changecolor(fp, xlc);

    /*** BACK ARROW HEAD */
    if(x1i!=x1 || y1i!=y1) {
#if 0
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            x1, y1, x1i, y1i);
#endif
    }
    if(xahpart & AR_BACK) {
        xdir = (int)(atan2((y1-y2),(x1-x2))/rf);
        epsdraw_warrowhead(fp, xahback, xdir, xlc, x1, y1);
    }

    /*** MAIN LINE */
    epsdraw_segwline(fp, xwlt, xltype, xlt, xlc, x1i, y1i, x2i, y2i);

    /*** FORE ARROW HEAD */
    if(x2i!=x2 || y2i!=y2) {
#if 0
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            x2i, y2i, x2, y2);
#endif
    }
    if(xahpart & AR_FORE) {
        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        epsdraw_warrowhead(fp, xahfore, xdir, xlc, x2, y2);
    }


out:
    return 0;
}


int
epsdraw_seglinearrow(FILE *fp,
    int ydir, int xox, int xoy, 
    int x1, int y1, int x2, int y2,
    ob *xu, ns *xns)
{
    int ik;
    int tx, ty;

#if 0
    if(xu->cafrom) {
        ik = ns_find_objposG(xns, xu->cafrom, &tx, &ty);
        if(ik) {
        }
        x1 = tx;
        y1 = ty;
    }
    if(xu->cato) {
        ik = ns_find_objposG(xns, xu->cato, &tx, &ty);
        if(ik) {
        }
        x2 = tx;
        y2 = ty;
    }
#endif

P;
    ik = epsdraw_Xseglinearrow(fp,
        xox, xoy,
        x1, y1, x2, y2,
        xu->cob.outlinetype,
        xu->cob.outlinethick,
        xu->cob.outlinecolor,
        xu->cob.arrowheadpart,
        xu->cob.arrowforeheadtype,
        xu->cob.arrowcentheadtype,
        xu->cob.arrowbackheadtype);

out:
    return ik;
}



int
epsdraw_segwlinearrow(FILE *fp,
    int ydir, int xox, int xoy, 
    int x1, int y1, int x2, int y2,
    ob *xu, ns *xns)
{
    int ik;

#if 0
    int dx, dy;
    int xdir;
    ob* pf;
    ob* pt;
    int r;
    int x1i, y1i;
    int x2i, y2i;
#endif

#if 0

    /*
     *   x1,y1 x1i,y1i  x2i,y2i  x2,y2
     *     <---+--------------+---->
     */

    pf = NULL;
    if(xu->cafrom) {
        pf = ns_find_obj(xns, xu->cafrom);
        Echo("%s: pf %p\n", __func__, pf);
        if(!pf) {
            goto to_phase;
        }
        if(pf->cfixed) {
            Echo("  from %d,%d\n", pf->gx, pf->gy);
            x1 = pf->gx;
            y1 = pf->gy;
        }
    }

to_phase:

    pt = NULL;
    if(xu->cato) {
        pt = ns_find_obj(xns, xu->cato);
        Echo("%s: pt %p\n", __func__, pt);
        if(!pt) {
            goto apply;
        }
        if(pt->cfixed) {
            Echo("  to %d,%d\n", pt->gx, pt->gy);
            x2 = pt->gx;
            y2 = pt->gy;
        }
    }

#endif

apply:

    ik = epsdraw_Xsegwlinearrow(fp,
        xox, xoy,
        x1, y1, x2, y2,
        xu->cob.wlinethick,
        xu->cob.outlinetype,
        xu->cob.outlinethick,
        xu->cob.outlinecolor,
        xu->cob.arrowheadpart,
        xu->cob.arrowforeheadtype,
        xu->cob.arrowcentheadtype,
        xu->cob.arrowbackheadtype);

out:
    return ik;
}

int
epsdraw_segblinearrow(FILE *fp,
    int ydir, int xox, int xoy, 
    int x1, int y1, int x2, int y2,
    ob *xu, ns *xns)
{
    int ik;
    int dx, dy;
    int bx, by;
    int l;
    int b;
    double a;
    int s1x, s1y, e1x, e1y;
    int s2x, s2y, e2x, e2y;

    ik = 0;

    l = SQRT_2DD_I2D(x1,y1,x2,y2);
    a = atan2(y2-y1, x2-x1)/rf;
    b = (int)(def_barrowgap/2*tan(a*rf));

    bx = (int)(b*cos(a*rf));
    by = (int)(b*sin(a*rf));
    dx = (int)(def_barrowgap/2*cos((a+90)*rf));
    dy = (int)(def_barrowgap/2*sin((a+90)*rf));

    Echo("l %d b %d bx %d by %d\n", l, b, bx, by);
    Echo("dx %d dy %d\n", dx, dy);

P;
PP;
    fprintf(fp, "  gsave %% for blinearrow\n");

    s1x = x1 + dx;
    s1y = y1 + dy;
    e1x = x2 + dx;
    e1y = y2 + dy;

    s2x = x1 - dx;
    s2y = y1 - dy;
    e2x = x2 - dx;
    e2y = y2 - dy;

    epsdraw_Xseglinearrow(fp, 0, 0, s1x, s1y, e1x, e1y,
        LT_SOLID, xu->cob.outlinethick, xu->cob.outlinecolor,
        AR_FORE, AH_NORMAL, AH_NONE, AH_NONE);
    epsdraw_Xseglinearrow(fp, 0, 0, e2x, e2y, s2x, s2y,
        LT_SOLID, xu->cob.outlinethick, xu->cob.outlinecolor,
        AR_FORE, AH_NORMAL, AH_NONE, AH_NONE);

    fprintf(fp, "  grestore %% for blinearrow\n");

out:
    return ik;
}

static
int
drawwave(FILE *fp, int x1, int y1, int x2, int y2, int ph)
{
    double na;
    double ra;
    double mx, my;
    double d, r;
    double px, py;

    mx = ((double)x2+x1)/2;
    my = ((double)y2+y1)/2;

    d = SQRT_2DD_I2D(x1,y1,x2,y2);
    r = d/2;

    na = atan2((y2-y1), (x2-x1))/rf;
    if(ph==0) {
        ra = na + 90;
    }
    if(ph==1) {
        ra = na - 90;
    }

    px = mx + r*cos(ra*rf);
    py = my + r*sin(ra*rf);

#if 0
    fprintf(fp, "gsave %d %d moveto %d %d lineto stroke grestore %% drawwave\n",
        x1, y1, x2, y2);

    fprintf(fp, "gsave %.3f %.3f moveto %.3f %.3f lineto stroke grestore %% drawwave\n",
        mx, my, px, py);
#endif

    if(ph==0) {
        fprintf(fp, "%.3f %.3f %.3f %.3f %.3f arcn %% drawwave\n",
            mx, my, r, na - 180, na);
    }
    if(ph==1) {
        fprintf(fp, "%.3f %.3f %.3f %.3f %.3f arc  %% drawwave\n",
            mx, my, r, na - 180, na);
    }

    return 0;
}


#define ADIR_ARC    (1)
#define ADIR_ARCN   (2)


static int __z = 0;





int
epsdraw_ping(FILE *fp,
    int ydir, int xox, int xoy, 
    ob *xu, ns *xns)
{
    int x1, y1, x2, y2;
    int r;
    int r0;
    double xdir;
    double ag;
    double p;

    x1 = xox+xu->csx;
    y1 = xoy+xu->csy;
    x2 = xox+xu->cex;
    y2 = xoy+xu->cey;

    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
    ag = (int)(atan2(xu->ht/2, xu->wd)/rf);

    r0 = (int)SQRT_2DD_I2D(x1,y1,x2,y2);

    fprintf(fp, "gsave %% ping\n");
    changethick(fp, xu->vob.outlinethick);

    for(p=0.1;p<1.0;p+=0.2) {
        r = r0*p;

        fprintf(fp, "  newpath\n");
        fprintf(fp, "  %d %d %d %.3f %.3f arc\n", x1, y1, r, xdir-ag, xdir+ag);
        fprintf(fp, "  stroke\n");
    }

    fprintf(fp, "grestore %% ping\n");

    return 0;
}

int
epsdraw_pingpong(FILE *fp,
    int ydir, int xox, int xoy, 
    ob *xu, ns *xns)
{
    int x1, y1, x2, y2;
    int r;
    int r0;
    double xdir;
    double ag;
    double p;
    int dx, dy;

    x1 = xox+xu->csx;
    y1 = xoy+xu->csy;
    x2 = xox+xu->cex;
    y2 = xoy+xu->cey;

    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
    ag = (int)(atan2(((xu->ht*7/8)*9/10)/2, xu->wd)/rf);

    r0 = (int)SQRT_2DD_I2D(x1,y1,x2,y2);

    dx = (int)(xu->ht/8*cos((xdir+90)*rf));
    dy = (int)(xu->ht/8*sin((xdir+90)*rf));



    fprintf(fp, "gsave %% ping\n");

#if 0
    fprintf(fp, "  gsave\n");
    fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                        x1+dx,y1+dy, x2+dx, y2+dy);
    fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                        x1-dx,y1-dy, x2+dx, y2-dy);
    fprintf(fp, "  grestore\n");
#endif

    changethick(fp, xu->vob.outlinethick);

    for(p=0.1;p<1.0;p+=0.2) 
    {
        r = r0*p;

        fprintf(fp, "  newpath\n");
        fprintf(fp, "  %d %d %d %.3f %.3f arc\n", x1+dx, y1+dy, r, xdir-ag, xdir+ag);
        fprintf(fp, "  stroke\n");

        r = r0*(p-0.1);

        fprintf(fp, "  newpath\n");
        fprintf(fp, "  %d %d %d %.3f %.3f arc\n", x2-dx, y2-dy, r, xdir+180-ag, xdir+180+ag);
        fprintf(fp, "  stroke\n");
    }

    fprintf(fp, "grestore %% ping\n");

    return 0;
}


int
epsdraw_plinearrow(FILE *fp,
    int ydir, int xox, int xoy, 
    ob *xu, ns *xns)
{
    int x1, y1, x2, y2;
    int x3, y3, x4, y4;
    int dx, dy;
    int r;
    double xdir;

    x1 = xox+xu->csx;
    y1 = xoy+xu->csy;
    x2 = xox+xu->cex;
    y2 = xoy+xu->cey;

    /* shorter one is used */
    if(xu->cht < xu->cwd) { r = xu->cht; } else { r = xu->cwd; }
#if 0
    r = r/2;
#endif

    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

    dx = (int)(r*cos((xdir+90)*rf));
    dy = (int)(r*sin((xdir+90)*rf));

P;
#if 0
    fprintf(fp, "%% x1,y1 %d,%d x2,y2 %d,%d\n", x1, y1, x2, y2);
    fprintf(fp, "%% xdir %.3f dx,dy %d,%d\n", xdir, dx, dy);
#endif

    x3 = xox+xu->cx+dx;
    y3 = xoy+xu->cy+dy;
    x4 = xox+xu->cx-dx;
    y4 = xoy+xu->cy-dy;

#if 0
    if(x3>x4) {
        int tmp;
        tmp = x3; x3  = x4; x4  = tmp;
        tmp = y3; y3  = y4; y4  = tmp;
    }
#endif

    return epsdraw_seglinearrow(fp, ydir, xoy, xoy,
        x3, y3, x4, y4, xu, xns);
}


int
epsdraw_plinearrowR(FILE *fp,
    int ydir, int xox, int xoy, int r,
    ob *xu, ns *xns)
{
    int x1, y1, x2, y2;
    int x3, y3, x4, y4;
    int dx, dy;
    double xdir;

    x1 = xox+xu->csx;
    y1 = xoy+xu->csy;
    x2 = xox+xu->cex;
    y2 = xoy+xu->cey;

    /* shorter one is used */

    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

    dx = (int)(r*cos((xdir+90)*rf));
    dy = (int)(r*sin((xdir+90)*rf));

#if 0
    fprintf(fp, "%% x1,y1 %d,%d x2,y2 %d,%d\n", x1, y1, x2, y2);
    fprintf(fp, "%% xdir %.3f dx,dy %d,%d\n", xdir, dx, dy);
#endif

    x3 = xox+xu->cx+dx;
    y3 = xoy+xu->cy+dy;
    x4 = xox+xu->cx-dx;
    y4 = xoy+xu->cy-dy;

#if 0
    if(x3>x4) {
        int tmp;
        tmp = x3; x3  = x4; x4  = tmp;
        tmp = y3; y3  = y4; y4  = tmp;
    }
#endif

    return epsdraw_seglinearrow(fp, ydir, xoy, xoy,
        x3, y3, x4, y4, xu, xns);
}




#define markfdot(c,x,y) \
    fprintf(fp, "    %% markfdot %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      newpath %d %d %d 0 360 arc fill grestore\n", x, y, def_marknoderad);

#define markwdot(c,x,y) \
    fprintf(fp, "    %% markwdot %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      newpath %d %d %d 0 360 arc stroke grestore\n", x, y, def_marknoderad);

#define markcross(c,x,y) \
    fprintf(fp, "    %% markcross %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      newpath %d %d translate %d 0 moveto %d 0 rlineto 0 %d moveto 0 %d rlineto stroke grestore\n", (int)x, (int)y, -def_marknoderad, 2*def_marknoderad, -def_marknoderad, 2*def_marknoderad);

#define markxross(c,x,y) \
    fprintf(fp, "    %% markcross %d\n", __LINE__); \
    fprintf(fp, "      gsave"); changecolor(fp, c); \
    fprintf(fp, "      newpath %d %d translate 45 rotate %d 0 moveto %d 0 rlineto 0 %d moveto 0 %d rlineto stroke grestore\n", (int)x, (int)y, -def_marknoderad, 2*def_marknoderad, -def_marknoderad, 2*def_marknoderad);


int
_drawpathX(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns, varray_t *qar,
    int f_new, int f_close)
{
    int  i;
    int  x0, y0;
    int  x1, y1, x2, y2, x3, y3, x4, y4;
    seg *s;
    int  cdir;
    int  tx, ty; 

    int  ap, fh, bh;
    int  arcx, arcy;
    int  qbx, qby;
    int  qex, qey;
    int  qcx, qcy;

    int  actfh, actch, actbh;
    int  ahbpos, ahfpos;

    int  bwmsz;

    bwmsz = xu->cob.outlinethick*4;

    Echo("%s: qar %p\n", __func__, qar);
#if 0
    fprintf(fp, "%% %s:%d %s qar %p f_new %d f_close %d\n",
        __FILE__, __LINE__, __func__, qar, f_new, f_close);
#endif

    cdir = ydir;
#if 0
    fprintf(fp, "%% cdir %d ydir %d\n", cdir, ydir);
#endif

#if 0
varray_fprintv(stderr, qar);
#endif

#if 0
    if(xu->cob.outlinecolor<0||xu->cob.outlinethick<0) {
        fprintf(fp, "  %% %s: skip no color or no thickness\n", __func__);
        goto out;
    }
#endif

#if 0
    fprintf(fp, "    %% %s: ydir %d xox %d xoy %d\n",
        __func__, ydir, xox, xoy);
#endif

    if(qar && qar->use<=0) {
        Error("null or empty qar %p\n", qar);
        goto out;
    }
    else {
        Echo("    qar.use %d\n", qar->use);
    }

    ahbpos = path_firstvisible(qar);
    ahfpos = path_lastvisible(qar);
    Echo("  oid %d ahbpos %d ahfpos %d\n", xu->oid, ahbpos, ahfpos);

    if(f_new) {
        fprintf(fp, "    newpath\n");
    }

    x0 = x1 = xox+xu->cx+xu->cox;
    y0 = y1 = xoy+xu->cy+xu->coy;
    x2 = y2 =  -1; /*dummy*/

Echo("LINEs X oid %d xox %6d cx %6d csx %6d cox %6d; x0 %6d x1 %6d\n",
    xu->oid, xox, xu->cx, xu->csx, xu->cox, x0, x1);
Echo("LINEs X oid %d xoy %6d cy %6d csy %6d coy %6d; y0 %6d y1 %6d\n",
    xu->oid, xoy, xu->cy, xu->csy, xu->coy, y0, y1);

    Echo("    csx,csy %d,%d\n", xu->csx, xu->csy);
    Echo("    x1,y1 %d,%d\n", x1, y1);
    if(INTRACE) {
        P;
        varray_fprintv(stdout, qar);
    }

P;

    fprintf(fp, "  %d %d moveto %% zero-point\n", x1, y1);

    for(i=0;i<qar->use;i++) {
        s = (seg*)qar->slot[i];
        if(!s) {
            continue;
        }

#if 0
Echo("%s: i %d ptype %d\n", __func__, i, s->ptype);
#endif

        actfh = actch = actbh = 0;
        if(xu->cob.arrowevery) {
            actfh = xu->cob.arrowforeheadtype;
            actch = xu->cob.arrowcentheadtype;
            actbh = xu->cob.arrowbackheadtype;
        }
        else {
            if(i==0) {
                actbh = xu->cob.arrowbackheadtype;
            }
            if(i==qar->use/2) {
                actch = xu->cob.arrowcentheadtype;
            }
            if(i==qar->use-1) {
                actfh = xu->cob.arrowforeheadtype;
            }
        }

#if 0
Echo("%s: oid %d i %d seg-arrow actbh %d actch %d achfh %d\n",
    __func__, xu->oid, i, actbh, actch, actfh);
Echo("%s: oid %d i %d s ptype %d x1,y1 %d,%d x2,y2 %d,%d\n",
    __func__, xu->oid, i, s->ptype, s->x1, s->y1, s->x2, s->y2);
#endif
#if 1
Echo("%s: oid %d seg i %d type %d; actbh %d actch %d actfh %d\n",
    __func__, xu->oid, i, s->ptype, actbh, actch, actfh);
#endif 

#if 0
        /* already apply 'translate'. do not plot */
        if(xu->cob.marknode) {
            markfdot(xu->cob.outlinecolor, x1, y1);
        }
#endif

        switch(s->ptype) {

        case OA_DIR:
            cdir = s->ang;
            x2 = x1;
            y2 = y1;
            break;
        case OA_INCDIR:
            cdir += s->ang;
            x2 = x1;
            y2 = y1;
            break;
        case OA_DECDIR:
            cdir -= s->ang;
            x2 = x1;
            y2 = y1;
            break;

        case OA_JOIN:
PP;
#if 1
            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d %d 0 360 arc fill %% join-mark\n",
                x1, y1, xu->cob.outlinethick*2);
            fprintf(fp, "grestore\n");
#endif
            x2 = x1;
            y2 = y1;

            break;

        case OA_SKIP:
PP;
            x2 = x1+s->x1;
            y2 = y1+s->y1;

            qcx = (x1+x2)/2;
            qcy = (y1+y2)/2;

            fprintf(fp, "      %d %d moveto ", x1, y1);
            fprintf(fp, "      %d %d %d %d %d arcn ",
                qcx, qcy, xu->cob.outlinethick*2, ydir-180, ydir);
            fprintf(fp, "      %d %d lineto", x2, y2);

            break;

        case OA_ARC:

#if 0
    fprintf(fp, "%% OA_ARC b cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif

#if 0
    fprintf(fp, "%%  cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif
#if 0
    fprintf(stderr, "%% b cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif

            arcx = x1 + s->rad*cos((cdir+90)*rf);
            arcy = y1 + s->rad*sin((cdir+90)*rf);
            x2 = arcx + s->rad*cos((cdir+s->ang-90)*rf);
            y2 = arcy + s->rad*sin((cdir+s->ang-90)*rf);
            
            fprintf(fp, "    %d %d %d %d %d arc %% seg-arc\n",
                arcx, arcy, s->rad, cdir-90, cdir-90+s->ang);

#if 1
            if(draft_mode) {
                int a;
                fprintf(fp, "     gsave\n");
                Echo("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
                for(a=cdir-90;a<=cdir-90+s->ang;a++) {
                    if(a==cdir-90||a==cdir-90+s->ang||a%90==0) {
                        tx = arcx + s->rad*cos((a)*rf);
                        ty = arcy + s->rad*sin((a)*rf);
                        Echo("      arc  %d tx,ty %d,%d\n", a, tx, ty);
                        if(draft_mode) {
                            MC(4, tx, ty);
                        }
                    }
                }
                fprintf(fp, "    grestore\n");
            }
#endif

            if(draft_mode) {
                fprintf(fp, "gsave\n");
                changedraft(fp);
                fprintf(fp, " currentlinewidth 2 div setlinewidth\n");
                fprintf(fp,
                    "  %d %d moveto %d %d lineto %d %d lineto stroke\n",
                        x1, y1, arcx, arcy, x2, y2);
                fprintf(fp, "grestore\n");
            }

            cdir += s->ang;

#if 0
    fprintf(fp, "%% OA_ARC a cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif
#if 0
    fprintf(fp, "%% OA_ARC a cdir %d\n", cdir);
#endif

            break;



        case OA_ARCN:

#if 0
    fprintf(fp, "%%  cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
    fprintf(stderr, "%% b cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif

            arcx = x1 + s->rad*cos((cdir-90)*rf);
            arcy = y1 + s->rad*sin((cdir-90)*rf);

            x2 = arcx + s->rad*cos((cdir-s->ang+90)*rf);
            y2 = arcy + s->rad*sin((cdir-s->ang+90)*rf);
            
            fprintf(fp, "    %d %d %d %d %d arcn %% seg-arcn\n",
                arcx, arcy, s->rad, cdir+90, cdir+90-s->ang);

#if 1
            if(draft_mode) {
                int tx, ty; 
                int a;
                fprintf(fp, "    gsave\n");
                Echo("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
                for(a=cdir+90;a>=cdir+90-s->ang;a--) {
                    if(a==cdir+90||a==cdir+90-s->ang||a%90==0) {
                        tx = arcx + s->rad*cos((a)*rf);
                        ty = arcy + s->rad*sin((a)*rf);
                        Echo("      arc  %d tx,ty %d,%d\n", a, tx, ty);
                        if(draft_mode) {
                            MC(5, tx, ty);
                        }
                    }
                }
                fprintf(fp, "     grestore\n");
            }
#endif

            if(draft_mode) {
                fprintf(fp, "gsave\n");
                changedraft(fp);
                fprintf(fp, " currentlinewidth 2 div setlinewidth\n");
                fprintf(fp,
                    "  %d %d moveto %d %d lineto %d %d lineto stroke\n",
                        x1, y1, arcx, arcy, x2, y2);
                fprintf(fp, "grestore\n");
            }

            cdir -= s->ang;

            break;


        case OA_CLOSE:
            x2 = x1;
            y2 = y1;
            if(f_close) {
                fprintf(fp, "    closepath %% CLOSE X\n");
            }
            goto next;
            break;

        case OA_MOVETO:
            x2 = s->x1+xox;
            y2 = s->y1+xoy;
            fprintf(fp, "    %d %d moveto\n", x2, y2);
#if 0
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#endif
            break;

        case OA_RMOVETO:
            x2 = s->x1;
            y2 = s->y1;
            fprintf(fp, "    %d %d rmoveto\n", x2, y2);
#if 0
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#endif
            break;

        case OA_LINETO:
            x2 = s->x1+xox;
            y2 = s->y1+xoy;
            fprintf(fp, "    %d %d lineto\n", x2, y2);
            break;

        case OA_RLINETO:
#if 0
            fprintf(fp, "    %% before RLINETO; %d %d\n", x1, y1);
            fprintf(fp, "    %% RLINETO %d %d\n", s->x1, s->y1);
#endif
#if 0
            x2 = s->x1;
            y2 = s->y1;
            fprintf(fp, "    %d %d rlineto\n", x2, y2);
#endif
            x2 = s->x1+x1;
            y2 = s->y1+y1;
            fprintf(fp, "    %d %d lineto\n", x2, y2);
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#if 0
            fprintf(fp, "    %% dx %d dy %d\n", x2-x1, y2-y1);
            fprintf(fp, "    %% cdir rad %f : %d\n", atan2(y2-y1,x2-x1), __LINE__);
            fprintf(fp, "    %% cdir %d : %d\n", cdir, __LINE__);
#endif
            break;

        case OA_CURVETO:
fprintf(fp, "%%    x1,y1   %d,%d\n", x1, y1);
fprintf(fp, "%%    xox,xoy %d,%d\n", xox, xoy);
fprintf(fp, "%% xu cox,coy %d,%d\n", xu->cox, xu->coy);
            x2 = s->x1+xox;
            y2 = s->y1+xoy;
            x3 = s->x2+xox;
            y3 = s->y2+xoy;
            x4 = s->x3+xox;
            y4 = s->y3+xoy;
            fprintf(fp, "    %d %d moveto\n", x1, y1);
            fprintf(fp, "    %d %d %d %d %d %d curveto %% CURVETO %d %d %d %d %d %d %d %d\n",
                x2, y2, x3, y3, x4, y4,
                s->x1, s->y1, s->x2, s->y2, s->x3, s->y3, s->x4, s->y4);
            if(xu->cob.marknode) {
                markxross(0, s->x1, s->y1);
                markxross(0, s->x2, s->y2);
            }
            break;

        case OA_RCURVETO:
#if 0
fprintf(fp, "%%    x1,y1   %d,%d\n", x1, y1);
fprintf(fp, "%%    xox,xoy %d,%d\n", xox, xoy);
fprintf(fp, "%% xu cox,coy %d,%d\n", xu->cox, xu->coy);
#endif
            x2 = s->x1+x1;
            y2 = s->y1+y1;
            x3 = s->x2+x1;
            y3 = s->y2+y1;
            x4 = s->x3+x1;
            y4 = s->y3+y1;
#if 0
            fprintf(fp, "    %d %d moveto\n", x1, y1);
#endif
            fprintf(fp, "    %d %d %d %d %d %d curveto %% RCURVETO %d %d %d %d %d %d %d %d\n",
                x2, y2, x3, y3, x4, y4,
                s->x1, s->y1, s->x2, s->y2, s->x3, s->y3, s->x4, s->y4);
#if 0
            fprintf(fp, " %% x4,y4 %d,%d\n", x4, y4);
#endif
            if(xu->cob.marknode) {
                markxross(0, s->x1, s->y1);
                markxross(0, s->x2, s->y2);
            }

            x2 = x4;
            y2 = y4;

            break;

        case OA_LINE:
            x1 = s->x1+xox;
            y1 = s->y1+xoy;
            x2 = s->x2+xox;
            y2 = s->y2+xoy;
#if 0
            fprintf(fp,
                "    %d %d moveto %d %d lineto %% LINE\n",
                x1, y1, x2, y2);
#endif
            /* in simple line, context reaches here.
             * we have to care empty line in such case.
             */
#if 0
            fprintf(fp, "%% color %d thick %d\n",
                xu->cob.outlinecolor, xu->cob.outlinethick);
#endif
            if(xu->cob.outlinecolor<0||xu->cob.outlinethick<=0) {
                fprintf(fp,
                    "    %d %d moveto %d %d moveto %% empty LINE\n",
                    x1, y1, x2, y2);
            }
            else {
                fprintf(fp,
                    "    %d %d moveto %d %d lineto %% LINE\n",
                    x1, y1, x2, y2);
            }

#if 1
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#endif

            goto confirm_arrow;

            break;

    /* BIWAY MARKS; do not change position(s) */
        case OA_BWCIR:
            {
            int nx, ny;
            nx = s->x1+xox;
            ny = s->y1+xoy;
            fprintf(fp,
    "gsave newpath %d %d moveto %d %d %d 0 360 arc closepath fill grestore %% BWCIR %d %d\n",
                nx, ny, nx, ny, bwmsz/2, s->x1, s->y1);
            }
            continue;
            break;

        case OA_BWBOX:
            {
            int nx, ny;
            nx = s->x1+xox;
            ny = s->y1+xoy;
            fprintf(fp,
    "gsave newpath %d %d moveto %d 0 rlineto 0 %d rlineto %d 0 rlineto closepath fill grestore %% BWBOX\n",
                nx-bwmsz/2, ny-bwmsz/2, bwmsz, bwmsz, -bwmsz);
            }
            continue;
            break;

        case OA_BWXSS:
            {
            int nx, ny;
            nx = s->x1+xox;
            ny = s->y1+xoy;
            fprintf(fp,
    "gsave newpath %d %d moveto %d %d rlineto %d %d moveto %d %d rlineto closepath stroke grestore %% BWXSS\n",
                nx-bwmsz/2, ny-bwmsz/2, bwmsz, bwmsz, 
                nx-bwmsz/2, ny+bwmsz/2, bwmsz, -bwmsz);
            }
            continue;
            break;

        case OA_BWPLS:
            {
            int nx, ny;
            nx = s->x1+xox;
            ny = s->y1+xoy;
            fprintf(fp,
    "gsave newpath %d %d moveto 0 %d rlineto %d %d moveto %d 0 rlineto closepath stroke grestore %% BWPLS %d %d\n",
                nx, ny-bwmsz/2, bwmsz, nx-bwmsz/2, ny, bwmsz, s->x1, s->y2);
            }
            continue;
            break;


#if 0
        default:
#endif
        case OA_FORWARD:

            if(s->ftflag & COORD_FROM) {
P;
                Echo("    part seg %d: ftflag %d : %d,%d\n",
                    i, s->ftflag, s->x1, s->y1);
            
#if 1
#if 0
                fprintf(fp, "    %d %d moveto %% start-point\n",
                    s->x1, s->y1);
#endif
                x1 = s->x1 + xox;
                y1 = s->y1 + xoy;
                fprintf(fp, "    %d %d moveto %% start-point by FROM\n",
                    x1, y1);

                /* skip */
                Echo("    set position and skip; FROM\n");
                continue;
#endif
#if 0
                goto confirm_arrow;
#endif
            }
            if(s->ftflag & COORD_TO) {
P;
                x2 = s->x1 + xox;
                y2 = s->y1 + xoy;
#if 1
                cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#endif
                goto coord_done;
            }

P;
            fprintf(fp, "    %% b x1,y1 %d,%d x2,y2 %d,%d\n", x1, y1, x2, y2);

#if 0
            fprintf(fp, "%% forward %d %d\n", s->x1, s->y1);
            x2 = x1+s->x1;
            y2 = y1+s->y1;
#endif
#if 1
            fprintf(fp, "%% forward %d with cdir %d\n", s->x1, cdir);
#endif
#if 1
            x2 = x1+s->x1*cos(cdir*rf);
            y2 = y1+s->x1*sin(cdir*rf);
#endif

coord_done:

#if 0
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#endif
#if 0
fprintf(fp, "%% OA_FORWARD cdir %d\n", cdir);
#endif
#if 0
fprintf(stderr, "%% m atan2 %.3f\n", atan2(y2-y1,x2-x1)/rf);
fprintf(stderr, "%% m cdir %d\n", cdir);
#endif

            fprintf(fp, "    %% a x1,y1 %d,%d x2,y2 %d,%d\n", x1, y1, x2, y2);
            fprintf(fp, "    %d %d lineto %% forward %s\n", x2, y2, __func__);

confirm_arrow:
#if 0
fprintf(fp, "%% arrow f %d c %d b %d; cdir %d\n", actfh, actch, actbh, cdir);
#endif
Echo("    arrow f %d c %d b %d; cdir %d\n", actfh, actch, actbh, cdir);

            break;

        default:
            Error("unsupported segment part <%d>\n", s->ptype);
            break;
#if 0
#endif
        }
        
next:

        if(i==ahfpos && xu->cob.arrowforeheadtype) {
Echo("AH F oid %d seg %d cdir %d\n", xu->oid, i, cdir);
            fprintf(fp, "gsave\n");
            epsdraw_arrowhead(fp,
                xu->cob.arrowforeheadtype, cdir,
                xu->cob.outlinecolor, x2, y2);
            fprintf(fp, "grestore\n");
        }
        if(i==ahbpos && xu->cob.arrowbackheadtype) {
Echo("AH B oid %d seg %d cdir %d\n", xu->oid, i, cdir);
            fprintf(fp, "gsave\n");
            epsdraw_arrowhead(fp,
                xu->cob.arrowbackheadtype, cdir-180,
                xu->cob.outlinecolor, x1, y1);
            fprintf(fp, "grestore\n");
        }

#if 0
        fprintf(fp, "%% a cdir %4d : %s\n", cdir, __func__);
#endif
        x1 = x2;
        y1 = y2;
    }

    if(xu->cob.marknode) {
        markfdot(xu->cob.outlinecolor, x1, y1);
    }

P;

out:
    return 0;
}

int
drawpath(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    Echo("%s: xu %p\n", __func__, xu);
    fprintf(fp, "%% %s:%d %s\n", __FILE__, __LINE__, __func__);
    return _drawpathX(fp, ydir, xox, xoy, xu, xns, xu->cob.segar, 1, 1);
}

#define drawpathN(f,d,x,y,u,n)         _drawpathX(f,d,x,y,u,n,u->cob.segar,1,1)
#define drawpathN_woclose(f,d,x,y,u,n) _drawpathX(f,d,x,y,u,n,u->cob.segar,1,0)
#define drawpathR(f,d,x,y,u,n)         _drawpathX(f,d,x,y,u,n,u->cob.seghar,1,1)
#define drawpathR_wonew(f,d,x,y,u,n)   _drawpathX(f,d,x,y,u,n,u->cob.seghar,0,1)


int
drawsym(FILE *fp, double x, double y, double a, double pt, int c, int ty,
    double lax, double lay, double *cax, double *cay)
{
    double x1, y1, x2, y2;
    double run;
    double dx, dy;
    double r;
    double fx;
    int    act=0;
    double px, py, qx, qy;
    double ww;
    double ax, ay;
    double mx, my;

    int    mact;
    int    mangle;

#if 0
    fprintf(stdout, "%% %s: x %.3f y %.3f a %.3f c %d ty %d\n", __func__, x, y, a, c, ty);
    fflush(stdout);
#endif

#if 0
    fprintf(fp, "%% %s: x %.3f y %.3f a %.3f c %d ty %d\n", __func__, x, y, a, c, ty);
#endif

    if(isnan(x) || isnan(y)) {
#if 0
        Error("%s: nan is detected; %.3f,%.3f\n", __func__, x, y);
#endif
        return -1;
    }

    mact = 0;
    mangle = 45;
    ww = def_linedecothick;

    switch(ty){ 
    case LT_LMUST:
        mact = 1;
        mangle = 90;
        act = 0;
        run = pt;
        break;
    case LT_RMUST:
        mact = 1;
        mangle = -90;
        act = 0;
        run = pt;
        break;

    case LT_DASHED:
    case LT_DOTTED:
    case LT_CHAINED:
    case LT_DOUBLECHAINED:
        act = solve_dashpart(ty, c);
        run = pt;
        goto purelinetype;
        
        break;    

    case LT_TRIANGLE:
        
        fprintf(fp, "  gsave\n");

        fprintf(fp, "    %.2f %.2f translate 0 0 moveto %.2f rotate\n", x, y, a);

        r  = objunit/10;
        fx = r*0.707;
        fprintf(fp, "    %.2f 0 translate\n", fx);

        dx = r*cos(135*rf);
        dy = r*sin(135*rf);
        fprintf(fp, "    %.2f %.2f moveto ", dx, dy);
        fprintf(fp, "    0 0 lineto ");

        dx = r*cos(-135*rf);
        dy = r*sin(-135*rf);
        fprintf(fp, "    %.2f %.2f lineto closepath fill\n",
            dx, dy);
        
        fprintf(fp, "  grestore\n");
        goto out;
        break;

    case LT_MOUNTAIN:
        
        fprintf(fp, "  gsave\n");

        fprintf(fp, "    %.2f %.2f translate 0 0 moveto %.2f rotate\n", x, y, a);

        r  = objunit/10;
        fx = r*0.707;
        fprintf(fp, "    %.2f 0 translate\n", fx);

        dx = r*cos(135*rf);
        dy = r*sin(135*rf);
        fprintf(fp, "    %.2f %.2f moveto ", dx, dy);
        fprintf(fp, "    0 0 lineto ");

        dx = r*cos(-135*rf);
        dy = r*sin(-135*rf);
        fprintf(fp, "    %.2f %.2f lineto stroke\n",
            dx, dy);
        
        fprintf(fp, "  grestore\n");
        goto out;
        break;

    case LT_DBL:
    case LT_DBR:
        if(ty==LT_DBL) {
            px = x + ww*cos((a+90)*rf);
            py = y + ww*sin((a+90)*rf);
        }
        else {
            px = x + ww*cos((a-90)*rf);
            py = y + ww*sin((a-90)*rf);
        }

        if(c==0) {
            fprintf(fp, "  %.3f %.3f moveto\n", px, py);
        }
        else {
            fprintf(fp, "  %.3f %.3f lineto\n", px, py);
        }

#if 0
        MP(4, (int)px, (int)py);
#endif

        *cax = px;
        *cay = py;
        
        break;

    case LT_WAVED:
        px = x + ww*cos((a+45)*rf);
        py = y + ww*sin((a+45)*rf);
        qx = x + ww*cos((a-45)*rf);
        qy = y + ww*sin((a-45)*rf);

#if 0
        MX(2, (int)px, (int)py);
        MP(4, (int)qx, (int)qy);
#endif

        if(c==0) {
        }
        else
        if(c%2==0) {
            drawwave(fp, lax, lay, x, y, 0);
        }
        else 
        if(c%2==1) {
            drawwave(fp, lax, lay, x, y, 1);
        }
        *cax = x;
        *cay = y;
    break;

    case LT_ZIGZAG:
#if 0
        mx = (lax+x)/2;
        my = (lay+y)/2;

        px = mx + ww*cos((a+90)*rf);
        py = my + ww*sin((a+90)*rf);
        qx = mx + ww*cos((a-90)*rf);
        qy = my + ww*sin((a-90)*rf);

#if 0
        MCF(2, (int)px, (int)py);
        MX(0, (int)mx, (int)my);
        MQF(4, (int)qx, (int)qy);
        MT(1, (int)x, (int)y, (int)a);
        MP(3, (int)lax, (int)lay);
#endif

        if(c%2==0) {
            ax = px; ay = py;
        }
        else 
        if(c%2==1) {
            ax = qx; ay = qy;
        }
#if 0
        fprintf(fp, "  %.3f %.3f moveto %.3f %.3f lineto stroke\n", lax, lay, ax, ay);
#endif
        if(c==0) {
            fprintf(fp, "  %.3f %.3f moveto\n", ax, ay);
        }
        fprintf(fp, "  %.3f %.3f lineto\n", ax, ay);
        fprintf(fp, "  %.3f %.3f lineto\n", x, y);
        *cax = x;
        *cay = y;
#endif

#if 0
        px = x + ww*cos((a+45)*rf);
        py = y + ww*sin((a+45)*rf);
        qx = x + ww*cos((a-45)*rf);
        qy = y + ww*sin((a-45)*rf);

#if 0
        MC(2, (int)px, (int)py);
        MQ(4, (int)qx, (int)qy);
#endif

        if(c%2==0) {
            ax = px; ay = py;
        }
        else 
        if(c%2==1) {
            ax = qx; ay = qy;
        }
#if 0
        fprintf(fp, "  %.3f %.3f moveto %.3f %.3f lineto stroke\n", lax, lay, ax, ay);
#endif
        if(c==0) {
            fprintf(fp, "  %.3f %.3f moveto\n", ax, ay);
        }
        fprintf(fp, "  %.3f %.3f lineto\n", ax, ay);
        *cax = ax;
        *cay = ay;
#endif

#if 1
        if(c==0) {
            fprintf(fp, "  %.3f %.3f moveto\n", x, y);
        }
        else {
            mx = (lax+x)/2;
            my = (lay+y)/2;

            px = mx + ww*cos((a+90)*rf);
            py = my + ww*sin((a+90)*rf);
            qx = mx + ww*cos((a-90)*rf);
            qy = my + ww*sin((a-90)*rf);

#if 0
            MX(0, (int)mx, (int)my);
            MC(2, (int)px, (int)py);
            MQ(4, (int)qx, (int)qy);
#endif

            if(c%2==0) {
                ax = px; ay = py;
            }
            else 
            if(c%2==1) {
                ax = qx; ay = qy;
            }
#if 0
            fprintf(fp, "  %.3f %.3f moveto %.3f %.3f lineto stroke\n", lax, lay, ax, ay);
#endif
            fprintf(fp, "  %.3f %.3f lineto\n", ax, ay);
        }
    
        *cax = x;
        *cay = y;
#endif

        break;

    case LT_CIRCLE:
        r  = def_linedecothick/2;
        fprintf(fp, "  newpath %.2f %.2f %.2f 0 360 arc fill\n", x, y, r);
        goto out;
        break;
    default:
        MX(1, (int)x, (int)y);
        break;
    }

purelinetype:

    if(act) {
        x1 = x;
        y1 = y;
        x2 = run*cos(a*rf);
        y2 = run*sin(a*rf);
        fprintf(fp,
            "  %9.2f %9.2f moveto %9.2f %9.2f rlineto stroke %% drawsym\n",
            x1, y1, x2, y2);
    }

#if 0
    MTF(1, (int)x1, (int)y1, (int)a);
#endif

    if(mact) {
        x1 = x;
        y1 = y;
        x2 = def_mustsize*cos((a+mangle)*rf);
        y2 = def_mustsize*sin((a+mangle)*rf);
        fprintf(fp,
            "  %9.2f %9.2f moveto %9.2f %9.2f rlineto stroke %% drawsym\n",
            x1, y1, x2, y2);
    }

out:

    return 0;
}

/*
 *
 */

extern int
_bez_LT(FILE *fp, ob *xu, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

  
int
__drawpath_LT(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns, varray_t *qar)
{
    double trip;
    double pitch;
    int    count;
    double etrip;
    int    nd;
    double u, ui;
    double vpitch;
    double v, vi;
    double px, py;
    double qx, qy;
    double ttrip;

    double us, ue;
    double vs, ve;

    int    i;
    int    x0, y0;
    int    x1, y1, x2, y2, x3, y3, x4, y4;
    seg   *s;
    int    cdir;
    double dcdir;

    double lx, ly;
    double cx, cy;

    int    ap, fh, bh;
    int    arcx, arcy;
    int    qbx, qby;
    int    qex, qey;
    int    qcx, qcy;

    int    gsym, gpitch;

    int    actfh, actch, actbh;
    int    ahbpos, ahfpos;

    int    bwmsz;

    int    isfseg;
    int    fsegtype;

#if 0
    bwmsz = objunit/20;
#endif
    bwmsz = xu->cob.outlinethick*4;

    Echo("%s: xu %p oid %d qar %p use %d\n",
        __func__, xu, xu->oid, qar, qar->use);

#if 0
    fprintf(fp, "%% %s: enter ydir %d\n", __func__, ydir);
#endif
P;
#if 0
    changecolor(fp, xu->vob.outlinecolor);
    changethick(fp, xu->vob.outlinethick);
#endif

    if(xu->cob.outlinetype==LT_SOLID) {
P;
        _drawpathX(fp, ydir, xox, xoy, xu, xns, qar, 1, 1);
        fprintf(fp, "    stroke %% solid\n");
#if 0
        fprintf(fp, "%% %s:%d out2 oid %d ydir %d cdir %4d\n",
            __func__, __LINE__, xu->oid, ydir, cdir);
#endif
        return 0;
    }
#if 0
    fprintf(fp, "%% %s: middle\n", __func__);
#endif

    gsym = xu->cob.outlinetype;
    pitch = solve_pitch(xu->cob.outlinetype);
#if 1
    Echo("%s: gsym %d pitch %.2f (def_linedecopitch %d)\n",
        __func__, gsym, pitch, def_linedecopitch); 
#endif

#if 0
Echo("%s: ydir %d xox %d xoy %d linetype %d\n",
    __func__, ydir, xox, xoy, xu->cob.outlinetype);
#endif
    Echo("%s: enter\n", __func__);

#if 0
    fprintf(fp, "%% %s: ydir %d xox %d xoy %d linetype %d\n",
        __func__, ydir, xox, xoy, xu->cob.outlinetype);
#endif

    cdir = ydir;
    dcdir = ydir;

    if(bbox_mode) {
    }

    fprintf(fp, "  newpath\n");

    if(qar && qar->use>0) {
    }
    else {
        goto out;
    }

    Echo("    qar.use %d\n", qar->use);

    ahbpos = path_firstvisible(qar);
    ahfpos = path_lastvisible(qar);
    Echo("  oid %d ahbpos %d ahfpos %d\n", xu->oid, ahbpos, ahfpos);

#if 0
    changethick(fp, xu->vob.outlinethick);
#endif

    x0 = x1 = xox+xu->cx+xu->cox;
    y0 = y1 = xoy+xu->cy+xu->coy;

Echo("LINEs d oid %d xox %6d cx %6d csx %6d cox %6d; x0 %6d x1 %6d\n",
    xu->oid, xox, xu->cx, xu->csx, xu->cox, x0, x1);
Echo("LINEs d oid %d xoy %6d cy %6d csy %6d coy %6d; y0 %6d y1 %6d\n",
    xu->oid, xoy, xu->cy, xu->csy, xu->coy, y0, y1);

    Echo("    csx,csy %d,%d\n", xu->csx, xu->csy);
    Echo("    x1,y1 %d,%d\n", x1, y1);
    if(INTRACE) {
        P;
        varray_fprintv(stdout, qar);
    }

P;

    trip  = 0.0;
    count = 0;

    for(i=0;i<qar->use;i++) {
P;
        isfseg = 0;
        if(i>=qar->use-1) {
            isfseg = 1;
        }

        s = (seg*)qar->slot[i];
        if(!s) {
            continue;
        }

        actfh = actch = actbh = 0;
        if(xu->cob.arrowevery) {
            actfh = xu->cob.arrowforeheadtype;
            actch = xu->cob.arrowcentheadtype;
            actbh = xu->cob.arrowbackheadtype;
        }
        else {
            if(i==0) {
                actbh = xu->cob.arrowbackheadtype;
            }
            if(i==qar->use%2) {
                actch = xu->cob.arrowcentheadtype;
            }
            if(i==qar->use-1) {
                actfh = xu->cob.arrowforeheadtype;
            }
        }

#if 0
Echo("oid %d %s %d ptype %d seg-arrow actbh %d actch %d achfh %d\n",
    xu->oid, __func__, __LINE__,  s->ptype, actbh, actch, actfh);
#endif
#if 0
Echo("%s: oid %d seg i %d type %d; actbh %d actch %d actfh %d\n",
    __func__, xu->oid, i, s->ptype, actbh, actch, actfh);
#endif
#if 0
    fprintf(fp, "%% cdir %d : before switch %d\n", cdir, __LINE__);
#endif

        switch(s->ptype) {

        case OA_DIR:
            cdir = s->ang;
            dcdir = s->ang;
            x2 = x1;
            y2 = y1;
            break;
        case OA_INCDIR:
            cdir += s->ang;
            dcdir += s->ang;
            x2 = x1;
            y2 = y1;
            break;
        case OA_DECDIR:
            cdir -= s->ang;
            dcdir -= s->ang;
            x2 = x1;
            y2 = y1;
            break;

        case OA_JOIN:
P;
#if 1
            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d %d 0 360 arc fill %% join-mark\n",
                x1, y1, xu->cob.outlinethick*2);
            fprintf(fp, "grestore\n");
#endif
            x2 = x1;
            y2 = y1;

            break;
        case OA_SKIP:
P;
            x2 = x1+s->x1;
            y2 = y1+s->y1;

            qcx = (x1+x2)/2;
            qcy = (y1+y2)/2;

            fprintf(fp, "    %d %d moveto ", x1, y1);
            fprintf(fp, "    %d %d %d %d %d arcn ",
                qcx, qcy, xu->cob.outlinethick*2, ydir-180, ydir);
            fprintf(fp, "    %d %d lineto", x2, y2);

            break;


        case OA_ARC:

            arcx = x1 + s->rad*cos((cdir+90)*rf);
            arcy = y1 + s->rad*sin((cdir+90)*rf);

            x2 = arcx + s->rad*cos((cdir+s->ang-90)*rf);
            y2 = arcy + s->rad*sin((cdir+s->ang-90)*rf);

            etrip = ((double)s->rad)*s->ang/rf;
            etrip = ((double)s->rad*2*M_PI)*((double)s->ang/360.0);
            nd = etrip / pitch;

    if(nd<=0) goto skip_arc;

            if(trip<=0) {
                ui = 0.0;
            }
            else {
                ui = pitch  - (trip - (int)(trip/pitch)*pitch);
            }

P;
            vpitch = ((double)s->ang)*pitch/etrip;
            vi = ui/(2*M_PI*s->rad)*360;

            ttrip = trip;

            us = 0;
            if(actbh) {
                us += def_arrowsize;
            }
            vs = us/(2*M_PI*s->rad)*360;

            ue = 0;
            if(actfh) {
                ue += def_arrowsize;
            }
            ve = ue/(2*M_PI*s->rad)*360;

Echo("us %.3f vs %.3f; ue %.3f ve %.3f; etrip %.3f vi %.3f s->ang %.3f\n",
    us, vs, ue, ve, etrip, vi, (double)s->ang);

            for(v=vi+vs;v<=(double)s->ang-ve;v+=vpitch) {
                px = arcx + s->rad*cos((cdir+v-90)*rf);
                py = arcy + s->rad*sin((cdir+v-90)*rf);
                ttrip = trip + (v*2*M_PI*s->rad)/360;

P;
#if 0
                if(count>10) {
                    printf("%s:%d oid %d count %d\n",
                        __func__, __LINE__, xu->oid, count);
                    printf("  ui %.3f us %.3f etrip %.3f ue %.3f ; pitch %.3f\n",
                        ui, us, etrip, ue, pitch);
                    fflush(stdout);
                }
#endif

                if(xu->cob.markpitch) {
                    MP(1, (int)px, (int)py);
                }

                drawsym(fp, px, py, cdir+v, pitch, count, gsym, lx, ly, &cx, &cy);

                lx = cx;
                ly = cy;
                count++;
            }
            trip += etrip;

#if 0
            if(actch) {
                px = arcx + s->rad*cos((cdir+(s->ang/2)-90)*rf);
                py = arcy + s->rad*sin((cdir+(s->ang/2)-90)*rf);
                if(actch>0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir+(s->ang/2),
                        xu->cob.outlinecolor, px, py);
                }
                if(actch<0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir+(s->ang/2) + 180,
                        xu->cob.outlinecolor, px, py);
                }
            }

            if(actfh>0) {
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, cdir+s->ang-ve/2,
                    xu->cob.outlinecolor, x2, y2);
            }
            if(actbh>0) {
                epsdraw_arrowhead(fp,
                    xu->cob.arrowbackheadtype, cdir-180+vs/2,
                    xu->cob.outlinecolor, x1, y1);
            }
#endif

skip_arc:
            cdir += s->ang;
            dcdir += s->ang;
#if 0
            fprintf(fp, "%% cdir %d : end arc %d\n", cdir, __LINE__);
#endif

            break;


        case OA_ARCN:

            arcx = x1 + s->rad*cos((cdir-90)*rf);
            arcy = y1 + s->rad*sin((cdir-90)*rf);

            x2 = arcx + s->rad*cos((cdir-s->ang+90)*rf);
            y2 = arcy + s->rad*sin((cdir-s->ang+90)*rf);

            etrip = ((double)s->rad)*s->ang/rf;
            etrip = ((double)s->rad*2*M_PI)*((double)s->ang/360.0);
            nd = etrip / pitch;
            fprintf(fp, "%% etrip %.3f pitch %.3f -> nd %d\n",
                etrip, pitch, nd);


#if 0
                fprintf(fp, "%% i %d count %d: trip %.3f etrip %.3f\n",
                    i, count, trip, etrip);
#endif

    if(nd<=0) goto skip_arcn;

            if(trip<=0) {
                ui = 0.0;
            }
            else {
                ui = pitch  - (trip - (int)(trip/pitch)*pitch);
            }

P;
            vpitch = ((double)s->ang)*pitch/etrip;
            vi = ui/(2*M_PI*s->rad)*360;

            ttrip = trip;

            us = 0;
            if(actbh) {
                us += def_arrowsize;
            }
            vs = us/(2*M_PI*s->rad)*360;

            ue = 0;
            if(actfh) {
                ue += def_arrowsize;
            }
            ve = ue/(2*M_PI*s->rad)*360;

Echo("us %.3f vs %.3f; ue %.3f ve %.3f; etrip %.3f vi %.3f s->ang %.3f\n",
    us, vs, ue, ve, etrip, vi, (double)s->ang);

#if 0
            fprintf(fp, "%% i %d count %d: ui %.3f vi %.3f vpitch %.3f arcn\n",
                    i, count, ui, vi, vpitch);
#endif
            for(v=vi+vs;v<=(double)s->ang-ve;v+=vpitch) {
                px = arcx + s->rad*cos((cdir-v+90)*rf);
                py = arcy + s->rad*sin((cdir-v+90)*rf);
                ttrip = trip + (v*2*M_PI*s->rad)/360;

                if(count>10) {
                    printf("%s:%d oid %d count %d\n",
                        __func__, __LINE__, xu->oid, count);
                    fflush(stdout);
                }

                if(xu->cob.markpitch) {
                    MP(1, (int)px, (int)py);
                }

                fprintf(fp,
                "%% i %3d count %3d: ttrip %8.2f v %8.2f px,py %8.2f %8.2f arcn\n",
                    i, count, ttrip, v, px, py);

                drawsym(fp, px, py, cdir-v, pitch, count, gsym, lx, ly, &cx, &cy);
                lx = cx;
                ly = cy;
                count++;
            }
            trip += etrip;
#if 0
                fprintf(fp, "%% i %d count %d: trip %.3f\n",
                    i, count, trip);
#endif

#if 0
            if(actch) {
                px = arcx + s->rad*cos((cdir-(s->ang/2)+90)*rf);
                py = arcy + s->rad*sin((cdir-(s->ang/2)+90)*rf);
                if(actch>0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir-(s->ang/2),
                        xu->cob.outlinecolor, px, py);
                }
                if(actch<0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir-(s->ang/2) + 180,
                        xu->cob.outlinecolor, px, py);
                }
            }

            if(actfh>0) {
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, (int)(cdir-(s->ang-ve/2)),
                    xu->cob.outlinecolor, x2, y2);
            }
            if(actbh>0) {
                epsdraw_arrowhead(fp,
                    xu->cob.arrowbackheadtype, cdir-180-vs/2,
                    xu->cob.outlinecolor, x1, y1);
            }
#endif

skip_arcn:
            cdir -= s->ang;
            dcdir -= s->ang;
#if 1
            fprintf(fp, "%% cdir %d : end arcn %d\n", cdir, __LINE__);
#endif

            break;

        case OA_CLOSE:
            x2 = x1;
            y2 = y1;
            fprintf(fp, "  closepath %% CLOSE d\n");
            goto next;
            break;

        case OA_MOVETO:
            x2 = s->x1 + xox;
            y2 = s->y1 + xoy;
            fprintf(fp, "  %d %d moveto\n", x2, y2);
            goto next;
            break;

        case OA_RMOVETO:
            x2 = s->x1 + x1;
            y2 = s->y1 + y1;
            fprintf(fp, "  %d %d moveto\n", x2, y2);
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
            break;

        case OA_LINETO:
            x2 = s->x1 + xox;
            y2 = s->y1 + xoy;
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
            goto coord_done;
            break;

        case OA_RLINETO:
            x2 = s->x1 + x1;
            y2 = s->y1 + y1;
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
            goto coord_done;
            break;

        case OA_CURVETO:
            x2 = s->x1;
            y2 = s->y1;
            x3 = s->x2;
            y3 = s->y2;
            x4 = s->x3;
            y4 = s->y3;

#if 0
            fprintf(fp, "%% RCUVETO %d %d %d %d %d %d\n", 
                s->x1, s->y1, s->x2, s->y2, s->x3, s->y3);
#endif

            fprintf(fp, "gsave\n");
            fprintf(fp, "newpath\n");
            _bez_LT(fp, xu, x1, y1, x2, y2, x3, y3, x4, y4);
            fprintf(fp, "grestore\n");

            x2 = x4;
            y2 = y4;
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
            goto next;

            break;

        case OA_RCURVETO:
            x2 = x1+s->x1;
            y2 = y1+s->y1;
            x3 = x1+s->x2;
            y3 = y1+s->y2;
            x4 = x1+s->x3;
            y4 = y1+s->y3;

#if 0
            fprintf(fp, "%% RCUVETO %d %d %d %d %d %d\n", 
                s->x1, s->y1, s->x2, s->y2, s->x3, s->y3);
#endif

#if 0
            fprintf(stderr, "_bez_LT in %s:%d\n", __FILE__,__LINE__);
#endif
            fprintf(fp, "gsave\n");
            fprintf(fp, "newpath\n");
            _bez_LT(fp, xu, x1, y1, x2, y2, x3, y3, x4, y4);
            fprintf(fp, "grestore\n");

            x2 = x4;
            y2 = y4;
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
            goto next;

            break;

        case OA_LINE:
            x1 = s->x1 + xox;
            y1 = s->y1 + xoy;
            x2 = s->x2 + xox;
            y2 = s->y2 + xoy;
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
            goto coord_done;
            break;


    /* BIWAY MARKS; do not change position(s) */
        case OA_BWCIR:
            {
            int nx, ny;
            nx = s->x1+xox;
            ny = s->y1+xoy;
            fprintf(fp,
    "gsave newpath %d %d moveto %d %d %d 0 360 arc closepath fill grestore %% BWCIR %d %d\n",   
                nx, ny, nx, ny, bwmsz/2, s->x1, s->y1);
            }
            goto next;
            break;




        case OA_FORWARD:

/* XXX */
/* FORWARD */
            if(s->ftflag & COORD_FROM) {
P;
                Echo("    part seg %d: ftflag %d : %d,%d\n",
                    i, s->ftflag, s->x1, s->y1);
            
Echo("FROM\n");
Echo("%s: FORWARD FROM %d,%d\n", __func__, s->x1, s->y1);

                x1 = s->x1 + xox;
                y1 = s->y1 + xoy;

                /* skip */
                continue;

            }
            if(s->ftflag & COORD_TO) {
Echo("%s: FORWARD TO   %d,%d\n", __func__, s->x1, s->y1);
P;
                x2 = s->x1 + xox;
                y2 = s->y1 + xoy;
                cdir = (int)(atan2(y2-y1,x2-x1)/rf);
                goto coord_done;
            }

#if 0
            x2 = x1+s->x1;
            y2 = y1+s->y1;
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#endif
            x2 = x1+s->x1*cos(cdir*rf);
            y2 = y1+s->x1*sin(cdir*rf);


coord_done:
/*****
 ***** finally drawing process
 ******/
#if 0
fprintf(fp, "%% line %d cdir %d\n", __LINE__, cdir);
#endif

#if 0
            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#endif
            dcdir = atan2(y2-y1,x2-x1)/rf;
            v = atan2(y2-y1,x2-x1)/rf;

            etrip = SQRT_2DD_I2D(x1,y1,x2,y2);
            nd = etrip / pitch;

            if(trip<=0) {
                ui = 0.0;
            }
            else {
                ui = pitch  - (trip - (int)(trip/pitch)*pitch);
            }

            us = 0;
            if(actbh) {
                us += def_arrowsize;
            }
            ue = 0;
            if(actfh) {
                ue += def_arrowsize;
            }

            ttrip = trip;

            for(u=ui+us;u<=etrip-ue;u+=pitch) {
                static double lpx, lpy;
                double xd, yd;

                px = x1 + (x2-x1) * u/etrip;
                py = y1 + (y2-y1) * u/etrip;

                ttrip = trip + u;

                xd = lpx - px;
                yd = lpy - py;
                lpx = px;
                lpy = py;

                if(count<10 || count>nd-10) {
                  if(_p_) {
                    printf("%s:%d oid %d count %d u %.2f ui %.2f us %.2f etrip %.2f ue %.2f ; pitch %.2f\n",
                        __func__, __LINE__, xu->oid, count,
                        u, ui, us, etrip, ue, pitch);
                    printf("%s:%d oid %d count %d px %.2f py %.2f xd %.2f yd %.2f\n",
                        __func__, __LINE__, xu->oid, count,
                        px, py, xd, yd);
                    fflush(stdout);
                  }
                }

                if(xu->cob.markpitch) {
                    MP(1, (int)px, (int)py);
                }


                drawsym(fp, px, py, v, pitch, count, gsym, lx, ly, &cx, &cy);
                lx = cx;
                ly = cy;
                count++;
            }

#define FIN_IGN     (-1)
#define FIN_NONE    (0)
#define FIN_LINE    (1)
#define FIN_SYM     (2)
#define FIN_ELINE   (3)

            if(isfseg>0) {
                fsegtype = FIN_NONE;
                switch(gsym) {
                case LT_MOUNTAIN:
                case LT_TRIANGLE:
                case LT_CIRCLE:
                    fsegtype = FIN_IGN;
                    break;
                case LT_WAVED:
                    fsegtype = FIN_LINE;
                    break;
                case LT_ZIGZAG:
                    fsegtype = FIN_ELINE;
                    break;
                case LT_DOTTED:
                case LT_DASHED:
                    break;
                case LT_DBL:
                case LT_DBR:
                    fsegtype = FIN_SYM;
                    break;
                default:
                    break;
                }

                u = etrip-ue;
                px = x1 + (x2-x1) * u/etrip;
                py = y1 + (y2-y1) * u/etrip;

                switch(fsegtype) {
                case FIN_LINE:
 if(count>0) {
                    fprintf(fp, "  %.3f %.3f lineto %% line-to-end\n", px, py);
 }
                    break;
                case FIN_ELINE:
 if(count>0) {
                    fprintf(fp, "  %.3f %.3f lineto %% final-seg\n", cx, cy);
                    fprintf(fp, "  %.3f %.3f lineto %% line-to-end\n", px, py);
 }
                    break;
                case FIN_SYM:
                    drawsym(fp, px, py, v, pitch, count, gsym, lx, ly, &cx, &cy);
                    break;
                }
            }


            trip += etrip;
#if 0
            fprintf(fp, "%% i %d count %d: trip %.3f\n",
                i, count, trip);
#endif

#if 0
            if(actch) {
                px = x1 + (x2-x1)/2;
                py = y1 + (y2-y1)/2;
                if(actch>0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir,
                        xu->cob.outlinecolor, px, py);
                }
                if(actch<0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir+180,
                        xu->cob.outlinecolor, px, py);
                }
            }
            if(actfh>0) {
P;
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, cdir,
                    xu->cob.outlinecolor, x2, y2);
            }
            if(actbh>0) {
P;
                epsdraw_arrowhead(fp,
                    xu->cob.arrowbackheadtype, cdir-180,
                    xu->cob.outlinecolor, x1, y1);
            }
#endif

            break;

        default:
            Error("unsupported segment part <%d>\n", s->ptype);
            break;
        }
        
next:
#if 0
    fprintf(fp, "%% cdir %d : after  switch %d\n", cdir, __LINE__);
#endif

        if(xu->cob.marknode) {
            markfdot(xu->cob.outlinecolor, x1, y1);
        }

        if(i==ahfpos && xu->cob.arrowforeheadtype) {
Echo("AH F oid %d seg %d cdir %d\n", xu->oid, i, cdir);
            fprintf(fp, "gsave\n");
            epsdraw_arrowhead(fp,
                xu->cob.arrowforeheadtype, cdir,
                xu->cob.outlinecolor, x2, y2);
            fprintf(fp, "grestore\n");
        }
        if(i==ahbpos && xu->cob.arrowbackheadtype) {
Echo("AH B oid %d seg %d cdir %d\n", xu->oid, i, cdir);
            fprintf(fp, "gsave\n");
            epsdraw_arrowhead(fp,
                xu->cob.arrowbackheadtype, cdir-180,
                xu->cob.outlinecolor, x1, y1);
            fprintf(fp, "grestore\n");
        }


#if 0
        fprintf(fp, "%% %s:%d oid %d cdir %4d\n",
            __func__, __LINE__, xu->oid, cdir);
#endif
        x1 = x2;
        y1 = y2;

    }

    if(gsym==LT_ZIGZAG) {
        fprintf(fp, "  stroke %% finish stroke\n");
    }
    if(gsym==LT_WAVED) {
        fprintf(fp, "  stroke %% finish stroke\n");
    }
    if(gsym==LT_DBL || gsym==LT_DBR) {
        fprintf(fp, "  stroke %% finish stroke\n");
    }
#if 0
    if(gsym==LT_ZIGZAG||gsym==LT_WAVED) {
        fprintf(fp, "  stroke %% finish stroke\n");
    }
#endif

    if(xu->cob.marknode) {
        markfdot(xu->cob.outlinecolor, x1, y1);
    }

P;
    if(xu->type==CMD_ULINE) {
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x2, y2, x0, y0, xu, xns);
    }

#if 0
    fprintf(fp, " %% %s:%d oid %d out3 ydir %d cdir %d dcdir %.3f\n",
        __func__, __LINE__, ydir, xu->oid, cdir, dcdir);
#endif

out:
    return 0;
}

int
_drawpath_LT(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    Echo("%s: xu %p oid %d\n", __func__, xu, xu->oid);
    return __drawpath_LT(fp, ydir, xox, xoy, xu, xns, xu->cob.segar);
}

int
drawpath_LT(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int ik;
    int orig_ltype;

    Echo("%s: xu %p\n", __func__, xu);

#if 1
    if(xu->cob.markpath) {
        fprintf(fp, " gsave\n");
        changecolor(fp, def_markcolor);
        changethick(fp, xu->cob.outlinethick);
        ik = drawpathN(fp, ydir, xox, xoy, xu, xns);
        fprintf(fp, "  stroke\n");
        fprintf(fp, " grestore\n");
    }
#endif

    orig_ltype = xu->cob.outlinetype;
    if(orig_ltype==LT_DOUBLED) {
        xu->cob.outlinetype = LT_DBL;
        ik = _drawpath_LT(fp, ydir, xox, xoy, xu, xns);
        xu->cob.outlinetype = LT_DBR;
        ik = _drawpath_LT(fp, ydir, xox, xoy, xu, xns);
        xu->cob.outlinetype = orig_ltype;
        return ik;
    }
    else
    if(orig_ltype==LT_LMUST || orig_ltype==LT_RMUST) {
        xu->cob.outlinetype = LT_SOLID;
        ik = _drawpath_LT(fp, ydir, xox, xoy, xu, xns);
        xu->cob.outlinetype = orig_ltype;
        ik = _drawpath_LT(fp, ydir, xox, xoy, xu, xns);
        return ik;
    }
    else {
        return _drawpath_LT(fp, ydir, xox, xoy, xu, xns);
    }
}


int epsdraw_hatch(FILE *fp, int aw, int ah, int hc, int hty, int hp);

int
QQ__solve_dir(ns *xns, ob *u, varray_t *opar,
    int X, int *ang)
{
    int    i;
    segop *e;
    int    ik;

    Echo("%s: enter\n", __func__);

    for(i=0;i<opar->use;i++) {
        e = (segop*)opar->slot[i];
        if(!e)
            continue;
        if(!e->val || !e->val[0]) {
            continue;
        }
        switch(e->cmd) {
        case OA_DIR:
            *ang = atoi(e->val);
            break;
        }
    }

    Echo("%s: leave\n", __func__);

    return 0;
}

        
int
_bez_solid(FILE *fp, ob *xu, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    int x, y;
    double t;
    double rx, ry;
    double lx, ly;
    double dx, dy;
    double s;
    double d;
    double sumd, maxd;
    
    fprintf(fp, "%% %s: %d %d %d %d %d %d %d %d\n",
        __func__, x1, y1, x2, y2, x3, y3, x4, y4);

    s = 1.0/50.0;
    sumd = 0.0;
    maxd = -1;
    
    lx = rx = (double)x1;
    ly = ry = (double)y1;

    fprintf(fp, "gsave\n");
    fprintf(fp, "  %d %d moveto\n", x1, y1);
    for(t=0;t<=1.0;t+=s) {

        _bez_pos(&rx, &ry, t,
            (double)x1, (double)y1, (double)x2, (double)y2,
            (double)x3, (double)y3, (double)x4, (double)y4);

        d  = sqrt((rx-lx)*(rx-lx)+(ry-ly)*(ry-ly));
        if(d>maxd) maxd = d;
        sumd += d;

        x = (int)rx;
        y = (int)ry;
        fprintf(fp, "  %d %d lineto\n", x, y);
#if 0
        fprintf(fp, "  %d %d lineto %% d %.2f\n", x, y, d);
        markcross(2,x,y);
#endif

        lx = rx;
        ly = ry;
    }
    fprintf(fp, "  %d %d lineto\n", x4, y4);
    fprintf(fp, "  stroke\n");
    fprintf(fp, "grestore\n");

    return 0;
}


int
_bez_LT(FILE *fp, ob *xu, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    int x, y;
    double lastt;
    double t;
    double rx, ry;
    double lx, ly;
    double dx, dy;
    double s;
    double d;
    double sumd, maxd;
    double pitch;
    int i;
    double u;
    double upitch, uwidth;
    double cap;
    int act;
    double a;
    double mx, my;
    double nx, ny;

#if 0
    fprintf(fp, "%% %s\n", __func__);
#endif

#if 1
    if(xu->cob.markpath) {
        fprintf(fp, "gsave\n");
        changecolor(fp, def_markcolor);
        changethick(fp, xu->cob.outlinethick);
        SLW_14(fp);
        fprintf(fp, "0 0 1 setrgbcolor\n");
        _bez_solid(fp, xu, x1, y1, x2, y2, x3, y3, x4, y4);
        fprintf(fp, "grestore\n");
    }
#endif

#if 0
    fprintf(fp, "%% %s: linetype %d\n", __func__, xu->cob.outlinetype);
#endif
    
    s = 1.0/50.0;
    sumd = 0.0;
    maxd = -1;

    cap = s/100.0;

    upitch = s/50.0;
    uwidth = s*30.0;

#if 0
    fprintf(fp, "%% s %.2f\n", s);
    fprintf(fp, "%% upitch %.2f uwidth %.2f\n", upitch, uwidth);
#endif

    pitch = solve_pitch(xu->cob.outlinetype);
#if 0
    fprintf(fp, "%% pitch %.2f\n", pitch);

    fprintf(fp, "%% pitch range [%.4f .. %.4f]\n", pitch-cap, pitch+cap);
#endif

    fprintf(fp, "gsave\n");

    lx = rx = (double)x1;
    ly = ry = (double)y1;

    lastt = -1;
    i = 0;

#if 0
    for(t=0;t<=1.0;t+=s) 
#endif

    for(t=0;t<=1.0;) {
        if(t==lastt) {
            fprintf(fp, "%% OVERRUN ? t %.4f lastt %.4f\n",
                t, lastt);
            break;
        }
        lastt = t;

        _bez_pos(&rx, &ry, t,
            (double)x1, (double)y1, (double)x2, (double)y2,
            (double)x3, (double)y3, (double)x4, (double)y4);

        dx = rx - lx;
        dy = ry - ly;
    
        d  = sqrt((rx-lx)*(rx-lx)+(ry-ly)*(ry-ly));
        if(d>maxd) maxd = d;
        sumd += d;

        x = (int)rx;
        y = (int)ry;

#if 0
        fprintf(fp, "%% t   %3d %9.4f %9.2f,%-9.2f -> %9.2f,%-9.2f %9.4f\n",
            i, t, rx, ry, lx, ly, d);
#endif

#if 0
        markfdot(1, x, y);
#endif

        for(u=t;u<=t+uwidth;u+=upitch) {

            _bez_pos(&nx, &ny, u,
                (double)x1, (double)y1, (double)x2, (double)y2,
                (double)x3, (double)y3, (double)x4, (double)y4);

            dx = nx - rx;
            dy = ny - ry;
        
            d  = sqrt((nx-rx)*(nx-rx)+(ny-ry)*(ny-ry));
            if(d>maxd) maxd = d;
            sumd += d;

            x = (int)nx;
            y = (int)ny;


#if 0
            fprintf(fp, "%%   u %3d %9.4f %9.2f,%-9.2f -> %9.2f,%-9.2f %9.4f\n",
                i, u, rx, ry, lx, ly, d);
#endif

            if(d>=pitch-cap || u>=1.0) {
#if 0
                fprintf(fp, "%%  pass\n");
#endif
                break;
            }
        }

        switch(xu->cob.outlinetype) {
        case LT_DASHED:
        case LT_DOTTED:
        case LT_CHAINED:
        case LT_DOUBLECHAINED:
            act = solve_dashpart(xu->cob.outlinetype, i);
            if(act) {
                fprintf(fp, "%.2f %.2f moveto %.2f %.2f rlineto stroke\n",
                    rx, ry, dx, dy);

#if 0
                markcross(1,rx,ry);
                markcross(2,nx,ny);
#endif

            }
            break;
        case LT_CIRCLE:
        case LT_TRIANGLE:
        case LT_MOUNTAIN:
        case LT_WAVED:
        case LT_ZIGZAG:
            a = atan2(dy,dx)/rf;
            drawsym(fp, (double)rx, (double)ry, (double)a,
                (double)d, i, xu->cob.outlinetype, 
                (double)lx, (double)ly, &mx, &my);
            break;
        }

        t = u;

        i++;
        lx = rx;
        ly = ry;
    }
    fprintf(fp, "grestore\n");

    return 0;
}

int
Zepsdraw_bcurveselfarrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    double mu, mv;
    int ux, uy, px, py, qx, qy, vx, vy;
    int ik;

    ik = solve_curveself_points(xu, xns,
            &mu, &mv, &ux, &uy, &px, &py, &qx, &qy, &vx, &vy);

    fprintf(fp, "%% cs param %.2f %.2f ; %d,%d %d,%d %d,%d %d,%d\n",
        mu, mv, ux, uy, px, py, qx, qy, vx, vy);

    fprintf(fp, "gsave\n");

    if(xu->cob.markguide) {
        fprintf(fp, "%% guide\n");
        fprintf(fp, "gsave\n");
        changethick(fp, xu->cob.outlinethick);
        SLW_12(fp);
        changecolor(fp, def_guide1color);
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            ux, uy, px, py);
        markcross(def_guide1color, ux, uy);
        markcross(def_guide1color, px, py);

        changecolor(fp, def_guide2color);
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            qx, qy, vx, vy);
        markxross(def_guide2color, qx, qy);
        markxross(def_guide2color, vx, vy);

        fprintf(fp, "grestore\n");
    }

#if 0
    if(xu->cob.marknode) {
        fprintf(fp, "%% guide\n");
        fprintf(fp, "gsave\n");
        changethick(fp, xu->cob.outlinethick);
        SLW_12(fp);
        markfdot(xu->cob.outlinecolor, ux, uy);
        markwdot(xu->cob.outlinecolor, vx, vy);
        fprintf(fp, "grestore\n");
    }
#endif

    /* main body */
    changecolor(fp, xu->cob.outlinecolor);
    changethick(fp, xu->cob.outlinethick);

    if(xu->type==CMD_XCURVESELF) {
        fprintf(fp, "  newpath %d %d moveto %d %d %d %d %d %d curveto stroke\n",
            ux, uy, px, py, qx, qy, vx, vy);
    }
    else {
        if(xu->cob.outlinetype==LT_SOLID) {
            _bez_solid(fp, xu, ux, uy, px, py, qx, qy, vx, vy);
#if 0
            _bez_solid(fp, xu, xox+ux, xoy+uy, xox+px, xoy+py, xox+qx, xoy+qy, xox+vx, xoy+vy);
#endif
        }
        else {
            _bez_LT(fp, xu, ux, uy, px, py, qx, qy, vx, vy);
#if 0
            _bez_LT(fp, xu, xox+ux, xoy+uy, xox+px, xoy+py, xox+qx, xoy+qy, xox+vx, xoy+vy);
#endif
        }
    }

#if 0
    {
        qbb_t bez_bb;
        _bez_mark(&bez_bb, ux, uy, px, py, qx, qy, vx, vy);
        qbb_fprint(stdout, &bez_bb);

        fprintf(fp, "  gsave\n");
        fprintf(fp, "   0 0 1 setrgbcolor\n");
        SLW_14(fp);
        fprintf(fp, "    %d %d moveto %d %d lineto\n",
                        bez_bb.lx, bez_bb.by, bez_bb.rx, bez_bb.by);
        fprintf(fp, "    %d %d lineto %d %d lineto\n",
                        bez_bb.rx, bez_bb.ty, bez_bb.lx, bez_bb.ty);
        fprintf(fp, "    closepath stroke\n");
        fprintf(fp, "  grestore\n");
    }
#endif

    if(xu->cob.arrowheadpart & AR_BACK) {
        epsdraw_arrowhead(fp, xu->cob.arrowbackheadtype,
            (int)(mu/rf)+180, xu->cob.outlinecolor, ux, uy);
    }
    if(xu->cob.arrowheadpart & AR_FORE) {
        epsdraw_arrowhead(fp, xu->cob.arrowforeheadtype,
            (int)(mv/rf)+180, xu->cob.outlinecolor, vx, vy);
    }
#if 1
    if(xu->cob.arrowheadpart & AR_CENT) {
        double na, nx, ny;
        double nt;
        
        if(xu->cob.arrowcentheadpos>0.0) {
            nt = xu->cob.arrowcentheadpos;
        }
        else {
            nt = 0.5;
        }
        _bez_posdir(&nx, &ny, &na, nt, ux, uy, px, py, qx, qy, vx, vy);

        epsdraw_arrowhead(fp, xu->cob.arrowforeheadtype,
            (int)(na/rf), xu->cob.outlinecolor, nx, ny);
    }
#endif

    fprintf(fp, "grestore\n");

    return 0;
}

int
Zepsdraw_bcurvearrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int     ux, uy, t1x, t1y, t2x, t2y, vx, vy;
    double  mu, mv;
    int     ik;
    int     tx, ty;

#if 1
    if(!xu->cob.originalshape) {

#if 0
        markfdot(1, xu->gsx, xu->gsy);
        markcross(2, xu->sx, xu->sy);
        markfdot(1, xu->gex, xu->gey);
        markcross(2, xu->ex, xu->ey);
#endif

        ux = xu->gsx;
        uy = xu->gsy;
        vx = xu->gex;
        vy = xu->gey;
        tx = (ux+vx)/2;
        ty = (uy+vy)/2+xu->ht/2;

        mu = xu->cob.bulge*rf;
        mv = (-xu->cob.bulge+180)*rf;

        goto pos_done;
    }
#endif

    ik = solve_curve_points(xu, xns,
            &mu, &mv, &ux, &uy, &t1x, &t1y, &t2x, &t2y, &vx, &vy);

    fprintf(fp, "%% c- param %.2f %.2f %d %d %d %d %d %d %d %d\n",
        mu, mv, ux, uy, t1x, t1y, t2x, t2y, vx, vy);

    tx = t1x;
    ty = t1y;

pos_done:

    fprintf(fp, "gsave\n");

    if(xu->cob.markguide) {
        fprintf(fp, "%% guide\n");
        fprintf(fp, "gsave\n");
        changethick(fp, xu->cob.outlinethick);
        SLW_12(fp);
        changecolor(fp, def_guide1color);
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            ux, uy, tx, ty);
        markcross(def_guide1color, ux, uy);
        markcross(def_guide1color, tx, ty);

        changecolor(fp, def_guide2color);
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            tx, ty, vx, vy);
        markxross(def_guide2color, tx, ty);
        markxross(def_guide2color, vx, vy);

        fprintf(fp, "grestore\n");
    }

#if 0
    if(xu->cob.marknode) {
        fprintf(fp, "%% guide\n");
        fprintf(fp, "gsave\n");
        changethick(fp, xu->cob.outlinethick);
        SLW_12(fp);
        markfdot(xu->cob.outlinecolor, ux, uy);
        markwdot(xu->cob.outlinecolor, vx, vy);
        fprintf(fp, "grestore\n");
    }
#endif

    /* main body */
    changecolor(fp, xu->cob.outlinecolor);
    changethick(fp, xu->cob.outlinethick);
    if(xu->type==CMD_XCURVE) {
        fprintf(fp, "  newpath %d %d moveto %d %d %d %d %d %d curveto stroke\n",
            ux, uy, tx, ty, tx, ty, vx, vy);
    }
    else {
        if(xu->cob.outlinetype==LT_SOLID) {
            _bez_solid(fp, xu, ux, uy, tx, ty, tx, ty, vx, vy);
        }
        else {
            _bez_LT(fp, xu, ux, uy, tx, ty, tx, ty, vx, vy);
        }
    }

#if 0
    {
        qbb_t bez_bb;
        _bez_mark(&bez_bb, ux, uy, tx, ty, tx, ty, vx, vy);
        qbb_fprint(stdout, &bez_bb);

        fprintf(fp, "  gsave\n");
        fprintf(fp, "   0 0 1 setrgbcolor\n");
        SLW_14(fp);
        fprintf(fp, "    %d %d moveto %d %d lineto\n",
                        bez_bb.lx, bez_bb.by, bez_bb.rx, bez_bb.by);
        fprintf(fp, "    %d %d lineto %d %d lineto\n",
                        bez_bb.rx, bez_bb.ty, bez_bb.lx, bez_bb.ty);
        fprintf(fp, "    closepath stroke\n");
        fprintf(fp, "  grestore\n");
    }
#endif

body_done:

    if(xu->cob.arrowheadpart & AR_BACK) {
        epsdraw_arrowhead(fp, xu->cob.arrowbackheadtype,
            (int)(mu/rf)+180, xu->cob.outlinecolor, ux, uy);
    }
    if(xu->cob.arrowheadpart & AR_FORE) {
        epsdraw_arrowhead(fp, xu->cob.arrowforeheadtype,
            (int)(mv/rf)+180, xu->cob.outlinecolor, vx, vy);
    }

#if 1
    if(xu->cob.arrowheadpart & AR_CENT) {
        double na, nx, ny;
        double nt;
        
        if(xu->cob.arrowcentheadpos>0.0) {
            nt = xu->cob.arrowcentheadpos;
        }
        else {
            nt = 0.5;
        }
        _bez_posdir(&nx, &ny, &na, nt, ux, uy, tx, ty, tx, ty, vx, vy);

        epsdraw_arrowhead(fp, xu->cob.arrowforeheadtype,
            (int)(na/rf), xu->cob.outlinecolor, nx, ny);
    }
#endif

    fprintf(fp, "grestore\n");

    return 0;
}


int
Zepsdraw_ulinearrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int r;
    int aw, ah;
P;
#if 1
Echo("%s: enter\n", __func__);
#endif

    if(!xu->cob.originalshape) {
P;
        path_regline(xu->cob.segar, xu->csx, xu->csy, xu->cex, xu->cey);
    }

    if(xu->type==CMD_ULINE) {
P;
        fprintf(fp, " %% fill color %d hatch %d\n",
            xu->cob.fillcolor, xu->cob.fillhatch);
        if(xu->cob.fillhatch != HT_NONE && xu->cob.fillcolor>=0) {
            fprintf(fp, " gsave\n");

            if(xu->cob.imargin>0) {
                aw = xu->wd - xu->cob.imargin*2;
                ah = xu->ht - xu->cob.imargin*2;
            }
            else {
                aw = xu->wd;
                ah = xu->ht;
            }

            changecolor(fp, xu->cob.fillcolor);
            r = drawpathN(fp, ydir, xox, xoy, xu, xns);
            fprintf(fp, "  clip\n");
#if 0
            fprintf(fp, " %% xox %d xoy %d; ydir %d\n", xox, xoy, ydir);
            fprintf(fp, " %% wd %d ht %d\n", xu->wd, xu->ht);
            fprintf(fp, " %% bb %d %d %d %d\n", xu->lx, xu->by, xu->rx, xu->ty);
            fprintf(fp, " %% x,y     %7d %7d\n", xu->x, xu->y);
            fprintf(fp, " %% ox,oy   %7d %7d\n", xu->ox, xu->oy);
            fprintf(fp, " %% gx,gy   %7d %7d\n", xu->gx, xu->gy);
#endif

            /*** BACK ***/

            fprintf(fp, "  gsave\n");
            changethick(fp, xu->cob.hatchthick);
            fprintf(fp, "  %d %d translate %% hcnt\n", xu->gx, xu->gy);
#if 0
            /* position mark for debug */
            fprintf(fp, "  gsave newpath 0 0 %d 0 360 arc fill grestore\n", objunit/20);
#endif
            epsdraw_hatch(fp, aw, ah, xu->cob.backcolor,
                xu->cob.backhatch, xu->cob.hatchpitch);
            fprintf(fp, "  grestore\n");

            /*** FILL ***/

            changethick(fp, xu->cob.hatchthick);
            fprintf(fp, "  %d %d translate %% hcnt\n", xu->gx, xu->gy);
#if 0
            /* position mark for debug */
            fprintf(fp, "  gsave newpath 0 0 %d 0 360 arc fill grestore\n", objunit/20);
#endif
            epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor,
                xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, " grestore\n");
        }
        else {
P;
            fprintf(fp, " %% no-fill\n");
        }
    }
#if 0
    else {
        fprintf(stderr, "%s: oid %d it is not ULINE\n", __func__, xu->oid);
    }
#endif

    /* OUTLINE */
P;
    fprintf(fp, " gsave\n");
    changecolor(fp, xu->cob.outlinecolor);
    r = drawpath_LT(fp, ydir, xox, xoy, xu, xns);
    fprintf(fp, " grestore\n");

P;
    return r;
}


#define X_RLINE     (0)
#define X_VLINE     (1)
#define X_HLINE     (2)

/*
    line is expressed following:
            y = k *x + u
    this function solves its k and u.
*/

int
lineparam(double *k, double *u, double x1, double y1, double x2, double y2)
{
    if(x2-x1==0) {
        return X_VLINE;
    }
    if(y2-y1==0) {
        return X_HLINE;
    }

    *k = (y2-y1)/(x2-x1);
    *u = y2 - *k*x2;

    return 0;
}


#define CP_CROSS        (10)
#define CP_INCROSS      (11)
#define CP_OUTCROSS     (12)
#define CP_MANYCROSS    (13)
#define CP_PARA         (20)

int
linecrosspoint(double *rcx, double *rcy,
    int L1x1, int L1y1, int L1x2, int L1y2,
    int L2x1, int L2y1, int L2x2, int L2y2)
{
    int rv;
    int ik1, ik2;
    double th1, th2;
    double a, b, c, d;

    rv = -1;

#if 0
    fprintf(stderr, "%s: %d %d %d %d vs %d %d %d %d\n",
        __func__, L1x1, L1y1, L1x2, L1y2, L2x1, L2y1, L2x2, L2y2);
#endif
    
#if 0
    th1 = (int)(atan2((L1y2-L1y1),(L1x2-L1x1))/rf);
    th2 = (int)(atan2((L2y2-L2y1),(L2x2-L2x1))/rf);

    fprintf(stderr, "\t th1 %.4f th2 %.4f\n", th1, th2);
#endif

    ik1 = lineparam(&a, &b, (double)L1x1, (double)L1y1, (double)L1x2, (double)L1y2);
    ik2 = lineparam(&c, &d, (double)L2x1, (double)L2y1, (double)L2x2, (double)L2y2);

#if 0
    fprintf(stderr, "\t ik1 %d, ik2 %d\n", ik1, ik2);
    fprintf(stderr, "\t a %.3f b %.3f c %.3f d %.3f\n", a, b, c, d);
#endif

    if(ik1==X_RLINE && ik2==X_RLINE) {
        goto normal;
    }

    if(ik1==X_RLINE) {
        if(ik2==X_VLINE) {
            *rcx = L2x1;
            *rcy = a*L2x1+b;
            rv = CP_CROSS;
            goto out;
        }
        else
        if(ik2==X_HLINE) {
            *rcy = L2y1;
            *rcx = (L2y1-b)/a;
            rv = CP_CROSS;
            goto out;
        }
    }
    else
    if(ik1==X_VLINE) {
        if(ik2==X_RLINE) {
            *rcx = L1x1;
            *rcy = c*L1x1+d;
            rv = CP_CROSS;
            goto out;
        }
        else
        if(ik2==X_HLINE) {
            *rcx = L1x1;
            *rcy = L2y1;
            rv = CP_CROSS;
            goto out;
        }
    }
    else
    if(ik1==X_HLINE) {
        if(ik2==X_RLINE) {
            *rcy = L1y1;
            *rcx = (L1y1-d)/c;
            rv = CP_CROSS;
            goto out;
        }
        else
        if(ik2==X_VLINE) {
            *rcy = L1y1;
            *rcx = L2x1;
            rv = CP_CROSS;
            goto out;
        }
    }


    goto out;

normal:
    *rcx = (int)( (d-b)/(a-c) );
    *rcy = a*(*rcx)+b;
    rv = CP_CROSS;

out:
#if 0
    fprintf(stderr, "\t rv %d, rcx %.3f, rcy %.3f\n", rv, *rcx, *rcy);
#endif

    return rv;
}


int
Zepsdraw_wlinearrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int i;
    int x0, y0;
    int x1, y1, x2, y2;
    seg *s;
    int cdir;

    int ap, fh, bh;

    int j;
    seg *Lsegs, *Rsegs;
    seg *Lpt, *Rpt;

    int af1_x, af1_y, af2_x, af2_y, afc_x, afc_y;
    int ab1_x, ab1_y, ab2_x, ab2_y, abc_x, abc_y;


    Lsegs = (seg*)malloc(sizeof(seg)*xu->cob.segar->use);
    if(!Lsegs) {
exit(9);
    }
    memset(Lsegs, 0, sizeof(seg)*xu->cob.segar->use);
    Rsegs = (seg*)malloc(sizeof(seg)*xu->cob.segar->use);
    if(!Rsegs) {
exit(10);
    }
    memset(Rsegs, 0, sizeof(seg)*xu->cob.segar->use);

    Lpt = (seg*)malloc(sizeof(seg)*(xu->cob.segar->use+1));
    if(!Lpt) {
exit(11);
    }
    memset(Lpt, 0, sizeof(seg)*(xu->cob.segar->use+1));
    Rpt = (seg*)malloc(sizeof(seg)*(xu->cob.segar->use+1));
    if(!Rpt) {
exit(12);
    }
    memset(Rpt, 0, sizeof(seg)*(xu->cob.segar->use+1));


    if(bbox_mode) {
fprintf(fp, "gsave\n");
changebbox(fp);
fprintf(fp, "  %d %d translate\n", xu->glx, xu->gby);
fprintf(fp, "  0 0 moveto %d 0 lineto %d %d lineto 0 %d lineto closepath stroke\n", xu->wd, xu->wd, xu->ht, xu->ht);
fprintf(fp, "grestore\n");
    }

    if(!xu->cob.originalshape) {
        x1 = xox+xu->csx;
        y1 = xoy+xu->csy;
        x2 = xox+xu->cex;
        y2 = xoy+xu->cey;

    fprintf(fp, " gsave %% %s\n", __func__);
        epsdraw_segwlinearrow(fp, ydir, xox, xoy, x1, y1, x2, y2, xu, xns);
    fprintf(fp, " grestore %% %s\n", __func__);
        goto out;
    }

    if(xu->cob.segar && xu->cob.segar->use>0) {
    }
    else {
        goto out;
    }

    fprintf(fp, " gsave %% %s\n", __func__);

Echo("    segar.use %d\n", xu->cob.segar->use);

        x0 = x1 = xox+xu->csx;
        y0 = y1 = xoy+xu->csy;
Echo("    csx,csy %d,%d\n", xu->csx, xu->csy);
Echo("    x1,y1 %d,%d\n", x1, y1);

#if 0
    fprintf(fp, "gsave\n");
        fprintf(fp, "currentlinewidth 5 mul setlinewidth\n");
#endif

    for(i=0;i<xu->cob.segar->use;i++) {
        s = (seg*)xu->cob.segar->slot[i];
        if(!s) {
            continue;
        }

        x2 = x1+s->x1;
        y2 = y1+s->y1;

        cdir = (int)atan2(y2-y1,x2-x1);

        Echo("    part seg %d: %d,%d : %d,%d - %d,%d cdir %d\n",
            i, s->x1, s->y1, x1, y1, x2, y2, cdir);
#if 0
        {
        ap = xu->cob.arrowheadpart;
        bh = xu->cob.arrowbackheadtype;
        fh = xu->cob.arrowforeheadtype;

        if(!xu->cob.arrowevery) {
            if(i==0) {
                xu->cob.arrowheadpart &= ~AR_FORE;
            }
            else 
            if(i==xu->cob.segar->use-1) {
                xu->cob.arrowheadpart &= ~AR_BACK;
            }
            else {
                xu->cob.arrowheadpart = 0;
            }
        }

        epsdraw_segwlinearrow(fp, cdir, xox, xoy, x1, y1, x2, y2, xu, xns);

        xu->cob.arrowheadpart       = ap;
        xu->cob.arrowbackheadtype   = bh;
        xu->cob.arrowforeheadtype   = fh;
        }
#endif


{
        double xdir;
        int bx, by;
        int dx, dy;
        int s1x, s1y;
        int s2x, s2y;
        int e1x, e1y;
        int e2x, e2y;
        int wlt;

        wlt = xu->cob.wlinethick;

        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

        bx = def_warrowsize*cos((xdir+180)*rf);
        by = def_warrowsize*sin((xdir+180)*rf);

        dx = wlt*cos((xdir+90)*rf);
        dy = wlt*sin((xdir+90)*rf);

        s1x = x1 + dx;
        s1y = y1 + dy;
        e1x = x2 + dx;
        e1y = y2 + dy;

        s2x = x1 - dx;
        s2y = y1 - dy;
        e2x = x2 - dx;
        e2y = y2 - dy;

        if(i==xu->cob.segar->use-1 && xu->cob.arrowheadpart & AR_FORE) {
            e1x = x2 + bx + dx;
            e1y = y2 + by + dy;
            e2x = x2 + bx - dx;
            e2y = y2 + by - dy;

            af1_x = x2 + bx + dx + dx;
            af1_y = y2 + by + dy + dy;
            af2_x = x2 + bx - dx - dx;
            af2_y = y2 + by - dy - dy;
            afc_x = x2;
            afc_y = y2;
        }
        if(i==0 && xu->cob.arrowheadpart & AR_BACK) {
            bx = def_warrowsize*cos((xdir)*rf);
            by = def_warrowsize*sin((xdir)*rf);

            s1x = x1 + bx + dx;
            s1y = y1 + by + dy;
            s2x = x1 + bx - dx;
            s2y = y1 + by - dy;

            ab1_x = x1 + bx + dx + dx;
            ab1_y = y1 + by + dy + dy;
            ab2_x = x1 + bx - dx - dx;
            ab2_y = y1 + by - dy - dy;
            abc_x = x1;
            abc_y = y1;
        }

#if 0
        fprintf(fp, "1 0.7 0.7 setrgbcolor\n");
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            s1x, s1y, e1x, e1y);
        fprintf(fp, "0.7 0.7 1 setrgbcolor\n");
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            s2x, s2y, e2x, e2y);
#endif

        Lsegs[i].x1 = s1x;
        Lsegs[i].y1 = s1y;

        Rsegs[i].x1 = s2x;
        Rsegs[i].y1 = s2y;

        Lsegs[i].x2 = e1x;
        Lsegs[i].y2 = e1y;

        Rsegs[i].x2 = e2x;
        Rsegs[i].y2 = e2y;

}
    

        x1 = x2;
        y1 = y2;
    }

#if 0
    fprintf(fp, "grestore\n");
#endif

#if 0
    /* Lsegs */
    fprintf(fp, "1 0 0 setrgbcolor\n");
        x1 = Lsegs[0].x1;   
        y1 = Lsegs[0].y1;   
        x2 = Lsegs[0].x2;   
        y2 = Lsegs[0].y2;   
    fprintf(fp, "%d %d moveto\n", x1, y1);
    for(i=0;i<xu->cob.segar->use;i++) {
        x1 = Lsegs[i].x1;   
        y1 = Lsegs[i].y1;   
        x2 = Lsegs[i].x2;   
        y2 = Lsegs[i].y2;   
    
        fprintf(fp, "%d %d lineto %d %d lineto\n",
            x1, y1, x2, y2);
    }

#if 0
    fprintf(fp, "stroke\n");

    /* Rsegs */
    fprintf(fp, "0 0 1 setrgbcolor\n");
    j = xu->cob.segar->use-1;
        x1 = Rsegs[j].x1;   
        y1 = Rsegs[j].y1;   
        x2 = Rsegs[j].x2;   
        y2 = Rsegs[j].y2;   
    fprintf(fp, "%d %d moveto\n", x2, y2);
#endif
    for(i=0;i<xu->cob.segar->use;i++) {
        j = xu->cob.segar->use-1-i;
        x1 = Rsegs[j].x1;   
        y1 = Rsegs[j].y1;   
        x2 = Rsegs[j].x2;   
        y2 = Rsegs[j].y2;   
    
        fprintf(fp, "%d %d lineto %d %d lineto\n",
            x2, y2, x1, y1);
    }
    fprintf(fp, "stroke\n");

#if 0
    fprintf(fp, "closepath fill\n");
#endif

#endif


    /* Lsegs */
#if 0
    fprintf(fp, "1 0 0 setrgbcolor\n");
        x1 = Lsegs[0].x1;   
        y1 = Lsegs[0].y1;   
        x2 = Lsegs[0].x2;   
        y2 = Lsegs[0].y2;   
    fprintf(fp, "%d %d moveto\n", x1, y1);
#endif

    Lpt[0].x1 = Lsegs[0].x1;
    Lpt[0].y1 = Lsegs[0].y1;

    for(i=0;i<xu->cob.segar->use-1;i++) {
        int ik;
        double qx, qy;

        ik = linecrosspoint(&qx, &qy,
            Lsegs[i].x1, Lsegs[i].y1, Lsegs[i].x2, Lsegs[i].y2,
            Lsegs[i+1].x1, Lsegs[i+1].y1, Lsegs[i+1].x2, Lsegs[i+1].y2);
#if 0
        fprintf(fp, "newpath %.3f %.3f %d 0 360 arc stroke\n",
            qx, qy, objunit/10);    
#endif
#if 0
        fprintf(stderr, "cross? %d\n", ik);
#endif

        if(ik>0) {
            Lpt[i+1].x1 = qx;
            Lpt[i+1].y1 = qy;
        }
    }
    Lpt[i+1].x1 = Lsegs[xu->cob.segar->use-1].x2;
    Lpt[i+1].y1 = Lsegs[xu->cob.segar->use-1].y2;



    /* Rsegs */
#if 0
    fprintf(fp, "1 0 0 setrgbcolor\n");
        x1 = Rsegs[0].x1;   
        y1 = Rsegs[0].y1;   
        x2 = Rsegs[0].x2;   
        y2 = Rsegs[0].y2;   
    fprintf(fp, "%d %d moveto\n", x1, y1);
#endif

    Rpt[0].x1 = Rsegs[0].x1;
    Rpt[0].y1 = Rsegs[0].y1;

    for(i=0;i<xu->cob.segar->use-1;i++) {
        int ik;
        double qx, qy;

        ik = linecrosspoint(&qx, &qy,
            Rsegs[i].x1, Rsegs[i].y1, Rsegs[i].x2, Rsegs[i].y2,
            Rsegs[i+1].x1, Rsegs[i+1].y1, Rsegs[i+1].x2, Rsegs[i+1].y2);
#if 0
        fprintf(fp, "newpath %.3f %.3f %d 0 360 arc stroke\n",
            qx, qy, objunit/10);    
#endif
#if 0
        fprintf(stderr, "cross? %d\n", ik);
#endif

        if(ik>0) {
            Rpt[i+1].x1 = qx;
            Rpt[i+1].y1 = qy;
        }
    }
    Rpt[i+1].x1 = Rsegs[xu->cob.segar->use-1].x2;
    Rpt[i+1].y1 = Rsegs[xu->cob.segar->use-1].y2;


#if 0
    for(i=0;i<=xu->cob.segar->use;i++) {
        fprintf(stderr, "%3d: Lpt %7d %7d\n", i, Lpt[i].x1, Lpt[i].y1);
    }
    for(i=0;i<=xu->cob.segar->use;i++) {
        fprintf(stderr, "%3d: Rpt %7d %7d\n", i, Rpt[i].x1, Rpt[i].y1);
    }
#endif

 if(xu->cob.fillcolor>=0) {
    fprintf(fp, "gsave\n");
    changecolor(fp, xu->cob.fillcolor);
    changethick(fp, xu->cob.hatchthick);

    fprintf(fp, "%d %d moveto\n", Lpt[0].x1, Lpt[0].y1);
    for(i=1;i<=xu->cob.segar->use;i++) {
        fprintf(fp, "%d %d lineto\n", Lpt[i].x1, Lpt[i].y1);
    }
    if(xu->cob.arrowheadpart & AR_FORE) {
        fprintf(fp, "%d %d lineto\n", af1_x, af1_y);
        fprintf(fp, "%d %d lineto\n", afc_x, afc_y);
        fprintf(fp, "%d %d lineto\n", af2_x, af2_y);
    }
    j = xu->cob.segar->use;
    fprintf(fp, "%d %d lineto\n", Rpt[j].x1, Rpt[j].y1);
    for(i=1;i<=xu->cob.segar->use;i++) {
        j = xu->cob.segar->use-i;
        fprintf(fp, "%d %d lineto\n", Rpt[j].x1, Rpt[j].y1);
    }
    if(xu->cob.arrowheadpart & AR_BACK) {
        fprintf(fp, "%d %d lineto\n", ab2_x, ab2_y);
        fprintf(fp, "%d %d lineto\n", abc_x, abc_y);
        fprintf(fp, "%d %d lineto\n", ab1_x, ab1_y);
        fprintf(fp, "closepath\n");
    }
#if 0
    fprintf(fp, "stroke\n");
#endif
    fprintf(fp, "clip\n");

#if 1
 {
    qbb_t *qbb;
    int    ccx, ccy;
    int    cw, ch;

    qbb = qbb_new();
#if 0
    qbb_fprint(stderr, qbb);
#endif

    for(i=0;i<=xu->cob.segar->use;i++) {
        qbb_mark(qbb, Lpt[i].x1, Lpt[i].y1);
    }
    for(i=0;i<=xu->cob.segar->use;i++) {
        qbb_mark(qbb, Rpt[i].x1, Rpt[i].y1);
    }
    if(xu->cob.arrowheadpart & AR_FORE) {
        qbb_mark(qbb, af1_x, af1_y);
        qbb_mark(qbb, af2_x, af2_y);
        qbb_mark(qbb, afc_x, afc_y);
    }
    if(xu->cob.arrowheadpart & AR_BACK) {
        qbb_mark(qbb, ab1_x, ab1_y);
        qbb_mark(qbb, ab2_x, ab2_y);
        qbb_mark(qbb, abc_x, abc_y);
    }
    
#if 0
    qbb_fprint(stderr, qbb);
#endif
    qbb_getcenter(qbb, &ccx, &ccy);
    qbb_getsize(qbb, &cw, &ch);

    fprintf(fp, "%d %d translate\n", ccx, ccy);

    epsdraw_hatch(fp, cw, ch, xu->cob.fillcolor,
        xu->cob.fillhatch, xu->cob.hatchpitch);
 }
#endif

    fprintf(fp, "grestore\n");
  }


 if(xu->cob.outlinecolor>=0) {
    fprintf(fp, "gsave\n");
    changecolor(fp, xu->cob.outlinecolor);
    changethick(fp, xu->cob.outlinethick);
    fprintf(fp, "%d %d moveto\n", Lpt[0].x1, Lpt[0].y1);
    for(i=1;i<=xu->cob.segar->use;i++) {
        fprintf(fp, "%d %d lineto\n", Lpt[i].x1, Lpt[i].y1);
    }
    if(xu->cob.arrowheadpart & AR_FORE) {
        fprintf(fp, "%d %d lineto\n", af1_x, af1_y);
        fprintf(fp, "%d %d lineto\n", afc_x, afc_y);
        fprintf(fp, "%d %d lineto\n", af2_x, af2_y);
    }
    j = xu->cob.segar->use;
    fprintf(fp, "%d %d lineto\n", Rpt[j].x1, Rpt[j].y1);
    for(i=1;i<=xu->cob.segar->use;i++) {
        j = xu->cob.segar->use-i;
        fprintf(fp, "%d %d lineto\n", Rpt[j].x1, Rpt[j].y1);
    }
    if(xu->cob.arrowheadpart & AR_BACK) {
        fprintf(fp, "%d %d lineto\n", ab2_x, ab2_y);
        fprintf(fp, "%d %d lineto\n", abc_x, abc_y);
        fprintf(fp, "%d %d lineto\n", ab1_x, ab1_y);
        fprintf(fp, "closepath\n");
    }
    fprintf(fp, "stroke\n");
 }


    if(xu->type==CMD_ULINE) {
#if 0
        epsdraw_segwlinearrow(fp, ydir, xox, xoy, x0, y0, x2, y2, xu, xns);
#endif
    }

    fprintf(fp, " grestore %% %s\n", __func__);

out:
    return 0;
}

int
epsdraw_blinearrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int i;
    int x0, y0;
    int x1, y1, x2, y2;
    seg *s;
    int cdir;

    int ap, fh, bh;

P;
PP;


    if(!xu->cob.originalshape) {
        x1 = xox+xu->csx;
        y1 = xoy+xu->csy;
        x2 = xox+xu->cex;
        y2 = xoy+xu->cey;

P;
PP;
        fprintf(fp, " gsave %% %s\n", __func__);
#if 0
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x1, y1, x2, y2, xu, xns);
#endif
        epsdraw_segblinearrow(fp, cdir, xox, xoy, x1, y1, x2, y2, xu, xns);
        fprintf(fp, " grestore %% %s\n", __func__);
        goto out;
    }


    if(xu->cob.segar && xu->cob.segar->use>0) {
    }
    else {
        goto out;
    }

    fprintf(fp, " gsave %% %s\n", __func__);

    Echo("    segar.use %d\n", xu->cob.segar->use);

    x0 = x1 = xox+xu->csx;
    y0 = y1 = xoy+xu->csy;

    Echo("    csx,csy %d,%d\n", xu->csx, xu->csy);
    Echo("    x1,y1 %d,%d\n", x1, y1);

    if(xu->type==CMD_ULINE) {
#if 1
        x0 = x1 = xox+xu->cx+xu->cox;
        y0 = y1 = xoy+xu->cy+xu->coy;
#endif

        Echo("ULINE\n");
        Echo("    xox,xoy %6d,%-6d\n", xox,     xoy);
        Echo("    cx,cy   %6d,%-6d\n", xu->cx,  xu->cy);
        Echo("    csx,csy %6d,%-6d\n", xu->csx, xu->csy);
        Echo("    cox,coy %6d,%-6d\n", xu->cox, xu->coy);
        Echo("    x1,y1 %d,%d\n", x1, y1);

#if 0
        MC(0, xox+xu->cx,               xoy+xu->cy);
        MC(1, xox+xu->csx,               xoy+xu->csy);
        MC(4, xox+xu->cx+xu->cox,       xoy+xu->cy+xu->coy);
#endif

    }

    for(i=0;i<xu->cob.segar->use;i++) {
        s = (seg*)xu->cob.segar->slot[i];
        if(!s) {
            continue;
        }

        if(s->ptype==OA_ARC) {
            int arcx, arcy;

#if 0
    fprintf(fp, "%%  cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
    fprintf(stderr, "%% b cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif

            arcx = x1 + s->rad*cos((cdir+90)*rf);
            arcy = y1 + s->rad*sin((cdir+90)*rf);

#if 0
    fprintf(fp, "%%  arcx %d arcy %d\n", arcx, arcy);
#endif

            x2 = arcx + s->rad*cos((cdir+s->ang-90)*rf);
            y2 = arcy + s->rad*sin((cdir+s->ang-90)*rf);
            
            fprintf(fp, " %d %d %d %d %d arc stroke %% seg-arc\n",
                arcx, arcy, s->rad, cdir-90, cdir-90+s->ang);

#if 1
        {
            int tx, ty; 
            int a;
            fprintf(fp, "gsave\n");
            Echo("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
            for(a=cdir-90;a<=cdir-90+s->ang;a++) {
                if(a==cdir-90||a==cdir-90+s->ang||a%90==0) {
                    tx = arcx + s->rad*cos((a)*rf);
                    ty = arcy + s->rad*sin((a)*rf);
                    Echo("  arc  %d tx,ty %d,%d\n", a, tx, ty);
                    if(draft_mode) {
                        MC(4, tx, ty);
                    }
                }
            }
            fprintf(fp, "grestore\n");
        }
#endif

            if(draft_mode) {
                fprintf(fp, "gsave\n");
                changedraft(fp);
                fprintf(fp, " currentlinewidth 2 div setlinewidth\n");
                fprintf(fp,
                    "  %d %d moveto %d %d lineto %d %d lineto stroke\n",
                        x1, y1, arcx, arcy, x2, y2);
                fprintf(fp, "grestore\n");
            }


            cdir += s->ang;
fprintf(fp, "%% a cdir %d\n", cdir);

            goto next;
        }
        else
        if(s->ptype==OA_ARCN) {
            int arcx, arcy;

#if 0
    fprintf(fp, "%%  cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
    fprintf(stderr, "%% b cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif

            arcx = x1 + s->rad*cos((cdir-90)*rf);
            arcy = y1 + s->rad*sin((cdir-90)*rf);

#if 0
    fprintf(fp, "%%  arcx %d arcy %d\n", arcx, arcy);
#endif

            x2 = arcx + s->rad*cos((cdir-s->ang+90)*rf);
            y2 = arcy + s->rad*sin((cdir-s->ang+90)*rf);
            
            fprintf(fp, " %d %d %d %d %d arcn stroke %% seg-arcn\n",
                arcx, arcy, s->rad, cdir+90, cdir+90-s->ang);

#if 1
        {
            int tx, ty; 
            int a;
            fprintf(fp, "gsave\n");
            Echo("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
            for(a=cdir-90;a<=cdir-90+s->ang;a++) {
                if(a==cdir-90||a==cdir-90+s->ang||a%90==0) {
                    tx = arcx + s->rad*cos((a)*rf);
                    ty = arcy + s->rad*sin((a)*rf);
                    Echo("  arc  %d tx,ty %d,%d\n", a, tx, ty);
                    if(draft_mode) {
                        MC(4, tx, ty);
                    }
                }
            }
            fprintf(fp, "grestore\n");
        }
#endif

            if(draft_mode) {
                fprintf(fp, "gsave\n");
                changedraft(fp);
                fprintf(fp, " currentlinewidth 2 div setlinewidth\n");
                fprintf(fp,
                    "  %d %d moveto %d %d lineto %d %d lineto stroke\n",
                        x1, y1, arcx, arcy, x2, y2);
                fprintf(fp, "grestore\n");
            }

            cdir -= s->ang;
fprintf(fp, "%% a cdir %d\n", cdir);

            goto next;
        }

/* XXX */
        else {
            Echo("unsupported type %d\n", s->ptype);
            if(s->ftflag & COORD_FROM) {
P;
            Echo("    part seg %d: ftflag %d : %d,%d\n",
                i, s->ftflag, s->x1, s->y1);
                
                /* skip */
                continue;
            }
            else
            if(s->ftflag & COORD_TO) {
P;
                x1 = s->x1 + xox;
                y2 = s->y1 + xoy;
                goto coord_done;
            }
        }

        x2 = x1+s->x1;
        y2 = y1+s->y1;

coord_done:

        cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#if 0
fprintf(stderr, "%% m atan2 %.3f\n", atan2(y2-y1,x2-x1)/rf);
fprintf(stderr, "%% m cdir %d\n", cdir);
#endif

        Echo("    part seg %d: ftflag %d : %d,%d : %d,%d - %d,%d cdir %d\n",
            i, s->ftflag, s->x1, s->y1, x1, y1, x2, y2, cdir);
        {
        ap = xu->cob.arrowheadpart;
        bh = xu->cob.arrowbackheadtype;
        fh = xu->cob.arrowforeheadtype;

        if(!xu->cob.arrowevery) {
            if(i==0) {
                xu->cob.arrowheadpart &= ~AR_FORE;
            }
            else 
            if(i==xu->cob.segar->use-1) {
                xu->cob.arrowheadpart &= ~AR_BACK;
            }
            else {
                xu->cob.arrowheadpart = 0;
            }
        }

#if 0
        epsdraw_seglinearrow(fp, cdir, xox, xoy, x1, y1, x2, y2, xu, xns);
#endif
P;
PP;
        epsdraw_segblinearrow(fp, cdir, xox, xoy, x1, y1, x2, y2, xu, xns);

        xu->cob.arrowheadpart       = ap;
        xu->cob.arrowbackheadtype   = bh;
        xu->cob.arrowforeheadtype   = fh;
        }
        
next:
fprintf(fp, "%% a cdir %4d : %s\n", cdir, __func__);
        x1 = x2;
        y1 = y2;
    }

    if(xu->type==CMD_ULINE) {
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x2, y2, x0, y0, xu, xns);
    }

    fprintf(fp, " grestore %% %s\n", __func__);
out:
    return 0;
}

/*
 *      W -aw/2 - 0 - aw/2
 *      H -ah/2 - 0 - ah/2
 */
int
epsdraw_hatch(FILE *fp, int aw, int ah, int hc, int hty, int hp)
{
    int x1, y1, x2, y2;
    int x3, y3;
    int dx, dy;
    int c;

    Echo("%s: aw %d ah %d hc %d hty %d\n", __func__,
        aw, ah, hc, hty);

#if 0
    fprintf(fp, "%% %s start aw %d ah %d hc %d hty %d\n",
        __func__, aw, ah, hc, hty);
#endif

    changecolor(fp, hc);

    fprintf(fp, "      newpath\n");

    switch(hty) {

    case HT_HZIGZAG:
#if 0
        for(y1=-ah*3/5;y1<ah*3/5;y1+=hp) {
            c = 0;
            for(x1=-aw*3/5;x1<aw*3/5;x1+=hp) {
                if(c%2==0) {
                    y2 = y1-hp/2;
                    y3 = y1;
                }
                else {
                    y3 = y1-hp/2;
                    y2 = y1;
                }
                x2 = x1;
                x3 = x1+hp;
                fprintf(fp, " %d %d moveto %d %d lineto stroke\n",
                    x2, y2, x3, y3);
                c++;
            }
        }
        break;
#endif
#if 1
        for(y1=-ah*3/5;y1<ah*3/5;y1+=hp) {
            c = 0;
            for(x1=-aw*3/5;x1<aw*3/5;x1+=hp) {
                if(c%2==0) {
                    y2 = y1-hp/2;
                    y3 = y1;
                }
                else {
                    y3 = y1-hp/2;
                    y2 = y1;
                }
                x2 = x1;
                x3 = x1+hp;
                if(c==0) {
                    fprintf(fp, " %d %d moveto\n", x2, y2);
                }
                fprintf(fp, " %d %d lineto\n", x3, y3);
                c++;
            }
            fprintf(fp, "stroke\n");
        }
        break;
#endif

    case HT_VZIGZAG:
#if 0
        for(x1=-aw*3/5;x1<aw*3/5;x1+=hp) {
            c = 0;
            for(y1=-ah*3/5;y1<ah*3/5;y1+=hp) {
                if(c%2==0) {
                    x2 = x1-hp/2;
                    x3 = x1;
                }
                else {
                    x3 = x1-hp/2;
                    x2 = x1;
                }
                y2 = y1;
                y3 = y1+hp;
                fprintf(fp, " %d %d moveto %d %d lineto stroke\n",
                    x2, y2, x3, y3);
                c++;
            }
        }
#endif
#if 1
        for(x1=-aw*3/5;x1<aw*3/5;x1+=hp) {
            c = 0;
            for(y1=-ah*3/5;y1<ah*3/5;y1+=hp) {
                if(c%2==0) {
                    x2 = x1-hp/2;
                    x3 = x1;
                }
                else {
                    x3 = x1-hp/2;
                    x2 = x1;
                }
                y2 = y1;
                y3 = y1+hp;
                if(c==0) {
                    fprintf(fp, " %d %d moveto\n", x2, y2);
                }
                fprintf(fp, " %d %d lineto\n", x3, y3);
                c++;
            }
            fprintf(fp, "stroke\n");
        }
#endif
        break;

    case HT_HLINED:
        x1 = -2*aw;
        x2 =  2*aw;
        for(y1=-2*ah;y1<2*ah;y1+=hp) {
            y2 = y1;
            fprintf(fp, "      %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        break;

    case HT_VLINED:
        y1 = -2*aw;
        y2 =  2*aw;
        for(x1=-2*aw;x1<2*aw;x1+=hp) {
            x2=x1;
            fprintf(fp, "      %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        break;
    
    case HT_CROSSED:
        x1 = -2*aw;
        x2 =  2*aw;
        for(y1=-2*ah;y1<2*ah;y1+=hp) {
            y2 = y1;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        y1 = -2*aw;
        y2 =  2*aw;
        for(x1=-2*aw;x1<2*aw;x1+=hp) {
            x2=x1;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        break;
    
    case HT_XCROSSED:
#if 0
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-2*aw;x1<aw*2+ah*2;x1+=hp) {
            x2 = x1-ah*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }

        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-aw*6/10;x1<aw*6/10+ah*2;x1+=hp) {
            x2 = x1-ah*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x2, y1, x1, y2);
        }
#endif
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-2*aw;x1<aw*2+ah*2;x1+=hp) {
            x2 = x1-ah*6/10*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-aw*6/10;x1<aw*6/10+ah*2;x1+=hp) {
            x2 = x1-ah*6/10*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x2, y1, x1, y2);
        }

        break;

    case HT_BACKSLASHED:
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-2*aw;x1<aw*2+ah*2;x1+=hp) {
            x2 = x1-ah*6/10*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        break;

    case HT_DOTTED:
        for(x1=-aw;x1<aw;x1+=hp) {
            for(y1=-ah;y1<ah;y1+=hp) {
                x2=x1+def_hatchthick;
                y2=y1;
                fprintf(fp, "      %d %d moveto %d %d lineto stroke\n",
                    x1, y1, x2, y2);
            }
        }
        break;

    case HT_SPARSEDOTTED:
        for(x1=-aw;x1<aw;x1+=hp*2) {
            for(y1=-ah;y1<ah;y1+=hp*2) {
                x2=x1+def_hatchthick;
                y2=y1;
                fprintf(fp, "      %d %d moveto %d %d lineto stroke\n",
                    x1, y1, x2, y2);
            }
        }
        break;

    case HT_CHECKED:
      {
            int u, v;
        u = 0;
        for(x1=-aw;x1<aw;x1+=hp) {
            v = 0;
            for(y1=-ah;y1<ah;y1+=hp) {
                
                if((u%2==0 && v%2==0) || (u%2==1 && v%2==1)) {
                    fprintf(fp, "      %d %d moveto %d 0 rlineto 0 %d rlineto %d 0 rlineto closepath fill\n",
                        x1, y1, hp, hp, -hp);
                }
                v++;
            }
            u++;
        }
      }
        break;
    

    case HT_BIGCHECKED:
      {
            int u, v;
        u = 0;
        for(x1=-aw;x1<aw;x1+=hp*2) {
            v = 0;
            for(y1=-ah;y1<ah;y1+=hp*2) {
                
                if((u%2==0 && v%2==0) || (u%2==1 && v%2==1)) {
                    fprintf(fp, "      %d %d moveto %d 0 rlineto 0 %d rlineto %d 0 rlineto closepath fill\n",
                        x1, y1, hp*2, hp*2, -hp*2);
                }
                v++;
            }
            u++;
        }
      }
        break;

    case HT_HEX:
        {
        int i;
        double dx;
        int idx;
        dx = sqrt(3)/2.0*hp;
        idx = (int)dx;
        i = 0;
        for(x1=-aw/2-idx;x1<aw/2+idx;x1+=idx*2) {
            fprintf(fp, "    %d %d moveto\n", x1, -ah);
            for(y1=-ah;y1<ah;y1+=hp*3) {
                x2=x1+def_hatchthick;
                fprintf(fp, "      %d %d rlineto %d %d rlineto\n",
                    -idx, hp/2, 0, hp);
                fprintf(fp, "      %d %d rlineto %d %d rlineto\n",
                     idx, hp/2, 0, hp);
                y2=y1;
            }
            fprintf(fp, "    %d %d moveto\n", x1, -ah);
            for(y1=-ah;y1<ah;y1+=hp*3) {
                fprintf(fp, "      %d %d rlineto %d %d rlineto\n",
                     idx, hp/2, 0, hp);
                fprintf(fp, "      %d %d rlineto %d %d rlineto\n",
                    -idx, hp/2, 0, hp);
                y2=y1;
            }
            fprintf(fp, "    stroke\n");
            i++;
        }
        }
        break;
    
    default:
    case HT_NONE:
        break;

    case HT_SOLID:
        fprintf(fp,
    "%d %d moveto %d 0 rlineto 0 %d rlineto %d 0 rlineto closepath fill\n",
            -aw/2, -ah/2, aw, ah, -aw);
        break;
#if 0
#endif
    case HT_SLASHED:
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-aw*6/10;x1<aw*6/10+ah*2;x1+=hp) {
            x2 = x1-ah*6/10*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x2, y1, x1, y2);
        }
        break;

    }

#if 0
    fprintf(fp, "%% %s end\n", __func__);
#endif

    return 0;
}

char qbox[BUFSIZ]="";
char qdia[BUFSIZ]="";

int
_epsdraw_deco(FILE *fp, int xw, int xh, int xlc, int xfc, char *xcmd)
{
    int bw;
    int cr;

#define CKBOX \
    if(!qbox[0]) { \
        sprintf(qbox, \
    " %d %d rmoveto %d 0 rlineto 0 %d rlineto %d 0 rlineto closepath fill", \
        -cr, -cr, 2*cr, 2*cr, -2*cr); \
    }

#define CKDIA \
    if(!qdia[0]) { \
        sprintf(qdia, \
    " %d 0 rmoveto %d %d rlineto %d %d rlineto %d %d rlineto closepath fill", \
        -cr, cr, -cr, cr, cr, -cr, cr); \
    }

    qbox[0] = '\0';
    qdia[0] = '\0';

    fprintf(fp, "%% wxh %dx%d lc %d fc %d cmd |%s|\n",
        xw, xh, xlc, xfc, xcmd);


    bw = xh/10;
    cr = xh/5;
    
        fprintf(fp, "  gsave %% _deco\n");
        changecolor(fp, xlc);
 /*
  *
  */
    if(strcasecmp(xcmd, "ibox")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath stroke\n",
                    -(xw/2-bw), -(xh/2-bw), xw-2*bw, 0, 0, xh-2*bw,
                    -(xw-2*bw), 0);
    }
    else
    if(strcasecmp(xcmd, "iibox")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath stroke\n",
                    -(xw/2-2*bw), -(xh/2-2*bw), xw-4*bw, 0, 0, xh-4*bw,
                    -(xw-4*bw), 0);
    }
 /*
  *
  */
    else
    if(strcasecmp(xcmd, "icir")==0) {
        int xr;
        xr = MAX(xh, xw)/2-bw;
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d %d -90 270 arc closepath stroke\n",
                    0, -xr, 0, 0, xr);
    }
    else
    if(strcasecmp(xcmd, "iicir")==0) {
        int xr;
        xr = MAX(xh, xw)/2-bw*2;
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d %d -90 270 arc closepath stroke\n",
                    0, -xr, 0, 0, xr);
    }
 /*
  *
  */
    else
    if(strcasecmp(xcmd, "wbar")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath fill\n",
                    -xw/2, -xh/2, bw, 0, 0, xh, -bw, 0);
    }
    else
    if(strcasecmp(xcmd, "ebar")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath fill\n",
                     xw/2, -xh/2, -bw, 0, 0, xh, bw, 0);
    }
    else
    if(strcasecmp(xcmd, "nbar")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath fill\n",
                    -xw/2, xh/2, xw, 0, 0, -bw, -xw, 0);
    }
    else
    if(strcasecmp(xcmd, "sbar")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath fill\n",
                    -xw/2, -xh/2, xw, 0, 0, bw, -xw, 0);
    }
 /*
  *
  */
    else
    if(strcasecmp(xcmd, "wbarw")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath stroke\n",
                    -xw/2, -xh/2, bw, 0, 0, xh, -bw, 0);
    }
    else
    if(strcasecmp(xcmd, "ebarw")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath stroke\n",
                     xw/2, -xh/2, -bw, 0, 0, xh, bw, 0);
    }
    else
    if(strcasecmp(xcmd, "nbarw")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath stroke\n",
                    -xw/2, xh/2, xw, 0, 0, -bw, -xw, 0);
    }
    else
    if(strcasecmp(xcmd, "sbarw")==0) {
        fprintf(fp, "    newpath %d %d moveto"
                    " %d %d rlineto %d %d rlineto %d %d rlineto"    
                    " closepath stroke\n",
                    -xw/2, -xh/2, xw, 0, 0, bw, -xw, 0);
    }
 /*
  *
  */
    else
    if(strcasecmp(xcmd, "ccir")==0) {
        fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n",
                    0, 0, cr);
    }
    else
    if(strcasecmp(xcmd, "nwcir")==0) {
        fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n",
                    -xw/2, xh/2, cr);
    }
    else
    if(strcasecmp(xcmd, "ncir")==0) {
        fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n",
                    0, xh/2, cr);
    }
    else
    if(strcasecmp(xcmd, "necir")==0) {
        fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n",
                    xw/2, xh/2, cr);
    }
    else
    if(strcasecmp(xcmd, "ecir")==0) {
        fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n",
                    xw/2, 0, cr);
    }
    else
    if(strcasecmp(xcmd, "secir")==0) {
        fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n",
                    xw/2, -xh/2, cr);
    }
    else
    if(strcasecmp(xcmd, "scir")==0) {
        fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n",
                    0, -xh/2, cr);
    }
    else
    if(strcasecmp(xcmd, "swcir")==0) {
        fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n",
                    -xw/2, -xh/2, cr);
    }
    else
    if(strcasecmp(xcmd, "wcir")==0) {
        fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n",
                    -xw/2, 0, cr);
    }
 /*
  *
  */
    else
    if(strcasecmp(xcmd, "cbox")==0) {
        CKBOX;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    0, 0, qbox);
    }
    else
    if(strcasecmp(xcmd, "nwbox")==0) {
        CKBOX;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    -xw/2, xh/2, qbox);
    }
    else
    if(strcasecmp(xcmd, "nbox")==0) {
        CKBOX;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    0, xh/2, qbox);
    }
    else
    if(strcasecmp(xcmd, "nebox")==0) {
        CKBOX;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    xw/2, xh/2, qbox);
    }
    else
    if(strcasecmp(xcmd, "ebox")==0) {
        CKBOX;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    xw/2, 0, qbox);
    }
    else
    if(strcasecmp(xcmd, "sebox")==0) {
        CKBOX;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    xw/2, -xh/2, qbox);
    }
    else
    if(strcasecmp(xcmd, "sbox")==0) {
        CKBOX;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    0, -xh/2, qbox);
    }
    else
    if(strcasecmp(xcmd, "swbox")==0) {
        CKBOX;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    -xw/2, -xh/2, qbox);
    }
    else
    if(strcasecmp(xcmd, "wbox")==0) {
        CKBOX;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    -xw/2, 0, qbox);
    }
 /*
  *
  */
    else
    if(strcasecmp(xcmd, "cdia")==0) {
        CKDIA;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    0, 0, qdia);
    }
    else
    if(strcasecmp(xcmd, "nwdia")==0) {
        CKDIA;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    -xw/2, xh/2, qdia);
    }
    else
    if(strcasecmp(xcmd, "ndia")==0) {
        CKDIA;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    0, xh/2, qdia);
    }
    else
    if(strcasecmp(xcmd, "nedia")==0) {
        CKDIA;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    xw/2, xh/2, qdia);
    }
    else
    if(strcasecmp(xcmd, "edia")==0) {
        CKDIA;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    xw/2, 0, qdia);
    }
    else
    if(strcasecmp(xcmd, "sedia")==0) {
        CKDIA;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    xw/2, -xh/2, qdia);
    }
    else
    if(strcasecmp(xcmd, "sdia")==0) {
        CKDIA;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    0, -xh/2, qdia);
    }
    else
    if(strcasecmp(xcmd, "swdia")==0) {
        CKDIA;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    -xw/2, -xh/2, qdia);
    }
    else
    if(strcasecmp(xcmd, "wdia")==0) {
        CKDIA;
        fprintf(fp, "    newpath %d %d moveto %s\n", 
                    -xw/2, 0, qdia);
    }
 /*
  *
  */
    else
    if(strcasecmp(xcmd, "slash")==0) {
        fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                    -xw/2, -xh/2, xw, xh);
    }
    else
    if(strcasecmp(xcmd, "backslash")==0) {
        fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                    -xw/2, xh/2, xw, -xh);
    }
    else
    if(strcasecmp(xcmd, "slashbar")==0) {
        fprintf(fp, "    newpath %d %d moveto %d %d rlineto"
                    " %d %d rlineto %d %d rlineto"
                    " %d %d rlineto %d %d rlineto fill\n",
                    -xw/2, -xh/2, bw, 0, xw-bw, xh-bw,
                    0, bw, -bw, 0, -(xw-bw), -(xh-bw));
    }
    else
    if(strcasecmp(xcmd, "backslashbar")==0) {
        fprintf(fp, "    newpath %d %d moveto %d %d rlineto"
                    " %d %d rlineto %d %d rlineto"
                    " %d %d rlineto %d %d rlineto fill\n",
                    -xw/2, xh/2, bw, 0, xw-bw, -(xh-bw),
                    0, -bw, -bw, 0, -(xw-bw), (xh-bw));
    }

        fprintf(fp, "  grestore %% _deco\n");

    return 0;
}

int
epsdraw_deco(FILE *fp, int xw, int xh, int xlc, int xfc, char *xcmdlist)
{
    int   r;
    int   c;
    char  cmd[BUFSIZ];
    char *p, *q;
    
    c = 0;
    p = xcmdlist;
    while(*p) {
        p = draw_word(p, cmd, BUFSIZ, DECO_SEPC);
        if(!cmd[0]) {
            break;
        }
        r = _epsdraw_deco(fp, xw, xh, xlc, xfc, cmd);
        c++;
    }

    return 0;
}

int
ss_strip(char *dst, int dlen, varray_t *ssar)
{
    int   i;
    sstr *uu;
    int   cc;
    int   w;

    dst[0] = '\0';
    cc = 0;

    for(i=0;i<ssar->use;i++) {
        uu = (sstr*)ssar->slot[i];
        if(uu && uu->ssval && *(uu->ssval) ) {
            w = strlen(uu->ssval);
#if 0
Echo(" %2d: %3d '%s'\n", i, w, uu->ssval);
#endif
            if(cc+w+1<dlen-1) {
                if(dst[0]) {
                    strcat(dst, "/");
                    cc += 1;
                }
                else {
                }
                strcat(dst, uu->ssval);
                cc += w;
            }
            else {
                return 1;
            }
        }
    }

    return 0;
}


int
ss_dump(FILE *ofp, varray_t *ssar)
{
    int   i;
    sstr *uu;

    for(i=0;i<ssar->use;i++) {
        uu = (sstr*)ssar->slot[i];
        fprintf(ofp, "ss %2d: val '%s' opt %d\n",
            i, uu->ssval, uu->ssopt);
    }

    return 0;
}

int
epsdraw_s2sstrbgX(FILE *fp, int x, int y, int wd, int ht,
        int al, int exof, int ro, int qof,
        int bgshape, int qbgmargin, int fgcolor, int bgcolor,
        char *src, int ugjust)
{
    varray_t *xar;
    sstr *ns;
    int   ik;
    char  tmp[BUFSIZ];
    char *s;
    char *p;
    char *q;
    int   nl;

    extern int sdumpNZ(FILE*, char*, char*, int);


#if 0
fprintf(stderr, "src |%s|\n", src);
#endif

    xar = varray_new();
    if(xar) {
        varray_entrysprintfunc(xar, ss_sprintf);

#if 0
        ns = (sstr*) malloc(sizeof(sstr));
        if(ns) {
            ns->ssval = strdup(src);
            ns->ssopt = 0;
            varray_push(xar, ns);
#if 1
            varray_fprint(stdout, xar);
#endif
        
        }
#endif

        s = src;
        while(*s) {
            nl = 0;
            memset(tmp, 0, sizeof(tmp));
#if 0
            sdumpNZ(stderr, "s  ", s, 32);
#endif
            p = s;
            while(*p) {
#if 0
                fprintf(stderr, "p  '%c' \t%p\n", *p, p);
#endif
                if(*p=='\\' && *(p+1)=='n') {
                    nl++;
                    break;
                }
                p++;
            }
#if 0
            sdumpNZ(stderr, "tmp", tmp, 32);
#endif
            memcpy(tmp, s, p-s);
#if 0
            sdumpNZ(stderr, "tmp", tmp, 31);
            fflush(stderr);
#endif

            if(tmp[0]) {
                ns = (sstr*) malloc(sizeof(sstr));
                if(ns) {
                    ns->ssval = strdup(tmp);
                    ns->ssopt = 0;
                    varray_push(xar, ns);
#if 0
                    varray_fprint(stdout, xar);
#endif
        
                }
            }

            if(nl) {
                p++;
                p++;
            }
            
            s = p++;
        }


        ik = epsdraw_sstrbgX(fp, x, y, wd, ht, al, exof, ro, qof,
            bgshape, qbgmargin, fgcolor, bgcolor, xar, ugjust);
    }
    else {
        ik = -1;
    }

    return ik;
}

#define MJ  {fprintf(stderr, "MJ %d i %d justify %d\n", \
        __LINE__, i, justify);}


char*
resolv_font(int xmode, int xface)
{
    char *rv;
    apair_t *ls;

    rv = NULL;
    if(xmode==FM_ASCII) {
        ls = ff_act_ial;
    }
    else {
        ls = ff_actk_ial;
    }
    rv = rassoc(ls, xface);

#if 0
    Echo("%s: xmode %s(%d) xface %s(%d) -> rv %s\n", __func__,
        rassoc(fm_ial, xmode), xmode,
        rassoc(ff_ial, xface),  xface, rv);
#endif
    return rv;
}

int
epsdraw_sstrbgX(FILE *fp, int x, int y, int wd, int ht,
        int al, int exof, int ro, int qof,
        int bgshape, int qbgmargin, int fgcolor, int bgcolor,
        varray_t *ssar, int ugjust)
{
/*
 * 2pass routine
 *     1st - calcurate width and draw backgound round-box
 *     1nd - draw string
 */

    int   i;
    int   py;
    int   pyb;
    int   gy;
    sstr *uu;
    int   fht;
    int   n;
    int   rh;
    char  qs[BUFSIZ];
    int   bgmargin;

    varray_t *tq;
    int   j;
    txe      *te;
    char      token[BUFSIZ];
    int       cc;
    char     *p, *q;
    int   curmode, newmode;
    int   cursize, curface;
    int   newsize, newface;
    char     *afn, *afhs;
    int       afh, afhmax;
    int     justify;
    int     hscale;
    char    mcontall[BUFSIZ];
    char    mcontline[BUFSIZ];
    char    mcpart[BUFSIZ];
    int     mcar[BUFSIZ];
    int     gjust;
    int     imof;
    int     offset;
    int     jsar[BUFSIZ];
    
    if(!ssar) {
        E;
        return -1;
    }

    memset(jsar, 0, sizeof(jsar));

#if 0
    Echo("%s: x,y %d,%d wd,ht %d,%d ro %d\n", __func__, x, y, wd, ht, ro);
#endif

#if 0
    ss_dump(stdout, ssar);
#endif
    ss_strip(mcontall, BUFSIZ, ssar);
#if 0
    Echo("mcontall '%s'\n", mcontall);
#endif

#if 1
    Echo("%s: x,y %d,%d wd,ht %d,%d al %d exof %d ro %d '%s' ugj %d\n",
        __func__, x, y, wd, ht, al, exof, ro, mcontall, ugjust);
#endif

#if 0
    varray_fprint(stdout, ssar);
#endif

    if(ssar->use<=0)  {
        goto skip_label;
    }

    fprintf(fp, "%% %s: pos %6d,%-6d %6dx%-6d %4d fg %d bg %d %d %d ; %s\n",
        __func__, x, y, wd, ht, ro,
        fgcolor, bgcolor, bgshape, qbgmargin, mcontall);
    fprintf(fp, "%% fg %d bg %d\n", fgcolor, bgcolor);



    memset(mcar, 0, sizeof(mcar));

    n = ssar->use;

    fht = def_textheight; 
    pyb = (int)((double)fht*textdecentfactor);
    bgmargin = (int)((double)fht*textbgmarginfactor);

    py = fht;
    rh = ht-(n*py+bgmargin*2);

    fprintf(fp, "%%  fht %d, ht %d, n %d, rh %d, py %d\n",
        fht, ht, n, rh, py);

    Echo("ht %d use %d -> py %d\n", 
        ht, ssar->use, py);

#if 0
fprintf(stderr, "wd %d bgmargin %d\n", wd, bgmargin);
#endif

    if(ugjust<0) {
        switch(al) {
        case PO_WEST:   gjust = SJ_LEFT;    imof = -wd/2;               break;
        case PO_WI:     gjust = SJ_LEFT;    imof = -wd/2 + 2*bgmargin;  break;
        case PO_WO:     gjust = SJ_RIGHT;   imof = -wd/2 - 2*bgmargin;  break;
        case PO_EAST:   gjust = SJ_RIGHT;   imof = wd/2;                break;
        case PO_EI:     gjust = SJ_RIGHT;   imof = wd/2 - 2*bgmargin;   break;
        case PO_EO:     gjust = SJ_LEFT;    imof = wd/2 + 2*bgmargin;   break;
        case PO_CE:     gjust = SJ_LEFT;    imof = 0;                   break;
        case PO_CEO:    gjust = SJ_LEFT;    imof = 0 + 2*bgmargin;      break;
        case PO_CW:     gjust = SJ_RIGHT;   imof = 0;                   break;
        case PO_CWO:    gjust = SJ_RIGHT;   imof = 0 - 2*bgmargin;      break;
        default:
                        gjust = SJ_CENTER;  imof = 0;                   break;
        }
    }
    else {
        gjust = ugjust;
        imof  = 0;
    }

    offset = exof + imof;

    fprintf(fp, "%% ugjust %d, al %d exof %d qof %d -> gjust %d imoff %d offset %d\n",
        ugjust, al, exof, qof, gjust, imof, offset);


    fprintf(fp, "gsave %% for sstr\n");
    fprintf(fp, "  %d %d translate\n", x, y);
    fprintf(fp, "  %d %d translate\n", offset, 0);
    fprintf(fp, "  %d rotate\n", ro);
    if(qof!=0) {
        fprintf(fp, "  %d %d translate\n", qof, 0);
    }

    if(text_mode) {
        fprintf(fp, "  gsave %% textmode\n");
        changetext(fp);
        /* center line */
        fprintf(fp, "    %d %d moveto %d %d rlineto stroke\n",
                            0, -ht/2, 0, ht);

        fprintf(fp, "    %d %d moveto %d %d rlineto %d %d rlineto stroke\n",
                            -wd/2, ht/2-rh/2,
                            -wd/10, 0, 0, -(n*py+bgmargin*2));
        fprintf(fp, "    %d %d moveto %d %d rlineto %d %d rlineto %d %d rlineto closepath fill\n",
                            -wd/2, ht/2-rh/2,
                            -wd/10, 0, 0, -bgmargin, wd/10, 0);
        fprintf(fp, "    %d %d moveto %d %d rlineto %d %d rlineto %d %d rlineto closepath fill\n",
                            -wd/2, ht/2-rh/2-(n*py+bgmargin*2),
                            -wd/10, 0, 0, bgmargin, wd/10, 0);

        fprintf(fp, "  grestore %% textmode\n");
    }

    fprintf(fp, "  %% text offset\n");
#if 0
    fprintf(fp, "  %d %d translate\n", 0, ht/2-rh/2);
#endif
    fprintf(fp, "  %d %d translate\n", 0, (py*n)/2-py+pyb);

    if(text_mode) {
        fprintf(fp, "  gsave\n");
        gy = 0;
        for(i=0;i<ssar->use;i++) {
            changetext3(fp);
    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                -wd/2, -gy, wd, 0);

            for(j=0;j<i+1;j++) {
    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                    -wd/2+wd/10*(j+1), -gy, 0, -pyb);
            }
            gy += py;
        }
        fprintf(fp, "  grestore\n");
    }

    tq = NULL;

    int lbg_h, lbg_b, lbg_t;

    lbg_h = 0;
    lbg_b = 0;
    lbg_t = 0;

    fprintf(fp, "  /sstrwar %d array def %% array for string-width\n",
        ssar->use);

    /*****
     ***** 1st pass
     *****/

    fprintf(fp, "  gsave %% bgdraw\n");

    gy = 0;
    for(i=0;i<ssar->use;i++) {
        gy -= py;
        uu = (sstr*)ssar->slot[i];

        if(tq) txe_release(tq);

        tq = varray_new();
        varray_entrysprintfunc(tq, txe_sprint);

        txe_parse(tq, uu->ssval);

#if 0
        varray_fprint(stdout, tq);
#endif

        justify = gjust;
        hscale = 100;

        fprintf(fp, "    %% enter str %d '%s' gy %d\n", i, uu->ssval, gy);

        fprintf(fp, "    gsave %% oneline\n");

Echo("  --- calc size 1\n");

        curmode = FM_ASCII;
        curface = FF_SERIF;
        cursize = FH_NORMAL;
        hscale  = 100;


        /* check content existance */
#if 0
        mcontline[0] = '\0';
        for(j=0;j<tq->use;j++) {
            te = tq->slot[j];
            qs[0] = '\0';
            if(te->ct==TXE_DATA) {
                if(te->st==TXE_CONST) {
                    psescape(qs, BUFSIZ, te->cs);
                }
                else {
                    psescape(qs, BUFSIZ, te->vs);
                }
            }
            strcat(mcontline, qs);
        }
#endif
        txear_extract(mcontline, BUFSIZ, tq);

        if(!mcontline[0]) {
            fprintf(fp, "    %% skip  sstr drawing %d '%s'\n", i, mcontline);
            goto skip_bgdrawing;
        }
        else {
            fprintf(fp, "    %% enter sstr drawing %d '%s'\n", i, mcontline);
        }
        mcar[i] = strlen(mcontline);

#if 0
        MTF(0, -wd/4, fht, 0);
        MTF(1, -wd/4, 0, 0);
        MTF(4, -wd/4, -pyb, 0);

        MCF(5, -wd/2, fht+bgmargin);
        MQF(5, -wd/2, -pyb-bgmargin);
#endif

        fprintf(fp, "      %% calc size (width)\n");
        fprintf(fp, "      /sstrw 0 def\n");
        fprintf(fp, "      /%s findfont %d scalefont setfont %% default\n",
            def_fontname, fht);
        afh = fht;

        afhmax = -1;
    
        qs[0] = '\0';
        for(j=0;j<tq->use;j++) {
            te = tq->slot[j];
            
            if(te->ct==TXE_CMD) {
                newmode = -1;
                newface = -1;
                newsize = -1;
                p = NULL;
                if(te->st==TXE_CONST) {
                    p = te->cs;
                }
                else {
                    p = te->vs;
                }
                while(*p) {
                    p = draw_word(p, token, BUFSIZ, SSTR_SEPC);
                    if(token[0]) {
Echo("    token '%s'\n", token);
                        if(strncasecmp(token,"kanji", 5)==0) {
                            newmode = FM_KANJI;
                        }
                        if(strncasecmp(token,"ascii", 5)==0) {
                            newmode = FM_ASCII;
                        }
                        if(strncasecmp(token,"scale", 5)==0) {
                            hscale = atoi(token+5);
Echo("    hscale %d\n", hscale);

                        }
                        cc = assoc(fh_ial, token);
                        switch(cc) {
                        case FH_HUGE:   newsize = FH_HUGE;  break;
                        case FH_LARGE:  newsize = FH_LARGE; break;
                        case FH_SMALL:  newsize = FH_SMALL; break;
                        case FH_TINY:   newsize = FH_TINY;  break;
                        case FH_NORMAL: newsize = FH_NORMAL; break;
                        }                   
                        cc = assoc(ff_ial, token);
                        switch(cc) {
                        case FF_SANSERIF:   newface = FF_SANSERIF;  break;
                        case FF_ITALIC:     newface = FF_ITALIC;    break;
                        case FF_TYPE:       newface = FF_TYPE;  break;
                        case FF_SERIF:      newface = FF_SERIF; break;
                        }
                        cc = assoc(sj_ial, token);
                        if(cc>=0) { justify = cc; }
                    }
                }
#if 0
                if(cursize != newsize || curface != newface) 
#endif
                {
Echo(" newface %d newsize %d newmode %d\n", newface, newsize, newmode);
                    if(newmode>=0) {
                        curmode = newmode;
                    }
                    if(newsize>=0) {
P;
                        cursize = newsize;
                    }
                    if(newface>=0) {
P;
                        curface = newface;
                    }
Echo(" curface %d cursize %d curmode %d\n", curface, cursize, curmode);
/*
                    afn  = rassoc(ff_act_ial, curface);
*/
                    afn  = resolv_font(curmode, curface);
                    afhs = rassoc(fh_act_ial, cursize);
                    afh  = fht;
                    if(afhs!=NULL) {
                        afh = atof(afhs)*fht;
                    }

                    if(curmode==FM_KANJI) {
                        afh = (int)(afh*akratio);
                    }

Echo("  afn '%s' afhs '%s' afh %d (max %d)\n", afn, afhs, afh, afhmax);

                    if(afn) {
Echo("   setfont!\n");
                        fprintf(fp, "        /%s findfont %d scalefont setfont\n",
                            afn, afh);
                    }
                    else {
                    }
                }
            }
            else
            if(te->ct==TXE_DATA) {
#if 0
                if(te->st==TXE_CONST) {
                    psescape(qs, BUFSIZ, te->cs);
                }
                else {
                    psescape(qs, BUFSIZ, te->vs);
                }
#endif
                txe_extract(mcpart, BUFSIZ, te);
                psescape(qs, BUFSIZ, mcpart);

                if(hscale!=100) {
                    fprintf(fp, "  gsave %% text-scale\n");
                    fprintf(fp, "    %.3f 1 scale\n", (double)hscale/100);
                }
    if(hscale!=100) {
        fprintf(fp,
        "      (%s) stringwidth pop /sstrw exch %.3f mul sstrw add def\n",
            qs, (double)hscale/100);
    }
    else {
        fprintf(fp,
        "      (%s) stringwidth pop /sstrw exch sstrw add def\n",
            qs);
    }
                if(hscale!=100) {
                    fprintf(fp, "  grestore %% text-scale\n");
                }
#if 1
                /* check max height when it used */
                if(afh>afhmax) {afhmax = afh; }
#endif
            }

        }

        fprintf(fp, 
            "      sstrwar %d sstrw put %% store value to reuse\n", i);

        if(text_mode) {
            fprintf(fp, "      %% textguide 1st\n");
            fprintf(fp, "      gsave\n");

#if 0
            changetext3(fp);
            fprintf(fp, "    %d 0 moveto\n",  -wd/2);
            fprintf(fp, "    %d 0 rlineto\n", wd);
            fprintf(fp, "    0  %d rlineto\n", wd/4);
            fprintf(fp, "    stroke\n");
#endif

            changetext2(fp);
            fprintf(fp, "        sstrw 2 div neg %d moveto\n", objunit*7/100);
            fprintf(fp, "        0 %d rlineto\n", -objunit*7/100);
            fprintf(fp, "        sstrw 0 rlineto\n");
            fprintf(fp, "        0 %d rlineto\n", objunit*7/100);
            fprintf(fp, "        stroke\n");

            fprintf(fp, "      grestore %% textguide\n");
        }


        if(i<BUFSIZ) jsar[i] = justify;

        fprintf(fp, "      %% justify %d\n", justify);
        
        switch(justify) {
        case SJ_LEFT:
            break;
        case SJ_RIGHT:
            fprintf(fp, "      sstrw neg 0 translate\n");
            break;
        default:
        case SJ_CENTER:
            fprintf(fp, "      sstrw 2 div neg 0 translate\n");
            break;
        case SJ_FIT:
#if 0
            fprintf(fp, "      sstrw 2 div neg 0 translate\n");
#endif
#if 0
            fprintf(fp, "      %d 2 div neg 0 translate\n", wd);
#endif
            fprintf(fp, "      %d sstrw div 1 scale\n", wd);
            fprintf(fp, "      sstrw 2 div neg 0 translate\n");
            break;
        }

        if(bgcolor>=0) {
            fprintf(fp, "      %% bg fill\n");
            fprintf(fp, "      gsave %% textbg\n");
            changecolor(fp, bgcolor);

            fprintf(fp, "        0 %d neg sstrw %d %d mrboxfill\n",
                pyb, pyb+afhmax, bgmargin);

#if 0
            MTF(1, 0, afhmax, 0);
#endif

            fprintf(fp, "      grestore %% textbg\n");
        }

skip_bgdrawing:
        (void)0;

        fprintf(fp, "    grestore %% oneline\n");

        fprintf(fp, "    0 %d translate\n", -py);
    }
    fprintf(fp, "  grestore %% bgdraw\n");


    /*****
     ***** 2nd pass
     *****/

    fprintf(fp, "  %%%% text\n");
    fprintf(fp, "  gsave %% txtdraw\n");

    gy = 0;
    for(i=0;i<ssar->use;i++) {
        gy -= py;
        uu = (sstr*)ssar->slot[i];

        if(tq) txe_release(tq);

        tq = varray_new();
        varray_entrysprintfunc(tq, txe_sprint);

        txe_parse(tq, uu->ssval);

#if 0
        varray_fprint(stdout, tq);
#endif

        if(i<BUFSIZ) justify = jsar[i];
        hscale = 100;

        fprintf(fp, "    %% enter str %d '%s' gy %d\n", i, uu->ssval, gy);

        fprintf(fp, "    gsave %% oneline\n");


Echo("  --- calc size 2\n");

        curmode = FM_ASCII;
        curface = FF_SERIF;
        cursize = FH_NORMAL;
        hscale  = 100;


        if(mcar[i]<=0) {
            fprintf(fp, "  %% skip  sstr drawing %d mcar %d\n", i, mcar[i]);
            goto skip_txtdrawing;
        }

        /* check content existance */
#if 0
        mcontline[0] = '\0';
        for(j=0;j<tq->use;j++) {
            te = tq->slot[j];
            qs[0] = '\0';
            if(te->ct==TXE_DATA) {
                if(te->st==TXE_CONST) {
                    psescape(qs, BUFSIZ, te->cs);
                }
                else {
                    psescape(qs, BUFSIZ, te->vs);
                }
            }
            strcat(mcontline, qs);
        }
#endif
        txear_extract(mcontline, BUFSIZ, tq);

        if(!mcontline[0]) {
            fprintf(fp, "      %% skip  sstr drawing %d '%s'\n", i, mcontline);
            goto skip_txtdrawing;
        }
        else {
            fprintf(fp, "      %% enter sstr drawing %d '%s'\n", i, mcontline);
        }


        fprintf(fp, "      /sstrw sstrwar %d get def %% reuse width\n", i);

#if 0
        MCF(1, -wd/2-(i+1)*wd/10, afhmax+bgmargin);
        MCF(1, -wd/2-(i+1)*wd/10, afhmax);

        MCF(5, -wd/2-(i+1)*wd/10, fht+bgmargin);
        MCF(5, -wd/2-(i+1)*wd/10, fht);
        MTF(0, -wd/2-(i+1)*wd/10, py, -180);
        MTF(0, -wd/2-(i+1)*wd/10, 0, 0);
        MCF(4, -wd/2-(i+1)*wd/10, -pyb);

        MQF(4, -wd/2-(i+1)*wd/10, -pyb-bgmargin);
#endif

        if(text_mode) {
            fprintf(fp, "      %% textguide 2nd\n");
            fprintf(fp, "      gsave\n");

#if 0
            changetext3(fp);
            fprintf(fp, "    %d 0 moveto\n",  -wd/2);
            fprintf(fp, "    %d 0 rlineto\n", wd);
            fprintf(fp, "    0  %d rlineto\n", wd/4);
            fprintf(fp, "    stroke\n");
#endif

            changetext2(fp);
            fprintf(fp, "        sstrw 2 div neg %d moveto\n", objunit*7/100);
            fprintf(fp, "        0 %d rlineto\n", -objunit*7/100);
            fprintf(fp, "        sstrw 0 rlineto\n");
            fprintf(fp, "        0 %d rlineto\n", objunit*7/100);
            fprintf(fp, "        stroke\n");

            fprintf(fp, "      grestore %% textguide\n");
        }


        /*** PASS 2 */

Echo("  --- drawing\n");
        fprintf(fp, "      /%s findfont %d scalefont setfont %% default\n",
            def_fontname, fht);
        fprintf(fp, "      0 0 moveto\n");
#if 1
        changecolor(fp, fgcolor);
#endif

        curmode = FM_ASCII;
        curface = FF_SERIF;
        cursize = FH_NORMAL;
        hscale  = 100;

#if 1
                fprintf(fp, "      %% justify %d\n", justify);
                
                switch(justify) {
                case SJ_LEFT:
                    break;
                case SJ_RIGHT:
                    fprintf(fp, "      sstrw neg 0 translate\n");
                    break;
                default:
                case SJ_CENTER:
                    fprintf(fp, "      sstrw 2 div neg 0 translate\n");
                    break;
                case SJ_FIT:
#if 0
                    fprintf(fp, "      sstrw 2 div neg 0 translate\n");
#endif
#if 0
                    fprintf(fp, "      %d 2 div neg 0 translate\n", wd);
#endif
                    fprintf(fp, "      %d sstrw div 1 scale\n", wd);
                    fprintf(fp, "      sstrw 2 div neg 0 translate\n");
                    break;
                }
                    fprintf(fp, "      0 0 moveto\n");
#endif

#if 0
        Echo("0 curface %s(%d) cursize %s(%d) curmode %s(%d)\n",
            rassoc(ff_ial, curface), curface,
            rassoc(fh_ial, cursize), cursize,
            rassoc(fm_ial, curmode), curmode);
#endif

        Echo("0 m/f/h %s(%d) %s(%d) %s(%d)\n",
            rassoc(fm_ial, curmode), curmode,
            rassoc(ff_ial, curface), curface,
            rassoc(fh_ial, cursize), cursize);

        for(j=0;j<tq->use;j++) {
            te = tq->slot[j];
            
            if(te->ct==TXE_CMD) {
                newmode = -1;
                newface = -1;
                newsize = -1;
                p = NULL;
                if(te->st==TXE_CONST) {
                    p = te->cs;
                }
                else {
                    p = te->vs;
                }
                while(*p) {
                    p = draw_word(p, token, BUFSIZ, SSTR_SEPC);
                    if(token[0]) {
                        if(strncasecmp(token,"kanji", 5)==0) {
                            newmode = FM_KANJI;
                        }
                        if(strncasecmp(token,"ascii", 5)==0) {
                            newmode = FM_ASCII;
                        }
                        if(strncasecmp(token,"scale", 5)==0) {
                            hscale = atoi(token+5);
Echo("    hscale %d\n", hscale);
                        }
                        if(strncasecmp(token,"hscale", 6)==0) {
                            hscale = atoi(token+6);
Echo("    hscale %d\n", hscale);
                        }
                        cc = assoc(fh_ial, token);
                        switch(cc) {
                        case FH_HUGE:   newsize = FH_HUGE;  break;
                        case FH_LARGE:  newsize = FH_LARGE; break;
                        case FH_SMALL:  newsize = FH_SMALL; break;
                        case FH_TINY:   newsize = FH_TINY;  break;
                        case FH_NORMAL: newsize = FH_NORMAL; break;
                        }                   
                        cc = assoc(ff_ial, token);
                        switch(cc) {
                        case FF_SANSERIF:   newface = FF_SANSERIF;  break;
                        case FF_ITALIC:     newface = FF_ITALIC;    break;
                        case FF_TYPE:       newface = FF_TYPE;  break;
                        case FF_SERIF:      newface = FF_SERIF; break;
                        }
                        cc = assoc(sj_ial, token);
#if 0
                        if(cc>=0) { justify = cc; MJ;}
#endif
                    }
                }
#if 0
                if(cursize != newsize || curface != newface) 
#endif
                {
Echo(" newface %d newsize %d newmode %d\n", newface, newsize, newmode);
                    if(newmode>=0) {
                        curmode = newmode;
                    }
                    if(newsize>=0) {
P;
                        cursize = newsize;
                    }
                    if(newface>=0) {
P;
                        curface = newface;
                    }
Echo(" curface %d cursize %d curmode %d\n", curface, cursize, curmode);

/*
                    afn  = xrassoc(ff_act_ial, curface);
*/
                    afn  = resolv_font(curmode, curface);
                    afhs = xrassoc(fh_act_ial, cursize);
                    afh  = fht;
                    if(afhs!=NULL) {
                        afh = atof(afhs)*fht;
                    }

                    if(curmode==FM_KANJI) {
                        afh = (int)(afh*akratio);
                    }

Echo("  afn '%s' afhs '%s' afh %d\n", afn, afhs, afh);

                    if(afn) {
Echo("   setfont!\n");
                        fprintf(fp, "        /%s findfont %d scalefont setfont\n",
                            afn, afh);
                    }
                    else {
                    }

                }
            }
            else
            if(te->ct==TXE_DATA) {
#if 0
                if(te->st==TXE_CONST) {
                    psescape(qs, BUFSIZ, te->cs);
                }
                else {
                    psescape(qs, BUFSIZ, te->vs);
                }
#endif
                txe_extract(mcpart, BUFSIZ, te);
                psescape(qs, BUFSIZ, mcpart);

#if 0
                fprintf(fp, "      %% justify %d\n", justify);
MJ;
                
                switch(justify) {
                case SJ_LEFT:
                    break;
                case SJ_RIGHT:
                    fprintf(fp, "      sstrw neg 0 translate\n");
                    break;
                default:
                case SJ_CENTER:
                    fprintf(fp, "      sstrw 2 div neg 0 translate\n");
                    break;
                case SJ_FIT:
#if 0
                    fprintf(fp, "      sstrw 2 div neg 0 translate\n");
#endif
#if 0
                    fprintf(fp, "      %d 2 div neg 0 translate\n", wd);
#endif
                    fprintf(fp, "      %d sstrw div 1 scale\n", wd);
                    fprintf(fp, "      sstrw 2 div neg 0 translate\n");
                    break;
                }
                fprintf(fp, " 0 0 moveto\n");
#endif

                if(hscale!=100) {
P;
                    fprintf(fp, "  gsave %% comp\n");
                    fprintf(fp, "    %.3f 1 scale\n", (double)hscale/100);
                }

Echo("D m/f/s %s(%d) %s(%d) %s(%d)\n",
    rassoc(fm_ial, curmode), curmode,
    rassoc(ff_ial, curface), curface,
    rassoc(fh_ial, cursize), cursize);

                fprintf(fp, "        (%s) show\n", qs);
                if(hscale!=100) {
P;
                    fprintf(fp, "  grestore %% comp\n");
                }
            }

        }

skip_txtdrawing:
        (void)0;

        fprintf(fp, "    grestore %% oneline\n");

        fprintf(fp, "    0 %d translate\n", -py);
    }

    fprintf(fp, "  grestore %% txtdraw\n");


    fprintf(fp, "grestore %% end of sstr\n");

skip_label:
    (void)0;

    return 0;
}





/*** OBJECTS */

int
epsdraw_ruler(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
P;
    fprintf(fp, "%% ruler\n");
    fprintf(fp, "gsave\n");
    fprintf(fp, "  /%s findfont %d scalefont setfont\n",
        def_fontname, 12*objunit/100);
    fprintf(fp, "  1 0 0 setrgbcolor\n");
    fprintf(fp, "  100 setlinewidth\n");
    fprintf(fp, "  %d %d moveto\n", xox, xoy);
    fprintf(fp, "  0 -360 moveto 0 0 lineto 7200 0 lineto 7200 -360 lineto stroke\n");
    fprintf(fp, "  7400 -360 moveto (1inch) show\n");
    fprintf(fp, "  0 -1440 translate\n");
    fprintf(fp, "  0 -360 moveto 0 0 lineto 2834 0 lineto 2834 -360 lineto stroke\n");
    fprintf(fp, "  7400 -360 moveto (1cm) show\n");
    fprintf(fp, "grestore\n");
    return 0;
}

int
epsdraw_plane(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    int x1, y1;
P;
    /*** XXX ***/
    x1 = xox + xu->cx;
    y1 = xoy + xu->cy;

    fprintf(fp, "gsave\n");
    fprintf(fp, "  %d %d translate\n", xox+xu->wd/2, xoy+xu->ht/2);
    {
        fprintf(fp, "  %d %d moveto\n", -xu->wd/4, -xu->ht*3/4);
        fprintf(fp, "  %d %d lineto\n", -xu->wd/2, -xu->ht*4/4);
        fprintf(fp, "  %d %d lineto\n",  xu->wd/4, -xu->ht*4/4);
        fprintf(fp, "  %d %d lineto\n",  xu->wd/2, -xu->ht*3/4);
        fprintf(fp, "  closepath \n");
        fprintf(fp, "  clip\n");
    }
    epsdraw_hatch(fp, xu->wd, xu->ht*2,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

    if(debug_clip) {
        fprintf(fp, "  initclip\n");
        fprintf(fp, "  0 0 0 setrgbcolor %% clip mark\n");
        fprintf(fp, "  %d %d moveto\n", -xu->wd/4, -xu->ht*3/4);
        fprintf(fp, "  %d %d lineto\n", -xu->wd/2, -xu->ht*4/4);
        fprintf(fp, "  %d %d lineto\n",  xu->wd/4, -xu->ht*4/4);
        fprintf(fp, "  %d %d lineto\n",  xu->wd/2, -xu->ht*3/4);
        fprintf(fp, "  closepath \n");
        fprintf(fp, "  stroke\n");
    }

    fprintf(fp, "grestore\n");

    return 0;
}

#define EPSOP_NONE      (0)
#define EPSOP_STROKE    (1)
#define EPSOP_FILL      (2)
#define EPSOP_CLIP      (3)

int
_box_path(FILE *fp, int x1, int y1, int aw, int ah, int r, int op)
{
    fprintf(fp, "    newpath\n");
#if 0
    fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
    fprintf(fp, "    %d %d rlineto\n", aw, 0);
    fprintf(fp, "    %d %d rlineto\n", 0, ah);
    fprintf(fp, "    %d %d rlineto\n", -aw, 0);
    fprintf(fp, "    %d %d rlineto\n", 0, -ah);
#endif
    if(r>0) {
        fprintf(fp, "    %d %d moveto\n", -aw/2+r, -ah/2);
        fprintf(fp, "    %d %d rlineto\n", aw-r*2, 0);
        fprintf(fp, "    %d %d %d -90 0 arc\n", aw/2-r, -ah/2+r, r);
        fprintf(fp, "    %d %d rlineto\n", 0, ah-r*2);
        fprintf(fp, "    %d %d %d 0 90 arc\n", aw/2-r, ah/2-r, r);
        fprintf(fp, "    %d %d rlineto\n", -aw+r*2, 0);
        fprintf(fp, "    %d %d %d 90 180 arc\n", -aw/2+r, ah/2-r, r);
        fprintf(fp, "    %d %d rlineto\n", 0, -ah+r*2);
        fprintf(fp, "    %d %d %d 180 270 arc\n", -aw/2+r, -ah/2+r, r);
    }
    else {
        fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
        fprintf(fp, "    %d %d rlineto\n", aw, 0);
        fprintf(fp, "    %d %d rlineto\n", 0, ah);
        fprintf(fp, "    %d %d rlineto\n", -aw, 0);
        fprintf(fp, "    %d %d rlineto\n", 0, -ah);
    }
    fprintf(fp, "    closepath\n");

    switch(op) {
    case EPSOP_STROKE:  fprintf(fp, "    stroke\n");    break;
    case EPSOP_FILL:    fprintf(fp, "    fill\n");      break;
    case EPSOP_CLIP:    fprintf(fp, "    clip\n");      break;
    default:            fprintf(fp, "    clip\n");      break;
    }
    return 0;
}


int
mkpath_box(varray_t *sar, int wd, int ht, int rad)
{
P;
    if(rad<=0) {
        path_regsegmoveto(sar,  -wd/2, -ht/2);
        path_regsegrlineto(sar,    wd,     0);
        path_regsegrlineto(sar,     0,    ht);
        path_regsegrlineto(sar,   -wd,     0);
        path_regsegrlineto(sar,     0,   -ht);
    }
    else {
        path_regsegmoveto(sar,  -wd/2+rad,        -ht/2);
        path_regsegforward(sar,  wd-2*rad);
        path_regsegarc(sar,      rad,   90);
        path_regsegforward(sar,  ht-2*rad);
        path_regsegarc(sar,      rad,   90);
        path_regsegforward(sar,  wd-2*rad);
        path_regsegarc(sar,      rad,   90);
        path_regsegforward(sar,  ht-2*rad);
        path_regsegarc(sar,      rad,   90);
    }
    path_regsegclose(sar);

    return 0;
}


int
mkpath_Rbox(varray_t *sar, int wd, int ht, int rad)
{
P;
    if(rad<=0) {
        path_regsegmoveto(sar,   wd/2, -ht/2);
        path_regsegrlineto(sar,   -wd,     0);
        path_regsegrlineto(sar,     0,    ht);
        path_regsegrlineto(sar,    wd,     0);
        path_regsegrlineto(sar,     0,   -ht);
    }
    else {
        path_regsegmoveto(sar,  -wd/2+rad, -ht/2);
        path_regsegdir(sar,     180);
        path_regsegarcn(sar,    rad,    90);
        path_regsegforward(sar, ht-2*rad);
        path_regsegarcn(sar,    rad,    90);
        path_regsegforward(sar, wd-2*rad);
        path_regsegarcn(sar,    rad,    90);
        path_regsegforward(sar, ht-2*rad);
        path_regsegarcn(sar,    rad,    90);
        path_regsegforward(sar, wd-2*rad);
    }
    path_regsegclose(sar);

    return 0;
}

int
mkpath_circle(varray_t *sar, int wd, int ht, int rad)
{
P;
    if(rad>0) {
        path_regsegmoveto(sar,     0,  -rad);
        path_regsegarc(sar,    rad,   360);
        path_regsegclose(sar);
    }
    else
    if(rad==0) {
        /* skip */
    }
    else {
        path_regsegmoveto(sar,     0, -ht/2);
        path_regsegarc(sar,   ht/2,   360);
        path_regsegclose(sar);
    }

    return 0;
}


int
mkpath_Rcircle(varray_t *sar, int wd, int ht, int rad)
{
P;
    if(rad>0) {
        path_regsegmoveto(sar,     0,    rad);
        path_regsegarcn(sar,    rad,   360);
        path_regsegclose(sar);
    }
    else
    if(rad==0) {
        /* skip */
    }
    else {
        path_regsegmoveto(sar,     0,   ht/2);
        path_regsegarcn(sar,   ht/2,   360);
        path_regsegclose(sar);
    }

    return 0;
}

int
mkpath_gear(varray_t *sar, int wd, int ht, int rad, int aoff, int n, int th, int ir)
{
    double ar, br;
    int i;
    double x2, y2;
    double x3, y3;
    double x4, y4;
P;

    if(rad==0) {
        goto out;
    }
    if(rad<0) {
        ar = ht/2;
    }
    if(rad>0) {
        ar = rad;
    }

    br = ar*cos(M_PI*2/(double)n)/cos(M_PI*2/(double)n/2.0);
    br = ar - th;

#if 0
    Echo("polygon:  ar %.3f, br %.3f, aoff %d, n %d, cc %d\n",
        ar, br, aoff, n, cc);
#endif

    for(i=0;i<n;i++) {
        x2 = ar*cos(M_PI*2/(double)n*(double)i+aoff*rf);
        y2 = ar*sin(M_PI*2/(double)n*(double)i+aoff*rf);
        x3 = br*cos(M_PI*2/(double)n*((double)i+0.5)+aoff*rf);
        y3 = br*sin(M_PI*2/(double)n*((double)i+0.5)+aoff*rf);
        x4 = ar*cos(M_PI*2/(double)n*((double)i+1.0)+aoff*rf);
        y4 = ar*sin(M_PI*2/(double)n*((double)i+1.0)+aoff*rf);

        if(i==0) {
            path_regsegmoveto(sar,     x2, y2);
        }
        else {
        }
        path_regseglineto(sar,    x3, y3);
        if(i==n-1) {
            /* skip */
        }
        else {
            path_regseglineto(sar,    x4, y4);
        }
    }
    path_regsegclose(sar);

out:
    return 0;
}


int
mkpath_polygon(varray_t *sar, int wd, int ht, int rad, int aoff, int n, int cc)
{
    double ar, br;
    int i;
    double x2, y2;
    double x3, y3;
    double x4, y4;
P;

    if(rad==0) {
        goto out;
    }
    if(rad<0) {
        ar = ht/2;
    }
    if(rad>0) {
        ar = rad;
    }

    br = ar*cos(M_PI*2/(double)n)/cos(M_PI*2/(double)n/2.0);

#if 0
    Echo("polygon:  ar %.3f, br %.3f, aoff %d, n %d, cc %d\n",
        ar, br, aoff, n, cc);
#endif

    for(i=0;i<n;i++) {
        x2 = ar*cos(M_PI*2/(double)n*(double)i+aoff*rf);
        y2 = ar*sin(M_PI*2/(double)n*(double)i+aoff*rf);
        x3 = br*cos(M_PI*2/(double)n*((double)i+0.5)+aoff*rf);
        y3 = br*sin(M_PI*2/(double)n*((double)i+0.5)+aoff*rf);
        x4 = ar*cos(M_PI*2/(double)n*((double)i+1.0)+aoff*rf);
        y4 = ar*sin(M_PI*2/(double)n*((double)i+1.0)+aoff*rf);

        if(i==0) {
            path_regsegmoveto(sar,     x2, y2);
        }
        else {
        }
        if(cc) {
            path_regseglineto(sar,    x3, y3);
        }
        if(i==n-1) {
            /* skip */
        }
        else {
            path_regseglineto(sar,    x4, y4);
        }
    }
    path_regsegclose(sar);

out:
    return 0;
}



int
mkpath_Rpolygon(varray_t *sar, int wd, int ht, int rad, int aoff, int n, int cc)
{
    double ar, br;
    int i;
    double x2, y2;
    double x3, y3;
    double x4, y4;
P;

    if(rad==0) {
        goto out;
    }
    if(rad<0) {
        ar = ht/2;
    }
    if(rad>0) {
        ar = rad;
    }

    br = ar*cos(M_PI*2/(double)n)/cos(M_PI*2/(double)n/2.0);
#if 0
    Echo("Rpolygon: ar %.3f, br %.3f, aoff %d, n %d, cc %d\n", 
        ar, br, aoff, n, cc);
#endif

    for(i=0;i<n;i++) {
        x2 = ar*cos(M_PI*2/(double)n*(double)-i+aoff*rf);
        y2 = ar*sin(M_PI*2/(double)n*(double)-i+aoff*rf);
        x3 = br*cos(M_PI*2/(double)n*((double)-i-0.5)+aoff*rf);
        y3 = br*sin(M_PI*2/(double)n*((double)-i-0.5)+aoff*rf);
        x4 = ar*cos(M_PI*2/(double)n*((double)-i-1.0)+aoff*rf);
        y4 = ar*sin(M_PI*2/(double)n*((double)-i-1.0)+aoff*rf);
        if(i==0) {
            path_regsegmoveto(sar,     x2, y2);
        }
        else {
        }
        if(cc) {
            path_regseglineto(sar,    x3, y3);
        }
            path_regseglineto(sar,    x4, y4);
    }
    path_regsegclose(sar);

out:
    return 0;
}



int
mkpath_ellipseXXX(varray_t *sar, int wd, int ht, int rad, int dir)
{
    double r;
    double x, y;
    double lx, ly;
    double a;
    double ep = 0.01*objunit;
P;
#if 0
    path_regsegmoveto(sar,     0, -ht/2);
    path_regsegarc(sar,   ht/2,   360);
#endif
    a = ((double)wd/ht);
Echo("a %9.2f\n", a);
    r = ((double)ht)/2;
    path_regsegmoveto(sar,     0, -r);
    lx = 0;
    ly = -r;
    for(y=-r;y<=r;y+=ep) {
        x = ((double)dir)*sqrt(r*r-y*y);
#if 0
fprintf(stderr, "u y %9.2f x %9.2f\n", y-ly, x-lx);
#endif
        path_regsegrlineto(sar, (int)(a*(x-lx)), (int)(y-ly));
        lx = x; ly = y;
    }
    lx = 0;
    ly = r;
    for(y=r;y>=-r;y+=-ep) {
        x = ((double)-dir)*sqrt(r*r-y*y);
#if 0
fprintf(stderr, "d y %9.2f x %9.2f\n", y-ly, x-lx);
#endif
        path_regsegrlineto(sar, (int)(a*(x-lx)), (int)(y-ly));
        lx = x; ly = y;
    }
    path_regsegclose(sar);

    return 0;
}


#define mkpath_ellipse(a,w,h,r)     mkpath_ellipseXXX(a,w,h,r, 1)
#define mkpath_Rellipse(a,w,h,r)    mkpath_ellipseXXX(a,w,h,r,-1)

int
mkpath_paper(varray_t *sar, int wd, int ht, int rad)
{
P;
    path_regsegmoveto(sar,  -wd/2, -ht/2);
    path_regsegrlineto(sar,    wd-ht/4,     0);
    path_regsegrlineto(sar,    ht/4, ht/4);
    path_regsegrlineto(sar,     0,    ht-ht/4);
    path_regsegrlineto(sar,   -wd,     0);
    path_regsegrlineto(sar,     0,   -ht);
    path_regsegclose(sar);

    return 0;
}

int
Gpaper_surface(FILE *fp, int wd, int ht)
{
    fprintf(fp, 
"  newpath %d %d moveto %d %d rlineto %d %d rlineto closepath fill %% paper-surface\n",
        wd/2-ht/4, -ht/2, ht/4, ht/4, -ht/4, 0);
    return 0;
}

int
mkpath_card(varray_t *sar, int wd, int ht, int rad)
{
P;
    path_regsegmoveto(sar,  -wd/2,   -ht/2);
    path_regsegrlineto(sar,    wd,       0);
    path_regsegrlineto(sar,     0,      ht);
    path_regsegrlineto(sar, -wd+ht/4,    0);
    path_regsegrlineto(sar, -ht/4,   -ht/4);
    path_regsegclose(sar);

    return 0;
}

int
mkpath_Rcard(varray_t *sar, int wd, int ht, int rad)
{
P;
    path_regsegmoveto(sar,  -wd/2,   -ht/2);
    path_regsegrlineto(sar,     0, ht-ht/4);
    path_regsegrlineto(sar,  ht/4,    ht/4);
    path_regsegrlineto(sar, wd-ht/4,     0);
    path_regsegrlineto(sar,     0,     -ht);
    path_regsegclose(sar);

    return 0;
}

int
mkpath_diamond(varray_t *sar, int wd, int ht, int rad)
{
P;
    path_regsegmoveto(sar,      0,   -ht/2);
    path_regsegrlineto(sar,  wd/2,    ht/2);
    path_regsegrlineto(sar, -wd/2,    ht/2);
    path_regsegrlineto(sar, -wd/2,   -ht/2);
    path_regsegclose(sar);

    return 0;
}

int
mkpath_Rdiamond(varray_t *sar, int wd, int ht, int rad)
{
P;
    path_regsegmoveto(sar,      0,   -ht/2);
    path_regsegrlineto(sar, -wd/2,    ht/2);
    path_regsegrlineto(sar,  wd/2,    ht/2);
    path_regsegrlineto(sar,  wd/2,   -ht/2);
    path_regsegclose(sar);

    return 0;
}

int
mkpath_house(varray_t *sar, int wd, int ht, int rad)
{
P;
    path_regsegmoveto(sar,  -wd/2,   -ht/2);
    path_regsegrlineto(sar,    wd,       0);
    path_regsegrlineto(sar,     0, ht-ht/4);
    path_regsegrlineto(sar, -wd/2,    ht/4);
    path_regsegrlineto(sar, -wd/2,   -ht/4);
    path_regsegclose(sar);

    return 0;
}

int
mkpath_Rhouse(varray_t *sar, int wd, int ht, int rad)
{
P;
    path_regsegmoveto(sar,  -wd/2,   -ht/2);
    path_regsegrlineto(sar,     0, ht-ht/4);
    path_regsegrlineto(sar,  wd/2,    ht/4);
    path_regsegrlineto(sar,  wd/2,   -ht/4);
    path_regsegrlineto(sar,     0, -ht+ht/4);
    path_regsegclose(sar);

    return 0;
}

int
Gdrum_surface(FILE *fp, int wd, int ht)
{
    int i;
    int p=10;
    double x, y;

P;
    fprintf(fp, "%% drum-surface\n");
    fprintf(fp, "  gsave\n");
    fprintf(fp, "  2 setlinejoin\n");
    fprintf(fp, "  newpath\n");
    fprintf(fp, "  %d %d moveto\n", -wd/2, 3*ht/8);
    for(i=180;i<=360;i+=p) {
        x = wd/2*cos(M_PI/180*i);
        y = ht/8*sin(M_PI/180*i);
        fprintf(fp, "  %d %d lineto\n", (int)x, (int)(3*ht/8+y));
    }
    fprintf(fp, "  stroke\n");
    fprintf(fp, "  grestore\n");

    return 0;
}

int
mkpath_drum(varray_t *sar, int wd, int ht, int rad)
{
    int i;
    int p=10;
    double x, y;

P;
    path_regsegmoveto(sar,  -wd/2,   -3*ht/8);
    for(i=180;i<=360;i+=p) {
        x = wd/2*cos(M_PI/180*i);
        y = ht/8*sin(M_PI/180*i);
        path_regseglineto(sar,  x,   -3*ht/8+y);
    }
    path_regsegrlineto(sar,     0,    6*ht/8);
    for(i=0;i<=180;i+=p) {
        x = wd/2*cos(M_PI/180*i);
        y = ht/8*sin(M_PI/180*i);
        path_regseglineto(sar,  x,    3*ht/8+y);
    }
    path_regseglineto(sar,  -wd/2,   -3*ht/8);
    path_regsegclose(sar);

    return 0;
}

int
mkpath_Rdrum(varray_t *sar, int wd, int ht, int rad)
{
    int i;
    int p=10;
    double x, y;

P;
    path_regsegmoveto(sar,  -wd/2,   -3*ht/8);
    path_regsegrlineto(sar,     0,    6*ht/8);
    for(i=180;i>=0;i-=p) {
        x = wd/2*cos(M_PI/180*i);
        y = ht/8*sin(M_PI/180*i);
        path_regseglineto(sar,  x,   3*ht/8+y);
    }
    for(i=0;i>=-180;i-=p) {
        x = wd/2*cos(M_PI/180*i);
        y = ht/8*sin(M_PI/180*i);
        path_regseglineto(sar,  x,   -3*ht/8+y);
    }
    path_regsegclose(sar);

    return 0;
}

int
Gpipe_surface(FILE *fp, int wd, int ht)
{
    int i;
    int p=10;
    double x, y;

P;
    fprintf(fp, "%% pipe-surface\n");
    fprintf(fp, "  gsave\n");
    fprintf(fp, "  2 setlinejoin\n");
    fprintf(fp, "  newpath\n");
    fprintf(fp, "  %d %d moveto\n", -3*wd/8, -ht/2);
    for(i=-90;i<=90;i+=p) {
        x = wd/8*cos(M_PI/180*i);
        y = ht/2*sin(M_PI/180*i);
        fprintf(fp, "  %d %d lineto\n", (int)x-3*wd/8, (int)y);
    }
    fprintf(fp, "  stroke\n");
    fprintf(fp, "  grestore\n");

    return 0;
}


int
mkpath_pipe(varray_t *sar, int wd, int ht, int rad)
{
    int i;
    int p=10;
    double x, y;

P;
    path_regsegmoveto(sar,  3*wd/8, -ht/2);
    for(i=-90;i<=90;i+=p) {
        x = wd/8*cos(M_PI/180*i);
        y = ht/2*sin(M_PI/180*i);
        path_regseglineto(sar,  x+3*wd/8,  y);
    }
    path_regsegrlineto(sar, -6*wd/8, 0);
    for(i=90;i<=270;i+=p) {
        x = wd/8*cos(M_PI/180*i);
        y = ht/2*sin(M_PI/180*i);
        path_regseglineto(sar,  x-3*wd/8, y);
    }
    path_regseglineto(sar,  3*wd/8, -ht/2);
    path_regsegclose(sar);

    return 0;
}

int
mkpath_Rpipe(varray_t *sar, int wd, int ht, int rad)
{
    int i;
    int p=10;
    double x, y;

P;
    path_regsegmoveto(sar,   3*wd/8, -ht/2);
    path_regsegrlineto(sar, -6*wd/8,    0);
    for(i=270;i>=80;i-=p) {
        x = wd/8*cos(M_PI/180*i);
        y = ht/2*sin(M_PI/180*i);
        path_regseglineto(sar,  x-3*wd/8, y);
    }
    for(i=90;i>=-90;i-=p) {
        x = wd/8*cos(M_PI/180*i);
        y = ht/2*sin(M_PI/180*i);
        path_regseglineto(sar,  x+3*wd/8, y);
    }
    path_regsegclose(sar);

    return 0;
}


int
epsdraw_bodyX(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    int ik;
    int mx, my;
    varray_t *saved_segar;
    seg *e;
    double a;
    int awd, aht;

    mx = xu->x+xox;
    my = xu->y+xoy;

#if 0
fprintf(fp, "%% %s oid %d type %d\n", __func__, xu->oid, xu->type);
#endif
Echo("%s: oid %d type %d\n", __func__, xu->oid, xu->type);

    saved_segar = xu->cob.segar;

    a = 1.0;

#if 0
    awd = xu->wd;
    aht = xu->ht;
#endif
    awd = xu->wd - xu->cob.imargin*2;
    aht = xu->ht - xu->cob.imargin*2;
    
    switch(xu->type) {
    case CMD_CHUNK:
    case CMD_BOX:
        ik = mkpath_box(xu->cob.segar, awd, aht, xu->cob.rad);
        ik = mkpath_Rbox(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    case CMD_PAPER:
        ik = mkpath_paper(xu->cob.segar, awd, aht, xu->cob.rad);
        ik = mkpath_Rbox(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    case CMD_CARD:
        ik = mkpath_card(xu->cob.segar, awd, aht, xu->cob.rad);
        ik = mkpath_Rcard(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    case CMD_DIAMOND:
        ik = mkpath_diamond(xu->cob.segar, awd, aht, xu->cob.rad);
        ik = mkpath_Rdiamond(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    case CMD_HOUSE:
        ik = mkpath_house(xu->cob.segar, awd, aht, xu->cob.rad);
        ik = mkpath_Rhouse(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    case CMD_CIRCLE:
        ik = mkpath_circle(xu->cob.segar, awd, aht, xu->cob.rad);
        ik = mkpath_Rcircle(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    case CMD_POINT:
        ik = mkpath_circle(xu->cob.segar, awd, aht, xu->cob.rad);
        ik = mkpath_Rcircle(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    case CMD_POLYGON:
        ik = mkpath_polygon(xu->cob.segar, awd, aht, xu->cob.rad,
                xu->cob.polyrotate, xu->cob.polypeak, xu->cob.concave);
        ik = mkpath_Rpolygon(xu->cob.seghar, awd, aht, xu->cob.rad,
                xu->cob.polyrotate, xu->cob.polypeak, xu->cob.concave);
        break;
    case CMD_GEAR:
        ik = mkpath_gear(xu->cob.segar, awd, aht, xu->cob.rad,
                xu->cob.polyrotate, xu->cob.polypeak, objunit/10, objunit/4);
        ik = mkpath_Rcircle(xu->cob.seghar, awd, aht, xu->cob.rad-objunit/10);
        break;
    case CMD_ELLIPSE:
        ik = mkpath_ellipse(xu->cob.segar,  awd, aht, xu->cob.rad);
        ik = mkpath_Rellipse(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    case CMD_DRUM:
        ik = mkpath_drum(xu->cob.segar,  awd, aht, xu->cob.rad);
        ik = mkpath_Rdrum(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    case CMD_PIPE:
        ik = mkpath_pipe(xu->cob.segar,  awd, aht, xu->cob.rad);
        ik = mkpath_Rpipe(xu->cob.seghar, awd, aht, xu->cob.rad);
        break;
    default:
        fprintf(fp, "%% unknown type '%s'(%d)\n",
            rassoc(cmd_ial, xu->type), xu->type);
        break;
    }

    fprintf(fp, "gsave %% bodyX\n");

#if 0
    changecolor(fp, xu->cob.outlinecolor);
    changethick(fp, xu->cob.outlinethick);
#endif
    fprintf(fp, "  %d %d translate 0 0 moveto %d rotate\n",
        xu->x+xox, xu->y+xoy, xu->cob.rotateval);
    fprintf(fp, "  %.3f %.3f scale\n", a, 1.0);

    if(xu->cob.marknode) {
        MX(0, 0, 0);
    }

    /*****
     ***** SHADE
     *****/
    if(xu->cob.shadow) {
        fprintf(fp, "  gsave    %% for shadow\n");
        fprintf(fp, "    %d -%d translate\n", objunit/10, objunit/10);
        fprintf(fp, "    %.2f setgray\n", def_shadowgray);
        changethick(fp, xu->cob.hatchthick);
        ik = drawpathN(fp, 0, 0, 0, xu, xns);
        fprintf(fp, "  fill     %% for shadow\n");
        fprintf(fp, "  grestore %% for shadow\n");
    }

    /*****
     ***** BACK
     *****/
    if(xu->cob.backhatch != HT_NONE && xu->cob.backcolor>=0) {
        fprintf(fp, "  gsave %% for back\n");
        changecolor(fp, xu->cob.backcolor);
        changethick(fp, xu->cob.hatchthick);
        ik = drawpathN(fp, 0, 0, 0, xu, xns);
        fprintf(fp, "  clip\n");

        epsdraw_hatch(fp, xu->wd, xu->ht,
          xu->cob.backcolor, xu->cob.backhatch, xu->cob.hatchpitch);

        fprintf(fp, "  grestore\n");
    }
    else {
        fprintf(fp, "  %% skip back\n");
    }

    /*****
     ***** FILL
     *****/
    if(xu->cob.fillhatch != HT_NONE && xu->cob.fillcolor>=0) {
        if(xu->cob.hollow && xu->cob.seghar) {
            fprintf(fp, "  gsave %% for fill + hollow\n");

            changecolor(fp, xu->cob.fillcolor);
            changethick(fp, xu->cob.hatchthick);
            ik = drawpathN_woclose(fp, 0, 0, 0, xu, xns);

#if 0
            fprintf(fp, "     0.8 0.8 scale\n");
#endif
            fprintf(fp, "     %.3f %.3f scale\n", 
                def_hollowratio, def_hollowratio);
            ik = drawpathR_wonew(fp, 0, 0, 0, xu, xns);
            fprintf(fp, "  clip\n");

#if 0
            fprintf(fp, "     1.25 1.25 scale\n");
#endif
            fprintf(fp, "     %.3f %.3f scale\n", 
                1.0/def_hollowratio, 1.0/def_hollowratio);

            epsdraw_hatch(fp, xu->wd, xu->ht,
              xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore %% for fill + hollow\n");
         }
         else {
            fprintf(fp, "  gsave %% for fill\n");
            changecolor(fp, xu->cob.fillcolor);
            changethick(fp, xu->cob.hatchthick);
            ik = drawpathN(fp, 0, 0, 0, xu, xns);
            fprintf(fp, "  clip\n");

            epsdraw_hatch(fp, xu->wd, xu->ht,
              xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore %% for fill\n");
        }
    }
    else {
        fprintf(fp, "  %% skip fill\n");
    }

    /*****
     ***** OUTLINE
     *****/
    if(xu->cob.outlinecolor>=0 && xu->cob.outlinethick>0) {
        fprintf(fp, "  gsave %% for outline\n");
#if 0
        changecolor(fp, xu->cob.outlinecolor);
        changethick(fp, xu->cob.outlinethick);
#endif
        if(xu->type==CMD_POINT) {
            changecolor(fp, xu->cob.outlinecolor);
            changethick(fp, xu->cob.outlinethick);
            ik = drawpathN(fp, 0, 0, 0, xu, xns);
            fprintf(fp, "  fill\n");
        }
        else {
            ik = drawpath_LT(fp, 0, 0, 0, xu, xns);
        }

        /*** SURFACE ***/
        if(xu->type==CMD_PAPER) {
            fprintf(fp, "  %% surface\n");
            ik = Gpaper_surface(fp, awd, aht);
        }
        if(xu->type==CMD_DRUM) {
            fprintf(fp, "  %% surface\n");
            ik = Gdrum_surface(fp,  awd, aht);
        }
        if(xu->type==CMD_PIPE) {
            fprintf(fp, "  %% surface\n");
            ik = Gpipe_surface(fp,  awd, aht);
        }
        fprintf(fp, "  grestore %% for outline\n");
    }
    else {
        fprintf(fp, "  %% skip outline\n");
    }

    /*****
     ***** DECO
     *****/
    if(xu->cob.deco) {
        int _dcolor;
        fprintf(fp, " %% deco |%s|\n", xu->cob.deco);
        fprintf(fp, " gsave %% for deco\n");
        changecolor(fp, xu->cob.fillcolor);
        changethick(fp, xu->cob.hatchthick);
        ik = drawpathN(fp, 0, 0, 0, xu, xns);
        fprintf(fp, "  clip\n");
        _dcolor = xu->cob.decocolor;
        if(_dcolor<0) {
            _dcolor = xu->cob.outlinecolor;
        }
        epsdraw_deco(fp, xu->wd, xu->ht,
            _dcolor, xu->cob.fillcolor, xu->cob.deco);
        fprintf(fp, " grestore %% for deco\n");
    }
    else {
        fprintf(fp, "  %% skip deco\n");
    }

    fprintf(fp, "grestore %% bodyX\n");

    return 0;
}


int
_cloud_shape(FILE *fp, double sx, double sy)
{
    fprintf(fp, "%.3f %.3f scale\n", sx, sy);

    fprintf(fp, "\
0.3 -0.4 moveto\n\
0.45 -0.55 0.72 -0.38 0.61 -0.21 curveto\n\
stroke\n\
0.63 -0.23 moveto\n\
0.75 -0.17 0.73 0.05 0.62 0.13 curveto\n\
stroke\n\
0.64 0.12 moveto\n\
0.75 0.2 0.55 0.43 0.44 0.37 curveto\n\
stroke\n\
0.45 0.35 moveto\n\
0.4 0.47 0.15 0.5 0.06 0.39 curveto\n\
stroke\n\
0.1 0.43 moveto\n\
0 0.53 -0.2 0.5 -0.25 0.43 curveto\n\
stroke\n\
-0.2 0.4 moveto\n\
-0.35 0.53 -0.57 0.37 -0.55 0.27 curveto\n\
stroke\n\
-0.5 0.27 moveto\n\
-0.68 0.3 -0.8 0.02 -0.67 -0.05 curveto\n\
stroke\n\
-0.63 -0.01 moveto\n\
-0.75 -0.08 -0.58 -0.38 -0.48 -0.28 curveto\n\
stroke\n\
-0.48 -0.27 moveto\n\
-0.5 -0.4 -0.2 -0.5 -0.12 -0.4 curveto\n\
stroke\n\
-0.15 -0.41 moveto\n\
0 -0.52 0.15 -0.53 0.33 -0.39 curveto\n\
stroke\n\
");

    return 0;
}

int
_cloud_clip(FILE *fp, double sx, double sy)
{
    fprintf(fp, "%.3f %.3f scale\n", sx, sy);

    fprintf(fp, "\
0.3 -0.4 moveto\n\
0.3 -0.4 lineto\n\
0.45 -0.55 0.72 -0.38 0.61 -0.21 curveto\n\
0.63 -0.23 lineto\n\
0.75 -0.17 0.73 0.05 0.62 0.13 curveto\n\
0.64 0.12 lineto\n\
0.75 0.2 0.55 0.43 0.44 0.37 curveto\n\
0.45 0.35 lineto\n\
0.4 0.47 0.15 0.5 0.06 0.39 curveto\n\
0.1 0.43 lineto\n\
0 0.53 -0.2 0.5 -0.25 0.43 curveto\n\
-0.2 0.4 lineto\n\
-0.35 0.53 -0.57 0.37 -0.55 0.27 curveto\n\
-0.5 0.27 lineto\n\
-0.68 0.3 -0.8 0.02 -0.67 -0.05 curveto\n\
-0.63 -0.01 lineto\n\
-0.75 -0.08 -0.58 -0.38 -0.48 -0.28 curveto\n\
-0.48 -0.27 lineto\n\
-0.5 -0.4 -0.2 -0.5 -0.12 -0.4 curveto\n\
-0.15 -0.41 lineto\n\
0 -0.52 0.15 -0.53 0.33 -0.39 curveto\n\
closepath\n\
");

    return 0;
}

int
epsdraw_cloud(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int r;
    double a;
    int aw, ah;
    double gws, ghs;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;
    r  = xu->vob.rad;
    a  = ((double)xu->wd)/((double)xu->ht);

    if(xu->cob.imargin>0) {
        aw = xu->wd - xu->cob.imargin*2;
        ah = xu->ht - xu->cob.imargin*2;
    }
    else {
        aw = xu->wd;
        ah = xu->ht;
    }

    gws = aw/1.5;
    ghs = ah/1.0;
    fprintf(fp, "%% gws %.3f ghs %.3f\n", gws, ghs);

apply:

    fprintf(fp, "%% cloud xy %d,%d wh %dx%d\n", x1, y1, aw, ah);
    fprintf(fp, "gsave %% for cloud\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);

    fprintf(fp, "gsave %% for inside\n");

    /***
     *** CLIP and HATCH
     ***/

    if(xu->cob.backhatch!=HT_NONE && xu->cob.backcolor>=0) {
        
        changecolor(fp, xu->cob.backcolor);

        fprintf(fp, "  %% clip & hatch\n");
        fprintf(fp, "  gsave\n");
        fprintf(fp, "    %d %d translate\n", x1, y1);
        fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
        fprintf(fp, "    newpath\n");
        fprintf(fp, "    0 setlinewidth\n");

        fprintf(fp, "  0 setlinewidth %% for cloud\n");
        _cloud_clip(fp, gws, ghs);
        if(debug_clip) {
            fprintf(fp, "  stroke %% debug\n");
        }
        else {
            fprintf(fp, "  clip\n");
        }

#if 0
        fprintf(fp, " %.3f %.3f scale\n", 1.0/gws, 1.0/ghs);
#endif
        fprintf(fp, " %.6f %.6f scale\n", 1.0/gws, 1.0/ghs);

        changecolor(fp, xu->cob.backcolor);
        changethick(fp, xu->cob.hatchthick);
        epsdraw_hatch(fp, aw, ah,
                xu->cob.backcolor, xu->cob.backhatch, xu->cob.hatchpitch);

        if(xu->cob.deco) {
            fprintf(fp, "%% deco |%s|\n", xu->cob.deco);
            epsdraw_deco(fp, aw, ah,
                xu->cob.outlinecolor, xu->cob.backcolor, xu->cob.deco);
        }
        else {
            fprintf(fp, "%% no-deco\n");
        }
        fprintf(fp, "  grestore\n");
    }

    if(xu->cob.fillhatch!=HT_NONE && xu->cob.fillcolor>=0) {
        
        changecolor(fp, xu->cob.fillcolor);

        fprintf(fp, "  %% clip & hatch\n");
        fprintf(fp, "  gsave\n");
        fprintf(fp, "    %d %d translate\n", x1, y1);
        fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
        fprintf(fp, "    newpath\n");
        fprintf(fp, "    0 setlinewidth\n");

        fprintf(fp, "  0 setlinewidth %% for cloud\n");
        _cloud_clip(fp, gws, ghs);
        if(debug_clip) {
            fprintf(fp, "  stroke %% debug\n");
        }
        else {
            fprintf(fp, "  clip\n");
        }

#if 0
        fprintf(fp, " %.3f %.3f scale\n", 1.0/gws, 1.0/ghs);
#endif
        fprintf(fp, " %.6f %.6f scale\n", 1.0/gws, 1.0/ghs);

        changecolor(fp, xu->cob.fillcolor);
        changethick(fp, xu->cob.hatchthick);
        epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

        if(xu->cob.deco) {
            fprintf(fp, "%% deco |%s|\n", xu->cob.deco);
            epsdraw_deco(fp, aw, ah,
                xu->cob.outlinecolor, xu->cob.fillcolor, xu->cob.deco);
        }
        else {
            fprintf(fp, "%% no-deco\n");
        }
        fprintf(fp, "  grestore\n");
    }

    fprintf(fp, "grestore %% for inside\n");


    fprintf(fp, "%% frame\n");
    fprintf(fp, "%%     outline color %d thick %d\n",
        xu->cob.outlinecolor, xu->cob.outlinethick);

    if(xu->cob.outlinecolor>=0 && xu->cob.outlinethick>0) {

        fprintf(fp, "gsave\n");
        fprintf(fp, "  %d %d translate\n", x1, y1);
        changecolor(fp, xu->cob.outlinecolor);
        fprintf(fp, "  %.3f setlinewidth %% for cloud\n", 
            ((double)xu->cob.outlinethick)/objunit);
        _cloud_shape(fp, gws, ghs);
        fprintf(fp, "grestore\n");

    }

    fprintf(fp, "grestore %% end of cloud\n");

out:
    return 0;
}

int
epsdraw_dots(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    int x1, y1;
    int x2, y2;
    int r;
    double a;
    int aw, ah;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;
    r  = xu->vob.rad;
    a  = ((double)xu->wd)/((double)xu->ht);

    if(xu->cob.imargin>0) {
        aw = xu->wd - xu->cob.imargin*2;
        ah = xu->ht - xu->cob.imargin*2;
    }
    else {
        aw = xu->wd;
        ah = xu->ht;
    }

apply:

    fprintf(fp, "%% dots xy %d,%d wh %dx%d\n", x1, y1, aw, ah);
    fprintf(fp, "gsave %% for dots\n");

    r  = objunit/20;
    x2 = objunit/4*cos((xu->cob.sepcurdir)*rf);
    y2 = objunit/4*sin((xu->cob.sepcurdir)*rf);

    fprintf(fp, "  gsave\n");
    fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n", x1-x2, y1-y2, r);
    fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n", x1,    y1,    r);
    fprintf(fp, "    newpath %d %d %d 0 360 arc fill\n", x1+x2, y1+y2, r);
    fprintf(fp, "  grestore\n");

    fprintf(fp, "grestore %% end of dots\n");

out:
    return 0;
}

/* 1segments : 2 points */
/*
 *  1         2
 *  +---------+
 */
static
int
mkpath_1seg(varray_t *sar,
    int x1, int y1, int x2, int y2)
{
#if 0
    fprintf(stderr, "%s: %d %d; %d %d\n",
        __func__, x1, x2, x2, y2);
#endif
    path_regsegmoveto(sar, x1, y1);
    path_regseglineto(sar, x2, y2);

    return 0;
}

/* seg+arc+seg : 5 points */

/* 3segments : 4 points */
/*
 *  1    2
 *  +----+
 *       |
 *       |
 *       +----+
 *       3    4
 */
static
int
mkpath_3seg(varray_t *sar,
    int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    Echo("%s: %d %d; %d %d; %d %d; %d %d\n",
        __func__, x1, y1, x2, y2, x3, y3, x4, y4);
    path_regsegmoveto(sar, x1, y1);
    path_regseglineto(sar, x2, y2);
    path_regseglineto(sar, x3, y3);
    path_regseglineto(sar, x4, y4);

    return 0;
}

static
int
mkpath_addbwcir(varray_t *sar,
    int x1, int y1)
{
    Echo("%s: %d %d\n",
        __func__, x1, y1);
    path_regsegbwcir(sar, x1, y1);

    return 0;
}

/* seg+arc+seg : 5 points */
/*
 *  1    2  3
 *  +----+  +  
 *        \
 *         \
 *          +----+
 *          4    5
 */
static
int
mkpath_segarcseg(varray_t *sar,
    int x1, int y1, int x2, int y2, int x3, int y3,
    int x4, int y4, int x5, int y5)
{
    Echo("%s: %d %d; %d %d; %d %d; %d %d\n",
        __func__, x1, x2, x2, y2, x3, y3, x4, y4);

    path_regsegmoveto(sar, x1, y1);
    path_regseglineto(sar, x2, y2);
    path_regsegcurveto(sar, x2, y2, x3, y3, x4, y4);
    path_regseglineto(sar, x5, y5);

    return 0;
}


/* seg+arc+seg : 4 points */
/*
 *  1    2  
 *  +----+ x  
 *        \
 *         \
 *          \
 *         x +----+
 *           4    5 
 */
static
int
mkpath_segarcseg2(varray_t *sar,
    int x1, int y1, int x2, int y2, 
    int x4, int y4, int x5, int y5)
{
    int x2b, y2b;
    int x4b, y4b;
    int cm;
    /* curve margin */
    cm = 2*objunit/10;
    
    if(x5>x1) {
        x2b = x2+cm; y2b = y2;
        x4b = x4-cm; y4b = y4;
    }
    else {
        x2b = x2-cm; y2b = y2;
        x4b = x4+cm; y4b = y4;
    }

    Echo("%s: %d %d; %d %d; %d %d; %d %d\n",
        __func__, x1, x2, x2, y2, x4, y4, x5, y5);

    path_regsegmoveto(sar, x1, y1);
    path_regseglineto(sar, x2, y2);
    path_regsegcurveto(sar, x2b, y2b, x4b, y4b, x4, y4);
    path_regseglineto(sar, x5, y5);

    return 0;
}

#define QMB(zqx, zqy) (void)0;
#define QM(zqx, zqy)  (void)0;

static
int
_drawgslink(varray_t *qar, int xid, int style, int jr,
    int j, int n, int h1, int h2, int v, 
    int sx, int sy, int maxsx,
    int mx, int my, int ex, int ey, int eey, int dsdir)
{

    /*
     *         h1
     * sx,sy ----+ t1x,t1y
     *           | v
     *   t2x,t2y +---- ex,ey
     *            h2
     */ 
    /*
     *
     *     maxsx,sy 
     * sx,sy +--+-----.
     *            h1  | v
     *                |  h2
     *                .-----+ ex,eey
     *                +-----+ ex,ey
     *              mx,my  
     */ 

    int rstyle;
    int focus;
    int join;

    rstyle = style & LS_M_TYPE;
    focus  = style & LS_FOCUS;
    join   = style & LS_JOIN;
#if 1
    join   = focus;
#endif

#if 0
    QMB(sx, sy);
    QMB(maxsx, sy);
    QMB(mx, my);
    QMB(ex, eey);
    QMB(ex, ey);
#endif

    Echo("%s: ar %p xid %d\n", __func__, qar, xid);

    Echo(
        "%s: xid %d style %3d %3xH rstyle %3d %3xH focus %d join %d j/n %d/%d\n",
        __func__, xid, style, style, rstyle, rstyle, focus, join, j, n);

#if 0
    if(dsdir>0) {
        path_regsegdir(qar, 180);
    }
    if(dsdir<0) {
        path_regsegdir(qar, -180);
    }
#endif

    switch(rstyle) {
    case LS_STRAIGHT:
        if(dsdir>=0) {
            mkpath_1seg(qar, sx, sy, ex, sy);
        }
        else {
            mkpath_1seg(qar, ex, sy, sx, sy);
        }
        break;
    case LS_SQUARE:
        if(focus) {
            if(dsdir>=0) {
                mkpath_3seg(qar, sx, sy, mx, sy, mx, ey, ex, ey);
            }
            else {
                mkpath_3seg(qar, ex, ey, mx, ey, mx, sy, sx, sy);
            }
            if(join) { mkpath_addbwcir(qar, mx, my); }
            if(join) {  if(j==0 || j==n-1) {}
                        else { mkpath_addbwcir(qar, mx, sy); }
            }
        }
        else {
            if(dsdir>=0) {
                mkpath_3seg(qar, sx, sy, maxsx+h1, sy, maxsx+h1, eey, ex, eey);
            }
            else {
                mkpath_3seg(qar, ex, eey, maxsx+h1, eey, maxsx+h1, sy, sx, sy);
            }
        }
        break;

    case LS_CURVE:
        if(focus) {
            if(dsdir>=0) {
                mkpath_segarcseg2(qar, sx, sy, maxsx, sy,
                   mx, ey, ex, ey);
            }
            else {
                mkpath_segarcseg2(qar, ex, ey, mx, ey,
                   maxsx, sy, sx, sy);
            }
            if(join) { mkpath_addbwcir(qar, mx, my); }
        }
        else {
            if(dsdir>=0) {

                mkpath_segarcseg2(qar, sx, sy, maxsx, sy, 
                   mx, eey, ex, eey);
            }
            else {
                mkpath_segarcseg2(qar, ex, eey, mx, eey,
                   maxsx, sy, sx, sy);
            }
        }
        break;

    case LS_ARC:
        if(focus) {
            if(dsdir>=0) {
                mkpath_segarcseg(qar, sx, sy, maxsx, sy, mx, sy,
                   mx, ey, ex, ey);
            }
            else {
                mkpath_segarcseg(qar, ex, ey, mx, ey, mx, sy,
                   maxsx, sy, sx, sy);
            }
            if(join) { mkpath_addbwcir(qar, mx, my); }
        }
        else {
            if(dsdir>=0) {
                mkpath_segarcseg(qar, sx, sy, maxsx, sy, mx, sy,
                   mx, eey, ex, eey);
            }
            else {
                mkpath_segarcseg(qar, ex, eey, mx, eey, mx, sy,
                   maxsx, sy, sx, sy);
            }
        }
        break;

    case LS_DIRECT:
    default:
        if(focus) {
            if(dsdir>=0) {
                mkpath_3seg(qar, sx, sy, maxsx, sy, mx, ey, ex, ey);
            }
            else {
                mkpath_3seg(qar, ex, ey, mx, ey, maxsx, sy, sx, sy);
            }
            if(join) { mkpath_addbwcir(qar, mx, my); }
        }
        else {
            mkpath_3seg(qar, sx, sy, maxsx, sy, mx, eey, ex, eey);
            if(dsdir>=0) {
                mkpath_3seg(qar, sx, sy, maxsx, sy, mx, eey, ex, eey);
            }
            else {
                mkpath_3seg(qar, ex, eey, mx, eey, maxsx, sy, sx, sy);
            }
        }
        break;
    }

out:
    return 0;
}
#undef QMB
#undef QM

#define QMB(zqx, zqy) \
    if(jr>0) { \
    fprintf(fp, "gsave newpath %d %d %d 0 360 arc stroke grestore\n", \
        zqx, zqy, jr*2); \
    }
#define QM(zqx, zqy) \
    if(jr>0) { \
    fprintf(fp, "gsave newpath %d %d %d 0 360 arc fill grestore\n", \
        zqx, zqy, jr); \
    }





/*
 *          (CHUNK)
 *            pb
 *           +--+ 
 *        pe |  |---+
 *           +--+   |    pb
 *           +--+   |   +--+
 *        pe |  |---+---|  |
 *           +--+   |   +--+
 *           +--+   |
 *        pe |  |---+
 *           +--+
 *
 *        pe is one of pb's object
 */

static
int
_drawgs(FILE *fp, int xdir, int xox, int xoy,
    ob *xu, ob *pf, ob *pb, ns *xns, int dsdir)
{
    ob   *pe;
    int   i;
    int   sx, sy;
    int   ex, ey;
    int   mx, my;
    int   minsx, maxsx;
    int   miny, maxy;
    int   mini, maxi;
    int   jr;
    int   am;
    int   j;
    int   k;
    int   eyt, eyb;
    int   cu, ce, cd, call;
    int   eex, eey;
    int   yp;
    int   g;
    varray_t *tmpar;

    int   usi, uei, esi, eei, dsi, dei;

    int   h1, h2, v;
    int   t1x, t1y, t2x, t2y;

    Echo("%s: xu %p oid %d, style\n", __func__, xu, xu->oid);


P;
    if(!pf || !pb) {
        goto out;
    }

    jr = xu->cob.outlinethick*2;

    /* array margin */
    am = 4*objunit/10;

    ex = xox + pf->cx - pf->cwd/2 * (dsdir);
    ey = xoy + pf->cy;

#if 0
    eyt = xoy + pf->cy + pf->cht/2;
    eyb = xoy + pf->cy - pf->cht/2;
#endif

#if 1
    eyt = xoy + pf->cy;
    eyb = xoy + pf->cy;
#endif
    minsx = INT_MAX;
    maxsx = -(INT_MAX-1);

    miny = INT_MAX;
    maxy = -(INT_MAX-1);

    mini = INT_MAX;
    maxi = -(INT_MAX-1);


    fprintf(fp, "    gsave\n");

    if(!ISCHUNK(pb->type)) {
        sx = xox + pb->cx + pb->cwd/2 *dsdir;
        sy = xoy + pb->cy;

        if(sy==ey) {
        }
            fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                sx, sy, ex, ey);
        goto final;
    }

    {
        changethick(fp, xu->cob.outlinethick);
        changecolor(fp, xu->cob.outlinecolor);
        cu = ce = cd = call = 0;
        for(i=0;i<pb->cch.nch;i++) {
            pe = (ob*)pb->cch.ch[i];
            if(EXVISIBLE(pe->type)||ISCHUNK(pe->type)) {
            }
            else {
                continue;
            }

            call++;
            if(i>maxi) maxi = i;
            if(i<mini) mini = i;
        }
        yp = pf->ht / (call+1);
        Echo("call %d yp %d\n", call, yp);

        usi = uei = esi = eei = dsi = dei = -1;

        j = 0;
        for(i=0;i<pb->cch.nch;i++) {
            pe = (ob*)pb->cch.ch[i];
            if(EXVISIBLE(pe->type)||ISCHUNK(pe->type)) {
            }
            else {
                continue;
            }

            sx = xox + pb->cx + pb->ox + pe->cx + pe->cwd/2 *dsdir;
            sy = xoy + pb->cy + pb->oy + pe->cy;

            if(sx>maxsx) maxsx = sx;
            if(sx<minsx) minsx = sx;
            if(sy>maxy) maxy = sy;
            if(sy<miny) miny = sy;

Echo(" sx i %d j %d sx %d minsx %d maxsx %d\n", i, j, sx, minsx, maxsx);

            eex = ex;
#if 0
            eey = ey+xu->ht/2-(j+1)*yp;
#endif
            eey = ey+pf->ht/2-(j+1)*yp;

Echo(" ag  j %d ; sx,sy %d,%d vs eey %d\n", j, sx, sy, eey);
#if 0
            fprintf(fp, "   newpath %d %d %d 0 360 arc stroke\n",
                eex, eey, jr);
#endif

            if(sy>eyt) {
#if 0
                fprintf(fp, "   newpath %d %d moveto (u) show\n",
                    eex+objunit/8, eey);
#endif
                cu++;
                if(usi<0) {
                    usi = j;
                }
                uei = j;
            }
            else
            if(sy<=eyt && sy>=eyb) {
#if 0
                fprintf(fp, "   newpath %d %d moveto (e) show\n",
                    eex+objunit/8, eey);
#endif
                ce++;
                if(esi<0) {
                    esi = j;
                }
                eei = j;
            }
            else {
#if 0
                fprintf(fp, "   newpath %d %d moveto (d) show\n",
                    eex+objunit/8, eey);
#endif
                cd++;
                if(dsi<0) {
                    dsi = j;
                }
                dei = j;
            }
    
            j++;
        }

        mx = xox+xu->cx;
        my = xoy+xu->cy;

        Echo("  sx dsdir %d minsx %d maxsx %d\n", dsdir, minsx, maxsx);
        if(dsdir==-1) {
            int dmy;
            dmy = minsx;
            maxsx = minsx;
            minsx = dmy;

        }
        else {
        }

        if(dsdir==-1) {
            maxsx -= am;
            if(mx-ex>am) {
                mx -= mx-ex-am;
            }
        }
        else {
            maxsx += am;
            if(ex-mx>am) {
                mx += ex-mx-am;
            }
        }

        Echo("  sx dsdir %d minsx %d maxsx %d\n", dsdir, minsx, maxsx);

        Echo("  cu %d ce %d cd %d\n", cu, ce, cd);
        Echo("  usi %d uei %d\n", usi, uei);
        Echo("  esi %d eei %d\n", esi, eei);
        Echo("  dsi %d dei %d\n", dsi, dei);

        j = 0;
        for(i=0;i<pb->cch.nch;i++) {
            g = 0;
            pe = (ob*)pb->cch.ch[i];
            if(EXVISIBLE(pe->type)||ISCHUNK(pe->type)) {
            }
            else {
                continue;
            }

            if(j>=usi && j<=uei) {
                k = uei - j;
                g = 1;
            }
            if(j>=esi && j<=eei) {
                k = j - esi;
#if 0
                g = 2;
#endif
                g = 7;
            }
            if(j>=dsi && j<=dei) {
                k = j - dsi;
                g = 3;
            }
            else {
            }
            Echo("i %d j %d: g %d k %d\n", i, j, g, k);

            eex = ex;
            eey = ey+pf->ht/2-(j+1)*yp;


            /*
             *         h1
             * sx,sy ----+ t1x,t1y
             *           | v
             *   t2x,t2y +---- ex,ey
             *            h2
             */ 
            sx = xox + pb->cx + pb->ox + pe->cx + pe->cwd/2 * dsdir;
            sy = xoy + pb->cy + pb->oy + pe->cy;

#if 0
            if(j>=esi && j<=eei)
#endif
            if(g==2)
            {
                fprintf(fp, "   %d %d moveto %d 0 rlineto stroke\n",
                        sx, eey, ex-sx
                        );
            }
            else {
                h1 = yp*(k+1) * dsdir;
                h2 = (ex-maxsx) - h1;
                v  = (eey-sy);

                Echo("  h1 %7d v %7d h2 %7d\n", h1, v, h2);

                tmpar = varray_new();
                varray_entrysprintfunc(tmpar, seg_sprintf);
Echo("%s: tmpar %p xu %p oid %d segar %p\n",
    __func__, tmpar, xu, xu->oid, xu->cob.segar);

#if 0
fprintf(fp,"%% sx,sy %d,%d maxsx %d mx,my %d,%d ex,ey %d,%d eey %d dsdir %d\n",
    sx, sy, maxsx, xox+xu->cx, xoy+xu->cy, ex, ey, eey, dsdir);
#endif

                _drawgslink(tmpar, xu->oid, xu->cob.linkstyle,
                    xu->cob.outlinethick*2,
                    j, call, h1, h2, v,
                    sx, sy, maxsx,
                    mx, my, ex, ey, eey, dsdir);

#if 0
                varray_fprintv(stdout, tmpar);
#endif

Echo("%s: tmpar %p use %d\n", __func__, tmpar, tmpar->use);
                if(tmpar->use>0) {
                    /*** NOTE offset is cared already. do not applay twice */
                    __drawpath_LT(fp, 0, 0, 0, xu, xns, tmpar);
                }
#if 0
                varray_del(tmpar);
#endif
            
                tmpar = NULL;
            }



            j++;
        }

    }

final:
    fprintf(fp, "     grestore\n");

out:
    return 0;
}

int
epsdraw_scatter(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns)
{
    ob *pf, *pb;
    int ik;

P;
    pf = (ob*)xu->cob.linkfore;
    pb = (ob*)xu->cob.linkback;
    fprintf(fp, "%% pf %p, pb %p\n", pf, pb);
    Echo("%% pf %p, pb %p\n", pf, pb);

#if 0
    if(xu->cob.linkmap) {
        Echo("%s: oid %d linkmap '%s'\n",
            __func__, xu->oid, xu->cob.linkmap);
    }
#endif

    if(!pf || !pb) {
        if(!pf) { Error("scatter oid %d; no fore object\n", xu->oid); }
        if(!pb) { Error("scatter oid %d; no back object\n", xu->oid); }
        goto out;
    }

    ik = _drawgs(fp, xdir, xox, xoy, xu, pb, pf, xns, -1);

out:
    return 0;
}

int
epsdraw_gather(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns)
{
    ob *pf, *pb;
    int ik;

P;
    Echo("%s: xu %p oid %d\n", __func__, xu, xu->oid);

    pf = (ob*)xu->cob.linkfore;
    pb = (ob*)xu->cob.linkback;
    fprintf(fp, "%% pf %p, pb %p\n", pf, pb);
    Echo("%% pf %p, pb %p\n", pf, pb);

    if(!pf || !pb) {
        if(!pf) { Error("gather oid %d; no fore object\n", xu->oid); }
        if(!pb) { Error("gather oid %d; no back object\n", xu->oid); }
        goto out;
    }

    ik = _drawgs(fp, xdir, xox, xoy, xu, pf, pb, xns, 1);

out:
    return 0;
}

int parse_aheads(char*,int*,int*,int*,int*, int);

static
int
_parse_o_n(char *ss, int *ro, int *rn)
{
    char *u;
    u = ss;
    if(*u=='b'||*u=='B') { *ro = AR_BACK; u++; }
    else
    if(*u=='f'||*u=='F') { *ro = AR_FORE; u++; }
Info(" u '%s'\n", u);
    *rn = atoi(u);
Echo("%s: ss '%s' -> %d %d\n", __func__, ss, *ro, *rn);
    return 0;
}

static
int
solve_se(char *tg,
    int *rso, int* rsn, int *reo, int* ren,
    int *rss, int* rat, int *raf, int *rac, int *rab)
{
    int   rv;
    char  tmp[BUFSIZ];
    char *p;
    char *q;
    char *u;
    int   c;
    int   ss;
    int   xso, xeo;
    int   xs, xe, xa;
    int   xf, xc, xb;

    rv = -1;
    xs = xe = xa = -1;
    xf = xc = xb = -1;


    ss = assoc(linetype_ial, tg);
    Echo("ss %d\n", ss);
    if(ss>=0) {
        *rss = ss;
        return 2;
    }

    p = tg;

    q = tmp;
    c = 0;
    while(p && *p && c<BUFSIZ &&
        ((*p>='0'&&*p<='9')||(*p=='b'||*p=='B')||(*p=='f'||*p=='F')) ) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    Info("  tmp '%s'\n", tmp);
    if(!tmp[0]) {
        goto out;
    }
    _parse_o_n(tmp, &xso, &xs);
    Info("  xso %d xs %d\n", xso, xs);

    q = tmp;
    c = 0;
    while(p && *p && (*p==':'||*p=='-' || *p=='>' || *p=='<') && c<BUFSIZ) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    Info("  tmp '%s'\n", tmp);
    if(!tmp[0]) {
        goto out;
    }
    if(tmp[0]!=':') { 
        int ik;
        ik = parse_aheads(tmp, &xa, &xf, &xc, &xb, 0);
        if(ik==0) {
            xa = -1;
        }
    }

    q = tmp;
    c = 0;
    while(p && *p && c<BUFSIZ &&
        ((*p>='0'&&*p<='9')||(*p=='b'||*p=='B')||(*p=='f'||*p=='F')) ) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    Info("  tmp '%s'\n", tmp);
    if(!tmp[0]) {
        goto out;
    }
    _parse_o_n(tmp, &xeo, &xe);
    Info("  xeo %d xe %d\n", xeo, xe);

    Echo("xso %d xs %d ; xeo %d xe %d ; xa %d\n", xso, xs, xeo, xe, xa);
    fflush(stdout);

    *rso = xso;
    *reo = xeo;

    *rsn = xs;
    *ren = xe;
    *rat = xa;
    *raf = xf;
    *rac = xc;
    *rab = xb;

    rv = 1;

out:
    return rv;
}

#ifndef FBSLOT_LEN
#define FBSLOT_LEN (128)
#endif


typedef struct {
    int so, sn;
    int eo, en;
    int x1, y1;
    int x2, y2;
    int x3, y3;
    int x4, y4;
    ob prop;
} clink_t;

int
clink_fprint(FILE *fp, clink_t *par, int np)
{
    int i;
    char os, es;

    printf("par %p np %d\n", par, np);
    for(i=0;i<np;i++) {
        os = '-';
        if(par[i].so==AR_FORE) { os = 'F'; }
        if(par[i].so==AR_BACK) { os = 'B'; }
        es = '-';
        if(par[i].eo==AR_FORE) { es = 'F'; }
        if(par[i].eo==AR_BACK) { es = 'B'; }

        printf("    i %d: %c %d:%d %c %d:%d %d,%d %d,%d; ss %d at %d f/c/b %d %d %d\n",
            i, 
            os, par[i].so, par[i].sn,
            es, par[i].eo, par[i].en,
            par[i].x1, par[i].y1,
            par[i].x2, par[i].y2,
            par[i].prop.cob.outlinetype,
            par[i].prop.cob.arrowheadpart,
            par[i].prop.cob.arrowforeheadtype,
            par[i].prop.cob.arrowcentheadtype,
            par[i].prop.cob.arrowbackheadtype);
    }   

    return 0;
}

int
epsdraw_thruX(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns)
{
    ob *pf, *pb;
    ob *fs[FBSLOT_LEN];
    ob *bs[FBSLOT_LEN];
    ob *pe, *se;
    int ik;
    int i, j;
    int u, v;
    int am;
    int cm;
    varray_t *tmpar;

    int bxmax, fxmin;

    am = 4*objunit/10;
    cm = 2*objunit/10;
    bxmax = -(INT_MAX-1);
    fxmin = INT_MAX;

    Echo("%s: oid %d\n", __func__, xu->oid);
P;
    pf = (ob*)xu->cob.linkfore;
    pb = (ob*)xu->cob.linkback;
    fprintf(fp, "%% pf %p, pb %p\n", pf, pb);
    Echo("%% pf %p, pb %p\n", pf, pb);

    if(!pf || !pb) {
        goto out;
    }
    Echo("pf %p oid %d %d\n", pf, pf->oid, pf->type);
    Echo("pb %p oid %d %d\n", pb, pb->oid, pb->type);

    memset(fs, 0, sizeof(fs));
    memset(bs, 0, sizeof(bs));

    u = 0;
    for(i=0;i<pb->cch.nch;i++) {
        pe = (ob*)pb->cch.ch[i];
        if(EXVISIBLE(pe->type)||ISCHUNK(pe->type)) {
        }
        else {
            continue;
        }
        u++;
        bs[u] = pe;
        Echo("i,u %2d %2d: %p oid %d %d\n", i, u, pe, pe->oid, pe->type);
        if(pe->gx + pe->cwd/2 > bxmax) {
            bxmax = pe->gx + pe->cwd/2;
        }
    }

    v = 0;
    for(j=0;j<pf->cch.nch;j++) {
        se = (ob*)pf->cch.ch[j];
        if(EXVISIBLE(se->type)||ISCHUNK(se->type)) {
        }
        else {
            continue;
        }
        v++;
        fs[v] = se;
        Echo("j,v %2d %2d: %p oid %d %d\n", j, v, se, se->oid, se->type);
        if(se->gx - se->cwd/2 < fxmin) {
            fxmin = se->gx - se->cwd/2;
        }
    }
    Echo(" u %d v %d\n", u, v);
    Echo(" bxmax %d fxmin %d\n", bxmax, fxmin);
    fflush(stdout);

    for(i=0;i<u;i++) {
        if(!bs[i]) {
            Echo("b %3d: %p\n", i, bs[i]);
        }
        else {
            Echo("b %3d: %p oid %d type %d\n",
                i, bs[i], bs[i]->oid, bs[i]->type);
        }
    }
    for(j=0;j<v;j++) {
        if(!fs[j]) {
            Echo("f %3d: %p\n", j, fs[j]);
        }
        else {
            Echo("f %3d: %p oid %d type %d\n",
                j, fs[j], fs[j]->oid, fs[j]->type);
        }
    }

    if(xu->cob.markguide)
    {
    int x1,y1;
    int fht;
    fprintf(fp, "      gsave\n");

    /* TEMP */
    fht = def_textheight;
    fprintf(fp, "      /%s findfont %d scalefont setfont %% thru backport\n",
        def_fontname, fht);

    for(i=0;i<=u;i++) {
        pe = bs[i];
        if(!pe) {
            continue;
        }
        x1 = pe->gx+pe->cwd/2;
        y1 = pe->gy;
        fprintf(fp, "      %d %d %d (%d) rrshow\n",
            x1, y1, 0, i);
    }

    for(j=0;j<=v;j++) {
        pe = fs[j];
        if(!pe) {
            continue;
        }
        x1 = pe->gx-pe->cwd/2;
        y1 = pe->gy;
        fprintf(fp, "      %d %d %d (%d) lrshow\n",
            x1, y1, 0, j);
    }

    fprintf(fp, "      grestore\n");
    }


    if(xu->cob.linkmap) {
        Echo("%s: oid %d linkmap '%s'\n",
            __func__, xu->oid, xu->cob.linkmap);
    }
    else {
        Echo("%s: oid %d no linkmap\n",
            __func__, xu->oid);
    }
    
 {
    char *p;
    char token[BUFSIZ];
    char cmap[BUFSIZ];
    int  ss;
    int  so, sn, eo, en, at;
    int  af, ac, ab;
    int  ik;
    int  cs;

    cmap[0] = '\0';

    if(xu->cob.linkmap) {
        strcpy(cmap, xu->cob.linkmap);
    }
    else {
        char w[BUFSIZ];
        int m, h;
        m = (u<v ? u : v); /* MIN */
        
        strcpy(cmap, "solid");
        for(h=0;h<=m;h++) {
            sprintf(w, ",b%d:f%d", h, h);
            strcat(cmap, w);
        }
    }
    Echo(" oid %d cmap '%s'\n", xu->oid, cmap);
    p = cmap;
    ss = xu->cob.outlinetype;
    at = xu->cob.arrowheadpart;
    af = xu->cob.arrowforeheadtype;
    ac = xu->cob.arrowcentheadtype;
    ab = xu->cob.arrowbackheadtype;
                Echo("g ss %d at %d af %d ac %d ab %d\n",
                    ss, at, af, ac, ab);

#if 0
    ob** par;
    par = (ob**)alloca(sizeof(ob*)*(u+v));
#endif

    int npar;
    clink_t *par;
    int i;

    par = (clink_t*)alloca(sizeof(clink_t)*(u+v));
    if(par) {
        memset(par, 0, sizeof(clink_t)*(u+v));
    }

    npar = 0;
    for(i=0;i<u+v;i++) {
        par[i].prop.cob.outlinetype     = xu->cob.outlinetype;
        par[i].prop.cob.arrowheadpart   = xu->cob.arrowheadpart;
        par[i].prop.cob.arrowforeheadtype = xu->cob.arrowforeheadtype;
        par[i].prop.cob.arrowcentheadtype = xu->cob.arrowcentheadtype;
        par[i].prop.cob.arrowbackheadtype = xu->cob.arrowbackheadtype;
    }   

    if(INTRACE) {
        fprintf(stdout, "npar %d\n", npar);
        clink_fprint(stdout, par, u+v);
        clink_fprint(stdout, par, npar);
    }

    while(*p) {
        p = draw_word(p, token, BUFSIZ, MAP_SEPC);
        Echo("token '%s'\n", token);
        if(!token[0]) {
            continue;
        }
        ik = solve_se(token,
                &so, &sn, &eo, &en, &ss, &at, &af, &ac, &ab);    
        Echo("  ik %d; so %d sn %d eo %d en %d; ss %d at %d af %d ac %d ab %d\n",
            ik, so, sn, eo, en, ss, at, af, ac, ab);
        fflush(stdout);

        if(ik==1) {
            int x1, y1, x2, y2, x3, y3, x4, y4;
            if(so==AR_BACK) { pe = bs[sn]; } else { pe = fs[sn]; }
            if(eo==AR_BACK) { se = bs[en]; } else { se = fs[en]; }

            if(pe && se) {
                if(so==AR_BACK) {
                    x1 = pe->gx + pe->cwd/2;
                    y1 = pe->gy;
                    x3 = bxmax + am;
                    y3 = pe->gy;
                }
                else {
                    x1 = pe->gx - pe->cwd/2;
                    y1 = pe->gy;
                    x3 = fxmin - am;
                    y3 = pe->gy;
                }
                if(eo==AR_FORE) {
                    x2 = se->gx - se->cwd/2;
                    y2 = se->gy;
                    x4 = fxmin - am;
                    y4 = se->gy;
                }
                else {
                    x2 = se->gx + se->cwd/2;
                    y2 = se->gy;
                    x4 = bxmax + am;
                    y4 = se->gy;
                }


                Echo("b ss %d at %d af %d ac %d ab %d\n",
                    ss, at, af, ac, ab);
                if(at<0) {  
                    ss = xu->cob.outlinetype;
                    at = xu->cob.arrowheadpart;
                    af = xu->cob.arrowforeheadtype;
                    ac = xu->cob.arrowcentheadtype;
                    ab = xu->cob.arrowbackheadtype;
                }
                Echo("a ss %d at %d af %d ac %d ab %d\n",
                    ss, at, af, ac, ab);

#if 0
                fprintf(fp, "newpath %d %d moveto %d %d %d 0 360 arc fill\n",
                    x3, y3, x3, y3, objunit/20);
                fprintf(fp, "newpath %d %d moveto %d %d %d 0 360 arc fill\n",
                    x4, y4, x4, y4, objunit/20);
#endif

{
                tmpar = varray_new();
                varray_entrysprintfunc(tmpar, seg_sprintf);

#if 0
                varray_fprintv(stdout, tmpar);
#endif

                path_regsegmoveto(tmpar, x1, y1);
                path_regseglineto(tmpar, x3, y3);
                if(x3==x4) {
                    if(so==AR_BACK) {
                        path_regsegcurveto(tmpar, x3+am, y3, x4+am, y4, x4, y4);
                    }
                    else {
                        path_regsegcurveto(tmpar, x3-am, y3, x4-am, y4, x4, y4);
                    }
                }
                else {
                    if(so==AR_BACK) {
                        path_regsegcurveto(tmpar, x3+cm, y3, x4-cm, y4, x4, y4);
                    }
                    else {
                        path_regsegcurveto(tmpar, x3-cm, y3, x4+cm, y4, x4, y4);
                    }
                }
                path_regseglineto(tmpar, x2, y2);

                if(tmpar->use>0) {
                    /*** NOTE offset is cared already. do not applay twice */
fprintf(fp, "%% thruX token '%s'\n", token);
                    __drawpath_LT(fp, 0, 0, 0, xu, xns, tmpar);
                }

    
}


                if(npar<u+v) {
Echo("ADD\n");
                    par[npar].so = so;
                    par[npar].sn = sn;
                    par[npar].eo = eo;
                    par[npar].en = en;
                    par[npar].x1 = x1;
                    par[npar].y1 = y1;
                    par[npar].x2 = x2;
                    par[npar].y2 = y2;
                    par[npar].x3 = x3;
                    par[npar].y3 = y3;
                    par[npar].x4 = x4;
                    par[npar].y4 = y4;

#if 1
                    par[npar].prop.cob.outlinetype = ss;
                    par[npar].prop.cob.arrowheadpart = at;
                    par[npar].prop.cob.arrowforeheadtype = af;
                    par[npar].prop.cob.arrowcentheadtype = ac;
                    par[npar].prop.cob.arrowbackheadtype = ab;
#endif

#if 0
                    par[npar].prop.cob.outlinetype = xu->cob.outlinetype;
                    par[npar].prop.cob.arrowheadpart = xu->cob.arrowheadpart;
                    par[npar].prop.cob.arrowforeheadtype = xu->cob.arrowforeheadtype;
                    par[npar].prop.cob.arrowcentheadtype = xu->cob.arrowcentheadtype;
                    par[npar].prop.cob.arrowbackheadtype = xu->cob.arrowbackheadtype;
#endif


                    npar++;

    if(INTRACE) {
        fprintf(stdout, "npar %d\n", npar);
        clink_fprint(stdout, par, u+v);
        clink_fprint(stdout, par, npar);
    }
                }
            }
            else {
                Error("ignore position in '%s'; recognized objects back %d fore %d\n",
                    token, u, v);
            }
        }
        if(ik==2) {
#if 0
            cs = ss;
#endif
        }
    }


    if(INTRACE) {
        fprintf(stdout, "npar %d\n", npar);
        clink_fprint(stdout, par, u+v);
        clink_fprint(stdout, par, npar);
    }

 }

out:
    return 0;
}


int
epsdraw_thru(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns)
{
    ob *pf, *pb;
    ob *fs[FBSLOT_LEN];
    ob *bs[FBSLOT_LEN];
    ob *pe, *se;
    int ik;
    int i, j;
    int u, v;

P;
    pf = (ob*)xu->cob.linkfore;
    pb = (ob*)xu->cob.linkback;
    fprintf(fp, "%% pf %p, pb %p\n", pf, pb);
    Echo("%% pf %p, pb %p\n", pf, pb);

    if(!pf || !pb) {
        goto out;
    }
    Echo("pf %p oid %d %d\n", pf, pf->oid, pf->type);
    Echo("pb %p oid %d %d\n", pb, pb->oid, pb->type);

    memset(fs, 0, sizeof(fs));
    memset(bs, 0, sizeof(bs));

    u = 0;
    for(i=0;i<pb->cch.nch;i++) {
        pe = (ob*)pb->cch.ch[i];
        if(EXVISIBLE(pe->type)||ISCHUNK(pe->type)) {
        }
        else {
            continue;
        }
        u++;
        bs[u] = pe;
        Echo("i,u %2d %2d: %p oid %d %d\n", i, u, pe, pe->oid, pe->type);
    }

    v = 0;
    for(j=0;j<pf->cch.nch;j++) {
        se = (ob*)pf->cch.ch[j];
        if(EXVISIBLE(se->type)||ISCHUNK(se->type)) {
        }
        else {
            continue;
        }
        v++;
        fs[v] = se;
        Echo("j,v %2d %2d: %p oid %d %d\n", j, v, se, se->oid, se->type);
    }
    Echo(" u %d v %d\n", u, v);
    fflush(stdout);

    for(i=0;i<u;i++) {
        if(!bs[i]) {
            Echo("b %3d: %p\n", i, bs[i]);
        }
        else {
            Echo("b %3d: %p oid %d type %d\n",
                i, bs[i], bs[i]->oid, bs[i]->type);
        }
    }
    for(j=0;j<v;j++) {
        if(!fs[j]) {
            Echo("f %3d: %p\n", j, fs[j]);
        }
        else {
            Echo("f %3d: %p oid %d type %d\n",
                j, fs[j], fs[j]->oid, fs[j]->type);
        }
    }

    if(xu->cob.markguide)
    {
    int x1,y1;
    int fht;
    fprintf(fp, "      gsave\n");

    /* TEMP */
    fht = def_textheight;
    fprintf(fp, "      /%s findfont %d scalefont setfont %% thru backport\n",
        def_fontname, fht);

    for(i=0;i<=u;i++) {
        pe = bs[i];
        if(!pe) {
            continue;
        }
        x1 = pe->gx+pe->cwd/2;
        y1 = pe->gy;
        fprintf(fp, "      %d %d %d (%d) rrshow\n",
            x1, y1, 0, i);
    }

    for(j=0;j<=v;j++) {
        pe = fs[j];
        if(!pe) {
            continue;
        }
        x1 = pe->gx-pe->cwd/2;
        y1 = pe->gy;
        fprintf(fp, "      %d %d %d (%d) lrshow\n",
            x1, y1, 0, j);
    }

    fprintf(fp, "      grestore\n");
    }


    if(xu->cob.linkmap) {
        Echo("%s: oid %d linkmap '%s'\n",
            __func__, xu->oid, xu->cob.linkmap);
    }
    else {
        Echo("%s: oid %d no linkmap\n",
            __func__, xu->oid);
    }
    
 {
    char *p;
    char token[BUFSIZ];
    char cmap[BUFSIZ];
    int  ss;
    int  so, sn, eo, en, at;
    int  af, ac, ab;
    int  ik;
    int  cs;

    cmap[0] = '\0';

    if(xu->cob.linkmap) {
        strcpy(cmap, xu->cob.linkmap);
    }
    else {
        char w[BUFSIZ];
        int m, h;
        m = (u<v ? u : v); /* MIN */
        
        strcpy(cmap, "solid");
        for(h=0;h<=m;h++) {
            sprintf(w, ",%d:%d", h, h);
            strcat(cmap, w);
        }
    }
    Echo(" oid %d cmap '%s'\n", xu->oid, cmap);
    p = cmap;
    ss = xu->cob.outlinetype;
    at = xu->cob.arrowheadpart;
    af = xu->cob.arrowforeheadtype;
    ac = xu->cob.arrowcentheadtype;
    ab = xu->cob.arrowbackheadtype;
                Echo("g ss %d at %d af %d ac %d ab %d\n",
                    ss, at, af, ac, ab);
    while(*p) {
#if 0
        sn = en = at = -1;
        af = ac = ab = -1;
#endif
        p = draw_word(p, token, BUFSIZ, MAP_SEPC); 
        Echo("token '%s'\n", token);
        if(!token[0]) {
            continue;
        }
        ik = solve_se(token, &ss, &so, &sn, &eo, &en, &at, &af, &ac, &ab);    
        Echo("  ik %d; ss %d sn %d en %d at %d af %d ac %d ab %d\n",
            ik, ss, sn, en, at, af, ac, ab);
        fflush(stdout);

        if(ik==1) {
            int x1, y1, x2, y2;
            if(bs[sn] && fs[en]) {
                x1 = bs[sn]->gx+bs[sn]->cwd/2;
                y1 = bs[sn]->gy;
                x2 = fs[en]->gx-fs[sn]->cwd/2;
                y2 = fs[en]->gy;
                Echo("b ss %d at %d af %d ac %d ab %d\n",
                    ss, at, af, ac, ab);
                if(at<0) {  
                    ss = xu->cob.outlinetype;
                    at = xu->cob.arrowheadpart;
                    af = xu->cob.arrowforeheadtype;
                    af = xu->cob.arrowforeheadtype;
                    ac = xu->cob.arrowcentheadtype;
                    ab = xu->cob.arrowbackheadtype;
                }
                Echo("a ss %d at %d af %d ac %d ab %d\n",
                    ss, at, af, ac, ab);
#if 0
                epsdraw_Xseglinearrow(fp, xox, xoy,
                    x1, y1, x2, y2,
                    ss, objunit/50, 0,
                    at, af, ac, ab);
#endif
                changecolor(fp, xu->cob.outlinecolor);
                changethick(fp, xu->cob.outlinethick);
                epsdraw_Xseglinearrow(fp, xox, xoy,
                    x1, y1, x2, y2,
                    ss, -1, -1,
                    at, af, ac, ab);
            }
        }
        if(ik==2) {
#if 0
            cs = ss;
#endif
        }
    
    }
 }

out:
    return 0;
}





int
epsdraw_paren(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    int x1, y1;
    int r;
    double a;
    int aw, ah;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;
    r  = (xu->wd/2);
    a  = ((double)xu->wd)/((double)xu->ht);

    if(xu->cob.imargin>0) {
        aw = xu->wd - xu->cob.imargin*2;
        ah = xu->ht - xu->cob.imargin*2;
    }
    else {
        aw = xu->wd;
        ah = xu->ht;
    }

apply:

    fprintf(fp, "%% box xy %d,%d wh %dx%d\n", x1, y1, aw, ah);
    fprintf(fp, "gsave %% for paren\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);

    double cx1, cx2;
    double r1, r2;
    double k = 0.7;
    double th1, th2;


    fprintf(fp, "%% frame\n");
    fprintf(fp, "%%     outline color %d thick %d\n",
        xu->cob.outlinecolor, xu->cob.outlinethick);

    if(xu->cob.outlinecolor>=0 && xu->cob.outlinethick>0) {
        fprintf(fp, "gsave\n");
        changecolor(fp, xu->cob.outlinecolor);
        changethick(fp, xu->cob.outlinethick);
        fprintf(fp, "  %d %d translate\n", x1, y1);
        fprintf(fp, "  newpath\n");
        fprintf(fp, "  gsave\n");
        fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
        fprintf(fp, "    newpath\n");

        if(xu->type==CMD_RPAREN) {
            fprintf(fp, " 180 rotate\n");
        }
        cx1 = ((double)ah*ah)/(8.0*aw);
        r1  = aw/2+((double)ah*ah)/(8.0*aw);
        cx2 = ((double)ah*ah)/(8.0*k*aw);
        r2  = (k*aw-aw/2)+((double)ah*ah)/(8.0*k*aw);

        th1 = atan2(ah/2, -(cx1-aw/2))/rf;
        th2 = atan2(ah/2, -(cx2-aw/2))/rf;
fprintf(fp,
  "%% L cx1 %.3f r1 %.3f th1 %.4f cx2 %.3f r2 %.3f th2 %.4f\n",
        cx1, r1, th1, cx2, r2, th2);

        fprintf(fp, "    %.3f 0 %.3f %.3f %.3f arc\n",
            cx1, r1, th1, -th1);
        fprintf(fp, "    %.3f 0 %.3f %.3f %.3f arcn\n",
            cx2, r2, -th2, th2);
        fprintf(fp, "    stroke\n");

        fprintf(fp, "    %.3f 0 %.3f %.3f %.3f arc\n",
            cx1, r1, th1, -th1);
            fprintf(fp, "    %.3f 0 %.3f %.3f %.3f arcn\n",
                cx2, r2, -th2, th2);

        fprintf(fp, "    closepath\n");
        fprintf(fp, "    fill\n");
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of paren\n");

out:
    return 0;
}

int
epsdraw_brace(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    int bx, by;
    int r;
    double a;
    int aw, ah;

    int r1, r2;
    int y1, y2;
    int g;

    bx = xox+xu->cx;
    by = xoy+xu->cy;
    r  = (xu->wd/2);
    a  = ((double)xu->wd)/((double)xu->ht);

    g  = xu->wd/4;
    r1 = xu->wd/2 - g/2;
    r2 = xu->wd/2 + g/2;
    y1 = xu->ht/2 - r1;
    y2 = xu->ht/2 - r2;

    if(xu->cob.imargin>0) {
        aw = xu->wd - xu->cob.imargin*2;
        ah = xu->ht - xu->cob.imargin*2;
    }
    else {
        aw = xu->wd;
        ah = xu->ht;
    }

apply:

    fprintf(fp, "%% box xy %d,%d wh %dx%d\n", bx, by, aw, ah);
    fprintf(fp, "gsave %% for brace\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);

    if(xu->cob.outlinecolor>=0 && xu->cob.outlinethick>0) {
        fprintf(fp, "gsave\n");
        changecolor(fp, xu->cob.outlinecolor);
        changethick(fp, xu->cob.outlinethick);
        fprintf(fp, "  %d %d translate\n", bx, by);
        fprintf(fp, "  newpath\n");
        fprintf(fp, "  gsave\n");
        fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
        fprintf(fp, "    newpath\n");

        if(xu->type==CMD_RBRACE) {
            /* R part -- rotate and draw L part */
            fprintf(fp, " 180 rotate\n");
        }
    
        /* L part */
        fprintf(fp, "    %d %d moveto\n",   aw/2,  ah/2);
        fprintf(fp, "    %d %d %d %d %d arc\n",   aw/2,  y2, r2, 90, 180);
        fprintf(fp, "    %d %d rlineto\n",  0, -(ah/2-r1-r2));
        fprintf(fp, "    %d %d %d %d %d arcn\n",  -aw/2,  r1, r1, 0, -90);

        fprintf(fp, "    %d %d %d %d %d arcn\n",  -aw/2, -r1, r1, 90, 0);
        fprintf(fp, "    %d %d rlineto\n",  0, -(ah/2-r1-r2));
        fprintf(fp, "    %d %d %d %d %d arc\n",   aw/2, -y2, r2, 180, 270);

        fprintf(fp, "    %d %d %d %d %d arcn\n",  aw/2, -y1, r1, 270,180);
        fprintf(fp, "    %d %d rlineto\n",  0, (ah/2-r1-r2));
        fprintf(fp, "    %d %d %d %d %d arc\n",  -aw/2, -r2, r2, 0, 90);

        fprintf(fp, "    %d %d %d %d %d arc\n",  -aw/2, r2, r2, -90, 0);
        fprintf(fp, "    %d %d rlineto\n",  0, (ah/2-r1-r2));
        fprintf(fp, "    %d %d %d %d %d arcn\n",  aw/2, y1, r1, 180, 90);

        fprintf(fp, "    closepath\n");
        fprintf(fp, "    fill\n");

        fprintf(fp, "    %d %d moveto\n",   aw/2,  ah/2);
        fprintf(fp, "    %d %d %d %d %d arc\n",   aw/2,  y2, r2, 90, 180);
        fprintf(fp, "    %d %d rlineto\n",  0, -(ah/2-r1-r2));
        fprintf(fp, "    %d %d %d %d %d arcn\n",  -aw/2,  r1, r1, 0, -90);

        fprintf(fp, "    %d %d %d %d %d arcn\n",  -aw/2, -r1, r1, 90, 0);
        fprintf(fp, "    %d %d rlineto\n",  0, -(ah/2-r1-r2));
        fprintf(fp, "    %d %d %d %d %d arc\n",   aw/2, -y2, r2, 180, 270);

        fprintf(fp, "    stroke\n");

        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of brace\n");

out:
    return 0;
}



int
epsdraw_bracket(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int r;
    double a;
    int aw, ah;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;
    r  = (xu->wd/2);
    a  = ((double)xu->wd)/((double)xu->ht);

    if(xu->cob.imargin>0) {
        aw = xu->wd - xu->cob.imargin*2;
        ah = xu->ht - xu->cob.imargin*2;
    }
    else {
        aw = xu->wd;
        ah = xu->ht;
    }

    fprintf(fp, "%% box xy %d,%d wh %dx%d\n", x1, y1, aw, ah);
    fprintf(fp, "gsave %% for bracket\n");

    fprintf(fp, "%% frame\n");
    fprintf(fp, "%%     outline color %d thick %d\n",
        xu->cob.outlinecolor, xu->cob.outlinethick);

    if(xu->cob.outlinecolor>=0 && xu->cob.outlinethick>0) {
        fprintf(fp, "gsave\n");
        changecolor(fp, xu->cob.outlinecolor);
        changethick(fp, xu->cob.outlinethick);
        fprintf(fp, "  %d %d translate\n", x1, y1);
        fprintf(fp, "  newpath\n");
        fprintf(fp, "  gsave\n");
        fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
        if(xu->type==CMD_RBRACKET) {
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    %d %d moveto\n",  aw/4, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw/4, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", -aw/4, 0);
            fprintf(fp, "    fill\n");

            fprintf(fp, "    %d %d moveto\n",  -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", -aw, 0);
            fprintf(fp, "    stroke\n");
        }
        else {
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    %d %d moveto\n",  -aw/4, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", -aw/4, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", aw/4, 0);
            fprintf(fp, "    fill\n");

            fprintf(fp, "    %d %d moveto\n",  aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", -aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    stroke\n");
        }
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of braket\n");

out:
    return 0;
}


int
epsdraw_sep(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
P;
    fprintf(fp, "%% sep oid %d\n", xu->oid);
    fprintf(fp, "gsave    %% for sep\n");


#if 0
    epsdraw_plinearrowR(fp, xu->cob.sepcurdir, xox, xoy, 1000, xu, xns);
#endif

Echo("%s: oid %d xox,xoy %d,%d\n", __func__, xu->oid, xox, xoy);
Echo("    cx,cy %d,%d\n",
                    xu->cx, xu->cy);
Echo("    x1,y1 %d,%d x2,y2 %d,%d\n", 
                    xu->cob.sepx1, xu->cob.sepy1, 
                    xu->cob.sepx2, xu->cob.sepy2);
Echo("    gx1,y1 %d,%d gx2,y2 %d,%d\n", 
                    xu->cob.gsepx1, xu->cob.gsepy1, 
                    xu->cob.gsepx2, xu->cob.gsepy2);
Echo("    xox,xoy+x1,y1 %d,%d gx2,y2 %d,%d\n", 
                    xox+xu->cob.sepx1, xoy+xu->cob.sepy1, 
                    xox+xu->cob.sepx2, xoy+xu->cob.sepy2);
Echo("    xox,xoy+gx1,y1 %d,%d gx2,y2 %d,%d\n", 
                    xox+xu->cob.gsepx1, xoy+xu->cob.gsepy1, 
                    xox+xu->cob.gsepx2, xoy+xu->cob.gsepy2);

    fprintf(fp, "  gsave\n");
    changethick(fp, xu->cob.outlinethick);
    changecolor(fp, xu->cob.outlinecolor);
#if 0
    fprintf(fp, "  7 setlinewidth\n");
    fprintf(fp, "  1 0 0 setrgbcolor\n");
    fprintf(fp, "  %d %d moveto %d %d 5 0 360 arc\n",
                    xox+xu->cx, xoy+xu->cy,
                    xox+xu->cx, xoy+xu->cy);
    fprintf(fp, "  3 setlinewidth\n");
    fprintf(fp, "  1 0 0 setrgbcolor\n");
    fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                    xu->cob.sepx1, xu->cob.sepy1, 
                    xu->cob.sepx2, xu->cob.sepy2);
    fprintf(fp, "  0 1 0 setrgbcolor\n");
    fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                    xu->cob.gsepx1, xu->cob.gsepy1, 
                    xu->cob.gsepx2, xu->cob.gsepy2);
#endif
#if 0
    fprintf(fp, "  1 1 0 setrgbcolor\n");
    fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                    xox+xu->cob.sepx1, xoy+xu->cob.sepy1, 
                    xox+xu->cob.sepx2, xoy+xu->cob.sepy2);
#endif
#if 0
    fprintf(fp, "  0 0 1 setrgbcolor\n");
    fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                    xox+xu->cob.gsepx1, xoy+xu->cob.gsepy1, 
                    xox+xu->cob.gsepx2, xoy+xu->cob.gsepy2);
#endif

#if 0
    epsdraw_Xseglinearrow(fp, xox, xoy,
                    xox+xu->cob.gsepx1, xoy+xu->cob.gsepy1, 
                    xox+xu->cob.gsepx2, xoy+xu->cob.gsepy2,
                    xu->cob.outlinetype,
                    xu->cob.outlinethick,
                    xu->cob.outlinecolor, 0, 0, 0, 0);
#endif

    epsdraw_Xseglinearrow(fp, xox, xoy,
                    xu->cob.gsepx1, xu->cob.gsepy1, 
                    xu->cob.gsepx2, xu->cob.gsepy2,
                    xu->cob.outlinetype,
                    xu->cob.outlinethick,
                    xu->cob.outlinecolor, 0, 0, 0, 0);

    fprintf(fp, "  grestore\n");

    fprintf(fp, "grestore %% for sep\n");
    
    return 0;
}


        
int
epsdraw_mark(FILE *fp, int gx, int gy, int ty)
{
    int d = objunit*1/10;

    fprintf(fp, "gsave\n");
    fprintf(fp, "  newpath\n");
    if(ty==0) {
        fprintf(fp, "  %d %d %d  90 -90 arc stroke\n", gx, gy, d);
        fprintf(fp, "  %d %d %d -90  90 arc fill\n",   gx, gy, d);
    }
    else if(ty==1) {
        fprintf(fp, "  %d %d %d  90 -90 arc stroke\n", gx, gy, d);
    }
    else {
        fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                        gx-d, gy-d, gx+d, gy+d);
        fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                        gx-d, gy+d, gx+d, gy-d);
    }
    fprintf(fp, "grestore\n");

    return 0;
}

int
bumpBB(int gx, int gy, int gw, int gh, int cx, int cy, int cth,
    int *rx, int *ry)
{
    int qx, qy;
    int tx, ty;
    int ik;
    int rv;
    int cr;
    int cc;

    rv = -1;
    cc = 0;
    cth = dirnormalize(cth);

    cr = gw+gh; /* so far */
    tx = cx+cr*cos(cth*rf);
    ty = cy+cr*sin(cth*rf);

    if(ty<=gy-gh/2) {
        ik = bumpH(gx-gw/2, gy-gh/2, gx+gw/2, gy-gh/2, cx, cy, cth, &qx, &qy);
        if(ik>0) {
            rv = 1;
            cc += 1;
            *rx = qx;
            *ry = qy;
        }
    }
    if(tx>=gx+gw/2) {
        ik = bumpV(gx+gw/2, gy+gh/2, gx+gw/2, gy-gh/2, cx, cy, cth, &qx, &qy);
        if(ik>0) {
            rv = 2;
            cc += 10;
            *rx = qx;
            *ry = qy;
        }
    }
    if(ty>=gy+gh/2) {
        ik = bumpH(gx-gw/2, gy+gh/2, gx+gw/2, gy+gh/2, cx, cy, cth, &qx, &qy);
        if(ik>0) {
            rv = 3;
            cc += 100;
            *rx = qx;
            *ry = qy;
        }
    }
    if(tx<=gx-gw/2) {
        ik = bumpV(gx-gw/2, gy+gh/2, gx-gw/2, gy-gh/2, cx, cy, cth, &qx, &qy);
        if(ik>0) {
            rv = 4;
            cc += 1000;
            *rx = qx;
            *ry = qy;
        }
    }

#if 0
    fprintf(stderr, "%s: rv %d cc %4d\n", __func__, rv, cc);
#endif

    return rv;
}

int
solvenotepos(int *rx, int *ry, int *ra, int *rj, ob *u, int pn,
    int ogap, int igap, int tht)
{
    int rv;

    *rj = SJ_CENTER;

    rv = 0;
    switch(pn) {
    case PO_CENTER:     *ra =   90;                              break;
    case PO_NORTH:      *ra =   90; *ry += u->ht/2+ogap;         break;
    case PO_SOUTH:      *ra =  -90; *ry -= u->ht/2+ogap;         break;
    case PO_EAST:       *ra =    0; *rx += u->wd/2+ogap;         break;
    case PO_WEST:       *ra =  180; *rx -= u->wd/2+ogap;         break;
    case PO_NORTHEAST:  *ra =   45; *rx += u->wd/2+ogap; *ry += u->ht/2+ogap; break;
    case PO_NORTHWEST:  *ra =  135; *rx -= u->wd/2+ogap; *ry += u->ht/2+ogap; break;
    case PO_SOUTHEAST:  *ra =  -45; *rx += u->wd/2+ogap; *ry -= u->ht/2+ogap; break;
    case PO_SOUTHWEST:  *ra = -135; *rx -= u->wd/2+ogap; *ry -= u->ht/2+ogap; break;
    case PO_START:
    case PO_END:
#if 0
        Error("not implemented yet %d\n", pn);
#endif
        rv = 1;
        break;

    case PO_CIC:
        *ra = 90;   *rj = SJ_CENTER;
        break;
    case PO_CIL:
        *ra =   90; *rj = SJ_LEFT;   *rx -= u->wd/2-igap;
        break;
    case PO_CIR:
        *ra =   90; *rj = SJ_RIGHT;  *rx += u->wd/2-igap;
        break;
        

    case PO_NOL:
        *ra =   90; *rj = SJ_LEFT;   *rx -= u->wd/2;    *ry += u->ht/2+ogap;
        break;
    case PO_NOC:
        *ra =   90; *rj = SJ_CENTER; *rx += 0;          *ry += u->ht/2+ogap;
        break;
    case PO_NOR:
        *ra =   90; *rj = SJ_RIGHT;  *rx += u->wd/2;    *ry += u->ht/2+ogap;
        break;

    case PO_NBL:
        *ra =   90; *rj = SJ_LEFT;   *rx -= u->wd/2;    *ry += u->ht/2-tht/2;
        break;
    case PO_NBC:
        *ra =   90; *rj = SJ_CENTER; *rx += 0;          *ry += u->ht/2-tht/2;
        break;
    case PO_NBR:
        *ra =   90; *rj = SJ_RIGHT;  *rx += u->wd/2;    *ry += u->ht/2-tht/2;
        break;

    case PO_EOL:
        *ra =    0; *rj = SJ_LEFT;   *rx += u->wd/2+ogap;   *ry += u->ht/2;
        break;
    case PO_EOC:
        *ra =    0; *rj = SJ_CENTER; *rx += u->wd/2+ogap;   *ry += 0;
        break;
    case PO_EOR:
        *ra =    0; *rj = SJ_RIGHT;  *rx += u->wd/2+ogap;   *ry -= u->ht/2;
        break;

    case PO_SOL:
        *ra =  -90; *rj = SJ_LEFT;   *rx += u->wd/2;    *ry -= u->ht/2+ogap;
        break;
    case PO_SOC:
        *ra =  -90; *rj = SJ_CENTER; *rx += 0;          *ry -= u->ht/2+ogap;
        break;
    case PO_SOR:
        *ra =  -90; *rj = SJ_RIGHT;  *rx -= u->wd/2;    *ry -= u->ht/2+ogap;
        break;

    case PO_WOL:
        *ra =  180; *rj = SJ_LEFT;   *rx -= u->wd/2+ogap;   *ry -= u->ht/2;
        break;
    case PO_WOC:
        *ra =  180; *rj = SJ_CENTER; *rx -= u->wd/2+ogap;   *ry += 0;
        break;
    case PO_WOR:
        *ra =  180; *rj = SJ_RIGHT;  *rx -= u->wd/2+ogap;   *ry += u->ht/2;
        break;

    case PO_NIL:
        *ra =   90; *rj = SJ_LEFT;   *rx -= u->wd/2-igap;   *ry += u->ht/2-tht-igap;
        break;
    case PO_NIC:
        *ra =   90; *rj = SJ_CENTER; *rx += 0;  *ry += u->ht/2-tht-igap;
        break;
    case PO_NIR:
        *ra =   90; *rj = SJ_RIGHT;  *rx += u->wd/2-igap;   *ry += u->ht/2-tht-igap;
        break;


    case PO_SIL:
        *ra =  -90; *rj = SJ_LEFT;   *rx += u->wd/2-igap;   *ry -= u->ht/2-tht-igap;
        break;
    case PO_SIC:
        *ra =  -90; *rj = SJ_CENTER; *rx += 0;  *ry -= u->ht/2-tht-igap;
        break;
    case PO_SIR:
        *ra =  -90; *rj = SJ_RIGHT;  *rx -= u->wd/2-igap;   *ry -= u->ht/2-tht-igap;
        break;


    case PO_EIL:
        *ra =    0; *rj = SJ_LEFT;   *rx += u->wd/2-tht-igap;   *ry += u->ht/2-igap;
        break;
    case PO_EIC:
        *ra =    0; *rj = SJ_CENTER; *rx += u->wd/2-tht-igap;   *ry += 0;
        break;
    case PO_EIR:
        *ra =    0; *rj = SJ_RIGHT;  *rx += u->wd/2-tht-igap;   *ry -= u->ht/2-igap;
        break;

    case PO_WIL:
        *ra =  180; *rj = SJ_LEFT;   *rx -= u->wd/2-tht-igap;   *ry -= u->ht/2-igap;
        break;
    case PO_WIC:
        *ra =  180; *rj = SJ_CENTER; *rx -= u->wd/2-tht-igap;   *ry += 0;
        break;
    case PO_WIR:
        *ra =  180; *rj = SJ_RIGHT;  *rx -= u->wd/2-tht-igap;   *ry += u->ht/2-igap;
        break;


    default:    
#if 0
        Error("ignore position %d\n", pn);
#endif
        rv = -1;
    }

    Echo("%s: pn %s(%d) rx,ry %d,%d\n", __func__,
        rassoc(pos_ial, pn), pn, *rx, *ry);
    
    return rv;
}

int
epsdraw_portboard_curve(FILE *fp, ns *xns, int xdir, ob *u)
{
    double  px, py, a, adeg;
    double  pag, bag;
    int     ux, uy, vx, vy;
    int     tx1, ty1, tx2, ty2;
    double  mu, mv;
    int     ik;
    int     gap;
    char    astr[BUFSIZ];

#if 0
    fprintf(fp, "%% %s: enter with xdir %d u %p\n", __func__, xdir, u);
#endif

    if(u->type==CMD_BCURVE) {
        ik = solve_curve_points(u, xns,
            &mu, &mv, &ux, &uy, &tx1, &ty1, &tx2, &ty2, &vx, &vy);
    }
    else
    if(u->type==CMD_BCURVESELF) {
        ik = solve_curveself_points(u, xns,
            &mu, &mv, &ux, &uy, &tx1, &ty1, &tx2, &ty2, &vx, &vy);
    }
    else {
    }
    

fprintf(fp, "%% %s: %d ik %d\n", __func__, __LINE__, ik);

    ik = _bez_posdir(&px, &py, &a, 0.5,
            ux, uy, tx1, ty1, tx2, ty2, vx, vy);

fprintf(fp, "%% %s: %d ik %d\n", __func__, __LINE__, ik);

    adeg = a/rf;
    pag = adeg-90;
    fprintf(fp, "%% a %f, adeg %f, pag %f\n", a, adeg, pag);

    if(u->cob.marknode) {
        fprintf(fp, "gsave 1 0 0 setrgbcolor "
            "newpath %.3f %.3f %d 0 360 arc fill grestore\n",
                px, py, objunit/20);
    }

    gap = def_pbstrgap;
    gap += u->cob.outlinethick/2;

    if(u->cob.portstr) {
        psescape(astr, BUFSIZ, u->cob.portstr);
        fprintf(fp, "%.3f %.3f %d %d %.2f %.2f (%s) xlrshow\n",
            px, py, gap, def_textheight,
                pag, (double)u->cob.portrotate, astr);
    }
    if(u->cob.boardstr) {
        psescape(astr, BUFSIZ, u->cob.boardstr);
        fprintf(fp, "%.3f %.3f %d %d %.2f %.2f (%s) xrrshow\n",
            px, py, gap, def_textheight,
                pag, (double)u->cob.boardrotate, astr);
    }

    return 0;
}


int
epsdraw_portboard_simpleline(FILE *fp, ns *xns, int xdir, ob *u)
{
    double  px, py, pag;
    int     ik;
    int     gap;
    varray_t *sar;
    char    astr[BUFSIZ];

#if 0
    fprintf(fp, "%% %s: enter with xdir %d u %p\n", __func__, xdir, u);
#endif

    sar = u->cob.segar;
    if(!sar) {
        return -2;
    }
    if(sar->use!=1) {
        return -3;
    }

    px = (u->gsx+u->gex)/2;
    py = (u->gsy+u->gey)/2;
    fprintf(fp, "%% oid %d px %f py %f\n", u->oid, px, py);

    pag = xdir-90;
#if 0
    fprintf(fp, "%% xdir %d, pag %f\n", xdir, pag);
#endif

    if(u->cob.marknode) {
        fprintf(fp, "gsave 1 0 0 setrgbcolor "
            "newpath %.3f %.3f %d 0 360 arc fill grestore\n",
                px, py, objunit/20);
    }

    gap = def_pbstrgap;
    gap += u->cob.outlinethick/2;

    if(u->cob.portstr) {
        psescape(astr, BUFSIZ, u->cob.portstr);
        fprintf(fp, "%.3f %.3f %d %d %.2f %.2f (%s) xlrshow\n",
            px, py, gap, def_textheight,
                pag, (double)u->cob.portrotate, astr);
    }
    if(u->cob.boardstr) {
        psescape(astr, BUFSIZ, u->cob.boardstr);
        fprintf(fp, "%.3f %.3f %d %d %.2f %.2f (%s) xrrshow\n",
            px, py, gap, def_textheight,
                pag, (double)u->cob.boardrotate, astr);
    }

    return 0;
}

int
_solve_pbpoint(FILE *fp, ns *xns, int xdir, int da, ob *u, int *px, int *py)
{
    int    rv;
    double th;
    double ph;
    int    fi;
    int    rr;
/*
 *     \  2
 *   +--\------+
 *   |   \     |
 * 3 |    x    | 1
 *   |         |
 *   +---------+
 *        4
 *
 *       1 r
 *   fi  2 u
 *       3 l
 *       4 d
 */

    rv = 0;
    fi = 0;
    ph = dirnormalize_positive(xdir + da);
    th = atan2(u->ht/2, u->wd/2)/rf;
    rr = sqrt( (u->ht/2)*(u->ht/2) + (u->wd/2)*(u->wd/2) );

    int kx, ky;

#if 0
    kx = u->gx;
    ky = u->gy;
#else
    kx = ( u->glx + u->grx ) / 2;
    ky = ( u->gby + u->gty ) / 2;
#endif

    if(0<=ph && ph<=th)                 { fi = 1; }
    else if(th<=ph && ph<=180-th)       { fi = 2; }
    else if(180-th<=ph && ph<=180+th)   { fi = 3; }
    else if(180+th<=ph && ph<=360-th)   { fi = 4; }
    else if(360-th<=ph)                 { fi = 1; }
    
    switch(fi) {
    case 1: /* right side */
            rv++; *px = u->grx; *py = ky + tan(ph*rf)*(*px-kx);
            break;
    case 3: /* left side */
            rv++; *px = u->glx; *py = ky + tan(ph*rf)*(*px-kx);
            break;
    case 2: /* upper side */
            rv++; *py = u->gty; *px = (*py-ky)/tan(ph*rf)+kx;
            break;
    case 4: /* bottom sude */
            rv++; *py = u->gby; *px = (*py-ky)/tan(ph*rf)+kx;
            break;
    default:
#if 0
        Error("normal line of oid %d does not touch bbox; ph %.2f th %.2f\n",
                u->oid, ph, th);
#endif
            Error("miss cross point oid %d, ph %.2f th %.2f\n",
                u->oid, ph, th);
            break;
    }

#if 0
    fprintf(stderr,
        "#Z oid %-3d ph %7.2f vs th %7.2f fi %d ; rv %d px,py %6d,%-6d\n",
        u->oid, ph, th, fi, rv, *px, *py);
    fflush(stdout);
#endif

    if(0) {
        fprintf(fp, "gsave 0 1 0 setrgbcolor %d setlinewidth "
            "newpath %d %d moveto %d %d lineto %d %d lineto "
            "%d %d lineto closepath stroke grestore\n",
            objunit/50,
            u->glx, u->gby, u->grx, u->gby,
            u->grx, u->gty, u->glx, u->gty);
    }
    if(0) {
        int nx, ny;
        nx = kx + rr*cos(ph*rf);
        ny = ky + rr*sin(ph*rf);
        fprintf(fp, "gsave 1 1 0 setrgbcolor %d setlinewidth "
            "newpath %d %d moveto %d %d lineto stroke grestore\n",
            objunit/50, kx, ky, nx, ny);
    }

    if(u->cob.marknode) {
        fprintf(fp, "gsave 1 0 0 setrgbcolor "
            "newpath %d %d %d 0 360 arc fill grestore\n",
            *px, *py, objunit/20);
    }

    if(0) {
        fprintf(fp, "gsave 0 0 1 setrgbcolor %d setlinewidth "
            "newpath %d %d %d 0 360 arc fill grestore\n",
            objunit/50, kx, ky, objunit/20);
    }

    return rv;
}

int
epsdraw_portboard(FILE *fp, ns *xns, int xdir, ob *u)
{
    int ik;
    int px, py;
    int bx, by;
    char astr[BUFSIZ];

    fprintf(fp, "%% %s: enter oid %-3d xdir %4d\n",
        __func__, u->oid, xdir);
#if 0
    fprintf(fp, "%% %s: enter with xdir %d u %p oid %d\n",
        __func__, xdir, u, u->oid);
#endif

    if(u->type==CMD_BCURVE||u->type==CMD_BCURVESELF) {
        return epsdraw_portboard_curve(fp, xns, xdir, u);
    }
#if 0
    fprintf(stderr, "# oid %d type %d original? %d\n",
        u->oid, u->type, u->cob.originalshape);
#endif
    if((u->type==CMD_LINE||u->type==CMD_ARROW||
        u->type==CMD_WLINE||u->type==CMD_WARROW) && u->cob.originalshape==0)  {
        return epsdraw_portboard_simpleline(fp, xns, xdir, u);
    }

#if 0
{
    int i;
    int cz, cp, cn;
    fflush(stdout);
    cz = cp = cn = 0;
    for(i=0;i<=360;i++) {
        ik = _solve_pbpoint(fp, xns, i,  90, u, &px, &py);
        if(ik>0)    {cp++;}
        if(ik==0)   {cz++;}
        if(ik<0)    {cn++;}

#if 1
        fprintf(stderr, "#PB %4d %4d %d\n", i, 90, ik);
#endif
#if 0
        fprintf(stderr, "#PB %d %4d %4d %d\n", u->oid, i, 90, ik);
        if(ik==0) {
            fprintf(stderr, "#PB %d %4d %4d %d\n", u->oid, i, 90, ik);
        }
#endif
    }
    fprintf(stderr, "#PB %d p/z/n %d %d %d\n", u->oid, cp, cz, cn);
}
#endif

    if(u->cob.portstr) {    
        ik = _solve_pbpoint(fp, xns, xdir,  90, u, &px, &py);
        if(ik) {
            psescape(astr, BUFSIZ, u->cob.portstr);
            fprintf(fp, "      %d %d %d %d %d %d (%s) xlrshow\n",
                px, py, def_pbstrgap, def_textheight,
                xdir-90, u->cob.portrotate, astr);
        }
        else {
            Error("cannot solve %dth obj portpoint\n", u->oid);
        }
#if 1
        ik = _solve_pbpoint(fp, xns, xdir, -90, u, &bx, &by);
        epsdraw_s2sstrbgX(fp, bx, by, objunit, objunit,
            PO_CENTER, 0, -90, def_pbstrgap,
            0, 0, u->cob.textcolor, u->cob.textbgcolor, 
            u->cob.portstr, SJ_LEFT);
#endif
    }
    if(u->cob.boardstr) {   
        ik = _solve_pbpoint(fp, xns, xdir, -90, u, &bx, &by);
        if(ik) {
            psescape(astr, BUFSIZ, u->cob.boardstr);
            fprintf(fp, "      %d %d %d %d %d %d (%s) xrrshow\n",
                bx, by, def_pbstrgap, def_textheight,
                xdir-90, u->cob.boardrotate, astr);
        }
        else {
            Error("cannot solve %dth obj boardpoint\n", u->oid);
        }
    }

    return 0;
}


int
epsdraw_note(FILE *fp, ob *u)
{
    int   rv;
    int   o;
    int   bx, by, ba, bj;
    int   tx, ty, ta;
    int   nx, ny, nr;
    int   lx, ly, rx, ry;
    int   ik;
    int   fht, fdc;
    char  mcont[BUFSIZ];
    char  amcont[BUFSIZ];
    int   exa;
    char  tmp[BUFSIZ];
    char *p, *q;
    int   fs;

#define INVREV      (180+360*10)        /* inveres but special number */

    rv = 0;
    nr = objunit/2;

    fht = def_textheight;
    fdc = (int)(textdecentfactor*fht);
    fs  = 0;

    for(o=0;o<PO_MAX;o++) {
        exa = 0;
        mcont[0] = '\0';

        if(u->cob.note[o] && u->cob.note[o][0]) {
            if(u->cob.note[o][0]=='!') {
                p = &u->cob.note[o][0];
                p++;
#if 1
                q = tmp;
                while(*p&&((*p>='0'&&*p<='9')||(*p=='-')||(*p=='.'))) {
                    *q++ = *p++;
                }
                *q = '\0';

                if(tmp[0]) {
                    exa = atoi(tmp);
                }
                else {
                    exa = INVREV;
                }
#else
                exa = INVREV;
#endif
                strcpy(mcont, p);
            }           
            else {  
                strcpy(mcont, u->cob.note[o]);
            }
        }
        else {
            continue;
        }


#if 0
        bx = u->cx;
        by = u->cy;
#endif
        bx = u->gx;
        by = u->gy;
        ba = 0;
#if 0
        ik = solvenotepos(&bx, &by, &ba, &bj, u, o, objunit/4, objunit/8, fht);
#endif
        ik = solvenotepos(&bx, &by, &ba, &bj, u, o,
                def_noteosep, def_noteisep, fht);

#if 0
        Echo("%2d: %-8s '%s' %6d,%-6d <%d>\n",
            o, rassoc(pos_ial, o), u->cob.note[o], bx, by, ik);
#endif

        if(ik) {
            continue;
        }

        if(o==PO_CENTER||o==PO_CIL||o==PO_CIC||o==PO_CIR) {
            by -= fht/2;
        }

        ta = ba - 90;
        tx = bx;
        ty = by;
        nx = bx + nr*cos(ba*rf);
        ny = by + nr*sin(ba*rf);

#if 0
        /* draw guide */
        fprintf(fp, "gsave 1 0 0 setrgbcolor %d setlinewidth\n"
                    "%d %d translate %d %d moveto %d rotate\n",
            def_linethick,
            bx, by, bx, by, ta);
        SLW_1x(fp, 8);
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            0, 0, 0, nr);
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            -nr, 0, nr, 0);
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            -nr, fht, nr, fht);
        SLW_41(fp);
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            -nr, fdc, nr, fdc);
        fprintf(fp, "grestore\n");
#endif

        fprintf(fp, "gsave %d %d translate %d rotate\n",
            tx, ty, ta);

        amcont[0] = '\0';
        psescape(amcont, BUFSIZ, mcont);
#if 0
fprintf(stderr, "mcont  |%s|\n", mcont);
fprintf(stderr, "amcont |%s|\n", amcont);
#endif

        if(exa) {
            fprintf(fp, "      %d rotate\n", exa);

            if(exa==INVREV) {
                fprintf(fp, "      %d %d translate\n", 0, -fht);
            }

#if 0
            /* draw guide for rotated */
            fprintf(fp, "gsave 0 0 1 setrgbcolor %d setlinewidth\n"
                        "%d %d translate %d %d moveto %d rotate\n",
                def_linethick,
                0, 0, 0, 0, 0);
            SLW_1x(fp, 8);
            fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
                0, 0, 0, nr);
            fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
                -nr, 0, nr, 0);
            fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
                -nr, fht, nr, fht);
            SLW_41(fp);
            fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
                -nr, fdc, nr, fdc);
            fprintf(fp, "grestore\n");
#endif


            if(fs==0) {
                fprintf(fp,
            "      /%s findfont %d scalefont setfont %% note default\n",
                def_fontname, fht);
                fs++;
            }

            if(exa==INVREV) {
#if 0
                changecolor(fp, 1);
#endif

                fprintf(fp, "      %d %d translate\n", 0, fdc);
                if(bj==SJ_LEFT) {
                    fprintf(fp, "      0 0 (%s) rshow\n", amcont);
                }
                if(bj==SJ_CENTER) {
                    fprintf(fp, "      0 0 (%s) cshow\n", amcont);
                }
                if(bj==SJ_RIGHT) {
                    fprintf(fp, "      0 0 (%s) lshow\n", amcont);
                }
            }
            else {

                fprintf(fp, "      %d %d translate\n", 0, fdc);
                if(bj==SJ_LEFT||bj==SJ_CENTER) {
                    fprintf(fp, "      0 0 (%s) lshow\n", amcont);
                }
                else {
                    fprintf(fp, "      0 0 (%s) rshow\n", amcont);
                }
            }
        }
        else {
            fprintf(fp, "      %d %d translate\n", 0, fdc);
            if(bj==SJ_LEFT) {
                fprintf(fp, "      0 0 (%s) lshow\n", amcont);
            }
            if(bj==SJ_CENTER) {
                fprintf(fp, "      0 0 (%s) cshow\n", amcont);
            }
            if(bj==SJ_RIGHT) {
                fprintf(fp, "      0 0 (%s) rshow\n", amcont);
            }
        }

        fprintf(fp, "grestore\n");
    }

    return rv;
}

int
epsdrawobj(FILE *fp, ob *u, int *xdir, int ox, int oy, ns *xns)
{
    int ik;
    int wd, ht;
    int g;
    int oldxdir;

    Echo("%s: arg xdir %d oxy %d,%d|u oid %d xy %d,%d gxy %d,%d\n",
        __func__, *xdir, ox, oy, u->oid, u->x, u->y, u->gx, u->gy);

    Echo("%s: arg xdir %d oxy %d,%d|u oid %d xy %d,%d oxy %d,%d\n",
        __func__, *xdir, ox, oy, u->oid, u->x, u->y, u->ox, u->oy);

    Echo("%s: u oid %d gxy %d,%d gsxy %d,%d gexy %d,%d\n",
        __func__, u->oid, 
        u->gx, u->gy,
        u->gsx, u->gsy,
        u->gex, u->gey
        );


    if(u->drawed) {
        Echo("%s: oid %d already drawed\n", __func__, u->oid);
        return 0;
    }

#if 0
    fprintf(fp, "%%\n%% --- oid %d %s\n",
        u->oid, rassoc(cmd_ial, u->type));
#endif

    wd = u->crx-u->clx;
    ht = u->cty-u->cby;

    oldxdir = *xdir;

    g = eval_dir(u, xdir);
    if(g>0) {

#if 0
        if(*xdir != oldxdir) {
            fprintf(fp,
                "%% dir change %d to %d\n", oldxdir, *xdir);
            fprintf(fp,
                "gsave %d %d moveto %d %d %d 0 360 arc fill grestore %% dir\n",
                ox, oy, ox, oy, objunit/10);
        }
#endif

        goto out;
    }

    if(draft_mode) {
        fprintf(fp, "gsave\n");
        changedraft(fp);
        epsdraw_mark(fp, u->gsx, u->gsy, 0);
        epsdraw_mark(fp, u->gex, u->gey, 1);
        epsdraw_mark(fp, u->gx,  u->gy,  2);
        fprintf(fp, "     gsave\n");
#if 0
#endif
        changethick(fp, def_linethick/2);
        fprintf(fp, "       newpath\n");
        fprintf(fp, "       %d %d moveto %d %d rlineto (%d) show stroke\n",
            u->gx, u->gy, objunit/4, objunit, u->pst);
        fprintf(fp, "     grestore\n");
        epsdraw_Xseglinearrow(fp, 0, 0,
            u->gsx, u->gsy, u->gex, u->gey,
            LT_MOUNTAIN, (int)((double)objunit*0.01),
            6, /*u->cob.outlinecolor, */
            AR_NONE, AH_WIRE, AH_WIRE, AH_WIRE);
        fprintf(fp, "grestore\n");
    }

    if(u->type==CMD_COMMENT) {
        if(u->cob.carg1) {
            fprintf(fp, "%% user-comment: %s\n", u->cob.carg1);
        }
    }

    if(draft_mode) {
        int fht;
        fht = def_textheight;
        fprintf(fp, "  /%s findfont %d scalefont setfont %% bbox\n",
            def_fontname, fht);

        epsdraw_bbox(fp, u);
    }

    if(u->type==CMD_NOTEFILE) {
        goto out;
    }
#if 1
    if(u->ignore) {
        goto out;
    }
#endif

#if 0
    if(bbox_mode) {
        epsdraw_bbox(fp, u);
    }
#endif

    if(u->cob.markbb) {
        fprintf(fp, "  gsave %% markbb %d\n", __LINE__);
        changecolor(fp, def_markcolor);
        changethick(fp, def_markbbthick);
        drawCRrect(fp, u->gx, u->gy, u->wd, u->ht, u->cob.rotateval);
        fprintf(fp, "  grestore\n");

    }

#if 0
    changenormal(fp); /* for faill safe */
#endif
    changecolor(fp, u->vob.outlinecolor);
    changethick(fp, u->vob.outlinethick);

    if(u->type==CMD_NOP) {
        /* nothing */
    }
    else
    if(u->type==CMD_EXIT) {
        /* nothing */
    }
    else
    if(u->type==CMD_SCATTER) {
        epsdraw_scatter(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_GATHER) {
        epsdraw_gather(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_THRU) {
        epsdraw_thruX(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_BCURVE || u->type==CMD_XCURVE) {
        Zepsdraw_bcurvearrow(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_BCURVESELF || u->type==CMD_XCURVESELF) {
        Zepsdraw_bcurveselfarrow(fp, *xdir, ox, oy, u, xns);
    }
#if 0
    else
    if(u->type==CMD_XCURVE) {
        Zepsdraw_xcurvearrow(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_XCURVESELF) {
        Zepsdraw_xcurveselfarrow(fp, *xdir, ox, oy, u, xns);
    }
#endif
    else
    if((u->type==CMD_LINK) || (u->type==CMD_LINE) ||
            (u->type==CMD_ARROW)) {
P;
#if 0
fprintf(stderr, "#before Zepsdraw_ulinearrow oid %d xdir %d\n", u->oid, *xdir);
#endif
        Zepsdraw_ulinearrow(fp, *xdir, ox, oy, u, xns);
#if 0
fprintf(stderr, "#after  Zepsdraw_ulinearrow oid %d xdir %d\n", u->oid, *xdir);
#endif
    }
    else
    if(u->type==CMD_ULINE) {
P;
        Zepsdraw_ulinearrow(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_WLINE) {
        Zepsdraw_wlinearrow(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_WARROW) {
        Zepsdraw_wlinearrow(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_BARROW) {
        epsdraw_blinearrow(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_PLINE) {
#if 0
        epsdraw_plinearrow(fp, *xdir, ox, oy, u, xns);
#endif
        epsdraw_sep(fp, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_SEP) {
        epsdraw_sep(fp, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_PING) {
        epsdraw_ping(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_PINGPONG) {
        epsdraw_pingpong(fp, *xdir, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_LPAREN || u->type==CMD_RPAREN) {
        epsdraw_paren(fp, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_LBRACKET || u->type==CMD_RBRACKET) {
        epsdraw_bracket(fp, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_LBRACE || u->type==CMD_RBRACE) {
        epsdraw_brace(fp, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_MOVE) {
        if(movevisit_mode) {
            int w = u->crx - u->clx;
            int h = u->cty - u->cby;
            int r = objunit/10;
#if 0
fprintf(stderr, "w %d h %d r %d\n", w, h, r);
#endif
            int aw = w - 3*objunit/10;
            int ah = h - 3*objunit/10;
#if 0
fprintf(stderr, "aw %d ah %d\n", aw, ah);
#endif
            if(aw<0 || ah<0) { aw = w; ah = h; r = 0; }
#if 0
fprintf(stderr, "aw %d ah %d\n", aw, ah);
#endif
            int ow = (w-aw)/2;
            int oh = (h-ah)/2;
            fprintf(fp, "gsave\n");
            fprintf(fp, "  0.8 setgray\n");
            fprintf(fp, "  %d %d %d %d %d mrboxfill\n",
                ox+u->clx+ow, oy+u->cby+oh, aw, ah, r);
#if 0
            fprintf(fp, "  %d setlinewidth\n", objunit/200);
            fprintf(fp, "  0.5 setgray\n");
            fprintf(fp, "  %d %d %d %d 0 mrbox\n",
                ox+u->clx, oy+u->cby, w, h);
#endif
            fprintf(fp, "grestore\n");
        }
    }
    else
    if(u->type==CMD_CLOUD) {
        epsdraw_cloud(fp, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_DOTS) {
        epsdraw_dots(fp, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_RULER) {
        epsdraw_ruler(fp, ox, oy, u, xns);
    }
    else
    if(u->type==CMD_BOX || u->type==CMD_CIRCLE || u->type==CMD_POINT ||
        u->type==CMD_ELLIPSE ||
        u->type==CMD_PAPER || u->type==CMD_CARD || u->type==CMD_DIAMOND ||
        u->type==CMD_HOUSE || u->type==CMD_POLYGON || u->type==CMD_GEAR ||
        u->type==CMD_DRUM  || u->type==CMD_PIPE) {
        epsdraw_bodyX(fp, ox, oy, u, xns);
    }
    else {
        Warn("undefined drawing object type '%s'(%d)\n",
            rassoc(cmd_ial ,u->type), u->type);
    }

    if(u->cob.ssar) {
        int _tbgc = -1;
        char cont[BUFSIZ];

        ss_strip(cont, BUFSIZ, u->cob.ssar);
        
        if(!cont[0]) {
            goto skip_sstr;
        }

        Echo("call sstrbgX oid %d gx,y %d,%d with ox,y %d,%d |%s|\n",
            u->oid, u->gx, u->gy, u->ox, u->oy, cont);

        if(ISGLUE(u->type)) {
        }
        else
        if(u->cob.fillhatch==HT_NONE) {
        }
        else {
            _tbgc = u->cob.textbgcolor;
        }

#if 0
        Echo("text angle %d\n",
            u->cob.rotateval + u->cob.textrotate);
#endif
#if 0
        epsdraw_sstrbgX(fp, u->gx, u->gy, u->wd, u->ht,
            PO_CENTER, 0,
            u->cob.rotateval + u->cob.textrotate, 0,
            0, 2, u->cob.textcolor, _tbgc, u->cob.ssar, -1);
#endif
        epsdraw_sstrbgX(fp, u->gx, u->gy, u->wd, u->ht,
            u->cob.textalign, u->cob.textoffset,
            u->cob.rotateval + u->cob.textrotate, 0,
            0, 2, u->cob.textcolor, _tbgc, u->cob.ssar, -1);

skip_sstr:
        (void)0;
    }

    ik = epsdraw_note(fp, u);
    if(u->cob.portstr || u->cob.boardstr) {
        ik = epsdraw_portboard(fp, xns, *xdir, u);
    }

    u->drawed = 1;

out:
    Echo("%s: oid %d xdir %d ox,oy %d,%d, x,y %d,%d gx,gy %d,%d\n",
        __func__, u->oid, *xdir, ox, oy, u->x, u->y, u->gx, u->gy);

    fflush(fp);
    return 0;
}


int
epsdrawchunk(FILE *fp, ob *xch, int gox, int goy, ns *xns)
{
    int ik;
    int i;
    ob* u;
    char msg[BUFSIZ];

P;
    Echo("%s: oid %d x,y %d,%d\n", __func__, xch->oid, gox, goy);

    if(xch->drawed) {
P;
    Echo("%s: oid %d already drawed\n", __func__, xch->oid);
        return 0;
    }
P;
    Echo("%s:   oid %d wxh %dx%d o %d,%d\n",
        __func__, xch->oid, xch->wd, xch->ht, xch->ox, xch->oy);
    Echo("%s: b oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid,
 xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey, xch->ox, xch->coy);

    fprintf(fp, "%%%%%%%%%%%%\n");
    fprintf(fp, "%% start chunk oid %d\n", xch->oid);

    cha_reset(&xch->cch);

    if(xch->cob.markbb || bbox_mode) {
        fprintf(fp, "  gsave %% markbb\n");
        changecolor(fp, def_markcolor);
        changethick(fp, def_markbbthick*2);
#if 0
        drawCRrectskel(fp, xch->gx, xch->gy, xch->wd, xch->ht,
            xch->cob.rotateval);
#endif
#if 0
        drawrectcm(fp, xch->glx, xch->gby, xch->grx, xch->gty, "whole");
#endif
#if 1 
        drawCRrectskel(fp, (xch->glx+xch->grx)/2, (xch->gby+xch->gty)/2,
            (xch->grx-xch->glx), (xch->gty-xch->gby),
            xch->cob.rotateval);
#endif
#if 0
        printf("gx,y %d,%d wdxht %d,%d\n",
            xch->gx, xch->gy, xch->wd, xch->ht);
        printf("glx,by,rx,ty %d,%d,%d,%d\n",
             xch->glx, xch->gby, xch->grx, xch->gty);
#endif
        fprintf(fp, "  grestore\n");
    }
#if 0
    if(bbox_mode) {
        epsdraw_bbox(fp, xch);
    }
#endif

#if 1
    if(
        (xch->cob.fillhatch!=HT_NONE && xch->cob.fillcolor>=0) ||
        (xch->cob.outlinethick>0) ) 
    {
        fprintf(fp, "%% chunk itself START\n");
        fprintf(fp, 
        "%% SELF gox,goy %d,%d ox,oy %d,%d x,y %d,%d w,h %d,%d\n",
            gox, goy, xch->ox, xch->oy, xch->x, xch->y,
            xch->wd, xch->ht);

        changecolor(fp, xch->cob.outlinecolor);
        changethick(fp, xch->cob.outlinethick);

        fprintf(fp, "%% bgshape %p\n", xch->cob.bgshape);
        if(xch->cob.bgshape) {
            fprintf(fp, "%% value shape '%s'\n", xch->cob.bgshape);
        }
        if(xch->cob.bgshape && strcmp(xch->cob.bgshape, "plane")==0) {
PP;
fprintf(fp, "%% plane\n");
            ik = epsdraw_plane(fp,
                    gox,
                    goy, xch, xns);
        }
        else {
fprintf(fp, "%% bodyX\n");
PP;
            ik = epsdraw_bodyX(fp, gox, goy, xch, xns);
        }

        fprintf(fp, "%% chunk itself END\n");
    }
#endif

#if 0
    if(xch->cob.outlinethick>0 && xch->cob.outlinecolor>=0) {
        fprintf(fp, "gsave %%  for outline\n");
        changethick(fp, xch->cob.outlinethick);
        changecolor(fp, xch->cob.outlinecolor);
        drawCRrect(fp, gox+xch->x, goy+xch->y, xch->wd, xch->ht,
            xch->cob.rotateval);
        fprintf(fp, "grestore %% for outline\n");
    }
#endif

    if(skelton_mode) {
        for(i=0;i<xch->cch.nch;i++) {
            u = (ob*)xch->cch.ch[i];
            epsdraw_bbox(fp, u);
#if 0
            MC(0, gox+xch->x+u->ox+u->cx, goy+xch->y+u->oy+u->cy);
            MCF(1, gox+xch->x,             goy+xch->y);
            MCF(2, gox+xch->x+u->ox,       goy+xch->y+u->oy);
            MCF(4, gox+xch->x+u->ox+u->csx, goy+xch->y+u->oy+u->csy);
            MCF(0, gox+xch->x+u->ox+u->cx, goy+xch->y+u->oy+u->cy);
            MC(2, gox+xch->x+u->cx, goy+xch->y+u->cy);
            MC(3, gox+xch->x+u->ox+u->csx, goy+xch->y+u->oy+u->csy);
#endif

#if 0
            epsdraw_Xseglinearrow(fp, 0, 0,
                u->gsx, u->gsy, u->gex, u->gey,
                LT_MOUNTAIN, (int)((double)objunit*0.01),
                6, /*u->cob.outlinecolor, */
                AR_NONE, AH_WIRE, AH_WIRE);
#endif

            MC(0, u->gx, u->gy);
            {
            int qqqx, qqqy;
            qqqx = u->gx - u->x;
            qqqy = u->gy - u->y;
            MC(1, qqqx+u->x, qqqy+u->y);
            MC(2, qqqx+u->ox, qqqy+u->oy);
            MC(3, qqqx+u->fx, qqqy+u->fy);
            MC(4, qqqx+u->sx, qqqy+u->sy);
            MC(6, qqqx+u->ex, qqqy+u->ey);
            }

            if(ISCHUNK(u->type)) {
                ik = epsdrawchunk(fp, u,
                        gox+xch->x+xch->ox,
                        goy+xch->y+xch->oy, xns);
            }
            else {
            }
        }
        return -1;
    }

    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*)xch->cch.ch[i];

        if(ISCHUNK(u->type)) {
            ik = epsdrawchunk(fp, u,
                    gox+xch->x+xch->ox,
                    goy+xch->y+xch->oy, xns);
        }
        else {

#if 1
            fprintf(fp, "%% --- oid %d %s\n",
                u->oid, rassoc(cmd_ial, u->type));
#endif

            if(bbox_mode) {
                if(VISIBLE(u->type)) {
                    epsdraw_bbox(fp, u);
                }
                else {
                    Info("; bbox invisible oid %d\n", u->oid);
                }
            }
    
Echo(" call obj oid %d drawing start %d,%d\n",
            u->oid, gox+xch->x+xch->ox, goy+xch->y+xch->oy);
            ik = epsdrawobj(fp, u, &xch->cch.dir,
                    gox+xch->x+xch->ox,
                    goy+xch->y+xch->oy, xns);
        }
    }

#if 0
    ik = epsdraw_note(fp, xch);
    ik = epsdraw_portboard(fp, xch->cch.dir, u);
#endif

    fprintf(fp, "%% oid %d DONE\n", xch->oid);

    Echo("%s: a oid %d - %d,%d s %d,%d e %d,%d o %d,%d\n",
        __func__, xch->oid, 
        xch->cx, xch->cy, xch->csx, xch->csy, xch->cex, xch->cey, xch->ox, xch->coy);

    xch->drawed = 1;

    fflush(fp);
    return 0;
}

int
epsdrawobj_oidl(FILE *fp, ob *u, int *xdir, int ox, int oy, ns *xns)
{
    int cx, cy;
    int r;
    int r2;
#if 0
    int dx, dy;
#endif
    int a;
    char tmp[BUFSIZ];

    cx = u->gx;
    cy = u->gy;
    r  = objunit*1.5;
    r2 = MAX((r*105)/100, r+40);

    a  = 45+(u->oid-1)*17;
    sprintf(tmp, "oid %d %s", u->oid, rassoc(cmd_ial, u->type));

    fprintf(fp, "  gsave\n");
    fprintf(fp, "    0.8 0.3 1 setrgbcolor\n");
    fprintf(fp, "    %d %d translate\n", cx, cy);
    fprintf(fp, "    %d rotate\n", a);
    fprintf(fp, "    0 0 moveto\n");
    fprintf(fp, "    %d %d lineto\n", r, 0);
    fprintf(fp, "    stroke\n");
    fprintf(fp, "    %d %d moveto\n", r2, 0);
    fprintf(fp, "    (%s) show\n", tmp);
    fprintf(fp, "  grestore\n");

    return 0;
}

int
epsdrawchunk_oidl(FILE *fp, ob *xch, int gox, int goy, ns *xns)
{
    int i;
    ob* u;
    int xdir;

    fprintf(fp, "gsave\n");
    fprintf(fp, "  /Helvetica findfont %d scalefont setfont\n",
        (10*objunit)/100);
    epsdrawobj_oidl(fp, xch, &xdir, gox, goy, xns);

    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*)xch->cch.ch[i];
        if(!u) continue;

        if(ISCHUNK(u->type)) {
            epsdrawchunk_oidl(fp, u, gox, goy, xns);
        }
        else {
            epsdrawobj_oidl(fp, u, &xdir, gox, goy, xns);
        }
    }

    fprintf(fp, "grestore\n");

    return 0;
}

int
epsdrawobj_namel(FILE *fp, ob *u, char *n, int *xdir, int ox, int oy, ns *xns)
{
    int cx, cy;
    int r;
#if 0
    int dx, dy;
#endif
    int a;
    int fht;
    char tmp[BUFSIZ];

#if 0
    cx = u->gx;
    cy = u->gy;
    r = objunit*1.5;
    a = 60+(u->oid-1)*17;
    sprintf(tmp, "oid %d %s", u->oid, rassoc(cmd_ial, u->type));

    fprintf(fp, "  gsave\n");
    fprintf(fp, "    0.2 0.5 1 setrgbcolor\n");
    fprintf(fp, "    %d %d translate\n", cx, cy);
    fprintf(fp, "    %d rotate\n", a);
    fprintf(fp, "    0 0 moveto\n");
    fprintf(fp, "    %d %d lineto\n", r, 0);
    fprintf(fp, "    stroke\n");
    fprintf(fp, "    %d %d moveto\n", r, 0);
    fprintf(fp, "    (%s) show\n", tmp);
    fprintf(fp, "  grestore\n");
#endif

#if 1
    cx = u->gx-u->wd/2;
    cy = u->gy+u->ht/2;
    fht = def_textheight;
    r = objunit*1.5;
    a = 60+(u->oid-1)*17;

    fprintf(fp, "  gsave\n");
    fprintf(fp, "    %d %d translate\n", cx, cy-fht);
    fprintf(fp, "    0 0 moveto\n");
    fprintf(fp, "    1 1 1 setrgbcolor\n");
    fprintf(fp, "    (%s) dup stringwidth pop dup\n", n);
    fprintf(fp, "    exch 0 rlineto 0 %d rlineto neg 0 rlineto closepath fill\n", fht);
    fprintf(fp, "    0.2 0.5 1 setrgbcolor\n");
    fprintf(fp, "    0 0 moveto show\n");
    fprintf(fp, "  grestore\n");
#endif

    return 0;
}

int
epsdrawchunk_namel(FILE *fp, ob *xch, int gox, int goy, ns *xns)
{
    int i;
    ob* u;
    int xdir;
    char* q;

    fprintf(fp, "gsave\n");
#if 0
    fprintf(fp, "  /Helvetica findfont 10 scalefont setfont\n");
#endif
    fprintf(fp, "  /Helvetica findfont %d scalefont setfont\n",
        def_textheight);

#if 0
    epsdrawobj_oidl(fp, xch, &xdir, gox, goy, xns);
#endif

    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*)xch->cch.ch[i];
        if(!u) continue;

        q = _ns_find_name(xns, u, 0);
Echo(" q %p\n", q);

        if(!q) {
            continue;
        }

        if(ISCHUNK(u->type)) {
            epsdrawchunk_namel(fp, u, gox, goy, xns);
        }
        else {
#if 0
            epsdrawobj_namel(fp, u, &xdir, gox, goy, xns);
#endif
            if(q) {
                epsdrawobj_namel(fp, u, q, &xdir, gox, goy, xns);
            }
        }
    }

    fprintf(fp, "grestore\n");

    return 0;
}


int
epsdraw_rulerframe(FILE *fp, ob *ych, int yx, int yy)
{
    double x, y;
    double px, py;
    int g;
    int w;
    int i;
    int c;

    g  = 500;
    px = py = 2834.6457;
    
    fprintf(fp, "%% ruleframe\n");

    fprintf(fp, "gsave\n");
    fprintf(fp, "  0 0 1 setrgbcolor\n");
    fprintf(fp, "  50 setlinewidth\n");

    fprintf(fp, "  %d %d moveto\n", ych->lx, ych->by-g*2);
    fprintf(fp, "  %d %d lineto\n", ych->lx, ych->by-g);
    fprintf(fp, "  %d %d rlineto\n", ych->wd, 0);
    fprintf(fp, "  stroke\n");

    c = (double)ych->wd/px;
    for(i=0;i<=c;i++) {
        if(i%5==0) w = -720; else w = -360;
        fprintf(fp, "  %.3f %.3f moveto 0 %d rlineto stroke\n",
            (double)ych->lx + i*py, (double)ych->by -g, w);
    }

    fprintf(fp, "  %d %d moveto\n", ych->lx - g*2, ych->by);
    fprintf(fp, "  %d %d lineto\n", ych->lx - g  , ych->by);
    fprintf(fp, "  %d %d rlineto\n", 0, ych->ht);
    fprintf(fp, "  stroke\n");

    c = (double)ych->ht/py;
    for(i=0;i<=c;i++) {
        if(i%5==0) w = -720; else w = -360;
        fprintf(fp, "  %.3f %.3f moveto %d 0 rlineto stroke\n",
            (double)ych->lx -g, (double)ych->by + i*py, w);
    }

    fprintf(fp, "grestore\n");

    return 0;
}


int
printdefs(FILE *fp)
{

    fprintf(fp, "\
%% x y s lshow -            left justify text show\n\
/lshow {\n\
    /s exch def /y exch def /x exch def\n\
    x y moveto s show\n\
} def\n\
");

    fprintf(fp, "\
%% x y s rshow -            right justify text show\n\
/rshow {\n\
    /s exch def /y exch def /x exch def\n\
    x s stringwidth pop sub y moveto s show\n\
} def\n\
");

    fprintf(fp, "\
%% x y s cshow -            center justify text show\n\
/cshow {\n\
    /s exch def /y exch def /x exch def\n\
    x s stringwidth pop 2 div sub y moveto s show\n\
} def\n\
");

    fprintf(fp, "\
%% x y r s lrshow -         left justify text show\n\
/lrshow {\n\
    gsave\n\
    /s exch def /r exch def /y exch def /x exch def\n\
    x y translate r rotate 0 0 moveto s show\n\
    grestore\n\
} def\n\
");

    fprintf(fp, "\
%% x y r s rrshow -         right justify rotate text show\n\
/rrshow {\n\
    gsave\n\
    /s exch def /r exch def /y exch def /x exch def\n\
    x y translate r rotate s stringwidth pop neg 0 moveto s show\n\
    grestore\n\
} def\n\
");

    fprintf(fp, "\
%% x y r s crshow -         center justify rotate text show\n\
/crshow {\n\
    gsave\n\
    /s exch def /r exch def /y exch def /x exch def\n\
    x y translate r rotate s stringwidth pop 2 div neg 0 moveto s show\n\
    grestore\n\
} def\n\
");

    fprintf(fp, "\
%% x y g h r1 r2 s xlrshow - double rotate left justify text show\n\
/xlrshow {\n\
    gsave\n\
      /s exch def /r2 exch def /r1 exch def\n\
      /h exch def /g exch def /y exch def /x exch def\n\
      /w s stringwidth pop def\n\
      x y translate r1 rotate\n\
  %% gsave 1 0 0 setrgbcolor 0 0 h 10 div 0 360 arc fill grestore\n\
  %% gsave 1 0 0 setrgbcolor w g add neg h 2 div neg w h 0 mrbox grestore\n\
      w 2 div g add neg 0 translate r2 rotate\n\
  %% gsave 0 0 1 setrgbcolor w 2 div neg h 2 div neg w h 0 mrbox grestore\n\
      newpath\n\
      w 2 div neg h 2 div neg moveto\n\
      s show\n\
    grestore\n\
} def\n\
");
fprintf(fp, "\
%% x y g h r1 r2 s xrrshow -\n\
/xrrshow {\n\
    gsave\n\
      /s exch def /r2 exch def /r1 exch def\n\
      /h exch def /g exch def /y exch def /x exch def\n\
      /w s stringwidth pop def\n\
      x y translate r1 rotate\n\
      w 2 div g add 0 translate r2 rotate\n\
      newpath\n\
      w 2 div neg h 2 div neg moveto\n\
      s show\n\
    grestore\n\
} def\n\
");


    fprintf(fp, "\
%% x y w h r mrbox -        margined round box\n\
/mrbox {\n\
    gsave\n\
    /r exch def /h exch def /w exch def /y exch def /x exch def\n\
    x r sub y moveto\n\
    x y r 180 270 arc\n\
    w 0 rlineto\n\
    x w add y r 270 360 arc\n\
    0 h rlineto\n\
    x w add y h add r 0 90 arc\n\
    w neg 0 rlineto\n\
    x y h add r 90 180 arc\n\
    closepath\n\
    stroke\n\
    grestore\n\
} def\n\
");

    fprintf(fp, "\
%% x y w h r mrbox -        margined round box fill\n\
/mrboxfill {\n\
    gsave\n\
    /r exch def /h exch def /w exch def /y exch def /x exch def\n\
    x r sub y moveto\n\
    x y r 180 270 arc\n\
    w 0 rlineto\n\
    x w add y r 270 360 arc\n\
    0 h rlineto\n\
    x w add y h add r 0 90 arc\n\
    w neg 0 rlineto\n\
    x y h add r 90 180 arc\n\
    closepath\n\
    fill\n\
    grestore\n\
} def\n\
");

#if 0
    fprintf(fp, "\
%% margined round box with mainbody box\n\
%% x y w h r mrbox -\n\
/mrbox {\n\
    gsave\n\
    /r exch def /h exch def /w exch def /y exch def /x exch def\n\
    x r sub y moveto\n\
    x y r 180 270 arc\n\
    w 0 rlineto\n\
    x w add y r 270 360 arc\n\
    0 h rlineto\n\
    x w add y h add r 0 90 arc\n\
    w neg 0 rlineto\n\
    x y h add r 90 180 arc\n\
    closepath\n\
    fill\n\
    1 1 1 setrgbcolor\n\
    x y moveto w 0 rlineto 0 h rlineto w neg 0 rlineto closepath stroke\n\
    grestore\n\
} def\n\
");
#endif

    return 0;
}


/*
 * my tool seems does not care negative bounding box
 * then, I shift objects as 0 0 width height (with margin)
 */


int
epsdraw(FILE *fp, int cwd, int cht, int crt, double csc,
    ob *xch, int x, int y, ns *xns)
{
    int ik;
    int epswd, epsht;

P;
    Echo("%s: cwd %d cht %d crt %d csc %.3f\n",
        __func__, cwd, cht, crt, csc);
    Echo(" xch %d x %d\n", xch->wd, xch->ht);
    Echo("     bb g %d %d %d %d\n", xch->glx, xch->gby, xch->grx, xch->gty);
    Echo("     bb _ %d %d %d %d\n", xch->lx,  xch->by,  xch->rx,  xch->ty);
    Echo(" epsoutmargin %d\n", epsoutmargin);

    epsdraftfontsize = (int)((double)10/100*objunit);
    epsdraftgap      = (int)((double)5/100*objunit);

    epswd = (int)(csc*xch->wd)+epsoutmargin*2;
    epsht = (int)(csc*xch->ht)+epsoutmargin*2;

    Echo(" %d %d %d %d\n", 0, 0, epswd, epsht);

#if 1
    fprintf(fp,  "%%!PS-Adobe-3.0 EPSF-3.0\n\
%%%%BoundingBox: %d %d %d %d\n",
        0, 0, epswd, epsht);
#endif


    printobjlist(fp, "% ", xch);

    printdefs(fp);


    /***
     *** main-body
     ***/
    fprintf(fp, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
    fprintf(fp, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
    fprintf(fp, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
    fprintf(fp, "%%%% START\n");

    fprintf(fp, "%d %d translate %% margin\n", epsoutmargin, epsoutmargin);
    fprintf(fp, "%.3f %.3f scale\n", csc, csc);
    fprintf(fp, "%d %d translate %% bbox\n", -xch->lx, -xch->by);

#if 1
    fprintf(fp, "/%s findfont %d scalefont setfont %% font (fail-safe)\n",
        def_fontname, def_textheight);
#endif

#if 0
    changedraft(fp);
    fprintf(fp, "  %d setlinewidth\n", def_linethick);
#endif

    if(grid_mode) {
        /* grid */
        int x, y;
        int gp;

        gp = def_gridpitch;

/*
#define GGH fprintf(fp, "  0.6 1.0 1.0 setrgbcolor %d setlinewidth\n", def_linethick/8);
#define GGM fprintf(fp, "  0.6 0.6 1.0 setrgbcolor %d setlinewidth\n", def_linethick/2);
#define GGL fprintf(fp, "  0.8 0.6 1.0 setrgbcolor %d setlinewidth\n", def_linethick/1);
*/

#define GGH fprintf(fp, "  0.6 1.0 1.0 setrgbcolor %d setlinewidth\n", def_linethick/8);
#define GGM fprintf(fp, "  0.6 1.0 1.0 setrgbcolor %d setlinewidth\n", def_linethick/4);
#define GGL fprintf(fp, "  0.6 1.0 1.0 setrgbcolor %d setlinewidth\n", def_linethick/2);

        fprintf(fp, "%%\n%% grid\n");
        fprintf(fp, "gsave\n");
        fprintf(fp, "  %d setlinewidth\n", def_linethick/4);
        GGH;

        for(x=-def_gridrange;x<=def_gridrange;x++) {
            if(x%5==0) {
                if(x%10==0) { GGL; }
                else        { GGM; }
            }
            fprintf(fp, "  %d -%d moveto 0 %d rlineto stroke\n",
                x*gp, def_gridrange*gp, (2*def_gridrange)*gp);
            if(x%5==0) GGH;
        }
        for(y=-def_gridrange;y<=def_gridrange;y++) {
            if(y%5==0) GGM;
            if(y%10==0) GGL;
            fprintf(fp, "  -%d %d moveto %d 0 rlineto stroke\n",
                def_gridrange*gp, y*gp, (2*def_gridrange)*gp);
            if(y%5==0) GGH;
        }

        changehot(fp);
        fprintf(fp, "  %d setlinewidth\n", def_linethick);
        fprintf(fp, "  0 %d moveto 0 %d lineto stroke\n",
            -def_gridrange*gp, def_gridrange*gp);
        fprintf(fp, "  %d 0 moveto %d 0 lineto stroke\n",
            -def_gridrange*gp, def_gridrange*gp);
        fprintf(fp, "grestore\n");
        fprintf(fp, "%% end-grid\n");
    }

    if(bbox_mode) {
        fprintf(fp, "%%\n%% whole bb g %d %d %d %d\n",
                        xch->glx, xch->gby, xch->grx, xch->gty);
        fprintf(fp,     "%% whole bb _ %d %d %d %d\n",
                        xch->lx, xch->by, xch->rx, xch->ty);
        fprintf(fp, "gsave\n");
        changeground(fp);
        fprintf(fp, "  %d setlinewidth\n", def_linedecothick);
        drawrectcm(fp, xch->glx, xch->gby, xch->grx, xch->gty, "whole");
#if 0
        drawrectcm(fp, xch->lx, xch->by, xch->rx, xch->ty, "whole");
#endif
        fprintf(fp, "grestore\n");
    }


    fflush(fp);

    if(debuglog[0]) {
        Echo("%s\n", debuglog);
    }

P;
    ik = epsdrawchunk(fp, xch, x, y, xns);

    if(oidl_mode) {
        ik = epsdrawchunk_oidl(fp, xch, x, y, xns);
    }
    if(namel_mode) {
        ik = epsdrawchunk_namel(fp, xch, x, y, xns);
    }

    if(ruler_mode) {
        ik = epsdraw_rulerframe(fp, xch, x, y);
    }
    
    fprintf(fp, "showpage\n");
    fprintf(fp, "%%%%EOF\n");
    fprintf(fp, "%%%% END EPS\n");
    fflush(fp);

    return ik;
}

