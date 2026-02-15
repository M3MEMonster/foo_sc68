/*
 * @file    src68_mbk.c
 * @brief   memory blocks.
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

#include "src68_mbk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char * mbk_mibstr(int mib, char * str)
{
  static char tmp[32];
  static const char bits[] = "RABWLEXDP";
  char *s; int bit;
  if (!str) str = tmp;
  s = str + sprintf(str,"$%04X/", mib);
  if (mib & MIB_SET) {
    *s++ = (mib & MIB_ODD) ? '1' : '0';
    *s++ = '/';
  }
  for (bit=0; bit<sizeof(bits)-1; ++bit)
    *s++ = (mib & (1<<bit)) ? bits[bit] : '.';
  bit = (mib & MIB_OPSZ_MSK) >> MIB_OPSZ_BIT;
  if (bit) {
    *s++ = '/';
    *s++ = '0' + bit;
  }
  *s = 0;
  return str;
}

mbk_t * mbk_new(uint_t org, uint_t len)
{
  mbk_t * mbk = 0;
  if (len > 0 && !(org&1)) {
    const uint_t alen = (len+3) & -4, bytes = sizeof(mbk_t)-1+alen*2;
    mbk = calloc(1,bytes);
    if (mbk) {
      mbk->org = org;
      mbk->len = len;
      mbk->mem = mbk->buf;
      mbk->_mib = mbk->buf+alen;
    }
  }
  return mbk;
}

void mbk_free(mbk_t * mbk)
{
  if (mbk) {
    if (mbk->mem != mbk->buf)
      free(mbk->mem);
    free(mbk);
  }
}

int mbk_getmib(const mbk_t * mbk, uint_t adr)
{
  int mib;
  assert(mbk); assert(mbk->_mib);

  adr -= mbk->org;
  if (adr < mbk->len) {
    mib = mbk->_mib[adr];
    if ( adr & 1 )
      mib = (mib & MIB_BOTH) | MIB_ODD | MIB_SET;
    else
      mib |= ((mbk->_mib[adr+1] >> MIB_BOTH_BITS) << 8) | MIB_SET;
  } else
    mib = 0;
  return mib;
}

int mbk_setmib(const mbk_t * mbk, uint_t adr, int clr, int set)
{
  int mib = mbk_getmib(mbk, adr);

  if (mib) {
    int old = mib;
    const int msk = ( old & MIB_ODD ) ? MIB_BOTH : MIB_ALL;

    assert ( !( adr & 1 ) == !( old & MIB_ODD ) );

    assert (old & MIB_SET);
    mib &= MIB_ALL;
    assert(!(mib & MIB_SET));

    clr &= msk;
    set &= msk;
    mib = (mib & ~clr) | set;           /* new mib value */

    /* Write back */
    adr -= mbk->org;
    if ( old & MIB_ODD ) {
      assert (adr & 1);
      assert(mib == (mib & MIB_BOTH));
      mbk->_mib[adr] = ( mbk->_mib[adr] & ~MIB_BOTH ) | mib;
    } else {
      int mib2;
      assert(!(adr&1));
      mbk->_mib[adr] = mib;
      mib2 = (( mib & MIB_ALL ) >> 8) << MIB_BOTH_BITS;
      assert ( (mib2 & ~MIB_BOTH) == mib2 );
      mbk->_mib[adr+1] = (mbk->_mib[adr+1] & MIB_BOTH) | mib2;
    }
    assert (mbk_getmib(mbk, adr+mbk->org) == (mib|(old&(MIB_SET|MIB_ODD))));

    mib ^= old;
    assert(mib & MIB_SET);
    assert( !( adr & 1 ) == !( mib & MIB_ODD ) );
    assert( (mbk_getmib(mbk, adr + mbk->org) & (MIB_SET|set)) == (MIB_SET|set));
  }
  return mib;
}
