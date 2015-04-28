
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/errno.h>
#include <sys/param.h>
#if defined(__OpenBSD__) || defined(__FreeBSD__)
#include <libgen.h>
#endif
#include <limits.h>
#include <ctype.h>

#include "trace.h"
#include "logging.h"
#include "poe_err.h"
#include "poe_exit.h"
#include "utils.h"
#include "vec.h"
#include "cstr.h"
#include "tabstops.h"
#include "margins.h"
#include "bufid.h"
#include "mark.h"
#include "key_interp.h"
#include "buffer.h"
#include "view.h"
#include "window.h"
#include "commands.h"
#include "cmd_interp.h"
#include "editor_globals.h"


enum token_t {tok_name, tok_strlit, tok_intlit, tok_unk, tok_end};

POE_ERR _finish_parsing_locate(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens);
POE_ERR _finish_parsing_change(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens);
POE_ERR _finish_parsing_define(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens);
POE_ERR _parse_define_subcommand(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens);
POE_ERR _finish_parsing_generic(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens);
bool _scancmdword(const cstr* str, cstr* tok_str, int* ppos);
bool _scanword(const cstr* str, cstr* tok_str, int* ppos);
bool _scannum(const cstr* str, cstr* tok_str, int* ppos);
enum token_t _scantoken(const cstr* str, cstr* tok_str, int* ppos);
bool _scankeyname(const cstr* str, cstr* tok_str, int* ppos);
//bool _scanlocateoptions(const cstr* str, cstr* tok_str, int* ppos);
bool _scan_locate_pattern(const cstr* str, cstr* tok_str, int* ppos, char* delimiter);
bool _scan_locate_replacement(const cstr* str, cstr* tok_str, int* ppos, char delimiter);


POE_ERR _parse_single_command(const cstr* str, cstr* tok, int* ppos, pivec* tokens, int level)
{
  TRACE_ENTER;
  POE_ERR err = POE_ERR_OK;
  int pos = *ppos;
  
  //logmsg("parsing single command '%s'", cstr_getcharptr(str, pos));

  if (cstr_get(str, pos) == '/') {
    // shortcut for locate
    pivec_append(tokens, CMD_STR(strsave("locate")));
    err = _finish_parsing_locate(str, tok, &pos, tokens);
    goto done;
  }
  else if (_scannum(str, tok, &pos)) {
    // shortcut for line
    long linenum = strtol(cstr_getbufptr(tok), NULL, 10);
    pivec_append(tokens, CMD_STR(strsave("LINE")));
    pivec_append(tokens, CMD_INT(linenum));
    err = _finish_parsing_generic(str, tok, &pos, tokens);
    goto done;
  }
  else if (_scancmdword(str, tok, &pos)) {
	//logmsg("parser scancmdword '%s'", cstr_getbufptr(tok));
    if (cstr_comparestr(tok, "l") == 0
        || cstr_comparestri(tok, "locate") == 0) {
      pivec_append(tokens, CMD_STR(strsave("locate")));
      err = _finish_parsing_locate(str, tok, &pos, tokens);
      goto done;
    }
    else if (cstr_comparestri(tok, "c") == 0
             || cstr_comparestri(tok, "change") == 0) {
      pivec_append(tokens, CMD_STR(strsave("change")));
      err = _finish_parsing_change(str, tok, &pos, tokens);
      goto done;
    }
    else if (level == 0 && (cstr_comparestri(tok, "def") == 0
							|| cstr_comparestri(tok, "define") == 0)) {
      pivec_append(tokens, CMD_STR(strsave("define")));
      err = _finish_parsing_define(str, tok, &pos, tokens);
      goto done;
    }
    else {
      pivec_append(tokens, CMD_STR(strsave(cstr_getbufptr(tok))));
      err = _finish_parsing_generic(str, tok, &pos, tokens);
      goto done;
    }
  }
  else {
    //logmsg("couldn't scan word");
    err = POE_ERR_UNK_CMD;
    goto done;
  }

 done:
  *ppos = pos;
  TRACE_RETURN(err);
}



