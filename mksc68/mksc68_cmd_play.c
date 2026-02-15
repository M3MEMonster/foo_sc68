/*
 * @file    mksc68_cmd_play.c
 * @brief   the "play" command
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

#include "mksc68_cmd.h"
#include "mksc68_dsk.h"
#include "mksc68_msg.h"
#include "mksc68_opt.h"
#include "mksc68_str.h"
#include "mksc68_gdb.h"

#include <sc68/file68.h>
#include <sc68/file68_vfs.h>
#include <sc68/sc68.h>
#include <emu68/emu68.h>
#include <emu68/excep68.h>
#include <io68/io68.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>

extern void sc68_emulators(sc68_t *, emu68_t **, io68_t ***);

static const opt_t longopts[] = {
  { "help",     0, 0, 'h' },            /* help             */
  { "loop",     1, 0, 'l' },            /* loopplay         */
  { "seek",     1, 0, 's' },            /* seek to position */
  { "to",       1, 0, 't' },            /* stop time        */
  { "fg",       0, 0, 'f' },            /* foreground play  */
  { "debug",    0, 0, 'd' },            /* gdb debug mode   */
  { "memory",   1, 0, 'm' },            /* 68k memory size  */
  { 0,0,0,0 }
};

typedef struct {
  volatile int  isplaying;              /* 1:play 2:init    */
  volatile int  debug;                  /* run gdb mode     */

  sc68_t      * sc68;                   /* sc68 instance    */
  emu68_t     * emu68;                  /* emu68 instance   */
  io68_t     ** ios68;                  /* other chip       */
  gdb_t       * gdb;                    /* gdb instance     */
  vfs68_t     * out;                    /* audio out stream */
  pthread_t     thread;                 /* play thread      */
  int           track;                  /* track to play    */
  int           loop;                   /* number of loop   */
  int           code;                   /* return code 0=OK */
  int           log2;                   /* 68k memory size  */
} playinfo_t;

static playinfo_t playinfo;             /* unique playinfo  */

static void play_info(playinfo_t * pi)
{
  sc68_music_info_t info;
  if (!sc68_music_info(pi->sc68,&info,SC68_CUR_TRACK,0)) {
    int i, len = 11;
    msginf("%-*s : %d/%d\n", len, "Track",    info.trk.track,info.tracks);
    msginf("%-*s : %s\n",    len, "Album",    info.album);
    msginf("%-*s : %s\n",    len, "Title",    info.title);
    msginf("%-*s : %s\n",    len, "Artist",   info.artist);
    msginf("%-*s : %s\n",    len, "Hardware", info.trk.hw);
    msginf("%-*s : %s\n",    len, "Duration", info.trk.time);

    if (info.dsk.tags) {
      msginf("Disk tags:\n");
      for (i=2; i<info.dsk.tags; ++i)
        msginf("%c%-*s : %s\n",
               toupper((int) *info.dsk.tag[i].key),
               len-1,
               info.dsk.tag[i].key+1,
               info.dsk.tag[i].val);
    }

    if (info.trk.tags) {
      msginf("Track tags:\n");
      for (i=2; i<info.trk.tags; ++i)
        msginf("%c%-*s : %s\n",
               toupper((int) *info.trk.tag[i].key),
               len-1,
               info.trk.tag[i].key+1,
               info.trk.tag[i].val);
    }
  }
}

static void play_hdl(emu68_t* const emu68, int vector, void * cookie)
{
  int gdbstat, gdbcode;
  playinfo_t * const pi = cookie;
  const char * gdbmsg;

  assert(pi == &playinfo);
  assert(emu68 == pi->emu68);
  assert(pi->gdb);

  gdbstat = gdb_event(pi->gdb, vector, emu68);
  gdbcode = gdb_error(pi->gdb, &gdbmsg);

  switch (gdbstat) {

  case RUN_EXIT:
    msgdbg("gdb-stub *EXIT* (%d) \"%s\", halting 68k\n", gdbcode, gdbmsg);
    emu68_error_add(emu68,"mksc68:play: "
                    "gdb *EXIT* (%d) \"%s\" -- %s(%d) => %s(%d)",
                    gdbcode, gdbmsg,
                    emu68_status_name(emu68->status), emu68->status,
                    emu68_status_name(EMU68_ERR), EMU68_ERR);
    emu68->status = EMU68_HLT;
  case RUN_CONT:
    break;

  case RUN_SKIP:
    msgdbg("destroying gdb stub and unregister 68k event handler\n");
    emu68_set_handler(emu68,0);
    gdb_destroy(pi->gdb);
    pi->gdb = 0;
    break;

  case RUN_STOP:
    /* msgdbg("stopped by interrupt or something\n"); */
    assert(!"gdb-stub *STOP* but we are not stopped !!!");
    emu68_error_add(emu68,"mksc68:play: "
                    "gdb-stub *STOP* -- %s(%d) => %s(%d)",
                    emu68_status_name(emu68->status), emu68->status,
                    emu68_status_name(EMU68_ERR), EMU68_ERR);
    emu68->status = EMU68_ERR;
    break;

  default:
    assert(!"unhandled or invalid gdb status");
    msgerr("gdb-stub: invalid status(%d,%d,\"%s\")\n",
           gdbstat, gdbcode, gdbmsg);
    emu68_error_add(emu68,
                    "mksc68:play: invalid gdb status(%d,%d,\"%s\")"
                    " -- %s(%d) => %s(%d)",
                    gdbstat, gdbcode, gdbmsg,
                    gdbstat,
                    emu68_status_name(emu68->status), emu68->status,
                    emu68_status_name(EMU68_ERR), EMU68_ERR);
    emu68->status = EMU68_ERR;
  }
}

