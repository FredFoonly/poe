
#if !defined(__OpenBSD__) && !defined(__FreeBSD__)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/errno.h>
#if defined(__OpenBSD__) || defined(__FreeBSD__)
#include <libgen.h>
#endif
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "trace.h"
#include "logging.h"
#include "poe_err.h"
#include "poe_exit.h"
#include "utils.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "tabstops.h"
#include "margins.h"
#include "mark.h"
#include "markstack.h"
#include "key_interp.h"
#include "buffer.h"
#include "editor_globals.h"


#define BUF_SIG (0xCAFEBABE)


#define LINE_INITIAL_FLAGS (0|LINE_FLG_VISIBLE)

struct buffer_t {
  int _sig;
  int bufnum;
  struct vec_t lines;
  int flags;
  cstr orig_filename;
  cstr curr_filename;
  cstr base_buffername;
  cstr buffername;
  cstr orig_dirname;
  cstr curr_dirname;
  // margins
  margins margins;
  // tabstops
  tabstops tabstops;
  // current longest line length
  int longest_line;
  // key -> command map
  PROFILEPTR profile;
};


struct line_t _blankline;

int _next_bufnum;


void _buffer_init(BUFFER buf, const char* buffer_name, int flags, int capacity,
                  PROFILEPTR profile);
void _buffer_free(BUFFER buf);
void _buffer_destroy(BUFFER buf);
void __buffer_copyinsertlines(BUFFER dstbuf, int di,
                              BUFFER srcbuf, int si,
                              int n);
bool _buffer_wrap_single_line(BUFFER buf, int row, int leftmargin, int rightmargin, bool update_marks);

cstr _buffer_make_unique_name(cstr* name);
int _buffer_name_exists(cstr* name);
void _buffer_updatefilename(BUFFER buf);

struct line_t* _line(BUFFER buf, int line);
void __check_line_exists(const char* dbgname, BUFFER buf, int line);
void __check_line_col_exists(const char* dbgname, BUFFER buf, int line, int col);
void __check_line_lim(const char* dbgname, BUFFER buf, int line);
void __check_lines_exist(const char* dbgname, BUFFER buf, int startline, int nlines);
int __find_buffer(BUFFER buf);

void _expand_to_line(BUFFER buf, int line);
void _expand_to_col(BUFFER buf, int line, int col);


void __line_init(struct line_t* l)
{
  TRACE_ENTER;
  cstr_init(&l->txt, 1);
  l->flags = LINE_INITIAL_FLAGS;
  TRACE_EXIT;
}


void __line_initfrom(struct line_t* l, struct line_t* src)
{
  TRACE_ENTER;
  cstr_initfrom(&l->txt, &src->txt);
  l->flags = src->flags;
  TRACE_EXIT;
}


void __line_destroy(struct line_t* l)
{
  TRACE_ENTER;
  cstr_destroy(&l->txt);
  l->flags = LINE_INITIAL_FLAGS;
  TRACE_EXIT;
}




//
// vector of lines
//
static struct pivec_t _all_buffers;

extern int _next_bufnum;



void init_buffer()
{
  TRACE_ENTER;
  _next_bufnum = 1;
  __line_init(&_blankline);
  pivec_init(&_all_buffers, 50);
  TRACE_EXIT;
}


void shutdown_buffer()
{
  TRACE_ENTER;
  int i, n = pivec_count(&_all_buffers);
  for (i = 0; i < n; i++) {
    BUFFER buf = (BUFFER)pivec_get(&_all_buffers, i);
    _buffer_free(buf);
  }
  pivec_destroy(&_all_buffers);
  TRACE_EXIT;
}


int buffers_count()
{
  TRACE_ENTER;
  int rval = pivec_count(&_all_buffers);
  TRACE_RETURN(rval);
}


int visible_buffers_count()
{
  TRACE_ENTER;
  int i, n = pivec_count(&_all_buffers);
  int visible_count = 0;
  for (i = 0; i < n; i++) {
    BUFFER buf = (BUFFER)pivec_get(&_all_buffers, i);
    if (buffer_tstflags(buf, BUF_FLG_VISIBLE))
      ++visible_count;
  }
  TRACE_RETURN(visible_count);
}


BUFFER buffers_next(BUFFER buf)
{
  TRACE_ENTER;
  BUFFER nextbuf = BUFFER_NULL;
  int i = __find_buffer(buf);
  int start = i;
  int nbuffers = buffers_count();
  if (nbuffers > 0 || i < 0) {
		if (buf == BUFFER_NULL)
			i = nbuffers-1;
		// keep scanning until we find a visible buffer.
		do {
			i = (i+1) % nbuffers;
			nextbuf = (BUFFER)pivec_get(&_all_buffers, i);
    } while (!buffer_tstflags(nextbuf, BUF_FLG_VISIBLE) && i != start);
  }
  TRACE_RETURN(nextbuf);
}


void buffers_switch_profiles(PROFILEPTR newprofile, PROFILEPTR oldprofile)
{
  TRACE_ENTER;
  int i, n = buffers_count();
  for (i = 0; i < n; i++) {
    BUFFER buf = (BUFFER)pivec_get(&_all_buffers, i);
    if (buffer_get_profile(buf) == oldprofile)
      buffer_set_profile(buf, newprofile);
  }
  TRACE_EXIT;
}


void _validatebufptr(const char* dbg, BUFFER buf)
{
  if (buf == NULL) {
    poe_err(1, "%s Attempted to use a null buffer.", dbg);
  }
  if (buf->_sig != BUF_SIG) {
    poe_err(1, "%s Attempted to use an invalid buffer.", dbg);
  }
}


#ifdef POE_DBG_BUFPTRS
#define VALIDATEBUFFER(pm) _validatebufptr(__func__, pm)
#else
#define VALIDATEBUFFER(pm) /**/
#endif


BUFFER buffer_alloc(const char* buffer_name, int flags, int capacity,
                    PROFILEPTR profile)
{
  TRACE_ENTER;
  BUFFER buf = (BUFFER)calloc(1, sizeof(struct buffer_t));
  _buffer_init(buf, buffer_name, flags, capacity, profile);
  pivec_append(&_all_buffers, (intptr_t)buf);
  VALIDATEBUFFER(buf);
  TRACE_RETURN(buf);
}


void buffer_free(BUFFER buf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int i = __find_buffer(buf);
  if (i < 0)
    poe_err(POE_ERR_NOT_FOUND, "%s: buffer %d not found", __func__, buf->bufnum);
  pivec_remove(&_all_buffers, i);
  _buffer_free(buf);
  TRACE_EXIT;
}


void _buffer_free(BUFFER buf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  _buffer_destroy(buf);
  free(buf);
  TRACE_EXIT;
}


bool buffer_exists(BUFFER buf)
{
  TRACE_ENTER;
  if (buf == NULL)
    TRACE_RETURN(false);
  int i = __find_buffer(buf);
  TRACE_RETURN(i >= 0 && buf->_sig == BUF_SIG);
}


void buffer_must_exist(const char* dbgstr, BUFFER buf)
{
  TRACE_ENTER;
  if (!buffer_exists(buf))
    poe_err(1, "%s error: buffer %d not found", dbgstr, buf->bufnum);
  VALIDATEBUFFER(buf);
  TRACE_EXIT;
}



