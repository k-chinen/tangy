#ifndef VARRAY_T
#define VARRAY_T


#define VARRAY_OPT_EXPANDALLOW          (0x0800)
#define VARRAY_OPT_EXPANDSQUARE         (0x0400)
#define VARRAY_OPT_EXPANDTHREESECONDS   (0x0200)
#define VARRAY_OPT_EXPANDDOUBLE         (0x0100)

typedef struct {
    int    use;
    int    len;
    void** slot;

    int    opt;

    int    userint1;
    void*  uservoid1;

    int (*sprintfunc)(char*, int, void*, int);
#if 0
    int (*purgefunc)(void*);
#endif
    int (*mempurgefunc)(void*);

} varray_t;


varray_t*   varray_new();
varray_t*   varray_clone();
int         varray_expand(varray_t *dict);
int         varray_push(varray_t *dict, void *value);
void*       varray_pop(varray_t *dict);
void*       varray_top(varray_t *dict);
void*       varray_findlast(varray_t *dict);
int         varray_member(varray_t *dict);

#define varray_add(d,v)     varray_push(d,v)
#define varray_del(d)       (void)varray_pop(d,v)
#define varray_last(d)      (void)varray_top(d,v)

int varray_entrysprintfunc(varray_t *ar, int(*subf)(char*,int,void*,int));
int varray_entrymempurgefunc(varray_t *ar, int(*subf)(void*));
#if 0
int varray_entrypurgefunc(varray_t *ar, int(*subf)(char*.int,void*,int));
#endif

int varray_dump(varray_t*ar);
int varray_fprint(FILE *fp, varray_t*ar);


#endif /* VARRAY_T */
