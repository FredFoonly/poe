
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <limits.h>

#include "trace.h"
#include "logging.h"
#include "poe_exit.h"
#include "poe_err.h"
#include "utils.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "tabstops.h"
#include "margins.h"
#include "mark.h"
#include "markstack.h"
#include "key_interp.h"
#include "view.h"
#include "window.h"
#include "buffer.h"
#include "commands.h"
#include "getkey.h"
#include "cmd_interp.h"
#include "default_profile.h"
#include "editor_globals.h"



struct cmdline_args {
  int test;
  int help;
  int logging;
  const char* escdelay;
  struct pivec_t/* cstr* */ files;
  char* error;
};



int parse_args(int argc, char** argv, struct cmdline_args* args);
void show_args(struct cmdline_args* args);
void show_help();

void _catch_signals(int* rc, sigjmp_buf* sigjmpbuf, jmp_buf* jmpbuf);
void _release_signals();
void _pe_catch_sig(int sigraised);
void _pe_resize_sig(int sigraised);

static int __rc = 0;
static int* __prc = &__rc;
static sigjmp_buf* __psigjmpbuf = NULL;
static jmp_buf* __pjmpbuf = NULL;


int _do_main(int argc, char** argv);
const char* decode_key(int c);


int main(int argc, char** argv)
{
  init_trace_stack();
  TRACE_ENTER;
  int rc = 0;
  static sigjmp_buf sigjmpbuf;
  static jmp_buf jmpbuf;
  int code = sigsetjmp(sigjmpbuf, 1);
  if (code == 0) {
    code = setjmp(jmpbuf);
    if (code == 0) {
      _catch_signals(&rc, &sigjmpbuf, &jmpbuf);
      _do_main(argc, argv);
      _release_signals();
    }
    else {
      TRACE_CATCH;
      rc = code;
      _release_signals();
    }
  }
  else {
    TRACE_CATCH;
    rc = code;
    _release_signals();
  }

  TRACE_RETURN(rc);
}


int _do_main(int argc, char** argv)
{
  TRACE_ENTER;

  ensure_poe_dir();
  
  // parse command args
  struct cmdline_args args;
  memset(&args, 0, sizeof(args));
  args.logging = LOG_LEVEL_ERR;
  args.escdelay = getenv("ESCDELAY");
  if (args.escdelay == NULL || strlen(args.escdelay) == 0)
    args.escdelay = "1000";
  pivec_init(&args.files, 1);

  //fprintf(stderr, "parsing arguments\n");
  if (!parse_args(argc, argv, &args)) {
    fprintf(stderr, "pe error: %s\n", args.error);
    show_help();
    poe_exit(1);
  }

  //fprintf(stderr, "showing help arguments\n");
  if (args.help) {
    show_help();
    poe_exit(1);
  }

  // show_args(&args);

  // attempt to set curses escape delay via the environment
  if (args.escdelay != NULL && strlen(args.escdelay) > 0)
    setenv("ESCDELAY", args.escdelay, 1);

  //fprintf(stderr, "logging = %d\n", args.logging);
  init_logging(args.logging);
  //logmsg("========================================");
  tabs_init(&default_tabstops, 0, 8, NULL);
  margins_init(&default_margins, 0, 79, 4);

  //logmsg("init marks");
  init_marks();
  //logmsg("init markstack");
  init_markstack();
  //logmsg("init buffer");
  init_buffer();
  //logmsg("init windows");
  init_windows();
  //logmsg("init getkey");
  init_getkey();
  //logmsg("init key interp");
  init_key_interp();
  //logmsg("setting default profile");
  set_default_profile();
  //logmsg("init commands");
  init_commands();

  // attempt to set curses escape delay directly.
  if (args.escdelay != NULL && strlen(args.escdelay) > 0)
    set_escdelay((int)atol(args.escdelay));

  // Create internal buffers (should probably do this before loading the files...)
  dir_buffer = buffer_alloc(".DIR", BUF_FLG_INTERNAL|BUF_FLG_NEW, 0);
  keys_buffer = buffer_alloc(".KEYS", BUF_FLG_INTERNAL|BUF_FLG_NEW, 0);
  unnamed_buffer = buffer_alloc(".UNNAMED", BUF_FLG_INTERNAL|BUF_FLG_NEW, 0);
  buffer_ensure_min_lines(dir_buffer, false);
  buffer_ensure_min_lines(keys_buffer, false);
  buffer_ensure_min_lines(unnamed_buffer, false);

  // load the files
  int i, n = pivec_count(&args.files);
  if (n == 0) {
    //logmsg("allocating initial empty buffer");
    BUFFER initial_buffer = buffer_alloc("", BUF_FLG_VISIBLE, 0);
    buffer_ensure_min_lines(initial_buffer, false);
  }
  else {
    //logmsg("loading files from command line");
    for (i = 0; i < n; i++) {
      BUFFER buf = buffer_alloc("", BUF_FLG_VISIBLE, 0);
      cstr* filename = (cstr*)pivec_get(&args.files, i);
      //logmsg("loading file '%s'", cstr_getbufptr(filename));
      /*POE_ERR err = */buffer_load(buf, filename, tabexpand);
    }
  }

  // Event loop for the editor
  char achKeyname[64];
  do {
    wins_ensure_initial_win(); // make darn sure we have a view in the main slot
    wins_repaint_all();
    refresh();

    achKeyname[0] = '\0';

    const char* lpszKeyname = ui_get_key();
    if (lpszKeyname != NULL) {
      //logmsg("---------------------------------------------------------");
      //logmsg("got key '%s'", lpszKeyname);
      strlcpy(achKeyname, lpszKeyname, sizeof(achKeyname));
      wins_handle_key(achKeyname);
      wins_set_message(poe_err_message(cmd_error));
    }
  } while (!__quit && visible_buffers_count() != 0);

  //logmsg("closing commands");
  close_commands();
  //logmsg("closing key interp");
  close_key_interp();
  //logmsg("closing getkey");
  close_getkey();
  //logmsg("closing windows");
  close_windows();
  //logmsg("shutting down marks");
  shutdown_marks();
  //logmsg("shutting down markstack");
  shutdown_markstack();
  //logmsg("shutting down buffer");
  shutdown_buffer();
  //logmsg("exiting");
  TRACE_RETURN(0);
  // shutdown_trace_stack();
}


