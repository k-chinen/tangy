/*** notefile.c ***/

/* note binding */
typedef struct {
    char *nbkey;
    ob   *nbtarget;
} nb_rec;


varray_t *gnotebindlist = NULL;

nb_rec*
nb_new()
{
    nb_rec *rv;
    rv = (nb_rec*)malloc(sizeof(nb_rec));
    if(!rv) {
        Error("no memory\n");
        exit(3);
    }
    memset(rv, 0, sizeof(nb_rec));
    return rv;
}

nb_rec*
nb_newpair(char *xk, ob *xt)
{
    nb_rec *rv;
    rv = nb_new();
    if(!rv) {
        Error("no memory\n");
        exit(3);
    }
    rv->nbkey    = strdup(xk);
    rv->nbtarget = xt;
    return rv;
}


int
nb_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    nb_rec *nb;

    nb = (nb_rec*)xv;
    ik = sprintf(dst, "(%s,%p)",
            nb->nbkey, nb->nbtarget);

    return ik;
}

ob*
nb_find_ob(char *xkey)
{
    int i;
    nb_rec *nb;

    for(i=0;i<gnotebindlist->use;i++) {
        nb = (nb_rec*)gnotebindlist->slot[i];
        if(strcasecmp(nb->nbkey, xkey)==0) {
            return nb->nbtarget;
        }
    }

    return NULL;
}



/* notefile */
typedef struct {
    char *fn;
    int   loadcount;
} nf_rec;


nf_rec*
nf_new()
{
    nf_rec *rv;
    rv = (nf_rec*)malloc(sizeof(nf_rec));
    if(!rv) {
        Error("no memory\n");
        exit(3);
    }
    memset(rv, 0, sizeof(nf_rec));
    return rv;
}

nf_rec*
nf_newfile(char *xfn)
{
    nf_rec *rv;
    rv = nf_new();
    if(!rv) {
        Error("no memory\n");
        exit(3);
    }
    rv->fn = strdup(xfn);
    return rv;
}

int
nf_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    nf_rec *nf;

    nf = (nf_rec*)xv;
    ik = sprintf(dst, "(%s,%d)",
            nf->fn, nf->loadcount);

    return ik;
}

varray_t *gnotefilelist = NULL;



#ifndef DEF_SEP
#define DEF_SEP     (':')
#endif

#ifndef NT_MAXFIELD
#define NT_MAXFIELD (100)
#endif

