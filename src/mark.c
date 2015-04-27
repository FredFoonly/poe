
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "trace.h"
#include "logging.h"
#include "utils.h"
#include "poe_err.h"
#include "poe_exit.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "mark.h"
#include "margins.h"
#include "tabstops.h"
#include "key_interp.h"
#include "buffer.h"


#define MARK_SIG (0xDEADBEEF)

typedef int mark_flags_t;

struct mark_t {
  int _sig;
  int marknum;
  BUFFER buf;
  mark_flags_t flags;
  int l1, c1, l2, c2;
  enum marktype typ;
  // True if l1 or c1 was the first mark chronologically
  unsigned short int firstlmark, firstcmark;
  void* data;
};



struct pivec_t/*MARK*/ _all_marks;
int _next_mark_id = 0;


void _mark_init(MARK mark, int markid, int flags);
void _mark_destroy(MARK mark);
void _mark_free(MARK mark);
int __find_mark(MARK srch);
void _mark_upd_insertedchars(MARK mark, BUFFER buf, int line, int col, int inserted_chars);
void _mark_upd_removedchars(MARK mark, BUFFER buf, int line, int col, int chars_removed);
void _mark_upd_split(MARK mark, BUFFER buf, int line, int col);
void _mark_upd_join(MARK mark, BUFFER buf, int line, int col);
void _mark_canonicalize(MARK mark);
POE_ERR _mark_check(MARK mark);


void init_marks()
{
  TRACE_ENTER;
  _next_mark_id = 1;
  pivec_init(&_all_marks, 10);
  TRACE_EXIT;
}


void shutdown_marks()
{
  TRACE_ENTER;
  int i, n = pivec_count(&_all_marks);
  for (i = n-1; i >= 0; i--) {
    _mark_free((MARK)pivec_get(&_all_marks, i));
  }
  pivec_destroy(&_all_marks);
  TRACE_EXIT;
}


void _validatemarkptr(const char* dbg, MARK mark)
{
  if (mark == NULL) {
    poe_err(1, "%s Attempted to use a null mark.", dbg);
  }
  if (mark->_sig != MARK_SIG) {
    poe_err(1, "%s Attempted to use an invalid mark.", dbg);
  }
}

#ifdef POE_DBG_MARKPTRS
#define VALIDATEMARK(mark) _validatemarkptr(__func__, (mark))
#else
#define VALIDATEMARK(mark) /**/
#endif


MARK mark_alloc(int flags)
{
  TRACE_ENTER;
  int marknum = _next_mark_id++;
  MARK newmark = calloc(1, sizeof(struct mark_t));
  _mark_init(newmark, marknum, flags);
  pivec_append(&_all_marks, (intptr_t)newmark);
  VALIDATEMARK(newmark);
  TRACE_RETURN(newmark);
}


void mark_free(MARK mark)
{
  TRACE_ENTER;
  int i = __find_mark(mark);
  if (i >= 0) {
    _mark_free(mark);
    pivec_remove(&_all_marks, i);
  }
  TRACE_EXIT;
}


void _mark_free(MARK mark)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  _mark_destroy(mark);
  free(mark);
  TRACE_EXIT;
}


void _mark_init(MARK mark, int marknum, int flags)
{
  TRACE_ENTER;
  mark->_sig = MARK_SIG;
  mark->marknum = marknum;
  mark->buf = BUFFER_NULL;
  mark->flags = flags;
  mark_unmark(mark);
  mark->firstlmark = mark->firstcmark = 1;
  mark->data = NULL;
  VALIDATEMARK(mark);
  TRACE_EXIT;
}


void _mark_destroy(MARK mark)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  mark_unmark(mark);
  mark->_sig = 0;
  TRACE_EXIT;
}


void mark_unmark(MARK mark)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  mark->buf = BUFFER_NULL;
  mark->flags &= ~(MARK_FLG_STARTED|MARK_FLG_ENDED|MARK_FLG_SEALED);
  mark->typ = Marktype_None;
  mark->l1 = 0;
  mark->c1 = 0;
  mark->l2 = 0;
  mark->c2 = 0;
  mark->firstlmark = mark->firstcmark = 1;
  TRACE_EXIT;
}


int mark_exists(MARK mark)
{
  TRACE_ENTER;
  int i = __find_mark(mark);
  int rval = (i >= 0) && (mark->_sig == MARK_SIG);
  TRACE_RETURN(rval);
}


