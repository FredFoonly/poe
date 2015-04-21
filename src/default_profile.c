
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

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
  tabexpand = true;
  blankcompress = false;
  margins_set(&default_margins, 0, 255, 0);
  tabs_set(&default_tabstops, 0, 4, NULL);
  tabexpand = 8;
  DEFKEY(dflt_data_profile, "RESIZE", CMD_STR("RESIZE"), CMD_STR("DISPLAY"));
  DEFKEY(dflt_data_profile, "UP", CMD_STR("UP"));
  DEFKEY(dflt_data_profile, "DOWN", CMD_STR("DOWN"));
  DEFKEY(dflt_data_profile, "LEFT", CMD_STR("LEFT"));
  DEFKEY(dflt_data_profile, "RIGHT", CMD_STR("RIGHT"));
  /* DEFKEY(dflt_data_profile, "LEFT", CMD_STR("LEFT"), CMD_STR("WRAP")); */
  /* DEFKEY(dflt_data_profile, "RIGHT", CMD_STR("RIGHT"), CMD_STR("WRAP")); */
  DEFKEY(dflt_data_profile, "PGUP", CMD_STR("PAGE"), CMD_STR("UP"));
  DEFKEY(dflt_data_profile, "PGDN", CMD_STR("PAGE"), CMD_STR("DOWN"));
  DEFKEY(dflt_data_profile, "HOME", CMD_STR("BEGIN"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "END", CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "INSERT", CMD_STR("INSERT"), CMD_STR("TOGGLE"));
  DEFKEY(dflt_data_profile, "DEL", CMD_STR("DELETE"), CMD_STR("CHAR"));

  // Dumb ENTER
  //  DEFKEY(dflt_data_profile, "ENTER", CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("DOWN"), CMD_SEP, CMD_STR("INDENT"));
  // Smart ENTER
  DEFKEY(dflt_data_profile, "ENTER",
         CMD_STR("INSERT"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("BACKTAB"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("POP"), CMD_STR("MARK"));

  DEFKEY(dflt_data_profile, "BACKSPACE", CMD_STR("RUBOUT"));
  DEFKEY(dflt_data_profile, "ESC", CMD_STR("COMMAND"), CMD_STR("TOGGLE"));
  DEFKEY(dflt_data_profile, "TAB", CMD_STR("TAB"));
  //DEFKEY(dflt_data_profile, "TAB", CMD_STR("TAB"), CMD_STR("WORD"));
  DEFKEY(dflt_data_profile, "A", CMD_STR("CHR"), CMD_INT('a'));
  DEFKEY(dflt_data_profile, "B", CMD_STR("CHR"), CMD_INT('b'));
  DEFKEY(dflt_data_profile, "C", CMD_STR("CHR"), CMD_INT('c'));
  DEFKEY(dflt_data_profile, "D", CMD_STR("CHR"), CMD_INT('d'));
  DEFKEY(dflt_data_profile, "E", CMD_STR("CHR"), CMD_INT('e'));
  DEFKEY(dflt_data_profile, "F", CMD_STR("CHR"), CMD_INT('f'));
  DEFKEY(dflt_data_profile, "G", CMD_STR("CHR"), CMD_INT('g'));
  DEFKEY(dflt_data_profile, "H", CMD_STR("CHR"), CMD_INT('h'));
  DEFKEY(dflt_data_profile, "I", CMD_STR("CHR"), CMD_INT('i'));
  DEFKEY(dflt_data_profile, "J", CMD_STR("CHR"), CMD_INT('j'));
  DEFKEY(dflt_data_profile, "K", CMD_STR("CHR"), CMD_INT('k'));
  DEFKEY(dflt_data_profile, "L", CMD_STR("CHR"), CMD_INT('l'));
  DEFKEY(dflt_data_profile, "M", CMD_STR("CHR"), CMD_INT('m'));
  DEFKEY(dflt_data_profile, "N", CMD_STR("CHR"), CMD_INT('n'));
  DEFKEY(dflt_data_profile, "O", CMD_STR("CHR"), CMD_INT('o'));
  DEFKEY(dflt_data_profile, "P", CMD_STR("CHR"), CMD_INT('p'));
  DEFKEY(dflt_data_profile, "Q", CMD_STR("CHR"), CMD_INT('q'));
  DEFKEY(dflt_data_profile, "R", CMD_STR("CHR"), CMD_INT('r'));
  DEFKEY(dflt_data_profile, "S", CMD_STR("CHR"), CMD_INT('s'));
  DEFKEY(dflt_data_profile, "T", CMD_STR("CHR"), CMD_INT('t'));
  DEFKEY(dflt_data_profile, "U", CMD_STR("CHR"), CMD_INT('u'));
  DEFKEY(dflt_data_profile, "V", CMD_STR("CHR"), CMD_INT('v'));
  DEFKEY(dflt_data_profile, "W", CMD_STR("CHR"), CMD_INT('w'));
  DEFKEY(dflt_data_profile, "X", CMD_STR("CHR"), CMD_INT('x'));
  DEFKEY(dflt_data_profile, "Y", CMD_STR("CONFIRM"), CMD_STR("CHANGE"), CMD_SEP, CMD_STR("CHR"), CMD_INT('y'));
  DEFKEY(dflt_data_profile, "Z", CMD_STR("CHR"), CMD_INT('z'));

  DEFKEY(dflt_data_profile, "F1", CMD_STR("E"), CMD_STR("~/.poe/poe.hlp"));
  DEFKEY(dflt_data_profile, "F2", CMD_STR("CURSOR"),CMD_STR("COMMAND"), CMD_SEP, CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"), CMD_SEP, CMD_STR("STR"), CMD_STR("save "));
  DEFKEY(dflt_data_profile, "F3", CMD_STR("CURSOR"), CMD_STR("COMMAND"), CMD_SEP, CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"), CMD_SEP, CMD_STR("STR"), CMD_STR("file "));
  DEFKEY(dflt_data_profile, "F4", CMD_STR("QUIT"));


  DEFKEY(dflt_data_profile, "F5", CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "F6", CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "F7", CMD_STR("CURSOR"),CMD_STR("COMMAND"), CMD_SEP, CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP, CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"), CMD_SEP, CMD_STR("STR"), CMD_STR("print"));
  DEFKEY(dflt_data_profile, "F8", CMD_STR("NEXT"), CMD_STR("FILE"));
  DEFKEY(dflt_data_profile, "F9", CMD_STR("INSERT"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "F10", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_SEP,
                CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP,
                CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP,
                CMD_STR("BACKTAB"), CMD_STR("WORD"), CMD_SEP,
                CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP,
                CMD_STR("BEGIN"), CMD_STR("WORD"), CMD_SEP,
                CMD_STR("BEGIN"), CMD_STR("MARK"), CMD_SEP,
                CMD_STR("POP"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "0", CMD_STR("CHR"), CMD_INT('0'));
  DEFKEY(dflt_data_profile, "1", CMD_STR("CHR"), CMD_INT('1'));
  DEFKEY(dflt_data_profile, "2", CMD_STR("CHR"), CMD_INT('2'));
  DEFKEY(dflt_data_profile, "3", CMD_STR("CHR"), CMD_INT('3'));
  DEFKEY(dflt_data_profile, "4", CMD_STR("CHR"), CMD_INT('4'));
  DEFKEY(dflt_data_profile, "5", CMD_STR("CHR"), CMD_INT('5'));
  DEFKEY(dflt_data_profile, "6", CMD_STR("CHR"), CMD_INT('6'));
  DEFKEY(dflt_data_profile, "7", CMD_STR("CHR"), CMD_INT('7'));
  DEFKEY(dflt_data_profile, "8", CMD_STR("CHR"), CMD_INT('8'));
  DEFKEY(dflt_data_profile, "9", CMD_STR("CHR"), CMD_INT('9'));

  DEFKEY(dflt_data_profile, "MINUS", CMD_STR("CHR"), CMD_INT('-'));
  DEFKEY(dflt_data_profile, "EQUALS", CMD_STR("CHR"), CMD_INT('='));
  DEFKEY(dflt_data_profile, "LBRACKET", CMD_STR("CHR"), CMD_INT('['));
  DEFKEY(dflt_data_profile, "RBRACKET", CMD_STR("CHR"), CMD_INT(']'));
  DEFKEY(dflt_data_profile, "SEMI", CMD_STR("CHR"), CMD_INT(';'));
  DEFKEY(dflt_data_profile, "QUOTE", CMD_STR("CHR"), CMD_INT('\''));
  DEFKEY(dflt_data_profile, "BACKQUOTE", CMD_STR("CHR"), CMD_INT('`'));
  DEFKEY(dflt_data_profile, "COMMA", CMD_STR("CHR"), CMD_INT(','));
  DEFKEY(dflt_data_profile, "PERIOD", CMD_STR("CHR"), CMD_INT('.'));
  DEFKEY(dflt_data_profile, "SLASH", CMD_STR("CHR"), CMD_INT('/'));
  DEFKEY(dflt_data_profile, "BACKSLASH", CMD_STR("CHR"), CMD_INT('\\'));
  DEFKEY(dflt_data_profile, "SPACE", CMD_STR("CHR"), CMD_INT(' '));

  DEFKEY(dflt_data_profile, "C-LEFT", CMD_STR("BACKTAB"), CMD_STR("WORD"));
  DEFKEY(dflt_data_profile, "C-RIGHT", CMD_STR("TAB"), CMD_STR("WORD"));
  DEFKEY(dflt_data_profile, "C-UP", CMD_STR("UP"), CMD_INT(20));
  DEFKEY(dflt_data_profile, "C-DOWN", CMD_STR("DOWN"), CMD_INT(20));

  DEFKEY(dflt_data_profile, "C-PGDN", CMD_STR("BOTTOM"), CMD_STR("EDGE"));
  DEFKEY(dflt_data_profile, "C-PGUP", CMD_STR("TOP"), CMD_STR("EDGE"));
  DEFKEY(dflt_data_profile, "C-HOME", CMD_STR("TOP"));
  DEFKEY(dflt_data_profile, "C-END", CMD_STR("BOTTOM"));
  //DEFKEY(dflt_data_profile, "C-ENTER", CMD_STR("EXECUTE")); // C-ENTER doesn't seem to be detected by ncurses
  DEFKEY(dflt_data_profile, "C-BACKSPACE", CMD_STR("DELETE"), CMD_STR("LINE"));

  DEFKEY(dflt_data_profile, "C-A", CMD_STR("BEGIN"), CMD_STR("LINE"));
  // DEFKEY(dflt_data_profile, "C-B", CMD_STR("BEGIN"), CMD_STR("WORD"));
  DEFKEY(dflt_data_profile, "C-C", CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP, 
         CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("CENTER"), CMD_STR("IN"), CMD_STR("MARGINS"), CMD_SEP,
         CMD_STR("UNMARK"), CMD_SEP,
         CMD_STR("POP"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "C-D", CMD_NULL);
  // DEFKEY(dflt_data_profile, "C-E", CMD_STR("END"), CMD_STR("WORD"));
  DEFKEY(dflt_data_profile, "C-E", CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "C-F", CMD_STR("COPY"), CMD_STR("FROM"), CMD_STR("COMMAND"));
  DEFKEY(dflt_data_profile, "C-G", CMD_NULL);
  DEFKEY(dflt_data_profile, "C-H", CMD_STR("RUBOUT"));
  DEFKEY(dflt_data_profile, "C-K", CMD_STR("ERASE"), CMD_STR("END"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "C-L", CMD_STR("RESIZE"), CMD_STR("DISPLAY"), CMD_SEP, CMD_STR("CENTER"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "C-M", CMD_NULL);
  DEFKEY(dflt_data_profile, "C-N", CMD_STR("DOWN"), CMD_INT(60));
  DEFKEY(dflt_data_profile, "C-O", CMD_NULL);
  DEFKEY(dflt_data_profile, "C-P", CMD_STR("PRINT"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "C-R",
         CMD_STR("PUSH"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("LINE"), CMD_SEP, 
         CMD_STR("COPY"), CMD_STR("MARK"), CMD_SEP,
         CMD_STR("POP"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "C-S", CMD_STR("SPLIT"), CMD_STR("SCREEN"));
  DEFKEY(dflt_data_profile, "C-T", CMD_STR("COPY"), CMD_STR("TO"), CMD_STR("COMMAND"));
  DEFKEY(dflt_data_profile, "C-U", CMD_STR("EDIT"), CMD_STR(".UNNAMED"));
  DEFKEY(dflt_data_profile, "C-V", CMD_STR("NEXT"), CMD_STR("VIEW"));
  DEFKEY(dflt_data_profile, "C-W", CMD_STR("NEXT"), CMD_STR("WINDOW"));
  DEFKEY(dflt_data_profile, "C-X", CMD_NULL);
  DEFKEY(dflt_data_profile, "C-Y", CMD_NULL);
  DEFKEY(dflt_data_profile, "C-Z", CMD_STR("ZOOM"), CMD_STR("WINDOW"));

  DEFKEY(dflt_data_profile, "C-SPACE", CMD_STR("EXECUTE")); // rebound here because C-ENTER
                                // doesn't seem to be detected by
                                // ncurses
  DEFKEY(dflt_data_profile, "C-F1", CMD_STR("INSERT"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "C-F2", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(2));
  DEFKEY(dflt_data_profile, "C-F3", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(3));
  DEFKEY(dflt_data_profile, "C-F4", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(4));
  DEFKEY(dflt_data_profile, "C-F5", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(5));
  DEFKEY(dflt_data_profile, "C-F6", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(6));
  DEFKEY(dflt_data_profile, "C-F7", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(7));
  DEFKEY(dflt_data_profile, "C-F8", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(8));
  DEFKEY(dflt_data_profile, "C-F9", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(9));
  DEFKEY(dflt_data_profile, "C-F10", CMD_STR("INSERT"), CMD_STR("LINE"), CMD_INT(10));


  //DEFKEY(dflt_data_profile, "SHIFT-TAB",  CMD_STR("BACKTAB"));
  DEFKEY(dflt_data_profile, "S-TAB",  CMD_STR("BACKTAB"), CMD_STR("WORD"));
  DEFKEY(dflt_data_profile, "S-A", CMD_STR("CHR"), CMD_INT('A'));
  DEFKEY(dflt_data_profile, "S-B", CMD_STR("CHR"), CMD_INT('B'));
  DEFKEY(dflt_data_profile, "S-C", CMD_STR("CHR"), CMD_INT('C'));
  DEFKEY(dflt_data_profile, "S-D", CMD_STR("CHR"), CMD_INT('D'));
  DEFKEY(dflt_data_profile, "S-E", CMD_STR("CHR"), CMD_INT('E'));
  DEFKEY(dflt_data_profile, "S-F", CMD_STR("CHR"), CMD_INT('F'));
  DEFKEY(dflt_data_profile, "S-G", CMD_STR("CHR"), CMD_INT('G'));
  DEFKEY(dflt_data_profile, "S-H", CMD_STR("CHR"), CMD_INT('H'));
  DEFKEY(dflt_data_profile, "S-I", CMD_STR("CHR"), CMD_INT('I'));
  DEFKEY(dflt_data_profile, "S-J", CMD_STR("CHR"), CMD_INT('J'));
  DEFKEY(dflt_data_profile, "S-K", CMD_STR("CHR"), CMD_INT('K'));
  DEFKEY(dflt_data_profile, "S-L", CMD_STR("CHR"), CMD_INT('L'));
  DEFKEY(dflt_data_profile, "S-M", CMD_STR("CHR"), CMD_INT('M'));
  DEFKEY(dflt_data_profile, "S-N", CMD_STR("CHR"), CMD_INT('N'));
  DEFKEY(dflt_data_profile, "S-O", CMD_STR("CHR"), CMD_INT('O'));
  DEFKEY(dflt_data_profile, "S-P", CMD_STR("CHR"), CMD_INT('P'));
  DEFKEY(dflt_data_profile, "S-Q", CMD_STR("CHR"), CMD_INT('Q'));
  DEFKEY(dflt_data_profile, "S-R", CMD_STR("CHR"), CMD_INT('R'));
  DEFKEY(dflt_data_profile, "S-S", CMD_STR("CHR"), CMD_INT('S'));
  DEFKEY(dflt_data_profile, "S-T", CMD_STR("CHR"), CMD_INT('T'));
  DEFKEY(dflt_data_profile, "S-U", CMD_STR("CHR"), CMD_INT('U'));
  DEFKEY(dflt_data_profile, "S-V", CMD_STR("CHR"), CMD_INT('V'));
  DEFKEY(dflt_data_profile, "S-W", CMD_STR("CHR"), CMD_INT('W'));
  DEFKEY(dflt_data_profile, "S-X", CMD_STR("CHR"), CMD_INT('X'));
  DEFKEY(dflt_data_profile, "S-Y", CMD_STR("CONFIRM"), CMD_STR("CHANGE"), CMD_SEP, CMD_STR("CHR"), CMD_INT('Y'));
  DEFKEY(dflt_data_profile, "S-Z", CMD_STR("CHR"), CMD_INT('Z'));
 
  DEFKEY(dflt_data_profile, "S-F1", CMD_STR("PAGE"), CMD_STR("DOWN"), CMD_SEP,
         CMD_STR("BOTTOM"), CMD_STR("EDGE"), CMD_SEP,
         CMD_STR("DOWN"), CMD_INT(2), CMD_SEP,
         CMD_STR("CURSOR"), CMD_STR("COMMAND"));
  DEFKEY(dflt_data_profile, "S-F2", CMD_STR("PAGE"), CMD_STR("UP"), CMD_SEP,
         CMD_STR("TOP"), CMD_STR("EDGE"), CMD_SEP,
         CMD_STR("UP"), CMD_INT(2), CMD_SEP,
         CMD_STR("CURSOR"), CMD_STR("COMMAND"));
  DEFKEY(dflt_data_profile, "S-F3", CMD_STR("REFLOW"));
  DEFKEY(dflt_data_profile, "S-F4", CMD_STR("UNDO"));
  DEFKEY(dflt_data_profile, "S-F5", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-F6",
         CMD_STR("ERASE"), CMD_STR("BEGIN"), CMD_STR("LINE"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "S-F7", CMD_STR("SHIFT"), CMD_STR("LEFT"));
  DEFKEY(dflt_data_profile, "S-F8", CMD_STR("SHIFT"), CMD_STR("RIGHT"));
  DEFKEY(dflt_data_profile, "S-F9", CMD_STR("DIR"));

  // KMP continue loading profile from here
  DEFKEY(dflt_data_profile, "S-F10", CMD_NULL);

  DEFKEY(dflt_data_profile, "S-0", CMD_STR("CHR"), CMD_INT(')'));
  DEFKEY(dflt_data_profile, "S-1", CMD_STR("CHR"), CMD_INT('!'));
  DEFKEY(dflt_data_profile, "S-2", CMD_STR("CHR"), CMD_INT('@'));
  DEFKEY(dflt_data_profile, "S-3", CMD_STR("CHR"), CMD_INT('#'));
  DEFKEY(dflt_data_profile, "S-4", CMD_STR("CHR"), CMD_INT('$'));
  DEFKEY(dflt_data_profile, "S-5", CMD_STR("CHR"), CMD_INT('%'));
  DEFKEY(dflt_data_profile, "S-6", CMD_STR("CHR"), CMD_INT('^'));
  DEFKEY(dflt_data_profile, "S-7", CMD_STR("CHR"), CMD_INT('&'));
  DEFKEY(dflt_data_profile, "S-8", CMD_STR("CHR"), CMD_INT('*'));
  DEFKEY(dflt_data_profile, "S-9", CMD_STR("CHR"), CMD_INT('('));

  DEFKEY(dflt_data_profile, "S-MINUS",       CMD_STR("CHR"), CMD_INT('_'));
  DEFKEY(dflt_data_profile, "S-EQUALS",      CMD_STR("CHR"), CMD_INT('+'));
  DEFKEY(dflt_data_profile, "S-LBRACKET",     CMD_STR("CHR"), CMD_INT('{'));
  DEFKEY(dflt_data_profile, "S-RBRACKET",     CMD_STR("CHR"), CMD_INT('}'));
  DEFKEY(dflt_data_profile, "S-SEMI",        CMD_STR("CHR"), CMD_INT(':'));
  DEFKEY(dflt_data_profile, "S-QUOTE",       CMD_STR("CHR"), CMD_INT('"'));
  DEFKEY(dflt_data_profile, "S-BACKQUOTE",  CMD_STR("CHR"), CMD_INT('~'));
  DEFKEY(dflt_data_profile, "S-COMMA",       CMD_STR("CHR"), CMD_INT('<'));
  DEFKEY(dflt_data_profile, "S-PERIOD",      CMD_STR("CHR"), CMD_INT('>'));
  DEFKEY(dflt_data_profile, "S-SLASH",       CMD_STR("CHR"), CMD_INT('?'));
  DEFKEY(dflt_data_profile, "S-BACKSLASH",  CMD_STR("CHR"), CMD_INT('|'));

  DEFKEY(dflt_data_profile, "S-UP", CMD_STR("DECR"), CMD_STR("HSPLIT"), CMD_INT(2));
  DEFKEY(dflt_data_profile, "S-DOWN", CMD_STR("INCR"), CMD_STR("HSPLIT"), CMD_INT(2));
  DEFKEY(dflt_data_profile, "S-LEFT", CMD_STR("DECR"), CMD_STR("VSPLIT"), CMD_INT(2));
  DEFKEY(dflt_data_profile, "S-RIGHT", CMD_STR("INCR"), CMD_STR("VSPLIT"), CMD_INT(2));

  DEFKEY(dflt_data_profile, "A-A", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-B", CMD_STR("MARK"), CMD_STR("BLOCK"));
  DEFKEY(dflt_data_profile, "A-C", CMD_STR("MARK"), CMD_STR("CHAR"));
  DEFKEY(dflt_data_profile, "A-D", CMD_STR("BEGIN"), CMD_STR("MARK"), CMD_SEP, CMD_STR("DELETE"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "A-E", CMD_STR("END"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "A-F", CMD_STR("FILL"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "A-G", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-H", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-I", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-J",
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
  DEFKEY(dflt_data_profile, "A-K", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-L", CMD_STR("MARK"), CMD_STR("LINE"));
  DEFKEY(dflt_data_profile, "A-M", CMD_STR("MOVE"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "A-N", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-O", CMD_STR("OVERLAY"), CMD_STR("BLOCK"));
  DEFKEY(dflt_data_profile, "A-P", 
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
  DEFKEY(dflt_data_profile, "A-Q", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-R", CMD_STR("REDRAW"));
  DEFKEY(dflt_data_profile, "A-S", CMD_STR("SPLIT"));
  DEFKEY(dflt_data_profile, "A-T", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-U", CMD_STR("UNMARK"));
  DEFKEY(dflt_data_profile, "A-V", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-W", 
         CMD_STR("UNMARK"), CMD_SEP,
         CMD_STR("TAB"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("CHAR"), CMD_SEP,
         CMD_STR("END"), CMD_STR("WORD"), CMD_SEP,
         CMD_STR("RIGHT"), CMD_SEP,
         CMD_STR("MARK"), CMD_STR("CHAR"), CMD_SEP,
         CMD_STR("BEGIN"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "A-X", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-Y", CMD_STR("BEGIN"), CMD_STR("MARK"));
  DEFKEY(dflt_data_profile, "A-Z", CMD_STR("COPY"), CMD_STR("MARK"));

  DEFKEY(dflt_data_profile, "A-F1", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F2", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F3", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F4", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F5", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F6", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F7", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F8", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F9", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F10", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F11", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-F12", CMD_NULL);

  DEFKEY(dflt_data_profile, "A-0", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-1", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-2", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-3", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-4", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-5", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-6", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-7", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-8", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-9", CMD_NULL);

  DEFKEY(dflt_data_profile, "A-LEFT", CMD_STR("LEFT"), CMD_INT(40));
  DEFKEY(dflt_data_profile, "A-RIGHT", CMD_STR("RIGHT"), CMD_INT(40));
  DEFKEY(dflt_data_profile, "A-UP", CMD_STR("UP"), CMD_INT(20));
  DEFKEY(dflt_data_profile, "A-DOWN", CMD_STR("DOWN"), CMD_INT(20));

  DEFKEY(dflt_data_profile, "S-A-F1", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F2", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F3", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F4", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F5", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F6", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F7", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F8", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F9", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F10", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F11", CMD_NULL);
  DEFKEY(dflt_data_profile, "S-A-F12", CMD_NULL);

  DEFKEY(dflt_data_profile, "A-MINUS", CMD_NULL);
  DEFKEY(dflt_data_profile, "A-EQUALS", CMD_NULL);

  //DEFKEY(dflt_data_profile, "A-F9", CMD_STR("BEGIN"), CMD_STR("WORD"));
  //DEFKEY(dflt_data_profile, "A-F10", CMD_STR("FIND"), CMD_STR("BLANK"), CMD_STR("LINE"));

  // 
  //DEFKEY(dflt_data_profile, "C-Q", CMD_STR("QQUIT"));

  DEFKEY(dflt_cmd_profile, "ENTER", CMD_STR("EXECUTE"));
  TRACE_EXIT;
}


