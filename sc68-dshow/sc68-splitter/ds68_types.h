/**
 * @ingroup   sc68_directshow
 * @file      ds68_types.h
 * @brief     Types and uids.
 * @author    Benjamin Gerard
 * @date      2014/06
 */

#pragma once

/**
 * @addtogroup sc68_directshow
 * @{
 */

/// Global Unique IDentifier for sc68 media type
DEFINE_GUID(MEDIATYPE_SC68,
	0x95043f3f,0x0aa3,0x4f1e,0x8C,0xC4,0x1F,0xD3,0x43,0xAF,0xB6,0x87);

/// String representation of MEDIATYPE_SC68.
#define MEDIATYPE_SC68_STR    "{95043F3F-0AA3-4F1E-8CC4-1FD343AFB687}"

/// CLass IDentifier for the sc68 directshow splitter filter.
#define CLID_SC68SPLITTER_STR  "B7C1A783-CD33-4E49-A709-E6C9F2EB890C"

/// String representation of the stream media type.
#define MEDIATYPE_STREAM_STR  "{E436EB83-524F-11CE-9F53-0020AF0BA770}"

/// String representation of the file source async directshow filter.
#define CLID_FILESRCASYNC_STR "{E436EBB5-524F-11CE-9F53-0020AF0BA770}"

/// CLass IDentifier for the sc68 directshow properties page.
#define CLID_SC68PROP_STR      "73E27CAC-0FEB-4E36-A98F-FFBDE21387B1"

/// Global Unique IDentifier for the sc68 directshow properties page.
#define IID_SC68PROP_STR       "43ff413d-f79e-4dcb-9DBA-7C597212959A"

// Global Unique IDentifier for the sc68 directshow properties page.
// DEFINE_GUID(IID_ISc68Prop,
// 0x43ff413d,0xf79e,0x4dcb,0x9D,0xBA,0x7C,0x59,0x72,0x12,0x95,0x9A);

/**
 * @}
 */
