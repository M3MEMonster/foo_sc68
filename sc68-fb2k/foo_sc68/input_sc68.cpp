/*
 * @file    input_sc68.cpp
 * @brief   sc68 foobar200 - implement foobar input component
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (C) 2013-2014 Benjamin Gerard
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"

// declare function from vfs_sc68.cpp
vfs68_t * vfs68_fb2k_create(service_ptr_t<file> p_file, const char * p_path,
  t_input_open_reason p_reason, abort_callback & p_abort);

// replay config (set by preferences page)
volatile int g_ym_engine      = 0;     // YM engine (blep or pulse)
volatile int g_ym_filter      = 0;     // YM filter for pulse
volatile int g_ym_asid        = 0;     // aSID (off/on/force)
volatile int g_sampling_rate  = 48000; // sampling rate in Hz

// C-Tor: create sc68 instance
input_sc68::input_sc68() {
  sc68_create_t create;
  InterlockedIncrement(&g_instance);
  ZeroMemory(&create, sizeof(create));
  create.cookie = this;
#ifdef _DEBUG
  create.emu68_debug = 1;
#else
  create.emu68_debug = 0;
#endif
  create.log2mem = 19;
  create.name = m_name;
  _snprintf(m_name,sizeof(m_name),"fb2k#%05d", ++g_counter);
  create.sampling_rate = g_sampling_rate;
  m_sc68 = sc68_create(&create);
  if (!m_sc68) {
    throw exception_aborted();
  }
  sc68_cntl(m_sc68, SC68_SET_OPT_INT, "ym-engine",   g_ym_engine);
  sc68_cntl(m_sc68, SC68_SET_OPT_INT, "ym-filter",   g_ym_filter);
  sc68_cntl(m_sc68, SC68_SET_ASID,    g_ym_asid);
}

input_sc68::~input_sc68() {
  sc68_destroy(m_sc68);
  //InterlockedCompareExchangePointer(&g_playing_sc68,0,m_sc68);
  m_sc68 = 0;
  InterlockedDecrement(&g_instance);
  msg68_debug("input_sc68: %d instance remaining\n", (int)g_instance);
}

//! Opens specified file for info read / decoding / info write. This is called only once, immediately after object creation, before any other methods, and no other methods are called if open() fails.
//! @param p_filehint Optional; passes file object to use for the operation; if set to null, the implementation should handle opening file by itself. Note that not all inputs operate on physical files that can be reached through filesystem API, some of them require this parameter to be set to null (tone and silence generators for an example). Typically, an input implementation that requires file access calls input_open_file_helper() function to ensure that file is open with relevant access mode (read or read/write).
//! @param p_path URL of resource being opened.
//! @param p_reason Type of operation requested. Possible values are: \n
//! - input_open_info_read - info retrieval methods are valid; \n
//! - input_open_decode - info retrieval and decoding methods are valid; \n
//! - input_open_info_write - info retrieval and retagging methods are valid; \n
//! Note that info retrieval methods are valid in all cases, and they may be called at any point of decoding/retagging process. Results of info retrieval methods (other than get_subsong_count() / get_subsong()) between retag_set_info() and retag_commit() are undefined however; those should not be called during that period.
//! @param p_abort abort_callback object signaling user aborting the operation.
void input_sc68::open(service_ptr_t<file> p_filehint, const char * p_path, t_input_open_reason p_reason, abort_callback & p_abort)
{
  DBG("SC68 component: open <%s>\n", p_path);
  // our input does not support retagging.
  if (p_reason == input_open_info_write)
    throw exception_io_unsupported_format(); 

  vfs68_t  * is = vfs68_fb2k_create(/*m_file = */p_filehint, p_path, p_reason, p_abort);
  vfs68_open(is);
  int ret = sc68_load(m_sc68, is);
  vfs68_close(is);
  vfs68_destroy(is); is = 0;

  if (ret)  {
    throw exception_aborted();
  }

  sc68_music_info(m_sc68, &m_fileinfo, 1, 0);
  DBG("SC68 component: as loaded <%s> <%s>\n", m_fileinfo.album, m_fileinfo.title);
}
//! See: input_info_reader::get_subsong_count(). Valid after open() with any reason.
unsigned input_sc68::get_subsong_count() {
  //DBG("SC68 component: get subsong count <%d>", m_fileinfo.tracks);
  return m_fileinfo.tracks;
}

