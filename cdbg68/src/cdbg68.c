/*
 * @file    cdbg68.c
 * @brief   a 68K curses debugger program
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (c) 1998-2015 Benjamin Gerard
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* generated config include */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

/* sc68 includes */
#include "sc68/sc68.h"
#include "sc68/msg68.h"
#include "sc68/msg68.h"
#include "emu68/emu68.h"

#include "register.h"
#include "status.h"
#include "disa.h"
#include "mem.h"
#include "cmd.h"
#include "expr.h"



WINDOW * regwin, * midwin, * msgwin, * stawin;
WINDOW * diswin, * memwin;
WINDOW * focwin;

int focus_i, focus_n;
WINDOW * focus_list[ 16 ];

enum {
  MID_TOP_L = 1,
  MID_TOP_R = 2,
  MID_MID   = 4,
  MID_BOT_L = 8,
  MID_BOT_R = 16,
  STA_TOP   = 32
};

static int borderdef[5] = {
  /* register    */
  MID_TOP_L + MID_TOP_R,
  /* disassembly */
  MID_TOP_L + MID_MID + MID_BOT_L,
  /* memory      */
  MID_TOP_R + MID_MID + MID_BOT_R,
  /* message     */
  MID_BOT_L + MID_BOT_R,
  /* status      */
  STA_TOP
};

sc68_t   * sc68;
emu68_t  * emu68;
io68_t  ** ios68;

sc68_music_info_t musinfo;

void sc68_emulators(sc68_t * sc68, emu68_t  ** p_emu68, io68_t  *** p_ios68);

/* Error output */
static int error_to_stderr = 1;

/* static */
void error_va(const char * fmt, va_list list)
{
  char tmp[1024];
  if (msgwin) {
    vsnprintf(tmp, sizeof(tmp), fmt, list);
    tmp [ sizeof(tmp) - 1 ] = 0;
    wattrset(msgwin, COLOR_PAIR(COLOR_RED|A_BOLD));
    waddstr(msgwin, tmp);
    wrefresh(msgwin);
  } else if (error_to_stderr) {
    vfprintf(stderr, fmt, list);
  }
}

/* static */
void error(const char * fmt, ...)
{
  va_list list;
  va_start(list, fmt);
  error_va(fmt, list);
  va_end(list);
}

/* Info output */
static int info_to_stderr = 1;

/* static */
void info_va(const char * fmt, va_list list)
{
  char tmp[1024];
  if (msgwin) {
    vsnprintf(tmp, sizeof(tmp), fmt, list);
    tmp [ sizeof(tmp) - 1 ] = 0;
    wattrset(msgwin, COLOR_PAIR(COLOR_WHITE));
    waddstr(msgwin, tmp);
    wrefresh(msgwin);
  } else if (info_to_stderr) {
    vfprintf(stderr, fmt, list);
  }
}

/* static */
void info(const char * fmt, ...)
{
  va_list list;
  va_start(list, fmt);
  info_va(fmt, list);
  va_end(list);
}


/* Debug output */

static int debug_to_stderr = 1;
static int isdebug = 1;

/* static */
void debug_va(const char * fmt, va_list list)
{
  char tmp[1024];
  if (isdebug) {
    if (msgwin) {
      vsnprintf(tmp, sizeof(tmp), fmt, list);
      tmp [ sizeof(tmp) - 1 ] = 0;
      wattrset(msgwin, COLOR_PAIR(COLOR_CYAN));
      waddstr(msgwin, tmp);
      wrefresh(msgwin);
    } else if (debug_to_stderr) {
      vfprintf(stderr, fmt, list);
    }
  }
}

/* static */
void debug(const char * fmt, ...)
{
  va_list list;
  va_start(list,fmt);
  debug_va(fmt, list);
  va_end(list);
}

int cmd_debug(int argc, char **argv) {
  int v = !isdebug;
  if (argc >= 2) {
    char * e = argv[1];
    if (expr(&v, &e)) {
      cmd_seterr(argv[0],"syntax error in expression",1);
      return -1;
    }
  }
  isdebug = !!v;
  info("debug is now %s\n", isdebug?"On":"Off");
  return 0;
}

