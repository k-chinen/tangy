

#define SEG_SEP         ','
#define SEG_SEPS        ","

int
sdump(FILE *fp, char *msg, char *s)
{
    fprintf(fp, "%s: |", msg);
    while(*s) {
        if(*s=='\n') {
            fprintf(fp, "<%02X>", *s);
        }
        else {  
            fprintf(fp, "%c", *s);
        }
        s++;
    }
    fprintf(fp, "|\n");
    fflush(fp);
    return 0;
}

int
dellastcr(char *bf)
{
    char *p;
    p = bf;
    while(*p) {
        if(*p=='\n'||*p=='\r') {
            *p = '\0';
            break;
        }
        p++;
    }
    return 0;
}

int
trimdoublequote(char *s)
{
    char *os;
    os = s;
#if 0
    sdump(stderr, "b", os);
#endif
    if(*s=='"') {
        memmove(s, s+1, strlen(s+1));
        while(*s) {
            if(*s=='"') {
                *s = '\0';
                break;
            }
            s++;
        }
    }
#if 0
    sdump(stderr, "a", os);
#endif
    return 0;
}


int
delfirstchars(char *bf, int w)
{
    char *p;
    char *q;

#if 0
fEcho(stderr, "w %d\n", w);
  sdump(stderr, "delfirstchars b ", bf);
#endif

    p = bf+w;
    q = bf;
    while(*p) {
        *q++ = *p++;
    }
    *q = '\0';

#if 0
  sdump(stderr, "delfirstchars a ", bf);
#endif
    return 0;
}

int
xatoi(char *src)
{
    extern int objunit;
    char  *p, *q;
    char   mstr[BUFSIZ];
    double dm;
    int    m;

    p = src;
    q = mstr;

    while(*p &&
        ((*p>='0'&& *p<='9') || (*p=='.') || (*p=='-'))) {
        *q++ = *p++;
    }
    *q = '\0';

    if(mstr[0]) {
        dm = atof(mstr);
    }
    else {
        /* empty means 1 unit */
        m = (int)objunit;
        goto skip_m;
    }

#if 0
    printf("  p '%s'\n", p);
#endif

    if(strcasecmp(p,"u")==0) {
        dm *= objunit;
    }
    else
    if(strcasecmp(p,"inch")==0) {
        dm *= 216;
    }
    else
    if(strcasecmp(p,"cm")==0) {

        dm *= 28.34;
    }
    else
    if(strcasecmp(p,"mm")==0) {
        dm *= 2.834;
    }

    m = (int)(dm);

skip_m:
#if 0
    printf("    src '%s' -> m %d\n", src, m);
#endif
    
    return m;
}





int   lineno=-1;
int   cmdno=-1;


int
gotlabel(char *lab, int lsize, char *line)
{
    char *p, *q;
    char  tmp[BUFSIZ];
    int   c;
    int   found;

    found = 0;
    q = tmp;
    *q = '\0';

    p = line;
    while(*p && (*p==' '||*p=='\t')) {
        p++;
    }

    c = 0;
    while(*p && (isalnum(*p)||*p=='_') && c<lsize-1) {
        *q++ = *p++;
    }
    *q = '\0';
    if(*p==':') {
        found++;
    }

    if(found) {
        strcpy(lab, tmp);
#if 0
        Echo("label '%s'\n", lab);
#endif
    }

    return found;
}

char*
skiplabel(char *lab, int lsize, char *line)
{
    char *s;
    char *p, *q;
    char  tmp[BUFSIZ];
    int   c;
    int   found;

    found = 0;
    s = NULL;
    q = tmp;
    *q = '\0';

    p = line;
    while(*p && (*p==' '||*p=='\t')) {
        p++;
    }

    c = 0;
    s = p;
    while(*p && (isalnum(*p)||*p=='_') && c<lsize-1) {
        *q++ = *p++;
    }
    *q = '\0';
    if(*p==':') {
        found++;
        p++;
    }

    if(found) {
        strcpy(lab, tmp);
#if 1
        Echo("  label '%s'\n", lab);
#endif
        return p;
    }

    return s;
}

char*
parsetype(int *rot, char *line)
{
    char *p;
    int   ot;
    char  word[BUFSIZ];

    ot = -1;
    
    p = line;
    p = draw_wordW(p, word, BUFSIZ);

    ot = assoc(cmd_ial, word);

    if(ot<0) {
        Echo("ERROR falldown unknown '%s' [line %d]\n", word, lineno);
        exit(3);
    }

    *rot = ot;

    return p;
}

