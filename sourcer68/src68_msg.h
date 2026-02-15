/**
 * @ingroup  sourcer68_prg
 * @file     src68_msg.h
 * @author   Benjamin Gerard
 * @date     2015-03-22
 * @brief    print messages.
 *
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef SRC68_MSG_H
#define SRC68_MSG_H

#include "src68_def.h"
#include <stdarg.h>

void emsg_va(int addr, const char * fmt, va_list list);
void emsg(int addr, const char * fmt, ...);

void wmsg_va(int addr, const char * fmt, va_list list);
void wmsg(int addr, const char * fmt, ...);

void dmsg_va(const char * fmt, va_list list);
void dmsg(const char * fmt, ...);

#endif
