/*
 * @file    mksc68_dsk.c
 * @brief   disk functions.
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
#include "config.h"
#endif

#include "mksc68_dsk.h"
#include "mksc68_msg.h"
#include "mksc68_cmd.h"
#include "mksc68_tag.h"
#include "mksc68_str.h"
#include "mksc68_snd.h"

#include <sc68/file68.h>
#include <sc68/file68_str.h>
#include <sc68/sc68.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

typedef struct dsk_s dsk_t;
struct dsk_s {
  unsigned int modified : 1;            /* disk has been modified */
  char * filename;                      /* filename               */
  disk68_t * disk;                      /* actual sc68 disk       */
  int cur_trk;                          /* current track          */
};

static dsk_t dsk;

int dsk_has_disk(void)
{
  return !! dsk.disk;
}

void * dsk_get_disk(void)
{
  return dsk.disk;
}

int dsk_get_tracks(void)
{
  return dsk.disk ? dsk.disk->nb_mus : -1;
}

static int has_disk(void)
{
  int ret = dsk_has_disk();
  if (!ret)
    msgerr("no disk\n");
  return ret;
}

static int is_valid_disk(void)
{
  if (!has_disk())
    return 0;
  if (dsk.disk->nb_mus <= 0) {
    msgerr("disk has no track.\n");
    return 0;
  }
  return 1;
}

static int is_valid_track(int trk)
{
  if (!is_valid_disk())
    return 0;
  if (trk <= 0 || trk > dsk.disk->nb_mus) {
    msgerr("track number #%d out of range [1..%d]\n", trk, dsk.disk->nb_mus);
    return 0;
  }
  return 1;
}

int dsk_load(const char * uri, int merge, int force)
{
  int ret = -1;
  disk68_t * newdisk = 0;

  if (!force && !merge && dsk.modified) {
    msgerr("modified data (save or try --force)\n");
    goto error;
  }

  newdisk = file68_load_uri(uri);
  if (!newdisk) {
    msgerr("failed to load \"%s\"\n",uri);
    goto error;
  }

  /* Fix `not implemented' below at least if not a real merge. */
  if (merge && !dsk_has_disk())
    merge = 0;

  if (!merge) {
    dsk_kill();
    dsk.filename = strdup68(uri);
    dsk.disk = newdisk;
    newdisk  = 0;
    dsk.cur_trk = dsk.disk->def_mus;
    dsk.modified = 0;
  } else {
    msgerr("not implemented\n");
    goto error;
  }

  ret = 0;
error:
  if (ret)
    free(newdisk);
  return ret;
}

int dsk_merge(const char * uri)
{
  msgerr("not implemented\n");
  return -1;
}

/* @param  version  0:auto >0:sc68 <0:sndh
 * @param  gzip     sc68: gzip level 0..9 (-1:auto), sndh: 0=raw else ice!
 */
int dsk_save(const char * uri, int version, int gzip)
{
  int err;
  const char * fmt;

  if (!uri)
    uri = dsk.filename;
  if (!uri || !*uri) {
    msgerr("missing filename\n");
    return -1;
  }

  if (!version) {
    const char * ext = str_fileext(uri);
    ext += (ext[0]=='.');
    while (!version) {
      if (!strcmp68(ext,"snd") || !strcmp68(ext,"sndh"))
        version = -1;
      else if (!strcmp68(ext,"sc68"))
        version = 1;
      else if (ext = dsk_tag_get(0, TAG68_FORMAT), !ext) {
        msgerr("unable to determine output file format. try -F\n");
        return -1;
      }
    }
  }

  msgdbg("saving disk as '%s' gzip:%d format:%s version:%d\n",
         uri, gzip, version<0?"sndh":"sc68",version^-(version<0));

  if (!is_valid_disk())
    return -1;

  if (dsk_validate())
    return -1;

  if (version > 0) {
    fmt = "sc68";
    if (gzip < 0) gzip = 0;       /* sc68 default is not compressed */
    err = file68_save_uri(uri, dsk_get_disk(), version-1, gzip);
  } else {
    fmt = "sndh";
    if (gzip < 0) gzip = 1;          /* sndh default is ICE! packed */
    err = sndh_save(uri, dsk_get_disk(), -version-1, gzip);
  }

  if (!err) {
    dsk.modified = 0;
    msginf("disk saved as %s -- \"%s\"\n", fmt, uri);
  } else
    msgerr("failed to save %s -- \"%s\"\n", fmt, uri);

  return err;
}

