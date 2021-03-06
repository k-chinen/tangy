#ifndef _EPSDRAW_H_
#define _EPSDRAW_H_

int psescape(char *dst, int dlen, char *src);
int changethick(FILE *fp, int lth);
int changecolor(FILE *fp, int cn);
int epsdraw_bbox_cwh(FILE *fp, ob *xu);
int epsdraw_bbox_glbrt(FILE *fp, ob *xu);
int epsdraw_bbox_glbrtB(FILE *fp, ob *xu);
int epsdraw_bbox_glbrtR(FILE *fp, ob *xu);
int epsdraw_bbox_lbrt(FILE *fp, int xox, int xoy, ob *xu);
int epsdraw_bbox_lbrtR(FILE *fp, int xox, int xoy, ob *xu);
double epsdraw_arrowhead(FILE *fp, int atype, int xdir, int lc, int x, int y);
int epsdraw_Xarrowhead(FILE *fp, int atype, double xdir, int lc, double x, double y);
int epsdraw_seglineSEP(FILE *fp, int ltype, int lt, int lc, int x1, int y1, int x2, int y2);
int epsdraw_seglineTICK(FILE *fp, int ltype, int lt, int lc, int x1, int y1, int x2, int y2);
int epsdraw_seglineTICK2(FILE *fp, int ltype, int lt, int lc, int x1, int y1, int x2, int y2);
int epsdraw_seglineM(FILE *fp, int ltype, int lt, int lc, int x1, int y1, int x2, int y2);
int epsdraw_seglineW(FILE *fp, int ltype, int lt, int lc, int x1, int y1, int x2, int y2);
int epsdraw_segline(FILE *fp, int ltype, int lt, int lc, int x1, int y1, int x2, int y2);
int epsdraw_warrowhead(FILE *fp, int atype, int xdir, int lc, int x, int y);
int epsdraw_segwline_orig(FILE *fp, int wlt, int ltype, int lt, int lc, int x1, int y1, int x2, int y2);
int epsdraw_segwline(FILE *fp, int wlt, int ltype, int lt, int lc, int x1, int y1, int x2, int y2);
int epsdraw_Xseglinearrow(FILE *fp, int xox, int xoy, int x1, int y1, int x2, int y2, int xltype, int xlt, int xlc, int xahpart, int xahfore, int xahcent, int xahback);
int epsdraw_Xsegwlinearrow(FILE *fp, int xox, int xoy, int x1, int y1, int x2, int y2, int xwlt, int xltype, int xlt, int xlc, int xahpart, int xahfore, int xahcent, int xahback);
int epsdraw_seglinearrow(FILE *fp, int ydir, int xox, int xoy, int x1, int y1, int x2, int y2, ob *xu, ns *xns);
int epsdraw_segwlinearrow(FILE *fp, int ydir, int xox, int xoy, int x1, int y1, int x2, int y2, ob *xu, ns *xns);
int epsdraw_segblinearrow(FILE *fp, int ydir, int xox, int xoy, int x1, int y1, int x2, int y2, ob *xu, ns *xns);
int epsdraw_segarc(FILE *fp, int cdir, int xox, int xoy, int x1, int y1, int x2, int y2, int arcx, int arcy, int rad, int ang, ob *xu, ns *xns);
int epsdraw_segarcarrow(FILE *fp, int cdir, int xox, int xoy, int x1, int y1, int x2, int y2, int arcx, int arcy, int rad, int ang, ob *xu, ns *xns);
int epsdraw_segarcn(FILE *fp, int cdir, int xox, int xoy, int x1, int y1, int x2, int y2, int arcx, int arcy, int rad, int ang, ob *xu, ns *xns);
int epsdraw_segarcnarrow(FILE *fp, int cdir, int xox, int xoy, int x1, int y1, int x2, int y2, int arcx, int arcy, int rad, int ang, ob *xu, ns *xns);
int epsdraw_ping(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_pingpong(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_plinearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_plinearrowR(FILE *fp, int ydir, int xox, int xoy, int r, ob *xu, ns *xns);
int drawpath(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int drawsym(FILE *fp, double x, double y, double a, double pt, int c, int ty, double lax, double lay, double *cax, double *cay);
double solve_pitch(int ty);
int _drawpath_LT(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int drawpath_LT(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int QQ__solve_dir(ns *xns, ob *u, varray_t *opar, int X, int *ang);
int QQ__solve_fandt(ns *xns, ob *u, varray_t *opar, int X, int *sx, int *sy, int *ex, int *ey);
int Zepsdraw_curveselfarrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int Zepsdraw_curvearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int Zepsdraw_ulinearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int lineparam(double *k, double *u, double x1, double y1, double x2, double y2);
int linecrosspoint(double *rcx, double *rcy, int L1x1, int L1y1, int L1x2, int L1y2, int L2x1, int L2y1, int L2x2, int L2y2);
int Zepsdraw_wlinearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_blinearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_hatch(FILE *fp, int aw, int ah, int hc, int hty, int hp);
int _epsdraw_deco(FILE *fp, int xw, int xh, int xlc, int xfc, char *xcmd);
int epsdraw_deco(FILE *fp, int xw, int xh, int xlc, int xfc, char *xcmdlist);
int ss_strip(char *dst, int dlen, varray_t *ssar);
int ss_dump(FILE *ofp, varray_t *ssar);
#if 0
int epsdraw_sstrbgX(FILE *fp, int x, int y, int wd, int ht, int ro, int bgshape, int qbgmargin, int fgcolor, int bgcolor, varray_t *ssar);
#endif
#if 0
int epsdraw_sstrbgX(FILE *fp, int x, int y, int wd, int ht, int al, int of, int ro, int qof, int bgshape, int qbgmargin, int fgcolor, int bgcolor, varray_t *ssar, int ugjust);
#endif
int epsdraw_sstrbgX(FILE *fp, int x, int y, int wd, int ht,
    int al, int hof, int vof, int ro,
    int qhof, int qvof, 
    int bgshape, int qbgmargin,
    int fgcolor, int bgcolor, varray_t *ssar, int ugjust);
int epsdraw_circle(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_polygon(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_ellipse(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_drum(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_ruler(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_plane(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int _box_path(FILE *fp, int x1, int y1, int aw, int ah, int r, int op);
int epsdraw_box(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int mkpath_box(varray_t *sar, int wd, int ht, int rad);
int mkpath_Rbox(varray_t *sar, int wd, int ht, int rad);
int mkpath_circle(varray_t *sar, int wd, int ht, int rad);
int mkpath_Rcircle(varray_t *sar, int wd, int ht, int rad);
int mkpath_ellipseXXX(varray_t *sar, int wd, int ht, int rad, int dir);
int XXXmkpath_ellipse(varray_t *sar, int wd, int ht, int rad);
int epsdraw_dmyX(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int _cloud_shape(FILE *fp, double sx, double sy);
int _cloud_clip(FILE *fp, double sx, double sy);
int epsdraw_cloud(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_dots(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_paper(FILE *fp, int xox, int xoy, ob *xu, ns *xns);

int Xepsdraw_scatter(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_gather_man(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns, int dsdir);
int epsdraw_gather_direct(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns, int dsdir);
int epsdraw_gather_arc(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns, int drad, int dsdir);
int epsdraw_gather_arc1(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns, int drad, int dsdir);
int epsdraw_gather_square1(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns, int drad, int dsdir);
int epsdraw_gather_direct1(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns);
int epsdraw_scatter_direct1(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns);

int epsdraw_gather(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_scatter(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_Xparen(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_Xbrace(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_Xbracket(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_sep(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_mark(FILE *fp, int gx, int gy, int ty);
int bumpBB(int gx, int gy, int gw, int gh, int x, int y, int cth, int *rx, int *ry);
int XbumpBB(int gx, int gy, int gw, int gh, int x, int y, int cr, int *rx, int *ry);
int solvenotepos(int *rx, int *ry, int *ra, int *rj, ob *u, int pn, int ogap, int igap, int tht);
int epsdraw_portboard(FILE *fp, ns *xns, int xdir, ob *u);
int epsdraw_note(FILE *fp, ob *u);
int epsdrawobj(FILE *fp, ob *u, int *xdir, int ox, int oy, ns *xns);
int epsdrawchunk(FILE *fp, ob *xch, int gox, int goy, ns *xns);
int epsdrawobj_oidl(FILE *fp, ob *u, int *xdir, int ox, int oy, ns *xns);
int epsdrawchunk_oidl(FILE *fp, ob *xch, int gox, int goy, ns *xns);
int epsdrawobj_namel(FILE *fp, ob *u, char *n, int *xdir, int ox, int oy, ns *xns);
int epsdrawchunk_namel(FILE *fp, ob *xch, int gox, int goy, ns *xns);
int epsdraw_rulerframe(FILE *fp, ob *ych, int yx, int yy);
int printdefs(FILE *fp);
int epsdraw(FILE *fp, int wd, int ht, int crt, double csc, ob *xch, int x, int y, ns *xns);

#endif/*_EPSDRAW_H_*/
