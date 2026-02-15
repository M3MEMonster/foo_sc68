/* $Id$ */

#ifndef _CDBG68_MSGLST_H_
#define _CDBG68_MSGLST_H_

#include <stdarg.h>

typedef struct {
  int n;                                /* number of message stored.    */
  int max;                              /* number of message allocated. */
  char ** msg;                          /* message buffer.              */
} msglst_t;

int  msglst_create(msglst_t * m, int max);
void msglst_destroy(msglst_t * m);
int  msglst_alloc(msglst_t * m);

void error_va(const char * fmt, va_list list);
void error(const char * fmt, ...);
void info_va(const char * fmt, va_list list);
void info(const char * fmt, ...);
void debug_va(const char * fmt, va_list list);
void debug(const char * fmt, ...);

#endif /* #ifndef _CDBG68_MSGLST_H_ */
