/*
 * @file    src68_src.c
 * @brief   source producer.
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

#include "src68_src.h"
#include "src68_dis.h"
#include "src68_msg.h"
#include <desa68.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern const char hexa[];               /* in src68_dis.c */

typedef struct {
  const char * uri;                     /* input URI */
  fmt_t * fmt;                          /* formatter */
  exe_t * exe;                          /* executable */
  int result;                           /* 0:on success */
  int symidx;                           /* index of current symbol */

  /* segment */
  struct {
    uint_t org;                         /* segment origin */
    uint_t end;                         /* segment end address */
    uint_t len;                         /* segment length */
    uint_t adr;                         /* segment current address */
  } seg;

  /* disassebler options */
  struct {
    uint_t show_opcode:1;         /* show opcodes */
    char * star_symbol;           /* symbol for current address (*) */
    uint_t dc_size;               /* default data size */
  } opt;

  /* disassemby temp buffer */
  struct {
    char * buf;                         /* disassembly buffer */
    uint_t max;                         /* sizeof of buf in byte */
  } str;

} src_t;

static char strbuf[1024];    /* static string buffer for disassembly */

enum {
  IFNEEDED = 0, ALWAYS = 1
};

/* Temporary replacement. Should be consistent with the ischar()
 * function used by desa68()
 */
static int src_ischar(src_t * src, uint_t v, int type)
{
  while (--type >= 0) {
    int c = v & 255;
    v >>= 8;
    if ( ! (c == '-' || c=='_' || c==' ' || c == '!' || c == '.' || c == '#'
            || (c>='a' && c<='z') || (c>='A' && c<='Z')
            || (c>='0' && c<='9')))
      return 0;
  }
  return 1;
}

static int src_islabel(src_t * src, uint_t adr)
{
  return 0 &
    ( symbol_byaddr(src->exe->symbols, adr, 0) >= 0 ) ||
    ( mbk_getmib(src->exe->mbk,adr) & (MIB_ADDR|MIB_ENTRY) );
}

static int src_eol(src_t * src, int always)
{
  int err = fmt_eol(src->fmt, always);
  src->result |= err;
  return err;
}

static int src_tab(src_t * src)
{
  int err = fmt_tab(src->fmt);
  src->result |= err;
  return err;
}

static int src_cat(src_t * src, const void * buf, int len)
{
  int err = fmt_cat(src->fmt, buf, len);
  src->result |= err;
  return err;
}

static int src_puts(src_t * src, const char * str)
{
  int err = fmt_puts(src->fmt, str);
  src->result |= err;
  return err;
}

static int src_putf(src_t * src, const char * fmt, ...)
{
  int err;
  va_list list;
  va_start(list, fmt);
  err = fmt_vputf(src->fmt, fmt, list);
  src->result |= err;
  va_end(list);
  return err;
}

static
/* print symbol a given address */
int symbol_at(src_t * src, uint_t adr)
{
  vec_t * const smb = src->exe->symbols;
  sym_t * sym = symbol_get(smb, symbol_byaddr(smb, adr, 0));
  return sym
    ? src_puts(src, sym->name)
    : src_putf(src, "L%06X", adr)
    ;
}

static
/* Print all labels from address adr+ioff.
 * ioff == 0 -> Label:
 * ioff != 0 -> Label: = *+
 */
int label_exactly_at(src_t * src, uint_t adr, uint_t ioff)
{
  exe_t * const exe = src->exe;
  int i = 0;
  int mib = mbk_getmib(exe->mbk, adr+ioff);
  int isym = symbol_byaddr(exe->symbols, adr+ioff, i);
  if ( (isym >= 0) || (mib & (MIB_ADDR|MIB_ENTRY)) ) {
    /* At least one */
    do {
      sym_t * sym = symbol_get(exe->symbols, isym);
      src_eol(src,IFNEEDED);
      if (sym)
        src_putf(src, "%s:", sym->name);
      else
        src_putf(src, "L%06X:", adr+ioff);
      if (ioff) {
        src_putf(src, " = %s%+d", src->opt.star_symbol, ioff);
        src_eol(src,IFNEEDED);
      }
      isym = symbol_byaddr(exe->symbols, adr+ioff, ++i);
    } while (isym >= 0);
  }
  return src->result;
}

static
/*
 * Print all labels from address adr up to adr+len exclusive.
 */