void mark_must_exist(const char* dbgstr, MARK mark)
{
  TRACE_ENTER;
  if (!mark_exists(mark))
    poe_err(1, "%s error: mark %d not found", dbgstr, mark->marknum);
  TRACE_EXIT;
}


int marks_count()
{
  TRACE_ENTER;
  int rval = pivec_count(&_all_marks);
  TRACE_RETURN(rval);
}


void mark_free_marks_in_buffer(BUFFER buf)
{
  TRACE_ENTER;
  int i;
  for (i = 0; i < pivec_count(&_all_marks); ) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
	if (mark->buf == buf) {
	  _mark_free(mark);
	  pivec_remove(&_all_marks, i);
	}
	else {
	  i++;
	}
  }
  TRACE_EXIT;
}


void mark_setflags(MARK mark, int flags)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  mark->flags |= flags;
  TRACE_EXIT;
}


void mark_clrflags(MARK mark, int flags)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  mark->flags &= ~flags;
  TRACE_EXIT;
}


int mark_tstflags(MARK mark, int flags)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  int rval = (mark->flags & flags) == flags;
  TRACE_RETURN(rval);
}


POE_ERR mark_get_buffer(MARK mark, BUFFER* buf)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  *buf = mark->buf;
  POE_ERR rval = _mark_check(mark);
  TRACE_RETURN(rval);
}


POE_ERR mark_get_type(MARK mark, enum marktype* typ)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  *typ = mark->typ;
  POE_ERR rval = _mark_check(mark);
  TRACE_RETURN(rval);
}


POE_ERR mark_get_start(MARK mark, int* line, int* col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  *line = mark->l1;
  *col = mark->c1;
  POE_ERR rval = _mark_check(mark);
  TRACE_RETURN(rval);
}


POE_ERR mark_get_end(MARK mark, int* line, int* col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  *line = mark->l2;
  *col = mark->c2;
  POE_ERR rval = _mark_check(mark);
  TRACE_RETURN(rval);
}


POE_ERR mark_get_bounds(MARK mark, enum marktype *typ, int* l1, int* c1, int* l2, int* c2)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  POE_ERR rc = _mark_check(mark);
  if (rc != POE_ERR_OK) {
    *typ = Marktype_None;
    *l1 = *l2 = *c1 = *c2 = 0;
    TRACE_RETURN(rc);
  }
  *typ = mark->typ;
  *l1 = mark->l1;
  *c1 = mark->c1;
  *l2 = mark->l2;
  *c2 = mark->c2;
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR mark_place(MARK mark, enum marktype typ, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  POE_ERR rval;
  if (mark->typ == Marktype_None)
    rval = mark_start(mark, typ, buf, line, col);
  else
    rval = mark_extend(mark, typ, buf, line, col);
  TRACE_RETURN(rval);
}


bool mark_hittest_point(MARK mark, BUFFER buf, int row, int col, int flags_mask, int flags_chk)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->buf != buf)
    TRACE_RETURN(false);
  if ((mark->flags & flags_mask) != flags_chk)
    TRACE_RETURN(false);
  bool rval = false;
  switch (mark->typ) {
  case Marktype_None:
    rval = false;
    break;
  case Marktype_Line:
    rval = row >= mark->l1 && row <= mark->l2;
    break;
  case Marktype_Char:
    if (row > mark->l1 && row < mark->l2)
      rval = true;
    else if (row == mark->l1 && row == mark->l2)
      rval = col >= mark->c1 && col <= mark->c2;
    else if (row == mark->l1 && col >= mark->c1)
      rval = true;
    else if (row == mark->l2 && col <= mark->c2)
      rval = true;
    else
      rval = false;
    break;
  case Marktype_Block:
    rval = (row >= mark->l1 && row <= mark->l2) && (col >= mark->c1 && col <= mark->c2);
    break;
  }
  TRACE_RETURN(rval);
}


bool mark_hittest_line(MARK mark, BUFFER buf, int row, int flags_mask, int flags_chk)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->buf != buf)
    TRACE_RETURN(false);
  if ((mark->flags & flags_mask) != flags_chk)
    TRACE_RETURN(false);
  bool rval = false;
  switch (mark->typ) {
  case Marktype_None:
    rval = false;
    break;
  case Marktype_Line: case Marktype_Char: case Marktype_Block:
    rval = row >= mark->l1 && row <= mark->l2;
    break;
  }
  TRACE_RETURN(rval);
}


