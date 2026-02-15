/*
 * @file    src68_eval.c
 * @brief   expression evaluator.
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

#include "src68_eva.h"

/* internal struct */
typedef struct {
  void * usr;                           /* user cookie */
  getmem_f mem;                         /* memory function */
  getsym_f sym;                         /* symbol function */
  const char *beg, *end, *ptr, *err;
  int org, val, pri, flg;
} expr_t;

static int term(expr_t * e);
static int expr(expr_t * e);

static int is_myspace(int c) {
  return c == ' ' || c == '\t';
}

static int curchar(expr_t *e) {
  return (!e->end || e->ptr < e->end) ? *e->ptr : 0;
}

static int skipspace(expr_t *e) {
  int c;
  if (is_myspace(c=curchar(e))) {
    if (!(e->flg & EVA_SKIPSPACE)) {
      e->err = "unexpected <SPACE>";
      c = -1;
    }
    else
      while (++e->ptr, is_myspace(c=curchar(e)));
  }
  return c;
}

static int digit2(int c) {
  return ( c >= '0' && c <= '1' ) ? c-'0' : -1;
}

static int digit8(int c) {
  return ( c >= '0' && c <= '7' ) ? c-'0' : -1;
}

static int digit10(int c) {
  return ( c >= '0' && c <= '9' ) ? c-'0' : -1;
}

static int digit16(int c) {
  if (c >= '0' && c <= '9')
    c -= '0';
  else if (c >= 'a' && c <= 'f')
    c -= 'a'-10;
  else if (c >= 'A' && c <= 'F')
    c -= 'A'-10;
  else
    c = -1;
  return c;
}

static int base2(expr_t *e) {
  int v, c;
  if ((v = digit2(*e->ptr)) < 0) {
    e->err = "missing <BIN-DIGIT>";
    return -1;
  }
  while ( (c = digit2(*++e->ptr)) >= 0) {
    v = v * 2 + c;
  }
  e->val = v;
  return 0;
}

static int base8(expr_t *e) {
  int v, c;
  if ((v = digit8(*e->ptr)) < 0) {
    e->err = "missing <OCT-DIGIT>";
    return -1;
  }
  while ( (c = digit2(*++e->ptr)) >= 0) {
    v = v * 8 + c;
  }
  e->val = v;
  return 0;
}

static int base10(expr_t *e) {
  int v, c;
  if ((v = digit10(*e->ptr)) < 0) {
    e->err = "missing <DEC-DIGIT>";
    return -1;
  }
  while ( (c = digit10(*++e->ptr)) >= 0) {
    v = v * 10 + c;
  }
  e->val = v;
  return 0;
}

static int base16(expr_t *e) {
  int v, c;
  if ((v = digit16(*e->ptr)) < 0) {
    e->err = "missing <HEX-DIGIT>";
    return -1;
  }
  while ( (c = digit16(*++e->ptr)) >= 0) {
    v = v * 16 + c;
  }
  e->val = v;
  return 0;
}

static int term(expr_t *e);

static int unary_minus(expr_t *e) {
  e->val = -e->val;
  return 0;
}

static int unary_plus(expr_t *e) {
  return 0;
}

static int unary_reorg(expr_t *e) {
  e->val += e->org;
  return 0;
}

static int unary_bitnot(expr_t *e) {
  e->val = ~e->val;
  return 0;
}

static int unary_condnot(expr_t *e) {
  e->val = !e->val;
  return 0;
}

static int unary_extbyte(expr_t *e) {
  e->val = (int8_t)e->val;
  return 0;
}

static int unary_extword(expr_t *e) {
  e->val = (int16_t)e->val;
  return 0;
}

static int unary_extlong(expr_t *e) {
  e->val = (int32_t)e->val;
  return 0;
}

static int unary_indirect(expr_t *e, int bytes) {
  int i, v, addr = e->val;
  for (v=i=0; i<bytes; ++i) {
    int res = e->mem(addr+i,e->usr);
    if (res & ~255)
      return -1;
    v = (v << 8) | res;
  }
  e->val = v;
  return 0;
}

static int unary_indbyte(expr_t *e) {
  return unary_indirect(e,1);
}

static int unary_indword(expr_t *e) {
  return unary_indirect(e,2);
}

static int unary_indlong(expr_t *e) {
  return unary_indirect(e,4);
}

typedef int (*unary_f)(expr_t*);

static unary_f isunary(expr_t * e)
{
  switch (*e->ptr) {
  case '-': e->ptr++; return unary_minus;
  case '+': e->ptr++; return unary_plus;
  case '@': e->ptr++; return unary_reorg;
  case '~': e->ptr++; return unary_bitnot;
  case '!': e->ptr++; return unary_condnot;
  case 'b': e->ptr++; return unary_extbyte;
  case 'w': e->ptr++; return unary_extword;
  case 'l': e->ptr++; return unary_extlong;
  case 'B': e->ptr++; return unary_indbyte;
  case 'W': e->ptr++; return unary_indword;
  case 'L': e->ptr++; return unary_indlong;
  }
  return 0;
}

typedef struct {
  uint8_t pri;
  char    sym[2];
  int   (*fct)(int lval, expr_t *e);
} operator_t;

static int op_mul(int lval, expr_t *e) {
  e->val = lval * e->val;
  return 0;
}