static void play_init(playinfo_t * pi)
{
  const char * outname = pi->debug ? "null://default" : "audio://default";
  /* const char * outname = "audio://default"; */
  sc68_create_t create68;

  pi->isplaying = 2;
  pi->code      = -1;
  pi->out       = sc68_vfs(outname, 2, 0);
  pi->gdb       = 0;

  if (!pi->out) return;
  if (vfs68_open(pi->out)) return;

  memset(&create68,0,sizeof(create68));
  create68.name = pi->debug ? "mksc68-debug" : "mksc68-play";
  create68.emu68_debug = !!pi->debug;
  create68.log2mem = pi->log2;
  if ( !(pi->sc68 = sc68_create(&create68) ) ) return;
  if (pi->debug) {
    sc68_cntl(pi->sc68, SC68_EMULATORS, &pi->ios68);
    pi->emu68 = (emu68_t *)*pi->ios68++;
    emu68_set_cookie(pi->emu68, pi);
    pi->gdb = gdb_create();
    if (!pi->gdb)
      return;
    emu68_set_handler(pi->emu68, play_hdl);
  }

  if ( sc68_open(pi->sc68, (sc68_disk_t) dsk_get_disk() ) ) return;
  if ( sc68_play(pi->sc68, pi->track, pi->loop) < 0 ) return;
  pi->code = -(sc68_process(pi->sc68, 0, 0) == SC68_ERROR);
  if (!pi->code)
    play_info(pi);
}

static void play_run(playinfo_t * pi)
{
  char buffer[512 * 4];
  int loop = 0, code;

  pi->isplaying = 1;
  pi->code      = 1;
  for ( ;; ) {
    int n = sizeof(buffer) >> 2;
    code = sc68_process(pi->sc68, buffer, &n);
    if (code & (SC68_END|SC68_CHANGE))
      break;
    if (code & SC68_LOOP)
      ++loop;
    if (vfs68_write(pi->out, buffer, (n << 2)) != (n << 2)) {
      code = SC68_ERROR;
      break;
    }
  }
  pi->code = -(code == SC68_ERROR);
}

static void play_end(playinfo_t * pi)
{
  pi->isplaying = 3;
  vfs68_close(pi->out);
  pi->out = 0;
  gdb_destroy(pi->gdb);
  pi->gdb = 0;
  sc68_destroy(pi->sc68);
  pi->sc68 = 0;
  pi->isplaying = 0;
}

static void * play_thread(void * userdata)
{
  playinfo_t * pi = (playinfo_t *) userdata;

  assert( pi == &playinfo );
  play_init(pi);
  if (!pi->code) play_run(pi);
  play_end(pi);
  return pi;
}

int dsk_stop(void)
{
  int err = -1;

  if (playinfo.isplaying) {
    if (playinfo.sc68) {
      const char * name = 0;
      sc68_cntl(playinfo.sc68, SC68_GET_NAME, &name);
      msgdbg("stop: signal stop to %s\n", name ? name : "(nil)");
      sc68_stop(playinfo.sc68);
    } else {
      pthread_cancel(playinfo.thread);
    }
    pthread_join(playinfo.thread, 0);
    err = playinfo.code;
  } else {
    msgnot("not playing\n");
  }
  return err;
}

int dsk_playing(void)
{
  return (playinfo.isplaying == 1) + (!!playinfo.debug << 1);
}

struct dsk_play_s {
  int track, loop, start, len, bg, debug, log2;
};


int dsk_play(dsk_play_t * p)
{
  int err = -1;

  msgdbg("play [track:%d loop:%d start:%d length:%d bg:%d dbg:%d mem:%d]\n",
         p->track, p->loop, p->start, p->len, p->bg, p->debug, p->log2);

  if (playinfo.isplaying) {
    dsk_stop();
  }

  memset(&playinfo,0,sizeof(playinfo));
  playinfo.track = p->track;
  playinfo.loop  = p->loop;
  playinfo.debug = p->debug;
  playinfo.log2 = p->log2;
  if ( pthread_create(&playinfo.thread, 0, play_thread, &playinfo) ) {
    msgerr("error creating play thread\n");
    goto error;
  }

  if (!p->bg) {
    pthread_join(playinfo.thread, 0);
    err = playinfo.code;
  } else {
    msginf("play: playing in background (use `stop' command to stop)\n");
    err = 0;
  }

error:
  return err;
}


