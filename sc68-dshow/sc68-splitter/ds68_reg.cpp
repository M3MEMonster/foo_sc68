/*
 * @file    ds68_reg.cpp
 * @brief   sc68 for directshow - register
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

#define _CRT_SECURE_NO_WARNINGS
#include "ds68_headers.h"

// Media Types
const AMOVIESETUP_MEDIATYPE sudPinTypes[] =
{
  /*0*/ { &MEDIATYPE_Stream, &MEDIATYPE_SC68 },
  /*1*/ { &MEDIATYPE_Stream, &MEDIASUBTYPE_NULL },
  /*2*/ { &MEDIATYPE_Audio,  &MEDIASUBTYPE_PCM },
};

// Pins
const AMOVIESETUP_PIN psudPins[] =
{
  // Input pin
  {
    L"",            // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    FALSE,          // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &GUID_NULL,     // Obsolete.
    NULL,           // Obsolete.
    2,              // Number of media types.
    sudPinTypes+0   // Pointer to media types.
  },

  // Output outPin
  {
    L"",            // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &GUID_NULL,     // Obsolete.
    NULL,           // Obsolete.
    1,              // Number of media types.
    sudPinTypes+2   // Pointer to media types.
  }

};

// Filters
const AMOVIESETUP_FILTER sudSc68Splitter =
{
  &__uuidof(Sc68Splitter),              // Filter CLSID
  L"SC68 Splitter",                             // Filter Name
  MERIT_UNLIKELY,                                       // Filter Merit
  2,                                                            // Filter Pin count
  psudPins,                                             // Filter Pins
  //CLSID_LegacyAmFilterCategory
};

// Templates
CFactoryTemplate g_Templates[]=
{
  {
    L"SC68 Splitter",
    &__uuidof(Sc68Splitter),
    Sc68Splitter::CreateInstance,
    Sc68Splitter::StaticInit,
    &sudSc68Splitter
  }
};

int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);