MARK marks_hittest_point(BUFFER buf, int row, int col, int flags_mask, int flags_chk)
{
  TRACE_ENTER;
  int i, n = pivec_count(&_all_marks);
  for (i = 0; i < n; i++) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark_hittest_point(mark, buf, row, col, flags_mask, flags_chk))
      TRACE_RETURN(mark);
  }
  TRACE_RETURN(NULL);
}


MARK marks_hittest_line(BUFFER buf, int row, int flags_mask, int flags_chk)
{
  TRACE_ENTER;
  int i, n = pivec_count(&_all_marks);
  for (i = 0; i < n; i++) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark_hittest_line(mark, buf, row, flags_mask, flags_chk))
      TRACE_RETURN(mark);
  }
  TRACE_RETURN(NULL);
}



void _mark_upd_insertedlines(MARK mark, BUFFER buf, int line, int lines_inserted)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->buf != buf) TRACE_EXIT; // Happened in a different buffer
  if (line > mark->l2) TRACE_EXIT;  // Happened after the marked lines
  if (lines_inserted <= 0) TRACE_EXIT;

  int l1 = mark->l1, l2 = mark->l2;

  switch (mark->typ) {
  case Marktype_Line: case Marktype_Char:
    if (line <= l1) {
      // Happened before mark - move both ends down
      mark->l1 += lines_inserted;
      mark->l2 += lines_inserted;
    }
    else if (line > l1 && line <= l2) {
      // Happened inside mark - move bottom end down
      mark->l2 += lines_inserted;
    }
    break;
  case Marktype_Block: case Marktype_None:
    break;
  }
  TRACE_EXIT; 
}


// Make sure this is called before actually removing the lines!  It
// uses the buffer line count in one spot.
void _mark_upd_removedlines(MARK mark, BUFFER buf, int line, int lines_removed)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  int buf_lines;
  if (mark->buf != buf) TRACE_EXIT; // Happened in a different buffer
  if (line > mark->l2) TRACE_EXIT;  // Happened after the marked lines
  if (lines_removed <= 0) TRACE_EXIT;

  int l1 = mark->l1, l2 = mark->l2;

  switch (mark->typ) {
  case Marktype_Line: case Marktype_Char:
        // l1 0  l2 3  line 2  lines_removed 1
        // 2+1 <= 0
    if (line+lines_removed <= l1) {
      // Happened completely before mark - move both ends up
      mark->l1 -= lines_removed;
      mark->l2 -= lines_removed;
    }
        // 2 >= 0 && 2+1 < 3
        else if (line >= l1 && line+lines_removed <= l2) {
      // Happened completely inside marked region
      mark->l2 = max(line, l2 - lines_removed);
    }
        // 2 < 0 && 2+1<3
        else if (line < l1 && line+lines_removed < l2) {
      // Flows into marked region, ends inside it
      mark->l1 = max(line, l1 - lines_removed);
      mark->l2 = max(mark->l1, l2 - lines_removed);
    }
        // 2>=0 && 2+1>3
        else if (line >= l1 && line+lines_removed > l2) {
      // Happened inside region, ends outside it
      mark->l2 = max(line, l2 - lines_removed);
    }
        // 2<=0 && 2+1>=3
        else if (line <= l1 && line+lines_removed >= l2) {
      // Envelopes marked region
      if (mark_tstflags(mark, MARK_FLG_BOOKMARK)) {
        // Bookmarks are special in that they represent a position,
        // not an actual block of text.
        mark->l1 = line; mark->l2 = line;
      }
      else {
        mark_unmark(mark);
      }
    }
        else {
      poe_err(1, "Missing case in marks_update_removelines l1 %d  l2 %d  line %d  lines_removed %d",
                          mark->l1, mark->l2, line, lines_removed);
    }
    break;
  case Marktype_Block:
    // Ignores changes in buffer contents, except that it won't run
    // off the end of the buffer.
    buf_lines = buffer_count(buf);
    if (l1 >= buf_lines - lines_removed) {
      mark_unmark(mark);
    }
    else {
      mark->l2 = max(l2, buf_lines - lines_removed);
    }
    break;
  case Marktype_None:
    break;
  }
  TRACE_EXIT;
}


