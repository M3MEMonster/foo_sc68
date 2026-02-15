/**
 * @ingroup  sourcer68_prg
 * @file     src68_eval.h
 * @author   Benjamin Gerard
 * @date     2015-03-18
 * @brief    expression evaluation.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_EVA_H
#define SOURCER68_EVA_H

#include "src68_def.h"

enum {
  /** use this with getmem_f() to get program origin address. */
  EVA_QUERY_ORG = -1,
  /** use this with getmem_f() to get eval flags. */
  EVA_QUERY_FLAGS = -2,

  /** flags for src68_eval() parser to ignore spaces. */
  EVA_SKIPSPACE = 1,
};

/**
 * Function type providing a way for the expression evaluation to
 * retrieve indirect memory value.
 *
 * @param  addr  address of byte to read (or SRC68_EVAL_QUERY_ORG)
 * @param  user  user cookie
 * @return byte value at address addr or memory origin
 * @retval -1 on error (out of range)
 */
typedef int (*getmem_f)(int addr, void * user);

/**
 * Function type to providing a way for the expression evaluation to
 * retrieve  symbol value.
 *
 * @param  pval  pointer to store the resulting symbol value
 * @param  pstr  input/output the string to parse
 * @param  user  user cookie
 * @return  byte value at address addr
 * @retval  0  on success
 * @retval  -1 on error.
 */
typedef int (*getsym_f)(int * pval, const char ** pstr, void * user);

/**
 * Evaluate an expression.
 *
 * @param  pval  pointer to store the result (can be null)
 * @param  pstr  input/output the string to parse
 * @param  maxl  maximum length to parse (0:no limit)
 * @param  user  user private data (for fmem() and fsym() calls)
 * @param  fmem  memory access function
 * @param  fsym  symbol access function
 *
 * @retval  0 on success
 * @retval  1 on partial success (have a result but not fully parsed)
 * @retval -1 on error
 */
const char * eva_expr(int * pval, const char ** pstr, int maxl,
                      void * user, getmem_f fmem,  getsym_f fsym);

#endif
