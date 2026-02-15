/*
 * @file    mksc68_opt.c
 * @brief   cli option functions
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

#include "mksc68_opt.h"

void opt_create_short(char * shortopts, const opt_t * longopts)
{
  int i, j;
  for (i=j=0; longopts[i].val; ++i) {
    shortopts[j++] = longopts[i].val;
    switch (longopts[i].has_arg) {
    case 2: shortopts[j++] = ':';
    case 1: shortopts[j++] = ':'; break;
    }
  }
  shortopts[j++] = 0;
  optind = 0;
}

int opt_get(int argc, char * const argv[], const char *optstring,
            const opt_t * longopts, int * longindex)
{
  return
    getopt_long(argc, argv, optstring, longopts, longindex);
}