void mark_upd_insertedcharblk(MARK mark, BUFFER buf, int line, int col, int lines_inserted, int leading_chars_inserted, int trailing_chars_inserted)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->buf != buf) TRACE_EXIT; // Happened in a different buffer
  if (line > mark->l2) TRACE_EXIT;  // Happened after the marked lines

  switch (mark->typ) {
  case Marktype_Line:
    _mark_upd_insertedlines(mark, buf, line+1, lines_inserted);
    break;
  case Marktype_Char:
    if (lines_inserted == 0) {
      _mark_upd_insertedchars(mark, buf, line, col, leading_chars_inserted);
    }
    else {
      _mark_upd_split(mark, buf, line, col);
      _mark_upd_insertedlines(mark, buf, line+1, lines_inserted-1);
      _mark_upd_insertedchars(mark, buf, line+lines_inserted, 0, trailing_chars_inserted);
    }
    break;
  case Marktype_Block:
    // Not affected by insertions within a line
    break;
  case Marktype_None:
    break;
  }
  TRACE_EXIT;
}


//void _mark_upd_insertedchars0(MARK mark, BUFFER buf, int line, int col, int inserted_chars)
void _mark_upd_insertedchars(MARK mark, BUFFER buf, int line, int col, int chars_inserted)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (chars_inserted <= 0)
    TRACE_EXIT;
  switch (mark->typ) {
  case Marktype_Line: case Marktype_Block:
    break;
  case Marktype_Char:
    {
      int l1=mark->l1, c1=mark->c1, l2=mark->l2, c2=mark->c2;
      if (line == l1 && line == l2 && col <= c1) {
        mark->c1 += chars_inserted;
        mark->c2 += chars_inserted;
      }
      else if (line == l1 && line == l2 && col <= c2) {
        mark->c2 += chars_inserted;
      }
      else if (line == l1 && col <= c1) {
        mark->c1 += chars_inserted;
      }
      else if (line == l2 && col <= c2) {
        mark->c2 += chars_inserted;
      }
    }
    break;
  case Marktype_None: default:
    break;
  }
  TRACE_EXIT;
}


void _mark_upd_split(MARK mark, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  int l1=mark->l1, c1=mark->c1, l2=mark->l2, c2=mark->c2;
  switch (mark->typ) {
  case Marktype_Line:
    if (line < l1)
      mark->l1++;
    if (line <= l2)
      mark->l2++;
    break;
  case Marktype_Char:
    if (line < l1) {
      mark->l1++;
      mark->l2++;
    }
    else if (line == l1 && line == l2 && col <= c1) {
      mark->l1++; mark->c1 = c1 - col;
      mark->l2++; mark->c2 = c2 - col;
    }
    else if (line == l1 && line == l2 && col <= c2) {
      mark->l2++; mark->c2 = c2 - col;
    }
    else if (line == l1 && line == l2 && col > c2) {
      // No effect
    }
    else if (line == l1 && col <= c1) {
      mark->l1++; mark->c1 = c1 - col;
      mark->l2++;
    }
    else if (line == l1 && col > c1 && col <= c2) {
      mark->l2++;
    }
    else if (line == l2 && col <= c2) {
      mark->l2++;
      mark->c2 = c2 - col;
    }
    else if (line >= l1 && line < l2) {
      mark->l2++;
    }
    break;
  case Marktype_Block: case Marktype_None: default:
    break;
  }
  TRACE_EXIT;
}


// Make sure this is called before actually removing the chars!  It
// uses the # lines in the buffer in one spot!
// lines_removed = 0 if the char range is contained in one line
// lines_removed = 1 if the char range was from (l1, c1) to (l1+1, c2)
void mark_upd_removedcharblk(MARK mark, BUFFER buf, int line, int col, int lines_removed, int leading_chars_removed, int trailing_chars_removed)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->buf != buf) TRACE_EXIT; // Happened in a different buffer
  if (line > mark->l2) TRACE_EXIT;  // Happened after the marked lines

  switch (mark->typ) {
  case Marktype_Line:
    _mark_upd_removedlines(mark, buf, line+1, lines_removed);
    break;
  case Marktype_Char:
    if (lines_removed == 0) {
      _mark_upd_removedchars(mark, buf, line, col, leading_chars_removed);
    }
    else {
      _mark_upd_removedchars(mark, buf, line, col, leading_chars_removed);
      _mark_upd_removedlines(mark, buf, line+1, lines_removed-1);
      _mark_upd_removedchars(mark, buf, line+1, 0, trailing_chars_removed);
      _mark_upd_join(mark, buf, line, col);
    }
    break;
  case Marktype_Block:
    _mark_upd_removedlines(mark, buf, line+1, lines_removed);
    break;
  case Marktype_None:
    break;
  }
  TRACE_EXIT;
}


