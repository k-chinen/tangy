int jj(int zx, int zy, int degth, int ir, int iw,
    int *ax, int *ay, int *bx, int *by);

int _X_pbpoint(int xdir, int da, ob *u, int *px, int *py);

int _est_portproject(int inag, ob *u, int px, int py,
    int *ax, int *ay, int *bx, int *by,
    int *cx, int *cy, int *dx, int *dy);
int _est_boardproject(int inag, ob *u, int px, int py,
    int *ax, int *ay, int *bx, int *by,
    int *cx, int *cy, int *dx, int *dy);

int est_portboard4c(ns *xns, int gggtdir, ob *u, int xside,
    int *ax, int *ay, int *bx, int *by,
    int *cx, int *cy, int *dx, int *dy);
