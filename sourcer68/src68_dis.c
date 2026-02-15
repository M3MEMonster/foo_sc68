/*
 * @file    src68_dis.c
 * @brief   disassembler and code walker.
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

#include "src68_dis.h"
#include "src68_mbk.h"
#include "src68_sym.h"
#include "src68_msg.h"

#include <string.h>
#include <stdio.h>

#include <desa68.h>

/* Walker returned status. */
enum {
  BRK_ERR = -1,                         /* break on error */
  BRK_NOT = 0,                          /* don't break */
  BRK_DCW,                              /* not a valid instruction */
  BRK_OOR,                              /* out of memory range */

  /* Valid scout replies: !!! BRK_EXE must be first !!! */
  BRK_EXE,                              /* already walk dis way */
  BRK_RTS,                              /* rts */
  BRK_JMP,                              /* undefined jump */
  BRK_JTB                               /* jump table */
};

typedef struct {
  desa68_t desa;                        /* disassembler instance */
  uint_t   maxdepth;                    /* maximum recursive depth */
  uint_t   curdepth;                    /* current recursive depth */
  uint_t   addr;                        /* instruction address */

  walkopt_t walkopt;                    /* walk options. */
  disaopt_t disaopt;                    /* disassembly options. */

  mbk_t   *mbk;                         /* memory block */
  vec_t   *symbols;                     /* symbols container */
  int      result;                      /* result code (0 for success) */
  uint_t   scouting;                    /* scouting countdown */
  uint_t   maxscout;                    /* scouting amount */

  uint_t   mib[10];         /* temporary mib buffer (1 instruction) */
  char     sym[16];         /* symbol buffer for auto label (LXXXXXX) */
  char     str[256];        /* disassembly buffer */
} dis_t;

const char hexa[] = { '0','1','2','3','4','5','6','7','8',
                      '9','A','B','C','D','E','F' };

static
/* callback for desa68() symbol lookup. */
const char * symget(desa68_t * d, uint_t addr, int type)
{
  dis_t * dis = d->user;
  char * name = 0;
  int wanted = 0;

  sym_t * sym = dis->symbols
    ? symbol_get(dis->symbols, symbol_byaddr(dis->symbols, addr, -1)) : 0;

  /* Honor desa68 forced symbols request */
  if (type <= DESA68_SYM_DABL)          /* those are destination ref */
    wanted = (d->flags & DESA68_DSTSYM_FLAG);
  else                                  /* all others are source ref */
    wanted = (d->flags & DESA68_SRCSYM_FLAG);

  if (sym)
    name = sym->name;
  else if (!wanted) {

    /* A long words that has been relocated should always be
     * disassembled as a symbol */
    if (type == DESA68_SYM_SABL || type == DESA68_SYM_SIMM)
      wanted = mbk_getmib(dis->mbk, (d->pc - 4) & d->memmsk) & MIB_RELOC;

    /* Reference or entry points have to be symbols. */
    if (!wanted)
      wanted = mbk_getmib(dis->mbk, addr) & (MIB_ADDR|MIB_ENTRY);

    /* Requested to auto-detect symbols inside the memory range */
    if (!wanted && (dis->disaopt.auto_symbol)) {
      if (type == DESA68_SYM_SIMM)
        wanted = addr >= d->immsym_min && addr < d->immsym_max;
      else
        wanted = addr >= d->memorg && addr < d->memorg + d->memlen;
    }
  }

  /* Finally automatic symbol */
  if ( wanted && !name ) {
    sprintf(name = dis->sym, "L%06X", addr & d->memmsk);
    /* TODO add the symbol ?
     *
     * Pros: Doesn't have to keep the automatic symbol generation
     *       coherent between this function and the sourcer, specially
     *       when generating labels or symbols in dc.l
     *
     * Cons: I don't know. Probably not much. We might generate too
     *       much symbols but that's not really a big deal.
     *
     * Idea: Possibly add a flag for those automatic symbols.
     */
  }

  return name;
}


