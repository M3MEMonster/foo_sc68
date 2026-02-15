/*
 * @file    mksc68_cmd_time.c
 * @brief   the "time" command
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
#include "mksc68_dsk.h"
#include "mksc68_msg.h"
#include "mksc68_opt.h"
#include "mksc68_str.h"

#include <sc68/file68.h>
#include <sc68/file68_vfs.h>
#include <sc68/sc68.h>
#include <emu68/emu68.h>
#include <emu68/excep68.h>
#include <io68/io68.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

enum {
  TRAP_ADDR    = 0x1000,           /* see libsc68/api68.c           */
  MAX_TIME     = 60 * 60 * 1000,   /* default max time              */
  SILENCE_TIME = 5 * 1000,         /* default silent time           */
  PASS_TIME    = 3 * 60 * 1000     /* default search time increment */
};

enum {
  EXIT_OK      = 0,                     /* exit success */
  EXIT_GENERIC,                         /* exit with generic error */
  EXIT_INIT,                            /* error during init */
  EXIT_PLAY,                            /* error during play */
  EXIT_LOOP,                            /* error during loop */
  EXIT_SILENT,                          /* only silence */
  EXIT_NOHW,                            /* no relevant hardware found */
  EXIT_MAX_PASS,                        /* could not find a loop */
  EXIT_ST_WRONG,                        /* unexpected error  */
};

enum {
  EMU68_I = EMU68_X << 1
};

/* HAXXX: Must be the same order than sc68 internal struct. */
enum {
  YM = 0,
  MW,
  SHIFTER,
  PAULA,
  MFP
};

typedef union {
  unsigned all;
  struct {
    unsigned ym:1;      /* YM used by this music                    */
    unsigned mw:1;      /* mw used by this music                    */
    unsigned pl:1;      /* Amiga/Paula used by this track           */
    unsigned sh:1;      /* shifter has been access                  */
    unsigned mk:1;      /* mfp has been access                      */

    unsigned ta:1;      /* MFP Timer-A used by this track           */
    unsigned tb:1;      /* MFP Timer-B used by this track           */
    unsigned tc:1;      /* MFP Timer-C used by this track           */
    unsigned td:1;      /* MFP Timer-D used by this track           */
  } bit;
} hw_t;

enum {
  TIMER_A = 0X134 >> 2,
  TIMER_B = 0X120 >> 2,
  TIMER_C = 0X114 >> 2,
  TIMER_D = 0X110 >> 2
};

static const opt_t longopts[] = {
  { "help",       0, 0, 'h' },
  { "max-time",   1, 0, 'M' },          /* max search time         */
  { "pass-time",  1, 0, 'p' },          /* search pass time        */
  { "silent",     1, 0, 's' },          /* silent detection length */
  { 0,0,0,0 }
};


typedef struct {
  int bas;
  int end;
  int act;
  int mod;
} mw_reg_t;

typedef struct
{
  io68_t   io;
  unsigned r;
  unsigned w;
  unsigned a;
  mw_reg_t mw;
  io68_t * pio;
} time_io_t;

typedef struct
{
  io68_t io;
  io68_t * pio;
} mem_io_t;

typedef struct measureinfo_s measureinfo_t;

struct measureinfo_s {
  int           code;       /* return code */
  volatile int  isplaying;  /* 0:stoped 1:playing 2:init 3:shutdown */
  pthread_t     thread;     /* thread instance.  */

  int           track;      /* track to measure. */
  sc68_t      * sc68;       /* sc68 instance.    */
  emu68_t     * emu68;      /* emu68 instance.   */
  io68_t     ** ios68;      /* other chip.       */

  unsigned sampling;        /* sampling rate (in hz) */
  unsigned replayhz;        /* replay rate (in hz)   */
  unsigned location;        /* memory start address  */

  unsigned startfr;         /* starting frame (loop)  */
  unsigned startms;         /* corrsponding ms        */

  unsigned max_ms;          /* maximum search time    */
  unsigned stp_ms;          /* search depth increment */
  unsigned sil_ms;          /* length of silence      */

  /* results */
  addr68_t minaddr;     /* lower memory location used by this track */
  addr68_t maxaddr;     /* upper memory location used by this track */
  unsigned frames;      /* last frame of the music                  */
  unsigned timems;      /* corresponding time in ms                 */
  unsigned loopfr;      /* loop duration in frames                  */
  unsigned loopms;      /* correponding time in ms                  */
  unsigned curfrm;      /* current frame counter                    */

  hw_t     hw;                        /* hardware used by play pass */
  /* hw_t     init_hw; */             /* hardware uses by init pass */

  hwflags68_t hwflags;  /* Save track hardware flags                */

  time_io_t timeios[5]; /* hooked IOs access                        */
  mem_io_t  memio;      /* hooked RAM access                        */
  struct {
    unsigned cnt;
    unsigned fst;
    unsigned lst;
  } vector[260];

};

#ifndef EMU68_ATARIST_CLOCK
# define EMU68_ATARIST_CLOCK (8010613u&~3u)
#endif
static const unsigned atarist_clk = EMU68_ATARIST_CLOCK;

static unsigned cycle_per_frame(unsigned hz, unsigned clk)
{
  hz  = hz  ? hz  : 50u;
  clk = clk ? clk : atarist_clk;
  return
    ( hz % 50u == 0 && clk == atarist_clk)
    ? 160256u * 50u / hz
    : clk / hz
    ;
}