void _buffer_init(BUFFER buf, const char* buffer_name, int flags, int capacity,
                  PROFILEPTR profile)
{
  TRACE_ENTER;
  capacity = max(capacity, 0);
  vec_init(&buf->lines, capacity, sizeof(struct line_t));
  buf->_sig = BUF_SIG;
  buf->bufnum = _next_bufnum++;
  buf->flags = flags;
  cstr_init(&buf->orig_filename, 0);
  cstr_init(&buf->curr_filename, 0);
  cstr_initstr(&buf->base_buffername, buffer_name);
  cstr_initstr(&buf->buffername, buffer_name);
  cstr_init(&buf->orig_dirname, 0);
  char tmp_cwd[PATH_MAX+1];
  getcwd(tmp_cwd, sizeof(tmp_cwd));
  cstr_initstr(&buf->curr_dirname, tmp_cwd);
  if (profile == NULL) {
    margins_init(&buf->margins, 0, 255, 0);
    tabs_init(&buf->tabstops, 0, 8, NULL);
  }
  else {
    margins_initfrom(&buf->margins, &profile->default_margins);
    tabs_initfrom(&buf->tabstops, &profile->default_tabstops);
  }
  buf->longest_line = 0;
  /* if (flags & BUF_FLG_CMDLINE) */
  /*   buf->profile = dflt_cmd_profile; */
  /* else */
  buf->profile = profile;
  VALIDATEBUFFER(buf);
  TRACE_EXIT;
}


void _buffer_destroy(BUFFER buf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  markstack_pop_marks_in_buffer(buf);
  mark_free_marks_in_buffer(buf);
  int i ,n = vec_count(&buf->lines);
  for (i = 0; i < n; i++) {
    __line_destroy(_line(buf, i));
  }
  vec_destroy(&buf->lines);
  cstr_destroy(&buf->orig_filename);
  cstr_destroy(&buf->curr_filename);
  cstr_destroy(&buf->base_buffername);
  cstr_destroy(&buf->buffername);
  tabs_destroy(&buf->tabstops);
  buf->_sig = 0;
  TRACE_EXIT;
}


int buffer_count(BUFFER buf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int rval = vec_count(&buf->lines);
  TRACE_RETURN(rval); 
}


int buffer_capacity(BUFFER buf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int rval = vec_capacity(&buf->lines);
  TRACE_RETURN(rval);
}


const char* buffer_name(BUFFER buf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  const char* rval = cstr_getbufptr(&buf->buffername);
  TRACE_RETURN(rval);
}


const char* buffer_curr_dirname(BUFFER buf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  const char* rval = cstr_getbufptr(&buf->curr_dirname);
  TRACE_RETURN(rval);
}


bool buffer_chdir(BUFFER buf, const cstr* new_dirname)
{
  TRACE_ENTER;
  bool rval = true;
  cstr_assign(&buf->curr_dirname, new_dirname);
  buffer_setflags(buf, BUF_FLG_DIRTY);
  _buffer_updatefilename(buf);
  TRACE_RETURN(rval);
}


bool buffer_setbasename(BUFFER buf, const cstr* new_basename)
{
  TRACE_ENTER;
  bool rval = true;
  cstr_assign(&buf->base_buffername, new_basename);
  buffer_setflags(buf, BUF_FLG_DIRTY);
  _buffer_updatefilename(buf);
  TRACE_RETURN(rval);
}


void _buffer_updatefilename(BUFFER buf)
{
  cstr tmp;
  cstr_initfrom(&tmp, &buf->curr_dirname);
  cstr_append(&tmp, '/');
  cstr_appendcstr(&tmp, &buf->base_buffername);
  cstr_assign(&buf->curr_filename, &tmp);
  cstr_destroy(&tmp);
}
int _buffer_isnum(BUFFER buf, int bufnum)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int rval = buf->bufnum == bufnum;
  TRACE_RETURN(rval);
}


int _buffer_isnamed(BUFFER buf, cstr* name)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int rval = cstr_compare(&buf->buffername, name) == 0;
  TRACE_RETURN(rval);
}


int _buffer_isfilenamed(BUFFER buf, cstr* name)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int rval = cstr_compare(&buf->curr_filename, name) == 0;
  TRACE_RETURN(rval);
}


void buffer_setflags(BUFFER buf, int flags)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  buf->flags |= flags;
  TRACE_EXIT;
}


void buffer_clrflags(BUFFER buf, int flags)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  buf->flags &= ~flags;
  TRACE_EXIT;
}


bool buffer_tstflags(BUFFER buf, int flags)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int rval = (buf->flags & flags) == flags;
  TRACE_RETURN(rval);
}


POE_ERR buffer_setmargins(BUFFER buf, int leftmargin, int rightmargin, int paragraph)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  POE_ERR rval = margins_set(&buf->margins, leftmargin, rightmargin, paragraph);
  TRACE_RETURN(rval);
}


void buffer_getmargins(BUFFER buf, int* pleftmargin, int* prightmargin, int* paragraph)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  margins_get(&buf->margins, pleftmargin, prightmargin, paragraph);
  TRACE_EXIT;
}


void buffer_gettabs(BUFFER buf, tabstops* tabs)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  tabs_assign(tabs, &buf->tabstops);
  TRACE_EXIT;
}


void buffer_settabs(BUFFER buf, tabstops* tabs)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  tabs_assign(&buf->tabstops, tabs);
  TRACE_EXIT;
}


PROFILEPTR buffer_get_profile(BUFFER buf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  PROFILEPTR profile = buf->profile;
  TRACE_RETURN(profile);
}


void buffer_set_profile(BUFFER buf, PROFILEPTR profile)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  buf->profile = profile;
  TRACE_EXIT;
}


int buffer_nexttab(BUFFER buf, int col)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int rval = tabs_next(&buf->tabstops, col);
  TRACE_RETURN(rval);
}


int buffer_prevtab(BUFFER buf, int col)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int rval = tabs_prev(&buf->tabstops, col);
  TRACE_RETURN(rval);
}


void buffer_setlineflags(BUFFER buf, int line, int flags)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  _line(buf, line)->flags |= flags;
  if (flags & LINE_FLG_DIRTY)
    buffer_setflags(buf, BUF_FLG_DIRTY);
  TRACE_EXIT;
}


void buffer_setlinesflags(BUFFER buf, int line, int n, int flags)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int i;
  __check_lines_exist(__func__, buf, line, n);
  for (i = 0; i < n; i++)
    _line(buf, line+i)->flags |= flags;
  if (flags & LINE_FLG_DIRTY)
    buffer_setflags(buf, BUF_FLG_DIRTY);
  TRACE_EXIT;
}


void buffer_clrlineflags(BUFFER buf, int line, int flags)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  _line(buf, line)->flags &= ~flags;
  TRACE_EXIT;
}


int buffer_tstlineflags(BUFFER buf, int line, int flags)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  int rval = (_line(buf, line)->flags & flags) == flags;
  TRACE_RETURN(rval);
}


int buffer_line_length(BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* pline = _line(buf, line);
  int rval = cstr_count(&pline->txt);
  TRACE_RETURN(rval);
}


