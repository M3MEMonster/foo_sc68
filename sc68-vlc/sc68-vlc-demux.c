/*
 * @file    sc68-vlc-demux.c
 * @brief   sc68 demuxer for vlc.
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

#if ! defined(DEBUG) && defined(DEBUG_SC68_VLC)
#define DEBUG DEBUG_SC68_VLC
#endif

#include <vlc_common.h>
#include <vlc_input.h>
#include <vlc_plugin.h>
#include <vlc_demux.h>
#include <vlc_codec.h>
#include <vlc_meta.h>

#include <assert.h>
#ifdef MSG_TO_STDERR
# include <stdio.h>
#endif

#include <sc68/file68_vfs_def.h>
#include <sc68/file68_vfs.h>
#include <sc68/sc68.h>
#include <sc68/file68.h>
#include <sc68/file68_msg.h>
#include <sc68/file68_tag.h>
#include <sc68/file68_opt.h>

enum { ALLIN1 = 1 };

vfs68_t * vfs68_vlc_create(stream_t * vlc, const char * uri);

#ifndef _
# define _(str)  (str)
#endif

#ifndef N_
# define N_(str) (str)
#endif

#ifndef vlc_meta_TrackTotal
# define vlc_meta_TrackTotal (vlc_meta_TrackID+1)
#endif

/*****************************************************************************
 * Forward declarations
 *****************************************************************************/

static int vlc_init_sc68(void);
static void vlc_shutdown_sc68(void);
static int Open( vlc_object_t * );
static void Close( vlc_object_t * );
static int Demux( demux_t * );
static int Control( demux_t *, int, va_list );
static void msg(const int bit, void *userdata, const char *fmt, va_list list);
static void meta_lut_sort(void);


/*****************************************************************************
 * Debug messages
 *****************************************************************************/

#ifdef DEBUG
#define dbg _dbg
static void _dbg(demux_t * demux, const char * fmt, ...) FMT23;
static void _dbg(demux_t * demux, const char * fmt, ...)
{
  va_list list;
  va_start(list,fmt);
  msg(msg68_DEBUG, demux, fmt, list);
  va_end(list);
}
#else
#define dbg(demux,fmt,...)
#endif

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/

#define CFG_PREFIX "sc68-"

static const int asidmodes[] = {
  SC68_ASID_ON, SC68_ASID_OFF, SC68_ASID_FORCE
};

static const char *const asidmodes_text[] = {
  N_("On"), N_("Off"), N_("Force"),
};

static const char *const ppsz_ymengine[] = {
  "blep", "pulse"
};

static const char *const ppsz_ymengine_text[] = {
  N_("BLEP synthesis"), N_("PULSE (sc68 legacy)")
};

static const char *const ppsz_ymfilter[] = {
  "2-poles", "mixed", "1-pole", "boxcar", "none"
};

static const char *const ppsz_ymfilter_text[] = {
  N_("2 poles Butterworth"), N_("4 tap boxcar + 1p-butterworth"),
  N_("1 pole Butterworth"), N_("4 tap boxcar"), N_("No filter")
};

static const char *const ppsz_ymvol[] = {
  "atari","linear"
};
static const char *const ppsz_ymvol_text[] = {
  N_("Atari-ST"), N_("Linear")
};

static const int blends[] = {
  0x00, 0x2a, 0x54, 0x80, 0xab, 0xd5, 0xff
};
static const char *const blends_text[] = {
  N_("L/R 100%"), N_("L/R 66%"), N_("L/R 33%"),
  N_("Center"),
  N_("R/L 33%"), N_("R/L 66%"), N_("R/L 100%"),
};

static const int sprs[] = {
  22050, 44100, 48000, 96000
};
static const char *const sprs_text[] = {
  N_("Low quality (22 Khz)"), N_("CD quality (44.1 Khz)"),
  N_("DVD quality (48 Khz)"), N_("Studio quality (96 Khz)"),
};

static volatile int mmm_init = 0;

