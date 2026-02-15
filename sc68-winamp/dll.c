/*
 * @file    in_sc68.c
 * @brief   sc68-ng plugin for winamp 5.5 - other exported functions
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (c) 1998-2015 Benjamin Gerard
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

/* generated config header */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "wasc68.h"

/* windows */
#include <windows.h>

/* winamp 2 */
#include "winamp/in2.h"

EXTERN In_Module g_mod;

EXPORT
In_Module *winampGetInModule2()
{
  return &g_mod;
}

/**
 * Called on file info request.
 *
 * @retval  0  Use plugin file info dialog (if it exists)
 * @retval  1  Use winamp unified file info dialog
 */
EXPORT
int winampUseUnifiedFileInfoDlg(const char * fn)
{
  DBG("winampUseUnifiedFileInfoDlg -> %d\n", g_useufi);
  return !g_mod.InfoBox || g_useufi > 0;
}

/**
 * Called before uninstalling the plugin DLL.
 *
 * @retval IN_PLUGIN_UNINSTALL_NOW     Plugin can be uninstalled
 * @retval IN_PLUGIN_UNINSTALL_REBOOT  Winamp needs to restart to uninstall
 */
EXPORT
int winampUninstallPlugin(HINSTANCE hdll, HWND parent, int param)
{
  DBG("winampUninstallPlugin -> uninstall-now\n");
  return IN_PLUGIN_UNINSTALL_NOW;
}
