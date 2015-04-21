
void init_getkey(void);
void close_getkey(void);

enum confirmation_t {confirmation_y, confirmation_n, confirmation_esc};


POE_ERR get_insertable_key(char* pchr);
enum confirmation_t get_confirmation(const char* prompt);
const char* ui_get_key();

