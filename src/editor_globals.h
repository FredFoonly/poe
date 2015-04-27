

#define SPLIT_MAX_VAL (1000)
#define SCALE_SPLITTER(a,r) (((a)*(r))/(1.0*SPLIT_MAX_VAL))

extern bool __quit;
extern bool __resize_needed;
extern POE_ERR cmd_error;
extern BUFFER dir_buffer;
extern BUFFER keys_buffer;
extern BUFFER unnamed_buffer;
extern int vsplitter;
extern int hsplitter;
extern PROFILEPTR default_profile;



