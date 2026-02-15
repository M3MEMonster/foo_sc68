/*
 * @file    mksc68_str.c
 * @brief   various string functions
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

#include "mksc68_dsk.h"
#include "mksc68_msg.h"

#include <sc68/file68.h>
#include <sc68/file68_str.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>

int str_tracklist(const char ** ptr_tl, int * a, int * b)
{
  int v, c, pass, tracks;
  const char * tracklist = *ptr_tl;

  if (!tracklist || !*tracklist)
    return 0;

  tracks = dsk_get_tracks();
  if (tracks <= 0) {
    msgerr("no %s\n", tracks ? "disk" : "track");
    return -1;
  }

  /* handle special case matching "all" */
  if (!strcmp68(*ptr_tl,"all")) {
    *ptr_tl += 3;
    *a = 1;
    *b = tracks;
    return 1;
  }

  for (pass=0; pass<2; ++pass) {
    v = *tracklist++ - '0';
    if (v < 0 || v > 9) {
      *ptr_tl = --tracklist;
      msgerr("digit expected in track-list\n");
      return -1;
    }
    for (c = *tracklist++; c >= '0' && c <= '9'; c = *tracklist++) {
      v = v * 10 + (c - '0');
      if (v > 99) {
        break;
      }
    }
    if (v == 0) v = tracks;
    if (v > tracks) {
      *ptr_tl = --tracklist;
      msgerr("track number out of range\n");
      return -1;
    }
    *b = v;
    if (!pass) *a = v;

    switch (c) {
    case 0:                             /* End */
      --tracklist;
    case ',':
      pass = 1;
      break;

    case '-':
      if (pass) {
        *ptr_tl = --tracklist;
        msgerr("unexpected '-' char in track-list\n");
        return -1;
      }
      break;

    default:
      *ptr_tl = --tracklist;
      msgerr("unexpected '%c' char in track-list\n", c);
      return -1;
    }
  }

  *ptr_tl = tracklist;
  return 1;
}

static int time_parse(const char ** ptr_time, int * ptr_ms, int endchar)
{
  const char * s = *ptr_time, * err = "";
  int ms = 0, ret = -1, c;
  char tmp[128];

  if (!s || !*s || *s == endchar)
    return 0;

  c = *s;
  /* detect `inf'inite */
  if (c == 'i' && s[1] == 'n' && s[2] == 'f' && s[3] == endchar) {
    ms = -1;
    s += 3;
    ret = 1;
    goto finish;
  }

  if (c >= '0' && c < '9') {
    do {
      ms = (ms * 10) + ( c - '0' );
      c = *++s;
    } while (c >= '0' && c < '9');
  } else {
    err = "digit expected";
    goto finish;
  }

  switch (c) {
  case 'h':                             /* hours   */
    ms *= 60;
  case 'm':                             /* minutes */
    ms *= 60;
  case 's':                             /* seconds */
    ms *= 1000;
    c = *++s;
    break;

  case ':': case ',':                   /* hh:mm:ss,ms */
  {
    int l = 0, acu = ms;

    do {
      if (c == ',') {
        acu *= 1000;
        l = 3;
      } else {
        acu *= 60;
        ++l;
      }
      c = *++s;
      if (c < '0' || c > '9') {
        err = "digit expected";
        goto finish;
      }
      ms = 0;
      do {
        ms = (ms * 10) + ( c - '0' );
        c = *++s;
      } while (c >= '0' && c < '9');
      acu += ms;
    } while ( (l < 2 && c == ':') || (l < 3 && c == ',') );
    ms = acu;
    if (l<3)
      ms *= 1000;
  } break;
  }

  if (c == endchar) {
    ret = 1;
  } else {
    sprintf(tmp, "unexpected char #%d `%c'",c, isgraph(c)?c:'?');
    err = tmp;
  }

finish:
  *ptr_time = s;
  *ptr_ms   = ms;

  switch (ret) {
  case 0:
  case 1:
    if (ret == 1) {
      int h,m,s;
      h = ms / (1000 * 60 * 60);
      ms -= h * 1000 * 60 * 60;
      m = ms / (1000 * 60);
      ms -= m * 1000 * 60;
      s = ms / 1000;
      ms -= s * 1000;
      msgdbg("time: %dms -> %02dh %02dm %02d,%03d\n",
             *ptr_ms, h, m, s, ms);
    }
    break;

  case -1:
    msgerr("syntax error in time string (%s)\n", err);
    break;
  default:
    assert(!"unexpected return code");
    ret = -1;
  }

  return ret;
}

