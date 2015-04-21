
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <err.h>

#include "trace.h"
#include "logging.h"


void poe_exit(int rc)
{
  exit(rc);
}


void poe_err(int rc, const char* fmt, ...)
{
  TRACE_ENTER;
  va_list(ap);
  va_start(ap, fmt);
  char message[1024];
  vsnprintf(message, sizeof message, fmt, ap);
  va_end(ap);
  logerr("%s", message);
  trace_stack_print();
  err(rc, message);
  TRACE_EXIT;
}

