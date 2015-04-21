
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>

#include "trace.h"
#include "poe_exit.h"
#include "utils.h"
#include "cstr.h"
#include "logging.h"


void _cstr_realloc(struct cstr_t* v, size_t newcap);


//
// strings
//

struct cstr_t* cstr_alloc(int capacity)
{
  TRACE_ENTER;
  struct cstr_t* v = calloc(1, sizeof(struct cstr_t));
  cstr_init(v, capacity);
  TRACE_RETURN(v);
}


struct cstr_t* cstr_allocstr(const char* s)
{
  TRACE_ENTER;
  struct cstr_t* v = calloc(1, sizeof(struct cstr_t));
  cstr_initstr(v, s);
  TRACE_RETURN(v);
}


void cstr_init(struct cstr_t* v, int capacity)
{
  TRACE_ENTER;
  if (capacity < 0)
    capacity = 0;
  if (capacity == 0)
    v->elts = NULL;
  else
    v->elts = calloc(capacity, sizeof(char));
  v->ct = 0;
  v->cap = capacity;
  TRACE_EXIT;
}


void cstr_initfrom(struct cstr_t* dst, const struct cstr_t* src)
{
  TRACE_ENTER;
  cstr_init(dst, src->cap);
  if (src->ct > 0)
    memcpy(dst->elts, src->elts, src->ct * sizeof(char));
  dst->ct = src->ct;
  TRACE_EXIT;
}


void cstr_initfromn(struct cstr_t* dst, const struct cstr_t* src, int i, int n)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i+n > src->ct)
    poe_err(1, "cstr_initfromn %d/%d", i+n, src->ct);
#endif
  cstr_init(dst, n);
  cstr_initstrn(dst, src->elts+i, n);
  TRACE_EXIT;
}


void cstr_initstr(struct cstr_t* dst, const char* s)
{
  TRACE_ENTER;
  int l = strlen(s);
  cstr_init(dst, l+1);
  cstr_appendm(dst, l, s);
  dst->ct = l;
  TRACE_EXIT;
}


void cstr_initstrn(struct cstr_t* dst, const char* s, int n)
{
  TRACE_ENTER;
  int l = strnlen(s, n);
  cstr_init(dst, l+1);
  cstr_appendm(dst, l, s);
  dst->ct = l;
  TRACE_EXIT;
}


void cstr_free(struct cstr_t* v)
{
  TRACE_ENTER;
  cstr_destroy(v);
  free(v);
  TRACE_EXIT;
}


void cstr_destroy(struct cstr_t* v)
{
  TRACE_ENTER;
  if (v->elts != NULL)
    free(v->elts);
  v->elts = NULL;
  v->ct = 0;
  v->cap = 0;
  TRACE_EXIT;
}


void cstr_assign(struct cstr_t* dst, struct cstr_t* src)
{
  TRACE_ENTER;
  cstr_clear(dst);
  cstr_appendm(dst, src->ct, src->elts);
  TRACE_EXIT;
}


void cstr_assignn(struct cstr_t* dst, struct cstr_t* src, int i, int n)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i+n > src->ct)
    poe_err(1, "cstr_assignn %d/%d", i+n, src->ct);
#endif
  cstr_clear(dst);
  cstr_appendm(dst, n, src->elts+i);
  TRACE_EXIT;
}


void cstr_assignstr(struct cstr_t* dst, const char* src)
{
  TRACE_ENTER;
  cstr_clear(dst);
  cstr_appendm(dst, strlen(src), src);
  TRACE_EXIT;
}


void cstr_assignstrn(struct cstr_t* dst, const char* src, int n)
{
  TRACE_ENTER;
  cstr_clear(dst);
  int l = strnlen(src, n);
  cstr_appendm(dst, l, src);
  TRACE_EXIT;
}


int cstr_count(const struct cstr_t* v)
{
  TRACE_ENTER;
  int rval = v->ct;
  TRACE_RETURN(rval);
}