char*
parse_segop(char *sin, int dt, ob *nob)
{
    char   *p;
    char    tmp[BUFSIZ];
    int     v;
    double  dv;
    int     co;
    char    vstr[BUFSIZ];
    char   *f;
    char   *m;
    segop  *newop;
    char   *r;
    int     sop;
    int     top;

#if 0
    Echo("%s: sin '%s' dt %d nob %p\n", __func__, sin, dt, nob);
#endif
    r = NULL;
#if 0
    Echo("%s: dt %d\n", __func__, dt);
    Echo("sin '%s'\n", sin);
#endif

    co = 0;
    p = sin;
    tmp[0] = '\0';
    vstr[0] = '\0';

    if(dt==OA_THEN) {
    }
    else {
        p = draw_wordW(p, tmp, BUFSIZ);

        sop = assoc(objattr_ial, tmp);
        if(sop>=0) {
Warn("tmp '%s' seems not value\n", tmp);
            tmp[0] = '\0';
        }
        else {
            co = 1;

            strcpy(vstr, tmp);

            if(dt==OA_ARC||dt==OA_ARCN) {

                p = draw_wordW(p, tmp, BUFSIZ);

                top = assoc(objattr_ial, tmp);
                if(sop>=0) {
#if 0
Warn("tmp '%s' seems not value\n", tmp);
#endif
                    tmp[0] = '\0';
                }
                else {
                    co = 1;

                    strcat(vstr, SEG_SEPS);
                    strcat(vstr, tmp);
                }
            }

        }

    }

    newop = (segop*)malloc(sizeof(segop));
    if(!newop) {
E;
    }
    memset(newop, 0, sizeof(segop));

    newop->cmd = dt;
    newop->val = strdup(vstr);

#if 0
Echo("  push cmd %d val '%s'\n", newop->cmd, newop->val);
#endif
    varray_push(nob->cob.segopar, (void*)newop);

    nob->cob.originalshape++;

    if(co) {
        r = p;
    }
    else {
        r = NULL;
    }

#if 0
    Echo("%s: r %p\n", __func__, r);
#endif

    return r;
}

char *
parseobjattr(ob *rob, char *src)
{
    char *p;
    char  name[BUFSIZ];
    char  value[BUFSIZ];
    char  tmp[BUFSIZ];
    char *q;
    int   x;
    int   oak;
    sstr *ns;
    int   k1;
    int   v1;
    int   uc;
    int   gc;
    char *s;


    if(!src || *src=='\0') {
        return NULL;
    }

    uc = 0;
    gc = 0;
    
    k1 = -1;
    ns = NULL;
    p = src;

    if((*p>='0'&&*p<='9')||*p=='-') {
        q = tmp;
        if(*(p+1)>='0'&&*(p+1)<='9') {
            /* nothing */
        }
        else {
            goto skip_number;
        }
        while(*p && ((*p>='0'&&*p<='9')||*p=='-')) {
            *q++ = *p++;
        }
        *q = '\0';
        rob->cob.iarg1 = atoi(tmp); 
#if 0
Echo("iarg1 %d as '%s'\n", rob->cob.iarg1, tmp);
#endif
        goto out;
    }
skip_number:

    if(*p=='"') {
        q = tmp;
        p++;
        while(*p && (*p!='"')) {
            *q++ = *p++;
        }
        *q = '\0';
        p++;

        if(rob->cob.carg1) {
            rob->cob.carg2 = strdup(tmp);   
#if 0
            Echo("carg2 '%s' as '%s'\n", rob->cob.carg2, tmp);
#endif
        }
        else {
            rob->cob.carg1 = strdup(tmp);   
#if 0
            Echo("carg1 '%s' as '%s'\n", rob->cob.carg1, tmp);
#endif
        }

        if(tmp[0]) {
        }
            ns = (sstr*) malloc(sizeof(sstr));
            ns->ssval = strdup(tmp);
            ns->ssopt = 0;
            varray_push(rob->cob.ssar, ns);
#if 0
            varray_fprint(stdout, rob->cob.ssar);
#endif

        goto out;
    }

    p = draw_wordW(p, name, BUFSIZ);
    if(name[0]=='\0') {
        return NULL;
    }

    s = &name[0];
    if(*s==':') {
        char pstr[BUFSIZ];
        int  pn;

        strcpy(pstr, s+1);
        fprintf(stderr, "  pstr '%s'\n", pstr);
        if(!pstr[0]) {
            goto skip_note;
        }
    
        pn = assoc(pos_ial, pstr);
        if(pn<0) {
            goto skip_note;
        }

        fprintf(stderr, "   pn %d\n", pn);

        p = draw_wordW(p, value, BUFSIZ);
        
        fprintf(stderr, "   value '%s'\n", value);

        rob->cob.note[pn] = strdup(value);

        fprintf(stderr, "THEN pn %d '%s'\n", pn, rob->cob.note[pn]);
        
    }
skip_note:

    if(strcasecmp(name, "->")==0)     {
        rob->cob.arrowheadpart     = AR_FORE;
        rob->cob.arrowforeheadtype = AH_NORMAL;
        rob->cob.arrowbackheadtype = AH_NONE;
        goto out;
    }
    if(strcasecmp(name, "<-")==0)     {
        rob->cob.arrowheadpart     = AR_BACK;
        rob->cob.arrowforeheadtype = AH_NONE;
        rob->cob.arrowbackheadtype = AH_NORMAL;
        goto out;
    }
    if(strcasecmp(name, "<->")==0)     {
        rob->cob.arrowheadpart     = AR_BOTH;
        rob->cob.arrowforeheadtype = AH_NORMAL;
        rob->cob.arrowbackheadtype = AH_NORMAL;
        goto out;
    }

    oak = assoc(objattr_ial, name);
    if(oak<0) {
#if 0
        Echo("WARNING unknown attribue for object '%s' [line %d]\n", name, lineno);
#endif
        oak = assoc(pos_ial, name);
        if(oak<0) {
            Echo("WARNING unknown attribue for object/position '%s' [line %d]\n", name, lineno);
        }
    }
#if 0
    Echo("oak %d\n", oak);
#endif


    /* integer setting */
#define ISET(x,y) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        rob->cob.y = xatoi(value); \
        uc++; \
    }

