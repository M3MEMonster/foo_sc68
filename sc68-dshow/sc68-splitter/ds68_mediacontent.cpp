/*
 * @file    ds68_mediacontent.cpp
 * @brief   sc68 for directshow - media content (metatags)
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

#define _CRT_SECURE_NO_WARNINGS
#include "ds68_headers.h"
#include "sc68/file68_str.h"

static const char *get_trk_tag(sc68_cinfo_t * cinfo, const char * key)
{
  for (int i=0; i<cinfo->tags; ++i)
    if (!strcmp68(cinfo->tag[i].key, key))
      return cinfo->tag[i].val;
  return 0;
}

static const char *get_tag(sc68_minfo_t * minfo, const char * key)
{
  const char * val;
  //DBG("Looking for tag #%d \"%s\"\n",minfo->trk.track, key);
  if (val = get_trk_tag(&minfo->trk, key), !val)
    val = get_trk_tag(&minfo->dsk, key);
  //DBG("\"%s\" => \"%s\"\n",key, val?val:"(nil)");
  return val;
}

#ifdef GET
#undef GET
#endif

#define GET(name) HRESULT Sc68Splitter::get_##name(BSTR *pbstr/*##name*/)

GET(AuthorName)  {
  if (!m_disk || m_info.tracks <= 0)
    return VFW_E_NOT_FOUND;
  const char * artist = get_tag(&m_info, TAG68_AKA);
  if (!artist || !*artist)
    artist = m_info.artist;
  if (!artist || !*artist)
    return VFW_E_NOT_FOUND;
  return BSTRset(pbstr/*AuthorName*/ ,artist);
}

GET(Title) {
  char tmp[256];
  const char * single = 0;
  if (!m_disk)
    return VFW_E_NOT_FOUND;
  if (!m_info.title || !*m_info.title)
    // No title
    if (!m_info.album|| !*m_info.album)
      // No album
      return VFW_E_NOT_FOUND;
    else
      // Album only
      single = m_info.album;
  else if (!m_info.album || !*m_info.album || m_info.album == m_info.title || !_stricmp(m_info.album,m_info.title))
    // No album or album same as title
    single = m_info.title;

  if (single) {
    if (m_info.tracks > 1) {
      _snprintf(tmp,sizeof(tmp),"%s - track #%02d", single, m_info.trk.track);
      single = tmp;
    }
  } else {
    // album and title differs
    if (m_info.tracks > 1)
      _snprintf(tmp,sizeof(tmp),"%s - %02d. %s",m_info.album,m_info.trk.track,m_info.title);
    else
      _snprintf(tmp,sizeof(tmp),"%s - %s",m_info.album,m_info.title);
    single = tmp;
  }
  tmp[sizeof(tmp)-1] = 0;
  return BSTRset(pbstr/*Title*/,single);
}

/**
 * Gets a base URL for the related web content.
 */
GET(BaseURL) {
  //DBG("%s()\n", __FUNCTION__);
  return VFW_E_NOT_FOUND;
}

/**
 * Gets copyright information.
 */
GET(Copyright) {
  const char * s = get_tag(&m_info, TAG68_COPYRIGHT);
  if (!s)
    return VFW_E_NOT_FOUND;
  return BSTRset(pbstr/*Copyright*/,s);
}

/**
 * Gets a description of the content.
 */
GET(Description) {
  const char * s = get_tag(&m_info, TAG68_COMMENT);
  if (!s)
    return VFW_E_NOT_FOUND;
  return BSTRset(pbstr/*Description*/,s);
  
}

/**
 * Gets a URL for the logo icon.
 */
GET(LogoIconURL) {
  //DBG("%s()\n", __FUNCTION__);
  return VFW_E_NOT_FOUND;
}

// Gets a URL for the logo.
GET(LogoURL) {
  //DBG("%s()\n", __FUNCTION__);
  return VFW_E_NOT_FOUND;
}

// Gets an image for a related-information banner.
GET(MoreInfoBannerImage) {
  //DBG("%s()\n", __FUNCTION__);
  return VFW_E_NOT_FOUND;
}

// Gets a URL for a related-information banner.
GET(MoreInfoBannerURL) {
  //DBG("%s()\n", __FUNCTION__);
  return VFW_E_NOT_FOUND;
}

// Gets additional information as text.
GET(MoreInfoText) {
  //DBG("%s()\n", __FUNCTION__);
  return VFW_E_NOT_FOUND;
}

// Gets a URL for additional information about the content.
GET(MoreInfoURL) {
  //DBG("%s()\n", __FUNCTION__);
  return VFW_E_NOT_FOUND;
}

// Gets the rating.
GET(Rating) {
  //DBG("%s()\n", __FUNCTION__);
  return VFW_E_NOT_FOUND;
}


// Gets a URL for the watermark.
GET(WatermarkURL) {
  //DBG("%s()\n", __FUNCTION__);
  return VFW_E_NOT_FOUND;
}
