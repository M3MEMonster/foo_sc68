/*
 * GStreamer
 * Copyright (c) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (c) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (c) 2011 Bennjamin Gerard <benjihan -4t- sourceforge>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GST_SC68_H__
#define __GST_SC68_H__

/* gst includes */
#include <gst/gst.h>

/* sc68 includes */
#include <sc68/file68.h>
#include <sc68/sc68.h>
#define SC68_CAPS SC68_MIMETYPE


G_BEGIN_DECLS

/* Debug */
#define GST_CAT_DEFAULT gst_sc68_debug
GST_DEBUG_CATEGORY_EXTERN(GST_CAT_DEFAULT);

/* #defines don't like whitespacey bits */
#define GST_TYPE_SC68 \
  (gst_sc68_get_type())
#define GST_SC68(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_SC68,Gstsc68))
#define GST_SC68_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_SC68,Gstsc68Class))
#define GST_IS_SC68(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_SC68))
#define GST_IS_SC68_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_SC68))

typedef struct _Gstsc68       Gstsc68;
typedef struct _Gstsc68Class  Gstsc68Class;
typedef struct _Gstsc68Buffer Gstsc68Buffer;

/* struct _Gstsc68Buffer { */
/* //  gboolean    in_use;                   /\* currently in use *\/ */
/*   int         id; */
/*   GstBuffer * gstbuffer;                  /\*  *\/ */
/*   guint8    * malloc_data; */
/*   GFreeFunc   free_func; */
/* }; */

struct _Gstsc68
{
  /* GST magic */
  GstElement element;

  /* Pads */
  GstPad   * sinkpad;
  GstPad   * srcpad;

  /* Properties */
  struct {
    gint  track;    /* W:default track R:current track  */
    gint  loop;     /* W:default loop  R:current loop   */
    gint  rate;     /* R/W:sampling rate                */
  } prop;

  /* sc68 engine */
  sc68_t   * sc68;                      /* sc68 engine instance  */
  sc68_music_info_t info;               /* current info          */

  guint64    samples;                   /* sample counter        */
  guint      buffer_frames;

  GstClockTime start;                   /* start time            */
  gboolean   single_track;              /* play just this track  */

  int        code;                      /* last sc68 return code */
  int        status;                    /* task status           */

  GstClockTime stamp;               /* timestamp of last play call. */
  gint64     position_ns;
  gint64     duration_ns;

  GstCaps   * bufcaps;                   /* buffer caps           */

  //  GstBuffer * buffers[8];

  /* sc68 file buffer */
  gint8 * filebuf;
  int     filemax;
  int     fileptr;
  int     filelen;

};

struct _Gstsc68Class
{
  GstElementClass parent_class;
};

GType gst_sc68_get_type (void);

/* gsc68_core.c */
gboolean gst_sc68_lib_init       (void);
gboolean gst_sc68_lib_is_init    (void);
void     gst_sc68_lib_shutdown   (void);

void     gst_sc68_flush_error    (Gstsc68 * filter);
void     gst_sc68_report_error   (Gstsc68 * filter);

gboolean gst_sc68_create_engine  (Gstsc68 * filter);
void     gst_sc68_shutdown_engine(Gstsc68 * filter);

gboolean gst_sc68_load_mem       (Gstsc68 * filter, void * data , int size);
gboolean gst_sc68_load_buf       (Gstsc68 * filter, GstBuffer * buffer);
gboolean gst_sc68_caps           (Gstsc68 * filter);
gboolean gst_sc68_onchangetrack  (Gstsc68 * filter);
guint64  gst_sc68_mstons         (guint ms);
guint    gst_sc68_nstoms         (guint64 ns);

G_END_DECLS

#endif /* __GST_SC68_H__ */
