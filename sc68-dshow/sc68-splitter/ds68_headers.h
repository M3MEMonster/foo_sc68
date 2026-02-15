/**
 * @ingroup   sc68_directshow
 * @file      ds68_headers.h
 * @brief     Include all headers
 * @author    Benjamin Gerard
 * @date      2014/06
 */

#pragma once

#define _CRT_SECURE_NO_WARNINGS

/* MS and libc */
#include <control.h>
#include <streams.h>
#include <tchar.h>
#include <pullpin.h>
#include <initguid.h>
#include <uuids.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <qnetwork.h>

/* sc68 */
#include "sc68/file68.h"
#include "sc68/file68_vfs_def.h"
#include "sc68/file68_vfs.h"
#include "sc68/file68_msg.h"
#include "sc68/file68_str.h"
#include "sc68/sc68.h"

/* sc68-directshow */
#include "ds68_dbg.h"
#include "ds68_utils.h"
#include "ds68_types.h"
#include "ds68_vfs.h"
#include "ds68_inppin.h"
#include "ds68_outpin.h"
#include "ds68_splitter.h"

#ifdef  _MSC_VER
#pragma warning(disable: 4355)
#endif

