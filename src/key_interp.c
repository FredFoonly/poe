
//#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <ncurses.h>

#include "trace.h"
#include "logging.h"
#include "utils.h"
#include "poe_err.h"
#include "poe_exit.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "mark.h"
#include "tabstops.h"
#include "margins.h"
#include "key_interp.h"
#include "buffer.h"
#include "view.h"
#include "window.h"
#include "commands.h"
#include "cmd_interp.h"
#include "default_profile.h"
#include "editor_globals.h"


void log_view_info();

struct keydef_t {
  const char* keyname;
  pivec* cmds;
};


struct profile_t {
  cstr name;
  struct profile_t* parent;
  struct vec_t/*keydef_t*/ key_defs;
};


void defkey(PROFILEPTR prof, const char* keyname, const intptr_t* cmds, size_t ncmds);
struct keydef_t* find_keydef(PROFILEPTR prof, const char* keyname);
pivec* _save_cmds(const intptr_t* cmds, size_t n);


PROFILEPTR alloc_profile(const char* name, PROFILEPTR parent)
{
  TRACE_ENTER;
  PROFILEPTR prof = (PROFILEPTR)malloc(sizeof(struct profile_t));
  cstr_initstr(&prof->name, name);
  prof->parent = parent;
  vec_init(&prof->key_defs, 10, sizeof(struct keydef_t));
  TRACE_RETURN(prof);
}


void free_profile(PROFILEPTR prof)
{
  TRACE_ENTER;
  if (prof == NULL) TRACE_EXIT;
  cstr_destroy(&prof->name);
  prof->parent = NULL;
  int i, n = vec_count(&prof->key_defs);
  for (i = 0; i < n; i++) {
    struct keydef_t* keydef = vec_get(&prof->key_defs, i);
    free((char*)keydef->keyname);
    keydef->keyname = NULL;
    // Fix!  This leaks the memory for any strings in the cmds vec...  
    pivec_free(keydef->cmds);
    keydef->cmds = NULL;
  }
  vec_destroy(&prof->key_defs);
  free(prof);
  TRACE_EXIT;
}


void init_key_interp(void)
{
  TRACE_ENTER;
  dflt_data_profile = alloc_profile("DEFAULT", NULL);
  dflt_cmd_profile = alloc_profile("DEFAULT_DATA", dflt_data_profile);
  TRACE_EXIT;
}


void close_key_interp(void)
{
  TRACE_ENTER;
  free_profile(dflt_data_profile);
  dflt_data_profile = NULL;
  free_profile(dflt_cmd_profile);
  dflt_cmd_profile = NULL;
  TRACE_EXIT;
}


POE_ERR translate_insertable_key(const char* keyname, char* pchr)
{
  TRACE_ENTER;
  char* ucKeyname = strupr(strsave(keyname));
  WINPTR cur_wnd = wins_get_cur();
  PROFILEPTR prof = win_get_profile(cur_wnd);
  struct keydef_t* keydef = find_keydef(prof, ucKeyname);
  if (keydef == NULL) {
    logmsg("can't find definition for key %s", ucKeyname);
    free(ucKeyname);
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  }
  pivec* cmds = keydef->cmds;
  if (END_OF_CMD(cmds, 0))
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  if (!CMD_IS_STR(pivec_get(cmds, 0)))
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  if (strcasecmp(CMD_STRVAL(pivec_get(cmds, 0)), "CHR") != 0)
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  if (END_OF_CMD(cmds, 1))
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  if (!CMD_IS_INT(pivec_get(cmds, 1)))
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  *pchr = (char)(CMD_INTVAL(pivec_get(cmds, 1)));
  TRACE_RETURN(POE_ERR_OK);
}


bool is_confirm_key(const char* keyname)
{
  TRACE_ENTER;
  char* ucKeyname = strupr(strsave(keyname));
  WINPTR cur_wnd = wins_get_cur();
  PROFILEPTR prof = win_get_profile(cur_wnd);
  struct keydef_t* keydef = find_keydef(prof, ucKeyname);
  if (keydef == NULL) {
    logmsg("can't find definition for key %s", ucKeyname);
    free(ucKeyname);
    TRACE_RETURN(false);
  }
  //logmsg("found definition for key %s", ucKeyname);
  //cstr cmdseq_descr = format_command_seq(keydef->cmds);
  //logmsg("executing commands: %s", cstr_getbufptr(&cmdseq_descr));
  //cstr_destroy(&cmdseq_descr);
  pivec* cmds = keydef->cmds;
  if (END_OF_CMD(cmds, 0))
    TRACE_RETURN(false);
  if (!CMD_IS_STR(pivec_get(cmds, 0)))
    TRACE_RETURN(false);
  if (strcasecmp(CMD_STRVAL(pivec_get(cmds, 0)), "CONFIRM") != 0)
    TRACE_RETURN(false);
  if (END_OF_CMD(cmds, 1))
    TRACE_RETURN(false);
  if (!CMD_IS_STR(pivec_get(cmds, 1)))
    TRACE_RETURN(false);
  if (strcasecmp(CMD_STRVAL(pivec_get(cmds, 1)), "CHANGE") != 0)
    TRACE_RETURN(false);
  TRACE_RETURN(true);
}


