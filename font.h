#ifndef _TANGY_FONT_H_
#define _TANGY_FONT_H_

#define FM_ASCII    (1)
#define FM_KANJI    (2)

extern apair_t fm_ial[];
extern double akratio;

#define FH_NORMAL   (0)
#define FH_LARGE    (1)
#define FH_HUGE     (2)
#define FH_SMALL    (3)
#define FH_TINY     (4)

extern apair_t fh_ial[];
extern apair_t fh_act_ial[];

#define FF_SERIF    (0)
#define FF_SANSERIF (1)
#define FF_ITALIC   (2)
#define FF_TYPE     (3)

extern apair_t ff_ial[];
extern apair_t ff_act_ial[];
extern apair_t ff_actk_ial[];

extern char *def_fontname;
extern char *def_fontspec;

extern char *ext_fontnamelist;
extern char *ext_fontfilelist;

int swap_font(int xv, char *xn);
int swap_Xfont(apair_t *ls, int xv, char *xn);

int fontset_fprint(FILE *fp, char *pmsg);
int font_edit(int cat, char *opseq);

#endif/*_TANGY_FONT_H_*/
