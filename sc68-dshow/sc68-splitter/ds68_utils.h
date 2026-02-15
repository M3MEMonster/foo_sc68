/**
 * @ingroup   sc68_directshow
 * @file      ds68_utils.h
 * @brief     Utility functions.
 * @author    Benjamin Gerard
 * @date      2014/06
 */

#pragma once

/**
 * @name Utility functions
 * @addtogroup sc68_directshow
 * @{
 */
/** GUID to wide-char string. */
wchar_t * GUIDtoW(GUID * id, wchar_t * tmp = 0);
/** GUID to byte string. */
char * GUIDtoS(const GUID * id, char * tmp = 0);

/** MediaType to wide-char string. */
const wchar_t * MediaTypeW(const CMediaType *pmt);
/** Media-type wide-char string. */
const wchar_t * MediaTypeW(const CMediaType pmt);

/** Media-type to byte string. */
const char * MediaTypeA(const CMediaType *pmt);
/** Media-type to byte string. */
const char * MediaTypeA(const CMediaType pmt);

/** Binary String from byte string. */
HRESULT BSTRset(BSTR * lpstr, const char * str);

/** Create a formatted widechar string from multibyte. */
WCHAR * FormatStrW(const char * fmt, ...);

/**
 * @}
 */
