

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "trace.h"
#include "utils.h"
#include "vec.h"
#include "cstr.h"
#include "tabstops.h"


void tabs_init(struct tabstops_t* tabs, int start, int step, struct pivec_t* ptabstops)
{
  TRACE_ENTER;
  tabs->start = start;
  tabs->step = step;
  if (ptabstops == NULL)
    pivec_init(&tabs->vtabs, 0);
  else
    pivec_initfrom(&tabs->vtabs, ptabstops);
  TRACE_EXIT;
}


void tabs_set(struct tabstops_t* tabs, int start, int step, struct pivec_t* ptabstops)
{
  TRACE_ENTER;
  tabs->start = start;
  tabs->step = step;
  if (ptabstops == NULL) {
    pivec_destroy(&tabs->vtabs);
    pivec_init(&tabs->vtabs, 0);
  }
  else {
    pivec_copy(&tabs->vtabs, ptabstops);
  }
  TRACE_EXIT;
}


void tabs_initfrom(struct tabstops_t* tabs, struct tabstops_t* src)
{
  TRACE_ENTER;
  tabs_init(tabs, 0, 8, NULL);
  tabs_assign(tabs, src);
  TRACE_EXIT;
}


void tabs_destroy(struct tabstops_t* tabs)
{
  TRACE_ENTER;
  pivec_destroy(&tabs->vtabs);
  TRACE_EXIT;
}


void tabs_assign(struct tabstops_t* dst, struct tabstops_t* src)
{
  TRACE_ENTER;
  tabs_init(dst, 0, 8, NULL);
  tabs_init(dst, src->start, src->step, &src->vtabs);
  TRACE_EXIT;
}


int tabs_next(struct tabstops_t* tabs, int col)
{
  TRACE_ENTER;
  int ntabstops = pivec_count(&tabs->vtabs);
  if (ntabstops > 0) {
    int i;
    for (i = 0; i < ntabstops; i++) {
      if (col < (int)pivec_get(&tabs->vtabs, i))
        TRACE_RETURN((int)pivec_get(&tabs->vtabs, i));
    }
  }
  int nextcol = (((col - tabs->start + tabs->step) / tabs->step) * tabs->step) + tabs->start;
  TRACE_RETURN(nextcol);
}


// Slighly trickier than nexttab - if the column is out of range of
// the table, we don't know if the tabstop is the last entry in the
// table, or if it needs to calculated from the start/stride.  Only
// after we've calculated both can we tell which one is right.
int tabs_prev(struct tabstops_t* tabs, int col)
{
  TRACE_ENTER;
  int c1 = 0;
  int ntabstops = pivec_count(&tabs->vtabs);
  if (ntabstops > 0) {
    int i;
    for (i = ntabstops-1; i >= 0; i--) {
      if (col > (int)pivec_get(&tabs->vtabs, i)) {
        c1 = (int)pivec_get(&tabs->vtabs, i);
        break;
      }
    }
  }
  TRACE_RETURN(max(c1, (((col - tabs->start - 1) / tabs->step) * tabs->step) + tabs->start));
}


