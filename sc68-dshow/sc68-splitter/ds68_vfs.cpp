/*
 * @file    ds68_vfs.cpp
 * @brief   sc68 for directshow - vfs
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

#define EC extern "C"
#define DECLARE_VFS Sc68VFS * vfs = ((Sc68VFS *)_vfs)

void Sc68VFS::Free()
{
  DBG("%s() ENTER #%d fragments\n", __FUNCTION__, m_ppSamples.size());
  /* /!\ Must be locked /!\ */
  for (std::vector<IMediaSample *>::iterator it = m_ppSamples.begin();
       it != m_ppSamples.end(); ++it) {
    ASSERT((*it));
    if (*it) {
      (*it)->Release();
      (*it) = nullptr;
    }
  }
  m_ppSamples.clear();
  DBG("%s() LEAVE #%d fragments\n", __FUNCTION__, m_ppSamples.size());
}

EC const char * name(vfs68_t * _vfs)
{
  DECLARE_VFS;
  return "sc68-splitter-VFS://";
}

EC int open(vfs68_t * _vfs)
{
  DECLARE_VFS;
  return vfs->Open();
}

int Sc68VFS::Open()
{
  DBG("%s()\n",__FUNCTION__);
  if (m_opened) return -1;
  Lock(); {
    ASSERT(m_ppSamples.size() == 0);
    Free();
    m_opened = true;
    m_eos = false;
    m_err = false;
    m_spl = 0;
    m_pos = 0;
  } Unlock();
  ResetEvent(m_event);
  return 0;
}

EC int close(vfs68_t * _vfs)
{
  DECLARE_VFS;
  return vfs->Close();
}

int Sc68VFS::Close()
{
  DBG("%s()\n",__FUNCTION__);
  if (!m_opened) return -1;
  Lock(); {
    m_opened = false;
    Free();
  } Unlock();
  return 0;
}

EC int length(vfs68_t * _vfs)
{
  DECLARE_VFS;
  return vfs->Length();
}

int Sc68VFS::Length()
{
  if (!m_opened) return -1;
  int len = -1;
  bool wait;

  for (;;) {
    Lock(); {
      if (m_err) {
        wait = false;
      } else if (m_eos) {
        len = 0;
        for (std::vector<IMediaSample *>::iterator it = m_ppSamples.begin();
             it != m_ppSamples.end(); ++it)
          len += (*it)->GetActualDataLength();
        wait = false;
      } else {
        wait = true;
      }
    } Unlock();
    if (!wait)
      break;
    DBG("%s() Have to wait\n",__FUNCTION__);
    WaitEvent();
    DBG("%s() Waking up\n",__FUNCTION__);
  }
  DBG("%s() => %d\n",__FUNCTION__, len);
  return len;
}

EC int seekb(vfs68_t * _vfs, int off)
{
  DECLARE_VFS;
  DBG("%s(%d)\n", __FUNCTION__, off);
  return vfs->Seek(off);
}

EC int seekf(vfs68_t * _vfs, int off)
{
  DECLARE_VFS;
  DBG("%s(%d)\n", __FUNCTION__, off);
  ASSERT(!"SEEK FORWARD SHOULD NOT HAPPEN");
  return -1;
}

int Sc68VFS::Seek(int off)
{
  if (!m_opened) return -1;
  if (!off) return 0;
  Lock();
  if (off < 0) {
    off = -off;
    while (off > 0) {
      if (m_spl >= (int)m_ppSamples.size())
        break;
      if (off <= m_pos) {
        m_pos -= off;
        off = 0;
      } else {
        off -= m_pos;
        if (m_spl > 0)
          m_pos = m_ppSamples[--m_spl]->GetActualDataLength();
        else {
          m_pos = 0;
          break;
        }
      }
    }
  } else {
    // Seek forward not implemented
  }
  Unlock();
  return -!!off;
}

EC int tell(vfs68_t * _vfs)
{
  DECLARE_VFS;
  return vfs->Tell();
}

int Sc68VFS::Tell()
{
  if (!m_opened) return -1;
  int tell = 0;
  Lock();
  if (m_spl < (int)m_ppSamples.size()) {
    for (std::vector<IMediaSample *>::iterator it=m_ppSamples.begin();
         it != m_ppSamples.end(); ++it) {
      if ( (*it) == m_ppSamples[m_spl]) {
        tell += m_pos;
        break;
      }
      tell += (*it)->GetActualDataLength();
    }
  }
  Unlock();
  DBG("%s() => %d\n",__FUNCTION__, tell);
  return tell;
}

