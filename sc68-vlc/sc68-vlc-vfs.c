/*
 * @file    sc68-vlc-vfs.c
 * @brief   sc68 vfs for vlc streams.
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

#define _(str)  (str)
#define N_(str) (str)

#include <vlc_common.h>
#include <vlc_stream.h>

/* #include <vlc_input.h> */
/* #include <vlc_plugin.h> */
/* #include <vlc_demux.h> */
/* #include <vlc_codec.h> */
/* #include <vlc_meta.h> */

#include <sc68/file68_vfs_def.h>
#include <string.h>
#include <stdlib.h>

#ifndef UNUSED
# define UNUSED(X) (void)(X)
#endif

/** vfs vlc structure. */
typedef struct {
  vfs68_t   vfs;                        /**< vfs function.   */
  stream_t *f;                          /**< vlc slaved stream.  */
  /** !!! always last !!! */
  char uri[1];                          /**< URI or filename */
} vfs68_vlc_t;

static const char * isf_name(vfs68_t * vfs)
{
  vfs68_vlc_t * isf = (vfs68_vlc_t *)vfs; UNUSED(isf);
  return isf->uri;
}

static int isf_open(vfs68_t * vfs)
{
  vfs68_vlc_t * isf = (vfs68_vlc_t *) vfs; UNUSED(isf);
  return 0;
}

static int isf_close(vfs68_t * vfs)
{
  vfs68_vlc_t * isf = (vfs68_vlc_t *) vfs; UNUSED(isf);
  return 0;
}

static int isf_read(vfs68_t * vfs, void * data, int n)
{
  vfs68_vlc_t * isf = (vfs68_vlc_t *)vfs;
  return stream_Read(isf->f, data, n);
}

static int isf_write(vfs68_t * vfs, const void * data, int n)
{
  vfs68_vlc_t * isf = (vfs68_vlc_t *)vfs; UNUSED(isf);
  return -1;
}

static int isf_flush(vfs68_t * vfs)
{
  vfs68_vlc_t * isf = (vfs68_vlc_t *)vfs; UNUSED(isf);
  return -1;
}

static int isf_length(vfs68_t * vfs)
{
  vfs68_vlc_t * isf = (vfs68_vlc_t *)vfs;
  return stream_Size(isf->f);
}

static int isf_tell(vfs68_t * vfs)
{
  vfs68_vlc_t * isf = (vfs68_vlc_t *)vfs;

  return stream_Tell(isf->f);
}

static int isf_seek(vfs68_t * vfs, int offset)
{
  vfs68_vlc_t * isf = (vfs68_vlc_t *)vfs;
  int cur = isf_tell(vfs);

  return (cur == -1)
    ? -1
    : stream_Seek(isf->f, cur+offset)
    ;
}

static void isf_destroy(vfs68_t * vfs)
{
  free(vfs);
}

static const vfs68_t vfs68_vlc = {
  isf_name,
  isf_open,
  isf_close,
  isf_read,
  isf_write,
  isf_flush,
  isf_length,
  isf_tell,
  isf_seek,
  isf_seek,
  isf_destroy
};

vfs68_t * vfs68_vlc_create(stream_t * vlc, const char * uri)
{
  vfs68_vlc_t * isf;
  int l = 0;

  if (!vlc) {
    return 0;
  }
  if (uri)
    l = strlen(uri);

  isf = malloc(sizeof(vfs68_vlc_t) + l);
  if (!isf)
    return 0;

  /* copy vfs functions. */
  memcpy(&isf->vfs, &vfs68_vlc, sizeof(vfs68_vlc));

  /* set vlc stream handle. */
  isf->f = vlc;

  /* copy URI */
  memcpy(isf->uri,uri,l);
  isf->uri[l] = 0;                      /* zero terminated */

  return &isf->vfs;
}