int cstr_capacity(const struct cstr_t* v)
{
  TRACE_ENTER;
  int rval = v->cap;
  TRACE_RETURN(rval); 
}


char cstr_get(const struct cstr_t* v, int i)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "cstr_get %d/%d", i, v->ct);
#endif
  char rval = v->elts[i];
  TRACE_RETURN(rval);
}


const char* cstr_getbufptr(const struct cstr_t* v)
{
  TRACE_ENTER;
  const char* rval = v->elts;
  TRACE_RETURN(rval);
}


const char* cstr_getcharptr(const struct cstr_t* v, int i)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "cstr_getcharptr %d/%d", i, v->ct);
#endif
  const char* rval = v->elts + i;
  TRACE_RETURN(rval);
}


void cstr_set(struct cstr_t* v, int i, char a)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "cstr_set %d/%d", i, v->ct);
#endif
  v->elts[i] = a;
  TRACE_EXIT;
}


void cstr_setct(struct cstr_t* v, int i, char a, int n)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i+n > v->ct)
    poe_err(1, "cstr_setct %d/%d", i+n, v->ct);
#endif
  int j;
  for (j = 0; j < n; j++)
    v->elts[i+j] = a;
  TRACE_EXIT;
}


void cstr_setstrn(struct cstr_t* v, int i, const char* s, int n)
{
  TRACE_ENTER;
  int j;
#ifdef DPOE_DBG_LIM
  if (i+n > v->ct)
    poe_err(1, "cstr_setstrn %d/%d", i, v->ct);
#endif
  for (j = 0; j < n && s[j] != '\0'; j++)
    v->elts[i+j] = s[j];
  TRACE_EXIT;
}


void cstr_upper(struct cstr_t* v, int i, int n)
{
  TRACE_ENTER;
  if (i >= v->ct)
    TRACE_EXIT;
  int j;
  for (j = i; j < i+n && j < v->ct; j++) {
    char c = v->elts[j];
    if (islower(c))
      v->elts[j] = toupper(c);
  }
  TRACE_EXIT;
}


void cstr_lower(struct cstr_t* v, int i, int n)
{
  TRACE_ENTER;
  if (i >= v->ct)
    TRACE_EXIT;
  int j;
  for (j = i; j < i+n && j < v->ct; j++) {
    char c = v->elts[j];
    if (isupper(c))
      v->elts[j] = tolower(c);
  }
  TRACE_EXIT;
}


void cstr_append(struct cstr_t* v, char a)
{
  TRACE_ENTER;
  if (v->ct + 1 > v->cap-1) {
    v->cap = max(1, v->cap);
    v->cap <<= 1;
    _cstr_realloc(v, v->cap);
  }
  v->elts[v->ct] = a;
  v->ct++;
  TRACE_EXIT;
}


void cstr_appendct(struct cstr_t* v, char a, int n)
{
  TRACE_ENTER;
  if (n <= 0)
    TRACE_EXIT;
  if (v->ct + n > v->cap - 1) {
    int tot = v->ct + n;
    v->cap = max(1, v->cap);
    while (tot > v->cap)
      v->cap <<= 1;
    _cstr_realloc(v, v->cap);
  }
  memset(v->elts+v->ct, a, n*sizeof(char));
  v->ct += n;
  TRACE_EXIT;
}


/* void cstr_appendto(struct cstr_t* v, char c, int i) */
/* { */
/*   TRACE_ENTER; */
/*   if (v->ct < i) */
/*  cstr_appendct(v, i-v->ct, c); */
/*   TRACE_EXIT; */
/* } */


void cstr_appendcstr(struct cstr_t* v, struct cstr_t* u)
{
  TRACE_ENTER;
  cstr_appendm(v, cstr_count(u), cstr_getbufptr(u));
  TRACE_EXIT;
}


