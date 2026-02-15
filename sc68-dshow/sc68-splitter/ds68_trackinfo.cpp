/*
 * @file    ds68_trackinfo.cpp
 * @brief   sc68 for directshow - track info
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

#ifdef WITH_TRACKINFO

#define DECLARE_MAXTRACKS const UINT maxTracks = (m_info.tracks + (m_info.tracks > 1))

STDMETHODIMP_(UINT)
Sc68Splitter::GetTrackCount()
{
  DECLARE_MAXTRACKS;
  DBG("%s() => %u\n", __FUNCTION__, maxTracks);
  return maxTracks;
}

STDMETHODIMP_(BOOL)
Sc68Splitter::GetTrackInfo(UINT aTrackIdx, struct TrackElement * pStructureToFill)
{
  DECLARE_MAXTRACKS;
  BOOL res = FALSE;
  if (aTrackIdx <= maxTracks  && pStructureToFill) {
    pStructureToFill->FlagDefault = aTrackIdx == 0;
    pStructureToFill->FlagForced  = false;
    pStructureToFill->FlagLacing  = false;
    pStructureToFill->MaxCache    = 0;
    pStructureToFill->MinCache    = 0;
    pStructureToFill->Size        = sizeof(*pStructureToFill);
    pStructureToFill->Type        = TypeAudio;
    res = TRUE;
  }
  DBG("%s(%d) => %u\n", __FUNCTION__, aTrackIdx, res?"OK":"FAIL");
  return res;
}

STDMETHODIMP_(BOOL)
Sc68Splitter::GetTrackExtendedInfo(UINT aTrackIdx, void* pStructureToFill)
{
  DECLARE_MAXTRACKS;
  BOOL res = FALSE;
  if (aTrackIdx <= maxTracks  && pStructureToFill) {
    struct TrackExtendedInfoAudio * p
      = (struct TrackExtendedInfoAudio *)pStructureToFill;
    p->BitDepth = 16;
    p->Channels = 2;
    p->OutputSamplingFrequency = (float)GetSamplingRate();
    p->SamplingFreq = p->OutputSamplingFrequency;
    p->Size = sizeof(*p);
    res = TRUE;
  }
  DBG("%s(%d) => %u\n", __FUNCTION__, aTrackIdx, res?"OK":"FAIL");
  return res;
}

STDMETHODIMP_(BSTR)
Sc68Splitter::GetTrackName(UINT aTrackIdx)
{
  DECLARE_MAXTRACKS;
  BSTR str = nullptr;
  if (aTrackIdx <= maxTracks) {
    char tmp[256];
    const char * use = tmp;
    if (!aTrackIdx) {
      if (maxTracks > 1)
        _snprintf(tmp,sizeof(tmp),"ALL. %s", m_info.album);
      else if (strcmp68(m_info.album,m_info.title))
        _snprintf(tmp,sizeof(tmp),"%s - %s", m_info.album, m_info.title);
      else
        use = m_info.album;
    } else {
      sc68_minfo_t info, *pinfo;
      if (aTrackIdx == m_info.trk.track)
        pinfo = &m_info;
      else if (!sc68_music_info(m_sc68,&info,aTrackIdx,m_disk))
        pinfo = &info;
      else
        use = NULL, pinfo = NULL;

      if (pinfo) {
        if (maxTracks > 1)
          if (strcmp68(pinfo->album,pinfo->title))
            _snprintf(tmp,sizeof(tmp),"%02u. %s - %s", pinfo->trk.track, pinfo->album, pinfo->title);
          else
            _snprintf(tmp,sizeof(tmp),"%02u. %s", pinfo->trk.track, pinfo->album);
        else
          if (strcmp68(pinfo->album,pinfo->title))
            _snprintf(tmp,sizeof(tmp),"%s - %s", pinfo->album, pinfo->title);
          else
            use = pinfo->album;
      }
      tmp[sizeof(tmp)-1] = 0;
    }
    if (use)
      BSTRset(&str, use);
  }
  DBG(L"%s(%d) => \"%s\"\n", __FUNCTION__, aTrackIdx, str?str:L"(nil)");
  return str;
}

STDMETHODIMP_(BSTR)
Sc68Splitter::GetTrackCodecID(UINT aTrackIdx)
{
  return nullptr;
}

STDMETHODIMP_(BSTR)
Sc68Splitter::GetTrackCodecName(UINT aTrackIdx)
{
  return nullptr;
}

STDMETHODIMP_(BSTR)
Sc68Splitter::GetTrackCodecInfoURL(UINT aTrackIdx)
{
  return nullptr;
}

STDMETHODIMP_(BSTR)
Sc68Splitter::GetTrackCodecDownloadURL(UINT aTrackIdx)
{
  return nullptr;
}

#endif
