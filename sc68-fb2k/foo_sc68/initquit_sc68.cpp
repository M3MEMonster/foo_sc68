/*
 * @file    initquit_sc68.c
 * @brief   sc68 foobar200 - implement startup/shutdown callback
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
#include "global_config.h"

// sc68 message function: everything debug or trace will appears in the DEBUG log
// anything else is printed in foobar2000 console accordingly.
static void message_cb(const int bit, sc68_t * sc68, const char *fmt, va_list list)
{
  //static char tmp[256];
  pfc::string8_fastalloc temp;

  //vsnprintf(tmp,sizeof(tmp),fmt,list);
  //tmp[sizeof(tmp)-1] = 0;
  //temp = tmp;

  // I know uPrintfV() is deprecated don't tell me about it.
# pragma warning(suppress : 4995) 
  uPrintfV(temp,fmt,list);
  
  switch (bit) {
  case msg68_CRITICAL: case msg68_ERROR:
    console::error(temp); break;
  case msg68_WARNING:
    console::warning(temp); break;
  case msg68_INFO: case msg68_NOTICE:
    console::info(temp); break;
  default:
      if (bit < msg68_DEBUG)
          console::print(temp);
      else
#ifdef _WIN32
          OutputDebugStringA(temp.toString());
#else
          console::print(temp);
#endif
    break;
  }
}

// Implements initquit callback.
namespace {
    // On init: initialize sc68 library.
    void on_init() {
        static char* argv[] = { "fb2k" };
        sc68_init_t init;
        memset(&init, 0, sizeof(init));
        init.argc = sizeof(argv) / sizeof(*argv);
        init.argv = argv;
        init.debug_clr_mask = 0;
        init.debug_set_mask = 0;
        init.msg_handler = (sc68_msg_t)message_cb;
        #ifdef NDEBUG
            init.debug_clr_mask = -1;
        #elif defined(_DEBUG)
            init.debug_set_mask = (1 << msg68_TRACE) - 1;
        #endif
        
        if (!sc68_init(&init)) {
            sc68_cntl(0, SC68_SET_OPT_INT, "sampling-rate", (int)cfg_spr);
            sc68_cntl(0, SC68_SET_OPT_INT, "default-time", (int)cfg_deftime);
            sc68_cntl(0, SC68_SET_ASID, (int)cfg_asid);
            sc68_cntl(0, SC68_SET_OPT_INT, "ym-engine", (int)cfg_ym_model);
            sc68_cntl(0, SC68_SET_OPT_INT, "ym-filter", (int)cfg_ym_filter);
            sc68_cntl(0, SC68_SET_OPT_INT, "ym-volmodel", (int)cfg_ym_volmodel);
            sc68_cntl(0, SC68_SET_OPT_INT, "amiga-blend", (int)cfg_amiga_blend);
            sc68_cntl(0, SC68_SET_OPT_INT, "amiga-filter", (int)cfg_amiga_filter);
            
        }
        msg68_notice("SC68 component: %s - %s [%s]",
            __DATE__, __TIME__,
        #ifdef _DEBUG
            "Debug"
        #else
            "Release"
        #endif
        );
        
    }

    // On quit: shutdown sc68 library
    void on_quit() {
        sc68_shutdown();
    }

    FB2K_ON_INIT_STAGE(on_init, init_stages::after_config_read);
    FB2K_RUN_ON_QUIT(on_quit);
}
