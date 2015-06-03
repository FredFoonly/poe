
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

void defkey(PROFILEPTR prof, const char* keyname, const intptr_t* cmds, size_t ncmds);
struct keydef_t* find_keydef(PROFILEPTR prof, const char* keyname);
pivec* _save_cmds(const intptr_t* cmds, size_t n);


PROFILEPTR alloc_profile(const char* name/* , PROFILEPTR parent */)
{
  TRACE_ENTER;
  PROFILEPTR prof = (PROFILEPTR)malloc(sizeof(struct profile_t));
  cstr_initstr(&prof->name, name);
  prof->keydefs_sorted = false;
  vec_init(&prof->key_defs, 10, sizeof(struct keydef_t));
  tabs_init(&prof->default_tabstops, 0, 8, NULL);
  margins_init(&prof->default_margins, 0, 255, 0);
  prof->tabexpand_size = 8;
  prof->tabexpand = true;
  prof->blankcompress = false;
  prof->autowrap = true;
  prof->oncommand = true;
  prof->searchmode = search_mode_smart;
  TRACE_RETURN(prof);
}


void free_profile(PROFILEPTR prof)
{
  TRACE_ENTER;
  if (prof == NULL) TRACE_EXIT;
  cstr_destroy(&prof->name);
  tabs_destroy(&prof->default_tabstops);
  margins_destroy(&prof->default_margins);
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
  default_profile = alloc_profile("base.pro"/*, NULL*/);
  TRACE_EXIT;
}


void close_key_interp(void)
{
  TRACE_ENTER;
  free_profile(default_profile);
  default_profile = NULL;
  TRACE_EXIT;
}


void load_current_key_definitions(BUFFER keys_buffer, PROFILEPTR profile)
{
  TRACE_ENTER;
  // buffer_removelines(keys_buffer, 0, buffer_count(keys_buffer), false);
  buffer_clear(keys_buffer, true, true);
  struct vec_t* pdefs = &profile->key_defs;
  int i, n = vec_count(pdefs);
  logmsg("adding %d definitions to .keydefs", n);
  for (i = 0; i < n; i++) {
    struct keydef_t* pkeydef = vec_get(pdefs, i);
    cstr fmt_seq = format_command_seq(pkeydef->cmds, 0);
    struct line_t line;
    line.flags = LINE_FLG_LF|LINE_FLG_DIRTY;
    cstr_init(&line.txt, 1);
    cstr_appendstr(&line.txt, "def ");
    cstr_appendstr(&line.txt, pkeydef->keyname);
    cstr_appendstr(&line.txt, " = ");
    cstr_appendstr(&line.txt, cstr_getbufptr(&fmt_seq));
    cstr_destroy(&fmt_seq);
    buffer_appendline(keys_buffer, &line);
  }
  TRACE_EXIT;
}


int _find_subcmd(const char* cmd, pivec* cmds, int pc)
{
  TRACE_ENTER;
  int rval = -1;
  int ncmds = pivec_count(cmds);
  while (pc < ncmds-1 && !END_OF_CMDSEQ(cmds, pc)) {
    if (CMD_IS_STR(pivec_get(cmds, pc)) && 
        (strcasecmp(CMD_STRVAL(pivec_get(cmds, pc)), cmd) == 0)) {
      rval = pc;
      break;
    }
    // skip to next command in list
    while (pc < ncmds && !END_OF_CMD(cmds, pc))
      pc++;
    pc++;
  }
  TRACE_RETURN(rval);
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
  int chr_pc = _find_subcmd("CHAR", cmds, 0);
  if (chr_pc < 0)
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  if (END_OF_CMD(cmds, chr_pc+1))
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  if (!CMD_IS_INT(pivec_get(cmds, chr_pc+1)))
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  *pchr = (char)(CMD_INTVAL(pivec_get(cmds, chr_pc+1)));
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
  int conf_pc = _find_subcmd("CONFIRM", cmds, 0);
  if (conf_pc < 0)
    TRACE_RETURN(false);
  if (END_OF_CMD(cmds, conf_pc+1))
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  if (!CMD_IS_INT(pivec_get(cmds, conf_pc+1)))
    TRACE_RETURN(POE_ERR_INVALID_KEY);
  if (END_OF_CMD(cmds, conf_pc+1))
    TRACE_RETURN(false);
  if (!CMD_IS_STR(pivec_get(cmds, conf_pc+1)))
    TRACE_RETURN(false);
  if (strcasecmp(CMD_STRVAL(pivec_get(cmds, conf_pc+1)), "CHANGE") != 0)
    TRACE_RETURN(false);
  TRACE_RETURN(true);
}