#define LADD(x,y) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        /* fprintf(stderr, "value  |%s|\n", value); */\
        if(rob->cob.y==NULL) { \
            rob->cob.y = strdup(value); \
        } \
        else { \
            char atmp[BUFSIZ]; \
            strcpy(atmp, rob->cob.y); \
            strcat(atmp, ","); \
            strcat(atmp, value); \
            free(rob->cob.y); \
            rob->cob.y = strdup(atmp); \
        } \
        /*fprintf(stderr, "result |%s|\n", rob->cob.y);*/ \
        uc++; \
    }

#define SADD(x,y) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        if(value[0]){ trimdoublequote(value); } \
        /*fprintf(stderr, "value  |%s|\n", value);*/ \
        if(rob->cob.y==NULL) { \
            rob->cob.y = strdup(value); \
        } \
        else { \
            char atmp[BUFSIZ]; \
            strcpy(atmp, rob->cob.y); \
            strcat(atmp, ","); \
            strcat(atmp, value); \
            free(rob->cob.y); \
            rob->cob.y = strdup(atmp); \
        } \
        /*fprintf(stderr, "result |%s|\n", rob->cob.y);*/ \
        uc++; \
    }

    /* integer setting */
#define OISET(x,y) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        rob->y = xatoi(value); \
        uc++; \
    }

    /* integer setting */
#define GFSET(x,y) \
    if(oak==(x)) { \
        p = draw_wordW(p, value, BUFSIZ); \
        y = atof(value); \
        if(0) fprintf(stderr, "%s:%d GFSET %d '%s'->%.3f \n", \
                __FILE__, __LINE__, x, value, (double)y); \
        gc++; \
        uc++; \
    }

#define AISET(N,L,P)    \
    if(oak==(N)) {  \
        p = draw_wordW(p, value, BUFSIZ); \
        x = assoc((L), value);  \
        if(x>=0) {  \
            rob->cob.P = x; \
        }   \
        else { \
            Error("ignore value '%s' [line %d]\n", value, lineno); \
        } \
        uc++;   \
    }

#define AISETN(N,L,P)   \
    if(oak==(N)) {  \
        p = draw_wordW(p, value, BUFSIZ); \
        x = assoc((L), value);  \
        if(x>=0) {  \
            rob->cob.P = x; \
        }   \
        else { \
            rob->cob.P = atoi(value);   \
        } \
        uc++;   \
    }

