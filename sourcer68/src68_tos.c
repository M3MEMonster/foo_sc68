/*
 * @file    src68_tos.c
 * @brief   Atari TOS executable format.
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

#include "src68_tos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DRI_SYMBOL_SIZE 14u

static inline uint_t readW(const uint8_t * b) {
  return (b[0]<<8) | b[1];
}

static inline uint_t readL(const uint8_t * b) {
  return (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | b[3];
}

static inline void writeL(uint8_t * b, uint_t v) {
  b[0] = v>>24; b[1] = v>>16; b[2] = v>>8; b[3] = v;
}

int tos_reloc(tosexec_t * tosexec, uint_t base, toscall_f toscall)
{
  const uint_t end = tosexec->txt.len + tosexec->dat.len - 4;
  uint_t idx=4 , off=0, add=readL(tosexec->img + tosexec->rel.off);

  while (add) {
    off += add;
    if (add == 1)
      off += 253;
    else {
      if (off & 1)
        return TOS_ERR_REL_ODD;
      if (off > end)
        return TOS_ERR_REL_OOR;
      if (base)
        writeL(tosexec->img+off, readL(tosexec->img+off) + base);
      if (toscall)
        toscall(tosexec, base, off);
    }
    if (idx >= tosexec->rel.len)
      return TOS_ERR_REL_IDX;
    add = tosexec->img[tosexec->rel.off + idx++];
  }
  return 0;
}

int tos_header(tosexec_t * tosexec, tosfile_t * tosfile, uint_t filesize)
{
  const uint_t off_max = filesize - TOS_HEADER_SIZE;
  int rel_size;

  memset(tosexec, 0, sizeof(*tosexec));

  /* Need at least the header plus a 68 instruction for a valid
   * executable. Then check for the magic bra.s instruction. */
  if (filesize < TOS_HEADER_SIZE+2 ||
      (tosfile->bras[0] != 0x60 || tosfile->bras[1] != 0x1A))
    return TOS_ERR_NOT_TOS;

  /* currently points to file data, later will point on TPA */
  tosexec->img     = tosfile->text;

  /* sections lengths */
  tosexec->txt.len = readL(tosfile->txt_sz);
  tosexec->dat.len = readL(tosfile->dat_sz);
  tosexec->bss.len = readL(tosfile->bss_sz);
  tosexec->sym.len = readL(tosfile->sym_sz);

  /* sections offsets */
  tosexec->txt.off = 0;
  tosexec->dat.off = tosexec->txt.off + tosexec->txt.len;
  tosexec->bss.off = tosexec->dat.off + tosexec->dat.len;
  tosexec->sym.off = tosexec->bss.off;
  tosexec->rel.off = tosexec->sym.off + tosexec->sym.len;

  /* no section should points outside of data range. */
  if (tosexec->dat.off >= off_max || tosexec->sym.off >= off_max ||
      tosexec->rel.off >= off_max)
    return TOS_ERR_SEC_OOR;

  /* no section should points to an odd address */
  if ((tosexec->dat.off | tosexec->sym.off | tosexec->rel.off)&1)
    return TOS_ERR_SEC_ODD;

  /* Atari MiNT extensions ? */
  tosexec->has.mint = (readL(tosfile->mint) == 0x4d694e54);
  if (tosexec->has.mint) {
    tosexec->flags   = readL(tosfile->flags);
    tosexec->has.abs = !!readW(tosfile->noreloc);
    /* Could verify more things here: mint header size (256), mint
     * start instructions (8 first TEST section bytes ... */
  }

  if (!tosexec->has.abs) {
    rel_size = filesize - tosexec->rel.off;
    if (rel_size < 4)
      return TOS_ERR_REL_IDX;
    tosexec->rel.len = rel_size;
  }

  /* runtime checks for the symbols structure size */
  assert(sizeof(tossymb_t) == DRI_SYMBOL_SIZE);
  if (sizeof(tossymb_t) != DRI_SYMBOL_SIZE)
    return TOS_ERR_INTERNAL;

  /* Paranoiac test: the length of the symbol section shoould be a
   * multiple of the symbol struct size. Remove this test if it
   * happens for a legit reason I can't think of ATM. */
  if (tosexec->sym.len % DRI_SYMBOL_SIZE)
    return TOS_ERR_SYM_SIZ;

  assert(sizeof(tossymb_t) == DRI_SYMBOL_SIZE);
  if (sizeof(tossymb_t) != DRI_SYMBOL_SIZE)
    return 666;
  /* else { */
  /*   uint_t si, nsi = tosexec->sym.len / sizeof(tossymb_t); */
  /*   for (si=0; si<nsi; ++si) { */
  /*     tossymb_t * symb = (tossymb_t *)(tosexec->img+tosexec->sym.off)+si; */
  /*     char name[9]; */
  /*     uint_t flag = readW(symb->type); */
  /*     strncpy(name, symb->name,8); */
  /*     name[8]=0; */
  /*     printf("[%d] \"%s\" %04x (%s) %08x\n", */
  /*            si, name, flag, "", readL(symb->addr)); */
  /*   } */
  /* } */

  return tosexec->rel.len
    ? tos_reloc(tosexec, 0, 0)
    : 0
    ;
}

int tos_symbol(tosxymb_t * xymb,
               tosexec_t * tosexec,  tosfile_t * tosfile, int walk)
{
  tossymb_t * symb;

  xymb->name[0] = 0;
  xymb->type = 0;
  xymb->addr = 0;

  if (walk == tosexec->sym.len)
    return 0;                           /* the end */

  if (walk % DRI_SYMBOL_SIZE || walk >= tosexec->sym.len)
    return -1;                          /* error */

  assert( sizeof(symb->name)+DRI_SYMBOL_SIZE < sizeof(xymb->name) );

  symb = (tossymb_t *) (tosfile->text + tosexec->sym.off + walk);
  walk += DRI_SYMBOL_SIZE;
  xymb->type = readW(symb->type);
  xymb->addr = readL(symb->addr);
  strncpy(xymb->name, symb->name, sizeof(symb->name));
  xymb->name[sizeof(symb->name)] = 0;
  if ((xymb->type & TOS_SYMB_CMASK) == TOS_SYMB_LNAM) {
    if (walk >= tosexec->sym.len)
      return -1;                          /* error */
    strncpy(xymb->name+sizeof(symb->name), symb[1].name, DRI_SYMBOL_SIZE);
    xymb->name[sizeof(symb->name)+DRI_SYMBOL_SIZE] = 0;
    walk += DRI_SYMBOL_SIZE;
  }
  return walk;
}
