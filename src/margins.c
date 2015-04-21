

#include "trace.h"
#include "poe_err.h"
#include "margins.h"


POE_ERR margins_init(struct margins_t* margins, int left, int right, int pgraph)
{
  TRACE_ENTER;
  POE_ERR rval = margins_set(margins, left, right, pgraph);
  TRACE_RETURN(rval);
}


void margins_initfrom(struct margins_t* margins, struct margins_t* src)
{
  TRACE_ENTER;
  margins_assign(margins, src);
  TRACE_EXIT;
}

void margins_destroy(struct margins_t* margins)
{
  TRACE_ENTER;
  margins->left = 0;
  margins->right = 79;
  margins->paragraph = 4;
  TRACE_EXIT;
}


void margins_assign(struct margins_t* dst, struct margins_t* src)
{
  TRACE_ENTER;
  dst->left = src->left;
  dst->right = src->right;
  dst->paragraph = src->paragraph;
  TRACE_EXIT;
}



POE_ERR margins_set(struct margins_t* margins, int left, int right, int paragraph)
{
  TRACE_ENTER;
  if (right <= left || right <= paragraph)
    TRACE_RETURN(POE_ERR_MARGINS);
  margins->left = left;
  margins->right = right;
  margins->paragraph = paragraph;
  TRACE_RETURN(POE_ERR_OK);
}


void margins_get(struct margins_t* margins, int* pleft, int* pright, int* pparagraph)
{
  TRACE_ENTER;
  *pleft = margins->left;
  *pright = margins->right;
  *pparagraph = margins->paragraph;
  TRACE_EXIT;
}


int margins_getleft(struct margins_t* margins)
{
  TRACE_ENTER;
  int rval = margins->left;
  TRACE_RETURN(rval);
}


int margins_getright(struct margins_t* margins)
{
  TRACE_ENTER;
  int rval = margins->right;
  TRACE_RETURN(rval);
}


int margins_getparagraph(struct margins_t* margins)
{
  TRACE_ENTER;
  int rval = margins->paragraph;
  TRACE_RETURN(rval);
}