#define AISETX(N,L,P,D) \
    if(oak==(N)) {  \
        p = draw_wordW(p, value, BUFSIZ); \
        x = assoc((L), value);  \
        if(x>=0) {  \
            rob->cob.P = x; \
        }   \
        else {  \
            rob->cob.P = (D);   \
        }   \
        uc++;   \
    }

    uc = 0;
    ISET(OA_LINECOLOR,      outlinecolor);
    ISET(OA_LINETHICK,      outlinethick);
    AISETN(OA_LINETYPE, linetype_ial, outlinetype);

    ISET(OA_FILLCOLOR,      fillcolor);
    AISETX(OA_FILLHATCH, hatchtype_ial, fillhatch, HT_XCROSSED);
    ISET(OA_HATCHTHICK,     hatchthick);
    ISET(OA_HATCHPITCH,     hatchpitch);

    ISET(OA_TEXTCOLOR,      textcolor);
    ISET(OA_TEXTBGCOLOR,    textbgcolor);

    AISET(OA_LINKSTYLE, ls_ial, linkstyle);
    ISET(OA_RAD,            rad);
    ISET(OA_PEAK,           polypeak);
    ISET(OA_PEAKROTATE,     polyrotate);
    ISET(OA_IMARGIN,        imargin);
    ISET(OA_GIMARGIN,       gimargin);
    ISET(OA_ROTATE,         rotateval);
    LADD(OA_DECO,           deco);

    AISET(OA_LANEORDER,     lo_ial, laneorder);
    ISET(OA_LANENUM,        lanenum);
    ISET(OA_LANEGAP,        lanegap);

    SADD(OA_PORT,           portstr);
    SADD(OA_STARBOARD,      boardstr);
    SADD(OA_BGSHAPE,        bgshape);

    OISET(OA_WIDTH,         wd);
    OISET(OA_HEIGHT,        ht);

    GFSET(OA_TEXTHEIGHTFACTOR,      textheightfactor);
    GFSET(OA_ARROWSIZEFACTOR,       arrowsizefactor);
    GFSET(OA_LINETHICKFACTOR,       linethickfactor);
    GFSET(OA_LINEDECOTHICKFACTOR,   linedecothickfactor);
    GFSET(OA_LINEDECOPITCHFACTOR,   linedecopitchfactor);
    GFSET(OA_HATCHTHICKFACTOR,      hatchthickfactor);
    GFSET(OA_HATCHPITCHFACTOR,      hatchpitchfactor);

#if 0
    if(strcasecmp(name, "linetype")==0)        {
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(linetype_ial, value);
        if(x>=0) {
            rob->cob.outlinetype = x;
        }
        else {
            Echo("WARNING strange linetype '%s' [line %d]\n", value, lineno);
            rob->cob.outlinetype = atoi(value);
        }
        uc++;
    }
    else
#endif
    /***
     ***
     ***/
    if(strcasecmp(name, "at")==0)     {
        rob->hasrel = 1;
        p = draw_wordW(p, value, BUFSIZ);
        rob->cob.aat = strdup(value);
        uc++;
    }
    else if(strcasecmp(name, "with")==0)     {
        rob->hasrel = 1;
        p = draw_wordW(p, value, BUFSIZ);
        rob->cob.awith = strdup(value);
        uc++;
    }
#if 0
    else if(strcasecmp(name, "from")==0)     {
        rob->hasrel = 1;
        p = draw_wordW(p, value, BUFSIZ);
        rob->cob.afrom = strdup(value);
        uc++;
        rob->cob.originalshape++;
    }
    else if(strcasecmp(name, "to")==0)     {
        rob->hasrel = 1;
        p = draw_wordW(p, value, BUFSIZ);
        rob->cob.ato   = strdup(value);
        uc++;
        rob->cob.originalshape++;
    }
