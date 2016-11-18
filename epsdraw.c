
#include "tx.h"
#include "qbb.h"

/* outside of scaling */
int epsoutmargin        = 16;

/* inside of scaling */
int epsdraftfontsize    = 10;
int epsdraftgap         = 5;

int debug_clip = 0;

#define PP  fprintf(fp, "%% PASS %s:%d\n", __func__, __LINE__); fflush(fp);

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
    fprintf(fp, "    1 0.5 0 setrgbcolor\n");
#if 0
    fprintf(fp, "    /Courier findfont %d scalefont setfont\n",
        epsdraftfontsize);
#endif
    return 0;
}

static int
changetext2(FILE *fp)
{
    fprintf(fp, "    1 1 0 setrgbcolor\n");
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
#if 0
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
    fprintf(fp, "    grestore\n");
#endif
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
    fprintf(fp, "%% bbox guide with CWH\n");
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

    fprintf(fp, "  %% bbox guide with GLBRT (%d %d %d %d)\n",
            xu->glx, xu->gby, xu->grx, xu->gty);
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
epsdraw_bbox_lbrt(FILE *fp, int xox, int xoy, ob *xu)
{
    char msg[BUFSIZ];

    fprintf(fp, "%% bbox guide with LBRT %d,%d (%d %d %d %d)\n",
            xox, xoy, xu->lx, xu->by, xu->rx, xu->ty);
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

#define epsdraw_bbox    epsdraw_bbox_glbrt

int
epsdraw_arrowhead(FILE *fp, int atype, int xdir, int lc, int x, int y)
{
    int  r;
    int  dx, dy;

    fprintf(fp, "%% arrowhead %d\n", atype);

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
    case AH_NORMAL:
    defalt:
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
        ll = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
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

#if 0
            if(j%2==0) {
                fprintf(fp, "%.2f %.2f moveto %.2f %.2f lineto stroke\n",
                    ux, uy, nx, ny);
            }
#endif

            if(ltype==LT_DOTTED||ltype==LT_DASHED) {
                if(j%2==0) {
                    fprintf(fp, "%.2f %.2f moveto %.2f %.2f lineto stroke\n",
                        ux, uy, nx, ny);
                }
            }
            if(ltype==LT_CHAINED) {
                if(j%7==4||j%7==6) {
                }
                else {
                    fprintf(fp, "%.2f %.2f moveto %.2f %.2f lineto stroke\n",
                        ux, uy, nx, ny);
                }
            }
            if(ltype==LT_DOUBLECHAINED) {
                if(j%9==4||j%9==6||j%9==8) {
                }
                else {
                    fprintf(fp, "%.2f %.2f moveto %.2f %.2f lineto stroke\n",
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
        ll = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
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

            if(ltype==LT_DOTTED||ltype==LT_DASHED) {
                if(j%2==0) {
                    fprintf(fp, "%.2f %.2f moveto %.2f %.2f lineto stroke\n",
                        ux, uy, nx, ny);
                }
            }
            if(ltype==LT_CHAINED) {
                if(j%7==4||j%7==6) {
                }
                else {
                    fprintf(fp, "%.2f %.2f moveto %.2f %.2f lineto stroke\n",
                        ux, uy, nx, ny);
                }
            }
            if(ltype==LT_DOUBLECHAINED) {
                if(j%9==4||j%9==6||j%9==8) {
                }
                else {
                    fprintf(fp, "%.2f %.2f moveto %.2f %.2f lineto stroke\n",
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
        ll = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
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
        ll = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
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
        ll = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
        cy = ((double)y2-y1)/sepw;
        px = (x2-x1)/cy;
        py = (y2-y1)/cy;


        r  = sqrt(px*px+py*py);

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
        ll = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
        cy = ((double)x2-x1)/sepw;
        px = (x2-x1)/cy;
        py = (y2-y1)/cy;


        r  = sqrt(px*px+py*py);

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

    ll = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
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
    int r;

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
    case LT_ARROWCENTERED:
    case LT_SOLID:
    default:
        changethick(fp, lt);
        changecolor(fp, lc);
        fprintf(fp, "  %% solid\n");
        fprintf(fp, "  %d %d moveto\n", x1, y1);
        fprintf(fp, "  %d %d lineto\n", x2, y2);
        fprintf(fp, "  stroke\n");
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
            r = epsdraw_arrowhead(fp, AH_NORMAL, xdir, lc, x3, y3);
        }
        break;
    }
    return r;
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

    ll = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
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
    return r;
}


int
epsdraw_Xseglinearrow(FILE *fp,
    int xox, int xoy, 
    int x1, int y1, int x2, int y2,
    int xltype, int xlt, int xlc, int xahpart, int xahfore, int xahback)
{
    ob* pf;
    ob* pt;
    int r;
    int dx, dy;
    int xdir;

    int x1i, y1i;
    int x2i, y2i;

    /*
     *   x1,y1 x1i,y1i  x2i,y2i  x2,y2
     *     <---+--------------+---->
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

    /*** MAIN LINE */
    epsdraw_segline(fp, xltype, xlt, xlc, x1i, y1i, x2i, y2i);


    /*** FORE ARROW HEAD */
    if(x2i!=x2 || y2i!=y2) {
        fprintf(fp, "%% fore arrow head\n");
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            x2i, y2i, x2, y2);
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
    int xltype, int xlt, int xlc, int xahpart, int xahfore, int xahback)
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
        xu->cob.arrowforeheadtype, xu->cob.arrowbackheadtype);

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
        xu->cob.arrowforeheadtype, xu->cob.arrowbackheadtype);

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

    l = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
    a = atan2(y2-y1, x2-x1)/rf;
    b = (int)(def_barrowgap/2*tan(a*rf));

    bx = (int)(b*cos(a*rf));
    by = (int)(b*sin(a*rf));
    dx = (int)(def_barrowgap/2*cos((a+90)*rf));
    dy = (int)(def_barrowgap/2*sin((a+90)*rf));

    fprintf(stderr, "l %d b %d bx %d by %d\n", l, b, bx, by);
    fprintf(stderr, "dx %d dy %d\n", dx, dy);

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
        AR_FORE, AH_NORMAL, AH_NONE);
    epsdraw_Xseglinearrow(fp, 0, 0, e2x, e2y, s2x, s2y,
        LT_SOLID, xu->cob.outlinethick, xu->cob.outlinecolor,
        AR_FORE, AH_NORMAL, AH_NONE);

    fprintf(fp, "  grestore %% for blinearrow\n");

out:
    return ik;
}



int
epsdraw_Xlinearrow(FILE *fp,
    int ydir, int xox, int xoy, 
    int x1, int y1, int x2, int y2,
    ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
#if 0
    int x1, y1;
    int x2, y2;
#endif
    int r;
    int dx, dy;
    int xdir;

    int x1i, y1i;
    int x2i, y2i;

    /*
     *   x1,y1 x1i,y1i  x2i,y2i  x2,y2
     *     <---+--------------+---->
     */

#if 0
    x1 = xox+xu->csx;
    y1 = xoy+xu->csy;
    x2 = xox+xu->cex;
    y2 = xoy+xu->cey;
#endif

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

apply:

    if(draft_mode) {
        changedraft(fp);
        fprintf(fp, "%d %d moveto\n", x1, y1);
        fprintf(fp, "%d %d lineto\n", x2, y2);
        fprintf(fp, "stroke\n");


        char tmp[BUFSIZ];

        sprintf(tmp, "oid %d", xu->oid);
        fprintf(fp, "gsave\n");
        changedraft(fp);
        fprintf(fp, "%d %d moveto (%s) show\n",
            (x1+x2)/2*12/10, (y1+y2)/2*12/10, tmp);
        fprintf(fp, "grestore\n");
    }

    fprintf(fp, "%% line-type          %d\n", xu->cob.outlinetype);
    fprintf(fp, "%% arrowhead-part     %d\n", xu->cob.arrowheadpart);
    fprintf(fp, "%% arrowforehead-type %d\n", xu->cob.arrowforeheadtype);
    fprintf(fp, "%% arrowbackhead-type %d\n", xu->cob.arrowbackheadtype);
    fflush(fp);
    xdir = (int)(atan2((y2-y1),(x2-x1))/rf);

    if(xu->cob.arrowheadpart & AR_BACK) {
        if( (xu->cob.arrowbackheadtype==AH_REVNORMAL)||
            (xu->cob.arrowbackheadtype==AH_REVWIRE)) {
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

    if((xu->cob.arrowheadpart & AR_FORE)) {
        if((xu->cob.arrowforeheadtype==AH_REVNORMAL)||
            (xu->cob.arrowforeheadtype==AH_REVWIRE)) {
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

    if(x1i!=x1 || y1i!=y1) {
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            x1, y1, x1i, y1i);
    }
    fprintf(fp, "%% linetype %d\n", xu->cob.outlinetype);
    epsdraw_segline(fp, xu->cob.outlinetype, 
            xu->cob.outlinethick, xu->cob.outlinecolor,
            x1i, y1i, x2i, y2i);
    if(x2i!=x2 || y2i!=y2) {
        fprintf(fp, "newpath\n");
        fprintf(fp, "%d %d moveto %d %d lineto stroke\n",
            x2i, y2i, x2, y2);
    }

    /*****
     ***** ARROW HEADs
     *****/

    if(xu->cob.arrowheadpart & AR_FORE) {
        xdir = (int)(atan2((y2-y1),(x2-x1))/rf);
        epsdraw_arrowhead(fp, xu->cob.arrowforeheadtype,
            xu->cob.outlinecolor,
            xdir, x2, y2);
    }

    if(xu->cob.arrowheadpart & AR_BACK) {
        xdir = (int)(atan2((y1-y2),(x1-x2))/rf);
        epsdraw_arrowhead(fp, xu->cob.arrowbackheadtype, 
            xu->cob.outlinecolor,
            xdir, x1, y1);
    }

out:
    return 0;
}

#define ADIR_ARC    (1)
#define ADIR_ARCN   (2)

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
    fflush(stdout);

    aa = (double*)alloca(sizeof(double)*(al+3));
    if(!aa) {
        fprintf(stdout, "ERROR no-memory\n");
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
                fprintf(stdout, "ERROR overrun %d/%d\n", i, al);
            }
            fprintf(fp, "%% a %f arc  set i %d/%d\n", a, i, al);
            aa[i++] = a;
        }
    }
    else {
        fprintf(fp, "%% a arcn %f..%f by %f\n", ang1*rf, ang2*rf, pa);
        for(a=ang1*rf; a>=ang2*rf; a-=pa) {
            if(i>al-1) {
                fprintf(stdout, "ERROR overrun %d/%d\n", i, al);
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
        fflush(stdout);
#endif

        if(i>al-1) {
            fprintf(stdout, "ERROR overrun %d/%d\n", i, al);
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
    fflush(stdout);

    aa = (double*)alloca(sizeof(double)*(al+3));
    if(!aa) {
        fprintf(stdout, "ERROR no-memory\n");
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
                fprintf(stdout, "ERROR overrun %d/%d\n", i, al);
            }
            fprintf(fp, "%% a %f arc  set i %d/%d\n", a, i, al);
            aa[i++] = a;
        }
    }
    else {
        fprintf(fp, "%% a arcn %f..%f by %f\n", ang1*rf, ang2*rf, pa);
        for(a=ang1*rf; a>=ang2*rf; a-=pa) {
            if(i>al-1) {
                fprintf(stdout, "ERROR overrun %d/%d\n", i, al);
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
        fflush(stdout);
#endif

        if(i>al-1) {
            fprintf(stdout, "ERROR overrun %d/%d\n", i, al);
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

    d = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
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
    fflush(stdout);

    aa = (double*)alloca(sizeof(double)*(al+3));
    if(!aa) {
        fprintf(stdout, "ERROR no-memory\n");
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
                fprintf(stdout, "ERROR overrun %d/%d\n", i, al);
            }
            fprintf(fp, "%% a %f arc  set i %d/%d\n", a, i, al);
            aa[i++] = a;
        }
    }
    else {
        fprintf(fp, "%% a arcn %f..%f by %f\n", ang1*rf, ang2*rf, pa);
        for(a=ang1*rf; a>=ang2*rf; a-=pa) {
            if(i>al-1) {
                fprintf(stdout, "ERROR overrun %d/%d\n", i, al);
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
        fflush(stdout);
#endif

        if(i>al-1) {
            fprintf(stdout, "ERROR overrun %d/%d\n", i, al);
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
                fitarc(fp, lx, ly, nx, ny, 0);
                lx = nx;
                ly = ny;
            }
            else 
            if(i%4==3) {
                fitarc(fp, lx, ly, nx, ny, 1);
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


static int __z = 0;

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

    r0 = (int)sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));

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

    r0 = (int)sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));

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

fprintf(fp, "%% x1,y1 %d,%d x2,y2 %d,%d\n", x1, y1, x2, y2);
fprintf(fp, "%% xdir %.3f dx,dy %d,%d\n", xdir, dx, dy);

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

fprintf(fp, "%% x1,y1 %d,%d x2,y2 %d,%d\n", x1, y1, x2, y2);
fprintf(fp, "%% xdir %.3f dx,dy %d,%d\n", xdir, dx, dy);

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
_line_deco(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int i;
    int x0, y0;
    int x1, y1, x2, y2;
    seg *s;
    int cdir;

    int ap, fh, bh;
    int arcx, arcy;
    int qbx, qby;
    int qex, qey;
    int qcx, qcy;


#if 1
fprintf(stdout, "%s: ydir %d xox %d xoy %d \n",
    __func__, ydir, xox, xoy);
#endif
    Echo("%s: enter\n", __func__);

    cdir = ydir;

    if(bbox_mode) {
#if 0
        epsdraw_bbox(fp, xu);
#endif
        printf("REMARK BEGIN oid %d\n", xu->oid);
#if 0
        printf("xox,xoy %d,%d cx,cy %d,%d csx,csy %d,%d cox,coy %d,%d\n",
            xox, xoy, xu->cx, xu->cy, xu->csx, xu->csy, xu->cox, xu->coy);
#endif
        printf("  xox,xoy %d,%d ox,oy %d,%d\n",
            xox, xoy, xu->cox, xu->coy);
        printf("  x,y %6d,%-6d sx,sy %6d,%-6d lbrt %6d,%6d,%6d,%6d\n",
            xu->cx, xu->cy, xu->csx, xu->csy,
            xu->clx, xu->cby, xu->crx, xu->cty);
        printf("g x,y %6d,%-6d sx,sy %6d,%-6d lbrt %6d,%6d,%6d,%6d\n",
            xu->cgx, xu->cgy, xu->cgsx, xu->cgsy, 
            xu->cglx, xu->cgby, xu->cgrx, xu->cgty);
        printf("LBRT  %6d,%-6d (%6d %6d %6d %6d)\n",
            xox, xoy, xu->lx, xu->by, xu->rx, xu->ty);
        printf("GLBRT               (%6d %6d %6d %6d)\n",
            xu->glx, xu->gby, xu->grx, xu->gty);

        epsdraw_bbox_glbrt(fp, xu);
#if 0
        epsdraw_bbox_lbrt(fp, xox, xoy, xu);
#endif

        SLW_12(fp);
        epsdraw_bbox_lbrt(fp, xox+xu->csx, xoy+xu->csy, xu);
        SLW_21(fp);

#if 0
        if(xu->type==CMD_CLINE) {
            epsdraw_bbox_lbrt(fp, xox+xu->cx+xu->cox, xoy+xu->cy+xu->coy, xu);
        }
        else {
            epsdraw_bbox_lbrt(fp, xox+xu->csx, xoy+xu->csy, xu);
        }
#endif

        printf("REMARK END\n");
    }

    fprintf(fp, "%% %s: ydir %d xox %d xoy %d\n",
        __func__, ydir, xox, xoy);

    fprintf(fp, "  newpath\n");

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


    if(xu->cob.segar && xu->cob.segar->use>0) {
    }
    else {
        goto out;
    }

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

    fprintf(fp, "  %d %d moveto\n", x1, y1);

    for(i=0;i<xu->cob.segar->use;i++) {
P;
        s = (seg*)xu->cob.segar->slot[i];
        if(!s) {
            continue;
        }

printf("%s: ptype %d\n", __func__, s->ptype);
printf("b cdir %d\n", cdir);

        switch(s->ptype) {

#if 1
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
#endif

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
P;

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

#if 0
            fprintf(fp, "  %d %d %d %d %d arc stroke %% seg-arc\n",
                arcx, arcy, s->rad, cdir-90, cdir-90+s->ang);
#endif

#if 0
            fprintf(fp, "  %d %d %d %d %d arc %% seg-arc\n",
                arcx, arcy, s->rad, cdir-90, cdir-90+s->ang);
#endif

#if 1
            if(draft_mode) {
                int tx, ty; 
                int a;
                fprintf(fp, "     gsave\n");
                printf("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
                for(a=cdir-90;a<=cdir-90+s->ang;a++) {
                    if(a==cdir-90||a==cdir-90+s->ang||a%90==0) {
                        tx = arcx + s->rad*cos((a)*rf);
                        ty = arcy + s->rad*sin((a)*rf);
                        printf("      arc  %d tx,ty %d,%d\n", a, tx, ty);
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


#if 0
            epsdraw_segarcarrow(fp, cdir, xox, xoy, x1, y1, x2, y2,
                arcx, arcy, s->rad, s->ang, xu, xns);
#endif
            epsdraw_segarcXarrow(fp,
                arcx, arcy, s->rad, cdir-90, cdir-90+s->ang, ADIR_ARC,
                xu, xns);


            cdir += s->ang;
fprintf(fp, "%% a cdir %d arc\n", cdir);

            break;
        case OA_ARCN:
P;

#if 0
    fprintf(fp, "%%  cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);
#endif
    fprintf(stderr, "%% b cdir %d rad %d ang %d\n", cdir, s->rad, s->ang);

            arcx = x1 + s->rad*cos((cdir-90)*rf);
            arcy = y1 + s->rad*sin((cdir-90)*rf);

#if 0
    fprintf(fp, "%%  arcx %d arcy %d\n", arcx, arcy);
#endif

            x2 = arcx + s->rad*cos((cdir-s->ang+90)*rf);
            y2 = arcy + s->rad*sin((cdir-s->ang+90)*rf);


            
#if 0
            fprintf(fp, "  %d %d %d %d %d arcn stroke %% seg-arcn\n",
                arcx, arcy, s->rad, cdir+90, cdir+90-s->ang);
#endif
#if 0
            fprintf(fp, "  %d %d %d %d %d arcn %% seg-arcn\n",
                arcx, arcy, s->rad, cdir+90, cdir+90-s->ang);
#endif

#if 1
            if(draft_mode) {
                int tx, ty; 
                int a;
                fprintf(fp, "    gsave\n");
                printf("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
                for(a=cdir+90;a>=cdir+90-s->ang;a--) {
                    if(a==cdir+90||a==cdir+90-s->ang||a%90==0) {
                        tx = arcx + s->rad*cos((a)*rf);
                        ty = arcy + s->rad*sin((a)*rf);
                        printf("      arc  %d tx,ty %d,%d\n", a, tx, ty);
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

#if 0
            epsdraw_segarcnarrow(fp, cdir, xox, xoy, x1, y1, x2, y2,
                arcx, arcy, s->rad, s->ang, xu, xns);
#endif
            epsdraw_segarcXarrow(fp,
                arcx, arcy, s->rad, cdir+90, cdir+90-s->ang, ADIR_ARCN,
                xu, xns);
            

            cdir -= s->ang;
fprintf(fp, "%% a cdir  arcn%d\n", cdir);

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
                if(i==xu->cob.segar->use-1) {
                    xu->cob.arrowheadpart &= ~AR_BACK;
                }
                else {
                    xu->cob.arrowheadpart = 0;
                }
            }

#if 1
            epsdraw_seglinearrow(fp, cdir, xox, xoy, x1, y1, x2, y2, xu, xns);
#endif  
#if 0
            fprintf(fp, "  %d %d lineto\n", x2, y2);
#endif
            

            xu->cob.arrowheadpart       = ap;
            xu->cob.arrowbackheadtype   = bh;
            xu->cob.arrowforeheadtype   = fh;
            break;
        }
        
next:
        x1 = x2;
        y1 = y2;

printf("a cdir %d\n", cdir);
P;
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
_line_path(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int i;
    int x0, y0;
    int x1, y1, x2, y2;
    seg *s;
    int cdir;

    int ap, fh, bh;
    int arcx, arcy;
    int qbx, qby;
    int qex, qey;
    int qcx, qcy;


#if 1
fprintf(stdout, "%s: ydir %d xox %d xoy %d \n",
    __func__, ydir, xox, xoy);
#endif
    Echo("%s: enter\n", __func__);

    cdir = ydir;

    if(bbox_mode) {
#if 0
        epsdraw_bbox(fp, xu);
        epsdraw_bbox_glbrt(fp, xu);
#endif
        epsdraw_bbox_lbrt(fp, xox, xoy, xu);
    }

    fprintf(fp, "%% %s: ydir %d xox %d xoy %d\n",
        __func__, ydir, xox, xoy);

    fprintf(fp, "  newpath\n");

    if(!xu->cob.originalshape) {
        x1 = xox+xu->csx;
        y1 = xoy+xu->csy;
        x2 = xox+xu->cex;
        y2 = xoy+xu->cey;

#if 0
        fprintf(fp, " gsave    %% regular shape %s\n", __func__);
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x1, y1, x2, y2, xu, xns);
        fprintf(fp, " grestore %% regular shape %s\n", __func__);
#endif
        fprintf(fp, "  %d %d moveto %d %d lineto\n", x1, y1, x2, y2);
        goto out;
    }


    if(xu->cob.segar && xu->cob.segar->use>0) {
    }
    else {
        goto out;
    }

#if 0
    fprintf(fp, " gsave %% %s\n", __func__);
#endif

    Echo("    segar.use %d\n", xu->cob.segar->use);

    x0 = x1 = xox+xu->csx;
    y0 = y1 = xoy+xu->csy;

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

    fprintf(fp, "  %d %d moveto\n", x1, y1);

    for(i=0;i<xu->cob.segar->use;i++) {
        s = (seg*)xu->cob.segar->slot[i];
        if(!s) {
            continue;
        }

printf("%s: ptype %d\n", __func__, s->ptype);

        switch(s->ptype) {

#if 1
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
#endif

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
            
#if 0
            fprintf(fp, "  %d %d %d %d %d arc stroke %% seg-arc\n",
                arcx, arcy, s->rad, cdir-90, cdir-90+s->ang);
#endif

            fprintf(fp, "  %d %d %d %d %d arc %% seg-arc\n",
                arcx, arcy, s->rad, cdir-90, cdir-90+s->ang);

#if 1
            if(draft_mode) {
                int tx, ty; 
                int a;
                fprintf(fp, "     gsave\n");
                printf("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
                for(a=cdir-90;a<=cdir-90+s->ang;a++) {
                    if(a==cdir-90||a==cdir-90+s->ang||a%90==0) {
                        tx = arcx + s->rad*cos((a)*rf);
                        ty = arcy + s->rad*sin((a)*rf);
                        printf("      arc  %d tx,ty %d,%d\n", a, tx, ty);
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
fprintf(fp, "%% a cdir %d\n", cdir);

            break;
        case OA_ARCN:

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
            
#if 0
            fprintf(fp, "  %d %d %d %d %d arcn stroke %% seg-arcn\n",
                arcx, arcy, s->rad, cdir+90, cdir+90-s->ang);
#endif
            fprintf(fp, "  %d %d %d %d %d arcn %% seg-arcn\n",
                arcx, arcy, s->rad, cdir+90, cdir+90-s->ang);

#if 1
            if(draft_mode) {
                int tx, ty; 
                int a;
                fprintf(fp, "    gsave\n");
                printf("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
                for(a=cdir+90;a>=cdir+90-s->ang;a--) {
                    if(a==cdir+90||a==cdir+90-s->ang||a%90==0) {
                        tx = arcx + s->rad*cos((a)*rf);
                        ty = arcy + s->rad*sin((a)*rf);
                        printf("      arc  %d tx,ty %d,%d\n", a, tx, ty);
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
fprintf(fp, "%% a cdir %d\n", cdir);

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
        x1 = x2;
        y1 = y2;
    }

    if(xu->type==CMD_CLINE) {
#if 0
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x2, y2, x0, y0, xu, xns);
#endif
#if 0
        fprintf(fp, "  %d %d moveto %d %d lineto\n", x1, y1, x2, y2);
#endif
        fprintf(fp, "  closepath\n");
    }

#if 0
    fprintf(fp, " grestore %% %s\n", __func__);
#endif

out:
    return 0;
}



int
Yepsdraw_linearrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int i;
    int x0, y0;
    int x1, y1, x2, y2;
    seg *s;
    int cdir;

    int ap, fh, bh;

    Echo("%s: enter\n", __func__);

    if(bbox_mode) {
#if 0
        epsdraw_bbox(fp, xu);
        epsdraw_bbox_glbrt(fp, xu);
#endif
        epsdraw_bbox_lbrt(fp, xox, xoy, xu);
    }

    if(!xu->cob.originalshape) {
        x1 = xox+xu->csx;
        y1 = xoy+xu->csy;
        x2 = xox+xu->cex;
        y2 = xoy+xu->cey;

        fprintf(fp, " gsave    %% regular shape %s\n", __func__);
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x1, y1, x2, y2, xu, xns);
        fprintf(fp, " grestore %% regular shape %s\n", __func__);
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

        if(s->ptype==OA_JOIN) {
PP;
            fprintf(fp, " %d %d %d 0 360 arc fill %% join-mark\n",
                x1, y1, xu->cob.outlinethick*2);
            x2 = x1;
            y2 = y1;

            goto next;
        }
        if(s->ptype==OA_SKIP) {
            int qbx, qby;
            int qex, qey;
            int qcx, qcy;
        
PP;
            x2 = x1+s->x1;
            y2 = y1+s->y1;

            qcx = (x1+x2)/2;
            qcy = (y1+y2)/2;

            fprintf(fp, "  gsave ");
#if 0
            fprintf(fp, "  0 0.7 1.0 setrgbcolor\n");
            fprintf(fp, "  %d setlinewidth ", xu->cob.outlinethick*4);
            fprintf(fp, "  %d %d moveto %d %d lineto stroke ",
                x1, y1, x2, y2);
            fprintf(fp, "  0 0 0 setrgbcolor\n");
            fprintf(fp, " %d %d %d 0 360 arc fill %% join-mark\n",
                qcx, qcy, xu->cob.outlinethick*2);
            fprintf(fp, "  %d setlinewidth ", xu->cob.outlinethick);
#endif
            fprintf(fp, "  %d %d moveto ", x1, y1);
            fprintf(fp, "  %d %d %d %d %d arcn ",
                qcx, qcy, xu->cob.outlinethick*2, ydir-180, ydir);
            fprintf(fp, "  %d %d lineto stroke", x2, y2);
            fprintf(fp, "  grestore %% skip-mark\n");

            goto next;
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
            
            fprintf(fp, "  %d %d %d %d %d arc stroke %% seg-arc\n",
                arcx, arcy, s->rad, cdir-90, cdir-90+s->ang);

#if 1
        {
            int tx, ty; 
            int a;
            fprintf(fp, "gsave\n");
            printf("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
            for(a=cdir-90;a<=cdir-90+s->ang;a++) {
                if(a==cdir-90||a==cdir-90+s->ang||a%90==0) {
                    tx = arcx + s->rad*cos((a)*rf);
                    ty = arcy + s->rad*sin((a)*rf);
                    printf("  arc  %d tx,ty %d,%d\n", a, tx, ty);
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
            
            fprintf(fp, "  %d %d %d %d %d arcn stroke %% seg-arcn\n",
                arcx, arcy, s->rad, cdir+90, cdir+90-s->ang);

#if 1
        {
            int tx, ty; 
            int a;
            fprintf(fp, "gsave\n");
            printf("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
            for(a=cdir+90;a>=cdir+90-s->ang;a--) {
                if(a==cdir+90||a==cdir+90-s->ang||a%90==0) {
                    tx = arcx + s->rad*cos((a)*rf);
                    ty = arcy + s->rad*sin((a)*rf);
                    printf("  arc  %d tx,ty %d,%d\n", a, tx, ty);
                    if(draft_mode) {
                        MC(5, tx, ty);
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

        epsdraw_seglinearrow(fp, cdir, xox, xoy, x1, y1, x2, y2, xu, xns);

        xu->cob.arrowheadpart       = ap;
        xu->cob.arrowbackheadtype   = bh;
        xu->cob.arrowforeheadtype   = fh;
        }
        
next:
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

int
Yepsdraw_clinearrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int i;
    int x0, y0;
    int x1, y1, x2, y2;
    seg *s;
    int cdir;
    int ik;
    int ap, fh, bh;

int epsdraw_hatch(FILE *fp, int aw, int ah, int hc, int hty);
int epsdraw_deco(FILE *fp, int xw, int xh, int xlc, int xfc, char *xcmdlist);

    Echo("%s: enter\n", __func__);

    if(bbox_mode) {
#if 0
fprintf(fp, "gsave\n");
changebbox(fp);
fprintf(fp, "  %d %d translate\n", xu->glx, xu->gby);
fprintf(fp, "  0 0 moveto %d 0 lineto %d %d lineto 0 %d lineto closepath stroke\n", xu->wd, xu->wd, xu->ht, xu->ht);
fprintf(fp, "grestore\n");
#endif
        epsdraw_bbox(fp, xu);
    }

    if(!xu->cob.originalshape) {
        x1 = xox+xu->csx;
        y1 = xoy+xu->csy;
        x2 = xox+xu->cex;
        y2 = xoy+xu->cey;

    fprintf(fp, " gsave %% %s\n", __func__);
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x1, y1, x2, y2, xu, xns);
    fprintf(fp, " grestore %% %s\n", __func__);
        goto out;
    }


    if(xu->cob.segar && xu->cob.segar->use>0) {
    }
    else {
        goto out;
    }

    fprintf(stdout, "oid %d segar ", xu->oid);
    varray_fprint(stdout, xu->cob.segar);


    fprintf(fp, " gsave %% %s\n", __func__);

Echo("    segar.use %d\n", xu->cob.segar->use);

        x0 = x1 = xox+xu->csx;
        y0 = y1 = xoy+xu->csy;
Echo("    csx,csy %d,%d\n", xu->csx, xu->csy);
Echo("    x1,y1 %d,%d\n", x1, y1);

    if(xu->type==CMD_CLINE) {
#if 0
        x0 = x1 = xox+xu->csx+xu->wd/2+xu->cox;
        y0 = y1 = xoy+xu->csy+xu->ht/2+xu->coy;
        y0 = y1 = xoy+xu->csy+xu->coy;
#endif
#if 1
        x0 = x1 = xox+xu->cx+xu->cox;
        y0 = y1 = xoy+xu->cy+xu->coy;
#endif
    }

        fprintf(fp, "  %d %d moveto\n", x0, y0);


    for(i=0;i<xu->cob.segar->use;i++) {
        s = (seg*)xu->cob.segar->slot[i];
        if(!s) {
            continue;
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


        cdir = (int)atan2(y2-y1,x2-x1);

        Echo("    part seg %d: ftflag %d : %d,%d : %d,%d -> %d,%d cdir %d\n",
            i, s->ftflag, s->x1, s->y1, x1, y1, x2, y2, cdir);

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

        epsdraw_seglinearrow(fp, cdir, xox, xoy, x1, y1, x2, y2, xu, xns);

        xu->cob.arrowheadpart       = ap;
        xu->cob.arrowbackheadtype   = bh;
        xu->cob.arrowforeheadtype   = fh;
        }
#endif
        fprintf(fp, "  %d %d lineto %% seg %d\n", x2, y2, i);
        
        x1 = x2;
        y1 = y2;
    }

    fprintf(fp, "  %d %d lineto %% backto first point\n", x0, y0);
        if(debug_clip) {
            fprintf(fp, "  stroke\n");
        }
        else {
            fprintf(fp, "  clip\n");
        }

    /* hatching function requires centerling by x,y */
    fprintf(fp, "  gsave\n");
    fprintf(fp, "  %d %d translate\n", xu->cgx, xu->cgy);
    changecolor(fp, xu->cob.fillcolor);
    changethick(fp, xu->cob.hatchthick);
    epsdraw_hatch(fp, xu->wd, xu->ht, 
        xu->cob.fillcolor, xu->cob.fillhatch);

    if(xu->cob.deco) {
        fprintf(fp, "%% deco |%s|\n", xu->cob.deco);
        epsdraw_deco(fp, xu->wd, xu->ht,
            xu->cob.outlinecolor, xu->cob.fillcolor, xu->cob.deco);
    }
    else {
        fprintf(fp, "%% no-deco\n");
    }
    fprintf(fp, "  grestore\n");

#if 0
    if(xu->type==CMD_CLINE) {
        epsdraw_seglinearrow(fp, ydir, xox, xoy, x2, y2, x0, y0, xu, xns);
    }
#endif

    fprintf(fp, " grestore %% %s\n", __func__);

    ik = Yepsdraw_linearrow(fp, ydir, xox, xoy, xu, xns);

out:
    return 0;
}

int
Zepsdraw_linearrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int r;
#if 0
fprintf(stdout, "%s: enter\n", __func__);
#endif

#if 0
    fprintf(fp, " gsave\n");
    changecolor(fp, 10);
    changethick(fp, objunit/10);
    r = _line_path(fp, ydir, xox, xoy, xu, xns);
    fprintf(fp, "  stroke\n");
    fprintf(fp, " grestore\n");
#endif

    fprintf(fp, " gsave\n");
    r = _line_deco(fp, ydir, xox, xoy, xu, xns);
#if 0
    fprintf(fp, "  stroke\n");
#endif
    fprintf(fp, " grestore\n");

    return r;
}

int
Zepsdraw_clinearrow(FILE *fp,
    int ydir, int xox, int xoy, ob *xu, ns *xns)
{
    int r;
    int aw, ah;
int epsdraw_hatch(FILE *fp, int aw, int ah, int hc, int hty);

#if 0
fprintf(stdout, "%s: enter\n", __func__);
#endif

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
#if 0
        r = _line_path(fp, ydir, xox, xoy+objunit/4, xu, xns);
        fprintf(fp, "  stroke\n");
#endif
        r = _line_path(fp, ydir, xox, xoy, xu, xns);
        fprintf(fp, "  clip\n");
#if 0
        fprintf(fp, "  eoclip\n");
        fprintf(fp, "  stroke\n");
#endif
fprintf(fp, " %% bb %d %d %d %d\n", xu->lx, xu->by, xu->rx, xu->ty);
fprintf(fp, " %% center %d %d\n", xu->x, xu->y);
fprintf(fp, " %% xox %d xoy %d\n", xox, xoy);
        fprintf(fp, "  %d %d translate\n", xu->x+xox, xu->y+xoy);
        epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor, xu->cob.fillhatch);

        fprintf(fp, " grestore\n");
    }
    else {
        fprintf(fp, " %% no-fill\n");
    }

#if 0
    fprintf(fp, " gsave\n");
    changecolor(fp, 10);
    changethick(fp, objunit/10);
    r = _line_path(fp, ydir, xox, xoy, xu, xns);
    fprintf(fp, "  stroke\n");
    fprintf(fp, " grestore\n");
#endif

    fprintf(fp, " gsave\n");
    changecolor(fp, xu->cob.outlinecolor);
    r = _line_deco(fp, ydir, xox, xoy, xu, xns);
#if 0
    fprintf(fp, "  stroke\n");
#endif
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

    int epsdraw_hatch(FILE *fp, int aw, int ah, int hc, int hty);

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

    epsdraw_hatch(fp, cw, ch, xu->cob.fillcolor, xu->cob.fillhatch);
 }
#endif

    fprintf(fp, "grestore\n");
  }


 if(xu->cob.outlinecolor>=0) {
    fprintf(fp, "gsave\n");
    changecolor(fp, xu->cob.outlinecolor);
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
Yepsdraw_blinearrow(FILE *fp,
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

    if(bbox_mode) {
#if 0
        fprintf(fp, "gsave\n");
        changethick(fp, objunit*4/100);
        changecolor(fp, 1);
        drawCrect(fp, xox+xu->lx, xoy+xu->by, xu->rx-xu->lx, xu->ty-xu->by);
        fprintf(fp, "grestore\n");
#endif
#if 0
        fprintf(fp, "gsave\n");
        changethick(fp, objunit*1/100);
        changecolor(fp, 4);
        drawCrect(fp, xu->glx, xu->gby, xu->grx-xu->glx, xu->gty-xu->gby);
        fprintf(fp, "grestore\n");
#endif
        epsdraw_bbox(fp, xu);
    }

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
            printf("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
            for(a=cdir-90;a<=cdir-90+s->ang;a++) {
                if(a==cdir-90||a==cdir-90+s->ang||a%90==0) {
                    tx = arcx + s->rad*cos((a)*rf);
                    ty = arcy + s->rad*sin((a)*rf);
                    printf("  arc  %d tx,ty %d,%d\n", a, tx, ty);
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
            printf("arc  %d .. %d\n", cdir-90, cdir-90+s->ang);
            for(a=cdir-90;a<=cdir-90+s->ang;a++) {
                if(a==cdir-90||a==cdir-90+s->ang||a%90==0) {
                    tx = arcx + s->rad*cos((a)*rf);
                    ty = arcy + s->rad*sin((a)*rf);
                    printf("  arc  %d tx,ty %d,%d\n", a, tx, ty);
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
epsdraw_hatch(FILE *fp, int aw, int ah, int hc, int hty)
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
        for(y1=-ah*3/5;y1<ah*3/5;y1+=def_hatchpitch) {
            c = 0;
            for(x1=-aw*3/5;x1<aw*3/5;x1+=def_hatchpitch) {
                if(c%2==0) {
                    y2 = y1-def_hatchpitch/2;
                    y3 = y1;
                }
                else {
                    y3 = y1-def_hatchpitch/2;
                    y2 = y1;
                }
                x2 = x1;
                x3 = x1+def_hatchpitch;
                fprintf(fp, " %d %d moveto %d %d lineto stroke\n",
                    x2, y2, x3, y3);
                c++;
            }
        }
        break;

    case HT_VZIGZAG:
        for(x1=-aw*3/5;x1<aw*3/5;x1+=def_hatchpitch) {
            c = 0;
            for(y1=-ah*3/5;y1<ah*3/5;y1+=def_hatchpitch) {
                if(c%2==0) {
                    x2 = x1-def_hatchpitch/2;
                    x3 = x1;
                }
                else {
                    x3 = x1-def_hatchpitch/2;
                    x2 = x1;
                }
                y2 = y1;
                y3 = y1+def_hatchpitch;
                fprintf(fp, " %d %d moveto %d %d lineto stroke\n",
                    x2, y2, x3, y3);
                c++;
            }
        }
        break;

    case HT_HLINED:
        x1 = -2*aw;
        x2 =  2*aw;
        for(y1=-2*ah;y1<2*ah;y1+=def_hatchpitch) {
            y2 = y1;
            fprintf(fp, "      %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        break;

    case HT_VLINED:
        y1 = -2*aw;
        y2 =  2*aw;
        for(x1=-2*aw;x1<2*aw;x1+=def_hatchpitch) {
            x2=x1;
            fprintf(fp, "      %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        break;
    
    case HT_CROSSED:
        x1 = -2*aw;
        x2 =  2*aw;
        for(y1=-2*ah;y1<2*ah;y1+=def_hatchpitch) {
            y2 = y1;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        y1 = -2*aw;
        y2 =  2*aw;
        for(x1=-2*aw;x1<2*aw;x1+=def_hatchpitch) {
            x2=x1;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        break;
    
    case HT_XCROSSED:
#if 0
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-2*aw;x1<aw*2+ah*2;x1+=def_hatchpitch) {
            x2 = x1-ah*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }

        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-aw*6/10;x1<aw*6/10+ah*2;x1+=def_hatchpitch) {
            x2 = x1-ah*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x2, y1, x1, y2);
        }
#endif
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-2*aw;x1<aw*2+ah*2;x1+=def_hatchpitch) {
            x2 = x1-ah*6/10*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-aw*6/10;x1<aw*6/10+ah*2;x1+=def_hatchpitch) {
            x2 = x1-ah*6/10*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x2, y1, x1, y2);
        }

        break;

    case HT_BACKSLASHED:
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-2*aw;x1<aw*2+ah*2;x1+=def_hatchpitch) {
            x2 = x1-ah*6/10*2;
            fprintf(fp, "       %d %d moveto %d %d lineto stroke\n",
                x1, y1, x2, y2);
        }
        break;

    case HT_DOTTED:
        for(x1=-aw;x1<aw;x1+=def_hatchpitch) {
            for(y1=-ah;y1<ah;y1+=def_hatchpitch) {
                x2=x1+def_hatchthick;
                y2=y1;
                fprintf(fp, "      %d %d moveto %d %d lineto stroke\n",
                    x1, y1, x2, y2);
            }
        }
        break;

    case HT_SPARSEDOTTED:
        for(x1=-aw;x1<aw;x1+=def_hatchpitch*2) {
            for(y1=-ah;y1<ah;y1+=def_hatchpitch*2) {
                x2=x1+def_hatchthick;
                y2=y1;
                fprintf(fp, "      %d %d moveto %d %d lineto stroke\n",
                    x1, y1, x2, y2);
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
#if 0
        break;
#endif
    case HT_SLASHED:
        y1 = -ah*6/10;
        y2 =  ah*6/10;
        for(x1=-aw*6/10;x1<aw*6/10+ah*2;x1+=def_hatchpitch) {
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
printf(" %2d: %3d '%s'\n", i, w, uu->ssval);
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



#define FH_NORMAL   (0)
#define FH_LARGE    (1)
#define FH_HUGE     (2)
#define FH_SMALL    (3)
#define FH_TINY     (4)

apair_t fh_ial[] = {
    {"normal",  FH_NORMAL},
    {"large",   FH_LARGE},
    {"huge",    FH_HUGE},
    {"small",   FH_SMALL},
    {"tiny",    FH_TINY},
    {NULL,      -1},
};

apair_t fh_act_ial[] = {
    {"1.0",     FH_NORMAL},
    {"1.414",   FH_LARGE},
    {"2.0",     FH_HUGE},
    {"0.707",   FH_SMALL},
    {"0.5",     FH_TINY},
    {NULL,      -1},
};

#define FF_SANSERIF (0)
#define FF_SERIF    (1)
#define FF_ITALIC   (2)
#define FF_TYPE     (3)

apair_t ff_ial[] = {
    {"serif",       FF_SERIF},
    {"sanserif",    FF_SANSERIF},
    {"italic",      FF_ITALIC},
    {"type",        FF_TYPE},
    {NULL,      -1},
};

apair_t ff_act_ial[] = {
    {"/Times-Roman",    FF_SERIF},
    {"/Helvetica",      FF_SANSERIF},
    {"/Times-Italic",   FF_ITALIC},
    {"/Courier",        FF_TYPE},
    {NULL,      -1},
};


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
    
    if(!ssar) {
        E;
        return -1;
    }

#if 0
    varray_fprint(stdout, ssar);
#endif

    if(ssar->use<=0)  {
        goto skip_label;
    }

#if 0
    ss_dump(stdout, ssar);
#endif
    ss_strip(mcontall, BUFSIZ, ssar);
#if 0
    fprintf(stdout, "mcontall '%s'\n", mcontall);
#endif

    fprintf(fp, "%% %s: pos %6d,%-6d %6dx%-6d %4d fg %d bg %d %d %d ; %s\n",
        __func__, x, y, wd, ht, ro,
        fgcolor, bgcolor, bgshape, qbgmargin, mcontall);
    fprintf(fp, "%% fg %d bg %d\n", fgcolor, bgcolor);


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

    fprintf(fp, "    %% text offset\n");
#if 0
    fprintf(fp, "    %d %d translate\n", 0, ht/2-rh/2);
#endif
    fprintf(fp, "    %d %d translate\n", 0, (py*n)/2-py+pyb);

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

    fprintf(fp, "gsave %% bgdraw\n");

    gy = 0;
    for(i=0;i<ssar->use;i++) {
        gy -= py;
        uu = (sstr*)ssar->slot[i];

        if(tq) txe_release(tq);

        tq = varray_new();
        varray_entrysprintfunc(tq, txe_sprint);

        txe_parse(tq, uu->ssval);

        varray_fprint(stdout, tq);

        justify = SJ_CENTER;
        hscale = 100;

        fprintf(fp, "%% enter str %d '%s' gy %d\n", i, uu->ssval, gy);

        fprintf(fp, " gsave %% oneline\n");

printf("  --- calc size\n");

        cursize = FH_NORMAL;
        curface = FF_SERIF;
        hscale    = 100;

        /* check content existance */
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

        if(!mcontline[0]) {
            fprintf(fp, "%% skip  sstr drawing %d '%s'\n", i, mcontline);
            goto skip_bgdrawing;
        }
        else {
            fprintf(fp, "%% enter sstr drawing %d '%s'\n", i, mcontline);
        }

#if 0
        MTF(0, -wd/4, fht, 0);
        MTF(1, -wd/4, 0, 0);
        MTF(4, -wd/4, -pyb, 0);

        MCF(5, -wd/2, fht+bgmargin);
        MQF(5, -wd/2, -pyb-bgmargin);
#endif

        fprintf(fp, "  %% calc size (width)\n");
        fprintf(fp, "  /sstrw 0 def\n");
        fprintf(fp, "  /Times-Roman findfont %d scalefont setfont\n", fht);
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
printf("    token '%s'\n", token);
                        if(strncasecmp(token,"scale", 5)==0) {
                            hscale = atoi(token+5);
printf("    hscale %d\n", hscale);

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
printf(" newface %d newsize %d\n", newface, newsize);
                    if(newsize>=0) {
P;
                        cursize = newsize;
                    }
                    if(newface>=0) {
P;
                        curface = newface;
                    }
printf(" curface %d cursize %d\n", curface, cursize);
                    afn  = rassoc(ff_act_ial, curface);
                    afhs = rassoc(fh_act_ial, cursize);
                    afh  = fht;
                    if(afhs!=NULL) {
                        afh = atof(afhs)*fht;
                    }

printf("  afn '%s' afhs '%s' afh %d (max %d)\n", afn, afhs, afh, afhmax);

                    if(afn) {
                        fprintf(fp, "  %s findfont %d scalefont setfont\n",
                            afn, afh);
                    }
                }
            }
            else
            if(te->ct==TXE_DATA) {
                if(te->st==TXE_CONST) {
                    psescape(qs, BUFSIZ, te->cs);
                }
                else {
                    psescape(qs, BUFSIZ, te->vs);
                }
                if(hscale!=100) {
                    fprintf(fp, "  gsave\n");
                    fprintf(fp, "    %.3f 1 scale\n", (double)hscale/100);
                }
    if(hscale!=100) {
        fprintf(fp, "  (%s) stringwidth pop /sstrw exch %f mul sstrw add def\n", qs, (double)hscale/100);
    }
    else {
        fprintf(fp, "  (%s) stringwidth pop /sstrw exch sstrw add def\n", qs);
    }
                if(hscale!=100) {
                    fprintf(fp, "  grestore\n");
                }
#if 1
                /* check max height when it used */
                if(afh>afhmax) {afhmax = afh; }
#endif
            }

        }

        fprintf(fp, 
            "  sstrwar %d sstrw put %% store value to reuse\n", i);

        if(text_mode) {
            fprintf(fp, "  %% textguide\n");
            fprintf(fp, "  gsave\n");

#if 0
            changetext3(fp);
            fprintf(fp, "    %d 0 moveto\n",  -wd/2);
            fprintf(fp, "    %d 0 rlineto\n", wd);
            fprintf(fp, "    0  %d rlineto\n", wd/4);
            fprintf(fp, "    stroke\n");
#endif

            changetext2(fp);
            fprintf(fp, "    sstrw 2 div neg %d moveto\n", objunit*7/100);
            fprintf(fp, "    0 %d rlineto\n", -objunit*7/100);
            fprintf(fp, "    sstrw 0 rlineto\n");
            fprintf(fp, "    0 %d rlineto\n", objunit*7/100);
            fprintf(fp, "    stroke\n");

            fprintf(fp, "  grestore\n");
        }



        fprintf(fp, "%% justify %d\n", justify);
        
        switch(justify) {
        case SJ_LEFT:
            fprintf(fp, "  sstrw neg 0 translate\n");
            break;
        case SJ_RIGHT:
            break;
        default:
        case SJ_CENTER:
            fprintf(fp, "  sstrw 2 div neg 0 translate\n");
            break;
        case SJ_FIT:
#if 0
            fprintf(fp, "  sstrw 2 div neg 0 translate\n");
#endif
#if 0
            fprintf(fp, "  %d 2 div neg 0 translate\n", wd);
#endif
            fprintf(fp, "  %d sstrw div 1 scale\n", wd);
            fprintf(fp, "  sstrw 2 div neg 0 translate\n");
            break;
        }

        if(bgcolor>=0) {
            fprintf(fp, "%% bg fill\n");
            fprintf(fp, "  gsave %% textbg\n");
            changecolor(fp, bgcolor);

            fprintf(fp, "    0 %d neg sstrw %d %d mrboxfill\n",
                pyb, pyb+afhmax, bgmargin);

#if 0
            MTF(1, 0, afhmax, 0);
#endif

            fprintf(fp, "  grestore %% textbg\n");
        }

skip_bgdrawing:
        (void)0;

        fprintf(fp, " grestore %% oneline\n");

        fprintf(fp, "   0 %d translate\n", -py);
    }
    fprintf(fp, "grestore %% bgdraw\n");


    /*****
     ***** 2nd pass
     *****/

    fprintf(fp, "gsave %% txtdraw\n");

    gy = 0;
    for(i=0;i<ssar->use;i++) {
        gy -= py;
        uu = (sstr*)ssar->slot[i];

        if(tq) txe_release(tq);

        tq = varray_new();
        varray_entrysprintfunc(tq, txe_sprint);

        txe_parse(tq, uu->ssval);

        varray_fprint(stdout, tq);

        justify = SJ_CENTER;
        hscale = 100;

        fprintf(fp, "%% enter str %d '%s' gy %d\n", i, uu->ssval, gy);

        fprintf(fp, " gsave %% oneline\n");


printf("  --- calc size\n");

        cursize = FH_NORMAL;
        curface = FF_SERIF;
        hscale    = 100;

        /* check content existance */
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

        if(!mcontline[0]) {
            fprintf(fp, "%% skip  sstr drawing %d '%s'\n", i, mcontline);
            goto skip_txtdrawing;
        }
        else {
            fprintf(fp, "%% enter sstr drawing %d '%s'\n", i, mcontline);
        }


        fprintf(fp, "  /sstrw sstrwar %d get def %% reuse width\n", i);

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
            fprintf(fp, "  %% textguide\n");
            fprintf(fp, "  gsave\n");

#if 0
            changetext3(fp);
            fprintf(fp, "    %d 0 moveto\n",  -wd/2);
            fprintf(fp, "    %d 0 rlineto\n", wd);
            fprintf(fp, "    0  %d rlineto\n", wd/4);
            fprintf(fp, "    stroke\n");
#endif

            changetext2(fp);
            fprintf(fp, "    sstrw 2 div neg %d moveto\n", objunit*7/100);
            fprintf(fp, "    0 %d rlineto\n", -objunit*7/100);
            fprintf(fp, "    sstrw 0 rlineto\n");
            fprintf(fp, "    0 %d rlineto\n", objunit*7/100);
            fprintf(fp, "    stroke\n");

            fprintf(fp, "  grestore\n");
        }



        fprintf(fp, "%% justify %d\n", justify);
        
        switch(justify) {
        case SJ_LEFT:
            fprintf(fp, "  sstrw neg 0 translate\n");
            break;
        case SJ_RIGHT:
            break;
        default:
        case SJ_CENTER:
            fprintf(fp, "  sstrw 2 div neg 0 translate\n");
            break;
        case SJ_FIT:
#if 0
            fprintf(fp, "  sstrw 2 div neg 0 translate\n");
#endif
#if 0
            fprintf(fp, "  %d 2 div neg 0 translate\n", wd);
#endif
            fprintf(fp, "  %d sstrw div 1 scale\n", wd);
            fprintf(fp, "  sstrw 2 div neg 0 translate\n");
            break;
        }

        /*** PASS 2 */

printf("  --- drawing\n");
        fprintf(fp, "  /Times-Roman findfont %d scalefont setfont\n", fht);
        fprintf(fp, "  0 0 moveto\n");

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
printf("    hscale %d\n", hscale);
                        }
                        if(strncasecmp(token,"hscale", 6)==0) {
                            hscale = atoi(token+6);
printf("    hscale %d\n", hscale);
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
printf(" newface %d newsize %d\n", newface, newsize);
                    if(newsize>=0) {
P;
                        cursize = newsize;
                    }
                    if(newface>=0) {
P;
                        curface = newface;
                    }
printf(" curface %d cursize %d\n", curface, cursize);

                    afn  = xrassoc(ff_act_ial, curface);
                    afhs = xrassoc(fh_act_ial, cursize);
                    afh  = fht;
                    if(afhs!=NULL) {
                        afh = atof(afhs)*fht;
                    }

printf("  afn '%s' afhs '%s' afh %d\n", afn, afhs, afh);

                    if(afn) {
                        fprintf(fp, "  %s findfont %d scalefont setfont\n",
                            afn, afh);
                    }

                }
            }
            else
            if(te->ct==TXE_DATA) {
                if(te->st==TXE_CONST) {
                    psescape(qs, BUFSIZ, te->cs);
                }
                else {
                    psescape(qs, BUFSIZ, te->vs);
                }
                if(hscale!=100) {
P;
                    fprintf(fp, "  gsave %% comp\n");
                    fprintf(fp, "    %.3f 1 scale\n", (double)hscale/100);
                }
                fprintf(fp, "    (%s) show\n", qs);
                if(hscale!=100) {
P;
                    fprintf(fp, "  grestore %% comp\n");
                }
            }

        }

skip_txtdrawing:
        (void)0;

        fprintf(fp, " grestore %% oneline\n");

        fprintf(fp, "   0 %d translate\n", -py);
    }
#if 1
    fprintf(fp, "grestore %% txtdraw\n");
#endif



    fprintf(fp, "grestore %% end of sstr\n");

skip_label:
    (void)0;

    return 0;
}






int
epsdraw_sstrbg(FILE *fp, int x, int y, int wd, int ht, int ro,
        int bgshape, int qbgmargin, int fgcolor, int bgcolor,
        varray_t *ssar)
{

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

    if(!ssar) {
        E;
        return -1;
    }

#if 0
    varray_fprint(stdout, ssar);
#endif

    if(ssar->use<=0)  {
        goto skip_label;
    }

    fprintf(fp, "%% %s: sstr %d,%d %dx%d\n", __func__, x, y, wd, ht);
    fprintf(fp, "%% fg %d bg %d\n", fgcolor, bgcolor);

    n = ssar->use;

    fht = def_textheight; 
    pyb = (int)((double)fht*textdecentfactor);
    bgmargin = (int)((double)fht*textbgmarginfactor);

    py = fht+bgmargin*2;
    rh = ht-n*py;

    fprintf(fp, "%%  fht %d, ht %d, n %d, rh %d, py %d\n",
        fht, ht, n, rh, py);

    Echo("ht %d use %d -> py %d\n", 
        ht, ssar->use, py);

    fprintf(fp, "gsave %% for sstr\n");
    fprintf(fp, "  /Times-Roman findfont %d scalefont setfont\n", fht);
    fprintf(fp, "  %d %d translate\n", x, y);
    fprintf(fp, "  %d rotate\n", ro);

    if(text_mode) {
        fprintf(fp, "  gsave\n");
        changetext(fp);
        fprintf(fp, "    %d %d moveto %d %d rlineto stroke\n",
                            0, -ht/2, 0, ht);
        fprintf(fp, "  grestore\n");
    }

    fprintf(fp, "    %% text offset\n");
    fprintf(fp, "    %d %d translate\n", 0, ht/2-rh/2);

    gy = 0;
    for(i=0;i<ssar->use;i++) {
        gy -= py;
        uu = (sstr*)ssar->slot[i];
        psescape(qs, BUFSIZ, uu->ssval);

        if(bgcolor>=0) {
            fprintf(fp, "  %% textbg\n");
            fprintf(fp, "  gsave\n");
            changecolor(fp, bgcolor);
            fprintf(fp, "    (%s) stringwidth pop %d add 2 div neg %d moveto\n",
                qs, bgmargin*2, gy-bgmargin);
            fprintf(fp, "    (%s) stringwidth pop %d add dup\n",
                qs, bgmargin*2);
            fprintf(fp, "    0 rlineto 0 %d rlineto neg 0 rlineto\n",
                fht+bgmargin*2);
            fprintf(fp, "    0 %d closepath fill\n",
                -(fht+bgmargin*2));
            fprintf(fp, "  grestore\n");
        }

        if(text_mode) {
            fprintf(fp, "  %% textguide\n");
            fprintf(fp, "  gsave\n");
            changetext2(fp);
            fprintf(fp, "  %d %d moveto %d 0 rlineto stroke\n",
                            -wd/2, gy, wd);
            changetext(fp);
            fprintf(fp, "  %d %d moveto %d 0 rlineto stroke\n",
                            -wd/2, gy+pyb, wd);
            fprintf(fp, "  grestore\n");
        }

        changecolor(fp, fgcolor);

        if(uu->ssopt & SJ_RIGHT) {
        /* right end on center */
        fprintf(fp, 
            "    (%s) dup stringwidth pop neg %d moveto show\n",
            qs, gy+pyb);
        }
        else
        if(uu->ssopt & SJ_RIGHT) {
        /* left end on center */
        fprintf(fp, "    %d %d moveto (%s) show\n",
            0, gy+pyb, qs);
        }
        else {
        /* centering */
        fprintf(fp, 
            "    (%s) dup stringwidth pop 2 div neg %d moveto show\n",
            qs, gy+pyb);
        }
    }
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

    if(bbox_mode) {
        epsdraw_bbox(fp, xu);
    }

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
                xu->cob.fillcolor, xu->cob.fillhatch);
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

int
epsdraw_polygon(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
{
    ob* pf;
    ob* pt;
    int x1, y1;
    int r;
    double a;
    double x2, y2;
    int    i;
    int    n;
    double aoff;

    int aw, ah, ar;

    x1 = xox+xu->cx;
    y1 = xoy+xu->cy;
    r  = (xu->wd/2);


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

apply:

    fprintf(fp, "%% polygon no imagin\n");

    if(bbox_mode) {
        epsdraw_bbox(fp, xu);
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
            fprintf(fp, "  newpath\n");
            for(i=0;i<n;i++) {
                x2 = ar*cos(M_PI*2/(double)n*(double)i+aoff);
                y2 = ar*sin(M_PI*2/(double)n*(double)i+aoff);
                if(i==0) {
                    fprintf(fp, "  %.3f %.3f moveto\n", x2, y2);
                }
                else {
                }
                    fprintf(fp, "  %.3f %.3f lineto\n", x2, y2);
            }
            fprintf(fp, "  closepath\n");
            fprintf(fp, "  fill\n");
            fprintf(fp, "grestore\n");
        }
        else {

            fprintf(fp, "gsave\n");
            fprintf(fp, "  %d %d translate\n", x1, y1);
            changecolor(fp, xu->cob.fillcolor);
            fprintf(fp, "  newpath\n");
            for(i=0;i<n;i++) {
                x2 = ar*cos(M_PI*2/(double)n*(double)i+aoff);
                y2 = ar*sin(M_PI*2/(double)n*(double)i+aoff);
                if(i==0) {
                    fprintf(fp, "  %.3f %.3f moveto\n", x2, y2);
                }
                else {
                }
                    fprintf(fp, "  %.3f %.3f lineto\n", x2, y2);
            }
            fprintf(fp, "  closepath\n");
            if(debug_clip) {
                fprintf(fp, "  stroke %% debug\n");
            }
            else {
                fprintf(fp, "  clip\n");
            }

            changethick(fp, xu->cob.hatchthick);
            epsdraw_hatch(fp, xu->wd, xu->ht,
                 xu->cob.fillcolor, xu->cob.fillhatch);

            fprintf(fp, "grestore\n");
        }
    }




    if(xu->cob.outlinecolor>=0 && xu->cob.outlinethick>0) {

        fprintf(fp, "gsave\n");
        fprintf(fp, "  %d %d translate\n", x1, y1);
        changethick(fp, xu->cob.outlinethick);
        changecolor(fp, xu->cob.outlinecolor);
        fprintf(fp, "  newpath\n");
        for(i=0;i<n;i++) {
            x2 = ar*cos(M_PI*2/(double)n*(double)i+aoff);
            y2 = ar*sin(M_PI*2/(double)n*(double)i+aoff);
            if(i==0) {
                fprintf(fp, "  %.3f %.3f moveto\n", x2, y2);
            }
            else {
            }
                fprintf(fp, "  %.3f %.3f lineto\n", x2, y2);
        }
        fprintf(fp, "  closepath\n");
        fprintf(fp, "  stroke\n");
        fprintf(fp, "grestore\n");
    }

    fprintf(fp, "grestore %% for polygon\n");

out:
    return 0;
}

        
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


    if(bbox_mode) {
        epsdraw_bbox(fp, xu);
    }

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
                xu->cob.fillcolor, xu->cob.fillhatch);
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

    if(bbox_mode) {
        epsdraw_bbox(fp, xu);
    }

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
            epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor, xu->cob.fillhatch);

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
    if(bbox_mode) {
        epsdraw_bbox(fp, xu);
    }
    fprintf(fp, "gsave\n");
    fprintf(fp, "  /Times-Roman findfont %d scalefont setfont\n",
        12*objunit/100);
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
    epsdraw_hatch(fp, xu->wd, xu->ht*2, xu->cob.fillcolor, xu->cob.fillhatch);

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

    if(bbox_mode) {
PP;
#if 0
        epsdraw_bbox_glbrt(fp, xu);
#endif
        epsdraw_bbox_lbrt(fp, 0, 0, xu);
        fprintf(fp, "  gsave %% bbox skel\n");
        fprintf(fp, "    1 0 0 setrgbcolor\n");
        fprintf(fp, "    currentlinewidth 4 mul setlinewidth\n");
        drawCRrectskel2(fp, x1, y1, xu->wd, xu->ht, 0);
        fprintf(fp, "  grestore %% bbox skel\n");
    }

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
        epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor, xu->cob.fillhatch);

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

    if(bbox_mode) {
PP;
        epsdraw_bbox(fp, xu);
        fprintf(fp, "gsave\n");
        fprintf(fp, "1 0 0 setrgbcolor\n");
        fprintf(fp, "currentlinewidth 4 mul setlinewidth\n");
        drawCRrectskel2(fp, x1, y1, xu->wd, xu->ht, 0);
        fprintf(fp, "grestore\n");
    }

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
        epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor, xu->cob.fillhatch);


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

    if(bbox_mode) {
        epsdraw_bbox(fp, xu);
    }

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
epsdraw_paper(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
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

apply:

    fprintf(fp, "%% box xy %d,%d wh %dx%d\n", x1, y1, aw, ah);
    fprintf(fp, "gsave %% for box\n");

    if(bbox_mode) {
PP;
        epsdraw_bbox(fp, xu);
    }

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
            epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor, xu->cob.fillhatch);

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
    ob *xu, ob *pf, ob *pb, ns *xns, int opt)
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

    ex = xox + pf->cx - pf->cwd/2;
    ey = xoy + pf->cy;

    eyt = xoy + pf->cy + pf->cht/2;
    eyb = xoy + pf->cy - pf->cht/2;

    miny = INT_MAX;
    maxy = -(INT_MAX-1);

    mini = INT_MAX;
    maxi = -(INT_MAX-1);

        fprintf(fp, "    gsave\n");

    if(ISCHUNK(pb->type)) {
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
        printf("call %d yp %d\n", call, yp);

        usi = uei = esi = eei = dsi = dei = -1;

        j = 0;
        for(i=0;i<pb->cch.nch;i++) {
            pe = (ob*)pb->cch.ch[i];
            if(ISATOM(pe->type)) {
            }
            else {
                continue;
            }

            sx = xox + pb->cx + pb->ox + pe->cx + pe->cwd/2;
            sy = xoy + pb->cy + pb->oy + pe->cy;

            if(sy>maxy) maxy = sy;
            if(sy<miny) miny = sy;

            eex = ex;
            eey = ey+xu->ht/2-(j+1)*yp;

printf(" ag  j %d ; sx,sy %d,%d vs eey %d\n", j, sx, sy, eey);
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

        printf("  cu %d ce %d cd %d\n", cu, ce, cd);
        printf("  usi %d uei %d\n", usi, uei);
        printf("  esi %d eei %d\n", esi, eei);
        printf("  dsi %d dei %d\n", dsi, dei);

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
            printf("i %d j %d: g %d k %d\n", i, j, g, k);

            eex = ex;
            eey = ey+xu->ht/2-(j+1)*yp;


            /*
             *         h1
             * sx,sy ----+ t1x,t1y
             *           | v
             *   t2x,t2y +---- ex,ey
             *            h2
             */ 
            sx = xox + pb->cx + pb->ox + pe->cx + pe->cwd/2;
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
                h1 = yp*(k+1);
                h2 = (ex-sx) - h1;
                v  = (eey-sy);

                printf("  h1 %7d v %7d h2 %7d\n", h1, v, h2);

                fprintf(fp, "   %d %d moveto %d 0 rlineto"
                            " 0 %d rlineto %d 0 rlineto stroke\n",
                            sx, sy, h1, v, h2);

            }

            j++;
        }

    }
    else {
        sx = xox + pb->cx + pb->cwd/2;
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
epsdraw_gather_square(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns, int opt)
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
    if(opt>0) {
#if 0
        jr = objunit/20;
#endif
        jr = xu->cob.outlinethick*2;
    }

    ex = pf->cx - pf->cwd/2;
    ey = pf->cy;

    miny = INT_MAX;
    maxy = -(INT_MAX-1);

    mini = INT_MAX;
    maxi = -(INT_MAX-1);

        fprintf(fp, "    gsave\n");

    if(ISCHUNK(pb->type)) {
        for(i=0;i<pb->cch.nch;i++) {
            pe = (ob*)pb->cch.ch[i];
            if(ISATOM(pe->type)) {
            }
            else {
                continue;
            }

            sx = pe->cgx + pe->cwd/2;
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


            sx = pe->cx + pe->cwd/2;
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
        sx = pb->cx + pb->cwd/2;
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
    case LS_DIRECT:
        ik = epsdraw_gather_direct(fp, xdir, xox, xoy, xu, pf, pb, xns);
        break;
    case LS_SQUARE:
        ik = epsdraw_gather_square(fp, xdir, xox, xoy, xu, pf, pb, xns, 0);
        break;
    case LS_SQUAREDOT:
        ik = epsdraw_gather_square(fp, xdir, xox, xoy, xu, pf, pb, xns, 1);
        break;
    case LS_ARC:
        Error("LS_ARC is not implemented yet.\n");
    case LS_MAN:
    default:
        ik = epsdraw_gather_man(fp, xdir, xox, xoy, xu, pf, pb, xns, 1);
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
    default:
    case LS_DIRECT:
        ik = epsdraw_scatter_direct(fp, xdir, xox, xoy, xu, pf, pb, xns);
        break;
#if 0
    case LS_SQUARE:
        ik = epsdraw_gather_square(fp, xdir, xox, xoy, xu, pf, pb, xns, 0);
        break;
    case LS_SQUAREDOT:
        ik = epsdraw_gather_square(fp, xdir, xox, xoy, xu, pf, pb, xns, 1);
        break;
    case LS_ARC:
        Error("LS_ARC is not implemented yet.\n");
    case LS_MAN:
    default:
        ik = epsdraw_gather_man(fp, xdir, xox, xoy, xu, pf, pb, xns, 1);
        break;
#endif
    }

out:
    return 0;
}


int
epsdraw_Xparen(FILE *fp, int xox, int xoy, ob *xu, ns *xns)
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

apply:

    fprintf(fp, "%% box xy %d,%d wh %dx%d\n", x1, y1, aw, ah);
    fprintf(fp, "gsave %% for box\n");

    if(bbox_mode) {
PP;
        epsdraw_bbox(fp, xu);
    }

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
            epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor, xu->cob.fillhatch);

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
    ob* pt;
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

    if(bbox_mode) {
PP;
        epsdraw_bbox(fp, xu);
    }

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
            epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor, xu->cob.fillhatch);

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

apply:

    fprintf(fp, "%% box xy %d,%d wh %dx%d\n", x1, y1, aw, ah);
    fprintf(fp, "gsave %% for box\n");

    if(bbox_mode) {
PP;
        epsdraw_bbox(fp, xu);
    }

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
            epsdraw_hatch(fp, aw, ah, xu->cob.fillcolor, xu->cob.fillhatch);

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

    if(bbox_mode) {
        epsdraw_bbox(fp, xu);
    }


#if 0
    epsdraw_plinearrowR(fp, xu->cob.sepcurdir, xox, xoy, 1000, xu, xns);
#endif

printf("%s: oid %d xox,xoy %d,%d\n", __func__, xu->oid, xox, xoy);
printf("    cx,cy %d,%d\n",
                    xu->cx, xu->cy);
printf("    x1,y1 %d,%d x2,y2 %d,%d\n", 
                    xu->cob.sepx1, xu->cob.sepy1, 
                    xu->cob.sepx2, xu->cob.sepy2);
printf("    gx1,y1 %d,%d gx2,y2 %d,%d\n", 
                    xu->cob.gsepx1, xu->cob.gsepy1, 
                    xu->cob.gsepx2, xu->cob.gsepy2);
printf("    xox,xoy+x1,y1 %d,%d gx2,y2 %d,%d\n", 
                    xox+xu->cob.sepx1, xoy+xu->cob.sepy1, 
                    xox+xu->cob.sepx2, xoy+xu->cob.sepy2);
printf("    xox,xoy+gx1,y1 %d,%d gx2,y2 %d,%d\n", 
                    xox+xu->cob.gsepx1, xoy+xu->cob.gsepy1, 
                    xox+xu->cob.gsepx2, xoy+xu->cob.gsepy2);

    fprintf(fp, "gsave\n");
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
                    xu->cob.outlinecolor, 0, 0, 0);
#endif

    epsdraw_Xseglinearrow(fp, xox, xoy,
                    xu->cob.gsepx1, xu->cob.gsepy1, 
                    xu->cob.gsepx2, xu->cob.gsepy2,
                    xu->cob.outlinetype,
                    xu->cob.outlinethick,
                    xu->cob.outlinecolor, 0, 0, 0);

    fprintf(fp, "grestore\n");

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
printf("%s: found B\n", __func__);
        *rx = qx;
        *ry = qy;
        rv = 1;
    }

    ik = bumpV(gx+gw/2, gy+gh/2, gx+gw/2, gy-gh/2, cx, cy, cr, &qx, &qy);
    if(ik) {
printf("%s: found R\n", __func__);
        *rx = qx;
        *ry = qy;
        rv = 2;
    }

    ik = bumpH(gx-gw/2, gy+gh/2, gx+gw/2, gy+gh/2, cx, cy, cr, &qx, &qy);
    if(ik) {
printf("%s: found T\n", __func__);
        *rx = qx;
        *ry = qy;
        rv = 3;
    }

    ik = bumpV(gx-gw/2, gy+gh/2, gx-gw/2, gy-gh/2, cx, cy, cr, &qx, &qy);
    if(ik) {
printf("%s: found L\n", __func__);
        *rx = qx;
        *ry = qy;
        rv = 4;
    }

    return rv;
}


int
epsdrawobj(FILE *fp, ob *u, int *xdir, int ox, int oy, ns *xns)
{
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

    fprintf(fp, "%%\n%% --- oid %d %s\n",
        u->oid, rassoc(cmd_ial, u->type));
    
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
            AR_NONE, AH_WIRE, AH_WIRE);
        fprintf(fp, "grestore\n");
    }

    if(u->type==CMD_COMMENT) {
        if(u->cob.carg1) {
            fprintf(fp, "%% user-comment: %s\n", u->cob.carg1);
        }
    }

    if(u->type==CMD_SCATTER) {
        epsdraw_scatter(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_GATHER) {
        epsdraw_gather(fp, *xdir, ox, oy, u, xns);
    }
#if 0
    if(u->type==CMD_LINK) {
        Yepsdraw_linearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_CLINE) {
        Yepsdraw_clinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_LINE) {
        Yepsdraw_linearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_ARROW) {
        Yepsdraw_linearrow(fp, *xdir, ox, oy, u, xns);
    }
#endif
    if((u->type==CMD_LINK) || (u->type==CMD_LINE) || (u->type==CMD_ARROW)) {
        Zepsdraw_linearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_CLINE) {
        Zepsdraw_clinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_WLINE) {
        Zepsdraw_wlinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_WARROW) {
        Zepsdraw_wlinearrow(fp, *xdir, ox, oy, u, xns);
    }
    if(u->type==CMD_BARROW) {
        Yepsdraw_blinearrow(fp, *xdir, ox, oy, u, xns);
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

    if(u->type==CMD_DMY1) {
#if 0
        fprintf(fp, "+   box at (%d,%d) width %d height %d rad %d \"%d\"\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, wd/8, u->oid);
#endif
    }
    if(u->type==CMD_DMY2) {
#if 0
        fprintf(fp, "+   box at (%d,%d) width %d height %d rad %d \"%d\"\n",
            ox+u->cx, oy+u->cy, u->crx-u->clx, u->cty-u->cby, wd/8, u->oid);
#endif
    }

    if(u->type==CMD_FORK) {
        changedraft(fp);
        drawrectcm(fp, u->cglx, u->cgby, u->cgrx, u->cgty, "fork");
        fflush(fp);
    }
    if(u->type==CMD_FORKEND) {
        changedraft(fp);
        drawrectcm(fp, u->cglx, u->cgby, u->cgrx, u->cgty, "      forkend");
        fflush(fp);
    }
    if(u->type==CMD_BRANCH) {
        changedraft(fp);
        drawrectcm(fp, u->cglx, u->cgby, u->cgrx, u->cgty, "branch");
        fflush(fp);
    }

    changenormal(fp); /* for faill safe */
    if(u->type==CMD_PAPER) {
        epsdraw_paper(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_CLOUD) {
        epsdraw_cloud(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_DRUM) {
        epsdraw_drum(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_BOX) {
        epsdraw_box(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_DOTS) {
        epsdraw_dots(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_CIRCLE) {
        epsdraw_circle(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_ELLIPSE) {
        epsdraw_ellipse(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_POLYGON) {
        epsdraw_polygon(fp, ox, oy, u, xns);
    }
    if(u->type==CMD_RULER) {
        epsdraw_ruler(fp, ox, oy, u, xns);
    }

    if(u->cob.ssar) {
        if(ISGLUE(u->type)) {
            epsdraw_sstrbgX(fp, u->gx, u->gy, u->wd, u->ht, u->cob.rotateval,
                0, 2, u->cob.textcolor, -1, u->cob.ssar);
        }
        else
        if(u->cob.fillhatch==HT_NONE) {
            epsdraw_sstrbgX(fp, u->gx, u->gy, u->wd, u->ht, u->cob.rotateval,
                0, 2, u->cob.textcolor, -1, u->cob.ssar);
        }
        else {
            epsdraw_sstrbgX(fp, u->gx, u->gy, u->wd, u->ht, u->cob.rotateval,
                0, 2, u->cob.textcolor, u->cob.textbgcolor, u->cob.ssar);
        }
    }

    if(u->cob.portstr || u->cob.boardstr) {
        int ik;
        int dx, dy;
        int qx, qy;
        int lax, lay;

        fprintf(fp, "      gsave\n");

        lax = (objunit/8)*cos((*xdir+90)*rf);
        lay = (objunit/8)*sin((*xdir+90)*rf);

        ik = bumpBB(u->gx, u->gy, u->wd, u->ht,
                u->gx, u->gy, *xdir+90, &qx, &qy);

        dx = qx - u->gx;
        dy = qy - u->gy;

        if(ik<=0) {
            goto skip_portboard;
        }

        if(u->cob.portstr) {
            fprintf(fp, "      %d %d %d (%s) rrshow\n",
                u->gx+dx+lax, u->gy+dy+lay, *xdir-90, u->cob.portstr);
        }
        if(u->cob.boardstr) {
            fprintf(fp, "      %d %d %d (%s) lrshow\n",
                u->gx-dx-lax, u->gy-dy-lay, *xdir-90, u->cob.boardstr);
        }
        fprintf(fp, "      grestore\n");
    }
skip_portboard:

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

    if(0*bbox_mode) {
        epsdraw_bbox(fp, xch);
    }

    if(0*draft_mode) {
        sprintf(msg, "chunk %d", xch->oid);

        fprintf(fp, "gsave\n");

#if 0
        changeground(fp);
        Xdrawrectcm(fp, xch->glx, xch->gby, xch->grx, xch->gty, msg);

        fprintf(fp, "  newpath %d %d 3 0 360 arc stroke\n", xch->x, xch->y);
#endif

        changedraft(fp);
        fprintf(fp, "    %d %d moveto (oid %d) show\n",
                        gox+xch->x,
                        goy+xch->y, xch->oid);

#if 0
        Xdrawrectcm(fp, gox+xch->x-xch->wd/2,
                        goy+xch->y-xch->ht/2,
                        gox+xch->x+xch->wd/2,
                        goy+xch->y+xch->ht/2, msg);
#endif
        fprintf(fp, "grestore\n");

        fflush(fp);
    }

#if 1
/*
    (xch->cob.fillhatch!=HT_NONE && xch->cob.fillcolor>=0) 
*/
    if(xch->cob.outlinethick>0)
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
            ik = epsdraw_box(fp,
                    gox,
                    goy, xch, xns);
        }

        fprintf(fp, "%% chunk itself END\n");
    }
#endif

    if(0*bbox_mode) {
        fprintf(fp, "gsave\n");
        changedraft(fp);
        drawCRrectG(fp, gox+xch->x, goy+xch->y, xch->wd, xch->ht,
            xch->cob.rotateval, objunit/10);
        fprintf(fp, "  gsave\n");
        fprintf(fp, "  %d %d translate 0 0 moveto %d rotate\n",
            gox+xch->x+xch->wd/2,
            goy+xch->y+xch->ht/2,
            45);
        fprintf(fp, "  %d %d moveto (chunk %d) show\n",
            objunit/10, 0, xch->oid);
        fprintf(fp, "  grestore\n");
        fprintf(fp, "grestore\n");
    }

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

PP;

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

            epsdraw_Xseglinearrow(fp, 0, 0,
                u->gsx, u->gsy, u->gex, u->gey,
                LT_MOUNTAIN, (int)((double)objunit*0.01),
                6, /*u->cob.outlinecolor, */
                AR_NONE, AH_WIRE, AH_WIRE);

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
#if 0
        if(u->type==CMD_CHUNK) 
#endif
        if(ISCHUNK(u->type)) {
            ik = epsdrawchunk(fp, u,
                    gox+xch->x+xch->ox,
                    goy+xch->y+xch->oy, xns);
        }
#if 0
        else
        if(u->type==CMD_BRANCH) {
            ob* bb;
            bb = u->cob.forkbranchbody;
            if(bb->drawed) {
                Echo("brach %d body %d skip already drawed\n",
                    u->oid, bb->oid);
                continue;
            }
            ik = epsdrawchunk(fp, bb,
                    xch->x+xch->ox+u->ox,
                    xch->y+xch->oy+u->oy, xns);
        }
#endif
        else {

Echo(" call obj drawing %d,%d\n", gox+xch->x+xch->ox, goy+xch->y+xch->oy);
            ik = epsdrawobj(fp, u, &xch->cch.dir,
                    gox+xch->x+xch->ox,
                    goy+xch->y+xch->oy, xns);
        }
    }

#if 0
 {
    int q;

    fprintf(fp, "gsave\n");
    fprintf(fp, " 0 0 1 setrgbcolor\n");
    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*)xch->cch.ch[i];
        q = (u->oid%10+1)*3;
        fprintf(fp, "newpath\n");
            fprintf(fp, "  %d %d moveto\n", u->gsx, u->gsy+q);
            fprintf(fp, "  %d %d lineto\n", u->gex, u->gey+q);
        fprintf(fp, "stroke\n");
    }
    fprintf(fp, "grestore\n");

 }
#endif

#if 0
 {
    int q;

    fprintf(fp, "gsave\n");
    fprintf(fp, " 0 0 1 setrgbcolor\n");
    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*)xch->cch.ch[i];
        q = (u->oid%10+1)*3;
        fprintf(fp, "newpath\n");
#if 0
            fprintf(fp, "  %d %d moveto\n", u->ox+u->sx, u->oy+u->gsy-q);
            fprintf(fp, "  %d %d lineto\n", u->ox+u->ex, u->oy+u->gey-q);
#endif
            fprintf(fp, "  %d %d moveto\n", xch->ox+u->sx, xch->oy+u->gsy-q);
            fprintf(fp, "  %d %d lineto\n", xch->ox+u->ex, xch->oy+u->gey-q);
        fprintf(fp, "stroke\n");
    }
    fprintf(fp, "grestore\n");

 }
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
    sprintf(tmp, "oid %d %s", u->oid, rassoc(cmd_ial, u->type));

    fprintf(fp, "  gsave\n");
    fprintf(fp, "    %d %d translate\n", cx, cy-fht);
    fprintf(fp, "    0 0 moveto\n");
    fprintf(fp, "    1 1 1 setrgbcolor\n");
    fprintf(fp, "    (%s) dup stringwidth pop dup\n", n);
    fprintf(fp, "    exch 0 rlineto 0 %d rlineto neg 0 rlineto closepath fill\n", fht);
    fprintf(fp, "    0.2 0.5 1 setrgbcolor\n");
    fprintf(fp, "    0 0 moveto show\n");
#if 0
    fprintf(fp, "    (%s) show\n", tmp);
#endif
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
    fprintf(fp, "  /Helvetica findfont %d scalefont setfont\n", def_textheight);
    epsdrawobj_oidl(fp, xch, &xdir, gox, goy, xns);

    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*)xch->cch.ch[i];
        if(!u) continue;

        q = _ns_find_name(xns, u, 0);
printf(" q %p\n", q);

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
%% x y s lshow -\n\
/lshow {\n\
    /s exch def /y exch def /x exch def\n\
    x y moveto s show\n\
} def\n\
");

    fprintf(fp, "\
%% x y s rshow -\n\
/rshow {\n\
    /s exch def /y exch def /x exch def\n\
    x s stringwidth pop sub y moveto s show\n\
} def\n\
");

    fprintf(fp, "\
%% x y s cshow -\n\
/cshow {\n\
    /s exch def /y exch def /x exch def\n\
    x s stringwidth pop 2 div sub y moveto s show\n\
} def\n\
");

    fprintf(fp, "\
%% x y r s lrshow -\n\
/lrshow {\n\
    gsave\n\
    /s exch def /r exch def /y exch def /x exch def\n\
    x y translate r rotate 0 0 moveto s show\n\
    grestore\n\
} def\n\
");

    fprintf(fp, "\
%% x y r s rrshow -\n\
/rrshow {\n\
    gsave\n\
    /s exch def /r exch def /y exch def /x exch def\n\
    x y translate r rotate s stringwidth pop neg 0 moveto s show\n\
    grestore\n\
} def\n\
");

    fprintf(fp, "\
%% x y r s crshow -\n\
/crshow {\n\
    gsave\n\
    /s exch def /r exch def /y exch def /x exch def\n\
    x y translate r rotate s stringwidth pop 2 div neg 0 moveto s show\n\
    grestore\n\
} def\n\
");

    fprintf(fp, "\
%% margined round box\n\
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
    stroke\n\
    grestore\n\
} def\n\
");

    fprintf(fp, "\
%% margined round box\n\
%% x y w h r mrbox -\n\
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
    int i;
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

    fprintf(fp, "%d %d translate\n", epsoutmargin, epsoutmargin);
    fprintf(fp, "%.3f %.3f scale\n", csc, csc);
    fprintf(fp, "%d %d translate\n", -xch->lx, -xch->by);

    changedraft(fp);
    fprintf(fp, "  %d setlinewidth\n", def_linethick);

    if(grid_mode) {
        /* grid */
        int x, y;
        int gp;

        gp = def_gridpitch;

#define GGH fprintf(fp, "  0.6 1.0 1.0 setrgbcolor %d setlinewidth\n", def_linethick/4);
#define GGL fprintf(fp, "  0.6 0.6 1.0 setrgbcolor %d setlinewidth\n", def_linethick/2);

        fprintf(fp, "%%\n%% grid\n");
        fprintf(fp, "gsave\n");
        fprintf(fp, "  %d setlinewidth\n", def_linethick/4);
        GGH;

        for(x=-def_gridrange;x<=def_gridrange;x++) {
            if(x%5==0) GGL;
            fprintf(fp, "  %d -%d moveto 0 %d rlineto stroke\n",
                x*gp, def_gridrange*gp, (2*def_gridrange)*gp);
            if(x%5==0) GGH;
        }
        for(y=-def_gridrange;y<=def_gridrange;y++) {
            if(y%5==0) GGL;
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
    }

    if(bbox_mode) {
        fprintf(fp, "%%\n%% whole bb g %d %d %d %d\n",
                        xch->glx, xch->gby, xch->grx, xch->gty);
        fprintf(fp,     "%% whole bb _ %d %d %d %d\n",
                        xch->lx, xch->by, xch->rx, xch->ty);
        fprintf(fp, "gsave\n");
        changeground(fp);
        fprintf(fp, "  %d setlinewidth\n", def_linedecothick);
#if 0
        drawrectcm(fp, xch->glx, xch->gby, xch->grx, xch->gty, "whole");
#endif
        drawrectcm(fp, xch->lx, xch->by, xch->rx, xch->ty, "whole");
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