//void _mark_upd_removedchars0(MARK mark, BUFFER buf, int line, int col, int chars_removed)
void _mark_upd_removedchars(MARK mark, BUFFER buf, int line, int col, int chars_removed)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (chars_removed <= 0)
    TRACE_EXIT;
  switch (mark->typ) {
  case Marktype_Line: case Marktype_Block:
    break;
  case Marktype_Char:
    {
      int end = col+chars_removed;
      int l1=mark->l1, c1=mark->c1, l2=mark->l2, c2=mark->c2;
      // Cases for single-line mark
      if (line == l1 && line == l2 && end <= c1) { // Before mark
        mark->c1 -= chars_removed;
        mark->c2 -= chars_removed;
      }
      else if (line == l1 && line == l2 && col > c2) { // After mark
        // No effect
      }
      else if (line == l1 && line == l2 && col > c1 && end <= c2) { // Inside mark
        mark->c2 -= chars_removed;
      }
      else if (line == l1 && line == l2 && col <= c1 && end > c2) { // Envelops mark
        if (mark_tstflags(mark, MARK_FLG_BOOKMARK)) {
          // Bookmarks are special in that they represent a position, not
          // an actual block of text.
          mark->l1 = line; mark->c1 = col;
          mark->l2 = line; mark->c2 = col;
        }
        else {
          mark_unmark(mark);
        }
      }
      else if (line == l1 && line == l2 && col <= c1 && end > c1 && end <= c2) { // Crossing into mark
        mark->c1 = col;
        mark->c2 -= chars_removed;
      }
      else if (line == l1 && line == l2 && col > c1 && end > c2) { // Crossing out of mark
        mark->c2 = col;
      }
      // Cases for multi-line mark
      else if (line == l1 && end <= c1) {
        mark->c1 -= chars_removed;
      }
      else if (line == l1 && col <= c1 && end > c1) {
        mark->c1 = col;
      }
      else if (line == l1 && end > c1) {
        // No effect
      }
      else if (line == l2 && end <= c2) {
        mark->c2 -= chars_removed;
      }
      else if (line == l2 && col <= c2 && end > c2) {
        mark->c2 = col;
      }
      else {
                // no affect
      }
    }
  case Marktype_None: default:
    break;
  }
  TRACE_EXIT;
}


void _mark_upd_join(MARK mark, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  int l1=mark->l1, l2=mark->l2;
  switch (mark->typ) {
  case Marktype_Line:
    if (line < l1)
      mark->l1--;
    if (line < l2)
      mark->l2--;
    break;
  case Marktype_Char:
    if (line < l1-1) {              // xxxxxxxx^
      mark->l1--;                   // xxxxxxxx
      mark->l2--;                   // xx c1 zz
    }
    else if (line == l1-1) {
      mark->l1--;                   // xxxxxxxx
      mark->c1 += col;              // xxxxxxxx^
      mark->l2--;                   // xx c1 zz
    }
    else if (line == l1 && line+1==l2) {
      mark->l2 = mark->l1;          // xxxxxxxx
      mark->c2 = col;               // xxxxxxxxc1^
                                    // zz c2 xxxx
    }
    else if (line >= l1 && line+1<l2) {
      mark->l2--;                   // c1 zzzzz^
                                    // zzzzzzzz
                                    // zzz c2 xxx
    }
    else if (line == l2-1) {
      mark->l2--;                   // zzzzzzzz
      mark->c2 += col;              // zzzzzzzz^
                                    // zz c2 zz
    }
    break;
  case Marktype_Block: case Marktype_None:
    break;
  }
  TRACE_EXIT;
}