#endif
    /***
     *** arrow head
     ***/
    else if(strcasecmp(name, "forehead")==0) {
        rob->cob.arrowheadpart |= AR_FORE;
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(arrowhead_ial, value);
        if(x>=0) {
            rob->cob.arrowforeheadtype = x;
        }
        else {
            rob->cob.arrowforeheadtype = atoi(value);
        }
        uc++;
    }
    else if(strcasecmp(name, "backhead")==0) {
        rob->cob.arrowheadpart |= AR_BACK;
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(arrowhead_ial, value);
        if(x>=0) {
            rob->cob.arrowbackheadtype = x;
        }
        else {
            rob->cob.arrowbackheadtype = atoi(value);
        }
        uc++;
    }
    else if(strcasecmp(name, "centhead")==0) {
        rob->cob.arrowheadpart |= AR_CENT;
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(arrowhead_ial, value);
        if(x>=0) {
            rob->cob.arrowcentheadtype = x;
        }
        else {
            rob->cob.arrowcentheadtype = atoi(value);
        }
        uc++;
    }
    else if(strcasecmp(name, "bothhead")==0) {
        rob->cob.arrowheadpart |= AR_BOTH;
        p = draw_wordW(p, value, BUFSIZ);
        x = assoc(arrowhead_ial, value);
        if(x>=0) {
            rob->cob.arrowforeheadtype = x;
            rob->cob.arrowbackheadtype = x;
        }
        else {
            rob->cob.arrowforeheadtype = atoi(value);
            rob->cob.arrowbackheadtype = atoi(value);
        }
        uc++;
    }
    else if((strcasecmp(name, "right")==0)  ||
            (strcasecmp(name, "left")==0)   ||
            (strcasecmp(name, "up")==0)     ||
            (strcasecmp(name, "down")==0)   ||

            (strcasecmp(name, "from")==0)   ||
            (strcasecmp(name, "to")==0)     ||

            (strcasecmp(name, "forward")==0)||
            (strcasecmp(name, "incdir")==0) ||
            (strcasecmp(name, "decdir")==0) ||
            (strcasecmp(name, "lturn")==0)  ||
            (strcasecmp(name, "rturn")==0)  ||
            (strcasecmp(name, "dir")==0)    ||
            (strcasecmp(name, "arc")==0)    ||
            (strcasecmp(name, "arcn")==0)   ||
            (strcasecmp(name, "join")==0)   ||
            (strcasecmp(name, "skip")==0)   ||

            (strcasecmp(name, "then")==0)   ) {
        char *j;
        int   dt;
        dt = assoc(objattr_ial, name);
        j = parse_segop(p, dt, rob);
        if(j) {
            p = j;
        }
        else {
#if 0
Warn("skip sub-command '%s' argument\n", name);
#endif
        }
        uc++;
    }
    else if((k1=assoc(pos_ial, name))>=0) {
        rob->cob.iarg1 = k1;        
        uc++;
    }
    else if(oak==OA_KEEPDIR) {
        rob->cob.keepdir = 1;
        uc++;
    }
    else if(oak==OA_MARKPITCH) {
        rob->cob.markpitch = 1;
        uc++;
    }
    else if(oak==OA_MARKNODE) {
        rob->cob.marknode = 1;
        uc++;
    }
    else if(oak==OA_MARKBB) {
        rob->cob.markbb = 1;
        uc++;
    }
    else if(oak==OA_MARKPATH) {
        rob->cob.markpath = 1;
        uc++;
    }
    else if(oak==OA_ARROWEVERY) {
        rob->cob.arrowevery = 1;
        uc++;
    }
    else if(oak==OA_ARROWEDGE) {
        rob->cob.arrowevery = 0;
        uc++;
    }
    else if(oak==OA_NOEXPAND) {
        rob->cob.noexpand = 1;
        uc++;
    }

    if(uc==0) {
        Echo("WARNING unknown object attribute '%s' [line %d]\n", name, lineno);
    }

    if(gc>0) {
        recalcsizeparam();
    }


#if 0
Echo("k1 %d\n", k1);
#endif

out:
    return p;
}


ob*
parseobj(char *p)
{
    int ot;
    ob* nob;
    char *u;
    char *oldp;

    oldp = p;

#if 1
    if(!*p) {
        return NULL;
    }
#endif

    u = parsetype(&ot, p);
    if(ot==-1) {
        Echo("ERROR unknow syntax/command or object\n");
        return NULL;
    }
    nob = newobj();
    if(!nob) {
        Echo("ERROR no memory\n");
        return NULL;
    }

    nob->type = ot;

    /* common */
    nob->cob.outlinecolor   = def_fgcolor;
    nob->cob.outlinethick   = def_linethick;
    nob->cob.hatchpitch     = def_hatchpitch;
    nob->cob.hatchthick     = def_hatchthick;
    nob->cob.fillcolor      = 0;
    nob->cob.fillhatch      = HT_NONE;
    nob->cob.textcolor      = def_fgcolor;
    nob->cob.textbgcolor    = def_bgcolor;
    nob->cob.arrowevery     = 0;

    nob->cob.wlinethick     = def_wlinethick;
    
    /* for automatic, set negative size as unsolved */
    nob->wd                 = -1;
    nob->ht                 = -1;
    nob->sizesolved         = 0;

    /* if need special procees, do that */
    switch(ot) {
    case CMD_CHUNK:
    case CMD_CHUNKOBJATTR:
Echo("CHECK CHUNK attr\n");
        nob->cob.fillhatch          = HT_NONE;
        nob->cob.fillcolor          = -1;
        nob->cob.outlinethick       = -1;
        break;
    case CMD_SAVE:
    case CMD_RESTORE:
    case CMD_PUSH:
    case CMD_POP:
    case CMD_BACK:
    case CMD_HBACK:
        nob->ignore = 1;
        break;

    case CMD_ARROW:
    case CMD_WARROW:
        nob->cob.arrowheadpart     = AR_FORE;
        nob->cob.arrowforeheadtype = AH_NORMAL;
        break;

    case CMD_LINE:
        nob->cob.keepdir = 0;
        break;
    case CMD_CLINE:
        nob->cob.keepdir = 1;
        break;

    case CMD_DIR:
    case CMD_INCDIR:
    case CMD_DECDIR:
    case CMD_LTURN:
    case CMD_RTURN:
    case CMD_DOWN:
    case CMD_RIGHT:
    case CMD_UP:
    case CMD_LEFT:
        nob->ignore = 1;
        break;

    default:
        break;
    }

    do {
        while(*u && (*u==' '||*u=='\t')) u++;

        if(*u=='#') {
            break;
        }
    
        u = parseobjattr(nob, u);
#if 0
Echo("u %p\n", u);
fflush(stdout);
#endif
        if(!u) break;
        if(*u=='\0') break;
    } while(1);


#define PV(x) if((x) && (x)->use) varray_fprint(stdout, (x));

#if 0
    dump_objattr(nob->cob);
#endif

    confirm_objattr(nob->cob);
    confirm_attr(nob);

#if 0
    dump_objattr(nob->cob);
#endif

#if 0
    Echo("; %s\n", oldp);
#endif
#if 0
    PV(nob->cob.ssar);
#endif
#if 0
    PV(nob->cob.segopar);
#endif

    if(nob->cob.originalshape) {
        segop *ent;
        ent = (segop*)varray_findlast(nob->cob.segopar);    
        if(ent) {
            if(ent->cmd!=OA_THEN) {
                /* add one more entry 'THEN' */
                ent = (segop*)malloc(sizeof(segop));
                if(ent) {
                    ent->cmd = OA_THEN;
                    ent->val = strdup("");
                    varray_push(nob->cob.segopar, ent);
                }
            }
        }
    }
#if 0
    PV(nob->cob.segopar);
#endif

#if 1
    /* XXX */
    if(nob->type==CMD_UNIT && nob->cob.iarg1>0) {
        objunit = nob->cob.iarg1; recalcsizeparam();
    }
#endif

    return nob;
}



