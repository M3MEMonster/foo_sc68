/**
 * @ingroup  sourcer68_prg
 * @file     src68_fmt.h
 * @author   Benjamin Gerard
 * @date     2015-04-06
 * @brief    source formatter.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SOURCER68_FMT_H
#define SOURCER68_FMT_H

#include "src68_exe.h"
#include <stdarg.h>

enum {
  FMT_EOK = 0,
  FMT_EGENERIC,
  FMT_EARG,
  FMT_EMEM,
  FMT_EIO,
};

/**
 * Opaque formatter type.
 */
typedef struct fmt_s fmt_t;

/**
 * Create a formatter.
 *
 * @param  out  output handler (FILE * or vfs68_t *)
 * @retval 0 on error.
 * @return formatter object.
 */
fmt_t * fmt_new(void * out);

/**
 * Destroy formatter.
 *
 * @param  fmt  formatter object.
 */
void fmt_del(fmt_t * fmt);

/**
 * Close the current line
 *
 * @param  fmt     formatter object.
 * @param  always  even if line is empty.
 * @return error code
 * @retval 0 on success.
 */
int fmt_eol(fmt_t * fmt, int always);

/**
 * Set formatter tabulation mode.
 *
 * @param  fmt  formatter object.
 * @param  tabc char used for tabulation.
 * @param  tabw width of tabchar (1 for anything other than '\t').
 * @param  tabs array of position of the 4 columns.
 */
void fmt_set_tab(fmt_t * fmt, uint_t tabc, uint_t tabw, uint_t tabs[]);

/**
 * Write data to buffer.
 *
 * @param  fmt  formatter object.
 * @param  dat  data
 * @param  len  number of byte
 * @retval  0 on success
 * @retval -1 on error
 */
int fmt_cat(fmt_t * fmt, const void * dat, int len);

/**
 * Add column separator.
 *
 * @param  fmt  formatter object.
 * @param  str  string to print
 * @retval  0 on success
 * @retval -1 on error
 */
int fmt_tab(fmt_t * fmt);

/**
 * Cat a word to string buffer without parsing.
 *
 * @param  fmt  formatter object.
 * @param  str  string to print
 * @retval  0 on success
 * @retval -1 on error
 */
int fmt_puts(fmt_t * fmt,const char * str);

/**
 * Format string data to buffer.
 *
 * @param  fmt  formatter object.
 * @param  pft  printf-like format dtring
 * @retval  0 on success
 * @retval -1 on error
 */
int fmt_putf(fmt_t * fmt, const char * pft, ...);

/**
 * Format string data to buffer (variable argument version).
 *
 * @param  fmt  formatter object.
 * @param  pft  printf-like format dtring
 * @retval  0 on success
 * @retval -1 on error
 */
int fmt_vputf(fmt_t * fmt, const char * pft, va_list list);

#endif