int label_at(src_t * src, uint_t adr, uint_t len)
{
  uint_t ioff;

  /* For each address in range */
  for (ioff = 0; ioff < len; ++ioff)
    if (label_exactly_at(src, adr, ioff) < 0)
      return -1;
  return 0;
}

static
/* Disassemble a single data line at segment current address. Never
 * more than <max> bytes can be consume.
 *
 * @return number of bytes consumed
 */
int data(src_t * src, uint_t max)
{
  static const int  maxpl[] = { 8,  8,  16, 16, 16 };
  static const char typec[] = {'0','b','w','3','l' };
  const char quote = '"';
  mbk_t * const mbk = src->exe->mbk;

  int typed = 0;                       /* 0:untyped 1:byte 2:word 4:long */
  int count = 0;                       /* byte current lines.  */
  uint_t ilen, iadr, iend, v;

  iadr = src->seg.adr;
  iend = iadr + max;
  if (iend > src->seg.end)
    iend = src->seg.end;

  while (!src->result && (ilen = iend - iadr)) {
    int type, mib;

    assert( (int)ilen > 0);

    /* Break for label */
    if (count > 0 && src_islabel(src, iadr))
      break;

    mib = mbk_getmib(mbk, iadr);
    if ( (mib & (MIB_EXEC|MIB_DATA) ) == MIB_EXEC )
      /* instruction ? time to leave */
      break;

    if (mib & MIB_ODD)                  /* odd address always byte */
      type = 1;
    else if (mib & MIB_RELOC)           /* relocated are always long */
      type = 4;
    else {
      if (mib & MIB_BYTE)               /* could use byte */
        type = 1;
      else if (mib & MIB_WORD)          /* could use word */
        type = 2;
      else if (mib & MIB_LONG)          /* could use long */
        type = 4;
      else if (typed)
        type = typed;
      else
        type = src->opt.dc_size;

      assert (type == 1 || type == 2 || type == 4);
      if (type > 1) {
        if (src_islabel(src, iadr+1))
          type = 1;
        else if ( type > 2 && ( src_islabel(src, iadr+2) ||
                                src_islabel(src, iadr+3) ) )
          type = 2;
      }
    }

    while (type > ilen)                 /* ensure in range */
      type >>= 1;

    if ( (mib & MIB_RELOC) && type != 4 )
      wmsg(iadr,"could not apply relocation to dc.%c\n", typec[type]);

    if (!typed) {
      /* First type */
      typed = type;
      count = 0;
      label_at(src, iadr, typed);
      src_tab(src);
      src_putf(src,"dc.%c",typec[typed]);
      src_tab(src);
    } else if (type != typed) {
      /* Change type */
      break;
    }

    if (count > 0)
      src_cat(src,",",1);

    switch (typed) {
    case 1:
      v = mbk_byte(mbk, iadr);
      if (src_ischar(src, v, 1))
        src_putf(src, "%cc%c", quote, v, quote);
      else
        src_putf(src, "$%02X", v);
      break;
    case 2:
      v = mbk_word(mbk, iadr);
      if (src_ischar(src, v, 2))
        src_putf(src, "%c%c%c%c",
                 quote, (char)(v>>8), (char)v, quote);
      else
        src_putf(src, "$%04X",v);
      break;
    case 4:
      v = mbk_long(mbk, iadr);
      if ( mib & MIB_RELOC ) {
        symbol_at(src, v);
      } else {
        if (src_ischar(src, v, 4))
          src_putf(src, "%c%c%c%c%c%c", quote,
                   (char)(v>>24), (char)(v>>16), (char)(v>>8), (char)v,
                   quote) ;
        else
          src_putf(src, "$%08X",v);
      }
      break;
    default:
      assert(!"unexpected size");
      src->result = -1;
      return -1;
    }
    count += typed;
    iadr += typed;

    if (count + typed > maxpl[typed])
      break;
  }

  src_eol(src,IFNEEDED);

  ilen = iadr - src->seg.adr;
  assert(ilen == count);
  src->seg.adr = iadr;

  return src->result ? -1 : ilen;
}

static
/* Disassemble a single instruction at segment current address
 * @return number of bytes consumed
 */