/* Declare VLC module */
vlc_module_begin()
{
#ifdef DEBUG
  fprintf(stderr,"sc68-vlc: ENTER %s() with mmm_init=%d !\n", __FUNCTION__, mmm_init);
  fflush(stderr);
#endif
  assert(mmm_init == 0);
  ++mmm_init;

  /* $$$ TEMP EVIL FIX FOR NOT UNLOADING PROPERLY ATM */
  cannot_unload_broken_library();

  if (vlc_init_sc68()) {
    goto error;
  }
  set_shortname("SC68");
  set_description( N_("sc68 demuxer") );
  set_category( CAT_INPUT );
  set_subcategory( SUBCAT_INPUT_DEMUX );
  set_capability( "demux", 100);
  set_callbacks( Open, Close );
  add_shortcut( "sc68" );

  /* ---------------------------------------------------------------------- */

  set_section( "General", N_("sc68 general options") );

  add_bool(
    CFG_PREFIX "all-in-1",
    true,
    N_("Sub-songs as chapters"),
    N_("Have sub-songs displayed as chapters instead of titles."),
    true);

  add_integer_with_range(
    CFG_PREFIX "samplerate",
    44100, 8000, 96000,
    N_("Sampling rate"),
    N_( "Sampling rate in hertz"),
    true);
  change_integer_list(sprs,sprs_text);

  /* ---------------------------------------------------------------------- */

  set_section( "aSID", N_("Automatic SID voices") );

  add_integer_with_range(
    CFG_PREFIX "asid-mode",
    SC68_ASID_ON, SC68_ASID_OFF, SC68_ASID_FORCE,
    N_("aSIDify songs"),
    N_( "Force might break the music"),
    true);
  change_integer_list(asidmodes,asidmodes_text);

  add_bool(
    CFG_PREFIX "asid-fav",
    false,
    N_( "Favour aSID tracks" ),
    N_( "Default to play aSIDdified tracks over normal Atari tracks." ),
    true);

  /* ---------------------------------------------------------------------- */

  set_section( "YM-2149", N_("Atari-ST soundchip simulation") );

  add_string(
    CFG_PREFIX "ym-engine",
    "blep",
    N_( "YM-2149 simulation model" ),
    N_("Choose BLEP (Band-Limited stEP) synthesis for the best quality. "
       "PULSE is sc68 legacy engine" ),
    true);
  change_string_list(ppsz_ymengine, ppsz_ymengine_text/* , 0 */)

  add_string(
    CFG_PREFIX "ym-filter",
    "2-poles",
    N_("YM-2149 filter (PULSE only)"),
    N_("Signal filter for PULSE engine. "                              \
       "2-poles is the best. "                                         \
       "Avoid none it will discard some SFX."),
    true);
  change_string_list(ppsz_ymfilter, ppsz_ymfilter_text/* , 0 */);

  add_string(
    CFG_PREFIX "ym-volmodel",
    "atari",
    N_("YM-2149 volume model"),
    N_("Atari-ST is a specially cooked YM channels mixer to "
       "sound more like an actual Atari-ST machine."),
    true);
  change_string_list(ppsz_ymvol, ppsz_ymvol_text/* , 0 */);

  /* ---------------------------------------------------------------------- */

  set_section( "Amiga", N_("Amiga soundchip simulation") );

  add_bool(
    CFG_PREFIX "amiga-filter",
    true,
    N_("filter"),
    N_("resampling filter"), true );

  add_integer(
    CFG_PREFIX "amiga-blend",
    blends[2],
    N_("L/R blend"),
    N_("Setup Amiga channel left/right balance"),
    true);
  change_integer_list(blends,blends_text);

#ifdef HAVE_FILEEXT
  fileext_Add( FILEEXT_POOL, "*.sc68;*.sndh;*.snd", FILEEXT_AUDIO );
#endif
}
vlc_module_end()


/*****************************************************************************
 * Private data
 *****************************************************************************/

struct demux_sys_t {
#ifdef DEBUG
  char              name[16];     /* sc68 instance name                      */
#endif
  sc68_t          * sc68;         /* sc68 emulator instance                  */
  date_t            pts;          /* Presentation Time-Stamp                 */
  es_out_id_t     * es;           /* Elementary Stream                       */
  int               pcm_per_loop; /* PCM per sound rendering loop            */
  int               sample_rate;  /* Sample rate in hz                       */
  int               allin1;       /* 0:track by track                        */
  int               asid;         /* aSID status 0:off 1:on                  */
  int               asidify;      /* Add asid songs (0:no)                   */
  int               asid_count;   /* Number of tracks that can asid          */
  int               track;        /* current track (1 based)                 */
  int               tracks;       /* number of tracks                        */
  sc68_minfo_t      infos[SC68_MAX_TRACK];
  vlc_meta_t      * meta;
};

static volatile int vlc_initialized = 0;

/* Init sc68 library (call once on load) */
static int vlc_init_sc68(void)
{
  int res = 0;
  sc68_init_t init68;
  static char appname[] = "vlc";
  /* static char message[] = "--sc68-debug=+loader"; */
  static char * argv[] = { appname/* , message */ };

  if (vlc_initialized) {
#ifdef DEBUG
    fprintf(stderr, "%s() already initialized ? [%d]\n",
            __FUNCTION__, vlc_initialized);
    fflush(stderr);
#endif
    msg_Err((demux_t*)0, "%s() already initialized ? [%d]\n",
            __FUNCTION__, vlc_initialized);
    return -1;
  }
  assert(vlc_initialized == 0);
  vlc_initialized = 2;          /* 2: during init */

  meta_lut_sort();
  memset(&init68,0,sizeof(init68));
  init68.argc = sizeof(argv)/sizeof(*argv);
  init68.argv = argv;
  init68.debug_clr_mask = -1;
#ifdef DEBUG
  init68.debug_set_mask = ((1<<(msg68_DEBUG+1))-1);
  init68.msg_handler    = msg;
#endif
  res = sc68_init(&init68);
  vlc_initialized = !res ? 1 : -1;
  return res;
}

/* $$$ Shutdown sc68 library (call once on unload) */
static void vlc_shutdown_sc68(void)
{
  assert(vlc_initialized == 1);
  if (vlc_initialized == 1) {
    vlc_initialized = 3;
    sc68_shutdown();
  } else {
#ifdef DEBUG
    fprintf(stderr, "%s() not initialized ? [%d]\n",
            __FUNCTION__, vlc_initialized);
    fflush(stderr);
#endif
    msg_Err((demux_t*)0,"%s() not initialized ? [%d]\n",
            __FUNCTION__, vlc_initialized);
  }
  vlc_initialized = 0;
}

