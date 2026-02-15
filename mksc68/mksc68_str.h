/**
 * @ingroup  mksc68_prg
 * @file     mksc68_str.h
 * @author   Benjamin Gerard
 * @date     2009-01-01
 * @brief    Various string functions.
 *
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef MKSC68_STR_H
#define MKSC68_STR_H

#include "mksc68_def.h"

EXTERN68 const char * str_fileext(const char * path);
EXTERN68 int    str_tracklist(const char ** ptr_tl, int * from, int * to);
EXTERN68 int    str_time_stamp(const char ** ptr_tl, int * ms);
EXTERN68 int    str_time_range(const char ** ptr_tl, int * from, int * to);
EXTERN68 char * str_timefmt(char * buf, int len, unsigned int ms);
EXTERN68 char * str_hardware(char * const buf, int max, int hw);

#endif