static void
sc68_debug_cb(const int bit, void *data, const char *fmt, va_list list)
{
  switch ( bit ) {
  case msg68_DEBUG: case msg68_TRACE:
    debug_va(fmt,list); break;
  case msg68_ERROR: case msg68_CRITICAL: case msg68_WARNING:
    error_va(fmt,list); break;
  case msg68_INFO:
  default:
    info_va(fmt,list); break;
  }
}

static int usage(void)
{
  puts(
    "Usage: cdbg68 [OPTIONS] [sc68-file [track]]\n"
    "\n"
    "Options:\n"
    " -h --help          Display this message and exit.\n"
    " -V --version       Display version and copyright and exit.\n"
    );
  return 1;
}

static int version(void)
{
  puts(
    PACKAGE_NAME " " PACKAGE_VERSION "\n"
    "\n"
    "Copyright (c) 2001-2011 Benjamin Gerard.\n"
    "License GPLv3+ or later <http://gnu.org/licenses/gpl.html>\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n"
    "\n"
    "Written by Benjamin Gerard"
    );
  return 0;
}

static void safe_delwin(WINDOW ** win)
{
  if (*win) {
    delwin(*win);
    *win = 0;
  }
}

int LoadSc68(const char * url, int track)
{
  int err;

  err = sc68_load_url(sc68, url);
  if ( ! err )
    err = sc68_music_info(sc68, &musinfo, track, 0);

  if ( ! err ) {
    if ( ! strcmp( musinfo.title, musinfo.album ) ) {
      sta_set("loaded: [%02d] %s - %s",
              musinfo.trk.track,
              musinfo.artist, musinfo.title);
    } else {
      sta_set("loaded: [%02d] %s - %s - %s",
              musinfo.trk.track,
              musinfo.artist, musinfo.album, musinfo.title);
    }
    err = sc68_play(sc68, track, -1);
    if ( ! err )
      err = -( sc68_process(sc68, 0, 0 ) == SC68_ERROR );
  }

  if ( err ) {
    const char * s;
    while ( s = sc68_error_get(sc68), s ) {
      error("sc68: %s\n", s);
    }
    sta_set("failed! %s",url);
  }

  dis_set(musinfo.addr);
  mem_set(musinfo.addr);
  reg_set();

  return err;
}

static void mywinopt(WINDOW * win)
{
  if ( win ) {
    intrflush(win, FALSE);
    keypad(win, TRUE);

    focus_list [ focus_n++ ] = win;

  }
}

static WINDOW * mynewwin(int h, int w, int y, int x)
{
  WINDOW * win = newwin(h, w, y, x);
  mywinopt(win);
  return win;
}

static WINDOW * myderwin(WINDOW * parent, int h, int w, int y, int x)
{
  WINDOW * win = derwin(parent, h, w, y, x);
  mywinopt(win);
  return win;
}

int msg_init() {
  return 0;
}