/* Shutdown sc68 demux module private */
static void sys_shutdown(demux_t * p_demux)
{
#ifdef DEBUG
  fprintf(stderr,"sc68-vlc: ENTER %s() demux:%p mmm_init=%d !\n",
          __FUNCTION__, (void*)p_demux,mmm_init);
  fflush(stderr);
#endif

  dbg(p_demux, "Enter %s()\n", __FUNCTION__);
  if (p_demux) {
    demux_sys_t * p_sys = p_demux->p_sys;
    p_demux->p_sys = 0;
    if (p_sys) {
      if (p_sys->meta)
        vlc_meta_Delete(p_sys->meta);
      sc68_cntl(p_sys->sc68,SC68_CONFIG_SAVE);
      sc68_destroy(p_sys->sc68);
      free(p_sys);
    }
  }
  dbg(p_demux, "Leave %s()\n", __FUNCTION__);
}

/* Get current track and adjust info pointer */
static int current_track(demux_t * const p_demux)
{
  demux_sys_t * const p_sys = p_demux->p_sys;
  return
    p_sys->track = sc68_cntl(p_sys->sc68, SC68_GET_TRACK);
}

typedef struct meta_lut_s {
  vlc_meta_type_t vlc;
  char * sc68;
} meta_lut_t;

static int cmp_meta_lut(const void * pa, const void * pb)
{
  const meta_lut_t * a = pa;
  const meta_lut_t * b = pb;
  return strcmp(a->sc68, b->sc68);
}

static meta_lut_t meta_lut[] = {
  { vlc_meta_Artist,      TAG68_AKA },
  { vlc_meta_ArtworkURL,  TAG68_IMAGE },
  { vlc_meta_Copyright,   TAG68_COPYRIGHT},
  { vlc_meta_Date,        "date" },
  { vlc_meta_Description, TAG68_COMMENT},
  { vlc_meta_EncodedBy,   TAG68_CONVERTER},
  { vlc_meta_Publisher,   "publisher" },
  { vlc_meta_Rating,      "rating"},
  /* { vlc_meta_Setting,     TAG68_HARDWARE}, */
  { vlc_meta_URL,         TAG68_URI},
};

static void meta_lut_sort(void)
{
  qsort(meta_lut, sizeof(meta_lut)/sizeof(*meta_lut),
        sizeof(*meta_lut), cmp_meta_lut);
}

static
vlc_meta_t * track_meta(demux_t * const p_demux, vlc_meta_t * meta)
{
  demux_sys_t * const p_sys = p_demux->p_sys;
  sc68_minfo_t * info;
  int i;
  sc68_tag_t tag;
  char trackstr[4] = "00";

  if (!meta)
    meta = vlc_meta_New();
  if (unlikely(!meta))
    return 0;

  if (current_track(p_demux) < 0)
    return meta;

  info = p_sys->infos + p_sys->track - 1;
  vlc_meta_Set( meta, vlc_meta_Album, info->album);
  vlc_meta_Set( meta, vlc_meta_Title, info->title);
  vlc_meta_Set( meta, vlc_meta_Artist,info->artist);
  vlc_meta_Set( meta, vlc_meta_Genre, info->trk.tag[TAG68_ID_GENRE].val);

  trackstr[0] = '0' + p_sys->track / 10;
  trackstr[1] = '0' + p_sys->track % 10;
  vlc_meta_Set(meta, vlc_meta_TrackNumber, trackstr);

  /* trackstr[0] = '0' + info->tracks / 10; */
  /* trackstr[1] = '0' + info->tracks % 10; */
  /* vlc_meta_Set( meta, vlc_meta_TrackTotal, trackstr); */
  /* vlc_meta_Set( meta, vlc_meta_Setting, info->trk.hw); */
  /* { vlc_meta_Setting,     TAG68_HARDWARE}, */
  /* TrackID ?
     vlc_meta_Set( meta, vlc_meta_TrackID, trackstr); */
  /* vlc_meta_Set( meta, vlc_meta_Description, */
  /*               info->dsk.tag[TAG68_ID_GENRE].val); */

  for (i=TAG68_ID_CUSTOM;
       !sc68_tag_enum(p_sys->sc68, &tag, p_sys->track, i, 0);
       ++i) {
    meta_lut_t * lut, ent;
    ent.sc68 = tag.key;
    lut = bsearch(&ent, meta_lut, sizeof(meta_lut)/sizeof(*meta_lut),
                  sizeof(*meta_lut), cmp_meta_lut);
    if (lut)
      vlc_meta_Set( meta, lut->vlc, tag.val);
  }
  return meta;
}

static char * mk_title(demux_sys_t * const p_sys, int i, const char * extra)
{
  char * name;
  const char * album, * title;

  extra = extra ? extra : "";
  album = p_sys->infos[i].album;
  title = p_sys->infos[i].trk.tag[TAG68_ID_TITLE].val;

  if (album == title || !strcmp(album, title)) {
    name = malloc(strlen(album) + strlen(extra) + 16);
    if (name)
      sprintf(name, "%s - #%02d%s", album, i+1, extra);
  } else {
    name = malloc( strlen(title) + strlen(album) + strlen(extra) + 4);
    if (name)
      sprintf(name, "%s - %s%s", album, title, extra);
  }
  return name;
}

/*****************************************************************************
 * open: initializes ES structures
 *****************************************************************************/