struct line_t* buffer_get(BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* rval = _line(buf, line);
  TRACE_RETURN(rval);
}


bool buffer_isblankline(BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int i = buffer_scantill_nowrap(buf, line, 0, 1, poe_isnotwhitespace);
  int rval = i >= buffer_line_length(buf, line);
  if (!rval) {
    char c = buffer_getchar(buf, line, i);
    if (c == '.')
      rval = true;
  }
  TRACE_RETURN(rval);
}


int buffer_findblankline(BUFFER buf, int row, int direction)
{
  TRACE_ENTER;
  direction = (direction < 0) ? -1 : 1;
  int i=row, nrows = buffer_count(buf);
  if (row < 0 || row >= nrows)
    TRACE_RETURN(row);
  if (direction > 0) {
    for (i = row; i < nrows && !buffer_isblankline(buf, i); i++)
      ;
  }
  else if (direction < 0) {
    for (i = row; i > 0 && !buffer_isblankline(buf, i); i--)
      ;
  }
  TRACE_RETURN(i);
}


// paragraph separators are either blank lines, or lines beginning
// with '.' (roff command), or lines beginning with '<' (html tag).
bool buffer_isparagraphsep(BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int i = buffer_scantill_nowrap(buf, line, 0, 1, poe_isnotwhitespace);
  int rval = false;
  if (i >= buffer_line_length(buf, line)) {
    rval = true;
  }
  else {
    char c = buffer_getchar(buf, line, i);
    if (c == '.' || c == '<' || c == '>' || c == '*' || c == '/' || c == '+' || c == '-' || c == '[' || c == ']')
      rval = true;
  }
  TRACE_RETURN(rval);
}


int buffer_findparagraphsep(BUFFER buf, int row, int direction)
{
  TRACE_ENTER;
  direction = (direction < 0) ? -1 : 1;
  int i=row, nrows = buffer_count(buf);
  if (row < 0 || row >= nrows)
    TRACE_RETURN(row);
  if (direction > 0) {
    for (i = row; i < nrows && !buffer_isparagraphsep(buf, i); i++)
      ;
  }
  else if (direction < 0) {
    for (i = row; i > 0 && !buffer_isparagraphsep(buf, i); i--)
      ;
  }
  TRACE_RETURN(i);
}


int buffer_findnonparagraphsep(BUFFER buf, int row, int direction)
{
  TRACE_ENTER;
  direction = (direction < 0) ? -1 : 1;
  int i=row, nrows = buffer_count(buf);
  if (row < 0 || row >= nrows)
    TRACE_RETURN(row);
  if (direction > 0) {
    for (i = row; i < nrows && buffer_isparagraphsep(buf, i); i++)
      ;
  }
  else if (direction < 0) {
    for (i = row; i > 0 && buffer_isparagraphsep(buf, i); i--)
      ;
  }
  TRACE_RETURN(i);
}


int buffer_scantill_nowrap(BUFFER buf, int line, int col, int direction, char_test testf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* pline = _line(buf, line);
  int len = cstr_count(&pline->txt);
  const char* s = cstr_getbufptr(&pline->txt);
  int i = 0;
  if (direction == -1) {
    if (col < 0)
      TRACE_RETURN(-1);
    for (i = col; i >= 0 && !(*testf)(s[i]); i--)
      ;
  }
  else if (direction == 1) {
    if (col >= len)
      TRACE_RETURN(len);
    for (i = col; i < len && s[i] != '\0' && !(*testf)(s[i]); i++)
      ;
  }
  TRACE_RETURN(i);
}


bool buffer_left_wrap(BUFFER buf, int* pline, int* pcol)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int line = *pline, col = *pcol;
  __check_line_exists(__func__, buf, line);
  if (line == 0 && col == 0)
    TRACE_RETURN(false);
  if (col == 0) {
    line--;
    col = buffer_line_length(buf, line);
  }
  else {
    col--;
  }
  *pline = line;
  *pcol = col;
  TRACE_RETURN(true);
}


bool buffer_right_wrap(BUFFER buf, int* pline, int* pcol)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int line = *pline, col = *pcol;
  __check_line_exists(__func__, buf, line);
  int nlines = buffer_count(buf);
  int linelen = buffer_line_length(buf, line);
  if (line == nlines-1 && col >= linelen)
    TRACE_RETURN(false);
  if (col >= linelen) {
    line++;
    col = 0;
  }
  else {
    col++;
  }
  *pline = line;
  *pcol = col;
  TRACE_RETURN(true);
}


void buffer_scantill_wrap(BUFFER buf, int* pline, int* pcol, int direction, char_test testf)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int line = *pline, col = *pcol;
  __check_line_exists(__func__, buf, line);
  bool scan = true;
  if (direction == -1) {
    while (scan && !(*testf)(buffer_getchar(buf, line, col))) {
      scan = buffer_left_wrap(buf, &line, &col);
    }
  }
  else if (direction == 1) {
    while (scan && !(*testf)(buffer_getchar(buf, line, col))) {
      scan = buffer_right_wrap(buf, &line, &col);
    }
  }
  *pline = line;
  *pcol = col;
  TRACE_EXIT;
}


bool buffer_trimleft(BUFFER buf, int row, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, row);
  struct line_t* line = buffer_get(buf, row);
  int nchars = cstr_trimleft(&line->txt, poe_iswhitespace);
  if (upd_marks && nchars > 0)
    marks_upd_removedchars(buf, row, 0, nchars);
  buffer_setlineflags(buf, row, LINE_FLG_DIRTY);
  TRACE_RETURN(nchars > 0);
}


bool buffer_trimright(BUFFER buf, int row, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, row);
  struct line_t* line = buffer_get(buf, row);
  int nchars = cstr_trimright(&line->txt, poe_iswhitespace);
  if (upd_marks && nchars > 0)
    marks_upd_removedchars(buf, row, cstr_count(&line->txt), nchars);
  buffer_setlineflags(buf, row, LINE_FLG_DIRTY);
  TRACE_RETURN(nchars > 0);
}


void buffer_setcstr(BUFFER buf, int line, struct cstr_t* a)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  cstr_assign(&(_line(buf, line)->txt), a);
  buf->longest_line = max(buf->longest_line, cstr_count(a));
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  TRACE_EXIT;
}


void buffer_ensure_min_lines(BUFFER buf, bool upd_dirty)
{
  if (buffer_count(buf) == 0) {
    bool wasdirty = buffer_tstflags(buf, BUF_FLG_DIRTY);
    _expand_to_line(buf, 0);
    if (!upd_dirty && !wasdirty)
      buffer_clrflags(buf, BUF_FLG_DIRTY);
    else
      trace_stack_print();
  }
}


int buffer_appendline(BUFFER buf, struct line_t* a)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  struct line_t tmp;
  __line_initfrom(&tmp, a);
  int line = vec_append(&buf->lines, &tmp);
  buf->longest_line = max(buf->longest_line, cstr_count(&tmp.txt));
  // ownership of tmp's data moves to buffer
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  TRACE_RETURN(line);
}


int buffer_appendblanklines(BUFFER buf, int n)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int i, rval = 0;
  for (i = 0; i < n; i++)
    rval = buffer_appendline(buf, &_blankline);
  TRACE_RETURN(rval);
}