int Init(void)
{
  int err = -1;
  int h,w,y,x,W,H;

  /* Setup ncurses */
  initscr();
  W = COLS;
  H = LINES;

  if (W < 80) {
    error("terminal must have at least 80 columns (got %d).\n", W);
    goto finish;
  }
  if (H < 16) {
    error("terminal must have at least 16 lines (got %d).\n", H);
    goto finish;
  }

  cbreak();                             /* do not wait for NL         */
  noecho();                             /* do not echo                */
  nonl();                               /* do not translate RET to NL */
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);

  if (has_colors()) {
    start_color();
    init_pair(COLOR_BLACK,   COLOR_BLACK,   COLOR_BLACK);
    init_pair(COLOR_GREEN,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(COLOR_RED,     COLOR_RED,     COLOR_BLACK);
    init_pair(COLOR_CYAN,    COLOR_CYAN,    COLOR_BLACK);
    init_pair(COLOR_WHITE,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_BLUE,    COLOR_BLUE,    COLOR_BLACK);
    init_pair(COLOR_YELLOW,  COLOR_YELLOW,  COLOR_BLACK);
  }


  /* Register Window */
  w = W;  h = 4;
  x = 0;  y = 0;
  regwin = mynewwin(h, w, y, x);
  if (!regwin) {
    error("creating register window\n");
    goto finish;
  }
  reg_init();

  /* Status window */
  w = w;  h = 2;
  x = 0;  y = H-h;
  stawin = mynewwin(h, w, y, x);
  if (!stawin) {
    error("creating status window\n");
    goto finish;
  }
  sta_init();


  /* Message window */
  w = w;  h  = (H - 7) / 3;
  x = 0;  y -= h;
  msgwin = mynewwin(h, w, y, x);
  if (!msgwin) {
    error("creating message window\n");
    goto finish;
  }
  scrollok(msgwin, TRUE);
  msg_init();

  /* Middle window */
  w = w;  h  = y - 4;
  x = 0;  y  = 4;
  midwin = mynewwin(h, w, y, x);
  if (!midwin) {
    error("creating middle window\n");
    goto finish;
  }
  focus_list [ --focus_n ] = 0;         /* remove from focus list */

  if (1) {
    const int mem_w = 4*8;

    mvwhline(midwin,0,x,0,w);
    mvwhline(midwin,h-1,x,0,w);
    mvwvline(midwin,1,w-mem_w-1,0,h-2);

    /* Disassembly window */
    w = W - mem_w - 1; h -= 2;
    x = 0; y = 1;
    diswin = myderwin(midwin, h, w, y, x);
    if (!diswin) {
      error("creating disassembly window\n");
      goto finish;
    }
    dis_init();
    dis_set(-1);
    dis_draw();
    dis_refresh();

    /* Memory window */
    w = mem_w; x = W-mem_w;
    memwin = myderwin(midwin, h, w, y, x);
    if (!memwin) {
      error("creating memory window\n");
      goto finish;
    }
    mem_init();
    mem_set(-1);
    mem_draw();
    mem_refresh();
  }

  reg_draw();
  reg_set();

  sta_draw();
  sta_refresh();
  wrefresh(midwin);

  err = 0;

finish:
  return err;
}


void Cleanup(void)
{
  /* ... */

  /* Destroy all windows */
  reg_destroy();
  sta_destroy();
  safe_delwin(&msgwin);
  dis_destroy();
  mem_destroy();
  safe_delwin(&midwin);
  endwin();
}


int msg_key(int key)
{
  return key;
}

static void redraw_borders(void)
{
  int w = getmaxx(midwin);
  int h = getmaxy(midwin);
  int x = getmaxx(diswin);
  int f_attr[2];
  int borders;

  f_attr[0] = COLOR_PAIR(COLOR_WHITE);
  f_attr[1] = COLOR_PAIR(COLOR_YELLOW) | A_BOLD | A_STANDOUT;
  borders = borderdef[focus_i];

  wattrset(midwin, f_attr[ !!(borders & MID_TOP_L) ]);
  mvwhline(midwin,0,0,0,x);
  wattrset(midwin, f_attr[ !!(borders & (MID_TOP_L|MID_TOP_R)) ]);
  wmove(midwin,0,x);
  if ( focus_i == 1 )
    waddch(midwin, ACS_URCORNER);
  else if ( focus_i == 2 )
    waddch(midwin, ACS_ULCORNER);
  else
    waddch(midwin, ACS_HLINE);
  wattrset(midwin, f_attr[ !!(borders & MID_TOP_R) ]);
  whline(midwin,0,w-x-1);

  wattrset(midwin, f_attr[ !!(borders & MID_MID) ]);
  mvwvline(midwin,1,x,0,h-2);

  wattrset(midwin, f_attr[ !!(borders & MID_BOT_L) ]);
  mvwhline(midwin, h-1, 0, 0, x);
  wattrset(midwin, f_attr[ !!(borders & (MID_BOT_L|MID_BOT_R)) ]);
  wmove(midwin,h-1,x);
  if ( focus_i == 1 )
    waddch(midwin, ACS_LRCORNER);
  else if ( focus_i == 2 )
    waddch(midwin, ACS_LLCORNER);
  else
    waddch(midwin, ACS_HLINE);
  wattrset(midwin, f_attr[ !!(borders & MID_BOT_R) ]);
  whline(midwin,0,w-x-1);


  wattrset(stawin, f_attr[ !!(borders & STA_TOP) ]);
  mvwhline(stawin, 0, 0, 0, getmaxx(stawin));

  wrefresh(midwin);
  wrefresh(stawin);
}



