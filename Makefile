### 
### temporary Makefile for tangy
###
CFLAGS=-g -O0

all: tangy 
tangy: a.c alist.h chas.c chs.c color.h epsdraw.c finalize.c forkchk.c \
    linkchk.c nss.c parse.c picdraw.c put.c varray.h \
	alist.c color.c varray.c seg.c notefile.c \
    tx.o qbb.o
	cc $(CFLAGS) -o tangy -DGITCHASH='"'`git show -s --format=%H`'"' \
	a.c tx.o qbb.o alist.c color.c varray.c -lm

clean:
	$(RM) tangy *.o

tx.o: tx.h varray.h
qbb.o: qbb.h 

# a.c: alist.h chas.c chs.c color.h epsdraw.c finalize.c forkchk.c \
#     linkchk.c nss.c parse.c picdraw.c put.c varray.h 
# alist.c: alist.h 
# color.c: color.h varray.h 
# epsdraw.c: qbb.c tx.c 
# tx.c: varray.h 
# varray.c: varray.h 
