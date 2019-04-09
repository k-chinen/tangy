
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

    Echo("%s: rv %f ty %d def_linedecopitch %d\n",
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

#if 0
Echo("%s: '%s' <- '%s'\n", __func__, dst, src);
#endif

    return 0;
}

int
changethick(FILE *fp, int lth)
{
    /*           12345678*/
    fprintf(fp, "        %d setlinewidth\n", lth);

    return 0;
}

int
changecolor(FILE *fp, int cn)
{
    color_t *c;
    char tmp[BUFSIZ];

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

#if 0
/* centerized rect */
static int
drawCrect(FILE *fp, int x1, int y1, int wd, int ht)
{
    fprintf(fp, "  gsave\n");
    fprintf(fp, "    %d %d translate\n", x1, y1);
    fprintf(fp, "    newpath\n");
    fprintf(fp, "    %d %d moveto\n",   -wd/2, -ht/2);
    fprintf(fp, "    %d %d lineto\n",    wd/2, -ht/2);
    fprintf(fp, "    %d %d lineto\n",    wd/2,  ht/2);
    fprintf(fp, "    %d %d lineto\n",   -wd/2,  ht/2);
    fprintf(fp, "    closepath\n");
    fprintf(fp, "    stroke\n");
    fprintf(fp, "  grestore\n");

    return 0;
}
#endif

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
#if 0
    fprintf(fp, "    newpath\n");
    fprintf(fp, "    %d %d moveto\n",   -wd/2, -ht/2);
    fprintf(fp, "    %d %d lineto\n",    wd/2, -ht/2);
    fprintf(fp, "    %d %d lineto\n",    wd/2,  ht/2);
    fprintf(fp, "    %d %d lineto\n",   -wd/2,  ht/2);
    fprintf(fp, "    closepath\n");
    fprintf(fp, "    stroke\n");
#endif

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
epsdraw_bbox_cwh(FILE *fp, ob *xu)
{
    if(INTRACE) {
        fprintf(fp, "%% bbox guide oid %d with WH (%d %d) by %s\n",
            xu->oid, xu->wd, xu->ht, __func__);
    }
    fprintf(fp, "  gsave %% for bbox of oid %d\n", xu->oid);
    changebbox(fp);
    drawCrect(fp, xu->gx, xu->gy, xu->wd, xu->ht);
    if(xu->cob.rotateval) {
        drawCRrect(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.rotateval);
    }
    fprintf(fp, "  grestore %% for bbox\n");

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
    fprintf(fp, "  gsave %% for bbox of oid %d\n", xu->oid);
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
    fprintf(fp, "  gsave %% for bbox of oid %d\n", xu->oid);
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
    fprintf(fp, "  gsave %% for bbox of oid %d\n", xu->oid);
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
    fprintf(fp, "  gsave %% for bbox of oid %d\n", xu->oid);
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
    fprintf(fp, "  gsave %% for bbox of oid %d\n", xu->oid);
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
epsdraw_arrowhead(FILE *fp, int atype, int xdir, int lc, int x, int y)
{
    int  r;
    int  dx, dy;
    int  r2;

    fprintf(fp, "%% arrowhead atype %d xdir %d lc %d x,y %d,%d\n",
        atype, xdir, lc, x, y);
    fprintf(fp, "gsave\n");

    changecolor(fp, lc);

    switch(atype) {
    case AH_DIAMOND:
    case AH_WDIAMOND:
        r = def_arrowsize/3;

        if(atype==AH_WDIAMOND) {
        fprintf(fp,"gsave\n");
        fprintf(fp,"1 setgray\n");
        fprintf(fp, "%d %d moveto\n",    x,  y+r);
        fprintf(fp, "%d %d rlineto\n",   r, -r);
        fprintf(fp, "%d %d rlineto\n",  -r, -r);
        fprintf(fp, "%d %d rlineto\n",  -r,  r);
        fprintf(fp, "%d %d rlineto\n",   r,  r);
        fprintf(fp, "fill\n");
        fprintf(fp,"grestore\n");
        }

        fprintf(fp, "%d %d moveto\n",    x,  y+r);
        fprintf(fp, "%d %d rlineto\n",   r, -r);
        fprintf(fp, "%d %d rlineto\n",  -r, -r);
        fprintf(fp, "%d %d rlineto\n",  -r,  r);
        fprintf(fp, "%d %d rlineto\n",   r,  r);
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
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d %d 0 360 arc\n", x, y, r);
        fprintf(fp, "fill\n");
        fprintf(fp,"grestore\n");
        }

#if 0
        fprintf(fp, "%d %d moveto\n",   x,  y);
#endif
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d %d 0 360 arc\n", x, y, r);
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
        int k;
        double th;
        int d;
        k = objunit/4;
        r = k;
        th = acos(0.5);
        d = (int)((double)r - (double)r*sin(th));
        fprintf(fp, "%% th %f; k %d d %d\n", th, k, d);

#if 0
        fprintf(fp, "  gsave\n");
        fprintf(fp, "    0 0 1 setrgbcolor\n");
        fprintf(fp, "    %d %d moveto\n", x, y);
        fprintf(fp, "    %d %d rlineto\n", 0, k/2);
        fprintf(fp, "    %d %d rlineto\n", -k*2, 0);
        fprintf(fp, "    %d %d rlineto\n", 0, -k);
        fprintf(fp, "    %d %d rlineto\n", k*2, 0);
        fprintf(fp, "    closepath\n");
        fprintf(fp, "    stroke\n");
        fprintf(fp, "  grestore\n");
#endif

        fprintf(fp, "  gsave\n");
#if 0
        fprintf(fp, "    1 0 0 setrgbcolor\n");
#endif
        fprintf(fp, "    %d %d translate\n", x, y);
#if 0
        fprintf(fp, "    0 %d translate\n", objunit);
#endif
        fprintf(fp, "    %d rotate\n", xdir);

        fprintf(fp, "    %d %d moveto\n", 0, 0);
        fprintf(fp, "    %d %d %d 30 90 arc\n", 0-k+d, 0-k/2, r);
        fprintf(fp, "    %d %d rlineto\n", -k-d, 0);
        fprintf(fp, "    %d %d rlineto\n", 0, -k);
        fprintf(fp, "    %d %d %d -90 -30 arc\n", 0-k+d, 0+k/2, r);
        fprintf(fp, "    closepath\n");

        if(atype==AH_SHIP) {
        fprintf(fp, "    fill\n");
        }
        else {
        fprintf(fp, "    gsave\n");
        fprintf(fp, "    1 setgray\n");
        fprintf(fp, "    1 0 0 setrgbcolor\n");
        fprintf(fp, "    fill\n");
        fprintf(fp, "    grestore\n");

        fprintf(fp, "    %d %d moveto\n", 0, 0);
        fprintf(fp, "    %d %d %d 30 90 arc\n", 0-k+d, 0-k/2, r);
        fprintf(fp, "    %d %d rlineto\n", -k-d, 0);
        fprintf(fp, "    %d %d rlineto\n", 0, -k);
        fprintf(fp, "    %d %d %d -90 -30 arc\n", 0-k+d, 0+k/2, r);
        fprintf(fp, "    closepath\n");
        fprintf(fp, "    stroke\n");

        }
        fprintf(fp, "  grestore\n");

        }

        break;

    case AH_REVNORMAL:
        xdir = xdir + 180;
        r = def_arrowsize*2;

        dx =  (int)(r*cos((xdir+180)*rf));
        dy =  (int)(r*sin((xdir+180)*rf));
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        r = def_arrowsize;

#if 1
  {
    int dx2, dy2;
        dx =  (int)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180+def_arrowangle/2)*rf));

        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);

        dx2 =  (int)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy2 =  (int)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%d %d rlineto\n", dx2-dx, dy2-dy);
        fprintf(fp, "closepath fill\n");
  }
#endif

#if 0
        dx =  (int)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180+def_arrowangle/2)*rf));

        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);

        dx =  (int)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%d %d lineto\n", x+dx, x+dy);
        fprintf(fp, "closepath fill\n");
#endif



        break;

    case AH_REVWIRE:
        xdir = xdir + 180;
        r = def_arrowsize*2;

        dx =  (int)(r*cos((xdir+180)*rf));
        dy =  (int)(r*sin((xdir+180)*rf));
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        r = def_arrowsize;

        dx =  (int)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        dx =  (int)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        break;

    case AH_WIRE:
        r = def_arrowsize;

        fprintf(fp, "gsave\n");
        fprintf(fp, "  0 setlinejoin\n");

        dx =  (int)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "  %d %d moveto\n", x+dx, y+dy);
        fprintf(fp, "  %d %d lineto\n", x, y);

        dx =  (int)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "  %d %d lineto\n", x+dx, y+dy);
        fprintf(fp, "  stroke\n");

        fprintf(fp, "grestore\n");

        break;

    case AH_DOUBLE:
      {
        int dx1, dy1, dx2, dy2;

        r = def_arrowsize;

        dx =  (int)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180+def_arrowangle/2)*rf));
    dx1 = dx; dy1 = dy;
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);

        dx =  (int)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180-def_arrowangle/2)*rf));
    dx2 = dx; dy2 = dy;
#if 0
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);
#endif
        fprintf(fp, "%d %d lineto\n", x+dx, y+dy);
        fprintf(fp, "closepath fill\n");
    
  epsdraw_arrowhead(fp, AH_NORMAL, xdir, lc, x+(dx2+dx1)/2, y+(dy2+dy1)/2);
#if 0
#endif
      }

        break;

    case AH_ARROW3:
        r = def_arrowsize;

        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto\n",   x,  y);

        dx =  (int)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "%d %d rlineto\n", dx, dy);

        dx =  (int)(r*2/3*cos((xdir+180)*rf));
        dy =  (int)(r*2/3*sin((xdir+180)*rf));
        fprintf(fp, "%d %d lineto\n", x+dx, y+dy);

        dx =  (int)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%d %d lineto\n", x+dx, y+dy);

        fprintf(fp, "closepath fill\n");
        break;

    case AH_ARROW4:
     {
        int q;
        int w;
        int mx, my;

        r = def_arrowsize;
        w = r/8;

        SLW_14(fp);
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto\n",   x,  y);

        dx =  (int)(r*2/3*cos((xdir+180+def_arrowangle)*rf));
        dy =  (int)(r*2/3*sin((xdir+180+def_arrowangle)*rf));
        mx = dx + w/2*cos((xdir+180+def_arrowangle-90)*rf);
        my = dy + w/2*sin((xdir+180+def_arrowangle-90)*rf);
#if 0
        MX(1, x+mx, y+my);
#endif
        fprintf(fp, "%d %d %d %d %d arcn\n",
            x+mx, y+my, w/2,
            xdir+180+def_arrowangle+90,
            xdir+180+def_arrowangle-90
            );

        q = w/sin(def_arrowangle*rf);
        dx =  (int)(q*cos((xdir+180)*rf));
        dy =  (int)(q*sin((xdir+180)*rf));
#if 0
        MP(4, x+dx, y+dy);
#endif
        fprintf(fp, "%d %d lineto\n",  x+dx, y+dy);

        dx =  (int)(r*2/3*cos((xdir+180-def_arrowangle)*rf));
        dy =  (int)(r*2/3*sin((xdir+180-def_arrowangle)*rf));
        mx = dx + w/2*cos((xdir+180-def_arrowangle+90)*rf);
        my = dy + w/2*sin((xdir+180-def_arrowangle+90)*rf);
#if 0
        MX(1, x+mx, y+my);
#endif
        fprintf(fp, "%d %d %d %d %d arcn\n",
            x+mx, y+my, w/2,
            xdir+180-def_arrowangle+90,
            xdir+180-def_arrowangle-90
            );

#if 0
        fprintf(fp, "closepath stroke\n");
#endif
        fprintf(fp, "closepath fill\n");

    }

        break;

    case AH_WNORMAL:
        r = def_arrowsize;

        dx =  (int)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);

        dx =  (int)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180-def_arrowangle/2)*rf));
#if 0
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);
#endif
        fprintf(fp, "%d %d lineto\n", x+dx, y+dy);
        fprintf(fp, "closepath stroke\n");

        break;

    case AH_NORMAL:
    default:
        r = def_arrowsize;

        dx =  (int)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);

        dx =  (int)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (int)(r*sin((xdir+180-def_arrowangle/2)*rf));
#if 0
        fprintf(fp, "%d %d moveto\n",   x,  y);
        fprintf(fp, "%d %d rlineto\n", dx, dy);
#endif
        fprintf(fp, "%d %d lineto\n", x+dx, y+dy);
        fprintf(fp, "closepath fill\n");

        break;

    }
    fprintf(fp, "grestore\n");

    return 0;
}

int
epsdraw_Xarrowhead(FILE *fp, int atype, double xdir, int lc, double x, double y)
{
    double  r;
    double  dx, dy;

    fprintf(fp, "%% arrowhead %d\n", atype);

    changecolor(fp, lc);

    switch(atype) {
    case AH_DIAMOND:
    case AH_WDIAMOND:
        r = def_arrowsize/3;

        if(atype==AH_WDIAMOND) {
        fprintf(fp,"gsave\n");
        fprintf(fp,"1 setgray\n");
        fprintf(fp, "%f %f moveto\n",    x,  y+r);
        fprintf(fp, "%f %f rlineto\n",   r, -r);
        fprintf(fp, "%f %f rlineto\n",  -r, -r);
        fprintf(fp, "%f %f rlineto\n",  -r,  r);
        fprintf(fp, "%f %f rlineto\n",   r,  r);
        fprintf(fp, "fill\n");
        fprintf(fp,"grestore\n");
        }

        fprintf(fp, "%f %f moveto\n",    x,  y+r);
        fprintf(fp, "%f %f rlineto\n",   r, -r);
        fprintf(fp, "%f %f rlineto\n",  -r, -r);
        fprintf(fp, "%f %f rlineto\n",  -r,  r);
        fprintf(fp, "%f %f rlineto\n",   r,  r);
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
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f %f 0 360 arc\n", x, y, r);
        fprintf(fp, "fill\n");
        fprintf(fp,"grestore\n");
        }

#if 0
        fprintf(fp, "%f %f moveto\n",   x,  y);
#endif
        fprintf(fp, "newpath\n");
        fprintf(fp, "%f %f %f 0 360 arc\n", x, y, r);
        if(atype==AH_WCIRCLE) {
        fprintf(fp, "stroke\n");
        }
        else {
        fprintf(fp, "fill\n");
        }


        break;

    case AH_REVNORMAL:
        xdir = xdir + 180;
        r = def_arrowsize*2;

        dx =  (double)(r*cos((xdir+180)*rf));
        dy =  (double)(r*sin((xdir+180)*rf));
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        r = def_arrowsize;

#if 1
  {
    double dx2, dy2;
        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));

        fprintf(fp, "newpath\n");
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);

        dx2 =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy2 =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%f %f rlineto\n", dx2-dx, dy2-dy);
        fprintf(fp, "closepath fill\n");
  }
#endif

#if 0
        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));

        fprintf(fp, "newpath\n");
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%f %f lineto\n", x+dx, x+dy);
        fprintf(fp, "closepath fill\n");
#endif



        break;

    case AH_REVWIRE:
        xdir = xdir + 180;
        r = def_arrowsize*2;

        dx =  (double)(r*cos((xdir+180)*rf));
        dy =  (double)(r*sin((xdir+180)*rf));
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        r = def_arrowsize;

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        break;

    case AH_WIRE:
        r = def_arrowsize;

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);
        fprintf(fp, "stroke\n");

        break;

    case AH_DOUBLE:
      {
        double dx1, dy1, dx2, dy2;

        r = def_arrowsize;

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
    dx1 = dx; dy1 = dy;
        fprintf(fp, "newpath\n");
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
    dx2 = dx; dy2 = dy;
#if 0
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);
#endif
        fprintf(fp, "%f %f lineto\n", x+dx, y+dy);
        fprintf(fp, "closepath fill\n");
    
  epsdraw_Xarrowhead(fp, AH_NORMAL, xdir, lc, x+(dx2+dx1)/2, y+(dy2+dy1)/2);
#if 0
#endif
      }

        break;
    case AH_NORMAL:
    default:
        r = def_arrowsize;

        dx =  (double)(r*cos((xdir+180+def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180+def_arrowangle/2)*rf));
        fprintf(fp, "newpath\n");
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);

        dx =  (double)(r*cos((xdir+180-def_arrowangle/2)*rf));
        dy =  (double)(r*sin((xdir+180-def_arrowangle/2)*rf));
#if 0
        fprintf(fp, "%f %f moveto\n",   x,  y);
        fprintf(fp, "%f %f rlineto\n", dx, dy);
