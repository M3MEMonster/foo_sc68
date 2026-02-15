/*
 * @file    src68_symbol.c
 * @brief   symbol container.
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

#include "src68_sym.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static obj_t * isymb_new(void);

static int isymb_byname_only(const obj_t * oa,const obj_t * ob)
{
  const sym_t * sa = (const sym_t *) oa;
  const sym_t * sb = (const sym_t *) ob;

  assert(sa && sb);
  assert(sa->obj == sb->obj);

  return strcmp(sa->name,sb->name);
}

static int isymb_byname(const obj_t * oa,const obj_t * ob)
{
  const sym_t * sa = (const sym_t *) oa;
  const sym_t * sb = (const sym_t *) ob;
  int res;

  assert(sa && sb);
  assert(sa->obj == sb->obj);

  if (res = strcmp(sa->name,sb->name), !res)
    if (res = sa->addr - sb->addr, !res)
      res = sa-sb;
  return res;
}

static int isymb_byaddr_only(const obj_t * oa,const obj_t * ob)
{
  const sym_t * sa = (const sym_t *) oa;
  const sym_t * sb = (const sym_t *) ob;

  assert(sa && sb);
  assert(sa->obj == sb->obj);

  return sa->addr-sb->addr;
}

static int isymb_byaddr(const obj_t * oa,const obj_t * ob)
{
  const sym_t * sa = (const sym_t *) oa;
  const sym_t * sb = (const sym_t *) ob;
  int res;

  assert(sa && sb);
  assert(sa->obj == sb->obj);

  if (res = sa->addr - sb->addr, !res)
    if (res = strcmp(sa->name,sb->name), !res)
      res = sa-sb;
  return res;
}

static void isymb_del(obj_t * obj)
{
  sym_t * sym = (sym_t *) obj;
  free(sym->name);
  free(sym);
}

static objif_t if_symbol = {
  sizeof(sym_t), isymb_new, isymb_del, isymb_byname
};

static obj_t * isymb_new(void)
{
  return obj_new(&if_symbol);
}

vec_t * symbols_new(unsigned int max)
{
  return vec_new(max,&if_symbol);
}

static int isvalid_name(const char * name)
{
  int c;
  assert(name);
  c = *name;
  c = c != '_' && !isalpha(c);          /* true if not a start char */
  if (!c)
    while (c=*++name, ( c == '_' || isalnum(c) ) )
      ;
  return !c;
}

sym_t * symbol_new(const char * name, unsigned int addr, unsigned int flag)
{
  sym_t * sym = 0;
  char tmp[32];
  if (!name) {
    snprintf(tmp,sizeof(tmp),"L%06X",addr);
    tmp[sizeof(tmp)-1] = 0;
    name = tmp;
  }
  if (isvalid_name(name)) {
    sym = (sym_t *) isymb_new();
    if (sym) {
      sym->name = strdup((char*)name);
      if (!sym->name) {
        isymb_del(&sym->obj);
        sym = 0;
      } else {
        sym->addr = addr;
        sym->flag = flag;
      }
    }
  }
  return sym;
}

int symbol_add(vec_t * syms,
               const char * name, unsigned int addr, unsigned int flag)
{
  int idx = -1;
  sym_t * sym = symbol_new(name, addr, flag);
  assert(syms);
  idx = vec_add(syms, &sym->obj, VEC_FNODUP);
  if (idx == -1)
    obj_del(&sym->obj);
  return idx;
}

sym_t * symbol_get(vec_t * symbs, int index)
{
  return (sym_t *)vec_get(symbs, index);
}

int symbol_byname(vec_t * symbs, const char * name)
{
  sym_t sym;
  sym.obj  = symbs->iface;
  sym.name = (char*)name;
  return vec_find(symbs, &sym.obj, isymb_byname_only, -1);
}

int symbol_byaddr(vec_t * symbs, unsigned int addr, int idx)
{
  sym_t sym;
  sym.obj  = symbs->iface;
  sym.addr = addr;
  return vec_find(symbs, &sym.obj, isymb_byaddr_only, idx);
}

void symbol_sort_byname(vec_t * symbs)
{
  vec_sort(symbs, isymb_byname);
  /* Sort by name but pretend it's by name only ! */
  symbs->sorted = isymb_byname_only;
}

void symbol_sort_byaddr(vec_t * symbs)
{
  vec_sort(symbs, isymb_byaddr);
  /* Sort by address but pretend it's by address only ! */
  symbs->sorted = isymb_byaddr_only;
}