static
/* callback for desa68() memory access.
 *
 * @param d    desa68 instance
 * @param adr  address of byte to read
 * @param flag 0:subsequent bytes 1:byte 2:word 4:long
 */
int memget(desa68_t * d, unsigned int adr, int flag)
{
  dis_t * const dis = d->user;
  const int bits = MIB_READ | ( (flag & 7) * MIB_BYTE );

  assert( (adr-dis->addr) < 10 );       /* <= 10 bytes long */
  dis->mib[adr-dis->addr] = bits;       /* write instruction mib */
  if (!mbk_ismyaddress(dis->mbk,adr))
    return -1;
  return dis->mbk->mem[adr - dis->mbk->org];
}

static
/* simple wrapper for mbk_setmib() to ignore when scouting.
 */
int dis_setmib(dis_t *d, uint_t adr, int mib)
{
  return d->scouting
    ? 0
    : mbk_setmib(d->mbk, adr, 0, mib)
    ;
}

static
/* Commit temporary mib and set MIB opcode size for the instruction. */
int commit_mib(dis_t * d)
{
  int i, len, mib=0;

  assert ( d->desa.itype != DESA68_DCW );
  assert ( !(d->addr & 1) );
  assert ( ! d->desa.error );
  assert ( ! d->scouting );

  len = d->desa.pc - d->addr;
  assert( !(len & 1) && len >=2 && len <= 10);
  d->mib[0] |= MIB_EXEC | MIB_WALK | ( ((len+1)>>1) << MIB_OPSZ_BIT );

  for (i=len; --i >= 0; )
    if ( ! ( mib = mbk_setmib(d->mbk, d->addr+i, 0, d->mib[i])) )
      assert(!"mib access out of range ?");

  return mib;
}

static int r_dis_pass(dis_t * dis);

static int scout_pass(dis_t * dis)
{
  int scout = BRK_NOT;

  if (! dis->scouting) {
    /* Don't scout a scouter */
    dis->scouting = dis->maxscout;
    dis->addr = dis->desa.pc;
    scout = r_dis_pass(dis);
    dis->scouting = 0;
    if (scout >= BRK_EXE) {
      /* Scout was a success ! Let's do this for real */
      dis->addr = dis->desa.pc;
      scout = r_dis_pass(dis);
    }
  }
  return scout;
}

