/*
 * @file    mksc68_cmd.c
 * @brief   command factory and exec
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

#include <assert.h>
#include <string.h>

static cmd_t * head;

cmd_t * cmd_lst(void) {
  return head;
}

int cmd_add(cmd_t * cmd)
{
  assert(cmd);
  assert(cmd->com);
  assert(cmd->des);

  if (!cmd) {
    return -1;
  }
  if (cmd->nxt) {
    msgerr("command '%s' already in used\n", cmd->com);
    return -1;
  }
  if (!cmd->hlp) cmd->hlp = cmd->des;
  cmd->nxt = head;
  head = cmd;
  msgdbg("command '%s' added\n", cmd->com);
  return 0;
}

static char * running_cmd;

char * cmd_cur(void)
{
  return running_cmd ? running_cmd : PACKAGE_NAME;
}

int cmd_run(int argc, char ** argv)
{
  cmd_t * cmd;
  int err = 0;

  if (argc >= 1) {
    for (cmd=head;
         cmd && strcmp(argv[0],cmd->com) &&
           (!cmd->alt || strcmp(argv[0],cmd->alt));
         cmd=cmd->nxt)
      ;
    if (!cmd) {
      running_cmd = 0;
      msgerr("%s: command not found\n", argv[0]);
      err = -1;
    } else {
      running_cmd = cmd->com;
      msgdbg("run: command '%s' (%d arg)\n", cmd->com, argc);
      err = cmd->run(cmd, argc, argv);
      msgdbg("run: %s: returns %d\n", cmd->com, err);
      running_cmd = 0;
    }
  }

  return err;
}
