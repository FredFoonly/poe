
typedef struct profile_t* PROFILEPTR;

#define MAX_KEYNAME_LEN (64)

void init_key_interp(void);
void close_key_interp(void);
POE_ERR wins_handle_key(const char* keyname);
POE_ERR translate_insertable_key(const char* keyname, char* pchr);
bool is_confirm_key(const char* keyname);
struct keydef_t* find_keydef(PROFILEPTR prof, const char* keyname);
//POE_ERR get_key_def(cstr* fmtted_def, const char* keyname);
POE_ERR get_key_def(BUFFER buf, cstr* fmtted_def, const char* keyname);

