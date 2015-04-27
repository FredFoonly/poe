
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/errno.h>
#include <libgen.h>
#include <unistd.h>


#include "trace.h"
#include "utils.h"
#include "poe_err.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "mark.h"
#include "tabstops.h"
#include "margins.h"
#include "key_interp.h"
#include "buffer.h"
#include "editor_globals.h"

#include "testing.h"


// test creation of buffer
void test_buffer_1()
{
  TRACE_ENTER;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  if (buffer_capacity(v) != 1)
    failtest("capacity %d != 1", buffer_capacity(v));
  if (buffer_count(v) != 0)
    failtest("count %d != 0", buffer_count(v));

  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());

  TRACE_EXIT;
}


// test appending single line
void test_buffer_2()
{
  TRACE_ENTER;
  struct line_t l;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;

  buffer_appendline(v, &l);

  if (buffer_capacity(v) != 1)
    failtest("capacity %d != 1", buffer_capacity(v));
  if (buffer_count(v) != 1)
    failtest("count %d != 1", buffer_count(v));

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test appending two lines
void test_buffer_3()
{
  TRACE_ENTER;
  struct line_t l;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  if (buffer_capacity(v) != 2)
    failtest("capacity %d != 2", buffer_capacity(v));
  if (buffer_count(v) != 2)
    failtest("count %d != 2", buffer_count(v));

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test appending three lines
void test_buffer_4()
{
  TRACE_ENTER;
  struct line_t l;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "As is this");
  buffer_appendline(v, &l);

  if (buffer_capacity(v) != 4)
    failtest("capacity %d != 4", buffer_capacity(v));
  if (buffer_count(v) != 3)
    failtest("count %d != 3", buffer_count(v));

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test get line
void test_buffer_5()
{
  TRACE_ENTER;
  struct line_t l;
  struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "As is this");
  buffer_appendline(v, &l);

  pl = buffer_get(v, 0);
  if (cstr_comparestr(&pl->txt, "This is a test") != 0)
    failtest("line 0 != 'This is a test'\n");
  pl = buffer_get(v, 1);
  if (cstr_comparestr(&pl->txt, "This is also a test") != 0)
    failtest("line 0 != 'This is also a test'\n");
  pl = buffer_get(v, 2);
  if (cstr_comparestr(&pl->txt, "As is this") != 0)
    failtest("line 0 != 'As is this'\n");

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test overwrite line text
void test_buffer_6()
{
  TRACE_ENTER;
  struct line_t l;
  struct line_t* pl;
  struct cstr_t s;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&s, "This is a test");
  cstr_initstr(&l.txt, "This is a test");

  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "As is this");
  buffer_appendline(v, &l);

  buffer_setcstr(v, 1, &s);
  
  pl = buffer_get(v, 1);
  if (cstr_comparestr(&pl->txt, "This is a test") != 0)
    failtest("line 1 != 'This is a test'\n");

  cstr_destroy(&l.txt);
  cstr_destroy(&s);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test remove multiple lines
void test_buffer_7()
{
  TRACE_ENTER;
  struct line_t l;
  //  struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "As is this");
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "And this too");
  buffer_appendline(v, &l);

  if (buffer_count(v) != 4)
    failtest("count %d != 4", buffer_count(v));

  buffer_removelines(v, 1, 2, false);
  if (buffer_count(v) != 2)  // this was commented out for some reason
    failtest("count %d != 2", buffer_count(v)); //...

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test insertchar
void test_buffer_8()
{
  TRACE_ENTER;
  struct line_t l;
  //struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  buffer_insert(v, 1, 0, '*', false);
  if (strcmp(buffer_getbufptr(v, 1), "*This is also a test") != 0) {
    failtest("didn't insert '*'");
  }

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test insert C string
void test_buffer_9()
{
  TRACE_ENTER;
  struct line_t l;
  //struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  buffer_insertstrn(v, 1, 0, "*!@", 4, false);
  if (strcmp(buffer_getbufptr(v, 1), "*!@This is also a test") != 0)
    failtest("didn't insert '*!@', -> '%s'", buffer_getbufptr(v, 1));

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test overwrite char
void test_buffer_10()
{
  TRACE_ENTER;
  struct line_t l;
  //struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  buffer_setchar(v, 1, 0, '*');
  if (strcmp(buffer_getbufptr(v, 1), "*his is also a test") != 0)
    failtest("didn't set '*'");

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test overwrite C string
void test_buffer_11()
{
  TRACE_ENTER;
  struct line_t l;
  //struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  // setstrn should stop at the null, even though we said to use it
  buffer_setstrn(v, 1, 0, "*!@", 4, false);
  if (strcmp(buffer_getbufptr(v, 1), "*!@s is also a test") != 0)
    failtest("didn't set '*!@', -> '%s'", buffer_getbufptr(v, 1));

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test remove char
void test_buffer_12()
{
  TRACE_ENTER;
  struct line_t l;
  //struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  buffer_removechar(v, 1, 0, false);
  if (strcmp(buffer_getbufptr(v, 1), "his is also a test") != 0)
    failtest("didn't remove char");

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test remove multiple chars on a line
void test_buffer_13()
{
  TRACE_ENTER;
  struct line_t l;
  //struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  buffer_removechars(v, 1, 0, 3, false);
  if (strcmp(buffer_getbufptr(v, 1), "s is also a test") != 0)
    failtest("didn't remove chars");

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test uppercase chars
void test_buffer_14()
{
  TRACE_ENTER;
  struct line_t l;
  //struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  buffer_upperchars(v, 1, 0, 4);
  if (strcmp(buffer_getbufptr(v, 1), "THIS is also a test") != 0)
    failtest("didn't uppercase");

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test lowercase chars
void test_buffer_15()
{
  TRACE_ENTER;
  struct line_t l;
  //struct line_t* pl;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  buffer_lowerchars(v, 1, 0, 4);
  if (strcmp(buffer_getbufptr(v, 1), "this is also a test") != 0)
    failtest("didn't lowercase");

  cstr_destroy(&l.txt);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test copyinsertlines
void test_buffer_16()
{
  TRACE_ENTER;
  struct line_t l;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);
  BUFFER u = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);

  cstr_initstr(&l.txt, "This is a test");
  l.flags = 0;
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "This is also a test");
  buffer_appendline(v, &l);

  cstr_assignstr(&l.txt, "As is this");
  buffer_appendline(u, &l);

  cstr_assignstr(&l.txt, "And this too");
  buffer_appendline(u, &l);

  if (buffer_count(v) != 2)
    failtest("count v %d != 2", buffer_count(v));

  if (buffer_count(u) != 2)
    failtest("count u %d != 2", buffer_count(u));

  buffer_copyinsertlines(v, 1, u, 0, 2, false);

  if (buffer_count(v) != 4)
    failtest("count v %d != 4", buffer_count(v));
  if (buffer_count(u) != 2)
    failtest("count u %d != 2", buffer_count(u));

  if (strcmp(buffer_getbufptr(v, 0), "This is a test") != 0)
    failtest("first line didn't copy correctly\n");
  if (strcmp(buffer_getbufptr(v, 1), "As is this") != 0)
    failtest("second line didn't copy correctly\n");
  if (strcmp(buffer_getbufptr(v, 2), "And this too") != 0)
    failtest("first line didn't copy correctly\n");
  if (strcmp(buffer_getbufptr(v, 3), "This is also a test") != 0)
    failtest("second line didn't copy correctly\n");

  cstr_destroy(&l.txt);
  buffer_free(u);
  buffer_free(v);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test moveinsertlines
// moveinsertlines been removed from buffer.c
void test_buffer_17()
{
  TRACE_ENTER;
  TRACE_EXIT;
}


// test buffer_load
void test_buffer_18()
{
  TRACE_ENTER;

  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);
  cstr t1filename;
  cstr_initstr(&t1filename, "t1.txt");
  POE_ERR err = buffer_load(v, &t1filename, 1);
  if (err != POE_ERR_OK)
    failtest("error %d loading t1_rdonly.txt", err);

  if (strcmp(buffer_name(v), "t1.txt") != 0)
    failtest("t1.txt buffer has name '%s'\n", buffer_name(v));

  const char* master = "Four score and seven years ago our fathers brought forth on this continent";
  const char* s = buffer_getbufptr(v, 0);
  if (strcmp(s, master) != 0) {
    printf("%s: c3\n", __func__);
    failtest("first line didn't load correctly\n");
  }

  master = "                        this is the last line";
  //       |                        this is the last line

  int linect = buffer_count(v);
  const char* lastline = buffer_getbufptr(v, linect-1);
  if (strcmp(lastline, master) != 0) {
    if (strlen(master) != strlen(lastline))
      failtest("last line didn't load correctly, expected len %d got %d\n",
               strlen(master), strlen(lastline));
    int i, n = strlen(lastline);
    for (i = 0; i < n; i++) {
      if (master[i] != lastline[i]) {
        failtest("difference @ %d expected char %c(%d) vs %c(%d)\n",
                 i,
                 master[i], master[i],
                 lastline[i], lastline[i]);
      }
    }
  }

  if (buffer_tstflags(v, BUF_FLG_RDONLY)) {
    failtest("buffer should be writable\n");
  }

  buffer_free(v);
  cstr_destroy(&t1filename);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());

  TRACE_EXIT;
}


// test buffer_load of rdonly file
void test_buffer_19()
{
  TRACE_ENTER;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);
  cstr t1ro_filename;
  cstr_initstr(&t1ro_filename, "t1_rdonly.txt");
  POE_ERR err = buffer_load(v, &t1ro_filename, 1);
  if (err != POE_ERR_OK)
    failtest("error %d loading t1_rdonly.txt", err);

  if (strcmp(buffer_name(v), "t1_rdonly.txt") != 0)
    failtest("t1_rdonly.txt buffer has name '%s'\n", buffer_name(v));
  
  const char* master = "Four score and seven years ago our fathers brought forth on this continent";
  if (strcmp(buffer_getbufptr(v, 0), master) != 0)
    failtest("first line didn't load correctly\n");

  master = "                        this is the last line";
  int linect = buffer_count(v);
  const char* lastline = buffer_getbufptr(v, linect-1);
  if (strcmp(lastline, master) != 0) {
    if (strlen(master) != strlen(lastline))
      failtest("last line didn't load correctly, expected len %d got %d\n",
               strlen(master), strlen(lastline));
    int i, n = strlen(lastline);
    for (i = 0; i < n; i++) {
      if (master[i] != lastline[i]) {
        failtest("difference @ %d expected char %c(%d) vs %c(%d)\n",
                 i,
                 master[i], master[i],
                 lastline[i], lastline[i]);
      }
    }
  }

  if (!buffer_tstflags(v, BUF_FLG_RDONLY)) {
    failtest("buffer t1_rdonly.txt should be readonly\n");
  }

  buffer_free(v);
  cstr_destroy(&t1ro_filename);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test buffer_load of rdonly file, twice
void test_buffer_20()
{
  TRACE_ENTER;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);
  cstr t1ro_filename;
  cstr_initstr(&t1ro_filename, "t1_rdonly.txt");
  POE_ERR err = buffer_load(v, &t1ro_filename, 1);
  if (err != POE_ERR_OK)
    failtest("error %d loading t1_rdonly.txt", err);

  BUFFER w = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);
  err = buffer_load(w, &t1ro_filename, 1);
  if (err != POE_ERR_OK)
    failtest("error %d loading t1_rdonly.txt the second time", err);

  if (strcmp(buffer_name(v), "t1_rdonly.txt") != 0)
    failtest("t1_rdonly.txt buffer has name '%s'\n", buffer_name(v));
  
  if (strcmp(buffer_name(w), "t1_rdonly.txt<2>") != 0)
    failtest("t1_rdonly.txt<2> buffer has name '%s'\n", buffer_name(v));
  
  // check first line
  const char* master = "Four score and seven years ago our fathers brought forth on this continent";
  if (strcmp(buffer_getbufptr(v, 0), master) != 0)
    failtest("first line didn't load correctly\n");

  // check line 27 (leading tabs + trailing tabs after quotation mark)
  master = "                        this is a test line with \"trailing\" tabs                                        ";
  int linect = buffer_count(v);
  const char* line27 = buffer_getbufptr(v, 27);
  if (strcmp(line27, master) != 0) {
    if (strlen(master) != strlen(line27))
      failtest("last line didn't load correctly, expected len %d got %d\n",
               strlen(master), strlen(line27));
    int i, n = strlen(line27);
    for (i = 0; i < n; i++) {
      if (master[i] != line27[i]) {
        failtest("difference @ %d expected char %c(%d) vs %c(%d)\n",
                 i,
                 master[i], master[i],
                 line27[i], line27[i]);
      }
    }
  }

  // check last line
  master = "                        this is the last line";
  linect = buffer_count(v);
  const char* lastline = buffer_getbufptr(v, linect-1);
  if (strcmp(lastline, master) != 0) {
    if (strlen(master) != strlen(lastline))
      failtest("last line didn't load correctly, expected len %d got %d\n",
               strlen(master), strlen(lastline));
    int i, n = strlen(lastline);
    for (i = 0; i < n; i++) {
      if (master[i] != lastline[i]) {
        failtest("difference @ %d expected char %c(%d) vs %c(%d)\n",
                 i,
                 master[i], master[i],
                 lastline[i], lastline[i]);
      }
    }
  }

  if (!buffer_tstflags(v, BUF_FLG_RDONLY)) {
    failtest("buffer t1_rdonly.txt should be readonly\n");
  }

  if (!buffer_tstflags(w, BUF_FLG_RDONLY)) {
    failtest("buffer t1_rdonly.txt should be readonly\n");
  }

  buffer_free(v);
  buffer_free(w);
  cstr_destroy(&t1ro_filename);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


// test buffer_load of rdonly file
void test_buffer_21()
{
  TRACE_ENTER;
  BUFFER v = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);
  cstr t1ro_filename;
  cstr_initstr(&t1ro_filename, "t1_rdonly.txt");
  POE_ERR err = buffer_load(v, &t1ro_filename, 1);
  if (err != POE_ERR_OK)
    failtest("error %d loading t1_rdonly.txt", err);
    
  cstr t1save_filename;
  cstr_initstr(&t1save_filename, "t1_save.txt");
  err = buffer_save(v, &t1save_filename, 1);
  if (err != POE_ERR_OK)
    failtest("error %d saving t1_save.txt", err);

  BUFFER w = buffer_alloc("", BUF_FLG_INTERNAL, 0, default_profile);
  err = buffer_load(w, &t1save_filename, 1);
  if (err != POE_ERR_OK)
    failtest("error %d loading t1_save.txt", err);

  cstr t1save2_filename;
  cstr_initstr(&t1save2_filename, "t1_save2.txt");
  err = buffer_save(w, &t1save2_filename, 1);
  if (err != POE_ERR_OK)
    failtest("error %d saving t1_save2.txt", err);

  // check the line counts
  if (buffer_count(v) != buffer_count(w))
    failtest("t1_rdonly.txt has %d lines, t1_save.txt has %d lines",
             buffer_count(v),
             buffer_count(w));

  // check the line contents
  int i;
  int n = buffer_count(v);
  for (i = 0; i < n; i++) {
    const char* a = buffer_getbufptr(v, i);
    const char* b = buffer_getbufptr(w, i);
    if (strlen(a) != strlen(b))
      failtest("line %d is %d chars long on t1_readonly.txt, but %d chars long on t1_save.txt",
               strlen(a), strlen(b));
    if (strcmp(a, b) != 0)
      failtest("line %d different between t1_readonly.txt and t1_save.txt", i);
  }

  unlink("t1_save.txt");
  unlink("t1_save2.txt");
  buffer_free(v);
  buffer_free(w);
  cstr_destroy(&t1ro_filename);
  cstr_destroy(&t1save_filename);
  cstr_destroy(&t1save2_filename);

  if (marks_count() > 1)
    failtest("%d unfreed marks", marks_count());
  if (buffers_count() != 0)
    failtest("%d unfreed buffers", buffers_count());
  TRACE_EXIT;
}


