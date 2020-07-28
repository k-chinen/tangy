





#if 0
int
epsdraw_sstrbgX(FILE *fp, int x, int y, int wd, int ht,
        int al, int exof, int ro, int qof,
        int bgshape, int qbgmargin, int fgcolor, int bgcolor,
        varray_t *ssar, int ugjust)
#endif

#define changetext(xxx)     {(void)x;}
#define changetext2(xxx)    {(void)x;}
#define changetext3(xxx)    {(void)x;}


int
sstr_heightdepth(FILE *fp, int x, int y, int wd, int ht,
        int al, int exof, int ro, int qof,
        int bgshape, int qbgmargin, int fgcolor, int bgcolor,
        varray_t *ssar, int ugjust, int opt, int *rht, int *rdp)
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
    int     imof;
    int     offset;
    int     jsar[BUFSIZ];

int by,ty;


ty = INT_MIN;
by = INT_MAX;
    
    if(!ssar) {
        E;
        return -1;
    }

    memset(jsar, 0, sizeof(jsar));

    ss_strip(mcontall, BUFSIZ, ssar);

#if 1
    Echo("%s: x,y %d,%d wd,ht %d,%d al %d exof %d ro %d qof %d bgshape %d qbgmargin %d '%s' ugj %d\n",
        __func__, x, y, wd, ht, al, exof, ro, qof, bgshape, qbgmargin, mcontall, ugjust);
#endif

    if(ssar->use<=0)  {
        goto skip_label;
    }

    fprintf(fp, "%% %s: pos %6d,%-6d %6dx%-6d %4d fg %d bg %d %d %d ; %s\n",
        __func__, x, y, wd, ht, ro,
        fgcolor, bgcolor, bgshape, qbgmargin, mcontall);
    fprintf(fp, "%% fg %d bg %d\n", fgcolor, bgcolor);



    memset(mcar, 0, sizeof(mcar));

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