EC int read(vfs68_t * _vfs, void *mem, int len)
{
  DECLARE_VFS;
  return vfs->Read(mem,len);
}

int Sc68VFS::Read(void *mem, int len)
{
  DBG("%s(%d)\n",__FUNCTION__,len);
  BYTE * dst = (BYTE *) mem;
  if (!m_opened || len < 0) return -1;
  int cnt = 0;
  bool exit = cnt >= len;
  while (!exit) {
    bool wait;
    int n = len - cnt;
    Lock();
    do {
      DBG("%s() %d/%d%s%s\n", __FUNCTION__,
          cnt, len, m_err?",ERR":"",m_eos?",EOS":"");
      if (m_err) {
        cnt = -1;
        wait = false;
        exit = true;
        break;
      }
      if (m_spl >= (int)m_ppSamples.size()) {
        wait = !m_eos;
        exit = m_eos;
        break;
      } else {
        BYTE * src;
        IMediaSample * pSample = m_ppSamples[m_spl];
        pSample->GetPointer(&src);
        const int max = pSample->GetActualDataLength();
        const int rem = max - m_pos;
        if (n > rem) n = rem;
        CopyMemory(dst+cnt,src+m_pos,n);
        cnt += n; ASSERT(cnt <= len);
        m_pos += n; ASSERT(m_pos <= max);

        exit = (cnt == len);
        if (m_pos == max && m_spl<(int)m_ppSamples.size()) {
          m_pos = 0;
          m_spl++;
        }
        wait = !exit && !m_eos;
      }
    } while (false);
    Unlock();
    if (wait) {
      DBG("%s() %d/%d Wait for event\n", __FUNCTION__, cnt, len);
      WaitEvent();
    }
  }
  DBG("%s(%d) => %d\n",__FUNCTION__,len,cnt);
  return cnt;
}

int Sc68VFS::Push(IMediaSample *pSample)
{
  DBG("%s() <%p,%d>\n",__FUNCTION__, pSample, pSample->GetActualDataLength());
  if (!m_opened || m_eos || !pSample) return -1;
  int n = pSample->GetActualDataLength();
  if (!n) return 0;
  Lock(); {
    pSample->AddRef();
    m_ppSamples.push_back(pSample);
  } Unlock();
  DBG("%s() pushed <%p,%d> as fragment #%d\n", __FUNCTION__,
      pSample, n, m_ppSamples.size());
  return n;
}

int Sc68VFS::PushEos()
{
  DBG("%s()\n",__FUNCTION__);
  if (!m_opened) return -1;
  Lock(); m_eos = true; Unlock();
  ::SetEvent(m_event);
  return 0;
}

int Sc68VFS::PushErr(HRESULT hr)
{
  DBG("%s()\n",__FUNCTION__);
  if (!m_opened) return -1;
  Lock(); m_eos = m_err = true; Unlock();
  ::SetEvent(m_event);
  return 0;
}

void destroy(vfs68_t * _vfs)
{
  DECLARE_VFS;
  vfs->Destroy();
}

void Sc68VFS::Destroy()
{
  DBG("%s()\n",__FUNCTION__);
}

Sc68VFS::Sc68VFS()
  : m_opened(false)
  , m_eos(false)
  , m_err(false)
  , m_spl(0)
  , m_pos(0)
{
  DBG("%s()\n",__FUNCTION__);
  m_event = CreateEvent(
    NULL,               // default security attributes
    FALSE,              // manual-reset event
    FALSE,              // initial state is nonsignaled
    NULL);                              // object name

  m_lock =  CreateMutex(NULL,FALSE,NULL);

  ZeroMemory(&vfs,sizeof(vfs));
  vfs.open    = open;
  vfs.close   = close;
  vfs.tell    = tell;
  vfs.length  = length;
  vfs.seekb   = seekb;
  vfs.seekf   = seekf;
  vfs.read    = read;
  vfs.destroy = destroy;
}

Sc68VFS::~Sc68VFS()
{
  DBG("%s()\n",__FUNCTION__);
  if (m_event) CloseHandle(m_event);
  if (m_lock) CloseHandle(m_lock);
}
