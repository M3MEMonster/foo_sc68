/**
 * @ingroup   sc68_directshow
 * @file      ds68_prop.h
 * @brief     Interface ISc68Prop (property page)
 * @author    Benjamin Gerard
 * @date      2014/06
 */

#pragma once

/**
 * @addtogroup sc68_directshow
 * @{
 */

#include "ds68_headers.h"

[uuid(IID_SC68PROP_STR)]
interface ISc68Prop: public IUnknown
{
  STDMETHOD(GetASID)(int * asid) = 0;
  STDMETHOD(SetASID)(int asid) = 0;
};

[uuid(CLID_SC68PROP_STR)]
class Sc68Prop/* : public CBasePropertyPage*/
  : public IPropertyPage
{
private:
  ISc68Prop  *m_pIf;    // Pointer to the filter's custom interface.

  int m_asid;

public:
  Sc68Prop(IUnknown *pUnk);

  HRESULT OnConnect(IUnknown *pUnk);
  HRESULT OnActivate(void);

  /* IPropertyPage */
  STDMETHOD(Activate)(HWND hWndParent,LPCRECT pRect,BOOL bModal);
  STDMETHOD(Apply)();
  STDMETHOD(Deactivate)();
  STDMETHOD(GetPageInfo)(PROPPAGEINFO *pPageInfo);
  STDMETHOD(IsPageDirty)();
  STDMETHOD(Move)(LPCRECT pRect);
  STDMETHOD(SetObjects)(ULONG cObjects,IUnknown **ppUnk);
  STDMETHOD(SetPageSite)(IPropertyPageSite *pPageSite);
  STDMETHOD(Show)(UINT nCmdShow);
  STDMETHOD(Help)(LPCOLESTR pszHelpDir);
  STDMETHOD(TranslateAccelerator)(MSG *pMsg);

  /* ... */
};

/**
 * @}
 */