POE_ERR parse_cmdline(const cstr* str, pivec* tokens, int* parsepos)
{
  TRACE_ENTER;
  pivec_clear(tokens);
  POE_ERR err = POE_ERR_OK;
  int pos = 0;
  cstr tmp;
  cstr_initfrom(&tmp, str);
  cstr_trimright(&tmp, poe_iswhitespace);
  cstr_trimleft(&tmp, poe_iswhitespace);
  if (cstr_count(&tmp) == 0) {
	*parsepos = 0;
    err = POE_ERR_OK;
    goto done;
  }

  cstr tok;
  cstr_init(&tok, 20);
  
  err = _parse_single_command(&tmp, &tok, &pos, tokens, 0);

 done:
  cstr_destroy(&tok);
  cstr_destroy(&tmp);
  *parsepos = pos;
  TRACE_RETURN(err);
}



//
// Parsing support
//

POE_ERR _finish_parsing_locate(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens)
{
  TRACE_ENTER;
  POE_ERR err = POE_ERR_UNK_CMD;
  char delim = ' ';
  cstr srch_str;
  cstr_init(&srch_str, 100);
  bool bHavePat = _scan_locate_pattern(str, &srch_str, ppos, &delim);
  if (bHavePat) {
    _scanword(str, tok_str, ppos); // get any search options
    pivec_append(tokens, CMD_STR(strsave(cstr_getbufptr(&srch_str))));
    pivec_append(tokens, CMD_STR(strsave(cstr_getbufptr(tok_str))));
    err = POE_ERR_OK;
  }
  cstr_destroy(&srch_str);
  TRACE_RETURN(err);
}


POE_ERR _finish_parsing_change(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens)
{
  TRACE_ENTER;
  POE_ERR err = POE_ERR_UNK_CMD;
  char delim = ' ';
  cstr srch_str;
  cstr repl_str;
  cstr_init(&srch_str, 100);
  cstr_init(&repl_str, 100);
  // get the pattern
  bool bHavePat = _scan_locate_pattern(str, &srch_str, ppos, &delim);
  if (bHavePat) {
	// get the replacement text
	bool bHaveReplace = _scan_locate_replacement(str, &repl_str, ppos, delim);
	if (bHaveReplace) {
	  _scanword(str, tok_str, ppos); // get any search options
	  pivec_append(tokens, CMD_STR(strsave(cstr_getbufptr(&srch_str))));
	  pivec_append(tokens, CMD_STR(strsave(cstr_getbufptr(&repl_str))));
	  pivec_append(tokens, CMD_STR(strsave(cstr_getbufptr(tok_str))));
	  err = POE_ERR_OK;
	}
  }
  cstr_destroy(&srch_str);
  cstr_destroy(&repl_str);
  TRACE_RETURN(err);
}


POE_ERR _finish_parsing_define(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens)
{
  TRACE_ENTER;

  if (!_scankeyname(str, tok_str, ppos))
	TRACE_RETURN(POE_ERR_INVALID_KEY);
  pivec_append(tokens, CMD_STR(strsave(cstr_getbufptr(tok_str))));

  //logmsg("keyname = '%s'", cstr_getbufptr(tok_str));
  if (*ppos < cstr_count(str)) {
	*ppos = cstr_skipwhile(str, *ppos, poe_iswhitespace);
  }
  if (*ppos >= cstr_count(str) || cstr_get(str, *ppos) != '=')
	TRACE_RETURN(POE_ERR_MISSING_EQ_DEFN);

  //logmsg("found =");
  (*ppos)++;
  POE_ERR err = POE_ERR_OK;
  do {
	err = _parse_define_subcommand(str, tok_str, ppos, tokens);
  } while (err == POE_ERR_OK && *ppos < cstr_count(str));
  pivec_append(tokens, CMD_NULL);

  //logmsg("finish_parsing_define err = %d, pos = %d, len = %d", err, *ppos, cstr_count(str));
  if (*ppos >= cstr_count(str))
	err = POE_ERR_OK;
  TRACE_RETURN(err);
}