#endif
        fprintf(fp, "%f %f lineto\n", x+dx, y+dy);
        fprintf(fp, "closepath fill\n");

        break;

    }

    return 0;
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
        fprintf(fp, "%% Y-base\n");

        if(y1>y2) {
            int tmp;
            fprintf(fp, "%% swap x1,x2 y1,y2\n");
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
            fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
                ltype, x1, y1, x2, y2);
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
        fprintf(fp, "%% X-base\n");
        
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

    fprintf(fp, "%% seglineTICK type %d %d,%d-%d,%d\n",
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
    oxdir = xdir;

    if((xdir<125&&xdir>45)||(xdir<-45&&xdir>-135)) {
        fprintf(fp, "%% Y-base\n");

        if(y1>y2) {
            int tmp;
            fprintf(fp, "%% swap x1,x2 y1,y2\n");
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
            fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
                ltype, x1, y1, x2, y2);
        }

        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        ll = SQRT_2DD_I2D(x1,y1,x2,y2);
        cy = ((double)y2-y1)/sepw;
        px = (x2-x1)/cy;
        py = (y2-y1)/cy;

        fprintf(fp, "%% ll %d cy %.2f px %.2f py %.2f\n",
            ll, cy, px, py);

        if(ltype==LT_TRIANGLE || ltype==LT_MOUNTAIN) {
            hx = sepw*cos((oxdir)*rf);
            hy = sepw*sin((oxdir)*rf);
            tx = sepw*cos((oxdir+90)*rf);
            ty = sepw*sin((oxdir+90)*rf);

fprintf(fp, "%% hx,hy %.2f,%.2f tx,ty %.2f,%.2f\n", hx, hy, tx, ty);
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
        fprintf(fp, "%% X-base\n");
        
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
        fprintf(fp, "%% Y-base\n");

        if(y1>y2) {
            int tmp;
            fprintf(fp, "%% swap x1,x2 y1,y2\n");
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
            fprintf(fp, "%% segline type %d %d,%d-%d,%d\n",
                ltype, x1, y1, x2, y2);
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
        fprintf(fp, "%% X-base\n");
        
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
            rv = epsdraw_Xarrowhead(fp, AH_NORMAL, xdir, lc, x3, y3);
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

#if 0
    switch(ltype) {
    case LT_DOTTED:
    case LT_DASHED:
    case LT_CHAINED:
    case LT_DOUBLECHAINED:
        r = epsdraw_seglineSEP(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_WAVED:
    case LT_ZIGZAG:
        r = epsdraw_seglineTICK2(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_CIRCLE:
    case LT_WCIRCLE:
    case LT_TRIANGLE:
    case LT_MOUNTAIN:
        r = epsdraw_seglineTICK(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_DOUBLED:
        r = epsdraw_seglineW(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_CUTTED:
        r = epsdraw_seglineM(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_SOLID:
    default:
        changethick(fp, lt);
        changecolor(fp, lc);
#if 0
        fprintf(fp, "%% solid\n");
        fprintf(fp, "%d %d moveto\n", x1, y1);
        fprintf(fp, "%d %d lineto\n", x2, y2);
        fprintf(fp, "stroke\n");
#endif

        
        r = epsdraw_segwline_orig(fp, wlt, ltype, lt, lc, x1, y1, x2, y2);

        break;
    }
#endif
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

    fprintf(fp, "%% %s\n", __func__);
    fprintf(fp, "%%   xdir               %4d\n", xdir);
    fprintf(fp, "%%   line-type          %4d\n", xltype);
    fprintf(fp, "%%   arrowhead-part     %4d fore-type %4d back-type %4d\n",
            xahpart, xahfore, xahback);
    fflush(fp);

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
fitarc(FILE *fp, int x1, int y1, int x2, int y2, int ph)
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
    fprintf(fp, "gsave %d %d moveto %d %d lineto stroke grestore %% fitarc\n",
        x1, y1, x2, y2);

    fprintf(fp, "gsave %f %f moveto %f %f lineto stroke grestore %% fitarc\n",
        mx, my, px, py);
#endif

    if(ph==0) {
        fprintf(fp, "%f %f %f %f %f arcn %% fitarc\n",
            mx, my, r, na - 180, na);
    }
    if(ph==1) {
        fprintf(fp, "%f %f %f %f %f arc  %% fitarc\n",
            mx, my, r, na - 180, na);
    }

    return 0;
}


#define ADIR_ARC    (1)
#define ADIR_ARCN   (2)

#if 0
int
epsdraw_segarcXSEP(FILE *fp, 
    int ltype, int lt, int lc,
    int arcx, int arcy, int rad, int ang1, int ang2, int adir)
{
    int r;
    int x1, y1, x2, y2;
    int fr, pr;
    int f4;
    int ad;
    double a;
    int pp;
    double pa;
    double na;
    double ra;
    double *aa;
    int    al;
    int    c;
    int    i;

    int nx, ny;
    int gx, gy;
    int hx, hy, tx, ty;
    int oxdir;

    Echo("%s: arcx,y %d,%d rad %d ang1,arg2 %d,%d adir %d\n",
        __func__, arcx, arcy, rad, ang1, ang2, adir);
    fprintf(fp, "%% %s: arcx,y %d,%d rad %d ang1,arg2 %d,%d adir %d\n",
        __func__, arcx, arcy, rad, ang1, ang2, adir);
    fprintf(fp, "%% ltype %d lt %d lc %d\n", ltype, lt, lc);

    x1 = arcx + (int)((double)rad*cos(ang1*rf));
    y1 = arcy + (int)((double)rad*sin(ang1*rf));
    x2 = arcx + (int)((double)rad*cos(ang2*rf));
    y2 = arcy + (int)((double)rad*sin(ang2*rf));

    if(adir==ADIR_ARC) {
        ad = ang2 - ang1;
    }
    else {
        ad = ang1 - ang2;
    }

    fr = (int)((double)rad*M_PI);
    pr = (int)((double)fr*ad/360.0);
    f4 = (int)((double)fr/4.0);

    fprintf(fp, "  %% ad %d fr %d pr %d f4 %d\n", ad, fr, pr, f4);

    if(ltype==LT_DASHED) {
        pp = def_linedecothick;
    }
    else 
    if(ltype==LT_DOTTED) {
        pp = def_linedecothick/2;
    }
    else {
        pp = def_linedecothick/4;
    }

    pa = 2*M_PI*(((double)pp)/fr);
    al = (int)(2*M_PI*(((double)fr)/pp));

    fprintf(fp, "  %% pp %d pa %f al %d\n", pp, pa, al);
    fflush(fp);

    aa = (double*)alloca(sizeof(double)*(al+3));
    if(!aa) {
        Error("ERROR no-memory\n");
        return -1;
    }

    fprintf(fp, "gsave %% arcXSEP\n");

    if(adir==ADIR_ARC) {
        oxdir = 0;
    }
    else {
        oxdir = 180;
    }
    hx = pp*cos((oxdir)*rf);
    hy = pp*sin((oxdir)*rf);
    tx = pp*cos((oxdir+90)*rf);
    ty = pp*sin((oxdir+90)*rf);

    c = i = 0;
    if(adir==ADIR_ARC) {
        fprintf(fp, "%% a arc %f..%f by %f\n", ang1*rf, ang2*rf, pa);
        for(a=ang1*rf; a<=ang2*rf; a+=pa) {
            if(i>al-1) {
                Error("ERROR overrun %d/%d\n", i, al);
            }
            fprintf(fp, "%% a %f arc  set i %d/%d\n", a, i, al);
            aa[i++] = a;
        }
    }
    else {
        fprintf(fp, "%% a arcn %f..%f by %f\n", ang1*rf, ang2*rf, pa);
        for(a=ang1*rf; a>=ang2*rf; a-=pa) {
            if(i>al-1) {
                Error("ERROR overrun %d/%d\n", i, al);
            }
            fprintf(fp, "%% a %f arcn set i %d/%d\n", a, i, al);
            fflush(fp);
            aa[i++] = a;
        }
    }
    c = i;

    changethick(fp, lt);
    changecolor(fp, lc);

    for(i=0;i<c;i++) {
#if 0
        fprintf(fp, "%% tyr use i %d\n", i);
        fflush(fp);
#endif

        if(i>al-1) {
            Error("ERROR overrun %d/%d\n", i, al);
        }
        a = aa[i];
        fprintf(fp, "%% a %f use i %d\n", a, i);

        gx = arcx + rad*cos(a);
        gy = arcy + rad*sin(a);
#if 0
        MP(1, gx, gy);
#endif

        ra = a/rf+90;
#if 0
        fprintf(fp, "%% ra %f\n", ra);
#endif

        if(ltype==LT_CHAINED||ltype==LT_DOUBLECHAINED) {
            na = a + pa;
        }
        else {
            na = a + pa/2;
        }
#if 0
        fprintf(fp, "%% na %f\n", na);
#endif

#if 0
        nx = gx + pp*cos(na*rf);
        ny = gy + pp*sin(na*rf);
#endif
        nx = arcx + rad*cos(na);
        ny = arcy + rad*sin(na);

#if 0
        switch(ltype) {
        default:
            break;
        case LT_DOTTED:
        case LT_DASHED:
            fprintf(fp,
                "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n",
                (double)gx, (double)gy, (double)nx, (double)ny);
            break;
        case LT_CHAINED:
fprintf(fp, "%% i%%7 %d\n", i%7);
            if(i%7==4||i%7==6) {
                /*nothing*/
            }
            else {
            fprintf(fp,
                "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n",
                (double)gx, (double)gy, (double)nx, (double)ny);
            }
            break;
        case LT_DOUBLECHAINED:
fprintf(fp, "%% i%%9 %d\n", i%9);
            if(i%9==4||i%9==6||i%9==8) {
                /*nothing*/
            }
            else {
            fprintf(fp,
                "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n",
                (double)gx, (double)gy, (double)nx, (double)ny);
            }
            break;
        }
#endif
            
    }

    fprintf(fp, "grestore %% arcXSEP\n");

    return r;
}

int
epsdraw_segarcXTICK(FILE *fp, 
    int ltype, int lt, int lc,
    int arcx, int arcy, int rad, int ang1, int ang2, int adir)
{
    int r;
#if 0
    int x1, y1, x2, y2;
#endif
    int fr, pr;
    int f4;
    int ad;
    double a;
    int pp;
    double pa;
    double na;
    double ra;
    double *aa;
    int    al;
    int    c;
    int    i;

    int gx, gy;
    int hx, hy, tx, ty;
    int oxdir;

    Echo("%s: arcx,y %d,%d rad %d ang1,arg2 %d,%d adir %d\n",
        __func__, arcx, arcy, rad, ang1, ang2, adir);
    fprintf(fp, "%% %s: arcx,y %d,%d rad %d ang1,arg2 %d,%d adir %d\n",
        __func__, arcx, arcy, rad, ang1, ang2, adir);
    fprintf(fp, "%% ltype %d lt %d lc %d\n", ltype, lt, lc);

#if 0
    x1 = arcx + (int)((double)rad*cos(ang1*rf));
    y1 = arcy + (int)((double)rad*sin(ang1*rf));
    x2 = arcx + (int)((double)rad*cos(ang2*rf));
    y2 = arcy + (int)((double)rad*sin(ang2*rf));
#endif

    if(adir==ADIR_ARC) {
        ad = ang2 - ang1;
    }
    else {
        ad = ang1 - ang2;
    }

    fr = (int)((double)rad*M_PI);
    pr = (int)((double)fr*ad/360.0);
    f4 = (int)((double)fr/4.0);

    fprintf(fp, "  %% ad %d fr %d pr %d f4 %d\n", ad, fr, pr, f4);

    pp = def_linedecothick;
    pa = 2*M_PI*(((double)pp)/fr);
    al = (int)(2*M_PI*(((double)fr)/pp));

    fprintf(fp, "  %% pp %d pa %f al %d\n", pp, pa, al);
    fflush(fp);

    aa = (double*)alloca(sizeof(double)*(al+3));
    if(!aa) {
        Error("ERROR no-memory\n");
        return -1;
    }

    fprintf(fp, "gsave %% arcXTICK\n");

#if 0
    fprintf(fp, "gsave\n");
    fprintf(fp, "  1 0 0 setrgbcolor\n");
    MX(1, arcx, arcy);
    fprintf(fp, "  currentlinewidth 4 mul setlinewidth\n");
    fprintf(fp, "grestore\n");
#endif

    if(adir==ADIR_ARC) {
        oxdir = 0;
    }
    else {
        oxdir = 180;
    }
    hx = pp*cos((oxdir)*rf);
    hy = pp*sin((oxdir)*rf);
    tx = pp*cos((oxdir+90)*rf);
    ty = pp*sin((oxdir+90)*rf);

    c = i = 0;
    if(adir==ADIR_ARC) {
        fprintf(fp, "%% a arc %f..%f by %f\n", ang1*rf, ang2*rf, pa);
        for(a=ang1*rf; a<=ang2*rf; a+=pa) {
            if(i>al-1) {
                Error( "ERROR overrun %d/%d\n", i, al);
            }
            fprintf(fp, "%% a %f arc  set i %d/%d\n", a, i, al);
            aa[i++] = a;
        }
    }
    else {
        fprintf(fp, "%% a arcn %f..%f by %f\n", ang1*rf, ang2*rf, pa);
        for(a=ang1*rf; a>=ang2*rf; a-=pa) {
            if(i>al-1) {
                Error("ERROR overrun %d/%d\n", i, al);
            }
            fprintf(fp, "%% a %f arcn set i %d/%d\n", a, i, al);
            fflush(fp);
            aa[i++] = a;
        }
    }
    c = i;


    changethick(fp, lt);
    changecolor(fp, lc);

    for(i=0;i<c;i++) {
#if 0
        fprintf(fp, "%% tyr use i %d\n", i);
        fflush(fp);
#endif

        if(i>al-1) {
            Error("ERROR overrun %d/%d\n", i, al);
        }
        a = aa[i];
        fprintf(fp, "%% a %f use i %d\n", a, i);

        gx = arcx + rad*cos(a);
        gy = arcy + rad*sin(a);
#if 0
        MP(1, gx, gy);
#endif

        ra = a/rf+90;
#if 0
        fprintf(fp, "%% ra %f\n", ra);
#endif

        switch(ltype) {
        case LT_CIRCLE:
            fprintf(fp, "newpath %.2f %.2f %.2f 0 360 arc fill\n",
                (double)gx, (double)gy, (double)pp/2);
            break;
        case LT_WCIRCLE:
            fprintf(fp, "newpath %.2f %.2f %.2f 0 360 arc stroke\n",
                (double)gx, (double)gy, (double)pp/2);
            break;
        case LT_TRIANGLE:           
            fprintf(fp, "gsave %f %f translate %d rotate 0 0 moveto\n",
                (double)gx, (double)gy, (int)ra);
            fprintf(fp, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto %.2f %.2f lineto closepath fill\n",
                    (double)0, (double)0, (double)-tx, (double)-ty, (double)+hx, (double)+hy, (double)+tx, (double)+ty);
            fprintf(fp, "grestore\n");
            break;

        case LT_MOUNTAIN:
            fprintf(fp, "gsave %f %f translate %d rotate 0 0 moveto\n",
                (double)gx, (double)gy, (int)ra);
            fprintf(fp, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto stroke\n",
                (double)-tx, (double)-ty, (double)hx, (double)hy, (double)tx, (double)ty);

            fprintf(fp, "grestore\n");
            break;
        }
    }

    fprintf(fp, "grestore %% arcXTICK\n");

    return r;
}

int
epsdraw_segarcXTICK2(FILE *fp, 
    int ltype, int lt, int lc,
    int arcx, int arcy, int rad, int ang1, int ang2, int adir)
{
    int r;
#if 1
    int x1, y1, x2, y2;
#endif
    int fr, pr;
    int f4;
    int ad;
    double a;
    int pp;
    double pa;
    double na;
    double ra;
    double *aa;
    int    al;
    int    c;
    int    i;

    int gx, gy;
    int nx, ny;
    int hx, hy, tx, ty;
    int oxdir;

    int px, py, qx, qy;
    int ww;
    int lx, ly;

    Echo("%s: arcx,y %d,%d rad %d ang1,arg2 %d,%d adir %d\n",
        __func__, arcx, arcy, rad, ang1, ang2, adir);
    fprintf(fp, "%% %s: arcx,y %d,%d rad %d ang1,arg2 %d,%d adir %d\n",
        __func__, arcx, arcy, rad, ang1, ang2, adir);
    fprintf(fp, "%% ltype %d lt %d lc %d\n", ltype, lt, lc);

#if 1
    x1 = arcx + (int)((double)rad*cos(ang1*rf));
    y1 = arcy + (int)((double)rad*sin(ang1*rf));
    x2 = arcx + (int)((double)rad*cos(ang2*rf));
    y2 = arcy + (int)((double)rad*sin(ang2*rf));
#endif

    if(adir==ADIR_ARC) {
        ad = ang2 - ang1;
    }
    else {
        ad = ang1 - ang2;
    }

    fr = (int)((double)rad*M_PI);
    pr = (int)((double)fr*ad/360.0);
    f4 = (int)((double)fr/4.0);

    fprintf(fp, "  %% ad %d fr %d pr %d f4 %d\n", ad, fr, pr, f4);

    ww = def_linedecothick;

    pp = def_linedecothick/2;
    pa = 2*M_PI*(((double)pp)/fr);
    al = (int)(2*M_PI*(((double)fr)/pp));

    fprintf(fp, "  %% pp %d pa %f al %d\n", pp, pa, al);
    fflush(fp);

    aa = (double*)alloca(sizeof(double)*(al+3));
    if(!aa) {
        Error("ERROR no-memory\n");
        return -1;
    }

    fprintf(fp, "gsave %% arcXTICK2\n");

#if 0
    fprintf(fp, "gsave\n");
    fprintf(fp, "  1 0 0 setrgbcolor\n");
    MX(1, arcx, arcy);
    fprintf(fp, "  currentlinewidth 4 mul setlinewidth\n");
    fprintf(fp, "grestore\n");
#endif

    if(adir==ADIR_ARC) {
        oxdir = 0;
    }
    else {
        oxdir = 180;
    }
    hx = pp*cos((oxdir)*rf);
    hy = pp*sin((oxdir)*rf);
    tx = pp*cos((oxdir+90)*rf);
    ty = pp*sin((oxdir+90)*rf);

    c = i = 0;
    if(adir==ADIR_ARC) {
        fprintf(fp, "%% a arc %f..%f by %f\n", ang1*rf, ang2*rf, pa);
        for(a=ang1*rf; a<=ang2*rf; a+=pa) {
            if(i>al-1) {
                Error("ERROR overrun %d/%d\n", i, al);
            }
            fprintf(fp, "%% a %f arc  set i %d/%d\n", a, i, al);
            aa[i++] = a;
        }
    }
    else {
        fprintf(fp, "%% a arcn %f..%f by %f\n", ang1*rf, ang2*rf, pa);
        for(a=ang1*rf; a>=ang2*rf; a-=pa) {
            if(i>al-1) {
                Error("ERROR overrun %d/%d\n", i, al);
            }
            fprintf(fp, "%% a %f arcn set i %d/%d\n", a, i, al);
            fflush(fp);
            aa[i++] = a;
        }
    }
    c = i;


    changethick(fp, lt);
    changecolor(fp, lc);

    lx = x1;
    ly = y1;

    fprintf(fp, " %d %d moveto\n", x1, y1);

    for(i=0;i<c-1;i++) {
#if 0
        fprintf(fp, "%% tyr use i %d\n", i);
        fflush(fp);
#endif

        if(i>al-1) {
            Error("ERROR overrun %d/%d\n", i, al);
        }
        a = aa[i];
        fprintf(fp, "%% a %f use i %d\n", a, i);

        gx = arcx + rad*cos(a);
        gy = arcy + rad*sin(a);
#if 0
        MP(1, gx, gy);
#endif

        na = a + pa;
#if 0
        fprintf(fp, "%% na %f\n", na);
#endif

        nx = arcx + rad*cos(na);
        ny = arcy + rad*sin(na);
#if 0
        MX(1, nx, ny);
#endif

        ra = a/rf+90;
        px = gx + ww*cos((ra+45)*rf);
        py = gy + ww*sin((ra+45)*rf);
        qx = gx + ww*cos((ra-45)*rf);
        qy = gy + ww*sin((ra-45)*rf);
#if 0
        fprintf(fp, "%% ra %f\n", ra);
#endif

#if 0
        MQ(2, px, py);
        MQ(4, qx, qy);
#endif


#if 0
        fprintf(fp, "gsave\n");
        fprintf(fp, "newpath %d %d moveto %d %d lineto stroke\n",
            px, py, qx, qy);
        fprintf(fp, "grestore\n");
#endif

#if 0
        if(i==0) {
            fprintf(fp, " %d %d moveto\n", gx, gy);
        }
#endif

        switch(ltype) {

        case LT_ZIGZAG:
            if(i%2==0) {
                fprintf(fp, " %d %d lineto\n", px, py);
            }
            else 
            if(i%2==1) {
                fprintf(fp, " %d %d lineto\n", qx, qy);
            }
            break;

        case LT_WAVED:
            if(i%4==1) {
                fitarc(fp, lx, ly, nx, ny, 1);
                lx = nx;
                ly = ny;
            }
            else 
            if(i%4==3) {
                fitarc(fp, lx, ly, nx, ny, 0);
                lx = nx;
                ly = ny;
            }
            break;

#if 0
        case LT_CIRCLE:
            fprintf(fp, "newpath %.2f %.2f %.2f 0 360 arc fill\n",
                (double)gx, (double)gy, (double)pp/2);
            break;
        case LT_WCIRCLE:
            fprintf(fp, "newpath %.2f %.2f %.2f 0 360 arc stroke\n",
                (double)gx, (double)gy, (double)pp/2);
            break;
        case LT_TRIANGLE:           
            fprintf(fp, "gsave %f %f translate %d rotate 0 0 moveto\n",
                (double)gx, (double)gy, (int)ra);
            fprintf(fp, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto %.2f %.2f lineto closepath fill\n",
                    (double)0, (double)0, (double)-tx, (double)-ty, (double)+hx, (double)+hy, (double)+tx, (double)+ty);
            fprintf(fp, "grestore\n");
            break;

        case LT_MOUNTAIN:
            fprintf(fp, "gsave %f %f translate %d rotate 0 0 moveto\n",
                (double)gx, (double)gy, (int)ra);
            fprintf(fp, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto stroke\n",
                (double)-tx, (double)-ty, (double)hx, (double)hy, (double)tx, (double)ty);

            fprintf(fp, "grestore\n");
            break;
#endif
        }
    }

    fprintf(fp, " %d %d lineto\n", x2, y2);
    fprintf(fp, "stroke\n");
    fprintf(fp, "grestore %% arcXTICK2\n");

    return r;
}
#endif


static int __z = 0;

#if 0
int
epsdraw_segarcX(FILE *fp, 
    int ltype, int lt, int lc,
    int arcx, int arcy, int rad, int ang1, int ang2, int adir)
{
    int rv;
#if 1
    int x1, y1, x2, y2;
#endif
    int fr, pr;
    int f4;
    int ad;
    int hx, hy;

    rv = 0;

    Echo("%s: arcx,y %d,%d rad %d ang1,arg2 %d,%d adir %d\n",
        __func__, arcx, arcy, rad, ang1, ang2, adir);
    fprintf(fp, "%% %s: arcx,y %d,%d rad %d ang1,arg2 %d,%d adir %d\n",
        __func__, arcx, arcy, rad, ang1, ang2, adir);
    fprintf(fp, "%% ltype %d lt %d lc %d\n", ltype, lt, lc);

#if 1
    x1 = arcx + (int)((double)rad*cos(ang1*rf));
    y1 = arcy + (int)((double)rad*sin(ang1*rf));
    x2 = arcx + (int)((double)rad*cos(ang2*rf));
    y2 = arcy + (int)((double)rad*sin(ang2*rf));
#endif

    if(adir==ADIR_ARC) {
        ad = ang2 - ang1;
    }
    else {
        ad = ang1 - ang2;
    }

    fr = (int)((double)rad*M_PI);
    pr = (int)((double)fr*ad/360.0);
    f4 = (int)((double)fr/4.0);

    fprintf(fp, "  %% ad %d fr %d pr %d f4 %d\n", ad, fr, pr, f4);
    fflush(fp);

#if 0
    fprintf(fp, "gsave %% arcX\n");

    hx = arcx + (int)((double)rad/2*cos(ang2*rf));
    hy = arcy + (int)((double)rad/2*sin(ang2*rf));

/*
    MQ(4, hx, hy);
*/

    MX(4, arcx, arcy);
    fprintf(fp, "  0 0 1 setrgbcolor\n");
    fprintf(fp, "  currentlinewidth 4 div setlinewidth\n");

    if(adir==ADIR_ARC) {
        fprintf(fp, "  newpath\n");
        fprintf(fp, "  %d %d %d %d %d arc  stroke %% arcx arc\n",
            arcx, arcy, rad/2, ang1, ang2);
    }
    else {
        fprintf(fp, "  newpath\n");
        fprintf(fp, "  %d %d %d %d %d arcn stroke %% arcx arcn\n",
            arcx, arcy, rad/2, ang1, ang2);
    }

    if(adir==ADIR_ARC) {
        MTF(4, hx, hy, ang2+90);
    }
    else {
        MTF(4, hx, hy, ang2-90);
    }

    fprintf(fp, "grestore %% arcX\n");

#endif

    switch(ltype) {
    case LT_DOTTED:
    case LT_DASHED:
    case LT_CHAINED:
    case LT_DOUBLECHAINED:
        rv = epsdraw_segarcXSEP(fp, ltype, lt, lc,
                arcx, arcy, rad, ang1, ang2, adir);
        break;
    case LT_CIRCLE:
    case LT_WCIRCLE:
    case LT_TRIANGLE:
    case LT_MOUNTAIN:
        rv = epsdraw_segarcXTICK(fp, ltype, lt, lc,
                arcx, arcy, rad, ang1, ang2, adir);
        break;
    case LT_WAVED:
    case LT_ZIGZAG:
#if 0
        rv = epsdraw_seglineTICK2(fp, ltype, lt, lc, x1, y1, x2, y2);
#endif
#if 1
        rv = epsdraw_segarcXTICK2(fp, ltype, lt, lc,
                arcx, arcy, rad, ang1, ang2, adir);
#endif
        break;
#if 0
    case LT_DOUBLED:
        rv = epsdraw_seglineW(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
    case LT_CUTTED:
        rv = epsdraw_seglineM(fp, ltype, lt, lc, x1, y1, x2, y2);
        break;
#endif
    case LT_SOLID:
    default:
        changethick(fp, lt);
        changecolor(fp, lc);

        fprintf(fp, "  %% solid\n");
        if(adir==ADIR_ARC) {
            fprintf(fp, "  newpath\n");
            fprintf(fp, "  %d %d %d %d %d arc  stroke %% arcx arc\n",
                arcx, arcy, rad, ang1, ang2);
        }
        else {
            fprintf(fp, "  newpath\n");
            fprintf(fp, "  %d %d %d %d %d arcn stroke %% arcx arcn\n",
                arcx, arcy, rad, ang1, ang2);
        }
        break;
    }
        
    return rv;
}
#endif

#if 0
int
epsdraw_segarcXarrow(FILE *fp, 
    int arcx, int arcy, int rad, int ang1, int ang2, int adir,
    ob *xu, ns *xns)
{
    int r;
    Echo("%s: arcx,y %d, %d rad %d ang1,arg2 %d,%d\n",
        __func__, arcx, arcy, rad, ang1, ang2);

    r = epsdraw_segarcX(fp,
            xu->cob.outlinetype, xu->cob.outlinethick, xu->cob.outlinecolor, 
            arcx, arcy, rad, ang1, ang2, adir);

    return r;
}
#endif


int
epsdraw_segarc(FILE *fp, int cdir, int xox, int xoy,
    int x1, int y1, int x2, int y2,
    int arcx, int arcy, int rad, int ang, ob *xu, ns *xns)
{
    Echo("%s: x1,y1 %d,%d x2,y2 %d,%d arcx,y %d, %d rad %d ang %d\n",
        __func__, x1, y1, x2, y2, arcx, arcy, rad, ang);

    return 0;
}

int
epsdraw_segarcarrow(FILE *fp, int cdir, int xox, int xoy,
    int x1, int y1, int x2, int y2,
    int arcx, int arcy, int rad, int ang, ob *xu, ns *xns)
{
    int r;
    Echo("%s: x1,y1 %d,%d x2,y2 %d,%d arcx,y %d, %d rad %d ang %d\n",
        __func__, x1, y1, x2, y2, arcx, arcy, rad, ang);

    r = epsdraw_segarc(fp, cdir, xox, xoy, x1, y1, x2, y2,
            arcx, arcy, rad, ang, xu, xns);

    return r;
}



int
epsdraw_segarcn(FILE *fp, int cdir, int xox, int xoy,
    int x1, int y1, int x2, int y2,
    int arcx, int arcy, int rad, int ang, ob *xu, ns *xns)
{
    Echo("%s: x1,y1 %d,%d x2,y2 %d,%d arcx,y %d, %d rad %d ang %d\n",
        __func__, x1, y1, x2, y2, arcx, arcy, rad, ang);

    return 0;
}

int
epsdraw_segarcnarrow(FILE *fp, int cdir, int xox, int xoy,
    int x1, int y1, int x2, int y2,
    int arcx, int arcy, int rad, int ang, ob *xu, ns *xns)
{
    int r;
    Echo("%s: x1,y1 %d,%d x2,y2 %d,%d arcx,y %d, %d rad %d ang %d\n",
        __func__, x1, y1, x2, y2, arcx, arcy, rad, ang);

    r = epsdraw_segarcn(fp, cdir, xox, xoy, x1, y1, x2, y2,
            arcx, arcy, rad, ang, xu, xns);

    return r;
}




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
        fprintf(fp, "  %d %d %d %f %f arc\n", x1, y1, r, xdir-ag, xdir+ag);
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
        fprintf(fp, "  %d %d %d %f %f arc\n", x1+dx, y1+dy, r, xdir-ag, xdir+ag);
        fprintf(fp, "  stroke\n");

        r = r0*(p-0.1);

        fprintf(fp, "  newpath\n");
        fprintf(fp, "  %d %d %d %f %f arc\n", x2-dx, y2-dy, r, xdir+180-ag, xdir+180+ag);
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


int
_line_pathMM(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, int MM, ns *xns, int f_new, int f_close)
{
    int i;
    int x0, y0;
    int x1, y1, x2, y2;
    seg *s;
    int cdir;

    varray_t *qar;

    int ap, fh, bh;
    int arcx, arcy;
    int qbx, qby;
    int qex, qey;
    int qcx, qcy;


#if 1
Echo("%s: ydir %d xox %d xoy %d MM %d f_new %d f_close %d\n",
    __func__, ydir, xox, xoy, MM, f_new, f_close);
fprintf(fp, "%% %s: ydir %d xox %d xoy %d MM %d f_new %d f_close %d\n",
    __func__, ydir, xox, xoy, MM, f_new, f_close);

#endif

    if(MM==1) {
        qar = xu->cob.seghar;
    }
    else {
        qar = xu->cob.segar;
    }

    Echo("%s: enter MM %d qar %p %d\n",
        __func__, MM, qar, ((qar)? qar->use : -1));

    cdir = ydir;

    fprintf(fp, "%% %s: ydir %d xox %d xoy %d\n",
        __func__, ydir, xox, xoy);

    if(f_new) {
        fprintf(fp, "  newpath\n");
    }

#if 0
    if(!xu->cob.originalshape) {
        x1 = xox+xu->csx;
        y1 = xoy+xu->csy;
        x2 = xox+xu->cex;
        y2 = xoy+xu->cey;

        fprintf(fp, "  %d %d moveto %d %d lineto\n", x1, y1, x2, y2);
        goto out;
    }
#endif

    if(qar && qar->use>0) {
    }
    else {
        goto out;
    }

#if 0
    fprintf(fp, " gsave %% %s\n", __func__);
#endif

    Echo("    segar.use %d\n", qar->use);

#if 0
    x0 = x1 = xox+xu->csx;
    y0 = y1 = xoy+xu->csy;
#endif
        x0 = x1 = xox+xu->cx+xu->cox;
        y0 = y1 = xoy+xu->cy+xu->coy;

    Echo("    csx,csy %d,%d\n", xu->csx, xu->csy);
    Echo("    x1,y1 %d,%d\n", x1, y1);

    if(xu->type==CMD_CLINE) {
        x0 = x1 = xox+xu->cx+xu->cox;
        y0 = y1 = xoy+xu->cy+xu->coy;

        Echo("CLINE\n");
        Echo("    xox,xoy %6d,%-6d\n", xox,     xoy);
        Echo("    cx,cy   %6d,%-6d\n", xu->cx,  xu->cy);
        Echo("    csx,csy %6d,%-6d\n", xu->csx, xu->csy);
        Echo("    cox,coy %6d,%-6d\n", xu->cox, xu->coy);
        Echo("    x1,y1 %d,%d\n", x1, y1);
    }

    fprintf(fp, "  %d %d moveto %% starting-point\n", x1, y1);
#if 0
#endif

    for(i=0;i<qar->use;i++) {
        s = (seg*)qar->slot[i];
        if(!s) {
            continue;
        }

#if 0
Echo("%s: ptype %d\n", __func__, s->ptype);
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
#if 0
            fprintf(fp, " %d %d %d 0 360 arc fill %% join-mark\n",
                x1, y1, xu->cob.outlinethick*2);
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

            fprintf(fp, "    %d %d moveto ", x1, y1);
            fprintf(fp, "    %d %d %d %d %d arcn ",
                qcx, qcy, xu->cob.outlinethick*2, ydir-180, ydir);
            fprintf(fp, "    %d %d lineto", x2, y2);

            break;
        case OA_ARC:

#if 0
    fprintf(fp, "%%  cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
    fprintf(stderr, "%% b cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif

            arcx = x1 + s->rad*cos((cdir+90)*rf);
            arcy = y1 + s->rad*sin((cdir+90)*rf);

            x2 = arcx + s->rad*cos((cdir+s->ang-90)*rf);
            y2 = arcy + s->rad*sin((cdir+s->ang-90)*rf);
            

            fprintf(fp, "  %d %d %d %d %d arc %% seg-arc\n",
                arcx, arcy, s->rad, cdir-90, cdir-90+s->ang);

#if 1
            if(draft_mode) {
                int tx, ty; 
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
            
            fprintf(fp, "  %d %d %d %d %d arcn %% seg-arcn\n",
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
                fprintf(fp, "  closepath\n");
            }
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
            y2 = s->y1 + x2;
            fprintf(fp, "  %d %d moveto\n", x2, y2);
            goto next;
            break;

        case OA_LINETO:
            x2 = s->x1;
            y2 = s->y1;
            fprintf(fp, "  %d %d lineto\n", x2, y2);
            break;

        case OA_RLINETO:
            x2 = s->x1;
            y2 = s->y1;
            fprintf(fp, "  %d %d rlineto\n", x2, y2);
            break;

        case OA_LINE:
            x1 = s->x1+xox;
            y1 = s->y1+xoy;
            x2 = s->x2+xox;
            y2 = s->y2+xoy;
            fprintf(fp, "  %d %d moveto\n", x1, y1);
            fprintf(fp, "  %d %d lineto\n", x2, y2);
            break;


        default:

/* XXX */
            if(s->ftflag & COORD_FROM) {
P;
                Echo("    part seg %d: ftflag %d : %d,%d\n",
                    i, s->ftflag, s->x1, s->y1);
            
#if 1
                /* skip */
                continue;
#endif
#if 0
                goto confirm_arrow;
#endif
#if 0
                goto coord_done;
#endif
            }
            if(s->ftflag & COORD_TO) {
P;
                x2 = s->x1 + xox;
                y2 = s->y1 + xoy;
                goto coord_done;
            }

            x2 = x1+s->x1;
            y2 = y1+s->y1;

coord_done:

            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#if 0
fprintf(stderr, "%% m atan2 %f\n", atan2(y2-y1,x2-x1)/rf);
fprintf(stderr, "%% m cdir %d\n", cdir);
#endif

            Echo("    part seg %d: ftflag %d : %d,%d : %d,%d - %d,%d cdir %d\n",
                i, s->ftflag, s->x1, s->y1, x1, y1, x2, y2, cdir);

            ap = xu->cob.arrowheadpart;
            bh = xu->cob.arrowbackheadtype;
            fh = xu->cob.arrowforeheadtype;

            if(!xu->cob.arrowevery) {
                if(i==0) {
                    xu->cob.arrowheadpart &= ~AR_FORE;
                }
                else 
                if(i==qar->use-1) {
                    xu->cob.arrowheadpart &= ~AR_BACK;
                }
                else {
                    xu->cob.arrowheadpart = 0;
                }
            }

            fprintf(fp, "  %d %d lineto\n", x2, y2);

            xu->cob.arrowheadpart       = ap;
            xu->cob.arrowbackheadtype   = bh;
            xu->cob.arrowforeheadtype   = fh;
            break;

#if 0
        default:
            Error("unsupported segment part <%d>\n", s->ptype);
            break;
#endif
        }
        
next:
#if 0
        fprintf(fp, "%% a cdir %4d : %s\n", cdir, __func__);
#endif
        x1 = x2;
        y1 = y2;
    }

    if(xu->type==CMD_CLINE) {
        fprintf(fp, "  closepath\n");
    }

#if 0
    fprintf(fp, " grestore %% %s\n", __func__);
#endif

out:
    Echo("%s: leave\n", __func__);
    return 0;
}

#define _line_path(f,d,x,y,u,n)     _line_pathMM(f,d,x,y,u,0,n,1,1)
#define _line_Rpath(f,d,x,y,u,n)    _line_pathMM(f,d,x,y,u,1,n,1,1)


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
_line_patharrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int i;
    int x0, y0;
    int x1, y1, x2, y2;
    seg *s;
    int cdir;
    int tx, ty; 

    int ap, fh, bh;
    int arcx, arcy;
    int qbx, qby;
    int qex, qey;
    int qcx, qcy;

    int actfh, actch, actbh;

    Echo("%s: oid %d enter ydir %d xox %d xoy %d \n",
        __func__, xu->oid, ydir, xox, xoy);
    if(xu->cob.segar) {
        if(xu->cob.segar->use>0) {
fprintf(fp, "%% %s: oid %d segar %p use %d\n",
    __func__, xu->oid, xu->cob.segar, xu->cob.segar->use);
            fprintf(stdout, "segar oid %d ", xu->oid);
            varray_fprintv(stdout, xu->cob.segar);
        }
        else {
fprintf(fp, "%% %s: oid %d segar %p use -\n",
    __func__, xu->oid, xu->cob.segar);
        }
    }
    else {
fprintf(fp, "%% %s: no segar %p use -\n",
    __func__, xu->cob.segar);
    }

    cdir = ydir;

#if 0
    if(bbox_mode) {
#if 0
        epsdraw_bbox(fp, xu);
        epsdraw_bbox_glbrt(fp, xu);
#endif
        epsdraw_bbox_lbrt(fp, xox, xoy, xu);
    }
#endif

    if(xu->cob.outlinecolor<0||xu->cob.outlinethick<0) {
        fprintf(fp, "  %% %s: skip no color or no thickness\n", __func__);
        goto out;
    }

    fprintf(fp, "    %% %s: ydir %d xox %d xoy %d\n",
        __func__, ydir, xox, xoy);

    fprintf(fp, "    newpath\n");

    if(xu->cob.segar && xu->cob.segar->use>0) {
    }
    else {
        goto out;
    }

#if 0
    varray_fprintv(stdout, xu->cob.segar);
#endif
#if 0
    fprintf(fp, " gsave %% %s\n", __func__);
#endif

    Echo("    segar.use %d\n", xu->cob.segar->use);

#if 0
    x0 = x1 = xox+xu->csx;
    y0 = y1 = xoy+xu->csy;
#endif
        x0 = x1 = xox+xu->cx+xu->cox;
        y0 = y1 = xoy+xu->cy+xu->coy;

    Echo("    csx,csy %d,%d\n", xu->csx, xu->csy);
    Echo("    x1,y1 %d,%d\n", x1, y1);

    if(xu->type==CMD_CLINE) {
        x0 = x1 = xox+xu->cx+xu->cox;
        y0 = y1 = xoy+xu->cy+xu->coy;

        Echo("CLINE\n");
        Echo("    xox,xoy %6d,%-6d\n", xox,     xoy);
        Echo("    cx,cy   %6d,%-6d\n", xu->cx,  xu->cy);
        Echo("    csx,csy %6d,%-6d\n", xu->csx, xu->csy);
        Echo("    cox,coy %6d,%-6d\n", xu->cox, xu->coy);
        Echo("    x1,y1 %d,%d\n", x1, y1);
    }

    fprintf(fp, "    %d %d moveto %% zero-point\n", x1, y1);

    for(i=0;i<xu->cob.segar->use;i++) {
        s = (seg*)xu->cob.segar->slot[i];
        if(!s) {
            continue;
        }

#if 1
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
            if(i==xu->cob.segar->use/2) {
                actch = xu->cob.arrowcentheadtype;
            }
            if(i==xu->cob.segar->use-1) {
                actfh = xu->cob.arrowforeheadtype;
            }
        }

Echo("%s: oid %d i %d seg-arrow actbh %d actch %d achbh %d\n",
    __func__, xu->oid, i, actbh, actch, actfh);
Echo("%s: oid %d i %d s ptype %d x1,y1 %d,%d x2,y2 %d,%d\n",
    __func__, xu->oid, i, s->ptype, s->x1, s->y1, s->x2, s->y2);



        if(xu->cob.marknode) {
            markfdot(xu->cob.outlinecolor, x1, y1);
        }

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
#if 0
            fprintf(fp, " %d %d %d 0 360 arc fill %% join-mark\n",
                x1, y1, xu->cob.outlinethick*2);
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

            fprintf(fp, "    %d %d moveto ", x1, y1);
            fprintf(fp, "    %d %d %d %d %d arcn ",
                qcx, qcy, xu->cob.outlinethick*2, ydir-180, ydir);
            fprintf(fp, "    %d %d lineto", x2, y2);

            break;

        case OA_ARC:

#if 0
    fprintf(fp, "%%  cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
    fprintf(stderr, "%% b cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif

            arcx = x1 + s->rad*cos((cdir+90)*rf);
            arcy = y1 + s->rad*sin((cdir+90)*rf);
            x2 = arcx + s->rad*cos((cdir+s->ang-90)*rf);
            y2 = arcy + s->rad*sin((cdir+s->ang-90)*rf);
            
            fprintf(fp, "  %d %d %d %d %d arc %% seg-arc\n",
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

            if(actch) {
                tx = arcx + s->rad*cos((cdir-90+s->ang/2)*rf);
                ty = arcy + s->rad*sin((cdir-90+s->ang/2)*rf);
                fprintf(fp, "gsave\n");
                if(actch>0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir+s->ang/2,
                        xu->cob.outlinecolor, tx, ty);
                }
                if(actch<0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir+s->ang/2+180,
                        xu->cob.outlinecolor, tx, ty);
                }
                fprintf(fp, "grestore\n");
            }

            if(actfh>0) {
                fprintf(fp, "gsave\n");
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, cdir+s->ang,
                    xu->cob.outlinecolor, x2, y2);
                fprintf(fp, "grestore\n");
            }
            if(actbh>0) {
                fprintf(fp, "gsave\n");
                epsdraw_arrowhead(fp,
                    xu->cob.arrowbackheadtype, cdir-180,
                    xu->cob.outlinecolor, x1, y1);
                fprintf(fp, "grestore\n");
            }


            cdir += s->ang;

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
            
            fprintf(fp, "  %d %d %d %d %d arcn %% seg-arcn\n",
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

            if(actch) {
                tx = arcx + s->rad*cos((cdir+90-s->ang/2)*rf);
                ty = arcy + s->rad*sin((cdir+90-s->ang/2)*rf);
                fprintf(fp, "gsave\n");
                if(actch>0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir-90+s->ang/2,
                        xu->cob.outlinecolor, tx, ty);
                }
                if(actch<0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir-90+s->ang/2+180,
                        xu->cob.outlinecolor, tx, ty);
                }
                fprintf(fp, "grestore\n");
            }

            if(actfh>0) {
                fprintf(fp, "gsave\n");
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, cdir-s->ang,
                    xu->cob.outlinecolor, x2, y2);
                fprintf(fp, "grestore\n");
            }
            if(actbh>0) {
                fprintf(fp, "gsave\n");
                epsdraw_arrowhead(fp,
                    xu->cob.arrowbackheadtype, cdir-180,
                    xu->cob.outlinecolor, x1, y1);
                fprintf(fp, "grestore\n");
            }

            cdir -= s->ang;

            break;


        case OA_CLOSE:
            x2 = x1;
            y2 = y1;
            fprintf(fp, "  closepath\n");
            goto next;
            break;

        case OA_MOVETO:
            x2 = s->x1+xox;
            y2 = s->y1+xoy;
            fprintf(fp, "  %d %d moveto %% MOVETO\n", x2, y2);
            break;

        case OA_RMOVETO:
            x2 = s->x1;
            y2 = s->y1;
            fprintf(fp, "  %d %d rmoveto %% RMOVETO\n", x2, y2);
            break;

        case OA_LINETO:
            x2 = s->x1+xox;
            y2 = s->y1+xoy;
            fprintf(fp, "  %d %d lineto %% LINETO\n", x2, y2);
            break;

        case OA_RLINETO:
            x2 = s->x1;
            y2 = s->y1;
            fprintf(fp, "  %d %d rlineto %% RLINETO\n", x2, y2);
            break;

        case OA_LINE:
            x1 = s->x1+xox;
            y1 = s->y1+xoy;
            x2 = s->x2+xox;
            y2 = s->y2+xoy;
            fprintf(fp,
                "    %d %d moveto %d %d lineto %% LINE\n",
                x1, y1, x2, y2);

            goto confirm_arrow;

            break;

        default:

/* XXX */
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
                goto coord_done;
            }

P;
            x2 = x1+s->x1;
            y2 = y1+s->y1;
#if 0
            x2 = x1 + s->x1 + xox;
            y2 = y1 + s->y1 + xoy;
#endif

coord_done:

            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#if 0
fprintf(stderr, "%% m atan2 %f\n", atan2(y2-y1,x2-x1)/rf);
fprintf(stderr, "%% m cdir %d\n", cdir);
#endif

            fprintf(fp, "    %d %d lineto %% forward\n", x2, y2);

confirm_arrow:
#if 1
fprintf(fp, "%% arrow f %d c %d b %d; cdir %d\n", actfh, actch, actbh, cdir);
#endif
Echo("    arrow f %d c %d b %d; cdir %d\n", actfh, actch, actbh, cdir);

            if(actch) {
                fprintf(fp, "gsave\n");
                if(actch>0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir-s->ang,
                        xu->cob.outlinecolor, (x1+x2)/2, (y1+y2)/2);
                }
                if(actch<0) {
                    epsdraw_arrowhead(fp,
                        xu->cob.arrowcentheadtype, cdir-s->ang,
                        xu->cob.outlinecolor, (x1+x2)/2, (y1+y2)/2);
                }
                fprintf(fp, "grestore\n");
            }
            if(actfh>0) {
                fprintf(fp, "gsave\n");
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, cdir-s->ang,
                    xu->cob.outlinecolor, x2, y2);
                fprintf(fp, "grestore\n");
            }
            if(actbh>0) {
P;
                fprintf(fp, "gsave\n");
                epsdraw_arrowhead(fp,
                    xu->cob.arrowbackheadtype, cdir-180,
                    xu->cob.outlinecolor, x1, y1);
                fprintf(fp, "grestore\n");
            }
            
            break;

#if 0
        default:
            Error("unsupported segment part <%d>\n", s->ptype);
            break;
#endif
        }
        
next:
#if 0
        fprintf(fp, "%% a cdir %4d : %s\n", cdir, __func__);
#endif
        x1 = x2;
        y1 = y2;
    }

    if(xu->cob.marknode) {
        markfdot(xu->cob.outlinecolor, x1, y1);
    }

    if(xu->type==CMD_CLINE) {
        fprintf(fp, "    closepath\n");
    }

#if 0
    fprintf(fp, " grestore %% %s\n", __func__);
#endif

out:
    return 0;
}

int
symdraw(FILE *fp, double x, double y, double a, double pt, int c, int ty,
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
    fprintf(stdout, "%% %s: x %f y %f a %f c %d ty %d\n", __func__, x, y, a, c, ty);
    fflush(stdout);
#endif

#if 0
    fprintf(fp, "%% %s: x %f y %f a %f c %d ty %d\n", __func__, x, y, a, c, ty);
#endif

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
            fprintf(fp, "  %f %f moveto\n", px, py);
        }
        else {
            fprintf(fp, "  %f %f lineto\n", px, py);
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
            fitarc(fp, lax, lay, x, y, 0);
        }
        else 
        if(c%2==1) {
            fitarc(fp, lax, lay, x, y, 1);
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
        fprintf(fp, "  %f %f moveto %f %f lineto stroke\n", lax, lay, ax, ay);
#endif
        if(c==0) {
            fprintf(fp, "  %f %f moveto\n", ax, ay);
        }
        fprintf(fp, "  %f %f lineto\n", ax, ay);
        fprintf(fp, "  %f %f lineto\n", x, y);
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
        fprintf(fp, "  %f %f moveto %f %f lineto stroke\n", lax, lay, ax, ay);
#endif
        if(c==0) {
            fprintf(fp, "  %f %f moveto\n", ax, ay);
        }
        fprintf(fp, "  %f %f lineto\n", ax, ay);
        *cax = ax;
        *cay = ay;
#endif

#if 1
        if(c==0) {
            fprintf(fp, "  %f %f moveto\n", x, y);
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
            fprintf(fp, "  %f %f moveto %f %f lineto stroke\n", lax, lay, ax, ay);
#endif
            fprintf(fp, "  %f %f lineto\n", ax, ay);
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
            "  %9.2f %9.2f moveto %9.2f %9.2f rlineto stroke %% symdraw\n",
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
            "  %9.2f %9.2f moveto %9.2f %9.2f rlineto stroke %% symdraw\n",
            x1, y1, x2, y2);
    }

out:

    return 0;
}




int
__line_deco2(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
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

    int i;
    int x0, y0;
    int x1, y1, x2, y2;
    seg *s;
    int cdir;
    double dcdir;

    double lx, ly;
    double cx, cy;

    int ap, fh, bh;
    int arcx, arcy;
    int qbx, qby;
    int qex, qey;
    int qcx, qcy;

    int gsym, gpitch;

    int actfh, actch, actbh;

    int isfseg;
    int fsegtype;

P;
    changecolor(fp, xu->vob.outlinecolor);
    changethick(fp, xu->vob.outlinethick);

    if(xu->cob.outlinetype==LT_SOLID) {
P;
        _line_patharrow(fp, ydir, xox, xoy, xu, xns);
        fprintf(fp, "    stroke\n");
        return 0;
    }


    gsym = xu->cob.outlinetype;
    pitch = solve_pitch(xu->cob.outlinetype);
#if 1
    Echo("%s: gsym %d pitch %.2f (def_linedecopitch %d)\n",
        __func__, gsym, pitch, def_linedecopitch); 
#endif

#if 1
Echo("%s: ydir %d xox %d xoy %d linetype %d\n",
    __func__, ydir, xox, xoy, xu->cob.outlinetype);
#endif
    Echo("%s: enter\n", __func__);

    fprintf(fp, "%% %s: ydir %d xox %d xoy %d linetype %d\n",
        __func__, ydir, xox, xoy, xu->cob.outlinetype);

    cdir = ydir;
    dcdir = ydir;

    if(bbox_mode) {

        Echo("REMARK BEGIN oid %d\n", xu->oid);
#if 0
        Echo("xox,xoy %d,%d cx,cy %d,%d csx,csy %d,%d cox,coy %d,%d\n",
            xox, xoy, xu->cx, xu->cy, xu->csx, xu->csy, xu->cox, xu->coy);
#endif

#if 0
        Echo("  xox,xoy %d,%d ox,oy %d,%d\n",
            xox, xoy, xu->cox, xu->coy);
        Echo("  x,y %6d,%-6d sx,sy %6d,%-6d lbrt %6d,%6d,%6d,%6d\n",
            xu->cx, xu->cy, xu->csx, xu->csy,
            xu->clx, xu->cby, xu->crx, xu->cty);
        Echo("g x,y %6d,%-6d sx,sy %6d,%-6d lbrt %6d,%6d,%6d,%6d\n",
            xu->cgx, xu->cgy, xu->cgsx, xu->cgsy, 
            xu->cglx, xu->cgby, xu->cgrx, xu->cgty);
        Echo("LBRT  %6d,%-6d (%6d %6d %6d %6d)\n",
            xox, xoy, xu->lx, xu->by, xu->rx, xu->ty);
        Echo("GLBRT               (%6d %6d %6d %6d)\n",
            xu->glx, xu->gby, xu->grx, xu->gty);
#endif


#if 0
        Echo("REMARK END\n");
#endif
    }

    fprintf(fp, "  newpath\n");

#if 0
    if(!xu->cob.originalshape) {
        x1 = xox+xu->csx;
        y1 = xoy+xu->csy;
        x2 = xox+xu->cex;
        y2 = xoy+xu->cey;

#if 1
        fprintf(fp, " gsave    %% regular shape %s\n", __func__);
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x1, y1, x2, y2, xu, xns);
        fprintf(fp, " grestore %% regular shape %s\n", __func__);
#endif
#if 0
        fprintf(fp, "  %d %d moveto %d %d lineto\n", x1, y1, x2, y2);
#endif
        goto out;
    }
#endif

    if(xu->cob.segar && xu->cob.segar->use>0) {
    }
    else {
        goto out;
    }

#if 0
    fprintf(fp, " gsave %% %s\n", __func__);
#endif

    Echo("    segar.use %d\n", xu->cob.segar->use);

    changethick(fp, xu->vob.outlinethick);

#if 0
    x0 = x1 = xox+xu->csx;
    y0 = y1 = xoy+xu->csy;
#endif
        x0 = x1 = xox+xu->cx+xu->cox;
        y0 = y1 = xoy+xu->cy+xu->coy;

    Echo("    csx,csy %d,%d\n", xu->csx, xu->csy);
    Echo("    x1,y1 %d,%d\n", x1, y1);

    if(xu->type==CMD_CLINE) {
        x0 = x1 = xox+xu->cx+xu->cox;
        y0 = y1 = xoy+xu->cy+xu->coy;

        Echo("CLINE\n");
        Echo("    xox,xoy %6d,%-6d\n", xox,     xoy);
        Echo("    cx,cy   %6d,%-6d\n", xu->cx,  xu->cy);
        Echo("    csx,csy %6d,%-6d\n", xu->csx, xu->csy);
        Echo("    cox,coy %6d,%-6d\n", xu->cox, xu->coy);
        Echo("    x1,y1 %d,%d\n", x1, y1);
    }

#if 0
    fprintf(fp, "  %d %d moveto %% starting-point\n", x1, y1);
#endif

#if 0
    MCF(1, x1, y1);
#endif

    trip  = 0.0;
    count = 0;

    for(i=0;i<xu->cob.segar->use;i++) {
P;

        if(count>10) {
            printf("%s:%d oid %d count %d\n",
                __func__, __LINE__, xu->oid, count);
            fflush(stdout);
        }

        isfseg = 0;
        if(i>=xu->cob.segar->use-1) {
            isfseg = 1;
        }
#if 0
        if(xu->cob.arrowevery) {
        }
            isfseg = 1;
#endif

        s = (seg*)xu->cob.segar->slot[i];
        if(!s) {
            continue;
        }

#if 0
Echo("%s: ptype %d\n", __func__, s->ptype);
Echo("b cdir %d\n", cdir);
#endif

#if 0
        MP(4, x1, y1);
        fprintf(fp, "%d %d moveto (%d) show\n", x1, y1, i);
#endif
#if 0
        fprintf(fp, "%d %d moveto %% seg-start\n", x1, y1);
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
            if(i==xu->cob.segar->use%2) {
                actch = xu->cob.arrowcentheadtype;
            }
            if(i==xu->cob.segar->use-1) {
                actfh = xu->cob.arrowforeheadtype;
            }
        }

Echo("oid %d %s %d ptype %d seg-arrow actbh %d actch %d achbh %d\n",
    xu->oid, __func__, __LINE__,  s->ptype, actbh, actch, actfh);

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
#if 0
            fprintf(fp, " %d %d %d 0 360 arc fill %% join-mark\n",
                x1, y1, xu->cob.outlinethick*2);
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

#if 0
            fprintf(fp, "  gsave ");
            fprintf(fp, "    %d %d moveto ", x1, y1);
            fprintf(fp, "    %d %d %d %d %d arcn ",
                qcx, qcy, xu->cob.outlinethick*2, ydir-180, ydir);
            fprintf(fp, "    %d %d lineto stroke", x2, y2);
            fprintf(fp, "  grestore %% skip-mark\n");
#endif
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

#if 0
            fprintf(fp, "%% etrip %f pitch %f -> nd %d\n",
                etrip, pitch, nd);

            fprintf(fp, "%% i %d count %d: trip %f etrip %f\n",
                i, count, trip, etrip);
#endif

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
#if 0
            fprintf(fp, "%% i %d count %d: ui %f vi %f vpitch %f arc\n",
                    i, count, ui, vi, vpitch);
#endif

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

Echo("us %f vs %f; ue %f ve %f; etrip %f vi %f s->ang %f\n",
    us, vs, ue, ve, etrip, vi, (double)s->ang);

#if 0
            for(v=vi;v<=(double)s->ang;v+=vpitch) {
                px = arcx + s->rad*9/10*cos((cdir+v-90)*rf);
                py = arcy + s->rad*9/10*sin((cdir+v-90)*rf);
                MP(1, (int)px, (int)py);
            }

            for(v=vi+vs;v<=(double)s->ang-ve;v+=vpitch) {
                px = arcx + s->rad*8/10*cos((cdir+v-90)*rf);
                py = arcy + s->rad*8/10*sin((cdir+v-90)*rf);
                MC(4, (int)px, (int)py);
            }
#endif

            for(v=vi+vs;v<=(double)s->ang-ve;v+=vpitch) {
                px = arcx + s->rad*cos((cdir+v-90)*rf);
                py = arcy + s->rad*sin((cdir+v-90)*rf);
                ttrip = trip + (v*2*M_PI*s->rad)/360;

P;
                if(count>10) {
                    printf("%s:%d oid %d count %d\n",
                        __func__, __LINE__, xu->oid, count);
                    printf("  ui %f us %f etrip %f ue %f ; pitch %f\n",
                        ui, us, etrip, ue, pitch);
                    fflush(stdout);
                }

#if 0
                MP(4, (int)px, (int)py);
#endif
                if(xu->cob.markpitch) {
                    MP(1, (int)px, (int)py);
                }

#if 0
                fprintf(fp,
        "%% i %3d count %3d: ttrip %8.2f v %8.2f px,py %8.2f %8.2f arc\n",
                    i, count, ttrip, v, px, py);
#endif

                symdraw(fp, px, py, cdir+v, pitch, count, gsym, lx, ly, &cx, &cy);


                lx = cx;
                ly = cy;
                count++;
            }
            trip += etrip;
#if 0
                fprintf(fp, "%% i %d count %d: trip %f\n",
                    i, count, trip);
#endif

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
#if 0
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, cdir+v,
                    xu->cob.outlinecolor, x2, y2);