unsigned fr2ms(unsigned fr, unsigned cpf_or_hz, unsigned clk)
{
  uint64_t ms;
  unsigned cpf;
  clk = clk ? clk : atarist_clk;
  cpf = (cpf_or_hz <= 1000u)
    ? cycle_per_frame(cpf_or_hz, clk)
    : cpf_or_hz
    ;

  ms  = fr;                             /* total frames. */
  ms *= cpf;                            /* total cycles. */
  ms *= 1000u;                          /* result in ms  */
  ms += clk >> 1;
  ms /= clk;

  return (unsigned) ms;
}

static unsigned ms2fr(unsigned ms, unsigned cpf_or_hz, unsigned clk)
{
  uint64_t fr;
  unsigned cpf;
  clk = clk ? clk : atarist_clk;
  cpf = (cpf_or_hz <= 1000u)
    ? cycle_per_frame(cpf_or_hz, clk)
    : cpf_or_hz
    ;

  fr   = ms;                             /* number of millisecond */
  fr  *= clk;                            /* number of millicycle  */
  cpf *= 1000u;
  fr  += cpf >> 1;
  fr  /= cpf;                            /* number of frame       */

  return (unsigned) fr;
}

static void time_rb(io68_t * pio) {
  time_io_t * ti = (time_io_t *)pio;
  ++ti->r;
  ++ti->a;
  ti->pio->r_byte(ti->pio);
}

static void time_rw(io68_t * pio) {
  time_io_t * ti = (time_io_t *)pio;
  ++ti->r;
  ++ti->a;
  ti->pio->r_word(ti->pio);
}

static void time_rl(io68_t * pio) {
  time_io_t * ti = (time_io_t *)pio;
  ++ti->r;
  ++ti->a;
  ti->pio->r_long(ti->pio);
}


static void time_wb(io68_t * pio) {
  time_io_t * ti = (time_io_t *)pio;
  ++ti->w;
  ++ti->a;
  ti->pio->w_byte(ti->pio);
}

static void time_ww(io68_t * pio) {
  time_io_t * ti = (time_io_t *)pio;
  ++ti->w;
  ++ti->a;
  ti->pio->w_word(ti->pio);
}

static void time_wl(io68_t * pio) {
  time_io_t * ti = (time_io_t *)pio;
  ++ti->w;
  ++ti->a;
  ti->pio->w_long(ti->pio);
}

static void mem_rb(io68_t * pio) {
  mem_io_t * mem = (mem_io_t *)pio;
  mem->pio->r_byte(mem->pio);
}

static void mem_rw(io68_t * pio) {
  mem_io_t * mem = (mem_io_t *)pio;
  mem->pio->r_word(mem->pio);
}

static void mem_rl(io68_t * pio) {
  mem_io_t * mem = (mem_io_t *)pio;
  mem->pio->r_long(mem->pio);
}

static void mem_wb(io68_t * pio) {
  mem_io_t * mem = (mem_io_t *)pio;
  mem->pio->w_byte(mem->pio);
}

static void mem_ww(io68_t * pio) {
  mem_io_t * mem = (mem_io_t *)pio;
  mem->pio->w_word(mem->pio);
}

static void mem_wl(io68_t * pio) {
  mem_io_t * mem = (mem_io_t *)pio;
  mem->pio->w_long(mem->pio);
}

static void chkframe_range(emu68_t * const emu68,
                           addr68_t from, addr68_t to, const int flags)
{
  while (from < to)
    chkframe(emu68, from++,flags);
}

static int mw_get(io68_t * const io, int adr)
{
  addr68_t addr = io->emu68->bus_addr;
  int68_t  data = io->emu68->bus_data;
  int ret;

  io->emu68->bus_addr = adr;
  io->r_byte(io);
  ret = io->emu68->bus_data;
  io->emu68->bus_addr = addr;
  io->emu68->bus_data = data;
  return ret;
}

static void mw_wreg(time_io_t * const io, int adr, unsigned int val)
{
  const int msk = 7<<24;

  switch (adr & 255) {
  case 0x03: /* MW_BASH */
    io->mw.bas = (io->mw.bas & 0xFF00FFFF) | (val << 16) | (4<<24); break;
  case 0x05: /* MW_BASM */
    io->mw.bas = (io->mw.bas & 0xFFFF00FF) | (val <<  8) | (2<<24); break;
  case 0x07: /* MW_BASL */
    io->mw.bas = (io->mw.bas & 0xFFFFFF00) | val         | (1<<24); break;
  case 0x0f: /* MW_ENDH */
    io->mw.end = (io->mw.end & 0xFF00FFFF) | (val << 16) | (4<<24); break;
  case 0x11: /* MW_ENDM */
    io->mw.end = (io->mw.end & 0xFFFF00FF) | (val <<  8) | (2<<24); break;
  case 0x13: /* MW_ENDL */
    io->mw.end = (io->mw.end & 0xFFFFFF00) | val         | (1<<24); break;
  case 0x21: /* MW_MODE */
    io->mw.mod = val; break;
  }

  if ( (mw_get(io->pio, 0xFF8901) & 1) &&
       (io->mw.bas & msk) == msk &&
       (io->mw.end & msk) == msk &&
       io->mw.bas < io->mw.end) {
    io->mw.bas &= 0xFFFFFF;
    io->mw.end &= 0xFFFFFF;
    /* msgdbg("MW-CHK %06x .. %06x\n", io->mw.bas, io->mw.end); */
    chkframe_range(io->pio->emu68, io->mw.bas, io->mw.end, EMU68_R);
  }

}

static void mw_wb(io68_t * pio) {
  time_io_t * ti = (time_io_t *)pio;
  mw_wreg(ti,pio->emu68->bus_addr, pio->emu68->bus_data & 255);
  ++ti->w;
  ++ti->a;
  ti->pio->w_byte(ti->pio);
}

