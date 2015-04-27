
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "trace.h"
#include "poe_exit.h"
#include "utils.h"
#include "vec.h"


//
// vector of intptrs
//

struct pivec_t* pivec_alloc(int capacity)
{
  TRACE_ENTER;
  struct pivec_t* v = calloc(1, sizeof(struct pivec_t));
  pivec_init(v, capacity);
  TRACE_RETURN(v);
}


void pivec_init(struct pivec_t* v, int capacity)
{
  TRACE_ENTER;
  if (capacity < 0)
    capacity = 0;
  if (capacity == 0)
    v->elts = NULL;
  else
    v->elts = calloc(capacity, sizeof(intptr_t));
  v->ct = 0;
  v->cap = capacity;
  TRACE_EXIT;
}


void pivec_initfrom(struct pivec_t* dst, const struct pivec_t* src)
{
  TRACE_ENTER;
  pivec_initfromarr(dst, src->elts, src->ct);
  TRACE_EXIT;
}


void pivec_initfromarr(struct pivec_t* dst, const intptr_t* src, size_t n)
{
  TRACE_ENTER;
  pivec_init(dst, max(0,n));
  if (n > 0)
    memcpy(dst->elts, src, n * sizeof(intptr_t));
  dst->ct = n;
  TRACE_EXIT;
}



void pivec_free(struct pivec_t* v)
{
  TRACE_ENTER;
  pivec_destroy(v);
  free(v);
  TRACE_EXIT;
}


void pivec_destroy(struct pivec_t* v)
{
  TRACE_ENTER;
  if (v->elts != NULL)
    free(v->elts);
  v->elts = NULL;
  v->ct = 0;
  v->cap = 0;
  TRACE_EXIT;
}


void pivec_copy(struct pivec_t* dst, const struct pivec_t* src)
{
  TRACE_ENTER;
  pivec_destroy(dst);
  pivec_initfrom(dst, src);
  TRACE_EXIT;
}


void pivec_clear(struct pivec_t* v)
{
  TRACE_ENTER;
  memset(v->elts, 0, v->ct * sizeof(intptr_t));
  v->ct = 0;
  TRACE_EXIT;
}


int pivec_count(const struct pivec_t* v)
{
  TRACE_ENTER;
  int rval = v->ct;
  TRACE_RETURN(rval);
}


int pivec_capacity(const struct pivec_t* v)
{
  TRACE_ENTER;
  int rval = v->cap;
  TRACE_RETURN(rval);
}


intptr_t pivec_get(const struct pivec_t* v, int i)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "pivec_get %d/%d", i, v->ct);
#endif
  intptr_t rval = v->elts[i];
  TRACE_RETURN(rval);
}


void pivec_set(struct pivec_t* v, int i, intptr_t a)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "pivec_set %d/%d", i, v->ct);
#endif
  v->elts[i] = a;
  TRACE_EXIT;
}


int pivec_append(struct pivec_t* v, intptr_t a)
{
  TRACE_ENTER;
  if (v->ct + 1 > v->cap) {
    v->cap = max(1, v->cap);
    v->cap <<= 1;
    v->elts = reallocarray(v->elts, v->cap, sizeof(intptr_t));
  }
  int pos = v->ct;
  v->elts[v->ct] = a;
  v->ct++;
  TRACE_RETURN(pos);
}


void pivec_insert(struct pivec_t* v, int i, intptr_t a)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i > v->ct)
    poe_err(1, "pivec_insert %d/%d", i, v->ct);
#endif
  if (v->ct + 1 >= v->cap) {
    v->cap = max(1, v->cap);
    v->cap <<= 1;
    v->elts = reallocarray(v->elts, v->cap, sizeof(intptr_t));
  }
  if (i <= v->ct) {
    memmove(v->elts+i+1, v->elts+i, (v->ct-i)*sizeof(intptr_t));
	v->elts[i] = a;
	v->ct++;
  }
  TRACE_EXIT;
}


void pivec_remove(struct pivec_t* v, int i)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "pivec_remove %d/%d", i, v->ct);
#endif
  if (i <= v->ct - 1) {
    memmove(v->elts+i, v->elts+i+1, (v->ct-i)*sizeof(intptr_t));
	v->elts[v->ct-1] = (intptr_t)NULL;
	v->ct--;
  }
  TRACE_EXIT;
}


