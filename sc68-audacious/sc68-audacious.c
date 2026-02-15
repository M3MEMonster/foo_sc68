/*
 * @file    sc68-audacious.c
 * @brief   sc68 plugin for audacious (version 2.4 to 3.4)
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

/* audacious headers */
#include <audacious/plugin.h>
#include <libaudcore/audstrings.h>

/* Select to deal with mutex (3 possibilities) :
 * -1- glib with new/free interface
 * -2- glib with init/clear interface
 * -3- pthreads
 */

#if AUDACIOUS_VERNUM < 30200 /* audacious>=3.2 does not depend on glib */

# include <glib.h>
# ifdef USE_NEW_GLIB_MUTEX
/* new mutex API for GLIB >= 2.32 */
static GMutex ctrl_mutex;
static GCond  ctrl_cond;
static inline gboolean my_mutex_new() {
  g_mutex_init(&ctrl_mutex); g_cond_init(&ctrl_cond);
  return TRUE;
}
static inline void my_mutex_del() {
  g_mutex_clear(&ctrl_mutex); g_cond_clear(&ctrl_cond);
}
static inline void my_mutex_lock()   { g_mutex_lock(&ctrl_mutex); }
static inline void my_mutex_unlock() { g_mutex_unlock(&ctrl_mutex); }
static inline void my_cond_wait()    { g_cond_wait(&ctrl_cond, &ctrl_mutex); }
static inline void my_cond_signal()  { g_cond_signal(&ctrl_cond); }
# else
static GMutex * ctrl_mutex = NULL;
static GCond  * ctrl_cond  = NULL;
static inline gboolean my_mutex_new() {
  ctrl_mutex = g_mutex_new();
  ctrl_cond  = g_cond_new();
  return ctrl_mutex && ctrl_cond;
}
static inline void my_mutex_del() {
  if (ctrl_mutex) { g_mutex_free(ctrl_mutex); ctrl_mutex = 0; }
  if (ctrl_cond) { g_cond_free(ctrl_cond); ctrl_cond = 0; }
}
static inline void my_mutex_lock()   { g_mutex_lock(ctrl_mutex); }
static inline void my_mutex_unlock() { g_mutex_unlock(ctrl_mutex); }
static inline void my_cond_wait()    { g_cond_wait(ctrl_cond, ctrl_mutex); }
static inline void my_cond_signal()  { g_cond_signal(ctrl_cond); }
# endif
#else

/* Since audacious>=3.2 plugins do not depend on glib headers anymore,
 * there is no more reason to use them. Instead we are using pthread
 * and we defines a few glib-like types.
 */

#include <pthread.h>

typedef bool_t  gboolean;
typedef int64_t gint64;
typedef char    gchar;
typedef int     gint;
typedef short   gshort;

static pthread_mutex_t ctrl_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  ctrl_cond  = PTHREAD_COND_INITIALIZER;
static inline gboolean my_mutex_new() {
  return
    (!pthread_mutex_init(&ctrl_mutex, NULL)) &
    (!pthread_cond_init(&ctrl_cond, NULL));
}
static inline void my_mutex_del() {
  pthread_mutex_destroy(&ctrl_mutex);
  pthread_cond_destroy(&ctrl_cond);
}
static inline void my_mutex_lock()   { pthread_mutex_lock(&ctrl_mutex); }
static inline void my_mutex_unlock() { pthread_mutex_unlock(&ctrl_mutex); }
static inline void my_cond_wait()    { pthread_cond_wait(&ctrl_cond, &ctrl_mutex); }
static inline void my_cond_signal()  { pthread_cond_signal(&ctrl_cond); }

#endif

/* standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* sc68 headers */
#include <sc68/sc68.h>
#include <sc68/file68_tag.h>
#include <sc68/file68_msg.h>
#include <sc68/file68_str.h>


static gboolean pause_flag;
static gint64   seek_value = -1;
static gboolean sc68a_error;
static gboolean sc68a_playing;
static gboolean sc68a_eof;

#ifdef DEBUG
static void dmsg(const int bit, sc68_t * sc68, const char *fmt, va_list list)
{
  fprintf(stderr, "sc68-audacious: ");
  vfprintf(stderr, fmt, list);
  if (fmt[strlen(fmt)-1] != '\n')
    fprintf(stderr, "\n");
  fflush(stderr);
}
#endif