static void mw_ww(io68_t * pio) {
  time_io_t * ti = (time_io_t *)pio;
  if ( !(pio->emu68->bus_addr & 1) ) {
    mw_wreg(ti,pio->emu68->bus_addr+1, pio->emu68->bus_data & 255);
  }
  ++ti->w;
  ++ti->a;
  ti->pio->w_word(ti->pio);
}

static void mw_wl(io68_t * pio) {
  time_io_t * ti = (time_io_t *)pio;
  if ( !(pio->emu68->bus_addr & 1) ) {
    mw_wreg(ti,pio->emu68->bus_addr+1, (pio->emu68->bus_data>>16) & 255);
    mw_wreg(ti,pio->emu68->bus_addr+3,  pio->emu68->bus_data      & 255);
  }
  ++ti->w;
  ++ti->a;
  ti->pio->w_long(ti->pio);
}

static measureinfo_t measureinfo;

static const char * vectorname(int vector)
{
  static char tmp[64];
  emu68_exception_name(vector,tmp);
  return tmp;
}

extern void sc68_emulators(sc68_t *, emu68_t **, io68_t ***);

static void dump_access_areas(measureinfo_t * mi)
{
  unsigned adr, stack, start = 0,
    endsp = ( mi->emu68->reg.a[7] - 1 ) & mi->emu68->memmsk;

  /* Walk the stack */
  for ( stack = endsp;
        stack >= start && ( mi->emu68->chk[stack] & ( EMU68_R | EMU68_W ) );
        --stack)
    ;

  for (adr=start; adr<stack; ++adr) {
    printf("MIB: 0x%06X %c%c%c\n", adr,
           (mi->emu68->chk[adr] & EMU68_X)?'X':'.',
           (mi->emu68->chk[adr] & EMU68_R)?'R':'.',
           (mi->emu68->chk[adr] & EMU68_W)?'W':'.');
  }
}


/* Find memory access range */
static void access_range(measureinfo_t * mi)
{
  unsigned i,j;
  unsigned stack;
  unsigned start = mi->location & mi->emu68->memmsk;
  unsigned endsp = ( mi->emu68->reg.a[7] - 1 ) & mi->emu68->memmsk;

  struct {
    unsigned min, max;
  } mod[4];                             /* R,W,X,A */

  /* $$$ HAXXX */
  if (getenv("SC68_DUMP_MIB"))
    dump_access_areas(mi);

  /* Walk the stack */
  for ( stack = endsp;
        stack >= start && ( mi->emu68->chk[stack] & ( EMU68_R | EMU68_W ) );
        --stack)
    ;
  if (stack != endsp)
    msginf("S access range: 0x%06X .. 0x%06X\n",
           stack, mi->emu68->reg.a[7] & (unsigned) mi->emu68->memmsk);
  else
    msginf("S access range: none\n");

  for (j=0; j<4; ++j) {
    if (j < 3) {
      mod[j].min = stack;
      mod[j].max = start;
      for ( i = start; i < stack ; i++ ) {
        if ( mi->emu68->chk[i] & ( EMU68_R << j ) ) {
          if ( i < mod[j].min ) mod[j].min = i;
          if ( i > mod[j].max ) mod[j].max = i;
        }
      }
      if (!j) {
        mod[3].min = mod[j].min;
        mod[3].max = mod[j].max;
      } else {
        if ( mod[j].min < mod[3].min ) mod[3].min = mod[j].min;
        if ( mod[j].max > mod[3].max ) mod[3].max = mod[j].max;
      }
    }
    if ( mod[j].min < mod[j].max )
      msginf("%c access range: 0x%06X .. 0x%06X (%d bytes)\n",
             j["RWXA"],mod[j].min, mod[j].max, mod[j].max-mod[j].min+1);
    else
      msginf("%c access range: none\n", j["RWXA"]);
  }

  mi->minaddr = mod[3].min;
  mi->maxaddr = mod[3].max;

  if (0) {
    for (i=0 ; i<mi->emu68->memmsk+1; ++i) {
      if (!(i&15))
        printf ("%06x ",i);
      printf ("%c%c%c%c",
              (mi->emu68->chk[i]&EMU68_R)?'R':'.',
              (mi->emu68->chk[i]&EMU68_W)?'W':'.',
              (mi->emu68->chk[i]&EMU68_X)?'X':'.',
              (i&15)==15 ? '\n' : ' ');
    }
  }
}

/* Timers use */
static void access_timers(measureinfo_t * mi, hw_t * hw)
{
  hw->bit.ta = mi->vector[TIMER_A].cnt > 0;
  hw->bit.tb = mi->vector[TIMER_B].cnt > 0;
  hw->bit.tc = mi->vector[TIMER_C].cnt > 0;
  hw->bit.td = mi->vector[TIMER_D].cnt > 0;
}

/* IO chip access */
static void access_ios(measureinfo_t * mi, hw_t * hw)
{
  hw->bit.ym = mi->timeios[YM].a      > 0;
  hw->bit.mw = mi->timeios[MW].a      > 0;
  hw->bit.pl = mi->timeios[PAULA].a   > 0;
  hw->bit.sh = mi->timeios[SHIFTER].a > 0;
  hw->bit.mk = mi->timeios[MFP].a     > 0;
}