static int dsk_alloc(int extra)
{
  msgdbg("disk allocation (extra data %d)\n", extra);
  if (dsk.disk) {
    msgerr("disk already allocated\n");
    return -1;
  }
  if (dsk.disk = file68_new(extra), !dsk.disk) {
    msgerr("disk allocation failed\n");
    return -1;
  }
  msgdbg("disk allocated with %d extra bytes\n", dsk.disk->datasz);
  return 0;
}

int dsk_new(const char * name, int force)
{
  static int cnt = 0;
  char tmp[16];
  const char * n = name;

  if (!name || !name[0]) {
    cnt = (cnt+1) % 100u;
    sprintf(tmp,"unnamed-%02d",cnt);
    n = tmp;
  }

  msgdbg("creating new disk: %s\n", n);
  if (dsk.modified && !force) {
    msgerr("modified data (save or try --force)\n");
    return -1;
  }
  dsk_kill();
  if (dsk_alloc(0) < 0)
    return -1;

  dsk.filename = malloc(strlen(n)+6);
  if (dsk.filename)
    sprintf(dsk.filename,"%s.sc68", n);
  if (name && name[0])
    dsk_tag_set(0,"title",name);

  return 0;
}

int dsk_kill(void)
{
  /* int i; */

  if (dsk.modified) {
    msgwrn("destroying a modified disk\n");
  }
  dsk.modified = 0;

  /* for (i=0; i<max_tags; ++i) */
  /*   tag_clr(dsk.tags); */

  file68_free(dsk.disk);
  dsk.disk = 0;

  free(dsk.filename);
  dsk.filename = 0;

  return 0;
}

extern unsigned int fr2ms(unsigned int, unsigned int, unsigned int);

static unsigned int fr_to_ms(unsigned int fr, unsigned int hz)
{
  return fr2ms(fr,hz,0);
}

int dsk_validate(void)
{
  int t, has_inf = 0, has_hw = SC68_XTD;

  if (!is_valid_disk())
    return -1;

  /* Validate time and loop */
  dsk.disk->time_ms = 0;
  dsk.disk->hwflags = 0;            /* Clear all disk flags ... */

  for (t = 0; t < dsk.disk->nb_mus; ++t) {
    music68_t * m = dsk.disk->mus + t;

    /** $$$ XXX FIXME loops is not coherent */
    m->loops    = ( m->loops > 0 ) ? m->loops : 1;
    m->first_ms = fr_to_ms(m->first_fr, m->frq);
    m->loops_ms = fr_to_ms(m->loops_fr, m->frq);
    if (m->loops > 0)
      dsk.disk->time_ms += fr_to_ms(m->first_fr+m->loops_fr*(m->loops-1), m->frq);
    else
      has_inf = 1;

    has_hw &= m->hwflags & SC68_XTD;
    dsk.disk->hwflags |= m->hwflags;
  }
  if (has_inf)
    dsk.disk->time_ms = 0;
  if (!has_hw)
    dsk.disk->hwflags &= SC68_XTD-1;

  return 0;
}

