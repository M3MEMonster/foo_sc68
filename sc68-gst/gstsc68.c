/*
 * GStreamer - sc68 decoder element
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

/**
 * SECTION:element-sc68
 *
 * sc68 - Atari ST and Amiga music player
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! sc68 ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include "gstsc68.h"

GST_DEBUG_CATEGORY(gst_sc68_debug);

/* Player status */
enum {
  GST68_ERROR = -1,
  GST68_INIT  = 0,
  GST68_LOAD,
  GST68_PLAY,
  GST68_PAUSE,
  GST68_STOP,
  GST68_SHUTDOWN
};

/* Filter signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};


/***********************************************************************
 * PROPERTIES
 ***********************************************************************/
enum {
  PROP_0,
  PROP_TRACK,                       /* current track                */
  PROP_LOOP,                        /* number of loop               */
  PROP_TRACKS,                      /* number of track              */
  PROP_SPLRATE,                     /* default sampling rate        */
#if 0
  PROP_CPUMEM,                      /* size of 68k memory (2^N)     */
  PROP_YMCHANS,                     /* which YM channels to render  */
  PROP_YMENGINE,                    /* which YM engine (blep/pulse) */
  PROP_YM_FILTER,                   /* which filter                 */
#endif
};

GST_BOILERPLATE(Gstsc68, gst_sc68, GstElement, GST_TYPE_ELEMENT)

/***********************************************************************
 * Function declaration
 ***********************************************************************/
static void          gst_sc68_set_property(GObject * object, guint prop_id,
                                           const GValue * value, GParamSpec * pspec);
static void          gst_sc68_get_property(GObject * object, guint prop_id,
                                           GValue * value, GParamSpec * pspec);