static int hook_ios(measureinfo_t * mi)
{
  int i;

  /* hook memory access handler */
  mi->memio.io.r_byte = mem_rb;
  mi->memio.io.r_word = mem_rw;
  mi->memio.io.r_long = mem_rl;
  mi->memio.io.w_byte = mem_wb;
  mi->memio.io.w_word = mem_ww;
  mi->memio.io.w_long = mem_wl;
  mi->memio.pio = mi->emu68->memio;
  snprintf(mi->memio.io.name,sizeof(mi->memio.io.name),
           "*%s", mi->emu68->memio->name);
  mi->emu68->memio = &mi->memio.io;
  msgdbg("hook memio '%-14s'\n", mi->memio.io.name);

  for (i=0; i<5; ++i) {
    io68_t    * pio = mi->ios68[i], * mio;
    int        line = (pio->addr_lo>>8) & 0xFF;
    time_io_t * ti  = mi->timeios+i;

    mio = mi->emu68->mapped_io[line];
    ti->pio = mio;                      /* legacy mapped io */
    ti->r = ti->w = 0;                  /* reset counters   */

    /* create new hooked io */
    snprintf(ti->io.name,sizeof(ti->io.name),"*%s", mio->name);
    ti->io.addr_lo = pio->addr_lo;
    ti->io.addr_hi = pio->addr_hi;

    ti->io.r_byte = time_rb;            /* hook memory access handler */
    ti->io.r_word = time_rw;
    ti->io.r_long = time_rl;

    if ( 0xFF8900 == (0xffffff & pio->addr_lo) )  {
      ti->io.w_byte = mw_wb;
      ti->io.w_word = mw_ww;
      ti->io.w_long = mw_wl;
    } else {
      ti->io.w_byte = time_wb;
      ti->io.w_word = time_ww;
      ti->io.w_long = time_wl;
    }
    ti->io.emu68  = mi->emu68;

    if ( mi->emu68->mapped_io[line] == pio )
      msgdbg("hook io #%d '%-14s' addr:$%06x-%06x #%02x\n",
             i, ti->io.name, (unsigned) pio->addr_lo & 0xFFFFFF,
             (unsigned) pio->addr_hi & 0xFFFFFF, line);
    else
      msgdbg("hook ?? #%d '%-14s' addr:$%06x-%06x #%02x\n",
             i, ti->io.name, (unsigned) mio->addr_lo & 0xFFFFFF,
             (unsigned) mio->addr_hi & 0xFFFFFF, line);

    mi->emu68->mapped_io[line] = &ti->io;
  }
  return 0;
}

static int unhook_ios(measureinfo_t * mi)
{
  int i;

  if (mi->emu68->memio == &mi->memio.io) {
    msgdbg("unhook memio '%-14s'\n", mi->memio.io.name);
    mi->emu68->memio = mi->memio.pio;
    mi->memio.pio = 0;
  }

  for (i=0; i<5; ++i) {
    io68_t    * pio = mi->ios68[i];
    int        line = (pio->addr_lo>>8) & 0xFF;
    time_io_t * ti  = mi->timeios+i;
    if (mi->emu68->mapped_io[line] == &ti->io) {
      msgdbg("unhook io #%d '%-14s' addr:$%06x-%06x #%02x, %u/%u/%u\n",
             i, ti->io.name, (unsigned) ti->io.addr_lo & 0xFFFFFF ,
             (unsigned)  ti->io.addr_hi & 0xFFFFFF,
             line, ti->r+ti->w, ti->r, ti->w);
      mi->emu68->mapped_io[line] = ti->pio;
    }
  }
  return 0;
}

static void timemeasure_hdl(emu68_t* const emu68, int vector, void * cookie)
{
  measureinfo_t * mi = cookie;
  assert(mi == &measureinfo);

  /* Detect and ignore system timer-C */
  if (vector == TIMER_C) {
    const addr68_t adr = vector << 2;
    if ( ( (emu68->mem [ adr+0 ] << 24) |
           (emu68->mem [ adr+1 ] << 16) |
           (emu68->mem [ adr+2 ] <<  8) |
           (emu68->mem [ adr+3 ] <<  0) ) == (TRAP_ADDR+2) )
      return;
  }

  if (vector == HWINIT_VECTOR)
    hook_ios(mi);
  else if (vector >= 0 && vector < sizeof(mi->vector)/sizeof(*mi->vector)) {
    if (!mi->vector[vector].cnt ++)
      mi->vector[vector].fst = mi->curfrm;
    mi->vector[vector].lst = mi->curfrm;
  }
}

static void timemeasure_init(measureinfo_t * mi)
{
  sc68_create_t create68;
  disk68_t * disk;
  music68_t * mus;
  int sampling;

  mi->isplaying = 2;
  mi->code      = EXIT_INIT;

  memset(&create68,0,sizeof(create68));
  create68.name        = "mksc68-time";
  create68.emu68_debug = 1;
  create68.log2mem     = 23;             /* 2Mb */

  mi->sc68 = sc68_create(&create68);
  if (!mi->sc68)
    return;

  /* Force aSID off. */
  sc68_cntl(mi->sc68, SC68_SET_ASID, SC68_ASID_OFF);

  /* Retrieve emulator instance to hack them. */
  sc68_cntl(mi->sc68, SC68_EMULATORS, &mi->ios68);
  mi->emu68 = (emu68_t *)*mi->ios68++;

  /* Install our interrupt handler. */
  emu68_set_handler(mi->emu68, timemeasure_hdl);

  /* Set our private data. */
  emu68_set_cookie(mi->emu68, mi);

  /* open the disk */
  disk = dsk_get_disk();
  if (sc68_open(mi->sc68, disk) < 0)
    return;

  /* retrieve music/track info. */
  mus = &disk->mus[mi->track-1];
  mi->replayhz = mus->frq;
  mi->location = mus->a0;
  mi->hwflags  = mus->hwflags;          /* Save hwflags */

  /* Set all hardware flags. */
  if (mus->hwflags & SC68_AGA) {
    mus->hwflags = SC68_AGA;
  } else {
    mus->hwflags = SC68_PSG|SC68_DMA|SC68_LMC;
  }

  if (sc68_play(mi->sc68, mi->track, SC68_INF_LOOP) < 0)
    return;

  sampling = sc68_cntl(mi->sc68, SC68_GET_SPR);
  if (sampling <= 0)
    return;
  mi->sampling = sampling;

  /* Run the music init code. */
  if (sc68_process(mi->sc68,0,0) == SC68_ERROR)
    return;

  /* Compute access after init. */
  access_range(mi);
  access_timers(mi,&mi->hw);
  access_ios(mi,&mi->hw);

  /* reset memory access flags after init. */
  emu68_chkset(mi->emu68, 0, 0, 0);

  mi->code = EXIT_OK;
}