void buffer_insertline(BUFFER buf, int line, struct line_t* a)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_lim("_buffer_insertline", buf, line);
  struct line_t tmp;
  __line_initfrom(&tmp, a);
  vec_insert(&buf->lines, line, &tmp);
  // ownership of tmp's data moves to buffer
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  TRACE_EXIT;
}


void buffer_insertblanklines(BUFFER buf, int line, int nlines, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  if (nlines == 1) {
    buffer_insertline(buf, line, &_blankline);
    if (upd_marks)
      marks_upd_insertedlines(buf, line, 1);
  }
  else if (nlines > 1) {
    LINE* lines = calloc(nlines, sizeof(LINE));
    int i;
    for (i = 0; i < nlines; i++)
      __line_init(lines+i);
    vec_insertm(&buf->lines, line, nlines, lines);
    if (upd_marks)
      marks_upd_insertedlines(buf, line, nlines);
    buffer_setflags(buf, BUF_FLG_DIRTY); 
    free(lines);
  }
  TRACE_EXIT;
}


void buffer_removeline(BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  __line_destroy(_line(buf, line));
  vec_remove(&buf->lines, line);
  buffer_setflags(buf, LINE_FLG_DIRTY);
  TRACE_EXIT;
}


void buffer_removelines(BUFFER buf, int line, int n, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int j;
  __check_lines_exist(__func__, buf, line, n);
  if (upd_marks)
    marks_upd_removedlines(buf, line, n);
  for (j = 0; j < n; j++) {
    __line_destroy(_line(buf, line+j));
  }
  vec_removem(&buf->lines, line, n);
  buffer_setflags(buf, LINE_FLG_DIRTY);
  TRACE_EXIT;
}


void buffer_insert(BUFFER buf, int line, int col, char c, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* l = _line(buf, line);
  _expand_to_col(buf, line, col-1);
  cstr_insert(&l->txt, col, c);
  buf->longest_line = max(buf->longest_line, cstr_count(&l->txt));
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  if (upd_marks)
    marks_upd_insertedchars(buf, line, col, 1);
  TRACE_EXIT;
}


void buffer_insertct(BUFFER buf, int line, int col, char c, int ct, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* l = _line(buf, line);
  _expand_to_col(buf, line, col-1);
  cstr_insertct(&l->txt, col, c, ct);
  buf->longest_line = max(buf->longest_line, cstr_count(&l->txt));
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  if (upd_marks)
    marks_upd_insertedchars(buf, line, col, ct);
  TRACE_EXIT;
}


void buffer_insertstrn(BUFFER buf, int line, int col, const char* s, int n, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* l = _line(buf, line);
  int len = strnlen(s, n);
  _expand_to_col(buf, line, col-1);
  cstr_insertm(&l->txt, col, len, s);
  buf->longest_line = max(buf->longest_line, cstr_count(&l->txt));
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  if (upd_marks)
    marks_upd_insertedchars(buf, line, col, len);
  TRACE_EXIT;
}


char buffer_getchar(BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* l = _line(buf, line);
  char c;
  if (col >= cstr_count(&l->txt))
    c = ' ';
  else
    c = cstr_get(&l->txt, col);
  TRACE_RETURN(c);
}


void buffer_setchar(BUFFER buf, int line, int col, char c)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  _expand_to_col(buf, line, col);
  struct line_t* l = _line(buf, line);
  cstr_set(&l->txt, col, c);
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  TRACE_EXIT;
}


void buffer_setcharct(BUFFER buf, int line, int col, char c, int ct)
{
  TRACE_ENTER;
  __check_line_exists(__func__, buf, line);
  struct line_t* pline = _line(buf, line);
  int linelen = cstr_count(&pline->txt);
  if (col > linelen) {
    _expand_to_col(buf, line, col-1);
    cstr_appendct(&pline->txt, c, ct);
  }
  else if (col+ct > linelen) {
    cstr_setct(&pline->txt, col, c, linelen-col);
    cstr_appendct(&pline->txt, c, col+ct-linelen);
  }
  else {
    cstr_setct(&pline->txt, col, c, ct);
  }
  TRACE_EXIT;
}


void buffer_setstrn(BUFFER buf, int line, int col, const char* s, int n, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* l = _line(buf, line);
  int linelen = cstr_count(&l->txt);
  int len = strnlen(s, n);
  _expand_to_col(buf, line, col+len);
  cstr_setstrn(&l->txt, col, s, len);
  buf->longest_line = max(buf->longest_line, cstr_count(&l->txt));
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  int nadded = max(0, col+n-linelen);
  if (upd_marks && nadded > 0)
    marks_upd_insertedchars(buf, line, linelen, nadded);
  TRACE_EXIT;
}


void buffer_removechar(BUFFER buf, int line, int col, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* l = _line(buf, line);
  if (cstr_count(&l->txt) <= col) {
    TRACE_EXIT;
  }
  else {
    if (upd_marks)
      marks_upd_removedchars(buf, line, col, 1);
    cstr_remove(&l->txt, col);
    buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  }
  TRACE_EXIT;
}


void buffer_removechars(BUFFER buf, int line, int col, int n, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* l = _line(buf, line);
  int len = cstr_count(&l->txt);
  if (col >= len) {
    TRACE_EXIT;
  }
  else {
    if (upd_marks)
      marks_upd_removedchars(buf, line, col, n);
    cstr_removem(&l->txt, col, min(len-col, n));
    buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  }
  TRACE_EXIT;
}


const char* buffer_getbufptr(BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* l = buffer_get(buf, line);
  const char* rval = cstr_getbufptr(&l->txt);
  TRACE_RETURN(rval);
}


const char* buffer_getcharptr(BUFFER buf, int line, int col)
{
  __check_line_exists(__func__, buf, line);
  struct line_t* l = buffer_get(buf, line);
  if (col >= cstr_count(&l->txt))
    return "";
  else
    return cstr_getcharptr(&l->txt, col);
}


void buffer_upperchars(BUFFER buf, int line, int col, int n)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  if (line >= buffer_count(buf))
    TRACE_EXIT;
  struct line_t* l = _line(buf, line);
  cstr_upper(&l->txt, col, n);
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  TRACE_EXIT;
}


void buffer_lowerchars(BUFFER buf, int line, int col, int n)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  if (line >= buffer_count(buf))
    TRACE_EXIT;
  struct line_t* l = _line(buf, line);
  cstr_lower(&l->txt, col, n);
  buffer_setlineflags(buf, line, LINE_FLG_DIRTY);
  TRACE_EXIT;
}



//
// more buffer-oriented functions.
//

POE_ERR buffer_copyinsertchars(BUFFER dstbuf, int dstline, int dstcol,
                               BUFFER srcbuf, int srcline, int srccol,
                               int n, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(dstbuf);
  VALIDATEBUFFER(srcbuf);
  __check_line_exists("buffer_copyinsertchars/src", srcbuf, srcline);
  _expand_to_line(dstbuf, dstline);
  int srclinelen = buffer_line_length(srcbuf, srcline);
  const char* srctxt = buffer_getbufptr(srcbuf, srcline);
  buffer_insertstrn(dstbuf, dstline, dstcol, srctxt+min(srccol, srclinelen), min(n, srclinelen-srccol), upd_marks);
  TRACE_RETURN(POE_ERR_OK);
}


