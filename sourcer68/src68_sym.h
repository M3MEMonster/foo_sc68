/**
 * @ingroup  sourcer68_prg
 * @file     src68_sym.h
 * @author   Benjamin Gerard
 * @date     2015-03-18
 * @brief    symbol container.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_SYM_H
#define SOURCER68_SYM_H

#include "src68_vec.h"

/**
 * Symbol flags.
 */
enum {
  SYM_INST = 1<<0,        /**< is a disassembled instruction. */
  SYM_BRA  = 1<<1,        /**< is jump/branch to. */
  SYM_BSR  = 1<<2,        /**< is a subroutine. */
  SYM_DATA = 1<<3,        /**< is data  */
  SYM_BYTE = 1<<4,        /**< byte access. */
  SYM_WORD = 1<<5,        /**< word access. */
  SYM_LONG = 1<<6,        /**< long access. */
  SYM_XTRN = 1<<8,        /**< External symbol */
  SYM_WEAK = 1<<9,        /**< Weak symbol */
};

/**
 * Symbol structure.
 */
typedef struct  {
  obj_t  obj;                         /**< object interface. */
  char  *name;                        /**< symbol name.  */
  uint_t addr;                        /**< symbol address or value. */
  uint_t flag;                        /**< symbol flags.  */
} sym_t;

/**
 * Create a symbol container.
 *
 * @param  max  Initial maximum number of symbol
 * @retval 0 on error
 * @return symbol container.
 */
vec_t * symbols_new(uint_t max);

/**
 * Add a new symbol. Adding a symbol might invalidated other symbol
 * index and/or values.
 *
 * @param  name  a name for the symbol (to be copied)
 * @param  addr  address of symbol start
 * @param  flag  set of flags for the symbol.
 *
 * @return symbol
 * @retval 0 on error
 */
sym_t * symbol_new(const char * name, uint_t addr, uint_t flag);

/**
 * Get symbol at index.
 */
sym_t * symbol_get(vec_t * symbs, int index);

/**
 * Create and add a symbol.
 *
 * @param  symbs  the symbol container
 * @param  name   a name for the symbol (to be copied)
 * @param  addr   symbol address (value)
 * @param  flag   set of flags for the symbol.
 *
 * @return added symbol index
 * @retval -1 on error
 */
int symbol_add(vec_t * symbs,
               const char * name, uint_t addr, uint_t flag);

/**
 */
void symbol_sort_byname(vec_t * symbs);

/**
 */
void symbol_sort_byaddr(vec_t * symbs);

/**
 * Find symbol by name.
 */
int symbol_byname(vec_t * symbs, const char * name);

/**
 * Find symbol by address.
 */
int symbol_byaddr(vec_t * symbs, uint_t addr, int idx);

#endif