static int Open(vlc_object_t * p_this)
{
  static int id;
  demux_t     * p_demux = (demux_t*)p_this;
  const uint8_t *p_peek;

  int err = VLC_EGENERIC;
  sc68_create_t  create68;     /* Parameters for creating sc68 instance */
  vfs68_t * stream68 = 0;
  int i, tracks, track;
  char * s;

  es_format_t fmt;

#ifdef DEBUG
  fprintf(stderr,"sc68-vlc: Enter %s() demux:%p mmm_init=%d !\n",
          __FUNCTION__, (void*)p_demux,mmm_init);
#endif
  dbg(p_demux,"Enter %s(%s)", __FUNCTION__, p_demux->psz_file);

  assert(vlc_initialized == 1);
  if (vlc_initialized != 1) {
#ifdef DEBUG
    fprintf(stderr, "sc68-vlc: wrong stat (%d)\n",vlc_initialized);
    fflush(stderr);
#endif
    msg_Err(p_demux, "sc68-vlc wrong stat (%d)\n",vlc_initialized);
    return -1;
  }
  /* assert(!p_demux->sc68); */

  /* if (!p_demux->s) { */
  /*   dbg(p_demux,"no stream ?"); */
  /*   goto exit; */
  /* } */
  assert(p_demux->s);

  /* Check if we are dealing with a sc68 file */
  if (i = stream_Peek(p_demux->s, &p_peek, 16), i < 16) {
    dbg(p_demux,"Can't peek (%d)",i);
    goto exit;
  }
  dbg(p_demux,"Have peek %d bytes\n",i);

  /* Fast check for our files ... */
  /* TODO: Gzip                   */
  if (!memcmp(p_peek, "SC68", 4)) {
    dbg(p_demux,"SC68 4cc detected");
  } else if (!memcmp(p_peek, "ICE!", 4)||!memcmp(p_peek, "Ice!", 4)) {
    dbg(p_demux,"ICE! 4cc detected");
  } else if (!memcmp(p_peek+12, "SNDH", 4)) {
    dbg(p_demux,"SNDH 4cc detected");
  } else if (!memcmp(p_peek, "\037\213\010", 3)) {
    dbg(p_demux,"GZIP detected");
  } else {
    dbg(p_demux,"Not sc68 ? '%02x-%02x-%02x-%02x' ?",
        p_peek[0], p_peek[1], p_peek[2], p_peek[3]);
    goto exit;
  }

  p_demux->p_sys = (demux_sys_t*) calloc (1, sizeof(demux_sys_t));
  if (unlikely(!p_demux->p_sys))
    goto error;

  /***********************************************************************
  * Apply VLC config
  */

  if (s = var_InheritString( p_demux, CFG_PREFIX "ym-engine"), s) {
    dbg(p_demux, "vlc-config -- ym-engine -- *%s*\n", s);
    sc68_cntl(0,SC68_SET_OPT_STR,"ym-engine",s);
  }

  if (s = var_InheritString( p_demux, CFG_PREFIX "ym-filter"), s) {
    dbg(p_demux, "vlc-config -- ym-filter -- *%s*\n", s);
    sc68_cntl(0,SC68_SET_OPT_STR,"ym-filter",s);
  }

  if (s = var_InheritString( p_demux, CFG_PREFIX "ym-volmodel"), s) {
    dbg(p_demux, "vlc-config -- ym-volmodel -- *%s*\n", s);
    sc68_cntl(0,SC68_SET_OPT_STR,"ym-volmodel",s);
  }

  i = var_InheritBool( p_demux, CFG_PREFIX "amiga-filter");
  sc68_cntl(0,SC68_SET_OPT_INT,"amiga-filter", i);
  dbg(p_demux, "vlc-config -- amiga-filter -- *%s*\n", i?"On":"Off");

  i = var_InheritInteger( p_demux, CFG_PREFIX "amiga-blend");
  sc68_cntl(0,SC68_SET_OPT_INT,"amiga-blend", i);
  dbg(p_demux, "vlc-config -- amiga-blend -- *%d*\n", i);

  /* How tracks are presented ? title or chapter ? */
  i = var_InheritBool( p_demux, CFG_PREFIX "all-in-1" );
  p_demux->p_sys->allin1 = i;
  dbg(p_demux, "vlc-config -- sub-song mode -- *%s*\n",
      p_demux->p_sys->allin1 ? "all-in-1" : "title-by-track");

  /* aSID mode */
  i = var_InheritInteger(p_demux, CFG_PREFIX "asid-mode");
  dbg(p_demux, "vlc-config -- aSID mode -- *%d*\n", i);
  i = sc68_cntl(0, SC68_SET_ASID, i);
  p_demux->p_sys->asidify = i;
  dbg(p_demux, "vlc-config -- aSIDify -- *%d*\n", i);

  /* Sample rate */
  i = var_InheritInteger(p_demux, CFG_PREFIX "samplerate");
  i = sc68_cntl(0,SC68_SET_SPR, i);
  p_demux->p_sys->sample_rate = i;
  dbg(p_demux, "vlc-config -- sample-rate -- *%d*\n", i);

  /* Create sc68 instance */
  memset(&create68,0,sizeof(create68));
#ifdef DEBUG
  memcpy(p_demux->p_sys->name,"vlc68#",6);
  p_demux->p_sys->name[6] = '0' +  id / 100 % 10;
  p_demux->p_sys->name[7] = '0' +  id / 10 % 10;
  p_demux->p_sys->name[8] = '0' +  id % 10;
  create68.name = p_demux->p_sys->name;
#endif
  p_demux->p_sys->sc68 = sc68_create(&create68);
  if (!p_demux->p_sys->sc68)
    goto error;

  /* Set sc68 cookie */
  sc68_cntl(p_demux->p_sys->sc68, SC68_SET_COOKIE, p_demux);

  /* Load and prepare sc68 file */
  stream68 = vfs68_vlc_create(p_demux->s, p_demux->psz_file);
  if (unlikely(!stream68))
    goto error;
  if (sc68_load(p_demux->p_sys->sc68, stream68))
    goto error;

  /* Play all track from first for now */
  if (sc68_play(p_demux->p_sys->sc68, 1, SC68_DEF_LOOP) == -1)
    goto error;

  /* This ensure the track is actually loaded and initialized */
  if (sc68_process(p_demux->p_sys->sc68, 0, 0) == SC68_ERROR)
    goto error;

  /* Get sampling rate */
  p_demux->p_sys->sample_rate
    = sc68_cntl(p_demux->p_sys->sc68, SC68_SET_SPR, p_demux->p_sys->sample_rate);
  if (p_demux->p_sys->sample_rate < 0)
    goto error;
  dbg(p_demux, "spr: %d\n", p_demux->p_sys->sample_rate);

  /* Set track info and count aSid compatible */
  p_demux->p_sys->tracks = tracks
    = sc68_cntl(p_demux->p_sys->sc68, SC68_GET_TRACKS);
  dbg(p_demux, "number-of-track: %d\n", p_demux->p_sys->tracks);
  for (i = 0; i < tracks; ++i) {
    sc68_minfo_t * info = p_demux->p_sys->infos+i;
    int can_asid;
    if (sc68_music_info(p_demux->p_sys->sc68, info, i+1, 0))
      continue;
    can_asid = info->trk.ym &&
      (info->trk.asid || p_demux->p_sys->asidify == SC68_ASID_FORCE);
    dbg(p_demux, "track #%02d: can aSID ? -- *%s*\n",
        i+1, can_asid ? "yes" : "no");
    p_demux->p_sys->asid_count += info->trk.asid = can_asid;
  }
  dbg(p_demux, "asid-count: %d\n", p_demux->p_sys->asid_count);

  /* Set aSID mode by default */
  if (!p_demux->p_sys->asid_count)
    p_demux->p_sys->asidify = SC68_ASID_OFF;
  i = var_InheritBool(p_demux,CFG_PREFIX "asid-fav");
  dbg(p_demux, "vlc-config -- favour aSID mode -- *%s*\n", i?"yes":"no");
  i = i ? p_demux->p_sys->asidify : SC68_ASID_OFF;
  i = sc68_cntl(p_demux->p_sys->sc68, SC68_SET_ASID, i);
  p_demux->p_sys->asid = i & SC68_ASID_ON;

  /* Set internal track */
  track = current_track(p_demux);
  if (track < 0)
    goto error;
  dbg(p_demux, "current-track: %d\n", p_demux->p_sys->track);

  dbg(p_demux, "aSID is *%s*\n", p_demux->p_sys->asid?"On":"Off");

  /* Setup Audio Elementary Stream. */
  p_demux->p_sys->pcm_per_loop = 512;
  es_format_Init (&fmt, AUDIO_ES, VLC_CODEC_S16N);
  fmt.audio.i_channels        = 2;
  fmt.audio.i_bitspersample   = 16;
  fmt.audio.i_rate            = p_demux->p_sys->sample_rate;
  fmt.audio.i_bytes_per_frame =
    fmt.audio.i_frame_length  =
    fmt.audio.i_blockalign    = 4;
  fmt.i_bitrate = fmt.audio.i_rate * fmt.audio.i_bytes_per_frame;
  p_demux->p_sys->es = es_out_Add(p_demux->out, &fmt);
  date_Init(&p_demux->p_sys->pts, fmt.audio.i_rate, 1);
  date_Set(&p_demux->p_sys->pts, 0);

  /* Callbacks */
  p_demux->pf_demux   = Demux;
  p_demux->pf_control = Control;


  if (p_demux->p_sys->allin1) {
    /* All in 1 mode. can have 1 or 2 titles (normal / aSID) */
    p_demux->info.i_title     = p_demux->p_sys->asid;
    p_demux->info.i_seekpoint = track - 1;
    p_demux->info.i_update
      = INPUT_UPDATE_META | INPUT_UPDATE_TITLE | INPUT_UPDATE_SEEKPOINT;
  } else {
    p_demux->info.i_title     = track - 1;
    p_demux->info.i_title    += p_demux->p_sys->asid ? tracks : 0;
    p_demux->info.i_seekpoint = 0;
    p_demux->info.i_update
      = INPUT_UPDATE_META | INPUT_UPDATE_TITLE;
  }

  err = VLC_SUCCESS;
error:
  /* Don't need our stream anymore */
  vfs68_destroy(stream68);

  /* flush sc68 engin error */
  /* flush_sc68_errors(p_demux); */

  if (err != VLC_SUCCESS) {
    /* On error ... */
    /* $$$ TODO : check for allocated stuff like titles and infos */
    sys_shutdown(p_demux);
  }

exit:
  dbg(p_demux,"Leave %s() => [%d \"%s\"]", __FUNCTION__,
      err, vlc_error(err));
  return err;
}