// Can be faster if the two buffers are different, or if the source range
// is before the dest range and there is no overlap.
POE_ERR buffer_copyinsertlines(BUFFER dstbuf, int di,
                               BUFFER srcbuf, int si,
                               int n, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(dstbuf);
  VALIDATEBUFFER(srcbuf);
  __check_line_lim("buffer_copyinsertlines/1", dstbuf, di);
  _expand_to_line(dstbuf, di+n);
  int nsrclines = buffer_count(srcbuf);
  if (si >= nsrclines)
    TRACE_RETURN(POE_ERR_OK);
  n = min(nsrclines - si, n);
  if (dstbuf != srcbuf) {
    // different buffers - safe to copy from src to dst
    if (upd_marks)
      marks_upd_insertedlines(dstbuf, di, n);
    __buffer_copyinsertlines(dstbuf, di, srcbuf, si, n);
    TRACE_RETURN(POE_ERR_OK);
  }
  else if (si+n <= di) {
    // src is completely before dst - can safely copy from src to dst
    if (upd_marks)
      marks_upd_insertedlines(dstbuf, di, n);
    __buffer_copyinsertlines(dstbuf, di, srcbuf, si, n);
    TRACE_RETURN(POE_ERR_OK);
  }
  else if (si >= di) {
    // src is completely after dst, but must use a temp buffer because
    // there's interactions between the two ranges, and we can't use
    // the other buffer to help.
    if (upd_marks)
      marks_upd_insertedlines(dstbuf, di, n);
    struct buffer_t tmp;
    _buffer_init(&tmp, "", BUF_FLG_INTERNAL, n, default_profile);
    __buffer_copyinsertlines(&tmp, 0, srcbuf, si, n);
    __buffer_copyinsertlines(dstbuf, di, &tmp, 0, n);
    _buffer_destroy(&tmp);
    TRACE_RETURN(POE_ERR_OK);
  }
  else {
    TRACE_RETURN(POE_ERR_SRC_DEST_CONFLICT);
  }
  TRACE_RETURN(POE_ERR_OK);
}


void __buffer_copyinsertlines(BUFFER dstbuf, int di,
                              BUFFER srcbuf, int si,
                              int n)
{
  TRACE_ENTER;
  VALIDATEBUFFER(dstbuf);
  VALIDATEBUFFER(srcbuf);
  int j;
  struct line_t* tmplines = PE_ALLOC_TMP(n, sizeof(struct line_t));
  for (j = 0; j < n; j++) {
    __line_initfrom(&tmplines[j], _line(srcbuf, si+j));
    tmplines[j].flags |= LINE_FLG_DIRTY;
    dstbuf->longest_line = max(dstbuf->longest_line, cstr_count(&tmplines[j].txt));
  }
  vec_insertm(&dstbuf->lines, di, n, tmplines);
  buffer_setflags(dstbuf, BUF_FLG_DIRTY);
  // Ownership of line_t data in tmplines goes to buffer, but not tmplines itself.
  PE_FREE_TMP(tmplines, n);
  TRACE_EXIT;
}


POE_ERR buffer_copyoverlaychars(BUFFER dstbuf, int dstline, int dstcol,
                                BUFFER srcbuf, int srcline, int srccol,
                                int nchars, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(dstbuf);
  VALIDATEBUFFER(srcbuf);
  __check_line_exists(__func__, srcbuf, srcline);
  _expand_to_line(dstbuf, dstline);
  POE_ERR err = POE_ERR_OK;
  int srclinelen = buffer_line_length(srcbuf, srcline);
  if (srccol >= srclinelen) {
    // Nothing to do
  }
  else if (srccol + nchars >= srclinelen) {
    const char* srctxt = buffer_getcharptr(srcbuf, srcline, srccol);
    buffer_setstrn(dstbuf, dstline, dstcol, srctxt, srclinelen-srccol, upd_marks);
  }
  else {
    const char* srctxt = buffer_getcharptr(srcbuf, srcline, srccol);
    buffer_setstrn(dstbuf, dstline, dstcol, srctxt, nchars, upd_marks);
  }
  TRACE_RETURN(err);
}