//! See: input_info_reader::get_subsong(). Valid after open() with any reason.
t_uint32 input_sc68::get_subsong(unsigned p_index) {
  //DBG("SC68 component: get subsong idx <%d>", p_index);
  return p_index+1;
}



//! See: input_info_reader::get_info(). Valid after open() with any reason.
void input_sc68::get_info(t_uint32 p_subsong, file_info & p_info, abort_callback & p_abort)
{
  sc68_music_info_t * use, tmp;
  sc68_tag_t tag;

  DBG("SC68 component: get info for track #%d\n", p_subsong);

  for (int i=0; !sc68_tag_enum(m_sc68, &tag, p_subsong, i, 0); ++i)
    DBG("SC68 component: [%02d:%s] = '%s'\n",p_subsong,tag.key,tag.val);

  if (p_subsong == m_fileinfo.trk.track)
    use = &m_fileinfo;
  else {
    use = &tmp;
    sc68_music_info(m_sc68, use, p_subsong, 0);
  }

  p_info.set_length(use->trk.time_ms/1000u);
  p_info.info_set("codec","sc68");

  p_info.info_set("encoding",use->dsk.tag[TAG68_ID_FORMAT].val);
  p_info.info_set_int("samplerate", sc68_cntl(m_sc68, SC68_GET_SPR));
  p_info.info_set_int("channels",2);
  p_info.info_set_int("bitspersample", 16);

  tag.key = TAG68_AKA;
  p_info.meta_add("artist",!sc68_tag_get(m_sc68,&tag,p_subsong,0) ? tag.val : use->artist);
  p_info.meta_add("album artist", !sc68_tag_get(m_sc68,&tag,0,0) ? tag.val : use->dsk.tag[TAG68_ID_ARTIST].val);

  p_info.meta_add("album", use->album);
  p_info.meta_add("title", use->title);
  p_info.meta_add("genre", use->trk.tag[TAG68_ID_GENRE].val);

  char traknum[3] = "00";
  traknum[0] += use->trk.track / 10;
  traknum[1] += use->trk.track % 10;
  p_info.meta_add("tracknumber", traknum);
  traknum[0] = '0' + m_fileinfo.tracks / 10;
  traknum[1] = '0' + m_fileinfo.tracks % 10;
  p_info.meta_add("totaltracks", traknum);

}

//! See: input_info_reader::get_file_stats(). Valid after open() with any reason.
t_filestats input_sc68::get_file_stats(abort_callback & p_abort)
{
  return filestats_invalid; // m_file->get_stats(p_abort);
}

void input_sc68::decode_initialize(t_uint32 p_subsong,unsigned p_flags,abort_callback & p_abort)
{
  m_subsong = p_subsong;

  sc68_cntl(m_sc68, SC68_SET_ASID, g_ym_asid);

  if (sc68_play(m_sc68, p_subsong, SC68_DEF_LOOP) < 0) {
    throw exception_aborted();
  }

  if (sc68_process(m_sc68, 0, 0) == SC68_ERROR) {
    throw exception_aborted();
  }

  m_sampling_rate = sc68_cntl(m_sc68, SC68_GET_SPR);

  if (sc68_music_info(m_sc68, &m_fileinfo , p_subsong, 0)) {
    throw exception_aborted();
  }
}

