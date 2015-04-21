
extern MARK INVALID_MARK;

#define MARK_FLG_VISIBLE (1<<0)
#define MARK_FLG_BOOKMARK (1<<1)


void init_marks(void);
void shutdown_marks(void);

MARK mark_alloc(int flags);
void mark_free(MARK mark);
void mark_unmark(MARK mark);
void mark_must_exist(const char* dbgstr, MARK mark);
int mark_exists(MARK mark);
int marks_count(void);
void mark_free_marks_in_buffer(BUFFER buf);

void mark_setflags(MARK mark, int flags);
void mark_clrflags(MARK mark, int flags);
int mark_tstflags(MARK mark, int flags);

POE_ERR mark_get_buffer(MARK mark, BUFFER* buf);
POE_ERR mark_get_type(MARK mark, enum marktype* typ);
POE_ERR mark_get_start(MARK mark, int * line, int* col);
POE_ERR mark_get_end(MARK mark, int * line, int* col);
POE_ERR mark_get_bounds(MARK mark, enum marktype *typ, int* l1, int* c1, int* l2, int* c2);

POE_ERR mark_bookmark(MARK mark, enum marktype typ, BUFFER buf, int line, int col);
POE_ERR mark_move_bookmark(MARK mark, enum marktype typ, int line, int col);
POE_ERR mark_get_bookmark(MARK mark, enum marktype typ, int *pline, int* pcol);

POE_ERR mark_place(MARK mark, enum marktype typ, BUFFER buf, int line, int col);
POE_ERR mark_start(MARK mark, enum marktype typ, BUFFER buf, int line, int col);
POE_ERR mark_extend(MARK mark, enum marktype typ, BUFFER buf, int line, int col);


bool mark_hittest_point(MARK mark, BUFFER buf, int row, int col, int flags_mask, int flags_chk);
bool mark_hittest_line(MARK mark, BUFFER buf, int row, int flags_mask, int flags_chk);

MARK marks_hittest_point(BUFFER buf, int row, int col, int flags_mask, int flags_chk);
MARK marks_hittest_line(BUFFER buf, int row, int flags_mask, int flags_chk);

// Update the marks with possible positional changes
void marks_upd_insertedlines(BUFFER buf, int line, int lines_inserted);
void marks_upd_removedlines(BUFFER buf, int line, int lines_removed);

void marks_upd_insertedchars(BUFFER buf, int line, int col, int chars_inserted);
void marks_upd_removedchars(BUFFER buf, int line, int col, int chars_deleted);

void marks_upd_split(BUFFER buf, int line, int col);
void marks_upd_join(BUFFER buf, int line, int col);

void marks_upd_insertedcharblk(BUFFER buf, int line, int col, int lines_inserted, int leading_chars_inserted, int trailing_chars_inserted);
void marks_upd_removedcharblk(BUFFER buf, int line, int col, int lines_removed, int leading_chars_deleted, int trailing_chars_deleted);
