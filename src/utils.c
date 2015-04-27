
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "poe_exit.h"
#include "utils.h"



char achPoeHome[PATH_MAX+1];

void ensure_poe_dir()
{
  // Ensure that ~/.poe exists
  const char* lpszHome = getenv("HOME");
  if (lpszHome == NULL) {
    fprintf(stderr, "Can't find $HOME in environment\n");
    poe_exit(1);
  }
  strlcpy(achPoeHome, lpszHome, sizeof(achPoeHome));
  strlcat(achPoeHome, "/.poe", sizeof(achPoeHome));
  struct stat st;
  if (stat(achPoeHome, &st) != 0) {
    if (mkdir(achPoeHome, S_IRWXU) != 0) {
      fprintf(stderr, "Can't create directory '%s'\n", achPoeHome);
    }
  }
}


int signextend_int(int x, int b)
{
  int m = 1U<<(b-1);
  x = x & ((1U<<b)-1);
  return (x ^ m) - m;
}


long signextend_long(long x, int b)
{
  intptr_t m = 1U<<(b-1);
  x = x & ((1U<<b)-1);
  return (x ^ m) - m;
}


int poe_iswhitespace(char c)
{
  return isblank(c) || c == '\n' || c == '\r';
}


int poe_isnotwhitespace(char c)
{
  return !poe_iswhitespace(c);
}


int poe_isword(char c)
{
  return !(c == '[' || c == ']' || c == '\'' || c == '"' || c == '=') && (isalnum(c) || ispunct(c));
}

int poe_isnotword(char c)
{
  return !poe_isword(c);
}


int poe_iscmdword(char c)
{
  return !(c == '[' || c == ']' || c == '\'' || c == '"' || c == '=') && (c == '?' || isalnum(c));
}

int poe_isnotcmdword(char c)
{
  return !poe_isword(c);
}


int poe_isdigit(char c)
{
  return (c == '+') || (c == '-') || isdigit(c);
}

int poe_isnotdigit(char c)
{
  return !poe_isdigit(c);
}



int poe_isstartbrace(char c)
{
  return c == '(' || c=='[' || c=='{';
}

int poe_isnotstartbrace(char c)
{
  return !poe_isstartbrace(c);
}


int poe_isendbrace(char c)
{
  return c == ')' || c==']' || c=='}';
}

int poe_isnotendbrace(char c)
{
  return !poe_isendbrace(c);
}


int poe_isquote(char c)
{
  return c == '"' || c=='\'';
}

int poe_isnotquote(char c)
{
  return !poe_isquote(c);
}


int poe_islocateoption(char c)
{
  return isalpha(c) || c == '*' || c == '+' || c == '-';
}


int poe_isnotlocateoption(char c)
{
  return !poe_islocateoption(c);
}



/* int max(int a, int b) */
/* { */
/*   if (a>b) */
/*  return a; */
/*   else */
/*  return b; */
/* } */


/* int min(int a, int b) */
/* { */
/*   if (a<b) */
/*  return a; */
/*   else */
/*  return b; */
/* } */




char* strsave(const char* s)
{
  return strlsave(s, strlen(s));
}


char* strlsave(const char* s, size_t n)
{
  int l;
  char* save;
  if (s == NULL)
    return NULL;
  l = strlen(s);
  l = min(n, l);
  save = malloc(l+1);
  strlcpy(save, s, l+1);
  return save;
}


char* strupr(char* s)
{
  char* p;
  for (p = s; *p; p++) {
    if (isalpha(*p))
      *p = toupper(*p);
  }
  return s;
}


char* strlwr(char* s)
{
  char* p;
  for (p = s; *p; p++) {
    if (isalpha(*p))
      *p = toupper(*p);
  }
  return s;
}
