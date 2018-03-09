#include <stdio.h>
#include "word.h"

char*
skipwhite(char *p)
{
    while(*p==' ' ||*p=='\t') {
        p++;
    }
    return p;
}

char*
chomp(char *line)
{
    char *p;

    if(line==NULL) {
        return NULL;
    }

    p = line;
    while(*p) {
        if(*p=='\r' && *(p+1)=='\n' && *(p+2)=='\0') {
            *p = '\0';
            break;
        }
        if(*p=='\n' && *(p+1)=='\0') {
            *p = '\0';
            break;
        }
        p++;
    }

    return line;
}

int
dellastcharif(char *src, int xch)
{
    char *p, *q;

    q = NULL;
    p = src;
    while(*p) {
        q = p;
        p++;
    }

#if 0
    printf("%s: ", __func__);
    printf(" b src |%s| -> ", src);
#endif
    if(q && *q==xch) *q='\0';
#if 0
    printf(" a src |%s|\n", src);
#endif
    return 0;
}

/* double quoted string */
char*
draw_wordDQ(char *src, char *dst, int wlen)
{
    register char *p, *q;
    int c;

    p = src;
    q = dst;
    c = 0;
    if(*p!='"') {
        return NULL;
    }
    p++;
    while(*p&&c<wlen&&(*p!='"')) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    if(*p=='"') {
        p++;
    }

    return p;
}


char*
draw_wordW(char *src, char *dst, int wlen)
{
    register char *p, *q;
    int c;

    p = src;
    q = dst;
    c = 0;
    while(*p&&c<wlen&&(*p!=' ' && *p!='\t' && *p!='\n')) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    while(*p==' '||*p=='\t'|| *p=='\n') {
        p++;
    }

    return p;
}

char*
draw_word(char *src, char *dst, int wlen, int sep)
{
    register char *p, *q;
    int c;

    p = src;
    q = dst;
    c = 0;
    while(*p&&c<wlen&&(unsigned char)*p!=(unsigned char)sep) {
        *q++ = *p++;
        c++;
    }
    *q = '\0';
    if((unsigned char)*p==(unsigned char)sep) {
        p++;
    }

    return p;
}
