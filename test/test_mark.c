
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "trace.h"
#include "utils.h"
#include "poe_err.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "mark.h"

#include "testing.h"


/*
 * Test marks
 */

// Test for unfreed marks has to check for > 1, because the mark stack
// keeps one around at all times.



void test_mark_1()
{
  TRACE_ENTER;
  MARK m = mark_alloc(0);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_2()
{
  TRACE_ENTER;
  int err;
  enum marktype typ;
  int l1, c1, l2, c2;
  MARK m = mark_alloc(0);

  err = mark_place(m, Marktype_Block, (BUFFER)0, 1, 2);
  if (err != POE_ERR_OK)
    failtest("mark_start_block error (err = %d)", err);
  err = mark_place(m, Marktype_Block, (BUFFER)0, 5, 6);
  if (err != POE_ERR_OK)
    failtest("mark_extend_block error (err = %d)", err);
  mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (typ != Marktype_Block)
    failtest("mark type != block");
  if (l1 != 1)
    failtest("l1 (%d) != 1", l1);
  if (l2 != 5)
    failtest("l2 (%d) != 5", l2);
  if (c1 != 2)
    failtest("c1 (%d) != 2", c1);
  if (c2 != 6)
    failtest("c2 (%d) != 6", c2);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


// block mark, backwards
void test_mark_3()
{
  TRACE_ENTER;
  int err;
  enum marktype typ;
  int l1, c1, l2, c2;
  MARK m = mark_alloc(0);

  err = mark_place(m, Marktype_Block, (BUFFER)0, 5, 2);
  if (err != POE_ERR_OK)
    failtest("mark_start_block error (err = %d)", err);
  err = mark_place(m, Marktype_Block, (BUFFER)0, 1, 6);
  if (err != POE_ERR_OK)
    failtest("mark_extend_block error (err = %d)", err);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    failtest("mark_get_bounds failed err = %d", err);
  if (typ != Marktype_Block)
    failtest("mark type != block");
  if (l1 != 1)
    failtest("l1 (%d) != 1", l1);
  if (c1 != 2)
    failtest("c1 (%d) != 2", c1);
  if (l2 != 5)
    failtest("l2 (%d) != 5", l2);
  if (c2 != 6)
    failtest("c2 (%d) != 6", c2);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


// line mark, forwards
void test_mark_4()
{
  TRACE_ENTER;
  int err;
  enum marktype typ;
  int l1, c1, l2, c2;
  MARK m = mark_alloc(0);

  err = mark_place(m, Marktype_Line, (BUFFER)0, 1, 2);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)0, 5, 6);
  if (err != 0)
    failtest("mark_extend_line error (err = %d)", err);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    failtest("mark_get_bounds failed err = %d", err);
  if (typ != Marktype_Line)
    failtest("mark type != line");
  if (l1 != 1)
    failtest("l1 (%d) != 1", l1);
  if (c1 != 2)
    failtest("c1 (%d) != 2", c1);
  if (l2 != 5)
    failtest("l2 (%d) != 5", l2);
  if (c2 != 6)
    failtest("c2 (%d) != 6", c2);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


// line mark, backwards
void test_mark_5()
{
  TRACE_ENTER;
  int err;
  enum marktype typ;
  int l1, c1, l2, c2;
  MARK m = mark_alloc(0);

  err = mark_place(m, Marktype_Line, (BUFFER)0, 5, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)0, 1, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line error (err = %d)", err);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    failtest("mark_get_bounds failed err = %d", err);
  if (typ != Marktype_Line)
    failtest("mark type != line");
  if (l1 != 1)
    failtest("l1 (%d) != 1", l1);
  if (c1 != 2)
    failtest("c1 (%d) != 2", c1);
  if (l2 != 5)
    failtest("l2 (%d) != 5", l2);
  if (c2 != 6)
    failtest("c2 (%d) != 6", c2);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


// char mark, forwards
void test_mark_6()
{
  TRACE_ENTER;
  int err;
  enum marktype typ;
  int l1, c1, l2, c2;
  MARK m = mark_alloc(0);

  err = mark_place(m, Marktype_Char, (BUFFER)0, 1, 2);
  if (err != POE_ERR_OK)
    failtest("mark_start_char error (err = %d)", err);
  err = mark_place(m, Marktype_Char, (BUFFER)0, 5, 6);
  if (err != 0)
    failtest("mark_extend_char error (err = %d)", err);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    failtest("mark_get_bounds failed err = %d", err);
  if (typ != Marktype_Char)
    failtest("mark type != char");
  if (l1 != 1)
    failtest("l1 (%d) != 1", l1);
  if (c1 != 2)
    failtest("c1 (%d) != 2", c1);
  if (l2 != 5)
    failtest("l2 (%d) != 5", l2);
  if (c2 != 6)
    failtest("c2 (%d) != 6", c2);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


// char mark, backwards
void test_mark_7()
{
  TRACE_ENTER;
  int err;
  enum marktype typ;
  int l1, c1, l2, c2;
  MARK m = mark_alloc(0);

  err = mark_place(m, Marktype_Char, (BUFFER)0, 5, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_char error (err = %d)", err);
  err = mark_place(m, Marktype_Char, (BUFFER)0, 1, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_char error (err = %d)", err);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    failtest("mark_get_bounds failed err = %d", err);
  if (typ != Marktype_Char)
    failtest("mark type != char");
  if (l1 != 1)
    failtest("l1 (%d) != 1", l1);
  if (c1 != 2)
    failtest("c1 (%d) != 2", c1);
  if (l2 != 5)
    failtest("l2 (%d) != 5", l2);
  if (c2 != 6)
    failtest("c2 (%d) != 6", c2);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


// mixed marks should give error
void test_mark_8()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);

  err = mark_place(m, Marktype_Char, (BUFFER)0, 5, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_char error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)0, 1, 2);
  if (err == POE_ERR_OK)
    failtest("mark_extend_line failed to return a wrong marktype error");

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


// mixed buffers should give error
void test_mark_9()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);

  err = mark_place(m, Marktype_Char, (BUFFER)0, 5, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_char error (err = %d)", err);
  err = mark_place(m, Marktype_Char, (BUFFER)1, 1, 2);
  if (err != POE_ERR_MARKED_BLOCK_EXISTS)
    failtest("mark_extend_char failed to return a wrong buffer error (err = %d)", err);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_10()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Line, (BUFFER)1, 5, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)1, 1, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  marks_upd_insertedlines((BUFFER)0, 0, 1);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 1 || c1 != 2 || l2 != 5 || c2 != 6)
    failtest("mark_upd_insertedlines/1 moved the mark for the wrong buffer (%d %d) (%d %d)",
             l1, 1, l2, 5);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_11()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Line, (BUFFER)1, 5, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)1, 1, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  marks_upd_insertedlines((BUFFER)1, 0, 1);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 2 || c1 != 2 || l2 != 6 || c2 != 6)
    failtest("mark_upd_insertedlines/2 didn't move the mark correctly (%d %d) (%d %d)",
             l1, 2, l2, 6);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_12()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Line, (BUFFER)1, 2, 2);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)1, 6, 6);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  marks_upd_insertedlines((BUFFER)1, 10, 10);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 2 || c1 != 2 || l2 != 6 || c2 != 6)
    failtest("mark_upd_insertedlines/3 moved the mark but shouldn't have (%d %d) (%d %d)",
             l1, 2, l2, 6);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_13()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Line, (BUFFER)1, 2, 2);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)1, 6, 6);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  marks_upd_insertedlines((BUFFER)1, 3, 1);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 2 || c1 != 2 || l2 != 7 || c2 != 6)
    failtest("mark_upd_insertedlines/3 didn't move the mark correctly (%d vs %d) (%d vs %d)",
             l1, 2, l2, 7);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}



