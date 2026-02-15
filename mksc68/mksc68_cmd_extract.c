/*
 * @file    mksc68_cmd_extract.c
 * @brief   the "extract" command
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

#include <sc68/file68.h>
#include <sc68/file68_uri.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const opt_t longopts[] = {
  { "help",       0, 0, 'h' },
  { "single",     0, 0, 's' },
  { "binary",     0, 0, 'b' },
  { "asm",        0, 0, 'a' },
  { "ascii",      0, 0, 'A' },
  { 0,0,0,0 }
};

enum {
  EXIT_OK = 0,
  EXIT_GENERIC,
  EXIT_TRACK,
  EXIT_VFS,
};

static inline int isq(const int c) {
  return c >= 32 && c <= 126;
}

static int extract(const char * uri, int track, int fmt, const void ** ptr_data)
{
  int ret = EXIT_TRACK;
  char tmp[8];
  disk68_t * const d = dsk_get_disk();
  const char * data;
  int size, i;
  vfs68_t * vfs = 0;

  msgdbg("extract track #%d into (%c) '%s'\n",
         track, fmt, uri);

  if (!d) {
    msgerr("no disk loaded\n");
    goto error;
  }

  if (track < 1 || track > d->nb_mus) {
    msgerr("track number #%d out of range [1..%d]\n", track, d->nb_mus);
    goto error;
  }

  data = d->mus[track-1].data;
  size = d->mus[track-1].datasz;

  /* ptr_data is used for --single mode to carry previous saved data */
  if (ptr_data) {
    if (*ptr_data == data) {
      msgdbg("did not extract track #%d (duplicated data)\n", track);
      return EXIT_OK;
    } else
      *ptr_data = data;
  }

  ret = EXIT_VFS;
  vfs = uri68_vfs(uri, 2, 0);
  if (!vfs || vfs68_open(vfs) < 0)
    goto error;

  switch (fmt) {
  case 'a': case 'A':
    /* ASM output */
    for (i=0; i<size; ++i) {
      const int m = 15;
      int j = i & m, c = data[i] & 255;

      if (vfs68_puts(vfs, j ? "," : "\tdc.b ") < 0)
        goto error;

      if (fmt == 'a' || !isq(c))
        sprintf(tmp, "$%02x", c);
      else if (c == '\'')
        strcpy(tmp,"\"'\"");
      else
        sprintf(tmp,"'%c'",c);
      if (vfs68_puts(vfs, tmp) < 0)
        goto error;
      if ( (i+1 == size || j == m) && vfs68_putc(vfs,'\n') < 0)
        goto error;
    }
    break;

  case 'b':
    if (vfs68_write(vfs, data, size) != size)
      goto error;
    break;
  }

  size = vfs68_tell(vfs);
  msginf("saved track #%d into \"%s\" (%d bytes)\n", track, uri, size);

  ret = EXIT_OK;
error:
  vfs68_destroy(vfs);
  return ret;
}

static
int run_extract(cmd_t * cmd, int argc, char ** argv)
{
  int ret = EXIT_GENERIC;

  char shortopts[(sizeof(longopts)/sizeof(*longopts))*3];
  int format = 'b', single = 0, i;
  char * filename = 0, * filebase, * ext;
  const char * tracklist;

  opt_create_short(shortopts, longopts);

  while (1) {
    int longindex;
    int val =
      getopt_long(argc, argv, shortopts, longopts, &longindex);

    switch (val) {
    case  -1: break;                /* Scan finish */
    case 'h':                       /* --help      */
      help(argv[0]); return 0;
    case 's': single = 1; break;    /* --single    */
    case 'A':                       /* --ascii     */
    case 'a':                       /* --asm       */
    case 'b':                       /* --binary    */
      format = val; break;
    case '?':                       /* Unknown or missing parameter */
      goto error;
    default:
      msgerr("unexpected getopt return value (%d)\n", val);
      goto error;
    }
    if (val == -1) break;
  }
  i = optind;


  if (i == argc) {
    msgerr("missing <URI> parameter. Try --help.\n");
    goto error;
  }
  filebase = argv[i++];
  ext = ( format == 'b' ) ? ".mus" : ".s";
  if (filebase[0] == '-' && !filebase[1])
    filebase = "stdout:";

  filename = malloc(strlen(filebase) + 3 + 4 + 1);
  if (!filename) {
    msgerr("%s\\n",strerror(errno));
    goto error;
  }

  if (i == argc) {
    sprintf(filename, "%s-%02d%s", filebase, dsk_trk_get_current(), ext);
    ret = extract(filename, dsk_trk_get_current(), format, 0);
  } else {
    const void * data = 0;
    int a, b, e;

    tracklist = argv[i++];
    if (i < argc)
      msgwrn("%d extra parameters ignored\n", argc-i);

    while (e = str_tracklist(&tracklist, &a, &b), e > 0) {
      for (; a <= b; ++a) {
        sprintf(filename, "%s-%02d%s", filebase, a, ext);
        ret = extract(filename, a, format, single ? &data : 0);

        if (ret)
          goto error;
      }
    }
    if (e < 0)
      goto error;
  }


error:
  free(filename);
  return ret;
}

cmd_t cmd_extract = {
  /* run */ run_extract,
  /* com */ "extract",
  /* alt */ "xt",
  /* use */ "<URI> [TRACKS]",
  /* des */ "extract track raw data",
  /* hlp */
  "The `extract' command extracts track data.\n"
  "\n"
  "OPTIONS\n"
  /* *****************   ********************************************** */
  "  -s --single         Save identical data only once.\n"
  "  -b --binary         Output binary (.mus) files (default).\n"
  "  -a --asm            Output 68k assembler (.s) compatible data.\n"
  "  -A --ascii          Output 68k assembler (.s) with ASCII.\n"
};