/*****************************************************************************
 * Close: frees unused data
 *****************************************************************************/
static void Close( vlc_object_t * p_this )
{
  demux_t        *p_demux = (demux_t*)p_this;
  sys_shutdown(p_demux);
}

/*****************************************************************************
 * Demux: reads and demuxes data packets
 *****************************************************************************
 * Returns -1 in case of error, 0 in case of EOF, 1 otherwise
 *****************************************************************************/
static int Demux( demux_t *p_demux )
{
  int code, /* ms, */ pcm_per_loop;
  demux_sys_t *p_sys   = p_demux->p_sys;
  block_t     *p_block = 0;

  p_block = block_Alloc(p_sys->pcm_per_loop << 2);
  if (unlikely(!p_block)) {
    return 0;                   /* Shouldn't we return -1 ? */
  }

  pcm_per_loop = p_block->i_buffer >> 2;
  code = sc68_process(p_sys->sc68,  p_block->p_buffer, &pcm_per_loop);
  if (code == SC68_ERROR) {
    block_Release(p_block);
    return 0;
  }

  if (pcm_per_loop <= 0) {
    block_Release(p_block);
  } else {
    p_block->i_nb_samples = pcm_per_loop;
    p_block->i_dts        = VLC_TS_0 + date_Get (&p_sys->pts);
//      (int64_t) sc68_cntl(p_sys->sc68,

    p_block->i_pts        = p_block->i_dts;

    /* set PCR */
    es_out_Control( p_demux->out, ES_OUT_SET_PCR, p_block->i_pts );
    es_out_Send( p_demux->out, p_sys->es, p_block );
    date_Increment( &p_sys->pts, pcm_per_loop);
  }

  if (code & SC68_LOOP) {
    /* Nothing to do really */
  }

  if (code & SC68_CHANGE) {
    int track = current_track(p_demux);
    dbg(p_demux,"Change to track -- %d\n", track);
    if (p_sys->allin1) {
      p_demux->info.i_seekpoint = track-1;
      p_demux->info.i_update    = INPUT_UPDATE_META | INPUT_UPDATE_SEEKPOINT;
      date_Set(&p_sys->pts,
               (int64_t) sc68_cntl(p_sys->sc68,SC68_GET_TRKORG) * 1000);
    } else {
      p_demux->info.i_title  = track - 1 + (p_sys->asid ? p_sys->tracks : 0);
      p_demux->info.i_update = INPUT_UPDATE_META | INPUT_UPDATE_TITLE;
      date_Set(&p_sys->pts, 0);
    }

    if (p_demux->info.i_update & INPUT_UPDATE_SEEKPOINT)
      dbg(p_demux,"seekpoint update -- %d\n", p_demux->info.i_seekpoint);

    if (p_demux->info.i_update & INPUT_UPDATE_TITLE)
      dbg(p_demux,"title update -- %d\n", p_demux->info.i_title);

  }

/* i_update field of access_t/demux_t */
/* #define INPUT_UPDATE_NONE       0x0000 */
/* #define INPUT_UPDATE_SIZE       0x0001 */
/* #define INPUT_UPDATE_TITLE      0x0010 */
/* #define INPUT_UPDATE_SEEKPOINT  0x0020 */
/* #define INPUT_UPDATE_META       0x0040 */
/* #define INPUT_UPDATE_SIGNAL     0x0080 */
/* #define INPUT_UPDATE_TITLE_LIST 0x0100 */

  return (code & SC68_END)
    ? 0                                 /* on EOF */
    : 1                                 /* on continue */
    ;
}

