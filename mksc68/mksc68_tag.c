/*
 * @file    mksc68_tag.c
 * @brief   metatag definitions
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

#include "mksc68_tag.h"
#include "mksc68_msg.h"
#include "mksc68_dsk.h"

#include <sc68/file68_tag.h>
#include <sc68/file68.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static struct tag_std {
  const char * var;                     /* tag name           */
  int alias;                            /* is alias           */
  const char * des;                     /* tag description    */
} stdtags[] = {
  { 0, 0, 0 },                              /* ???  */
  { TAG68_TITLE      ,0, "a name for is track or album" },
  { TAG68_ARTIST     ,0, "who wrote is track or album" },
  { TAG68_GENRE      ,0, "kind of music (track only, auto)" },
  { TAG68_FORMAT     ,0, "file format (album only, auto)"   },
  { TAG68_AKA        ,0, "scene alias for the artist" },
  { TAG68_COMMENT    ,0, "tell me something special about it" },
  { TAG68_COPYRIGHT  ,0, "copyright owner" },
  { TAG68_IMAGE      ,0, "URI of an illustration for this track/album" },
  { TAG68_RATE       ,0, "replay rate" },
  { TAG68_REPLAY     ,0, "replay routine been used by this track" },
  { TAG68_RIPPER     ,0, "who rips the music from its original environment" },
  { TAG68_YEAR       ,0, "year (yyyy)" },
  { TAG68_COMPOSER   ,0, "original composer or author if this is a remake" },
  { TAG68_CONVERTER  ,0, "who converts the track to this format" },
  { TAG68_LENGTH     ,0, "duration in milliseconds" },
  { TAG68_FRAMES     ,0, "duration in frames" },
  { TAG68_HASH       ,0, "file content hash code" },
  { TAG68_URI        ,0, "URI or path for this file" },
  { TAG68_HARDWARE   ,0, "hardware used" },

  /* Aliases */
  { "author"         ,1, TAG68_ARTIST },
  { "album"          ,1, TAG68_TITLE  },
  { "duration"       ,1, TAG68_LENGTH },

};

static
const int max = sizeof(stdtags) / sizeof(*stdtags);

static void setup_stdtags(void)
{
  int i,j;

  for (i=1; i<max; ++i) {
    if (!stdtags[i].des)
      stdtags[i].des = "no description";
    if (!stdtags[i].alias)
      continue;
      for (j=1; j<max && strcmp(stdtags[i].des,stdtags[j].var); ++j)
        ;
      if (j == max) {
        msgerr("unable to setup %s alias for %s\n",
               stdtags[i].des, stdtags[i].var);
        j = 0;
      }
      stdtags[i].alias = j;
  }
}

int tag_std(int i, const char ** var, const char ** des)
{
  const int max = sizeof(stdtags)/sizeof(*stdtags);
  static int setup = 0;

  if (!setup) {
    setup = 1;
    setup_stdtags();
  }

  if (i<1 || i>=max)
    return -1;

  if (var) *var = stdtags[i].var;
  if (des) *des = stdtags[i].des;

  return stdtags[i].alias;
}

int tag_max(void)
{
  return TAG68_ID_MAX;
}

int tag_enum(int trk, int idx, const char ** var, const char ** val)
{
  return file68_tag_enum(dsk_get_disk(), trk, idx, var, val);
}

const char * tag_get(int trk, const char * var)
{
  return file68_tag_get(dsk_get_disk(), trk, var);
}

const char * tag_set(int trk, const char * var, const char * val)
{
  return file68_tag_set(dsk_get_disk(), trk, var, val);
}

void tag_del(int trk, const char * var)
{
  tag_set(trk, var, 0);
}

void tag_clr(int trk, const char * var)
{
  tag_set(trk, var, "");
}

static void tag_do_trk(int trk, const char * newval)
{
  const int max = TAG68_ID_MAX;
  void * d = dsk_get_disk();
  int i;
  for (i=0; i<max; ++i) {
    const char * key, * val;
    file68_tag_enum(d, trk, i, &key, &val);
    file68_tag_set (d, trk, key, newval);
  }
}

void tag_del_trk(int trk)
{
  tag_do_trk(trk, 0);
}

void tag_clr_trk(int trk)
{
  tag_do_trk(trk, "");
}

static void tag_do_all(const char * val)
{
  int t;
  for (t = dsk_get_tracks(); t >= 0; --t)
    tag_do_trk(t,val);
}

void tag_clr_all(void)
{
  tag_do_all("");
}

void tag_del_all(void)
{
  tag_do_all(0);
}
