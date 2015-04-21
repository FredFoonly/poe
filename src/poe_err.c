
#include <stdio.h>

#include "poe_err.h"


const char* poe_err_message(POE_ERR err)
{
  static char errmsg[64];
  const char* rval = "";
  switch (err) {
  case POE_ERR_OK: rval = ""; break;
  case POE_ERR_BLOCK_MARK_REQ: rval = "Block mark required"; break;
  case POE_ERR_CANNOT_NAME_UNNAMED: rval = "Cannot rename the .unnamed file"; break;
  case POE_ERR_CANNOT_SAVE_INTERNAL: rval = "Cannot save an internal file"; break;
  case POE_ERR_MEM_FULL: rval = "Memory is full"; break;
  case POE_ERR_CMD_FILE_NOT_FOUND: rval = "POE.PRO file not found"; break;
  case POE_ERR_CMD_FILES_NESTED: rval = "Command files nested too deep "; break;
  case POE_ERR_MARK_TYPE_CONFLICT: rval = "Mark type conflict"; break;
  case POE_ERR_DISK_FULL: rval = "Disk is full"; break;
  case POE_ERR_CLOSING_FILE: rval = "Can't close file due to some error"; break;
  case POE_ERR_DISP_TYPE: rval = "Error in set display format"; break;
  case POE_ERR_MARGINS: rval = "Error in set margins"; break;
  case POE_ERR_NOTABS: rval = "Error in load/save - misspelled notabs"; break;
  case POE_ERR_REPL_STR: rval = "Error in replacement string"; break;
  case POE_ERR_SRCH_STR: rval = "Error in search string"; break;
  case POE_ERR_TABS: rval = "Error in tab stops settings"; break;
  case POE_ERR_CANT_OPEN: rval = "Can't open file"; break;
  case POE_ERR_READING_CMD: rval = "Can't read POE.PRO or a macro file"; break;
  case POE_ERR_READING_FILE: rval = "Can't read file"; break;
  case POE_ERR_WRITING_FILE: rval = "Can't write file"; break;
  case POE_ERR_FILE_NOT_FOUND: rval = "Can't find file"; break;
  case POE_ERR_INVALID_FUNCTION: rval = "Don't recognize function"; break;
  case POE_ERR_INVALID_KEY: rval = "Bad key in key definition, or after ? command"; break;
  case POE_ERR_INVALID_NAME: rval = "Name given to NAME or EDIT was invalid"; break;
  case POE_ERR_KEY_NOT_DEFINED: rval = "No definition in POE.PRO"; break;
  case POE_ERR_LINE_MARK_REQ: rval = "Line mark required"; break;
  case POE_ERR_MARKED_BLOCK_EXISTS: rval = "Already have a block in another buffer"; break;
  case POE_ERR_MISSING_FILE_NAME: rval = "Save or FILE was missing file name"; break;
  case POE_ERR_MISSING_QUOTE: rval = "Missing ', \", or ]"; break;
  case POE_ERR_NAME_IN_USE: rval = "Attempted to NAME current buffer to a name that is already in use"; break;
  case POE_ERR_MISSING_EQ_DEFN: rval = "Missing '=' in definition"; break;
  case POE_ERR_NO_CHANGE_PENDING: rval = "Tried to execute [confirm change] when a change wasn't pending"; break;
  case POE_ERR_NO_MARKED_AREA: rval = "No mark"; break;
  case POE_ERR_NOT_FOUND: rval = "Search text not found"; break;
  case POE_ERR_SET_OPTION_UNK: rval = "Set of unknown option"; break;
  case POE_ERR_SRC_DEST_CONFLICT: rval = "Overlap of source mark and target position"; break;
  case POE_ERR_TOO_MANY_TABSTOPS: rval = "Too many tabstops"; break;
  case POE_ERR_UNK_CMD: rval = "Unknown command"; break;
  case POE_ERR_WRITE_PROTECTED: rval = "Attempted to write to a write-protected file"; break;
  case POE_ERR_NO_MARKS_SAVED: rval = "No marks saved"; break;
  case POE_ERR_SET_VAL_UNK: rval = "Attempted to SET an unrecognized value for this option"; break;  
  default:
    snprintf(errmsg, sizeof(errmsg), "Error %d", err);
    rval = errmsg;
  }
  return rval;
}
