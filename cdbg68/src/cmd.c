/*
 * @file    cmd,c
 * @brief   sc68 curses debugger - commands
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

#include <string.h>
#include <stdio.h>

#include "cmd.h"
#include "msg.h"

#define CDBG68_CAT_MAX 8


/* Array of command category. */
static cmdcat_t * cats[CDBG68_CAT_MAX];

/* Register a category of command. */
int cmd_register(cmdcat_t * cat)
{
  const int max = CDBG68_CAT_MAX;
  if (cat->i < 0 || cat->i >= max)
    return -1;
  cats[cat->i] = cat;
  return 0;
}

void cmd_unregister(int i)
{
  const int max = CDBG68_CAT_MAX;
  if (i >= 0 && i < max)
    cats[i] = 0;
}


/* Last error message. */
static const char * cmd_error = 0;

/* Get and reset last error message. */
const char * cmd_geterr()
{
  const char * err = cmd_error;
  cmd_error = 0;
  return err;
}

/* Set last error message. */
int cmd_seterr( const char * cat, const char * desc, int override)
{
  static char buf[1024];
  if (override || !cmd_error) {
    snprintf(buf,sizeof(buf),"%s: %s", cat, desc);
    buf[sizeof(buf)-1] = 0;
    cmd_error = buf;
  }
  error("%s: %s\n", cat, desc);
  return -1;
}

static
cmd_t * cmd_search_in(cmd_t * cmds, const int max, const char * name)
{
  int i;
  cmd_t * cmd = 0;

  for (i=0; i<max; ++i) {
    cmd = cmds+i;
    if (cmd && !strcmp(cmd->name, name))
      break;
    cmd = 0;
  }
  return cmd;
}

static
cmd_t * cmd_lookup_in(cmd_t * cmds, const int max, int mod, int key)
{
  int i;
  cmd_t * cmd = 0;

  for (i=0; i<max; ++i) {
    cmd = cmds+i;
    if (cmd->mod == mod && cmd->key == key)
      break;
    cmd = 0;
  }
  return cmd;
}

cmd_t * cmd_search(int cat, const char * name)
{
  cmd_t * cmd = 0;
  if (cat >= 0 && cat < CDBG68_CAT_MAX) {
    if (cats[cat])
      cmd = cmd_search_in(cats[cat]->cmds, cats[cat]->max, name);
  } else if (cat == -1) {
    for (cat=0; cat<CDBG68_CAT_MAX; ++cat)
      if (cats[cat] && (cmd = cmd_search_in(cats[cat]->cmds, cats[cat]->max, name)))
        break;
  }
  return cmd;
}

cmd_t * cmd_lookup(int cat, int mod, int key)
{
  cmd_t * cmd = 0;
  if (cat >= 0 && cat < CDBG68_CAT_MAX) {
    if (cats[cat])
      cmd = cmd_lookup_in(cats[cat]->cmds, cats[cat]->max, mod, key);
  } else if (cat == -1) {
    for (cat=0; cat<CDBG68_CAT_MAX; ++cat)
      if (cats[cat] && (cmd = cmd_lookup_in(cats[cat]->cmds, cats[cat]->max, mod, key)))
        break;
  }
  return cmd;
}

int cmd_run(cmd_t * cmd, int argc, char ** argv)
{
  char * freeme = 0;
  int err = 0;

  cmd_error = 0;

  if (!cmd)
    err = cmd_seterr( "internal", "null command pointer", 1);
  else if (argc == -1) {
    debug("running '%s' in interractive mode\n", cmd->name);
    argc = 0;
    argv = 0;
    if (!cmd->args)
      err = cmd_seterr(cmd->name, "null interactive argument function", 1);
    else if (cmd->args(&argc, &argv))
      err = cmd_seterr(cmd->name, "failed to get interactive argument", 0);
    if (!err) {
      debug(" -> got %d args (%s)\n", argc, argv[0]);
      /* freeme  = argv[0]; */
      /* argv[0] = cmd->name; */
    }
  }
  if (!err) {
    if (!cmd->func)
      err = cmd_seterr(cmd->name, "null execution function", 1);
    else if (cmd->func(argc, argv))
      err = cmd_seterr(cmd->name, "execution failed", 0);
  }
  free(freeme);
  return err;
}