int main(int argc, char ** argv)
{
  int i, esc, done, err = -1;

  sc68_init_t   init68;
  sc68_create_t create68;

  /* Initialize sc68 api. */
  memset(&init68, 0, sizeof(init68));
  init68.argc = argc;
  init68.argv = argv;
  init68.msg_handler = sc68_debug_cb;
  init68.debug_clr_mask = (1 << msg68_DEBUG) | (1 << msg68_TRACE);
  init68.debug_set_mask = 0;
  if ( sc68_init(&init68) ) {
    goto finish;
  }
  argc = init68.argc;
  argv = init68.argv;

  /* Create emulator instance */
  memset(&create68,0,sizeof(create68));
  create68.name        = "cdbg68";
  create68.emu68_debug = 1;
  sc68 = sc68_create(&create68);
  if (!sc68) {
    goto finish;
  }

  sc68_emulators(sc68, &emu68, &ios68);

  /* Options parsing */
  for ( i=1; i<argc; ++i ) {
    if ( argv[i][0] == '-' ) {
      if ( argv[i][1] == '-') {
        if ( !argv[i][2] ) {
          /* -- */
          ++i;
          break;
        }
        if (!strcmp(argv[i], "--version"))
          return version();
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "--usage"))
          return usage();
        else {
          fprintf(stderr,"cdbg68: invalid argument `%s'\n",argv[i]);
          return -1;
        }
      } else {
        int j;
        for ( j=1; argv[i][j]; ++j ) {
          switch ( argv[i][j] ) {
          case 'V': return version();
          case 'h': return usage();
          default:
            fprintf(stderr,"cdbg68: invalid argument `-%c'\n",argv[i][j]);
            return -1;
          }
        }
      }
    } else {
      break;
    }
  }

  err = Init();
  info(
    "Welcome to " PACKAGE_NAME " " PACKAGE_VERSION "!\n"
    " F1  or ESC-h for help\n"
    " F10 or ESC-x to exit\n");

  if ( !err && i<argc ) {
    const char * url = argv[i++];
    int track = -1;

    if ( i < argc) {
      errno = 0;
      track = strtol(argv[i++],0,0);
      if (errno) {
        fprintf(stderr,"cdbg68: invalid track number '%s'\n", argv[i-1]);
        err = -1;
      }
    }

    if ( ! err ) {
      debug("Loading '%s' #%d\n", url, track);
      LoadSc68(url, track);
    }
  }

  focus_i = 1;
  redraw_borders();
  done = !!err;
  esc  = 0;

  while ( ! done ) {
    int key;

    focwin = focus_list[focus_i];
    key = wgetch(focwin);

    if (esc) {
      esc = 0;
      switch ( key ) {
      case '0': case '1': case '2': case '3': case '4':
        focus_i = key - '0';
        redraw_borders();
        break;
      case 'x':
        done = 1; break;
      case 'h':
        /* help */
        debug ("help requested\n");
        break;
      case 'd':
        /* debug toggle */
        cmd_debug(0,0);
        break;
      }
    } else
    switch ( key ) {
    case 27:
      esc = 1; break;
    case KEY_F(1):
      /* help */
      debug ("help requested\n");
      break;
    case KEY_F(10):
      done = 1; break;

    case 9:
      focus_i = (focus_i + 1) % focus_n;
      redraw_borders();
      break;

    case KEY_BTAB:
      focus_i = (focus_i - 1 + focus_n) % focus_n;
      redraw_borders();
      break;

    default:
      switch (focus_i) {
      case  0:  err = reg_key(key); break;
      case  1:  err = dis_key(K_NRM, key); break;
      case  2:  err = mem_key(key); break;
      case  3:  err = msg_key(key); break;
      case  4:  err = sta_key(key); break;
      default:  done = err = -1; break;
      }
      if (!err)
        debug ("unhandled key code %d by window %d\n", key, focus_i);
      break;
    }
  }

finish:
  Cleanup();
  return err;
}
