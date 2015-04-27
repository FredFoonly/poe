

//
// vector of intptrs
//
struct pivec_t {
 intptr_t* elts;
 int ct;
 int cap;
};
typedef struct pivec_t pivec;

struct pivec_t* pivec_alloc(int capacity);
void pivec_init(struct pivec_t* v, int capacity);
void pivec_initfrom(struct pivec_t* dst, const struct pivec_t* src);
void pivec_initfromarr(struct pivec_t* dst, const intptr_t* src, size_t n);
void pivec_free(struct pivec_t* v);
void pivec_destroy(struct pivec_t* v);
void pivec_clear(struct pivec_t* v);
void pivec_copy(struct pivec_t* dst, const struct pivec_t* src);
int pivec_count(const struct pivec_t* v);
int pivec_capacity(const struct pivec_t* v);
intptr_t pivec_get(const struct pivec_t* v, int i);
void pivec_set(struct pivec_t* v, int i, intptr_t a);
int pivec_append(struct pivec_t* v, intptr_t a);
void pivec_insert(struct pivec_t* v, int i, intptr_t a);
void pivec_remove(struct pivec_t* v, int i);
int pivec_appendm(struct pivec_t* v, int n, intptr_t* a);
void pivec_insertm(struct pivec_t* v, int i, int n, intptr_t* a);
void pivec_removem(struct pivec_t* v, int i, int n);




//
// vector of structs
//
struct vec_t {
 char* elts;
 int ct;
 int cap;
 size_t eltsize;
};
typedef struct vec_t vec;

struct vec_t* vec_alloc(int capacity, size_t element_size);
void vec_init(struct vec_t* v, int capacity, size_t element_size);
void vec_initfrom(struct vec_t* dst, const struct vec_t* src);
void vec_free(struct vec_t* v);
void vec_destroy(struct vec_t* v);
void vec_copy(struct vec_t* dst, const struct vec_t* src);
void vec_clear(struct vec_t* dst);
int vec_count(const struct vec_t* v);
int vec_capacity(const struct vec_t* v);
void* vec_get(const struct vec_t* v, int i);
void* vec_getbufptr(const struct vec_t* v);
void vec_set(struct vec_t* v, int i, void* a);
int vec_append(struct vec_t* v, void* a);
void vec_insert(struct vec_t* v, int i, void* a);
void vec_remove(struct vec_t* v, int i);
int vec_appendm(struct vec_t* v, int n, void* a);
void vec_insertm(struct vec_t* v, int i, int n, void* a);
void vec_removem(struct vec_t* v, int i, int n);

