/*
 * @file    transcoder.c
 * @brief   sc68-ng plugin for winamp 5.5 - transcoder functions
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
#include <stdint.h>
#include <ctype.h>

/* sc68 */
#include <sc68/sc68.h>
#include <sc68/file68_features.h>
#include <sc68/file68_str.h>

/* windows */
#include <windows.h>

/* winamp */
#include "winamp/in2.h"

/* in_sc68.c */
EXTERN In_Module g_mod;
EXTERN HMODULE g_cfgdll;

/* tracksel.c */
EXTERN int tracksel_dialog(HINSTANCE hinst, HWND hwnd, sc68_disk_t disk);

struct transcon {
  sc68_t * sc68;                        /* sc68 instance               */
  int done;                             /* 0:not done, 1:done -1:error */
  int allin1;                           /* 1:all tracks at once        */
  size_t pcm;                           /* pcm counter                 */
};

static int get_track_from_uri(const char * uri)
{
  /* $$$ TODO: this function ! */
  return 0;
}

EXPORT
/**
 * Open sc68 transcoder.
 *
 * @param   uri  URI to transcode.
 * @param   siz  pointer to expected raw output size (progress bar)
 * @param   bps  pointer to output bit-per-sample
 * @param   nch  pointer to output number of channel
 * @param   spr  pointer to output sampling rate
 * @return  Transcoding context struct (sc68_t right now)
 * @retval  0 on errror
 */
intptr_t winampGetExtendedRead_open(
  const char *uri,int *siz, int *bps, int *nch, int *spr)
{
  struct transcon * trc;
  int res, ms, tracks, track, asid = 0;

  DBG("(\"%s\")\n", uri);

  trc = malloc(sizeof(struct transcon));
  if (!trc)
    goto error;
  trc->pcm = 0;
  trc->allin1 = 0;
  trc->sc68 = sc68_create(0);
  if (!trc->sc68)
    goto error;
  if (sc68_load_uri(trc->sc68, uri))
    goto error;
  if (tracks = sc68_cntl(trc->sc68,SC68_GET_TRACKS), tracks <= 0)
    goto error;
  if (tracks < 2 &&
      sc68_cntl(trc->sc68, SC68_CAN_ASID, SC68_DSK_TRACK) <= 0) {
    DBG("only the one track and can't aSID, no need for a dialiog\n");
    track = 1;
  } else {
    track = get_track_from_uri(uri);
    if (track < 1 || track > tracks) {
      sc68_disk_t disk = 0;
      if (!sc68_cntl(trc->sc68,SC68_GET_DISK,&disk)) {
        track = tracksel_dialog(DLGHINST, DLGHWND, disk);
        if (track >= 0) {
          asid = (track >> 8) & 3;
          track &= 255;
        }
        else if (track == -1) {
          DBG("could not open the track-select dialog, fallback !!\n");
          track = 0;
        } else {
          DBG("Abort by user ? code=%d !!\n", track);
          track = -1;
        }
      }
      else {
        DBG("could not retrieve sc68 disk !!\n");
        track = -1;
      }
    }
    if (track == 0) {
      DBG("transcode all tracks as one (asid=%d)\n", asid);
      trc->allin1 = 1;
      track = 1;
    } else if (track > 0 && track <= tracks) {
      DBG("transcode track #%d (asid=%d)\n", track, asid);
    } else {
      goto error;
    }
  }
  sc68_cntl(trc->sc68, SC68_SET_ASID, asid);
  if (sc68_play(trc->sc68, track, 1))
    goto error;
  res = sc68_process(trc->sc68, 0, 0);
  if (res == SC68_ERROR)
    goto error;
  trc->done = !!(res & SC68_END);
  ms = sc68_cntl(trc->sc68, trc->allin1 ? SC68_GET_DSKLEN : SC68_GET_LEN);
  *nch = 2;
  *spr = sc68_cntl(trc->sc68, SC68_GET_SPR);
  *bps = 16;
  *siz = (int) ((uint64_t)ms * (*spr) / 1000) << 2;
  return (intptr_t)trc;

error:
  if (trc) {
    sc68_destroy(trc->sc68);
    free(trc);
  }
  DBG("=> FAILED\n");
  return 0;
}

EXPORT
/**
 * Run sc68 transcoder.
 *
 * @param   hdl  Pointer to transcoder context
 * @patam   dst  PCM buffer to fill
 * @param   len  dst size in byte
 * @param   end  pointer to a variable set by winamp to query the transcoder
 *               to kindly exit (abort)
 * @return  The number of byte actually filled in dst
 * @retval  0 to notify the end
 */
intptr_t winampGetExtendedRead_getData(
  intptr_t hdl, char *dst, int len, int *end)
{
  struct transcon * trc = (struct transcon *) hdl;
  int pcm, res;

  if (*end || trc->done)
    return 0;
  pcm = len >> 2;
  res = sc68_process(trc->sc68, dst, &pcm);
  if (res == SC68_ERROR) {
    trc->done = -1;
    pcm = 0;
    goto error;
  }
  trc->pcm += pcm;
  trc->done = (res & SC68_END) ||
    ((res & (SC68_LOOP|SC68_CHANGE)) && !trc->allin1);
  pcm <<= 2;
error:
  return pcm;
}

EXPORT
/**
 * Close sc68 transcoder.
 *
 * @param   hdl  Pointer to transcoder context
 */
void winampGetExtendedRead_close(intptr_t hdl)
{
  struct transcon * trc = (struct transcon *) hdl;
  if (trc) {
    sc68_destroy(trc->sc68);
    free(trc);
  }
}
