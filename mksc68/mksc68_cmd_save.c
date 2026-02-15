/*
 * @file    mksc68_cmd_save.c
 * @brief   the "save" command
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

#include "sc68/file68_str.h"
#include <ctype.h>

static const opt_t longopts[] = {
  { "help",    0, 0, 'h' },
  { "gzip",    1, 0, 'z' },
  { "format",  1, 0, 'F' },
  { 0,0,0,0 }
};


static
int run_save(cmd_t * cmd, int argc, char ** argv)
{
  char shortopts[(sizeof(longopts)/sizeof(*longopts))*3];
  int ret = -1, gzip = -1, version = 0, i;
  char * filename = 0;

  opt_create_short(shortopts, longopts);

  while (1) {
    int longindex;
    int val =
      getopt_long(argc, argv, shortopts, longopts, &longindex);

    switch (val) {
    case  -1: break;                /* Scan finish  */
    case 'h':                       /* --help */
      help(argv[0]); return 0;
    case 'z':                       /* --gzip=[0-9] */
      if (!isdigit((int)*optarg) || optarg[1]) {
        msgerr("invalid compression level \"%s\"\n", optarg);
        goto error;
      }
      gzip = *optarg - '0';
      break;
    case 'F':                           /* --format=sc68|sndh */
      if (!strcmp68(optarg,"auto")) {
        version = 0;
      } else if (!strcmp68(optarg,"sc68")) {
        version = 1;
      } else if (!strcmp68(optarg,"sndh")) {
        version = -1;
      } else {
        msgerr("unknown format -- `%s'\n", optarg);
        goto error;
      }
      break;

    case '?':                       /* Unknown or missing parameter */
      goto error;
    default:
      msgerr("unexpected getopt return value `%c'(%d)\n",
             isgraph(val)?val:'.',val);
      goto error;
    }
    if (val == -1) break;
  }
  i = optind;

  if (i < argc)
    filename = argv[i++];
  if (i < argc)
    msgwrn("%d extra parameters ignored\n", argc-i);

  ret = dsk_save(filename, version, gzip);

error:
  return ret;
}

cmd_t cmd_save = {
  /* run */ run_save,
  /* com */ "save",
  /* alt */ "sd",
  /* use */ "[opts] [<url>]",
  /* des */ "save disk",
  /* hlp */
  "The `save' command saves the disk.\n"
  "\n"
  "If the format is ommited the filename extension is used [sc68|snd|sndh].\n"
  "If it can not be determined automatically the previous format is used.\n"
  "As a last resort sc68 is used.\n"
  "\n"
  "If compression level is ommited the default for the format is used.\n"
  "sc68 defaults to uncompressed whereas sndh defaults to ICE! packed.\n"
  "\n"
  "OPTIONS\n"
  /* *****************   ********************************************** */
  "  -z --gzip=#         Set compression level [0..9].\n"
  "  -F --format=fmt     Set output format [auto*,sc68,sndh].\n"
};
