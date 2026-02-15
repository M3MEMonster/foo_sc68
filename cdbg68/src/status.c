/*
 * @file    status,c
 * @brief   sc68 curses debugger - status
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "status.h"

extern WINDOW * focwin;

static char sta_buffer[512];

int sta_init(void)
{
  return 0;
}

void sta_destroy(void)
{
  if (stawin) {
    delwin(stawin);
    stawin = 0;
  }
}

void sta_draw(void)
{
  int    w = getmaxx(stawin);
  int attr = COLOR_PAIR(COLOR_WHITE);

  /* draw separator line */
  mvwhline(stawin, 0, 0, 0, w);
}

void sta_refresh(void)
{
  wattrset(stawin, A_NORMAL);
  mvwaddstr(stawin, 1, 0, sta_buffer);
  wrefresh(stawin);
}

void sta_set(const char * fmt, ...)
{
  va_list list;

  va_start(list, fmt);
  vsnprintf(sta_buffer, sizeof(sta_buffer), fmt, list);
  sta_buffer[ sizeof(sta_buffer) - 1 ] = 0;
  sta_refresh();
}

int sta_key(int key)
{
  return key;
}
