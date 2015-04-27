
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "trace.h"
#include "logging.h"
#include "utils.h"
#include "poe_err.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "mark.h"
#include "markstack.h"
#include "margins.h"
#include "tabstops.h"
#include "key_interp.h"
#include "buffer.h"


void _ensure_have_mark();

static struct pivec_t/*MARK*/ _mark_stack;


void init_markstack()
{
  TRACE_ENTER;
  pivec_init(&_mark_stack, 10);
  _ensure_have_mark();
  TRACE_EXIT;
}


void shutdown_markstack()
{
  TRACE_ENTER;
  pivec_destroy(&_mark_stack);
  TRACE_EXIT;
}



//
// markstack
//

void markstack_pop_marks_in_buffer(BUFFER buf)
{
  TRACE_ENTER;
  int i;
  for (i = 0; i < pivec_count(&_mark_stack); ) {
        MARK mark = (MARK)pivec_get(&_mark_stack, i);
        BUFFER markbuf = BUFFER_NULL;
        POE_ERR err = mark_get_buffer(mark, &markbuf);
        if (err == POE_ERR_OK && markbuf == buf) {
          pivec_remove(&_mark_stack, i);
        }
        else {
          i++;
        }
  }
  _ensure_have_mark();
  TRACE_EXIT;
}


MARK markstack_push()
{
  TRACE_ENTER;
  if (pivec_count(&_mark_stack) > 0)
    mark_clrflags(markstack_current(), MARK_FLG_VISIBLE);
  MARK m = mark_alloc(MARK_FLG_VISIBLE);
  pivec_insert(&_mark_stack, 0, (intptr_t)m);
  TRACE_RETURN(m);
}


POE_ERR markstack_pop()
{
  TRACE_ENTER;
  int nmarks = pivec_count(&_mark_stack);
  if (nmarks == 0) {
    TRACE_RETURN(POE_ERR_NO_MARKS_SAVED);
  }
  else if (nmarks == 1) {
    mark_setflags(markstack_current(), MARK_FLG_VISIBLE);
    TRACE_RETURN(POE_ERR_NO_MARKS_SAVED);
  }
  else {
    MARK oldmark = (MARK)pivec_get(&_mark_stack, 0);   
    mark_free(oldmark);
    pivec_remove(&_mark_stack, 0);
    mark_setflags(markstack_current(), MARK_FLG_VISIBLE);
    TRACE_RETURN(POE_ERR_OK);
  }
}


MARK markstack_current()
{
  TRACE_ENTER; 
  _ensure_have_mark();
  MARK rval = MARK_NULL;
  int nmarks = pivec_count(&_mark_stack);
  if (nmarks > 0)
    rval = (MARK)pivec_get(&_mark_stack, 0);
  TRACE_RETURN(rval);
}


MARK markstack_hittest_point(BUFFER buf, int row, int col, int flags_mask, int flags_chk)
{
  TRACE_ENTER;
  int i, n = pivec_count(&_mark_stack);
  for (i = 0; i < n; i++) {
    MARK m = (MARK)pivec_get(&_mark_stack, i);
    if (mark_hittest_point(m, buf, row, col, flags_mask, flags_chk))
      TRACE_RETURN(m);
  }
  TRACE_RETURN(MARK_NULL);
}


MARK markstack_hittest_line(BUFFER buf, int row, int flags_mask, int flags_chk)
{
  TRACE_ENTER;
  int i, n = pivec_count(&_mark_stack);
  for (i = 0; i < n; i++) {
    MARK m = (MARK)pivec_get(&_mark_stack, i);
    if (mark_hittest_line(m, buf, row, flags_mask, flags_chk)) {
      TRACE_RETURN(m);
    }
  }
  TRACE_RETURN(MARK_NULL);
}


POE_ERR markstack_cur_unmark()
{
  TRACE_ENTER;
  _ensure_have_mark();
  mark_unmark(markstack_current());
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR markstack_cur_get_buffer(BUFFER* buf)
{
  TRACE_ENTER;
  _ensure_have_mark();
  MARK curmark = markstack_current();
  POE_ERR rval;
  if (mark_exists(curmark))
    rval = mark_get_buffer(curmark, buf);
  else
    rval = POE_ERR_NO_MARKED_AREA;
  TRACE_RETURN(rval);
}


POE_ERR markstack_cur_get_type(enum marktype* typ)
{
  TRACE_ENTER;
  _ensure_have_mark();
  MARK curmark = markstack_current();
  POE_ERR rval;
  if (mark_exists(curmark))
    rval = mark_get_type(curmark, typ);
  else
    rval = POE_ERR_NO_MARKED_AREA;
  TRACE_RETURN(rval);
}


POE_ERR markstack_cur_get_start(int* line, int* col)
{
  TRACE_ENTER;
  _ensure_have_mark();
  MARK curmark = markstack_current();
  POE_ERR rval;
  if (mark_exists(curmark))
    rval = mark_get_start(curmark, line, col);
  else
    rval = POE_ERR_NO_MARKED_AREA;
  TRACE_RETURN(rval);
}


POE_ERR markstack_cur_get_end(int* line, int* col)
{
  TRACE_ENTER;
  _ensure_have_mark();
  MARK curmark = markstack_current();
  POE_ERR rval;
  if (mark_exists(curmark))
    rval = mark_get_end(curmark, line, col);
  else
    rval = POE_ERR_NO_MARKED_AREA;
  TRACE_RETURN(rval);
}


POE_ERR markstack_cur_get_bounds(enum marktype *typ, int* l1, int* c1, int* l2, int* c2)
{
  TRACE_ENTER;
  _ensure_have_mark();
  MARK curmark = markstack_current();
  POE_ERR rval;
  if (mark_exists(curmark))
    rval = mark_get_bounds(curmark, typ, l1, c1, l2, c2);
  else
    rval = POE_ERR_NO_MARKED_AREA;
  TRACE_RETURN(rval);
}


POE_ERR markstack_cur_place(enum marktype typ, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  _ensure_have_mark();
  MARK curmark = markstack_current();
  POE_ERR rval;
  buffer_must_exist(__func__, buf);
  if (mark_exists(curmark))
    rval = mark_place(curmark, typ, buf, line, col);
  else
    rval = POE_ERR_NO_MARKED_AREA;
  TRACE_RETURN(rval);
}


POE_ERR markstack_cur_start(enum marktype typ, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  _ensure_have_mark();
  MARK curmark = markstack_current();
  POE_ERR rval;
  if (mark_exists(curmark))
    rval = mark_start(curmark, typ, buf, line, col);
  else
    rval = POE_ERR_NO_MARKED_AREA;
  TRACE_RETURN(rval);
}


POE_ERR markstack_cur_extend(enum marktype typ, BUFFER buf, int line, int col)
{
  TRACE_ENTER;
  _ensure_have_mark();
  MARK curmark = markstack_current();
  POE_ERR rval;
  if (mark_exists(curmark))
    rval = mark_extend(curmark, typ, buf, line, col);
  else
    rval = POE_ERR_NO_MARKED_AREA;
  TRACE_RETURN(rval);
}


void markstack_cur_seal(void)
{
  TRACE_ENTER;
  _ensure_have_mark();
  MARK curmark = markstack_current();
  if (mark_exists(curmark))
    mark_seal(curmark);
  TRACE_EXIT;
}



//
// internal
//

void _ensure_have_mark()
{
  if (pivec_count(&_mark_stack) == 0)
    markstack_push();
}


