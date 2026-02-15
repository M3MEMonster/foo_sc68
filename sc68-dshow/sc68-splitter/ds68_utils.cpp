/*
 * @file    ds68_utils.cpp
 * @brief   sc68 for directshow - utility functions
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (c) 1998-2014 Benjamin Gerard
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ds68_headers.h"

wchar_t * GUIDtoW(GUID * id, wchar_t * tmp)
{
  static wchar_t buf[64];
  if (!tmp) tmp = buf;
  wsprintfW(tmp,L"%08x-%04x-%04x-%02x%02x-%02x-%02x-%02x-%02x-%02x-%02x",
           id->Data1,id->Data2,id->Data3,
           id->Data4[0],id->Data4[1],id->Data4[2],id->Data4[3],
           id->Data4[4],id->Data4[5],id->Data4[6],id->Data4[7]);
  return tmp;
}

char * GUIDtoS(const GUID * id, char * tmp)
{
  static char buf[64];
  if (!tmp) tmp = buf;
  sprintf_s(tmp,64,"%08x-%04x-%04x-%02x%02x-%02x-%02x-%02x-%02x-%02x-%02x",
            id->Data1,id->Data2,id->Data3,
            id->Data4[0],id->Data4[1],id->Data4[2],id->Data4[3],
            id->Data4[4],id->Data4[5],id->Data4[6],id->Data4[7]);
  return tmp;
}

const wchar_t * MediaTypeW(const CMediaType *pmt)
{
  const GUID & T = *pmt->Type();
  const GUID & S = *pmt->Subtype();

  if (T == MEDIATYPE_Stream) {
    if (S == MEDIASUBTYPE_NULL)
      return L"STREAM+<*>";
    else if (S == MEDIATYPE_SC68)
      return L"STREAM+SC68";
    else {
      DBG("Unknown subtype: %s\n",GUIDtoS(&S));
      return L"STREAM+<?>";
    }
  } else if (T == MEDIATYPE_Audio) {
    if (S == MEDIASUBTYPE_NULL)
      return L"AUDIO+<*>";
    else if (S == MEDIASUBTYPE_PCM)
      return L"AUDIO+PCM";
    else {
      DBG("Unknown subtype: %s\n",GUIDtoS(&S));
      return L"AUDIO+<?>";
    }
  } else if (T == MEDIATYPE_NULL) {
    return L"STREAM_NULL";
  } else {
    DBG("Unknown type: %s\n",GUIDtoS(&T));
    return L"MEDIATYPE<?>";
  }
}

const wchar_t * MediaTypeW(const CMediaType pmt) {
  return MediaTypeW(&pmt);
}

const char * MediaTypeA(const CMediaType *pmt)
{
  const GUID & T = *pmt->Type();
  const GUID & S = *pmt->Subtype();

  if (T == MEDIATYPE_Stream) {
    if (S == MEDIASUBTYPE_NULL)
      return "STREAM+<*>";
    else if (S == MEDIATYPE_SC68)
      return "STREAM+SC68";
    else {
      DBG("Unknown subtype: %s\n",GUIDtoS(&S));
      return "STREAM+<?>";
    }
  } else if (T == MEDIATYPE_Audio) {
    if (S == MEDIASUBTYPE_NULL)
      return "AUDIO+<*>";
    else if (S == MEDIASUBTYPE_PCM)
      return "AUDIO+PCM";
    else {
      DBG("Unknown subtype: %s\n",GUIDtoS(&S));
      return "AUDIO+<?>";
    }
  } else if (T == MEDIATYPE_NULL) {
    return "STREAM_NULL";
  } else {
    DBG("Unknown type: %s\n",GUIDtoS(&T));
    return "MEDIATYPE<?>";
  }
}

const char * MediaTypeA(const CMediaType pmt) {
  return MediaTypeA(&pmt);
}

HRESULT BSTRset(BSTR * lpstr, const char * str)
{
  int chars = strlen(str);
  int bytes = MultiByteToWideChar(CP_UTF8,0,str,chars,NULL,0);
  if (bytes <= 0)
    return GetLastError();
  bytes *= sizeof(OLECHAR);
  *lpstr= SysAllocStringByteLen(NULL,bytes);
  if (!*lpstr)
    return E_OUTOFMEMORY;
  MultiByteToWideChar(CP_UTF8, 0, str, chars, *lpstr, chars);
  return S_OK;
}

WCHAR * FormatStrW(const char * fmt, ...)
{
  WCHAR * wstr = 0;
  char str[256];
  const int max = sizeof(str);
  int icnt, ocnt;
  va_list list;

  va_start(list,fmt);
  icnt = vsnprintf(str,max,fmt,list) + 1;
  ASSERT(icnt == strlen(str)+1);
  str[max-1] = 0;
  va_end(list);

  ocnt = MultiByteToWideChar(CP_UTF8,0,str,icnt+1,NULL,0);
  if (ocnt > 0) {
    wstr = (WCHAR *) CoTaskMemAlloc(ocnt*sizeof(WCHAR));
    if (wstr)
      MultiByteToWideChar(CP_UTF8, 0, str, icnt, wstr, ocnt);
  }
  return wstr;
}
