#ifndef _TANGY_WORD_H_
#define _TANGY_WORD_H_
char *skipwhite(char *p);
char *chomp(char *line);
int dellastcharif(char *src, int xch);
char *draw_wordDQ(char *src, char *dst, int wlen);
char *draw_wordW(char *src, char *dst, int wlen);
char *draw_word(char *src, char *dst, int wlen, int sep);
#endif/*_TANGY_WORD_H_*/