int str_time_stamp(const char ** ptr_time, int * ptr_ms)
{
  int ret = -1, dummy;
  assert(ptr_time);

  if (ptr_time) {
    if (!ptr_ms) ptr_ms = &dummy;
    ret = time_parse(ptr_time, ptr_ms, 0);
  }
  return - (ret != 1);
}

int str_time_range(const char ** ptr_time, int * from, int * to)
{
  int ret = -1, dummy;

  assert(ptr_time);
  if (ptr_time) {
    if (!from) from = &dummy;
    ret = time_parse(ptr_time, from, '-');
    if (ret == 1) {
      assert(**ptr_time == '-');
      ++(*ptr_time);
      if (*from == -1) {
        msgerr("syntax error in time range (infinite in range)\n");
        ret = -1;
      } else {
        if (!to) to = &dummy;
        ret = time_parse(ptr_time, to, 0);
      }
    }
  }
  return - (ret != 1);
}

char * str_timefmt(char * buf, int len, unsigned int ms)
{
  char tmp[64];
  int n, h, m, s;

  h = (ms / 3600000u) % 24u;
  ms %= 3600000u;
  m = ms / 60000u;
  ms %= 60000u;
  s = ms / 1000u;
  ms %= 1000u;
  ms /= 10u;

  if (h)
    n = snprintf(tmp, sizeof(tmp), "%02uh%02u'%02u\"%02u", h,m,s,ms);
  else
    n = snprintf(tmp, sizeof(tmp), "%02u:%02u,%02u",m,s,ms);
  n = n < len ? n : len-1;
  strncpy(buf, tmp, n);
  buf[n] = 0;

  return buf;
}

static int catflag(char * tmp, int i, int max, int bit, const char * l) {
  if (bit) {
    if (i && i<max)
      tmp[i++] = ',';
    while (i<max && (tmp[i] = *l++))
      ++i;
  }
  return i;
}

char * str_hardware(char * const buf, int max, int hwbit)
{
  int i;
  hwflags68_t hw;
  hw = hwbit;

  i = 0;
  i = catflag(buf, i, max, hw & SC68_AGA, "AGA");
  i = catflag(buf, i, max, hw & SC68_PSG, "PSG");
  i = catflag(buf, i, max, hw & SC68_DMA, "DMA");
  if (! (hw & SC68_XTD) ) {
    i = catflag(buf, i, max, 1, "?");
  } else {
    i = catflag(buf, i, max, hw & SC68_LMC, "LMC");
    i = catflag(buf, i, max, hw & SC68_MFP_TA, "TiA");
    i = catflag(buf, i, max, hw & SC68_MFP_TB, "TiB");
    i = catflag(buf, i, max, hw & SC68_MFP_TC, "TiC");
    i = catflag(buf, i, max, hw & SC68_MFP_TD, "TiD");
    i = catflag(buf, i, max, hw & SC68_HBL, "HBL");
    i = catflag(buf, i, max, hw & SC68_BLT, "BLT");
    i = catflag(buf, i, max, hw & SC68_DSP, "DSP");
  }
  if (!i)
    i = catflag(buf, 0, max, 1, "N/A");

  buf[i<max ? i : max-1] = 0;

  return buf;
}

#ifndef PATHSEP
# define PATHSEP '/'
# if defined(NATIVE_WIN32) && ! defined(PATHSEP2)
#  define PATHSEP2 '\\'
# endif
#endif

const char * str_fileext(const char * path)
{
  int l, p;

  if (!path)
    return 0;

  for (l=0; path[l]; ++l)               /* strlen */
    ;

  p = l;                                /* default to "" */
  while (--l > 0) {
    int c = path[l];

    if (c == PATHSEP)
      break;
#ifdef PATHSEP2
    if (c == PATHSEP2)
      break;
#endif
#ifdef NATIVE_WIN32
    if (c == ':' && l == 1)
      break;
#endif
    if (c == '.') {
      p = l;
      break;
    }
  }

  return path+p;
}