void cstr_appendstr(struct cstr_t* v, const char* s)
{
  TRACE_ENTER;
  cstr_appendm(v, strlen(s), s);
  TRACE_EXIT;
}


void cstr_appendf(struct cstr_t* v, const char* fmt, ...)
{
  TRACE_ENTER;
  char tmp[1024];

  va_list ap;
  va_start(ap, fmt);
  vsnprintf(tmp, sizeof(tmp), fmt, ap);
  cstr_appendstr(v, tmp);
  va_end(ap);

  TRACE_EXIT;
}


void cstr_insert(struct cstr_t* v, int i, char a)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i > v->ct)
    poe_err(1, "cstr_insert %d/%d", i, v->ct);
#endif
  if (v->ct + 1 > v->cap - 1) {
    v->cap = max(1, v->cap);
    v->cap <<= 1;
    _cstr_realloc(v, v->cap);
  }
  if (i < v->ct)
    memmove(v->elts+i+1, v->elts+i, (v->ct-i)*sizeof(char));
  v->elts[i] = a;
  v->ct++;
  TRACE_EXIT;
}


void cstr_insertct(struct cstr_t* v, int i, char a, int n)
{
  TRACE_ENTER;
  if (n < 0)
    TRACE_EXIT;
#ifdef DPOE_DBG_LIM
  if (i > v->ct)
    poe_err(1, "cstr_insertct %d/%d", i, v->ct);
#endif
  if (v->ct + n >= v->cap) {
    int tot = v->ct + n;
    v->cap = max(1, v->cap);
    while (tot >= v->cap)
      v->cap <<= 1;
    _cstr_realloc(v, v->cap);
  }
  if (i <= v->ct) {
    memmove(v->elts+i+n, v->elts+i, (v->ct-i)*sizeof(char));
	memset(v->elts+i, a, n*sizeof(char));
	v->ct += n;
	v->elts[v->ct] = '\0';
  }
  TRACE_EXIT;
}


void cstr_remove(struct cstr_t* v, int i)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i >= v->ct)
    poe_err(1, "cstr_remove %d/%d", i, v->ct);
#endif
  if (i <= v->ct - 1) {
    memmove(v->elts+i, v->elts+i+1, (v->ct-i)*sizeof(char));
	v->ct--;
	v->elts[v->ct] = '\0';
  }
  TRACE_EXIT;
}


void cstr_appendm(struct cstr_t* v, int n, const char* a)
{
  TRACE_ENTER;
  if (v->ct + n > v->cap - 1) {
    int tot = v->ct + n;
    v->cap = max(1, v->cap);
    while (tot > v->cap)
      v->cap <<= 1;
    _cstr_realloc(v, v->cap);
  }
  memcpy(v->elts+v->ct, a, n*sizeof(char));
  v->ct += n;
  TRACE_EXIT;
}


void cstr_insertm(struct cstr_t* v, int i, int n, const char* a)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i > v->ct)
    poe_err(1, "cstr_insertm %d/%d", i, v->ct);
#endif
  if (v->ct + n > v->cap - 1) {
    int tot = v->ct + n;
    v->cap = max(1, v->cap);
    while (tot >= v->cap)
      v->cap <<= 1;
    _cstr_realloc(v, v->cap);
  }
  if (i <= v->ct) {
    memmove(v->elts+i+n, v->elts+i, (v->ct-i)*sizeof(char));
	memcpy(v->elts+i, a, n*sizeof(char));
	v->ct += n;
	v->elts[v->ct] = '\0';
  }
  TRACE_EXIT;
}


void cstr_removem(struct cstr_t* v, int i, int n)
{
  TRACE_ENTER;
#ifdef DPOE_DBG_LIM
  if (i+n > v->ct)
    poe_err(1, "cstr_removem %d/%d", i+n, v->ct);
#endif
  if (i <= v->ct - n) {
    memmove(v->elts+i, v->elts+i+n, (v->ct-i)*sizeof(char));
	memset(v->elts+v->ct-n, 0, n*sizeof(char));
	v->ct -= n;
  }
  TRACE_EXIT;
}


