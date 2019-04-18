
int
fkchkchunk(ob *xch, ns *xns)
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

P;

    lobj = NULL;
    for(i=0;i<xch->cch.nch;i++) {
        cobj = (ob*) xch->cch.ch[i];
        Echo("%d: oid %d type %d\n", i, cobj->oid, cobj->type);
    
        if(cobj->type==CMD_GATHER||cobj->type==CMD_SCATTER) {
            cobj->cob.linkback = (void*)lobj;
            Echo("\tgather or scatter\n");
        }
        if(lobj && (lobj->type==CMD_GATHER||lobj->type==CMD_SCATTER)) {
            Echo("\tgather or scatter\n");
            lobj->cob.linkfore = (void*)cobj;
        }

        if(cobj->type==CMD_CHUNK) {
            ik = fkchkchunk(cobj, xns);
        }

        lobj = cobj;
    }

    for(i=0;i<xch->cch.nch;i++) {
        cobj = (ob*) xch->cch.ch[i];
        if(cobj->type==CMD_GATHER||cobj->type==CMD_SCATTER) {
            Echo("link %d: oid %d type %d back %d fore %d\n",
                i, cobj->oid, cobj->type,
                cobj->cob.linkback ?  ((ob*)cobj->cob.linkback)->oid : -1,
                cobj->cob.linkfore ?  ((ob*)cobj->cob.linkfore)->oid : -1
                );
        }
    }

    return 0;
}

int
fkchk(ob *xch, ns *xns)
{
    int ik;

    ik = fkchkchunk(xch, xns);

    return ik;
}
