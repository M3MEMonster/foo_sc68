/*
 * @file    src68_sec.c
 * @brief   program sections.
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

#include "src68_sec.h"
#include <stdlib.h>
#include <string.h>

static obj_t * isection_new(void);

static int isection_cmp(const obj_t * oa,const obj_t * ob)
{
  const sec_t * sa = (const sec_t *) oa;
  const sec_t * sb = (const sec_t *) ob;
  return sa->addr - sb->addr;
}

static void isection_del(obj_t * obj)
{
  sec_t * sec = (sec_t *) obj;
  free(sec->name);
  free(sec);
}

static objif_t if_section = {
  sizeof(sec_t), isection_new, isection_del, isection_cmp
};

static obj_t * isection_new(void)
{
  return obj_new(&if_section);
}

vec_t * sections_new(unsigned int max)
{
  return vec_new(max,&if_section);
}

sec_t * section_new(const char * name, unsigned int addr,
                    unsigned int size, unsigned int flag)
{
  sec_t * sec = (sec_t *)isection_new();
  if (sec) {
    sec->name = strdup( (char *)(name ? name : "") );
    sec->addr = addr;
    sec->size = size;
    sec->flag = flag;
  }
  return sec;
}

int section_add(vec_t * sections,
                const char * name, unsigned int addr,
                unsigned int size, unsigned int flag)
{
  int idx = -1;
  sec_t * sec = section_new(name, addr, size, flag);
  assert(sections);
  idx = vec_add(sections, &sec->obj, VEC_FNONE);
  if (idx == -1)
    obj_del(&sec->obj);
  return idx;
}

sec_t * section_get(vec_t * sections, int idx)
{
  return (sec_t *)vec_get(sections, idx);
}