int inst(src_t * src)
{
  mbk_t * const mbk = src->exe->mbk;
  int ityp;
  uint_t ilen, iadr = src->seg.adr;
  char * space;

  assert( ! (iadr & 1 ) );
  assert( ! src->result );

  ityp = dis_disa(src->exe, &src->seg.adr, src->str.buf, src->str.max);
  if (src->seg.adr > src->seg.end)
    ityp = DESA68_DCW;                  /* out of range: as data */
  if (ityp <= DESA68_DCW) {
    /* Error or data */
    src->seg.adr = iadr;                /* restore position */
    src->result = ityp == DESA68_DCW ? 0 : -1;
    return 0;
  }
  ilen = src->seg.adr - iadr;

  assert( ilen >= 2 && ilen <= 10 );
  assert( !(ilen & 1) );

  /* Labels */
  label_at(src, iadr, ilen);

  /* Instruction */
  src_tab(src);
  space = strchr(src->str.buf,' ');
  if (space) {
    src_cat(src, src->str.buf, space-src->str.buf);
    src_tab(src);
    src_puts(src,space+1);
  } else {
    src_puts(src, src->str.buf);
    src_tab(src);
  }

  /* Opcode bytes */
  if (ilen && src->opt.show_opcode) {
    uint_t i;
    char * s = src->str.buf;
    strcpy(s,"; "); s += 2;
    for (i=0; i<ilen; ++i) {
      const uint8_t byte = mbk->buf[iadr+i-mbk->org];
      *s++ = hexa[byte>>4];
      *s++ = hexa[byte&15];
      if ( (i & 1) ) {
        if ( i > 2 && i+3 <= ilen && mbk_getmib(mbk, iadr+i-1) & MIB_LONG )
          continue;
        *s++ = '-';
      }
      /* dmsg("%s%d:%s\n",!i?"\n":"",i,mbk_mibstr(mbk_getmib(mbk, iadr+i),0)); */
    }
    *--s = 0;
    src_tab(src);
    src_cat(src, src->str.buf, s-src->str.buf);
  }

  /* next line */
  src_eol(src,IFNEEDED);

  /* $$$ XXX ignore errors for now */

  return ilen;
}

static
/* Auto detect code and data blocks */
void mixed(src_t * src, const uint_t adr, const uint_t len)
{
  mbk_t * const mbk = src->exe->mbk;

  assert(adr >= mbk->org);
  assert(adr+len <= mbk->org+mbk->len);

  src->seg.adr = adr;                   /* segment address.     */
  src->seg.len = len;                   /* segment length.      */
  src->seg.end = adr + len;             /* segment end address. */

  while (!src->result && src->seg.adr < src->seg.end) {
    uint_t mib = mbk_getmib(mbk, src->seg.adr);
    assert(mib);

    if ( !(mib & MIB_EXEC) || inst(src) <= 0) {
      if (data(src, src->seg.end-src->seg.adr) <= 0)
        src->result = -1;
    }
  }
}

int src_exec(fmt_t * fmt, exe_t * exe)
{
  src_t src;

  if (!fmt || !exe)
    return -1;

  /* setup sourcer info struct */
  memset(&src,0,sizeof(src));

  src.fmt = fmt;
  src.exe = exe;
  src.str.buf = strbuf;                 /* tmp string buffer */
  src.str.max = sizeof(strbuf);
  src.opt.star_symbol = "*";            /* star symbol */
  src.opt.show_opcode = 1;              /* show opcodes */
  src.opt.dc_size = 2;                  /* default to dc.w */

  symbol_sort_byaddr(exe->symbols);     /* to walk symbols in order */

  src_tab(&src);
  src_puts(&src,"org");
  src_tab(&src);
  src_putf(&src,"$%x", exe->mbk->org);
  src_eol(&src,ALWAYS);
  src_eol(&src,ALWAYS);

  mixed(&src, exe->mbk->org, exe->mbk->len);

  return src.result;

  /* src_code(fmt, exe, exe->mbk->org, exe->mbk->len); */
  /* src_data(fmt, exe, exe->mbk->org, exe->mbk->len); */
  /* src_bss (fmt, exe, exe->mbk->org, exe->mbk->len); */

/*
  section = section_get(exe->sections, section_idx = 0);
  for (off = 0; off < mbk->len; ) {
  adr = off + mbk->org;
  while (section && adr < section->addr) {
  if ( !(adr & 1) && (mbk->mib[off] & MIB_MIB_EXEC) ) {
  // disassemble
  } else {
  // data
  }
  }
  }
*/
}
