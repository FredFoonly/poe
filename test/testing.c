
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <stdarg.h>

#include "trace.h"

void _catch_signals(const char* filename, int linenum, const char* name, int* rc, sigjmp_buf *sigjmpbuf, jmp_buf *jmpbuf);
void _release_signals();
void _testing_catch_sig(int sigraised);


static int __tests_failed = 0;
static const char* __system_name = NULL;
static int __verbose = 0;
static const char* __test_filename = NULL;
static int __test_linenum = 0;
static const char* __test_name = NULL;
static int __rc = 0;
static int* __prc = &__rc;
static sigjmp_buf* __psigjmpbuf = NULL;
static jmp_buf* __pjmpbuf = NULL;



void start_testing(const char* system, int verbose)
{
  TRACE_ENTER;
  __system_name = system;
  __verbose = verbose;
  TRACE_EXIT;
}


int end_testing()
{
  TRACE_ENTER;
  if (__tests_failed == 0)
    printf("all tests succeeded\n");
  else
    printf("%d tests failed\n", __tests_failed);
  TRACE_RETURN(__tests_failed);
}



void _runtest(const char* filename, int linenum, const char* testname, void (*f)())
{
  TRACE_ENTER;
  static sigjmp_buf sigjmpbuf;
  static jmp_buf jmpbuf;
  int rc = 0;
  int code = sigsetjmp(sigjmpbuf, 1);
  if (code == 0) {
    code = setjmp(jmpbuf);
    if (code == 0) {
      _catch_signals(filename, linenum, testname, &rc, &sigjmpbuf, &jmpbuf);
      (*f)();
      _release_signals();
    } else {
      TRACE_CATCH;
      rc = code;
      _release_signals();
    }
  } else {
    //printf("after siglongjmp\n");
    TRACE_CATCH;
    //printf("after trace_catch\n");
    rc = code;
    _release_signals();
    //printf("after release_signals\n");
  }

  if (rc == 0 && __verbose) {
    printf("Passed %s\n\n", testname);
  }
  else if (rc != 0) {
    __tests_failed++;
    printf("%s:%d: Failed %s\n\n", filename, linenum, testname);
  }
  TRACE_EXIT;
}


void failtest(const char* fmt, ...)
{
  TRACE_ENTER;
  va_list ap;
  va_start(ap, fmt);
  char message[1024];
  vsnprintf(message, sizeof message, fmt, ap);
  va_end(ap);
  printf("Failing test %s: %s\n", __test_name, message);
  if (__pjmpbuf != NULL)
    longjmp(*__pjmpbuf, 1);
  TRACE_EXIT;
}


void _catch_signals(const char* filename, int linenum, const char* name, int* rc, sigjmp_buf* sigjmpbuf, jmp_buf* jmpbuf)
{
  TRACE_ENTER;
  *rc = 0;
  __test_filename = filename;
  __test_linenum = linenum;
  __test_name = name;
  __prc = rc;
  __psigjmpbuf = sigjmpbuf;
  __pjmpbuf = jmpbuf;
  signal(SIGHUP, _testing_catch_sig);
  //  signal(SIGINT, _testing_catch_sig);
  signal(SIGQUIT, _testing_catch_sig);
  signal(SIGILL, _testing_catch_sig);
  signal(SIGTRAP, _testing_catch_sig);
  signal(SIGABRT, _testing_catch_sig);
  signal(SIGFPE, _testing_catch_sig);
  signal(SIGBUS, _testing_catch_sig);
  signal(SIGSEGV, _testing_catch_sig);
  signal(SIGSYS, _testing_catch_sig);
  signal(SIGPIPE, _testing_catch_sig);
  signal(SIGTERM, _testing_catch_sig);
  signal(SIGTSTP, _testing_catch_sig);
  TRACE_EXIT;
}


void _release_signals()
{
  TRACE_ENTER;
  __test_filename = NULL;
  __test_linenum = 0;
  __test_name = NULL;
  __psigjmpbuf = NULL;
  __pjmpbuf = NULL;
  signal(SIGHUP, SIG_DFL);
  //  signal(SIGINT, SIG_DFL);
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
  TRACE_EXIT;
}


void _testing_catch_sig(int sigraised)
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
    printf("Error in test %s, caught signal %s\n", __test_name, signame);
    trace_stack_print();
  }
  if (__psigjmpbuf != NULL)
    siglongjmp(*__psigjmpbuf, sigraised);
}


void pe_exit(int rc)
{
  exit(rc);
}


void pe_err(int rc, const char* fmt, ...)
{
  va_list(ap);
  va_start(ap, fmt);
  char message[1024];
  vsnprintf(message, sizeof message, fmt, ap);
  va_end(ap);
  failtest( message);
}