void test_mark_14()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Line, (BUFFER)1, 25, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)1, 11, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  // update for a delete in a different buffer
  marks_upd_removedlines((BUFFER)0, 0, 1);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 11 || c1 != 2 || l2 != 25 || c2 != 6)
    failtest("mark_upd_removedlines/1 moved the mark for the wrong buffer (%d %d) (%d %d)",
             l1, 1, l2, 5);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_15()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Line, (BUFFER)1, 25, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)1, 11, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  // update for a delete before the mark
  marks_upd_removedlines((BUFFER)1, 0, 1);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 10 || c1 != 2 || l2 != 24 || c2 != 6)
    failtest("mark_upd_removedlines/2 didn't move the mark correctly (%d %d) (%d %d)",
             l1, 10, l2, 24);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_16()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Line, (BUFFER)1, 10, 2);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)1, 24, 6);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  // update for a delete after the mark
  marks_upd_removedlines((BUFFER)1, 30, 10);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 10 || c1 != 2 || l2 != 24 || c2 != 6)
    failtest("mark_upd_removedlines/3 moved the mark but shouldn't have (%d %d) (%d %d)",
             l1, 10, l2, 24);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_17()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Line, (BUFFER)1, 10, 2);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)1, 24, 6);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  // update for a delete crossing into the mark
  marks_upd_removedlines((BUFFER)1, 9, 5);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 9 || c1 != 2 || l2 != 19 || c2 != 6)
    failtest("mark_upd_removedlines/4 didn't move the mark correctly (%d vs %d) (%d vs %d)",
             l1, 9, l2, 19);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_18()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Line, (BUFFER)1, 9, 2);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Line, (BUFFER)1, 19, 6);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  // update for a delete crossing out of the mark
  marks_upd_removedlines((BUFFER)1, 17, 5);
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 9 || c1 != 2 || l2 != 17 || c2 != 6)
    failtest("mark_upd_removedlines/5 didn't move the mark correctly (%d vs %d) (%d vs %d)",
             l1, 9, l2, 17);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_19()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Char, (BUFFER)1, 5, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Char, (BUFFER)1, 1, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  // should wind up with a mark (1,2) (5,6)

  // test wrong buffer
  marks_upd_insertedcharblk((BUFFER)0, 0, 3, 5, 6, 5);
  // no-op -> (1,2) (5,6)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 1 || c1 != 2 || l2 != 5 || c2 != 6)
    failtest("mark_upd_removedchars/1 moved the mark for the wrong buffer (%d %d) (%d %d)",
             l1, 1, l2, 5);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_20()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Char, (BUFFER)1, 5, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Char, (BUFFER)1, 1, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  // should wind up with a mark (1,2) (5,6)

  // test entire insertion on previous lines
  marks_upd_insertedcharblk((BUFFER)1, 0, 3, 5, 5, 5);
  // -> (6,2) (10,6)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 6 || c1 != 2 || l2 != 10 || c2 != 6)
    failtest("mark_upd_insertedchars/2 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 6, c1, 2, l2, 10, c2, 6);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_21()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test insertion starting on the mark line before mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("3: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_insertedcharblk((BUFFER)1, 2, 2, 5, 5, 5);
  // -> (7,9) (7,14)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("3: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 7 || c1 != 8 || l2 != 7 || c2 != 13)
    failtest("mark_upd_insertedchars/3 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 7, c1, 8, l2, 7, c2, 13);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_22()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test insertion starting on the mark line inside mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("4: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_insertedcharblk((BUFFER)1, 2, 7, 5, 5, 5);
  // -> (2,5) (7,8)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("4: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 5 || l2 != 7 || c2 != 8)
    failtest("mark_upd_insertedchars/4 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 5, l2, 7, c2, 8);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_23()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test insertion starting on the mark line beyond mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("5: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_insertedcharblk((BUFFER)1, 2, 11, 5, 5, 5);
  // -> (2,5) (7,8)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("5: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 5 || l2 != 2 || c2 != 10)
    failtest("mark_upd_insertedchars/5 moved the mark when it shouldn't (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 5, l2, 2, c2, 10);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_24()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test insertion starting on the first mark line before start col
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 3, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("6: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_insertedcharblk((BUFFER)1, 2, 2, 5, 5, 5);
  // -> (7,8) (8,9)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("6: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 7 || c1 != 8 || l2 != 8 || c2 != 10)
    failtest("mark_upd_insertedchars/6 did't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 7, c1, 8, l2, 8, c2, 10);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_25()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test insertion starting on the first mark line after start col
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 3, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("7: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_insertedcharblk((BUFFER)1, 2, 7, 5, 5, 5);
  // -> (2,5) (8,9)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("7: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 5 || l2 != 8 || c2 != 10)
    failtest("mark_upd_insertedchars/7 didn't move the mark correctly(%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 5, l2, 8, c2, 10);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_26()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test insertion starting on the last mark line inside mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 3, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("8: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_insertedcharblk((BUFFER)1, 3, 6, 5, 5, 5);
  // -> (2,5) (9,9)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("8: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 5 || l2 != 8 || c2 != 9)
    failtest("mark_upd_insertedchars/8 did't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 5, l2, 8, c2, 9);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_27()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test insertion starting on the end mark line after end col
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 3, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("9: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_insertedcharblk((BUFFER)1, 3, 11, 5, 5, 5);
  // -> (2,5) (9,9)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("9: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 5 || l2 != 3 || c2 != 10)
    failtest("mark_upd_insertedchars/9 moved the mark when it shouldn't (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 5, l2, 3, c2, 10);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_28()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Char, (BUFFER)1, 15, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Char, (BUFFER)1, 10, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  // should wind up with a mark (1,2) (5,6)

  // test wrong buffer
  marks_upd_removedcharblk((BUFFER)0, 0, 3, 5, 6, 5);
  // no-op -> (1,2) (5,6)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  if (l1 != 10 || c1 != 2 || l2 != 15 || c2 != 6)
    failtest("mark_upd_removedchars/1 moved the mark for the wrong buffer (%d %d) (%d %d)",
             l1, 10, c1, 2, l2, 15, c2, 6);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_29()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  err = mark_place(m, Marktype_Char, (BUFFER)1, 15, 6);
  if (err != POE_ERR_OK)
    failtest("mark_start_line error (err = %d)", err);
  err = mark_place(m, Marktype_Char, (BUFFER)1, 10, 2);
  if (err != POE_ERR_OK)
    failtest("mark_extend_line  error (err = %d)", err);

  // should wind up with a mark (1,2) (5,6)

  // test entire deletion on previous lines
  marks_upd_removedcharblk((BUFFER)1, 0, 3, 5, 5, 5);
  // -> (5,2) (10,6)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (l1 != 5 || c1 != 2 || l2 != 10 || c2 != 6)
    failtest("mark_upd_removedchars/2 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 6, c1, 2, l2, 10, c2, 6);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


//
// single-line deletions
//

void test_mark_30()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion starting on the mark line before mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("3: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 2, 2, 0, 2, 0);
  // -> (2,3) (2,8)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("3: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 3 || l2 != 2 || c2 != 8)
    failtest("mark_upd_removedchars/3 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 3, l2, 2, c2, 8);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_31()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion starting on the mark line inside mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("4: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 2, 6, 0, 2, 0);
  // -> (2,5) (2,8)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("4: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 5 || l2 != 2 || c2 != 8)
    failtest("mark_upd_removedchars/4 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 5, l2, 2, c2, 8);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_32()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion starting on the mark line after mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("5: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 2, 12, 0, 2, 0);
  // -> (2,5) (2,10)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("5: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 5 || l2 != 2 || c2 != 10)
    failtest("mark_upd_removedchars/5 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 5, l2, 2, c2, 10);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_33()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion starting on the mark line enveloping mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err/1 not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 10);
  if (err != POE_ERR_OK) failtest("err/2 not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err/3 not ok");
  //printf("6: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 2, 3, 0, 10, 0);
  // -> unmarked
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  //printf("6: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (err == POE_ERR_OK || typ != Marktype_None)
    failtest("mark_upd_removedchars/6 didn't unmark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 0, c1, 0, l2, 0, c2, 0);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_34()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion starting on the mark line crossing into mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("7: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 2, 3, 0, 5, 0);
  // -> (2,3) (2,5)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("7: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 3 || l2 != 2 || c2 != 5)
    failtest("mark_upd_removedchars/7 didn't unmark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 3, l2, 2, c2, 5);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_35()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion starting on the mark line crossing out of mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 2, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("8: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 2, 8, 0, 5, 0);
  // -> (2,5) (2,8)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("8: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 2 || c1 != 5 || l2 != 2 || c2 != 8)
    failtest("mark_upd_removedchars/8 didn't unmark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 2, c1, 5, l2, 2, c2, 8);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


//
// multi-line deletions
//

void test_mark_36()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion ending on the line before mark start line
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 5, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 10, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("9: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 2, 2, 2, 2, 2);
  // -> (3,5) (8,10)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("9: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 3 || c1 != 5 || l2 != 8 || c2 != 10)
    failtest("mark_upd_removedchars/9 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 3, c1, 5, l2, 8, c2, 10);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_37()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion ending on the mark start line, with a 2-line mark
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 5, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 6, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("10: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 3, 25, 2, 2, 2);
  // -> (3,28) (4,10)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("10: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 3 || c1 != 28 || l2 != 4 || c2 != 10)
    failtest("mark_upd_removedchars/10 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 3, c1, 28, l2, 4, c2, 10);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_38()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion starting and ending between the mark lines.
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 5, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 10, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("11: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 6, 25, 2, 2, 2);
  // -> (5,5) (8,10)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("11: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 5 || c1 != 5 || l2 != 8 || c2 != 10)
    failtest("mark_upd_removedchars/11 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 5, c1, 5, l2, 8, c2, 10);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}


void test_mark_39()
{
  TRACE_ENTER;
  int err;
  MARK m = mark_alloc(0);
  enum marktype typ;
  int l1, l2, c1, c2;

  // test deletion starting on the last mark line.
  // reset marks
  err = mark_place(m, Marktype_Char, (BUFFER)1, 5, 5);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_place(m, Marktype_Char, (BUFFER)1, 10, 10);
  if (err != POE_ERR_OK) failtest("err not ok");
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("12: pre mark  = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  // insert
  marks_upd_removedcharblk((BUFFER)1, 10, 3, 5, 30, 2);
  // -> (5,5) (10,3) // pe2 gets this wrong - it's end mark column doesn't change - ends on (10,10)
  err = mark_get_bounds(m, &typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK) failtest("err not ok");
  //printf("12: post mark = (%d %d) (%d %d)\n", l1, c1, l2, c2);
  if (l1 != 5 || c1 != 5 || l2 != 10 || c2 != 3)
    failtest("mark_upd_removedchars/12 didn't move the mark correctly (%d/%d %d/%d) (%d/%d %d/%d)",
             l1, 5, c1, 5, l2, 10, c2, 3);

  mark_unmark(m);
  mark_free(m);
  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  TRACE_EXIT;
}
