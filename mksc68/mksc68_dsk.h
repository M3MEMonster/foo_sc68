/**
 * @ingroup  mksc68_prg
 * @file     mksc68/mksc68_dsk.h
 * @author   Benjamin Gerard
 * @date     2009-01-01
 * @brief    disk function
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef MKSC68_DSK_H
#define MKSC68_DSK_H

#include "mksc68_def.h"

EXTERN68 int dsk_new(const char * name, int force);
EXTERN68 int dsk_load(const char * uri, int merge, int force);
EXTERN68 int dsk_merge(const char * uri);
EXTERN68 int dsk_save(const char * uri, int version, int gzip);
EXTERN68 int dsk_kill(void);

EXTERN68 int    dsk_has_disk(void);
EXTERN68 void * dsk_get_disk(void);
EXTERN68 int    dsk_get_tracks(void);

EXTERN68 int dsk_trk_get_current(void);
EXTERN68 int dsk_trk_set_current(int track);
EXTERN68 int dsk_trk_get_default(void);
EXTERN68 int dsk_trk_set_default(int track);

EXTERN68 const char * dsk_tag_get(int trk, const char * var);
EXTERN68 const char * dsk_tag_set(int trk, const char * var, const char * val);

EXTERN68 int  dsk_validate(void);
EXTERN68 int  dsk_tag_geti(int trk, const char * var);
EXTERN68 int  dsk_tag_seti(int trk, const char * var, int val);
EXTERN68 void dsk_tag_del(int trk, const char * var);
EXTERN68 void dsk_tag_clr(int trk);

typedef struct dsk_play_s dsk_play_t;

EXTERN68 int dsk_play(dsk_play_t * params);
EXTERN68 int dsk_stop(void);
EXTERN68 int dsk_playing(void);

#endif