static
int run_play(cmd_t * cmd, int argc, char ** argv)
{
  char shortopts[(sizeof(longopts)/sizeof(*longopts))*3];
  int ret = -1, i, tracks;
  const char * str;
  int seek_mode = '?', dur_mode = '?';
  dsk_play_t params;

  opt_create_short(shortopts, longopts);
  memset(&params,0,sizeof(params));
  params.bg = 1;
  params.loop = SC68_DEF_LOOP;

  while (1) {
    int longindex;
    int val =
      getopt_long(argc, argv, shortopts, longopts, &longindex);

    switch (val) {
    case  -1: break;                    /* Scan finish */

    case 'h':                           /* --help */
      help(argv[0]); return 0;

    case 'l':                           /* --loop      */
      str = optarg;
      if (!strcmp(str,"inf"))
        params.loop = SC68_INF_LOOP;
      else if (!strcmp(str,"def"))
        params.loop = SC68_DEF_LOOP;
      else if (isdigit((int)*str)) {
        params.loop = strtol(str,0,0);
        if (!params.loop)
          params.loop = SC68_DEF_LOOP;
      }
      break;

    case 'm':                           /* --memory    */
      str = optarg;
      if (isdigit((int)*str))
        params.log2 = strtol(str,0,0);
      break;

    case 'f':                           /* --fg        */
      params.bg = 0; break;

    case 's':                           /* --seek      */
      str = optarg;
      if (*str == '+' || *str == '-')
        seek_mode = *str++; /* $$$ TODO ? */
      seek_mode = seek_mode;
      if (str_time_stamp(&str, &params.start)) {
        msgerr("invalid seek value \"%s\"\n", optarg);
        goto error;
      }
      break;

    case 't':                           /* --to        */
      str = optarg;
      if (*str == '+' || *str == '-')
        dur_mode = *str++; /* $$$ TODO ? */
      dur_mode = dur_mode;
      if (str_time_stamp(&str, &params.len)) {
        msgerr("invalid duration value \"%s\"\n", optarg);
        goto error;
      }
      break;

    case 'd':
      params.debug = 1; break;

    case '?':                       /* Unknown or missing parameter */
      goto error;
    default:
      msgerr("unexpected getopt return value (%d)\n", val);
      goto error;
    }
    if (val == -1) break;
  }
  i = optind;

  if (!dsk_has_disk()) {
    msgerr("no disk loaded\n");
    goto error;
  }
  if (tracks = dsk_get_tracks(), tracks <= 0) {
    msgerr("disk has no track\n");
    goto error;
  }
  params.track = dsk_trk_get_current();

  if (i < argc) {
    int t;
    errno = 0;
    t = strtol(argv[i++],0,0);
    if (errno) {
      msgerr("invalid track number \"%s\"\n",argv[--i]);
      goto error;
    }
    params.track = t;
  }

  if (!params.track)
    params.track = dsk_trk_get_default();
  if (params.track < 0 || params.track > tracks) {
    msgerr("track number #%d out of range [1..%d]\n", params.track, tracks);
    goto error;
  }

  if (i < argc)
    msgwrn("%d extra parameters ignored\n", argc-i);

  ret =  dsk_play(&params);

error:
  return ret;
}

cmd_t cmd_play = {
  /* run */ run_play,
  /* com */ "play",
  /* alt */ 0,
  /* use */ "[opts] [track]",
  /* des */ "play a track",
  /* hlp */
  "The `play' command plays a track.\n"
  "\n"
  "OPTIONS\n"
  /* *****************   ********************************************** */
  "  -l --loop=N         Number of loop (or inf or def)\n"
  "  -s --seek=POS       Seek to this position.\n"
  "  -t --to=POS         End position.\n"
  "  -f --fg             Foreground play.\n"
  "  -m --memory=N       68k memory size of 2^N.\n"
  "  -d --debug          Debug via gdb.\n"
  "\n"
  "POS := [+|-]ms | [+|-]mm:ss[,ms]\n"
  "  '+' is relative to start position.\n"
  "  '-' is relative to end position.\n"
  "  no prefix is absolute time posiiton.\n"
  "  The value can be express by either a single integer in milliseconds\n"
  "  or by a mm:ss,ms form where the \",ms\" part is optionnal."
};

static
int run_stop(cmd_t * cmd, int argc, char ** argv)
{
  if (argc > 1) {
    int i;
    for (i=1; i<argc; ++i)
      if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")) {
        help(argv[0]);
        return 0;
      }
    msgwrn("%d extra parameters ignored\n", argc-i);
  }
  return
    dsk_stop();
}

cmd_t cmd_stop = {
  /* run */ run_stop,
  /* com */ "stop",
  /* alt */ 0,
  /* use */ 0,
  /* des */ "stop a background play",
  /* hlp */
  "The `stop' command stops a background music started by the `play' command."
};