static char ibuf[BUFSIZ]="";

int
getcmd(FILE *fp, char *ebuf, int elen)
{
    char  tmp[BUFSIZ];
    char *p, *q;
    char *ck;
    int   ol, nl;
    int   u;


    p = ibuf;
    q = ebuf;
    u = 0;
    while(1) {
        if(!*p) {
            ck = fgets(tmp, BUFSIZ, fp);
            lineno++;
            if(ck==NULL) {
#if 0
                fEcho(stderr, "EOF\n");
#endif
                return 1;
                break;
            }
            else {
                ol = strlen(ibuf);  
                nl = strlen(tmp);
                if(ol+nl+1>=BUFSIZ) {
                    Echo("ERROR too long line [line %d]\n", lineno);
                    exit(7);
                }
                strcat(ibuf, tmp);
                continue;
            }
        }
        if(*p=='\\' && *(p+1)=='\n') {
            p++;
            p++;
#if 0
            sdump(stderr, "k ibuf", ibuf);
            sdump(stderr, "k ebuf", ebuf);
#endif
        }
        else
        if(*p=='\\') {
            p++;
            *q++ = *p++;
        }
        else
        if(*p=='"') {
            u++;
            *q++ = *p++;
        }
        else {
            if(u%2==0&&*p==';') {
                p++;
                break;
            }
            else
            if(*p=='\n') {
                *q++ = *p++;
                break;
            }
            else {
                *q++ = *p++;
            }
        }
    }
    
    *q = '\0';

#if 0
  sdump(stderr, "b ibuf", ibuf);
  sdump(stderr, "b ebuf", ebuf);
#endif

    delfirstchars(ibuf, p-ibuf);

#if 0
  sdump(stderr, "a ibuf", ibuf);
  sdump(stderr, "a ebuf", ebuf);
#endif

    cmdno++;

    return 0;
}

typedef struct {
    char src[BUFSIZ];
    char dst[BUFSIZ];
} alias_cell;


int
aliasprintf(char *dst, int dlen, void* v, int opt)
{
    alias_cell *m;
    m = (alias_cell*)v;
    sprintf(dst, "(\"%s\"->\"%s\")", m->src, m->dst);
    return 0;
}

int
alias_add(varray_t *ar, char *xsrc, char *xdst)
{
    alias_cell *c;

    c = (alias_cell*)malloc(sizeof(alias_cell));
    memset(c, 0, sizeof(alias_cell));

    strcpy(c->src, xsrc);
    strcpy(c->dst, xdst);

    varray_push(ar, c);

    return 0;
}

char*
alias_find(varray_t *ar, char *xsrc)
{
    int i;
    int pos;
    alias_cell *c;

    pos = -1;
    for(i=0;i<ar->use;i++) {
        c = (alias_cell*)ar->slot[i];
        if(c && c->src[0] && strcasecmp(c->src, xsrc)==0) {
            pos = i;
        }
    }

    if(pos>=0) {
        c = (alias_cell*)ar->slot[pos];
        return c->dst;
    }

    return NULL;
}

