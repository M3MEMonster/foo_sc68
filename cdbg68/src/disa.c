/*
 * @file    disa,c
 * @brief   sc68 curses debugger - disassembly
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
# include <config.h>
#endif

#include "disa.h"
#include "cmd.h"
#include "msg.h"
#include "expr.h"
#include "desa68.h"
#include "emu68/emu68.h"
#include "emu68/excep68.h"

extern emu68_t * emu68;
WINDOW * diswin;
static char str[256];
static desa68_parm_t parm;
static int disa_addr;

static void setparm(desa68_parm_t * parm, emu68_t * emu68)
{
  parm->mem    = emu68->mem;            /* Base of 68K memory. */
  parm->memmsk = emu68->memmsk;         /* Size of memory - 1 (mask). */
  parm->pc     = emu68->reg.pc;
  parm->flags  = DESA68_ASCII_FLAG;     /* Disassemble options */
  parm->str    = str;                   /* Disassembly output  */
  parm->strmax = sizeof(str);
  parm->immsym_min  = -1;
  parm->immsym_max  = -1;
}

void dis_draw(void)
{
}

void dis_refresh(void)
{
  int h, y;

  setparm(&parm, emu68);
  parm.pc   = disa_addr;
  disa_addr = parm.pc & parm.memmsk;

  h = getmaxy(diswin);
  for ( y=0; y<h; ++y) {
    char addr[16];
    const int ispc = ! ( (parm.pc ^ emu68->reg.pc) & parm.memmsk );
    const int chk = emu68_chkpeek(emu68, parm.pc);
    int attr;

    switch ( (ispc << 1) + !!( chk & ~EMU68_A ) ) {

    case 0:
      attr = COLOR_PAIR(COLOR_WHITE);
      break;

    case 1:
      attr = COLOR_PAIR(COLOR_RED);
      break;

    case 2:
      attr = COLOR_PAIR(COLOR_YELLOW) | A_BOLD;
      break;

    case 3:
      attr = COLOR_PAIR(COLOR_GREEN) | A_BOLD;
      break;
    }

    wattrset(diswin,attr);
    sprintf(addr, "%06X %s%c",
            parm.pc & parm.memmsk,
            "[  ]",
            ispc[" >"]
      );
    mvwaddstr(diswin, y, 0, addr);
    desa68(&parm);
    waddstr(diswin, str);
    wclrtoeol(diswin);
  }

  wrefresh(diswin);
}

int dis_get(void)
{
  return disa_addr;
}


void dis_set(int address)
{
  if ( address == -1 )
    address = emu68->reg.pc;
  disa_addr = address;
  dis_refresh();
}

static int arg_0(int *argc, char *** argv) {
  char ** v;

  v = malloc(sizeof(char *) + 1);
  if (!v) {
    error("internal: malloc failed\n");
    return -1;
  }
  *argc   = 1;
  v[0]    = (char *)(v+1);
  v[0][0] = 0;
  *argv   = v;
  return 0;
}

static int cmd_set(int argc, char ** argv)
{
  int addr = -1;
  if (argc >= 2) {
    char * e = argv[1];
    if (expr(&addr, &e)) {
      cmd_seterr(argv[0],"syntax error in expression",1);
      return -1;
    }
  }
  dis_set(addr);
  return 0;
}

static int cmd_bak(int argc, char ** argv)
{
  if (argc < 2)
    dis_set(disa_addr-2);
  return 0;
}

static int cmd_fwd(int argc, char ** argv)
{
  if (argc < 2)
    dis_set(disa_addr+2);
  return 0;
}

static int cmd_prv(int argc, char ** argv)
{
  int n = 1, addr;

  if (argc >= 2) {
    char * e = argv[1];
    if (expr(&n, &e) || n < 0)
      return -1;
  }
  setparm(&parm, emu68);
  addr = disa_addr;
  if (!n)
    addr -= 2;
  else do {
      int i;
      addr -= 2;
      for (i=0; i<10; i+=2) {
        parm.pc = addr - i;
        desa68(&parm);
        if (parm.status & DESA68_INST) {
          addr = addr - i;
          break;
        }
      }
      if (i == 10)
        break;
    } while (--n);
  dis_set(addr);
  return 0;
}

static int cmd_nxt(int argc, char ** argv)
{
  int n, addr, start = disa_addr;

  if (argc < 2)
    n = 1;
  else if (expr(&n, &argv[1]) || n <= 0)
    return -1;
  else if (argc >= 3 && expr(&start, &argv[2]))
    return -1;
  addr = start;
  while (n-- > 0) {
    parm.pc = addr;
    desa68(&parm);
    addr = parm.pc;
  }
  dis_set(addr);
  return 0;
}

static int cmd_ppg(int argc, char ** argv)
{
  char tmp[32];
  sprintf(tmp,"%d",getmaxy(diswin)-1);
  char * args[2] = { argv[0], tmp };
  return cmd_prv(2, args);
}

