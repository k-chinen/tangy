seg *seg_new();
int try_regsegclose(varray_t *segar);
int try_regline(varray_t *segar, int x1, int y1, int x2, int y2);
int try_regsegmove(varray_t *segar, int x1, int y1);
int try_regsegrmove(varray_t *segar, int x1, int y1);
int try_regsegrline(varray_t *segar, int x1, int y1);
int try_regsegforward(varray_t *segar, int x1, int y1);
int try_regsegarc(varray_t *segar, int rad, int ang);
int try_regsegarcn(varray_t *segar, int rad, int ang);
