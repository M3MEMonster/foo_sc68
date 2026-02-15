/**
 * @ingroup  sourcer68_prg
 * @file     src68_dis.h
 * @author   Benjamin Gerard
 * @date     2015-03-18
 * @brief    disassembler and code walker.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_DIS_H
#define SOURCER68_DIS_H

#include "src68_exe.h"

/**
 * Walk options.
 */
typedef struct {
  uint_t def_maxdepth;            /**< default max depth. */
  uint_t def_maxscout;            /**< default max scout. */
  uint_t brk_on_ndef_jmp:1;       /**< stop walk on undefined jump. */
  uint_t brk_on_rts:1;            /**< don't walk after rts. */
} walkopt_t;


/**
 * Disassembler options.
 */
typedef struct {
  uint_t auto_symbol:1;    /**< produce symbol instead of address. */
  uint_t ascii_imm:1;      /**< */
  uint_t ascii_dcb:1;      /**< */
  uint_t print_opcode:1;   /**< */
  uint_t print_address:1;  /**< */

} disaopt_t;

/* void walkopt_set_default(walkopt_t * wopt); */

typedef struct {
  /**
   * @name input fields.
   * @{
   */
  uint_t    adr;                        /**< Entry point.  */
  exe_t    *exe;                        /**< Executable.   */
  walkopt_t opt;                        /**< Walk options. */

  /**
   * @}
   */

  /**
   * @name output fields.
   * @{
   */

  uint_t score;     /**< consistency score. */
  uint_t depth;     /**< maximum depth. */
  uint_t inscnt;    /**< total number of disassembled instructions. */

  /**
   * @}
   */

} walk_t;

/**
 * Start a walk.
 *
 * @param  walk
 */
int dis_walk(walk_t * walk);

/**
 * Disasemble an instruction.
 *
 * @paran  exe
 * @paran  adr
 * @paran  buf
 * @paran  max
 * @return instruction type (@see desa68_inst_types)
 */
int dis_disa(exe_t * exe, uint_t * adr, char * buf, uint_t max);

#endif