POE_ERR buffer_copyoverlaylines(BUFFER dstbuf, int dstline,
                                BUFFER srcbuf, int srcline,
                                int nlines, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(dstbuf);
  VALIDATEBUFFER(srcbuf);
  __check_line_exists(__func__, srcbuf, srcline);
  _expand_to_line(dstbuf, dstline+nlines);
  int i;
  for (i = 0; i < nlines; i++) {
    int srclinelen = buffer_line_length(srcbuf, srcline+i);
    int dstlinelen = buffer_line_length(dstbuf, dstline+i);
    const char* srctxt = buffer_getbufptr(srcbuf, srcline);
    buffer_setstrn(dstbuf, dstline+i, 0, srctxt, srclinelen, upd_marks);
    if (dstlinelen > srclinelen)
      buffer_removechars(dstbuf, dstline+i, srclinelen, dstlinelen-srclinelen, upd_marks);
  }
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR buffer_splitline(BUFFER buf, int row, int col, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  if (upd_marks)
    marks_upd_split(buf, row, col);
  const char* tail = buffer_getcharptr(buf, row, col);
  int taillen = strlen(tail);
  buffer_insertblanklines(buf, row+1, 1, false); // updates handled by upd_split
  buffer_insertstrn(buf, row+1, 0, tail, taillen, false);
  buffer_removechars(buf, row, col, taillen, false);
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR buffer_joinline(BUFFER buf, int row, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int nrows = buffer_count(buf);
  if (row >= nrows-1)
    TRACE_RETURN(POE_ERR_OK);
  const char* tail = buffer_getbufptr(buf, row+1);
  int linelen = buffer_line_length(buf, row);
  int taillen = buffer_line_length(buf, row+1);
  if (upd_marks) {
    marks_upd_join(buf, row, linelen);
  }
  buffer_insertstrn(buf, row, linelen, tail, taillen, false); // mark updates handled above
  buffer_removelines(buf, row+1, 1, false);
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR buffer_load(BUFFER buf, cstr* filename, bool tabexpand)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  
  tabstops load_tabs;
  tabs_init(&load_tabs, 0, buf->profile->tabexpand_size, NULL);
  
  POE_ERR err = POE_ERR_OK;
  cstr cpy_filename;
  cstr_initfrom(&cpy_filename, filename);
  cstr_trimleft(&cpy_filename, poe_iswhitespace);
  cstr_trimright(&cpy_filename, poe_iswhitespace);
  if (cstr_count(&cpy_filename) >= 2 && cstr_get(&cpy_filename, 0) == '~' && cstr_get(&cpy_filename, 1) == '/') {
    cstr_remove(&cpy_filename, 0);
    const char* home = getenv("HOME");
    if (home != NULL) {
      int homelen = strlen(home);
      cstr_insertm(&cpy_filename, 0, homelen, home);
    }
  }
  int flg_rdonly = 0;
  char expanded_filename[PATH_MAX+1];
  if (NULL == realpath(cstr_getbufptr(&cpy_filename), expanded_filename))
    TRACE_RETURN(POE_ERR_FILE_NOT_FOUND);
  const char* pszFilename = expanded_filename;
  
  // clean out the buffer
  //buffer_removelines(buf, 0, buffer_count(buf), true);
  buffer_clear(buf, false, true);
  
  // Decide on a buffer name (may have to try basename<1>, basename<2>, etc...
  cstr_assignstr(&buf->orig_filename, pszFilename);
  cstr_assignstr(&buf->curr_filename, pszFilename);
  
  const char* pszBasename = (const char*)basename(pszFilename);
  const char* pszDirname = (const char*)dirname(pszFilename);
  if (pszBasename == NULL)
    logerr("basename('%s') returned error %d", pszFilename, errno);
  
  if (pszBasename[0] == '/') {
    cstr_assignstr(&buf->base_buffername, pszBasename+1);
  }
  else {
    cstr_assignstr(&buf->base_buffername, pszBasename);
  }
  cstr_assignstr(&buf->orig_dirname, pszDirname);
  cstr_assignstr(&buf->curr_dirname, pszDirname);
  
  cstr cand_buffername = _buffer_make_unique_name(&buf->base_buffername);
  cstr_assign(&buf->buffername, &cand_buffername);
  
  buffer_setflags(buf, BUF_FLG_VISIBLE);
  buffer_clrflags(buf, BUF_FLG_DIRTY|BUF_FLG_RDONLY|BUF_FLG_NEW);
  
  // Open the file...
  FILE* f = fopen(pszFilename, "r+");
  if (f == NULL && errno == EACCES) {
    errno = 0;
    f = fopen(pszFilename, "r");
    flg_rdonly = BUF_FLG_RDONLY;
  }
  if (f == NULL) {
    logerr("error %d opening file '%s'", errno, pszFilename);
    buffer_setflags(buf, BUF_FLG_NEW);
    switch (errno) {
    case EPERM: case EIO: case EACCES:
      buffer_ensure_min_lines(buf, false);
      err = POE_ERR_READING_FILE;
      goto done;
      break;
    case ENOENT:
      buffer_ensure_min_lines(buf, false);
      err = POE_ERR_FILE_NOT_FOUND;
      goto done;
      break;
    default:
      buffer_ensure_min_lines(buf, false);
      err = POE_ERR_CANT_OPEN;
      goto done;
      break;
    }
  }
  
  // load the file...
  struct line_t line;
  __line_init(&line);
  cstr* str = &line.txt;
  int col = 0;
  int c;
  int seenquotes = 0;
  while (!feof(f)) {
    c = fgetc(f);
    while (c != '\n' && c != '\r' && !feof(f)) {
      if (c == '"' || c == '\'')
        seenquotes++;
      if (c == '\t' && ((seenquotes&1) == 0) && tabexpand) {
        int nextcol = tabs_next(&load_tabs, col);
        cstr_appendct(str, ' ', nextcol-col);
        col = nextcol;
      }
      else {
        cstr_append(str, (char)c);
        col++;
      }
      c = fgetc(f);
    }
    line.flags = LINE_INITIAL_FLAGS;
    if (c == '\r') {
      c = fgetc(f);
      line.flags |= LINE_FLG_CR;
      if (c == '\n') {
        line.flags |= LINE_FLG_LF;
      }
      else {
        ungetc(c, f);
      }
    }
    else if (c == '\n') {
      line.flags |= LINE_FLG_LF;
    }
        
    // if we are at eof, then we only write out the line if it has
    // something (i.e. we have an unterminated last line)
    if (!feof(f) || cstr_count(str) > 0) {
      buffer_appendline(buf, &line);
      col = 0;
      seenquotes = 0;
      cstr_clear(str);
    }
  }
  fclose(f);
  
  // finish up
  cstr_destroy(&cand_buffername);
  __line_destroy(&line);
  
 done:
  cstr_destroy(&cpy_filename);
  // update buffer flags
  buffer_clrflags(buf, BUF_FLG_DIRTY);
  buffer_setflags(buf, BUF_FLG_VISIBLE | flg_rdonly);
  tabs_destroy(&load_tabs);
  buffer_ensure_min_lines(buf, false);

  TRACE_RETURN(err);
}


POE_ERR buffer_save(BUFFER buf, cstr* filename, bool blankcompress)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  
  tabstops save_tabs;
  tabs_init(&save_tabs, 0, buf->profile->tabexpand_size, NULL);
  
  POE_ERR rval = POE_ERR_OK;
  if (filename == NULL)
    filename = &buf->curr_filename;
  const char* pszFilename = cstr_getbufptr(filename);
  //logmsg("buffer_save, curr_filename = '%s'", cstr_getbufptr(&buf->curr_filename));
  if (cstr_count(&buf->curr_filename) == 0 && (pszFilename == NULL || strlen(pszFilename) == 0))
    return POE_ERR_MISSING_FILE_NAME;
  
  // figure out which name to use
  // should really construct out of basename(filename) + "/" + base_buffername)
  cstr save_filename;
  cstr_init(&save_filename, 128);
  if (pszFilename == NULL) {
    cstr_assignstr(&save_filename, dirname(cstr_getbufptr(&buf->curr_filename)));
    cstr_append(&save_filename, '/');
    cstr_appendcstr(&save_filename, &buf->base_buffername);
  }
  else if (*pszFilename == '/' || *pszFilename == '~') {
    cstr_assign(&save_filename, filename);
  }
  else {
    cstr_assignstr(&save_filename, dirname(cstr_getbufptr(&buf->curr_filename)));
    cstr_append(&save_filename, '/');
    cstr_appendm(&save_filename, strlen(pszFilename), pszFilename);
  }
  
  // Open, and handle any errors...
  FILE* f = fopen(cstr_getbufptr(&save_filename), "w");
  if (f == NULL) {
    cstr_destroy(&save_filename);
    switch (errno) {
    case EPERM: case EIO: case EACCES:
      TRACE_RETURN(POE_ERR_READING_FILE);
      break;
    case ENOENT:
      TRACE_RETURN(POE_ERR_FILE_NOT_FOUND);
      break;
    default:
      TRACE_RETURN(POE_ERR_CANT_OPEN);
      break;
    }
  }
  
  int nlines = buffer_count(buf);
  int i;
  for (i = 0; i < nlines; i++) {
    struct line_t* line = _line(buf, i);
    // Write the line, compressing tabs as necessary
    int j, col = 0;
    int len = cstr_count(&line->txt);
    int seenquotes = 0;
    for (j = 0; j < len; j++) {
      char c = cstr_get(&line->txt, j);
      if (c == ' ' && blankcompress && ((seenquotes&1) == 0)) {
        int nextcol = tabs_next(&save_tabs, col);
        int runlen = strspn(cstr_getcharptr(&line->txt, j), " ");
        if (runlen > 2 && runlen >= nextcol-col) {
          fputc('\t', f);
          col = nextcol;
          j = nextcol-1; // will be incremented in 'for'
        }
        else {
          int k;
          for (k = 0; k < runlen; k++)
            fputc(c, f);
          col = col+runlen;
          j += runlen-1;
        }
      }
      else if (c == '"' || c == '\"') {
        seenquotes++;
        fputc((int)c, f);
        col++;
      }
      else {
        fputc((int)c, f);
        col++;
      }
    }
    // terminate the line appropriately
    if ((line->flags & (LINE_FLG_CR | LINE_FLG_LF)) == (LINE_FLG_CR | LINE_FLG_LF)) { // crlf
      fputc('\r', f); fputc('\n', f);
    }
    else if ((line->flags & LINE_FLG_CR) == LINE_FLG_CR) { // cr
      fputc('\r', f);
    }
    else if ((line->flags & LINE_FLG_LF) == LINE_FLG_LF) { // lf
      fputc('\n', f);
    }
    else {
      // Unterminated line. This is a fairly common occurrence on the
      // last line (the eof is a terminator), but if it's in the
      // middle of the file then there's a problem.
      // printf("unterminated line %d / %d\n", i, nlines);
      if (i < nlines-1)
        fputc('\n', f);
    }
  }
  
  fclose(f);
  cstr_destroy(&save_filename);
  
  buffer_clrflags(buf, BUF_FLG_DIRTY|BUF_FLG_NEW);
  tabs_destroy(&save_tabs);
  TRACE_RETURN(rval);
}


bool buffer_wrap_line(BUFFER buf,
                      int row, int lastrow,
                      int pmargin, int lmargin, int rmargin,
                      bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, row);
  int leftmargin, rightmargin, paragraphmargin;
  buffer_getmargins(buf, &leftmargin, &rightmargin, &paragraphmargin);
  if (lmargin < 0)
    lmargin = leftmargin;
  if (rmargin < 0)
    rmargin = rightmargin;
  // Make sure first line is indented properly
  int this_leftmargin;
  if (pmargin < 0) {
    pmargin = paragraphmargin;
    if (row==0 || buffer_isblankline(buf, row-1))
      this_leftmargin = pmargin;
    else
      this_leftmargin = lmargin;
  }
  else {
    this_leftmargin = pmargin;
  }
  int firstnb = buffer_scantill_nowrap(buf, row, 0, 1, poe_isnotwhitespace);
  if (firstnb < this_leftmargin) {
    buffer_trimleft(buf, row, upd_marks);
    buffer_trimright(buf, row, upd_marks);
    buffer_respace(buf, row, poe_iswhitespace, upd_marks);
    buffer_insertct(buf, row, 0, ' ', this_leftmargin, upd_marks);
  }
  
  // bail if there's nothing more to do
  int linelen = buffer_line_length(buf, row);
  if (linelen <= rightmargin) {
    TRACE_RETURN(false);
  }
  
  MARK rgn = mark_alloc(0);
  mark_start(rgn, Marktype_Line, buf, row, 0);
  if (lastrow < 0)
    lastrow = max(row, max(0, buffer_findparagraphsep(buf, row, 1)-1));
  mark_extend(rgn, Marktype_Line, buf, max(row, lastrow), 0);
  int l1 = row, l2 = lastrow, c2 = 0;
  
  int i;
  bool didsplit = false;
  for (i = l1; i <= l2; ) {
    bool didsplitthis = false;
    buffer_trimleft(buf, i, upd_marks);
    buffer_trimright(buf, i, upd_marks);
    buffer_respace(buf, i, poe_iswhitespace, upd_marks);
    buffer_insertct(buf, i, 0, ' ', this_leftmargin, upd_marks);
    if (i < l2) {
      buffer_trimleft(buf, i+1, upd_marks);
      buffer_trimright(buf, i+1, upd_marks);
      buffer_respace(buf, i+1, poe_iswhitespace, upd_marks);
      buffer_joinline(buf, i, upd_marks);
      didsplitthis = _buffer_wrap_single_line(buf, i, leftmargin, rightmargin, upd_marks);
      didsplit |= didsplitthis;
      if (didsplitthis)
        i++;
    }
    else {
      didsplitthis = _buffer_wrap_single_line(buf, i, leftmargin, rightmargin, true);
      didsplit |= didsplitthis;
      i++;
    }
    mark_get_end(rgn, &l2, &c2);
    this_leftmargin = lmargin;
  }
  
  mark_free(rgn);
  TRACE_RETURN(didsplit);
}


