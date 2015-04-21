
#include <stdlib.h>
#include <stdint.h>

#include "trace.h"
#include "vec.h"
#include "testing.h"
#include "logging.h"


//
// pivec tests
//
void test_pivec_1()
{
  TRACE_ENTER;
  struct pivec_t v;
  pivec_init(&v, 1);
  if (pivec_capacity(&v) != 1)
	failtest("capacity %d != 1", pivec_capacity(&v));
  if (pivec_count(&v) != 0)
	failtest("count %d != 0", pivec_count(&v));
  
  pivec_destroy(&v);
  TRACE_EXIT;
}


void test_pivec_2()
{
  TRACE_ENTER;
  struct pivec_t v;
  pivec_init(&v, 1);
  
  pivec_append(&v, 0);
  
  if (pivec_capacity(&v) != 1)
	failtest("capacity %d != 1", pivec_capacity(&v));
  if (pivec_count(&v) != 1)
	failtest("count %d != 1", pivec_count(&v));
  
  pivec_destroy(&v);
  TRACE_EXIT;
}


void test_pivec_3()
{
  TRACE_ENTER;
  struct pivec_t v;
  pivec_init(&v, 1);
  
  pivec_append(&v, (intptr_t)NULL);
  pivec_append(&v, (intptr_t)NULL);
  
  if (pivec_capacity(&v) != 2)
	failtest("capacity %d != 2", pivec_capacity(&v));
  if (pivec_count(&v) != 2)
	failtest("count %d != 2", pivec_count(&v));
  
  pivec_destroy(&v);
  TRACE_EXIT;
}


void test_pivec_4()
{
  TRACE_ENTER;
  struct pivec_t v;
  pivec_init(&v, 1);
  
  pivec_append(&v, (intptr_t)1);
  pivec_append(&v, (intptr_t)2);
  pivec_append(&v, (intptr_t)3);
  
  if (pivec_capacity(&v) != 4)
	failtest("capacity %d != 2", pivec_capacity(&v));
  if (pivec_count(&v) != 3)
	failtest("count %d != 3", pivec_count(&v));
  if (pivec_get(&v, 0) != (intptr_t)1)
	failtest("vec[0] %d != 1", pivec_get(&v, 0));
  if (pivec_get(&v, 1) != (intptr_t)2)
	failtest("vec[1] %d != 2", pivec_get(&v, 1));
  if (pivec_get(&v, 2) != (intptr_t)3)
	failtest("vec[2] %d != 3", pivec_get(&v, 2));
  
  pivec_destroy(&v);
  TRACE_EXIT;
}


void test_pivec_5()
{
  TRACE_ENTER;
  struct pivec_t v;
  pivec_init(&v, 1);
  
  pivec_append(&v, (intptr_t)1);
  pivec_append(&v, (intptr_t)2);
  pivec_append(&v, (intptr_t)3);
  pivec_remove(&v, 1);
  
  if (pivec_capacity(&v) != 4) {
	failtest("capacity = %d\n", pivec_capacity(&v));
  }
  if (pivec_count(&v) != 2) {
	failtest("count = %d != 2", pivec_count(&v));
  }
  if (pivec_get(&v, 0) != (intptr_t)1) {
	failtest("vec[0] = %d != 1", pivec_get(&v, 0));
  }
  if (pivec_get(&v, 1) != (intptr_t)3) {
	failtest("vec[1] = %d != 3", pivec_get(&v, 1));
  }
  
  pivec_destroy(&v);
  TRACE_EXIT;
}


void test_pivec_6()
{
  TRACE_ENTER;
  struct pivec_t v;
  pivec_init(&v, 1);
  
  pivec_insert(&v, 0, (intptr_t)3);
  pivec_insert(&v, 0, (intptr_t)2);
  pivec_insert(&v, 0, (intptr_t)1);
  
  if (pivec_capacity(&v) != 4) {
	failtest("capacity %d != 2", pivec_capacity(&v));
  }
  if (pivec_count(&v) != 3) {
	failtest("count = %d != 3", pivec_count(&v));
  }
  if (pivec_get(&v, 0) != (intptr_t)1) {
	failtest("vec[0] = %d != 1", pivec_get(&v, 0));
  }
  if (pivec_get(&v, 1) != (intptr_t)2) {
	failtest("vec[1] = %d != 2", pivec_get(&v, 1));
  }
  if (pivec_get(&v, 2) != (intptr_t)3) {
	failtest("vec[2] = %d != 3", pivec_get(&v, 2));
  }
  
  pivec_destroy(&v);
  TRACE_EXIT;
}


