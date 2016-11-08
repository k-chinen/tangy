### 
### temporary Makefile for tangy
###
tangy: a.c alist.h chas.c chs.c color.h epsdraw.c finalize.c forkchk.c \
    linkchk.c nss.c parse.c picdraw.c put.c varray.h \
	alist.c color.c varray.c tx.c qbb.c
	cc -o tangy a.c alist.c color.c varray.c -lm

clean:
	$(RM) tangy *.o

# a.c: alist.h chas.c chs.c color.h epsdraw.c finalize.c forkchk.c \
#     linkchk.c nss.c parse.c picdraw.c put.c varray.h 
# alist.c: alist.h 
# color.c: color.h varray.h 
# epsdraw.c: qbb.c tx.c 
# tx.c: varray.h 
# varray.c: varray.h 