void cstr_clear(struct cstr_t* v)
{
  TRACE_ENTER;
  if (v->cap > 0)
    memset(v->elts, 0, v->ct);
  v->ct = 0;
  TRACE_EXIT;
}


int cstr_mismatch(const struct cstr_t* a, const struct cstr_t* b)
{
  TRACE_ENTER;
  int i;
  int la = cstr_count(a);
  int lb = cstr_count(b);
  int l = min(la, lb);
  for (i = 0; i < l; i++) {
    if (cstr_get(a, i) != cstr_get(b, i))
      TRACE_RETURN(i);
  }
  if (i == la && i == lb)
    TRACE_RETURN(CSTR_NO_MISMATCH)
  else
    TRACE_RETURN(i)
}


int cstr_compare(const struct cstr_t* a, const struct cstr_t* b)
{
  TRACE_ENTER;
  int rval = cstr_comparestriat(a, 0, b->elts, cstr_count(b));
  TRACE_RETURN(rval);
}


int cstr_comparei(const struct cstr_t* a, const struct cstr_t* b)
{
  TRACE_ENTER;
  int rval = cstr_comparestriat(a, 0, b->elts, cstr_count(b));
  TRACE_RETURN(rval);
}


int cstr_comparestr(const struct cstr_t* a, const const char* b)
{
  TRACE_ENTER;
  int rval = cstr_comparestrat(a, 0, b, strlen(b));
  TRACE_RETURN(rval);
}


int cstr_comparestri(const struct cstr_t* a, const const char* b)
{
  TRACE_ENTER;
  int rval = cstr_comparestriat(a, 0, b, strlen(b));
  TRACE_RETURN(rval);
}


int cstr_comparestrat(const struct cstr_t* a, int offset, const char* b, int nchars)
{
  TRACE_ENTER;
  int i;
  int la = cstr_count(a);
  const char* sa = a->elts;
  int lb = strlen(b);
  for (i = 0; i+offset < la && i<lb; i++) {
    char ca = sa[i+offset];
    char cb = b[i];
    if (ca != cb)
      TRACE_RETURN(ca - cb);
  }
  if (i+offset == la && i == lb)
    TRACE_RETURN(0)
  else if (la < lb)
    TRACE_RETURN(-1)
  else
    TRACE_RETURN(1)
}


int cstr_comparestriat(const struct cstr_t* a, int offset, const char* b, int nchars)
{
  TRACE_ENTER;
  int i;
  int la = cstr_count(a);
  const char* sa = a->elts;
  int lb = strlen(b);
  for (i = 0; i+offset < la && i<lb; i++) {
    char ca = sa[i+offset];
    char cb = b[i];
    if (ca != cb) {
      if (isalpha(ca) && isalpha(cb)) {
        if (toupper(ca) != toupper(cb))
          TRACE_RETURN(toupper(ca) - toupper(cb));
      }
      else {
        TRACE_RETURN(ca - cb);
      }
    }
  }
  if (i+offset == la && i == lb)
    TRACE_RETURN(0)
  else if (la < lb)
    TRACE_RETURN(-1)
  else
    TRACE_RETURN(1)
}


int cstr_skip_ws(const struct cstr_t* str, int i)
{
  TRACE_ENTER;
  int j = cstr_skipwhile(str, i, poe_iswhitespace);
  TRACE_RETURN(j);
}


int cstr_skip_nonws(const struct cstr_t* str, int i)
{
  TRACE_ENTER;
  int j = cstr_skiptill(str, i, poe_iswhitespace);
  TRACE_RETURN(j);
}