void test_pivec_7()
{
  TRACE_ENTER;
  struct pivec_t v;
  pivec_init(&v, 1);
  
  intptr_t a[3] = {1, 2, 3};
  pivec_appendm(&v, 3, a);
  
  if (pivec_capacity(&v) != 4) {
	failtest("capacity = %d != 2", pivec_capacity(&v));
  }
  if (pivec_count(&v) != 3) {
	failtest("count = %d != 3", pivec_count(&v));
  }
  if (pivec_get(&v, 0) != (intptr_t)1) {
	failtest("vec[0] = %d != 1", pivec_get(&v, 0));
  }
  if (pivec_get(&v, 1) != (intptr_t)2) {
	failtest("vec[1] %d != 2", pivec_get(&v, 1));
  }
  if (pivec_get(&v, 2) != (intptr_t)3) {
	failtest("vec[2] = %d != 3", pivec_get(&v, 2));
  }
  
  pivec_destroy(&v);
  TRACE_EXIT;
}


void test_pivec_8()
{
  TRACE_ENTER;
  struct pivec_t v;
  pivec_init(&v, 1);
  
  intptr_t a[3] = {1, 2, 3};
  pivec_appendm(&v, 3, a);
  pivec_removem(&v, 1, 1);
  
  if (pivec_capacity(&v) != 4) {
	failtest("capacity = %d != 2", pivec_capacity(&v));
  }
  if (pivec_count(&v) != 2) {
	failtest("count = %d != 2", pivec_count(&v));
  }
  if (pivec_get(&v, 0) != (intptr_t)1) {
	failtest("vec[0] = %d != 1", pivec_get(&v, 0));
  }
  if (pivec_get(&v, 1) != (intptr_t)3) {
	failtest("vec[1] = %d != 3", pivec_get(&v, 1));
  }
  
  pivec_destroy(&v);
  TRACE_EXIT;
}


void test_pivec_9()
{
  TRACE_ENTER;
  struct pivec_t v;
  pivec_init(&v, 1);
  
  intptr_t a[3] = {1, 2, 3};
  //printf("test_pivec_9 cap a %d\n", pivec_capacity(&v));
  pivec_appendm(&v, 3, a);
  //printf("test_pivec_9 cap b %d\n", pivec_capacity(&v));
  pivec_removem(&v, 0, 2);
  //printf("test_pivec_9 cap c %d\n", pivec_capacity(&v));
  pivec_removem(&v, 0, 1);
  //printf("test_pivec_9 cap d %d\n", pivec_capacity(&v));

  pivec_insertm(&v, 0, 3, a);
  //printf("test_pivec_9 cap e %d\n", pivec_capacity(&v));
  
  if (pivec_capacity(&v) != 4) {
	failtest("capacity = %d != 4", pivec_capacity(&v));
  }
  if (pivec_count(&v) != 3) {
	failtest("count = %d != 3", pivec_count(&v));
  }
  if (pivec_get(&v, 0) != (intptr_t)1) {
	failtest("vec[0] = %d != 1", pivec_get(&v, 0));
  }
  if (pivec_get(&v, 1) != (intptr_t)2) {
	failtest("vec[1] = %d != 2", pivec_get(&v, 1));
  }
  if (pivec_get(&v, 2) != (intptr_t)3) {
	failtest("vec[2] = %d != 3", pivec_get(&v, 2));
  }
  
  pivec_destroy(&v);
  TRACE_EXIT;
}




//
// vec tests
//
struct XXX {
  int i;
};

struct XXX mkXXX(int i) {
  struct XXX v;
  v.i = i;
  return v;
}

