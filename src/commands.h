
#define __CMD_INT_MASK__ (0x0FFFFF)

#define CMD_STR(s) ((intptr_t)(s))
#define CMD_INT(i) ((((intptr_t)(i))<<1)|1)
#define __CMD_INT_SHIFTED_MASK__ (CMD_INT(__CMD_INT_MASK__))
#define CMD_NULL ((intptr_t)0)
#define CMD_SEP (CMD_NULL)

#define END_OF_CMD(v, i) (pivec_get(v, i) == CMD_SEP)
#define END_OF_CMDSEQ(v, i) ( (pivec_get(v, i) == CMD_SEP) && (pivec_get(v, i+1) == CMD_NULL) )

#define CMD_IS_INT(p) ( ((p)&__CMD_INT_SHIFTED_MASK__) == (p))
#define CMD_IS_STR(p) ((p)>__CMD_INT_SHIFTED_MASK__)

#define CMD_INTVAL(p) (signextend_int((int)(((p)>>1)&__CMD_INT_MASK__), 20))
#define CMD_STRVAL(p) ((const char*)(void*)(p))

struct cmd_ctx_t {
  bool src_is_commandline;
  bool save_commandline;
  WINPTR wnd;
  VIEWPTR targ_view, data_view, cmd_view;
  BUFFER targ_buf, data_buf, cmd_buf;
  int targ_row, targ_col, data_row, data_col, cmd_row, cmd_col;
  const pivec* cmdseq;
  int pc;
};
typedef struct cmd_ctx_t cmd_ctx;

typedef POE_ERR (*command_handler_t)(cmd_ctx* ctx);


////////////////////////////////////////
// Not being used yet...
enum parm_type_t { intlit, stringlit, charlit };
struct parm_t {
  enum parm_type_t parm_type;
  int ival;
  const char* sval;
  char cval;
};

struct preproc_cmd_t {
  command_handler_t cmd_func;
  vec/*parm_t*/* parms_idx;
};
////////////////////////////////////////


void init_commands(void);
void close_commands(void);
command_handler_t lookup_command(const pivec* cmd, int pc, int* args_idx);

