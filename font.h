#ifndef _TANGY_FONT_H_
#define _TANGY_FONT_H_

#include <iconv.h>

#define FM_NONE     (0)
#define FM_ASCII    (1)
#define FM_KANJI    (2)
#define FM_ALL    (999)

extern apair_t fm_ial[];
extern double akratio;

#define FF_NONE     (0)
#define FF_SERIF    (1)
#define FF_SANSERIF (2)
#define FF_ITALIC   (3)
#define FF_TYPE     (4)
#define FF_ALL    (999)

extern apair_t ff_ial[];

#define FA_NONE     (0)
#define FA_FONTNAME (1)
#define FA_ENCODE   (2)
#define FA_SCALE    (3)

extern apair_t fa_ial[];



#define FH_NONE     (0)
#define FH_NORMAL   (1)
#define FH_LARGE    (2)
#define FH_HUGE     (3)
#define FH_SMALL    (4)
#define FH_TINY     (5)

extern apair_t fh_ial[];
extern apair_t fh_val_ial[];


extern char *def_fontname;
extern char *def_fontspec;

extern char *ext_fontnamelist;
extern char *ext_fontfilelist;


#define resolv_fontname(xm,xf)  tgyfont_resolv_fontname(xm,xf)
#define resolv_encode(xm,xf)    tgyfont_resolv_encode(xm,xf)
#define resolv_scale(xm,xf)     tgyfont_resolv_scale(xm,xf)




typedef struct {
    int     fsid;
    char   *fsname;
    double  fsval;
} tgyfont_size_t;

typedef struct {
    /* key */
    int     fmode;
    int     fface;
    /* attribute */
    char   *fname;
    char   *fencode;
    double  fscale;
    /* working place */
    iconv_t fcq;
} tgyfont_t;

#ifndef TGYFONTSET_N
#define TGYFONTSET_N    (12)
#endif

extern int tgyfontset_use;
extern int tgyfontset_max;
extern tgyfont_t *tgyfontset;

int tgyfont_fprint(FILE *fp, tgyfont_t *tf, char *pmsg);
int tgyfontset_fprint(FILE *fp, char *pmsg); 
int tgyfontset_fdump(FILE *fp, char *pmsg); 
int tgyfont_setup(int);

tgyfont_t *tgyfontset_find(int, int, void*);
char* tgyfont_resolv_fontname(int xmode, int xface);
char* tgyfont_resolv_encode(int xmode, int xface);
double tgyfont_resolv_scale(int xmode, int xface);

int tgyfont_wipe_fontnameMF(int xmode, int xface, char *xval);
int tgyfont_wipe_encodeMF(int xmode, int xface, char *xval);
int tgyfont_wipe_scaleMF(int xmode, int xface, double xval);

int tgyfont_wipe_fontnameM(int xmode, char *xval);
int tgyfont_wipe_encodeM(int xmode, char *xval);
int tgyfont_wipe_scaleM(int xmode, double xval);

int tgyfont_edit(int, char*);





#endif/*_TANGY_FONT_H_*/