#endif
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, cdir+s->ang-ve/2,
                    xu->cob.outlinecolor, x2, y2);
            }
            if(actbh>0) {
#if 0
                epsdraw_arrowhead(fp,
                    xu->cob.arrowbackheadtype, cdir-180,
                    xu->cob.outlinecolor, x1, y1);
#endif
                epsdraw_arrowhead(fp,
                    xu->cob.arrowbackheadtype, cdir-180+vs/2,
                    xu->cob.outlinecolor, x1, y1);
            }

skip_arc:
            cdir += s->ang;
            dcdir += s->ang;

            break;


        case OA_ARCN:

            arcx = x1 + s->rad*cos((cdir-90)*rf);
            arcy = y1 + s->rad*sin((cdir-90)*rf);

            x2 = arcx + s->rad*cos((cdir-s->ang+90)*rf);
            y2 = arcy + s->rad*sin((cdir-s->ang+90)*rf);

#if 0
            MX(6, arcx, arcy);
            MQF(6, x2, y2);
#endif

            etrip = ((double)s->rad)*s->ang/rf;
            etrip = ((double)s->rad*2*M_PI)*((double)s->ang/360.0);
            nd = etrip / pitch;
            fprintf(fp, "%% etrip %f pitch %f -> nd %d\n",
                etrip, pitch, nd);


                fprintf(fp, "%% i %d count %d: trip %f etrip %f\n",
                    i, count, trip, etrip);

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

