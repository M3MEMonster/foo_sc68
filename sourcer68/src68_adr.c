/*
 * @file    src68_adr.c
 * @brief   address container.
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

#include "src68_adr.h"
#include <stdlib.h>
#include <string.h>

static obj_t * iaddr_new(void);

static int iaddr_cmp(const obj_t * oa, const obj_t * ob)
{
  const adr_t * sa = (const adr_t *) oa;
  const adr_t * sb = (const adr_t *) ob;
  return sa->adr - sb->adr;
}

static void iaddr_del(obj_t * obj)
{
  adr_t * rel = (adr_t *) obj;
  free(rel);
}

static objif_t if_reloc = {
  sizeof(adr_t), iaddr_new, iaddr_del, iaddr_cmp
};

static obj_t * iaddr_new(void)
{
  return obj_new(&if_reloc);
}

vec_t * address_new(unsigned int max)
{
  return vec_new(max,&if_reloc);
}

adr_t * addr_new(uint_t adr)
{
  adr_t * obj = (adr_t *) iaddr_new();
  if (obj)
    obj->adr = adr;
  return obj;
}

int addr_add(vec_t * adrs, uint_t adr)
{
  int idx = -1;
  adr_t * obj = addr_new(adr);
  idx = vec_add(adrs, &obj->obj, VEC_FNODUP);
  if (idx == -1)
    obj_del(&obj->obj);
  return idx;
}

adr_t * addr_get(vec_t * adrs, int idx)
{
  return (adr_t *) vec_get(adrs, idx);
}

int addr_exists(vec_t * adrs, uint_t adr)
{
  adr_t obj;
  obj.obj = adrs->iface;
  obj.adr = adr;
  return vec_find(adrs, &obj.obj, 0, -1) >= 0;
}
