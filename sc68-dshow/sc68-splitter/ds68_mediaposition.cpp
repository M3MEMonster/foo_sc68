/*
 * @file    ds68_mediaposition.cpp
 * @brief   sc68 for directshow - media position
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

HRESULT Sc68Splitter::CanSeekBackward(LONG *pCanSeek)
{
  CheckPointer(pCanSeek,E_POINTER);
  *pCanSeek = OAFALSE;
  return S_OK;
}

HRESULT Sc68Splitter::CanSeekForward(LONG *pCanSeek)
{
  CheckPointer(pCanSeek,E_POINTER);
  *pCanSeek = OAFALSE;
  return S_OK;
}

HRESULT Sc68Splitter::get_CurrentPosition(REFTIME *pllTime)
{
  CheckPointer(pllTime,E_POINTER);
  if (!m_track)
    return E_NOTIMPL;
  else
    return E_NOTIMPL;
}

HRESULT Sc68Splitter::get_Duration(REFTIME *plength)
{
  CheckPointer(plength,E_POINTER);
  if (!m_disk)
    return E_NOTIMPL;
  if (!m_track)
    *plength = (m_info.dsk.time_ms+999) / 1000;
  else
    *plength = (m_info.trk.time_ms+999) / 1000;
  return S_OK;
}

HRESULT Sc68Splitter::get_PrerollTime(REFTIME *pllTime)
{
  return E_NOTIMPL;
}

HRESULT Sc68Splitter::get_Rate(double *pdRate)
{
  return E_NOTIMPL;
}

HRESULT Sc68Splitter::get_StopTime(REFTIME *pllTime)
{
  return E_NOTIMPL;
}

HRESULT Sc68Splitter::put_CurrentPosition(REFTIME llTime)
{
  return E_NOTIMPL;
}

HRESULT Sc68Splitter::put_PrerollTime(REFTIME llTime)
{
  return E_NOTIMPL;
}

HRESULT Sc68Splitter::put_Rate(double dRate)
{
  return E_NOTIMPL;
}

HRESULT Sc68Splitter::put_StopTime(REFTIME llTime)
{
  return E_NOTIMPL;
}
