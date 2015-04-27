
#define LINE_FLG_DIRTY      (1<<0)
#define LINE_FLG_VISIBLE    (1<<1)
#define LINE_FLG_LF         (1<<2)
#define LINE_FLG_CR         (1<<3)
#define LINE_FLG_ANNOTATION (1<<4)
#define LINE_FLG_RSVD1      (1<<5)
#define LINE_FLG_RSVD2      (1<<6)
#define LINE_FLG_RSVD3      (1<<7)

#define BUF_FLG_DIRTY       (1<<0)
#define BUF_FLG_VISIBLE     (1<<1)
#define BUF_FLG_INTERNAL    (1<<2)
#define BUF_FLG_CMDLINE     (1<<3)
#define BUF_FLG_RDONLY      (1<<4)
#define BUF_FLG_NEW         (1<<5)


typedef unsigned short int line_flags_t;
struct line_t {
  struct cstr_t txt;
  line_flags_t flags;
};
typedef struct line_t LINE;


typedef int (*char_test)(char c);

void init_buffer(void);
void shutdown_buffer(void);

int buffers_count(void);
int visible_buffers_count(void);
BUFFER buffers_next(BUFFER buf);
void buffers_switch_profiles(PROFILEPTR newprofile, PROFILEPTR oldprofile);

BUFFER buffer_alloc(const char* buffer_name, int flags, int capacity,
					PROFILEPTR profile);
void buffer_free(BUFFER buf);
void buffer_must_exist(const char* dbgstr, BUFFER buf);
bool buffer_exists(BUFFER buf);
int buffer_count(BUFFER buf);
int buffer_capacity(BUFFER buf);
const char* buffer_filename(BUFFER buf);
const char* buffer_name(BUFFER buf);
const char* buffer_curr_dirname(BUFFER buf);

void buffer_setflags(BUFFER buf, int flg);
void buffer_clrflags(BUFFER buf, int flg);
bool buffer_tstflags(BUFFER buf, int flg);

POE_ERR buffer_setmargins(BUFFER buf, int leftmargin, int rightmargin, int paragraph);
void buffer_getmargins(BUFFER buf, int* pleftmargin, int* prightmargin, int* pparagraph);
void buffer_gettabs(BUFFER buf, tabstops* tabs);
void buffer_settabs(BUFFER buf, tabstops* tabs);

PROFILEPTR buffer_get_profile(BUFFER buf);
void buffer_set_profile(BUFFER buf, PROFILEPTR profile);

int buffer_nexttab(BUFFER buf, int col);
int buffer_prevtab(BUFFER buf, int col);
void buffer_setlineflags(BUFFER pbuf, int line, int flags);
void buffer_setlinesflags(BUFFER pbuf, int line, int n, int flags);
void buffer_clrlineflags(BUFFER pbuf, int line, int flags);
int buffer_tstlineflags(BUFFER pbuf, int line, int flags);
int buffer_line_length(BUFFER buf, int line);
struct line_t* buffer_get(BUFFER buf, int line);
int buffer_scantill_nowrap(BUFFER buf, int line, int col, int direction, char_test testf);
bool buffer_isblankline(BUFFER buf, int line);
int buffer_findblankline(BUFFER buf, int line, int direction);
bool buffer_isparagraphsep(BUFFER buf, int line);
int buffer_findparagraphsep(BUFFER buf, int line, int direction);
int buffer_findnonparagraphsep(BUFFER buf, int line, int direction);
bool buffer_right_wrap(BUFFER buf, int* pline, int* pcol);
bool buffer_left_wrap(BUFFER buf, int* pline, int* pcol);
void buffer_scantill_wrap(BUFFER buf, int* pline, int* pcol, int direction, char_test testf);
bool buffer_trimleft(BUFFER buf, int line, bool upd_marks);
bool buffer_trimright(BUFFER buf, int line, bool upd_marks);
char buffer_getchar(BUFFER buf, int line, int col);
void buffer_setchar(BUFFER buf, int line, int col, char c);
void buffer_setct(BUFFER buf, int line, int col, char c, int n);
void buffer_setcharct(BUFFER buf, int line, int col, char c, int ct);
int buffer_appendline(BUFFER buf, struct line_t* a);
int buffer_appendblanklines(BUFFER buf, int n);
void buffer_ensure_min_lines(BUFFER buf, bool upd_dirty);
void buffer_setcstr(BUFFER buf, int line, struct cstr_t* a);
void buffer_setstrn(BUFFER buf, int line, int col, const char* s, int n, bool upd_marks);
void buffer_insertblanklines(BUFFER buf, int line, int nlines, bool upd_marks);
void buffer_removelines(BUFFER buf, int line, int n, bool upd_marks);
void buffer_insert(BUFFER buf, int line, int col, char c, bool upd_marks);
void buffer_insertct(BUFFER buf, int line, int col, char c, int ct, bool upd_marks);
void buffer_insertstrn(BUFFER buf, int line, int col, const char* s, int n, bool upd_marks);
const char* buffer_getbufptr(BUFFER buf, int line);
const char* buffer_getcharptr(BUFFER buf, int line, int col);
void buffer_removechar(BUFFER buf, int line, int col, bool upd_marks);
void buffer_removechars(BUFFER buf, int line, int col, int n, bool upd_marks);
void buffer_upperchars(BUFFER buf, int line, int col, int n);
void buffer_lowerchars(BUFFER buf, int line, int col, int n);
POE_ERR buffer_copyinsertchars(BUFFER dstbuf, int dstline, int dstcol,
                               BUFFER srcbuf, int srcline, int srccol,
                               int nchars, bool upd_marks);
POE_ERR buffer_copyinsertlines(BUFFER dstbuf, int dstline,
                               BUFFER srcbuf, int srcline,
                               int nlines, bool upd_marks);
POE_ERR buffer_copyoverlaychars(BUFFER dstbuf, int dstline, int dstcol,
                                BUFFER srcbuf, int srcline, int srccol,
                                int nchars, bool upd_marks);
POE_ERR buffer_copyoverlaylines(BUFFER dstbuf, int dstline,
                                BUFFER srcbuf, int srcline,
                                int nlines, bool upd_marks);

POE_ERR buffer_splitline(BUFFER buf, int row, int col, bool update_marks);
POE_ERR buffer_joinline(BUFFER buf, int row, bool update_marks);

POE_ERR buffer_load(BUFFER dst, cstr* filename, bool tabexpand);
POE_ERR buffer_save(BUFFER dst, cstr* filename, bool blankcompress);

bool buffer_wrap_line(BUFFER dst,
					  int row, int lastrow,
					  int pmargin, int lmargin, int rmargin,
					  bool update_marks);

int buffer_respace(BUFFER buf, int line, char_pred_t spacepred, bool upd_marks);

bool buffer_search(BUFFER buf, int* row, int* col, int* endcol,
				   const cstr* pat, bool exact, int direction);

BUFFER buffers_find_named(cstr* name);
BUFFER buffers_find_eithername(cstr* name);

