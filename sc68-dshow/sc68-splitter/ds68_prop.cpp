/*
 * @file    ds68_prop.cpp
 * @brief   sc68 for directshow - properties page
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

HRESULT Sc68Splitter::GetASID(int * asid)
{
  HRESULT hr = S_OK;
  int v = sc68_cntl(m_sc68, SC68_GET_ASID);
  if (v == -1)
    hr = E_FAIL;
  else if (!asid)
    hr = E_POINTER;
  else
    *asid = v;
  DBG("%s()=%d => %s\n",__FUNCTION__,v,!hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68Splitter::SetASID(int asid)
{
  int v = sc68_cntl(m_sc68, SC68_SET_ASID,asid);
  HRESULT hr = (v == -1) ? E_INVALIDARG : S_OK;
  DBG("%s(%d)=%d => %s\n",__FUNCTION__,asid,v,!hr?"OK":"FAIL");
  return hr;
}

HRESULT Sc68Splitter::GetPages(CAUUID *pPages)
{
  HRESULT hr;
  if (!pPages)
    hr = E_POINTER;
  else if (pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID)), !pPages->pElems)
    hr = E_OUTOFMEMORY;
  else {
    pPages->pElems[0] = __uuidof(Sc68Prop);
    hr = S_OK;
  }
  DBG("%s() => %s\n",__FUNCTION__,!hr?"OK":"FAIL");
  return hr;
}

Sc68Prop::Sc68Prop(IUnknown *pUnk)
  : /*CBasePropertyPage(NAME("Sc68Prop"), pUnk, IDD_DLG_CONFIG, IDS_PROPPAGE_TITLE)
      , */m_pIf(nullptr)
          , m_asid(SC68_ASID_OFF)
{
  DBG("%s()\n",__FUNCTION__);
}

HRESULT Sc68Prop::OnConnect(IUnknown *pUnk)
{
  HRESULT hr;
  if (!pUnk)
    hr = E_POINTER;
  else {
    ASSERT(m_pIf == NULL);
    hr = pUnk->QueryInterface(__uuidof(ISc68Prop), reinterpret_cast<void**>(&m_pIf));
  }
  DBG("%s()\n",__FUNCTION__);
  return hr;
}

HRESULT Sc68Prop::OnActivate(void)
{
  HRESULT hr = S_OK;

  //INITCOMMONCONTROLSEX icc;
  //icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
  //icc.dwICC = ICC_BAR_CLASSES;
  //if (InitCommonControlsEx(&icc) == FALSE)
  //{
  //    return E_FAIL;
  //}

  //ASSERT(m_pIf != NULL);
  //HRESULT hr = m_pIf->GetASID(&m_asid);
  //if (SUCCEEDED(hr))
  //{
  //    SendDlgItemMessage(m_Dlg, IDC_SLIDER1, TBM_SETRANGE, 0,
  //        MAKELONG(SATURATION_MIN, SATURATION_MAX));

  //    SendDlgItemMessage(m_Dlg, IDC_SLIDER1, TBM_SETTICFREQ,
  //        (SATURATION_MAX - SATURATION_MIN) / 10, 0);

  //    SendDlgItemMessage(m_Dlg, IDC_SLIDER1, TBM_SETPOS, 1, m_lVal);
  //}
  return hr;
}

HRESULT Sc68Prop::Activate(HWND hWndParent,LPCRECT pRect,BOOL bModal)
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}
HRESULT Sc68Prop::Apply()
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}

HRESULT Sc68Prop::Deactivate()
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}

HRESULT Sc68Prop::GetPageInfo(PROPPAGEINFO *pPageInfo)
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}
HRESULT Sc68Prop::IsPageDirty()
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}
HRESULT Sc68Prop::Move(LPCRECT pRect)
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}
HRESULT Sc68Prop::SetObjects(ULONG cObjects,IUnknown **ppUnk)
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}
HRESULT Sc68Prop::SetPageSite(IPropertyPageSite *pPageSite)
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}
HRESULT Sc68Prop::Show(UINT nCmdShow)
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}

HRESULT Sc68Prop::Help(LPCOLESTR pszHelpDir)
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}

HRESULT Sc68Prop::TranslateAccelerator(MSG *pMsg)
{
  HRESULT hr = E_NOTIMPL;
  DBG("%s -> [%s]\n", __FUNCTION__, !hr ?"OK":"FAIL");
  return hr;
}
