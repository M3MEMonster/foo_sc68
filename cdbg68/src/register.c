/*
 * @file    register,c
 * @brief   sc68 curses debugger - register
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

#include <curses.h>
#include "register.h"

#include "emu68/srdef68.h"
#include "emu68/emu68.h"

typedef struct {
  int val;                              /* current value         */
  int chg;                              /* bits that had changed */
} reg_t;

#define REG_MAX 27
typedef union {
  reg_t reg[REG_MAX];
  struct {
    reg_t dn[8];
    reg_t an[8];
    reg_t in[8];
    reg_t pc;
    reg_t usp;
    reg_t sr;
  } regs;
} regs_t;

WINDOW * regwin;
static regs_t cgdb68_regs;

int reg_init(void)
{
  return 0;
}

void reg_destroy(void)
{
  if (regwin) {
    delwin(regwin);
    regwin = 0;
  }
}

void reg_draw(void)
{
  WINDOW * const win  = regwin;
  regs_t * const regs = &cgdb68_regs;
  int i,j;

  for ( i=0; i<REG_MAX; ++i) {
    regs->reg[i].chg = 0;
  }
  wclear(win);

  /* Print register labels */
  wattrset (win, A_UNDERLINE|COLOR_PAIR(COLOR_GREEN));
  mvwaddch (win, 0,  0, 'D');
  mvwaddch (win, 1,  0, 'A');
  mvwaddstr(win, 3,  0, "PC");
  mvwaddstr(win, 3, 14, "SR");
  mvwaddstr(win, 3, 69, "USP");

  for ( j=0; j<2; ++j ) {
    for ( i = 0; i<8; ++i ) {
      mvwaddch(win, j, i*10+1, '0'+i);
    }
  }

  wattrset(win,A_NORMAL|COLOR_PAIR(COLOR_WHITE));
}

static const char thex[16] = {
  '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

static const char srsymb[16] = {
  'C','V','Z','N','X','?','?','?','?','?','?','I','I','I','S','T'
};

void reg_refresh(void)
{
  WINDOW * const win = regwin;
  regs_t * const regs = &cgdb68_regs;
  int k,j,i;

  const int attr_regchg = A_BOLD   | COLOR_PAIR(COLOR_WHITE);
  const int attr_regnrm = A_NORMAL | COLOR_PAIR(COLOR_WHITE);

  /* Dn, An and (An) */
  for ( j=0; j<3; ++j ) {
    for ( i=0; i<8; ++i ) {
      const int rn = j*8+i;
      const int v = regs->reg[rn].val;

      wmove(win, j, i*10+2);
      for ( k=28; k>=0; k-=4 ) {
        const int w = ( v >> k ) & 15;
        const int m = ( regs->reg[rn].chg >> k ) & 15;
        wattrset(win,  m ? attr_regchg: attr_regnrm);
        waddch(win,thex[ w ]);
      }
    }
  }

  /* PC */
  wmove(win, j, 2);
  for ( k=28; k>=0; k-=4 ) {
    const int w = ( regs->regs.pc.val >> k ) & 15;
    const int m = ( regs->regs.pc.chg >> k ) & 15;
    wattrset(win,  m ? attr_regchg: attr_regnrm);
    waddch(win,thex[ w ]);
  }

  /* SR */
  wmove(win, j, 2*10-4);
  for ( k=12; k>=0; k-=4 ) {
    const int w = ( regs->regs.pc.val >> k ) & 15;
    const int m = ( regs->regs.pc.chg >> k ) & 15;
    wattrset(win,  m ? attr_regchg: attr_regnrm);
    waddch(win,thex[ w ]);
  }
  waddch(win, ' ');
  waddch(win, ' ');

  for ( i=15; i>13; --i ) {
    const int bit = 1 << i;
    int c = ( regs->regs.sr.val & bit ) ? srsymb[i] : '.';
    wattrset(win, ( regs->regs.sr.chg & bit ) ? attr_regchg: attr_regnrm);
    waddch(win, c);
  }
  i = ( regs->regs.sr.val >> SR_I_BIT ) & 7;
  wattrset(win, ( regs->regs.sr.chg & SR_I ) ? attr_regchg: attr_regnrm);
  waddch(win, i+'0');
  for ( i=4; i>=0  ;  --i ) {
    const int bit = 1 << i;
    int c = ( regs->regs.sr.val & bit ) ? i["CVZNX"] : '.';
    wattrset(win, ( regs->regs.sr.chg & bit ) ? attr_regchg: attr_regnrm);
    waddch(win, c);
  }

  /* USP */
  wmove(win, j, 7*10+2);
  for ( k=28; k>=0; k-=4 ) {
    const int w = ( regs->regs.usp.val >> k ) & 15;
    const int m = ( regs->regs.usp.chg >> k ) & 15;
    wattrset(win,  m ? attr_regchg: attr_regnrm);
    waddch(win,thex[ w ]);
  }

  wrefresh(win);
}

extern emu68_t * emu68;

static void update_dn(reg_t * reg, int v)
{
  int o = reg->val;
  int c = o ^ v;
  reg->val  = v;
  reg->chg  = -!!( c & 0xFFFF0000 ) & 0xFFFFFFFF;
  reg->chg |= -!!( c & 0x0000FF00 ) & 0x0000FFFF;
  reg->chg |= -!!( c & 0x000000FF ) & 0x000000FF;
}

static void update_an(reg_t * reg, int v)
{
  int o = reg->val;
  reg->val  = v;
  reg->chg  = -!!( o ^ v ) & 0xFFFFFFFF;
}

static void update_in(reg_t * reg, int v)
{
  int o = reg->val;
  int c = o ^ v;
  unsigned int m;
  reg->val  = v;

  reg->chg  = 0;
  for ( m = 0xF0000000; m; m >>= 4 )
    reg->chg |= -!!( c & m ) & m;
}

static int lpeek(int addr)
{
  return 0
    | ( emu68_peek(emu68, addr+0) << 24 )
    | ( emu68_peek(emu68, addr+1) << 16 )
    | ( emu68_peek(emu68, addr+2) <<  8 )
    | ( emu68_peek(emu68, addr+3) <<  9 )
    ;
}

void reg_set(void)
{
  regs_t * const regs = &cgdb68_regs;
  int i;

  /* Dn, An and (An) */
  for ( i=0; i<8; ++i ) {
    update_dn( regs->regs.dn+i , emu68->reg.d[i] );
    update_an( regs->regs.an+i , emu68->reg.a[i] );
    update_in( regs->regs.in+i , lpeek(emu68->reg.a[i]));
  }

  /* PC & USP */
  update_an( &regs->regs.pc,  emu68->reg.pc  );
  update_an( &regs->regs.usp, emu68->reg.usp );

  /* SR */
  regs->regs.sr.chg = ( regs->regs.sr.val ^ emu68->reg.sr ) & 0xFFFF;
  regs->regs.sr.val = emu68->reg.sr;

  reg_refresh();
}

int reg_key(int key)
{
  return key;
}