Echo("us %f vs %f; ue %f ve %f; etrip %f vi %f s->ang %f\n",
    us, vs, ue, ve, etrip, vi, (double)s->ang);

            fprintf(fp, "%% i %d count %d: ui %f vi %f vpitch %f arcn\n",
                    i, count, ui, vi, vpitch);
            for(v=vi+vs;v<=(double)s->ang-ve;v+=vpitch) {
                px = arcx + s->rad*cos((cdir-v+90)*rf);
                py = arcy + s->rad*sin((cdir-v+90)*rf);
                ttrip = trip + (v*2*M_PI*s->rad)/360;

                if(count>10) {
                    printf("%s:%d oid %d count %d\n",
                        __func__, __LINE__, xu->oid, count);
                    fflush(stdout);
                }

#if 0
                MP(4, (int)px, (int)py);
#endif
                if(xu->cob.markpitch) {
                    MP(1, (int)px, (int)py);
                }

                fprintf(fp,
                "%% i %3d count %3d: ttrip %8.2f v %8.2f px,py %8.2f %8.2f arcn\n",
                    i, count, ttrip, v, px, py);

                symdraw(fp, px, py, cdir-v, pitch, count, gsym, lx, ly, &cx, &cy);
                lx = cx;
                ly = cy;
                count++;
            }
            trip += etrip;
                fprintf(fp, "%% i %d count %d: trip %f\n",
                    i, count, trip);

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
#if 0
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, (int)(cdir-v),
                    xu->cob.outlinecolor, x2, y2);
#endif
                epsdraw_arrowhead(fp,
                    xu->cob.arrowforeheadtype, (int)(cdir-(s->ang-ve/2)),
                    xu->cob.outlinecolor, x2, y2);
            }
            if(actbh>0) {
                epsdraw_arrowhead(fp,
                    xu->cob.arrowbackheadtype, cdir-180-vs/2,
                    xu->cob.outlinecolor, x1, y1);
            }

skip_arcn:
            cdir -= s->ang;
            dcdir -= s->ang;

            break;

        case OA_CLOSE:
            x2 = x1;
            y2 = y1;
            fprintf(fp, "  closepath\n");
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
            goto next;
            break;

        case OA_RLINETO:
            x2 = s->x1 + x1;
            y2 = s->y1 + y1;
#if 0
            fprintf(fp, "  %d %d rlineto\n", x2, y2);
#endif
            goto coord_done;
            break;

        case OA_LINE:
            x1 = s->x1 + xox;
            y1 = s->y1 + xoy;
            x2 = s->x2 + xox;
            y2 = s->y2 + xoy;
            goto coord_done;
            break;

        default:

/* XXX */
            if(s->ftflag & COORD_FROM) {
P;
                Echo("    part seg %d: ftflag %d : %d,%d\n",
                    i, s->ftflag, s->x1, s->y1);
            
                /* skip */
                continue;
            }
            if(s->ftflag & COORD_TO) {
P;
                x2 = s->x1 + xox;
                y2 = s->y1 + xoy;
                goto coord_done;
            }

            x2 = x1+s->x1;
            y2 = y1+s->y1;

#if 0
            MC(2, x1, y1);
            MX(3, x2, y2);
#endif

coord_done:

            cdir = (int)(atan2(y2-y1,x2-x1)/rf);
            dcdir = atan2(y2-y1,x2-x1)/rf;
            v = atan2(y2-y1,x2-x1)/rf;

            etrip = SQRT_2DD_I2D(x1,y1,x2,y2);
            nd = etrip / pitch;
#if 0
            fprintf(fp, "%% x1,y1 %d,%d x2,y2 %d,%d ; cdir %d dcdir %f v %f\n",
                x1, y1, x2, y2, cdir, dcdir, v);
            fprintf(fp, "%% etrip %f pitch %f -> nd %d\n",
                etrip, pitch, nd);
            fprintf(fp, "%% i %d count %d: trip %f etrip %f\n",
                i, count, trip, etrip);
#endif

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

#if 0
            fprintf(fp, "%% i %d count %d: ui %f\n", i, count, ui);
#endif

#if 0
            fprintf(fp, "%% i %d count %d: ui %f us %f ue %f\n",
                i, count, ui, us, ue);
#endif

            ttrip = trip;
#if 0
                fprintf(fp,
    "%% i %3d count %3d: ttrip %8.2f u %8.2f x1,y1 %8.2f %8.2f line*\n",
                    i, count, ttrip, -1.0, (double)x1, (double)y1);
#endif

#if 0
            {
                int g=objunit/4;
                int lx, ly;

                u=ui+us;
                lx = x1 + (x2-x1) * u/etrip;
                ly = y1 + (y2-y1) * u/etrip;
                MCF(4, (int)lx,   (int)ly+g);
                MCF(4, (int)lx+g, (int)ly);
                u=etrip-ue;
                lx = (double)x1 + ((double)x2-(double)x1) * u/etrip;
                ly = (double)y1 + ((double)y2-(double)y1) * u/etrip;
                MQF(1, (int)lx,   (int)ly+g);
                MQF(1, (int)lx+g, (int)ly);
            }
#endif

#if 0
            px = x1;
            py = y1;
            qx = x1 + (x2-x1) * (ui+us)/etrip;
            qy = y1 + (y2-y1) * (ui+us)/etrip;
            fprintf(fp, "gsave\n");
            fprintf(fp, "0 1 1 setrgbcolor\n");
            SLW_x(fp, 2000);
            fprintf(fp, "%.2f %.2f moveto %.2f %.2f lineto %% pre-line\n",
                px, py, qx, qy);
            fprintf(fp, "stroke\n");
            fprintf(fp, "grestore\n");
#endif

            for(u=ui+us;u<=etrip-ue;u+=pitch) {
static double lpx, lpy;
double xd, yd;
#if 1
                px = x1 + (x2-x1) * u/etrip;
                py = y1 + (y2-y1) * u/etrip;
#endif

                ttrip = trip + u;

                xd = lpx - px;
                yd = lpy - py;
                lpx = px;
                lpy = py;

                if(count<10 || count>nd-10) {
                  if(_p_) {
#if 0
                    printf("%s:%d oid %d count %d\n",
                        __func__, __LINE__, xu->oid, count);
                    printf("  u %f ui %f us %f etrip %f ue %f ; pitch %f\n",
#endif
                    printf("%s:%d oid %d count %d u %.2f ui %.2f us %.2f etrip %.2f ue %.2f ; pitch %.2f\n",
                        __func__, __LINE__, xu->oid, count,
                        u, ui, us, etrip, ue, pitch);
                    printf("%s:%d oid %d count %d px %.2f py %.2f xd %.2f yd %.2f\n",
                        __func__, __LINE__, xu->oid, count,
                        px, py, xd, yd);
                    fflush(stdout);
                  }
                }

#if 0
                MP(4, (int)px, (int)py);
#endif
                if(xu->cob.markpitch) {
                    MP(1, (int)px, (int)py);
                }

#if 0
                fprintf(fp,
        "%% i %3d count %3d: ttrip %8.2f u %8.2f px,py %8.2f %8.2f line\n",
                    i, count, ttrip, u, px, py);
#endif

                symdraw(fp, px, py, v, pitch, count, gsym, lx, ly, &cx, &cy);
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
#if 0
                    fsegtype = FIN_LINE;
#endif
                    break;
                case LT_DBL:
                case LT_DBR:
                    fsegtype = FIN_SYM;
                    break;
                default:
#if 0
                    fsegtype = FIN_SYM;
#endif
                    break;
                }

                u = etrip-ue;
                px = x1 + (x2-x1) * u/etrip;
                py = y1 + (y2-y1) * u/etrip;

                switch(fsegtype) {
                case FIN_LINE:
 if(count>0) {
                    fprintf(fp, "%f %f lineto %% line-to-end\n", px, py);
 }
                    break;
                case FIN_ELINE:
 if(count>0) {
                    fprintf(fp, "%f %f lineto %% final-seg\n", cx, cy);
                    fprintf(fp, "%f %f lineto %% line-to-end\n", px, py);
 }
                    break;
                case FIN_SYM:
                    symdraw(fp, px, py, v, pitch, count, gsym, lx, ly, &cx, &cy);
                    break;
                }
            }


            trip += etrip;
            fprintf(fp, "%% i %d count %d: trip %f\n",
                i, count, trip);

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

            break;
        }
        
