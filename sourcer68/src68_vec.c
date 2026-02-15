/*
 * @file    src68_vec.c
 * @brief   generic vector container.
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

#include "src68_vec.h"
#include <stdlib.h>
#include <string.h>

int obj_cmp(const obj_t * a, const obj_t * b)
{
  return (*a)->cmp(a,b);
}

void obj_del(obj_t * obj)
{
  if (obj)
    (*obj)->del(obj);
}

obj_t * obj_new(objif_t * iface)
{
  obj_t * obj = (obj_t *) calloc(1,iface->objsz);
  if (obj)
    *obj = iface;
  return obj;
}

vec_t * vec_new(unsigned int max, objif_t * iface)
{
  vec_t *vec = 0, *v = calloc(1, sizeof(*vec));
  assert(iface);
  if (v) {
    v->max = 0;
    v->cnt = 0;
    v->flg = 0;
    v->iface = iface;
    v->sorted = 0;
    v->obj = max ? calloc(sizeof(obj_t *),max) : 0;
    if (v->obj)
      v->max = max;
    vec = v;
    v = 0;
  }
  free(v);
  return vec;
}

void vec_del(vec_t * vec)
{
  if (vec) {
    unsigned int i;
    for (i=0; i<vec->cnt; ++i)
      obj_del(vec->obj[i]);
    free(vec->obj);
    free(vec);
  }
}

int vec_add(vec_t * vec, obj_t * obj, int flags)
{
  int index = -1;

  assert(vec);

  /* Add null object is valid call and must return an error. */
  if (obj) {

    if (flags & VEC_FNODUP) {
      index = vec_find(vec, obj, 0, 0);
      if (index != -1) {
        if (vec->sorted && vec->sorted(obj,vec->obj[index]))
          vec->sorted = 0;
        obj_del(vec->obj[index]);
        vec->obj[index] = obj;
        return index;
      }
    }

    if (vec->cnt == vec->max) {
      unsigned int need = vec->max ? vec->max*2u : 8u;
      obj_t ** newobj = realloc(vec->obj, sizeof(*newobj) * need);
      if (newobj) {
        memset(newobj+vec->cnt, 0, sizeof(*newobj)*(need-vec->cnt));
        vec->max = need;
        vec->obj = newobj;
      }
    }
    if (vec->cnt < vec->max) {
      index = vec->cnt++;
      vec->obj[index] = obj;
      vec->sorted = 0;
    }
  }

  return index;
}

obj_t * vec_pop(vec_t * vec)
{
  obj_t * o = 0;
  assert(vec);
  if (vec->cnt > 0) {
    o = vec->obj[--vec->cnt];
    vec->obj[vec->cnt] = 0;
  }
  return o;
}

obj_t * vec_get(vec_t * vec, unsigned int idx)
{
  assert(vec);
  return (idx < vec->cnt) ? vec->obj[idx] : 0;
}

static int cmp_pobj(const void * pa, const void * pb)
{
  const obj_t * a = *(const obj_t **)pa;
  const obj_t * b = *(const obj_t **)pb;

  if (a == b) return 0;
  if (!a) return 1;
  if (!b) return -1;
  assert (*a == *b);
  return (*a)->cmp(a,b);
}

void vec_sort(vec_t * vec, cmp_f cmp)
{
  cmp_f sav = 0;
  assert(vec);
  if (cmp) {
    sav = vec->iface->cmp;
    vec->iface->cmp = cmp;
  }
  if (vec->sorted != vec->iface->cmp)
    qsort(vec->obj, vec->cnt, sizeof(*vec->obj), cmp_pobj);
  vec->sorted = vec->iface->cmp;
  if (sav)
    vec->iface->cmp = sav;
}


static int vec_iter_find(vec_t * vec, const obj_t * obj, cmp_f cmp, int idx)
{
  unsigned int i, j, k = (idx < 0) ? 0 : idx;
  assert(cmp);
  idx = -1;
  for (i=j=0; i<vec->cnt; ++i)
    if (!cmp(obj, vec->obj[i]) && j++ == k) {
      idx = i;
      break;
    }
  return idx;
}

int vec_find(vec_t * vec, const obj_t * obj, cmp_f cmp, int idx)
{
  assert(vec);

  if (!cmp)
    cmp = vec->iface->cmp;

  assert(cmp);
  if (vec->cnt > 8 && cmp == vec->sorted) {
    obj_t ** pobj;
    cmp_f sav = vec->iface->cmp;
    vec->iface->cmp = cmp;
    pobj = bsearch(&obj, vec->obj, vec->cnt, sizeof(obj), cmp_pobj);
    if (pobj) {
      if (idx == -1)
        idx = pobj - vec->obj;
      else {
        int i;                          /* find first match */
        for (i = pobj-vec->obj; i > 0 && !cmp(obj, vec->obj[i-1]); --i);
        assert (i>=0 && i <vec->cnt);
        i += idx;                       /* add the index */
        idx = (i<vec->cnt && !cmp(obj, vec->obj[i])) ? i : -1;
      }
    } else
      idx = -1;
    vec->iface->cmp = sav;
  } else {
    idx = vec_iter_find(vec, obj, cmp, idx);
  }
  return idx;
}