static int r_dis_pass(dis_t * dis)
{
  const uint_t save_pc = dis->desa.pc;
  char rts;

  if (dis->maxdepth && dis->curdepth > dis->maxdepth) {
    wmsg(dis->addr,"skip (level %u is too deep)\n", dis->curdepth);
    return BRK_NOT;
  }
  ++dis->curdepth;

  dis->desa.pc = dis->addr;
  dmsg("%*s$%06x (%u)\n", dis->curdepth, "", dis->curdepth);

  for (rts=BRK_NOT; rts == BRK_NOT; ) {
    uint_t off = dis->desa.pc - dis->mbk->org;
    uint_t mib = mbk_getmib(dis->mbk,dis->desa.pc);
    int ityp, ilen;

    if ( mib & MIB_WALK ) {
      /* Already walked dis way */
      rts = BRK_EXE;
      break;
    }

    if (off >= dis->mbk->len) {
      /* Offset out of range */
      rts = BRK_OOR;
      break;
    }

    dis->addr = dis->desa.pc;

    ityp = desa68(&dis->desa);
    if (ityp < 0) {
      /* On error */
      dis->result = dis->desa.error ? dis->desa.error : -1;
      rts = BRK_ERR;
      break;
    }

    if (ityp == DESA68_DCW) {
      /* On data */
      rts = BRK_DCW;
      break;
    }

    if ( ! dis->scouting )
      commit_mib(dis);
    else if (! --dis->scouting ) {
      /* Scout over */
      rts = BRK_RTS;                    /* pretend it's a RTS ? */
      break;
    }

    ilen = dis->desa.pc - dis->addr;

    switch (ityp) {

    case DESA68_RTS:
      rts = BRK_RTS;



      //////////////////////////////////////////////////////////////////////
      // $$$ test scouting
      //
      // INFO: this is not good as it will disassemble too much. We
      // need a multi pass walk to sort this out.
      //
      if (0)
        scout_pass(dis);
      //
      // $$$ test scouting
      //////////////////////////////////////////////////////////////////////

      break;

    case DESA68_INT: case DESA68_NOP:
      break;

    case DESA68_INS:
    {
      int i, types;
      uint_t adrs[2];

      for (types = i = 0; i < 2; ++i) {
        mbk_t * mbk = dis->mbk;
        struct desa68_ref * ref = !i ? &dis->desa.sref : &dis->desa.dref;
        uint_t adr = ref->addr, off = ref->addr - mbk->org, end = off;
        int mib = MIB_ADDR;

        types = (types << 8) + (ref->type&255);
        adrs[i] = adr;
        if (ref->type == DESA68_OP_NDEF) /* set ? */
          continue;
        switch (ref->type) {
        case DESA68_OP_A: break;
        case DESA68_OP_B: case DESA68_OP_W: case DESA68_OP_L:
          if ( dis->desa.regs & ( 1 << DESA68_REG_PC ) ) {
            /* d8(pc,rn) mode: it's an address not an access */
          } else if ( (off & 1) && ref->type != DESA68_OP_B ) {
            /* odd address for word or long access. */
            wmsg(adr, "%s reference on odd address (+$%x)\n",
                 ref->type == DESA68_OP_W ? "word" : "long", off);
          } else {
            const int log2 = ref->type - DESA68_OP_B;
            end = off + ( 1 << log2 ) - 1;
            mib |= MIB_BYTE << log2;
          }
          break;
        default:
          assert(!"invalid reference type");
          continue;
        }
        dis_setmib(dis, adr, mib);
        if (end < off || off >= mbk->len) /* out of range ? */
          continue;

      }

      /* Detects move.l #ADR,$VECTOR */
      if ( types == DESA68_OP_A*256+DESA68_OP_L
           && !(adrs[1] & 3) && adrs[1] >= 8 && adrs[1] < 0x200
           && !(adrs[0] & 1) && mbk_ismyaddress(dis->mbk,adrs[0])) {
        dis->addr = adrs[0];
        r_dis_pass(dis);
      }


    } break;

    case DESA68_BRA:
      rts = BRK_JMP;                    /* assume unknown jump */

      if (dis->desa.dref.type == DESA68_OP_NDEF)
        /* no reference address: he could have a jump table relative
         * to an address register xi(An,Rn). It would be nice to at
         * least produce a warning in that case.
         */
        break;

      assert(dis->desa.dref.type == DESA68_OP_A);
      if (dis->desa.dref.type != DESA68_OP_A)
        /* Should not happen but just in case */
        break;
      dis_setmib(dis, dis->desa.dref.addr, MIB_ADDR);

      if (! (dis->desa.regs & (1<<DESA68_REG_PC) )) {
        /* Address is known ( even jmp(pc) ) */
        const uint_t base = dis->addr;
        const uint_t jmppc = dis->desa.dref.addr;
        const uint_t opw = mbk_word(dis->mbk, dis->addr);
        uint_t nxtpc, mask = 0xFFFF;

        if ( (opw & 0xFF00) == 0x6000 && opw > 0x6000)
          mask = 0xFF00;                /* bra.s */

        for (nxtpc = dis->desa.pc;
             mbk_ismyaddress(dis->mbk, nxtpc+ilen-1);
             nxtpc += ilen) {
          const uint_t w = mbk_word(dis->mbk, nxtpc);

          dmsg("jmp table @$%06x[%02u]/$%06x opw:%04x got:%04x\n",
               base, (nxtpc-base) / ilen, nxtpc,
               opw, w);

          if ( (opw^w) & mask )
            break;
          dis->addr = nxtpc;
          r_dis_pass(dis);
        }

        //////////////////////////////////////////////////////////////////////
        // $$$ test scouting
        //
        scout_pass(dis);
        //
        // $$$ test scouting
        //////////////////////////////////////////////////////////////////////

        dis->desa.pc = jmppc;
        rts = BRK_NOT;                  /* resume as it was */
      } else {
        /* jmp table : $$$ TODO */
      }
      break;

    case DESA68_BSR:
      if (dis->desa.dref.type == DESA68_OP_A) {
        if (! (dis->desa.regs & (1<<DESA68_REG_PC) )) {
          dis->addr = dis->desa.dref.addr;
          dis_setmib(dis, dis->addr, MIB_ADDR);
          r_dis_pass(dis);
        } else {
          /* jsr table : $$$ TODO */
        }
        /* dis->desa.pc = save_pc; */
      }
      break;

    default:
      assert(!"unknowm instruction type");
      break;
    }
  }

  --dis->curdepth;
  dis->desa.pc = save_pc;
  return rts;
}


