#ifndef _SSTR_H_
#define _SSTR_H_

int ss_strip(char *dst, int dlen, varray_t *ssar);
int ss_dump(FILE *ofp, varray_t *ssar);
int ss_sprintf(char*dst, int dlen, void* xv, int opt);

#endif/*_SSTR_H_*/
