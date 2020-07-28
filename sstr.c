#include "obj.h"

int
ss_strip(char *dst, int dlen, varray_t *ssar)
{
    int   i;
    sstr *uu;
    int   cc;
    int   w;

    dst[0] = '\0';
    cc = 0;

    for(i=0;i<ssar->use;i++) {
        uu = (sstr*)ssar->slot[i];
        if(uu && uu->ssval && *(uu->ssval) ) {
            w = strlen(uu->ssval);
#if 0
Echo(" %2d: %3d '%s'\n", i, w, uu->ssval);
#endif
            if(cc+w+1<dlen-1) {
                if(dst[0]) {
                    strcat(dst, "/");
                    cc += 1;
                }
                else {
                }
                strcat(dst, uu->ssval);
                cc += w;
            }
            else {
                return 1;
            }
        }
    }

    return 0;
}

int
ss_dump(FILE *ofp, varray_t *ssar)
{
    int   i;
    sstr *uu;

    for(i=0;i<ssar->use;i++) {
        uu = (sstr*)ssar->slot[i];
        fprintf(ofp, "ss %2d: val '%s' opt %d\n",
            i, uu->ssval, uu->ssopt);
    }

    return 0;
}

int
ss_sprintf(char*dst, int dlen, void* xv, int opt)
{
    int ik;
    sstr *sv;

    if(!xv) return -1;
    sv = (sstr*)xv;
    ik = sprintf(dst, "<'%s' %x>", sv->ssval, sv->ssopt);

    return ik;
}
