
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "trace.h"
#include "logging.h"
#include "poe_err.h"
#include "poe_exit.h"
#include "utils.h"
#include "cstr.h"
#include "vec.h"
#include "srchpath.h"


pivec/* cstr* */ * _profile_path = NULL;

void _profile_path_alloc(void);
void _profile_path_free(void);
pivec* _search_path_alloc(void);
void _search_path_free(pivec* srchpath);
bool _search_path_parse(pivec* srchpath, const char* path);
bool _srchpath_find(pivec* srchpath, const char* name, char* filepath, size_t filepath_size);


bool set_profile_searchpath(const char* path)
{
  TRACE_ENTER;
  if (path == NULL)
	TRACE_RETURN(false);
  pivec* newpath = _search_path_alloc();
  bool parsed = _search_path_parse(newpath, path);
  if (parsed) {
	if (_profile_path != NULL)
	  _profile_path_free();
	_profile_path = newpath;
  }
  else {
	_search_path_free(newpath);
  }
  TRACE_RETURN(parsed);
}



bool find_profile_file(const char* profilename, char* filepath, size_t filepath_size)
{
  TRACE_ENTER;
  bool bFound = _srchpath_find(_profile_path, profilename, filepath, filepath_size);
  TRACE_RETURN(bFound);
}



bool _srchpath_find(pivec* srchpath, const char* pszName, char* filepath, size_t filepath_size)
{
  TRACE_ENTER;
  bool bFound = false;
  memset(filepath, 0, filepath_size);
  char achExpName[PATH_MAX+1];
  char achTmpName[PATH_MAX+1];

  // expand ~
  if (pszName[0]=='~') {
	const char* pszHome = getenv("HOME");
	strlcpy(achExpName, pszHome==NULL?"":pszHome, sizeof(achExpName));
	strlcat(achExpName, "/", sizeof(achExpName));
	strlcat(achExpName, pszName+2, sizeof(achExpName));
  }
  else {
	strlcpy(achExpName, pszName, sizeof(achExpName));
  }

  struct stat sb;
  if (achExpName[0] == '/') {
	memset(&sb, 0, sizeof(sb));
	errno = -1;
	if (stat(achExpName, &sb) == 0) {
	  if (S_ISREG(sb.st_mode)) {
		strlcpy(filepath, achExpName, filepath_size);
		bFound = true;
	  }
	}
  }
  else {
	int i, n = pivec_count(srchpath);
	for (i = 0; i < n && !bFound; i++) {
	  memset(achTmpName, 0, sizeof(achTmpName));
	  cstr* elt = (cstr*)pivec_get(srchpath, i);
	  strlcpy(achTmpName, cstr_getbufptr(elt), sizeof achTmpName);
	  strlcat(achTmpName, "/", sizeof(achTmpName));
	  strlcat(achTmpName, achExpName, sizeof(achTmpName));
	  memset(&sb, 0, sizeof(sb));
	  errno = -1;
      if (stat(achTmpName, &sb) == 0) {
		if (S_ISREG(sb.st_mode)) {
		  strlcpy(filepath, achTmpName, filepath_size);
		  bFound = true;
		}
	  }
	}
  }
  TRACE_RETURN(bFound);
}


bool _search_path_parse(pivec* srchpath, const char* path)
{
  TRACE_ENTER;
  const char* p = path;
  const char* q = strchr(p, ':');
  while (p != NULL) {
	cstr* pstr = NULL;
	if (q == NULL) {
	  pstr = cstr_allocstr(p);
	}
	else {
	  pstr = cstr_allocstrn(p, q-p);
	}
	cstr_trimright(pstr, poe_iswhitespace);
	cstr_trimleft(pstr, poe_iswhitespace);
	// expand ~/
	if (cstr_count(pstr) >= 2 && cstr_get(pstr, 0) == '~' && cstr_get(pstr, 1) == '/') {
	  const char* pszHome = getenv("HOME");
	  if (pszHome == NULL) pszHome = "";
	  cstr_removem(pstr, 0, 2);
	  cstr_insert(pstr, 0, '/');
	  cstr_insertm(pstr, 0, strlen(pszHome), pszHome);
	}
	pivec_append(srchpath, (intptr_t)pstr);
	if (q == NULL) {
	  p = q;
	}
	else {
	  p = q+1;
	  q = strchr(q+1, ':');
	}
  }
  TRACE_RETURN(true);
}


void _profile_path_alloc(void)
{
  TRACE_ENTER;
  _profile_path = _search_path_alloc();
  TRACE_EXIT;
}


void _profile_path_free(void)
{
  TRACE_ENTER;
   _search_path_free(_profile_path);
  _profile_path = NULL;
  TRACE_EXIT;
}



pivec* _search_path_alloc(void)
{
  TRACE_ENTER;
  pivec* rval = pivec_alloc(1);
  TRACE_RETURN(rval);
}


void _search_path_free(pivec* srchpath)
{
  TRACE_ENTER;
  int i, n = pivec_count(srchpath);
  for (i = 0; i < n; i++) {
	cstr* s = (cstr*)pivec_get(srchpath, i);
	cstr_free(s);
  }
  pivec_free(srchpath);
  TRACE_EXIT;
}