int
try_alias(varray_t *ar, char *orig, char *ns, int nlen)
{
    char *u;
    char  first[BUFSIZ];
    char  second[BUFSIZ];
    char  third[BUFSIZ];
    char *d;
    char *q;

#if 0
Echo("%s: orig |%s|\n", __func__, orig);
#endif

    u = orig;
    u = draw_wordW(u, first, BUFSIZ);
    if(!first[0]) {
        return 0;
    }

#if 0
Echo("  first '%s'\n", first);
#endif
    if(strcasecmp(first, "alias")==0) {
        while(*u && (*u==' ' || *u=='\t')) {
            u++;
        }
        
        u = draw_wordW(u, second, BUFSIZ);

        while(*u && (*u==' ' || *u=='\t')) {
            u++;
        }

#if 0
Echo(" 1 *u '%c'\n", *u);
Echo(" 2 *u '%c'\n", *u);
Echo(" 3 *u '%c'\n", *u);
#endif
        
        q = third;
        if(*u=='"') {
            u++;
            while(*u && (*u!='"') ) {
                *q++ = *u++;
            }
            *q = '\0';
        }
        else {
            while(*u) {
                *q++ = *u++;
            }
            *q = '\0';
        }
#if 0
Echo("  second'%s'\n", second);
Echo("  third '%s'\n", third);
#endif
        alias_add(ar, second, third);
#if 0
        varray_fprint(stdout, ar);
#endif
        return 2;
    }
    else {

        d = alias_find(ar, first);
        if(d) {
            strcpy(ns, d);
            strcat(ns, " ");
            strcat(ns, u);

#if 0
Echo("%s: ns   |%s|\n", __func__, ns);
#endif
            return 1;
        }

        return 0;
    }

    return -1;
}