static gboolean      gst_sc68_set_caps     (GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_sc68_chain        (GstPad * pad, GstBuffer * buf);
static gboolean      gst_sc68_event        (GstPad * pad, GstEvent *event);
static const GstQueryType *gst_sc68_query_types(GstPad *pad);
static gboolean      gst_sc68_query        (GstPad * pad, GstQuery *query);

static gboolean      gst_sc68_activate     (GstPad * pad);
static gboolean      gst_sc68_activate_push(GstPad * pad, gboolean active);
static gboolean      gst_sc68_activate_pull(GstPad * pad, gboolean active);

static GstFlowReturn gst_sc68_get_range    (GstPad * pad, guint64 offset,
                                            guint length, GstBuffer **buffer);
static gboolean      gst_sc68_check_get_range(GstPad * pad);
//static void          gst_sc68_typefind     (GstTypeFind * tf, gpointer data);

/***********************************************************************
 * PAD TEMPLATES
 ***********************************************************************/
static GstStaticPadTemplate sink_factory =
  GST_STATIC_PAD_TEMPLATE(
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(SC68_MIMETYPE)
    );

static GstStaticPadTemplate src_factory =
  GST_STATIC_PAD_TEMPLATE(
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(
      "audio/x-raw-int, "
      "signed     = (bool) TRUE, "
      "width      = (int)  16, "
      "depth      = (int)  16, "
      "endianness = (int)  BYTE_ORDER, "
      "channels   = (int)  2, "
      "rate       = (int)  [ 8000, 96000 ]")
    );

/***********************************************************************
 * BASE INIT
 *
 * meant to initialize class and child class properties during each
 * new child class creation
 ***********************************************************************/
static void gst_sc68_base_init(gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS(gclass);
  gst_element_class_set_details_simple(
    /* klass          */ element_class,
    /* longname       */ "SC68 Audio Decoder",
    /* classification */ "Decoder/Audio",
    /* description    */ "Atari ST and Amiga audio decoder based on sc68 engine",
    /* author         */ "Benjamin Gerard <http://sourceforge.net/users/benjihan>");
  gst_element_class_add_pad_template(
    element_class,gst_static_pad_template_get(&sink_factory));
  gst_element_class_add_pad_template(
    element_class,gst_static_pad_template_get(&src_factory));
}

/***********************************************************************
 * CLASS INIT
 *
 * initialise the class only once (specifying what signals, arguments
 * and virtual functions the class has and setting up global state)
 ***********************************************************************/
static void gst_sc68_class_init(Gstsc68Class * klass)
{
  GObjectClass    * gobject_class     = (GObjectClass    *) klass;

  /****************************
   * Install class properties *
   ****************************/
  gobject_class->set_property = gst_sc68_set_property;
  gobject_class->get_property = gst_sc68_get_property;

  g_object_class_install_property(
    gobject_class, PROP_TRACK,
    g_param_spec_int(
      "track",                          /* name */
      "track",                          /* nick */
      "Get/Set track number",           /* blurb */
      -1, 99, 0,                        /* min/max/def */
      G_PARAM_READWRITE)
    );

  g_object_class_install_property(
    gobject_class, PROP_LOOP,
    g_param_spec_int(
      "loop",                          /* name */
      "loop",                          /* nick */
      "Get/Set loop counter",          /* blurb */
      -1, 99, -1,                      /* min/max/def */
      G_PARAM_READWRITE)
    );

  g_object_class_install_property(
    gobject_class, PROP_SPLRATE,
    g_param_spec_int(
      "rate",                          /* name */
      "rate",                          /* nick */
      "Get/Set sampling rate",         /* blurb */
      8000, 96000, 44100,              /* min/max/def */
      G_PARAM_READWRITE)
    );

  g_object_class_install_property(
    gobject_class, PROP_TRACKS,
    g_param_spec_int(
      "tracks",                          /* name */
      "tracks",                          /* nick */
      "Number of sub-track",             /* blurb */
      1, 99, 1,                          /* min/max/def */
      G_PARAM_READABLE)
    );

  /*****************************
   * Install private meta-tags *
   *****************************/

  /* gst_tag_register ("ripper", GST_TAG_FLAG_META, */
  /*                G_TYPE_STRING, */
  /*                _("my own tag"), */
  /*                _("a tag that is specific to my own element"), */
  /*                NULL); */

  /* Init sc68 library. */
  gst_sc68_lib_init();
}


/***********************************************************************
 * ELEMENT INIT
 *
 * used to initialise a specific instance of this type
 ***********************************************************************/
/* - initialize the new element
 * - instantiate pads and add them to element
 * - set pad calback functions
 * - initialize instance structure
 */
static void gst_sc68_init(Gstsc68 * filter, Gstsc68Class * gclass)
{
  GstPad * pad;

  filter->status = GST68_INIT;

  /*********************
   * Init SINK element *
   *********************/
  pad = gst_pad_new_from_static_template(&sink_factory, "sink");
  gst_pad_use_fixed_caps(pad);
  gst_pad_set_caps(pad, gst_static_pad_template_get_caps(&sink_factory));
  /* gst_pad_set_chain_function              (pad, GST_DEBUG_FUNCPTR(gst_sc68_chain)); */
  gst_pad_set_event_function              (pad, GST_DEBUG_FUNCPTR(gst_sc68_event));
  gst_pad_set_activate_function           (pad, GST_DEBUG_FUNCPTR(gst_sc68_activate));
  gst_pad_set_activatepull_function       (pad, GST_DEBUG_FUNCPTR(gst_sc68_activate_pull));
  /* gst_pad_set_activatepush_function       (pad, GST_DEBUG_FUNCPTR(gst_sc68_activate_push)); */

  filter->sinkpad = pad;
  gst_element_add_pad(GST_ELEMENT(filter), pad);

  /***********************
   * Init SOURCE element *
   ***********************/
  pad = gst_pad_new_from_static_template(&src_factory, "src");
  /* gst_pad_set_getcaps_function(pad, */
  /*                              GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps)); */

  // !!! CAN'T SET CHAIN TO SOURCE PAD
  // gst_pad_set_chain_function              (pad, GST_DEBUG_FUNCPTR(gst_sc68_chain));

  gst_pad_set_setcaps_function            (pad, GST_DEBUG_FUNCPTR(gst_sc68_set_caps));
  gst_pad_set_event_function              (pad, GST_DEBUG_FUNCPTR(gst_sc68_event));

  gst_pad_set_query_type_function         (pad, GST_DEBUG_FUNCPTR(gst_sc68_query_types));
  gst_pad_set_query_function              (pad, GST_DEBUG_FUNCPTR(gst_sc68_query));


  /* gst_pad_set_activate_function           (pad, GST_DEBUG_FUNCPTR(gst_sc68_activate)); */
  /* gst_pad_set_activatepull_function       (pad, GST_DEBUG_FUNCPTR(gst_sc68_activate_pull)); */
  /* gst_pad_set_activatepush_function       (pad, GST_DEBUG_FUNCPTR(gst_sc68_activate_push)); */
  /* gst_pad_set_checkgetrange_function      (pad, GST_DEBUG_FUNCPTR(gst_sc68_check_get_range)); */

  filter->srcpad = pad;
  gst_element_add_pad(GST_ELEMENT(filter), pad);

  /**********************
   * Init CLASS element *
   **********************/
  filter->filebuf = 0;
  filter->filemax = 0;
  filter->fileptr = 0;
  memset(&filter->info,0,sizeof(filter->info));

  filter->prop.track = 0;
  filter->prop.loop  = -1;
  filter->prop.rate  = 44100;

  //memset(filter->buffers, 0, sizeof(filter->buffers));

  if (!gst_sc68_create_engine(filter))
    filter->status = GST68_ERROR;

}

static gboolean gst_sc68_get_buffer(Gstsc68 * filter, GstBuffer ** bufptr)
{
  const gint frames = filter->buffer_frames;
  GstFlowReturn flow;

  *bufptr = NULL;
  flow =
    gst_pad_alloc_buffer_and_set_caps(/* pad    */ filter->srcpad,
                                      /* offset */ filter->samples,
                                      /* size   */ frames << 2,
                                      /* caps   */ gst_caps_ref(filter->bufcaps),
                                      /* buffer */ bufptr);
  if (flow != GST_FLOW_OK) {
    GST_ERROR_OBJECT(filter,"failed to alloc new buffer [%s]", gst_flow_get_name(flow));
    return FALSE;
  }
  return TRUE;
}

static gboolean gst_sc68_task_load(Gstsc68 * filter)
{
  gboolean res = FALSE;

  GstBuffer * buf = NULL;
  GstQuery  * q   = NULL;
  gint64      seg_start, seg_stop;
  gdouble     seg_rate;
  GstFormat   seg_fmt;
  GstFlowReturn flow;

  /* GST_ELEMENT_ERROR (modplug, STREAM, DECODE, (NULL), */
  /*                    ("Unable to load song")); */


  q = gst_query_new_segment(GST_FORMAT_BYTES);
  if (!q) {
    GST_ERROR_OBJECT(filter, "failed to create new segment query");
    goto exit;
  }
  if (!gst_pad_peer_query(filter->sinkpad, q)) {
    GST_ERROR_OBJECT(filter, "failed to query new segment");
    goto exit;
  }
  gst_query_parse_segment(q, &seg_rate, &seg_fmt, &seg_start, &seg_stop);
  if (seg_fmt != GST_FORMAT_BYTES || seg_start != 0 || seg_stop < 256) {
    GST_ERROR_OBJECT(filter, "invalid segment <%s> <%d-%d>",
                     gst_format_get_name(seg_fmt),    (int)seg_start,(int)seg_stop);
    goto exit;
  }
  if (!gst_pad_check_pull_range(filter->sinkpad)) {
    GST_ERROR_OBJECT(filter, "pad <%s> can't pull range", gst_pad_get_name(filter->sinkpad));
    goto exit;
  }
  flow = gst_pad_pull_range(filter->sinkpad, seg_start, seg_stop-seg_start, &buf);
  if (GST_FLOW_OK != flow) {
    GST_DEBUG_OBJECT (filter, "pad <%s> pull range failed [%s]",
                      gst_pad_get_name(filter->sinkpad),gst_flow_get_name(flow));
    goto exit;
  }
  if (!gst_sc68_load_buf(filter, buf))
    goto exit;
  if (!gst_sc68_caps(filter))
    goto exit;
  res = TRUE;

exit:
  if (q)
    gst_query_unref(q);
  if (buf)
    gst_buffer_unref(buf);
  return res;
}

static gboolean gst_sc68_task_play(Gstsc68 * filter)
{
  gboolean res;

  GstBuffer *buf = NULL/* , * meta = NULL, * wbuf = NULL */;
  int frames, n;
  GstFlowReturn flow;
  GstClockTime stamp   = gst_util_get_timestamp();
  /* GstClockTime elapsed = stamp - filter->stamp; */
  /* if (elapsed < 10000000) */
  /*   return TRUE; */
  filter->stamp = stamp;

  if (res = gst_sc68_get_buffer(filter, &buf), (res && buf)) {

    /* if (!gst_buffer_is_metadata_writable(buf)) { */
    /*   gst_buffer_unref(buf); */
    /*   buf = NULL; */
    /*   GST_ERROR_OBJECT(filter, "buffer is not metadata writable"); */
    /*   goto exit; */
    /* } */

    GST_BUFFER_OFFSET   (buf) = filter->samples;
    GST_BUFFER_TIMESTAMP(buf) = GST_CLOCK_TIME_NONE; // $$$ TODO or not ?
    GST_BUFFER_DURATION (buf) = GST_CLOCK_TIME_NONE; // $$$ TODO or not ?
    n = frames = GST_BUFFER_SIZE(buf) >> 2;

    /* if (!gst_buffer_is_writable(buf)) { */
    /*   gst_buffer_unref(buf); */
    /*   buf = NULL; */
    /*   GST_ERROR_OBJECT(filter, "buffer is not writable"); */
    /*   goto exit; */
    /* } */

    filter->code = sc68_process(filter->sc68,  GST_BUFFER_DATA(buf), &n);
    res = filter->code != SC68_ERROR;
    if (res) {
      int is_seeking, ms;
      filter->samples += n;
      /* $$$ Check dis SC68_SEEK_DISK ? SC68_SEEK_TRAKC */
      ms = sc68_cntl(filter->sc68, SC68_GET_POS);
      filter->position_ns = gst_sc68_mstons(ms);
      flow = gst_pad_push(filter->srcpad, buf);
      if (flow != GST_FLOW_OK) {
        res = FALSE;
        gst_buffer_unref(buf);
        GST_ERROR_OBJECT(filter, "pad <%s> push buffer failed [%s]",
                         gst_pad_get_name(filter->srcpad),
                         gst_flow_get_name(flow));
      }
    }
  }

  return res;
}


static void gst_sc68_task(void * data)
{
  Gstsc68 * filter = data;

  switch (filter->status) {

  case GST68_ERROR:
    GST_DEBUG_OBJECT(filter,"flushing sc68 errors:");
    gst_sc68_report_error(filter);
    GST_DEBUG_OBJECT(filter,"pausing task because of error");
    gst_pad_pause_task(filter->sinkpad);
    break;

  case GST68_LOAD:
    if (!gst_sc68_task_load(filter)) {
      filter->status = GST68_ERROR;
      break;
    }
    filter->status = GST68_PLAY;
    filter->stamp  = gst_util_get_timestamp();

    /* gst_pad_push_event(filter->srcpad, */
    /*                    gst_event_new_new_segment( */
    /*                      FALSE, // gboolean update, */
    /*                      1.0,   // rate, */
    /*                      GST_FORMAT_TIME, // GstFormat format, */
    /*                      0, // gint64 start, */
    /*                      gst_sc68_mstons( (filter->single_track ? &filter->trkinfo : &filter->dskinfo) -> time_ms ), //gint64 stop, */
    /*                      0 // gint64 position */
    /*                      )); */

    break;

  case GST68_PLAY:
    if (!gst_sc68_task_play(filter)) {
      GST_DEBUG_OBJECT(filter, "task play has failed, changing status (%d)", filter->code);
      filter->status = GST68_ERROR;
      break;
    }
    if (filter->code & SC68_END) {
      GST_DEBUG_OBJECT(filter, "end detected");
      filter->status = GST68_STOP;
    } else if (filter->code & SC68_CHANGE) {
      //GST_DEBUG_OBJECT(filter, "change track detected");
      gst_sc68_onchangetrack(filter);
    }
    /* if (filter->code & SC68_LOOP) */
    /*   GST_DEBUG_OBJECT(filter, "loop detected"); */
    break;

  case GST68_STOP:
    GST_DEBUG_OBJECT(filter,"pausing task because end");
    gst_pad_push_event(filter->srcpad,gst_event_new_eos());
    gst_pad_pause_task(filter->sinkpad);
    break;


  case GST68_INIT:
  case GST68_PAUSE:
  case GST68_SHUTDOWN:

  default:
    GST_DEBUG_OBJECT(filter, "unexpected task status [%d]", filter->status);
    break;
  }
}

static const char * activatemode2str( int mode ) {
  switch ( mode ) {
  case GST_ACTIVATE_NONE: return "none";
  case GST_ACTIVATE_PUSH: return "push";
  case GST_ACTIVATE_PULL: return "pull";
  }
  return "????";
}

static gboolean gst_sc68_check_get_range(GstPad *pad)
{
  GST_DEBUG("<%s>", gst_pad_get_name(pad));
  return TRUE;
}

static gboolean gst_sc68_activate(GstPad *pad) {
  gboolean res;

  if (gst_pad_check_pull_range(pad)) {
    GST_DEBUG("<%s> check pull range",
              gst_pad_get_name(pad));
    res = gst_pad_activate_pull(pad, TRUE);
  } else {
    GST_DEBUG("<%s> can't check pull range",
              gst_pad_get_name(pad));
    res = FALSE;
  }

  /* if ( GST_PAD_DIRECTION(pad) == GST_PAD_SRC) */
  /*   res = gst_pad_activate_pull(pad,TRUE); */
  /* else */
  /*   res = gst_pad_activate_pull(pad,TRUE); */

  GST_DEBUG("<%s> current-mode:%s -> [%s]",
            gst_pad_get_name(pad),
            activatemode2str(GST_PAD_MODE_ACTIVATE(pad)),
            res?"ON":"OFF");

  return res;
}

static gboolean gst_sc68_activate_pull(GstPad *pad, gboolean active)
{
  gboolean res;
  Gstsc68 * filter = GST_SC68(GST_OBJECT_PARENT(pad));
  if (active) {
    filter->status  = GST68_LOAD;
    res = gst_pad_start_task(pad, gst_sc68_task, filter);
  } else {
    filter->status  = GST68_STOP;
    res = gst_pad_stop_task (pad);
  }
  GST_DEBUG("<%s> %s -> [%s]",gst_pad_get_name(pad), active?"ON":"OFF", res?"OK":"ERR");
  return res;
}


static gboolean gst_sc68_activate_push(GstPad *pad,  gboolean active)
{
  GST_DEBUG("<%s> (%s)",gst_pad_get_name(pad), active?"ON":"OFF");
  return TRUE;
}


static const GstQueryType * gst_sc68_query_types(GstPad *pad)
{
  static const GstQueryType qtypes[] = {
    GST_QUERY_DURATION, // query the total length of the stream.
    GST_QUERY_POSITION, // query the current position of playback in the stream.
//    GST_QUERY_SEEKING,
    GST_QUERY_SEGMENT,  // query the currently configured segment for playback.
    GST_QUERY_NONE
  };
  return qtypes;
}

static gboolean gst_sc68_query(GstPad *pad, GstQuery *query)
{
  Gstsc68 * filter = GST_SC68(GST_OBJECT_PARENT(pad));
  const GstQueryType qtype = GST_QUERY_TYPE(query);

  switch (qtype) {
  case GST_QUERY_DURATION: {
    GstFormat format = GST_FORMAT_TIME;
    gst_query_parse_duration(query, &format, NULL);
    switch (format) {
    case GST_FORMAT_TIME:
      gst_query_set_duration(query, format, filter->duration_ns);
      return TRUE;
    default:
      GST_DEBUG("<%s> %s -- %s",
                gst_pad_get_name(pad), gst_query_type_get_name(qtype),
                gst_format_get_name(format));
      break;
    }
  } break;

  case GST_QUERY_POSITION: {
    GstFormat format = GST_FORMAT_TIME;
    gst_query_parse_position(query, &format, NULL);
    switch (format) {
#if 0 /* Sample counter get screwed as soon as we seek */
    case GST_FORMAT_BYTES:
      gst_query_set_position(query, format, filter->samples << 2);
      return TRUE;
    case GST_FORMAT_DEFAULT:
      gst_query_set_position(query, format, filter->samples);
      return TRUE;
#endif
    case GST_FORMAT_TIME:
      gst_query_set_position(query, format, filter->position_ns);
      return TRUE;
    default:
      GST_DEBUG("<%s> %s -- %s",
                gst_pad_get_name(pad), gst_query_type_get_name(qtype),
                gst_format_get_name(format));
    }
  } break;

  case GST_QUERY_SEEKING: {
    GstFormat format= GST_FORMAT_TIME;
    /* $$$ Should requested format be honnored ? */
    gst_query_set_seeking(query, format, TRUE,
                          filter->position_ns,
                          filter->duration_ns);
    return TRUE;
  } break;

  default:
    break;
  }

  return gst_pad_query_default(pad,query);
}

static gboolean gst_sc68_event(GstPad *pad, GstEvent *event)
{
  Gstsc68 * filter = GST_SC68(GST_OBJECT_PARENT(pad));

  switch (GST_EVENT_TYPE(event)) {
  case GST_EVENT_NAVIGATION:
    break;
  case GST_EVENT_SEEK:
#if 0
    if (pad == filter->srcpad) {
      GstFormat format;
      GstSeekType start_type, stop_type;
      gint64 start, stop;

      gst_event_parse_seek(event,
                           NULL, // gdouble *rate,
                           &format,
                           NULL, //GstSeekFlags *flags,
                           &start_type,
                           &start,
                           &stop_type,
                           &stop);
      GST_DEBUG("<%s> event <%d,%s> [%lld(%d)..%lld(%d)]",
                gst_pad_get_name(pad),
                GST_EVENT_TYPE(event),
                GST_EVENT_TYPE_NAME(event),
                start_type,start,
                stop_type,stop);
      switch (start_type) {
      case GST_SEEK_TYPE_NONE:
      case GST_SEEK_TYPE_CUR:
      case GST_SEEK_TYPE_END: break;
      case GST_SEEK_TYPE_SET:
        if (!sc68_seek(filter->sc68, gst_sc68_nstoms(start) ,0))
          return TRUE;
        break;
      }
    }
#endif
    break;

  case GST_EVENT_NEWSEGMENT:
    if (pad == filter->sinkpad) {
      gboolean  update;
      gdouble   rate;
      GstFormat format;
      gint64    start, stop, position;
      gst_event_parse_new_segment(event,&update,&rate,&format,&start,&stop,&position);
      GST_DEBUG("Start to read sc68 file [%d .. %d], rate=%.2lf", (int)start, (int)stop, rate);
      filter->filelen = (int) stop; // - (int) start;
      filter->fileptr = 0;
      if (filter->filemax < filter->filelen) {
        void * filenew = realloc(filter->filebuf, filter->filelen);
        if (filenew) {
          filter->filebuf = filenew;
          filter->filemax = filter->filelen;
        }
      }
      GST_DEBUG("preallocated file buffer <%d>", filter->filemax);
    }
    break;

  case GST_EVENT_EOS:

    if (pad == filter->sinkpad) {
      int err;
      /* if ( GST_PAD_TASK(filter->srcpad) ) { */
      /*   gst_pad_stop_task(filter->srcpad); */
      /*   /\* ??? destroy ??? *\/ */
      /*   break; */
      /* } */

      GST_DEBUG("file buffer complete: %d bytes", filter->fileptr);

      /* Load sc68 disk here */
      err = sc68_load_mem(filter->sc68,filter->filebuf,filter->fileptr);
      if (!err) {

        /* filter->dskinfo = sc68_music_info(filter->sc68,0,0); */
        /* GST_DEBUG("<%s> disk loaded: %s %s - %s", */
        /*           gst_pad_get_name(pad), */
        /*           filter->dskinfo.time, */
        /*           filter->dskinfo.author, */
        /*           filter->dskinfo.title); */
        /* */
        filter->code = sc68_process(filter->sc68, 0, 0);


        /* GST_DEBUG("<%s> trak loaded: %s %s - %s ", */
        /*           gst_pad_get_name(pad), */
        /*           filter->trkinfo.time, */
        /*           filter->trkinfo.author, */
        /*           filter->trkinfo.title); */

        /* if (!gst_pad_start_task(filter->srcpad, gst_sc68_task, filter)) { */
        /*   GST_DEBUG("start task failed"); */
        /* } else { */
        /*   GST_DEBUG("start task success"); */
        /* } */
        /* gst_task_start(filter->task); */
        /* if (filter->filelen && filter->fileptr>=filter->filelen) { */
        /*   GstBuffer *  buf = // gst_buffer_new(); */
        /*     gst_buffer_new_and_alloc(1 << 20); */
        /*   gst_buffer_set_caps(buf, */
        /*                       gst_caps_new_simple( */
        /*                         "audio/x-raw-int", */
        /*                         "signed",     G_TYPE_BOOLEAN, TRUE, */
        /*                         "width",      G_TYPE_INT,  16, */
        /*                         "depth",      G_TYPE_INT,  16, */
        /*                         "endianness", G_TYPE_INT,  G_BYTE_ORDER, */
        /*                         "rate",       G_TYPE_INT,  sc68_sampling_rate(filter->sc68, SC68_SPR_QUERY), */
        /*                         "channels",   G_TYPE_INT,  2, */
        /*                         NULL)); */

        /*   if ( !gst_buffer_is_writable(buf) ) { */
        /*     GST_DEBUG("buffer not writable"); */
        /*     gst_buffer_make_writable(buf); */
        /*   } */
        /*   if ( !gst_buffer_is_writable(buf) ) { */
        /*     GST_DEBUG("buffer not writable after make writable"); */
        /*   } else { */
        /*     buf->timestamp = 0; */
        /*     buf->duration  = 1000; */
        /*     filter->code = sc68_process(filter->sc68, buf->data, buf->size>>2); */
        /*     GST_DEBUG("sc68: code=%x data:%p samples:%d", filter->code, buf->data, buf->size>>2); */
        /*     /\* set timestamp and all *\/ */
        /*     gst_pad_push (filter->srcpad, buf); */
        /*   } */
        /* } */
      }
      /* Free file buffer */
      free(filter->filebuf);
      filter->filebuf = 0;
      filter->filemax = filter->filelen = filter->fileptr = 0;
    }
    break;
  default:
    GST_DEBUG("<%s> event <%d,%s>",
              gst_pad_get_name(pad),
              GST_EVENT_TYPE(event),
              GST_EVENT_TYPE_NAME(event));

    break;
  }
  return gst_pad_event_default(pad, event);
}

/***********************************************************************
 * SET PROPERTIES
 ***********************************************************************/
static void gst_sc68_set_property(GObject * object, guint prop_id,
                                  const GValue * value, GParamSpec * pspec)
{
  Gstsc68 * filter = GST_SC68(object);
  filter = filter;

  GST_DEBUG("prop_id: %d", prop_id);

  switch (prop_id) {
  case PROP_TRACK:
    filter->prop.track = g_value_get_int(value);
    break;
  case PROP_LOOP:
    filter->prop.loop = g_value_get_int(value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

/***********************************************************************
 * GET PROPERTIES
 ***********************************************************************/
static void gst_sc68_get_property(GObject * object, guint prop_id,
                                  GValue * value, GParamSpec * pspec)
{
  Gstsc68 *filter = GST_SC68 (object);
  filter = filter;

  GST_DEBUG("prop_id: %d", prop_id);

  switch (prop_id) {

  case PROP_TRACK: {
    int track = sc68_play(filter->sc68, -1, 0);
    if (track == -1) track = filter->prop.track;
    g_value_set_int(value, track);
  } break;

  case PROP_LOOP: {
    int loop = sc68_play(filter->sc68, -1, -1);
    if (loop == -1) loop = filter->prop.loop;
    g_value_set_int(value, loop);
  } break;

  case PROP_TRACKS:
    g_value_set_int(value, filter->info.tracks);
    break;

  case PROP_SPLRATE:
    g_value_set_int(value, filter->prop.rate);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

/***********************************************************************
 * SET CAPS
 ***********************************************************************/
static gboolean gst_sc68_set_caps(GstPad * pad, GstCaps * caps)
{
  /* Gstsc68      * filter; */
  /* GstPad       * otherpad; */
  /* gint           sample_rate; */
  /* GstStructure *s; */

  GST_DEBUG("<%s> %s", gst_pad_get_name(pad),
            gst_caps_to_string(caps));
  /* s = gst_caps_get_structure(caps,0); */
  /* gst_structure_get_int(s,"rate",&sample_rate); */
  /* GST_DEBUG("new sampling-rate is %d", sample_rate); */

  return TRUE;
}


/***********************************************************************
 * CHAIN
 ***********************************************************************/
#if 0
static GstFlowReturn gst_sc68_chain(GstPad * pad, GstBuffer * buf)
{
  Gstsc68 *filter = GST_SC68(GST_OBJECT_PARENT(pad));
  int buflen = GST_BUFFER_SIZE(buf);
  int bufreq = filter->fileptr + buflen;

  GST_DEBUG("<%s> size:%d offset:%d caps:%s",
            gst_pad_get_name(pad),
            GST_BUFFER_SIZE(buf),
            GST_BUFFER_OFFSET(buf),
            gst_caps_to_string(gst_buffer_get_caps(buf)));

  /* assert( offset == filter->fileptr ); */
  /* assert( filelen && offset+size <= filelen ); */

  if (bufreq > filter->filemax) {
    void * filenew;
    /* GST_DEBUG("bufreq<%d> bufmax<%d>",bufreq,filter->filemax); */
    filenew = realloc(filter->filebuf, bufreq);
    if (filenew) {
      filter->filebuf = filenew;
      filter->filemax = bufreq;
    }
  }
  if (bufreq > filter->filemax) {
    /* GST_ERROR("can't re-alloc sc68 file buffer"); */
    /* GST_ELEMENT_ERROR(GST_ELEMENT(filter), STREAM, FAILED, (NULL), (NULL)); */
    return GST_FLOW_ERROR;
  }

  memcpy(filter->filebuf+filter->fileptr, GST_BUFFER_DATA(buf), buflen);
  filter->fileptr += buflen;
  gst_buffer_unref(buf);

  GST_DEBUG("<%s> got %6d/%d",gst_pad_get_name(pad), filter->fileptr, filter->filelen);

  return GST_FLOW_OK;
}
#endif


#if 0
static GstFlowReturn gst_sc68_srcchain (GstPad * pad, GstBuffer * buf)
{
  Gstsc68 *filter = GST_SC68(GST_OBJECT_PARENT(pad));

  GST_DEBUG("<%s> buffer-caps: %d bytes \n%s\n",
            gst_pad_get_name(pad),
            GST_BUFFER_SIZE(buf),
            gst_caps_to_string(gst_buffer_get_caps(buf)));

  gst_buffer_unref(buf);
  return GST_FLOW_OK;
  return gst_pad_push (filter->srcpad, buf);
}
#endif

/***********************************************************************
 * PLUGIN INIT
 *
 * called as soon as the plugin is loaded
 ***********************************************************************/
/* register the element factories and other features
 ***********************************************************************/
static gboolean gst_plugin_init (GstPlugin * plugin)
{
  /* Add a debug category. */
  GST_DEBUG_CATEGORY_INIT(gst_sc68_debug, "sc68", 0, "sc68 music player");

#if USE_TYPEFINDER
  /* Add a dependency to our type finder. */
  gst_plugin_add_dependency_simple(
    plugin,             /* Plugin   */
    NULL,               /* Env vars */
    NULL,               /* Path     */
    "libgstsc68-tf.so", /* Name ?  */
    GST_PLUGIN_DEPENDENCY_FLAG_NONE);
#endif
  /* Register our element */
  return
    gst_element_register(plugin, "sc68", GST_RANK_PRIMARY, GST_TYPE_SC68);
}

/***********************************************************************
 * PLUGIN DEFINITION FOR REGISTRATION
 ***********************************************************************/

GST_PLUGIN_DEFINE (
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "sc68",
  "Atari ST and Amiga music player",
  gst_plugin_init,
  VERSION,
  "LGPL",
  "sc68-gst",
  PACKAGE_URL "/" PACKAGE_NAME
  )
