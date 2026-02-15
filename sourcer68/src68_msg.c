/*
 * @file    src68_msg.c
 * @brief   print messages.
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
# include "config.h"
#endif

#include "src68_msg.h"

#ifdef USE_FILE68
# include <sc68/file68_msg.h>
extern int sourcer68_cat; /* defined in sourcer68.c */
#endif
#include <stdio.h>
#include <string.h>

extern const char prg[];                      /* defined in sourcer68.c */

void dmsg_va(const char * fmt, va_list list)
{
#ifdef USE_FILE68
  msg68_va(sourcer68_cat,fmt, list);
#elif defined(DEBUG)
  vfprintf(stderr,fmt,list);
  fflush(stderr);
#endif
}

void dmsg(const char * fmt, ...)
{
  va_list list;
  va_start(list,fmt);
  dmsg_va(fmt,list);
  va_end(list);
}

void wmsg_va(int addr, const char * fmt, va_list list)
{
  int l;

  if (addr != -1)
    fprintf(stderr, "warning:$%06X: ", addr);
  else
    fprintf(stderr, "warning: ");
  vfprintf(stderr, fmt, list);
  if ( (l = strlen(fmt)) > 0 && fmt[l-1] != '\n')
    fputc('\n',stderr);
  fflush(stderr);
}

void wmsg(int addr, const char * fmt, ...)
{
  va_list list;
  va_start(list,fmt);
  wmsg_va(addr, fmt, list);
  va_end(list);
}

void emsg_va(int addr, const char * fmt, va_list list)
{
  int l;

  if (addr == -1)
    fprintf(stderr,"%s: ", prg);
  else
    fprintf(stderr,"%s: @$%06x", prg, addr);
  vfprintf(stderr,fmt,list);
  if ( (l = strlen(fmt)) > 0 && fmt[l-1] != '\n')
    fputc('\n', stderr);
  fflush(stderr);
}

void emsg(int addr, const char * fmt, ...)
{
  va_list list;
  va_start(list,fmt);
  emsg_va(addr, fmt, list);
  va_end(list);
}
