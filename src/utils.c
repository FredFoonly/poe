
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>

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


#ifndef __OpenBSD__
void * reallocarray(void *optr, size_t nmemb, size_t size)
{
  const size_t mul_no_overflow = (size_t)1 << (sizeof(size_t) * 4);
  if ((nmemb >= mul_no_overflow || size >= mul_no_overflow) &&
	  nmemb > 0 && SIZE_MAX / nmemb < size) {
	errno = ENOMEM;
	return NULL;
  }
  return realloc(optr, size * nmemb);
}
#endif


#if !defined(__OpenBSD__) && !defined(__FreeBSD__)
size_t strlcpy(char *dst, const char *src, size_t dsize)
{
    const char *osrc = src;
    size_t nleft = dsize;

    /* Copy as many bytes as will fit. */
    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src. */
    if (nleft == 0) {
        if (dsize != 0)
            *dst = '\0';        /* NUL-terminate dst */
        while (*src++)
            ;
    }

    return(src - osrc - 1); /* count does not include NUL */
}


size_t strlcat(char *dst, const char *src, size_t dsize)
{
    const char *odst = dst;
    const char *osrc = src;
    size_t n = dsize;
    size_t dlen;

    /* Find the end of dst and adjust bytes left but don't go past end. */
    while (n-- != 0 && *dst != '\0')
        dst++;
    dlen = dst - odst;
    n = dsize - dlen;

    if (n-- == 0)
        return(dlen + strlen(src));
    while (*src != '\0') {
        if (n != 0) {
            *dst++ = *src;
            n--;
        }
        src++;
    }
    *dst = '\0';

    return(dlen + (src - osrc));    /* count does not include NUL */
}


char* dirname(const char* path)
{
  static  char  bname[PATH_MAX];
  register  const  char  *endp;
  
  /*  Empty  or  NULL  string  gets  treated  as  "."  */
  if  (path  ==  NULL  ||  *path  ==  '\0')  {
    (void)strlcpy(bname,  ".",  sizeof  bname);
    return(bname);
  }
  
  /*  Strip  trailing  slashes  */
  endp  =  path  +  strlen(path)  -  1;
  while  (endp  >  path  &&  *endp  ==  '/')
    endp--;
  
  /*  Find  the  start  of  the  dir  */
  while  (endp  >  path  &&  *endp  !=  '/')
    endp--;
  
  /*  Either  the  dir  is  "/"  or  there  are  no  slashes  */
  if  (endp  ==  path)  {
    (void)strlcpy(bname,  *endp  ==  '/'  ?  "/"  :  ".",  sizeof  bname);
    return(bname);
  }  else  {
    do  {
      endp--;
    }  while  (endp  >  path  &&  *endp  ==  '/');
  }
  
  if  (endp  -  path  +  2  >  sizeof(bname))  {
    errno  =  ENAMETOOLONG;
    return(NULL);
  }
  strlcpy(bname,  path,  endp  -  path  +  2);
  return(bname);
}

#endif
