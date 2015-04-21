
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

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
#include "key_interp.h"
#include "buffer.h"
#include "view.h"
#include "window.h"
#include "commands.h"
#include "cmd_interp.h"
#include "editor_globals.h"




POE_ERR interpret_command_seq(cmd_ctx* ctx)
{
  TRACE_ENTER;
  POE_ERR rval = POE_ERR_UNK_CMD;
  //logmsg("interpreting cmd seq %d tokens long", pivec_count(cmdseq));
  while (!END_OF_CMD(ctx->cmdseq, ctx->pc) && cmd_error == POE_ERR_OK) {
    int args = -1;
    //logmsg("scanning for next command, *pc = %s", CMD_STRVAL(pivec_get(cmdseq, pc)));
    int cmdend = ctx->pc;
    while (!END_OF_CMD(ctx->cmdseq, cmdend))
	  cmdend++;
	cmdend++;
	
    //-- dump the command out for debugging
    //logmsg("before exec pc = %ld nextpc = %ld cmd len = %ld", pc, nextpc, nextpc-pc);
    /* cstr cmd_text = format_command(ctx->cmdseq, ctx->pc); */
    /* logmsg("looking up command %s", cstr_getbufptr(&cmd_text)); */
    /* cstr_destroy(&cmd_text); */
    //-- 
	
    command_handler_t func = lookup_command(ctx->cmdseq, ctx->pc, &args);
    //logmsg("back in dokey");
    if (func == NULL) {
	  // no cmd found
	  //logmsg("didn't find definition for command");
	  rval = cmd_error = POE_ERR_UNK_CMD;
    }
    else {
	  //logmsg("found definition for command");
	  // found command, possibly have args
	  //logmsg("checking args");
	  if (args < 0 || END_OF_CMD(ctx->cmdseq, args))
		args = -1;
	  //logmsg("running func");
	  ctx->pc = args;
	  update_context(ctx);
	  rval = cmd_error = func(ctx);
	  //logmsg("back from func");
    }
    //logmsg("after exec nextpc = %ld", nextpc);
	if (ctx->pc < cmdend)
	  ctx->pc = cmdend;
  }
  
  //logmsg("cmd err = %d", cmd_error);
  TRACE_RETURN(rval);
}


cstr format_command(const pivec* cmdseq, int pc)
{
  TRACE_ENTER;
  cstr s;
  int i;
  
  cstr_init(&s, 100);
  cstr_append(&s, '[');
  for (i = 0; !END_OF_CMD(cmdseq, pc+i); i++) {
    if (i > 0)
	  cstr_append(&s, ' ');
    if (CMD_IS_INT(pivec_get(cmdseq, pc+i))) {
	  int ival = CMD_INTVAL(pivec_get(cmdseq, pc+i));
	  cstr_appendf(&s, "%d", ival);
    }
    else {
	  const char* sval = CMD_STRVAL(pivec_get(cmdseq, pc+i));
	  // Check if there are any non-alphabetic chars
	  int hasnonalpha = false;
	  const char* pchval;
	  for (pchval = sval; *pchval != '\0'; pchval++) {
		if (!isalnum(*pchval))
		  hasnonalpha = true;
	  }
	  if (hasnonalpha) {
		cstr_appendf(&s, "\"%s\"", sval);
	  }
	  else {
		cstr_appendf(&s, "%s", sval);
	  }
    }
  }
  cstr_append(&s, ']');
  TRACE_RETURN(s);
}


cstr format_command_seq(const pivec* cmdseq, int pc)
{
  TRACE_ENTER;
  cstr s;
  int i;
  
  cstr_init(&s, 100);
  cstr_append(&s, '[');
  for (i = 0; !END_OF_CMDSEQ(cmdseq, pc+i); i++) {
    if (pivec_get(cmdseq, pc+i) == CMD_SEP) {
	  cstr_appendstr(&s, "] [");
    }
    else if (CMD_IS_INT(pivec_get(cmdseq, pc+i))) {
	  int ival = CMD_INTVAL(pivec_get(cmdseq, pc+i));
	  cstr_appendf(&s, "%d", ival);
	  if (!END_OF_CMDSEQ(cmdseq, pc+i+1))
		cstr_append(&s, ' ');
    }
    else {
	  const char* sval = CMD_STRVAL(pivec_get(cmdseq, pc+i));
	  // Check if there are any non-alphabetic chars
	  int hasnonalpha = false;
	  const char* pchval;
	  for (pchval = sval; *pchval != '\0'; pchval++) {
		if (!isalnum(*pchval))
		  hasnonalpha = true;
	  }
	  if (hasnonalpha) {
		cstr_appendf(&s, "\"%s\"", sval);
	  }
	  else {
		cstr_appendf(&s, "%s", sval);
	  }
	  if (!END_OF_CMD(cmdseq, pc+i+1))
		cstr_append(&s, ' ');
    }
  }
  cstr_append(&s, ']');
  TRACE_RETURN(s);
}