int
nf_load(FILE *fp)
{
    char *key;
    char  line[BUFSIZ];
    char  back[BUFSIZ];
    char  tmp[BUFSIZ];
    ob   *tg;
    int   phase;
    int   lno;
    char *p;
    char *q;
    int   sep;
    char *f[NT_MAXFIELD];
    char *v[NT_MAXFIELD];
    int   fmax=NT_MAXFIELD;
    int   fuse;
    int   fno;
    char *ck;
    char *oldp;
    char *u;
    int   pn;

    int   i;

    for(i=0;i<fmax;i++) {
        f[i] = NULL;
    }

    sep = DEF_SEP;
    phase = 0;
    lno = 0;
    while(1) {
        tg = NULL;
        ck = fgets(line, BUFSIZ, fp);
        if(!ck) {
            break;
        }
        lno++;
#if 0
        Echo(";%d %3d %s", phase, lno, line);
#endif

        if(!line[0]) {
            continue;
        }

        chomp(line);

        p = line;
        p = skipwhite(p);
        if(!*p) {
            continue;
        }   
        if(*p=='#') {
            continue;
        }   

        if(phase==0) {
            if(strncasecmp(p, "tab(", 4)==0) {
                p = p+4;
                q = tmp;
                while(*p&&*p!=')') {
                    *q++ = *p++;
                }
                *q = '\0';
#if 0
                Echo("sep '%s'\n", tmp);
#endif
                if(tmp[0]) {
                    sep = tmp[0];
                }
            }
            if(strncasecmp(p, "format", 6)==0) {
                p = p+6;
                if(*p==sep) {
                    phase++;

                    p++;
                    fno = 0;
                    while(*p) {
                        p = draw_word(p, tmp, BUFSIZ, sep);
                        if(!tmp[0]) {
                            break;
                        }

                        if(fno<fmax) { 
                            f[fno] = strdup(tmp);
                            fno++;
                        }
                    }
                    fuse = fno;

#if 0
                    for(i=0;i<fuse;i++) {
                        Echo(" %3d: %-16s\n", i, f[i]);
                    }
#endif

                }
            }
            continue;
        }
        if(phase>0) {
            strcpy(back, line);

            for(i=0;i<fmax;i++) {
                v[i] = NULL;
            }
            p = back;
            p = skipwhite(p);
        
            oldp = p;
#if 0
            Echo("       back %p\n", back);
            Echo("       oldp %p\n", oldp);
#endif

            fno = 0;
            while(*p) {
                if(*p==sep) {
#if 0
                    Echo("  fno %3d p %p *p %c\n", fno, p, *p);
#endif
                    v[fno] = oldp;
                    *p = '\0';
                    fno++;
                    p++;
                    oldp = p;
                }
                else {
                    p++;
                }
            }
            if(oldp) {
                    v[fno] = oldp;
            }

#if 0
            for(i=0;i<fuse;i++) {
                Echo("v %3d '%s'\n", i, v[i]);
            }
#endif

            key = NULL;
            for(i=0;i<fuse;i++) {
                if(strcasecmp(f[i],"key")==0) {
                    key = v[i];
                }
            }

            if(!key) {
                continue;
            }

#if 0
            Echo("key '%s'\n", key);
#endif
            tg = nb_find_ob(key);
#if 0
            Echo("tg %p\n", tg);
#endif

            if(!tg) {
                continue;
            }
            
            for(i=0;i<fuse;i++) {
                pn = assoc(pos_ial, f[i]);
#if 0
                Echo(" i %3d pn %3d 0%03d %3xH %s\n", i, pn, pn, pn, f[i]);
#endif
                if(pn==PO_KEY) {
                    continue;
                }
                if(tg->cob.note[pn]) {
                    Info("overwrite note %s:%s\n", key, f[i]);
                }
                tg->cob.note[pn] = strdup(v[i]);
            }

        }
    }


    for(i=0;i<fmax;i++) {
        if(f[i]) free(f[i]);
        f[i] = NULL;
    }

    return 0;
}

int
nf_loadfn(char *fn)
{
    int   ik;
    FILE *fp;

    fp = fopen(fn, "r");
    if(!fp) {
        Error("cannot open notefile '%s'\n", fn);
        return -1;
    }

    ik = nf_load(fp);

    return ik;
}

int
nf_registfilename(char *fn)
{
    nf_rec *ne;

    ne = nf_newfile(fn);
    if(ne) {
        varray_push(gnotefilelist, ne);
        return 0;
    }

    return -1;
}

int
nb_regist(char *xkey, ob *xob)
{
    nb_rec *nb;
    nb = nb_newpair(xkey, xob);
    if(nb) {
        varray_push(gnotebindlist, nb);
        return 0;
    }

    return -1;
}

int
notefile_load()
{
    int i;
    nf_rec *nf;
    int savedflag;

    savedflag = _t_;
#if 0
    _t_ = 1;
#endif

    for(i=0;i<gnotefilelist->use;i++) {
        nf = (nf_rec*)gnotefilelist->slot[i];
        Echo("load '%s'\n", nf->fn);
        if(nf->loadcount==0) {
            nf_loadfn(nf->fn);
            nf->loadcount++;
        }
    }

    _t_ = savedflag;

    return 0;
}

int
notefile_setup()
{
    gnotefilelist = varray_new();
    varray_entrysprintfunc(gnotefilelist, nf_sprintf);
    gnotebindlist = varray_new();
    varray_entrysprintfunc(gnotebindlist, nb_sprintf);

    return 0;
}