#if 1
{
    int u;
    int lby, lty;
    int lzy;
    int maxht, maxdp;

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
    return 0;
}
#endif


    if(ugjust<0) {
        switch(al) {
        case PO_WEST:   gjust = SJ_LEFT;    imof = -wd/2;               break;
        case PO_WI:     gjust = SJ_LEFT;    imof = -wd/2 + 2*bgmargin;  break;
        case PO_WO:     gjust = SJ_RIGHT;   imof = -wd/2 - 2*bgmargin;  break;
        case PO_EAST:   gjust = SJ_RIGHT;   imof = wd/2;                break;
        case PO_EI:     gjust = SJ_RIGHT;   imof = wd/2 - 2*bgmargin;   break;
        case PO_EO:     gjust = SJ_LEFT;    imof = wd/2 + 2*bgmargin;   break;
        case PO_CE:     gjust = SJ_LEFT;    imof = 0;                   break;
        case PO_CEO:    gjust = SJ_LEFT;    imof = 0 + 2*bgmargin;      break;
        case PO_CW:     gjust = SJ_RIGHT;   imof = 0;                   break;
        case PO_CWO:    gjust = SJ_RIGHT;   imof = 0 - 2*bgmargin;      break;
        default:
                        gjust = SJ_CENTER;  imof = 0;                   break;
        }
    }
    else {
        gjust = ugjust;
        imof  = 0;
    }

    offset = exof + imof;

    fprintf(fp, "%% ugjust %d, al %d exof %d qof %d -> gjust %d imoff %d offset %d\n",
        ugjust, al, exof, qof, gjust, imof, offset);


    fprintf(fp, "gsave %% for sstr\n");
    fprintf(fp, "  %d %d translate\n", x, y);
    fprintf(fp, "  %d %d translate\n", offset, 0);
    fprintf(fp, "  %d rotate\n", ro);
    if(qof!=0) {
        fprintf(fp, "  %d %d translate\n", qof, 0);
    }

    if(text_mode) {
        fprintf(fp, "  gsave %% textmode\n");
        changetext(fp);
        /* center line */
        fprintf(fp, "    %d %d moveto %d %d rlineto stroke\n",
                            0, -ht/2, 0, ht);

        fprintf(fp, "    %d %d moveto %d %d rlineto %d %d rlineto stroke\n",
                            -wd/2, ht/2-rh/2,
                            -wd/10, 0, 0, -(n*py+bgmargin*2));
        fprintf(fp, "    %d %d moveto %d %d rlineto %d %d rlineto %d %d rlineto closepath fill\n",
                            -wd/2, ht/2-rh/2,
                            -wd/10, 0, 0, -bgmargin, wd/10, 0);
        fprintf(fp, "    %d %d moveto %d %d rlineto %d %d rlineto %d %d rlineto closepath fill\n",
                            -wd/2, ht/2-rh/2-(n*py+bgmargin*2),
                            -wd/10, 0, 0, bgmargin, wd/10, 0);

        fprintf(fp, "  grestore %% textmode\n");
    }

    fprintf(fp, "  %% text offset\n");
    fprintf(fp, "  %d %d translate\n", 0, (py*n)/2-py+pyb);

    if(text_mode) {
        fprintf(fp, "  gsave\n");
        gy = 0;
        for(i=0;i<ssar->use;i++) {
            changetext3(fp);
    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                -wd/2, -gy, wd, 0);

            for(j=0;j<i+1;j++) {
    fprintf(fp, "    newpath %d %d moveto %d %d rlineto stroke\n",
                    -wd/2+wd/10*(j+1), -gy, 0, -pyb);
            }
            gy += py;
        }
        fprintf(fp, "  grestore\n");
    }

    tq = NULL;

    int lbg_h, lbg_b, lbg_t;

    lbg_h = 0;
    lbg_b = 0;
    lbg_t = 0;

    fprintf(fp, "  /sstrwar %d array def %% array for string-width\n",
        ssar->use);

    /*****
     ***** 1st pass
     *****/

    fprintf(fp, "  gsave %% bgdraw\n");

    gy = 0;
    for(i=0;i<ssar->use;i++) {
        gy -= py;
        uu = (sstr*)ssar->slot[i];

        if(tq) txe_release(tq);

        tq = varray_new();
        varray_entrysprintfunc(tq, txe_sprint);

        txe_parse(tq, uu->ssval);

        justify = gjust;
        hscale = 100;

        fprintf(fp, "    %% enter str %d '%s' gy %d\n", i, uu->ssval, gy);

        fprintf(fp, "    gsave %% oneline\n");

Echo("  --- calc size 1\n");

        curmode = FM_ASCII;
        curface = FF_SERIF;
        cursize = FH_NORMAL;
        hscale  = 100;


        /* check content existance */

        txear_extract(mcontline, BUFSIZ, tq);

        if(!mcontline[0]) {
            fprintf(fp, "    %% skip  sstr drawing %d '%s'\n", i, mcontline);
            goto skip_bgdrawing;
        }
        else {
            fprintf(fp, "    %% enter sstr drawing %d '%s'\n", i, mcontline);
        }
        mcar[i] = strlen(mcontline);

        fprintf(fp, "      %% calc size (width)\n");
        fprintf(fp, "      /sstrw 0 def\n");
        fprintf(fp, "      /%s findfont %d scalefont setfont %% default\n",
            def_fontname, fht);
        afh = fht;

        afhmax = -1;
    
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
                        fprintf(fp, "        /%s findfont %d scalefont setfont\n",
                            afn, afh);
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
                }
                else {
                    psescape(qs, BUFSIZ, mcpart);
                }

                if(hscale!=100) {
                    fprintf(fp, "  gsave %% text-scale\n");
                    fprintf(fp, "    %.3f 1 scale\n", (double)hscale/100);
                }
    if(hscale!=100) {
        fprintf(fp,
        "      (%s) stringwidth pop /sstrw exch %.3f mul sstrw add def\n",
            qs, (double)hscale/100);
    }
    else {
        fprintf(fp,
        "      (%s) stringwidth pop /sstrw exch sstrw add def\n",
            qs);
    }
                if(hscale!=100) {
                    fprintf(fp, "  grestore %% text-scale\n");
                }