int parse_args(int argc, char** argv, struct cmdline_args* args)
{
  TRACE_ENTER;
  int i;
  //int errors = 0;
  for (i = 1; i < argc; i++) {
    if (strcasecmp(argv[i], "-test") == 0) {
      ++args->test;
    }
    else if (strcasecmp(argv[i], "-help") == 0) {
      ++args->help;
    }
    else if (strcasecmp(argv[i], "-logerr") == 0) {
      args->logging = LOG_LEVEL_ERR;
    }
    else if (strcasecmp(argv[i], "-logmsg") == 0) {
      args->logging = LOG_LEVEL_MSG;
    }
    else if (strcasecmp(argv[i], "-escdelay") == 0) {
      if (argc <= i+1) {
        fprintf(stderr, "missing value for -escdelay\b");
        exit(1);
      }
      args->escdelay = argv[i+1];
      ++i;
    }
    else if (argv[i][0] == '-') {
      char msgbuf[1024];
      snprintf(msgbuf, sizeof(msgbuf), "unknown option %s", argv[i]);
      args->error = strsave(msgbuf);
      TRACE_RETURN(0);
    }
    else {
      cstr* filename = cstr_alloc(strlen(argv[i])+1);
      cstr_assignstr(filename, argv[i]);
      pivec_append(&args->files, (intptr_t)filename);
    }
  }
  TRACE_RETURN(1);
}


void show_args(struct cmdline_args* args)
{
  TRACE_ENTER;
  int i, n;
  fprintf(stderr, "pe args:\n");
  fprintf(stderr, "  test = %d\n", args->test);
  fprintf(stderr, "  help = %d\n", args->help);
  fprintf(stderr, "  log level = %d\n", args->logging);
  fprintf(stderr, "  escdelay = %s\n", args->escdelay);
  n = pivec_count(&args->files);
  for (i = 0; i < n; i++) {
    cstr* filename = (cstr*)pivec_get(&args->files, i);
    fprintf(stderr, "  file %d = %s\n", i, cstr_getbufptr(filename));
  }
  TRACE_EXIT;
}


void show_help()
{
  TRACE_ENTER;
  fprintf(stderr, "pe [option]* file1...\n");
  fprintf(stderr, " -test\trun tests\n");
  fprintf(stderr, " -help\tshow this help\n");
  fprintf(stderr, " -logerr\tlog only errors (default)\n");
  fprintf(stderr, " -logmsg\tlog informational messages\n");
  fprintf(stderr, " -escdelay n\tset escape delay (msec)\n");
  TRACE_EXIT;
}