/* Audacious tuple functions changed a couple of time ... */
#if AUDACIOUS_VERNUM >= 30500
# define TUPLE_STR(A,B,C,D) tuple_set_str(A,!C?B:tuple_field_by_name(C),D)
# define TUPLE_INT(A,B,C,D) tuple_set_int(A,!C?B:tuple_field_by_name(C),D)
# define TUPLE_SUBTUNES(T,N,A) tuple_set_subtunes(T,N,A)
#elif AUDACIOUS_VERNUM < 30200
# define TUPLE_STR(A,B,C,D) tuple_associate_string(A,B,C,D)
# define TUPLE_INT(A,B,C,D) tuple_associate_int(A,B,C,D)
# define TUPLE_SUBTUNES(T,N,A) (T)->nsubtunes = N
#else
# define TUPLE_STR(A,B,C,D) tuple_set_str(A,B,C,D)
# define TUPLE_INT(A,B,C,D) tuple_set_int(A,B,C,D)
# define TUPLE_SUBTUNES(T,N,A) tuple_set_subtunes(T,N,A)
#endif

static gboolean plg_init(void)
{
  static char prog[] = "audacious";
  static char * argv[] = { prog };
  sc68_init_t init68;
  memset(&init68, 0, sizeof(init68));
  init68.argc = sizeof(argv)/sizeof(*argv);
  init68.argv = argv;
  init68.debug_clr_mask = -1;
#ifdef DEBUG
  init68.debug_set_mask = ((1<<(msg68_DEBUG+1))-1);
  init68.msg_handler = dmsg;
#endif
  if (sc68_init(&init68) < 0)
    return FALSE;
  if (!my_mutex_new())
    return FALSE;
  return TRUE;
}

static void plg_cleanup(void)
{
  my_mutex_del();
  sc68_shutdown();
}

static void * read_file(VFSFile * file, int * psize)
{
  void * buf = 0;
  gint64 fsize;
  int isize;

  fsize = vfs_fsize (file);
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

static Tuple * tuple_from_track(const gchar * filename,
                                sc68_t * sc68, sc68_disk_t disk, int track)
{
  Tuple * tu = filename
    ? tuple_new_from_filename(filename)
    : tuple_new();

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
          /* Seems uncessary, audacious ignores tag it does not know ? */
          /* else */
          /*   TUPLE_STR(tu, -1, tag->key, val); */
        }
      }
    } else {
      msg68_error("sc68-audacious: "
                  "could not retrieve sc68 music info for track #%d\n", track);
    }
  }

  return tu;
}

#if AUDACIOUS_VERNUM < 30200
/* Quick dirty replacement for this missing function just used to
 * retrieve sub-song number in the uri.
 */
static
void uri_parse(const gchar * fn, void *x, void *y, void *z, gint * ptrk)
{
  int trk = -1;
  int i = strlen(fn) - 1;
  if (i >= 3 && isdigit((int)fn[i])) {
    int t, r;
    for (t=0, r=1; i > 0 && isdigit((int)fn[i]); --i, r*=10)
      t = t + r * (fn[i]-'0');
    if (i > 0 && fn[i] == '?')
      trk = t;
  }
  *ptrk = trk;
}
#endif

/* Must try to play this file.  "playback" is a structure containing
 * output- related functions which the plugin may make use of.  It
 * also contains a "data" pointer which the plugin may use to refer
 * private data associated with the playback state.  This pointer can
 * then be used from pause, mseek, and stop. If the file could not be
 * opened, "file" will be NULL.  "start_time" is the position in
 * milliseconds at which to start from, or -1 to start from the
 * beginning of the file.  "stop_time" is the position in milliseconds
 * at which to end playback, or -1 to play to the end of the file.
 * "paused" specifies whether playback should immediately be paused.
 * Must return nonzero if some of the file was successfully played or
 * zero on failure. */