#if 1
                /* check max height when it used */
                if(afh>afhmax) {afhmax = afh; }
#endif
            }

        }

        fprintf(fp, 
            "      sstrwar %d sstrw put %% store value to reuse\n", i);

        if(text_mode) {
            fprintf(fp, "      %% textguide 1st\n");
            fprintf(fp, "      gsave\n");

            changetext2(fp);
            fprintf(fp, "        sstrw 2 div neg %d moveto\n", objunit*7/100);
            fprintf(fp, "        0 %d rlineto\n", -objunit*7/100);
            fprintf(fp, "        sstrw 0 rlineto\n");
            fprintf(fp, "        0 %d rlineto\n", objunit*7/100);
            fprintf(fp, "        stroke\n");

            fprintf(fp, "      grestore %% textguide\n");
        }


        if(i<BUFSIZ) jsar[i] = justify;

        fprintf(fp, "      %% justify %d\n", justify);
        
        switch(justify) {
        case SJ_LEFT:
            break;
        case SJ_RIGHT:
            fprintf(fp, "      sstrw neg 0 translate\n");
            break;
        default:
        case SJ_CENTER:
            fprintf(fp, "      sstrw 2 div neg 0 translate\n");
            break;
        case SJ_FIT:
            fprintf(fp, "      %d sstrw div 1 scale\n", wd);
            fprintf(fp, "      sstrw 2 div neg 0 translate\n");
            break;
        }

        if(bgcolor>=0) {
            fprintf(fp, "      %% bg fill\n");
            fprintf(fp, "      gsave %% textbg\n");
            changecolor(fp, bgcolor);

            fprintf(fp, "        0 %d neg sstrw %d %d mrboxfill\n",
                pyb, pyb+afhmax, bgmargin);


            fprintf(fp, "      grestore %% textbg\n");
        }

