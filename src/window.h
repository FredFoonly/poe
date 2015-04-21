

#define POE_SPECIAL_KEY (1000)

typedef struct window_t* WINPTR;

void init_windows(void);
void shutdown_windows(void);
void close_windows(void);

void wins_ensure_initial_win(void);
void wins_ensure_initial_view(void);
void wins_split(void);
void wins_zoom(void);
void wins_nextwindow(void);
void wins_nextview(void);
void wins_cur_switchbuffer(BUFFER buf);
void wins_cur_nextbuffer(void);
void wins_hidebuffer(BUFFER buf);
void wins_resize(void);
WINPTR wins_get_cur(void);

void win_setflags(WINPTR pwin, int flags);
void win_clrflags(WINPTR pwin, int flags);
int win_tstflags(WINPTR pwin, int flags);

bool win_get_commandmode(WINPTR pwin);
void win_set_commandmode(WINPTR pwin, bool commandmode);
void win_toggle_commandmode(WINPTR pwin);

PROFILEPTR win_get_profile(WINPTR pwin);
PROFILEPTR win_get_data_profile(WINPTR pwin);
PROFILEPTR win_get_cmd_profile(WINPTR pwin);

typedef struct cmd_ctx_t* cmd_ctx_ptr;
bool update_context(cmd_ctx_ptr ctx);

void wins_repaint_all(void);
void wins_set_message(const char* message);

const char* ui_get_key();