int cstr_skipwhile(const struct cstr_t* str, int i, char_pred_t whilepred)
{
  TRACE_ENTER;
  int l = cstr_count(str);
  int j;
  for (j = i; j < l; j++) {
    char c = cstr_get(str, j);
    if (!(*whilepred)(c))
      TRACE_RETURN(j);
  }
  TRACE_RETURN(j);
}


int cstr_skiptill(const struct cstr_t* str, int i, char_pred_t tillpred)
{
  TRACE_ENTER;
  int l = cstr_count(str);
  int j;
  for (j = i; j < l; j++) {
    char c = cstr_get(str, j);
    if ((*tillpred)(c))
      TRACE_RETURN(j);
  }
  TRACE_RETURN(j);
}


int cstr_skiptill_chr(const struct cstr_t* str, int i, char tillchar)
{
  TRACE_ENTER;
  int l = cstr_count(str);
  int j;
  for (j = i; j < l; j++) {
    char c = cstr_get(str, j);
    if (c == tillchar)
      TRACE_RETURN(j);
  }
  TRACE_RETURN(j);
}


int cstr_trimleft(struct cstr_t* str, char_pred_t spacepred)
{
  TRACE_ENTER;
  int i, len = str->ct;
  char* elts = str->elts;
  for (i = 0; i < len && (*spacepred)(elts[i]); i++)
    ;
  strncpy(str->elts, str->elts+i, len-i+1);
  int nchars_removed = i;
  str->ct -= nchars_removed;
  TRACE_RETURN(nchars_removed);
}


int cstr_trimright(struct cstr_t* str, char_pred_t pred)
{
  TRACE_ENTER;
  int i, len = str->ct;
  char* elts = str->elts;
  for (i = len-1; i >= 0 && (*pred)(elts[i]); i--)
    ;
  elts[i+1] = '\0';
  int nchars_removed = len-i-1;
  str->ct -= nchars_removed;
  TRACE_RETURN(nchars_removed);
}


int cstr_find(const struct cstr_t* str, int i, const struct cstr_t* pat, int direction)
{
  TRACE_ENTER;
  if (pat->ct <= 0 || str->ct <= 0)
    TRACE_RETURN(-1);
  const char* s = str->elts;
  const char* p = pat->elts;
  if (direction > 0) {
    int slen = str->ct;
    i = min(slen, i);
    const char* r = strstr(s+i, p);
    if (r == NULL)
      TRACE_RETURN(-1)
    else
      TRACE_RETURN(r-s)
  }
  else if (direction < 0) {
    int slen = str->ct;
    int plen = pat->ct;
    i = min(slen, i);
    for (i--; i >= 0; i--) {
      if (strncmp(s+i, p, plen) == 0)
        break;
    }
    TRACE_RETURN(i);
  }
  TRACE_RETURN(-1);
}


int cstr_findi(const struct cstr_t* str, int i, const struct cstr_t* pat, int direction)
{
  TRACE_ENTER;
  if (pat->ct <= 0 || str->ct <= 0)
    TRACE_RETURN(-1);
  const char* s = str->elts;
  const char* p = pat->elts;
  if (direction > 0) {
    int slen = str->ct;
    i = min(slen, i);
    const char* r = strcasestr(s+i, p);
    if (r == NULL)
      TRACE_RETURN(-1)
    else
      TRACE_RETURN(r-s)
  }
  else if (direction < 0) {
    int slen = str->ct;
    int plen = pat->ct;
    i = min(slen, i);
    for (i--; i >= 0; i--) {
      if (strncasecmp(s+i, p, plen) == 0)
        break;
    }
    TRACE_RETURN(i);
  }
  TRACE_RETURN(-1);
}


void _cstr_realloc(struct cstr_t* v, size_t newcap)
{
  TRACE_ENTER;
  char* old = (char*)v->elts;
  v->elts = calloc(newcap+1, sizeof(char));
  v->cap = newcap;
  memcpy(v->elts, old, v->ct);
  free(old);
  TRACE_EXIT;
}


