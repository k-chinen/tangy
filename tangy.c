/*
 * tangy second trial since 2016 Aug 
 * K.Chinen
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "alist.h"
#include "varray.h"
#include "color.h"

#include "qbb.h"
#include "geom.h"
#include "obj.h"
#include "gv.h"
#include "chas.h"
#include "notefile.h"
#include "font.h"
#include "seg.h"
#include "epsdraw.h"
#include "a.h"

int linkchk(ob *xch, ns *xns);

int set_eoutcharset(char *nenc);
char *get_eoutcharset();
extern char *get_innercharset();
extern int   set_eincharset(char*);
extern char *get_eincharset();

int     nodraw   = 0;
char   *outfile  = "out.eps";
int     canvaswd = (int)( 8.27 * 72);
int     canvasht = (int)(11.69 * 72);
int     canvasrt = 0;
double  canvassc = 0.01;

int
print_usage()
{
    printf("tangy - picture generator according to special language\n");
    printf("usage: tangy [option] [files]\n");
    printf("option:     # unit of length is bigpoint = 1/72inch = 0.3527mm\n");
    printf("  -h        print this message\n");
    printf("  -m        print hints for language\n");
    printf("  -V        print version\n");
    printf("  -P        print parameters\n");
    printf("  -o file   set output file (current '%s')\n",
            outfile);
    printf("  -u num    set unit by 0.01bp (current %d)\n",
            objunit);
    printf("  -M num    set EPS outside margin by bp (current %d)\n",
            epsoutmargin);

    printf("  -F fontconfig font configuration\n");
    printf("         \"default=sanserif,large;kanji.all=IPAMincho\"\n");
    printf("         \"kanji.all.encode=CP932\"\n");
    {
        char *u = NULL;
        u = get_innercharset();
        if(u==NULL) {
            u = "*none*";
        }
    printf("              /* inner           charset (%s) */\n", u);
    }
    {
        char *u = NULL;
        u = get_eincharset();
        if(u==NULL) {
            u = "*none*";
        }
    printf("              /* external input  charset (%s) */\n", u);
    }

    printf("  -k ratio  set ascii/kanji size ratio (current %.3f)\n", akratio);
    printf("  -q        quiet mode\n");

    printf("--- debug ---\n");
    printf("  -v        verbose mode (current %d)\n", _t_);
    printf("  -b        draw boundingbox of objects\n");
    printf("  -B        draw visible boundingbox of objects\n");
    printf("  -U        draw object MOVE\n");
    printf("  -i        draw object IDs\n");
    printf("  -l        print object list\n");
    printf("  -c        print color list\n");
    printf("  -p        passing trace mode (current %d)\n", _p_);
    printf("  -g        draw grid\n");
    printf("  -G num    set grid pitch in bp (current %d)\n", 
            def_gridpitch);
    printf("  -R num    set grid range; how many times of pitch (current %d)\n",
        def_gridrange);
    printf("following itmes are reserved for future. do not use.\n");
    printf(" *-r        draw ruler\n");
    printf(" *-d        draft mode\n");
    printf(" *-L        draw labels\n");
    printf(" *-D        debug mode; equal -g -B -d -i -r -t\n");
    printf(" *-s num    set scale\n");
    printf(" *-n        no draw\n");
    printf(" *-N file   set notefile\n");


    return 0;
}


int
_print_alistmemkeys(char *pre, int ind, int wmax, apair_t *ap)
{
    int i;
    int j;
    apair_t *cp;
    int w;
    int l;

    printf("%s\n", pre);

    cp = ap;
    i  = 0;
    w  = ind;
            for(j=0;j<ind;j++) printf(" ");
    while(1) {
        if(!cp->name) {
            break;
        }
        l = strlen(cp->name)+1;

        if(w+l<wmax) {
            printf("%s ", cp->name);
            w += l;
        }
        else {
            printf("\n");
            for(j=0;j<ind;j++) printf(" ");
            printf("%s ", cp->name);
            w = ind + l;
        }
        
        cp++;
        i++;
    }


    printf("\n");
    
    return 0;
}

int
print_alistmemkeys(char *pre, apair_t *ap)
{
    return _print_alistmemkeys(pre, 4, 72, ap);
}

int
print_hints()
{
    printf("You may find your forget keyword...\n");
    print_alistmemkeys("object/command names:", cmd_ial);
    print_alistmemkeys("object attributes:", objattr_ial);
    print_alistmemkeys("line type:", linetype_ial);
    print_alistmemkeys("arrowhead type:", arrowhead_ial);
    print_alistmemkeys("hatching type:", hatchtype_ial);
    print_alistmemkeys("font type:", ff_ial);
    print_alistmemkeys("font size:", fh_ial);
    printf("charset:\n    EUC-JP, CP932, UTF-8\n");

    return 0;
}

