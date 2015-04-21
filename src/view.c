
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <curses.h>

#include "trace.h"
#include "poe_err.h"
#include "utils.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "mark.h"
#include "tabstops.h"
#include "key_interp.h"
#include "buffer.h"
#include "view.h"


#define MAX_CURSOR_COL (65535)

struct viewport_t {
  int flags;
  BUFFER buf;
  MARK cursor;
  MARK topleft;
  int ht; // number of lines visible in viewport
  int wd; // number of columns visible in viewport
};

typedef struct viewport_t VIEW;
//typedef struct viewport_t* VIEWPTR;


void _view_init(VIEWPTR pview, BUFFER buf, int rows, int cols);
void _view_destroy(VIEWPTR pview);

void _view_move_port_to(VIEWPTR pview, int row, int col);
void _view_move_port_by(VIEWPTR pview, int rows, int cols);
void _view_move_curs_to(VIEWPTR pview, int row, int col);
void _view_move_curs_by(VIEWPTR pview, int rows, int cols);



VIEWPTR view_alloc(BUFFER buf, int rows, int cols)
{
  TRACE_ENTER;
  VIEW* pview = (VIEW*)calloc(1, sizeof(struct viewport_t));
  _view_init(pview, buf, rows, cols);
  TRACE_RETURN(pview);
}


void view_free(VIEWPTR pview)
{
  TRACE_ENTER;
  _view_destroy(pview);
  free(pview);
  TRACE_EXIT;
}


BUFFER view_buffer(VIEWPTR pview)
{
  TRACE_ENTER;
  BUFFER buf = pview->buf;
  TRACE_RETURN(buf);
}


void view_get_port(VIEWPTR pview, int* ptop, int* pleft, int* pbot, int* pright)
{
  TRACE_ENTER;
  mark_get_bookmark(pview->topleft, Marktype_Block, ptop, pleft);
  *pbot = *ptop + pview->ht;
  *pright = *pleft + pview->wd;
  TRACE_EXIT;
}


void view_get_portsize(VIEWPTR pview, int* pheight, int* pwidth)
{
  TRACE_ENTER;
  *pheight = pview->ht;
  *pwidth = pview->wd;
  TRACE_EXIT;
}


void view_get_cursor(VIEWPTR pview, int* pline, int* pcol)
{
  TRACE_ENTER;
  mark_get_bookmark(pview->cursor, Marktype_Char, pline, pcol);
  TRACE_EXIT;
}


void view_resize(VIEWPTR pview, int rows, int cols)
{
  TRACE_ENTER;
  pview->ht = max(1, rows);
  pview->wd = max(1, cols);
  TRACE_EXIT;
}


void view_set_vsize(VIEWPTR pview, int ht)
{
  TRACE_ENTER;
  view_resize(pview, ht, pview->wd);
  TRACE_EXIT;
}


void view_set_hsize(VIEWPTR pview, int wd)
{
  TRACE_ENTER;
  view_resize(pview, pview->ht, wd);
  TRACE_EXIT;
}


int view_get_insertmode(VIEWPTR pview)
{
  TRACE_ENTER;
  int rval = view_tstflags(pview, VIEW_FLG_INSERTMODE);
  TRACE_RETURN(rval);
}


void view_set_insertmode(VIEWPTR pview, int insertmode)
{
  TRACE_ENTER;
  if (insertmode)
    view_setflags(pview, VIEW_FLG_INSERTMODE);
  else
    view_clrflags(pview, VIEW_FLG_INSERTMODE);
  TRACE_EXIT;
}


void view_toggle_insertmode(VIEWPTR pview)
{
  TRACE_ENTER;
  view_set_insertmode(pview, !view_tstflags(pview, VIEW_FLG_INSERTMODE));
  TRACE_EXIT;
}



//
// internal funcs
//

void _view_init(VIEWPTR pview, BUFFER buf, int rows, int cols)
{
  TRACE_ENTER;
  pview->flags = VIEW_FLG_INSERTMODE;
  pview->buf = buf;
  pview->topleft = mark_alloc(MARK_FLG_BOOKMARK);
  mark_bookmark(pview->topleft, Marktype_Block, buf, -1, 0);
  pview->cursor = mark_alloc(MARK_FLG_BOOKMARK);
  mark_bookmark(pview->cursor, Marktype_Char, buf, 0, 0);
  view_resize(pview, rows, cols);
  TRACE_EXIT;
}