//! See: input_decoder::run(). Valid after decode_initialize().
bool input_sc68::decode_run(audio_chunk & p_chunk,abort_callback & p_abort)
{
  t_int32 buf[1024];
  const int max = sizeof(buf) / sizeof(*buf);
  //const int hz  = m_fileinfo.rate ? m_fileinfo.rate : 50; 
  //int         n = m_sampling_rate / m_fileinfo.rate;
  //if (n > max) n = max;
  int n = max;

  // InterlockedExchangePointer(&g_playing_sc68, m_sc68);

  sc68_cntl(m_sc68, SC68_SET_ASID, g_ym_asid);
  int code = sc68_process(m_sc68, buf, &n);
  if (code == SC68_ERROR)
    throw exception_aborted();
  else {
    p_chunk.set_data_fixedpoint_ex(
      buf, n << 2,
      m_sampling_rate, 2, 16,
      audio_chunk::FLAG_SIGNED | audio_chunk::FLAG_LITTLE_ENDIAN,
      audio_chunk::channel_config_stereo);
  }
  return !(code & (SC68_END|SC68_CHANGE)); // false is EOF
}

void input_sc68::decode_seek(double p_seconds,abort_callback & p_abort)
{
  const int target_ms = (int)(p_seconds * 1000.0);

  // Restart the current track from the beginning
  sc68_cntl(m_sc68, SC68_SET_ASID, g_ym_asid);

  if (sc68_play(m_sc68, m_subsong, SC68_DEF_LOOP) < 0)
    throw exception_aborted();

  if (sc68_process(m_sc68, 0, 0) == SC68_ERROR)
    throw exception_aborted();

  // Fast-forward: process audio into a discard buffer until we reach
  // the target position.  Use a large buffer to minimise the number
  // of iterations (8192 stereo samples ~ 170 ms at 48 kHz).
  t_int32 discard[8192];

  for (;;) {
    p_abort.check();                    // honour user cancellation

    int pos_ms = sc68_cntl(m_sc68, SC68_GET_POS);
    if (pos_ms >= target_ms)
      break;

    int n = sizeof(discard) / sizeof(*discard);
    int code = sc68_process(m_sc68, discard, &n);
    if (code == SC68_ERROR || (code & SC68_END))
      break;                            // reached end of track
  }
}

bool input_sc68::decode_can_seek()
{
  return true;
}

bool input_sc68::decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta)
{
  return false;
}

//! See: input_decoder::get_dynamic_info_track(). Valid after decode_initialize().
bool input_sc68::decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta)
{
  return false;
}

//! See: input_decoder::on_idle(). Valid after decode_initialize().
void input_sc68::decode_on_idle(abort_callback & p_abort)
{
}

void input_sc68::retag_set_info(t_uint32 p_subsong,const file_info & p_info,abort_callback & p_abort)
{
  DBG("SC68 component: retag set info #%d\n", p_subsong);
  throw exception_io_unsupported_format();
}

void input_sc68::retag_commit(abort_callback & p_abort)
{
  DBG("SC68 component: retag commit\n");
  throw exception_io_unsupported_format();
}

bool input_sc68::g_is_our_content_type(const char * p_content_type)
{
  bool ret = !stricmp_utf8(sc68_mimetype(), p_content_type);
  DBG("SC68 component: is our content-type: '%s' ? %s\n", p_content_type, ret ? "yes" : "no");
  return ret;
}

// TODO: add sc68:// scheme
bool input_sc68::g_is_our_path(const char * p_path,const char * p_extension)
{
  bool ret =
    !stricmp_utf8(p_extension,"sc68") ||
    !stricmp_utf8(p_extension,"sndh") ||
    !stricmp_utf8(p_extension, "snd");
  DBG("SC68 component: is our file: '%s' ? %s\n", p_path, ret ? "yes" : "no");
  return ret;
}

int input_sc68::g_counter = 0; // Counter used for naming newly created sc68 instance
volatile LONG input_sc68::g_instance = 0; // Count exiting input_sc68 instances
static input_factory_t<input_sc68> g_input_sc68_factory;

//volatile PVOID input_sc68::g_playing_sc68 = 0;