POE_ERR _parse_define_subcommand(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens)
{
  TRACE_ENTER;
  POE_ERR err = POE_ERR_OK;

  //logmsg("entering parse_define_subcommand");
  int pos = *ppos;
  if (pos < cstr_count(str)) {
	pos = cstr_skipwhile(str, pos, poe_iswhitespace);
  }
  // skip '['
  if (pos >= cstr_count(str)) {
	logmsg("command parser: scanned past end looking for start of cmd seq");
	TRACE_RETURN(POE_ERR_MISSING_QUOTE);
  }
  char c = cstr_get(str, pos);
  //logmsg("parse_define_subcommand @ %d c1 = '%c'", pos, c);
  if (c != '[') {
	logmsg("command parser: missing [ @ %d, got %d (%c)", pos, c, c);
	TRACE_RETURN(POE_ERR_MISSING_QUOTE);
  }

  // parse and check command
  pos++;
  int start_of_cmd_pos = pos;
  int start_of_cmd_tokens = pivec_count(tokens);
  err = _parse_single_command(str, tok_str, &pos, tokens, 1);
  if (err != POE_ERR_OK)
	TRACE_RETURN(err);
  pivec_append(tokens, CMD_SEP);
  int end_of_cmd_tokens = pivec_count(tokens);
  pivec_append(tokens, CMD_NULL);
  err = check_command(tokens, start_of_cmd_tokens);
  pivec_remove(tokens, end_of_cmd_tokens);
  if (err != POE_ERR_OK) {
	*ppos = start_of_cmd_pos;
	TRACE_RETURN(err);
  }

  // skip spaces again
  if (pos < cstr_count(str)) {
	pos = cstr_skipwhile(str, pos, poe_iswhitespace);
  }

  // skip ']'
  //logmsg("attempting to get trailing ']' @ %d", pos);
  c = cstr_get(str, pos);
  //logmsg("parse_define_subcommand @ %d c2 = '%c'", pos, c);
  if (c != ']') {
	logmsg("command parser: missing ] @ %d -- have %d (%c)", pos, c, c);
	//logmsg("cmd = '%s'", cstr_getbufptr(str));
	TRACE_RETURN(POE_ERR_MISSING_QUOTE);
  }

  // skip whitespace
  pos++;
  //logmsg("parse_define_subcommand skipping whitespace @ %d of %d", pos, cstr_count(str));
  if (pos < cstr_count(str)) {
	pos = cstr_skipwhile(str, pos, poe_iswhitespace);
  }

  //logmsg("exiting parse_define_subcommand");
  *ppos = pos;
  TRACE_RETURN(err);
}


POE_ERR _finish_parsing_generic(const cstr* str, cstr* tok_str, int* ppos, pivec* tokens)
{
  TRACE_ENTER;
  POE_ERR err = POE_ERR_OK;
  enum token_t tok;
  for (tok = _scantoken(str, tok_str, ppos);
       tok != tok_end && tok != tok_unk;
       tok = _scantoken(str, tok_str, ppos)) {
    const char* pszTok = cstr_getbufptr(tok_str);
    switch (tok) {
    case tok_name:
      pivec_append(tokens, CMD_STR(strsave(pszTok)));
      break;
    case tok_strlit:
      pivec_append(tokens, CMD_STR(strsave(pszTok)));
      break;
    case tok_intlit:
      {
        long l = strtol(pszTok, NULL, 10);
        pivec_append(tokens, CMD_INT((int)l));
      }
      break;
    case tok_unk:
      break;
    case tok_end:
      break;
    }
  }
  if (tok == tok_unk)
    err = POE_ERR_UNK_CMD;
  TRACE_RETURN(err);
}


bool _scan_locate_pattern(const cstr* str, cstr* tok_str, int* ppos, char* pdelimiter)
{
  TRACE_ENTER;
  int len = cstr_count(str);
  cstr_clear(tok_str);
  if (*ppos >= len)
    TRACE_RETURN(false);
  
  int patstart = cstr_skip_ws(str, *ppos);
  if (patstart >= len) {
    TRACE_RETURN(false);
  }

  char delimiter = cstr_get(str, patstart);
  if (isalnum(delimiter)) {
    TRACE_RETURN(false);
   }
  int patend = cstr_skiptill_chr(str, patstart+1, delimiter);
  if (patend >= len) {
    // Means that the user entered /pattern<return>.
    // I'm ok with this - it's one character shorter.
    const char* s = cstr_getcharptr(str, patstart+1);
    cstr_assignstrn(tok_str, s, patend-patstart);
    *ppos = patend;
    *pdelimiter = delimiter;
    TRACE_RETURN(true);
  }
  else {
    const char* s = cstr_getcharptr(str, patstart+1);
    cstr_assignstrn(tok_str, s, patend-patstart-1);
    *ppos = patend+1;
    *pdelimiter = delimiter;
    TRACE_RETURN(true);
  }
}


