/**
 * @ingroup   sc68_directshow
 * @file      ds68_inppin.h
 * @brief     class Sc68InpPin (Input pin).
 * @author    Benjamin Gerard
 * @date      2014/06
 */

#pragma once

/**
 * @addtogroup sc68_directshow
 * @{
 */

class Sc68Splitter;
class Sc68InpPin;

class Sc68PullPin : public CPullPin, public Sc68VFS
{
public:
  Sc68PullPin(Sc68InpPin * pPin);
  ~Sc68PullPin();

  //  Overrides CPullPin
  virtual HRESULT Active();
  virtual HRESULT Inactive();
  virtual HRESULT Receive(IMediaSample *pSample);
  virtual HRESULT EndOfStream();
  virtual void OnError(HRESULT hr);

  // Implements CPullPin
  virtual HRESULT BeginFlush();
  virtual HRESULT EndFlush();

protected:
  Sc68InpPin  *m_pInpPin;
};

class Sc68InpPin : public CBasePin, public CCritSec
{
protected:
  CMediaType m_MediaTypes[2];
  Sc68PullPin  m_PullPin;

public:
  Sc68InpPin(Sc68Splitter *pFilter, CCritSec *pLock, HRESULT *phr);
  // HRESULT GotSc68(const CDynArray & mem);

  Sc68Splitter *GetSc68Splitter() const { return (Sc68Splitter *)(m_pFilter); }

  virtual HRESULT GetMediaType(int iPosition,  CMediaType *pMediaType);
  virtual HRESULT CheckMediaType(const CMediaType *pmt);
  virtual HRESULT CompleteConnect(IPin *pReceivePin);
  virtual HRESULT CheckConnect(IPin *pPin);

  virtual HRESULT BreakConnect();
  virtual HRESULT Active();
  virtual HRESULT Inactive();

  STDMETHODIMP Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);

  // IPin abstract
  STDMETHODIMP BeginFlush();
  STDMETHODIMP EndFlush();

};

/**
 * @}
 */
