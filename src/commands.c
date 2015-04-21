
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <ctype.h>

#include "trace.h"
#include "logging.h"
#include "poe_err.h"
#include "poe_exit.h"
#include "utils.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "mark.h"
#include "markstack.h"
#include "tabstops.h"
#include "margins.h"
#include "key_interp.h"
#include "buffer.h"
#include "view.h"
#include "window.h"
#include "commands.h"
#include "cmd_interp.h"
#include "editor_globals.h"
#include "getkey.h"
#include "parser.h"


// from kbd_interp.c
void defkey(PROFILEPTR prof, const char* keyname, const intptr_t* cmds, size_t ncmds);


extern POE_ERR __cmd_err;

struct cmddef_t {
  const char** cmdspec;
  command_handler_t func;
};


struct vec_t __cmd_defs;


void defcmds(void);


void init_commands(void)
{
  TRACE_ENTER;
  vec_init(&__cmd_defs, 100, sizeof(struct cmddef_t));
  defcmds();
  TRACE_EXIT;
}


void close_commands(void)
{
  TRACE_ENTER;
  vec_destroy(&__cmd_defs);
  TRACE_EXIT;
}


void defcmd(const char** cmdspec, command_handler_t func)
{
  TRACE_ENTER;
  struct cmddef_t newcmddef;
  newcmddef.cmdspec = cmdspec;
  newcmddef.func = func;
  vec_append(&__cmd_defs, &newcmddef);
  TRACE_EXIT;
}


command_handler_t lookup_command(const pivec* cmdseq, int pc, int* args_idx)
{
  TRACE_ENTER;
  int i, j, n = vec_count(&__cmd_defs);
  for (i = 0; i < n; i++) {
    struct cmddef_t* cmddef = vec_get(&__cmd_defs, i);
    for (j = 0;
        !END_OF_CMD(cmdseq,pc+j)
          && cmddef->cmdspec[j] != NULL
          && CMD_IS_STR(pivec_get(cmdseq, pc+j)) 
          && strcasecmp(CMD_STRVAL(pivec_get(cmdseq, pc+j)), cmddef->cmdspec[j]) == 0;
        j++)
     ;
    if (cmddef->cmdspec[j] == NULL) {
     *args_idx = pc+j;
     command_handler_t func = cmddef->func;
     TRACE_RETURN(func);
    }
  }
  *args_idx = -1;
  TRACE_RETURN(NULL);
}


POE_ERR _savelines_other(BUFFER src, int line, int nlines)
{
  TRACE_ENTER;
  char hdr_line[PATH_MAX+64];
  const char* buffername = buffer_name(src);
  snprintf(hdr_line, sizeof(hdr_line), "********** %s %d %d **********", 
            buffername==NULL?"???":buffername,
            line+1, line+nlines+1);
  buffer_insertblanklines(unnamed_buffer, 0, 2, true);
  POE_ERR err = buffer_copyinsertlines(unnamed_buffer, 1, src, line, nlines, true);
  buffer_insertstrn(unnamed_buffer, 0, 0, hdr_line, strlen(hdr_line), true);
  TRACE_RETURN(err);
}


void xtract_targ_context(cmd_ctx* ctx, WINPTR* w, VIEWPTR* v, BUFFER* b, int* r, int* c)
{
  TRACE_ENTER;
  *w=ctx->wnd;
  *v=ctx->targ_view;
  *b=ctx->targ_buf;
  *r=ctx->targ_row;
  *c=ctx->targ_col;
  TRACE_EXIT;
}


void xtract_data_context(cmd_ctx* ctx, WINPTR* w, VIEWPTR* v, BUFFER* b, int* r, int* c)
{
  TRACE_ENTER;
  *w=ctx->wnd;
  *v=ctx->data_view;
  *b=ctx->data_buf;
  *r=ctx->data_row;
  *c=ctx->data_col;
  TRACE_EXIT;
}


void xtract_cmd_context(cmd_ctx* ctx, WINPTR* w, VIEWPTR* v, BUFFER* b, int* r, int* c)
{
  TRACE_ENTER;
  *w=ctx->wnd;
  *v=ctx->cmd_view;
  *b=ctx->cmd_buf;
  *r=ctx->cmd_row;
  *c=ctx->cmd_col;
  TRACE_EXIT;
}



#define CMD_RETURN(e) TRACE_RETURN(e)

#define CMD_ENTER(ctx) TRACE_ENTER

#define CMD_ENTER_BND(ctx,w,v,b,r,c)           \
  TRACE_ENTER;                             \
  WINPTR w; VIEWPTR v; BUFFER b; int r, c;     \
  xtract_targ_context(ctx, &w,&v,&b,&r,&c);
  
#define CMD_ENTER_DATAONLY(ctx)            \
  TRACE_ENTER;                             \
  if (ctx->targ_buf == ctx->cmd_buf)           \
    CMD_RETURN(POE_ERR_UNK_CMD)

#define CMD_ENTER_DATAONLY_BND(ctx,w,v,b,r,c)     \
  TRACE_ENTER;                                \
  if (ctx->targ_buf == ctx->cmd_buf)              \
    CMD_RETURN(POE_ERR_UNK_CMD);                  \
  WINPTR w; VIEWPTR v; BUFFER b; int r, c;        \
  xtract_targ_context(ctx, &w,&v,&b,&r,&c);



int next_parm_int(cmd_ctx* ctx, int deflt)
{
  TRACE_ENTER;
  int rval = deflt;
  if ((ctx->cmdseq != NULL && ctx->pc >= 0)
     && (!(END_OF_CMD(ctx->cmdseq, ctx->pc) || END_OF_CMDSEQ(ctx->cmdseq, ctx->pc)))
     && (CMD_IS_INT(pivec_get(ctx->cmdseq, ctx->pc)))) {
    rval = CMD_INTVAL(pivec_get(ctx->cmdseq, ctx->pc));
    ctx->pc++;
  }
  TRACE_RETURN(rval);
}


const char* next_parm_str(cmd_ctx* ctx, const char* deflt)
{
  CMD_ENTER(ctx);
  const char* rval = deflt;
  if ((ctx->cmdseq != NULL && ctx->pc >= 0)
     && (!(END_OF_CMD(ctx->cmdseq, ctx->pc) || END_OF_CMDSEQ(ctx->cmdseq, ctx->pc)))
     && (CMD_IS_STR(pivec_get(ctx->cmdseq, ctx->pc)))) {
    rval = CMD_STRVAL(pivec_get(ctx->cmdseq, ctx->pc));
    ctx->pc++;
  }
  CMD_RETURN(rval);
}


bool next_parm_is_int(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  if (ctx->cmdseq != NULL && ctx->pc >= 0 && CMD_IS_INT(pivec_get(ctx->cmdseq, ctx->pc)))
    CMD_RETURN(true);
  CMD_RETURN(false);
}


bool next_parm_is_str(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  bool rval = false;
  if (ctx->cmdseq != NULL && ctx->pc >= 0) {
    if (END_OF_CMD(ctx->cmdseq, ctx->pc) || END_OF_CMDSEQ(ctx->cmdseq, ctx->pc))
     rval = false;
    else if (CMD_IS_STR(pivec_get(ctx->cmdseq, ctx->pc)))
     rval = true;
    else
     rval = false;
  }
  else {
    rval = false;
  }
  CMD_RETURN(rval);
}


POE_ERR cmd_erase_command_line(cmd_ctx* ctx);

void _printf_cmdline(cmd_ctx* ctx, const char* fmt, ...)
{
  char tmp[128];
  va_list ap;
  va_start(ap, fmt);
  cmd_erase_command_line(ctx);
  vsnprintf(tmp, sizeof(tmp), fmt, ap);
  buffer_insertstrn(ctx->cmd_buf, 0, 0, tmp, strlen(tmp), true);
  view_move_cursor_to(ctx->cmd_view, 0, 0);
  va_end(ap);
}



POE_ERR cmd_erase_command_line(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  buffer_removechars(ctx->cmd_buf, 0, 0, buffer_line_length(ctx->cmd_buf, 0), true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_exit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  __quit = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR _cmd_right(cmd_ctx* ctx, int ncols)
{
  CMD_ENTER(ctx);
  POE_ERR err = view_move_cursor_by(ctx->targ_view, 0, ncols);
  CMD_RETURN(err);
}


POE_ERR cmd_right(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int ncols = next_parm_int(ctx, 1);
  POE_ERR err = _cmd_right(ctx, ncols);
  CMD_RETURN(err);
}


POE_ERR _cmd_left(cmd_ctx* ctx, int ncols)
{
  CMD_ENTER(ctx);
  POE_ERR err = view_move_cursor_by(ctx->targ_view, 0, -ncols);
  CMD_RETURN(err);
}


POE_ERR cmd_left(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int ncols = next_parm_int(ctx, 1);
  POE_ERR err = _cmd_left(ctx, ncols);
  CMD_RETURN(err);
}


POE_ERR cmd_left_wrap(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, win, view, buf, row, col);
  int i, npos = next_parm_int(ctx, 1);
  for (i = 0; i < npos; i++)
    buffer_left_wrap(buf, &row, &col);
  POE_ERR err = view_move_cursor_to(view, row, col);
  CMD_RETURN(err);
}


POE_ERR cmd_right_wrap(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, win, view, buf, row, col);
  int i, npos = next_parm_int(ctx, 1);
  for (i = 0; i < npos; i++)
    buffer_right_wrap(buf, &row, &col);
  POE_ERR err = view_move_cursor_to(view, row, col);
  CMD_RETURN(err);
}


