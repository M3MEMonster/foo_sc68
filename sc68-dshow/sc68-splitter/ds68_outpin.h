/**
 * @ingroup   sc68_directshow
 * @file      ds68_outpin.h
 * @brief     class Sc68OutPin (output pin).
 * @author    Benjamin Gerard
 * @date      2014/06
 */

#pragma once

/**
 * @addtogroup sc68_directshow
 * @{
 */

class Sc68OutPin : public CSourceStream
{
public:
  Sc68OutPin(Sc68Splitter *pFilter, CCritSec *pLock, HRESULT *phr);
  int GetSamplingRate();

  virtual HRESULT Inactive();
  virtual HRESULT Active();

  //virtual HRESULT CheckConnect(IPin *pPin);
  //STDMETHODIMP ReceiveConnection(IPin *pConnector,const AM_MEDIA_TYPE *pmt);
  //virtual HRESULT AgreeMediaType(IPin *pReceivePin, const CMediaType *pmt);
  //virtual HRESULT AttemptConnection(IPin *pReceivePin, const CMediaType *pmt);
  //virtual HRESULT TryMediaTypes(IPin *pReceivePin, const CMediaType *pmt, IEnumMediaTypes *pEnum);
  //virtual HRESULT CompleteConnect(IPin *pReceivePin);

  // CBaseOutpuPin
  virtual HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * ppropInputRequest);

  // CBasePin
  virtual HRESULT GetMediaType(CMediaType *pMediaType);

  // virtual HRESULT GetMediaType(int iPosition,  CMediaType *pMediaType);
  STDMETHODIMP BeginFlush();
  STDMETHODIMP EndFlush();

  // CSourceStream
  HRESULT FillBuffer(IMediaSample *pSample);
  virtual HRESULT Run(/*REFERENCE_TIME tStart*/);

  // Thread callbacks
  virtual HRESULT OnThreadCreate();
  virtual HRESULT OnThreadDestroy();
  virtual HRESULT OnThreadStartPlay();

  // Mine
  Sc68Splitter * GetFilter() const { return (Sc68Splitter *)m_pFilter; }
  const CMediaType & GetMediaType() const { return m_MediaType; }
protected:
  CMediaType m_MediaType;
  WAVEFORMATEX m_waveformatex;
};

/**
 * @}
 */
