/*
 * @file    win,c
 * @brief   sc68 curses debugger - windows
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

#include "win.h"

int win_is_root(win_t * win) 
{
  return win->root == 0;
}

/* Default callbaks */

void windef_getbox(win_t * win, box_t * box)
{
  box->x = win->win->start_x;
  box->y = win->win->start_y;
  box->w = win->win->width;
  box->h = win->win->height;
}

void windef_setbox(win_t * win, box_t * box)
{
  int old_x = win->win->start_x;
  int old_y = win->win->start_y;
  int old_w = win->win->width;
  int old_h = win->win->height;

  if ( box->w != old_w || box->h != old_h ) {
    wresize(win->win, box->h, box->w);
  }
  if ( box->x != old_x || box->y != old_y ) {
    wmove(win->win, box->y, box->x);
  }

  box->x = win->win->start_x;
  box->y = win->win->start_y;
  box->w = win->win->width;
  box->h = win->win->height;
}