int pivec_appendm(struct pivec_t* v, int n, intptr_t* a)
{
  TRACE_ENTER;
  int pos = v->ct;
  if (n <= 0)
	TRACE_RETURN(pos);
  if (v->ct + n > v->cap) {
    int tot = v->ct + n;
    v->cap = max(1, v->cap);
    while (tot > v->cap)
      v->cap <<= 1;
    v->elts = reallocarray(v->elts, v->cap, sizeof(intptr_t));
  }
  memcpy(v->elts+pos, a, n*sizeof(intptr_t));
  v->ct += n;
  TRACE_RETURN(pos);
}


void pivec_insertm(struct pivec_t* v, int i, int n, intptr_t* a)
{
  TRACE_ENTER;
  if (n <= 0)
	TRACE_EXIT;
#ifdef DPOE_DBG_LIM
  if (i > v->ct)
    poe_err(1, "pivec_insertm %d/%d", i, v->ct);
#endif
  if (v->ct + n >= v->cap) {
    int tot = v->ct + n;
    v->cap = max(1, v->cap);
    while (tot >= v->cap)
      v->cap <<= 1;
    v->elts = reallocarray(v->elts, v->cap, sizeof(intptr_t));
  }
  if (i <= v->ct) {
    memmove(v->elts+i+n, v->elts+i, (v->ct-i)*sizeof(intptr_t));
	memcpy(v->elts+i, a, n*sizeof(intptr_t));
	v->ct += n;
  }
  TRACE_EXIT;
}


void pivec_removem(struct pivec_t* v, int i, int n)
{
  TRACE_ENTER;
  if (n <= 0)
	TRACE_EXIT;
#ifdef DPOE_DBG_LIM
  if (i+n > v->ct)
    poe_err(1, "pivec_removem %d/%d", i+n, v->ct);
#endif
  if (i <= v->ct - n) {
    memmove(v->elts+i, v->elts+i+n, (v->ct-i-n)*sizeof(intptr_t));
	memset(v->elts+v->ct-n, 0, n*sizeof(intptr_t));
	v->ct -= n;
  }
  TRACE_EXIT;
}





//
// vector of structs
//

struct vec_t* vec_alloc(int capacity, size_t element_size)
{
  TRACE_ENTER;
  struct vec_t* v = calloc(1, sizeof(struct vec_t));
  vec_init(v, element_size, capacity);
  TRACE_RETURN(v);
}


void vec_init(struct vec_t* v, int capacity, size_t element_size)
{
  TRACE_ENTER;
  if (capacity < 1)
    capacity = 1;
  v->elts = calloc(capacity, element_size);
  v->ct = 0;
  v->cap = capacity;
  v->eltsize = element_size;
  TRACE_EXIT;
}


void vec_initfrom(struct vec_t* dst, const struct vec_t* src)
{
  TRACE_ENTER;
  vec_init(dst, src->eltsize, src->cap);
  if (src->ct > 0)
    memcpy(dst->elts, src->elts, src->ct*src->eltsize);
  dst->ct = src->ct;
  TRACE_EXIT;
}


void vec_free(struct vec_t* v)
{
  TRACE_ENTER;
  vec_destroy(v);
  free(v);
  TRACE_EXIT;
}


void vec_destroy(struct vec_t* v)
{
  TRACE_ENTER;
  if (v->elts != NULL)
    free(v->elts);
  v->elts = NULL;
  v->ct = 0;
  v->cap = 0;
  v->eltsize = 0;
  TRACE_EXIT;
}


void vec_copy(struct vec_t* dst, const struct vec_t* src)
{
  TRACE_ENTER;
  vec_destroy(dst);
  vec_initfrom(dst, src);
  TRACE_EXIT;
}


void vec_clear(struct vec_t* v)
{
  TRACE_ENTER;
  memset(v->elts, 0, v->ct * v->eltsize);;
  v->ct = 0;
  TRACE_EXIT;
}


int vec_count(const struct vec_t* v)
{
  TRACE_ENTER;
  int rval = v->ct;
  TRACE_RETURN(rval);
}


int vec_capacity(const struct vec_t* v)
{
  TRACE_ENTER;
  int rval = v->cap;
  TRACE_RETURN(rval);
}


