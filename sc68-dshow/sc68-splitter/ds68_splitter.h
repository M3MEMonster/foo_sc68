/**
 * @ingroup   sc68_directshow
 * @file      ds68_splitter.h
 * @brief     Class Sc68Splitter
 * @author    Benjamin Gerard
 * @date      2014/06
 */

#pragma once

/**
 * @defgroup sc68_directshow  sc68 for DirectShow
 * @ingroup  sc68_players
 * @{
 */

#include "ds68_prop.h"

#define WITH_TRACKINFO
#define WITH_STREAMSELECT

#ifdef WITH_TRACKINFO
#include "ds68_trackinfo.h"
#endif

[uuid(CLID_SC68SPLITTER_STR)]
/// sc68 DirectShow Splitter filter.
class Sc68Splitter : public CSource
                   , public IAMMediaContent
                   , public IMediaPosition
                   , public ISc68Prop
                   , public ISpecifyPropertyPages
#ifdef WITH_TRACKINFO
                   , public ITrackInfo
#endif
#ifdef WITH_STREAMSELECT
                   , public IAMStreamSelect
#endif

{

private:
  Sc68InpPin * m_InpPin; ///< Pointer to input pin (byte stream).
  sc68_t * m_sc68;       ///< sc68 instance.
  sc68_disk_t m_disk;    ///< loaded disk
  sc68_minfo_t m_info;   ///< current track info
  int m_code;            ///< Last sc68_process() return code.

  int m_track;           ///< Track select: 0=all
  // bool m_allin1;
  // int m_spr;
  

public:
  // IUnknown
  DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

  // IDispatch
  STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) {return E_NOTIMPL;}
  STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) {return E_NOTIMPL;}
  STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid) {return E_NOTIMPL;}
  STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) {return E_NOTIMPL;}

  // constructor & destructor
  Sc68Splitter(LPUNKNOWN pUnk, HRESULT *phr);
  virtual ~Sc68Splitter();
  static void CALLBACK StaticInit(BOOL bLoading, const CLSID *clsid);
  static CUnknown *WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

  HRESULT CreateSc68(sc68_disk_t disk);
  HRESULT DestroySc68();
  HRESULT SetDisk(sc68_disk_t disk);

  int FillBuffer(IMediaSample *pSample);
  void DumpSc68Error(const char * func = 0);
  static void DumpSc68Error(const char * func, sc68_t * m_sc68);

  HRESULT GetSegment(REFERENCE_TIME &tStart,REFERENCE_TIME &tStop);

  int GetSamplingRate();
  int SetSamplingRate(int spr);




  // Implements CBaseFilter
  virtual int GetPinCount();
  virtual CBasePin* GetPin(int n);

  //  Override
  //virtual HRESULT Active();
  //virtual HRESULT Inactive();

  //  Override CBaseFilter
  STDMETHODIMP Run(REFERENCE_TIME tStart);
  STDMETHODIMP Stop();
  STDMETHODIMP Pause();

  // IAMMediaContent
#define GET(name) STDMETHODIMP get_##name(BSTR *pbstr##name)
  GET(AuthorName);
  GET(BaseURL);
  GET(Copyright);
  GET(Description);
  GET(LogoIconURL);
  GET(LogoURL);
  GET(MoreInfoBannerImage);
  GET(MoreInfoBannerURL);
  GET(MoreInfoText);
  GET(MoreInfoURL);
  GET(Rating);
  GET(Title);
  GET(WatermarkURL);

  // IMediaPosition
  STDMETHODIMP CanSeekBackward(LONG *pCanSeek);
  STDMETHODIMP CanSeekForward(LONG *pCanSeek);
  STDMETHODIMP get_CurrentPosition(REFTIME *pllTime);
  STDMETHODIMP get_Duration(REFTIME *plength);
  STDMETHODIMP get_PrerollTime(REFTIME *pllTime);
  STDMETHODIMP get_Rate(double *pdRate);
  STDMETHODIMP get_StopTime(REFTIME *pllTime);
  STDMETHODIMP put_CurrentPosition(REFTIME llTime);
  STDMETHODIMP put_PrerollTime(REFTIME llTime);
  STDMETHODIMP put_Rate(double dRate);
  STDMETHODIMP put_StopTime(REFTIME llTime);

#ifdef WITH_TRACKINFO
  // ITrackInfo
  STDMETHODIMP_(UINT) GetTrackCount();
  STDMETHODIMP_(BOOL) GetTrackInfo(UINT aTrackIdx, struct TrackElement * pStructureToFill);
  STDMETHODIMP_(BOOL) GetTrackExtendedInfo(UINT aTrackIdx, void* pStructureToFill);
  STDMETHODIMP_(BSTR) GetTrackCodecID(UINT aTrackIdx);
  STDMETHODIMP_(BSTR) GetTrackName(UINT aTrackIdx);
  STDMETHODIMP_(BSTR) GetTrackCodecName(UINT aTrackIdx);
  STDMETHODIMP_(BSTR) GetTrackCodecInfoURL(UINT aTrackIdx);
  STDMETHODIMP_(BSTR) GetTrackCodecDownloadURL(UINT aTrackIdx);
#endif

#ifdef WITH_STREAMSELECT
  /// The Count() method retrieves the number of available streams.
  STDMETHODIMP Count(DWORD *pcStreams);
  /// The Enable() method enables or disables a given stream.
  STDMETHODIMP Enable(long lIndex,DWORD dwFlags);
  STDMETHODIMP Info(
    long lIndex,
    AM_MEDIA_TYPE **ppmt,
    DWORD *pdwFlags,
    LCID *plcid,
    DWORD *pdwGroup,
    WCHAR **ppszName,
    IUnknown **ppObject,
    IUnknown **ppUnk);
#endif
  // Implements ISc68Prop
  STDMETHODIMP GetASID(int * asid);
  STDMETHODIMP SetASID(int asid);

  // Implements ISpecifyPropertyPages
  STDMETHODIMP GetPages(CAUUID *pPages);

  volatile static bool sc68_inited;
};

/**
 * @}
 */
