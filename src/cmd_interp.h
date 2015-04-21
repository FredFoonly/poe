

extern POE_ERR cmd_error;

POE_ERR interpret_command_seq(cmd_ctx* ctx);
cstr format_command(const pivec* cmdseq, int pc);
cstr format_command_seq(const pivec* cmdseq, int pc);

