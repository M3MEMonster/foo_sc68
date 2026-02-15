/*
 * @file    input_sc68.h
 * @author  http://sourceforge.net/users/benjihan
 * @brief   sc68 input plugin for foobar2000 class defintion.
 *
 * Copyright (c) 1998-2015 Benjamin Gerard
 *
 * This  program is  free  software: you  can  redistribute it  and/or
 * modify  it under the  terms of  the GNU  General Public  License as
 * published by the Free Software  Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 *
 * You should have  received a copy of the  GNU General Public License
 * along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SC68FB2K_INPUT_SC68_H
#define SC68FB2K_INPUT_SC68_H

#ifdef _DEBUG
#define DBG(fmt,...) msg68_debug(fmt, ## __VA_ARGS__)
#else
#define DBG(fmt,...) for(;0;)
#endif

class input_sc68 {

public:
  char m_name[16];
  sc68_t * m_sc68;
  sc68_minfo_t m_fileinfo;
  int m_sampling_rate;
  t_uint32 m_subsong;          // current subsong for seek support

  static int g_counter;
  static volatile LONG g_instance;

  input_sc68();
  ~input_sc68();

  // static volatile PVOID g_playing_sc68;

  void open(service_ptr_t<file>,
            const char *, t_input_open_reason, abort_callback &);
  unsigned get_subsong_count();
  t_uint32 input_sc68::get_subsong(unsigned);
  void get_info(t_uint32, file_info &, abort_callback &);
  t_filestats get_file_stats(abort_callback & p_abort);
  void decode_initialize(t_uint32,unsigned,abort_callback &);
  bool decode_run(audio_chunk &,abort_callback &);
  void input_sc68::decode_seek(double,abort_callback &);
  bool decode_can_seek();
  bool decode_get_dynamic_info(file_info &, double &);
  bool decode_get_dynamic_info_track(file_info &, double &);
  void decode_on_idle(abort_callback &);
  void retag_set_info(t_uint32,const file_info &,abort_callback &);
  void retag_commit(abort_callback &);
  static bool g_is_our_content_type(const char *);
  static bool g_is_our_path(const char *,const char *);
};

extern volatile int g_ym_engine;
extern volatile int g_ym_filter;
extern volatile int g_ym_asid;
extern volatile int g_sampling_rate;

/* Apply current preferences to sc68 library and global variables.
 * Defined in pref_sc68.cpp. Called on startup and on preferences Apply. */
void apply_sc68_preferences();

#endif