bool _buffer_wrap_single_line(BUFFER buf, int row, int leftmargin, int rightmargin, bool upd_marks)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, row);
  bool didsplit = false;
  int linelen = buffer_line_length(buf, row);
  if (linelen > rightmargin) {
    int last_word_pos = buffer_scantill_nowrap(buf, row, buffer_line_length(buf, row)-1, -1, poe_isnotwhitespace);
    if (last_word_pos > rightmargin) {
      int brk_word_pos = buffer_scantill_nowrap(buf, row, rightmargin-1, -1, poe_iswhitespace);
      if (brk_word_pos > leftmargin) {
        buffer_splitline(buf, row, brk_word_pos+1, upd_marks);
        buffer_trimleft(buf, row+1, upd_marks);
        buffer_insertct(buf, row+1, 0, ' ', leftmargin, upd_marks);
        didsplit = true;
      }
    }
  }
  TRACE_RETURN(didsplit);
}


int buffer_respace(BUFFER buf, int line, char_pred_t spacepred, bool upd_marks)
{
  TRACE_ENTER;
  int nchars_delta = 0;
  int i, len = buffer_line_length(buf, line);
  const char* str = buffer_getbufptr(buf, line);
  
  // Skip leading spaces
  for (i = 0; i < len && (*spacepred)(str[i]); i++)
    ;
  while (i < len) {
    // Find end of word
    for (; i < len && !(*spacepred)(str[i]); i++)
      ;
    int wrd_end = i;
    // Skip trailing spaces
    for (; i < len && (*spacepred)(str[i]); i++)
      ;
    int space_end = i;
    int nspaces = space_end - wrd_end;
    int nspaces_needed;
        
    // Figure out how many spaces we need after this word
    if (str[wrd_end-1] == '.' || str[wrd_end-1] == ':')
      nspaces_needed = 2;
    else
      nspaces_needed = 1;
        
    // Remove/insert extra spaces as needed.
    if (nspaces < nspaces_needed) {
      int nadd = nspaces_needed - nspaces;
      buffer_insertct(buf, line, wrd_end, ' ', nadd, upd_marks);
      nchars_delta += nadd;
      i += nadd;
    }
    else if (nspaces > nspaces_needed) {
      int nrem = nspaces - nspaces_needed;
      buffer_removechars(buf, line, wrd_end, nrem, upd_marks);
      nchars_delta -= nrem;
      i -= nrem;
    }
        
    len = buffer_line_length(buf, line);
    str = buffer_getbufptr(buf, line);
  }
  TRACE_RETURN(nchars_delta);
}