void _catch_signals(int* rc, sigjmp_buf* sigjmpbuf, jmp_buf* jmpbuf)
{
  TRACE_ENTER;
  *rc = 0;
  /* __test_filename = filename; */
  /* __test_linenum = linenum; */
  /* __test_name = name; */
  __prc = rc;
  __psigjmpbuf = sigjmpbuf;
  __pjmpbuf = jmpbuf;
  signal(SIGHUP, _pe_catch_sig);
  signal(SIGINT, _pe_catch_sig);
  signal(SIGQUIT, _pe_catch_sig);
  signal(SIGILL, _pe_catch_sig);
  signal(SIGTRAP, _pe_catch_sig);
  signal(SIGABRT, _pe_catch_sig);
  signal(SIGFPE, _pe_catch_sig);
  signal(SIGBUS, _pe_catch_sig);
  signal(SIGSEGV, _pe_catch_sig);
  signal(SIGSYS, _pe_catch_sig);
  signal(SIGPIPE, _pe_catch_sig);
  signal(SIGTERM, _pe_catch_sig);
  signal(SIGTSTP, _pe_catch_sig);
  signal(SIGWINCH, _pe_resize_sig);
  TRACE_EXIT;
}


void _release_signals()
{
  TRACE_ENTER;
  /* __test_filename = NULL; */
  /* __test_linenum = 0; */
  /* __test_name = NULL; */
  __psigjmpbuf = NULL;
  __pjmpbuf = NULL;
  signal(SIGHUP, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGILL, SIG_DFL);
  signal(SIGTRAP, SIG_DFL);
  signal(SIGABRT, SIG_DFL);
  signal(SIGFPE, SIG_DFL);
  signal(SIGBUS, SIG_DFL);
  signal(SIGSEGV, SIG_DFL);
  signal(SIGSYS, SIG_DFL);
  signal(SIGPIPE, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGTSTP, SIG_DFL);
  signal(SIGWINCH, SIG_DFL);
  TRACE_EXIT;
}


void _pe_resize_sig(int sigraised)
{
  if (sigraised == SIGWINCH)
    __resize_needed = true;
}


void _pe_catch_sig(int sigraised)
{
  char* signame = NULL;
  switch (sigraised) {
  case SIGHUP: signame="SIGHUP"; break;
  case SIGINT: signame="SIGINT"; break;
  case SIGQUIT: signame="SIGQUIT"; break;
  case SIGILL: signame="SIGILL"; break;
  case SIGTRAP: signame="SIGTRAP"; break;
  case SIGABRT: signame="SIGABRT"; break;
  case SIGEMT: signame="SIGEMT"; break;
  case SIGFPE: signame="SIGFPE"; break;
  case SIGKILL: signame="SIGKILL"; break;
  case SIGBUS: signame="SIGBUS"; break;
  case SIGSEGV: signame="SIGSEGV"; break;
  case SIGSYS: signame="SIGSYS"; break;
  case SIGPIPE: signame="SIGPIPE"; break;
  case SIGALRM: signame="SIGALRM"; break;
  case SIGTERM: signame="SIGTERM"; break;
  case SIGURG: signame="SIGURG"; break;
  case SIGSTOP: signame="SIGSTOP"; break;
  case SIGTSTP: signame="SIGTSTP"; break;
  case SIGCONT: signame="SIGCONT"; break;
  case SIGCHLD: signame="SIGCHLD"; break;
  case SIGTTIN: signame="SIGTTIN"; break;
  case SIGTTOU: signame="SIGTTOU"; break;
  case SIGIO: signame="SIGIO"; break;
  case SIGXCPU: signame="SIGXCPU"; break;
  case SIGXFSZ: signame="SIGXFSZ"; break;
  case SIGVTALRM: signame="SIGVTALRM"; break;
  case SIGPROF: signame="SIGPROF"; break;
  case SIGWINCH: signame="SIGWINCH"; break;
  case SIGINFO: signame="SIGINFO"; break;
  case SIGUSR1: signame="SIGUSR1"; break;
  case SIGUSR2: signame="SIGUSR2"; break;
  case SIGTHR: signame="SIGTHR"; break;
  default: signame = NULL;
  }
  if (signame != NULL) {
    //  printf("Error in test %s, caught signal %s\n", __test_name, signame);
    logerr("Caught signal %s", signame);
    trace_stack_print();
  }
  if (__psigjmpbuf != NULL)
    siglongjmp(*__psigjmpbuf, sigraised);
}