static int cmd_npg(int argc, char ** argv)
{
  char tmp[32];
  sprintf(tmp,"%d",getmaxy(diswin)-1);
  char * args[2] = { argv[0], tmp };
  return cmd_nxt(2, args);
}

static int cmd_brk(int argc, char ** argv)
{
  if (argc < 2) {
    /* toggle breakpoint */
    int addr = disa_addr & emu68->memmsk;
    int bkp  = emu68_bp_find(emu68,addr);
    if (bkp == -1) {
      bkp = emu68_bp_set(emu68,bkp,addr,1,1);
      info("breakpoint %X set at $%06X\n", bkp, addr);
    } else {
      emu68_bp_del(emu68,bkp);
      info("breakpoint %X del at $%06X\n", bkp, addr);
    }
    /* int chk  = emu68->chk[addr] ^ EMU68_B; */
    /* emu68->chk[addr] = chk; */
    /* if (chk & EMU68_B) */
    /*   info("breakpoint set at $%06X\n", addr); */
    /* else */
    /*   info("breakpoint del at $%06X\n", addr); */
  }
  dis_refresh();
  return 0;
}

static int disa_cb(emu68_t* const emu68, int vector, void * cookie)
{
  switch (vector) {
  case HWBREAK_VECTOR:
    break;
  case HWTRACE_VECTOR:
    debug("hwtrace: %06x\n",emu68->reg.pc);
    break;
  }
  return 0;
}

static int cmd_stp(int argc, char ** argv)
{
  int status;
  emu68_handler_t hdl;

  int pc = emu68->reg.pc;
  hdl = emu68_set_handler(emu68, disa_cb);

  status = emu68_step(emu68);
  if (status != EMU68_NRM)
    info("step status: %s\n", emu68_status_name(status));
  debug("step: %06x -> %06x\n", pc, (int)emu68->reg.pc);
  dis_set(-1);
  reg_set();
  mem_refresh();
  return 0;
}

static cmd_t disa_cmds[] = {
  /* mod,  key,       win,    name,          desc,                                   help, func,    args */
  { K_NRM, '=',       DISWIN, "dis.set",     "disassembler to address",              0 ,   cmd_set, arg_0 },
  { K_NRM, KEY_UP,    DISWIN, "dis.prev",    "disassembler to previous instruction", 0,    cmd_prv, arg_0 },
  { K_NRM, KEY_DOWN,  DISWIN, "dis.next",    "disassembler to next instruction",     0,    cmd_nxt, arg_0 },
  { K_NRM, KEY_LEFT,  DISWIN, "dis.back",    "disassembler to previous word",        0,    cmd_bak, arg_0 },
  { K_NRM, KEY_RIGHT, DISWIN, "dis.forward", "disassembler to next word",            0,    cmd_fwd, arg_0 },
  { K_NRM, KEY_PPAGE, DISWIN, "dis.prev",    0,                                      0,    cmd_ppg, arg_0 },
  { K_NRM, KEY_NPAGE, DISWIN, "dis.nxt",     0,                                      0,    cmd_npg, arg_0 },
  { K_NRM, 'b',       DISWIN, "dis.bp",      "dissassembler breakpoint",             0,    cmd_brk, arg_0 },
  { K_NRM, 's',       DISWIN, "dis.step",    "dissassembler step",                   0,    cmd_stp, arg_0 },
};

static cmdcat_t disa_cat = {
  DISWIN,
  "disa",
  "disassembler / debug / execution related commands",
  sizeof(disa_cmds)/sizeof(*disa_cmds),
  disa_cmds
};

int dis_key(int mod, int key)
{
  int rkey = 0;
  cmd_t * cmd;

  if (cmd = cmd_lookup(DISWIN, mod, key), !cmd)
    return 0;

  if (cmd_run(cmd, -1, 0))
    return -1;

  return 1;

  switch ( key ) {

  case '=':
    dis_set(-1);
    break;

  case KEY_UP: {
    int i, addr = disa_addr - 2;
    setparm(&parm, emu68);
    for ( i=0; i<10; i+=2 ) {
      parm.pc = addr - i;
      desa68(&parm);
      if ( parm.status & DESA68_INST ) {
        addr = addr - i;
        break;
      }
    }
    dis_set(addr);
  } break;

  case KEY_DOWN:
    setparm(&parm, emu68);
    parm.pc   = disa_addr;
    disa_addr = parm.pc & parm.memmsk;
    desa68(&parm);
    dis_set(parm.pc);
    break;

  case KEY_LEFT:
    dis_set(disa_addr - 2); break;
  case KEY_RIGHT:
    dis_set(disa_addr + 2); break;
    break;

  default:
    rkey = key;
    break;
  }

  return rkey;
}


int dis_init(void)
{
  cmd_register(&disa_cat);
  return 0;
}

void dis_destroy(void)
{
  if (diswin) {
    delwin(diswin);
    diswin = 0;
  }
}