POE_ERR get_key_def(BUFFER buf, cstr* fmtted_def, const char* keyname)
{
  TRACE_ENTER;
  POE_ERR err = POE_ERR_OK;

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
  cmd_ctx ctx;
  ctx.src_is_commandline = false;
  update_context(&ctx);

  char ucKeyname[MAX_KEYNAME_LEN];
  struct keydef_t* keydef = NULL;
  if (buffer_tstflags(ctx.targ_buf, BUF_FLG_CMDLINE)) {
    // first check cmd-ins/repl-keyname
    memset(ucKeyname, 0, sizeof(ucKeyname));
    if (buffer_tstflags(ctx.targ_buf, BUF_FLG_CMDLINE))
      strlcat(ucKeyname, "CMD-", sizeof(ucKeyname));
    if (view_tstflags(ctx.targ_view, VIEW_FLG_INSERTMODE))
      strlcat(ucKeyname, "INS-", sizeof(ucKeyname));
    else
      strlcat(ucKeyname, "REP-", sizeof(ucKeyname));
    strlcat(ucKeyname, keyname, sizeof(ucKeyname));
    strupr(ucKeyname);
    //logmsg("trying to look up key '%s'", ucKeyname);
    keydef = find_keydef(prof, ucKeyname);

    if (keydef == NULL) {
      // then check cmd-keyname
      memset(ucKeyname, 0, sizeof(ucKeyname));
      if (buffer_tstflags(ctx.targ_buf, BUF_FLG_CMDLINE))
            strlcat(ucKeyname, "CMD-", sizeof(ucKeyname));
      strlcat(ucKeyname, keyname, sizeof(ucKeyname));
      strupr(ucKeyname);
      //logmsg("trying to look up key '%s'", ucKeyname);
      keydef = find_keydef(prof, ucKeyname);
    }
  }
  else {
    // first check ins/repl-keyname
    memset(ucKeyname, 0, sizeof(ucKeyname));
    if (view_tstflags(ctx.targ_view, VIEW_FLG_INSERTMODE))
      strlcat(ucKeyname, "INS-", sizeof(ucKeyname));
    else
      strlcat(ucKeyname, "REP-", sizeof(ucKeyname));
    strlcat(ucKeyname, keyname, sizeof(ucKeyname));
    strupr(ucKeyname);
    //logmsg("trying to look up key '%s'", ucKeyname);
    keydef = find_keydef(prof, ucKeyname);
  }
  
  if (keydef == NULL) {
    // then just try the keyname
    strlcpy(ucKeyname, keyname, sizeof(ucKeyname));
    strupr(ucKeyname);
    //logmsg("finally try to look up key '%s'", ucKeyname);
    keydef = find_keydef(prof, ucKeyname);
  }

  //struct keydef_t* keydef = find_keydef(prof, ucKeyname);
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
        
    if (update_context(&kbd_ctx)) {
      view_move_cursor_to(kbd_ctx.data_view, kbd_ctx.data_row, kbd_ctx.data_col);
      view_move_cursor_to(kbd_ctx.cmd_view, kbd_ctx.cmd_row, kbd_ctx.cmd_col);
    }
  }
  TRACE_RETURN(err);
}


int compare_keydefs_keyname(const void* a, const void* b)
{
  TRACE_ENTER;
  struct keydef_t* pa = (struct keydef_t*)a;
  struct keydef_t* pb = (struct keydef_t*)b;
  int icmp = strcasecmp(pa->keyname, pb->keyname);
  TRACE_RETURN(icmp);
}


struct keydef_t* _find_keydef(PROFILEPTR prof, const char* keyname)
{
  TRACE_ENTER;
  //logmsg("looking up key '%s' in profile '%s'", keyname, cstr_getbufptr(&prof->name));
  if (prof == NULL)
    TRACE_RETURN(NULL);
  if (prof->keydefs_sorted) {
    struct keydef_t srchkey;
    srchkey.keyname = keyname;
    srchkey.cmds = NULL;
    void* found = bsearch(&srchkey,
                          vec_getbufptr(&prof->key_defs),
                          vec_count(&prof->key_defs),
                          sizeof(struct keydef_t),
                          compare_keydefs_keyname);
    TRACE_RETURN((struct keydef_t*)found);
  }
  else {
    int i, n = vec_count(&prof->key_defs);
    for (i = 0; i < n; i++) {
      struct keydef_t* pkeydef = vec_get(&prof->key_defs, i);
      if (strcmp(pkeydef->keyname, keyname) == 0)
        TRACE_RETURN(pkeydef);
    }
  }
  TRACE_RETURN(NULL);
}


void sort_profile_keydefs(PROFILEPTR prof)
{
  TRACE_ENTER;
  if (prof->keydefs_sorted)
    TRACE_EXIT;
  qsort(vec_getbufptr(&prof->key_defs),
        vec_count(&prof->key_defs),
        sizeof(struct keydef_t),
        compare_keydefs_keyname);
  prof->keydefs_sorted = true;
  TRACE_EXIT;
}


struct keydef_t* find_keydef(PROFILEPTR prof, const char* keyname)
{
  TRACE_ENTER;
  struct keydef_t* keydef = NULL;
  //for (; keydef == NULL && prof != NULL; prof = prof->parent) {
    keydef = _find_keydef(prof, keyname);
  //}
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
    if (prof->keydefs_sorted)
      sort_profile_keydefs(prof);
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

