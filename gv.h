/*
 * tangy second trial since 2016 Aug 
 * K.Chinen
 *
 */

#ifndef _GV_H_
#define _GV_H_


extern char debuglog[];

extern int objunit;

extern int epsoutmargin;
extern char *def_fontname;
extern int   def_markcolor;


extern double arrowsizefactor;
extern double warrowsizefactor;
extern double linethickfactor;
extern double linedecothickfactor;
extern double linedecopitchfactor;
extern double wlinethickfactor;
extern double barrowgapfactor;
extern double mustsizefactor;
extern double textheightfactor;
extern double textdecentfactor;
extern double textbgmarginfactor;
extern double textdecentfactor;
extern double textbgmarginfactor;
extern double hatchthickfactor;
extern double hatchpitchfactor;
extern double marknoderadfactor;
extern double markbbthickfactor;
extern double noteosepfactor;
extern double noteisepfactor;

extern double pbstrgapfactor;

extern int draft_mode;
extern int skelton_mode;
extern int grid_mode;
extern int bbox_mode;
extern int oidl_mode;
extern int namel_mode;
extern int ruler_mode;
extern int text_mode;
extern int movevisit_mode;

extern double def_hollowratio;
extern double def_shadowgray;

extern int def_dir;
extern int def_scale;
extern int def_color;
extern int def_fgcolor;
extern int def_bgcolor;
extern int def_linethickmode;
extern int def_linethick;
extern int def_arrowangle;
extern int def_arrowsize;
extern int def_linedecothick;
extern int def_linedecopitch;
extern int def_mustsize;
extern int def_wlinethick;
extern int def_warrowangle;
extern int def_warrowsize;
extern int def_barrowgap;
extern int def_textheight;
extern int def_hatchpitch;
extern int def_hatchthick;
extern int def_marknoderad;
extern int def_markbbthick;
extern int def_gridpitch;
extern int def_gridrange;
extern int def_noteosep;
extern int def_noteisep;
extern char *def_fontname;

extern int def_pbstrgap;


extern pallet_t *pallet;


extern double rf;
extern int _t_;
extern int _p_;

#ifndef MAX
#define MAX(a,b)    ((a)>(b) ? (a) : (b))
#endif

#define INTRACE (_t_>0)
#define P   if(_p_){printf("PASS %s:%d;%s\n", __FILE__, __LINE__, __func__); fflush(stdout); }
#define E   printf("%s:%d;%s probably ERROR\n", __FILE__, __LINE__, __func__); fflush(stdout);


#define Error   printf("ERROR %s:%s ", __FILE__, __func__);fflush(stdout);printf
#define Warn    printf("WARNING %s:%s ", __FILE__, __func__);fflush(stdout);printf
#define Info    if(_t_>=2)printf("INFO %s:%s ", __FILE__, __func__),fflush(stdout),printf
#define Echo    if(_t_)printf




#endif /* _GV_H_ */


