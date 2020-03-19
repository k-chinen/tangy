### 
### temporary Makefile for tangy
###
CFLAGS=-g -O0

OBJS= alist.o varray.o vdict.o word.o \
   tx.o qbb.o bez.o color.o \
   font.o seg.o xns.o \
   xcur.o chs.o chas.o nss.o \
   gv.o epsdraw.o notefile.o \
   put.o finalize.o linkchk.o parse.o a.o tangy.o

all: tangy 
tangy: a.c $(OBJS)
	cc $(CFLAGS) -o tangy $(OBJS) -lm -liconv

clean:
	$(RM) tangy *.o

##### dependency list 
a.o: a.h alist.h chas.h color.h epsdraw.h font.h geom.h gv.h \
    notefile.h obj.h qbb.h seg.h varray.h 
alist.o: alist.h 
bez.o: qbb.h 
chas.o: chas.h obj.h 
chs.o: a.h chas.h gv.h notefile.h obj.h seg.h 
color.o: color.h varray.h 
epsdraw.o: a.h alist.h bez.h chas.h epsdraw.h font.h gv.h notefile.h \
    obj.h put.h qbb.h seg.h tx.h vdict.h word.h xcur.h xlink.c xns.h 
finalize.o: a.h alist.h chas.h gv.h notefile.h obj.h seg.h 
font.o: alist.h font.h 
gv.o: gv.h obj.h 
linkchk.o: chas.h gv.h obj.h 
notefile.o: color.h gv.h notefile.h obj.h varray.h word.h 
nss.o: alist.h gv.h obj.h 
parse.o: a.h alist.h chas.h gv.h notefile.h obj.h seg.h varray.h \
    word.h 
put.o: a.h alist.h chas.h gv.h notefile.h obj.h seg.h xcur.h 
qbb.o: qbb.h 
seg.o: gv.h obj.h seg.h 
tangy.o: a.h alist.h chas.h color.h epsdraw.h font.h geom.h gv.h \
    notefile.h obj.h qbb.h seg.h varray.h 
tx.o: tx.h varray.h 
varray.o: varray.h 
vdict.o: alist.h vdict.h 
word.o: word.h 
xcur.o: a.h alist.h bez.h chas.h font.h gv.h notefile.h obj.h put.h \
    seg.h tx.h word.h xns.h 
xns.o: chas.h gv.h obj.h seg.h word.h xns.h 
