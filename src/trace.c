

#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "poe_exit.h"


#define MAX_DEPTH (256)
const char *_trace_stack[MAX_DEPTH];
int _stack_top = 0;


void init_trace_stack()
{
  memset(_trace_stack, 0, sizeof(_trace_stack));
  _stack_top = 0;
}


void trace_stack_print()
{
  int i;
  logerr("Call Stack: (%d frames)", _stack_top);
  for (i = 0; i < _stack_top; i++)
    logerr("%s %03d : %s", (i==0?"   " : "-->"), i, _trace_stack[i]);
  logerr("--------");
}


int _trace_enter(const char* func_name)
{
  if (_stack_top >= MAX_DEPTH)
    poe_err(1, "trace_enter: Trace stack overflow");
  _trace_stack[_stack_top++] = func_name;
  return _stack_top-1;
}


void _trace_exit(const char* func_name, int level)
{
  if (_stack_top <= 0)
    poe_err(1, "trace_exit : Trace stack underflow");
  if (level != _stack_top-1 || _trace_stack[_stack_top-1] != func_name)
    poe_err(1, "trace_exit : Trace stack mismatch at pop - expected %d:%s but found %d:%s\n",
           level, func_name, _stack_top-1, _trace_stack[_stack_top-1]);
  _trace_stack[--_stack_top] = NULL;
}


void _trace_catch(const char* func_name, int new_level)
{
  int old_level = _stack_top;
  if (old_level == new_level)
    return;
  if (old_level <= new_level)
    poe_err(1, "trace_catch : Caught downward throw");
  if (_trace_stack[new_level] != func_name)
    poe_err(1, "trace_catch : Trace stack mismatch at throw from %d - expected %d:%s but found %d:%s\n",
           old_level, new_level, func_name, old_level-1, _trace_stack[old_level-1]);
  _stack_top = new_level+1;
  memset(_trace_stack+new_level+1, 0, (old_level - _stack_top - 1) * sizeof(_trace_stack[0]));
}

