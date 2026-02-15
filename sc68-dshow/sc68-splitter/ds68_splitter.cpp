/*
 * @file    ds68_splitter.cpp
 * @brief   sc68 for directshow - splitter
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

/////////////////////////////////////////////////////////////////////
// Splitter Filter
/////////////////////////////////////////////////////////////////////

Sc68Splitter::~Sc68Splitter()
{
  DBG("%s()\n", __FUNCTION__);
  if (m_InpPin) delete m_InpPin;
  if (m_sc68) DestroySc68();
  if (m_disk) sc68_disk_free(m_disk);
}

HRESULT Sc68Splitter::Run(REFERENCE_TIME tStart)
{
  HRESULT hr = __super::Run(tStart);
  DBG("%s(%lld) => %s\n",__FUNCTION__,tStart,!hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68Splitter::Stop()
{
  HRESULT hr = __super::Stop();
  DBG("%s() => %s\n",__FUNCTION__,!hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68Splitter::Pause()
{
  HRESULT hr = __super::Pause();
  DBG("%s() => %s\n",__FUNCTION__,!hr?"OK":"FAIL");
  return hr;
}

int Sc68Splitter::GetSamplingRate()
{
  int spr = sc68_cntl(m_sc68,SC68_GET_SPR);
  spr = spr <= 0 ? 44100 : spr;
  DBG("%s => %d hz\n",__FUNCTION__, spr);
  return spr;
}

int Sc68Splitter::SetSamplingRate(int spr)
{
  if (spr < 8000) spr = 8000;
  else if (spr > 96000) spr = 96000;
  spr = sc68_cntl(m_sc68,SC68_SET_SPR, spr);
  DBG("%s => %d hz\n",__FUNCTION__, spr);
  return spr;
}

Sc68Splitter::Sc68Splitter(LPUNKNOWN pUnk, HRESULT *phr)
  : CSource(NAME("SC68 Splitter"), pUnk, __uuidof(this))
  , m_sc68(0)
  , m_disk(0)
  , m_code(SC68_ERROR)
  , m_track(0)
  //, m_spr(GetSamplingRate())
  //, m_allin1(true)
{
  HRESULT hr = NOERROR;
  Sc68OutPin * l_OutPin = nullptr;
  SetDisk(0);
  m_InpPin = new Sc68InpPin(this, m_pLock, phr);
  l_OutPin = new Sc68OutPin(this, m_pLock, phr);
  if (!m_InpPin || ! l_OutPin) {
    DBG("ERROR in %s() => Pins creation inp:%p out:%p!\n", __FUNCTION__, m_InpPin, l_OutPin);
    hr = E_OUTOFMEMORY;
    if (m_InpPin) { delete m_InpPin; m_InpPin = 0; }
    if (l_OutPin) { delete l_OutPin; l_OutPin = 0; }
  }
  if (phr) *phr = hr;
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
}

CBasePin* Sc68Splitter::GetPin(int n)
{
  CBasePin* pin;
  switch (n) {
  case 0: pin = m_InpPin; break;
  case 1: pin = m_paStreams[0]; break;
  default: pin = nullptr; break;
  }
  return pin;
}

int Sc68Splitter::GetPinCount()
{
  return 2;
}

void Sc68Splitter::DumpSc68Error(const char * func, sc68_t * m_sc68)
{
  const char * e;
  if (m_sc68) {
    e = sc68_error(m_sc68);
    if (e && *e)
      DBG("%s() ERROR %s\n",func, e);
  }
  e = sc68_error(0);
  if (e && *e)
    DBG("%s() ERROR %s\n", func, e);
}

void Sc68Splitter::DumpSc68Error(const char * func)
{
  DumpSc68Error(func?func:__FUNCTION__, m_sc68);
}

HRESULT Sc68Splitter::DestroySc68()
{
  HRESULT hr = S_OK;
  DumpSc68Error(__FUNCTION__);
  if (m_sc68) {
    sc68_t * sc68 = m_sc68;
    m_sc68 = 0;
    sc68_destroy(sc68);
  }
  DBG("%s() => %s\n", __FUNCTION__, !hr ?"OK" : "FAIL");
  return hr;
}

HRESULT Sc68Splitter::GetSegment(REFERENCE_TIME &tStart,REFERENCE_TIME &tStop)
{
  HRESULT hr = S_FALSE;
  int ms;
  const REFERENCE_TIME msToRef = 10000;
  if (!m_sc68 || (m_code & SC68_END))
    goto exit;
  ms = sc68_cntl(m_sc68,SC68_GET_POS);
  if (ms == -1)
    goto exit;
  tStart = ms * msToRef;
  ms = sc68_cntl(m_sc68,SC68_GET_LEN);
  if (ms == -1)
    goto exit;
  tStop = ms * msToRef;
  hr = S_OK;
exit:
  return hr;
}

int Sc68Splitter::FillBuffer(IMediaSample *pSample)
{
  BYTE * spl;
  BOOL discont;

  if (!pSample)
    return SC68_IDLE;

  if (!m_sc68) {
    DBG("%s() No sc68 instance\n", __FUNCTION__);
    m_code = SC68_ERROR;
    goto exit;
  }

  if (m_code == SC68_ERROR) {
    DBG("%s() sc68 has failed\n", __FUNCTION__);
    goto exit;
  }

  discont = (m_code & SC68_CHANGE) ? TRUE : FALSE;

  if (SUCCEEDED(pSample->GetPointer(&spl))) {
    REFERENCE_TIME start, end;
    int n = pSample->GetSize(), nspl;
    nspl = n >> 2;

    int dskpos, trkpos;

    dskpos = sc68_cntl(m_sc68,SC68_GET_DSKPOS);
    trkpos = sc68_cntl(m_sc68,SC68_GET_POS);

    m_code = sc68_process(m_sc68, (void*)spl, &nspl);
    if (m_code == SC68_ERROR)
      goto exit;
    pSample->SetActualDataLength(nspl<<2);
    pSample->SetSyncPoint(TRUE);
    pSample->SetDiscontinuity(discont);
    //if (discont)
    //  pSample->SetMediaTime(&start,NULL);
    pSample->GetMediaTime(&start,&end);
    DBG("MEDIATIME: %c dsk:%u trk:%u %llu %llu\n", discont?'*':' ', dskpos, trkpos,start,end);

    // pSample->SetTime(
    //   [in]  REFERENCE_TIME *pTimeStart,
    //   [in]  REFERENCE_TIME *pTimeEnd
    //   );

    if (m_code & SC68_CHANGE)
      sc68_music_info(m_sc68,&m_info,SC68_CUR_TRACK,0);
  }
exit:
  return m_code;
}

HRESULT Sc68Splitter::SetDisk(sc68_disk_t disk)
{
  if (m_disk) {
    DBG("%s() FREE previous disk <%p>\n", __FUNCTION__, m_disk);
    sc68_close(m_sc68);
    sc68_disk_free(m_disk);
  }
  m_disk = disk;
  ZeroMemory(&m_info,sizeof(m_info));
  if (!disk)
    return S_OK;
  if (sc68_music_info(m_sc68,&m_info,1,disk))
    return E_FAIL;
  DBG("%s()\n",__FUNCTION__);
  DBG(" ARTIST: %s\n", m_info.artist);
  DBG(" ALBUM: %s\n", m_info.album);
  DBG(" TITLE: %s\n", m_info.title);
  DBG(" COUNT: %d\n", m_info.tracks);
  return S_OK;
}

HRESULT Sc68Splitter::CreateSc68(sc68_disk_t disk)
{
  HRESULT hr;
  DBG("%s(%p)\n", __FUNCTION__, disk);
  if (!m_sc68) {
    sc68_create_t info;
    ZeroMemory(&info, sizeof(info));
    m_sc68 = sc68_create(&info);
  }
  m_code = SC68_ERROR;
  SetDisk(disk);
  hr = (m_sc68 && m_disk
        //&& (m_spr = GetSamplingRate()) > 0
        && !sc68_open(m_sc68, m_disk)
        && !sc68_play(m_sc68, m_track<=0 || m_track>m_info.tracks ? 1 : m_track, SC68_DEF_LOOP)
        && (m_code = sc68_process(m_sc68,0,0)) != SC68_ERROR
    ) ? S_OK : E_FAIL;
  DumpSc68Error(__FUNCTION__);
  DBG("%s() code=0x%x spr=%dhz => %s\n", __FUNCTION__,
      m_code, GetSamplingRate()/*m_spr*/, !hr ?"OK" : "FAIL");
  return hr;
}

