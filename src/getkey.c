
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ncurses.h>

#include "utils.h"
#include "trace.h"
#include "logging.h"
#include "poe_err.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "tabstops.h"
#include "margins.h"
#include "key_interp.h"
#include "getkey.h"
#include "editor_globals.h"
#include "view.h"
#include "window.h"

//
// key decoding
//

#define MAX_KEYSEQ_NAME_LEN (64)

struct keyxlat_t {
  int code;
  const char* name;
};

struct keyxlat_t _misc_key_xlat[] = {
  {27, "ESC"},
  {KEY_RESIZE, "RESIZE"},
  {0, "C-SPACE"},  {1, "C-a"},  {2, "C-b"},  {3, "C-c"},  {4, "C-d"},
  {5, "C-e"},  {6, "C-f"},  {7, "C-g"},  {8, "C-h"},  {9, "TAB"},
  {10, "ENTER"},  {11, "C-k"},  {12, "C-l"},  {13, "C-m"},  {14, "C-n"},
  {15, "C-o"},  {16, "C-p"},  {17, "C-q"},  {18, "C-r"},  {19, "C-s"},
  {20, "C-t"},  {21, "C-u"},  {22, "C-v"},  {23, "C-w"},  {24, "C-x"},
  {25, "C-y"},  {26, "C-z"},
  {28, "FS"},  {29, "GS"},  {30, "RS"},  {31, "US"},
  {32, "SPACE"},
  {127, "BACKSPACE"},
  {'`', "BACKQUOTE"},
  {'~', "S-BACKQUOTE"},
  {'!', "S-1"},  {'@', "S-2"},  {'#', "S-3"},  {'$', "S-4"},  {'%', "S-5"},
  {'^', "S-6"},  {'&', "S-7"},  {'*', "S-8"},  {'(', "S-9"},  {')', "S-0"},
  {'-', "MINUS"},  {'_', "S-MINUS"},
  {'=', "EQUALS"},  {'+', "S-EQUALS"},
  {'[', "LBRACKET"},  {'{', "S-LBRACKET"},
  {']', "RBRACKET"},  {'}', "S-RBRACKET"},
  {'\\', "BACKSLASH"},  {'|', "S-BACKSLASH"},
  {';', "SEMI"},  {':', "S-SEMI"},
  {'\'', "QUOTE"},  {'"', "S-QUOTE"},
  {'/', "SLASH"},  {'?', "S-SLASH"},
  {',', "COMMA"},  {'<', "S-COMMA"},
  {'.', "PERIOD"},  {'>', "S-PERIOD"},
  {KEY_DOWN, "DOWN"},  {KEY_UP, "UP"},  {KEY_LEFT, "LEFT"},  {KEY_RIGHT, "RIGHT"},
  {KEY_HOME, "HOME"},  {KEY_END, "END"},
  {KEY_BACKSPACE, "BACKSPACE"},
  {161, "A-S-1"},  {192, "A-S-2"},  {163, "A-S-3"},  {164, "A-S-4"},  {165, "A-S-5"},
  {222, "A-S-6"},  {166, "A-S-7"},  {170, "A-S-8"},  {168, "A-S-9"},  {169, "A-S-0"},
  {224, "A-BACKQUOTE"},  {254, "A-S-BACKQUOTE"},
  {173, "A-MINUS"},  {223, "A-S-MINUS"},
  {189, "A-EQUALS"},  {171, "A-S-EQUALS"},
  {219, "A-LBRACKET"},  {251, "A-S-LBRACKET"},
  {221, "A-RBRACKET"},  {253, "A-S-RBRACKET"},
  {187, "A-SEMI"},  {186, "A-S-SEMI"},
  {167, "A-QUOTE"},  {162, "A-S-QUOTE"},
  {172, "A-COMMA"},  {188, "A-S-COMMA"},
  {174, "A-PERIOD"},  {190, "A-S-PERIOD"},
  {175, "A-SLASH"},  {191, "A-S-SLASH"},
  {220, "A-BACKSLASH"},  {252, "A-S-BACKSLASH"},
  {255, "A-BACKSPACE"},
  {KEY_F(1), "F1"},  {KEY_F(2), "F2"},  {KEY_F(3), "F3"},  {KEY_F(4), "F4"},  {KEY_F(5), "F5"},
  {KEY_F(6), "F6"},  {KEY_F(7), "F7"},  {KEY_F(8), "F8"},  {KEY_F(9), "F9"},  {KEY_F(10), "F10"},
  {KEY_F(11), "S-F1"},  {KEY_F(12), "S-F2"},  {KEY_F(13), "S-F3"},  {KEY_F(14), "S-F4"},  {KEY_F(15), "S-F5"},
  {KEY_F(16), "S-F6"},  {KEY_F(17), "S-F7"},  {KEY_F(18), "S-F8"},  {KEY_F(19), "S-F9"},  {KEY_F(20), "S-F10"},
  {KEY_F(21), "S-F11"},  {KEY_F(22), "S-F12"},
  {KEY_F(23), "C-F1"},  {KEY_F(24), "C-F2"},  {KEY_F(25), "C-F3"},  {KEY_F(26), "C-F4"},  {KEY_F(27), "C-F5"},
  {KEY_F(28), "C-F6"},  {KEY_F(29), "C-F7"},  {KEY_F(30), "C-F8"},  {KEY_F(31), "C-F9"},  {KEY_F(32), "C-F10"},
  {KEY_F(33), "C-S-F1"},  {KEY_F(34), "C-S-F2"},  {KEY_F(35), "C-S-F3"},  {KEY_F(36), "C-S-F4"},  {KEY_F(37), "C-S-F5"},
  {KEY_F(38), "C-S-F6"},  {KEY_F(39), "C-S-F7"},  {KEY_F(40), "C-S-F8"},  {KEY_F(41), "C-S-F9"},  {KEY_F(42), "C-S-F10"},
  {KEY_F(43), "C-S-F11"},  {KEY_F(44), "C-S-F12"},
  {KEY_F(45), "A-F1"},  {KEY_F(46), "A-F2"},  {KEY_F(47), "A-F3"},  {KEY_F(48), "A-F4"},  {KEY_F(49), "A-F5"},
  {KEY_F(50), "A-F6"},  {KEY_F(51), "A-F7"},  {KEY_F(52), "A-F8"},  {KEY_F(53), "A-F9"},  {KEY_F(54), "A-F10"},
  {KEY_F(55), "A-F11"},  {KEY_F(56), "A-F12"},
  {KEY_DL, "DL"},  {KEY_IL, "IL"},  {KEY_DC, "DEL"},  {KEY_IC, "INSERT"},  {KEY_EIC, "EIC"},
  {KEY_CLEAR, "CLEAR"},  {KEY_EOS, "EOS"},  {KEY_EOL, "C-END"},  {KEY_SF, "S-DOWN"},  {KEY_SR, "S-UP"},
  {KEY_NPAGE, "PGDN"},  {KEY_PPAGE, "PGUP"},  {KEY_STAB, "STAB"},  {KEY_CTAB, "CTAB"},  {KEY_CATAB, "CATAB"},
  {KEY_ENTER, "ENTER"},  {KEY_PRINT, "PRINT"},
  {KEY_LL, "LL"},  {KEY_A1, "A1"},  {KEY_A3, "A3"},  {KEY_B2, "B2"},  {KEY_C1, "C1"},  {KEY_C3, "C3"},
  {KEY_BTAB, "S-TAB"},  {KEY_BEG, "BEG"},  {KEY_CANCEL, "CANCEL"},  {KEY_CLOSE, "CLOSE"},
  {KEY_COMMAND, "COMMAND"},  {KEY_COPY, "COPY"},  {KEY_CREATE, "CREATE"},
  {KEY_EXIT, "EXIT"},  {KEY_FIND, "FIND"},  {KEY_HELP, "HELP"},  {KEY_MARK, "MARK"},
  {KEY_MESSAGE, "MESSAGE"},  {KEY_MOVE, "MOVE"},  {KEY_NEXT, "NEXT"},  {KEY_OPTIONS, "OPTIONS"},
  {KEY_REDO, "REDO"},  {KEY_REFERENCE, "REFERENCE"},  {KEY_REPLACE, "REPLACE"},  {KEY_RESTART, "RESTART"},
  {KEY_RESUME, "RESUME"},  {KEY_SAVE, "SAVE"},  {KEY_SBEG, "S-BEG"},  {KEY_SCANCEL, "S-CANCEL"},
  {KEY_SCOMMAND, "S-COMMAND"},  {KEY_SCOPY, "S-COPY"},  {KEY_SCREATE, "S-CREATE"},  {KEY_SDC, "S-DC"},
  {KEY_SDL, "S-DL"},  {KEY_SELECT, "SELECT"},  {KEY_SEND, "S-END"},  {KEY_SEOL, "S-EOL"},
  {KEY_SEXIT, "S-EXIT"},  {KEY_SFIND, "S-FIND"},  {KEY_SHELP, "S-HELP"},  {KEY_SHOME, "S-HOME"},
  {KEY_SIC, "S-INSERT"},  {KEY_SLEFT, "S-LEFT"},  {KEY_SMESSAGE, "S-MESSAGE"},  {KEY_SMOVE, "S-MOVE"},
  {KEY_SNEXT, "S-NEXT"},  {KEY_SOPTIONS, "S-OPTIONS"},  {KEY_SPREVIOUS, "S-PREVIOUS"},  {KEY_SPRINT, "S-PRINT"},
  {KEY_SREDO, "S-REDO"},  {KEY_SREPLACE, "S-REPLACE"},  {KEY_SRIGHT, "S-RIGHT"},  {KEY_SRSUME, "S-RESUME"},
  {KEY_SSAVE, "S-SAVE"},  {KEY_SSUSPEND, "S-SUSPEND"},  {KEY_SUNDO, "S-UNDO"},  {KEY_SUSPEND, "SUSPEND"},
  {KEY_UNDO, "UNDO"},  {528, "C-UP"},  {566, "C-UP"},  {511, "C-DEL"},
  {514, "C-DOWN"},  {525, "C-DOWN"},  {521, "C-LEFT"},  {545, "C-LEFT"},
  {526, "C-RIGHT"},  {560, "C-RIGHT"},  {517, "C-HOME"},  {535, "C-HOME"},
  {530, "C-END"},  {524, "C-PGUP"},  {555, "C-PGUP"},  {522, "C-PGDN"},
  {550, "C-PGDN"},  {564, "A-UP"},  {523, "A-DOWN"},  {543, "A-LEFT"},
  {558, "A-RIGHT"},  {533, "A-HOME"},  {553, "A-PGUP"},  {548, "A-PGDN"},

