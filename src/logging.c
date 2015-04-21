
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <ncurses.h>
#include <limits.h>

#include "logging.h"
#include "utils.h"
#include "poe_exit.h"


int _logging_level = LOG_LEVEL_NONE;


char poe_err_file[PATH_MAX+1];
char poe_msg_file[PATH_MAX+1];

char* poe_err_mode = "w";
char* poe_msg_mode = "w";

void _shutdown_logging(int log_level);


void init_logging(int log_level)
{
  _logging_level = log_level;
  strlcpy(poe_err_file, achPoeHome, sizeof(poe_err_file));
  strlcat(poe_err_file, "/err.log", sizeof(poe_err_file));
  strlcpy(poe_msg_file, achPoeHome, sizeof(poe_msg_file));
  strlcat(poe_msg_file, "/msg.log", sizeof(poe_msg_file));
  _shutdown_logging(_logging_level);
}


void shutdown_logging(void)
{
  _shutdown_logging(LOG_LEVEL_NONE);
}


void _shutdown_logging(int log_level)
{
  _logging_level = log_level;
  poe_err_mode = "w";
  poe_msg_mode = "w";
}


void logmsg(const char* fmt, ...)
{
  if (_logging_level < LOG_LEVEL_MSG) {
    return;
  }
  va_list ap;
  char buf[1024];
  char timebuf[26];
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  time_t now = time(NULL);
  char* pszTime = asctime_r(localtime(&now), timebuf);
  pszTime[strlen(pszTime)-1] = '\0';

  FILE* _msgfile = fopen(poe_msg_file, poe_msg_mode);
  if (_msgfile == NULL) {
    endwin();
    fprintf(stderr, "Can't open %s for writing.\n", poe_msg_file);
    poe_exit(1);
  }

  //printf("%s: INF %s\n", pszTime, buf);
  fprintf(_msgfile, "%s: INF %s\n", pszTime, buf);
  fflush(_msgfile);
  fclose(_msgfile);
  poe_msg_mode = "a";
  va_end(ap);
}


void logerr(const char* fmt, ...)
{
  if (_logging_level < LOG_LEVEL_ERR)
    return;

  va_list ap;
  char buf[1024];
  char timebuf[26];
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  time_t now = time(NULL);
  char* pszTime = asctime_r(localtime(&now), timebuf);
  pszTime[strlen(pszTime)-1] = '\0';

  FILE* _msgfile = NULL;
  if (_logging_level >= LOG_LEVEL_MSG) {
    _msgfile = fopen(poe_msg_file, poe_msg_mode);
    if (_msgfile == NULL) {
      endwin();
      fprintf(stderr, "Can't open %s for writing.\n", poe_msg_file);
      poe_exit(1);
    }
  }

  FILE* _errfile = NULL;
  _errfile = fopen(poe_err_file, poe_err_mode);
  if (_errfile == NULL) {
    endwin();
    fprintf(stderr, "Can't open %s for writing.\n", poe_err_file);
    poe_exit(1);
  }

  // fprintf(stdout, "%s: ERR %s\n", pszTime, buf);
  if (_msgfile != NULL) {
    fprintf(_msgfile, "%s: ERR %s\n", pszTime, buf);
    fflush(_msgfile);
    fclose(_msgfile);
    poe_msg_mode = "a";
  }

  if (_errfile != NULL) {
    fprintf(_errfile, "%s: ERR %s\n", pszTime, buf);
    fflush(_errfile);
    fclose(_errfile);
    poe_err_mode = "a";
  }

  va_end(ap);
}