void test_vec_1()
{
  struct vec_t v;
  vec_init(&v, 1, sizeof(struct XXX));
  if (vec_capacity(&v) != 1) {
	failtest("capacity %d != 1", vec_capacity(&v));
  }
  if (vec_count(&v) != 0) {
	failtest("count %d != 0", vec_count(&v));
  }
  vec_destroy(&v);
}


void test_vec_2()
{
  struct vec_t v;
  vec_init(&v, 1, sizeof(struct XXX));
  
  struct XXX a0 = mkXXX(0);
  vec_append(&v, &a0);
  
  if (vec_capacity(&v) != 1) {
	failtest("capacity %d != 1", vec_capacity(&v));
  }
  if (vec_count(&v) != 1) {
	failtest("count %d != 1", vec_count(&v));
  }
  vec_destroy(&v);
}


void test_vec_3()
{
  struct vec_t v;
  vec_init(&v, 1, sizeof(struct XXX));
  
  struct XXX a0 = mkXXX(0);
  vec_append(&v, &a0);
  vec_append(&v, &a0);
  
  if (vec_capacity(&v) != 2) {
	failtest("capacity %d != 2", vec_capacity(&v));
  }
  if (vec_count(&v) != 2) {
	failtest("count %d != 2", vec_count(&v));
  }
  vec_destroy(&v);
}


void test_vec_4()
{
  struct vec_t v;
  vec_init(&v, 1, sizeof(struct XXX));
  
  struct XXX a1 = mkXXX(1);
  struct XXX a2 = mkXXX(2);
  struct XXX a3 = mkXXX(3);
  vec_append(&v, &a1);
  vec_append(&v, &a2);
  vec_append(&v, &a3);
  
  if (vec_capacity(&v) != 4) {
	failtest("capacity %d != 2", vec_capacity(&v));
  }
  if (vec_count(&v) != 3) {
	failtest("count %d != 3", vec_count(&v));
  }
  struct XXX* b1 = (struct XXX*)vec_get(&v, 0);
  if (b1->i != 1) {
	failtest("vec[0] %d != 1", b1->i);
  }
  struct XXX* b2 = (struct XXX*)vec_get(&v, 1);
  if (b2->i != 2) {
	failtest("vec[1] %d != 2", b2->i);
  }
  struct XXX* b3 = (struct XXX*)vec_get(&v, 2);
  if (b3->i != 3) {
	failtest("vec[2] %d != 3", b3->i);
  }
  
  vec_destroy(&v);
}


void test_vec_5()
{
  struct vec_t v;
  vec_init(&v, 1, sizeof(struct XXX));
  
  struct XXX a1 = mkXXX(1);
  struct XXX a2 = mkXXX(2);
  struct XXX a3 = mkXXX(3);
  vec_append(&v, &a1);
  vec_append(&v, &a2);
  vec_append(&v, &a3);
  vec_remove(&v, 1);
  
  if (vec_capacity(&v) != 4) {
	failtest("capacity %d != 2", vec_capacity(&v));
  }
  if (vec_count(&v) != 2) {
	failtest("count %d != 2", vec_count(&v));
  }
  struct XXX* b1 = (struct XXX*)vec_get(&v, 0);
  if (b1->i != 1) {
	failtest("vec[0] %d != 1", b1->i);
  }
  struct XXX* b3 = (struct XXX*)vec_get(&v, 1);
  if (b3->i != 3) {
	failtest("vec[1] %d != 3", b3->i);
  }
  
  vec_destroy(&v);
}


void test_vec_6()
{
  struct vec_t v;
  vec_init(&v, 1, sizeof(struct XXX));
  
  struct XXX a1 = mkXXX(1);
  struct XXX a2 = mkXXX(2);
  struct XXX a3 = mkXXX(3);
  vec_insert(&v, 0, &a3);
  vec_insert(&v, 0, &a2);
  vec_insert(&v, 0, &a1);
  
  if (vec_capacity(&v) != 4) {
	failtest("capacity %d != 2", vec_capacity(&v));
  }
  if (vec_count(&v) != 3) {
	failtest("count %d != 3", vec_count(&v));
  }
  struct XXX* b1 = (struct XXX*)vec_get(&v, 0);
  if (b1->i != 1) {
	failtest("vec[0] %d != 1", b1->i);
  }
  struct XXX* b2 = (struct XXX*)vec_get(&v, 1);
  if (b2->i != 2) {
	failtest("vec[1] %d != 2", b2->i);
  }
  struct XXX* b3 = (struct XXX*)vec_get(&v, 2);
  if (b3->i != 3) {
	failtest("vec[2] %d != 3", b3->i);
  }
  
  vec_destroy(&v);
}