static int op_div(int lval, expr_t *e) {
  if (!e->val) {
    e->err = "divide by zero";
    return -1;
  }
  e->val = lval / e->val;
  return 0;
}

static int op_mod(int lval, expr_t *e) {
  if (!e->val) {
    e->err = "divide by zero";
    return -1;
  }
  e->val = lval % e->val;
  return 0;
}


static int op_lsl(int lval, expr_t *e) {
  e->val = lval << e->val;
  return 0;
}

static int op_lsr(int lval, expr_t *e) {
  e->val = lval >> e->val;
  return 0;
}

static int op_add(int lval, expr_t *e) {
  e->val = lval + e->val;
  return 0;
}

static int op_sub(int lval, expr_t *e) {
  e->val = lval - e->val;
  return 0;
}

static int op_and(int lval, expr_t *e) {
  e->val = lval & e->val;
  return 0;
}

static int op_eor(int lval, expr_t *e) {
  e->val = lval ^ e->val;
  return 0;
}

static int op_orr(int lval, expr_t *e) {
  e->val = lval | e->val;
  return 0;
}


static operator_t operators[] = {
  { 9, {'*', 0 }, op_mul },
  { 9, {'/', 0 }, op_div },
  { 9, {'%', 0 }, op_mod },

  { 8, {'<','<'}, op_lsl },           /* in "C" invert << >> and + - */
  { 8, {'>','>'}, op_lsr },

  { 7, {'+', 0 }, op_add },
  { 7, {'-', 0 }, op_sub },

  { 6, {'&', 0 }, op_and },
  { 5, {'^', 0 }, op_eor },
  { 4, {'|', 0 }, op_orr },
  { 0, { 0 , 0 }, 0     }
};

static operator_t * isoperator(expr_t * e)
{
  operator_t * op;
  int c1, c2;
  if (c1 = curchar(e), c1 <= 0)
    return 0;
  ++e->ptr;
  c2 = curchar(e);
  --e->ptr;

  for (op = operators; op->fct; ++op) {
    if (c1 == op->sym[0]) {
      if (!op->sym[1]) {
        e->ptr += 1;
        return op;
      } else if (op->sym[1] == c2) {
        e->ptr += 2;
        return op;
      }
    }
  }
  return 0;
}

static int term(expr_t * e) {
  int c;
  unary_f unary;

  if (c = skipspace(e), c < 0)
    return c;
  if (!c) {
    e->err = "unexpected <EOS>";
    return -1;
  }

  if (c == '(') {
    int pri = e->pri;
    e->pri = 0;
    ++e->ptr;
    c = expr(e);
    if (c < 0)
      return c;
    c = skipspace(e);
    if (c != ')') {
      e->err = "missing `)'";
      return -1;
    }
    ++e->ptr;
    e->pri = pri;
    return 0;
  }

  unary = isunary(e);
  if (unary) {
    c = term(e);
    if (!c)
      c = unary(e);
    return c;
  }

  if (c == '0') {
    ++e->ptr;
    switch (curchar(e)) {
    case 'x': case 'X': ++e->ptr; return base16(e);
    case 'd': case 'D': ++e->ptr; return base10(e);
    case 'o': case 'O': ++e->ptr; return base8(e);
    case 'b': case 'B': ++e->ptr; return base2(e);
    default:
      --e->ptr;
    }
  } else if (c == '$') {
    ++e->ptr;
    return base16(e);
  } else if (c == '%') {
    ++e->ptr;
    return base2(e);
  }
  return base10(e);
}


static int expr(expr_t * e)
{
  int res;
  const char *sav = 0;

  res = term(e);

  while (!res) {
    operator_t * op;

    sav = e->ptr;
    if (res = skipspace(e), res <= 0)
      break;
    op = isoperator(e);
    if (!op) {
      res = 1;
      e->err = "<garbage>";
      break;
    }
    if (op->pri <= e->pri) {
      res = 0;
      break;
    } else {
      int lval = e->val, pri = e->pri;

      sav = e->ptr;
      e->pri = op->pri;
      res = expr(e);

      if (res < 0) {
        sav = 0;
        break;
      }
      if (op->fct(lval, e)) {
        res = -1;
        break;
      }
      sav = 0;
      e->pri = pri;
    }
  }
  if (sav)
    e->ptr = sav;
  return res;

}

static int default_fmem(int addr, void * user)
{
  switch (addr) {
  case EVA_QUERY_FLAGS: case EVA_QUERY_ORG:
    return 0;
  }
  return -1;
}

static int default_fsym(int * pval, const char ** pstr, void * user)
{
  return -1;
}

const char * eva_expr(int * pval, const char ** pstr, int maxl,
                      void * user, getmem_f fmem,  getsym_f fsym)
{
  expr_t e;
  int res;

  if (!fmem) fmem = default_fmem;
  if (!fsym) fsym = default_fsym;

  e.usr = user;
  e.beg = *pstr;
  e.end = maxl > 0 ? e.beg + maxl : 0;
  e.ptr = e.beg;
  e.val = 0;
  e.pri = 0;
  e.org = fmem(EVA_QUERY_ORG,user);
  e.mem = fmem;
  e.sym = fsym;
  e.flg = fmem(EVA_QUERY_FLAGS,user);
  e.err = 0;

  res = expr(&e);
  if (pval)
    *pval = e.val;
  *pstr = e.ptr;

  assert(res >= 0 || e.err);
  return res < 0 ? e.err : 0;
}
