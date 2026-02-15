/*
 * @file    mksc68_cmd_msg.c
 * @brief   the "debug" command
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
# include "config.h"
#endif

#include "mksc68_cmd.h"
#include "mksc68_msg.h"
#include "mksc68_opt.h"

#include <sc68/file68_msg.h>
#include <sc68/file68_str.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static const opt_t longopts[] = {
  { "help",       0, 0, 'h' },
  {0,0,0,0}
};

/* Callback for message category printing. */
static void
print_cat(void * data,
          const int bit, const char * name, const char * desc)
{
  const char * fmt = "%02d | %-10s | %-40s | %-3s\n";
  const int mask = (msg68_cat_filter(0,0) >> bit) & 1;
  fprintf(data,fmt, bit, name, desc, mask?"ON":"OFF");
}

/* Print message category. */
static int print_cats(void)
{
  printf("message category: current mask is %08X\n",msg68_cat_filter(0,0));
  msg68_cat_help(stdout,print_cat);
  return 0;
}

static
int run_msg(cmd_t * cmd, int argc, char ** argv)
{
  char shortopts[(sizeof(longopts)/sizeof(*longopts))*3];
  int ret = -1, i;

  opt_create_short(shortopts, longopts);


  while (1) {
    int longindex;
    int val =
      getopt_long(argc, argv, shortopts, longopts, &longindex);

    switch (val) {
    case  -1: break;                /* Scan finish */
    case 'h':                       /* --help */
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

  if (i == argc) {
    print_cats();
    return 0;
  }

  for (; i<argc; ++i) {
    const char * delim = "+-~=";
    char * arg = argv[i];
    int op, bits = 0;

    op = strchr(delim, *arg) ? *arg++ : '+';
    for (arg = strtok(arg,","); arg; arg = strtok(0, ",")) {
      if (!strcmp68(arg,"all"))
        bits = -1;
      else if (!strcmp68(arg,"none"))
        bits = 0;
      else if (arg[0] == '#' && isdigit((int)arg[1]))
        bits |= 1 << strtol(arg+1,0,0);
      else if (isdigit((int)arg[0]))
        bits |= strtol(arg,0,0);
      else if (*arg) {
        int bit = msg68_cat_bit(arg);
        if (bit < 0) {
          msgwrn("unknown category \"%s\". Try msg (without parameter).\n",arg);
        } else {
          bits |= 1 << bit;
        }
      }
    }

    switch (op) {
    case '=':
      bits = msg68_cat_filter(~0, bits); break;
    case '-': case '~':
      bits = msg68_cat_filter(bits, 0); break;
    case '+':
      bits = msg68_cat_filter(0, bits); break;
    default:
      msgwrn("invalid operator '%c'\n",op);
    }
  }

  ret = 0;
error:
  return ret;
}

cmd_t cmd_msg = {
  /* run */ run_msg,
  /* com */ "message",
  /* alt */ "msg",
  /* use */ "[cats ...]",
  /* des */ "change message output level",
  /* hlp */
  "The `message' command control message verbosity.\n"
  "Multiple expressions are evaluated sequencially in given order.\n"
  "If no argument is given the command display the list of existing\n"
  "debug categories and their current status.\n"
  "\n"
  "CATS := a `,' (comma) separated list of categories (CAT) optionally\n"
  "         prefixed by an operator (OP).\n"
  "CAT  := a category-name OR a number OR a bit-number (prefixed by `#')\n"
  "OP   := `=' (equal) | `+' (plus) | `~' (tilde) | `-' (minus) | `,' (coma)\n"
  "  `=' set the specified categories (SET)\n"
  "  `+' enable the specified categories (OR) (default)\n"
  "  `~' disable the specified categories (NAND)\n"
  "  `-' as `~' (might confuse the options parser)\n"
  "  `,' set ONLY the specified categories and clear others (SET)\n"
};
