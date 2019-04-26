
int
linkchkchunk(ob *xch, ns *xns)
{
    int ik;
    int i;
    ob* lobj;
    ob* cobj;
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

    ob*   lastfork;
    ob*   curfork;

#define RM(g) \
  ((g)->type==CMD_GATHER||(g)->type==CMD_SCATTER||(g)->type==CMD_THRU) 

P;

    lobj = NULL;
    for(i=0;i<xch->cch.nch;i++) {
        cobj = (ob*) xch->cch.ch[i];
        Echo("%d: oid %d type %d\n", i, cobj->oid, cobj->type);
    
        if(RM(cobj)) {
            cobj->cob.linkback = (void*)lobj;
            Echo("\tgather, scatter or thru\n");
        }
        if(lobj && RM(lobj)) {
            lobj->cob.linkfore = (void*)cobj;
            Echo("\tgather, scatter or thru\n");
        }

        if(cobj->type==CMD_CHUNK) {
            ik = linkchkchunk(cobj, xns);
        }

        lobj = cobj;
    }

    for(i=0;i<xch->cch.nch;i++) {
        cobj = (ob*) xch->cch.ch[i];
        if(RM(cobj)) {
            Echo("link %d: oid %d type %d back %d fore %d\n",
                i, cobj->oid, cobj->type,
                cobj->cob.linkback ?  ((ob*)cobj->cob.linkback)->oid : -1,
                cobj->cob.linkfore ?  ((ob*)cobj->cob.linkfore)->oid : -1
                );
        }
    }

#undef RM

    return 0;
}

int
linkchk(ob *xch, ns *xns)
{
    int ik;

    ik = linkchkchunk(xch, xns);

    return ik;
}
