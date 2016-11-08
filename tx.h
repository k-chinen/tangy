/*
 * tx.h - text entry data structure and its procedures
 */
#ifndef _TX_H_
#define _TX_H_

#ifndef TXELEN  
#define TXELEN      (32)
#endif

#define TXE_CMD         ('o')
#define TXE_DATA        ('d')
#define TXE_CONST       ('c')
#define TXE_VARIABLE    ('v')

typedef struct {
    int   ct;
    int   st;
    char  cs[TXELEN];
    char *vs;
} txe;

int txe_sprint(char *dst, int dlen, void *rv, int dmy);
txe *txe_new(int xnt, char *xns);
int txe_parse(varray_t *ar, char *fs);
int txe_release(varray_t *ar);

#endif/*_TX_H_*/
