### 
### temporary Makefile for tangy
###
CFLAGS=-g -O0

OBJS=tx.o qbb.o alist.o color.o varray.o font.o seg.o xns.o word.o put.o finalize.o gv.o epsdraw.o parse.o a.o tangy.o chs.o bez.o xcur.o

all: tangy 
tangy: a.c $(OBJS)
	cc $(CFLAGS) -o tangy $(OBJS) -lm

clean:
	$(RM) tangy *.o


a.o: a.h alist.h chas.c chas.h color.h epsdraw.h font.h geom.h \
    gv.h notefile.h nss.c obj.h qbb.h seg.h varray.h 
alist.o: alist.h 
chas.o: chas.h 
color.o: color.h varray.h 
epsdraw.o: a.h alist.h chas.h epsdraw.h font.h gv.h notefile.h obj.h \
    put.h seg.h tx.h word.h xns.h 
finalize.o: a.h alist.h chas.h gv.h notefile.h obj.h seg.h 
font.o: alist.h font.h 
gv.o: gv.h obj.h 
notefile.o: notefile.h 
parse.o: a.h alist.h chas.h gv.h notefile.h obj.h seg.h varray.h \
    word.h 
put.o: a.h alist.h chas.h gv.h notefile.h obj.h seg.h xcur.c 
qbb.o: qbb.h 
seg.o: gv.h obj.h seg.h 
tangy.o: a.h alist.h chas.h color.h epsdraw.h font.h linkchk.c geom.h \
    gv.h notefile.c notefile.h obj.h picdraw.c qbb.h seg.h varray.h 
tx.o: tx.h varray.h 
varray.o: varray.h 
word.o: word.h 
xns.o: chas.c chas.h chs.c gv.h nss.c obj.h seg.h word.h xns.h 
