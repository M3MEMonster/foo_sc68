/*
 * @file    fb2k_sc68.c
 * @brief   sc68 foobar200 - component declaration
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (C) 2013-2014 Benjamin Gerard
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

#include "stdafx.h"

static pfc::string_formatter g_get_component_about()
{
  pfc::string_formatter about;
  about <<
    "Atari-ST and Amiga music player"
    "\n"
#ifdef DEBUG
    "\n" " !!! DEBUG VERSION !!!"
    "\n"
#endif
    "\n" "http://sc68.atari.org"
    "\n"
    "\n" << sc68_versionstr() <<
    "\n" << file68_versionstr() <<
    "\n"
    "\n" "Copyright (C) 2001-2015 Benjamin Gerard.";
  return about;
}

// Declare our component
DECLARE_COMPONENT_VERSION(
  "sc68 for foobar2000",
  "0.7.0",
  g_get_component_about()
);

DECLARE_FILE_TYPE("SC68 files","*.SC68;*.SNDH;*.SND");

// Prevent component renaming
VALIDATE_COMPONENT_FILENAME("foo_sc68.dll");
