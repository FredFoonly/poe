
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

extern char achPoeHome[];
void ensure_poe_dir();

int signextend_int(int x, int b);
long signextend_long(long x, int b);

/* int max(int a, int b); */
/* int min(int a, int b); */

#define ALIGNOF(type) offsetof (struct { char c; type member; }, member)

char* strsave(const char* s);
char* strlsave(const char* s, size_t n);
char* strupr(char* s);
char* strlwr(char* s);

typedef int (*char_pred_t)(char);

int poe_iswhitespace(char c);
int poe_isnotwhitespace(char c);
int poe_isword(char c);
int poe_isnotword(char c);
int poe_iscmdword(char c);
int poe_isnotcmdword(char c);
int poe_isdigit(char c);
int poe_isnotdigit(char c);
int poe_isstartbrace(char c);
int poe_isnotstartbrace(char c);
int poe_isendbrace(char c);
int poe_isnotendbrace(char c);
int poe_isquote(char c);
int poe_isnotquote(char c);
int poe_islocateoption(char c);
int poe_isnotlocateoption(char c);


#define PE_STK_ALLOC_LIMIT (10)

#define PE_ALLOC_TMP(n,siz) ((n <= PE_STK_ALLOC_LIMIT) ? alloca(n * sizeof(struct line_t)) : calloc(n, sizeof(struct line_t)))
#define PE_FREE_TMP(p, n) {\
  if (n > PE_STK_ALLOC_LIMIT)\
    free(p);\
  }


#ifndef __OpenBSD__
void *reallocarray(void *optr, size_t nmemb, size_t size);
#endif