/*****************************************************************************
 * Control:
 *****************************************************************************/

static const char * qstr(int q) {
  switch (q) {
  case DEMUX_GET_POSITION: return "GET_POSITION";
  case DEMUX_SET_POSITION: return "SET_POSITION";
  case DEMUX_GET_LENGTH: return "GET_LENGTH";
  case DEMUX_GET_TIME: return "GET_TIME";
  case DEMUX_SET_TIME: return "SET_TIME";
  case DEMUX_GET_TITLE_INFO: return "GET_TITLE_INFO";
  case DEMUX_SET_TITLE: return "SET_TITLE";
  case DEMUX_SET_SEEKPOINT: return "SET_SEEKPOINT";
  case DEMUX_SET_GROUP: return "SET_GROUP";
  case DEMUX_SET_NEXT_DEMUX_TIME: return "SET_NEXT_DEMUX_TIME";
  case DEMUX_GET_FPS: return "GET_FPS";
  case DEMUX_GET_META: return "GET_META";
  case DEMUX_HAS_UNSUPPORTED_META: return "HAS_UNSUPPORTED_META";
  case DEMUX_GET_ATTACHMENTS: return "GET_ATTACHMENTS";
  case DEMUX_CAN_RECORD: return "CAN_RECORD";
  case DEMUX_SET_RECORD_STATE: return "SET_RECORD_STATE";
  case DEMUX_CAN_PAUSE: return "CAN_PAUSE";
  case DEMUX_SET_PAUSE_STATE: return "SET_PAUSE_STATE";
  case DEMUX_GET_PTS_DELAY: return "GET_PTS_DELAY";
  case DEMUX_CAN_CONTROL_PACE: return "CAN_CONTROL_PACE";
  case DEMUX_CAN_CONTROL_RATE: return "CAN_CONTROL_RATE";
  case DEMUX_SET_RATE: return "SET_RATE";
  case DEMUX_CAN_SEEK: return "CAN_SEEK";
    /* Navigation */
  case DEMUX_NAV_ACTIVATE: return "NAV_ACTIVATE";
  case DEMUX_NAV_UP:       return "NAV_UP";
  case DEMUX_NAV_DOWN:     return "NAV_DOWN";
  case DEMUX_NAV_LEFT:     return "NAV_LEFT";
  case DEMUX_NAV_RIGHT:    return "NAV_RIGHT";
  }
  return "???";
}

