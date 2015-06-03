
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "trace.h"
#include "logging.h"
#include "utils.h"
#include "poe_err.h"
#include "vec.h"
#include "cstr.h"
#include "margins.h"
#include "tabstops.h"
#include "bufid.h"
#include "key_interp.h"
#include "buffer.h"
#include "window.h"
#include "view.h"
#include "commands.h"
#include "editor_globals.h"


//#define FULL_DEFAULT_PROFILE



// From key_interp.h
void defkey(PROFILEPTR profile, const char* keyname, const intptr_t* cmds, int ncmds);

#define DEFKEY(profile, name,...) {                             \
    static intptr_t _cmds_[] = {__VA_ARGS__, CMD_SEP, CMD_NULL};        \
    defkey(profile, name, _cmds_, (sizeof(_cmds_)/sizeof(_cmds_[0])));  \
  }



//
// Based on the default PE2.PRO
//
// Once the profile parser is done, this can just parse the literal string...
//
void set_default_profile(void)
{
  TRACE_ENTER;
  default_profile->tabexpand = true;
  default_profile->blankcompress = false;
  margins_set(&default_profile->default_margins, 0, 255, 0);
  tabs_set(&default_profile->default_tabstops, 0, 4, NULL);
  default_profile->tabexpand = 8;
  DEFKEY(default_profile, "RESIZE", CMD_STR("RESIZE"), CMD_STR("DISPLAY"));
  DEFKEY(default_profile, "UP", CMD_STR("UP"));
  DEFKEY(default_profile, "DOWN", CMD_STR("DOWN"));
  DEFKEY(default_profile, "LEFT", CMD_STR("LEFT"));
  DEFKEY(default_profile, "RIGHT", CMD_STR("RIGHT"));
  DEFKEY(default_profile, "INSERT", CMD_STR("INSERT"), CMD_STR("TOGGLE"));
  DEFKEY(default_profile, "DEL", CMD_STR("DELETE"), CMD_STR("CHAR"));
  DEFKEY(default_profile, "BACKSPACE", CMD_STR("RUBOUT"));
  DEFKEY(default_profile, "ESC", CMD_STR("COMMAND"), CMD_STR("TOGGLE"));
  DEFKEY(default_profile, "ENTER",
         CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("DOWN"), CMD_SEP,
         CMD_STR("INDENT"));
  DEFKEY(default_profile, "CMD-ENTER", CMD_STR("EXECUTE"));
  char kname[4];
  intptr_t cmds[4];
  cmds[0] = CMD_STR("CHAR"); cmds[2] = CMD_SEP; cmds[3] = CMD_NULL;
  int i;
  for (i = 'A'; i <= 'Z'; i++) {
    if (i == 'y') {
      DEFKEY(default_profile, "Y", CMD_STR("CONFIRM"), CMD_STR("CHANGE"), CMD_SEP, CMD_STR("CHAR"), CMD_INT('y'));
      DEFKEY(default_profile, "S-Y", CMD_STR("CONFIRM"), CMD_STR("CHANGE"), CMD_SEP, CMD_STR("CHAR"), CMD_INT('Y'));
    }
    else {
      kname[0] = i; kname[1] = '\0'; cmds[1] = CMD_INT(tolower(i));
      defkey(default_profile, kname, cmds, 4);
      kname[0] = 'S'; kname[1] = '-'; kname[2] = i; kname[3] = '\0'; cmds[1] = CMD_INT(tolower(i));
      defkey(default_profile, kname, cmds, 4);
    }
  }
  DEFKEY(default_profile, "0", CMD_STR("CHAR"), CMD_INT('0'));
  DEFKEY(default_profile, "1", CMD_STR("CHAR"), CMD_INT('1'));
  DEFKEY(default_profile, "2", CMD_STR("CHAR"), CMD_INT('2'));
  DEFKEY(default_profile, "3", CMD_STR("CHAR"), CMD_INT('3'));
  DEFKEY(default_profile, "4", CMD_STR("CHAR"), CMD_INT('4'));
  DEFKEY(default_profile, "5", CMD_STR("CHAR"), CMD_INT('5'));
  DEFKEY(default_profile, "6", CMD_STR("CHAR"), CMD_INT('6'));
  DEFKEY(default_profile, "7", CMD_STR("CHAR"), CMD_INT('7'));
  DEFKEY(default_profile, "8", CMD_STR("CHAR"), CMD_INT('8'));
  DEFKEY(default_profile, "9", CMD_STR("CHAR"), CMD_INT('9'));
  DEFKEY(default_profile, "MINUS", CMD_STR("CHAR"), CMD_INT('-'));
  DEFKEY(default_profile, "EQUALS", CMD_STR("CHAR"), CMD_INT('='));
  DEFKEY(default_profile, "LBRACKET", CMD_STR("CHAR"), CMD_INT('['));
  DEFKEY(default_profile, "RBRACKET", CMD_STR("CHAR"), CMD_INT(']'));
  DEFKEY(default_profile, "SEMI", CMD_STR("CHAR"), CMD_INT(';'));
  DEFKEY(default_profile, "QUOTE", CMD_STR("CHAR"), CMD_INT('\''));
  DEFKEY(default_profile, "BACKQUOTE", CMD_STR("CHAR"), CMD_INT('`'));
  DEFKEY(default_profile, "COMMA", CMD_STR("CHAR"), CMD_INT(','));
  DEFKEY(default_profile, "PERIOD", CMD_STR("CHAR"), CMD_INT('.'));
  DEFKEY(default_profile, "SLASH", CMD_STR("CHAR"), CMD_INT('/'));
  DEFKEY(default_profile, "BACKSLASH", CMD_STR("CHAR"), CMD_INT('\\'));
  DEFKEY(default_profile, "SPACE", CMD_STR("CHAR"), CMD_INT(' '));
  DEFKEY(default_profile, "S-0", CMD_STR("CHAR"), CMD_INT(')'));
  DEFKEY(default_profile, "S-1", CMD_STR("CHAR"), CMD_INT('!'));
  DEFKEY(default_profile, "S-2", CMD_STR("CHAR"), CMD_INT('@'));
  DEFKEY(default_profile, "S-3", CMD_STR("CHAR"), CMD_INT('#'));
  DEFKEY(default_profile, "S-4", CMD_STR("CHAR"), CMD_INT('$'));
  DEFKEY(default_profile, "S-5", CMD_STR("CHAR"), CMD_INT('%'));
  DEFKEY(default_profile, "S-6", CMD_STR("CHAR"), CMD_INT('^'));
  DEFKEY(default_profile, "S-7", CMD_STR("CHAR"), CMD_INT('&'));
  DEFKEY(default_profile, "S-8", CMD_STR("CHAR"), CMD_INT('*'));
  DEFKEY(default_profile, "S-9", CMD_STR("CHAR"), CMD_INT('('));
  DEFKEY(default_profile, "S-MINUS",      CMD_STR("CHAR"), CMD_INT('_'));
  DEFKEY(default_profile, "S-EQUALS",     CMD_STR("CHAR"), CMD_INT('+'));
  DEFKEY(default_profile, "S-LBRACKET",   CMD_STR("CHAR"), CMD_INT('{'));
  DEFKEY(default_profile, "S-RBRACKET",   CMD_STR("CHAR"), CMD_INT('}'));
  DEFKEY(default_profile, "S-SEMI",       CMD_STR("CHAR"), CMD_INT(':'));
  DEFKEY(default_profile, "S-QUOTE",      CMD_STR("CHAR"), CMD_INT('"'));
  DEFKEY(default_profile, "S-BACKQUOTE",  CMD_STR("CHAR"), CMD_INT('~'));
  DEFKEY(default_profile, "S-COMMA",      CMD_STR("CHAR"), CMD_INT('<'));
  DEFKEY(default_profile, "S-PERIOD",     CMD_STR("CHAR"), CMD_INT('>'));
  DEFKEY(default_profile, "S-SLASH",      CMD_STR("CHAR"), CMD_INT('?'));
  DEFKEY(default_profile, "S-BACKSLASH",  CMD_STR("CHAR"), CMD_INT('|'));
  DEFKEY(default_profile, "F8", CMD_STR("NEXT"), CMD_STR("FILE"));
  DEFKEY(default_profile, "F9", CMD_STR("INSERT"), CMD_STR("LINE"));
  DEFKEY(default_profile, "A-S", CMD_STR("SPLIT"));
  DEFKEY(default_profile, "A-J", CMD_STR("JOIN"), CMD_STR("LINE"));



#ifdef FULL_DEFAULT_PROFILE
  /* DEFKEY(default_profile, "LEFT", CMD_STR("LEFT"), CMD_STR("WRAP")); */
  /* DEFKEY(default_profile, "RIGHT", CMD_STR("RIGHT"), CMD_STR("WRAP")); */
  DEFKEY(default_profile, "PGUP", CMD_STR("PAGE"), CMD_STR("UP"));
  DEFKEY(default_profile, "PGDN", CMD_STR("PAGE"), CMD_STR("DOWN"));
  DEFKEY(default_profile, "HOME", CMD_STR("BEGIN"), CMD_STR("LINE"));
  DEFKEY(default_profile, "END", CMD_STR("END"), CMD_STR("LINE"));

  // Dumb ENTER
  //  DEFKEY(default_profile, "ENTER", CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("DOWN"), CMD_SEP, CMD_STR("INDENT"));
  // Smart ENTER
  DEFKEY(default_profile, "ENTER",
         CMD_STR("INSERT"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("BACKTAB"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("POP"), CMD_STR("MARK"));

  DEFKEY(default_profile, "TAB", CMD_STR("TAB"));
  //DEFKEY(default_profile, "TAB", CMD_STR("TAB"), CMD_STR("WORD"));

  DEFKEY(default_profile, "F1", CMD_STR("E"), CMD_STR("~/.poe/poe.hlp"));
  DEFKEY(default_profile, "F2", CMD_STR("CURSOR"),CMD_STR("COMMAND"), CMD_SEP, CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"), CMD_SEP, CMD_STR("STR"), CMD_STR("save "));
  DEFKEY(default_profile, "F3", CMD_STR("CURSOR"), CMD_STR("COMMAND"), CMD_SEP, CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"), CMD_SEP, CMD_STR("STR"), CMD_STR("file "));
  DEFKEY(default_profile, "F4", CMD_STR("QUIT"));


  DEFKEY(default_profile, "F5", CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "F6", CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "F7", CMD_STR("CURSOR"),CMD_STR("COMMAND"), CMD_SEP, CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"), CMD_SEP, CMD_STR("STR"), CMD_STR("print"));
  DEFKEY(default_profile, "F8", CMD_STR("NEXT"), CMD_STR("FILE"));
  DEFKEY(default_profile, "F9", CMD_STR("INSERT"), CMD_STR("LINE"));
  DEFKEY(default_profile, "F10", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_SEP,
                CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP,
                CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP,
                CMD_STR("BACKTAB"), CMD_STR("WORD"), CMD_SEP,
                CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP,
                CMD_STR("BEGIN"), CMD_STR("WORD"), CMD_SEP,
                CMD_STR("BEGIN"), CMD_STR("MARK"), CMD_SEP,
                CMD_STR("POP"), CMD_STR("MARK"));

  DEFKEY(default_profile, "C-LEFT", CMD_STR("BACKTAB"), CMD_STR("WORD"));
  DEFKEY(default_profile, "C-RIGHT", CMD_STR("TAB"), CMD_STR("WORD"));
  DEFKEY(default_profile, "C-UP", CMD_STR("UP"), CMD_INT(20));
  DEFKEY(default_profile, "C-DOWN", CMD_STR("DOWN"), CMD_INT(20));

  DEFKEY(default_profile, "C-PGDN", CMD_STR("BOTTOM"), CMD_STR("EDGE"));
  DEFKEY(default_profile, "C-PGUP", CMD_STR("TOP"), CMD_STR("EDGE"));
  DEFKEY(default_profile, "C-HOME", CMD_STR("TOP"));
  DEFKEY(default_profile, "C-END", CMD_STR("BOTTOM"));
  //DEFKEY(default_profile, "C-ENTER", CMD_STR("EXECUTE")); // C-ENTER doesn't seem to be detected by ncurses
  DEFKEY(default_profile, "C-BACKSPACE", CMD_STR("DELETE"), CMD_STR("LINE"));

  DEFKEY(default_profile, "C-A", CMD_STR("BEGIN"), CMD_STR("LINE"));
  // DEFKEY(default_profile, "C-B", CMD_STR("BEGIN"), CMD_STR("WORD"));
  DEFKEY(default_profile, "C-C", CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP, 
         CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("CENTER"), CMD_STR("IN"), CMD_STR("MARGINS"), CMD_SEP,
         CMD_STR("UNMARK"), CMD_SEP,
         CMD_STR("POP"), CMD_STR("MARK"));
  DEFKEY(default_profile, "C-D", CMD_NULL);
  // DEFKEY(default_profile, "C-E", CMD_STR("END"), CMD_STR("WORD"));
  DEFKEY(default_profile, "C-E", CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "C-F", CMD_STR("COPY"), CMD_STR("FROM"), CMD_STR("COMMAND"));
  DEFKEY(default_profile, "C-G", CMD_NULL);
  DEFKEY(default_profile, "C-H", CMD_STR("RUBOUT"));
  DEFKEY(default_profile, "C-K", CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "C-L", CMD_STR("RESIZE"), CMD_STR("DISPLAY"), CMD_SEP, CMD_STR("CENTER"), CMD_STR("LINE"));
  DEFKEY(default_profile, "C-M", CMD_NULL);
  DEFKEY(default_profile, "C-N", CMD_STR("DOWN"), CMD_INT(60));
  DEFKEY(default_profile, "C-O", CMD_NULL);
  DEFKEY(default_profile, "C-P", CMD_STR("PRINT"), CMD_STR("MARK"));
  DEFKEY(default_profile, "C-R",
         CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP, 
         CMD_STR("COPY"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("POP"), CMD_STR("MARK"));
  DEFKEY(default_profile, "C-S", CMD_STR("SPLIT"), CMD_STR("SCREEN"));
  DEFKEY(default_profile, "C-T", CMD_STR("COPY"), CMD_STR("TO"), CMD_STR("COMMAND"));
  DEFKEY(default_profile, "C-U", CMD_STR("EDIT"), CMD_STR(".UNNAMED"));
  DEFKEY(default_profile, "C-V", CMD_STR("NEXT"), CMD_STR("VIEW"));
  DEFKEY(default_profile, "C-W", CMD_STR("NEXT"), CMD_STR("WINDOW"));
  DEFKEY(default_profile, "C-X", CMD_NULL);
  DEFKEY(default_profile, "C-Y", CMD_NULL);
  DEFKEY(default_profile, "C-Z", CMD_STR("ZOOM"), CMD_STR("WINDOW"));

  DEFKEY(default_profile, "C-SPACE", CMD_STR("EXECUTE")); // rebound here because C-ENTER
                                // doesn't seem to be detected by
                                // ncurses
  DEFKEY(default_profile, "C-F1", CMD_STR("INSERT"), CMD_STR("LINE"));
  DEFKEY(default_profile, "C-F2", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(2));
  DEFKEY(default_profile, "C-F3", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(3));
  DEFKEY(default_profile, "C-F4", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(4));
  DEFKEY(default_profile, "C-F5", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(5));
  DEFKEY(default_profile, "C-F6", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(6));
  DEFKEY(default_profile, "C-F7", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(7));
  DEFKEY(default_profile, "C-F8", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(8));
  DEFKEY(default_profile, "C-F9", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(9));
  DEFKEY(default_profile, "C-F10", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(10));


  //DEFKEY(default_profile, "SHIFT-TAB",  CMD_STR("BACKTAB"));
  DEFKEY(default_profile, "S-TAB",  CMD_STR("BACKTAB"), CMD_STR("WORD"));
 
  DEFKEY(default_profile, "S-F1", CMD_STR("PAGE"), CMD_STR("DOWN"), CMD_SEP,
         CMD_STR("BOTTOM"), CMD_STR("EDGE"), CMD_SEP,
         CMD_STR("DOWN"), CMD_INT(2), CMD_SEP,
         CMD_STR("CURSOR"), CMD_STR("COMMAND"));
  DEFKEY(default_profile, "S-F2", CMD_STR("PAGE"), CMD_STR("UP"), CMD_SEP,
         CMD_STR("TOP"), CMD_STR("EDGE"), CMD_SEP,
         CMD_STR("UP"), CMD_INT(2), CMD_SEP,
         CMD_STR("CURSOR"), CMD_STR("COMMAND"));
  DEFKEY(default_profile, "S-F3", CMD_STR("REFLOW"));
  DEFKEY(default_profile, "S-F4", CMD_STR("UNDO"));
  DEFKEY(default_profile, "S-F5", CMD_NULL);
  DEFKEY(default_profile, "S-F6",
         CMD_STR("ERASE"), CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("LINE"));
  DEFKEY(default_profile, "S-F7", CMD_STR("SHIFT"), CMD_STR("LEFT"));
  DEFKEY(default_profile, "S-F8", CMD_STR("SHIFT"), CMD_STR("RIGHT"));
  DEFKEY(default_profile, "S-F9", CMD_STR("DIR"));

  // KMP continue loading profile from here
  DEFKEY(default_profile, "S-F10", CMD_NULL);

  DEFKEY(default_profile, "S-UP", CMD_STR("MOVE"), CMD_STR("SPLITTER"), CMD_STR("UP"), CMD_INT(2));
  DEFKEY(default_profile, "S-DOWN", CMD_STR("MOVE"), CMD_STR("SPLITTER"), CMD_STR("DOWN"), CMD_INT(2));
  DEFKEY(default_profile, "S-LEFT", CMD_STR("MOVE"), CMD_STR("SPLITTER"), CMD_STR("LEFT"), CMD_INT(2));
  DEFKEY(default_profile, "S-RIGHT", CMD_STR("MOVE"), CMD_STR("SPLITTER"), CMD_STR("RIGHT"), CMD_INT(2));

  DEFKEY(default_profile, "A-A", CMD_NULL);
  DEFKEY(default_profile, "A-B", CMD_STR("MARK"), CMD_STR("BLOCK"));
  DEFKEY(default_profile, "A-C", CMD_STR("MARK"), CMD_STR("CHAR"));
  DEFKEY(default_profile, "A-D", CMD_STR("BEGIN"), CMD_STR("MARK"), CMD_SEP, CMD_STR("DELETE"), CMD_STR("MARK"));
  DEFKEY(default_profile, "A-E", CMD_STR("END"), CMD_STR("MARK"));
  DEFKEY(default_profile, "A-F", CMD_STR("FILL"), CMD_STR("MARK"));
  DEFKEY(default_profile, "A-G", CMD_NULL);
  DEFKEY(default_profile, "A-H", CMD_NULL);
  DEFKEY(default_profile, "A-I", CMD_NULL);
  DEFKEY(default_profile, "A-J",
         CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("DOWN"), CMD_SEP,
         CMD_STR("FIRST"), CMD_STR("NONBLANK"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("BLOCK"), CMD_SEP,
         CMD_STR("END"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("END"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("BLOCK"), CMD_SEP,
         CMD_STR("UP"), CMD_SEP,
         CMD_STR("END"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("END"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("RIGHT"), CMD_SEP, CMD_STR("RIGHT"), CMD_SEP,
         CMD_STR("COPY"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("DOWN"), CMD_SEP,
         CMD_STR("UNMARK"), CMD_SEP,
         CMD_STR("DELETE"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("UP"), CMD_SEP,
         CMD_STR("POP"), CMD_STR("MARK"));
  DEFKEY(default_profile, "A-K", CMD_NULL);
  DEFKEY(default_profile, "A-L", CMD_STR("MARK"), CMD_STR("LINE"));
  DEFKEY(default_profile, "A-M", CMD_STR("MOVE"), CMD_STR("MARK"));
  DEFKEY(default_profile, "A-N", CMD_NULL);
  DEFKEY(default_profile, "A-O", CMD_STR("OVERLAY"), CMD_STR("BLOCK"));
  DEFKEY(default_profile, "A-P", 
         CMD_STR("CURSOR"), CMD_STR("DATA"), CMD_SEP,
         CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("FIND"), CMD_STR("BLANK"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("UP"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("REFLOW"), CMD_SEP,
         CMD_STR("END"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("DOWN"), CMD_SEP,
         CMD_STR("TAB"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("POP"), CMD_STR("MARK"));
  DEFKEY(default_profile, "A-Q", CMD_NULL);
  DEFKEY(default_profile, "A-R", CMD_STR("REDRAW"));
  DEFKEY(default_profile, "A-T", CMD_NULL);
  DEFKEY(default_profile, "A-U", CMD_STR("UNMARK"));
  DEFKEY(default_profile, "A-V", CMD_NULL);
  DEFKEY(default_profile, "A-W", 
         CMD_STR("UNMARK"), CMD_SEP,
         CMD_STR("TAB"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("CHAR"), CMD_SEP,
         CMD_STR("END"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("RIGHT"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("CHAR"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("MARK"));
  DEFKEY(default_profile, "A-X", CMD_NULL);
  DEFKEY(default_profile, "A-Y", CMD_STR("BEGIN"), CMD_STR("MARK"));
  DEFKEY(default_profile, "A-Z", CMD_STR("COPY"), CMD_STR("MARK"));

  DEFKEY(default_profile, "A-F1", CMD_STR("?"), CMD_STR("MARGINS"), CMD_SEP, CMD_STR("CURSOR"), CMD_STR("COMMAND"), CMD_SEP, CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "A-F2", CMD_NULL);
  DEFKEY(default_profile, "A-F3", CMD_NULL);
  DEFKEY(default_profile, "A-F4", CMD_NULL);
  DEFKEY(default_profile, "A-F5", CMD_STR("?"), CMD_STR("TABS"), CMD_SEP, CMD_STR("CURSOR"), CMD_STR("COMMAND"), CMD_SEP, CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "A-F6", CMD_NULL);
  DEFKEY(default_profile, "A-F7", CMD_STR("?"), CMD_STR("TABEXPAND"), CMD_SEP, CMD_STR("CURSOR"), CMD_STR("COMMAND"), CMD_SEP, CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "A-F8", CMD_STR("?"), CMD_STR("BLANKCOMPRESS"), CMD_SEP, CMD_STR("CURSOR"), CMD_STR("COMMAND"), CMD_SEP, CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "A-F9", CMD_STR("?"), CMD_STR("SEARCHCASE"), CMD_SEP, CMD_STR("CURSOR"), CMD_STR("COMMAND"), CMD_SEP, CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "A-F10", CMD_STR("?"), CMD_STR("CHAR"), CMD_SEP, CMD_STR("CURSOR"), CMD_STR("COMMAND"), CMD_SEP, CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(default_profile, "A-F11", CMD_NULL);
  DEFKEY(default_profile, "A-F12", CMD_NULL);

  DEFKEY(default_profile, "A-0", CMD_NULL);
  DEFKEY(default_profile, "A-1", CMD_NULL);
  DEFKEY(default_profile, "A-2", CMD_NULL);
  DEFKEY(default_profile, "A-3", CMD_NULL);
  DEFKEY(default_profile, "A-4", CMD_NULL);
  DEFKEY(default_profile, "A-5", CMD_NULL);
  DEFKEY(default_profile, "A-6", CMD_NULL);
  DEFKEY(default_profile, "A-7", CMD_NULL);
  DEFKEY(default_profile, "A-8", CMD_NULL);
  DEFKEY(default_profile, "A-9", CMD_NULL);

  DEFKEY(default_profile, "A-LEFT", CMD_STR("LEFT"), CMD_INT(40));
  DEFKEY(default_profile, "A-RIGHT", CMD_STR("RIGHT"), CMD_INT(40));
  DEFKEY(default_profile, "A-UP", CMD_STR("UP"), CMD_INT(20));
  DEFKEY(default_profile, "A-DOWN", CMD_STR("DOWN"), CMD_INT(20));

  DEFKEY(default_profile, "S-A-F1", CMD_NULL);
  DEFKEY(default_profile, "S-A-F2", CMD_NULL);
  DEFKEY(default_profile, "S-A-F3", CMD_NULL);
  DEFKEY(default_profile, "S-A-F4", CMD_NULL);
  DEFKEY(default_profile, "S-A-F5", CMD_NULL);
  DEFKEY(default_profile, "S-A-F6", CMD_NULL);
  DEFKEY(default_profile, "S-A-F7", CMD_NULL);
  DEFKEY(default_profile, "S-A-F8", CMD_NULL);
  DEFKEY(default_profile, "S-A-F9", CMD_NULL);
  DEFKEY(default_profile, "S-A-F10", CMD_NULL);
  DEFKEY(default_profile, "S-A-F11", CMD_NULL);
  DEFKEY(default_profile, "S-A-F12", CMD_NULL);

  DEFKEY(default_profile, "A-MINUS", CMD_NULL);
  DEFKEY(default_profile, "A-EQUALS", CMD_NULL);

  //DEFKEY(default_profile, "A-F9", CMD_STR("BEGIN"), CMD_STR("WORD"));
  //DEFKEY(default_profile, "A-F10", CMD_STR("FIND"), CMD_STR("BLANK"), CMD_STR("LINE"));

  // 
  //DEFKEY(default_profile, "C-Q", CMD_STR("QQUIT"));

#endif

  TRACE_EXIT;
}


