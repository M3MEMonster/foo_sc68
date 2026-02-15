/*
 * @file    mksc68_cli.c
 * @brief   command line functions.
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

#include "mksc68_cli.h"
#include "mksc68_msg.h"
#include "mksc68_cmd.h"

#include <sc68/file68_str.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_READLINE_READLINE_H
# include <readline/readline.h>
# include <readline/history.h>
#endif

static char * wo_readline (const char * prompt)
{
  char tmp[1024], *s;

  if (prompt) {
    msg_lock();
    fputs(prompt, stdout);
    fflush(stdout);
    msg_unlock();
  }
  errno = 0;
  s = fgets(tmp, sizeof(tmp)-1, stdin);

  if (s) {
    s[sizeof(tmp)-1] = 0;
    s = strdup(s);
  } else if (errno) {
    msgerr("readline: %s\n", strerror(errno));
  }
  return s;
}

#ifndef HAVE_READLINE_READLINE_H

/* A very simple replacement functions. */

static void initialize_readline(void) {}
static void add_history(const char *s) {}

static char * readline (const char * prompt)
{
  return wo_readline(prompt);
}

#else /* readline: the real deal */

// static int idx; /* completiom index */
static cmd_t * cmd;
static int alt = 0;

static char * command_generator(const char *text, int state)
{
  static int len;

  if (!state) {
    cmd = cmd_lst();
    alt = 0;
    len = strlen (text);
  }

  while (cmd) {
    cmd_t * c = cmd;
    cmd = cmd->nxt;
    if (!cmd && !alt) {
      cmd = cmd_lst();
      alt = 1;
    }
    if (!alt && ! strncmp68(c->com, text, len))
      return strdup68(c->com);
    else if (alt && ! strncmp68(c->alt, text, len))
      return strdup68(c->alt);
  }
  return 0;

}

static char ** mksc68_completion(const char * text, int start, int end)
{
  char ** matches = 0;

  rl_sort_completion_matches = 1;

  if (!start) {
    matches = rl_completion_matches (text, command_generator);
  }

  rl_filename_completion_desired = 1;
  rl_filename_quoting_desired    = 1;

  return matches;
}


static void initialize_readline(void)
{
  rl_readline_name = "mksc68"; /* allow specific parsing of .inputrc  */
  rl_attempted_completion_function = mksc68_completion;
  rl_basic_quote_characters      = "";
  rl_basic_word_break_characters = " ";    /* break between words */
  rl_completer_quote_characters  = "\"";   /* char used to quote  */
  rl_filename_quote_characters   = " ";    /* what needs to be quoted */
}

#endif

static char * cli, * prompt;


static char * killspace(char *s)
{
  while (isspace((int)*s)) s++;
  return s;
}

/* Get word start & end. Could be inside quote.
 * @retval next word
 */
static
char * word(char * word, char ** wordstart)
{
  char * start = word;
  int c, esc = 0, len = 0;

  word = killspace(word);
  if (!*word) {
    start = 0;
  } else {
    for ( ;; ) {
      c = *word++;
      if (!c) {
        --word;
        break;
      } else if (esc == '\\') {
/* backslashed */
        esc = 0;
        start[len++] = c;
      } else if (esc == '"') {
/* quoted string */
        if (c != '"')
          start[len++] = c;
        else
          esc = 0;
      } else if (c == '\\' || c == '"') {
        esc = c;
      } else if (!isspace(c)) {
        start[len++] = c;
      } else {
        break;
      }
    }
  }

  *wordstart = start;
  if (start) start[len] = 0;
  return word;
}

static
int dispatch_word(char ** here, int max, char *str)
{
  int i;

  for (i=0 ;; ++i) {
    char * start;
    str = word(str, &start);
    if (!start) break;
    assert (i < 512);
    if (i < max) {
      here[i] = start;
    } else {
      msgwrn("skipping arg[%2d] (%s)\n", i, start);
    }
  }
  return i;
}

void cli_release()
{
  free(prompt); prompt = 0;
  free(cli);    cli = 0;
}

char * cli_prompt(const char * fmt, ...)
{
  char * newprompt = 0;

  free(prompt);
  if (fmt) {
    va_list list;
    va_start(list,fmt);
#ifdef HAVE_VASPRINTF
    if (vasprintf(&newprompt,fmt,list) < 0)
      newprompt = 0;
#else
    {
      char tmp[256];
# ifdef HAVE_VSNPRINTF
      vsnprintf(tmp,sizeof(tmp),fmt,list);
# else
      vsprintf(tmp,fmt,list);
# endif
      newprompt = strdup(tmp);
    }
#endif
    va_end(list);
    if (!newprompt)
      msgerr("set prompt: %s\n", strerror(errno));
  }
  return prompt = newprompt;
}


/* defined in mksc68.c */
extern int no_readline, is_interactive;

int cli_read(char * argv[], int max)
{
  char * strip;

  initialize_readline();

  free(cli);

  if (is_interactive && !no_readline)
    cli = readline(prompt);
  else
    cli = wo_readline(is_interactive ? prompt : 0);
  if (!cli) {
    return -1;
  }

  if (is_interactive) {
    strip = killspace(cli);
    if (*strip) {
      add_history(strip);
    }
  }

  return
    dispatch_word(argv, max, cli);
}