bool _scan_locate_replacement(const cstr* str, cstr* tok_str, int* ppos, char delimiter)
{
  TRACE_ENTER;
  int len = cstr_count(str);
  cstr_clear(tok_str);
  if (*ppos >= len)
    TRACE_RETURN(false);

  int replstart = *ppos;
  int replend = cstr_skiptill_chr(str, replstart, delimiter);
  if (replend >= len) {
    // Means that the user entered /pattern<return>.
    // I'm ok with this - it's one character shorter.
    const char* s = cstr_getcharptr(str, replstart);
    cstr_assignstrn(tok_str, s, replend-replstart);
    *ppos = replend;
    TRACE_RETURN(true);
  }
  else {
    const char* s = cstr_getcharptr(str, replstart);
    cstr_assignstrn(tok_str, s, replend-replstart);
    *ppos = replend+1;
    TRACE_RETURN(true);
  }
}


bool _scannum(const cstr* str, cstr* tok_str, int* ppos)
{
  TRACE_ENTER;
  int len = cstr_count(str);
  cstr_clear(tok_str);
  int wrdstart = cstr_skip_ws(str, *ppos);
  if (wrdstart >= len) {
    TRACE_RETURN(false);
  }
  int wrdend = cstr_skipwhile(str, wrdstart, poe_isdigit);
  if (wrdend == wrdstart) {
    TRACE_RETURN(false);
  }
  const char* s = cstr_getcharptr(str, wrdstart);
  cstr_assignstrn(tok_str, s, wrdend-wrdstart);
  *ppos = wrdend;
  TRACE_RETURN(true);
}


bool _scancmdword(const cstr* str, cstr* tok_str, int* ppos)
{
  TRACE_ENTER;
  int len = cstr_count(str);
  cstr_clear(tok_str);
  int wrdstart = cstr_skip_ws(str, *ppos);
  if (wrdstart >= len) {
    TRACE_RETURN(false);
  }
  int wrdend = cstr_skipwhile(str, wrdstart, poe_iscmdword);
  if (wrdend == wrdstart) {
    TRACE_RETURN(false);
  }
  const char* s = cstr_getcharptr(str, wrdstart);
  cstr_assignstrn(tok_str, s, wrdend-wrdstart);
  *ppos = wrdend;
  TRACE_RETURN(true);
}


bool _scanword(const cstr* str, cstr* tok_str, int* ppos)
{
  TRACE_ENTER;
  int len = cstr_count(str);
  cstr_clear(tok_str);
  int wrdstart = cstr_skip_ws(str, *ppos);
  if (wrdstart >= len) {
    TRACE_RETURN(false);
  }
  int wrdend = cstr_skipwhile(str, wrdstart, poe_isword);
  if (wrdend == wrdstart) {
    TRACE_RETURN(false);
  }
  const char* s = cstr_getcharptr(str, wrdstart);
  cstr_assignstrn(tok_str, s, wrdend-wrdstart);
  *ppos = wrdend;
  TRACE_RETURN(true);
}


/* bool _scanlocateoptions(const cstr* str, cstr* tok_str, int* ppos) */
/* { */
/*   TRACE_ENTER; */
/*   int len = cstr_count(str); */
/*   cstr_clear(tok_str); */
/*   int wrdstart = cstr_skip_ws(str, *ppos); */
/*   if (wrdstart >= len) { */
/*     TRACE_RETURN(false); */
/*   } */
/*   int wrdend = cstr_skipwhile(str, wrdstart, poe_islocateoption); */
/*   if (wrdend == wrdstart) { */
/*     TRACE_RETURN(false); */
/*   } */
/*   const char* s = cstr_getcharptr(str, wrdstart); */
/*   cstr_assignstrn(tok_str, s, wrdend-wrdstart); */
/*   *ppos = wrdend; */
/*   TRACE_RETURN(true); */
/* } */


enum token_t _scantoken(const cstr* str, cstr* tok_str, int* ppos)
{
  TRACE_ENTER;
  int len = cstr_count(str);
  cstr_clear(tok_str);
  if (*ppos >= len)
    TRACE_RETURN(tok_end);

  int wrdstart = cstr_skip_ws(str, *ppos);
  if (wrdstart >= len) {
    TRACE_RETURN(tok_end);
  }

