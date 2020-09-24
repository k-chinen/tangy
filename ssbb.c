/* */


extern int psescape(char *dst, int dlen, char *src);
extern int psescape7(char *dst, int dlen, char *src);

int
est_sstrbb(FILE *fp, int xoid, int x, int y, int wd, int ht,
        int pos, int exhof, int exvof, int ro, int qhof, int qvof,
        int bgshape, int qbgmargin, int fgcolor, int bgcolor,
        varray_t *ssar, int ugjust, qbb_t *obb)
{
/*
 * 2pass routine
 *     1st - calcurate width and draw backgound round-box
 *     1nd - draw string
 */

    int   i;
    int   py;
    int   pyb;
    int   gy;
    sstr *uu;
    int   fht;
    int   n;
    int   rh;
    char  qs[BUFSIZ];
    int   bgmargin;

    varray_t *tq;
    int   j;
    txe      *te;
    char      token[BUFSIZ];
    int       cc;
    char     *p, *q;
    int   curmode, newmode;
    int   cursize, curface;
    int   newsize, newface;
    char     *afn, *afhs;
    int       afh, afhmax;
    int     justify;
    int     hscale;
    char    mcontall[BUFSIZ];
    char    mcontline[BUFSIZ];
    char    mcpart[BUFSIZ];
    int     mcar[BUFSIZ];
    int     gjust;
    int     imhof;
    int     imvof;
    int     hoffset;
    int     voffset;
    int     jsar[BUFSIZ];

    int     wcur, wsum, wmax;
    double  twratio_a = 0.7, twratio_k = 1.0;

    
    if(!ssar) {
        E;
        return -1;
    }

    qbb_reset(obb);
    wmax = -1;

    memset(jsar, 0, sizeof(jsar));

#if 0
    Echo("%s: x,y %d,%d wd,ht %d,%d ro %d\n", __func__, x, y, wd, ht, ro);
#endif

#if 0
    ss_dump(stdout, ssar);
#endif
    ss_strip(mcontall, BUFSIZ, ssar);
#if 0
    Echo("mcontall '%s'\n", mcontall);
#endif

#if 1
    Echo("%s: x,y %d,%d wd,ht %d,%d pos %d exhof %d ro %d '%s' ugj %d\n",
        __func__, x, y, wd, ht, pos, exhof, ro, mcontall, ugjust);
#endif

#if 0
    varray_fprint(stdout, ssar);
#endif

    if(ssar->use<=0)  {
        goto skip_label;
    }

    if(fp) {
    fprintf(fp, "%% %s: pos %6d,%-6d %6dx%-6d %4d fg %d bg %d %d %d ; %s\n",
        __func__, x, y, wd, ht, ro,
        fgcolor, bgcolor, bgshape, qbgmargin, mcontall);
    }



    memset(mcar, 0, sizeof(mcar));

    n = ssar->use;

    fht = def_textheight; 
    pyb = (int)((double)fht*textdecentfactor);
    bgmargin = (int)((double)fht*textbgmarginfactor);

    py = fht;
    rh = ht-(n*py+bgmargin*2);

    if(fp) {
    fprintf(fp, "%%  fht %d, ht %d, n %d, rh %d, py %d, bgmargin %d\n",
        fht, ht, n, rh, py, bgmargin);
    }

    Echo("ht %d use %d -> py %d\n", 
        ht, ssar->use, py);

#if 0
fprintf(stderr, "wd %d bgmargin %d\n", wd, bgmargin);
#endif

    imhof = 0;
    imvof = 0;
    gjust = SJ_CENTER;

    if(ugjust<0) {
        switch(pos) {
        case PO_WEST:   gjust = SJ_LEFT;    imhof = -wd/2;               break;
        case PO_WI:     gjust = SJ_LEFT;    imhof = -wd/2 + 2*bgmargin;  break;
        case PO_WO:     gjust = SJ_RIGHT;   imhof = -wd/2 - 2*bgmargin;  break;
        case PO_EAST:   gjust = SJ_RIGHT;   imhof = wd/2;                break;
        case PO_EI:     gjust = SJ_RIGHT;   imhof = wd/2 - 2*bgmargin;   break;
        case PO_EO:     gjust = SJ_LEFT;    imhof = wd/2 + 2*bgmargin;   break;
        case PO_CE:     gjust = SJ_LEFT;    imhof = 0;                   break;
        case PO_CEO:    gjust = SJ_LEFT;    imhof = 0 + 2*bgmargin;      break;
        case PO_CW:     gjust = SJ_RIGHT;   imhof = 0;                   break;
        case PO_CWO:    gjust = SJ_RIGHT;   imhof = 0 - 2*bgmargin;      break;

        case PO_NORTH:  imvof =  ht/2 - py/2;               break;
        case PO_NI:     imvof =  ht/2 - py/2 - 2*bgmargin;  break;
        case PO_NO:     imvof =  ht/2 + py/2 + 2*bgmargin;  break;

        case PO_SOUTH:  imvof = -ht/2 + py/2;               break;
        case PO_SI:     imvof = -ht/2 + py/2 + 2*bgmargin;  break;
        case PO_SO:     imvof = -ht/2 - py/2 - 2*bgmargin;  break;

        case PO_CN:     imvof =  py/2;                      break;
        case PO_CNO:    imvof =  py/2 + 2*bgmargin;         break;
        case PO_CS:     imvof = -py/2;                      break;
        case PO_CSO:    imvof = -py/2 - 2*bgmargin;         break;

#if 0
        default:
                        gjust = SJ_CENTER;  imhof = 0;                   break;
#endif
        }
    }
    else {
        gjust = ugjust;
        imhof  = 0;
    }

    hoffset = exhof + imhof;
    voffset = exvof + imvof;

    if(fp) {
    fprintf(fp,
    "%% ugjust %d, pos %d exhof %d qhof %d -> gjust %d imhoff %d hoffset %d\n",
        ugjust, pos, exhof, qhof, gjust, imhof, hoffset);
    fprintf(fp,
    "%% ugjust %d, pos %d exvof %d qvof %d -> gjust %d imvoff %d voffset %d\n",
        ugjust, pos, exvof, qvof, gjust, imvof, voffset);
    }

#if 0
    qbb_mark(obb, hoffset, voffset);
#endif

    tq = NULL;

    int lbg_h, lbg_b, lbg_t;

    lbg_h = 0;
    lbg_b = 0;
    lbg_t = 0;

    /*****
     ***** 1st pass
     *****/

#if 0
    fprintf(fp, "  %d %d translate\n", 0, (py*n)/2-py+pyb);
#endif
    int mmvoff = (py*n)/2-py+pyb;
    if(fp) {
    fprintf(fp, "%% mmvoff %d\n", mmvoff);
    }

    gy = 0;
    for(i=0;i<ssar->use;i++) {
        gy -= py;
        uu = (sstr*)ssar->slot[i];

        if(tq) txe_release(tq);

        tq = varray_new();
        varray_entrysprintfunc(tq, txe_sprint);

        txe_parse(tq, uu->ssval);

#if 0
        varray_fprint(stdout, tq);
#endif

        justify = gjust;
        hscale = 100;

        if(fp) {
        fprintf(fp, "    %% enter str %d '%s' gy %d\n", i, uu->ssval, gy);
        }


Echo("  --- calc size 1\n");

        curmode = FM_ASCII;
        curface = FF_SERIF;
        cursize = FH_NORMAL;
        hscale  = 100;

        /* check content existance */
        txear_extract(mcontline, BUFSIZ, tq);

        if(!mcontline[0]) {
            if(fp) {
            fprintf(fp, "    %% skip  sstr drawing %d '%s'\n", i, mcontline);
            }
            goto skip_bgdrawing;
        }
        else {
            if(fp) {
            fprintf(fp, "    %% enter sstr drawing %d '%s'\n", i, mcontline);
            }
        }
        mcar[i] = strlen(mcontline);

        afh = fht;
        afhmax = -1;

        wcur = 0;
        wsum = 0;
    
        qs[0] = '\0';
        for(j=0;j<tq->use;j++) {
            te = tq->slot[j];
            
            if(te->ct==TXE_CMD) {
                newmode = -1;
                newface = -1;
                newsize = -1;
                p = NULL;
                if(te->st==TXE_CONST) {
                    p = te->cs;
                }
                else {
                    p = te->vs;
                }
                while(*p) {
                    p = draw_word(p, token, BUFSIZ, SSTR_SEPC);
                    if(token[0]) {
Echo("    token '%s'\n", token);
                        if(strncasecmp(token,"kanji", 5)==0) {
                            newmode = FM_KANJI;
                        }
                        if(strncasecmp(token,"ascii", 5)==0) {
                            newmode = FM_ASCII;
                        }
                        if(strncasecmp(token,"scale", 5)==0) {
                            hscale = atoi(token+5);
Echo("    hscale %d\n", hscale);

                        }
                        cc = assoc(fh_ial, token);
                        switch(cc) {
                        case FH_HUGE:   newsize = FH_HUGE;  break;
                        case FH_LARGE:  newsize = FH_LARGE; break;
                        case FH_SMALL:  newsize = FH_SMALL; break;
                        case FH_TINY:   newsize = FH_TINY;  break;
                        case FH_NORMAL: newsize = FH_NORMAL; break;
                        }                   
                        cc = assoc(ff_ial, token);
                        switch(cc) {
                        case FF_SANSERIF:   newface = FF_SANSERIF;  break;
                        case FF_ITALIC:     newface = FF_ITALIC;    break;
                        case FF_TYPE:       newface = FF_TYPE;  break;
                        case FF_SERIF:      newface = FF_SERIF; break;
                        }
                        cc = assoc(sj_ial, token);
                        if(cc>=0) { justify = cc; }
                    }
                }
#if 0
                if(cursize != newsize || curface != newface) 
#endif
                {
Echo(" newface %d newsize %d newmode %d\n", newface, newsize, newmode);
                    if(newmode>=0) {
                        curmode = newmode;
                    }
                    if(newsize>=0) {
P;
                        cursize = newsize;
                    }
                    if(newface>=0) {
P;
                        curface = newface;
                    }
Echo(" curface %d cursize %d curmode %d\n", curface, cursize, curmode);
/*
                    afn  = rassoc(ff_act_ial, curface);
*/
                    afn  = resolv_font(curmode, curface);
                    afhs = rassoc(fh_act_ial, cursize);
                    afh  = fht;
                    if(afhs!=NULL) {
                        afh = atof(afhs)*fht;
                    }

                    if(curmode==FM_KANJI) {
                        afh = (int)(afh*akratio);
                    }

Echo("  afn '%s' afhs '%s' afh %d (max %d)\n", afn, afhs, afh, afhmax);

                    if(afn) {
Echo("   setfont!\n");
                    }
                    else {
                    }
                }
            }
            else
            if(te->ct==TXE_DATA) {
                txe_extract(mcpart, BUFSIZ, te);
                if(curmode==FM_KANJI) {
                    psescape7(qs, BUFSIZ, mcpart);
                    wcur = (int)(afh*strlen(mcpart)*twratio_k);
                }
                else {
                    psescape(qs, BUFSIZ, mcpart);
                    wcur = (int)(afh*strlen(mcpart)*twratio_a);
                }
if(fp) {
fprintf(fp, "%% '%s' -> %d\n", mcpart, wcur);
}

#if 1
                /* check max height when it used */
                if(afh>afhmax) {afhmax = afh; }
#endif
            }

            wsum += wcur;
            if(fp) {
            fprintf(fp, "%% wcur %d wsum %d\n", wcur, wsum);
            }
        }

        if(text_mode) {
        }


        if(i<BUFSIZ) jsar[i] = justify;

        if(fp) {
        fprintf(fp, "      %% justify %d\n", justify);
        }
        
        switch(justify) {
        case SJ_LEFT:
            qbb_mark(obb, 0, gy+afh-pyb+mmvoff);
            qbb_mark(obb, wsum, gy+afh+(afh-pyb)+mmvoff);
            break;
        case SJ_RIGHT:
            qbb_mark(obb, -wsum, gy+afh-pyb+mmvoff);
            qbb_mark(obb, 0, gy+afh+(afh-pyb)+mmvoff);
            break;
        default:
        case SJ_CENTER:
            if(fp) {
            fprintf(fp, "%% gy %d mmvoff %d ; %d %d ; %d %d\n",
                gy, mmvoff,
                -wsum/2, gy+afh-pyb+mmvoff,
                wsum/2, gy+afh+(afh-pyb)+mmvoff);
            }

            qbb_mark(obb, -wsum/2, gy+afh-pyb+mmvoff);
            qbb_mark(obb, wsum/2, gy+afh+(afh-pyb)+mmvoff);
            break;
        case SJ_FIT:
            break;
        }

        if(bgcolor>=0) {
        }

skip_bgdrawing:
        (void)0;

    }


    /*****
     ***** 2nd pass
     *****/

skip_label:
    (void)0;

    qbb_shift(obb, hoffset, voffset);
#if 0
    fprintf(stdout, "oid %d s - ", xoid); 
    qbb_fprint(stdout, obb);
#endif

    return 0;
}

