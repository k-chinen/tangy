

#define changetext(xxx)     {(void)x;}
#define changetext2(xxx)    {(void)x;}
#define changetext3(xxx)    {(void)x;}

int
sstr_heightdepth(FILE *fp, int x, int y, int wd, int ht,
        int al, int exof, int ro, int qof,
        int bgshape, int qbgmargin, int fgcolor, int bgcolor,
        varray_t *ssar, int ugjust, int opt, int *rht, int *rdp)
{
    int   rv;
    int   py;
    int   pyb;
    int   gy;
    int   fht;
    int   n;
    int   rh;
    int   bgmargin;
    char  mcontall[BUFSIZ];

    int   u;
    int   lby, lty;
    int   lzy;
    int   maxht, maxdp;

    rv = -2;

    if(!ssar) {
        E;
        goto out;
    }

    ss_strip(mcontall, BUFSIZ, ssar);

#if 1
    Echo("%s: x,y %d,%d wd,ht %d,%d al %d exof %d ro %d qof %d bgshape %d qbgmargin %d '%s' ugj %d\n",
        __func__, x, y, wd, ht, al, exof, ro, qof, bgshape, qbgmargin, mcontall, ugjust);
#endif

    if(ssar->use<=0)  {
        rv = -1;
        goto skip_label;
    }

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

#if 0
    fprintf(fp, "%%  fht %d, ht %d, n %d, rh %d, py %d\n",
        fht, ht, n, rh, py);
#endif
    Echo("%%  fht %d, ht %d, n %d, rh %d, py %d\n",
        fht, ht, n, rh, py);
    Echo("%% # pyb %d bgmargin %d\n", pyb, bgmargin);

    Echo("ht %d use %d -> py %d\n", 
        ht, ssar->use, py);


    maxht = INT_MIN;
    maxdp = INT_MAX;

    lzy = (py*n)/2 - py + pyb;
    lby = lzy - pyb;
    lty = lby + fht;
    Echo("sshd lzy 0: %d %d %d \n", lby, lzy, lty);
    for(u=0;u<n;u++) {
        if(lby<maxdp) maxdp = lby;
        if(lty>maxht) maxht = lty;

        lzy += -py;
        lby = lzy - pyb;
        lty = lby + fht;
        if(u+1<n) {
            Echo("sshd lzy %d: %d %d %d\n", u+1, lby, lzy, lty);
        }
    }
    Echo("sshd mid maxdp %d maxht %d; opt %d\n", maxdp, maxht, opt);

    if(opt) {
        maxdp -= bgmargin;
        maxht += bgmargin;
    }

    Echo("sshd fin maxdp %d maxht %d\n", maxdp, maxht);

    *rht = maxht;
    *rdp = maxdp;

    rv = 0;

skip_label:
    (void)0;

out:
    return rv;
}