  int wrdend = wrdstart;
  enum token_t tok = tok_unk;
  char c = cstr_get(str, wrdstart);
  //logmsg("scantoken pos %d char %c", wrdstart, c);
  if (c == '[' || c == ']')
	TRACE_RETURN(tok_end);
  if (poe_isdigit(c)) {
    wrdend = cstr_skipwhile(str, wrdstart, poe_isdigit);
    if (wrdend > wrdstart) {
      tok = tok_intlit;
      const char* pachTok = cstr_getcharptr(str, wrdstart);
      cstr_assignstrn(tok_str, pachTok, wrdend-wrdstart);
      //logmsg("_scantoken found int %s", cstr_getbufptr(tok_str));
    }
  }
  else if (c == '\'') {
    wrdend = cstr_skiptill(str, wrdstart+1, poe_isquote);
    if (wrdend > wrdstart+1 && cstr_get(str, wrdend) == '\'') {
      tok = tok_intlit;
      char tmp[16];
      char c = cstr_get(str, wrdstart+1);
      snprintf(tmp, sizeof(tmp), "%d", c);
      cstr_assignstr(tok_str, tmp);
      //logmsg("_scantoken found char %s (%c)", tmp, c);
      wrdend++; // account for trailing quote
    }
  }
  else if (c == '"') {
    wrdend = cstr_skiptill(str, wrdstart+1, poe_isquote);
    if (wrdend > wrdstart+2 && cstr_get(str, wrdend) == '"') {
      tok = tok_strlit;
      const char* pachTok = cstr_getcharptr(str, wrdstart+1);
      cstr_assignstrn(tok_str, pachTok, wrdend-wrdstart-1);
      //logmsg("_scantoken found strlit \"%s\"", cstr_getbufptr(tok_str));
      wrdend++; // account for trailing quote
    }
  }
  else {
    wrdend = cstr_skipwhile(str, wrdstart+1, poe_isword);
    if (wrdend > wrdstart) {
      tok = tok_name;
      const char* pachTok = cstr_getcharptr(str, wrdstart);
      cstr_assignstrn(tok_str, pachTok, wrdend-wrdstart);
      //logmsg("_scantoken found word %s", cstr_getbufptr(tok_str));
    }
  }
  *ppos = wrdend;
  TRACE_RETURN(tok);
}


bool _scankeyname(const cstr* str, cstr* tok_str, int* ppos)
{
  TRACE_ENTER;
  int len = cstr_count(str);
  cstr_clear(tok_str);
  if (*ppos >= len)
    TRACE_RETURN(false);

  int wrdstart = cstr_skip_ws(str, *ppos);
  if (wrdstart >= len) {
    TRACE_RETURN(false);
  }

  int wrdend = wrdstart;
  bool havetoken = false;
  char c = cstr_get(str, wrdstart);
  //logmsg("scantoken pos %d char %c", wrdstart, c);
  /* if (poe_isdigit(c)) { */
  /* 	havetoken = false; */
  /* } */
  /* else */
  if (c == '\'') {
    wrdend = cstr_skiptill(str, wrdstart+1, poe_isquote);
    if (wrdend > wrdstart+1 && cstr_get(str, wrdend) == '\'') {
      char tmp[16];
      char c = cstr_get(str, wrdstart+1);
      snprintf(tmp, sizeof(tmp), "%d", c);
      cstr_assignstr(tok_str, tmp);
      //logmsg("_scantoken found char %s (%c)", tmp, c);
      wrdend++; // account for trailing quote
	  havetoken = true;
    }
  }
  else if (c == '"') {
    wrdend = cstr_skiptill(str, wrdstart+1, poe_isquote);
    if (wrdend > wrdstart+2 && cstr_get(str, wrdend) == '"') {
      const char* pachTok = cstr_getcharptr(str, wrdstart+1);
      cstr_assignstrn(tok_str, pachTok, wrdend-wrdstart-1);
      //logmsg("_scantoken found strlit \"%s\"", cstr_getbufptr(tok_str));
      wrdend++; // account for trailing quote
	  havetoken = true;
    }
  }
  else {
    wrdend = cstr_skiptill(str, wrdstart+1, poe_isnotcmdword);
    if (wrdend > wrdstart) {
      const char* pachTok = cstr_getcharptr(str, wrdstart);
      cstr_assignstrn(tok_str, pachTok, wrdend-wrdstart);
      //logmsg("_scantoken found word %s", cstr_getbufptr(tok_str));
	  havetoken = true;
    }
  }

  if (havetoken)
	*ppos = wrdend;
  TRACE_RETURN(havetoken);
}