int
parse(FILE *fp, ob* ch0, ns *ns0) 
{
    ob*   curch;
    ns*   curns;
    char  line[BUFSIZ];
    char  label[BUFSIZ];
    char *p;
    int   ot;
    int   flag;
    int   do_rotate;
    int   ik;
    int   tmpv;
    char  tmps[BUFSIZ];

    varray_t *al;
    char  rep[BUFSIZ];

    al = varray_new();
    varray_entrysprintfunc(al, aliasprintf);


    curch = ch0;
    curns = ns0;
    ch0->cch.qns = ns0;

    lineno = 0;
    cmdno  = 0;

    while(1) {
        ik = getcmd(fp, line, BUFSIZ);
        if(ik) {
#if 0
Echo("getcmd ik %d\n", ik);
#endif
            break;
        }

#if 0
        Echo("\t\t\tcurns %p, curch %p\n", curns, curch);
        Echo("; %s", line);
#endif
    
        dellastcr(line);

#if 1
        Echo(";; %3d %3d; %s\n", cmdno, lineno, line);
#endif
        if(line[0]=='\0') {
            continue;
        }


        label[0] = '\0';
        p = line;
        p = skiplabel(label, BUFSIZ, p);
        while(*p && (*p==' ' || *p=='\t')) {
            p++;
        }
        if(!*p) {
            continue;
        }
        if(*p=='#') {
            continue;
        }
        if(*p=='\r'||*p=='\n') {
            continue;
        }

        /***
         *** try to replace by alias 
         ***/
        ik = try_alias(al, p, rep, BUFSIZ);
        if(ik==1) {
            int d;
            d = p - line;
            strcpy(p, rep);

#if 0
Echo("new line |%s|\n", line);
Echo("  line %p, p %p, p-line %d\n", line, p, (int)((char*)p-(char*)line));
#endif
        }
        if(ik==2) {
            continue;
        }

#if 0
Echo("new line |%s|\n", line);
sdump(stdout, "raw line ", line);
Echo("new p |%s|\n", p);
sdump(stdout, "raw p ", p);
#endif

        if(*p=='\\') {

        }
        else
        if(*p=='@') {
        }
        else
        /***
         ***/
        if(*p=='{') {
            ns* nns;

            nss_push(curns);
            nns = newnamespace_child(curns);
            if(label[0]) {
                nsaddns(curns, nns, label);
            }
            else {
                nsaddns(curns, nns, "nolabel");
            }
            curns = nns;
        }
        else
        if(*p=='}') {
            ns* ons;

            ons = nss_pop();
            curns = ons;
        }
        else
        /***
         ***/
        if(*p=='[') {
            ns* nns;
            ob* nch;
            ob* nob;

            chs_push(curch);
            nch = newchunk_child(curch);
            chadd(curch, nch);
            if(label[0]) {
                nsaddobj(curns, nch, label);
            }
#if 0
#endif
            curch = nch;

            nss_push(curns);
            nns = newnamespace_child(curns);
            if(label[0]) {
#if 0
                nsaddns(curns, nns, label);
#endif
                nsaddnsobj(curns, nns, nch, label);
            }
            else {
                nsaddnsobj(curns, nns, nch, "nolabel");
            }
            curns = nns;

            curch->cch.qns = nns;

        }
        else
        if(*p==']') {
            ns* ons;
            ob* och;

            ons = nss_pop();
            curns = ons;

            och = chs_pop();
            curch = och;
        }
        else
        /***
         ***/
        if(*p=='<') {
            ob* nch;

            chs_push(curch);
            nch = newchunk_child(curch);
            chadd(curch, nch);
            if(label[0]) {
                nsaddobj(curns, nch, label);
            }
            curch = nch;

            curch->cch.qns = curns;
        }
        else
        if(*p=='>') {
            ob* och;

            och = chs_pop();
            curch = och;
        }
        else {
    /*****
     *****
     ***** regular object parsing 
     *****
     *****/
            ob* nch;
            ob* och;
            ob* nob;
            ob *curfork;
            int kkh;

            nob = parseobj(p);
            if(!nob) {
Error("not found object '%s'\n", p);
                break;
            }

            chadd(curch, nob);

            if(label[0]) {
                nsaddobj(curns, nob, label);
            }

            if(nob->type==CMD_EXIT) {
                break;
            }

            /*** XXX ***/
            if(nob->type==CMD_CHUNKOBJATTR) {
                Echo("curch oid %d copy from nob oid %d\n",
                    curch->oid, nob->oid);
                curch->vob = nob->vob;
                chdel(curch, nob);

                goto skip_specialforms;
            }

#if 0
            /*** XXX ***/
            if(nob->type==CMD_CHUNKCHILDATTR) {
                Echo("curch oid %d copy from nob oid %d\n",
                    curch->oid, nob->oid);
                curch->vob = nob->vob;
                chdel(curch, nob);

                goto skip_specialforms;
            }
#endif

#if 0
            curfork = NULL;
            if(nob->cob.afrom) {
                curfork = ns_find_obj(ns0, nob->cob.afrom);
                if(!curfork) {
                    Echo("\tcurfork %p\n", curfork);
                    Echo("ERROR not found fork '%s'\n", nob->cob.afrom);
                }
                else {
                    kkh = curfork->cob.forkhasbranch;
                }
            }
#endif

#if 0
    Echo("\tcurfork %p oid %d\n", curfork, curfork->oid);
    curfork->cob.forkbranchmem[curfork->cob.forkhasbranch] = nob;
    curfork->cob.forkhasbranch++;

    kkh = curfork->cob.forkhasbranch;
#endif

            if(nob->type==CMD_FORK) {
                ob* nch;

#if 0
                ik = chdel(curch, nob);
#endif

#if 0
                chs_push(curch);
                nch = newchunk_child(curch);
                chadd(curch, nch);
                if(label[0]) {
                    nsaddobj(curns, nch, label);
                }
                curch = nch;
#endif
                chs_push(curch);
                curch = nob;

            }
            else
            if(nob->type==CMD_FORKEND) {
                ob* och;

                ik = chdel(curch, nob);

                /* pop twice: escape BRANCH and FORK */
                och = chs_pop();
                curch = och;
                och = chs_pop();
                curch = och;

                if(curfork) {
                    kkh = ++curfork->cob.forkhasend;
                }
            }
            else
            if(nob->type==CMD_BRANCH) {
                ob* nch;

Echo("  kkh %d\n", kkh);
                if(kkh==0) {
Echo("      -   push\n");
                    goto branch_push;
                }
Echo("      pop push\n");


                ik = chdel(curch, nob);

                och = chs_pop();
                curch = och;

                ik = chadd(curch, nob);

branch_push:


#if 0
                chs_push(curch);
                nch = newchunk_child(curch);
                chadd(curch, nch);
                if(label[0]) {
                    nsaddobj(curns, nch, label);
                }
                curch = nch;
#endif
        
                chs_push(curch);
                curch = nob;

                if(curfork) {
                    kkh = ++curfork->cob.forkhasbranch;
                }
                Echo("  new kkh %d\n", kkh);
            }

skip_specialforms:
            (void)0;
        }
    }
    
#if 0
    ob_dump(ch0);
    ns_dump(ns0);
#endif

    return 0;
}
