/*
 * @file    src68_fmt.c
 * @brief   source formatter.
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

#include "src68_fmt.h"
#include "src68_msg.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef USE_FILE68
#include <sc68/file68_vfs.h>
typedef vfs68_t * out_t;
#else
#include <stdio.h>
typedef FILE * out_t;
#endif

enum {
  FMT_COL_LABEL,FMT_COL_MNEMONIC,FMT_COL_OPERANDS,FMT_COL_REMARK
};

struct fmt_s {
  out_t out;                            /* output file */
  int error;                            /* error number */
  int lineno;                           /* current line number */

  struct {
    int ch;                             /* using space or tab. */
    int w;                              /* width of tab char.  */
    int stops[4];                       /* tab stops values.   */
  } tab;

  int col;                              /* current columns */
  int pos;                              /* position (proper tab) */
  int cur;                              /* cursor position (can be ahead) */
};

static int check(fmt_t * fmt)
{
  if (!fmt)
    return FMT_EARG;
  return fmt->error;
}


static int outwrite(fmt_t * fmt, const void * dat, int len)
{
#ifdef USE_FILE68
  int n = vfs68_write(fmt->out, dat, len);
#else
  int n = fwrite(dat, 1, len, fmt->out);
#endif
  if (n != len) {
    fmt->error = FMT_EIO;
    return -1;
  }
  return 0;
}

int fmt_tab(fmt_t * fmt)
{
  int newpos;
  if (check(fmt)<0)
    return -1;
  if (fmt->col < FMT_COL_REMARK)
    ++fmt->col;
  newpos = fmt->tab.stops[fmt->col];
  if (newpos <= fmt->cur) {
    fmt->cur ++;
  } else {
    fmt->cur = newpos;
  }
  return 0;
}

int fmt_cat(fmt_t * fmt, const void * dat, int len)
{
  if (!len)
    return 0;
  if (check(fmt) < 0)
    return -1;
  if (!dat || len < 0) {
    fmt->error = FMT_EARG;
    return -1;
  }
  assert(fmt->pos <= fmt->cur);
  while (fmt->pos < fmt->cur) {
    static const char c = ' ';
    if (outwrite(fmt,&c,1) < 0)
      return -1;
    fmt->pos ++;
  }

  if (outwrite(fmt, dat, len) < 0)
    return -1;

  fmt->cur = fmt->pos += len;

  return 0;
}

int fmt_vputf(fmt_t * fmt, const char * pft, va_list list)
{
  char tmp[128];
  int len;
  len = vsnprintf(tmp, sizeof(tmp), pft, list);
  return fmt_cat(fmt,tmp,len);
}

int fmt_puts(fmt_t * fmt,const char * str)
{
  return fmt_cat(fmt, str, strlen(str));
}

int fmt_putf(fmt_t * fmt, const char * pft, ...)
{
  int err;
  va_list list;
  va_start(list, pft);
  err = fmt_vputf(fmt, pft, list);
  va_end(list);
  return err;
}

int fmt_eol(fmt_t * fmt, int always)
{
  static const char c = '\n';
  if (check(fmt) < 0)
    return -1;

  if (always || fmt->pos > 0) {
    if (outwrite(fmt,&c,1) < 0)
      return -1;
    fmt->lineno ++;
    fmt->col = FMT_COL_LABEL;
    fmt->pos = 0;
    fmt->cur = 0;
  }
  return 0;
}

void fmt_del(fmt_t * fmt)
{
  free(fmt);
}

fmt_t * fmt_new(void * out)
{
  fmt_t * fmt = !out ? 0 : calloc(1,sizeof(*fmt));
  if (fmt) {
    /* setup output */
    fmt->out = out;

    /* setup tabulation mode. */
    fmt->tab.ch = '\t';
    fmt->tab.w  = 8;
    fmt->tab.stops[0] = 0;
    fmt->tab.stops[1] = 13;
    fmt->tab.stops[2] = 22;
    fmt->tab.stops[3] = 52;
  }
  return fmt;
}