static
gboolean plg_play(InputPlayback * playback,
                  const gchar * filename, VFSFile * file,
                  gint start_time, gint stop_time, gboolean pause)
{
  char buffer[512 * 4];
  int code = SC68_ERROR;
  sc68_create_t create68;
  sc68_t * sc68 = 0;
  void   * mem = 0;
  int len, rate;
  int track;
  int myfile = 0;
  Tuple *tu = 0;
  gshort paused = 0;

  /* Error on exit. */
  my_mutex_lock(); {
    playback->set_data(playback, NULL);
    sc68a_error = TRUE;
    sc68a_playing = FALSE;
    sc68a_eof = FALSE;
  } my_mutex_unlock();

  uri_parse (filename, NULL, NULL, NULL, &track);
  /* fprintf(stderr,"filename: <%s> track:%d\n",filename, track); */

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
  sc68_play(sc68, track > 0 ? track : 1, SC68_DEF_LOOP);
  rate = sc68_cntl(sc68, SC68_GET_SPR);
  /* Safety net */
  if (rate <= 8000)
    rate = sc68_cntl(sc68, SC68_SET_SPR, 8000);
  else if (rate > 48000)
    rate = sc68_cntl(sc68, SC68_SET_SPR, 48000);

  /* Open and setup audio playback. */
  if (! playback->output->open_audio(FMT_S16_NE, rate, 2))
    goto out;

  my_mutex_lock(); {
    /* Set rate and channels (other info are deprecated */
    playback->set_params(playback, -1, rate, 2);

    playback->set_data(playback, sc68);
    code = sc68_process(sc68, 0, 0);
    tu = tuple_from_track(filename, sc68, 0, track);
    if (tu) {
      playback->set_tuple(playback, tu);
      /* playback->set_gain_from_playlist(playback); */
      tu = NULL;
    }

    pause_flag = pause;
    seek_value = (start_time > 0) ? start_time : -1;

    sc68a_playing = TRUE;
    sc68a_eof = FALSE;
    sc68a_error = FALSE;
    playback->set_pb_ready(playback);
  } my_mutex_unlock();

  /* Update return code (load the track) */
  while (!sc68a_error && sc68a_playing && !sc68a_eof) {
    int n;

    if (stop_time >= 0 && playback->output->written_time() >= stop_time) {
      sc68a_eof = TRUE;
      continue;
    }

    my_mutex_lock(); {
      /* Handle seek request */
      /* not supported
         if (seek_value >= 0) {
         sc68_seek(sc68, seek_value, 0);
         seek_value = -1;
         my_cond_signal();
         }
      */

      /* Handle pause/unpause request */
      if (pause_flag != paused) {
        playback->output->pause(pause_flag);
        paused = pause_flag;
        my_cond_signal();
      }

      if (paused) {
        my_cond_wait();
        my_mutex_unlock();
        continue;
      }
    } my_mutex_unlock();

    /* Run sc68 engine. */

    n = sizeof(buffer) >> 2;
    code = sc68_process(sc68, buffer, &n);
    if (code == SC68_ERROR) {
      sc68a_error = TRUE;
      continue;
    }
    if (code & SC68_END) {
      sc68a_eof = TRUE;
      continue;
    }
    if (code & SC68_CHANGE) {
      sc68a_eof = TRUE;
      continue;
    }

    /* Is sc68 currently seeking ? */
    /*
      pos = sc68_seek(sc68, -1, &seeking);
      if (pos >= 0 && seeking) {
      while (playback->output->buffer_playing())
      g_usleep(5000);
      playback->output->flush(pos);
      }
    */
    playback->output->write_audio(buffer, n << 2);
  }
  sc68_stop(sc68);

  /* Flush buffer */
  my_mutex_lock(); {
    playback->set_data(playback, NULL);
    sc68a_playing = FALSE;
    my_cond_signal();
  } my_mutex_unlock();

/* close: */

#if AUDACIOUS_VERNUM < 30300
  playback->output->close_audio();
#else
  playback->output->abort_write();
#endif

out:
  free(mem);
  sc68_destroy(sc68);
  return ! sc68a_error;
}

static
/* Must signal a currently playing song to stop and cause play to
 * return.  This function will be called from a different thread than
 * play.  It will only be called once. It should not join the thread
 * from which play is called.
 */
void plg_stop(InputPlayback * playback)
{
  my_mutex_lock(); {
    sc68a_playing = FALSE;
  } my_mutex_unlock();
}

static
/* Must pause or unpause a file currently being played.  This function
 * will be called from a different thread than play, but it will not
 * be called before the plugin calls set_pb_ready or after stop is
 * called. */
/* Bug: paused should be a gboolean, not a gshort. */
/* Bug: There is no way to indicate success or failure. */
void plg_pause(InputPlayback * playback, gboolean paused)
{
  my_mutex_lock(); {
    if (sc68a_playing) {
      pause_flag = paused;
      my_cond_signal();
      my_cond_wait();
    }
  } my_mutex_unlock();
}

