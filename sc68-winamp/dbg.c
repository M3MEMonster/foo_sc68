/*
 * @file    dbg.c
 * @brief   sc68-ng plugin for winamp 5.5 - debug messages
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

/* generated config header */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* winamp sc68 declarations */
#include "wasc68.h"

/* libc */
#include <stdio.h>
#include <string.h>

/* windows */
#include <windows.h>

static void dbgmsg(int prefix, const char * fmt, va_list list)
{
  static const char pref[] = "wasc68 : ";
  char s[1024];
  int  i = 0;

  if (prefix) {
    strcpy(s,pref);
    i = sizeof(pref)-1;
  }
  vsnprintf(s+i, sizeof(s)-i, fmt, list);
  OutputDebugStringA(s);
}


void dbg_va(const char * fmt, va_list list)
{
  dbgmsg(1,fmt,list);
}

void dbg(const char * fmt, ...)
{
  va_list list;
  va_start(list,fmt);
  dbgmsg(1,fmt,list);
  va_end(list);
}

EXTERN int wasc68_cat;

void msgfct(const int bit, void *userdata, const char *fmt, va_list list)
{
    dbgmsg(bit == wasc68_cat, fmt,list);
}