int
print_version()
{
    printf("tangy version 2.106\n");
    return 0;
}

int
print_param()
{
    printf("tangy parameters:\n");

    printf("    %-24s %7s %7s\n", "name", "value", "factor");
    printf("    ---\n");

#define QPV(x)  \
    printf("    %-24s %7d %7.3f\n", #x, def_ ## x, x ## factor);
#define QP(x)  \
    printf("    %-24s %7s %7.3f\n", #x, "", x ## factor);

    printf("  unit-base: multiply by unitsize %d\n", objunit);

    QPV(ersize);
    QPV(arrowsize);
    QPV(warrowsize);
    QPV(linethick);
    QPV(linedecothick);
    QPV(linedecopitch);
    QPV(wlinethick);
    QPV(barrowgap);
    QPV(textheight);
    QPV(hatchthick);
    QPV(hatchpitch);
    QPV(marknoderad);
    QPV(markbbthick);
    QPV(noteosep);
    QPV(noteisep);

    printf("  text-base: multiply by text height (dynamic; normal %d)\n",
        def_textheight);

    QP(textdecent);
    QP(textbgmargin);

#undef QPV
#undef QP

    return 0;
}

int
test0()
{
    int i;

    i = 0;
    while(cmd_ial[i].name) {
        printf("%-16s ", cmd_ial[i].name);
        if(ISCHUNK(cmd_ial[i].value)) {
            printf("CHUNK ");
        }
        else {
            printf("-     ");
        }
        if(ISATOM(cmd_ial[i].value)) {
            printf("ATOM ");
        }
        else {
            printf("-    ");
        }
        if(ISGLUE(cmd_ial[i].value)) {
            printf("GLUE ");
        }
        else {
            printf("-    ");
        }
        if(HASBODY(cmd_ial[i].value)) {
            printf("BODY ");
        }
        else {
            printf("-    ");
        }
        if(MAYEXPAND(cmd_ial[i].value)) {
            printf("EXPAND ");
        }
        else {
            printf("-      ");
        }

            printf("\n");

        i++;
    }
    return 0;
}

extern int tx_trace;

int
main(int argc, char *argv[])
{
    ob*   ch0;
    ns*   ns0;
    int   flag;
    int   ik;
    int   do_colorlist, do_objlist;
    FILE *fp;
    int   x, y;
    int   i;
    int   touch_font=0;

    do_objlist = 0;
    do_colorlist = 0;
    
    pallet = new_default_pallet();

    tgyfont_setup(-1);

    while((flag=getopt(argc, argv,
            "0hmVPvqpngbBSUdiLrtDo:u:G:R:M:I:F:K:k:lcs:e:EQ"))!=EOF) {
        switch(flag) {
        case '0':
            test0();
            exit(0);
        case 'h':
            print_usage();
            exit(0);
            break;
        case 'm':
            print_hints();
            exit(0);
            break;
        case 'V':
            print_version();
            exit(0);
            break;
        case 'P':
            recalcsizeparam(); /* XXX */
            print_param();
            exit(0);
            break;
        case 'v':
            _t_ ++;
            break;
        case 'q':
            _t_ = 0;
            break;
        case 'p':
            _p_ = 1 - _p_;
            break;
        case 'n':
            nodraw = 1;
            break;
        case 'g':
            grid_mode = 1;
            break;
        case 'b':
            bbox_mode = 1;
            break;
        case 'B':
            visbbox_mode = 1;
            break;
        case 'i':
            oidl_mode = 1;
            break;
        case 'L':
            namel_mode = 1;
            break;
        case 'S':
            skelton_mode = 1;
            break;
        case 'U':
            movevisit_mode = 1;
            break;
        case 'd':
            draft_mode = 1;
            break;
        case 'r':
            ruler_mode = 1;
            break;
        case 't':
            text_mode = 1;
            break;
        case 'D':
            grid_mode = 1;
            bbox_mode = 1;
            draft_mode = 1;
            oidl_mode = 1;
            ruler_mode = 1;
            text_mode = 1;
            tx_trace = 1;
            break;
            
        case 'o':
            outfile = strdup(optarg);
            break;
        case 'u':
            objunit = atoi(optarg);
            break;
        case 'G':
            def_gridpitch = atoi(optarg);
            break;
        case 'R':
            def_gridrange = atoi(optarg);
            break;
        case 'M':
            epsoutmargin = atoi(optarg);
            break;

        case 'I':
            set_eincharset(optarg);
            break;

        case 'F':
            touch_font++;
            tgyfont_edit(FM_ASCII, optarg);
            break;
#if 0
        case 'K':
            touch_font++;
            tgyfont_edit(FM_KANJI, optarg);
            break;
#endif

        case 'k':
            akratio = atof(optarg);
            break;

#if 0
        case 'M':
            if(strcasecmp(optarg, "a4")==0) {
                /* inch x bp/inch */
                canvaswd = (int)( 8.27 * 72);
                canvasht = (int)(11.69 * 72);
            }
            break;
#endif
        case 'l':
            do_objlist++;
            break;
        case 'c':
            do_colorlist++;
            break;
        case 's':
            canvassc *= atof(optarg);
            break;
    
#if 0
        case 'e':
            touch_font++;
#if 0
            encode_edit(FM_KANJI, optarg);
#endif

#if 1
            tgyfont_wipe_encodeM(FM_KANJI, optarg);
#endif

            break;
#endif
        case 'E':
#if 0
            tgyfontset_fdump(stdout, "font parameters");
#endif
            tgyfontset_fprint(stdout, "font parameters");
            fflush(stdout);
            break;
        case 'Q':
            Echo("quit by explicit option\n");
            exit(0);
            break;
        }
    }
    if(_t_ && touch_font) {
#if 0
        fontset_fprint(stderr, "font parameters changed");
#endif
        tgyfontset_fprint(stderr, "font parameters changed");
        fprintf(stderr, "def_fontspec |%s|\n", def_fontspec);
        fprintf(stderr, "def_fontname |%s|\n", def_fontname);
    }

#if 0
    if(do_rotate) {
        int x;
        x = canvasht;
        canvasht = canvaswd;
        canvaswd = x;
        canvasrt = 90;
    }
#endif


    notefile_setup();

    ch0   = newchunk();
    ns0   = newnamespace();
    if(ch0) strcpy(ch0->cch.name, "*root*");
    if(ns0) strcpy(ns0->name, "*root*");

    recalcsizeparam();

    /*** parse files ***/
#if 0
fprintf(stderr, "optind %d/argc %d\n", optind, argc);
#endif
    if(optind==argc) {
        ik = parse(stdin, ch0, ns0);
        if(ik) {
        }
    }
    else {
        for(i=optind;i<argc;i++) {
#if 0
            fprintf(stderr, "file %d; %s\n", i, argv[i]);
#endif
            ik = fparse(argv[i], ch0, ns0);
            if(ik) {
                break;
            }
        }
    }

    if(INTRACE) {
        ob_cndump(ch0);
    }

    if(INTRACE) {
        varray_fprint(stdout, gnotefilelist);
        varray_fprint(stdout, gnotebindlist);
    }

    notefile_load();

#if 0
Echo("ch0 oid %d LANE? %d\n", ch0->oid, ch0->cch.lanenum);
#endif
Echo("ch0 oid %d LANE? %d\n", ch0->oid, ch0->cob.lanenum);

    ik = linkchk(ch0, ns0);
P;

    x = 0;
    y = 0;
P;
    ik = put(ch0, &x, &y, ns0);
    if(INTRACE) {
        ob_gdump(ch0);
    }

P;
    finalize(ch0, 0, 0, ns0);

    if(INTRACE) {
        ob_adump(ch0);
        ob_bgdump(ch0);
        ob_bldump(ch0);
    }

#if 0
    printf("ht root-chunk %d vs canvas %d\n", ch0->ht, canvasht);
#endif

    if(ch0->ht<=0 || ch0->wd<=0) {
        Error("figure is empty or negative size\n");
        exit(9);
    }

#if 0
    canvassc = canvasht/ch0->ht;
    canvassc = 0.01;
#endif

#if 0
    picdraw(ch0, 0, 0, ns0);
    fflush(stdout);
#endif

P;
    fp = fopen(outfile, "w");
    if(!fp) {
        Error("cannot open output file '%s'\n", outfile);
        goto skip_output;
    }

    epsdraw(fp, canvaswd, canvasht, canvasrt, canvassc,
        ch0, 0, 0, ns0);
    fclose(fp);
    fp = NULL;

skip_output:
    (void) 0;

    if(do_objlist) {
        printobjlist(stdout, "  ", ch0);
    }
    if(do_colorlist) {
        varray_fprint(stdout, pallet);
    }

    exit(0);
}


