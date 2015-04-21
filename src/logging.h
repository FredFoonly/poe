

#define LOG_LEVEL_NONE (0)
#define LOG_LEVEL_ERR (1)
#define LOG_LEVEL_MSG (2)

void init_logging(int log_level);
void shutdown_logging(void);
void logmsg(const char* fmt, ...);
void logerr(const char* fmt, ...);

