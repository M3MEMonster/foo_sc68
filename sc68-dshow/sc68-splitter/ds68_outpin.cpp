/*
 * @file    ds68_outpin.cpp
 * @brief   sc68 for directshow - output pin
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

/////////////////////////////////////////////////////////////////////
// sc68 splitter - Output Pin
/////////////////////////////////////////////////////////////////////

#include "ds68_headers.h"

#define CParent __super

HRESULT Sc68OutPin::Inactive()
{
  //CAutoLock cAutoLock(m_pLock);
  HRESULT hr = CParent::Inactive();
  DBG("%s() [%c,%c] => %d\n", __FUNCTION__,
      IsStopped()?'S':'R', IsConnected()?'C':'U', hr);
  return hr;
}

HRESULT Sc68OutPin::Active()
{
  //CAutoLock cAutoLock(m_pLock);
  HRESULT hr = CParent::Active();
  DBG("%s() [%c,%c] => %d\n", __FUNCTION__,
      IsStopped()?'S':'R', IsConnected()?'C':'U', hr);
  return hr;
}

HRESULT Sc68OutPin::Run()
{
  HRESULT hr = CParent::Run();
  DBG("%s() [%c,%c] => %d\n", __FUNCTION__,
      IsStopped()?'S':'R', IsConnected()?'C':'U', hr);
  return hr;
}

HRESULT Sc68OutPin::GetMediaType(CMediaType *pMediaType)
{
  HRESULT hr = NOERROR;
  if (pMediaType)
    *pMediaType = m_MediaType;
  //DBG(L"%s(%s) => %s\n",__FUNCTIONW__,this->Name(),MediaTypeW(m_MediaType));
  return hr;
}

/////////////////////////////////////////////////////////////////////
// Output Pin
/////////////////////////////////////////////////////////////////////
Sc68OutPin::Sc68OutPin(Sc68Splitter *pFilter, CCritSec *pLock, HRESULT *phr) :
  CSourceStream(_T("Sc68OutPin"), phr, pFilter, L"PCM Out")
{
  m_bTryMyTypesFirst = true;

  //////////////////////
  // Setup Media type //
  //////////////////////
  const int spr = pFilter->GetSamplingRate();

  // Setup PCM format
  m_waveformatex.wFormatTag = WAVE_FORMAT_PCM;
  m_waveformatex.nChannels  = 2;
  m_waveformatex.wBitsPerSample = 16;
  m_waveformatex.nSamplesPerSec = spr;
  m_waveformatex.cbSize = 0;
  m_waveformatex.nBlockAlign =
    m_waveformatex.nChannels * m_waveformatex.wBitsPerSample >> 3;
  m_waveformatex.nAvgBytesPerSec =
    m_waveformatex.nBlockAlign * m_waveformatex.nSamplesPerSec;

  // Setup MediaType with PCM format
  CreateAudioMediaType(&m_waveformatex,&m_MediaType,TRUE);
  DBG("%s() Created Mediatype -- %s\n",__FUNCTION__,MediaTypeA(m_MediaType));
  pFilter->SetSamplingRate(m_waveformatex.nSamplesPerSec);
}

int Sc68OutPin::GetSamplingRate() {
  return m_waveformatex.nSamplesPerSec;
}

HRESULT Sc68OutPin::BeginFlush()
{
  HRESULT hr = CParent::BeginFlush();
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68OutPin::EndFlush()
{
  HRESULT hr = CParent::EndFlush();
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68OutPin::DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * ppropInputRequest)
{
  HRESULT hr = E_POINTER;
  ALLOCATOR_PROPERTIES act;

  if (pAlloc && ppropInputRequest) {
    int totalBytes = m_waveformatex.nAvgBytesPerSec >> 2; // 250 ms
    ppropInputRequest->cbAlign  = 4;
    ppropInputRequest->cBuffers = 16;
    ppropInputRequest->cbBuffer =
      ((totalBytes/ppropInputRequest->cBuffers)+7) & ~3;

    hr = pAlloc->SetProperties(ppropInputRequest,&act);
    DBG("%s() OUT -- align=%d prefix=%d buffer=%d x %d\n", __FUNCTION__,
        act.cbAlign, act.cbPrefix, act.cBuffers, act.cbBuffer);
  }
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68OutPin::OnThreadCreate()
{
  HRESULT hr = __super::OnThreadCreate();

  REFERENCE_TIME tStart=0,tStop=0;
  if (!GetFilter()->GetSegment(tStart, tStop))
    DeliverNewSegment(tStart,tStop,1.0);

  DBG("%s() => %d\n", __FUNCTION__, hr);
  return hr;
}

HRESULT Sc68OutPin::OnThreadDestroy()
{
  HRESULT hr = __super::OnThreadDestroy();
  DBG("%s() => %d\n", __FUNCTION__, hr);
  return hr;
}

HRESULT Sc68OutPin::OnThreadStartPlay()
{
  HRESULT hr = __super::OnThreadStartPlay();
  DBG("%s() => %d\n", __FUNCTION__, hr);
  return hr;
}

HRESULT Sc68OutPin::FillBuffer(IMediaSample *pSample)
{
  Sc68Splitter * splitter = GetFilter();
  int code = splitter->FillBuffer(pSample);
  if (code == SC68_ERROR) {
    return S_FALSE;
  } else if (code & SC68_END) {
    HRESULT hr = DeliverEndOfStream();
    DBG("%s() -- End detected, send EOS => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
    return S_FALSE;
  } else if (code & SC68_CHANGE) {
    REFERENCE_TIME tStart=0,tStop=0;
    if (!splitter->GetSegment(tStart, tStop))
      DeliverNewSegment(tStart,tStop,1.0);
    DBG("%s() -- Track changed (%lld %lld)\n",__FUNCTION__, tStart, tStop);
  }
  return S_OK;
}
