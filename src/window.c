
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <unistd.h>

#include "trace.h"
#include "logging.h"
#include "utils.h"
#include "poe_err.h"
#include "poe_exit.h"
#include "vec.h"
#include "cstr.h"
#include "bufid.h"
#include "mark.h"
#include "markstack.h"
#include "tabstops.h"
#include "margins.h"
#include "key_interp.h"
#include "buffer.h"
#include "view.h"
#include "window.h"
#include "commands.h"
#include "editor_globals.h"



#define C_NORM_TXT (1)
#define C_MARK_TXT (2)
#define C_CTRL_TXT (3)
#define C_CURS_TXT (4)
#define C_CMDLINE (5)
#define C_INFOLINE (6)
#define C_INFOLINE_MOD (7)
#define C_MSGLINE (8)
#define C_SPLITTERS (9)

#define A_NORM_TXT (A_BOLD)
#define A_MARK_TXT (A_BOLD)
#define A_CTRL_TXT (A_BOLD)
#define A_CURS_TXT (A_BOLD)
#define A_SYS_TXT (A_NORMAL)
#define A_CMDLINE (A_BOLD)
#define A_INFOLINE (A_BOLD)
#define A_INFOLINE_MOD (A_BOLD)
#define A_MSGLINE (A_BOLD)
#define A_SPLITTERS (A_BOLD)


struct window_t {
  int flags;
  int slot;
  pivec views;
  BUFFER data_buf;
  VIEWPTR data_view;
  int t, l, b, r; // screen coordinates - bounds of window area
  int data_top, data_bot; // screen coordinates - top/bottom of data area
  int cmdline, infoline, msgline; // screen coordinates - row# for cmd, info, and msg lines

  // These fields should be replaced by another buffer/viewptr pair.
  // That way all editing commands can be written against a view and
  // as long as they correctly handle the special restrictions for a
  // single-line command buffer, everything should work.
  /* BUFFER cmd_txt; */
  /* MARK cmd_cur; */
  BUFFER cmd_buf;
  VIEWPTR cmd_view;

  int in_data;
  cstr msg_txt;
};

typedef struct window_t WIN;


// PE2 has 4 ways of arranging windows
// normal       - 1 window
// first split  - horizontal
// second split - 4-way split
// third split  - vertical


// as represented in the windows array:
// v1   NULL NULL NULL      [ ]
// v1   v2   NULL NULL      [|]
// v1   v2   v3   v4        [+]
// v1   NULL v3   NULL      [-]

#define MAX_WINDOWS (4)
WINPTR _wins[MAX_WINDOWS];
int _cur_win;

#define ISMODE0 (_wins[0]!=NULL&&_wins[1]==NULL&&_wins[2]==NULL&&_wins[3]==NULL)
#define ISMODE1 (_wins[0]!=NULL&&_wins[1]!=NULL&&_wins[2]==NULL&&_wins[3]==NULL)
#define ISMODE2 (_wins[0]!=NULL&&_wins[1]!=NULL&&_wins[2]!=NULL&&_wins[3]!=NULL)
#define ISMODE3 (_wins[0]!=NULL&&_wins[1]==NULL&&_wins[2]!=NULL&&_wins[3]==NULL)


WINPTR _win_alloc(int slot, int t, int l, int b, int r);
void _win_free(int slot);
void _win_init(WINPTR pwin, int slot, int t, int l, int b, int r);
void _win_destroy(WINPTR pwin);

void _win_move(WINPTR pwin, int t, int l, int b, int r);
void _win_setflags(WINPTR pwin, int flags);
void _win_clrflags(WINPTR pwin, int flags);
int _win_tstflags(WINPTR pwin, int flags);
void _win_repaint(WINPTR pwin, int slot);

void _ensure_view_for_buffer(WINPTR pwin);
void _window_forget_buffer(WINPTR pwin, BUFFER buf);
void _window_stash_view(WINPTR pwin);
void _window_restore_view(WINPTR pwin);
void _window_hide_buffer(WINPTR pwin, BUFFER buf);

void  _wins_curr_chdir(void);

void _wins_repaint_splitters();
void _win_clr_eol(WINPTR pwin, int scrrow, int scrcol, char c);
WINPTR _getwin(const char* dbgstr, int slot);
void _init_curses();
void _init_curses_keys();


void init_windows()
{
  TRACE_ENTER;
  int i;
  //logmsg("initting curses");
  _init_curses();
  //logmsg("initting curses keys");
  _init_curses_keys();
  //logmsg("nulling windows");
  for (i = 0; i < MAX_WINDOWS; i++)
    _wins[i] = NULL;
  _cur_win = 0;
  TRACE_EXIT;
}


void shutdown_windows()
{
  TRACE_ENTER;
  //logmsg("shutting down curses");
  endwin();
  //logmsg("finished shutting down curses");
  TRACE_EXIT;
}


void close_windows()
{
  TRACE_ENTER;
  int i;
  //logmsg("closing windows");
  for (i = 0; i < MAX_WINDOWS; i++) {
    //logmsg("closing window %d", i);
    _win_free(i);
  }
  //logmsg("finished closing windows");
  TRACE_EXIT;
}



