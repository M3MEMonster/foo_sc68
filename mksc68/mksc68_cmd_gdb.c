/*
 * @file    mksc68_cmd_gdb.c
 * @brief   the "gdb" command.
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

#include "mksc68_msg.h"
#include "mksc68_cmd.h"
#include "mksc68_opt.h"
#include "mksc68_gdb.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

static const opt_t longopts[] = {
  { "help",     0, 0, 'h' },            /* help             */
  { 0,0,0,0 }
};

extern int dsk_playing(void);

static
int run_gdb(cmd_t * cmd, int argc, char ** argv)
{
  char shortopts[(sizeof(longopts)/sizeof(*longopts))*3];
  int ret = -1, i;
  /* const char * str; */

  opt_create_short(shortopts, longopts);

  while (1) {
    int longindex;
    int val =
      getopt_long(argc, argv, shortopts, longopts, &longindex);

    switch (val) {
    case  -1: break;                    /* Scan finish */

    case 'h':                           /* --help */
      help(argv[0]); return 0;

    case '?':                       /* Unknown or missing parameter */
      goto error;
    default:
      msgerr("unexpected getopt return value (%d)\n", val);
      goto error;
    }
    if (val == -1) break;
  }
  i = optind;

  switch (argc-i) {
  case 0: {
    unsigned char addr[4];
    int port;
    gdb_get_conf(addr, &port);
    msginf("gdb server will listen to %d.%d.%d.%d:%i\n"
           "  *** Run gdb for m68k target\n"
           "  *** To connect gdb to mksc68 type the command:\n"
           "      > target remote <server>:<port>\n"
           "%s",
           addr[0],addr[1],addr[2],addr[3], port,
           dsk_playing() & 2 ? "  *** gdb session in progress\n" : "");
    ret = 0;
  } break;
  default:
    msgwrn("%d extra parameters ignored\n", argc-i-1);
  case 1:
    ret = gdb_conf(argv[i]);
    break;
  }

error:
  return ret;
}

cmd_t cmd_gdb = {
  /* run */ run_gdb,
  /* com */ "gdb",
  /* alt */ 0,
  /* use */ "[opts] [host[:port]]",
  /* des */ "get/set gdb server parameters",
  /* hlp */
  "The `gdb' command setup gdb server ipv4/tcp bind address."
};
