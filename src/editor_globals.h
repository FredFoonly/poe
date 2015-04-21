

enum search_mode_t {search_mode_exact, search_mode_any, search_mode_smart};

#define SPLIT_MAX_VAL (1000)

#define SCALE_SPLITTER(a,r) (((a)*(r))/(1.0*SPLIT_MAX_VAL))

extern bool __quit;
extern bool __resize_needed;
extern BUFFER dir_buffer;
extern BUFFER keys_buffer;
extern BUFFER unnamed_buffer;
extern margins default_margins;
extern tabstops default_tabstops;
extern bool tabexpand;
extern bool blankcompress;
extern bool autowrap;
extern int tabexpand_size;
extern int vsplitter;
extern int hsplitter;
extern enum search_mode_t searchmode;
extern PROFILEPTR dflt_data_profile;
extern PROFILEPTR dflt_cmd_profile;


