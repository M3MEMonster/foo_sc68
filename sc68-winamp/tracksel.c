/*
 * @file    tracksel.c
 * @brief   sc68-ng plugin for winamp 5.5 - track selection dialog
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

/* winamp 2 */
#include "winamp/in2.h"

/* libc */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

/* sc68 */
#include "sc68/sc68.h"

static const int magic = ('T'<<24)|('S'<<16)|('E'<<8)|'L';

struct selcookie_s {
  int          magic;                   /* magic id      */
  HINSTANCE    hinst;                   /* hinst ?       */
  HWND         hwnd;                    /* Parent window */
  sc68_disk_t  disk;                    /* sc68 disk     */
};
typedef struct selcookie_s cookie_t;

static inline int ismagic(cookie_t * cookie) {
  return cookie && cookie->magic == magic;
}

static void del_cookie(cookie_t * cookie)
{
  DBG("trackselect -- <%p>\n", (void*)cookie);
  assert(ismagic(cookie));
  if (ismagic(cookie))
    free(cookie);
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
      val->s = "trackselect";
    else if (keyis(SC68_DIAL_WAIT))
      val->i = 1;
    else if (keyis("instance"))
      val->s = (const char *) cookie->hinst;
    else if (keyis("parent"))
      val->s = (const char *) cookie->hwnd;
    else if (keyis("disk"))
      val->s = (const char *) cookie->disk;
    else break;
    return 0;
  }
  return 1;
}

/* Only exported function. */
int tracksel_dialog(HINSTANCE hinst, HWND hwnd, sc68_disk_t disk)
{
  int res = -1;
  cookie_t * cookie = (cookie_t *) malloc(sizeof(cookie_t));
  if (cookie) {
    cookie->magic = magic;
    cookie->hinst = hinst;
    cookie->hwnd  = hwnd;
    cookie->disk  = disk;
    res = sc68_cntl(0, SC68_DIAL, cookie, cntl);
  }
  DBG("*%d*\n", res);
  return res;
}
