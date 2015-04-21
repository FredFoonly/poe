
void init_markstack(void);
void shutdown_markstack(void);


// push/pop marks
MARK markstack_push(void);
POE_ERR markstack_pop(void);
MARK markstack_current(void);


MARK markstack_hittest_point(BUFFER buf, int row, int col, int flags_mask, int flags_chk);
MARK markstack_hittest_line(BUFFER buf, int row, int flags_mask, int flags_chk);

POE_ERR markstack_cur_get_buffer(BUFFER* buf);
POE_ERR markstack_cur_get_type(enum marktype* typ);
POE_ERR markstack_cur_get_start(int* line, int* col);
POE_ERR markstack_cur_get_end(int* line, int* col);
POE_ERR markstack_cur_get_bounds(enum marktype *typ, int* l1, int* c1, int* l2, int* c2);

POE_ERR markstack_cur_unmark(void);

POE_ERR markstack_cur_place(enum marktype typ, BUFFER buf, int line, int col);
POE_ERR markstack_cur_start(enum marktype typ, BUFFER buf, int line, int col);
POE_ERR markstack_cur_extend(enum marktype typ, BUFFER buf, int line, int col);