/* Optional.  Must seek to the given position in milliseconds within a
 * file currently being played.  This function will be called from a
 * different thread than play, but it will not be called before the
 * plugin calls set_pb_ready or after stop is called. */
/* Bug: time should be a gint, not a gulong. */
/* Bug: There is no way to indicate success or failure. */
void plg_seek(InputPlayback * playback, gint time)
{
  my_mutex_lock(); {
    if (sc68a_playing) {
      seek_value = time;
      my_cond_signal();
      my_cond_wait();
    }
  } my_mutex_unlock();
}

static void fill_subtunes(Tuple *tuple)
{
  int tunes = tuple_get_int (tuple, FIELD_SUBSONG_NUM, NULL)/* , i */;
  /* int subtunes[tunes]; */
  /* for (i = 0; i < tunes; ++i) */
  /*   subtunes[i] = i + 1; */
  TUPLE_SUBTUNES(tuple, tunes, NULL/* subtunes */);
}

/* Must return a tuple containing metadata for this file, or NULL if
 * no metadata could be read.  If the file could not be opened, "file"
 * will be NULL.  Audacious takes over one reference to the tuple
 * returned. */
Tuple * plg_probe(const gchar * filename, VFSFile * file)
{
  Tuple *tu = NULL;
  sc68_disk_t disk = 0;
  int size/* , track = 0 */;
  void * buf = 0;
  int track;

  if (!file)
    goto done;
  if (buf = read_file(file, &size), !buf)
    goto done;
  if(disk = sc68_disk_load_mem(buf, size), !disk)
    goto done;

  uri_parse (filename, NULL, NULL, NULL, & track);
  tu = tuple_from_track(filename, 0, disk, track);
  fill_subtunes(tu);


done:
  sc68_disk_free(disk);
  free(buf);
  return tu;
}

static
/* Must return nonzero if the plugin can handle this file.  If the
 * file could not be opened, "file" will be NULL.  (This is normal in
 * the case of special URI schemes like cdda:// that do not represent
 * actual files.) */
/* Bug: The return value should be a gboolean, not a gint. */
gint plg_is_our(const gchar * filename, VFSFile * file)
{
  sc68_disk_t disk = 0;
  char * buf = 0;
  int size;
  if (!file)
    goto done;
  if (buf = read_file(file, &size), !buf)
    goto done;
  disk = sc68_disk_load_mem(buf, size);
done:
  free(buf);
  sc68_disk_free(disk);
  return disk != 0;
}


#define PRIORITY 5 /* medium */

static const gchar * exts[] = { "sc68", "sndh", "snd", NULL };

#if AUDACIOUS_VERNUM < 30000 || AUDACIOUS_VERNUM >= 30300
static gchar desc[] = "Atari and Amiga music player";
#endif

#if AUDACIOUS_VERNUM < 30300
# define D_ABOUT_TEXT
# define D_DOMAIN
#else
# define D_ABOUT_TEXT .about_text = desc,
# define D_DOMAIN .domain = PACKAGE,
#endif

#if AUDACIOUS_VERNUM < 30000

/* Audacious 2 */

static InputPlugin sc68_plugin = {
  .init = plg_init,
  .description = desc,
  .cleanup = plg_cleanup,
  .play = plg_play,
  .stop = plg_stop,
  .pause = plg_pause,
  .probe_for_tuple = plg_probe,
  .is_our_file_from_vfs = plg_is_our,
  .vfs_extensions = exts,
  .have_subtune = 1,
  .priority = PRIORITY,
};
InputPlugin * ip_list [] = { &sc68_plugin, NULL };
SIMPLE_INPUT_PLUGIN("sc68", ip_list)

#else

static gchar name[] = "sc68";
/* Audacious 3 */

AUD_INPUT_PLUGIN (
  .name = name,
  D_ABOUT_TEXT
  D_DOMAIN
  .init = plg_init,
  .cleanup = plg_cleanup,
  .play = plg_play,
  .stop = plg_stop,
  .pause = plg_pause,
  .probe_for_tuple = plg_probe,
  .is_our_file_from_vfs = plg_is_our,
  .extensions = exts,
  .have_subtune = 1,
  .priority = PRIORITY,
)

#endif