POE_ERR get_key_def(BUFFER buf, cstr* fmtted_def, const char* keyname)
{
  TRACE_ENTER;
  POE_ERR err = POE_ERR_OK;
  //WINPTR cur_wnd = wins_get_cur();
  //PROFILEPTR prof = win_get_profile(cur_wnd);
  PROFILEPTR prof = buffer_get_profile(buf);
  char ucKeyname[MAX_KEYNAME_LEN];
  strlcpy(ucKeyname, keyname, sizeof(ucKeyname));
  strupr(ucKeyname);
  struct keydef_t* keydef = find_keydef(prof, ucKeyname);
  if (keydef == NULL) {
	err = POE_ERR_INVALID_KEY;
  }
  else {
	cstr fmt_seq = format_command_seq(keydef->cmds, 0);
	cstr_clear(fmtted_def);
	cstr_appendstr(fmtted_def, "def ");
	cstr_appendstr(fmtted_def, keyname);
	cstr_appendstr(fmtted_def, " = ");
	cstr_appendstr(fmtted_def, cstr_getbufptr(&fmt_seq));
	cstr_destroy(&fmt_seq);
	err = POE_ERR_OK;
  }
  TRACE_RETURN(err);
}


POE_ERR wins_handle_key(const char* keyname)
{
  TRACE_ENTER;
  WINPTR cur_wnd = wins_get_cur();
  PROFILEPTR prof = win_get_profile(cur_wnd);
  char ucKeyname[MAX_KEYNAME_LEN];
  strlcpy(ucKeyname, keyname, sizeof(ucKeyname));
  strupr(ucKeyname);
  struct keydef_t* keydef = find_keydef(prof, ucKeyname);
  POE_ERR err = POE_ERR_OK;
  if (keydef == NULL) {
    //logmsg("can't find definition for key %s", ucKeyname);
    err = POE_ERR_INVALID_KEY;
  }
  else {
    //logmsg("found definition for key %s", ucKeyname);
    //cstr cmdseq_descr = format_command_seq(keydef->cmds);
    //logmsg("executing commands: %s", cstr_getbufptr(&cmdseq_descr));
    //cstr_destroy(&cmdseq_descr);
    cmd_error = POE_ERR_OK;
    cmd_ctx kbd_ctx;
    kbd_ctx.src_is_commandline = false;
    kbd_ctx.save_commandline = false;
    update_context(&kbd_ctx);
    kbd_ctx.cmdseq = keydef->cmds;
    kbd_ctx.pc = 0;
    err = interpret_command_seq(&kbd_ctx);
    update_context(&kbd_ctx);
	view_move_cursor_to(kbd_ctx.data_view, kbd_ctx.data_row, kbd_ctx.data_col);
	view_move_cursor_to(kbd_ctx.cmd_view, kbd_ctx.cmd_row, kbd_ctx.cmd_col);
  }
  TRACE_RETURN(err);
}


struct keydef_t* _find_keydef(PROFILEPTR prof, const char* keyname)
{
  TRACE_ENTER;
  if (prof != NULL) {
    int i, n = vec_count(&prof->key_defs);
    for (i = 0; i < n; i++) {
      struct keydef_t* pkeydef = vec_get(&prof->key_defs, i);
      if (strcmp(pkeydef->keyname, keyname) == 0)
        TRACE_RETURN(pkeydef);
    }
  }
  TRACE_RETURN(NULL);
}


struct keydef_t* find_keydef(PROFILEPTR prof, const char* keyname)
{
  TRACE_ENTER;
  struct keydef_t* keydef = NULL;
  for (; keydef == NULL && prof != NULL; prof = prof->parent) {
    keydef = _find_keydef(prof, keyname);
  }
  TRACE_RETURN(keydef);
}



void defkey(PROFILEPTR prof, const char* keyname, const intptr_t* cmds, size_t ncmds)
{
  TRACE_ENTER;
  char* ucKeyname = strupr(strsave(keyname));
  struct keydef_t* keydef = _find_keydef(prof, ucKeyname);
  if (keydef != NULL) {
    //logmsg("redefining key %s with %d cmds", keyname, ncmds);
    // FIX!  This leaks memory from any strings in keydef->cmds
    pivec_free(keydef->cmds);
    keydef->cmds = _save_cmds(cmds, ncmds);
    free(ucKeyname);
  }
  else {
    struct keydef_t newkeydef;
    newkeydef.keyname = ucKeyname;
    newkeydef.cmds = _save_cmds(cmds, ncmds);
    vec_append(&prof->key_defs, &newkeydef);
  }
  TRACE_EXIT;
}


pivec* _save_cmds(const intptr_t* cmds, size_t n)
{
  TRACE_ENTER;
  pivec* saved = pivec_alloc(n);
  int i;
  for (i = 0; i < n; i++)
    pivec_append(saved, cmds[i]);
  TRACE_RETURN(saved);
}