int dis_walk(walk_t * walk)
{
  uint_t entry = walk->adr;
  mbk_t * mbk = walk->exe->mbk;
  vec_t * symbols = walk->exe->symbols;
  dis_t dis;

  memset(&dis, 0, sizeof(dis));

  dis.addr        = entry;
  dis.mbk         = mbk;

  /* walker options */
  dis.walkopt.def_maxdepth = 32;
  dis.walkopt.def_maxscout = 5;

  dis.walkopt.brk_on_ndef_jmp = 1;
  dis.walkopt.brk_on_rts = 1;

  /* disassembler options */
  dis.disaopt.auto_symbol  = 1;

  dis.desa.user   = &dis;
  dis.desa.memget = memget;
  dis.desa.memorg = mbk->org;
  dis.desa.memlen = mbk->len;
  dis.desa.flags  = DESA68_SYMBOL_FLAG; /* need to follow symbolic reference */

  dis.symbols = symbols;
  if (dis.symbols)
    dis.desa.symget = symget;

  dis.desa.str = dis.str;
  dis.desa.strmax = sizeof(dis.str);

  dis.maxdepth = dis.walkopt.def_maxdepth;
  dis.maxscout = dis.walkopt.def_maxscout;

  dis.result = 0;

  dmsg("Starting walking '%s'\n", walk->exe->uri);
  dmsg(" mem [$%06x-$%06x (%u)\n", mbk->org, mbk->org+mbk->len-1, mbk->len);

  r_dis_pass(&dis);
  assert(!dis.curdepth);

  return dis.result;
}

int dis_disa(exe_t * exe, uint_t * adr, char * buf, uint_t max)
{
  dis_t dis;
  int itype;

  memset(&dis, 0, sizeof(dis));

  dis.addr        = *adr;
  dis.mbk         = exe->mbk;
  dis.disaopt.auto_symbol = 1;

  dis.desa.pc     = dis.addr;
  dis.desa.user   = &dis;
  dis.desa.memget = memget;
  dis.desa.memorg = exe->mbk->org;
  dis.desa.memlen = exe->mbk->len;
  dis.desa.str    = buf;
  dis.desa.strmax = max;
  dis.desa.flags  =
    DESA68_SYMBOL_FLAG | DESA68_GRAPH_FLAG | DESA68_LCASE_FLAG;

  dis.symbols     = exe->symbols;
  if (dis.symbols)
    dis.desa.symget = symget;

  if (itype = desa68(&dis.desa), itype > DESA68_DCW) {
    commit_mib(&dis);
    *adr = dis.desa.pc;
  }
  return itype;
}

