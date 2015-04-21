
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "trace.h"
#include "utils.h"
#include "cstr.h"
#include "testing.h"



void test_cstr_1()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_init(&v, 1);
  if (cstr_capacity(&v) != 1)
    failtest("capacity %d != 1", cstr_capacity(&v));
  if (cstr_count(&v) != 0)
    failtest("count %d != 0", cstr_count(&v));

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_2()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_init(&v, 1);

  cstr_append(&v, 'a');

  if (cstr_capacity(&v) != 2)  // need a null terminator that isn't reflected in count
    failtest("capacity %d != 2", cstr_capacity(&v));
  if (cstr_count(&v) != 1)
    failtest("count %d != 1", cstr_count(&v));

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_3()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_init(&v, 1);

  cstr_append(&v, 'a');
  cstr_append(&v, 'b');

  if (cstr_capacity(&v) != 4)  // need a null terminator that isn't reflected in coutn
    failtest("capacity %d != 4", cstr_capacity(&v));
  if (cstr_count(&v) != 2)
    failtest("count %d != 2", cstr_count(&v));

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_4()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_init(&v, 1);

  cstr_append(&v, 'a');
  cstr_append(&v, 'b');
  cstr_append(&v, 'c');

  if (cstr_capacity(&v) != 4)
    failtest("capacity %d != 2", cstr_capacity(&v));
  if (cstr_count(&v) != 3)
    failtest("count %d != 3", cstr_count(&v));
  if (cstr_get(&v, 0) != 'a')
    failtest("cstr[0] %c != 'a'", cstr_get(&v, 0));
  if (cstr_get(&v, 1) != 'b')
    failtest("cstr[1] %c != 'b'", cstr_get(&v, 1));
  if (cstr_get(&v, 2) != 'c')
    failtest("cstr[2] %c != 'c'", cstr_get(&v, 2));

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_5()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_init(&v, 1);

  cstr_append(&v, 'a');
  cstr_append(&v, 'b');
  cstr_append(&v, 'c');
  cstr_remove(&v, 1);

  if (cstr_capacity(&v) != 4)
    failtest("capacity = %d\n", cstr_capacity(&v));
  if (cstr_count(&v) != 2)
    failtest("count = %d\n", cstr_count(&v));
  if (cstr_get(&v, 0) != 'a')
    failtest("cstr[0] %c != 'a'\n", cstr_get(&v, 0));
  if (cstr_get(&v, 1) != 'c')
    failtest("cstr[1] %c != 'c'\n", cstr_get(&v, 1));

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_6()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_init(&v, 1);

  cstr_insert(&v, 0, 'c');
  cstr_insert(&v, 0, 'b');
  cstr_insert(&v, 0, 'a');

  if (cstr_capacity(&v) != 4)
    failtest("capacity = %d\n", cstr_capacity(&v));
  if (cstr_count(&v) != 3)
    failtest("count = %d\n", cstr_count(&v));
  if (cstr_get(&v, 0) != 'a')
    failtest("cstr[0] %c != 'a'\n", cstr_get(&v, 0));
  if (cstr_get(&v, 1) != 'b')
    failtest("cstr[1] %c != 'b'\n", cstr_get(&v, 1));
  if (cstr_get(&v, 2) != 'c')
    failtest("cstr[2] %c != 'c'\n", cstr_get(&v, 2));

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_7()
{
  TRACE_ENTER;
  struct cstr_t v, w;
  cstr_initstr(&v, "this is a test");
  cstr_initstr(&w, "this is a test");

  if (strcmp(cstr_getbufptr(&v), "this is a test") != 0)
    failtest("v != 'this is a test'\n");
  if (cstr_compare(&v, &w) != 0)
    failtest("v != w\n");

  cstr_destroy(&v);
  cstr_destroy(&w);
  TRACE_EXIT;
}


