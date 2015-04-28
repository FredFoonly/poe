
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/param.h>
#ifdef BSD
#include <libgen.h>
#endif
#include <limits.h>

#include "trace.h"
#include "utils.h"
#include "testing.h"
#include "logging.h"
#include "poe_err.h"
#include "poe_exit.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "tabstops.h"
#include "margins.h"
#include "mark.h"
#include "markstack.h"
#include "key_interp.h"
#include "buffer.h"
#include "editor_globals.h"

#include "test_vec.h"
#include "test_cstr.h"
#include "test_tabstops.h"
#include "test_mark.h"
#include "test_markstack.h"
#include "test_buffer.h"


void test_test_1();
void test_test_2();
void test_test_3();

int main(int argc, char** argv)
{
  init_trace_stack();
  TRACE_ENTER;

  ensure_poe_dir();

  init_logging(LOG_LEVEL_ERR);

  init_marks();
  init_markstack();
  init_buffer();
  default_profile = alloc_profile("testing");
  
  /* tabs_init(&default_tabstops, 0, 8, NULL); */
  /* margins_init(&default_margins, 0, 79, 4); */

  clock_t starttime = clock();
  int i;
  int testing_test = 0;
  int verbose = 0;
  int reps = 1;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--test") == 0)
      testing_test = 1;
    if (strcmp(argv[i], "-v") == 0)
      verbose = 1;
    if (strcmp(argv[i], "-n") == 0)
      reps = atoi(argv[i+1]);
  }

  for (i = 0; i < reps; ++i) {
    if (testing_test) {
      // most of these are supposed to fail for various interesting reasons
      start_testing("pe", 1);
      runtest(test_test_1);
      runtest(test_test_2);
      runtest(test_test_3);
    }
    else {
      start_testing("pe", verbose);
      runtest(test_pivec_1);
      runtest(test_pivec_2);
      runtest(test_pivec_3);
      runtest(test_pivec_4);
      runtest(test_pivec_5);
      runtest(test_pivec_6);
      runtest(test_pivec_7);
      runtest(test_pivec_8);
      runtest(test_pivec_9);

      /* runtest(test_ivec_1); */
      /* runtest(test_ivec_2); */
      /* runtest(test_ivec_3); */
      /* runtest(test_ivec_4); */
      /* runtest(test_ivec_5); */
      /* runtest(test_ivec_6); */
      /* runtest(test_ivec_7); */
      /* runtest(test_ivec_8); */
      /* runtest(test_ivec_9); */

      /* runtest(test_pvec_1); */
      /* runtest(test_pvec_2); */
      /* runtest(test_pvec_3); */
      /* runtest(test_pvec_4); */
      /* runtest(test_pvec_5); */
      /* runtest(test_pvec_6); */
      /* runtest(test_pvec_7); */
      /* runtest(test_pvec_8); */
      /* runtest(test_pvec_9); */

      runtest(test_cstr_1);
      runtest(test_cstr_2);
      runtest(test_cstr_3);
      runtest(test_cstr_4);
      runtest(test_cstr_5);
      runtest(test_cstr_6);
      runtest(test_cstr_7);
      runtest(test_cstr_8);
      runtest(test_cstr_9);
      runtest(test_cstr_10);
      runtest(test_cstr_11);
      runtest(test_cstr_12);
      runtest(test_cstr_13);
      runtest(test_cstr_14);
      runtest(test_cstr_15);
      runtest(test_cstr_16);
      runtest(test_cstr_17);
      runtest(test_cstr_18);
      runtest(test_cstr_19);


      runtest(test_vec_1);
      runtest(test_vec_2);
      runtest(test_vec_3);
      runtest(test_vec_4);
      runtest(test_vec_5);
      runtest(test_vec_6);
      runtest(test_vec_7);
      runtest(test_vec_8);
      runtest(test_vec_9);


      runtest(test_tabstops_1);
      runtest(test_tabstops_2);
      runtest(test_tabstops_3);
      runtest(test_tabstops_4);
      runtest(test_tabstops_5);
      runtest(test_tabstops_6);

      runtest(test_mark_1);
      runtest(test_mark_2);
      runtest(test_mark_3);
      runtest(test_mark_4);
      runtest(test_mark_5);
      runtest(test_mark_6);
      runtest(test_mark_7);
      runtest(test_mark_8);
      runtest(test_mark_9);
      runtest(test_mark_10);
      runtest(test_mark_11);
      runtest(test_mark_12);
      runtest(test_mark_13);
      runtest(test_mark_14);
      runtest(test_mark_15);
      runtest(test_mark_16);
      runtest(test_mark_17);
      runtest(test_mark_18);
      runtest(test_mark_19);
      runtest(test_mark_20);
      runtest(test_mark_21);
      runtest(test_mark_22);
      runtest(test_mark_23);
      runtest(test_mark_24);
      runtest(test_mark_25);
      runtest(test_mark_26);
      runtest(test_mark_27);
      runtest(test_mark_28);
      runtest(test_mark_29);
      runtest(test_mark_30);
      runtest(test_mark_31);
      runtest(test_mark_32);
      runtest(test_mark_33);
      runtest(test_mark_34);
      runtest(test_mark_35);
      runtest(test_mark_36);
      runtest(test_mark_37);
      runtest(test_mark_38);
      runtest(test_mark_39);



      runtest(test_markstack_1);
      runtest(test_markstack_2);
      runtest(test_markstack_3);
      runtest(test_markstack_4);



      runtest(test_buffer_1);
      runtest(test_buffer_2);
      runtest(test_buffer_3);
      runtest(test_buffer_4);
      runtest(test_buffer_5);
      runtest(test_buffer_6);
      runtest(test_buffer_7);
      runtest(test_buffer_8);
      runtest(test_buffer_9);
      runtest(test_buffer_10);
      runtest(test_buffer_11);
      runtest(test_buffer_12);
      runtest(test_buffer_13);
      runtest(test_buffer_14);
      runtest(test_buffer_15);
      runtest(test_buffer_16);
      runtest(test_buffer_17);
      runtest(test_buffer_18);
      runtest(test_buffer_19);
      runtest(test_buffer_20);
      runtest(test_buffer_21);
    }
  }

  int errors = end_testing();
  clock_t endtime = clock();
  clock_t elapsed = endtime - starttime;
  printf("elapsed = %ld.%02ld secs\n", (long int)(elapsed/CLOCKS_PER_SEC), (long int)((elapsed%CLOCKS_PER_SEC)*100)/CLOCKS_PER_SEC);

  free_profile(default_profile);
  shutdown_buffer();
  shutdown_markstack();
  shutdown_marks();
  shutdown_logging();

  TRACE_RETURN(errors);
  // shutdown_trace_stack();
}






//
// meta tests
//
void test_test_1()
{
  TRACE_ENTER;
  TRACE_EXIT;
}

static int __x = 0;
static int __z = 0;
void test_test_2()
{
  TRACE_ENTER;
  __x = 1/__z;
  TRACE_EXIT;
}



void test_test_3()
{
  TRACE_ENTER;
  failtest("testing %d %s", 1, "two");
  TRACE_EXIT;
}