static void timemeasure_run(measureinfo_t * mi)
{
  char    str[256];
  int32_t buf[32];
  const int slice = sizeof(buf) / sizeof(*buf);
  int code, n;
  int i,lst = 0x1000000;
  unsigned acu;

  unsigned frm_cnt;           /* frame counter                      */
  unsigned frm_max;           /* maximum frame to try to detect end */
  unsigned frm_lim;           /* frame limit for this pass          */
  unsigned frm_stp;           /* frame limit increment              */
  unsigned upd_frm;           /* last updated frame                 */
  unsigned cpf;               /* 68k cycle per frame                */
  unsigned sil_val;           /* pcm value                          */
  unsigned sil_frm;           /* first silent frame                 */
  unsigned sil_nop = ~0;      /* special value for no silent        */
  unsigned sil_max;           /* frames to concider real silence    */
  unsigned sli_cnt;           /* slices counter                     */
  int      updated;           /* has been updated this pass ?       */

  memset(buf,0,sizeof(buf));
  sli_cnt = frm_cnt = 0;
  mi->isplaying = 1;
  mi->code      = EXIT_OK;
  cpf = cycle_per_frame(mi->replayhz, mi->emu68->clock);
  mi->startms   = fr2ms(mi->startfr, cpf, mi->emu68->clock);
  msgdbg("time-measure: run [track:%d skp:%d max:%d stp:%d sil:%d spr:%u hz:%u]\n",
         mi->track, mi->startms, mi->max_ms, mi->stp_ms, mi->sil_ms,
         mi->sampling, mi->replayhz);

  /* The trick to measure loop length is to run up to the previously
   * detected end, reset the menory access flags and continue as if it
   * were a new song.
   */
  if (mi->startfr) {
    msgdbg("time-measure: now measuring loop, skipping %u frames (%s)\n",
           mi->startfr, str_timefmt(str+0x00, 0x20, mi->startms));
    while (n = slice,
           code = sc68_process(mi->sc68,buf,&n),
           code != SC68_ERROR ) {
      ++sli_cnt;
      if (code & SC68_IDLE)
        continue;
      if (code & (SC68_CHANGE|SC68_END)) {
        msgwrn(
          "sc68_process() returns an wrong return code (%x) at frame %u (%s)\n",
          code, frm_cnt,
          str_timefmt(str,32,fr2ms(frm_cnt, cpf, mi->emu68->clock))
          );
        mi->code = EXIT_ST_WRONG;
        break;
      }
      assert( (code & (SC68_CHANGE|SC68_END)) == 0 );
      assert( n == slice );
      if (++frm_cnt == mi->startfr)
        break;
    }
    if (code == SC68_ERROR)
      mi->code = EXIT_LOOP;
    if (mi->code != EXIT_OK)
      return;

    emu68_chkset(mi->emu68, 0, 0, 0);   /* reset memory access flags */
    msgdbg("time-measure: %u frames skipped\n", frm_cnt);
  }

  /* sil_val==0 won't trigger silent detection */
  sil_val = (mi->sil_ms > 0) ? 0x08 * slice * 2 : 0;
  sil_frm = sil_nop;

  sil_max = ms2fr(mi->sil_ms, cpf, mi->emu68->clock);
  frm_max = frm_cnt + ms2fr(mi->max_ms, cpf, mi->emu68->clock);
  frm_stp = ms2fr(mi->stp_ms, cpf, mi->emu68->clock);
  frm_lim = frm_cnt + frm_stp;
  upd_frm = sil_nop;
  updated = 0;

  msgdbg("time search:\n"
         " max: %-6u fr, %s\n"
         " stp: %-6u fr, %s\n"
         " sil: %-6u fr, %s\n",
         (unsigned) frm_max, str_timefmt(str+0x00,0x20,mi->max_ms),
         (unsigned) frm_stp, str_timefmt(str+0x20,0x20,mi->stp_ms),
         (unsigned) sil_max, str_timefmt(str+0x40,0x20,mi->sil_ms));

  while (n = slice,
         code = sc68_process(mi->sc68,buf,&n),
         code != SC68_ERROR ) {
    ++sli_cnt;
    if (code & (SC68_CHANGE|SC68_END)) {
      msgwrn(
        "sc68_process() returns an wrong return code (%x) at frame %u (%s)\n",
             code, frm_cnt,
             str_timefmt(str,32,fr2ms(frm_cnt, cpf, mi->emu68->clock))
        );
      mi->code = EXIT_ST_WRONG;
      break;
    }
    assert( (code & (SC68_CHANGE|SC68_END)) == 0 );
    assert( n == slice );

    /* Compute the sum of deltas for silence detection */
    /* msgdbg("slice #%d", sli_cnt); */

    /* Get the very first value */
    if (lst == 0x1000000)
      lst = ( (int)(s16)buf[0] ) + ( (int)(s16)(buf[0]>>16) );

    for (acu=i=0; i<n; ++i) {
      const int val
        = ( (int)(s16)buf[i] ) + ( (int)(s16)(buf[i]>>16) );
      const int dif = val - lst;
      /* msgdbg(" %08x(%d,%d)", (uint32_t)buf[i], val, dif); */
      acu += (dif >= 0) ? dif : -dif;
      lst = val;
    }
    /* msgdbg(" -> "); */

    if (0) {
      static int acu_avg = 0;
      acu_avg = (n*acu_avg + acu) / (n+1);
      msgdbg("frame #%u/%u: acu=%d avg=%d sil_val=%d\n",
             frm_cnt, sli_cnt, acu, acu_avg, sil_val);
    }

    if ( acu < sil_val ) {
      /* This slice is silent */
      if ( sil_frm == sil_nop ) {
        sil_frm = frm_cnt; /* first of a serie, keep the frame number */
        /* msgdbg("silence start at frame #%u/%u (%s)\n", */
        /*        sil_frm, sli_cnt, str_timefmt(str+0x00,0x20,fr2ms(sil_frm, cpf, mi->emu68->clock))); */
      } else {
        /* msgdbg("silence continue at frame #%u/%u (%s) %u frames (%s) and counting\n", */
        /*        frm_cnt, sli_cnt, str_timefmt(str+0x00,0x20,fr2ms(frm_cnt, cpf, mi->emu68->clock)), */
        /*        frm_cnt-sil_frm, str_timefmt(str+0x20,0x20,fr2ms(frm_cnt-sil_frm, cpf, mi->emu68->clock))); */
      }
    } else {
      /* This slice is not silent */
      /* if ( sil_frm != sil_nop ) { */
      /*   unsigned sil_len = frm_cnt - sil_frm; */
      /*   msgdbg("silence too short at frame #%u (%s) for %u frames (%s) \n", */
      /*          sil_frm, str_timefmt(str+0x00,0x20,fr2ms(sil_frm, cpf, mi->emu68->clock)), */
      /*          sil_len, str_timefmt(str+0x20,0x20,fr2ms(sil_len, cpf, mi->emu68->clock))); */
      /* } */
      sil_frm = sil_nop;                /* Not silent slice */
    }

    /* Silence is detected only if it does not start at the beginning
     * and its length greater than sil_max */
    if ( sil_frm != sil_nop && sil_frm > 0 && frm_cnt - sil_frm > sil_max ) {
      unsigned sil_len = frm_cnt - sil_frm;
      msgdbg("silence detected at frame #%u (%s) for %u frames (%s)\n",
             sil_frm,
             str_timefmt(str+0x00,0x20,fr2ms(sil_frm, cpf, mi->emu68->clock)),
             sil_len,
             str_timefmt(str+0x20,0x20,fr2ms(sil_len, cpf, mi->emu68->clock)));
      mi->frames = sil_frm;
      mi->loopfr = sil_nop;
      break;
    }

    /* If no emulation pass has been run by sc68_process() we are done here */
    if (code & SC68_IDLE) continue;

    if (mi->emu68->frm_chk_fl) {
      /* something as change here */
      upd_frm = frm_cnt;
      updated = 1;

      msgdbg("AC #%d (%s) %c%c%c:%06x/%06x %c%c%c:%06x/%06x\n",
             (unsigned) upd_frm,
             str_timefmt(str+0x00,0x200,fr2ms(upd_frm, cpf, mi->emu68->clock)),
             (mi->emu68->fst_chk.fl & EMU68_X) ? 'X' : '.',
             (mi->emu68->fst_chk.fl & EMU68_W) ? 'W' : '.',
             (mi->emu68->fst_chk.fl & EMU68_R) ? 'R' : '.',
             (unsigned)mi->emu68->fst_chk.pc,
             (unsigned)mi->emu68->fst_chk.ad,
             (mi->emu68->lst_chk.fl & EMU68_X) ? 'X' : '.',
             (mi->emu68->lst_chk.fl & EMU68_W) ? 'W' : '.',
             (mi->emu68->lst_chk.fl & EMU68_R) ? 'R' : '.',
             (unsigned)mi->emu68->lst_chk.pc,
             (unsigned)mi->emu68->lst_chk.ad);
    }

    /* That was a new frame. Let's do the memory access checking trick. */
    mi->curfrm = ++frm_cnt;

    if ( frm_cnt > frm_max ) {
      msgerr("#%02d: reach max limit (%ufr %ums)\n",
             mi->track, frm_max, mi->max_ms);
      mi->code = EXIT_MAX_PASS;
      break;
    }

    if ( frm_cnt > frm_lim ) {
      if ( !updated && upd_frm != sil_nop ) {
        /* found something */
        msgdbg("#%02d: end of track detected at frame %u (%s)\n",
               mi->track, upd_frm,
               str_timefmt(str+0x00,0x20,fr2ms(upd_frm, cpf, mi->emu68->clock)));
        mi->frames = upd_frm;
        break;
      } else {
        msgdbg("#%02d: unable to detect end of track at frame %u (%s)\n",
               mi->track, frm_lim,
               str_timefmt(str+0x00,0x20,fr2ms(frm_lim, cpf, mi->emu68->clock)));
        frm_lim += frm_stp;
        updated = 0;
      }
    }
  }

  if (code == SC68_ERROR)
    mi->code = EXIT_PLAY;

  if (mi->code != EXIT_OK)
    return;

  /* Got only silent ? */
  if (sil_frm < 4) {
    msgwrn("#%02d is this a song of silent ???\n", mi->track);
    /* $$$ Do we want this to be an error ? */
    mi->code = EXIT_SILENT;
    return;
  }

  /* Don't want to do this while computing loops.
   *
   * $$$ Ben: Or do we ? At least access range might help to detect
   *          some errors. In the other hand it might also be a
   *          problem.
   */
  if (!mi->startms) {
    /* memory access */
    access_range(mi);
    access_timers(mi,&mi->hw);
    access_ios(mi,&mi->hw);
  }

  /* Display interrupt vectors */
  for (i=0; i<sizeof(mi->vector)/sizeof(*mi->vector); ++i)
    if (mi->vector[i].cnt)
      msginf("vector #%03d \"%s\" triggered %d times fist:%u last:%u\n",
             i, vectorname(i),
             mi->vector[i].cnt, mi->vector[i].fst, mi->vector[i].lst);

  if (!mi->hw.bit.ym && !mi->hw.bit.mw && !mi->hw.bit.pl) {
    msgerr("#%02d: no relevant hardware detected ???\n", mi->track);
    mi->code = EXIT_NOHW;
    return;
  }


  if (mi->frames) {
    unsigned frames, timems;

    frames = mi->frames - mi->startfr;
    timems = fr2ms(frames, cpf, mi->emu68->clock);

    msgdbg("this pass -- %u fr (%s)\n",
           frames, str_timefmt(str, 32, timems));

    if (!mi->startfr) {
      mi->frames = frames;
      mi->timems = timems;
      if (mi->loopfr == ~0) {
        msgdbg("silence was detected, set loop length to 0\n");
        mi->loopfr = mi->loopms = 0;
      } else {
        mi->loopfr = mi->frames;
        mi->loopms = mi->timems;
      }
    } else {
      msgdbg("2nd pass set time and loop to %ufr (%s)\n",
             mi->startfr, str_timefmt(str, 32, mi->startms));
      mi->frames = mi->startfr;
      mi->timems = mi->startms;
      mi->loopfr = frames;
      mi->loopms = timems;
    }
    msgdbg("set time: %u fr (%s) -- loop: %u fr (%s)\n",
           mi->frames, str_timefmt(str+0x00, 32, mi->timems),
           mi->loopfr, str_timefmt(str+0x20, 32, mi->loopms) );
  }
}