skip_bgdrawing:
        (void)0;

        fprintf(fp, "    grestore %% oneline\n");

        fprintf(fp, "    0 %d translate\n", -py);
    }
    fprintf(fp, "  grestore %% bgdraw\n");


    /*****
     ***** 2nd pass
     *****/

    fprintf(fp, "  %%%% text\n");
    fprintf(fp, "  gsave %% txtdraw\n");

    gy = 0;
    for(i=0;i<ssar->use;i++) {
        gy -= py;
        uu = (sstr*)ssar->slot[i];

        if(tq) txe_release(tq);

        tq = varray_new();
        varray_entrysprintfunc(tq, txe_sprint);

        txe_parse(tq, uu->ssval);

        if(i<BUFSIZ) justify = jsar[i];
        hscale = 100;

        fprintf(fp, "    %% enter str %d '%s' gy %d\n", i, uu->ssval, gy);

        fprintf(fp, "    gsave %% oneline\n");


Echo("  --- calc size 2\n");

        curmode = FM_ASCII;
        curface = FF_SERIF;
        cursize = FH_NORMAL;
        hscale  = 100;


        if(mcar[i]<=0) {
            fprintf(fp, "  %% skip  sstr drawing %d mcar %d\n", i, mcar[i]);
            goto skip_txtdrawing;
        }

        /* check content existance */
        txear_extract(mcontline, BUFSIZ, tq);

        if(!mcontline[0]) {
            fprintf(fp, "      %% skip  sstr drawing %d '%s'\n", i, mcontline);
            goto skip_txtdrawing;
        }
        else {
            fprintf(fp, "      %% enter sstr drawing %d '%s'\n", i, mcontline);
        }


        fprintf(fp, "      /sstrw sstrwar %d get def %% reuse width\n", i);

        if(text_mode) {
            fprintf(fp, "      %% textguide 2nd\n");
            fprintf(fp, "      gsave\n");

            changetext2(fp);
            fprintf(fp, "        sstrw 2 div neg %d moveto\n", objunit*7/100);
            fprintf(fp, "        0 %d rlineto\n", -objunit*7/100);
            fprintf(fp, "        sstrw 0 rlineto\n");
            fprintf(fp, "        0 %d rlineto\n", objunit*7/100);
            fprintf(fp, "        stroke\n");

            fprintf(fp, "      grestore %% textguide\n");
        }


        /*** PASS 2 */

Echo("  --- drawing\n");
        fprintf(fp, "      /%s findfont %d scalefont setfont %% default\n",
            def_fontname, fht);
        fprintf(fp, "      0 0 moveto\n");
#if 1
        changecolor(fp, fgcolor);
#endif

        curmode = FM_ASCII;
        curface = FF_SERIF;
        cursize = FH_NORMAL;
        hscale  = 100;

#if 1
                fprintf(fp, "      %% justify %d\n", justify);
                
                switch(justify) {
                case SJ_LEFT:
                    break;
                case SJ_RIGHT:
                    fprintf(fp, "      sstrw neg 0 translate\n");
                    break;
                default:
                case SJ_CENTER:
                    fprintf(fp, "      sstrw 2 div neg 0 translate\n");
                    break;
                case SJ_FIT:
                    fprintf(fp, "      %d sstrw div 1 scale\n", wd);
                    fprintf(fp, "      sstrw 2 div neg 0 translate\n");
                    break;
                }
                    fprintf(fp, "      0 0 moveto\n");
#endif

        Echo("0 m/f/h %s(%d) %s(%d) %s(%d)\n",
            rassoc(fm_ial, curmode), curmode,
            rassoc(ff_ial, curface), curface,
            rassoc(fh_ial, cursize), cursize);

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
                        if(strncasecmp(token,"hscale", 6)==0) {
                            hscale = atoi(token+6);
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
                    }
                }
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
                    afn  = xrassoc(ff_act_ial, curface);
*/
                    afn  = resolv_font(curmode, curface);
                    afhs = xrassoc(fh_act_ial, cursize);
                    afh  = fht;
                    if(afhs!=NULL) {
                        afh = atof(afhs)*fht;
                    }

                    if(curmode==FM_KANJI) {
                        afh = (int)(afh*akratio);
                    }

Echo("  afn '%s' afhs '%s' afh %d\n", afn, afhs, afh);

                    if(afn) {
Echo("   setfont!\n");
                        fprintf(fp, "        /%s findfont %d scalefont setfont\n",
                            afn, afh);
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
                }
                else {
                    psescape(qs, BUFSIZ, mcpart);
                }

                if(hscale!=100) {
P;
                    fprintf(fp, "  gsave %% comp\n");
                    fprintf(fp, "    %.3f 1 scale\n", (double)hscale/100);
                }

Echo("D m/f/s %s(%d) %s(%d) %s(%d)\n",
    rassoc(fm_ial, curmode), curmode,
    rassoc(ff_ial, curface), curface,
    rassoc(fh_ial, cursize), cursize);

                fprintf(fp, "      (%s) show\n", qs);
                if(hscale!=100) {
P;
                    fprintf(fp, "  grestore %% comp\n");
                }
            }

        }

skip_txtdrawing:
        (void)0;

        fprintf(fp, "    grestore %% oneline\n");

        fprintf(fp, "    0 %d translate\n", -py);
    }

    fprintf(fp, "  grestore %% txtdraw\n");


    fprintf(fp, "grestore %% end of sstr\n");

Echo("%s: by %d ty %d\n", __func__, by, ty);

skip_label:
    (void)0;

    return 0;
}
