

#define DEFAULT_TAB_START (0)
#define DEFAULT_TAB_STEP (8)
#define MAX_PARSABLE_TABSTOPS (20)

struct tabstops_t
{
  int start;
  int step;
  struct pivec_t vtabs;
};
typedef struct tabstops_t tabstops;

void tabs_init(struct tabstops_t* tabs, int start, int step, struct pivec_t* ptabstops);
void tabs_initfrom(struct tabstops_t* tabs, struct tabstops_t* src);
void tabs_destroy(struct tabstops_t* tabs);
void tabs_assign(struct tabstops_t* dst, struct tabstops_t* src);

void tabs_set(struct tabstops_t* tabs, int start, int step, struct pivec_t* ptabstops);

int tabs_next(struct tabstops_t* tabs, int col);
int tabs_prev(struct tabstops_t* tabs, int col);