static void timemeasure_end(measureinfo_t * mi)
{
  mi->isplaying = 3;
  if (mi->sc68) {
    sc68_t * sc68 = mi->sc68;
    unhook_ios(mi);
    mi->sc68  = 0;
    mi->emu68 = 0;
    mi->ios68 = 0;
    sc68_destroy(sc68);
  }
  mi->isplaying = 0;
}

static inline addr68_t mymin(const addr68_t a, const addr68_t b)
{
  return a < b ? a : b;
}

static inline addr68_t mymax(const addr68_t a, const addr68_t b)
{
  return a > b ? a : b;
}

static void * time_thread(void * userdata)
{
  measureinfo_t * mi = (measureinfo_t *) userdata;
  addr68_t range_min, range_max;
  hw_t hardware;

  assert( mi == &measureinfo );

  /* First pass detects the music time. */
  timemeasure_init(mi);
  range_min = mi->minaddr;
  range_max = mi->maxaddr;
  hardware  = mi->hw;

  if (mi->code == EXIT_OK) {
    timemeasure_run(mi);
    range_min = mymin(range_min,mi->minaddr);
    range_max = mymax(range_max,mi->maxaddr);
    hardware.all |= mi->hw.all;
  }
  timemeasure_end(mi);

  if (mi->code == EXIT_OK && mi->loopfr) {
    const unsigned startfr = mi->frames;
    /* Second pass detects the loop time. */
    timemeasure_init(mi);
    if (!mi->code) {
      range_min = mymin(range_min,mi->minaddr);
      range_max = mymax(range_max,mi->maxaddr);
      hardware.all |= mi->hw.all;

      mi->startfr = startfr;
      timemeasure_run(mi);
      mi->minaddr = mymin(range_min,mi->minaddr);
      mi->maxaddr = mymax(range_max,mi->maxaddr);
      mi->hw.all |= hardware.all;
    }
    timemeasure_end(mi);
  }

  return mi;
}