void test_cstr_8()
{
  TRACE_ENTER;
  struct cstr_t v, w;
  cstr_initstr(&v, "this is a test");
  cstr_initstr(&w, "this is a test2");

  int j = cstr_mismatch(&v, &w);
  if (j != 14)
    failtest("mismatch not found at 14\n");
  if (cstr_compare(&v, &w) == 0)
    failtest("v == w\n");
  if (cstr_compare(&v, &w) > 0)
    failtest("v > w\n");

  cstr_destroy(&v);
  cstr_destroy(&w);
  TRACE_EXIT;
}


void test_cstr_9()
{
  TRACE_ENTER;
  struct cstr_t v, w;
  cstr_initstr(&v, "this is a test");
  cstr_initstr(&w, "this is a test2");

  cstr_assign(&v, &w);
  if (cstr_compare(&v, &w) != 0)
    failtest("v != w\n");

  cstr_destroy(&v);
  cstr_destroy(&w);
  TRACE_EXIT;
}


void test_cstr_10()
{
  TRACE_ENTER;
  struct cstr_t v, w;
  cstr_initstr(&v, "this is a test");
  cstr_initstrn(&w, "this is a test2", 14);

  if (cstr_compare(&v, &w) != 0)
    failtest("v != w\n");

  cstr_destroy(&v);
  cstr_destroy(&w);
  TRACE_EXIT;
}


void test_cstr_11()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_initstr(&v, "this is a test");

  cstr_upper(&v, 0, cstr_count(&v));
  if (strcmp(cstr_getbufptr(&v), "THIS IS A TEST") != 0)
    failtest("v is not uppercased\n");

  cstr_lower(&v, 0, cstr_count(&v));
  if (strcmp(cstr_getbufptr(&v), "this is a test") != 0)
    failtest("v is not lowercased\n");

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_12()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_initstr(&v, "this is a");

  cstr_append(&v, ' ');
  if (strcmp(cstr_getbufptr(&v), "this is a ") != 0)
    failtest("v is missing trailing space\n");

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_13()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_initstr(&v, "this is a");

  cstr_appendct(&v, ' ', 2);
  if (strcmp(cstr_getbufptr(&v), "this is a  ") != 0)
    failtest("v is missing trailing spaces\n");

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_14()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_initstr(&v, "this is a");

  cstr_insertct(&v, 0, ' ', 2);
  if (strcmp(cstr_getbufptr(&v), "  this is a") != 0)
    failtest("v is missing leading spaces\n");

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_15()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_initstr(&v, "this is a");

  cstr_clear(&v);
  if (cstr_count(&v) != 0)
    failtest("v is not empty\n");

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_16()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_initstr(&v, "this is a test");

  if (strcmp(cstr_getcharptr(&v, 10), "test") != 0)
    failtest("cstr_getcharptr returned wrong ptr\n");

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_17()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_initstr(&v, "   this is a test");

  int j = cstr_skip_ws(&v, 0);

  if (strcmp(cstr_getcharptr(&v, j), "this is a test") != 0)
    failtest("cstr_skipws failed\n");

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_18()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_initstr(&v, "xyzzy   this is a test");

  int j = cstr_skip_nonws(&v, 0);
  j = cstr_skip_ws(&v, j);

  if (strcmp(cstr_getcharptr(&v, j), "this is a test") != 0)
    failtest("cstr_skipnonws failed\n");

  cstr_destroy(&v);
  TRACE_EXIT;
}


void test_cstr_19()
{
  TRACE_ENTER;
  struct cstr_t v;
  cstr_initstr(&v, "  xyzzy   this is a test  ");

  cstr_trimright(&v, poe_iswhitespace);

  if (strcmp(cstr_getbufptr(&v), "  xyzzy   this is a test") != 0)
    failtest("cstr_trimright failed '%s'\n", cstr_getbufptr(&v));

  cstr_trimleft(&v, poe_iswhitespace);

  if (strcmp(cstr_getbufptr(&v), "xyzzy   this is a test") != 0)
    failtest("cstr_trimleft failed '%s'\n", cstr_getbufptr(&v));

  cstr_destroy(&v);
  TRACE_EXIT;
}