void _view_destroy(VIEWPTR pview)
{
  TRACE_ENTER;
  mark_free(pview->cursor);
  pview->cursor = MARK_NULL;
  mark_free(pview->topleft);
  pview->topleft = MARK_NULL;
  TRACE_EXIT;
}


void _view_move_port_to(VIEWPTR pview, int row, int col)
{
  TRACE_ENTER;
  if (buffer_tstflags(pview->buf, BUF_FLG_CMDLINE)) {
    row = 0;
  }
  int nlines = buffer_count(pview->buf);
  row = min(nlines - pview->ht, row);
  col = min(MAX_CURSOR_COL, col);
  row = max(-1, row);
  col = max(0, col);
  mark_move_bookmark(pview->topleft, Marktype_Block, row, col);
  TRACE_EXIT;
}


void _view_move_port_by(VIEWPTR pview, int rows, int cols)
{
  TRACE_ENTER;
  int port_row, port_col;
  mark_get_bookmark(pview->topleft, Marktype_Block, &port_row, &port_col);
  _view_move_port_to(pview, port_row+rows, port_col+cols);
  TRACE_EXIT;
}


void _view_move_curs_to(VIEWPTR pview, int row, int col)
{
  TRACE_ENTER;
  if (buffer_tstflags(pview->buf, BUF_FLG_CMDLINE)) {
     row = 0;
  }
  int orig_row = row;
  int nlines = buffer_count(pview->buf);
  row = min(nlines-1, row);
  col = min(MAX_CURSOR_COL, col);
  row = max(0, row);
  col = max(0, col);
  
  int p_top, p_left, p_bot, p_right;
  view_get_port(pview, &p_top, &p_left, &p_bot, &p_right);
  
  // If we were moving up before the start, then move the viewport
  // up so he can see the SOF mark
  if (orig_row < 0) {
     row = 0;
     _view_move_port_to(pview, -1, p_left);
     view_get_port(pview, &p_top, &p_left, &p_bot, &p_right);
  }
  
  if (!(row >= p_top && row <= p_bot)) {
     _view_move_port_to(pview, row - (pview->ht >> 1), p_left);
     view_get_port(pview, &p_top, &p_left, &p_bot, &p_right);
  }
  
  if (!(col >= p_left && col < p_right)) {
     _view_move_port_to(pview, p_top, col - (pview->wd >> 1));
     view_get_port(pview, &p_top, &p_left, &p_bot, &p_right);
  }
  
  // if we were moving past the end, then move the viewport down so he can see the BOF mark
  if (orig_row >= nlines)
     _view_move_port_by(pview, 1, 0);
  
  mark_move_bookmark(pview->cursor, Marktype_Char, row, col);
  TRACE_EXIT;
}


void _view_move_curs_by(VIEWPTR pview, int rows, int cols)
{
  TRACE_ENTER;
  int curs_row, curs_col;
  mark_get_bookmark(pview->cursor, Marktype_Char, &curs_row, &curs_col);
  _view_move_curs_to(pview, curs_row+rows, curs_col+cols);
  TRACE_EXIT;
}


void view_setflags(VIEWPTR pview, int flags)
{
  TRACE_ENTER;
  pview->flags |= flags;
  TRACE_EXIT;
}


void view_clrflags(VIEWPTR pview, int flags)
{
  TRACE_ENTER;
  pview->flags &= ~flags;
  TRACE_EXIT;
}


int view_tstflags(VIEWPTR pview, int flags)
{
  TRACE_ENTER;
  int rval = (pview->flags & flags) == flags;
  TRACE_RETURN(rval);
}



//
// command support
//
POE_ERR view_move_cursor_by(VIEWPTR pview, int rows, int cols)
{
  TRACE_ENTER;
  _view_move_curs_by(pview, rows, cols);
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR view_move_port_by(VIEWPTR pview, int rows, int cols)
{
  TRACE_ENTER;
  _view_move_port_by(pview, rows, cols);
  TRACE_RETURN(POE_ERR_OK);
}

POE_ERR view_move_cursor_to(VIEWPTR pview, int rows, int cols)
{
  TRACE_ENTER;
  _view_move_curs_to(pview, rows, cols);
  TRACE_RETURN(POE_ERR_OK);
}


POE_ERR view_move_port_to(VIEWPTR pview, int rows, int cols)
{
  TRACE_ENTER;
  _view_move_port_to(pview, rows, cols);
  TRACE_RETURN(POE_ERR_OK);
}


