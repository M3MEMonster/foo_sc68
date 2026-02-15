/**
 * @ingroup  sourcer68_prg
 * @file     src68_exe.h
 * @author   Benjamin Gerard
 * @date     2015-03-18
 * @brief    executable loader.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_EXE_H
#define SOURCER68_EXE_H

#include "src68_def.h"
#include "src68_sym.h"
#include "src68_mbk.h"
#include "src68_sec.h"
#include "src68_adr.h"
#include "src68_rel.h"

/**
 * Executable types.
 * @anchor src68_load_as
 */
enum {
  LOAD_AS_AUTO,                         /**< automatically select. */
  LOAD_AS_BIN,                          /**< binary */
  LOAD_AS_TOS,                          /**< Atari TOS.  */
  LOAD_AS_SC68                          /* *< sc68 compatible file. */
};

enum {
  EXE_ORIGIN = 0x10000,                 /**< Default origin address. */
  EXE_DEFAULT = -1                      /**< Used default origin. */
};

/**
 * Loaded executable.
 */
typedef struct {
  char    * uri;                        /**< source URI. */
  /**/
  uint8_t loadas;                       /**< loaded as.  */
  uint8_t ispic;                        /**< position independant code ? */
  /**/
  vec_t   * sections;                   /**< sections container. */
  vec_t   * symbols;                    /**< symbols container.  */
  vec_t   * relocs;                     /**< relocations container. */
  vec_t   * entries;                    /**< entry points container. */
  /**/
  mbk_t   * mbk;                        /**< memory block. */
} exe_t;

/**
 * Load an executable to disassemble.
 *
 * @param  uri   file path or IRI.
 * @param  org   address to load the file at.
 * @param  type  Load as type.
 * @return pointer to loaded executable.
 * @retval 0  on error
 */
exe_t * exe_load(char * uri, uint_t org, int loadas);

/**
 * Free executable.
 *
 * @param  exe  executable to free.
 */
void exe_del(exe_t * exe);

#endif
