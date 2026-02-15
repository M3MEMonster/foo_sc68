/*
 * @file    ds68_dbg.cpp
 * @brief   sc68 for directshow - debug
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (c) 1998-2014 Benjamin Gerard
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
#include "config.h"
#endif

#define _CRT_SECURE_NO_WARNINGS 1


#include "ds68_dbg.h"

#include <stdio.h>
#include <string.h>
#include <Shlwapi.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

const char dbg_pre[] = "sc68-splitter: ";

void dbg_va(const char * fmt, va_list list)
{
  char temp[512];
  const int max = sizeof(temp)/sizeof(*temp);
  int i;
  for (i=0; dbg_pre[i]; ++i)
    temp[i] = dbg_pre[i];
  i += _snprintf(temp+i,max-i,"[%08x] ",GetCurrentThreadId());
  vsnprintf(temp+i,max-i,fmt,list);
  temp[max-1] = 0;
  OutputDebugStringA(temp);
}

void dbg(const char * fmt, ...)
{
  va_list list;
  va_start(list,fmt);
  dbg_va(fmt,list);
  va_end(list);
}

void dbg_va(const wchar_t * fmt, va_list list)
{
  wchar_t temp[512];
  const int max = sizeof(temp)/sizeof(*temp);
  int i;
  for (i=0; dbg_pre[i]; ++i)
    temp[i] = dbg_pre[i];
  i += _snwprintf(temp+i,max-i,L"[%08x] ",GetCurrentThreadId());
  _vsnwprintf(temp+i,max-i,fmt,list);
  temp[max-1] = 0;
  OutputDebugStringW(temp);
}

void dbg(const wchar_t * fmt, ...)
{
  va_list list;
  va_start(list,fmt);
  dbg_va(fmt,list);
  va_end(list);
}

void dbg_error(int err, const char * fmt, ...)
{
  char str[512];
  int max = sizeof(str), i;

  i = FormatMessageA(
    FORMAT_MESSAGE_FROM_SYSTEM, 0, err,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    str, max, 0);
  if (i<=0)
    i = _snprintf(str,max,"#%d",err);
  else
    while (--i > 0 && isspace(str[i])) ;

  dbg("ERROR: %s\n",str);
  if (fmt) {
    va_list list;
    va_start(list,fmt);
    vsnprintf(str,max,fmt,list);
    va_end(list);
    dbg("ERROR: %s", str);
  }
  str[max-1] = 0;
}

void dbg_error(int err, const wchar_t * fmt, ...)
{
  wchar_t str[512];
  int max = sizeof(str)/sizeof(*str), i;

  i = FormatMessageW(
    FORMAT_MESSAGE_FROM_SYSTEM, 0, err,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    str, max, 0);
  if (i<=0)
    i = wnsprintfW(str,max,L"#%d",err);
  else
    while (--i > 0 && isspace(str[i])) ;

  dbg("ERROR: %s\n",str);
  if (fmt) {
    va_list list;
    va_start(list,fmt);
    wvnsprintfW(str,max,fmt,list);
    va_end(list);
    dbg("ERROR: %s", str);
  }
  str[max-1] = 0;
}

#ifdef _DEBUG
void msg_for_sc68(int bit, void * cookie, const char * fmt, va_list list)
{
  dbg_va(fmt,list);
}
#endif
