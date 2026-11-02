#pragma once

#include "stdafx.h"
#include "GUID.h"

inline constexpr int default_spr = 48000;
inline constexpr int default_time = 180;
inline constexpr int default_asid = 0;        /* off */
inline constexpr int default_ym_model = 0;      /* blep */
inline constexpr int default_ym_filter = 0;      /* 2-poles */
inline constexpr int default_ym_volmodel = 0;    /* atari */
inline constexpr int default_amiga_blend = 128;     /* center */
inline constexpr bool default_amiga_filter = false;/* off */
inline constexpr int default_spr_idx = 4;

extern cfg_int cfg_spr;
extern cfg_int cfg_deftime;
extern cfg_int cfg_asid;
extern cfg_int cfg_ym_model;
extern cfg_int cfg_ym_filter;
extern cfg_int cfg_ym_volmodel;
extern cfg_int cfg_amiga_blend;
extern cfg_bool cfg_amiga_filter;