  {POE_SPECIAL_KEY+0, "A-S-F1"},  {POE_SPECIAL_KEY+1, "A-S-F2"},
  {POE_SPECIAL_KEY+2, "A-S-F3"},  {POE_SPECIAL_KEY+3, "A-S-F4"},
  {POE_SPECIAL_KEY+4, "A-S-F5"},  {POE_SPECIAL_KEY+5, "A-S-F6"},
  {POE_SPECIAL_KEY+6, "A-S-F7"},  {POE_SPECIAL_KEY+7, "A-S-F8"},
  {POE_SPECIAL_KEY+8, "A-S-F9"},  {POE_SPECIAL_KEY+9, "A-S-F10"},
  {POE_SPECIAL_KEY+10, "A-S-F11"},  {POE_SPECIAL_KEY+11, "A-S-F12"},
};


int _compare_xlats(const void* a, const void* b)
{
  TRACE_ENTER;
  int rval = ((struct keyxlat_t*)a)->code - ((struct keyxlat_t*)b)->code;
  TRACE_RETURN(rval);
}


void init_getkey(void)
{
  TRACE_ENTER;
  qsort(_misc_key_xlat,
        sizeof(_misc_key_xlat)/sizeof(_misc_key_xlat[0]),
        sizeof(_misc_key_xlat[0]),
        _compare_xlats);
  TRACE_EXIT;
}