static int Control( demux_t *p_demux, int i_query, va_list args )
{
  demux_sys_t *p_sys  = p_demux->p_sys;

  switch (i_query) {

      /* Ignore those */
  case DEMUX_CAN_SEEK: case DEMUX_CAN_RECORD: case DEMUX_GET_FPS:
  case DEMUX_GET_PTS_DELAY: case DEMUX_SET_GROUP:
    break;

    /* Check those later */
  case DEMUX_GET_ATTACHMENTS:
    /* input_attachment_t*** */
    /* int* res */
    break;

  case DEMUX_GET_META: {
    vlc_meta_t * p_meta = (vlc_meta_t *)va_arg(args, vlc_meta_t*);
    p_meta = track_meta(p_demux, p_meta);
    return VLC_SUCCESS;
  } break;

  case DEMUX_HAS_UNSUPPORTED_META:
    *va_arg(args, bool*) = true;
    return VLC_SUCCESS;

  case DEMUX_GET_LENGTH: {
    /* LENGTH/TIME in microsecond, 0 if unknown */
    int64_t * pi64 = (int64_t *) va_arg(args, int64_t *);
    const int fct  = p_sys->allin1 ? SC68_GET_DSKLEN : SC68_GET_LEN;
    int ms
      = sc68_cntl(p_sys->sc68, fct);
    if (ms < 0) ms = 0;
    *pi64 = (int64_t) ms * 1000;
    return VLC_SUCCESS;
  } break;

  case DEMUX_GET_TIME: {
    /* time in us */
    int64_t * pi64 = (int64_t *) va_arg(args, int64_t *);
    if (0) {
      const int fct  = p_sys->allin1 ? SC68_GET_DSKPOS : SC68_GET_POS;
      int ms
        = sc68_cntl(p_sys->sc68, fct);
      if (ms < 0) ms = 0;
      *pi64 = (int64_t) ms * 1000;
    } else {
      *pi64 = date_Get (&p_sys->pts);
    }
    return VLC_SUCCESS;

  } break;

  case DEMUX_GET_POSITION: {
    /* position in range 0.0 .. 1.0 */
    double lf, * plf = (double *) va_arg(args, double *);
    int pos_ms, len_ms;
    if (p_sys->allin1) {
      pos_ms = sc68_cntl(p_sys->sc68, SC68_GET_DSKPOS);
      len_ms = sc68_cntl(p_sys->sc68, SC68_GET_DSKLEN);
    } else {
      pos_ms = sc68_cntl(p_sys->sc68, SC68_GET_POS);
      len_ms = sc68_cntl(p_sys->sc68, SC68_GET_LEN);
    }
    if (pos_ms >= 0 && len_ms > 0) {
      *plf = lf = (double) pos_ms / (double) len_ms;
      return VLC_SUCCESS;
    }
  } break;

    /* case DEMUX_SET_POSITION: { */
    /*   double lf = (double)va_arg(args, double); */
    /*   int ms; */
    /*   ms = p_sys->info.start_ms + (int)(p_sys->info.trk.time_ms * lf); */
    /*   if (ms < p_sys->info.start_ms) */
    /*     ms = p_sys->info.start_ms; */
    /*   else if (ms > p_sys->info.start_ms+p_sys->info.trk.time_ms) */
    /*     ms = p_sys->info.start_ms+p_sys->info.trk.time_ms; */
    /*   if (sc68_seek(p_sys->sc68, ms, 0) == -1) */
    /*     break; */
    /*   return VLC_SUCCESS; */
    /* } break; */

    /* case DEMUX_SET_TIME: */
    /*   break; */

  case DEMUX_GET_TITLE_INFO: {
    input_title_t /* *** ppp_title, */ ** titles;
    /* int * pi_int, pi_title_offset, * pi_seekpoint_offset */;
    int   n , can_asid;
    static const char asid_xtra[] = " (aSid)";

    can_asid = 1 + (p_sys->asid_count > 0);
    dbg(p_demux,"%s -- can-asid=%d\n",qstr(i_query),can_asid);

    n = p_sys->allin1 ? can_asid : p_sys->tracks * can_asid;
    dbg(p_demux,"%s -- n-titles=%d\n",qstr(i_query),n);

    titles = (input_title_t **) malloc(sizeof(void*) * n);
    if (unlikely(!titles))
      break;

    dbg(p_demux,"%s -- all-in-1=%d\n",qstr(i_query), p_sys->allin1);

    if (p_sys->allin1) {
      const char * album = p_sys->infos[0].album;
      int i, j;
      for (j=0; j<n; ++j) {
        input_title_t * tp;

        dbg(p_demux,"%s -- title #%d/%d\n",qstr(i_query), j+1, n);

        tp = vlc_input_title_New();
        if (unlikely(!tp))
          continue;

        dbg(p_demux,"%s -- title #%d -- album=%s\n",qstr(i_query), j+1, album);
        tp->i_length = (int64_t) p_sys->infos[0].dsk.time_ms * 1000;
        tp->b_menu   = false;
        if (!j) {
          tp->psz_name = strdup(album);
        } else {
          sprintf(tp->psz_name
                  = malloc(strlen(album)+sizeof(asid_xtra)),
                  "%s%s", album, asid_xtra);
        }

        tp->seekpoint
          = (seekpoint_t **) malloc(sizeof(void*) * p_sys->tracks);
        tp->i_seekpoint = p_sys->tracks;

        dbg(p_demux,"%s -- title #%d -- seekpoints=%d\n",
            qstr(i_query), j+1, p_sys->tracks);

        for (i=0; i<p_sys->tracks; i++) {
          seekpoint_t * sp = vlc_seekpoint_New();
          if (unlikely(!sp))
            continue;
          sp->i_time_offset =
            (int64_t) sc68_cntl(p_sys->sc68, SC68_GET_TRKORG, i+1) * 1000;
          /* sp->psz_name = strdup(p_sys->infos[i].tag[TAG68_ID_TITLE].val); */
          sp->psz_name = mk_title(p_sys, i, !j ? 0 : asid_xtra);
          tp->seekpoint[i] = sp;
        }
        titles[j] = tp;
      }
    } else {
      int j;
      for (j=0; j<n; j++) {
        input_title_t * tp;

        dbg(p_demux,"%s -- title #%d/%d\n",qstr(i_query), j+1, n);

        tp = vlc_input_title_New();
        if (unlikely(!tp))
          continue;

        tp->b_menu   = false;
        if (j < p_sys->tracks) {
          tp->i_length = (int64_t) p_sys->infos[j].trk.time_ms * 1000;
          tp->psz_name = mk_title(p_sys, j, 0);
        } else {
          int k = j - p_sys->tracks;
          tp->i_length = (int64_t) p_sys->infos[k].trk.time_ms * 1000;
          tp->psz_name = mk_title(p_sys, k, asid_xtra);
        }
        titles[j]    = tp;
      }
    }
    *va_arg(args, input_title_t***) = titles; /* titles array       */
    *va_arg(args, int*) = n;                  /* number of titles   */
    *va_arg(args, int*) = 0;                  /* titles offset ?    */
    *va_arg(args, int*) = 0;                  /* seekpoint offset ? */

    dbg(p_demux,"DEMUX_GET_TITLE_INFO => %d titles\n", n);

    return VLC_SUCCESS;
  } break;

  case DEMUX_SET_TITLE: {
    int track = (int) va_arg(args, int);
    /* int can_asid = 1 + (p_sys->asid_count > 0); */

    if (p_sys->allin1) {
      p_sys->asid = track > 0 ;
      sc68_cntl(p_sys->sc68, SC68_SET_ASID, p_sys->asid);
    } else {
      int trk;
      if (track >= p_sys->tracks) {
        p_sys->asid = 1;
        trk = 1 + track - p_sys->tracks;
      } else {
        p_sys->asid = 0;
        trk = 1 + track;
      }
      sc68_cntl(p_sys->sc68, SC68_SET_ASID, p_sys->asid);
      if (-1 == sc68_play(p_sys->sc68, trk, SC68_DEF_LOOP))
        break;
    }
    p_demux->info.i_title     = track;
    p_demux->info.i_update    = INPUT_UPDATE_META | INPUT_UPDATE_TITLE;

    return VLC_SUCCESS;
  } break;


  case DEMUX_SET_SEEKPOINT: {
    int track = (int) va_arg(args, int);

    dbg(p_demux,"%s(%d)\n", qstr(i_query), track);

    if (!p_sys->allin1)
      break;

    if (track < 0 || track >= p_sys->tracks)
      break;

    sc68_cntl(p_demux->p_sys->sc68, SC68_SET_ASID, p_sys->asid);
    if (-1 == sc68_play(p_sys->sc68, track+1, SC68_DEF_LOOP))
      break;
    p_demux->info.i_seekpoint = track;
    p_demux->info.i_update    = INPUT_UPDATE_META | INPUT_UPDATE_SEEKPOINT;

    return VLC_SUCCESS;
  } break;

  default:
    dbg(p_demux, "unhandled cntl query -- %s (%d)\n",
        qstr(i_query), i_query);
  }

  return VLC_EGENERIC;
}

