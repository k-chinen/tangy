
#ifndef QLIMIT
#define QLIMIT  (0.01)
#endif

int
linkchk_obj(ob *u, int dir)
{

P;

    return 0;
}

int
linkchk_chunk(ob *xch, ns *xns)
{
    int ik;
    int i;
    ob* u;
    int v;
    int minx, maxx;
    int miny, maxy;
    int c;
    int oldx, oldy;
    int lx, ly;
    int nx, ny;
    int zx, zy;
    int g;
    
    chas  regst;
    ch    bs;
    int   bsc=0;

    ob*   lob;
    ob*   f;
    ob*   b;

P;

    chas_init(&regst);
    cha_reset(&xch->cch);

    lob = NULL;
    c = 0;
    for(i=0;i<xch->cch.nch;i++) {
        c++;
        u = (ob*) xch->cch.ch[i];

        Echo("%d u %p oid %d %s lob %p\n",
            i, u, u->oid, rassoc(cmd_ial, u->type), lob);

        if(u->type==CMD_CHUNK) {
            ik = linkchk_chunk(u, xns);
        }
        else {
            ik = linkchk_obj(u, 0);
        }

        if(u->type==CMD_LINK) {
            if(lob) {
P;
                u->cob.linkfore = (void*)lob;
            }
        }
        if(lob && lob->type==CMD_LINK) {
P;
            lob->cob.linkback = (void*)u;
        }
        Echo("                f %p b %p\n", 
            (ob*)u->cob.linkfore, (ob*)u->cob.linkback);

        lob = u;
    }
    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*) xch->cch.ch[i];
        f = (ob*)u->cob.linkfore;
        b = (ob*)u->cob.linkback;
        Echo("    %d oid %d fork %p back %p\n",
            i, u->oid, f, b);
        Echo("%d oid %d fork %d back %d\n",
            i, u->oid, f? f->oid : -1, b ? b->oid: -1);
    }

    return 0;
}

int
linkchk(ob *xch, ns *xns)
{
    int ik;
P;
    ik = linkchk_chunk(xch, xns);
    return ik;
}
