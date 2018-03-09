### 
### temporary Makefile for tangy
###
#CFLAGS=-g -O0 -DDO_FORKCHK -DDO_LINKCHK
#CFLAGS=-g -O0 -DDO_LINKCHK
CFLAGS=-g -O0 -DDO_FORKCHK

OBJS=tx.o qbb.o alist.o color.o varray.o font.o seg.o xns.o word.o put.o finalize.o gv.o epsdraw.o parse.o a.o tangy.o

all: tangy 
tangy: a.c $(OBJS)
	cc $(CFLAGS) -o tangy -DGITCHASH='"'`git show -s --format=%H`'"' \
   		$(OBJS) -lm

clean:
	$(RM) tangy *.o

#a.c: alist.h chas.c chs.c color.h epsdraw.c finalize.c forkchk.c \
#    geom.h linkchk.c notefile.c nss.c parse.c picdraw.c put.c qbb.h \
#    seg.c varray.h 

#epsdraw.o: tx.h 

put.c: a.h alist.h chas.h gv.h notefile.h seg.h xcur.c 
gv.o: gv.c

alist.o: alist.h 
color.o: color.h varray.h 
qbb.o: qbb.h 
seg.o: seg.h
tx.o: tx.h varray.h 
varray.o: varray.h 