static
int time_measure(measureinfo_t * mi, int trk,
                 int stp_ms, int max_ms, int sil_ms)
{
  int ret = -1, err;
  const int    tracks = dsk_get_tracks();
  disk68_t  * const d = dsk_get_disk();
  music68_t * const m = d->mus + trk - 1;
  struct timespec ts;
  ts.tv_sec  = 180;
  ts.tv_nsec = 0;

  msgdbg("time_measure() trk:%d, stp:%dms, max:%dms sil:%dms, time-out:%d\n",
         trk, stp_ms, max_ms, sil_ms, (int)ts.tv_sec);

  assert(mi);
  assert(trk > 0 && trk <= tracks);

  if (trk <= 0 || trk > tracks) {
    msgerr("#%02d: track out of range\n", trk);
    goto error;
  }

  if (mi->isplaying) {
    msgerr("#%02d: busy\n", trk);
    goto error;
  }

  memset(mi,0,sizeof(*mi));
  mi->stp_ms = stp_ms;
  mi->max_ms = max_ms;
  mi->sil_ms = sil_ms;
  mi->track  = trk;

  if ( pthread_create(&mi->thread, 0, time_thread, mi) ) {
    msgerr("#%02d: failed to create time thread\n", trk);
    goto error;
  }

#ifdef HAVE_PTHREAD_TIMEDJOIN_NP
  msgdbg("time-measure thread created ... Waiting for %d seconds\n",
         (int)ts.tv_sec);
  ts.tv_sec += time(0);
  err = pthread_timedjoin_np(mi->thread, 0, &ts);
  if (err == ETIMEDOUT) {
    msgdbg("time-measure thread time-out, canceling\n");
    err = pthread_cancel(mi->thread);
  }
#else
  msgdbg("time-measure thread created ... Waiting for %d seconds\n",
         (int)ts.tv_sec);
  err = pthread_join(mi->thread, 0);
#endif
  msgdbg("time-measure thread ended with: %d, %d\n", err, mi->code);

  ret = err ? -1 : mi->code;
  if (!ret) {
    char        s1[32],s2[32],s3[32],s4[32];
    unsigned    total_fr, total_ms;

    m->first_fr = mi->frames;
    m->first_ms = mi->timems;
    m->loops    = 1;
    m->loops_fr = mi->loopfr;
    m->loops_ms = mi->loopms;
    /* m-> */total_fr = m->first_fr + (m->loops-1) * m->loops_fr;
    /* m-> */total_ms = fr2ms(/* m-> */total_fr, m->frq, 0);
    m->has.time = 1;
    m->has.loop = 1;

    mi->hwflags  = 0;
    mi->hwflags |= SC68_XTD;
    mi->hwflags |= mi->hw.bit.ym ? SC68_PSG : 0;
    mi->hwflags |= mi->hw.bit.mw ? SC68_DMA|SC68_LMC : 0;
    mi->hwflags |= mi->hw.bit.pl ? SC68_AGA : 0;
    mi->hwflags |= mi->hw.bit.ta ? SC68_MFP_TA : 0;
    mi->hwflags |= mi->hw.bit.tb ? SC68_MFP_TB : 0;
    mi->hwflags |= mi->hw.bit.tc ? SC68_MFP_TC : 0;
    mi->hwflags |= mi->hw.bit.td ? SC68_MFP_TD : 0;

    /* mi->minaddr; */
    /* mi->maxaddr; */

    msginf("%02u - %s + %d x %s => %s [%s]\n",
           trk,
           str_timefmt(s1,sizeof(s1),m->first_ms),
           m->loops-1,
           m->loops_ms ? str_timefmt(s2,sizeof(s2),m->loops_ms) : "no loop",
           str_timefmt(s3,sizeof(s3),/* m-> */total_ms),
           str_hardware(s4,sizeof(s4),mi->hwflags)
      );
  }

  /* Restore hardware flags (either unmodified in case of error, or
   * detected in case of success. */
  m->hwflags = mi->hwflags;


  /* Just clean up the struct. */
  memset(mi,0,sizeof(*mi));

error:

  return ret;
}