POE_ERR _cmd_up(cmd_ctx* ctx, int nrows)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  if (buffer_tstflags(ctx->targ_buf, BUF_FLG_CMDLINE)) {
    // move up from commandline = move to last visible row in data buf
    int cmd_col = ctx->cmd_col;
    win_set_commandmode(ctx->wnd, false);
    int top, left, bot, right;
    view_get_port(ctx->targ_view, &top, &left, &bot, &right);
    err = view_move_cursor_to(ctx->targ_view, bot, cmd_col);
  }
  else {
    err = view_move_cursor_by(ctx->targ_view, -nrows, 0);
  }
  CMD_RETURN(err);
}


POE_ERR cmd_up(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int nrows = next_parm_int(ctx, 1);
  POE_ERR err = _cmd_up(ctx, nrows);
  CMD_RETURN(err);
}


POE_ERR _cmd_down(cmd_ctx* ctx, int nrows)
{
  TRACE_ENTER;
  POE_ERR err = view_move_cursor_by(ctx->targ_view, nrows, 0);
  TRACE_RETURN(err);
}


POE_ERR cmd_down(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int nrows = next_parm_int(ctx, 1);
  POE_ERR err = _cmd_down(ctx, nrows);
  CMD_RETURN(err);
}


POE_ERR cmd_move_view_left(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int ncols = next_parm_int(ctx, 1);
  POE_ERR err = view_move_port_by(ctx->targ_view, 0, -ncols);
  CMD_RETURN(err);
}


POE_ERR cmd_move_view_right(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int ncols = next_parm_int(ctx, 1);
  POE_ERR err = view_move_port_by(ctx->targ_view, 0, ncols);
  CMD_RETURN(err);
}


POE_ERR cmd_move_view_up(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int nrows = next_parm_int(ctx, 1);
  POE_ERR err = view_move_port_by(ctx->targ_view, -nrows, 0);
  CMD_RETURN(err);
}


POE_ERR cmd_move_view_down(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int nrows = next_parm_int(ctx, 1);
  POE_ERR err = view_move_port_by(ctx->targ_view, nrows, 0);
  CMD_RETURN(err);
}


POE_ERR cmd_page_up(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int ht, wd;
  view_get_portsize(ctx->targ_view, &ht, &wd);
  POE_ERR err = _cmd_up(ctx, ht);
  CMD_RETURN(err);
}


POE_ERR cmd_page_down(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int ht, wd;
  view_get_portsize(ctx->targ_view, &ht, &wd);
  POE_ERR err = _cmd_down(ctx, ht);
  CMD_RETURN(err);
}


POE_ERR cmd_begin_line(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = view_move_cursor_to(ctx->targ_view, ctx->targ_row, 0);
  CMD_RETURN(err);
}


POE_ERR cmd_end_line(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int linelen = buffer_line_length(ctx->targ_buf, ctx->targ_row);
  int lastpos = buffer_scantill_nowrap(ctx->targ_buf, ctx->targ_row, linelen-1, -1, poe_isnotwhitespace);
  POE_ERR err = view_move_cursor_to(ctx->targ_view, ctx->targ_row, lastpos+1);
  CMD_RETURN(err);
}