const char * dsk_tag_get(int trk, const char * var)
{
  const char * val = 0;
  assert(trk >= 0);
  assert(var);

  if (!is_valid_disk())
    return 0;
  if (trk && !is_valid_track(trk))
    return 0;
  val = tag_get(trk, var);
  /* Some internal tags */
  if (!val) {
    static char str[64];
    const int max = sizeof(str)-1;
    music68_t * m = dsk.disk->mus + trk - 1;

    if ( ! strcmp (var, TAG68_REPLAY) && trk) {
      val = m->replay;
    }
    else if ( ! strcmp (var, TAG68_HASH) && !trk) {
      snprintf(str, max, "%08x", (uint32_t) dsk.disk->hash);
      val = str;
    }
    else if ( ! strcmp (var, TAG68_FRAMES) && trk) {
      snprintf(str, max, "%u", m->first_fr);
      val = str;
    }
/* $$$ TODO: add loop-length and loop count */

    else if ( ! strcmp (var, TAG68_LENGTH)) {
      unsigned int ms = !trk
        ? dsk.disk->time_ms
        : m->loops > 0
        ? fr_to_ms(m->first_fr+m->loops_fr*(m->loops-1),m->frq)
        : 0;
        ;
      /* unsigned int h, m , s; */
      /* h = ms / 3600000u; */
      /* m = ms % 3600000u / 60000u; */
      /* s = ms % 60000u / 1000u; */
      /* ms %= 1000u; */
      snprintf(str, max, "%u", ms);
      val = str;
    }
    else if ( ! strcmp (var, TAG68_RATE) && trk) {
      snprintf(str, max, "%u", m->frq);
      val = str;
    } else if ( ! strcmp (var, TAG68_URI) && !trk) {
      val = dsk.filename;
    }
    else if ( ! strcmp (var, TAG68_HARDWARE)) {
      val = str_hardware(str, max,
                         !trk ? dsk.disk->hwflags : m->hwflags);
    }
  }
  return val;
}

static const char * tostr(int v)
{
  static char tmp[64];
  snprintf(tmp,63,"%d",v);
  tmp[63] = 0;
  return tmp;
}


/* Does this memory belongs to the disk data ? */
static int is_disk_data(const disk68_t * const mb, const void * const _s)
{
  const char * const s = (const char *) _s;
  return (mb && s >= mb->data && s < mb->data+mb->datasz);
}

static void free_replay(disk68_t * const mb, char ** _s)
{
  if (!is_disk_data(mb, *_s))
    free(*_s);
  *_s = 0;
}

static const char * set_trk_tag(int trk, const char * var, const char * val)
{
  if (trk > 0 && !strcmp(TAG68_REPLAY,var)) {
    if (!val || !val[0]) {
      msginf("track #%02d replay has been deleted\n", trk);
      free_replay(dsk.disk, &dsk.disk->mus[trk-1].replay);
    } else if (strcmp(val, dsk.disk->mus[trk-1].replay)) {
      free_replay(dsk.disk, &dsk.disk->mus[trk-1].replay);
      dsk.disk->mus[trk-1].replay = strdup68(val);
      msginf("track #%02d replay has changed to -- '%s'\n",
             trk, dsk.disk->mus[trk-1].replay);
    }
    return dsk.disk->mus[trk-1].replay;
  }
  return tag_set(trk, var, val);
}

const char * dsk_tag_set(int trk, const char * var, const char * val)
{
  assert(var);
  return (trk == 0 || is_valid_track(trk))
    ? set_trk_tag(trk, var, val)
    : 0
    ;
}

int dsk_tag_seti(int trk, const char * var, int ival)
{
  const char * val = tostr(ival);
  return
    (trk == 0 || is_valid_track(trk))
    ? set_trk_tag(trk, var, val), 0
    : -1
    ;
}

void dsk_tag_del(int trk, const char * var)
{
  tag_del(trk, var);
}

void dsk_tag_clr(int trk)
{
  tag_del_trk(trk);
}

int dsk_trk_get_default(void)
{
  return dsk.disk ? dsk.disk->def_mus+1 : -1;
}

int dsk_trk_get_current(void)
{
  return dsk.disk ? dsk.cur_trk+1 : 0;
}