static
int run_time(cmd_t * cmd, int argc, char ** argv)
{
  int ret = EXIT_GENERIC;
  char shortopts[(sizeof(longopts)/sizeof(*longopts))*3];
  int i, tracks;
  const char * tracklist = 0;
  int max_ms = MAX_TIME, sil_ms = SILENCE_TIME, stp_ms = PASS_TIME;


  opt_create_short(shortopts, longopts);

  while (1) {
    int longindex;
    int val =
      getopt_long(argc, argv, shortopts, longopts, &longindex);

    switch (val) {
    case  -1: break;                    /* Scan finish */
    case 'h':                           /* --help */
      help(argv[0]); return 0;

    /* case 't':                           /\* --tracks    *\/ */
    /*   if (tracklist) { */
    /*     msgerr("multiple track-list not allowed.\n"); */
    /*     goto error; */
    /*   } */
    /*   tracklist = optarg; */
    /*   /\* check track-list syntax *\/ */
    /*   while (i = str_tracklist(&tracklist, &a, &b), i > 0) */
    /*     ; */
    /*   if (i) */
    /*     goto error; */
    /*   tracklist = optarg; */
    /*   break; */

    case 'M':                           /* --max-time  */
      if (str_time_stamp((const char**)&optarg, &max_ms))
        goto error;
      break;
    case 's':                           /* --silent    */
      if (str_time_stamp((const char**)&optarg, &sil_ms))
        goto error;
      break;
    case 'p':                           /* --pass-time */
      if (str_time_stamp((const char**)&optarg, &stp_ms))
        goto error;
      break;

    case '?':                       /* Unknown or missing parameter */
      goto error;
    default:
      msgerr("unexpected getopt return value (%d)\n", val);
      goto error;
    }
    if (val == -1) break;
  }
  i = optind;

  /* if (i < argc) */
  /*   msgwrn("%d extra parameters ignored\n", argc-i); */

  if (!dsk_has_disk()) {
    msgerr("no disk loaded\n");
    goto error;
  }
  if (tracks = dsk_get_tracks(), tracks <= 0) {
    msgerr("disk has no track\n");
    goto error;
  }

  if (i == argc) {
    int track = dsk_trk_get_current();
    ret = time_measure(&measureinfo, track, stp_ms, max_ms, sil_ms);
  } else {
    int a, b, e;

    tracklist = argv[i++];
    if (i < argc)
      msgwrn("%d extra parameters ignored\n", argc-i);

    while (e = str_tracklist(&tracklist, &a, &b), e > 0) {
      for (; a <= b; ++a) {
        ret = time_measure(&measureinfo, a, stp_ms, max_ms, sil_ms);
        if (ret) goto validate;
      }
    }
    if (e < 0)
      goto error;
  }

validate:
  dsk_validate();

error:
  return ret;
}

cmd_t cmd_time = {
  /* run */ run_time,
  /* com */ "time",
  /* alt */ 0,
  /* use */ "[opts] [TRACKS ...]",
  /* des */ "Autodetect track duration",
  /* hlp */
  "The `time' command run sc68 music emulator in a special way that allows\n"
  "to autodetect tracks duration.\n"
  "\n"
  "TRACKS\n"
  "  List of tracks (eg: 1,2-5,7), or all.\n"
  "\n"
  "OPTIONS\n"
  /* *****************   ********************************************** */
  "  -s --silent=MS      Duration for silent detection (0:disable).\n"
  "  -M --max-time=MS    Maximum time.\n"
  "  -p --pass-time=MS   Search pass duration."
};
