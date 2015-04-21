

#ifdef POE_DBG_TRON

#define TRACE_ENTER int __trace_level__ = _trace_enter(__func__);
#define TRACE_EXIT {_trace_exit(__func__, __trace_level__); return; }
#define TRACE_RETURN(v) { _trace_exit(__func__, __trace_level__); return (v); }
#define TRACE_CATCH (_trace_catch(__func__, __trace_level__))

#else

#define TRACE_ENTER /**/
#define TRACE_EXIT { return; }
#define TRACE_RETURN(v) { return (v); }
#define TRACE_CATCH /**/

#endif


void init_trace_stack(void);
void trace_stack_print(void);

int _trace_enter(const char* func_name);
void _trace_exit(const char* func_name, int level);
void _trace_catch(const char* func_name, int new_level);
