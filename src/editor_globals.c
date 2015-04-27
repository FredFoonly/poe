
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "poe_err.h"
#include "vec.h"
#include "cstr.h"
#include "tabstops.h"
#include "margins.h"
#include "bufid.h"
#include "key_interp.h"
#include "buffer.h"
#include "editor_globals.h"


//
// global editor state
//

bool __quit = false;
bool __resize_needed = false;
POE_ERR cmd_error = POE_ERR_OK;
BUFFER dir_buffer;
BUFFER keys_buffer;
BUFFER unnamed_buffer;
int vsplitter = 500;
int hsplitter = 500;
PROFILEPTR default_profile = NULL;

