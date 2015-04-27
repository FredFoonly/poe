

//
// vector of chars
//
struct cstr_t {
  char* elts;
  int ct;
  int cap;
  size_t eltsize;
};
typedef struct cstr_t cstr;

#define CSTR_NO_MISMATCH (-1)
struct cstr_t* cstr_alloc(int capacity);
struct cstr_t* cstr_allocstr(const char* s);
struct cstr_t* cstr_allocstrn(const char* s, int len);
void cstr_init(struct cstr_t* v, int capacity);
void cstr_initfrom(struct cstr_t* dst, const struct cstr_t* src);
void cstr_initfromn(struct cstr_t* dst, const struct cstr_t* src, int i, int len);
void cstr_initstr(struct cstr_t* dst, const char* s);
void cstr_initstrn(struct cstr_t* cst, const char* src, int len);
void cstr_free(struct cstr_t* v);
void cstr_destroy(struct cstr_t* v);
void cstr_assign(struct cstr_t* dst, struct cstr_t* src);
void cstr_assignn(struct cstr_t* dst, struct cstr_t* src, int i, int n);
void cstr_assignstr(struct cstr_t* cst, const char* src);
void cstr_assignstrn(struct cstr_t* cst, const char* src, int len);
int cstr_count(const struct cstr_t* v);
int cstr_capacity(const struct cstr_t* v);
char cstr_get(const struct cstr_t* v, int i);
const char* cstr_getbufptr(const struct cstr_t* v);
const char* cstr_getcharptr(const struct cstr_t* v, int i);
void cstr_set(struct cstr_t* v, int i, char a);
void cstr_setstrn(struct cstr_t* v, int i, const char* a, int n);
void cstr_setct(struct cstr_t* v, int col, char a, int ct);
void cstr_upper(struct cstr_t* v, int i, int n);
void cstr_lower(struct cstr_t* v, int i, int n);
void cstr_append(struct cstr_t* v, char a);
void cstr_appendct(struct cstr_t* v, char a, int ct);
void cstr_appendcstr(struct cstr_t* v, struct cstr_t* u);
void cstr_appendstr(struct cstr_t* v, const char* s);
void cstr_appendf(struct cstr_t* v, const char* fmt, ...);
void cstr_insert(struct cstr_t* v, int i, char a);
void cstr_insertct(struct cstr_t* v, int i, char a, int n);
void cstr_remove(struct cstr_t* v, int i);
void cstr_appendm(struct cstr_t* v, int n, const char* a);
void cstr_insertm(struct cstr_t* v, int i, int n, const char* a);
void cstr_removem(struct cstr_t* v, int i, int n);
void cstr_clear(struct cstr_t* v);
int cstr_mismatch(const struct cstr_t* a, const struct cstr_t* b);
int cstr_compare(const struct cstr_t* a, const struct cstr_t* b);
int cstr_comparei(const struct cstr_t* a, const struct cstr_t* b);
int cstr_comparestr(const struct cstr_t* a, const char* b);
int cstr_comparestri(const struct cstr_t* a, const char* b);
int cstr_comparestrat(const struct cstr_t* a, int offset, const char* b, int nchars);
int cstr_comparestriat(const struct cstr_t* a, int offset, const char* b, int nchars);
int cstr_skip_ws(const struct cstr_t* str, int i);
int cstr_skip_nonws(const struct cstr_t* str, int i);
int cstr_skiptill(const struct cstr_t* str, int i, char_pred_t tillpred);
int cstr_skipwhile(const struct cstr_t* str, int i, char_pred_t whilepred);
int cstr_skiptill_chr(const struct cstr_t* str, int i, char tillchar);

int cstr_trimleft(struct cstr_t* str, char_pred_t spacepred);
int cstr_trimright(struct cstr_t* str, char_pred_t spacepred);

int cstr_find(const struct cstr_t* str, int i, const struct cstr_t* pat, int direction);
int cstr_findi(const struct cstr_t* str, int i, const struct cstr_t* pat, int direction);

