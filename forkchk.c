
#ifndef QLIMIT
#define QLIMIT  (0.01)
#endif

int
fkchkobj(ob *u, int dir)
{

P;

    return 0;
}

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
Xfkchkchunk(ob *xch, ns *xns)
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

    ob*   lastfork;
    ob*   curfork;

#if 0
P;
#endif

#if 0

    chas_init(&regst);
    cha_reset(&xch->cch);

    lastfork = NULL;

    c = 0;
    for(i=0;i<xch->cch.nch;i++) {
        c++;
        u = (ob*) xch->cch.ch[i];

        curfork = NULL;

        if(u->type==CMD_CHUNK) {
            ik = fkchkchunk(u, xns);
            continue;
        }

        if(u->type==CMD_FORK) {
#if 0
            printf("oid %d FORK\n", u->oid);
#endif

            lastfork = u;
        }
        if(u->type==CMD_FORKEND) {
#if 0
            printf("oid %d FORKEND %s\n", u->oid, 
                (u->cob.afrom ?  u->cob.afrom : "*none"));
#endif

            if(!u->cob.afrom) {
                printf("ERROR no FROM specify\n");
                continue;
            }

            curfork = ns_find_obj(xns, u->cob.afrom);
            if(!curfork) {
#if 0
                printf("\tcurfork %p\n", curfork);
#endif
                printf("ERROR not fond fork '%s'\n", u->cob.afrom);
                continue;
            }

#if 0
            printf("\tcurfork %p oid %d\n", curfork, curfork->oid);
#endif
            curfork->cob.forkhasend++;

            lastfork = NULL;
        }
        if(u->type==CMD_BRANCH) {
#if 0
            printf("oid %d FORKEND %s\n", u->oid, 
                (u->cob.afrom ?  u->cob.afrom : "*none"));
#endif

            if(!u->cob.afrom) {
                printf("ERROR no FROM specify\n");
                continue;
            }

            curfork = ns_find_obj(xns, u->cob.afrom);
            if(!curfork) {
#if 0
                printf("\tcurfork %p\n", curfork);
#endif
                printf("ERROR not fond fork '%s'\n", u->cob.afrom);
                continue;
            }

#if 0
            printf("\tcurfork %p oid %d\n", curfork, curfork->oid);
#endif
            curfork->cob.forkhasbranch++;

        }

#if 0
        ik = fkchkobj(u, 0);
#endif
    }


#endif

    for(i=0;i<xch->cch.nch;i++) {
        u = (ob*) xch->cch.ch[i];
        if(u->type==CMD_FORK) {
#if 0
            printf("oid %d branch %d end %d\n",
                u->oid, u->cob.forkhasbranch, u->cob.forkhasend);
            if(u->cob.forkhasend<=0) {
                printf("WARNING no FORKEND oid %d\n", u->oid);
            }
            if(u->cob.forkhasbranch<=0) {
                printf("WARNING no BRANCH oid %d\n", u->oid);
            }
#endif
        }
        if(u->type==CMD_CHUNK) {
            ik = fkchkchunk(u, xns);
        }
    }

    return 0;
}

int
fkchk(ob *xch, ns *xns)
{
    int ik;

#if 0
    printf("--- fork consitency\n");
#endif
    ik = fkchkchunk(xch, xns);
#if 0
    printf("---\n");
#endif

    return ik;
}