POE_ERR mark_bookmark(MARK mark, enum marktype typ, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->typ != Marktype_None || (mark->flags & MARK_FLG_BOOKMARK) != MARK_FLG_BOOKMARK) {
    TRACE_RETURN(POE_ERR_MARK_TYPE_CONFLICT);
  }
  mark->typ = typ;
  mark->buf = buf;
  mark->l1 = mark->l2 = line;
  mark->c1 = mark->c2 = col;
  mark->firstlmark = mark->firstcmark = 1;
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR mark_move_bookmark(MARK mark, enum marktype typ, int line, int col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->typ != typ || (mark->flags & MARK_FLG_BOOKMARK) != MARK_FLG_BOOKMARK) {
    TRACE_RETURN(POE_ERR_MARK_TYPE_CONFLICT);
  }
  int buf_lines = buffer_count(mark->buf);
  line = max(-1, min(buf_lines-1, line));
  mark->l1 = line; mark->l2 = line;
  mark->c1 = col; mark->c2 = col;
  mark->firstlmark = mark->firstcmark = 1;
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR mark_get_bookmark(MARK mark, enum marktype typ, int* line, int* col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->typ != typ || (mark->flags & MARK_FLG_BOOKMARK) != MARK_FLG_BOOKMARK) {
    *line = 0;
    *col = 0;
    TRACE_RETURN(POE_ERR_MARK_TYPE_CONFLICT);
  }
  *line = mark->l1;
  *col = mark->c1;
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR mark_start(MARK mark, enum marktype typ, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->typ != Marktype_None) {
    TRACE_RETURN(POE_ERR_MARK_TYPE_CONFLICT);
  }
  mark->typ = typ;
  mark->buf = buf;
  mark->l1 = mark->l2 = line;
  mark->c1 = mark->c2 = col;
  mark->firstlmark = mark->firstcmark = 1;
  mark_setflags(mark, MARK_FLG_STARTED);
  TRACE_RETURN(POE_ERR_OK);
}


int mark_extend(MARK mark, enum marktype typ, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->typ != typ)
    TRACE_RETURN(POE_ERR_MARK_TYPE_CONFLICT);
  if (mark->buf != buf)
    TRACE_RETURN(POE_ERR_MARKED_BLOCK_EXISTS);
  if (mark_tstflags(mark, MARK_FLG_SEALED))
    TRACE_RETURN(POE_ERR_MARKED_BLOCK_EXISTS);
  mark_setflags(mark, MARK_FLG_ENDED);
  if (mark->firstlmark)
    mark->l2 = line;
  else
    mark->l1 = line;
  if (mark->firstcmark)
    mark->c2 = col;
  else
    mark->c1 = col;
  _mark_canonicalize(mark);
  TRACE_RETURN(POE_ERR_OK);
}


void mark_seal(MARK mark)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->typ == Marktype_None) 
    TRACE_EXIT;
  if (mark_tstflags(mark, MARK_FLG_ENDED))
	mark_setflags(mark, MARK_FLG_SEALED);
  TRACE_EXIT;
}



//
// Update the marks with possible positional changes
//

void marks_upd_insertedlines(BUFFER buf, int line, int lines_inserted)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_marks);
  int i;
  for (i = 0; i < n; ++i) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark->buf != buf) continue; // Happened in a different buffer
    if (line > mark->l2) continue;  // Happened after the marked lines
    _mark_upd_insertedlines(mark, buf, line, lines_inserted);
  }
  TRACE_EXIT;
}


// Make sure this is called before actually removing the lines!  It
// uses the # lines in the buffer in one spot!
void marks_upd_removedlines(BUFFER buf, int line, int lines_removed)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_marks);
  int i;
  for (i = 0; i < n; ++i) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark->buf != buf) continue; // Happened in a different buffer
    if (line > mark->l2) continue;  // Happened after the marked lines
    _mark_upd_removedlines(mark, buf, line, lines_removed);
  }
  TRACE_EXIT;
}


void marks_upd_insertedcharblk(BUFFER buf, int line, int col, int lines_inserted, int leading_chars_inserted, int trailing_chars_inserted)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_marks);
  int i;
  for (i = 0; i < n; ++i) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark->buf != buf) continue; // Happened in a different buffer
    if (line > mark->l2) continue;  // Happened after the marked lines
    mark_upd_insertedcharblk(mark, buf, line, col, lines_inserted, leading_chars_inserted, trailing_chars_inserted);
  }
  TRACE_EXIT;
}


