
#include <stdlib.h>
#include <stdint.h>

#include "trace.h"
#include "utils.h"
#include "vec.h"
#include "cstr.h"
#include "tabstops.h"
#include "testing.h"



void test_tabstops_1()
{
  TRACE_ENTER;
  struct tabstops_t tabs;

  tabs_init(&tabs, 0, 5, NULL);

  int t1 = tabs_next(&tabs, 0);
  int t2 = tabs_next(&tabs, t1);
  int t3 = tabs_next(&tabs, t2);
  int t3_1 = tabs_next(&tabs, t2+1);
  if (t1 != 5)
    failtest("t1 %d != 5\n", t1);
  if (t2 != 10)
    failtest("t2 %d != 10\n", t2);
  if (t3 != 15)
    failtest("t3 %d != 15\n", t3);
  if (t3_1 != 15)
    failtest("t3_1 %d != 15\n", t3_1);

  tabs_destroy(&tabs);
  TRACE_EXIT;
}


void test_tabstops_2()
{
  TRACE_ENTER;
  struct tabstops_t tabs;

  tabs_init(&tabs, 0, 5, NULL);

  int t1 = tabs_prev(&tabs, 5);
  int t2 = tabs_prev(&tabs, 10);
  int t3 = tabs_prev(&tabs, 16);
  int t3_1 = tabs_prev(&tabs, 20);
  if (t1 != 0)
    failtest("t1 %d != 0\n", t1);
  if (t2 != 5)
    failtest("t2 %d != 5\n", t2);
  if (t3 != 15)
    failtest("t3 %d != 15\n", t3);
  if (t3_1 != 15)
    failtest("t3_1 %d != 15\n", t3_1);

  tabs_destroy(&tabs);
  TRACE_EXIT;
}


void test_tabstops_3()
{
  TRACE_ENTER;
  intptr_t _marks[] = {0, 5, 10, 15, 20, 25, 30, 35, 40 ,45, 50, 55, 60, 65, 70, 75 };
  struct pivec_t vmarks;
  struct tabstops_t tabs;
  
  pivec_init(&vmarks, sizeof(_marks)/sizeof(_marks[0]));
  tabs_init(&tabs, 0, 5, &vmarks);

  pivec_appendm(&vmarks, sizeof(_marks)/sizeof(_marks[0]), _marks);
  int t1 = tabs_next(&tabs, 0);
  int t2 = tabs_next(&tabs, t1);
  int t3 = tabs_next(&tabs, t2);
  int t3_1 = tabs_next(&tabs, t2+1);
  if (t1 != 5)
    failtest("t1 %d != 5\n", t1);
  if (t2 != 10)
    failtest("t2 %d != 10\n", t2);
  if (t3 != 15)
    failtest("t3 %d != 15\n", t3);
  if (t3_1 != 15)
    failtest("t3_1 %d != 15\n", t3_1);

  pivec_destroy(&vmarks);
  tabs_destroy(&tabs);
  TRACE_EXIT;
}


void test_tabstops_4()
{
  TRACE_ENTER;
  /* struct cstr_t tabstr; */
  /* struct tabstops_t tabs; */
  /* const char* errstr = NULL; */
  /* int parse_rc; */

  /* cstr_initstr(&tabstr, "1 6 11 16 21 26 31 36 41 46 51 56 61 66 71 76"); */
  /* parse_rc = tabs_init_parse(&tabs, &tabstr, &errstr); */
  /* if (!parse_rc) */
  /*   failtest("tabs_init_parse error %s", errstr); */

  /* int t1 = tabs_prev(&tabs, 5); */
  /* int t2 = tabs_prev(&tabs, 10); */
  /* int t3 = tabs_prev(&tabs, 16); */
  /* int t3_1 = tabs_prev(&tabs, 20); */
  /* if (t1 != 0) */
  /*   failtest("t1 %d != 0\n", t1); */
  /* if (t2 != 5) */
  /*   failtest("t2 %d != 5\n", t2); */
  /* if (t3 != 15) */
  /*   failtest("t3 %d != 15\n", t3); */
  /* if (t3_1 != 15) */
  /*   failtest("t3_1 %d != 15\n", t3_1); */

  /* cstr_destroy(&tabstr); */
  /* tabs_destroy(&tabs); */
  TRACE_EXIT;
}


void test_tabstops_5()
{
  TRACE_ENTER;
  intptr_t _marks[] = {0, 5 };
  struct pivec_t vmarks;
  struct tabstops_t tabs;
  pivec_init(&vmarks, sizeof(_marks)/sizeof(_marks[0]));
  tabs_init(&tabs, 0, 5, &vmarks);

  pivec_appendm(&vmarks, sizeof(_marks)/sizeof(_marks[0]), _marks);
  int t1 = tabs_next(&tabs, 0);
  int t2 = tabs_next(&tabs, t1);
  int t3 = tabs_next(&tabs, t2);
  int t3_1 = tabs_next(&tabs, t2+1);
  if (t1 != 5)
    failtest("t1 %d != 5\n", t1);
  if (t2 != 10)
    failtest("t2 %d != 10\n", t2);
  if (t3 != 15)
    failtest("t3 %d != 15\n", t3);
  if (t3_1 != 15)
    failtest("t3_1 %d != 15\n", t3_1);

  pivec_destroy(&vmarks);
  tabs_destroy(&tabs);
  TRACE_EXIT;
}


void test_tabstops_6()
{
  TRACE_ENTER;
  intptr_t _marks[] = {0, 5};
  struct pivec_t vmarks;
  struct tabstops_t tabs;
  pivec_init(&vmarks, sizeof(_marks)/sizeof(_marks[0]));
  tabs_init(&tabs, 0, 5, &vmarks);

  pivec_appendm(&vmarks, sizeof(_marks)/sizeof(_marks[0]), _marks);
  int t1 = tabs_prev(&tabs, 5);
  int t2 = tabs_prev(&tabs, 10);
  int t3 = tabs_prev(&tabs, 16);
  int t3_1 = tabs_prev(&tabs, 20);
  if (t1 != 0)
    failtest("t1 %d != 0\n", t1);
  if (t2 != 5)
    failtest("t2 %d != 5\n", t2);
  if (t3 != 15)
    failtest("t3 %d != 15\n", t3);
  if (t3_1 != 15)
    failtest("t3_1 %d != 15\n", t3_1);

  pivec_destroy(&vmarks);
  tabs_destroy(&tabs);
  TRACE_EXIT;
}



