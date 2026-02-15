/*
 * @file    ds68_inppin.cpp
 * @brief   sc68 for directshow - input pin
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
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////
//// Sc68PullPin - Overrides CPullpin and Sc68VFS
///////////////////////////////////////////////////////////////////////

Sc68PullPin::Sc68PullPin(Sc68InpPin * pPin) : m_pInpPin(pPin)
{
  DBG(L"%s() buddy=%s\n",__FUNCTIONW__,pPin->Name());
}

Sc68PullPin::~Sc68PullPin()
{
  DBG("%s()\n",__FUNCTION__);
}

HRESULT Sc68PullPin::BeginFlush()
{
  DBG("%s()\n",__FUNCTION__);
  return S_OK;
}

HRESULT Sc68PullPin::EndFlush()
{
  DBG("%s()\n",__FUNCTION__);
  return S_OK;
}

HRESULT Sc68PullPin::Active()
{
  HRESULT hr = E_FAIL;
  DBG("%s()\n",__FUNCTION__);
  if (!Sc68VFS::Open()) {
    DBG("%s() Opened\n",__FUNCTION__);
    hr = __super::Active();
    if (!hr) {
      sc68_disk_t disk = sc68_load_disk(GetVFS());
      DBG("%s() disk -> %s\n",__FUNCTION__, disk?"OK":"FAIL");
      if (!disk) {
        hr = E_FAIL;
      } else {
        hr = m_pInpPin->GetSc68Splitter()->CreateSc68(disk);
      }
    }
    Sc68VFS::Close();
  }
  return hr;
}

HRESULT Sc68PullPin::Inactive()
{
  Sc68VFS::Close();
  return __super::Inactive();
}

HRESULT Sc68PullPin::Receive(IMediaSample *pSample)
{
  CheckPointer(pSample,E_POINTER);
  DBG("%s() pushing <%p,%d>\n", __FUNCTION__, pSample,pSample->GetActualDataLength());
  int n = Sc68VFS::Push(pSample);
  DBG("%s() pushed <%p,%d>\n",__FUNCTION__, pSample, n);
  return (n < 0) ? E_FAIL : S_OK;
}

HRESULT Sc68PullPin::EndOfStream()
{
  HRESULT hr = S_OK;
  DBG("%s()\n",__FUNCTION__);
  if (PushEos())
    hr = E_FAIL;
  DBG("%s() => %s\n",__FUNCTION__,!hr?"OK":"FAIL");
  return hr;
}

void Sc68PullPin::OnError(HRESULT hr)
{
  DBG("%s() => %s\n",__FUNCTION__,!hr?"OK":"FAIL");
  PushErr(hr);

}

HRESULT Sc68InpPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
  HRESULT hr;

  ASSERT(pMediaType);

  if (iPosition < 0)
    hr = E_INVALIDARG;
  else if (iPosition >= sizeof(m_MediaTypes)/sizeof(*m_MediaTypes))
    hr = VFW_S_NO_MORE_ITEMS;
  else {
    hr = S_OK;
    if (pMediaType)
      *pMediaType = m_MediaTypes[iPosition];
  }
  //DBG(L"%s(%s)[%d] => %d (%s)\n",__FUNCTIONW__,this->Name(),iPosition,hr,hr==S_OK?MediaTypeW(pMediaType):L"n/a");
  return hr;
}

HRESULT Sc68InpPin::CheckMediaType(const CMediaType *pmt)
{
  HRESULT hr;
  CheckPointer(pmt, E_POINTER);
  const GUID & T = *pmt->Type();
  const GUID & S = *pmt->Subtype();

  hr = (T == MEDIATYPE_Stream) && (S == MEDIATYPE_SC68 || S == MEDIASUBTYPE_NULL) ? S_OK : S_FALSE;
  if (hr) {
    DBG(L"%s(%s,%s) => %s\n", __FUNCTIONW__, Name(), MediaTypeW(pmt), hr==S_OK ? L"OK" : L"FAILED");
  }
  return S_OK;
}

HRESULT Sc68InpPin::CompleteConnect(IPin *pReceivePin)
{
  HRESULT hr = CBasePin::CompleteConnect(pReceivePin);
  DBG("%s() => %s\n", __FUNCTION__, hr==S_OK?"OK":"FAIL");
  return hr;
}

HRESULT Sc68InpPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
  HRESULT hr = CBasePin::Connect(pReceivePin, pmt);
  DBG("%s() Type:%s => %s\n", __FUNCTION__,
      pmt ? MediaTypeA(CMediaType(*pmt)) : "N/A",
      hr==S_OK?"OK":"FAIL");
  return hr;
}

HRESULT Sc68InpPin::Inactive()
{
  HRESULT hr = m_PullPin.Inactive();
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68InpPin::Active()
{
  HRESULT hr = S_OK;
  hr = m_PullPin.Active();
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68InpPin::BeginFlush()
{
  HRESULT hr = S_OK; /*CBasePin::BeginFlush();*/
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68InpPin::EndFlush()
{
  HRESULT hr = S_OK; /*CBasePin::EndFlush();*/
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68InpPin::CheckConnect(IPin *pPin)
{
  HRESULT       hr = m_PullPin.Connect(pPin,NULL,TRUE);
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68InpPin::BreakConnect()
{
  HRESULT       hr = m_PullPin.Disconnect();
  DBG("%s() => %s\n",__FUNCTION__, !hr?"OK":"FAIL");
  return hr;
}

Sc68InpPin::Sc68InpPin(Sc68Splitter *pFilter, CCritSec *pLock, HRESULT *phr)
  : CBasePin(_T("Sc68InpPin"), pFilter, pLock, phr, L"Sc68",PINDIR_INPUT)
  , m_PullPin(this)
{
  m_MediaTypes[0].SetType(&MEDIATYPE_Stream);
  m_MediaTypes[0].SetSubtype(&MEDIATYPE_SC68);
  m_MediaTypes[1].SetType(&MEDIATYPE_Stream);
  m_MediaTypes[1].SetSubtype(&MEDIASUBTYPE_NULL);
}