void _init_curses()
{
  TRACE_ENTER;

  //logmsg("initscr");
  initscr();

  //logmsg("cbreak");
  raw();
  //logmsg("keypad");
  keypad(stdscr, true);
  //logmsg("noecho");
  noecho();
  //logmsg("atexit");
  atexit(shutdown_windows);
  
  //logmsg("has_colors");
  if (has_colors()) {
    //logmsg("using colors");
    start_color();
    //logmsg("defining color pairs");
    init_pair(C_NORM_TXT, COLOR_WHITE, COLOR_BLUE);
    init_pair(C_MARK_TXT, COLOR_WHITE, COLOR_RED);
    init_pair(C_CTRL_TXT, COLOR_BLACK, COLOR_CYAN);
    init_pair(C_CURS_TXT, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(C_CMDLINE, COLOR_WHITE, COLOR_GREEN);
    init_pair(C_INFOLINE, COLOR_WHITE, COLOR_BLACK);
    init_pair(C_INFOLINE_MOD, COLOR_RED, COLOR_BLACK);
    init_pair(C_MSGLINE, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(C_SPLITTERS, COLOR_YELLOW, COLOR_BLACK);
  }
  TRACE_EXIT;
}

struct other_codes { char c; int code; };

void _init_curses_keys()
{
  TRACE_ENTER;

  const char* term = getenv("TERM");
  if (term == NULL)
    TRACE_EXIT;

  int isxterm = strstr(term, "xterm") != NULL;
  int isrxvt = strstr(term, "rxvt") != NULL;

  if (isrxvt) {
    int i;
    char termseq[32];
    // define various flavors of alted letters
    for (i = 0; i <= 25; i++) {
      // alt-ctrl-
      snprintf(termseq, sizeof(termseq), "\033%c", i+1);
      define_key(termseq, 134+i);
      // alt-
      snprintf(termseq, sizeof(termseq), "\033%c", i+'a');
      define_key(termseq, 225+i);
      // alt-shift-
      snprintf(termseq, sizeof(termseq), "\033%c", i+'A');
      define_key(termseq, 193+i);
    }
    // define various flavors of alted numbers
    const char ucnums[] = "!@#$%^&*()";
    for (i = 0; i < sizeof(ucnums)/sizeof(ucnums[0]); i++) {
      // alt-
      snprintf(termseq, sizeof(termseq), "\033%c", i+'0');
      define_key(termseq, 176+i);
      // alt-shift-
      snprintf(termseq, sizeof(termseq), "\033%c", ucnums[i]);
      define_key(termseq, 161+i);
    }
    // define various flavors of alted function keys
    for (i = 0; i <= 10; i++) {
      // Alt-
      snprintf(termseq, sizeof(termseq), "\033\033[%d~", 11+i);
      define_key(termseq, KEY_F(45)+i);
      // Alt-Shift-
      snprintf(termseq, sizeof(termseq), "\033\033[%d~", 23+i);
      define_key(termseq, POE_SPECIAL_KEY+i);
    }
    // define assorted other codes
    struct other_codes others[] = {
      {'~', 254}, {'`', 224}, {'-', 173}, {'_', 223},
      {'=', 189}, {'+', 171}, {'[', 219}, {'{', 251},
      {']', 221}, {'}', 253}, {';', 187}, {':', 186},
      {'\'', 167}, {'"', 162}, {',', 172}, {'<', 188},
      {'.', 174}, {'>', 190}, {'/', 175}, {'?', 191},
      {'\\', 220}, {'|', 252}, {127, 255/*backspace*/}
    };
    for (i = 0; i < sizeof(others)/sizeof(others[0]); i++) {
      snprintf(termseq, sizeof(termseq), "\033%c", others[i].c);
      define_key(termseq, others[i].code);
    }
  }

  if (isxterm) {
    // These definitions attempt to match xterm to rxvt, which is what
    // the key decoder is going by.  The basic problem is termcap
    // doesn't really adequately describe the terminals, as currentl
    // implemented.  VT100 is the base, but beyond that each terminal
    // emulator seems to roll their own escape sequences, and these are
    // generally far beyond what termcap provides.

    // xterm Shift-Fxx
    define_key("\033O2P", KEY_F(11));
    define_key("\033O2Q", KEY_F(12));
    define_key("\033O2R", KEY_F(13));
    define_key("\033O2S", KEY_F(14));
    define_key("\033[15;2~", KEY_F(15));
    define_key("\033[17;2~", KEY_F(16));
    define_key("\033[18;2~", KEY_F(17));
    define_key("\033[19;2~", KEY_F(18));
    define_key("\033[20;2~", KEY_F(19));
    define_key("\033[21;2~", KEY_F(20));

    // xterm Ctrl-Fxx
    define_key("\033O5P", KEY_F(23));
    define_key("\033O5Q", KEY_F(24));
    define_key("\033O5R", KEY_F(25));
    define_key("\033O5S", KEY_F(26));
    define_key("\033[15;5~", KEY_F(27));
    define_key("\033[17;5~", KEY_F(28));
    define_key("\033[18;5~", KEY_F(29));
    define_key("\033[19;5~", KEY_F(30));
    define_key("\033[20;5~", KEY_F(31));
    define_key("\033[21;5~", KEY_F(32));

    // xterm Shift-Ctrl-Fxx
    define_key("\033O6P", KEY_F(33));
    define_key("\033O6Q", KEY_F(34));
    define_key("\033O6R", KEY_F(35));
    define_key("\033O6S", KEY_F(36));
    define_key("\033[15;6~", KEY_F(37));
    define_key("\033[17;6~", KEY_F(38));
    define_key("\033[18;6~", KEY_F(39));
    define_key("\033[19;6~", KEY_F(30));
    define_key("\033[20;6~", KEY_F(41));
    define_key("\033[21;6~", KEY_F(42));
    define_key("\033[23;6~", KEY_F(43)); // S-C-F11
    define_key("\033[24;6~", KEY_F(44)); // S-C-F12

    // xterm Alt-Fxx
    define_key("\033O3P", KEY_F(45));
    define_key("\033O3Q", KEY_F(46));
    define_key("\033O3R", KEY_F(47));
    define_key("\033O3S", KEY_F(48));
    define_key("\033[15;3~", KEY_F(49));
    define_key("\033[17;3~", KEY_F(50));
    define_key("\033[18;3~", KEY_F(51));
    define_key("\033[19;3~", KEY_F(52));
    define_key("\033[20;3~", KEY_F(53));
    define_key("\033[21;3~", KEY_F(54));
    define_key("\033[23;3~", KEY_F(55)); // A-F11
    define_key("\033[24;3~", KEY_F(56)); // A-F12

    // xterm Shift-Alt-Fxx
    define_key("\033O4P", (POE_SPECIAL_KEY+0));
    define_key("\033O4Q", (POE_SPECIAL_KEY+1));
    define_key("\033O4R", (POE_SPECIAL_KEY+2));
    define_key("\033O4S", (POE_SPECIAL_KEY+3));
    define_key("\033[15;4~", (POE_SPECIAL_KEY+4));
    define_key("\033[17;4~", (POE_SPECIAL_KEY+5));
    define_key("\033[18;4~", (POE_SPECIAL_KEY+6));
    define_key("\033[19;4~", (POE_SPECIAL_KEY+7));
    define_key("\033[20;4~", (POE_SPECIAL_KEY+8));
    define_key("\033[21;4~", (POE_SPECIAL_KEY+9));
    define_key("\033[23;4~", (POE_SPECIAL_KEY+10)); // S-A-F11
    define_key("\033[24;4~", (POE_SPECIAL_KEY+11)); // S-A-F12
  }

  TRACE_EXIT;
}


void wins_ensure_initial_win()
{
  TRACE_ENTER;
  if (_wins[0] != NULL)
    TRACE_EXIT;
  int begrow, begcol;
  int endrow, endcol;
  getbegyx(stdscr, begrow, begcol);
  getmaxyx(stdscr, endrow, endcol);

  //logmsg("wins_ensure_initial_win, t=%d, l=%d, b=%d, r=%d", begrow, begcol, endrow-1, endcol-1);
  _win_alloc(0, begrow, begcol, endrow-1, endcol-1);
  _wins_curr_chdir();
  //WINPTR pwin = _getwin(__func__, _cur_win);
  //logmsg("wins_ensure_initial_win, cur t=%d, l=%d, b=%d, r=%d", pwin->t, pwin->l, pwin->b, pwin->r);
  TRACE_EXIT;
}


void wins_resize(void)
{
  TRACE_ENTER;
  endwin();
  refresh();
  int begrow, begcol;
  int endrow, endcol;
  getbegyx(stdscr, begrow, begcol);
  getmaxyx(stdscr, endrow, endcol);

  int midrow = SCALE_SPLITTER(endrow+begrow, hsplitter);
  int midcol = SCALE_SPLITTER(endcol+begcol, vsplitter);

  if (ISMODE0) {                // []
    WINPTR pv0 = _getwin(__func__, 0);
    _win_move(pv0, begrow, begcol, endrow-1, endcol-1);
  }
  else if (ISMODE1) {           // [|]
    WINPTR pv0 = _getwin(__func__, 0);
    WINPTR pv1 = _getwin(__func__, 1);
    _win_move(pv0, begrow, begcol, endrow-1, midcol-1);
    _win_move(pv1, begrow, midcol+1, endrow-1, endcol-1);
  }
  else if (ISMODE2) {           // [+]
    WINPTR pv0 = _getwin(__func__, 0);
    WINPTR pv1 = _getwin(__func__, 1);
    WINPTR pv2 = _getwin(__func__, 2);
    WINPTR pv3 = _getwin(__func__, 3);
    _win_move(pv0, begrow, begcol, midrow-1, midcol-1);
    _win_move(pv1, begrow, midcol+1, midrow-1, endcol-1);
    _win_move(pv2, midrow+1, begcol, endrow-1, midcol-1);
    _win_move(pv3, midrow+1, midcol+1, endrow-1, endcol-1);
  }
  else if (ISMODE3) {           // [-]
    WINPTR pv0 = _getwin(__func__, 0);
    WINPTR pv2 = _getwin(__func__, 2);
    _win_move(pv0, begrow, begcol, midrow-1, endcol-1);
    _win_move(pv2, midrow+1, begcol, endrow-1, endcol-1);
  }
  TRACE_EXIT;
}


WINPTR wins_get_cur(void)
{
  TRACE_ENTER;
  WINPTR pwin = _getwin(__func__, _cur_win);
  TRACE_RETURN(pwin);
}



// This is simplified tremendously by the existence of wins_resize() that will
// put everybody in the right spot.
void wins_split()
{
  TRACE_ENTER;
  if (ISMODE0) {
    _win_alloc(1, 0, 0, 0, 0);
  }
  else if (ISMODE1) {
    _win_alloc(2, 0, 0, 0, 0);
    _win_alloc(3, 0, 0, 0, 0);
  }
  else if (ISMODE2) {
    _win_free(1); _win_free(3);
  }
  else if (ISMODE3) {
    _win_free(2);
  }
  if (_wins[_cur_win] == NULL)
    wins_nextwindow();
  wins_resize();
  _wins_curr_chdir();
  TRACE_EXIT;
}


void wins_zoom()
{
  TRACE_ENTER;
  
  switch (_cur_win) {
  case 0:
	_win_free(1); _win_free(2); _win_free(3);
	break;
  case 1:
	_win_free(0); _win_free(2); _win_free(3);
	_wins[0] = _wins[1];
	_wins[1] = NULL;
	break;
  case 2:
	_win_free(0); _win_free(1); _win_free(3);
	_wins[0] = _wins[2];
	_wins[2] = NULL;
	break;
  case 3:
	_win_free(0); _win_free(1); _win_free(2);
	_wins[0] = _wins[3];
	_wins[3] = NULL;
	break;
  }
  _cur_win = 0;
  wins_resize();
  _wins_curr_chdir();
  
  TRACE_EXIT;
}


void wins_nextwindow()
{
  TRACE_ENTER;
  int i;
  for (i = 1; i <= MAX_WINDOWS; i++) {
    int j = (_cur_win+i) % MAX_WINDOWS;
    if (_wins[j] != NULL) {
      _cur_win = j;
      break;
    }
  }
  _wins_curr_chdir();
  TRACE_EXIT;
}


void wins_nextview()
{
  TRACE_ENTER;
  WINPTR pwin = _getwin(__func__, _cur_win);
  BUFFER data_buf = pwin->data_buf;
  int i;
  for (i = 1; i <= MAX_WINDOWS; i++) {
    int j = (_cur_win+i) % MAX_WINDOWS;
    if (_wins[j] != NULL && _wins[j]->data_buf == data_buf) {
      _cur_win = j;
      break;
    }
  }
  _wins_curr_chdir();
  TRACE_EXIT;
}


// aka "F8"
void wins_cur_nextbuffer(void)
{
  TRACE_ENTER;
  wins_ensure_initial_win();
  WINPTR pwin = _getwin(__func__, _cur_win);
  wins_cur_switchbuffer(buffers_next(pwin->data_buf));
  TRACE_EXIT;
}


void wins_cur_switchbuffer(BUFFER buf)
{
  TRACE_ENTER;
  wins_ensure_initial_win();
  WINPTR pwin = _getwin(__func__, _cur_win);
  _window_stash_view(pwin);
  pwin->data_buf = buf;
  _window_restore_view(pwin);
  _wins_curr_chdir();
  TRACE_EXIT;
}


void wins_hidebuffer(BUFFER buf)
{
  TRACE_ENTER;
  wins_ensure_initial_win();
  int i;
  for (i = 0; i < MAX_WINDOWS; i++) {
    WINPTR pwin = _wins[i];
    if (pwin == NULL)
      continue;
    _window_hide_buffer(pwin, buf);
  }
  TRACE_EXIT;
}


void _window_hide_buffer(WINPTR pwin, BUFFER buf)
{
  TRACE_ENTER;
  if (pwin->data_buf == buf) {
    pwin->data_buf = buffers_next(pwin->data_buf);
    _window_restore_view(pwin);
  }
  _window_forget_buffer(pwin, buf);
  TRACE_EXIT;
}


void _wins_curr_chdir(void)
{
  TRACE_ENTER;
  WINPTR pwin = _getwin(__func__, _cur_win);
  if (pwin != NULL) {
    const char* dirname = buffer_curr_dirname(pwin->data_buf);
    if (dirname != NULL) {
      //trace_stack_print();
      //logmsg("changing dir to '%s'", dirname);
      chdir(dirname);
    }
  }
  TRACE_EXIT;
}


void wins_set_message(const char* message)
{
  TRACE_ENTER;
  WINPTR pwin = _getwin(__func__, _cur_win);
  if (pwin != NULL)
    cstr_assignstr(&pwin->msg_txt, message);
  TRACE_EXIT;
}


void wins_repaint_all()
{
  TRACE_ENTER;
  int i;
  // paint the current window last so the cursor is left in the right spot
  for (i = 0; i < MAX_WINDOWS; ++i) {
    if (_wins[i] == NULL || _cur_win == i)
      continue;
    _win_repaint(_wins[i], i);
  }
  _wins_repaint_splitters();
  _win_repaint(_wins[_cur_win], _cur_win);

  TRACE_EXIT;
}


void _wins_repaint_splitters()
{
  bkgdset(' ' | COLOR_PAIR(C_SPLITTERS) | A_SPLITTERS);
  if (ISMODE0) {
    // no splitters
  }
  else if (ISMODE1) {
    // draw vertical splitter between v0 and v1
    WINPTR pv0 = _getwin(__func__, 0);
    int t0=pv0->t, /*l0=pv0->l,*/ b0=pv0->b, r0=pv0->r;
    mvvline(t0, r0+1, ACS_VLINE, b0-t0+1);
  }
  else if (ISMODE2) {
    // draw vertical splitter between v0 and v1 (and v2 and v3)
    // draw horizontal splitter between v0 and v2 (and v1 and v3)
    WINPTR pv0 = _getwin(__func__, 0);
    WINPTR pv1 = _getwin(__func__, 1);
    WINPTR pv2 = _getwin(__func__, 2);
    WINPTR pv3 = _getwin(__func__, 3);
    int t0=pv0->t, b0=pv0->b, l0=pv0->l, r0=pv0->r;
    int /*t1=pv1->t,*/ b1=pv1->b, l1=pv1->l, r1=pv1->r;
    int t2=pv2->t, b2=pv2->b, /*l2=pv2->l,*/ r2=pv2->r;
    int t3=pv3->t/*, b3=pv3->b, l3=pv3->l, r3=pv3->r*/;
    mvvline(t0, r0+1, ACS_VLINE, b0-t0+1); // vertical line between v0 and v1
    mvvline(t3, r2+1, ACS_VLINE, b2-t2+1); // vertical line between v2 and v3
    mvhline(b0+1, l0, ACS_HLINE, r0-l0+1); // horizontal line between v0 and v2
    mvhline(b1+1, l1, ACS_HLINE, r1-l1+1); // horizontal line between v1 and v3
    mvaddch(b0+1, r0+1, ACS_PLUS);       // plus at the central corner
  }
  else if (ISMODE3) {
    // draw horizontal splitter between v0 and v2
    WINPTR pv0 = _getwin(__func__, 0);
    int /*t0=pv0->t,*/ l0=pv0->l, b0=pv0->b, r0=pv0->r+1;
    mvhline(b0+1, l0, ACS_HLINE, r0-l0+1);
  }
}





//
// internal funcs
//

WINPTR _win_alloc(int slot, int t, int l, int b, int r)
{
  TRACE_ENTER;
  if (_wins[slot] != NULL)
    _win_free(slot);
  //logmsg("creating window %d (%d, %d) (%d, %d)", slot, t, l, b, r);
  WINPTR pwin = (WINPTR)calloc(1, sizeof(struct window_t));
  //logmsg("initting window %d (%d, %d) (%d, %d)", slot, t, l, b, r);
  _win_init(pwin, slot, t, l, b, r);
  _wins[slot] = pwin;
  TRACE_RETURN(pwin);
}


void _win_free(int slot)
{
  TRACE_ENTER;
  //logmsg("attempting to free window %d", slot);
  if (_wins[slot] == NULL)
    TRACE_EXIT;
  //logmsg("destroying window %d", slot);
  _win_destroy(_wins[slot]);
  //logmsg("freeing window %d", slot);
  free(_wins[slot]);
  _wins[slot] = NULL;
  //logmsg("finished freeing window %d", slot);
  TRACE_EXIT;
}


void _win_init(WINPTR pwin, int slot, int t, int l, int b, int r)
{
  TRACE_ENTER;
  pwin->flags = 0;
  pwin->slot = slot;
  pivec_init(&pwin->views, 0);
  pwin->data_buf = buffers_next(BUFFER_NULL);
  pwin->data_view = NULL;
  _window_restore_view(pwin);
  
  static int cmd_buf_num = 0;
  char cmd_buf_name[64];
  snprintf(cmd_buf_name, sizeof cmd_buf_name, "CMD_%d", ++cmd_buf_num);
  pwin->cmd_buf = buffer_alloc(cmd_buf_name, BUF_FLG_INTERNAL|BUF_FLG_CMDLINE, 1, default_profile);
  pwin->cmd_view = view_alloc(pwin->cmd_buf, 1, pwin->r - pwin->l + 1);
  buffer_ensure_min_lines(pwin->cmd_buf, false);
  
  _win_move(pwin, t, l, b, r);
  
  cstr_init(&pwin->msg_txt, 256);
  pwin->in_data = default_profile->oncommand == false;
  TRACE_EXIT;
}


void _win_destroy(WINPTR pwin)
{
  TRACE_ENTER;
  
  //logmsg("_win_destroy: attempting to destroy window");
  int i, nviews = pivec_count(&pwin->views);
  //logmsg("window has %d views", nviews);
  for (i = 0; i < nviews; ++i) {
    //logmsg("destroying view %d", i);
    view_free((VIEWPTR)pivec_get(&pwin->views, i));
  }
  
  //logmsg("destroying views vector");
  pivec_destroy(&pwin->views);
  
  //logmsg("destroying cmd buffer");
  //cstr_destroy(&pwin->cmd_txt);
  view_free(pwin->cmd_view);
  buffer_free(pwin->cmd_buf);
  pwin->cmd_view = NULL;
  pwin->cmd_buf = BUFFER_NULL;
  
  //logmsg("destroying msg string");
  cstr_destroy(&pwin->msg_txt);
  TRACE_EXIT;
}


void _win_move(WINPTR pwin, int t, int l, int b, int r)
{
  TRACE_ENTER;
  pwin->t = t; pwin->l = l; pwin->b = b; pwin->r = r;
  pwin->msgline = b; pwin->infoline = b-1; pwin->cmdline = b-2;
  pwin->data_bot = b-3; pwin->data_top = t;
  //logmsg("moving win, %d %d  %d %d, vsize = %d", t, l, b, r, pwin->data_bot - pwin->data_top);
  view_set_hsize(pwin->data_view, r-l);
  view_set_vsize(pwin->data_view, pwin->data_bot - pwin->data_top);
  view_set_hsize(pwin->cmd_view, r-l);
  TRACE_EXIT;
}


void win_setflags(WINPTR pwin, int flags)
{
  TRACE_ENTER;
  pwin->flags |= flags;
  TRACE_EXIT;
}


void win_clrflags(WINPTR pwin, int flags)
{
  TRACE_ENTER;
  pwin->flags &= ~flags; 
  TRACE_EXIT;
}


int win_tstflags(WINPTR pwin, int flags)
{
  TRACE_ENTER;
  int rval = (pwin->flags & flags) == flags;
  TRACE_RETURN(rval);
}


bool win_get_commandmode(WINPTR pwin)
{
  TRACE_ENTER;
  bool rval = pwin->in_data == 0;
  TRACE_RETURN(rval);
}


void win_set_commandmode(WINPTR pwin, bool go_to_command)
{
  TRACE_ENTER;
  // Have to make sure the insert mode is carried over to the new buffer.
  int new_in_data = go_to_command == 0;
  if (pwin->in_data == new_in_data)
    TRACE_EXIT;
  if (pwin->in_data && !new_in_data)
    view_set_insertmode(pwin->cmd_view, view_get_insertmode(pwin->data_view));
  else if (!pwin->in_data && new_in_data)
    view_set_insertmode(pwin->data_view, view_get_insertmode(pwin->cmd_view));
  // now we can switch views.
  pwin->in_data = new_in_data;
  TRACE_EXIT;
}


void win_toggle_commandmode(WINPTR pwin)
{
  TRACE_ENTER;
  win_set_commandmode(pwin, !win_get_commandmode(pwin));
  TRACE_EXIT;
}


PROFILEPTR win_get_profile(WINPTR pwin)
{
  TRACE_ENTER;
  PROFILEPTR prof;
  if (win_get_commandmode(pwin)) {
    prof = buffer_get_profile(pwin->cmd_buf);
  }
  else {
    prof = buffer_get_profile(pwin->data_buf);
  }
  TRACE_RETURN(prof);
}


PROFILEPTR win_get_data_profile(WINPTR pwin)
{
  TRACE_ENTER;
  PROFILEPTR prof = buffer_get_profile(pwin->data_buf);
  TRACE_RETURN(prof);
}


PROFILEPTR win_get_cmd_profile(WINPTR pwin)
{
  TRACE_ENTER;
  PROFILEPTR prof = buffer_get_profile(pwin->cmd_buf);
  TRACE_RETURN(prof);
}



// Make sure the specified buffer is no longer the current buffer, and
// no longer in our view list
void _window_forget_buffer(WINPTR pwin, BUFFER buf)
{
  TRACE_ENTER;
  if (pwin->data_buf == buf) {
    pwin->data_buf = BUFFER_NULL;
    pwin->data_view = NULL;
  }
  int i, n = pivec_count(&pwin->views);
  for (i = 0; i < n; i++) {
    VIEWPTR pview = (VIEWPTR)pivec_get(&pwin->views, i);
    if (view_buffer(pview) == buf) {
      pivec_remove(&pwin->views, i);
      view_free(pview);
      break;
    }
  }
  TRACE_EXIT;
}


void _window_stash_view(WINPTR pwin)
{
  TRACE_ENTER;
  BUFFER buf = pwin->data_buf;
  VIEWPTR view = pwin->data_view;
  if (buf == BUFFER_NULL)
    TRACE_EXIT;
  int i, n = pivec_count(&pwin->views);
  for (i = 0; i < n; i++) {
    VIEWPTR pview = (VIEWPTR)pivec_get(&pwin->views, i);
    if (view == pview)
      TRACE_EXIT;;
  }
  pivec_append(&pwin->views, (intptr_t)view);
  TRACE_EXIT;
}


void _window_restore_view(WINPTR pwin)
{
  TRACE_ENTER;

  // Try to restore it from cache.
  BUFFER buf = pwin->data_buf;
  int i, n = pivec_count(&pwin->views);
  pwin->data_view = NULL;
  for (i = 0; i < n; i++) {
    VIEWPTR pview = (VIEWPTR)pivec_get(&pwin->views, i);
    if (view_buffer(pview) == buf) {
      pwin->data_view = pview;
      break;
    }
  }

  if (pwin->data_view == NULL) {
    // Didn't find the view in cache, so create a new one.
    pwin->data_view = view_alloc(buf, pwin->data_bot - pwin->data_top, pwin->r - pwin->l + 1);
    pivec_append(&pwin->views, (intptr_t)pwin->data_view);
  }

  TRACE_EXIT;
}


bool update_context(cmd_ctx* ctx)
{
  TRACE_ENTER;
  bool rc = false;
  WINPTR pwnd = _wins[_cur_win];
  if (pwnd == NULL) {
    //logmsg("update_context cmdline, window is NULL");
    ctx->wnd = NULL;
    ctx->targ_view = ctx->data_view = ctx->cmd_view = NULL;
    ctx->targ_buf = ctx->data_buf = ctx->cmd_buf = BUFFER_NULL;
    ctx->targ_row = ctx->data_row = ctx->cmd_row = -1;
    ctx->targ_col = ctx->data_col = ctx->cmd_col = -1;
    rc = false;
  }
  else {
    ctx->wnd = pwnd;
    ctx->data_view = pwnd->data_view;
    ctx->data_buf = pwnd->data_buf;
    ctx->cmd_view = pwnd->cmd_view;
    ctx->cmd_buf = pwnd->cmd_buf;
	if (ctx->data_view != NULL)
	  view_get_cursor(pwnd->data_view, &ctx->data_row, &ctx->data_col);
	else
	  ctx->data_row = ctx->data_col = 0;
	if (ctx->cmd_view != NULL)
	  view_get_cursor(pwnd->cmd_view, &ctx->cmd_row, &ctx->cmd_col);
	else
	  ctx->cmd_row = ctx->cmd_col = 0;
    if (ctx->src_is_commandline) {
      //logmsg("update_context cmdline, targ = data");
      // Commands entered from the commandline always target the data area
      ctx->targ_view = ctx->data_view;
      ctx->targ_buf = ctx->data_buf;
      ctx->targ_row = ctx->data_row;
      ctx->targ_col = ctx->data_col;
    }
    else if (pwnd->in_data) {
      //logmsg("update_context kbd, targ = data");
      // Commands entered from the keyboard in the data area target the data area
      ctx->targ_view = ctx->data_view;
      ctx->targ_buf = ctx->data_buf;
      ctx->targ_row = ctx->data_row;
      ctx->targ_col = ctx->data_col;
    }
    else {
      //logmsg("update_context kbd, targ = cmdline");
      // Commands entered from the keyboard in the cmdline target the command area
      ctx->targ_view = ctx->cmd_view;
      ctx->targ_buf = ctx->cmd_buf;
      ctx->targ_row = ctx->cmd_row;
      ctx->targ_col = ctx->cmd_col;
    }
    rc = ctx->data_view != NULL && ctx->cmd_view != NULL;
  }
  TRACE_RETURN(rc);
}


void _win_clr_eol(WINPTR pwin, int scrrow, int scrcol, char c)
{
  int j;
  move(scrrow, scrcol);
  for (j = scrcol; j <= pwin->r; j++) {
    addch(c);
  }
}


WINPTR _getwin(const char* dbgstr, int slot)
{
  TRACE_ENTER;
  if (slot < 0 || slot >= MAX_WINDOWS)
    poe_err(1, "Invalid current win position %d", _cur_win);
  if (_wins[slot] == NULL)
    poe_err(1, "Missing win in position %d", slot);
  TRACE_RETURN(_wins[slot]);
}



void _log_view_info()
{
  TRACE_ENTER;
  WINPTR pwin = _getwin(__func__, _cur_win);
  int cur_line, cur_col, view_top, view_left, view_bot, view_right;
  view_get_cursor(pwin->data_view, &cur_line, &cur_col);
  view_get_port(pwin->data_view, &view_top, &view_left, &view_bot, &view_right);
  logmsg("cursor %d, %d   view %d, %d -> %d %d", cur_line, cur_col, view_top, view_left, view_bot, view_right);
  TRACE_EXIT;
}




#define FILTER_MARK_FLAGS_MASK (0)
#define FILTER_MARK_FLAGS_CHK  (0)
void _win_repaint(WINPTR pwin, int slot)
{
  TRACE_ENTER;
  int i, j;
  //logmsg("painting window %d, (%d %d) (%d %d)", slot, pwin->t, pwin->l, pwin->b, pwin->r);
  int data_ht = pwin->data_bot - pwin->data_top + 1, view_wid = pwin->r - pwin->l + 1;
  int view_top, view_left, view_bot, view_right;
  int cursor_line, cursor_col;
  view_get_port(pwin->data_view, &view_top, &view_left, &view_bot, &view_right);
  view_get_cursor(pwin->data_view, &cursor_line, &cursor_col);

  int cmd_top, cmd_left, cmd_bot, cmd_right;
  int cmd_cursor_line, cmd_cursor_col;
  view_get_port(pwin->cmd_view, &cmd_top, &cmd_left, &cmd_bot, &cmd_right);
  view_get_cursor(pwin->cmd_view, &cmd_cursor_line, &cmd_cursor_col);

  VIEWPTR active_view;
  if (pwin->in_data)
    active_view = pwin->data_view;
  else
    active_view = pwin->cmd_view;

  /* logmsg("cmd viewport %d %d", cmd_top, cmd_left); */
  /* logmsg("cmd cursor %d %d", cmd_cursor_line, cmd_cursor_col); */
  //logmsg("view_left = %d", view_left);

  // draw data area
  BUFFER data_buf = pwin->data_buf;
  int nlines = buffer_count(data_buf);
  //color_set(C_NORM_TXT, NULL);
  //attron(A_NORM_TXT);
  bkgdset(' ' | COLOR_PAIR(C_NORM_TXT) | A_NORM_TXT);
  int disp_line = pwin->data_top;
  int in_mark = 0;
  MARK cur_mark = markstack_current();
  BUFFER xbuf = 0L;
  mark_get_buffer(cur_mark, &xbuf);
  //logmsg("mark buffer = %ld, cur buffer = %ld", xbuf, data_buf);
  for (i = 0; i < data_ht; i++) {
    attroff(A_SYS_TXT);
    const char* disptxt = NULL;
    int displinelen;
    if (view_top+i < -1) {
      disptxt = "";
      displinelen = 0;
    }
    else if (view_top+i == -1) {
      disptxt = "==== Top of File ====";
      displinelen = 21;
      attron(A_SYS_TXT);
    }
    else if (view_top+i == nlines) {
      disptxt = "==== Bottom of File ====";
      displinelen = 24;
      attron(A_SYS_TXT);
    }
    else if (view_top+i > nlines) {
      disptxt = "";
      displinelen = 0;
    }
    else {
      const char* lpszLine = buffer_getbufptr(data_buf, view_top+i);
      disptxt = lpszLine + min(strlen(lpszLine), view_left);
      int linelen = buffer_line_length(data_buf, view_top+i) - view_left;
      displinelen = min(linelen, view_wid);
      displinelen = max(0, displinelen);
      //logmsg("line %d, linelen = %d displinelen = %d", i, linelen, displinelen);
    }

    // Draw the line.
    move(disp_line, pwin->l);
    // Have to loop over the entire window width to make sure the hilighting displays correctly.
    bool in_txt = true;
    int line_has_mark = markstack_hittest_line(data_buf, view_top+i, FILTER_MARK_FLAGS_MASK, FILTER_MARK_FLAGS_CHK) != MARK_NULL;
    //logmsg("line %d has mark %ld", i, line_mark);
    for (j = 0; j < view_wid; j++) {
      if (j >= displinelen)
        in_txt = false;
      int new_in_mark = line_has_mark && markstack_hittest_point(data_buf, view_top+i, view_left+j, FILTER_MARK_FLAGS_MASK, FILTER_MARK_FLAGS_CHK) != MARK_NULL;
      if (!in_mark && new_in_mark) {
        bkgdset(' ' | COLOR_PAIR(C_MARK_TXT) | A_MARK_TXT);
      }
      else if (in_mark && !new_in_mark) {
        bkgdset(' ' | COLOR_PAIR(C_NORM_TXT) | A_NORM_TXT);
      }
      in_mark = new_in_mark;
      char dispch;
      bool in_ctrl = false;
      if (in_txt) {
        dispch = disptxt[j];
        if (dispch == '\0' || iscntrl(dispch)) {
          dispch = '@' + dispch;
          bkgdset(' ' | COLOR_PAIR(C_CTRL_TXT) | A_CTRL_TXT);
          in_ctrl = true;
        }
      }
      else {
        dispch = ' ';
      }

      // Figure out if we're at the cursor position...
      bool in_curs = false;
      if (!pwin->in_data
          && i == pwin->t + cursor_line - view_top
          && j == pwin->l + cursor_col - view_left) {
        bkgdset(' ' | COLOR_PAIR(C_CURS_TXT) | A_CURS_TXT);
        in_curs = 1;
      }

      // paint the character
      addch(dispch);

      // Reset the background to either the normal color or the mark colors
      if (in_ctrl || in_curs) {
        if (in_mark) {
          bkgdset(' ' | COLOR_PAIR(C_MARK_TXT) | A_MARK_TXT);
        }
        else {
          bkgdset(' ' | COLOR_PAIR(C_NORM_TXT) | A_NORM_TXT);
        }
      }
    }
    //_win_clr_eol(pwin, ' ');
    ++disp_line;
  }
  attroff(A_NORM_TXT);
  
  // draw cmdline
  bkgdset(' ' | COLOR_PAIR(C_CMDLINE) | A_CMDLINE);
  const char* lpszCmdLine = buffer_getbufptr(pwin->cmd_buf, 0);
  int cmdlinelen = strlen(lpszCmdLine);
  int left_cmd_charidx = min(cmdlinelen, cmd_left);
  int n_cmd_chars = cmdlinelen - left_cmd_charidx;
  mvaddnstr(pwin->cmdline, pwin->l, lpszCmdLine + left_cmd_charidx, n_cmd_chars);
  _win_clr_eol(pwin, pwin->cmdline, pwin->l+n_cmd_chars, ' ');
  attroff(A_CMDLINE);
  
  // draw info line
  //logmsg("drawing info line win %d  row %d col %d", slot, pwin->infoline, pwin->l);
  int insert_mode = view_get_insertmode(active_view);
  bkgdset(' ' | COLOR_PAIR(C_INFOLINE) | A_INFOLINE);
  _win_clr_eol(pwin, pwin->infoline, pwin->l, ' ');
  if (buffer_tstflags(data_buf, BUF_FLG_DIRTY))
    bkgdset(' ' | COLOR_PAIR(C_INFOLINE_MOD) | A_INFOLINE);
  const char* filename = buffer_filename(data_buf);
  const char* bufname = buffer_name(data_buf);
  if (filename != NULL && strlen(filename) > 0)
    mvaddnstr(pwin->infoline, pwin->l, filename, view_wid);
  else if (bufname != NULL && strlen(bufname) > 0)
    mvaddnstr(pwin->infoline, pwin->l, bufname, view_wid);
  if (buffer_tstflags(data_buf, BUF_FLG_DIRTY))
    bkgdset(' ' | COLOR_PAIR(C_INFOLINE) | A_INFOLINE);
  char linenum_info[256];
  snprintf(linenum_info, sizeof(linenum_info), " %d %d %s ", cursor_line+1, cursor_col+1, insert_mode ? "Insert":"Replace");
  mvaddstr(pwin->infoline, pwin->l+view_wid-strlen(linenum_info), linenum_info);
  
  // draw msg line 
  bkgdset(' ' | COLOR_PAIR(C_MSGLINE) | A_MSGLINE);
  _win_clr_eol(pwin, pwin->msgline, pwin->l, ' ');
  mvaddstr(pwin->msgline, pwin->l, cstr_getbufptr(&pwin->msg_txt));
  
  // draw cursor
  if (slot == _cur_win) {
    curs_set(insert_mode ? 1 : 2); // 0 = invisible, 1 = visible, 2 = more visible
    if (pwin->in_data) {
      if (cursor_line >= view_top && cursor_line <= view_bot && cursor_col >= view_left && cursor_col <= view_right) {
        move(pwin->t + cursor_line - view_top, pwin->l + cursor_col - view_left);
	  }
    }
    else {
      move(pwin->cmdline, pwin->l + cmd_cursor_col - cmd_left);
    }
  }
  TRACE_EXIT;
}


