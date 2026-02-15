/*
 * GStreamer - sc68 typefinder plugin
 * Copyright (c) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (c) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (c) 2011 Benjamin Gerard <http://sourceforge.net/users/benjihan>
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gst/gst.h>
#include <string.h>
#include <sc68/file68.h>

/***********************************************************************
 * TYPE DETECTION
 *
 *   We should probably put this function in a standalone plugin to
 *   avoid loading the whole sc68 library just for type detection.
 *
 ***********************************************************************/
static void gst_sc68_typefind(GstTypeFind *tf, gpointer cookie)
{
  guint8 * data = gst_type_find_peek(tf, 0, 32);
  guint score = 0;
  if (data) {
    /* Official sc68 file headers. */
    if (!memcmp(data, "SC68 M",6))
      score = GST_TYPE_FIND_MAXIMUM;
    /* ice! are likely sndh files. */
    else if (!memcmp(data, "ICE!",4))
      score = GST_TYPE_FIND_LIKELY;
    /* gzip might be sc68 ... gzip filter would be much better */
    else if (!memcmp(data, "gzip",4))
      score = (GST_TYPE_FIND_MINIMUM+GST_TYPE_FIND_POSSIBLE) >> 1;
    /* raw sndh (experimental) */
    else if ( (data[0] == 0x4e && (data[1] == 0x75 || data[1] == 0x71)) ||
              (data[0] == 0x60 ) ) {
      int i;
      for (i=6; i<28; ++i)
        if (!memcmp(data+i, "SNDH", 4)) {
          score = GST_TYPE_FIND_LIKELY;
          break;
        }
    }
  }
  if (score)
    gst_type_find_suggest(tf, score,
                          gst_caps_new_simple(SC68_MIMETYPE, NULL));
}

/***********************************************************************
 * PLUGIN INIT
 *
 * called as soon as the plugin is loaded
 ***********************************************************************/
/* register sc68 type finder function
 ***********************************************************************/
static gboolean gst_plugin_init (GstPlugin * plugin)
{
  static gchar *exts[] = {
    "sc68", "sc68.gz", "sndh", "snd", NULL
  };
  static const char mimetype[] = SC68_MIMETYPE;
  const GstCaps * caps = gst_caps_new_simple(mimetype, NULL);

  /* Register our file type detection. */
  return
    gst_type_find_register(
      plugin,                           /* plugin             */
      mimetype,                         /* use mime type as name ??? */
      GST_RANK_PRIMARY,                 /* rank               */
      gst_sc68_typefind,                /* find function      */
      exts,                             /* extension list     */
      caps,                             /* possible caps      */
      NULL,                             /* data (cookie)      */
      NULL                              /* Destroy notify     */
      );
}


/***********************************************************************
 * PLUGIN DEFINITION FOR REGISTRATION
 ***********************************************************************/

GST_PLUGIN_DEFINE (
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "sc68-type",
  "sc68 and compatible file finder",
  gst_plugin_init,
  VERSION,
  "LGPL",
  PACKAGE_NAME,
  PACKAGE_URL "/" PACKAGE_NAME
  )
