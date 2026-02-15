/*
 * @file    mem,c
 * @brief   sc68 curses debugger - memory
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <curses.h>
#include <ctype.h>

#include "emu68/emu68.h"

extern emu68_t * emu68;

WINDOW * memwin;
static int mem_addr;

int mem_init(void)
{
  return 0;
}

void mem_destroy(void)
{
  if (memwin) {
    delwin(memwin);
    memwin = 0;
  }
}

void mem_draw(void)
{
}

static const char thex[16] = {
  '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

void mem_refresh(void)
{
  int addr, h, y, w, x;

  h = getmaxy(memwin);
  w = getmaxx(memwin) >> 2;

  for ( addr=mem_addr, y=0; y<h; ++y ) {
    for ( x=0; x<w; ++x, ++addr ) {
      const int chk = emu68_chkpeek(emu68, addr);
      const int val = emu68_peek(emu68, addr);
      int attr;

      switch ( chk & ( EMU68_R | EMU68_W ) ) {

      case 0:
        attr = COLOR_PAIR(COLOR_WHITE);
        break;

      case EMU68_R:
        attr = COLOR_PAIR(COLOR_YELLOW) | A_BOLD;
        break;

      case EMU68_W:
        attr = COLOR_PAIR(COLOR_CYAN) | A_BOLD;
        break;

      case EMU68_R | EMU68_W:
        attr = COLOR_PAIR(COLOR_GREEN) | A_BOLD;
        break;
      }
      wattrset(memwin,attr);
      mvwaddch(memwin, y, x*3, thex[ val >> 4 ]);
      waddch(memwin, thex[val & 15 ]);
      mvwaddch(memwin, y, w*3+x, isgraph(val) ? val : '.');
    }
    wclrtoeol(memwin);
  }
  wrefresh(memwin);
}

int mem_get(void)
{
  return mem_addr;
}

void mem_set(int address)
{
  if (address == -1) {
    address = emu68->reg.pc;
  }
  mem_addr = address;
  mem_refresh();
}

int mem_key(int key)
{
  int rkey = 0;
  switch ( key ) {

  case KEY_HOME: case '=':
    mem_set(-1);
    break;

  case '0':  case '1':  case '2':  case '3':
  case '4':  case '5':  case '6':  case '7':
    mem_set(emu68->reg.a[key-'0']);
    break;

  case KEY_UP:
    mem_set(mem_addr - 8); break;

  case KEY_DOWN:
    mem_set(mem_addr + 8); break;

  case KEY_LEFT:
    mem_set(mem_addr - 1); break;

  case KEY_RIGHT:
    mem_set(mem_addr + 1); break;
    break;

  case KEY_NPAGE: case KEY_PPAGE: {
    int h = getmaxy(memwin);
    int s = key == KEY_NPAGE ? 8 : -8;
    mem_set(mem_addr+s*(h-1)); break;
  } break;

  default:
    rkey = key;
    break;
  }

  return rkey;
}
