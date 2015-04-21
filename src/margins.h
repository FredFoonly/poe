
struct margins_t
{
  int left;
  int right;
  int paragraph;
};
typedef struct margins_t margins;

POE_ERR margins_init(struct margins_t* margins, int left, int right, int pgraph);
void margins_initfrom(struct margins_t* margins, struct margins_t* src);
void margins_destroy(struct margins_t* margins);
void margins_assign(struct margins_t* dst, struct margins_t* src);
POE_ERR margins_set(struct margins_t* margins, int left, int right, int pgraph);

void margins_get(struct margins_t* margins, int* pleftmargin, int* prightmargin, int* pindent);
int margins_getleft(struct margins_t* margins);
int margins_getright(struct margins_t* margins);
int margins_getparagraph(struct margins_t* margins);
