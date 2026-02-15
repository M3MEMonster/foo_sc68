/*
 * @file    sc68-aud35.c
 * @brief   sc68 audacious (3.5 and later) plugin
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
#include "config.h"
#endif

/* standard headers */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* audacious headers */
#include <libaudcore/audstrings.h>
#include <audacious/input.h>
#include <audacious/plugin.h>

/* sc68 headers */
#include <sc68/sc68.h>
#include <sc68/file68.h>
#include <sc68/file68_tag.h>
#include <sc68/file68_msg.h>
#include <sc68/file68_str.h>

/* Global variables */
/* pthread_mutex_t plg_mutex = PTHREAD_MUTEX_INITIALIZER; */

#define TUPLE_STR(A,B,C,D) tuple_set_str(A,!C?B:tuple_field_by_name(C),D)
#define TUPLE_INT(A,B,C,D) tuple_set_int(A,!C?B:tuple_field_by_name(C),D)
#define TUPLE_SUBTUNES(T,N,A) tuple_set_subtunes(T,N,A)
#define NOP if (0) {} else
#define LOCK() NOP /* pthread_mutex_lock(&plg_mutex) */
#define UNLOCK() NOP /* pthread_mutex_unlock(&plg_mutex) */


#ifdef DEBUG
static void dmsg(const int bit, sc68_t * sc68, const char *fmt, va_list list)
{
  fprintf(stderr, "sc68-audacious: ");
  vfprintf(stderr, fmt, list);
  if (fmt[strlen(fmt)-1] != '\n')
    fprintf(stderr, "\n");
  fflush(stderr);
}
# define DBG(fmt,...) msg68_debug(fmt, ## __VA_ARGS__)
#else
# define DBG(fmt,...) while (0)
#endif

static
bool_t plg_init(void)
{
  bool_t success = TRUE;

  static char prog[] = "audacious";
  static char * argv[] = { prog };
  sc68_init_t init68;

  DBG("%s()\n", __FUNCTION__);

  memset(&init68, 0, sizeof(init68));
  init68.argc = sizeof(argv)/sizeof(*argv);
  init68.argv = argv;
  init68.debug_clr_mask = -1;
#ifdef DEBUG
  init68.debug_set_mask = ((1<<(msg68_DEBUG+1))-1);
  init68.msg_handler = dmsg;
#endif

  LOCK();
  if (sc68_init(&init68) < 0)
    success = FALSE;
  UNLOCK();

  DBG("%s() => %s\n", __FUNCTION__, success?"OK":"FAIL");
  return success;
}

static
void plg_cleanup(void)
{
  DBG("%s()\n", __FUNCTION__);
  LOCK();
  sc68_shutdown();
  UNLOCK();
  /* pthread_mutex_destroy(&plg_mutex); */
  DBG("%s() => OK\n", __FUNCTION__);
}

static void * read_file(VFSFile * file, int * psize)
{
  void * buf = 0;
  int64_t fsize;
  int isize;

  fsize = vfs_fsize(file);
  if (fsize <= 0 || fsize > (1<<20))
    goto done;
  isize  = (int) fsize;
  *psize = isize;

  buf = malloc(isize);
  if (!buf)
    goto done;
  fsize = vfs_fread(buf, 1, isize, file);
  if (isize != (int)fsize) {
    free(buf);
    buf = 0;
  }

done:
  DBG("%s() => %s %d\n", __FUNCTION__, buf?"OK":"FAIL", (int)fsize);
  return buf;
}

/* Not quiet exact but shoud be ok for all sc68 files :) */
static int year_of_str(const char * val)
{
  int c, y = 0;
  while (c = *val++, (c >= '0' && c <= '9')) {
    y = y * 10 + c - '0';
  }
  if (y < 100)
    y += (y >= 70) ? 1900 : 2000;
  else if (y < 1970)
    y = 0;
  return y;
}

static
const char * get_tag(const sc68_cinfo_t * const cinfo, const char * const key)
{
  int i;
  for (i=0; i<cinfo->tags; ++i)
    if (cinfo->tag[i].key && !strcmp68(cinfo->tag[i].key, key))
      return cinfo->tag[i].val;
  return 0;
}

