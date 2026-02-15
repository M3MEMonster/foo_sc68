/*
 * @file    vfs_sc68.c
 * @author  http://sourceforge.net/users/benjihan
 * @brief   implements vfs68 VFS for foobar2000
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
#include <sc68/file68_vfs.h>
#include <sc68/file68_vfs_def.h>

using namespace foobar2000_io;

/** vfs file structure. */
struct vfs68_fb2k {
  vfs68_fb2k(
    service_ptr_t<file> p_file, const char * p_path,
    t_input_open_reason p_reason, abort_callback & p_abort);
  ~vfs68_fb2k();

  vfs68_t         vfs;
  service_ptr_t<file> file;
  abort_callback     &abort;
  t_input_open_reason reason;
  char               *path;
};

typedef struct vfs68_fb2k vfs68_fb2k_t;

static const char * isn_name(vfs68_t * vfs)
{
  vfs68_fb2k_t * isn = (vfs68_fb2k_t *)vfs;
  return isn->path;
}

static int isn_open(vfs68_t * vfs)
{
  vfs68_fb2k_t * isn = (vfs68_fb2k_t *)vfs;
  input_open_file_helper(isn->file, isn->path, isn->reason, isn->abort);
  return 0;
}

static int isn_close(vfs68_t * vfs)
{
  vfs68_fb2k_t * isn = (vfs68_fb2k_t *)vfs;
  isn->file.release();
  return 0;
}

static int isn_read(vfs68_t * vfs, void * data, int n)
{
  vfs68_fb2k_t * isn = (vfs68_fb2k_t *)vfs;
  return isn->file->read(data, (int)n, isn->abort);
}

static int isn_write(vfs68_t * vfs, const void * data, int n)
{
  vfs68_fb2k_t * isn = (vfs68_fb2k_t *)vfs;
  isn->file->write(data, (int)n, isn->abort);
  return n;
}

static int isn_flush(vfs68_t * vfs)
{
  return 0;
}

static int isn_length(vfs68_t * vfs)
{
  vfs68_fb2k_t * isn = (vfs68_fb2k_t *)vfs;
  t_filesize size = isn->file->get_size(isn->abort);
  return (size != filesize_invalid)
    ? (int) size
    : -1
    ;
}

static int isn_tell(vfs68_t * vfs)
{
  vfs68_fb2k_t * isn = (vfs68_fb2k_t *)vfs;
  t_filesize pos = isn->file->get_position(isn->abort);
  return (pos != filesize_invalid)
    ? (int) pos
    : -1
    ;
}

static int isn_seek(vfs68_t * vfs, int offset)
{
  vfs68_fb2k_t * isn = (vfs68_fb2k_t *)vfs;
  int pos = isn_tell(vfs);

  if (pos == -1)
    return -1;
  isn->file->seek(pos + offset, isn->abort);
  return 0;
}

static void isn_destroy(vfs68_t * vfs)
{
  isn_close(vfs);
  delete vfs;
}

vfs68_fb2k::vfs68_fb2k(
  service_ptr_t<foobar2000_io::file> p_file, const char * p_path,
  t_input_open_reason p_reason, abort_callback & p_abort)
  : abort(p_abort), reason(p_reason), file(p_file)
{
  vfs.name    = isn_name;
  vfs.open    = isn_open;
  vfs.close   = isn_close;
  vfs.read    = isn_read;
  vfs.write   = isn_write;
  vfs.flush   = isn_flush;
  vfs.length  = isn_length;
  vfs.tell    = isn_tell;
  vfs.destroy = isn_destroy;
  vfs.seekb   = isn_seek;
  vfs.seekf   = isn_seek;

  path = p_path ? _strdup(p_path) : 0;
}

vfs68_fb2k::~vfs68_fb2k()
{
  free(path);
}

vfs68_t * vfs68_fb2k_create(
  service_ptr_t<file> p_file, const char * p_path,
  t_input_open_reason p_reason, abort_callback & p_abort)
{
  vfs68_fb2k_t *isn = new vfs68_fb2k_t(p_file, p_path, p_reason, p_abort);
  if (!isn)
    return 0;
  return &isn->vfs;
}
