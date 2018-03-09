#ifndef _TANGY_XNS_H_
#define _TANGY_XNS_H_

extern int dumptabstop;
extern int dumplabel;

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

#endif/*_TANGY_XNS_H_*/