static
Tuple * tuple_from_track(const char * uri, sc68_t * sc68,
                         sc68_disk_t disk, int track)
{
  Tuple * tu = uri
    ? tuple_new_from_filename(uri)
    : tuple_new();

  /* static int tunes[SC68_MAX_TRACK]; */
  /* if (!tunes[0]) { */
  /*   int i; */
  /*   for (i=0; i<SC68_MAX_TRACK; ++i) */
  /*     tunes[i]=i+1; */
  /* } */

  if (tu) {
    sc68_music_info_t  info;
    sc68_cinfo_t     * cinfo;

    TUPLE_STR(tu, FIELD_CODEC,    NULL, "sc68");
    TUPLE_STR(tu, FIELD_QUALITY,  NULL, "sequenced");
    TUPLE_STR(tu, FIELD_MIMETYPE, NULL, sc68_mimetype());

    if (track <= 0)
      track = 1;

    if ( !sc68_music_info(sc68, &info, track, disk)) {
      const char * artist;

      if (strcmp68(info.album,info.title)) {
        TUPLE_STR(tu, FIELD_ALBUM, NULL, info.album);
        TUPLE_STR(tu, FIELD_TITLE, NULL, info.title);
      } else {
        TUPLE_STR(tu, FIELD_TITLE, NULL, info.title);
      }

      artist = get_tag(&info.dsk,"aka");
      if (!artist)
        artist = info.dsk.tag[TAG68_ID_ARTIST].val;
      TUPLE_STR(tu, FIELD_ARTIST, NULL, artist);

      artist = get_tag(&info.trk,"aka");
      if (!artist)
        artist = info.trk.tag[TAG68_ID_ARTIST].val;
      TUPLE_STR(tu, FIELD_SONG_ARTIST, NULL, artist);

      TUPLE_STR(tu, FIELD_GENRE,  NULL, info.trk.tag[TAG68_ID_GENRE].val);
      TUPLE_INT(tu, FIELD_LENGTH, NULL,  info.trk.time_ms);
      TUPLE_INT(tu, FIELD_SUBSONG_NUM,  NULL, info.tracks);
      TUPLE_INT(tu, FIELD_SUBSONG_ID,   NULL, track);
      TUPLE_INT(tu, FIELD_TRACK_NUMBER, NULL, track);
      TUPLE_SUBTUNES(tu, info.tracks, 0/* tunes */);

      for (cinfo = &info.dsk;
           cinfo;
           cinfo = (cinfo == &info.dsk) ? &info.trk : 0) {
        int i;
        for (i=TAG68_ID_CUSTOM; i<cinfo->tags; ++i) {
          sc68_tag_t * tag = cinfo->tag+i;
          const char * val = tag->val;
          int           id = -1;
          if (!strcmp68(tag->key, TAG68_COMMENT))
            id = FIELD_COMMENT;
          else if (!strcmp68(tag->key, TAG68_COPYRIGHT))
            id = FIELD_COPYRIGHT;
          else if (!strcmp68(tag->key, TAG68_COMPOSER))
            id = FIELD_COMPOSER;
          else if (!strcmp68(tag->key, TAG68_FORMAT))
            id = FIELD_CODEC;
          else if (!strcmp68(tag->key, TAG68_YEAR)) {
            int year = year_of_str(val);
            if (year)
              TUPLE_INT(tu, FIELD_YEAR, NULL, year);
          } else if (!strcmp68(tag->key, TAG68_AKA)) {
            continue;
          }
          if (id != -1)
            TUPLE_STR(tu, id, NULL, val);
        }
      }
    }
  }

  DBG("%s() track=%d => %s\n", __FUNCTION__, track, tu ? "OK":"FAIL");

#ifdef DEBUG
  if (tu) {
    int i, v; const char * s;
    for (i=0;i<TUPLE_FIELDS;++i) {
      const char * name = tuple_field_get_name (i);
      if (!name) continue;
      switch (tuple_get_value_type (tu,i)) {
      case TUPLE_STRING:
        s = tuple_get_str (tu, i);
        DBG("tag #%02d \"%s\" = \"%s\"\n", i, name, s ? s : "(nil)");
        break;
      case TUPLE_INT:
        v =  tuple_get_int (tu, i);
        DBG("tag #%02d \"%s\" = %d\n", i, name, v);
        break;
      case TUPLE_UNKNOWN: break;
      }
    }
  }
#endif

  return tu;
}