next:
        if(xu->cob.marknode) {
            markfdot(xu->cob.outlinecolor, x1, y1);
        }

#if 0
        fprintf(fp, "%% a cdir %4d : %s\n", cdir, __func__);
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

    if(xu->type==CMD_CLINE) {
#if 1
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x2, y2, x0, y0, xu, xns);
#endif
#if 0
        fprintf(fp, "  %d %d moveto %d %d lineto\n", x1, y1, x2, y2);
        fprintf(fp, "  closepath\n");
#endif
    }

#if 0
    fprintf(fp, " grestore %% %s\n", __func__);
#endif

out:
    return 0;
}

int
_line_deco2(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int ik;
    int orig_ltype;

#if 1
    if(xu->cob.markpath) {
        fprintf(fp, " gsave\n");
        changecolor(fp, def_markcolor);
        changethick(fp, xu->cob.outlinethick);
        ik = _line_path(fp, ydir, xox, xoy, xu, xns);
        fprintf(fp, "  stroke\n");
        fprintf(fp, " grestore\n");
    }
#endif

    orig_ltype = xu->cob.outlinetype;
    if(orig_ltype==LT_DOUBLED) {
        xu->cob.outlinetype = LT_DBL;
        ik = __line_deco2(fp, ydir, xox, xoy, xu, xns);
        xu->cob.outlinetype = LT_DBR;
        ik = __line_deco2(fp, ydir, xox, xoy, xu, xns);
        xu->cob.outlinetype = orig_ltype;
        return ik;
    }
    else
    if(orig_ltype==LT_LMUST || orig_ltype==LT_RMUST) {
        xu->cob.outlinetype = LT_SOLID;
        ik = __line_deco2(fp, ydir, xox, xoy, xu, xns);
        xu->cob.outlinetype = orig_ltype;
        ik = __line_deco2(fp, ydir, xox, xoy, xu, xns);
        return ik;
    }
    else {
        return __line_deco2(fp, ydir, xox, xoy, xu, xns);
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
QQ__solve_fandt(ns *xns, ob *u, varray_t *opar,
    int X, int *sx, int *sy, int *ex, int *ey)
{
    int    i;
    segop *e;
    int    ik;

    Echo("%s: enter\n", __func__);
    Echo("%s: sx,y %d,%d ex,y %d,%d\n", __func__, *sx, *sy, *ex, *ey);

    for(i=0;i<opar->use;i++) {
        e = (segop*)opar->slot[i];
        if(!e)
            continue;
        if(!e->val || !e->val[0]) {
            continue;
        }
        switch(e->cmd) {
        case OA_FROM:
            ik = _ns_find_objposP(xns, u, e->val, X, sx, sy);
            Echo("  FROM ik %d\n", ik);
            if(ik<0) {
                printf("ERROR not found label-'%s' as FROM\n", e->val);
                exit(11);
            }
            break;
        case OA_TO:
            ik = _ns_find_objposP(xns, u, e->val, X, ex, ey);
            Echo("  TO   ik %d\n", ik);
            if(ik<0) {
                printf("ERROR not found label-'%s' as TO\n", e->val);
                exit(11);
            }
            break;
        }
    }

    Echo("%s: sx,y %d,%d ex,y %d,%d\n", __func__, *sx, *sy, *ex, *ey);
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
_bez_deco(FILE *fp, ob *xu, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
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

    fprintf(fp, "%% %s\n", __func__);

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

    fprintf(fp, "%% %s: linetype %d\n", __func__, xu->cob.outlinetype);
    
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
            symdraw(fp, (double)rx, (double)ry, (double)a,
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
            _bez_deco(fp, xu, ux, uy, px, py, qx, qy, vx, vy);
#if 0
            _bez_deco(fp, xu, xox+ux, xoy+uy, xox+px, xoy+py, xox+qx, xoy+qy, xox+vx, xoy+vy);
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
            _bez_deco(fp, xu, ux, uy, tx, ty, tx, ty, vx, vy);
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
Zepsdraw_clinearrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int r;
    int aw, ah;
P;
#if 0
Echo("%s: enter\n", __func__);
#endif

    if(!xu->cob.originalshape) {
P;
        try_regline(xu->cob.segar, xu->csx, xu->csy, xu->cex, xu->cey);
    }

    if(xu->type==CMD_CLINE) {
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
            r = _line_path(fp, ydir, xox, xoy, xu, xns);
            fprintf(fp, "  clip\n");
    #if 0
            fprintf(fp, "  eoclip\n");
            fprintf(fp, "  stroke\n");
    #endif
            fprintf(fp, " %% bb %d %d %d %d\n", xu->lx, xu->by, xu->rx, xu->ty);
            fprintf(fp, " %% center %d %d\n", xu->x, xu->y);
            fprintf(fp, " %% xox %d xoy %d\n", xox, xoy);

            changethick(fp, xu->cob.hatchthick);
            fprintf(fp, "  %d %d translate\n", xu->x+xox, xu->y+xoy);
            epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor,
                xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, " grestore\n");
        }
        else {
            fprintf(fp, " %% no-fill\n");
        }
    }

    fprintf(fp, " gsave\n");
    changecolor(fp, xu->cob.outlinecolor);
    r = _line_deco2(fp, ydir, xox, xoy, xu, xns);
    fprintf(fp, " grestore\n");

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

    fprintf(stderr, "\t th1 %f th2 %f\n", th1, th2);
#endif

    ik1 = lineparam(&a, &b, (double)L1x1, (double)L1y1, (double)L1x2, (double)L1y2);
    ik2 = lineparam(&c, &d, (double)L2x1, (double)L2y1, (double)L2x2, (double)L2y2);

#if 0
    fprintf(stderr, "\t ik1 %d, ik2 %d\n", ik1, ik2);
    fprintf(stderr, "\t a %f b %f c %f d %f\n", a, b, c, d);
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
    fprintf(stderr, "\t rv %d, rcx %f, rcy %f\n", rv, *rcx, *rcy);
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
        fprintf(fp, "newpath %f %f %d 0 360 arc stroke\n",
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
        fprintf(fp, "newpath %f %f %d 0 360 arc stroke\n",
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


    if(xu->type==CMD_CLINE) {
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

    if(xu->type==CMD_CLINE) {
#if 1
        x0 = x1 = xox+xu->cx+xu->cox;
        y0 = y1 = xoy+xu->cy+xu->coy;
#endif

        Echo("CLINE\n");
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
        if(s->ftflag & COORD_FROM) {
P;
        Echo("    part seg %d: ftflag %d : %d,%d\n",
            i, s->ftflag, s->x1, s->y1);
            
            /* skip */
            continue;
        }
        if(s->ftflag & COORD_TO) {
P;
            x2 = s->x1 + xox;
            y2 = s->y1 + xoy;
            goto coord_done;
        }

        x2 = x1+s->x1;
        y2 = y1+s->y1;

coord_done:

        cdir = (int)(atan2(y2-y1,x2-x1)/rf);
#if 0
fprintf(stderr, "%% m atan2 %f\n", atan2(y2-y1,x2-x1)/rf);
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

    if(xu->type==CMD_CLINE) {
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
    fprintf(fp, "%% %s start aw %d ah %d hc %d hty %d\n",
        __func__, aw, ah, hc, hty);

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

fprintf(fp, "%% %s end\n", __func__);

    return 0;
}

int
_epsdraw_deco(FILE *fp, int xw, int xh, int xlc, int xfc, char *xcmd)
{
    int bw;
    int cr;

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
        p = draw_word(p, cmd, BUFSIZ, ',');
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
epsdraw_sstrbgX(FILE *fp, int x, int y, int wd, int ht, int ro,
        int bgshape, int qbgmargin, int fgcolor, int bgcolor,
        varray_t *ssar)
{
/*
 * 2pass routine
 *     1st - calcurate width and draw backgound round-box
 *     2nd - draw string
 */

    int   i;
    int   py;
    int   pyb;
    int   gy;
    sstr *uu;
    int   fht;
#if 0
    int   fsz;
#endif
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
    
    if(!ssar) {
        E;
        return -1;
    }

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
    Echo("%s: x,y %d,%d wd,ht %d,%d ro %d '%s'\n",
        __func__, x, y, wd, ht, ro, mcontall);
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
#if 0
    fht = def_textheight; 
    fsz = fht;
#endif
#if 0
    fht = def_textheight; 
    fsz = (int)(def_textheight*0.8); 
#endif
#if 0
    fsz = def_textheight; 
    fht = (int)(def_textheight*0.8); 
#endif

    pyb = (int)((double)fht*textdecentfactor);
    bgmargin = (int)((double)fht*textbgmarginfactor);

    py = fht;
    rh = ht-(n*py+bgmargin*2);

    fprintf(fp, "%%  fht %d, ht %d, n %d, rh %d, py %d\n",
        fht, ht, n, rh, py);

    Echo("ht %d use %d -> py %d\n", 
        ht, ssar->use, py);

    fprintf(fp, "gsave %% for sstr\n");
    fprintf(fp, "  %d %d translate\n", x, y);
    fprintf(fp, "  %d rotate\n", ro);

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

        justify = SJ_CENTER;
        hscale = 100;

        fprintf(fp, "    %% enter str %d '%s' gy %d\n", i, uu->ssval, gy);

        fprintf(fp, "    gsave %% oneline\n");

Echo("  --- calc size 1\n");

        cursize = FH_NORMAL;
        curface = FF_SERIF;
        hscale    = 100;


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
        fprintf(fp, "      /%s findfont %d scalefont setfont\n",
            def_fontname, fht);
        afh = fht;

        afhmax = -1;
    
        qs[0] = '\0';
        for(j=0;j<tq->use;j++) {
            te = tq->slot[j];
            
            if(te->ct==TXE_CMD) {
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
                    p = draw_word(p, token, BUFSIZ, ',');
                    if(token[0]) {
Echo("    token '%s'\n", token);
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
Echo(" newface %d newsize %d\n", newface, newsize);
                    if(newsize>=0) {
P;
                        cursize = newsize;
                    }
                    if(newface>=0) {
P;
                        curface = newface;
                    }
Echo(" curface %d cursize %d\n", curface, cursize);
                    afn  = rassoc(ff_act_ial, curface);
                    afhs = rassoc(fh_act_ial, cursize);
                    afh  = fht;
                    if(afhs!=NULL) {
                        afh = atof(afhs)*fht;
                    }

Echo("  afn '%s' afhs '%s' afh %d (max %d)\n", afn, afhs, afh, afhmax);

                    if(afn) {
                        fprintf(fp, "    /%s findfont %d scalefont setfont\n",
                            afn, afh);
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
        "      (%s) stringwidth pop /sstrw exch %f mul sstrw add def\n",
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


        fprintf(fp, "      %% justify %d\n", justify);
        
        switch(justify) {
        case SJ_LEFT:
            fprintf(fp, "      sstrw neg 0 translate\n");
            break;
        case SJ_RIGHT:
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

        justify = SJ_CENTER;
        hscale = 100;

        fprintf(fp, "    %% enter str %d '%s' gy %d\n", i, uu->ssval, gy);

        fprintf(fp, "    gsave %% oneline\n");


Echo("  --- calc size 2\n");

        cursize = FH_NORMAL;
        curface = FF_SERIF;
        hscale    = 100;


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



        fprintf(fp, "      %% justify %d\n", justify);
        
        switch(justify) {
        case SJ_LEFT:
            fprintf(fp, "      sstrw neg 0 translate\n");
            break;
        case SJ_RIGHT:
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

        /*** PASS 2 */

Echo("  --- drawing\n");
        fprintf(fp, "      /%s findfont %d scalefont setfont\n",
            def_fontname, fht);
        fprintf(fp, "      0 0 moveto\n");
#if 1
        changecolor(fp, fgcolor);
#endif

        cursize = FH_NORMAL;
        curface = FF_SERIF;
        hscale    = 100;

        for(j=0;j<tq->use;j++) {
            te = tq->slot[j];
            
            if(te->ct==TXE_CMD) {
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
                    p = draw_word(p, token, BUFSIZ, ',');
                    if(token[0]) {
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
                    }
                }
#if 0
                if(cursize != newsize || curface != newface) 
#endif
                {
Echo(" newface %d newsize %d\n", newface, newsize);
                    if(newsize>=0) {
P;
                        cursize = newsize;
                    }
                    if(newface>=0) {
P;
                        curface = newface;
                    }
Echo(" curface %d cursize %d\n", curface, cursize);

                    afn  = xrassoc(ff_act_ial, curface);
                    afhs = xrassoc(fh_act_ial, cursize);
                    afh  = fht;
                    if(afhs!=NULL) {
                        afh = atof(afhs)*fht;
                    }

Echo("  afn '%s' afhs '%s' afh %d\n", afn, afhs, afh);

                    if(afn) {
                        fprintf(fp, "      /%s findfont %d scalefont setfont\n",
                            afn, afh);
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
P;
                    fprintf(fp, "  gsave %% comp\n");
                    fprintf(fp, "    %.3f 1 scale\n", (double)hscale/100);
                }
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
epsdraw_circle(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int r;

    int aw, ah, ar;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;
    r  = xu->wd/2;

    if(xu->cob.imargin>0) {
        aw = xu->wd - xu->cob.imargin*2;
        ah = xu->ht - xu->cob.imargin*2;
        ar  = aw/2;
    }
    else {
        aw = xu->wd;
        ah = xu->ht;
        ar = r;
    }

apply:

    fprintf(fp, "%% circle no imargin\n");
    fprintf(fp, "%% circle xox,xoy %d,%d cx,cy %d,%d\n",
                    xox, xoy, xu->cx, xu->cy);

    fprintf(fp, "gsave %% for circle\n");


    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);
        if(xu->cob.fillhatch==HT_NONE) {
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            fprintf(fp, "gsave\n");
            fprintf(fp, "  newpath\n");
            fprintf(fp, "  %d %d %d 0 360 arc\n", x1, y1, ar);
            fprintf(fp, "  fill\n");
            fprintf(fp, "grestore\n");
        }
        else {
            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d translate\n", x1, y1);
            fprintf(fp, "  0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "  newpath\n");
            fprintf(fp, "  0 0 %d 0 360 arc\n", ar);
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }
            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, xu->wd, xu->ht, 
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);
            fprintf(fp, "grestore\n");
        }
    }

    if(xu->cob.outlinecolor>=0&&xu->cob.outlinethick>0) {

            fprintf(fp, "gsave\n");
            changecolor(fp, xu->cob.outlinecolor);
            changethick(fp, xu->cob.outlinethick);
            fprintf(fp, "  newpath\n");
            fprintf(fp, "  %d %d %d 0 360 arc\n", x1, y1, ar);
            fprintf(fp, "  stroke\n");
            fprintf(fp, "grestore\n");

    }

    fprintf(fp, "grestore %% for circle\n");
out:
    return 0;
}

#if 1
static
int
Gpolygon(FILE *fp, int n, double ar, int cc, double aoff)
{
    double br;
    int i;
    double x2, y2;
    double x3, y3;
    double x4, y4;

    br = ar*cos(M_PI*2/(double)n)/cos(M_PI*2/(double)n/2.0);

        fprintf(fp, "  newpath\n");
        for(i=0;i<n;i++) {
            x2 = ar*cos(M_PI*2/(double)n*(double)i+aoff);
            y2 = ar*sin(M_PI*2/(double)n*(double)i+aoff);
            x3 = br*cos(M_PI*2/(double)n*((double)i+0.5)+aoff);
            y3 = br*sin(M_PI*2/(double)n*((double)i+0.5)+aoff);
            x4 = ar*cos(M_PI*2/(double)n*((double)i+1.0)+aoff);
            y4 = ar*sin(M_PI*2/(double)n*((double)i+1.0)+aoff);
            if(i==0) {
                fprintf(fp, "  %.3f %.3f moveto\n", x2, y2);
            }
            else {
            }
            if(cc) {
                fprintf(fp, "  %.3f %.3f lineto\n", x3, y3);
            }
                fprintf(fp, "  %.3f %.3f lineto\n", x4, y4);
        }
        fprintf(fp, "  closepath\n");

    return 0;
}


int
epsdraw_polygonX(FILE *fp, int xox, int xoy, ob *xu, ns *xns, int cc)
{
    int    x1, y1, r;
    int    n;
    double aoff;
    int    aw, ah, ar;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;
    r  = (xu->wd/2);

    if(xu->cob.imargin>0) {
        fprintf(fp, "%% polygon w/ imagin\n");
        aw = xu->wd - xu->cob.imargin*2;
        ah = xu->ht - xu->cob.imargin*2;
        if(aw<ah) {
            ar = aw/2;
        }
        else {
            ar = ah/2;
        }
    }
    else {
        aw = xu->wd;
        ah = xu->ht;
        ar = r;
    }

    aoff = 0;
    if(xu->cob.polyrotate) {
        aoff = ((double)xu->cob.polyrotate*rf);
    }

    if(xu->cob.polypeak) {  
        n = (xu->cob.polypeak);
    }
    else {
        n = 3;
    }
    if(cc) {
        if(n<=4) {
            cc = 0;
        }
    }


    fprintf(fp, "gsave %% for polygon\n");

    if(xu->cob.fillcolor>=0) {
        if(xu->cob.fillhatch==HT_NONE) {
        }
        else 
        if(xu->cob.fillhatch==HT_SOLID) {
            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d translate\n", x1, y1);
            changecolor(fp, xu->cob.fillcolor);
            Gpolygon(fp, n, ar, cc, aoff);
            fprintf(fp, "  fill\n");
            fprintf(fp, "grestore\n");
        }
        else {

            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d translate\n", x1, y1);
            changecolor(fp, xu->cob.fillcolor);
            Gpolygon(fp, n, ar, cc, aoff);
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, xu->wd, xu->ht,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "grestore\n");
        }
    }

    if(xu->cob.outlinecolor>=0 && xu->cob.outlinethick>0) {
        fprintf(fp, "gsave\n");
        fprintf(fp, "  %d %d translate\n", x1, y1);
        changethick(fp, xu->cob.outlinethick);
        changecolor(fp, xu->cob.outlinecolor);
        Gpolygon(fp, n, ar, cc, aoff);
        fprintf(fp, "  stroke\n");
        fprintf(fp, "grestore\n");
    }

    fprintf(fp, "grestore %% for polygon\n");

out:
    return 0;
}
#endif

        
int
epsdraw_ellipse(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int r;
    double a;
    double aa;
    int aw, ah, ar;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;
    r  = (xu->wd/2);
    a  = ((double)xu->wd)/((double)xu->ht);

    if(xu->cob.imargin>0) {
        aw = xu->wd - xu->cob.imargin*2;
        ah = xu->ht - xu->cob.imargin*2;
        ar = aw/2;
        aa = ((double)aw)/((double)ah);
    }
    else {
        aw = xu->wd;
        ah = xu->ht;
        ar = r;
        aa = a;
    }
#if 0
#endif

apply:

    fprintf(fp, "%% ellipse no imargin\n");

    fprintf(fp, "gsave %% for ellipse\n");

    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);
        if(xu->cob.fillhatch==HT_NONE) {
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d translate\n", x1, y1);
            fprintf(fp, "  0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "  %.3f 1 scale\n", aa);
            fprintf(fp, "  newpath\n");
            fprintf(fp, "  0 0 %d 0 360 arc\n", ah/2);
            fprintf(fp, "  fill\n");
            fprintf(fp, "grestore\n");
        }
        else {
            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d translate\n", x1, y1);
            fprintf(fp, "  0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "  %.3f 1 scale\n", aa);
            fprintf(fp, "  newpath\n");
            fprintf(fp, "  0 0 %d 0 360 arc\n", ah/2);
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }
            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, xu->wd, xu->ht, 
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);
            fprintf(fp, "grestore\n");
        }
    }

    if(xu->cob.outlinecolor>=0 && xu->cob.outlinethick>0) {
        changethick(fp, xu->cob.outlinethick);
        changecolor(fp, xu->cob.outlinecolor);

        fprintf(fp, "gsave\n");
        fprintf(fp, "  %d %d translate\n", x1, y1);
        fprintf(fp, "  0 0 moveto %d rotate\n", xu->cob.rotateval);
        fprintf(fp, "  /oldm matrix currentmatrix def\n");
        fprintf(fp, "  %.3f 1 scale\n", aa);
        fprintf(fp, "  newpath\n");
        fprintf(fp, "  0 0 %d 0 360 arc\n", ah/2);
        fprintf(fp, "  oldm setmatrix\n");
        fprintf(fp, "  stroke\n");
        fprintf(fp, "grestore\n");
    }

    fprintf(fp, "grestore %% for ellipse\n");
out:
    return 0;
}

int
epsdraw_drum(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int r;
    double a;
    int aw, ah;
    int im;
    double aa;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;
    r  = (xu->wd/2);
    a  = ((double)xu->wd)/((double)xu->ht/4);

    if(xu->cob.imargin>0) {
        aw = xu->wd - xu->cob.imargin*2;
        ah = xu->ht - xu->cob.imargin*2;
        aa = ((double)aw)/((double)ah/4);
        im = xu->cob.imargin;
    }
    else {
        aw = xu->wd;
        ah = xu->ht;
        aa = a;
        im = 0;
    }

apply:

    fprintf(fp, "%% drum\n");
    fprintf(fp, "gsave %% for drum\n");

    fprintf(fp, "%% frame\n");

    fprintf(fp, "%% inside\n");

    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);
        if(xu->cob.fillhatch==HT_NONE) {
            /* nothing */
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            if(xu->cob.fillcolor>=0) {
                fprintf(fp, "gsave\n");
                fprintf(fp, "  %d %d translate\n", x1, y1);
                changethick(fp, xu->cob.outlinethick);
                fprintf(fp, "  gsave\n");
                fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
                fprintf(fp, "    /oldm matrix currentmatrix def\n");
                fprintf(fp, "    %.3f 1 scale\n", aa);
                fprintf(fp, "    %d %d moveto\n", (int)(-(aw/2)/aa), -ah*3/8);
                fprintf(fp, "    0 %d %d 180 360 arc\n", -ah*3/8, ah/8);
                fprintf(fp, "    %d %d lineto\n", (int)((aw/2)/aa), ah*3/8);
                fprintf(fp, "    0 %d %d 0 180 arc\n", ah*3/8, ah/8);
                fprintf(fp, "    oldm setmatrix\n");
                fprintf(fp, "    closepath\n");
                fprintf(fp, "    fill\n");
                fprintf(fp, "    newpath\n");
                fprintf(fp, "    /oldm matrix currentmatrix def\n");
                fprintf(fp, "    %.3f 1 scale\n", aa);
                fprintf(fp, "    0 %d %d 0 360 arc\n", ah*3/8, ah/8);
                fprintf(fp, "    oldm setmatrix\n");
                fprintf(fp, "    stroke\n");
                fprintf(fp, "  grestore\n");
                fprintf(fp, "grestore\n");
            }
        }
        else {
            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d translate\n", x1, y1);
            changecolor(fp, xu->cob.outlinecolor);
            changethick(fp, xu->cob.outlinethick);
            fprintf(fp, "  gsave\n");
            fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "    /oldm matrix currentmatrix def\n");
            fprintf(fp, "    %.3f 1 scale\n", aa);
            fprintf(fp, "    %d %d moveto\n", (int)(-(aw/2)/aa), -ah*3/8);
            fprintf(fp, "    0 %d %d 180 360 arc\n", -ah*3/8, ah/8);
            fprintf(fp, "    %d %d lineto\n", (int)((aw/2)/aa), ah*3/8);
            fprintf(fp, "    0 %d %d 0 180 arc\n", ah*3/8, ah/8);
            fprintf(fp, "    oldm setmatrix\n");
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");
            fprintf(fp, "grestore\n");
        }
    }

    if(xu->cob.outlinecolor>=0 &&xu->cob.outlinethick>0) {
        fprintf(fp, "gsave\n");
        fprintf(fp, "  %d %d translate\n", x1, y1);
        changecolor(fp, xu->cob.outlinecolor);
        changethick(fp, xu->cob.outlinethick);
        fprintf(fp, "  gsave\n");
        fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
        fprintf(fp, "    /oldm matrix currentmatrix def\n");
        fprintf(fp, "    %.3f 1 scale\n", aa);
        fprintf(fp, "    %d %d moveto\n", (int)(-(aw/2)/aa), -ah*3/8);
        fprintf(fp, "    0 %d %d 180 360 arc\n", -ah*3/8, ah/8);
        fprintf(fp, "    %d %d lineto\n", (int)((aw/2)/aa), ah*3/8);
        fprintf(fp, "    0 %d %d 0 180 arc\n", ah*3/8, ah/8);
        fprintf(fp, "    oldm setmatrix\n");
        fprintf(fp, "    closepath\n");
        fprintf(fp, "    stroke\n");
        fprintf(fp, "    newpath\n");
        fprintf(fp, "    /oldm matrix currentmatrix def\n");
        fprintf(fp, "    %.3f 1 scale\n", aa);
        fprintf(fp, "    0 %d %d 0 360 arc\n", ah*3/8, ah/8);
        fprintf(fp, "    oldm setmatrix\n");
        fprintf(fp, "    stroke\n");
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }


    fprintf(fp, "grestore %% for drum\n");

out:
    return 0;
}

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
epsdraw_box(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
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

    fprintf(fp, "%% box xy %d,%d wh %dx%d\n", x1, y1, aw, ah);
    fprintf(fp, "%%     bb %d %d %d %d\n", xu->lx, xu->by, xu->rx, xu->ty);
    fprintf(fp, "gsave %% for box\n");


    fprintf(fp, " %% mainbody\n");
    fprintf(fp, "    %d %d translate\n", x1, y1);
    fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);

    fprintf(fp, " %% inside\n");
    fprintf(fp, " %%    fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.fillcolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);


    /***
     *** CLIP and HATCH
     ***/

    if(xu->cob.fillhatch!=HT_NONE && xu->cob.fillcolor>=0) {
        fprintf(fp, "  %% clip & hatch\n");

        changecolor(fp, xu->cob.fillcolor);

        if(debug_clip) {
            _box_path(fp, x1, y1, aw, ah, r, EPSOP_STROKE);
        }
        else {
            _box_path(fp, x1, y1, aw, ah, r, EPSOP_CLIP);
        }

        changethick(fp, xu->cob.hatchthick);
        epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

        fprintf(fp, "  initclip\n");
    }
    else {
        fprintf(fp, " %% no-fill\n");
    }


#if 1
    fprintf(fp, " %% frame\n");
    fprintf(fp, " %%     outline color %d thick %d\n",
        xu->cob.outlinecolor, xu->cob.outlinethick);

    if(xu->cob.outlinecolor>=0 && xu->cob.outlinethick>0) {

        changecolor(fp, xu->cob.outlinecolor);
        changethick(fp, xu->cob.outlinethick);

        _box_path(fp, x1, y1, aw, ah, r, EPSOP_STROKE);
    }
#endif

    if(xu->cob.deco) {
        fprintf(fp, " %% deco |%s|\n", xu->cob.deco);
            _box_path(fp, x1, y1, aw, ah, r, EPSOP_CLIP);
            epsdraw_deco(fp, aw, ah,
                xu->cob.outlinecolor, xu->cob.fillcolor, xu->cob.deco);
    }
    else {
        fprintf(fp, "%% no-deco\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of box\n");

out:
    return 0;
}



int
mkpath_box(varray_t *sar, int wd, int ht, int rad)
{
P;
    if(rad<=0) {
        try_regsegmoveto(sar,  -wd/2, -ht/2);
        try_regsegrlineto(sar,    wd,     0);
        try_regsegrlineto(sar,     0,    ht);
        try_regsegrlineto(sar,   -wd,     0);
        try_regsegrlineto(sar,     0,   -ht);
    }
    else {
        try_regsegmoveto(sar,     -wd/2+rad,        -ht/2);
        try_regsegforward(sar,   wd-2*rad,            0);
        try_regsegarc(sar,            rad,           90);
        try_regsegforward(sar,          0,     ht-2*rad);
        try_regsegarc(sar,            rad,           90);
        try_regsegforward(sar,  -(wd-2*rad),          0);
        try_regsegarc(sar,            rad,           90);
        try_regsegforward(sar,          0,  -(ht-2*rad));
        try_regsegarc(sar,            rad,           90);
    }
    try_regsegclose(sar);

    return 0;
}


int
mkpath_Rbox(varray_t *sar, int wd, int ht, int rad)
{
P;
    if(rad<=0) {
        try_regsegmoveto(sar,   wd/2, -ht/2);
        try_regsegrlineto(sar,   -wd,     0);
        try_regsegrlineto(sar,     0,    ht);
        try_regsegrlineto(sar,    wd,     0);
        try_regsegrlineto(sar,     0,   -ht);
    }
    else {
        try_regsegmoveto(sar,      wd/2-rad,        -ht/2);
        try_regsegforward(sar,-(wd-2*rad),            0);
        try_regsegarcn(sar,           rad,           90);
        try_regsegforward(sar,          0,     ht-2*rad);
        try_regsegarcn(sar,           rad,           90);
        try_regsegforward(sar, (wd-2*rad),            0);
        try_regsegarcn(sar,           rad,           90);
        try_regsegforward(sar,          0,  -(ht-2*rad));
        try_regsegarcn(sar,           rad,           90);
    }
    try_regsegclose(sar);

    return 0;
}

int
mkpath_circle(varray_t *sar, int wd, int ht, int rad)
{
P;
    if(rad>0) {
        try_regsegmoveto(sar,     0,  -rad);
        try_regsegarc(sar,    rad,   360);
        try_regsegclose(sar);
    }
    else
    if(rad==0) {
        /* skip */
    }
    else {
        try_regsegmoveto(sar,     0, -ht/2);
        try_regsegarc(sar,   ht/2,   360);
        try_regsegclose(sar);
    }

    return 0;
}


int
mkpath_Rcircle(varray_t *sar, int wd, int ht, int rad)
{
P;
    if(rad>0) {
        try_regsegmoveto(sar,     0,    rad);
        try_regsegarcn(sar,    rad,   360);
        try_regsegclose(sar);
    }
    else
    if(rad==0) {
        /* skip */
    }
    else {
        try_regsegmoveto(sar,     0,   ht/2);
        try_regsegarcn(sar,   ht/2,   360);
        try_regsegclose(sar);
    }

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

Echo("polygon:  ar %f, br %f, aoff %d, n %d, cc %d\n", ar, br, aoff, n, cc);

    for(i=0;i<n;i++) {
        x2 = ar*cos(M_PI*2/(double)n*(double)i+aoff);
        y2 = ar*sin(M_PI*2/(double)n*(double)i+aoff);
        x3 = br*cos(M_PI*2/(double)n*((double)i+0.5)+aoff);
        y3 = br*sin(M_PI*2/(double)n*((double)i+0.5)+aoff);
        x4 = ar*cos(M_PI*2/(double)n*((double)i+1.0)+aoff);
        y4 = ar*sin(M_PI*2/(double)n*((double)i+1.0)+aoff);
        if(i==0) {
            try_regsegmoveto(sar,     x2, y2);
        }
        else {
        }
        if(cc) {
            try_regseglineto(sar,    x3, y3);
        }
            try_regseglineto(sar,    x4, y4);
    }
    try_regsegclose(sar);

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

Echo("Rpolygon: ar %f, br %f, aoff %d, n %d, cc %d\n", ar, br, aoff, n, cc);

    for(i=0;i<n;i++) {
        x2 = ar*cos(M_PI*2/(double)n*(double)-i+aoff);
        y2 = ar*sin(M_PI*2/(double)n*(double)-i+aoff);
        x3 = br*cos(M_PI*2/(double)n*((double)-i-0.5)+aoff);
        y3 = br*sin(M_PI*2/(double)n*((double)-i-0.5)+aoff);
        x4 = ar*cos(M_PI*2/(double)n*((double)-i-1.0)+aoff);
        y4 = ar*sin(M_PI*2/(double)n*((double)-i-1.0)+aoff);
        if(i==0) {
            try_regsegmoveto(sar,     x2, y2);
        }
        else {
        }
        if(cc) {
            try_regseglineto(sar,    x3, y3);
        }
            try_regseglineto(sar,    x4, y4);
    }
    try_regsegclose(sar);

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
    try_regsegmoveto(sar,     0, -ht/2);
    try_regsegarc(sar,   ht/2,   360);
#endif
    a = ((double)wd/ht);
Echo("a %9.2f\n", a);
    r = ((double)ht)/2;
    try_regsegmoveto(sar,     0, -r);
    lx = 0;
    ly = -r;
    for(y=-r;y<=r;y+=ep) {
        x = ((double)dir)*sqrt(r*r-y*y);
#if 0
fprintf(stderr, "u y %9.2f x %9.2f\n", y-ly, x-lx);
#endif
        try_regsegforward(sar, (int)(a*(x-lx)), (int)(y-ly));
        lx = x; ly = y;
    }
#if 0
    try_regsegforward(sar, 0, r);
#endif
    lx = 0;
    ly = r;
    for(y=r;y>=-r;y+=-ep) {
        x = ((double)-dir)*sqrt(r*r-y*y);
#if 0
fprintf(stderr, "d y %9.2f x %9.2f\n", y-ly, x-lx);
#endif
        try_regsegforward(sar, (int)(a*(x-lx)), (int)(y-ly));
        lx = x; ly = y;
    }
    try_regsegclose(sar);

    return 0;
}


#define mkpath_ellipse(a,w,h,r)     mkpath_ellipseXXX(a,w,h,r, 1)
#define mkpath_Rellipse(a,w,h,r)    mkpath_ellipseXXX(a,w,h,r,-1)

int
XXXmkpath_ellipse(varray_t *sar, int wd, int ht, int rad)
{
    double r;
    double x, y;
    double lx, ly;
    double a;
    double ep = 0.01*objunit;
P;
#if 0
    try_regsegmoveto(sar,     0, -ht/2);
    try_regsegarc(sar,   ht/2,   360);
#endif
    a = ((double)wd/ht);
Echo("a %9.2f\n", a);
    r = ((double)ht)/2;
    try_regsegmoveto(sar,     0, -r);
    lx = 0;
    ly = -r;
    for(y=-r;y<=r;y+=ep) {
        x = sqrt(r*r-y*y);
#if 0
fprintf(stderr, "u y %9.2f x %9.2f\n", y-ly, x-lx);
#endif
        try_regsegforward(sar, (int)(a*(x-lx)), (int)(y-ly));
        lx = x; ly = y;
    }
#if 0
    try_regsegforward(sar, 0, r);
#endif
    lx = 0;
    ly = r;
    for(y=r;y>=-r;y+=-ep) {
        x = -sqrt(r*r-y*y);
#if 0
fprintf(stderr, "d y %9.2f x %9.2f\n", y-ly, x-lx);
#endif
        try_regsegforward(sar, (int)(a*(x-lx)), (int)(y-ly));
        lx = x; ly = y;
    }
    try_regsegclose(sar);

    return 0;
}


int
mkpath_paper(varray_t *sar, int wd, int ht, int rad)
{
P;
    try_regsegmoveto(sar,  -wd/2, -ht/2);
    try_regsegrlineto(sar,    wd-ht/4,     0);
    try_regsegrlineto(sar,    ht/4, ht/4);
    try_regsegrlineto(sar,     0,    ht-ht/4);
    try_regsegrlineto(sar,   -wd,     0);
    try_regsegrlineto(sar,     0,   -ht);
    try_regsegclose(sar);

    return 0;
}

int
Gpaper_surface(FILE *fp, int wd, int ht)
{
    fprintf(fp, 
"  newpath %d %d moveto %d %d rlineto %d %d rlineto closepath fill %% paper-surface\n",
        wd/2-ht/4, -ht/2, ht/4, ht/4, -ht/4, 0);
}

int
mkpath_card(varray_t *sar, int wd, int ht, int rad)
{
P;
    try_regsegmoveto(sar,  -wd/2,   -ht/2);
    try_regsegrlineto(sar,    wd,       0);
    try_regsegrlineto(sar,     0, ht-ht/4);
    try_regsegrlineto(sar, -ht/4,    ht/4);
    try_regsegrlineto(sar, -wd+ht/4,    0);
    try_regsegrlineto(sar,     0,    -ht);
    try_regsegclose(sar);

    return 0;

}

int
mkpath_diamond(varray_t *sar, int wd, int ht, int rad)
{
P;
    try_regsegmoveto(sar,      0,   -ht/2);
    try_regsegrlineto(sar,  wd/2,    ht/2);
    try_regsegrlineto(sar, -wd/2,    ht/2);
    try_regsegrlineto(sar, -wd/2,   -ht/2);
    try_regsegclose(sar);

    return 0;
}

int
mkpath_Rdiamond(varray_t *sar, int wd, int ht, int rad)
{
P;
    try_regsegmoveto(sar,      0,   -ht/2);
    try_regsegrlineto(sar, -wd/2,    ht/2);
    try_regsegrlineto(sar,  wd/2,    ht/2);
    try_regsegrlineto(sar,  wd/2,   -ht/2);
    try_regsegclose(sar);

    return 0;
}

int
mkpath_house(varray_t *sar, int wd, int ht, int rad)
{
P;
    try_regsegmoveto(sar,  -wd/2,   -ht/2);
    try_regsegrlineto(sar,    wd,       0);
    try_regsegrlineto(sar,     0, ht-ht/4);
    try_regsegrlineto(sar, -wd/2,    ht/4);
    try_regsegrlineto(sar, -wd/2,   -ht/4);
    try_regsegclose(sar);

    return 0;
}

int
mkpath_Rhouse(varray_t *sar, int wd, int ht, int rad)
{
P;
    try_regsegmoveto(sar,  -wd/2,   -ht/2);
    try_regsegrlineto(sar,     0, ht-ht/4);
    try_regsegrlineto(sar,  wd/2,    ht/4);
    try_regsegrlineto(sar,  wd/2,   -ht/4);
    try_regsegrlineto(sar,     0, -ht+ht/4);
    try_regsegclose(sar);

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

    mx = xu->x+xox;
    my = xu->y+xoy;

fprintf(fp, "%% %s oid %d type %d\n", __func__, xu->oid, xu->type);
Echo("%s: oid %d type %d\n", __func__, xu->oid, xu->type);

    saved_segar = xu->cob.segar;

    a = 1.0;
    
    switch(xu->type) {
    case CMD_CHUNK:
    case CMD_BOX:
        ik = mkpath_box(xu->cob.segar, xu->wd, xu->ht, xu->cob.rad);
        ik = mkpath_Rbox(xu->cob.seghar, xu->wd, xu->ht, xu->cob.rad);
        break;
    case CMD_PAPER:
        ik = mkpath_paper(xu->cob.segar, xu->wd, xu->ht, xu->cob.rad);
        ik = mkpath_Rbox(xu->cob.seghar, xu->wd, xu->ht, xu->cob.rad);
        break;
    case CMD_CARD:
        ik = mkpath_card(xu->cob.segar, xu->wd, xu->ht, xu->cob.rad);
        ik = mkpath_Rbox(xu->cob.seghar, xu->wd, xu->ht, xu->cob.rad);
        break;
    case CMD_DIAMOND:
        ik = mkpath_diamond(xu->cob.segar, xu->wd, xu->ht, xu->cob.rad);
        ik = mkpath_Rdiamond(xu->cob.seghar, xu->wd, xu->ht, xu->cob.rad);
        break;
    case CMD_HOUSE:
        ik = mkpath_house(xu->cob.segar, xu->wd, xu->ht, xu->cob.rad);
        ik = mkpath_Rhouse(xu->cob.seghar, xu->wd, xu->ht, xu->cob.rad);
        break;
    case CMD_CIRCLE:
        ik = mkpath_circle(xu->cob.segar, xu->wd, xu->ht, xu->cob.rad);
        ik = mkpath_Rcircle(xu->cob.seghar, xu->wd, xu->ht, xu->cob.rad);
        break;
    case CMD_POLYGON:
        ik = mkpath_polygon(xu->cob.segar, xu->wd, xu->ht, xu->cob.rad,
                xu->cob.polyrotate, xu->cob.polypeak, xu->cob.concave);
        ik = mkpath_Rpolygon(xu->cob.seghar, xu->wd, xu->ht, xu->cob.rad,
                xu->cob.polyrotate, xu->cob.polypeak, xu->cob.concave);
        break;
    case CMD_ELLIPSE:
        ik = mkpath_ellipse(xu->cob.segar,  xu->wd, xu->ht, xu->cob.rad);
        ik = mkpath_Rellipse(xu->cob.seghar, xu->wd, xu->ht, xu->cob.rad);
        break;
    default:
        fprintf(fp, "%% unknown type %d\n", xu->type);
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

#if 1
    /*****
     ***** SHADE
     *****/
    if(xu->cob.shadow) {
            fprintf(fp, "  gsave    %% for shadow\n");
            fprintf(fp, "    %d -%d translate\n", objunit/10, objunit/10);
            changecolor(fp, xu->cob.outlinecolor);
            changethick(fp, xu->cob.hatchthick);
            ik = _line_path(fp, 0, 0, 0, xu, xns);
            fprintf(fp, "  fill     %% for shadow\n");
            fprintf(fp, "  grestore %% for shadow\n");
    }
#endif

    /*****
     ***** BACK
     *****/
    if(xu->cob.backhatch != HT_NONE && xu->cob.backcolor>=0) {
            fprintf(fp, "  gsave %% for clip+back\n");
            changecolor(fp, xu->cob.backcolor);
            changethick(fp, xu->cob.hatchthick);
            ik = _line_path(fp, 0, 0, 0, xu, xns);
            fprintf(fp, "  clip\n");

            epsdraw_hatch(fp, xu->wd, xu->ht,
              xu->cob.backcolor, xu->cob.backhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");
    }
    else {
        fprintf(fp, "  %% skip clip+back\n");
    }

    /*****
     ***** FILL
     *****/
    if(xu->cob.fillhatch != HT_NONE && xu->cob.fillcolor>=0) {
        if(xu->cob.hollow && xu->cob.seghar) {
            fprintf(fp, "  gsave %% for hollow+clip+fill\n");

            changecolor(fp, xu->cob.fillcolor);
            changethick(fp, xu->cob.hatchthick);
            ik = _line_pathMM(fp, 0, 0, 0, xu, 0, xns, 1, 0);

            fprintf(fp, "     0.8 0.8 scale\n");
            ik = _line_pathMM(fp, 0, 0, 0, xu, 1, xns, 0, 1);
            fprintf(fp, "  clip\n");

            fprintf(fp, "     1.25 1.25 scale\n");

            epsdraw_hatch(fp, xu->wd, xu->ht,
              xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");
         }
         else {
            fprintf(fp, "  gsave %% for clip+fill\n");
            changecolor(fp, xu->cob.fillcolor);
            changethick(fp, xu->cob.hatchthick);
            ik = _line_path(fp, 0, 0, 0, xu, xns);
            fprintf(fp, "  clip\n");

            epsdraw_hatch(fp, xu->wd, xu->ht,
              xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");
        }

    }
    else {
        fprintf(fp, "  %% skip clip+fill\n");
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
        ik = _line_deco2(fp, 0, 0, 0, xu, xns);

        /*** SURFACE ***/
        if(xu->type==CMD_PAPER) {
            fprintf(fp, "  %% surface\n");
            ik = Gpaper_surface(fp,  xu->wd, xu->ht);
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
        fprintf(fp, " %% deco |%s|\n", xu->cob.deco);
        fprintf(fp, " gsave %% for clip+fill\n");
        changecolor(fp, xu->cob.fillcolor);
        changethick(fp, xu->cob.hatchthick);
        ik = _line_path(fp, 0, 0, 0, xu, xns);
#if 0
        fprintf(fp, "  closepath\n");
#endif
        fprintf(fp, "  clip\n");
        epsdraw_deco(fp, xu->wd, xu->ht,
            xu->cob.outlinecolor, xu->cob.fillcolor, xu->cob.deco);
        fprintf(fp, " grestore\n");
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
    fprintf(fp, "%f %f scale\n", sx, sy);

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
    fprintf(fp, "%f %f scale\n", sx, sy);

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
    fprintf(fp, "%% gws %f ghs %f\n", gws, ghs);

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

        fprintf(fp, " %f %f scale\n", 1.0/gws, 1.0/ghs);

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
        fprintf(fp, "  %f setlinewidth %% for cloud\n", 
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

int
epsdraw_diamond(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int aw, ah;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;

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
    fprintf(fp, "gsave %% for box\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);

    fprintf(fp, "gsave %% for inside\n");

    /***
     *** CLIP and HATCH
     ***/

    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);

        if(xu->cob.fillhatch==HT_NONE) {
            /* nothing */
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            if(xu->cob.fillcolor>=0) {
                fprintf(fp, "  %% solid fill\n");
                fprintf(fp, "  gsave\n");
                fprintf(fp, "    %d %d translate\n", x1, y1);
                fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
                fprintf(fp, "    newpath\n");

                fprintf(fp, "    %d %d moveto\n",  0, -ah/2);
                fprintf(fp, "    %d %d rlineto\n", aw/2, ah/2);
                fprintf(fp, "    %d %d rlineto\n", -aw/2, ah/2);
                fprintf(fp, "    %d %d rlineto\n", -aw/2, -ah/2);
                fprintf(fp, "    %d %d rlineto\n", aw/2, -ah/2);

                fprintf(fp, "    closepath\n");
                fprintf(fp, "    fill\n");
                fprintf(fp, "  grestore\n");

            }

        }
        else {

            fprintf(fp, "  %% clip & hatch\n");
            fprintf(fp, "  gsave\n");
            fprintf(fp, "    %d %d translate\n", x1, y1);
            fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    0 setlinewidth\n");

            fprintf(fp, "    %d %d moveto\n",  0, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw/2, ah/2);
            fprintf(fp, "    %d %d rlineto\n", -aw/2, ah/2);
            fprintf(fp, "    %d %d rlineto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw/2, -ah/2);

            fprintf(fp, "    closepath\n");
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");

        }

    }

    fprintf(fp, "grestore %% for inside\n");


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

        fprintf(fp, "    %d %d moveto\n",  0, -ah/2);
        fprintf(fp, "    %d %d rlineto\n", aw/2, ah/2);
        fprintf(fp, "    %d %d rlineto\n", -aw/2, ah/2);
        fprintf(fp, "    %d %d rlineto\n", -aw/2, -ah/2);
        fprintf(fp, "    %d %d rlineto\n", aw/2, -ah/2);

        fprintf(fp, "    closepath\n");
        fprintf(fp, "    stroke\n");
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of box\n");

out:
    return 0;
}

int
epsdraw_house(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int aw, ah;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;

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
    fprintf(fp, "gsave %% for box\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);

    fprintf(fp, "gsave %% for inside\n");

    /***
     *** CLIP and HATCH
     ***/

    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);

        if(xu->cob.fillhatch==HT_NONE) {
            /* nothing */
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            if(xu->cob.fillcolor>=0) {
                fprintf(fp, "  %% solid fill\n");
                fprintf(fp, "  gsave\n");
                fprintf(fp, "    %d %d translate\n", x1, y1);
                fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
                fprintf(fp, "    newpath\n");

                fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
                fprintf(fp, "    %d %d rlineto\n", aw, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, ah-ah/4);
                fprintf(fp, "    %d %d rlineto\n", -aw/2, ah/4);
                fprintf(fp, "    %d %d rlineto\n", -aw+aw/2, -ah/4);
                fprintf(fp, "    %d %d rlineto\n", 0, -ah+ah/4);

                fprintf(fp, "    closepath\n");
                fprintf(fp, "    fill\n");
                fprintf(fp, "  grestore\n");

            }

        }
        else {

            fprintf(fp, "  %% clip & hatch\n");
            fprintf(fp, "  gsave\n");
            fprintf(fp, "    %d %d translate\n", x1, y1);
            fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    0 setlinewidth\n");

            fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah-ah/4);
            fprintf(fp, "    %d %d rlineto\n", -aw/2, ah/4);
            fprintf(fp, "    %d %d rlineto\n", -aw+aw/2, -ah/4);
            fprintf(fp, "    %d %d rlineto\n", 0, -ah+ah/4);

            fprintf(fp, "    closepath\n");
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");

#if 0
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", -aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, -ah);
            fprintf(fp, "    closepath\n");
            fprintf(fp, "    stroke\n");

#endif

        }

    }

    fprintf(fp, "grestore %% for inside\n");


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

        fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
        fprintf(fp, "    %d %d rlineto\n", aw, 0);
        fprintf(fp, "    %d %d rlineto\n", 0, ah-ah/4);
        fprintf(fp, "    %d %d rlineto\n", -aw/2, ah/4);
        fprintf(fp, "    %d %d rlineto\n", -aw+aw/2, -ah/4);
        fprintf(fp, "    %d %d rlineto\n", 0, -ah+ah/4);

        fprintf(fp, "    closepath\n");
        fprintf(fp, "    stroke\n");
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of box\n");

out:
    return 0;
}


int
epsdraw_card(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int aw, ah;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;

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
    fprintf(fp, "gsave %% for box\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);

    fprintf(fp, "gsave %% for inside\n");

    /***
     *** CLIP and HATCH
     ***/

    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);

        if(xu->cob.fillhatch==HT_NONE) {
            /* nothing */
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            if(xu->cob.fillcolor>=0) {
                fprintf(fp, "  %% solid fill\n");
                fprintf(fp, "  gsave\n");
                fprintf(fp, "    %d %d translate\n", x1, y1);
                fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
                fprintf(fp, "    newpath\n");
                fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
                fprintf(fp, "    %d %d rlineto\n", aw, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, ah-aw/8);
                fprintf(fp, "    %d %d rlineto\n", -aw/4, aw/8);
                fprintf(fp, "    %d %d rlineto\n", -aw+aw/4, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, -ah);
                fprintf(fp, "    closepath\n");
                fprintf(fp, "    fill\n");
                fprintf(fp, "  grestore\n");

            }

        }
        else {

            fprintf(fp, "  %% clip & hatch\n");
            fprintf(fp, "  gsave\n");
            fprintf(fp, "    %d %d translate\n", x1, y1);
            fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    0 setlinewidth\n");
            fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah-aw/8);
            fprintf(fp, "    %d %d rlineto\n", -aw/4, aw/8);
            fprintf(fp, "    %d %d rlineto\n", -aw+aw/4, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, -ah);
            fprintf(fp, "    closepath\n");
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");

#if 0
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", -aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, -ah);
            fprintf(fp, "    closepath\n");
            fprintf(fp, "    stroke\n");

#endif

        }

    }

    fprintf(fp, "grestore %% for inside\n");


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
        fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
        fprintf(fp, "    %d %d rlineto\n", aw, 0);
        fprintf(fp, "    %d %d rlineto\n", 0, ah-aw/8);
        fprintf(fp, "    %d %d rlineto\n", -aw/4, aw/8);
        fprintf(fp, "    %d %d rlineto\n", -aw+aw/4, 0);
        fprintf(fp, "    %d %d rlineto\n", 0, -ah);
        fprintf(fp, "    closepath\n");
        fprintf(fp, "    stroke\n");
#if 0
        fprintf(fp, "    %d %d moveto\n", aw/4, -ah/2);
        fprintf(fp, "    %d %d rlineto\n", aw/8, ah/3);
        fprintf(fp, "    %d %d rlineto\n", aw/8, -(ah/3-aw/8));
        fprintf(fp, "    closepath\n");
        fprintf(fp, "    fill\n");
#endif
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of box\n");

out:
    return 0;
}

int
epsdraw_paper(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int aw, ah;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;

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
    fprintf(fp, "gsave %% for box\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);

    fprintf(fp, "gsave %% for inside\n");

    /***
     *** CLIP and HATCH
     ***/

    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);

        if(xu->cob.fillhatch==HT_NONE) {
            /* nothing */
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            if(xu->cob.fillcolor>=0) {
                fprintf(fp, "  %% solid fill\n");
                fprintf(fp, "  gsave\n");
                fprintf(fp, "    %d %d translate\n", x1, y1);
                fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
                fprintf(fp, "    newpath\n");
                fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
                fprintf(fp, "    %d %d rlineto\n", aw-aw/4, 0);
                fprintf(fp, "    %d %d rlineto\n", aw/4, aw/8);
                fprintf(fp, "    %d %d rlineto\n", 0, ah-aw/8);
                fprintf(fp, "    %d %d rlineto\n", -aw, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, -ah);
                fprintf(fp, "    closepath\n");
                fprintf(fp, "    fill\n");
                fprintf(fp, "  grestore\n");

            }

        }
        else {

            fprintf(fp, "  %% clip & hatch\n");
            fprintf(fp, "  gsave\n");
            fprintf(fp, "    %d %d translate\n", x1, y1);
            fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    0 setlinewidth\n");
            fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw-aw/4, 0);
            fprintf(fp, "    %d %d rlineto\n", aw/4, aw/8);
            fprintf(fp, "    %d %d rlineto\n", 0, ah-aw/8);
            fprintf(fp, "    %d %d rlineto\n", -aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, -ah);
            fprintf(fp, "    closepath\n");
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");

#if 0
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", -aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, -ah);
            fprintf(fp, "    closepath\n");
            fprintf(fp, "    stroke\n");

#endif

        }

    }

    fprintf(fp, "grestore %% for inside\n");


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
        fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
        fprintf(fp, "    %d %d rlineto\n", aw-aw/4, 0);
        fprintf(fp, "    %d %d rlineto\n", aw/4, aw/8);
        fprintf(fp, "    %d %d rlineto\n", 0, ah-aw/8);
        fprintf(fp, "    %d %d rlineto\n", -aw, 0);
        fprintf(fp, "    %d %d rlineto\n", 0, -ah);
        fprintf(fp, "    closepath\n");
        fprintf(fp, "    stroke\n");
        fprintf(fp, "    %d %d moveto\n", aw/4, -ah/2);
        fprintf(fp, "    %d %d rlineto\n", aw/8, ah/3);
        fprintf(fp, "    %d %d rlineto\n", aw/8, -(ah/3-aw/8));
        fprintf(fp, "    closepath\n");
        fprintf(fp, "    fill\n");
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of box\n");

out:
    return 0;
}



int
Xepsdraw_scatter(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns)
{
    ob *pf, *pb;
    ob *pe;
    int i;
    int sx, sy;
    int ex, ey;

P;
    pf = (ob*)xu->cob.linkfore;
    pb = (ob*)xu->cob.linkback;
    fprintf(fp, "%% pf %p, pb %p\n", pf, pb);
    Echo("%% pf %p, pb %p\n", pf, pb);

    if(!pf || !pb) {
        goto out;
    }

    sx = pb->cgx + pb->cwd/2;
    sy = pb->cgy;

    fprintf(fp, "gsave\n");
    if(ISCHUNK(pf->type)) {
        for(i=0;i<pf->cch.nch;i++) {
            pe = (ob*)pf->cch.ch[i];
            if(ISATOM(pe->type)) {
            }
            else {
                continue;
            }

            ex = pe->cgx - pe->cwd/2;
            ey = pe->cgy;

            fprintf(fp, "   %d %d moveto %d %d lineto stroke\n",
                sx, sy, ex, ey);
        }
    }
    else {
            ex = pf->cgx - pf->cwd/2;
            ey = pf->cgy;
            fprintf(fp, "   %d %d moveto %d %d lineto stroke\n",
                sx, sy, ex, ey);
    }
    fprintf(fp, "grestore\n");

out:
    return 0;
}

int
epsdraw_gather_man(FILE *fp, int xdir, int xox, int xoy,
    ob *xu, ob *pf, ob *pb, ns *xns, int dsdir)
{
    ob *pe;
    int i;
    int sx, sy;
    int ex, ey;
    int miny, maxy;
    int mini, maxi;
    int jr;
    int j;
    int k;
    int eyt, eyb;
    int cu, ce, cd, call;
    int eex, eey;
    int yp;
    int g;

    int usi, uei, esi, eei, dsi, dei;

    int h1, h2, v;
    int t1x, t1y, t2x, t2y;

P;
    if(!pf || !pb) {
        goto out;
    }

    jr = xu->cob.outlinethick*2;

    ex = xox + pf->cx - pf->cwd/2 * (dsdir);
    ey = xoy + pf->cy;

    eyt = xoy + pf->cy + pf->cht/2;
    eyb = xoy + pf->cy - pf->cht/2;

    miny = INT_MAX;
    maxy = -(INT_MAX-1);

    mini = INT_MAX;
    maxi = -(INT_MAX-1);

        fprintf(fp, "    gsave\n");

    if(ISCHUNK(pb->type)) {
        changethick(fp, xu->cob.outlinethick);
        changecolor(fp, xu->cob.outlinecolor);
        cu = ce = cd = call = 0;
        for(i=0;i<pb->cch.nch;i++) {
            pe = (ob*)pb->cch.ch[i];
            if(ISATOM(pe->type)) {
            }
            else {
                continue;
            }

            call++;
            if(i>maxi) maxi = i;
            if(i<mini) mini = i;
        }
        yp = xu->ht / (call+1);
        Echo("call %d yp %d\n", call, yp);

        usi = uei = esi = eei = dsi = dei = -1;

        j = 0;
        for(i=0;i<pb->cch.nch;i++) {
            pe = (ob*)pb->cch.ch[i];
            if(ISATOM(pe->type)) {
            }
            else {
                continue;
            }

            sx = xox + pb->cx + pb->ox + pe->cx + pe->cwd/2 *dsdir;
            sy = xoy + pb->cy + pb->oy + pe->cy;

            if(sy>maxy) maxy = sy;
            if(sy<miny) miny = sy;

            eex = ex;
            eey = ey+xu->ht/2-(j+1)*yp;

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

        Echo("  cu %d ce %d cd %d\n", cu, ce, cd);
        Echo("  usi %d uei %d\n", usi, uei);
        Echo("  esi %d eei %d\n", esi, eei);
        Echo("  dsi %d dei %d\n", dsi, dei);

        j = 0;
        for(i=0;i<pb->cch.nch;i++) {
            g = 0;
            pe = (ob*)pb->cch.ch[i];
            if(ISATOM(pe->type)) {
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
                g = 2;
            }
            if(j>=dsi && j<=dei) {
                k = j - dsi;
                g = 3;
            }
            else {
            }
            Echo("i %d j %d: g %d k %d\n", i, j, g, k);

            eex = ex;
            eey = ey+xu->ht/2-(j+1)*yp;


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
                h2 = (ex-sx) - h1;
                v  = (eey-sy);

                Echo("  h1 %7d v %7d h2 %7d\n", h1, v, h2);

                fprintf(fp, "   %d %d moveto %d 0 rlineto"
                            " 0 %d rlineto %d 0 rlineto stroke\n",
                            sx, sy, h1, v, h2);

            }

            j++;
        }

    }
    else {
        sx = xox + pb->cx + pb->cwd/2 *dsdir;
        sy = xoy + pb->cy;

        if(sy==ey) {
        }
            fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                sx, sy, ex, ey);
    }
    fprintf(fp, "     grestore\n");

out:
    return 0;
}


int
epsdraw_gather_square(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns, int drad, int dsdir)
{
    ob *pe;
    int i;
    int sx, sy;
    int ex, ey;
    int miny, maxy;
    int mini, maxi;
    int jr;
    int j;

P;
    if(!pf || !pb) {
        goto out;
    }

    jr = -1;
    if(drad>0) {
#if 0
        jr = objunit/20;
#endif
        jr = xu->cob.outlinethick*2;
    }

    ex = pf->cx - pf->cwd/2 *(dsdir);
    ey = pf->cy;

    miny = INT_MAX;
    maxy = -(INT_MAX-1);

    mini = INT_MAX;
    maxi = -(INT_MAX-1);

        fprintf(fp, "    gsave\n");

    if(ISCHUNK(pb->type)) {
        changethick(fp, xu->cob.outlinethick);
        changecolor(fp, xu->cob.outlinecolor);
        for(i=0;i<pb->cch.nch;i++) {
            pe = (ob*)pb->cch.ch[i];
            if(ISATOM(pe->type)) {
            }
            else {
                continue;
            }

            sx = pe->cgx + pe->cwd/2 *dsdir;
            sy = pe->cgy;

            if(sy>maxy) maxy = sy;
            if(sy<miny) miny = sy;

            if(i>maxi) maxi = i;
            if(i<mini) mini = i;
        }


        fprintf(fp, "    %d %d moveto %d %d rlineto stroke\n",
                        xox+xu->cx, miny, 0, maxy-miny);
        fprintf(fp, "    %d %d moveto %d %d lineto stroke\n",
                        xox+ex, xoy+ey, xox+xu->cx, xoy+ey);
        if(jr) {
            fprintf(fp, "   newpath %d %d %d 0 360 arc fill\n",
                xox+xu->cx, xoy+ey, jr);
        }

        for(i=0;i<pb->cch.nch;i++) {
            pe = (ob*)pb->cch.ch[i];
            if(ISATOM(pe->type)) {
            }
            else {
                continue;
            }


            sx = pe->cx + pe->cwd/2*dsdir;
            sy = pe->cy;

            fprintf(fp, "   %d %d moveto %d %d lineto stroke\n",
                xox+pb->cx+pb->ox+sx,
                xoy+pb->cy+pb->oy+sy,
                xox+xu->cx, 
                xoy+pb->cy+pb->oy+sy);

            if(jr>0) {
                if(i==mini ||i==maxi) {
#if 0
                    fprintf(fp, "   newpath %d %d %d 0 360 arc stroke\n",
                        xox+xu->cx, xoy+pb->cy+pb->oy+sy, jr);
#endif
                }
                else {
                    fprintf(fp, "   newpath %d %d %d 0 360 arc fill\n",
                        xox+xu->cx, xoy+pb->cy+pb->oy+sy, jr);
                }
            }
        }
    }
    else {
        sx = pb->cx + pb->cwd/2*dsdir;
        sy = pb->cy;

        if(sy==ey) {
        }
            fprintf(fp, "  %d %d moveto %d %d lineto stroke\n",
                xox+sx, xoy+sy, xox+ex, xoy+ey);
    }
    fprintf(fp, "     grestore\n");

out:
    return 0;
}

int
epsdraw_gather_direct(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns)

{
    ob *pe;
    int i;
    int sx, sy;
    int ex, ey;

P;
    if(!pf || !pb) {
        goto out;
    }

    ex = pf->cx - pf->cwd/2;
    ey = pf->cy;

    fprintf(fp, "    gsave\n");

    if(ISCHUNK(pb->type)) {
        for(i=0;i<pb->cch.nch;i++) {
            pe = (ob*)pb->cch.ch[i];
            if(ISATOM(pe->type) || ISCHUNK(pe->type)) {
            }
            else {
                continue;
            }

            sx = pe->cx + pe->cwd/2;
            sy = pe->cy;

#if 0
            fprintf(fp, "   %d %d moveto %d %d lineto stroke\n",
                xox+pb->cx+pb->ox+sx, xoy+pb->cy+pb->oy+sy, xox+ex, xoy+ey);
#endif
#if 1
            epsdraw_seglinearrow(fp, xdir, 0, 0,
                xox+pb->cx+pb->ox+sx, xoy+pb->cy+pb->oy+sy, xox+ex, xoy+ey, xu, xns);
#endif
        }
    }
    else {
            sx = pb->cx + pb->cwd/2;
            sy = pb->cy;
            fprintf(fp, "   %d %d moveto %d %d lineto stroke\n",
                xox+sx, xoy+sy, xox+ex, xoy+ey);
#if 0
            epsdraw_seglinearrow(fp, xdir, xox, xoy, sx, sy, ex, ey, pb, xns);
#endif
            
    }
    fprintf(fp, "     grestore\n");

out:
    return 0;
}

int
epsdraw_gather(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns)
{
    ob *pf, *pb;
    int ik;
#if 0
    int i;
    ob *pe;
    int sx, sy;
    int ex, ey;
#endif

P;
    pf = (ob*)xu->cob.linkfore;
    pb = (ob*)xu->cob.linkback;
    fprintf(fp, "%% pf %p, pb %p\n", pf, pb);
    Echo("%% pf %p, pb %p\n", pf, pb);

    if(!pf || !pb) {
        goto out;
    }

    switch(xu->cob.linkstyle) {
    case LS_SQUARE:
        ik = epsdraw_gather_square(fp, xdir, xox, xoy, xu, pf, pb, xns, 0, 1);
        break;
    case LS_SQUAREDOT:
        ik = epsdraw_gather_square(fp, xdir, xox, xoy, xu, pf, pb, xns, 1, 1);
        break;
    case LS_MAN:
        ik = epsdraw_gather_man(fp, xdir, xox, xoy, xu, pf, pb, xns, 1);
        break;
    case LS_ARC:
        Error("LS_ARC is not implemented yet.\n");
    case LS_DIRECT:
    default:
        ik = epsdraw_gather_direct(fp, xdir, xox, xoy, xu, pf, pb, xns);
        break;
    }

out:
    return 0;
}

int
epsdraw_scatter_direct(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns)

{
    ob *pe;
    int i;
    int sx, sy;
    int ex, ey;

P;
    if(!pf || !pb) {
        goto out;
    }

    sx = pb->cx + pb->cwd/2;
    sy = pb->cy;

    fprintf(fp, "    gsave\n");

    if(ISCHUNK(pf->type)) {
        for(i=0;i<pf->cch.nch;i++) {
            pe = (ob*)pf->cch.ch[i];
            if(ISATOM(pe->type) || ISCHUNK(pe->type)) {
            }
            else {
                continue;
            }

            ex = pe->cx - pe->cwd/2;
            ey = pe->cy;

#if 0
            fprintf(fp, "   %d %d moveto %d %d lineto stroke\n",
                xox+pb->cx+pb->ox+sx, xoy+pb->cy+pb->oy+sy, xox+ex, xoy+ey);
#endif
#if 1
            epsdraw_seglinearrow(fp, xdir, 0, 0,
/*
                xox+pf->cx+pf->ox+sx, xoy+pf->cy+pf->oy+sy, xox+ex, xoy+ey, xu, xns);
*/
                xox+sx, xoy+sy, xox+pf->cx+pf->ox+ex, xoy+pf->cy+pf->oy+ey,
                xu, xns);
#endif
        }
    }
    else {
            sx = pf->cx + pb->cwd/2;
            sy = pf->cy;
            fprintf(fp, "   %d %d moveto %d %d lineto stroke\n",
                xox+sx, xoy+sy, xox+ex, xoy+ey);
#if 0
            epsdraw_seglinearrow(fp, xdir, xox, xoy, sx, sy, ex, ey, pb, xns);
#endif
            
    }
    fprintf(fp, "     grestore\n");

out:
    return 0;
}

int
epsdraw_scatter(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns)
{
    ob *pf, *pb;
    int ik;
#if 0
    int i;
    ob *pe;
    int sx, sy;
    int ex, ey;
#endif

P;
    pf = (ob*)xu->cob.linkfore;
    pb = (ob*)xu->cob.linkback;
    fprintf(fp, "%% pf %p, pb %p\n", pf, pb);
    Echo("%% pf %p, pb %p\n", pf, pb);

    if(!pf || !pb) {
        goto out;
    }

    switch(xu->cob.linkstyle) {
    case LS_SQUAREDOT:
        ik = epsdraw_gather_square(fp, xdir, xox, xoy, xu, pb, pf, xns, 1, -1);
        break;
    case LS_SQUARE:
        ik = epsdraw_gather_square(fp, xdir, xox, xoy, xu, pb, pf, xns, 0, -1);
        break;
    case LS_MAN:
/*
        ik = epsdraw_gather_man(fp, xdir, xox, xoy, xu, pf, pb, xns, 1);
*/
        ik = epsdraw_gather_man(fp, xdir, xox, xoy, xu, pb, pf, xns, -1);
        break;
    case LS_ARC:
        Error("LS_ARC is not implemented yet.\n");
    case LS_DIRECT:
    default:
        ik = epsdraw_scatter_direct(fp, xdir, xox, xoy, xu, pf, pb, xns);
        break;
    }

out:
    return 0;
}


int
epsdraw_Xparen(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
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
    fprintf(fp, "gsave %% for box\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);


#if 0
    fprintf(fp, "gsave %% for inside\n");

    /***
     *** CLIP and HATCH
     ***/

    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);

        if(xu->cob.fillhatch==HT_NONE) {
            /* nothing */
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            if(xu->cob.fillcolor>=0) {
                fprintf(fp, "  %% solid fill\n");
                fprintf(fp, "  gsave\n");
                fprintf(fp, "    %d %d translate\n", x1, y1);
                fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
                fprintf(fp, "    newpath\n");
                fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
                fprintf(fp, "    %d %d rlineto\n", aw, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, ah);
                fprintf(fp, "    %d %d rlineto\n", -aw, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, -ah);
                fprintf(fp, "    closepath\n");
                fprintf(fp, "    fill\n");
                fprintf(fp, "  grestore\n");

            }

        }
        else {

            fprintf(fp, "  %% clip & hatch\n");
            fprintf(fp, "  gsave\n");
            fprintf(fp, "    %d %d translate\n", x1, y1);
            fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    0 setlinewidth\n");
            fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", -aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, -ah);
            fprintf(fp, "    closepath\n");
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");
        }
    }

    fprintf(fp, "grestore %% for inside\n");

#endif

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
#if 0
        cx1 = ((double)ah*ah)/(8.0*aw);
        r1  = aw/2+((double)ah*ah)/(8.0*aw);
        cx2 = ((double)ah*ah)/(8.0*k*aw);
        r2  = (k*aw-aw/2)+((double)ah*ah)/(8.0*k*aw);

        th1 = atan2(ah/2, (cx1-aw/2))/rf;
        th2 = atan2(ah/2, (cx2-aw/2))/rf;

fprintf(fp, "%% R cx1 %f r1 %f th1 %f cx2 %f r2 %f th2 %f\n",
        cx1, r1, th1, cx2, r2, th2);

        fprintf(fp, "    %.3f 0 %.3f %f %f arc\n",
            -cx1, r1, -th1, th1);
        fprintf(fp, "    %.3f 0 %.3f %f %f arcn\n",
            -cx2, r2, th2, -th2);
#endif
    }
    {
        cx1 = ((double)ah*ah)/(8.0*aw);
        r1  = aw/2+((double)ah*ah)/(8.0*aw);
        cx2 = ((double)ah*ah)/(8.0*k*aw);
        r2  = (k*aw-aw/2)+((double)ah*ah)/(8.0*k*aw);

        th1 = atan2(ah/2, -(cx1-aw/2))/rf;
        th2 = atan2(ah/2, -(cx2-aw/2))/rf;
fprintf(fp, "%% L cx1 %f r1 %f th1 %f cx2 %f r2 %f th2 %f\n",
        cx1, r1, th1, cx2, r2, th2);

        fprintf(fp, "    %.3f 0 %.3f %f %f arc\n",
            cx1, r1, th1, -th1);
        fprintf(fp, "    %.3f 0 %.3f %f %f arcn\n",
            cx2, r2, -th2, th2);
        fprintf(fp, "    stroke\n");

        fprintf(fp, "    %.3f 0 %.3f %f %f arc\n",
            cx1, r1, th1, -th1);
        fprintf(fp, "    %.3f 0 %.3f %f %f arcn\n",
            cx2, r2, -th2, th2);
    }
#if 0
        fprintf(fp, "    stroke\n");
#endif
        fprintf(fp, "    closepath\n");
        fprintf(fp, "    fill\n");
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of box\n");

out:
    return 0;
}

int
epsdraw_Xbrace(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
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
    fprintf(fp, "gsave %% for box\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);


#if 0
    fprintf(fp, "gsave %% for inside\n");

    /***
     *** CLIP and HATCH
     ***/

    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);

        if(xu->cob.fillhatch==HT_NONE) {
            /* nothing */
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            if(xu->cob.fillcolor>=0) {
                fprintf(fp, "  %% solid fill\n");
                fprintf(fp, "  gsave\n");
                fprintf(fp, "    %d %d translate\n", bx, by);
                fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
                fprintf(fp, "    newpath\n");
                fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
                fprintf(fp, "    %d %d rlineto\n", aw, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, ah);
                fprintf(fp, "    %d %d rlineto\n", -aw, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, -ah);
                fprintf(fp, "    closepath\n");
                fprintf(fp, "    fill\n");
                fprintf(fp, "  grestore\n");

            }

        }
        else {

            fprintf(fp, "  %% clip & hatch\n");
            fprintf(fp, "  gsave\n");
            fprintf(fp, "    %d %d translate\n", bx, by);
            fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    0 setlinewidth\n");
            fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", -aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, -ah);
            fprintf(fp, "    closepath\n");
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");
        }
    }

    fprintf(fp, "grestore %% for inside\n");

#endif


    fprintf(fp, "%% frame\n");
    fprintf(fp, "%%     outline color %d thick %d\n",
        xu->cob.outlinecolor, xu->cob.outlinethick);

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
    {
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

    }
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

#if 0
    epsdraw_sstr(fp, xu->gx, xu->gy, xu->wd, xu->ht, xu->cob.ssar);
#endif

    fprintf(fp, "grestore %% end of box\n");

out:
    return 0;
}



int
epsdraw_Xbracket(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
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
    fprintf(fp, "gsave %% for box\n");

    fprintf(fp, "%% inside\n");
    fprintf(fp, "%%     fill color %d hatch %d; hatch thick %d pitch %d\n",
        xu->cob.outlinecolor, xu->cob.fillhatch,
        xu->cob.hatchthick, xu->cob.hatchpitch);


#if 0
    fprintf(fp, "gsave %% for inside\n");

    /***
     *** CLIP and HATCH
     ***/

    if(xu->cob.fillcolor>=0) {
        changecolor(fp, xu->cob.fillcolor);

        if(xu->cob.fillhatch==HT_NONE) {
            /* nothing */
        }
        else
        if(xu->cob.fillhatch==HT_SOLID) {
            if(xu->cob.fillcolor>=0) {
                fprintf(fp, "  %% solid fill\n");
                fprintf(fp, "  gsave\n");
                fprintf(fp, "    %d %d translate\n", x1, y1);
                fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
                fprintf(fp, "    newpath\n");
                fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
                fprintf(fp, "    %d %d rlineto\n", aw, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, ah);
                fprintf(fp, "    %d %d rlineto\n", -aw, 0);
                fprintf(fp, "    %d %d rlineto\n", 0, -ah);
                fprintf(fp, "    closepath\n");
                fprintf(fp, "    fill\n");
                fprintf(fp, "  grestore\n");

            }

        }
        else {

            fprintf(fp, "  %% clip & hatch\n");
            fprintf(fp, "  gsave\n");
            fprintf(fp, "    %d %d translate\n", x1, y1);
            fprintf(fp, "    0 0 moveto %d rotate\n", xu->cob.rotateval);
            fprintf(fp, "    newpath\n");
            fprintf(fp, "    0 setlinewidth\n");
            fprintf(fp, "    %d %d moveto\n", -aw/2, -ah/2);
            fprintf(fp, "    %d %d rlineto\n", aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, ah);
            fprintf(fp, "    %d %d rlineto\n", -aw, 0);
            fprintf(fp, "    %d %d rlineto\n", 0, -ah);
            fprintf(fp, "    closepath\n");
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, aw, ah,
                xu->cob.fillcolor, xu->cob.fillhatch, xu->cob.hatchpitch);

            fprintf(fp, "  grestore\n");
        }
    }

    fprintf(fp, "grestore %% for inside\n");

#endif


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

    fprintf(fp, "grestore %% end of box\n");

out:
    return 0;
}


int
epsdraw_sep(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{

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
XbumpBB(int gx, int gy, int gw, int gh, int cx, int cy, int cr, int *rx, int *ry)
{
    int qx, qy;
    int ik;
    int rv;

    rv = -1;
    cr = dirnormalize(cr);

    ik = bumpH(gx-gw/2, gy-gh/2, gx+gw/2, gy-gh/2, cx, cy, cr, &qx, &qy);
    if(ik) {
Echo("%s: found B\n", __func__);
        *rx = qx;
        *ry = qy;
        rv = 1;
    }

    ik = bumpV(gx+gw/2, gy+gh/2, gx+gw/2, gy-gh/2, cx, cy, cr, &qx, &qy);
    if(ik) {
Echo("%s: found R\n", __func__);
        *rx = qx;
        *ry = qy;
        rv = 2;
    }

    ik = bumpH(gx-gw/2, gy+gh/2, gx+gw/2, gy+gh/2, cx, cy, cr, &qx, &qy);
    if(ik) {
Echo("%s: found T\n", __func__);
        *rx = qx;
        *ry = qy;
        rv = 3;
    }

    ik = bumpV(gx-gw/2, gy+gh/2, gx-gw/2, gy-gh/2, cx, cy, cr, &qx, &qy);
    if(ik) {
Echo("%s: found L\n", __func__);
        *rx = qx;
        *ry = qy;
        rv = 4;
    }

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
epsdraw_portboard(FILE *fp, int xdir, ob *u)
{
    int ik;
    int dx, dy;
    int qx, qy;
    int lax, lay;
    int fht;

    if(u->cob.portstr || u->cob.boardstr) {
        fprintf(fp, "      gsave\n");

        lax = (objunit/8)*cos((xdir+90)*rf);
        lay = (objunit/8)*sin((xdir+90)*rf);

        ik = bumpBB(u->gx, u->gy, u->wd, u->ht,
                u->gx, u->gy, xdir+90, &qx, &qy);

        dx = qx - u->gx;
        dy = qy - u->gy;

        if(ik<=0) {
            goto skip_portboard;
        }

        /* TEMP */
        fht = def_textheight;
        fprintf(fp, "      /%s findfont %d scalefont setfont %% port/board\n",
            def_fontname, fht);

        if(u->cob.portstr) {
            fprintf(fp, "      %d %d %d (%s) rrshow\n",
                u->gx+dx+lax, u->gy+dy+lay, xdir-90, u->cob.portstr);
        }
        if(u->cob.boardstr) {
            fprintf(fp, "      %d %d %d (%s) lrshow\n",
                u->gx-dx-lax, u->gy-dy-lay, xdir-90, u->cob.boardstr);
        }
        fprintf(fp, "      grestore\n");
    }
skip_portboard:

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
                    fprintf(fp, "      0 0 (%s) rshow\n", mcont);
                }
                if(bj==SJ_CENTER) {
                    fprintf(fp, "      0 0 (%s) cshow\n", mcont);
                }
                if(bj==SJ_RIGHT) {
                    fprintf(fp, "      0 0 (%s) lshow\n", mcont);
                }
            }
            else {

                fprintf(fp, "      %d %d translate\n", 0, fdc);
                if(bj==SJ_LEFT||bj==SJ_CENTER) {
                    fprintf(fp, "      0 0 (%s) lshow\n", mcont);
                }
                else {
                    fprintf(fp, "      0 0 (%s) rshow\n", mcont);
                }
            }
        }
        else {
            fprintf(fp, "      %d %d translate\n", 0, fdc);
            if(bj==SJ_LEFT) {
                fprintf(fp, "      0 0 (%s) lshow\n", mcont);
            }
            if(bj==SJ_CENTER) {
                fprintf(fp, "      0 0 (%s) cshow\n", mcont);
            }
            if(bj==SJ_RIGHT) {
                fprintf(fp, "      0 0 (%s) rshow\n", mcont);
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

#if 0
        epsdraw_bbox(fp, u);
#endif
#if 0
    if(bbox_mode) {
        epsdraw_bbox_lbrt(fp, ox, oy, u);
    }
#endif
    
    wd = u->crx-u->clx;
    ht = u->cty-u->cby;


    g = eval_dir(u, xdir);
    if(g>0) {
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
        fprintf(fp, "  gsave %% markbb\n");
        changecolor(fp, def_markcolor);
        changethick(fp, def_markbbthick);
        drawCRrect(fp, u->gx, u->gy, u->wd, u->ht, u->cob.rotateval);
        fprintf(fp, "  grestore\n");

#if 0
        epsdraw_bbox(fp, u);

        epsdraw_bbox_lbrt(fp, u->ox, u->oy, u);
        epsdraw_bbox_lbrtR(fp, u->x, u->y, u);
#endif
    }

    changenormal(fp); /* for faill safe */

    if(u->type==CMD_SCATTER) {
        epsdraw_scatter(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_GATHER) {
        epsdraw_gather(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_BCURVE || u->type==CMD_XCURVE) {
        Zepsdraw_bcurvearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_BCURVESELF || u->type==CMD_XCURVESELF) {
        Zepsdraw_bcurveselfarrow(fp, *xdir, ox, oy, u, xns);
    }
#if 0
    if(u->type==CMD_XCURVE) {
        Zepsdraw_xcurvearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_XCURVESELF) {
        Zepsdraw_xcurveselfarrow(fp, *xdir, ox, oy, u, xns);
    }
#endif
    if((u->type==CMD_LINK) || (u->type==CMD_LINE) ||
            (u->type==CMD_ARROW)) {
P;
        Zepsdraw_clinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_CLINE) {
P;
        Zepsdraw_clinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_WLINE) {
        Zepsdraw_wlinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_WARROW) {
        Zepsdraw_wlinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_BARROW) {
        epsdraw_blinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_PLINE) {
        epsdraw_plinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_PING) {
        epsdraw_ping(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_PINGPONG) {
        epsdraw_pingpong(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_SEP) {
        epsdraw_sep(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_LPAREN || u->type==CMD_RPAREN) {
        epsdraw_Xparen(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_LBRACKET || u->type==CMD_RBRACKET) {
        epsdraw_Xbracket(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_LBRACE || u->type==CMD_RBRACE) {
        epsdraw_Xbrace(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_MOVE) {
#if 0
        fprintf(fp, "+   # empty as \"move\"\n");
        fprintf(fp, "+   box at (%d,%d) width %d height %d \"%d\" invis\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, u->oid);
#endif
    }

    if(u->type==CMD_CLOUD) {
        epsdraw_cloud(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_DRUM) {
        epsdraw_drum(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_DOTS) {
        epsdraw_dots(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_RULER) {
        epsdraw_ruler(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_BOX || u->type==CMD_CIRCLE || u->type==CMD_ELLIPSE ||
        u->type==CMD_PAPER || u->type==CMD_CARD || u->type==CMD_DIAMOND ||
        u->type==CMD_HOUSE || u->type==CMD_POLYGON) {
        epsdraw_bodyX(fp, ox, oy, u, xns);
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
        epsdraw_sstrbgX(fp, u->gx, u->gy, u->wd, u->ht,
            u->cob.rotateval + u->cob.textrotate,
            0, 2, u->cob.textcolor, _tbgc, u->cob.ssar);


skip_sstr:
        (void)0;
    }

    ik = epsdraw_note(fp, u);
    ik = epsdraw_portboard(fp, *xdir, u);

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
    fprintf(fp, "%% start oid %d\n", xch->oid);

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

        fprintf(fp, "%% bgshape %p\n", xch->cob.bgshape);
        if(xch->cob.bgshape) {
            fprintf(fp, "%% value shape '%s'\n", xch->cob.bgshape);
        }
        if(xch->cob.bgshape && strcmp(xch->cob.bgshape, "plane")==0) {
PP;
            ik = epsdraw_plane(fp,
                    gox,
                    goy, xch, xns);
        }
        else {
PP;
#if 0
            ik = epsdraw_box(fp, gox, goy, xch, xns);
#endif
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
            fprintf(fp, "%%\n%% --- oid %d %s\n",
                u->oid, rassoc(cmd_ial, u->type));
#endif

#if 0
                epsdraw_bbox(fp, u);
                epsdraw_bbox_lbrt(fp, xch->x, xch->y, u);
                epsdraw_bbox_lbrtR(fp, xch->x, xch->y, u);
#endif
#if 1
            if(bbox_mode) {
                if(VISIBLE(u->type)) {
                    epsdraw_bbox(fp, u);
                }
                else {
                    fprintf(stderr, "; bbox invisible oid %d\n", u->oid);
                }
            }
#endif
    
Echo(" call obj oid %d drawing start %d,%d\n",
            u->oid, gox+xch->x+xch->ox, goy+xch->y+xch->oy);
            ik = epsdrawobj(fp, u, &xch->cch.dir,
                    gox+xch->x+xch->ox,
                    goy+xch->y+xch->oy, xns);
        }
    }

    ik = epsdraw_note(fp, xch);
    ik = epsdraw_portboard(fp, xch->cch.dir, u);

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

