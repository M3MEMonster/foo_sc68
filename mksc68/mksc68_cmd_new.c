/*
 * @file    mksc68_cmd_new.c
 * @brief   the "new" command
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
#include "mksc68_msg.h"
#include "mksc68_opt.h"
#include "mksc68_dsk.h"

static const opt_t longopts[] = {
  { "help",       0, 0, 'h' },
  { "force",      0, 0, 'f' },
  {0,0,0,0}
};


static
int run_new(cmd_t * cmd, int argc, char ** argv)
{
  char shortopts[(sizeof(longopts)/sizeof(*longopts))*3];
  int ret = -1, force = 0, i;
  char * diskname = 0;

  opt_create_short(shortopts, longopts);

  while (1) {
    int longindex;
    int val =
      getopt_long(argc, argv, shortopts, longopts, &longindex);

    switch (val) {
    case  -1: break;                /* Scan finish */
    case 'h':                       /* --help */
      help(argv[0]); return 0;
    case 'f': force = 1; break;     /* --force     */
    case '?':                       /* Unknown or missing parameter */
      goto error;
    default:
      msgerr("unexpected getopt return value (%d)\n", val);
      goto error;
    }
    if (val == -1) break;
  }
  i = optind;

  if (i < argc)
    diskname = argv[i++];

  if (i < argc)
    msgwrn("%d extra parameters ignored\n", argc-i);

  ret = dsk_new(diskname, force);

error:
  return ret;
}

cmd_t cmd_new = {
  /* run */ run_new,
  /* com */ "new",
  /* alt */ "n",
  /* use */ "[opts] [name]",
  /* des */ "create a new disk",
  /* hlp */
  "The `new' creates a brand new disk.\n"
  "\n"
  "OPTIONS\n"
  /* *****************   ********************************************** */
  "  -f --force          Force a new disk creation."
};
