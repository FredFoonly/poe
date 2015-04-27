
enum search_mode_t {search_mode_exact, search_mode_any, search_mode_smart};

struct cmdseq_t {
  pivec* raw_cmdseq;
  vec/* preproc_cmd_t */ * preproc_cmdseq;
};

struct keydef_t {
  const char* keyname;
  pivec* cmds;
};

struct profile_t {
  cstr name;
  bool keydefs_sorted;
  struct vec_t/*keydef_t*/ key_defs;
  margins default_margins;
  tabstops default_tabstops;
  bool tabexpand;
  bool blankcompress;
  bool autowrap;
  bool oncommand;
  int tabexpand_size;
  enum search_mode_t searchmode;
};

typedef struct profile_t* PROFILEPTR;

#define MAX_KEYNAME_LEN (64)

void init_key_interp(void);
void close_key_interp(void);
void load_current_key_definitions(BUFFER keys_buffer, PROFILEPTR profile);
POE_ERR wins_handle_key(const char* keyname);
POE_ERR translate_insertable_key(const char* keyname, char* pchr);
bool is_confirm_key(const char* keyname);
struct keydef_t* find_keydef(PROFILEPTR prof, const char* keyname);
POE_ERR get_key_def(BUFFER buf, cstr* fmtted_def, const char* keyname);
void sort_profile_keydefs(PROFILEPTR prof);

PROFILEPTR alloc_profile(const char* name);
void free_profile(PROFILEPTR prof);