void marks_upd_insertedchars(BUFFER buf, int line, int col, int chars_inserted)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_marks);
  int i;
  for (i = 0; i < n; ++i) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark->buf != buf) continue; // Happened in a different buffer
    if (line > mark->l2) continue;  // Happened after the marked lines
    _mark_upd_insertedchars(mark, buf, line, col, chars_inserted);
  }
  TRACE_EXIT;
}


// Make sure this is called before actually removing the chars!  It
// uses the # lines in the buffer in one spot!
// lines_removed = 0 if the char range is contained in one line e.g. (l1, c1) (l1, c2)
// lines_removed = 1 if the char range was from (l1, c1) to (l1+1, c2)
void marks_upd_removedcharblk(BUFFER buf, int line, int col, int lines_removed, int leading_chars_removed, int trailing_chars_removed)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_marks);
  int i;
  for (i = 0; i < n; ++i) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark->buf != buf) continue; // Happened in a different buffer
    if (line > mark->l2) continue;  // Happened after the marked lines
    mark_upd_removedcharblk(mark, buf, line, col, lines_removed, leading_chars_removed, trailing_chars_removed);
  }
  TRACE_EXIT;
}



// Make sure this is called before actually removing the chars!  It
// uses the # lines in the buffer in one spot!
// lines_removed = 0 if the char range is contained in one line e.g. (l1, c1) (l1, c2)
// lines_removed = 1 if the char range was from (l1, c1) to (l1+1, c2)
void marks_upd_removedchars(BUFFER buf, int line, int col, int chars_removed)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_marks);
  int i;
  for (i = 0; i < n; ++i) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark->buf != buf) continue; // Happened in a different buffer
    if (line > mark->l2) continue;  // Happened after the marked lines
    _mark_upd_removedchars(mark, buf, line, col, chars_removed);
  }
  TRACE_EXIT;
}


void marks_upd_split(BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_marks);
  int i;
  for (i = 0; i < n; ++i) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark->buf != buf) continue; // Happened in a different buffer
    if (line > mark->l2) continue;  // Happened after the marked lines
    _mark_upd_split(mark, buf, line, col);
  }
  TRACE_EXIT;
}


void marks_upd_join(BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  int n = pivec_count(&_all_marks);
  int i;
  for (i = 0; i < n; ++i) {
    MARK mark = (MARK)pivec_get(&_all_marks, i);
    if (mark->buf != buf) continue; // Happened in a different buffer
    if (line > mark->l2) continue;  // Happened after the marked lines
    _mark_upd_join(mark, buf, line, col);
  }
  TRACE_EXIT;
}



#define SWAPLINES(mark) {\
    int tmp = (mark)->l1; (mark)->l1 = (mark)->l2; (mark)->l2 = tmp;    \
    (mark)->firstlmark = (!(mark)->firstlmark);                         \
  }
#define SWAPCOLS(mark) {\
    int tmp = (mark)->c1; (mark)->c1 = (mark)->c2; (mark)->c2 = tmp;    \
    (mark)->firstcmark = (!(mark)->firstcmark);                         \
  }

void _mark_canonicalize(MARK mark)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  switch (mark->typ) {
  case Marktype_Char: case Marktype_Line:
    {
      int rel = (mark->l1 == mark->l2) ? mark->c1 - mark->c2 : mark->l1 - mark->l2;
      if (rel > 0) {
        SWAPLINES(mark)
        SWAPCOLS(mark)
      }
    }
    break;
  case Marktype_Block:
    {
      if (mark->l1 > mark->l2)
        SWAPLINES(mark)
      if (mark->c1 > mark->c2)
        SWAPCOLS(mark)
    }
    break;
  default:
    break;
  }
  TRACE_EXIT;
}


POE_ERR _mark_check(MARK mark)
{
  TRACE_ENTER;
  VALIDATEMARK(mark);
  if (mark->buf < 0 || mark->typ == Marktype_None)
    TRACE_RETURN(POE_ERR_NO_MARKED_AREA)
  else
    TRACE_RETURN(POE_ERR_OK)
}



int __find_mark(MARK srch)
{
  TRACE_ENTER;
  int i, n = pivec_count(&_all_marks);
  for (i = 0; i < n; i++) {
    MARK probe = (MARK)pivec_get(&_all_marks, i);
    if (srch == probe)
      TRACE_RETURN(i);
  }
  TRACE_RETURN(-1);
}

