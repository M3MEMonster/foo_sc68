/**
 * @ingroup   sc68_directshow
 * @file      ds68_dbg.h
 * @brief     Debug facilities
 * @author    Benjamin Gerard
 * @date      2014/06
 */

#pragma once

/**
 * @addtogroup sc68_directshow
 * @{
 */

#ifndef SC68_SPLITTER_DBG_H
#define SC68_SPLITTER_DBG_H

#include <stdarg.h>
#include <wchar.h>

void dbg(const char * fmt, ...);
void dbg_va(const char * fmt, va_list list);

void dbg(const wchar_t * fmt, ...);
void dbg_va(const wchar_t * fmt, va_list list);

void dbg_error(int err, const char * fmt = 0, ...);
void dbg_error(int err, const wchar_t * fmt, ...);

extern "C" void msg_for_sc68(int bit, void *, const char * fmt, va_list list);

#ifndef NOP
# define NOP while (0)
#endif

#ifdef _DEBUG
# define ERR(E,FMT,...) dbg_error(E, FMT,  ## __VA_ARGS__)
# define DBG(FMT,...)   dbg(FMT,  ## __VA_ARGS__)
# define DBGVA(FMT,LST) dbg_va(FMT,LST)
#else
# define ERR(E,FMT,...)  NOP
# define DBG(FMT,...)    NOP
# define DBGVA(FMT,LST)  NOP
#endif

#endif

/**
 * @}
 */