bool buffer_search(BUFFER buf, int* prow, int* pcol, int* pendcol, const cstr* pat, bool exact, int direction)
{
  TRACE_ENTER;
  int row = *prow, col = *pcol;
  bool found = false;
  if (direction > 0) {
    int nrows = buffer_count(buf);
    while (!found && row < nrows) {
      struct line_t* line = _line(buf, row);
      int i = (exact ? cstr_find : cstr_findi)(&line->txt, col, pat, 1);
      if (i >= 0) {
        col = i;
        found = true;
      }
      else {
        row++;
        col = 0;
      }
    }
  }
  else if (direction < 0) {
    while (!found && row >= 0) {
      struct line_t* line = _line(buf, row);
      int i = (exact ? cstr_find : cstr_findi)(&line->txt, col, pat, -1);
      if (i >= 0) {
        col = i;
        found = true;
      }
      else {
        row--;
        col = INT_MAX;
      }
    }
  }
  
  if (found) {
    *prow = row;
    *pcol = col;
    *pendcol = col + cstr_count(pat);
  }
  TRACE_RETURN(found);
}


void buffer_load_dir_listing(BUFFER buf, const char* dirname)
{
  TRACE_ENTER;
  buffer_clear(buf, true, true);
  // reset the .dir buffer's directory and flags
  cstr sdirname;
  cstr_initstr(&sdirname, dirname);

  cstr_assign(&buf->curr_dirname, &sdirname);
  cstr_assign(&buf->orig_dirname, &sdirname);

  cstr_destroy(&sdirname);
  // 
  DIR* dir = opendir(dirname);
  if (dir) {
    struct dirent* d = NULL;
    int linenum = 0;
    while ((d = readdir(dir)) != NULL) {
      if (d->d_type != DT_REG && d->d_type != DT_DIR)
        continue;
      int filelen = 0;
      char* filetype;
      char line[MAXNAMLEN+128];
      if (d->d_type == DT_REG) {
        filetype = "FILE";
        struct stat st; 
        if (stat(d->d_name, &st) == 0)
          filelen = st.st_size;
      }
      else if (d->d_type == DT_DIR) {
        filetype = "DIR ";
      }
      else {
        filetype = "UNK ";
      }
      snprintf(line, sizeof(line), "%4s %13d %s", filetype, filelen, d->d_name);
      int bufct = buffer_count(buf);
      if (linenum >= bufct)
        buffer_appendblanklines(buf, bufct-linenum+1);
      buffer_setstrn(buf, linenum, 0, line, strlen(line), false);
      ++linenum;
    }
  }
  
  closedir(dir);
  buffer_clrflags(buf, BUF_FLG_DIRTY|BUF_FLG_NEW);
  buffer_setflags(buf, BUF_FLG_RDONLY);
  TRACE_EXIT;
}


void buffer_clear(BUFFER buf, bool ensure_min_lines, bool upd_marks)
{
  TRACE_ENTER;
  buffer_ensure_min_lines(buf, false);
  if (ensure_min_lines) {
    buffer_removelines(buf, 1, max(0, buffer_count(buf)-1), upd_marks);
    buffer_removechars(buf, 0, 0, buffer_line_length(buf, 0), upd_marks);
  }
  else {
    buffer_removelines(buf, 0, buffer_count(buf), upd_marks);
  }
  TRACE_EXIT;
}


BUFFER buffers_find_named(cstr* name)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_buffers);
  int i;
  for (i = 0; i < n; ++i) {
    BUFFER buf = (BUFFER)pivec_get(&_all_buffers, i);
    if (_buffer_isnamed(buf, name))
      TRACE_RETURN(buf);
  }
  TRACE_RETURN(BUFFER_NULL);
}


BUFFER buffers_find_eithername(cstr* name)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_buffers);
  int i;
  for (i = 0; i < n; ++i) {
    BUFFER buf = (BUFFER)pivec_get(&_all_buffers, i);
    if (_buffer_isnamed(buf, name) || _buffer_isfilenamed(buf, name))
      TRACE_RETURN(buf);
  }
  TRACE_RETURN(BUFFER_NULL);
}




//
// *really* internal functions
//
struct line_t* _line(BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  struct line_t* rval = (struct line_t*)vec_get(&buf->lines, line);
  TRACE_RETURN(rval);
}


void _expand_to_line(BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  int nlines = buffer_count(buf);
  if (line >= nlines)
    buffer_appendblanklines(buf, nlines - line + 1);
  TRACE_EXIT;
}


// Expands the line with spaces so that col is on a valid character.
void _expand_to_col(BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  __check_line_exists(__func__, buf, line);
  struct line_t* l = _line(buf, line);
  int linelen = cstr_count(&l->txt);
  if (col >= linelen) {
    cstr_appendct(&l->txt, ' ', col - linelen + 1);
  }
  TRACE_EXIT;
}


void __check_line_exists(const char* dbgname, BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  if (line >= vec_count(&buf->lines) || line < 0)
    poe_err(1, "%s error: line out of bounds %d %d", dbgname, line, vec_count(&buf->lines));
  TRACE_EXIT;
}


void __check_line_col_exists(const char* dbgname, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  if (line >= vec_count(&buf->lines) || line < 0)
    poe_err(1, "%s error: line %d out of bounds %d",
            dbgname, line, vec_count(&buf->lines));
  struct line_t* pline = _line(buf, line);
  if (col >= cstr_count(&pline->txt))
    poe_err(1, "%s error: col %d of line %d out of bounds (%d)", 
            dbgname, col, line, cstr_count(&pline->txt));
  TRACE_EXIT;
}


void __check_line_lim(const char* dbgname, BUFFER buf, int line)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  if (line > vec_count(&buf->lines) || line < 0)
    poe_err(1, "%s error: line out of bounds %d", dbgname, line);
  TRACE_EXIT;
}


void __check_lines_exist(const char* dbgname, BUFFER buf, int startline, int nlines)
{
  TRACE_ENTER;
  VALIDATEBUFFER(buf);
  if (startline+nlines > vec_count(&buf->lines))
    poe_err(1, "%s error: line out of bounds %d", dbgname, startline+nlines);
  if (startline < 0)
    poe_err(1, "%s error: negative line %d", dbgname, startline);
  TRACE_EXIT;
}


int __find_buffer(BUFFER srch)
{
  TRACE_ENTER;
  int i, n = pivec_count(&_all_buffers);
  for (i = 0; i < n; i++) {
    BUFFER probe = (BUFFER)pivec_get(&_all_buffers, i);
    if (srch == probe)
      TRACE_RETURN(i);
  }
  TRACE_RETURN(-1);
}


cstr _buffer_make_unique_name(cstr* name)
{
  TRACE_ENTER;
  cstr cand_buffername;
  cstr_init(&cand_buffername, 100);
  int i = 0;
  char tmp[32];
  do {
    i++;
    cstr_assign(&cand_buffername, name);
    if (i > 1) {
      cstr_append(&cand_buffername, '<');
      snprintf(tmp, sizeof(tmp), "%d", i);
      cstr_appendm(&cand_buffername, strlen(tmp), tmp);
      cstr_append(&cand_buffername, '>');
    }
  } while (_buffer_name_exists(&cand_buffername));
  TRACE_RETURN(cand_buffername);
}


int _buffer_name_exists(cstr* name)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_buffers);
  int i;
  for (i = 0; i < n; ++i) {
    BUFFER buf = (BUFFER)pivec_get(&_all_buffers, i);
    if (_buffer_isnamed(buf, name))
      TRACE_RETURN(1);
  }
  TRACE_RETURN(0);
}

