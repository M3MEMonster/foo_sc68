/*
 * @file    configdlg.c
 * @brief   sc68-ng plugin for winamp 5.5 - configuration dialog
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

/* windows */
#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include <commctrl.h>

/* libc */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* sc68 */
#include "sc68/sc68.h"
#include "sc68/file68_opt.h"

static const int magic = ('C'<<24)|('O'<<16)|('N'<<8)|'F';

struct cfgcookie_s {
  int          magic;                   /* Points on me  */
  HINSTANCE    hinst;                   /* DLL hmodule   */
  HWND         hwnd;                    /* Parent window */
};
typedef struct cfgcookie_s cookie_t;

static inline int ismagic(cookie_t * cookie) {
  return cookie && cookie->magic == magic;
}

static inline void del_cookie(cookie_t * cookie)
{
  DBG("config <%p>\n", (void *) cookie);
  if (ismagic(cookie))
    free(cookie);
}

static inline int get_int(const char * key, int def)
{
  const option68_t * opt = option68_get(key,opt68_ISSET);
  return opt ? !!opt->val.num : def;
}

#define keyis(N) !strcmp(key,N)

static int cntl(void * _cookie, const char * key, int op, sc68_dialval_t *val)
{
  cookie_t * cookie = (cookie_t *) _cookie;

  assert(ismagic(cookie));
  assert(key && val);

  if (!key || !val || !ismagic(cookie))
    return -1;

  switch (op) {
  case SC68_DIAL_CALL:
    if (keyis(SC68_DIAL_KILL))
      del_cookie(cookie);
    else if (keyis(SC68_DIAL_HELLO))
      val->s = "config";
    else if (keyis(SC68_DIAL_WAIT))
      val->i = 1;
    else if (keyis("instance"))
      val->s = (const char *) cookie->hinst;
    else if (keyis("parent"))
      val->s = (const char *) cookie->hwnd;
    else if (keyis("asid"))
      set_asid(val->i);
    else break;
    return 0;
  }
  return 1;
}

/* Only exported function. */
int config_dialog(HINSTANCE hinst, HWND hwnd)
{
  int res = -1;
  cookie_t * cookie = (cookie_t *) malloc(sizeof(cookie_t));
  if (cookie) {
    cookie->magic = magic;
    cookie->hinst = hinst;
    cookie->hwnd  = hwnd;
    res = sc68_cntl(0, SC68_DIAL, cookie, cntl);
  }
  DBG("*%d*\n", res);
  return res;
}