#if 0
HRESULT Sc68Splitter::Run(REFERENCE_TIME tStart)
{
  DBG("%s() ENTER\n", __FUNCTION__);
  CAutoLock lock_it(m_pLock);
  HRESULT hr = __super::Run(tStart);
  DBG("%s() => %s\n", __FUNCTION__, !hr ?"OK" : "FAIL");
  return hr;
}

HRESULT Sc68Splitter::Pause()
{
  DBG("%s() ENTER\n", __FUNCTION__);
  CAutoLock lock_it(m_pLock);
  /* TODO: Create filter resources. */
  /*hr = CreateSc68(*/
  HRESULT hr = __super::Pause();
  DBG("%s() => %s\n", __FUNCTION__, !hr ?"OK" : "FAIL");
  return hr;
}

HRESULT Sc68Splitter::Stop()
{
  DBG("%s() ENTER\n", __FUNCTION__);
  CAutoLock lock_it(m_pLock);
  // Inactivate all the pins, to protect the filter resources.
  HRESULT hr = __super::Stop();
  DestroySc68();
  DBG("%s() => %s\n", __FUNCTION__, !hr ?"OK" : "FAIL");
  return hr;
}
#endif

static const char * Iname(REFIID riid) {
    return  0 ? 0
    : (riid == IID_IAMMediaContent)
    ? "IAMMediaContent"

    : (riid == IID_IMediaPosition)
    ? "IMediaPosition"

#ifdef WITH_STREAMSELECT
    : (riid == IID_IAMStreamSelect)
    ? "IAMStreamSelect"
#endif

#ifdef WITH_TRACKINFO
    : (riid == __uuidof(ITrackInfo))
    ? "ITrackInfo"
#endif

    : (riid == IID_ISpecifyPropertyPages)
    ? "ISpecifyPropertyPages"

    : (riid == __uuidof(ISc68Prop))
    ? "ISc68Prop"
    : GUIDtoS(&riid)
    ;
}

