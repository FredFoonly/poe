
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
#include "markstack.h" 
#include "key_interp.h"
#include "tabstops.h"
#include "buffer.h"

#include "testing.h"


void test_markstack_1()
{
  TRACE_ENTER;
  markstack_cur_unmark();
  TRACE_EXIT;
}


void test_markstack_2()
{
  TRACE_ENTER;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0);
  BUFFER w = BUFFER_NULL;
  enum marktype typ;
  int l1, c1, l2, c2;

  markstack_cur_unmark();

  POE_ERR err;
  err = markstack_cur_place(Marktype_Block, v, 10, 20);
  if (err != POE_ERR_OK)
    failtest("error from markstack_cur_mark_block/1");
  err = markstack_cur_place(Marktype_Block, v, 15, 30);
  if (err != POE_ERR_OK)
    failtest("error from markstack_cur_mark_block/2");


  // check the mark
  err = markstack_cur_get_buffer(&w);
  if (err != POE_ERR_OK)
    failtest("error from markstack_get_buffer");
  if (w != v)
    failtest("markstack_get_buffer returned the wrong buffer handle");

  err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    failtest("error from markstack_cur_mark");

  if (typ != Marktype_Block)
    failtest("got wrong type, expected %d got %d", Marktype_Block, typ);
  if (l1 != 10)
    failtest("got wrong l1, expected %d got %d", 10, typ);
  if (c1 != 20)
    failtest("got wrong c1, expected %d got %d", 20, typ);
  if (l2 != 15)
    failtest("got wrong l2, expected %d got %d", 15, typ);
  if (c2 != 30)
    failtest("got wrong c2, expected %d got %d", 30, typ);

  // attempt to pop it
  err = markstack_pop();
  if (err != POE_ERR_NO_MARKS_SAVED)
    failtest("should have gotten an error popping last mark from stack");

  // verify that this did nothing (can't pop top mark)
  err = markstack_cur_get_type(&typ);
  if (err != POE_ERR_OK)
    failtest("couldn't get top mark type");
  if (typ != Marktype_Block)
    failtest("top mark wasn't block type");

  if (marks_count() != 1)
    failtest("still have %d marks after popping them all\n", marks_count());

  buffer_free(v);
  if (buffers_count() != 0)
    failtest("still have %d buffers after freeing them all\n", buffers_count());

  TRACE_EXIT;
}


void test_markstack_3()
{
  TRACE_ENTER;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0);
  //BUFFER w = BUFFER_NULL;

  markstack_cur_unmark();
  //POE_ERR err;
  markstack_cur_place(Marktype_Block, v, 10, 20);
  markstack_cur_place(Marktype_Block, v, 15, 30);

  markstack_pop();

  markstack_pop();

  if (marks_count() != 1)
    failtest("still have %d marks after popping them all\n", marks_count());

  buffer_free(v);

  if (buffers_count() != 0)
    failtest("still have %d buffers after freeing them all\n", buffers_count());

  TRACE_EXIT;
}


void test_markstack_4()
{
  TRACE_ENTER;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0);
  BUFFER w = BUFFER_NULL;
  POE_ERR err;
  enum marktype typ;
  int l1, c1, l2, c2;

  markstack_cur_unmark();

  // mark a block
  markstack_cur_place(Marktype_Block, v, 15, 30);
  markstack_cur_place(Marktype_Block, v, 10, 20);

  // push
  markstack_push();

  // mark a char region
  err = markstack_cur_place(Marktype_Char, v, 5, 6);
  if (err != POE_ERR_OK)
    failtest("error from markstack_cur_char/2");
  err = markstack_cur_place(Marktype_Char, v, 1, 2);
  if (err != POE_ERR_OK)
    failtest("error from markstack_cur_char/1");


  // make sure the char mark is ok
  err = markstack_cur_get_buffer(&w);
  if (err != POE_ERR_OK)
    failtest("error from markstack_get_buffer");
  if (w != v)
    failtest("markstack_get_buffer returned the wrong buffer handle");

  err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    failtest("error from markstack_cur_mark");

  if (typ != Marktype_Char)
    failtest("got wrong type, expected %d got %d", Marktype_Block, typ);
  if (l1 != 1)
    failtest("got wrong l1, expected %d got %d", 1, typ);
  if (c1 != 2)
    failtest("got wrong c1, expected %d got %d", 2, typ);
  if (l2 != 5)
    failtest("got wrong l2, expected %d got %d", 5, typ);
  if (c2 != 6)
    failtest("got wrong c2, expected %d got %d", 6, typ);


  // pop the char mark
  err = markstack_pop();
  if (err != POE_ERR_OK)
    failtest("couldn't pop mark from stack");

  
  // make sure the mark is still our first (block) mark
  err = markstack_cur_get_buffer(&w);
  if (err != POE_ERR_OK)
    failtest("error from markstack_get_buffer");
  if (w != v)
    failtest("markstack_get_buffer returned the wrong buffer handle");

  err = markstack_cur_get_bounds(&typ, &l1, &c1, &l2, &c2);
  if (err != POE_ERR_OK)
    failtest("error from markstack_cur_mark");

  if (typ != Marktype_Block)
    failtest("got wrong type, expected %d got %d", Marktype_Block, typ);
  if (l1 != 10)
    failtest("got wrong l1, expected %d got %d", 10, typ);
  if (c1 != 20)
    failtest("got wrong c1, expected %d got %d", 20, typ);
  if (l2 != 15)
    failtest("got wrong l2, expected %d got %d", 15, typ);
  if (c2 != 30)
    failtest("got wrong c2, expected %d got %d", 30, typ);


  // pop the block mark
  err = markstack_pop();
  if (err != POE_ERR_NO_MARKS_SAVED)
    failtest("should have gotten an error popping last mark from stack");


  if (marks_count() != 1)
    failtest("still have %d marks after popping them all\n", marks_count());

  buffer_free(v);

  if (buffers_count() != 0)
    failtest("still have %d buffers after freeing them all\n", buffers_count());

  TRACE_EXIT;
}


