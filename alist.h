/*
 * Copyright (C) 2002-2009 StarBED Project All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * assoc. list
 *  by k-chinen@jaist.ac.jp, 2003-2007
 *  
 *  $Id$
 */

#ifndef _ALIST_H_
#define  _ALIST_H_

typedef struct {
  char *name;
  int value;
} apair_t;

/* search by name */
int assoc_f(apair_t *al, char *xname);  /* first match */
int assoc_l(apair_t *al, char *xname);  /* last match */

#define assoc   assoc_f

/* search by value */
void* xrassoc_f(apair_t *al, int xval); /* first match */
void* xrassoc_l(apair_t *al, int xval); /* last match */
char* rassoc(apair_t *al, int xval);    /* first match */

#define xrassoc xrassoc_f

int al_count(apair_t *ax);
apair_t *al_dup(apair_t *ax);


#endif /* _ALIST_H_ */
