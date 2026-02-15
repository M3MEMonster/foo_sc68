/**
 * @ingroup  sourcer68_prg
 * @file     src68_vec.h
 * @author   Benjamin Gerard
 * @date     2015-03-18
 * @brief    generic vector container.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_VEC_H
#define SOURCER68_VEC_H

#include "src68_def.h"

typedef struct objif_s objif_t;
typedef struct objif_s * obj_t;
typedef int (*cmp_f)(const obj_t *,const obj_t *);
typedef obj_t * (*new_f)(void);
typedef void (*del_f)(obj_t *);

struct objif_s {
  uint_t objsz;
  new_f new;
  del_f del;
  cmp_f cmp;
};

obj_t * obj_new(objif_t *iface);
void    obj_del(obj_t * obj);
int     obj_cmp(const obj_t * a, const obj_t * b);

typedef struct {
  uint_t max;
  uint_t cnt;
  uint_t flg;
  objif_t * iface;
  cmp_f sorted;
  obj_t ** obj;
} vec_t;

vec_t * vec_new(uint_t max, objif_t * iface);
void    vec_del(vec_t * vec);
int     vec_add(vec_t * vec, obj_t * obj, int flags);
obj_t * vec_pop(vec_t * vec);
obj_t * vec_get(vec_t * vec, uint_t idx);
int     vec_find(vec_t * vec, const obj_t * obj, cmp_f cmp, int idx);
void    vec_sort(vec_t * vec, cmp_f cmp);

enum {
  VEC_FNONE  = 0,
  VEC_FNODUP = 1,
};

#endif