void close_getkey(void)
{
  TRACE_ENTER;
  TRACE_EXIT;
}


POE_ERR get_insertable_key(char* pchr)
{
  TRACE_ENTER;
  *pchr = '\0';
  wins_set_message("Type a letter");
  wins_repaint_all();
  const char* keyname = ui_get_key();
  if (keyname == NULL)
    TRACE_RETURN(POE_ERR_KEY_NOT_DEFINED);
  POE_ERR err = translate_insertable_key(keyname, pchr);
  wins_set_message("");
  TRACE_RETURN(err);
}


enum confirmation_t get_confirmation(const char* prompt)
{
  TRACE_ENTER;
  wins_set_message(prompt);
  wins_repaint_all();
  const char* keyname = ui_get_key();
  if (keyname == NULL) {
	wins_set_message("");
    TRACE_RETURN(confirmation_n);
  }
  if (strcasecmp(keyname, "ESC") == 0) {
	wins_set_message("");
	TRACE_RETURN(confirmation_esc);
  }
  bool ok = is_confirm_key(keyname);
  wins_set_message("");
  if (ok)
	TRACE_RETURN(confirmation_y)
  else
	TRACE_RETURN(confirmation_n)
}


const char* ui_get_key()
{
  TRACE_ENTER;
  static char _keyname[MAX_KEYSEQ_NAME_LEN];
  int c;
  const char* keyname = _keyname;

  timeout(100);

  do {
    c = getch();
    if (__resize_needed) {
      c = KEY_RESIZE;
      __resize_needed = false;
    }
  } while (c == -1 || c == ERR);

  if (c >= 'a' && c <= 'z') {
    _keyname[0] = c;
    _keyname[1] = '\0';
  }
  else if (c >= 'A' && c <= 'Z') {
    snprintf(_keyname, sizeof(_keyname), "S-%c", c-'A'+'a');
  }
  else if (c >= 225 && c <= 250) { /* xterm alt-letters */
    snprintf(_keyname, sizeof(_keyname), "A-%c", c-225+'a');
  }
  else if (c >= 193 && c <= 218) { /* xterm alt-shift-letters */
    snprintf(_keyname, sizeof(_keyname), "A-S-%c", c-193+'a');
  }
  else if (c >= 134 && c <= 154) { /* xterm ctrl-alt-letters */
    snprintf(_keyname, sizeof(_keyname), "A-C-%c", c-134+'a');
  }
  else if (c >= 176 && c <= 185) { /* xterm alt-numbers */
    snprintf(_keyname, sizeof(_keyname), "A-%c", c-176+'0');
  }
  else if (c >= '0' && c <= '9') {
    _keyname[0] = c;
    _keyname[1] = '\0';
  }
  else {
    struct keyxlat_t key;
    key.code = c;
    key.name = NULL;
    void* p = bsearch(&key,
                      _misc_key_xlat, 
                      sizeof(_misc_key_xlat)/sizeof(_misc_key_xlat[0]),
                      sizeof(_misc_key_xlat[0]),
                      _compare_xlats);
    if (p == NULL) {
      snprintf(_keyname, sizeof(_keyname), "KEY%04d", c);
    }
    else {
      keyname = ((struct keyxlat_t*)p)->name;
    }
  }

  if (keyname[0] == '\0')
    TRACE_RETURN(NULL)
  else
    TRACE_RETURN(keyname)
}