POE_ERR cmd_insert_toggle(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  view_toggle_insertmode(ctx->targ_view);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_insert_mode(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  view_set_insertmode(ctx->targ_view, true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_replace_mode(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  view_set_insertmode(ctx->targ_view, false);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_command_toggle(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  win_toggle_commandmode(ctx->wnd);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_cursor_command(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  win_set_commandmode(ctx->wnd, true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_cursor_data(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  win_set_commandmode(ctx->wnd, false);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_tab(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int newcol = buffer_nexttab(ctx->targ_buf, ctx->targ_col);
  view_move_cursor_to(ctx->targ_view, ctx->targ_row, newcol);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_backtab(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int newcol = buffer_prevtab(ctx->targ_buf, ctx->targ_col);
  view_move_cursor_to(ctx->targ_view, ctx->targ_row, newcol);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_top(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  view_move_cursor_to(ctx->targ_view, 0, ctx->targ_col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_bottom(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int nlines = buffer_count(ctx->targ_buf);
  view_move_cursor_to(ctx->targ_view, nlines-1, ctx->targ_col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR _cmd_chr(cmd_ctx* ctx, char chr)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  POE_ERR err = POE_ERR_OK;
  int insert_mode = view_get_insertmode(ctx->targ_view);
  if (iscntrl(chr))
    chr = ' ';
  if (insert_mode) {
    buffer_insert(buf, row, col, chr, true);
  }
  else {
    buffer_setchar(buf, row, col, chr);
    // In overstrike mode the mark adjustment code doesn't
    // automagically move the cursor.
    err = _cmd_right(ctx, 1);
  }
  
  if (chr == ' ' && autowrap) {
    buffer_wrap_line(buf, row, -1, -1, -1, -1, true);
	if (insert_mode) {
	  update_context(ctx);
	  xtract_targ_context(ctx, &wnd, &view, &buf, &row, &col);
	  if (col > 0 && buffer_getchar(buf, row, col-1) != ' ')
	  	buffer_insert(buf, row, col, ' ', true);
	}
  }
  CMD_RETURN(err);
}


POE_ERR cmd_chr(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  if (ctx->cmdseq == NULL || !next_parm_is_int(ctx)) {
    err = POE_ERR_INVALID_KEY;
  }
  else {
    int chr = next_parm_int(ctx, ' ');
    err = _cmd_chr(ctx, chr);
  }
  CMD_RETURN(err);
}


POE_ERR cmd_str(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  POE_ERR err = POE_ERR_OK;
  if (!next_parm_is_str(ctx)) {
    err = POE_ERR_INVALID_KEY;
  }
  else {
    const char* str = next_parm_str(ctx, "");
    char* safe = strsave(str);
    int i;
    int hasspace = false;
    for (i = 0; safe[i] != '\0'; i++)
      if (iscntrl(safe[i]) || isspace(safe[i])) {
        safe[i] = ' ';
        hasspace = true;
      }
    int insert_mode = view_get_insertmode(view);
    if (insert_mode) {
      int n = strlen(safe);
      buffer_insertstrn(buf, row, col, str, n, true);
    }
    else {
      buffer_setstrn(buf, row, col, str, strlen(safe), true);
      err = _cmd_right(ctx, strlen(str));
    }
    free(safe);
    if (hasspace && autowrap)
      buffer_wrap_line(buf, row, row, -1, -1, -1, true);
  }
  CMD_RETURN(err);
}


POE_ERR cmd_rubout(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  int targ_col = max(0, ctx->targ_col-1);
  if (targ_col >= buffer_line_length(ctx->targ_buf, ctx->targ_row)) {
    view_move_cursor_to(ctx->targ_view, ctx->targ_row, targ_col); 
  }
  else {
    buffer_removechar(ctx->targ_buf, ctx->targ_row, targ_col, true);
  }
  CMD_RETURN(err);
}


POE_ERR cmd_delete_char(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  buffer_removechar(ctx->targ_buf, ctx->targ_row, ctx->targ_col, true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_top_edge(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int top, left, bot, right;
  view_get_port(ctx->targ_view, &top, &left, &bot, &right);
  view_move_cursor_to(ctx->targ_view, top, ctx->targ_col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_bottom_edge(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int top, left, bot, right;
  view_get_port(ctx->targ_view, &top, &left, &bot, &right);
  view_move_cursor_to(ctx->targ_view, bot, ctx->targ_col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_right_edge(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int top, left, bot, right;
  view_get_port(ctx->targ_view, &top, &left, &bot, &right);
  view_move_cursor_to(ctx->targ_view, ctx->targ_row, right);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_left_edge(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int top, left, bot, right;
  view_get_port(ctx->targ_view, &top, &left, &bot, &right);
  view_move_cursor_to(ctx->targ_view, ctx->targ_row, left);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_center_line(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx)
  int ht, wd;
  view_get_portsize(ctx->targ_view, &ht, &wd);
  int top, left, bot, right;
  view_get_port(ctx->targ_view, &top, &left, &bot, &right);
  int dest_line = ctx->targ_row - (ht>>1);
  view_move_port_to(ctx->targ_view, dest_line, left);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_indent(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  if (!ctx->src_is_commandline) {
	int leftmargin, rightmargin, paragraphmargin;
	buffer_getmargins(buf, &leftmargin, &rightmargin, &paragraphmargin);
	if (row == 0 || buffer_isblankline(buf, row-1))
	  view_move_cursor_to(view, row, paragraphmargin);
	else
	  view_move_cursor_to(view, row, leftmargin);
  }
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_erase_begin_line(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  _savelines_other(ctx->targ_buf, ctx->targ_row, 1);
  buffer_removechars(ctx->targ_buf, ctx->targ_row, 0, ctx->targ_col, true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_erase_end_line(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  _savelines_other(buf, row, 1);
  int len = buffer_line_length(buf, row);
  int nCharsToRemove = max(0, len-col);
  buffer_removechars(buf, row, col, nCharsToRemove, true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_delete_line(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  _savelines_other(buf, row, 1);
  if (buffer_count(buf) == 1) {
    int len = buffer_line_length(buf, 0);
    buffer_removechars(buf, 0, 0, len, true);
  }
  else {
    buffer_removelines(buf, row, 1, true);
  }
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_insert_line(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  int nlines = next_parm_int(ctx, 1);
  buffer_insertblanklines(buf, row+1, nlines, true);
  view_move_cursor_to(view, row+nlines, 0);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_tab_word(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  buffer_scantill_wrap(buf, &row, &col, 1, poe_iswhitespace);
  buffer_scantill_wrap(buf, &row, &col, 1, poe_isnotwhitespace);
  view_move_cursor_to(view, row, col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_backtab_word(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  buffer_scantill_wrap(buf, &row, &col, -1, poe_iswhitespace);
  buffer_scantill_wrap(buf, &row, &col, -1, poe_isnotwhitespace);
  buffer_scantill_wrap(buf, &row, &col, -1, poe_iswhitespace);
  if (poe_iswhitespace(buffer_getchar(buf, row, col)))
    buffer_right_wrap(buf, &row, &col);
  view_move_cursor_to(view, row, col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_begin_word(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  if (poe_iswhitespace(buffer_getchar(buf, row, col))) {
    buffer_scantill_wrap(buf, &row, &col, 1, poe_isnotwhitespace);
  }
  else {
    buffer_scantill_wrap(buf, &row, &col, -1, poe_iswhitespace);
    if (poe_iswhitespace(buffer_getchar(buf, row, col)))
     buffer_right_wrap(buf, &row, &col);
  }
  view_move_cursor_to(view, row, col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_end_word(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  if (poe_iswhitespace(buffer_getchar(buf, row, col))) {
    buffer_scantill_wrap(buf, &row, &col, -1, poe_isnotwhitespace);
  }
  else {
    buffer_scantill_wrap(buf, &row, &col, 1, poe_iswhitespace);
    buffer_left_wrap(buf, &row, &col);
  }
  view_move_cursor_to(view, row, col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_first_nonblank(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  if (buffer_isblankline(buf, row)) {
    view_move_cursor_to(view, row, 0);
  }
  else {
    int newcol = buffer_scantill_nowrap(buf, row, 0, 1, poe_isnotwhitespace);
    view_move_cursor_to(view, row, newcol);
  }
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_find_blank_line(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  int newrow = buffer_findblankline(buf, row, 1);
  view_move_cursor_to(view, newrow, col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_find_prev_blank_line(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  int newrow = buffer_findblankline(buf, row, -1);
  view_move_cursor_to(view, newrow, col);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_split(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  _savelines_other(ctx->targ_buf, ctx->targ_row, 1);
  POE_ERR err = buffer_splitline(ctx->targ_buf, ctx->targ_row, ctx->targ_col, true);
  CMD_RETURN(err);
}


POE_ERR cmd_join(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int nrows = buffer_count(ctx->targ_buf);
  if (ctx->targ_row >= nrows-1)
    CMD_RETURN(POE_ERR_OK);
  _savelines_other(ctx->targ_buf, ctx->targ_row, 2);
  buffer_joinline(ctx->targ_buf, ctx->targ_row, true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_rubout_join(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  POE_ERR err = POE_ERR_OK;
  if (row == 0 && col == 0)
    CMD_RETURN(err);
  if (col == 0) {
    _savelines_other(buf, row-1, 2);
    view_move_cursor_to(view, row-1, buffer_line_length(buf, row-1));
    row = row-1;
    err = buffer_joinline(buf, row-1, true);
  }
  else {
    err = cmd_rubout(ctx);
  }
  CMD_RETURN(err);
}


POE_ERR cmd_unmark(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  POE_ERR rval = markstack_cur_unmark();
  CMD_RETURN(rval);
}


POE_ERR cmd_mark_line(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  POE_ERR rval = markstack_cur_place(Marktype_Line, ctx->targ_buf, ctx->targ_row, ctx->targ_col);
  CMD_RETURN(rval);
}


POE_ERR cmd_mark_block(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  POE_ERR rval = markstack_cur_place(Marktype_Block, ctx->targ_buf, ctx->targ_row, ctx->targ_col);
  CMD_RETURN(rval);
}


POE_ERR cmd_mark_char(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  POE_ERR rval = markstack_cur_place(Marktype_Char, ctx->targ_buf, ctx->targ_row, ctx->targ_col);
  CMD_RETURN(rval);
}


POE_ERR cmd_begin_mark(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK || typ == Marktype_None)
    CMD_RETURN(POE_ERR_NO_MARKED_AREA);
  // Switch buffers if necessary
  BUFFER markbuf = BUFFER_NULL;
  err = markstack_cur_get_buffer(&markbuf);
  if (buf != markbuf) {
    wins_cur_switchbuffer(markbuf);
    update_context(ctx);
    xtract_targ_context(ctx, &wnd, &view, &buf, &row, &col);
  }
  // Move to the mark start
  if (err == POE_ERR_OK) {
    switch (typ) {
    case Marktype_Line:
     err = view_move_cursor_to(view, l1, col);
     break;
    case Marktype_Char: case Marktype_Block:
     err = view_move_cursor_to(view, l1, c1);
     break;
    case Marktype_None: default:
     err = POE_ERR_NO_MARKED_AREA;
     break;
    }
  }
  CMD_RETURN(err);
}


POE_ERR cmd_end_mark(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (typ == Marktype_None)
    CMD_RETURN(POE_ERR_NO_MARKED_AREA);
  // Switch buffers if necessary
  BUFFER markbuf = BUFFER_NULL;
  err = markstack_cur_get_buffer(&markbuf);
  if (buf != markbuf) {
    wins_cur_switchbuffer(markbuf);
    update_context(ctx);
    xtract_targ_context(ctx, &wnd, &view, &buf, &row, &col);
  }
  // Move to the mark end
  if (err == POE_ERR_OK) {
    switch (typ) {
    case Marktype_Line:
     err = view_move_cursor_to(view, l2, col);
     break;
    case Marktype_Char: case Marktype_Block:
     err = view_move_cursor_to(view, l2, c2);
     break;
    case Marktype_None: default:
     err = POE_ERR_NO_MARKED_AREA;
     break;
    }
  }
  CMD_RETURN(err);
}


POE_ERR cmd_push_mark(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  markstack_push();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_pop_mark(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  POE_ERR rval = markstack_pop();
  CMD_RETURN(rval);
}


POE_ERR cmd_resize_display(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  wins_resize();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_next_file(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  wins_cur_nextbuffer();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_next_window(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  wins_nextwindow();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_next_view(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  wins_nextview();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_split_screen(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  wins_split();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_zoom_window(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  wins_zoom();
  CMD_RETURN(POE_ERR_OK);
}

// !!!!This uses 1-based line and column numbers
POE_ERR cmd_line(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int targ_row = next_parm_int(ctx, ctx->targ_row+1);
  int targ_col = next_parm_int(ctx, ctx->targ_col+1);
  view_move_cursor_to(ctx->targ_view, targ_row-1, targ_col-1);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_column(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int targ_col = next_parm_int(ctx, ctx->targ_col);
  view_move_cursor_to(ctx->targ_view, ctx->targ_row, targ_col);
  CMD_RETURN(POE_ERR_OK);
}


// Used by both upper and lower case operations
typedef void (*upperop_t)(BUFFER, int, int, int);
POE_ERR _cmd_upperlower(cmd_ctx* ctx, upperop_t op)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  int i;
  switch (typ) {
  case Marktype_Line:
    _savelines_other(buf, l1, l2-l1+1);
    for (i = l1; i <= l2; i++)
      (*op)(buf, i, 0, buffer_line_length(buf, i));
    err = POE_ERR_OK;
    break;
  case Marktype_Block:
    {
      _savelines_other(buf, l1, l2-l1+1);
      int mark_width = c2 - c1 + 1;
      for (i = l1; i <= l2; i++)
        (*op)(buf, i, c1, mark_width);
      err = POE_ERR_OK;
    }
    break;
  case Marktype_Char:
    _savelines_other(buf, l1, l2-l1+1);
    if (l1 == l2) {
      (*op)(buf, l1, c1, c2-c1+1);
    }
    else {
      (*op)(buf, l1, c1, max(0, buffer_line_length(buf, l1) - c1));
      for (i = l1+1; i < l2; i++)
        (*op)(buf, i, 0, buffer_line_length(buf, i));
      (*op)(buf, l2, 0, c2);
    }
    err = POE_ERR_OK;
    break;
  case Marktype_None: default:
    err = POE_ERR_NO_MARKED_AREA;
    break;
  }
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_uppercase(cmd_ctx* ctx)
{
  return _cmd_upperlower(ctx, buffer_upperchars);
}


POE_ERR cmd_lowercase(cmd_ctx* ctx)
{
  return _cmd_upperlower(ctx, buffer_lowerchars);
}


// Shift left if cols < 0, right if cols > 0
POE_ERR _cmd_shift(cmd_ctx* ctx, int cols_to_shift)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    CMD_RETURN(err); 
  int i;
  switch (typ) {
  case Marktype_Line: case Marktype_Block:
    _savelines_other(buf, l1, l2-l1+1);
    if (typ == Marktype_Line)
     c1 = 0;
    for (i = l1; i <= l2; i++) {
     if (cols_to_shift < 0) {
       buffer_removechars(buf, i, c1, -cols_to_shift, true);
     }
     else if (cols_to_shift > 0) {
       buffer_insertct(buf, i, c1, ' ', cols_to_shift, true);
     }
    }
    err = POE_ERR_OK;
    break;
  case Marktype_Char:
    _savelines_other(buf, l1, l2-l1+1);
    for (i = l1; i <= l2; i++) {
     int col = i == l1 ? c1 : 0;
     if (cols_to_shift < 0) {
       buffer_removechars(buf, i, col, -cols_to_shift, true);
     }
     else if (cols_to_shift > 0){
       buffer_insertct(buf, i, col, ' ', cols_to_shift, true);
     }
    }
    err = POE_ERR_OK;
    break;
  case Marktype_None: default:
    err = POE_ERR_NO_MARKED_AREA;
    break;
  }
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_shift_left(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int cols_to_shift = next_parm_int(ctx, 1);
  POE_ERR err = _cmd_shift(ctx, -cols_to_shift);
  CMD_RETURN(err);
}


POE_ERR cmd_shift_right(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  int cols_to_shift = next_parm_int(ctx, 1);
  POE_ERR err = _cmd_shift(ctx, cols_to_shift);
  CMD_RETURN(err);
}


POE_ERR cmd_clear_marks(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  POE_ERR err;
  for (err = markstack_pop(); err == POE_ERR_OK; err = markstack_pop())
    ;
  markstack_cur_unmark();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_escape(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  const char* scode = next_parm_str(ctx, (const char*)NULL);
  POE_ERR err = POE_ERR_OK;
  if (scode == NULL) {
    // Should instead read next few keys (must be numbers) and insert
    // that ascii value.
    err = POE_ERR_INVALID_KEY;
  }
  else {
    long lcode = strtol(scode, NULL, 10);
    if (lcode > 255) {
     err = POE_ERR_INVALID_KEY;
    }
    else {
     err = _cmd_chr(ctx, (intptr_t)lcode);
    }
  }
  CMD_RETURN(err);
}


// blech
POE_ERR cmd_delete_mark(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  cmd_begin_mark(ctx);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    CMD_RETURN(err); 
  switch (typ) {
  case Marktype_Line:
    {
     _savelines_other(buf, l1, l2-l1+1);
     err = markstack_cur_unmark();
     buffer_removelines(buf, l1, l2-l1+1, true);
     buffer_ensure_min_lines(buf, true);
    }
    break;
  case Marktype_Block:
    {
     _savelines_other(buf, l1, l2-l1+1);
     err = markstack_cur_unmark();
     int i, cols_to_shift = c2-c1+1;
     for (i = l1; i <= l2; i++) {
       buffer_removechars(buf, i, c1, cols_to_shift, true);
     }
    }
    break;
  case Marktype_Char:
    {
     _savelines_other(buf, l1, l2-l1+1);
     err = markstack_cur_unmark();
     if (l1 == l2) {
       buffer_removechars(buf, l1, c1, c2-c1+1, true);
     }
     else {
       int l1len = buffer_line_length(buf, l1);
       int l2len = buffer_line_length(buf, l2);
       int leading = max(0, l1len-c1);
       int trailing = min(c2+1, l2len);
       buffer_removechars(buf, l1, c1, leading, true);
       buffer_removechars(buf, l2, 0, trailing, true);
       if (l2-l1-1>0) {
        buffer_removelines(buf, l1+1, l2-l1-1, true);
       }
       err = buffer_joinline(buf, l1, true);
     }
    }
    break;
  case Marktype_None: default:
    err = POE_ERR_NO_MARKED_AREA;
    break;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_copy_mark(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK || typ == Marktype_None)
    CMD_RETURN(err);
  BUFFER markbuf = BUFFER_NULL;
  err = markstack_cur_get_buffer(&markbuf);
  if (err != POE_ERR_OK)
    CMD_RETURN(err);
  int i, nc, nl;
  switch (typ) {
  case Marktype_Line:
    nl = l2-l1+1;
    err = buffer_copyinsertlines(buf, row+1, markbuf, l1, nl, true);
    break;
  case Marktype_Block:
    nl = l2-l1+1;
    _savelines_other(buf, row, 2);
    nc = c2-c1+1;
    for (i = 0; i < nl; i++) {
     buffer_copyinsertchars(buf, row+i, col, markbuf, l1+i, c1, c2-c1+1, true);
    }
    break;
  case Marktype_Char:
    nl = l2-l1+1;
    _savelines_other(buf, row, 1);
    if (l1 == l2) {
	  //int srclinelen = buffer_line_length(markbuf, l1);
     buffer_copyinsertchars(buf, row, col, markbuf, l1, c1, c2-c1+1, true);
    }
    else if (markbuf != buf || row > l2) {
	  // can safely do it the easy way
	  buffer_splitline(buf, row, col, true);
	  int srclinelen = buffer_line_length(markbuf, l1);
	  // copy first line
	  buffer_copyinsertchars(buf, row, col, markbuf, l1, c1, srclinelen-c1, true);
	  // copy last line
	  srclinelen = buffer_line_length(markbuf, l2);
	  int trl_chrs = min(srclinelen, c2+1);
	  buffer_copyinsertchars(buf, row+1, 0, markbuf, l2, 0, trl_chrs, true);
	  // copy middle lines
	  nl = l2-l1-1;
	  if (nl > 0) {
		err = buffer_copyinsertlines(buf, row+1, markbuf, l1+1, nl, true);
	  }
	}
	else {
	  // have to be careful - the process of copying the text will
	  // move the source mark around.
	  buffer_splitline(buf, row, col, true);
	  // update our knowledge of the source mark after the split
	  markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
	  int fml = row+1, lml = row+1+l2-l1, nml=l2-l1+1;
	  // copy the marked lines in between the two halves of the original line
	  buffer_copyinsertlines(buf, fml, markbuf, l1, nml, true);
	  // delete unneeded half of the first marked line
	  buffer_removechars(buf, fml, 0, c1, true);
	  // delete the unneeded half of the last marked line
	  int len_lml = buffer_line_length(buf, lml);
	  buffer_removechars(buf, lml, c2+1, len_lml-c2-1, true);
	  // join the last half of the original line with the first part
	  // of the last marked line with the
	  buffer_joinline(buf, lml, true);
	  // join the first half line with the surviving half  of the first marked line
	  buffer_joinline(buf, row, true);
    }
    break;
  case Marktype_None: default:
    err = POE_ERR_NO_MARKED_AREA;
    break;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_move_mark(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY(ctx);
  POE_ERR err = cmd_copy_mark(ctx);
  if (err == POE_ERR_OK) {
    err = cmd_delete_mark(ctx);
    // cmd_delete_mark may switch us to the src buffer, so we need to
    // switch back.
    wins_cur_switchbuffer(ctx->data_buf);
    view_move_cursor_to(ctx->data_view, ctx->data_row, ctx->data_col);
  }
  CMD_RETURN(err);
}


POE_ERR cmd_fill_mark(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    CMD_RETURN(err); 
  if (typ == Marktype_None)
    CMD_RETURN(POE_ERR_NO_MARKED_AREA);
  BUFFER markbuf = BUFFER_NULL;
  err = markstack_cur_get_buffer(&markbuf);
  if (err != POE_ERR_OK)
    CMD_RETURN(err);
  int parm_chr = next_parm_int(ctx, -1);
  char chr;
  if (parm_chr >= 0)
	chr = (char)parm_chr;
  else
	err = get_insertable_key(&chr);
  if (err != POE_ERR_OK)
    CMD_RETURN(err); 
  int i, n;
  switch (typ) {
  case Marktype_Line:
    {
     _savelines_other(markbuf, l1, l2-l1+1);
     for (i = l1; i <= l2; i++) {
       n = buffer_line_length(markbuf, i);
       buffer_setcharct(markbuf, i, 0, chr, n);
     }
    }
    err = POE_ERR_OK;
    break;
  case Marktype_Block:
    {
     n = l2-l1+1;
     _savelines_other(markbuf, l1, n);
     for (i = l1; i <= l2; i++)
       buffer_setcharct(markbuf, i, c1, chr, c2-c1+1);
    }
    err = POE_ERR_OK;
    break;
  case Marktype_Char:
    {
     n = l2-l1+1;
     _savelines_other(markbuf, l1, n);
     if (l1 == l2) {
       buffer_setcharct(markbuf, l1, c1, chr, c2-c1+1);
     }
     else {
       buffer_setcharct(markbuf, l1, c1, chr, buffer_line_length(markbuf, l1) - c1);
       for (i = l1+1; i < l2; i++) {
        n = buffer_line_length(markbuf, i);
        buffer_setcharct(markbuf, i, 0, chr, n);
       }
       buffer_setcharct(markbuf, l2, 0, chr, c2);
     }
    }
    err = POE_ERR_OK;
    break;
  case Marktype_None: default:
    err = POE_ERR_NO_MARKED_AREA;
    break;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_confirm_change(cmd_ctx* ctx)
{
  return POE_ERR_OK;
  // PE2 returned POE_ERR_NO_CHANGE_PENDING, but I'm treating it as a
  // no-op for purposes of execution.  It's presence is recognized by
  // the get_confirmation_key() function.
}


POE_ERR cmd_overlay_block(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    CMD_RETURN(err); 
  if (typ == Marktype_None)
    CMD_RETURN(POE_ERR_NO_MARKED_AREA);
  BUFFER markbuf = BUFFER_NULL;
  err = markstack_cur_get_buffer(&markbuf);
  if (err != POE_ERR_OK)
    CMD_RETURN(err);
  int i, nl, nc;
  switch (typ) {
  case Marktype_Block:
    nl = l2-l1+1;
    nc = c2-c1+1;
    _savelines_other(buf, row, nl);
    for (i = 0; i < nl; i++)
     buffer_copyoverlaychars(buf, row+i, col, markbuf, l1+i, c1, nc, true);
    err = POE_ERR_OK;
    break;
  case Marktype_Line: case Marktype_Char: 
    err = POE_ERR_BLOCK_MARK_REQ;
    break;
  case Marktype_None: default:
    err = POE_ERR_NO_MARKED_AREA;
    break;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_reflow(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    CMD_RETURN(err); 
  if (typ != Marktype_Line)
    CMD_RETURN(POE_ERR_LINE_MARK_REQ);
  BUFFER markbuf = BUFFER_NULL;
  err = markstack_cur_get_buffer(&markbuf);
  if (err != POE_ERR_OK)
    CMD_RETURN(err);
  // Switch buffers if necessary
  if (buf != markbuf) {
    wins_cur_switchbuffer(markbuf);
    update_context(ctx);
    xtract_targ_context(ctx, &wnd, &view, &buf, &row, &col);
  }
  
  switch (typ) {
  case Marktype_Line: 
    {
      int leftmargin, rightmargin, paragraphmargin;
      buffer_getmargins(buf, &leftmargin, &rightmargin, &paragraphmargin);
      _savelines_other(buf, l1, l2-l1+1);
      int i;
      int didsplit = false;
      for (i = l1; i <= l2; ) {
        if (i < l2) {
          buffer_insert(buf, i, buffer_line_length(buf, i), ' ', true);
          buffer_joinline(buf, i, true);
        }
        int firstmargin = (i==l1)?paragraphmargin:leftmargin;
        int restmargin = leftmargin;
        didsplit = buffer_wrap_line(buf, i, max(i, l2-1), firstmargin, restmargin, rightmargin, true);
        if (didsplit || i == l2)
          i++;
        markstack_cur_get_end(&l2, &c2);
      }
    }
    break;
  case Marktype_Char: case Marktype_Block:
    err = POE_ERR_LINE_MARK_REQ;
    break;
  case Marktype_None: default:
    err = POE_ERR_NO_MARKED_AREA;
    break;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_execute(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);

  WINPTR wnd; VIEWPTR view; BUFFER cmdbuf;
  int row, col;
  xtract_cmd_context(ctx, &wnd, &view, &cmdbuf, &row, &col);

  // Get command text and parse it
  cstr cmd_text;
  pivec tokens;
  struct line_t* line = buffer_get(cmdbuf, 0);
  cstr_initfrom(&cmd_text, &line->txt);
  pivec_init(&tokens, 10);
  cstr_trimleft(&cmd_text, poe_iswhitespace);
  cstr_trimright(&cmd_text, poe_iswhitespace);
  POE_ERR err = parse_cmdline(&cmd_text, &tokens);

  // Dump out the parsed tokens for debugging
  /* int i, n = pivec_count(&tokens); */
  /* logmsg("parsed tokens:"); */
  /* for (i = 0; i < n; i++) { */
  /*   intptr_t v = pivec_get(&tokens, i); */
  /*   if (CMD_IS_INT(v)) { */
  /*     logmsg("   %2d INT %d", i, CMD_INTVAL(v)); */
  /*   } */
  /*   else if (CMD_IS_STR(v)) { */
  /*     logmsg("   %2d STR '%s'", i, CMD_STRVAL(v)); */
  /*   } */
  /*   else { */
  /*     logmsg("   %2d UNK", i); */
  /*   } */
  /* } */

  // Make sure it's terminated properly, and interpret it
  pivec_append(&tokens, CMD_SEP);
  pivec_append(&tokens, CMD_NULL);

  // figure out our new context for the stuff we're about to interpret
  cmd_ctx cmdline_ctx;
  cmdline_ctx.src_is_commandline = true;
  cmdline_ctx.save_commandline = false;
  update_context(&cmdline_ctx);
  cmdline_ctx.cmdseq = &tokens;
  cmdline_ctx.pc = 0;
  err = interpret_command_seq(&cmdline_ctx);

  // For most commands we should clear the command line afterwards.
  // But not all commands...
  if (err == POE_ERR_OK && !cmdline_ctx.save_commandline) {
    cmd_erase_command_line(ctx);
    /* if (ctx->targ_buf == ctx->cmd_buf) */
    /*   cmd_cursor_data(ctx); */
  }

  cstr_destroy(&cmd_text);
  pivec_destroy(&tokens);
  
  TRACE_RETURN(err);
}


POE_ERR cmd_edit(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  POE_ERR err = POE_ERR_OK;
  const char* filename = next_parm_str(ctx, (const char*)NULL);
  const char* exp_mode = next_parm_str(ctx, (const char*)NULL);
  if (filename == NULL) {
    wins_cur_nextbuffer();
  }
  else {
    bool file_tab_expand = tabexpand;
    if (exp_mode != NULL) {
     if (strcasecmp(exp_mode, "notabs") == 0)
       file_tab_expand = true;
     else if (strcasecmp(exp_mode, "tabs") == 0)
       file_tab_expand = false;
    }
    cstr tmp_filename;
    cstr_initstr(&tmp_filename, filename);
    cstr_trimleft(&tmp_filename, poe_iswhitespace);
    cstr_trimright(&tmp_filename, poe_iswhitespace);
    // logmsg("attempting to load file '%s'", cstr_getbufptr(&tmp_filename));
    BUFFER editbuf = BUFFER_NULL;
    if (cstr_comparestri(&tmp_filename, ".unnamed") == 0) {
	  buffer_setflags(unnamed_buffer, BUF_FLG_VISIBLE);
	  editbuf = unnamed_buffer;
    }
    else if (cstr_comparestri(&tmp_filename, ".keys") == 0) {
	  buffer_setflags(keys_buffer, BUF_FLG_VISIBLE);
	  editbuf = keys_buffer;
    }
    else if (cstr_comparestri(&tmp_filename, ".dir") == 0) {
	  buffer_setflags(dir_buffer, BUF_FLG_VISIBLE);
	  editbuf = dir_buffer;
    }
    else {
	  BUFFER foundbuf = buffers_find_eithername(&tmp_filename);
	  if (foundbuf != BUFFER_NULL) {
		// logmsg("found existing buffer");
		editbuf = foundbuf;
	  }
	  else {
		//logmsg("loading new buffer");
		editbuf = buffer_alloc("", BUF_FLG_VISIBLE, 0);
		err = buffer_load(editbuf, &tmp_filename, file_tab_expand);
		if (err == POE_ERR_FILE_NOT_FOUND) {
		  err = POE_ERR_OK;
		  wins_set_message("New file");
		}
	  }
    }
    
    // Switch to the buffer if we can.
    if (err == POE_ERR_OK && editbuf != BUFFER_NULL) {
	  //logmsg("switching to buffer");
	  wins_cur_switchbuffer(editbuf);
    }
    else {
	  buffer_free(editbuf);
    }
    cstr_destroy(&tmp_filename);
  }
  CMD_RETURN(err);
}


POE_ERR cmd_quit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  WINPTR wnd; VIEWPTR view; BUFFER databuf;
  int row, col;
  xtract_data_context(ctx, &wnd, &view, &databuf, &row, &col);
  
  POE_ERR err = POE_ERR_OK;
  // If it's a visible internal buffer, just hide it.
  if (buffer_tstflags(databuf, BUF_FLG_INTERNAL|BUF_FLG_VISIBLE)) {
    buffer_clrflags(databuf, BUF_FLG_VISIBLE);
    wins_hidebuffer(databuf);
  }
  else if (buffer_tstflags(databuf, BUF_FLG_DIRTY)) {
    enum confirmation_t confirmed = get_confirmation("Do you really want to quit? Type y or n");
    if (confirmed == confirmation_y) {
     wins_hidebuffer(databuf);
     buffer_free(databuf);
    }
  }
  else {
    wins_hidebuffer(databuf);
    buffer_free(databuf);
  }
  CMD_RETURN(err);
}


POE_ERR cmd_save(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  // Get context for data buf
  WINPTR wnd; VIEWPTR view; BUFFER databuf;
  int row, col;
  xtract_data_context(ctx, &wnd, &view, &databuf, &row, &col);
  
  POE_ERR err = POE_ERR_OK;
  if (buffer_tstflags(databuf, BUF_FLG_INTERNAL)) {
    const char* bufname = buffer_name(databuf);
    if (strcasecmp(bufname, ".UNNAMED") == 0)
     err = POE_ERR_CANNOT_NAME_UNNAMED;
    else
     err = POE_ERR_CANNOT_SAVE_INTERNAL;
  }
  else {
    const char* filename = next_parm_str(ctx, (const char*)NULL);
    const char* exp_mode = next_parm_str(ctx, (const char*)NULL);
    bool file_blank_compress = blankcompress;
    if (exp_mode != NULL) {
     if (strcasecmp(exp_mode, "notabs") == 0)
       file_blank_compress = true;
     else if (strcasecmp(exp_mode, "tabs") == 0)
       file_blank_compress = false;
    }
    // Save it appropriately
    if (filename == NULL) {
     err = buffer_save(databuf, NULL, file_blank_compress);  
    }
    else {
     cstr tmp_filename;
     cstr_initstr(&tmp_filename, filename);
     cstr_trimleft(&tmp_filename, poe_iswhitespace);
     cstr_trimright(&tmp_filename, poe_iswhitespace);
     err = buffer_save(databuf, &tmp_filename, file_blank_compress);
     cstr_destroy(&tmp_filename);
    }
  }
  CMD_RETURN(err);
}


POE_ERR cmd_file(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  err = cmd_save(ctx);
  if (err == POE_ERR_OK) {
    cmd_quit(ctx);
  }
  CMD_RETURN(err);
}


POE_ERR cmd_name(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  logmsg("in cmd_name, unimplemented");
  POE_ERR err = POE_ERR_OK;
  CMD_RETURN(err);
}


// options:
// '-' == search backwards
// 'e' == force case sensitivity
POE_ERR _cmd_locate(BUFFER buf,
                    int* prow, int* pcol, int* pendcol,
                    const cstr* patstr, const cstr* optstr)
{
  TRACE_ENTER;
  POE_ERR err = POE_ERR_OK;
  const char* pat = cstr_getbufptr(patstr);
  const char* opts = cstr_getbufptr(optstr);
  int direction = (strchr(opts, '-') != NULL) ? -1 : 1;
  bool bSrchExact = strchr(opts, 'e') != NULL;
  if (direction > 0)
    buffer_right_wrap(buf, prow, pcol);
  else if (direction < 0)
    buffer_left_wrap(buf, prow, pcol);
  bool bFound = false;
  if (!bSrchExact) {
    switch (searchmode) {
    case search_mode_exact:
      bSrchExact = true;
      break;
    case search_mode_any:
      bSrchExact = false;
      break;
    case search_mode_smart:
      // Case sensitive if there are any upper case letters in the
      // search pattern.
      {
        bSrchExact = true;
        bool bup = false;
        const char* s;
        for (s = pat; *s; s++)
          if (isupper(*s)) bup |= true;
        bSrchExact = bup;
      }
      break;
    default:
      bSrchExact = false;
      break;
    }
  }

  *pendcol = *pcol;
  bFound = buffer_search(buf, prow, pcol, pendcol, patstr, bSrchExact, direction);
  if (bFound)
    err = POE_ERR_OK;
  else
    err = POE_ERR_NOT_FOUND;
  TRACE_RETURN(err);
}


// options:
// '-' == search backwards
// 's' == mark found string with a character mark
// 'e' == force case sensitivity
// 'o' == force search from end (top or bottom depending on '-')
POE_ERR cmd_locate(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  POE_ERR err = POE_ERR_OK;
  const char* pat = next_parm_str(ctx, NULL);
  const char* opts = next_parm_str(ctx, "");

  int direction = (strchr(opts, '-') != NULL) ? -1 : 1;
  bool bSelectFound = strchr(opts, 'e') != NULL;
  bool bFromEnd = strchr(opts, 'o') != NULL;

  if (bFromEnd) {
    if (direction > 0) {
      row = col =0;
    }
    else if (direction < 0) {
      row = buffer_count(buf)-1;
      col = buffer_line_length(buf, row);
    }
  }
  int endcol = col;
  if (pat == NULL || strlen(pat) == 0) {
    err = POE_ERR_SRCH_STR;
  }
  else {
    cstr patstr, optstr;
    cstr_initstr(&patstr, pat);
    cstr_initstr(&optstr, opts);
    err = _cmd_locate(buf, &row, &col, &endcol, &patstr, &optstr);
    cstr_destroy(&patstr);
    cstr_destroy(&optstr);
  }

  if (err == POE_ERR_OK) {
    if (bSelectFound) {
      markstack_cur_unmark();
      markstack_cur_place(Marktype_Char, buf, row, col);
      markstack_cur_place(Marktype_Char, buf, row, endcol-1);
    }
    view_move_cursor_to(view, row, col);
    cmd_cursor_data(ctx);
  }
  ctx->save_commandline = true;
  CMD_RETURN(err);
}


// options:
// '-' == search backwards
// 'e' == force case sensitivity
// '*' == replace all occurrences
// 'm' == replace only marked occurrences
// 'n' == replace only unmarked occurrences
POE_ERR _cmd_change(cmd_ctx* ctx,
                    int row, int col,
                    const cstr* patstr, const cstr* replstr,
                    const cstr* optstr)
{
  TRACE_ENTER;
  POE_ERR err = POE_ERR_OK;
  int endcol = col;
  const char* opts = cstr_getbufptr(optstr);
  bool bReplaceAll = strchr(opts, '*') != NULL;
  bool bOnlyMarked = strchr(opts, 'm') != NULL;
  bool bOnlyUnmarked = strchr(opts, 'n') != NULL;
  int nReplacements = 0;
  bool bDone = false;
  enum confirmation_t confirmation;
  do {
    err = _cmd_locate(ctx->targ_buf, &row, &col, &endcol, patstr, optstr);
    confirmation = confirmation_n;
    // Is this one we care about?
    bool bShouldAttempt = err == POE_ERR_OK;
    if (err == POE_ERR_OK && (bOnlyMarked || bOnlyUnmarked)) {
      bool bIsMarked = markstack_hittest_point(ctx->targ_buf, row, col,
                                               MARK_FLG_VISIBLE, MARK_FLG_VISIBLE);
      bShouldAttempt = (bOnlyMarked && bIsMarked) || (bOnlyUnmarked && !bIsMarked);
    }
    // Ask the user 
    if (bShouldAttempt) {
      if (err == POE_ERR_OK) {
        view_move_cursor_to(ctx->targ_view, row, col);
        cmd_cursor_data(ctx);
      }
      confirmation = get_confirmation("Confirm change");
      if (confirmation == confirmation_y) {
        buffer_removechars(ctx->targ_buf, row, col, endcol-col, true);
        buffer_insertstrn(ctx->targ_buf, row, col, cstr_getbufptr(replstr), cstr_count(replstr), true);
        ++nReplacements;
      }
    }
    bDone = bShouldAttempt && !bReplaceAll;
  } while (!bDone && err == POE_ERR_OK && confirmation != confirmation_esc);

  // If we were doing a replace-all, the last search would have
  // returned an error, that isn't reflective of what actual happened.
  if (nReplacements > 0)
    err = POE_ERR_OK;
  TRACE_RETURN(err);
}


// options:
// '-' == search backwards
// 'e' == force case sensitivity
// '*' == replace all occurrences
// 'm' == replace only marked occurrences
// 'n' == replace only unmarked occurrences
// 'o' == force search from end (top or bottom depending on '-')
POE_ERR cmd_change(cmd_ctx* ctx)
{
  CMD_ENTER_BND(ctx, wnd, view, buf, row, col);
  POE_ERR err = POE_ERR_OK;
  const char* pat = next_parm_str(ctx, NULL);
  const char* repl = next_parm_str(ctx, NULL);
  const char* opts = next_parm_str(ctx, "");

  int direction = (strchr(opts, '-') != NULL) ? -1 : 1;
  bool bFromEnd = strchr(opts, 'o') != NULL;
  if (bFromEnd) {
    if (direction > 0) {
      row = col =0;
    }
    else if (direction < 0) {
      row = buffer_count(buf)-1;
      col = buffer_line_length(buf, row);
    }
  }

  if (pat == NULL || strlen(pat) == 0 || repl == NULL) {
    err = POE_ERR_SRCH_STR;
  }
  else {
    cstr patstr, replstr, optstr;
    cstr_initstr(&patstr, pat);
    cstr_initstr(&replstr, repl);
    cstr_initstr(&optstr, opts);
    err = _cmd_change(ctx, row, col, &patstr, &replstr, &optstr);
    cstr_destroy(&patstr);
    cstr_destroy(&replstr);
    cstr_destroy(&optstr);
  }
  ctx->save_commandline = true;
  CMD_RETURN(err);
}


POE_ERR cmd_trim_leading(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  buffer_trimleft(ctx->targ_buf, ctx->targ_row, true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_trim_trailing(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  buffer_trimright(ctx->targ_buf, ctx->targ_row, true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_trim(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  buffer_trimright(ctx->targ_buf, ctx->targ_row, true);
  buffer_trimleft(ctx->targ_buf, ctx->targ_row, true);
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_set_margins(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int leftmargin, rightmargin, paramargin;
  buffer_getmargins(ctx->data_buf, &leftmargin, &rightmargin, &paramargin);
  leftmargin = next_parm_int(ctx, leftmargin);
  rightmargin = next_parm_int(ctx, rightmargin);
  paramargin = next_parm_int(ctx, leftmargin);
  POE_ERR err = margins_set(&default_margins, leftmargin-1, rightmargin-1, paramargin-1);
  if (err == POE_ERR_OK)
    err = buffer_setmargins(ctx->data_buf, leftmargin-1, rightmargin-1, paramargin-1);
  CMD_RETURN(err);
}


POE_ERR cmd_qry_margins(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int leftmargin, rightmargin, paramargin;
  buffer_getmargins(ctx->data_buf, &leftmargin, &rightmargin, &paramargin);
  _printf_cmdline(ctx, "set margins %d %d %d", leftmargin+1, rightmargin+1, paramargin+1);
  ctx->save_commandline = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_set_tabs(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  int t1 = next_parm_int(ctx, -1);
  int t2 = next_parm_int(ctx, -1);
  int t3 = next_parm_int(ctx, -1);
  if (t1 >= 1 && t2 >= 1 && t3 >= 0) {
    struct pivec_t tabstops;
    pivec_init(&tabstops, 20);
    pivec_append(&tabstops, t1-1);
    pivec_append(&tabstops, t2-1);
    pivec_append(&tabstops, t3-1);
    for (t3 = next_parm_int(ctx, -1); t3 >= 1; t3 = next_parm_int(ctx, -1)) {
      pivec_append(&tabstops, t3-1);
      t2 = t3;
    }
    t2--;
    t3--;
    int tab_step = t3-t2;
    tabs_set(&default_tabstops, t3%tab_step, tab_step, &tabstops);
    buffer_settabs(ctx->data_buf, &default_tabstops);
    pivec_destroy(&tabstops);
  }
  else if (t1 >= 1 && t2 >= 1) {
    tabs_set(&default_tabstops, t1-1, t2, NULL);
    buffer_settabs(ctx->data_buf, &default_tabstops);
  }
  else if (t1 >= 1) {
    tabs_set(&default_tabstops, 0, t1, NULL);
    buffer_settabs(ctx->data_buf, &default_tabstops);
  }
  else {
    err = POE_ERR_TABS;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_qry_tabs(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  tabstops tabs;
  tabs_init(&tabs, 0, 0, NULL);
  buffer_gettabs(ctx->data_buf, &tabs);
  int i, n = pivec_count(&tabs.vtabs);
  if (n == 0) {
    _printf_cmdline(ctx, "set tabs %d %d", tabs.start+1, tabs.start+tabs.step);
  }
  else {
    cstr s;
    cstr_init(&s, 0);
    for (i = 0; i < n; i++) {
      char tmp[64];
      snprintf(tmp, sizeof(tmp), "%d ", (int)pivec_get(&tabs.vtabs, i)+1);
      cstr_appendstr(&s, tmp);
    }
    _printf_cmdline(ctx, "set tabs %s", cstr_getbufptr(&s));
    cstr_destroy(&s);
  }
  ctx->save_commandline = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_qry_key(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  const char* keyname = next_parm_str(ctx, NULL);
  if (keyname != NULL) {
    cstr fmtted_def;
    cstr_init(&fmtted_def, 1024);
    err = get_key_def(ctx->data_buf, &fmtted_def, keyname);
    if (err == POE_ERR_OK) {
      _printf_cmdline(ctx, "%s", cstr_getbufptr(&fmtted_def));
      ctx->save_commandline = true;
    }
  }
  CMD_RETURN(err);
}


POE_ERR cmd_set_blankcompress(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  const char* flag = next_parm_str(ctx, NULL);
  if (flag == NULL) {
    err = POE_ERR_SET_VAL_UNK;
  }
  else if (strcasecmp(flag, "ON") == 0) {
    blankcompress = true;
  }
  else if (strcasecmp(flag, "OFF") == 0) {
    blankcompress = false;
  }
  else {
    err = POE_ERR_SET_VAL_UNK;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_qry_blankcompress(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  _printf_cmdline(ctx, "set blankcompress %s", blankcompress?"on":"off");
  ctx->save_commandline = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_set_tabexpand(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  const char* flag = next_parm_str(ctx, NULL);
  if (flag == NULL) {
    err = POE_ERR_SET_VAL_UNK;
  }
  else if (strcasecmp(flag, "ON") == 0) {
    tabexpand = true;
  }
  else if (strcasecmp(flag, "OFF") == 0) {
    tabexpand = false;
  }
  else {
    err = POE_ERR_SET_VAL_UNK;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_qry_tabexpand(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  _printf_cmdline(ctx, "set tabexpand %s", tabexpand?"on":"off");
  ctx->save_commandline = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_set_tabexpand_size(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  int tabsize = next_parm_int(ctx, 0);
  if (tabsize <= 0 || tabsize >= 80) {
    err = POE_ERR_SET_VAL_UNK;
  }
  else {
	tabexpand_size = tabsize;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_qry_tabexpand_size(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  _printf_cmdline(ctx, "set tabexpand size %s", tabexpand_size);
  ctx->save_commandline = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_set_searchcase(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  const char* flag = next_parm_str(ctx, NULL);
  if (flag == NULL) {
    err = POE_ERR_SET_VAL_UNK;
  }
  else if (strcasecmp(flag, "EXACT") == 0) {
    searchmode = search_mode_exact;
  }
  else if (strcasecmp(flag, "ANY") == 0) {
    searchmode = search_mode_any;
  }
  else if (strcasecmp(flag, "SMART") == 0) {
    searchmode = search_mode_smart;
  }
  else {
    err = POE_ERR_SET_VAL_UNK;
  }
  CMD_RETURN(err);
}


POE_ERR cmd_qry_searchcase(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  switch (searchmode) {
  case search_mode_exact:
    _printf_cmdline(ctx, "set searchcase exact");
    break;
  case search_mode_any:
    _printf_cmdline(ctx, "set searchcase any");
    break;
  case search_mode_smart:
    _printf_cmdline(ctx, "set searchcase smart");
    break;
  }
  ctx->save_commandline = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_set_vsplit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int split = next_parm_int(ctx, -1);
  if (split <= 0 || split >= SPLIT_MAX_VAL)
    CMD_RETURN(POE_ERR_SET_OPTION_UNK);
  vsplitter = (split*SPLIT_MAX_VAL)/100;
  wins_resize();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_qry_vsplit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  _printf_cmdline(ctx, "set vsplit %d", (int)SCALE_SPLITTER(100,vsplitter));
  ctx->save_commandline = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_set_hsplit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int split = next_parm_int(ctx, -1);
  if (split <= 0 || split >= SPLIT_MAX_VAL)
    CMD_RETURN(POE_ERR_SET_OPTION_UNK);
  hsplitter = (split*SPLIT_MAX_VAL)/100;
  wins_resize();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_qry_hsplit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  _printf_cmdline(ctx, "set hsplit %d", (int)SCALE_SPLITTER(100,hsplitter));
  ctx->save_commandline = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_incr_vsplit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int vsplit = next_parm_int(ctx, 0);
  vsplit = (vsplit*SPLIT_MAX_VAL)/100;
  int new_vsplitter = vsplitter + vsplit;
  if (new_vsplitter <= 0 || new_vsplitter >= SPLIT_MAX_VAL)
    CMD_RETURN(POE_ERR_SET_OPTION_UNK);
  vsplitter = new_vsplitter;
  wins_resize();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_decr_vsplit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int vsplit = next_parm_int(ctx, 0);
  vsplit = (vsplit*SPLIT_MAX_VAL)/100;
  int new_vsplitter = vsplitter - vsplit;
  if (new_vsplitter <= 0 || new_vsplitter >= SPLIT_MAX_VAL)
    CMD_RETURN(POE_ERR_SET_OPTION_UNK);
  vsplitter = new_vsplitter;
  wins_resize();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_incr_hsplit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int hsplit = next_parm_int(ctx, 0);
  hsplit = (hsplit*SPLIT_MAX_VAL)/100;
  int new_hsplitter = hsplitter + hsplit;
  if (new_hsplitter <= 0 || new_hsplitter >= SPLIT_MAX_VAL)
    CMD_RETURN(POE_ERR_SET_OPTION_UNK);
  hsplitter = new_hsplitter;
  wins_resize();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_decr_hsplit(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  int hsplit = next_parm_int(ctx, 0);
  hsplit = (hsplit*SPLIT_MAX_VAL)/100;
  int new_hsplitter = hsplitter - hsplit;
  if (new_hsplitter <= 0 || new_hsplitter >= SPLIT_MAX_VAL)
    CMD_RETURN(POE_ERR_SET_OPTION_UNK);
  hsplitter = new_hsplitter;
  wins_resize();
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_qry_wrap(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  _printf_cmdline(ctx, "set wrap %s", autowrap?"ON":"OFF");
  ctx->save_commandline = true;
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_set_wrap(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  const char* parm = next_parm_str(ctx, NULL);
  if (parm == NULL)
	err = POE_ERR_SET_VAL_UNK;
  else if (strcasecmp(parm, "ON") == 0)
	autowrap = true;
  else if (strcasecmp(parm, "OFF") == 0)
	autowrap = false;
  else
	err = POE_ERR_SET_VAL_UNK;
  CMD_RETURN(err);
}



POE_ERR cmd_center_in_margins(cmd_ctx* ctx)
{
  CMD_ENTER_DATAONLY_BND(ctx, wnd, view, buf, row, col);
  enum marktype typ;
  int l1, c1, l2, c2;
  POE_ERR err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    CMD_RETURN(err); 
  if (typ != Marktype_Line)
    CMD_RETURN(POE_ERR_LINE_MARK_REQ);
  BUFFER markbuf = BUFFER_NULL;
  err = markstack_cur_get_buffer(&markbuf);
  if (err != POE_ERR_OK)
    CMD_RETURN(err);
  // Switch buffers if necessary
  if (buf != markbuf) {
    wins_cur_switchbuffer(markbuf);
    update_context(ctx);
    xtract_targ_context(ctx, &wnd, &view, &buf, &row, &col);
  }
  
  switch (typ) {
  case Marktype_Line: 
    {
      int leftmargin, rightmargin, paragraphmargin;
      buffer_getmargins(buf, &leftmargin, &rightmargin, &paragraphmargin);
      _savelines_other(buf, l1, l2-l1+1);
      int i;
      for (i = l1; i <= l2; i++) {
        buffer_trimright(buf, i, true);
        buffer_trimleft(buf, i, true);
        if (buffer_isblankline(buf, i))
          continue;
        int linelen=buffer_line_length(buf, i);
        int marginwidth = rightmargin - leftmargin;
        if (linelen >= marginwidth) {
          // line is longer than margins - just align at left margin
          buffer_insertct(buf, i, 0, ' ', leftmargin, true);
        }
        else {
          // line is shorter than margins, so it can be centered
          int xtraindent = (marginwidth-linelen)/2;
          buffer_insertct(buf, i, 0, ' ', leftmargin + xtraindent, true);
        }
      }
    }
    break;
  case Marktype_Char: case Marktype_Block:
    err = POE_ERR_LINE_MARK_REQ;
    break;
  case Marktype_None: default:
    err = POE_ERR_NO_MARKED_AREA;
    break;
  }
  CMD_RETURN(POE_ERR_OK);
}


POE_ERR cmd_define(cmd_ctx* ctx)
{
  CMD_ENTER(ctx);
  POE_ERR err = POE_ERR_OK;
  const char* keyname = next_parm_str(ctx, NULL);
  if (keyname == NULL)
    CMD_RETURN(POE_ERR_INVALID_KEY);

  PROFILEPTR profile = buffer_get_profile(ctx->data_buf);
  int i, nkeycmds = pivec_count(ctx->cmdseq) - ctx->pc;
  intptr_t* keycmds = calloc(nkeycmds, sizeof(intptr_t));
  for (i = 0; i < nkeycmds; i++) {
    keycmds[i] = pivec_get(ctx->cmdseq, ctx->pc+i);
  }
  defkey(profile, keyname, keycmds, nkeycmds);
   free(keycmds);
  ctx->pc += i-1;
  CMD_RETURN(err);
}




////////////////////////////////////////
// Command table
////////////////////////////////////////

#define DEFCMD(func,...) {                          \
    static const char* _cmdspec_[] = {__VA_ARGS__, NULL};  \
    defcmd(_cmdspec_, func);                            \
  }

void defcmds(void)
{
  DEFCMD(cmd_qry_blankcompress,        "?", "BLANKCOMPRESS");
  DEFCMD(cmd_qry_hsplit,               "?", "HSPLIT");
  DEFCMD(cmd_qry_key,                  "?", "KEY");
  DEFCMD(cmd_qry_margins,              "?", "MARGINS");
  DEFCMD(cmd_qry_searchcase,           "?", "SEARCHCASE");
  DEFCMD(cmd_qry_tabexpand_size,       "?", "TABEXPAND", "SIZE");
  DEFCMD(cmd_qry_tabexpand,            "?", "TABEXPAND");
  DEFCMD(cmd_qry_tabs,                 "?", "TABS");
  DEFCMD(cmd_qry_vsplit,               "?", "VSPLIT");
  DEFCMD(cmd_qry_wrap,                 "?", "WRAP");

  DEFCMD(cmd_backtab_word,             "BACKTAB",     "WORD");
  DEFCMD(cmd_backtab,                  "BACKTAB");    
  DEFCMD(cmd_begin_line,               "BEGIN",       "LINE");
  DEFCMD(cmd_begin_mark,               "BEGIN",       "MARK");
  DEFCMD(cmd_begin_word,               "BEGIN",       "WORD");
  DEFCMD(cmd_bottom_edge,              "BOTTOM",      "EDGE");
  DEFCMD(cmd_bottom,                   "BOTTOM");     
                                                      
  DEFCMD(cmd_center_in_margins,        "CENTER",      "IN",      "MARGINS");
  DEFCMD(cmd_center_line,              "CENTER",      "LINE");
  DEFCMD(cmd_change,                   "CHANGE");
  DEFCMD(cmd_chr,                      "CHR");        
  DEFCMD(cmd_clear_marks,              "CLEAR",       "MARKS");
  DEFCMD(cmd_column,                   "COLUMN");
  DEFCMD(cmd_command_toggle,           "COMMAND",     "TOGGLE");
  DEFCMD(cmd_confirm_change,           "CONFIRM",     "CHANGE");
  DEFCMD(cmd_copy_mark,                "COPY",        "MARK");
  DEFCMD(cmd_cursor_command,           "CURSOR",      "COMMAND");
  DEFCMD(cmd_cursor_data,              "CURSOR",      "DATA");
                                                      
  DEFCMD(cmd_decr_hsplit,              "DECR",        "HSPLIT");
  DEFCMD(cmd_decr_vsplit,              "DECR",        "VSPLIT");
  DEFCMD(cmd_define,                   "DEFINE");
  DEFCMD(cmd_delete_char,              "DELETE",      "CHAR");
  DEFCMD(cmd_delete_line,              "DELETE",      "LINE");
  DEFCMD(cmd_delete_mark,              "DELETE",      "MARK");
  DEFCMD(cmd_down,                     "DOWN");       
                                                      
  DEFCMD(cmd_edit,                     "E");
  DEFCMD(cmd_edit,                     "EDIT");
  DEFCMD(cmd_end_line,                 "END",         "LINE");
  DEFCMD(cmd_end_mark,                 "END",         "MARK");
  DEFCMD(cmd_end_word,                 "END",         "WORD");
  DEFCMD(cmd_erase_begin_line,         "ERASE",       "BEGIN",   "LINE");
  DEFCMD(cmd_erase_command_line,       "ERASE",       "COMMAND", "LINE");
  DEFCMD(cmd_erase_end_line,           "ERASE",       "END",     "LINE");
  DEFCMD(cmd_escape,                   "ESCAPE");
  DEFCMD(cmd_execute,                  "EXECUTE");
  //DEFCMD(cmd_exit,                     "EXIT");       
                                                      
  DEFCMD(cmd_file,                     "FILE");
  DEFCMD(cmd_fill_mark,                "FILL",        "MARK");
  DEFCMD(cmd_find_blank_line,          "FIND",        "BLANK",   "LINE");
  DEFCMD(cmd_find_prev_blank_line,     "FIND",        "PREV",    "BLANK", "LINE");
  DEFCMD(cmd_first_nonblank,           "FIRST",       "NONBLANK");
                                                      
  DEFCMD(cmd_incr_hsplit,              "INCR",        "HSPLIT");
  DEFCMD(cmd_incr_vsplit,              "INCR",        "VSPLIT");
  DEFCMD(cmd_indent,                   "INDENT");     
  DEFCMD(cmd_insert_line,              "INSERT",      "LINE");
  DEFCMD(cmd_insert_mode,              "INSERT",      "MODE");
  DEFCMD(cmd_insert_toggle,            "INSERT",      "TOGGLE");

  DEFCMD(cmd_join,                     "JOIN");
                                                      
  DEFCMD(cmd_left_edge,                "LEFT",        "EDGE");
  DEFCMD(cmd_left_wrap,                "LEFT",        "WRAP");
  DEFCMD(cmd_left,                     "LEFT");       
  DEFCMD(cmd_line,                     "LINE");
  DEFCMD(cmd_locate,                   "LOCATE");
  DEFCMD(cmd_lowercase,                "LOWERCASE");
                                                      
  DEFCMD(cmd_mark_block,               "MARK",        "BLOCK");
  DEFCMD(cmd_mark_char,                "MARK",        "CHAR");
  DEFCMD(cmd_mark_line,                "MARK",        "LINE");
  DEFCMD(cmd_move_mark,                "MOVE",        "MARK");
  DEFCMD(cmd_move_view_down,           "MOVE",        "VIEW",    "DOWN");
  DEFCMD(cmd_move_view_left,           "MOVE",        "VIEW",    "LEFT");
  DEFCMD(cmd_move_view_right,          "MOVE",        "VIEW",    "RIGHT");
  DEFCMD(cmd_move_view_up,             "MOVE",        "VIEW",    "UP");

  DEFCMD(cmd_name,                     "NAME");
  DEFCMD(cmd_next_file,                "NEXT",        "FILE");
  DEFCMD(cmd_next_view,                "NEXT",        "VIEW");
  DEFCMD(cmd_next_window,              "NEXT",        "WINDOW");

  DEFCMD(cmd_overlay_block,            "OVERLAY",     "BLOCK");
                                                      
  DEFCMD(cmd_page_down,                "PAGE",        "DOWN");
  DEFCMD(cmd_page_up,                  "PAGE",        "UP");
  DEFCMD(cmd_pop_mark,                 "POP",         "MARK");
  DEFCMD(cmd_push_mark,                "PUSH",        "MARK");
                                                      
  DEFCMD(cmd_exit,                     "QQUIT");      
  DEFCMD(cmd_quit,                     "QUIT");
                                                      
  DEFCMD(cmd_replace_mode,             "REPLACE",     "MODE");
  DEFCMD(cmd_resize_display,           "REDRAW");
  DEFCMD(cmd_reflow,                   "REFLOW");
  DEFCMD(cmd_resize_display,           "RESIZE",      "DISPLAY");
  DEFCMD(cmd_right_edge,               "RIGHT",       "EDGE");
  DEFCMD(cmd_right_wrap,               "RIGHT",       "WRAP");
  DEFCMD(cmd_right,                    "RIGHT");      
  DEFCMD(cmd_rubout_join,              "RUBOUT",      "JOIN");
  DEFCMD(cmd_rubout,                   "RUBOUT");     
                                                      
  DEFCMD(cmd_save,                     "S");
  DEFCMD(cmd_save,                     "SAVE");
  DEFCMD(cmd_set_blankcompress,        "SET",         "BLANKCOMPRESS");
  DEFCMD(cmd_set_hsplit,               "SET",         "HSPLIT");
  DEFCMD(cmd_set_margins,              "SET",         "MARGINS");
  DEFCMD(cmd_set_searchcase,           "SET",         "SEARCHCASE");
  DEFCMD(cmd_set_tabexpand_size,       "SET",         "TABEXPAND", "SIZE");
  DEFCMD(cmd_set_tabexpand,            "SET",         "TABEXPAND");
  DEFCMD(cmd_set_tabs,                 "SET",         "TABS");
  DEFCMD(cmd_set_vsplit,               "SET",         "VSPLIT");
  DEFCMD(cmd_set_wrap,                 "SET",         "WRAP");
  DEFCMD(cmd_shift_left,               "SHIFT",       "LEFT");
  DEFCMD(cmd_shift_right,              "SHIFT",       "RIGHT");
  DEFCMD(cmd_str,                      "STR");        
  DEFCMD(cmd_split_screen,             "SPLIT",       "SCREEN");
  DEFCMD(cmd_split,                    "SPLIT");
                                                      
  DEFCMD(cmd_tab_word,                 "TAB",         "WORD");
  DEFCMD(cmd_tab,                      "TAB");        
  DEFCMD(cmd_trim_leading,             "TRIM",        "LEADING");
  DEFCMD(cmd_trim_trailing,            "TRIM",        "TRAILING");
  DEFCMD(cmd_trim,                     "TRIM");
  DEFCMD(cmd_top_edge,                 "TOP",         "EDGE");
  DEFCMD(cmd_top,                      "TOP");
                                                      
  DEFCMD(cmd_unmark,                   "UNMARK");
  DEFCMD(cmd_up,                       "UP");
  DEFCMD(cmd_uppercase,                "UPPERCASE");

  DEFCMD(cmd_zoom_window,              "ZOOM",        "WINDOW");
}