void test_vec_7()
{
  struct vec_t v;
  vec_init(&v, 1, sizeof(struct XXX));
  
  struct XXX a[3] = {{1}, {2}, {3}};
  vec_appendm(&v, 3, a);
  
  if (vec_capacity(&v) != 4) {
	failtest("capacity %d != 2", vec_capacity(&v));
  }
  if (vec_count(&v) != 3) {
	failtest("count %d != 3", vec_count(&v));
  }
  struct XXX* b1 = (struct XXX*)vec_get(&v, 0);
  if (b1->i != 1) {
	failtest("vec[0] %d != 1", b1->i);
  }
  struct XXX* b2 = (struct XXX*)vec_get(&v, 1);
  if (b2->i != 2) {
	failtest("vec[1] %d != 2", b2->i);
  }
  struct XXX* b3 = (struct XXX*)vec_get(&v, 2);
  if (b3->i != 3) {
	failtest("vec[2] %d != 3", b3->i);
  }
  
  vec_destroy(&v);
}


void test_vec_8()
{
  struct vec_t v;
  vec_init(&v, 1, sizeof(struct XXX));
  
  struct XXX a[3] = {{1}, {2}, {3}};
  vec_appendm(&v, 3, a);
  vec_removem(&v, 1, 1);
  
  if (vec_capacity(&v) != 4) {
	failtest("capacity %d != 2", vec_capacity(&v));
  }
  if (vec_count(&v) != 2) {
	failtest("count %d != 2", vec_count(&v));
  }
  struct XXX* b1 = (struct XXX*)vec_get(&v, 0);
  if (b1->i != 1) {
	failtest("vec[0] %d != 1", b1->i);
  }
  struct XXX* b3 = (struct XXX*)vec_get(&v, 1);
  if (b3->i != 3) {
	failtest("vec[1] %d != 3", b3->i);
  }
  
  vec_destroy(&v);
}


void test_vec_9()
{
  struct vec_t v;
  vec_init(&v, 1, sizeof(struct XXX));
  
  struct XXX a[3] = {{1}, {2}, {3}};
  //printf("test_vec_9 cap a %d %d\n", vec_count(&v), vec_capacity(&v));
  vec_appendm(&v, 3, a);
  //printf("test_vec_9 cap b %d %d\n", vec_count(&v), vec_capacity(&v));
  vec_removem(&v, 0, 2);
  //printf("test_vec_9 cap c %d %d\n", vec_count(&v), vec_capacity(&v));
  vec_removem(&v, 0, 1);
  //printf("test_vec_9 cap d %d %d\n", vec_count(&v), vec_capacity(&v));
  vec_insertm(&v, 0, 3, a);
  //printf("test_vec_9 cap e %d %d\n", vec_count(&v), vec_capacity(&v));
  
  if (vec_capacity(&v) != 4) {
	failtest("capacity %d != 4", vec_capacity(&v));
  }
  if (vec_count(&v) != 3) {
	failtest("count %d != 3", vec_count(&v));
  }
  struct XXX* b1 = (struct XXX*)vec_get(&v, 0);
  if (b1->i != 1) {
	failtest("vec[0] %d != 1", b1->i);
  }
  struct XXX* b2 = (struct XXX*)vec_get(&v, 1);
  if (b2->i != 2) {
	failtest("vec[1] %d != 2", b2->i);
  }
  struct XXX* b3 = (struct XXX*)vec_get(&v, 2);
  if (b3->i != 3) {
	failtest("vec[2] %d != 3", b3->i);
  }
  
  vec_destroy(&v);
}