static HRESULT RegisterSc68Protocol()
{
  HRESULT hr = S_OK;
  DBG("%s() => %s\n", __FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

static HRESULT UnregisterSc68Protocol()
{
  HRESULT hr = S_OK;
  DBG("%s() => %s\n", __FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

static const char regpref_mtext[] = "Media Type\\Extensions\\";


static const char * reg_exts[] = { ".sc68", ".sndh" };
static const char vn_sourcefilter[] = "Source Filter";
static const char vn_mediatype[] = "Media Type";
static const char vn_subtype[] = "Subtype";
static const char sz_streammediatype[] = MEDIATYPE_STREAM_STR;
static const char sz_sc68mediatype[] = MEDIATYPE_SC68_STR;
static const char sz_sc68clid[] = "{" CLID_SC68SPLITTER_STR "}";
static const char sz_filesrcasync[] = CLID_FILESRCASYNC_STR;

// Byte Checker
static const char regpref_mtcheckbyte[] =
  "Media Type\\" MEDIATYPE_STREAM_STR "\\" MEDIATYPE_SC68_STR "\\";
static const char bytecheck_sc68[] =  "0,4,,53433638"; // SC68
static const char bytecheck_iceu[] =  "0,4,,49434521"; // ICE!
static const char bytecheck_icel[] =  "0,4,,49636521"; // Ice!
static const char bytecheck_sndh[] = "12,4,,534e4448"; // SNDH

static struct {
  const char * key, *val;
  int len;
} bytecheckers[] = {
  { "0", bytecheck_sc68, sizeof(bytecheck_sc68) },
  { "1", bytecheck_iceu, sizeof(bytecheck_iceu) },
  { "2", bytecheck_icel, sizeof(bytecheck_icel) },
  { "3", bytecheck_sndh, sizeof(bytecheck_sndh) },
  //{ vn_sourcefilter, sz_sc68clid, sizeof(sz_sc68clid) }
  { vn_sourcefilter, sz_filesrcasync, sizeof(sz_filesrcasync) }
};

static HRESULT RegisterSc68FileExtension()
{
  HRESULT hr = S_OK;
  char tmp[256], *stop;

  CopyMemory(tmp,regpref_mtext,sizeof(regpref_mtext));
  stop = tmp + sizeof(regpref_mtext) - 1;

  for (int i=0; i<sizeof(reg_exts)/sizeof(*reg_exts); ++i) {
    HRESULT res;
    strncpy(stop,reg_exts[i],sizeof(tmp)-sizeof(regpref_mtext));

    /* Source Filter */
    //res = RegSetKeyValueA(
    //  HKEY_CLASSES_ROOT, tmp, vn_sourcefilter, REG_SZ,sz_sc68clid, sizeof(sz_sc68clid));

    res = RegSetKeyValueA(HKEY_CLASSES_ROOT, tmp, vn_sourcefilter, REG_SZ,
                          sz_filesrcasync, sizeof(sz_filesrcasync));

    if (res != ERROR_SUCCESS) hr = res;

    /* Media Type */
    res = RegSetKeyValueA(HKEY_CLASSES_ROOT, tmp, vn_mediatype, REG_SZ,
                          sz_streammediatype,sizeof(sz_streammediatype));
    if (res != ERROR_SUCCESS) hr = res;

    /* Sub Media Type */
    res = RegSetKeyValueA(HKEY_CLASSES_ROOT, tmp, vn_subtype, REG_SZ,
                          sz_sc68mediatype, sizeof(sz_sc68mediatype));
    if (res != ERROR_SUCCESS) hr = res;
  }
  DBG("%s() => %s\n", __FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

static HRESULT UnregisterSc68FileExtension()
{
  HRESULT hr = S_OK;
  char tmp[256], *stop;
  CopyMemory(tmp,regpref_mtext,sizeof(regpref_mtext));
  stop = tmp + sizeof(regpref_mtext) - 1;
  for (int i=0; i<sizeof(reg_exts)/sizeof(*reg_exts); ++i) {
    HRESULT res;
    strncpy(stop,reg_exts[i],sizeof(tmp)-sizeof(regpref_mtext));
    DBG("%s() delete key -- \"%s\"\n",__FUNCTION__,tmp);
    res = RegDeleteTreeA(HKEY_CLASSES_ROOT,tmp);
    if (res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND) {
      dbg_error(res,"RegDeleteTreeA (%d) -- HKCR\\%s", res, tmp);
      hr = res;
    }
  }
  DBG("%s() => %s\n", __FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

static HRESULT RegisterSc68ByteCheck()
{
  HRESULT hr = S_OK;

  HRESULT res;

  for (int i=0; i<sizeof(bytecheckers)/sizeof(*bytecheckers); ++i) {
    res = RegSetKeyValueA(
      HKEY_CLASSES_ROOT, regpref_mtcheckbyte,   bytecheckers[i].key,
      REG_SZ, bytecheckers[i].val, bytecheckers[i].len);
    if (res != ERROR_SUCCESS)
      hr = res;
  }

  DBG("%s() => %s\n", __FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

static HRESULT UnregisterSc68ByteCheck()
{
  HRESULT hr = S_OK;
  HRESULT res = RegDeleteKeyA(HKEY_CLASSES_ROOT,regpref_mtcheckbyte);
  if (res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND)
    hr = res;
  DBG("%s() => %s\n", __FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

static HRESULT RegisterExtra()
{
  HRESULT hr = S_OK, hr2;
  if (FAILED(hr2 = RegisterSc68Protocol())) hr = hr2;
  if (FAILED(hr2 = RegisterSc68FileExtension())) hr = hr2;
  if (FAILED(hr2 = RegisterSc68ByteCheck())) hr = hr2;
  DBG("%s() => %s\n", __FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

static HRESULT UnregisterExtra()
{
  HRESULT hr = S_OK, hr2;
  if (FAILED(hr2 = UnregisterSc68Protocol())) hr = hr2;
  if (FAILED(hr2 = UnregisterSc68FileExtension())) hr = hr2;
  if (FAILED(hr2 = UnregisterSc68ByteCheck())) hr = hr2;
  DBG("%s() => %s\n", __FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

STDAPI DllRegisterServer()
{
  HRESULT hr;
  DBG("ENTER %s()\n",__FUNCTION__);
  hr = AMovieDllRegisterServer2(TRUE);
  UnregisterExtra();
  if (!hr) hr = RegisterExtra();
  DBG("LEAVE %s() => %d\n",__FUNCTION__, hr);
  return hr;
}

STDAPI DllUnregisterServer()
{
  HRESULT hr;
  DBG("ENTER %s()\n",__FUNCTION__);
  UnregisterExtra();
  hr = AMovieDllRegisterServer2(FALSE);
  DBG("LEAVE %s() => %d\n",__FUNCTION__, hr);
  return hr;
}

// if we declare the correct C runtime entrypoint and then forward it to the DShow base
// classes we will be sure that both the C/C++ runtimes and the base classes are initialized
// correctly
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
  HRESULT hr;
  DBG("%s() -- %s the dragon (%d) %s (%s)\n", __FUNCTION__,
      dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH
      ? "ENTER":"LEAVE", dwReason,__DATE__,__TIME__);
  hr = DllEntryPoint(reinterpret_cast<HINSTANCE>(hDllHandle),
                     dwReason, lpReserved);
  switch (dwReason) {
  case DLL_PROCESS_ATTACH:
    if (hr == TRUE && !Sc68Splitter::sc68_inited) {
      DBG("%s() -- sc68 init has failed\n", __FUNCTION__);
      hr = FALSE;
    }
    break;
  }
#ifdef _DEBUG
  Sc68Splitter::DumpSc68Error(__FUNCTION__,0);
#endif
  return hr;
}