STDMETHODIMP Sc68Splitter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
  CheckPointer(ppv, E_POINTER);
  *ppv = nullptr;
  HRESULT hr = 0 ? 0
    : (riid == IID_IAMMediaContent)
    ? GetInterface((IAMMediaContent*)this, ppv)

    : (riid == IID_IMediaPosition)
    ? GetInterface((IMediaPosition*)this, ppv)

#ifdef WITH_STREAMSELECT
    : (riid == IID_IAMStreamSelect)
    ? GetInterface((IAMStreamSelect*)this, ppv)
#endif

#ifdef WITH_TRACKINFO
    : (riid == __uuidof(ITrackInfo))
    ? GetInterface((ITrackInfo*)this, ppv)
#endif

    : (riid == IID_ISpecifyPropertyPages)
    ? GetInterface(static_cast<ISpecifyPropertyPages*>(this),ppv)

    : (riid == __uuidof(ISc68Prop))
    ?  GetInterface(static_cast<ISc68Prop*>(this),ppv)

    : __super::NonDelegatingQueryInterface(riid, ppv);
  if (!hr)
  DBG("%s() riid=%s ptr=%p -- %s\n", "QI"/*__FUNCTION__*/,
  Iname(riid), *ppv, !hr?"OK":"FAILED");
  return hr;
}

CUnknown *WINAPI Sc68Splitter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
  *phr = S_OK;
  CUnknown *punk = new Sc68Splitter(pUnk, phr);
  if(punk == nullptr) {
    *phr = E_OUTOFMEMORY;
  }
  return punk;
}

volatile bool Sc68Splitter::sc68_inited = false;

static char *s_argv[] = { "dshow68" };
void CALLBACK Sc68Splitter::StaticInit(BOOL bLoading, const CLSID *clsid)
{
  DBG("%s(loading=%d)\n",__FUNCTION__,bLoading);

  switch (bLoading) {

  case 0:
    if (sc68_inited) {
#ifdef _DEBUG
      Sc68Splitter::DumpSc68Error(__FUNCTION__,0);
#endif
      sc68_shutdown();
      sc68_inited = false;
    } else {
      DBG("%s() -- sc68 has not been init, no need to shutdown\n", __FUNCTION__);
    }
    break;

  default:
    if (sc68_inited) {
      DBG("%s() -- sc68 already init ?\n", __FUNCTION__);
      ASSERT(!"SC68 ALREADY INIT");
    } else {
      int err;
      sc68_init_t init;
      ZeroMemory(&init,sizeof(init));
      init.argc = sizeof(s_argv)/sizeof(*s_argv);
      init.argv = s_argv;
      init.flags.no_save_config = true;
      init.debug_clr_mask = -1;
#ifdef _DEBUG
      init.debug_clr_mask = 0;
      init.debug_set_mask = 0;
      init.msg_handler = (sc68_msg_t) msg_for_sc68;
#endif
      err = sc68_init(&init);
      DBG("%s() -- sc68_init() => %d\n", __FUNCTION__, err);
      sc68_inited = !err;
#ifdef _DEBUG
      Sc68Splitter::DumpSc68Error(__FUNCTION__,0);
#endif
    }
    break;
  }
  DBG("%s(loading=%d) => %s\n", __FUNCTION__, bLoading,
      (!bLoading == !sc68_inited) ? "OK":"FAIL");
}
