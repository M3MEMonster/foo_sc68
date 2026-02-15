/**
 * @ingroup  mksc68_prg
 * @file     mksc68/mksc68_tag.h
 * @author   Benjamin Gerard
 * @date     2009-01-01
 * @brief    metatags.
 *
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef _MKSC68_TAG_H_
#define _MKSC68_TAG_H_

#include "mksc68_def.h"

EXTERN68 int     tag_std(int idx, const char ** var, const char ** des);
EXTERN68 int     tag_max(void);
EXTERN68 int     tag_enum(int trk, int idx, const char ** var, const char ** val);
EXTERN68 void    tag_del(int trk, const char * var);
EXTERN68 void    tag_clr(int trk, const char * var);
EXTERN68 void    tag_del_trk(int trk);
EXTERN68 void    tag_clr_trk(int trk);
EXTERN68 void    tag_clr_all(void);
EXTERN68 void    tag_del_all(void);
EXTERN68 const char * tag_get(int trk, const char * var);
EXTERN68 const char * tag_set(int trk, const char * var, const char * val);

#endif