static
bool_t plg_play(const char * uri, VFSFile *file)
{
  bool_t success = FALSE;
  char buffer[512 * 4];
  int code = SC68_ERROR;
  sc68_create_t create68;
  sc68_t * sc68 = 0;
  void   * mem = 0;
  int len, rate;
  int track;
  int myfile = 0;
  Tuple *tu = 0;

  DBG("%s() uri=\"%s\"\n", __FUNCTION__, uri);
  uri_parse (uri, NULL, NULL, NULL, &track);
  DBG("%s() track=%d", __FUNCTION__, track);

  /* Currently load the file in memory. Would be better to support
   * audacious VFS with stream68.
   */
  len = 0;                              /* GCC warning ... */
  mem = read_file(file, &len);
  if (myfile) {
    vfs_fclose(file);
    file = NULL;
  }
  if (!mem)
    goto out;

  /* Create emulator instance. */
  memset(&create68,0,sizeof(create68));
  sc68 = sc68_create(&create68);
  if (!sc68)
    goto out;

  /* Load sc68 file. */
  if (sc68_load_mem(sc68, mem, len))
    goto out;

  /* Set first track unless specified otherwise */
  track = track <= 0 ? 1 : track;
  if (sc68_play(sc68, track, SC68_DEF_LOOP) < 0) {
    goto out;
  }
  rate = sc68_cntl(sc68, SC68_GET_SPR);
  /* Safety net */
  if (rate <= 8000)
    rate = sc68_cntl(sc68, SC68_SET_SPR, 8000);
  else if (rate > 48000)
    rate = sc68_cntl(sc68, SC68_SET_SPR, 48000);

  /* Force track init. */
  code = sc68_process(sc68,0,0);
  if (code == SC68_ERROR)
    goto out;

  /* Open the audio output */
  if (!aud_input_open_audio(FMT_S16_NE, rate, 2))
    goto out;

  tu = tuple_from_track(uri, sc68, 0, track);
  if (tu)
    aud_input_set_tuple(tu);     /* Give ownership: no need to free */

  while (! aud_input_check_stop ()) {
    int n = sizeof(buffer) >> 2;

    /* Run sc68 engine. */
    code = sc68_process(sc68, buffer, &n);
    if (code == SC68_ERROR)
      goto out;
    aud_input_write_audio(buffer, n<<2);
    if (code & (SC68_END|SC68_CHANGE)) {
      break;
    }
  }
  sc68_stop(sc68);
  success = TRUE;
out:
  free(mem);
  sc68_destroy(sc68);

  DBG("%s(\"%s\") => %s\n", __FUNCTION__, uri, success?"OK":"FAIL");
  return success;
}


static
Tuple * plg_probe(const char * uri, VFSFile * file)
{
  Tuple * tu = NULL;
  sc68_disk_t disk = 0;
  int size;
  void * buf = 0;
  int track;

  if (!file)
    goto done;
  if (buf = read_file(file, &size), !buf)
    goto done;
  if(disk = sc68_disk_load_mem(buf, size), !disk)
    goto done;
  uri_parse (uri, NULL, NULL, NULL, &track);
  tu = tuple_from_track(uri, 0, disk, track);

done:
  sc68_disk_free(disk);
  free(buf);
  return tu;
}

static const char *plg_sc68_fmts[] = { "sc68", "sndh", "snd", NULL };

AUD_INPUT_PLUGIN
(
  .name = "SC68 Music Player",          /* Plugin name */
  .about_text = PACKAGE_DESC,           /* Short description  */
  .domain = PACKAGE,                    /* For gettext */
  .init = plg_init,                     /* Initialization */
  .cleanup = plg_cleanup,               /* Cleanup */
  .play = plg_play,                     /* Play given file */
  .probe_for_tuple = plg_probe,         /* probe metatags */
  .extensions = plg_sc68_fmts,          /* File ext assist */
  .have_subtune = TRUE,                 /* we have sub-tunes */
  .priority = 5,                        /* medium */
  )
