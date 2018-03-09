#ifndef _TANGY_A_H_
#define _TANGY_A_H_

char *skipwhite(char *p);
char *chomp(char *line);
int dellastcharif(char *src, int xch);
char *draw_wordDQ(char *src, char *dst, int wlen);
char *draw_wordW(char *src, char *dst, int wlen);
char *draw_word(char *src, char *dst, int wlen, int sep);
int confirm_objattr(struct obattr xoa);
int confirm_attr(ob *xo);
int dump_objattr(struct obattr xo);
int cha_fprint(FILE *fp, char *msg, ch *tg);
int cha_reset(ch *xch);
int cha_copy(ch *dst, ch *src);
int _ns_dump(ns *s, int w);
int ns_dump(ns *s);
int _ob_dump(ob *s, int w);
int ob_dump(ob *s);
int _ob_gdump(ob *s, int w);
int ob_gdump(ob *s);
int _ob_adump(ob *s, int w);
int ob_adump(ob *s);
int _ob_bldump(ob *s, int w);
int _ob_bgdump(ob *s, int w);
int _ob_cndump(ob *s, int w);
int ob_bldump(ob *s);
int ob_bgdump(ob *s);
int ob_cndump(ob *s);
int _printobjlist(FILE *fp, char *pre, ob *s, int w);
int printobjlist(FILE *fp, char *pre, ob *s);
ns *newnamespace();
ns *newnamespace_child(ns *pns);
int ss_sprintf(char *dst, int dlen, void *xv, int opt);
int segop_sprintf(char *dst, int dlen, void *xv, int opt);
int seg_sprintf(char *dst, int dlen, void *xv, int opt);
ob *newobj();
ob *cloneobj(ob *oo);
ob *newbox();
ob *newchunk();
ob *newchunk_child(ob *pch);
int chs_init();
int chs_dump();
int chs_push(ob *x);
ob *chs_pop();
int nss_init();
int nss_dump();
int nss_push(ns *x);
ns *nss_pop();
int chas_initN(chas *st, int n);
int chas_init(chas *st);
int chas_push(chas *st, ch *src);
int chas_pop(chas *st, ch *dst);
int chas_top(chas *st, ch *dst);
int chadd(ob *xch, ob *xob);
int chdel(ob *xch, ob *xob);
int nsaddobj(ns *xns, ob *xob, char *label);
int nsaddns(ns *xns, ns *xob, char *label);
int nsaddnsobj(ns *xns, ns *nns, ob *xob, char *label);
int splitdot(char *h, int hlen, char *r, int rlen, char *full);
ob *_ns_find_objP(ns *xns, char *xname, int *xx, int *xy);
ob *ns_find_obj(ns *xns, char *xname);
int ch_sprintf(char *dst, int dlen, void *xv, int opt);
int revch(ob *x, ch *ref, int *rx, int *ry);
int _ns_find_objposP(ns *xns, ob *b, char *xname, int pmode, int *rx, int *ry);
int _ns_find_objpos(ns *xns, char *xname, int *rx, int *ry, int pmode);
int ns_find_objpos(ns *xns, char *xname, int *rx, int *ry);
int ns_find_objposG(ns *xns, char *xname, int *rx, int *ry);
ob *_ns_find_objX(ns *xns, char *xname, int *ux, int *uy);
int _ns_find_objposX(ns *xns, char *xname, int *rx, int *ry, int pmode);
int ns_find_objposX(ns *xns, char *xname, int *rx, int *ry);
int ns_find_objposXG(ns *xns, char *xname, int *rx, int *ry);
char *_ns_find_name(ns *s, ob *xob, int w);
int recalcsizeparam();
nb_rec *nb_new();
nb_rec *nb_newpair(char *xk, ob *xt);
int nb_sprintf(char *dst, int dlen, void *xv, int opt);
ob *nb_find_ob(char *xkey);
nf_rec *nf_new();
nf_rec *nf_newfile(char *xfn);
int nf_sprintf(char *dst, int dlen, void *xv, int opt);
int nf_load(FILE *fp);
int nf_loadfn(char *fn);
int nf_registfilename(char *fn);
int nb_regist(char *xkey, ob *xob);
int notefile_load();
int notefile_setup();
seg *seg_new();
int try_regsegclose(varray_t *segar);
int try_regline(varray_t *segar, int x1, int y1, int x2, int y2);
int try_regsegmove(varray_t *segar, int x1, int y1);
int try_regsegrmove(varray_t *segar, int x1, int y1);
int try_regsegrline(varray_t *segar, int x1, int y1);
int try_regsegforward(varray_t *segar, int x1, int y1);
int try_regsegarc(varray_t *segar, int rad, int ang);
int try_regsegarcn(varray_t *segar, int rad, int ang);
int sdump(FILE *fp, char *msg, char *s);
int dellastcr(char *bf);
int trimdoublequote(char *s);
int delfirstchars(char *bf, int w);
int xatoi(char *src);
int gotlabel(char *lab, int lsize, char *line);
char *skiplabel(char *lab, int lsize, char *line);
char *parsetype(int *rot, char *line);
char *parse_segop(char *sin, int dt, ob *nob);
char *parseobjattr(ob *rob, char *src);
ob *parseobj(char *p);
int getcmd(FILE *fp, char *ebuf, int elen);
int aliasprintf(char *dst, int dlen, void *v, int opt);
int alias_add(varray_t *ar, char *xsrc, char *xdst);
char *alias_find(varray_t *ar, char *xsrc);
int try_alias(varray_t *ar, char *orig, char *ns, int nlen);
int parse(FILE *fp, ob *ch0, ns *ns0);
int dirnormalize(int xxdir);
int rewindcenter(ob *u, int qdir, int *rx, int *ry);
int applywith(ob *u, char *xpos, int *rx, int *ry);
int find_from(ob *xob, int *risx, int *risy);
int find_to_first(ob *xob, int *riex, int *riey);
int find_to_last(ob *xob, int *riex, int *riey);
int est_seg(ns *xns, ob *u, varray_t *opar, varray_t *segar, int kp, int *zdir, int *rlx, int *rby, int *rrx, int *rty, int *rfx, int *rfy);
int takelastobjpos(ob *lob, int pos, int kp, int *nx, int *ny, int *dir);
int OP_GDUMP(const char *pre, char *mid, ob *u);
int OP_LDUMP(const char *pre, char *mid, ob *u);
int insboxpath(varray_t *xar, int xwd, int xht);
int EST_curveself(FILE *fp, ob *xu, ns *xns, int *_lx, int *_by, int *_rx, int *_ty);
int EST_curve(FILE *fp, ob *xu, ns *xns, int *_lx, int *_by, int *_rx, int *_ty);
int putobj(ob *u, ns *xns, int *gdir);
int fitobj_LBRT(ob *u, int xxdir, int *x, int *y, int *fx, int *fy, ns *xns);
int fitobj_wdht(ob *u, int xxdir, int *x, int *y, ns *xns);
int eval_dir(ob *u, int *xdir);
int bumpH(int x1, int y1, int x2, int y2, int x, int y, int cdir, int *rgx, int *rgy);
int bumpV(int x1, int y1, int x2, int y2, int x, int y, int cdir, int *rgx, int *rgy);
int bump(int x1, int y1, int x2, int y2, int x, int y, int cdir, int *rgx, int *rgy);
int expand_paren(int bx, int by, int w, int h, int ox, int oy, ob *u);
int expand_sep(int bx, int by, int w, int h, int ox, int oy, ob *u);
int _putchunk_lane(ob *xch, int *rx, int *ry, ns *xns);
int _putchunk(ob *xch, int *x, int *y, ns *xns);
int putchunk(ob *xch, int *x, int *y, ns *xns);
int put(ob *xch, int *x, int *y, ns *xns);
int finalizeobj(ob *u, int *xdir, int ox, int oy, ns *xns);
int finalizechunk(ob *xch, int gox, int goy, ns *xns);
int finalize(ob *xch, int x, int y, ns *xns);
int picdraw_arrowhead(int atype, int xdir, int x, int y);
int picdraw_linearrow(int xox, int xoy, ob *xu, ns *xns);
int picdraw_line(int xox, int xoy, ob *xu, ns *xns);
int picdrawobj(ob *u, int *xdir, int ox, int oy, ns *xns);
int picdrawchunk(ob *xch, int gox, int goy, ns *xns);
int picdraw(ob *xch, int x, int y, ns *xns);
int psescape(char *dst, int dlen, char *src);
int changethick(FILE *fp, int lth);
int changecolor(FILE *fp, int cn);
static int changehot(FILE *fp);
static int changeground(FILE *fp);
static int changebbox(FILE *fp);
static int changedraft(FILE *fp);
static int changetext(FILE *fp);
static int changetext2(FILE *fp);
static int changetext3(FILE *fp);
static int changenormal(FILE *fp);
static int drawCRrectMJ(FILE *fp, int x1, int y1, int wd, int ht, int ro, char *msg, int sj);
static int drawCRrectM(FILE *fp, int x1, int y1, int wd, int ht, int ro, char *msg);
static int drawCRrect(FILE *fp, int x1, int y1, int wd, int ht, int ro);
static int drawCRrectskel(FILE *fp, int x1, int y1, int wd, int ht, int ro);
static int drawCRrectskel2(FILE *fp, int x1, int y1, int wd, int ht, int ro);
static int XdrawCRrectskel2(FILE *fp, int x1, int y1, int wd, int ht, int ro);
static int drawCrect(FILE *fp, int x1, int y1, int wd, int ht);
static int drawrect(FILE *fp, int x1, int y1, int x2, int y2);
static int drawrectcm(FILE *fp, int x1, int y1, int x2, int y2, char *cm);
static int Xdrawrectcm(FILE *fp, int x1, int y1, int x2, int y2, char *cm);
static int drawCRrectG(FILE *fp, int x1, int y1, int wd, int ht, int ro, int gl);
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
static int fitarc(FILE *fp, int x1, int y1, int x2, int y2, int ph);
int epsdraw_segarc(FILE *fp, int cdir, int xox, int xoy, int x1, int y1, int x2, int y2, int arcx, int arcy, int rad, int ang, ob *xu, ns *xns);
int epsdraw_segarcarrow(FILE *fp, int cdir, int xox, int xoy, int x1, int y1, int x2, int y2, int arcx, int arcy, int rad, int ang, ob *xu, ns *xns);
int epsdraw_segarcn(FILE *fp, int cdir, int xox, int xoy, int x1, int y1, int x2, int y2, int arcx, int arcy, int rad, int ang, ob *xu, ns *xns);
int epsdraw_segarcnarrow(FILE *fp, int cdir, int xox, int xoy, int x1, int y1, int x2, int y2, int arcx, int arcy, int rad, int ang, ob *xu, ns *xns);
int epsdraw_ping(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_pingpong(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_plinearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_plinearrowR(FILE *fp, int ydir, int xox, int xoy, int r, ob *xu, ns *xns);
int _line_pathMM(FILE *fp, int ydir, int xox, int xoy, ob *xu, int MM, ns *xns, int f_new, int f_close);
int _line_patharrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int symdraw(FILE *fp, double x, double y, double a, double pt, int c, int ty, double lax, double lay, double *cax, double *cay);
double solve_pitch(int ty);
int __line_deco2(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int _line_deco2(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int QQ__solve_dir(ns *xns, ob *u, varray_t *opar, int X, int *ang);
int QQ__solve_fandt(ns *xns, ob *u, varray_t *opar, int X, int *sx, int *sy, int *ex, int *ey);
int Zepsdraw_curveselfarrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int Zepsdraw_curvearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int Zepsdraw_clinearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int lineparam(double *k, double *u, double x1, double y1, double x2, double y2);
int linecrosspoint(double *rcx, double *rcy, int L1x1, int L1y1, int L1x2, int L1y2, int L2x1, int L2y1, int L2x2, int L2y2);
int Zepsdraw_wlinearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_blinearrow(FILE *fp, int ydir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_hatch(FILE *fp, int aw, int ah, int hc, int hty, int hp);
int _epsdraw_deco(FILE *fp, int xw, int xh, int xlc, int xfc, char *xcmd);
int epsdraw_deco(FILE *fp, int xw, int xh, int xlc, int xfc, char *xcmdlist);
int ss_strip(char *dst, int dlen, varray_t *ssar);
int ss_dump(FILE *ofp, varray_t *ssar);
int epsdraw_sstrbgX(FILE *fp, int x, int y, int wd, int ht, int ro, int bgshape, int qbgmargin, int fgcolor, int bgcolor, varray_t *ssar);
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
int epsdraw_gather_square(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns, int drad, int dsdir);
int epsdraw_gather_direct(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns);
int epsdraw_gather(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_scatter_direct(FILE *fp, int xdir, int xox, int xoy, ob *xu, ob *pf, ob *pb, ns *xns);
int epsdraw_scatter(FILE *fp, int xdir, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_Xparen(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_Xbrace(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_Xbracket(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_sep(FILE *fp, int xox, int xoy, ob *xu, ns *xns);
int epsdraw_mark(FILE *fp, int gx, int gy, int ty);
int bumpBB(int gx, int gy, int gw, int gh, int x, int y, int cth, int *rx, int *ry);
int XbumpBB(int gx, int gy, int gw, int gh, int x, int y, int cr, int *rx, int *ry);
int solvenotepos(int *rx, int *ry, int *ra, int *rj, ob *u, int pn, int ogap, int igap, int tht);
int epsdraw_portboard(FILE *fp, int xdir, ob *u);
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
#endif/*_TANGY_A_H_*/