static void msg(const int cat, void * cookie, const char * fmt, va_list list)
{
#if defined (MSG_TO_STDERR)
  /* Use stderr regardless ... */
  vfprintf(stderr,fmt,list);
  fflush(stderr);
#elif defined (WIN32)
  /* Use Windows debug output (could if we have a console) */
  char tmp[512];
  const int max = sizeof(tmp)-1;
  strcpy(tmp,"vlc68: ");
  vsnprintf(tmp+7,max-7,fmt,list);
  tmp[max] = 0;
  OutputDebugStringA(tmp);
#else
  /* Use vlc message function. Apparently the vlc_object can be null
   * when calling vlc_vALog()
   */
  demux_t * p_demux = (demux_t *) cookie;
  vlc_object_t * vlc_object = VLC_OBJECT(p_demux);
  int level;
  switch (cat) {
  case  msg68_CRITICAL: case  msg68_ERROR:
    level = VLC_MSG_ERR; break;
  case msg68_WARNING:
    level = VLC_MSG_WARN; break;
  case msg68_INFO: case msg68_NOTICE:
    level = VLC_MSG_INFO; break;
  default:
    level = VLC_MSG_DBG;
  }
  vlc_vaLog(vlc_object, level, MODULE_STRING, fmt , list);
#endif
}