void* vec_get(const struct vec_t* v, int i)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "vec_get %d/%d", i, v->ct);
#endif
  void* rval = v->elts + (i * v->eltsize);
  TRACE_RETURN(rval);
}


void* vec_getbufptr(const struct vec_t* v)
{
  TRACE_ENTER;
  void* rval = (void*)v->elts;
  TRACE_RETURN(rval);
}


void vec_set(struct vec_t* v, int i, void* a)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "vec_set %d/%d", i, v->ct);
#endif
  memcpy(v->elts + (v->ct * v->eltsize), a, v->eltsize);
  TRACE_EXIT;
}


int vec_append(struct vec_t* v, void* a)
{
  TRACE_ENTER;
  if (v->ct + 1 > v->cap) {
    v->cap = max(1, v->cap);
    v->cap <<= 1;
    v->elts = reallocarray(v->elts, v->cap, v->eltsize);
  }
  int pos = v->ct;
  memcpy(v->elts + (pos * v->eltsize), a, v->eltsize);
  v->ct++;
  TRACE_RETURN(pos);
}


void vec_insert(struct vec_t* v, int i, void* a)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i > v->ct)
    poe_err(1, "vec_insert %d/%d", i, v->ct);
#endif
  if (v->ct + 1 > v->cap) {
    v->cap = max(1, v->cap);
    v->cap <<= 1;
    v->elts = reallocarray(v->elts, v->cap, v->eltsize);
  }
  if (i <= v->ct) {
    memmove(v->elts + ((i+1) * v->eltsize), v->elts + (i * v->eltsize), (v->ct - i) * v->eltsize);
	memcpy(v->elts + (i * v->eltsize), a, v->eltsize);
	v->ct++;
  }
  TRACE_EXIT;
}


void vec_remove(struct vec_t* v, int i)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "vec_remove %d/%d", i, v->ct);
#endif
  if (i < v->ct - 1) {
    memmove(v->elts+(i*v->eltsize), v->elts+(i+1)*v->eltsize, (v->ct-i)*v->eltsize);
	memset(v->elts + (v->ct-1) * v->eltsize, 0, v->eltsize);
	v->ct--;
  }
  TRACE_EXIT;
}


int vec_appendm(struct vec_t* v, int n, void* a)
{
  TRACE_ENTER;
  int pos = v->ct;
  if (n <= 0)
	TRACE_RETURN(pos);
  if (v->ct + n > v->cap) {
    int tot = v->ct + n;
    while (tot > v->cap)
      v->cap <<= 1;
    v->elts = reallocarray(v->elts, v->cap, v->eltsize);
  }
  memcpy(v->elts + pos*v->eltsize, a, n*v->eltsize);
  v->ct += n;
  TRACE_RETURN(pos);
}


void vec_insertm(struct vec_t* v, int i, int n, void* a)
{
  TRACE_ENTER;
  if (n <= 0)
	TRACE_EXIT;
#ifdef DPOE_DBG_LIM
  if (i > v->ct)
    poe_err(1, "vec_insertm %d/%d", i, v->ct);
#endif
  if (v->ct + n >= v->cap) {
    int tot = v->ct + n;
    v->cap = max(1, v->cap);
    while (tot >= v->cap)
      v->cap <<= 1;
    v->elts = reallocarray(v->elts, v->cap, v->eltsize);
  }
  if (i <= v->ct) {
    memmove(v->elts + (i+n)*v->eltsize, v->elts + i*v->eltsize, (v->ct-i)*v->eltsize);
	memcpy(v->elts + i*v->eltsize, a, n*v->eltsize);
	v->ct += n;
  }
  TRACE_EXIT;
}


void vec_removem(struct vec_t* v, int i, int n)
{
  TRACE_ENTER;
  if (n <= 0)
	TRACE_EXIT;
#ifdef DPOE_DBG_LIM
  if (i+n > v->ct)
    poe_err(1, "vec_removem %d/%d", i+n, v->ct);
#endif
  if (i <= v->ct - n) {
    memmove(v->elts + (i*v->eltsize), v->elts + ((i+n)*v->eltsize), (v->ct-i-n)*v->eltsize);
	memset(v->elts + (v->ct-n)*v->eltsize, 0, n*v->eltsize);
	v->ct -= n;
  }
  TRACE_EXIT;
}

