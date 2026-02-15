/*
 * @file    in_sc68.c
 * @brief   sc68-ng plugin for winamp 5.5 - main
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

/* winamp sc68 declarations */
#include "wasc68.h"

/* libc */
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

/* sc68 */
#include <sc68/sc68.h>
#include <sc68/file68.h>
#include <sc68/file68_features.h>
#include <sc68/file68_str.h>
#include <sc68/file68_msg.h>
#include <sc68/file68_opt.h>

/* windows */
#include <windows.h>

/* winamp 2 */
#include "winamp/in2.h"
#undef   _MSC_VER                       /* fix intptr_t redefinition */
#define  _MSC_VER 2000
#include "winamp/wa_ipc.h"
#include "winamp/ipc_pe.h"

#ifdef __cplusplus
/* winamp 3 */
#include "api/syscb/api_syscb.h"
#include "api/syscb/callbacks/syscb.h"
#include "api/service/waservicefactory.h"
#include "api/service/services.h"
#include "api/service/api_service.h"
#include "api/memmgr/api_memmgr.h"
#include "../Agave/Language/api_language.h"
#define WITH_API_SERVICE
static api_service * g_service;
#endif

/* Fake command line for sc68_init() */
static char appname[] = "winamp";
static char *argv[] = { appname };

/*****************************************************************************
 * Plugin private data.
 ****************************************************************************/

int g_usehook = -1, g_useufi = -1;

#ifdef USE_LOCK
static HANDLE        g_lock;       /* mutex handle           */
#endif
static char          g_magic[8] = "wasc68!";

static WNDPROC       g_mwhkproc;   /* main window chain proc */
static HANDLE        g_thdl;       /* thread handle          */
static DWORD         g_tid;        /* thread id              */
static char        * g_uri;        /* allocated URI          */
static sc68_t      * g_sc68;       /* sc68 emulator instance */
static int           g_code;       /* sc68 process code      */
static int           g_spr;        /* sampling rate in hz    */
static int           g_maxlatency; /* max latency in ms      */
static int           g_trackpos;   /* track position in ms   */
static int           g_allin1;     /* play all tracks as one */
static int           g_track;      /* current playing track  */
static int           g_tracks;     /* number of tracks       */

static volatile LONG g_playing;    /* true while playing     */
static volatile LONG g_stopreq;    /* stop requested         */
static volatile LONG g_paused;     /* pause status           */
static volatile LONG g_settrack;   /* request change track   */

static BYTE          g_spl[576*8]; /* Sample buffer          */

/*****************************************************************************
 * Declaration
 ****************************************************************************/

/* The decode thread */
static DWORD WINAPI playloop(LPVOID b);
static void init();
static void quit();
static void config(HWND);
static void about(HWND);
static  int infobox(const char *, HWND);
static  int isourfile(const char *);
static void pause();
static void unpause();
static  int ispaused();
static  int getlength();
static  int getoutputtime();
static void setoutputtime(int);
static void setvolume(int);
static void setpan(int);
static  int play(const char *);
static void stop();
static void getfileinfo(const in_char *, in_char *, int *);
static void seteq(int, char *, int);

EXTERN int fileinfo_dialog(HINSTANCE hinst, HWND hwnd, const char * uri);
EXTERN int config_dialog(HINSTANCE hinst, HWND hwnd);

/*****************************************************************************
 * Debug
 ****************************************************************************/
int wasc68_cat = msg68_NEVER;

/*****************************************************************************
 * LOCKS
 ****************************************************************************/
#ifdef USE_LOCK

static inline int lock(void)
{ return WaitForSingleObject(g_lock, INFINITE) == WAIT_OBJECT_0; }

static inline int lock_noblock(void)
{ return WaitForSingleObject(g_lock, 0) == WAIT_OBJECT_0; }

static inline void unlock(void)
{ ReleaseMutex(g_lock); }

#else

static inline int lock(void) { return 1; }
static inline int lock_noblock(void) { return 1; }
static inline void unlock(void) { }

#endif

static inline LONG atomic_set(LONG volatile * ptr, LONG v)
{ return InterlockedExchange(ptr,v); }

static inline LONG atomic_get(LONG volatile * ptr)
{ return *ptr; }

sc68_t * sc68_lock(void) {
  return lock() ? g_sc68 : 0;
}

void sc68_unlock(sc68_t * sc68) {
  if (sc68 == g_sc68 && g_sc68)
    unlock();
}

/* static */
/*****************************************************************************
 * THE INPUT MODULE
 ****************************************************************************/
In_Module g_mod =
{
  IN_VER,               /* Input plugin version as defined in in2.h */
  (char*)
  "sc68 (Atari ST & Amiga music) v" PACKAGE_VERSION, /* Description */
  0,                          /* hMainWindow (filled in by winamp)  */
  0,                          /* hDllInstance (filled in by winamp) */
  (char*)
  "sc68\0" "sc68 file (*.sc68)\0"
  "snd\0" "sndh file (*.snd)\0" "sndh\0" "sndh file (*.sndh)\0",
  0,                                  /* is_seekable */
  1,                                  /* uses output plug-in system */

  config,
  about,
  init,
  quit,
  getfileinfo,
  infobox,
  isourfile,
  play,
  pause,
  unpause,
  ispaused,
  stop,

  getlength,
  getoutputtime,
  setoutputtime,

  setvolume,
  setpan,

  0,0,0,0,0,0,0,0,0,     /* visualization calls filled in by winamp */
  0,0,                   /* dsp calls filled in by winamp */
  seteq,                 /* set equalizer */
  NULL,                  /* setinfo call filled in by winamp */
   0                      /* out_mod filled in by winamp */
};

/*****************************************************************************
 * Message Hook
 ****************************************************************************/

/* Process winamp main window messages */
static LRESULT mwproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  const int isplaying = atomic_get(&g_playing);
  int step = 0;                         /* sub song step */

  switch (Msg) {

  case WM_SYSCOMMAND:
    DBG("WM_SYSCOMMAND #%d w=%d l=%d\n",
        (int)LOWORD(wParam), (int)wParam, (int)lParam);

  case WM_COMMAND:
    if (isplaying)
      switch (LOWORD(wParam)) {
        /* Hook prev/next track messages
         *
         *   When a playing multi-song sc68 file prev/next track button
         *   will change the subsong instead of adjusting the playlist
         *   entry. However pressing shift key brings back original
         *   winamp befavior.
         */
      case 40061:                       /* shift + prev track */
        wParam = MAKEWPARAM(40044,HIWORD(wParam));
        break;
      case 40044:                       /* prev track */
        step = -1;
        break;
      case 40060:                       /* shift + next track */
        wParam = MAKEWPARAM(40048, HIWORD(wParam));
        break;
      case 40048:                       /* next track */
        step = 1;
        break;
      default:
        DBG("WM_COMMAND #%d w=%d l=%d\n",
            (int)LOWORD(wParam), (int)wParam, (int)lParam);
      }
    break;
  }

  if (step) {
    int newtrack = g_track + step;
    if (g_allin1 && newtrack > 0 && newtrack <= g_tracks) {
      DBG("request new track -- %02d\n", newtrack);
      atomic_set(&g_settrack, newtrack);
      return TRUE;
    }
  }

  return g_mwhkproc
    ? CallWindowProc(g_mwhkproc, hWnd, Msg, wParam, lParam)
    : DefWindowProc(hWnd, Msg, wParam, lParam)
    ;
}

static
LRESULT CALLBACK myproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  if (hWnd == g_mod.hMainWindow)
    return mwproc(hWnd, Msg, wParam, lParam);
  else {
    assert(!"unexpected window handler in my hook");
    return DefWindowProc(hWnd, Msg, wParam, lParam);
  }
}

static void mw_hook(void)
{
  if (g_usehook) {
    if (!g_mwhkproc) {
      const HWND hwnd = g_mod.hMainWindow;
      char name[512];
      if (GetClassName(hwnd, name, sizeof(name)) <= 0)
        strcpy(name,"<no-name>");
      g_mwhkproc = (WNDPROC)
        SetWindowLong(hwnd, GWL_WNDPROC, (LONG)myproc);
      DBG("hook winamp main window #%08x \"%s\" -- %s (%08x)\n",
          (unsigned) hwnd, name,
          !g_mwhkproc?"failed":"success", (unsigned)g_mwhkproc);
    }
  } else
    DBG("hook disabled\n");
}

static void mw_unhook(void)
{
  if (!g_mwhkproc)
    DBG("winamp main window not hooked\n");
  else {
    DBG("unhook winamp main window\n");
    SetWindowLong(g_mod.hMainWindow, GWL_WNDPROC, (LONG)g_mwhkproc);
    g_mwhkproc = 0;
  }
}

static
/*****************************************************************************
 * CONFIG DIALOG
 ****************************************************************************/
void config(HWND hwnd)
{
  config_dialog(DLGHINST, hwnd);
}

static
/*****************************************************************************
 * ABOUT DIALOG
 ****************************************************************************/
void about(HWND hwnd)
{
  char temp[512];
  snprintf(temp,sizeof(temp),
           "sc68 for winamp\n"
           "Atari ST and Amiga music player\n"
           "using %s and %s"
#ifdef DEBUG
           "\n" " !!! DEBUG Build !!! "
#endif
#ifndef NDEBUG
           "\n" "buid on " __DATE__
#endif
           "\n(c) 1998-2015 Benjamin Gerard",
           sc68_versionstr(),file68_versionstr());

  MessageBox(hwnd,
             temp,
             "About sc68 for winamp",
             MB_OK);
}

static
/*****************************************************************************
 * INFO DIALOG
 ****************************************************************************/
int infobox(const char * uri, HWND hwnd)
{
  fileinfo_dialog(DLGHINST, hwnd, uri);
  return INFOBOX_UNCHANGED;
}

static
/*****************************************************************************
 * FILE DETECTION
 ****************************************************************************/
int isourfile(const char * uri)
{
  if (uri && *uri) {
    if (!strncmp68(uri,"sc68:",5))
      return 1;
    else {
      const char * ext = strrchr(uri,'.');
      if (ext &&
          (0
           || !strcmp68(ext,".sc68")
#ifdef FILE68_Z
           || !strcmp68(ext,".sc68.gz")
#endif
           || !strcmp68(ext,".snd")
           || !strcmp68(ext,".sndh")
            ))
        return 1;
    }
  }
  return 0;
}

/*****************************************************************************
 * PAUSE
 ****************************************************************************/
static void pause() {
  atomic_set(&g_paused,1);
  g_mod.outMod->Pause(1);
}

static void unpause() {
  atomic_set(&g_paused,0);
  g_mod.outMod->Pause(0);
}

static int ispaused() {
  return atomic_get(&g_paused);
}

static
/*****************************************************************************
 * GET LENGTH (MS)
 ****************************************************************************/
int getlength()
{
  int ms = 0;
  if (lock()) {
    const int fct = g_allin1 ? SC68_GET_DSKLEN : SC68_GET_LEN;
    int res = sc68_cntl(g_sc68, fct);
    if (ms != -1)
      ms = res;
    unlock();
  }
  return ms;
}

static
/*****************************************************************************
 * GET CURRENT POSITION (MS)
 ****************************************************************************/
int getoutputtime()
{
  int ms = 0;
  if (lock()) {
    ms = g_trackpos + g_mod.outMod->GetOutputTime();
    unlock();
  }
  return ms;
}

static
/*****************************************************************************
 * SET CURRENT POSITION (MS)
 ****************************************************************************/
void setoutputtime(int ms)
{
/* Not supported ATM.
 * It has to signal the play thread it has to seek.
 */
}

static
/*****************************************************************************
 * SET VOLUME
 ****************************************************************************/
void setvolume(int volume)
{
  g_mod.outMod->SetVolume(volume);
}

static
/*****************************************************************************
 * SET PAN
 ****************************************************************************/
void setpan(int pan)
{
  g_mod.outMod->SetPan(pan);
}

static
/*****************************************************************************
 * SET EQUALIZER : Do nothing to ignore
 ****************************************************************************/
void seteq(int on, char data[10], int preamp) {}


static void clean_close(void)
{
  if (g_thdl) {
    TerminateThread(g_thdl,1);
    CloseHandle(g_thdl);
    g_thdl = 0;
    DBG("%s\n","thread cleaned");
  }
  atomic_set(&g_playing,0);
  mw_unhook();
  if (g_sc68) {
    sc68_destroy(g_sc68);
    g_sc68 = 0;
  }
  if (g_uri) {
    free(g_uri);
    g_uri = 0;
  }
  /* Close output system. */
  g_mod.outMod->Close();
  /* Deinitialize visualization. */
  g_mod.SAVSADeInit();
}

static
/*****************************************************************************
 * STOP
 ****************************************************************************/
void stop()
{
  if (lock()) {
    atomic_set(&g_stopreq,1);
    if (g_thdl) {
      switch (WaitForSingleObject(g_thdl,10000)) {
      case WAIT_OBJECT_0:
        CloseHandle(g_thdl);
        g_thdl = 0;
        break;
      default:
        DBG("thread did not exit normally\n");
      }
    }
    clean_close();
    unlock();
  }
}

static
int track_from_uri(const char ** ptruri)
{
  int track = 0;
  return track;
}

static
/*****************************************************************************
 * PLAY
 *
 * @retval  0 on success
 * @reval  -1 on file not found
 * @retval !0 stopping winamp error
 ****************************************************************************/
int play(const char * uri)
{
  int err = 1;
  int settrack = 0;

  if (!uri || !*uri)
    return -1;

  if (!lock_noblock())
    goto cantlock;

  /* Safety net */
  if (g_sc68 || g_thdl || g_uri)
    goto inused;

  /* cleanup */
  g_mwhkproc   = 0;
  g_maxlatency = 0;
  g_trackpos   = 0;
  g_track      = 0;
  g_stopreq    = 0;
  g_paused     = 0;
  g_settrack   = 0;

  /* Create sc68 emulator instance */
  if (g_sc68 = sc68_create(0), !g_sc68)
    goto exit;

  settrack = track_from_uri(&uri);
  if (settrack) {
    DBG("got specific track -- %d\n", settrack);
  }

  /* Duplicate URI an */
  if (g_uri = strdup(uri), !g_uri)
    goto exit;

  /* Load */
  if (sc68_load_uri(g_sc68, g_uri)) {
    err = -1;                           /* File not found or such */
    goto exit;
  }

  /* Get sampling rate */
  g_spr = sc68_cntl(g_sc68, SC68_GET_SPR);
  if (g_spr <= 0)
    goto exit;

  /* Get track count */
  g_tracks =  sc68_cntl(g_sc68, SC68_GET_TRACKS);
  if (g_tracks <= 0)
    goto exit;
  DBG("tracks=%d\n",g_tracks);

  /* Only mode supported ATM */
  g_allin1 = !settrack;
  DBG("all-in-1: %d\n", g_allin1);

  /* Get disk and track info */
  if (sc68_play(g_sc68, settrack ? settrack : 1, SC68_DEF_LOOP) < 0)
    goto exit;

  /* Run music init code. Ensure everything is okay to really play */
  g_code = sc68_process(g_sc68, 0, 0);
  if (g_code == SC68_ERROR)
    goto exit;

  /* Get current track */
  g_track = sc68_cntl(g_sc68, SC68_GET_TRACK);

  /* Init output module */
  g_maxlatency = g_mod.outMod->Open(g_spr, 2, 16, 0, 0);
  if (g_maxlatency < 0)
    goto exit;

  /* set default volume */
  g_mod.outMod->SetVolume(-666);

  /* Init info and visualization stuff */
  g_mod.SetInfo(0, g_spr/1000, 2, 1);
  g_mod.SAVSAInit(g_maxlatency, g_spr);
  g_mod.VSASetInfo(g_spr, 2);


  /* Init play thread */
  g_thdl = (HANDLE)
    CreateThread(NULL,                  /* Default Security Attributs */
                 0,                     /* Default stack size         */
                 (LPTHREAD_START_ROUTINE)playloop, /* Thread function */
                 (LPVOID) g_magic,      /* Thread Cookie              */
                 0,                     /* Thread status              */
                 &g_tid                 /* Thread Id                  */
      );

  if (err = !g_thdl, !err) {
    atomic_set(&g_playing,1);
    mw_hook();
  }
exit:
  if (err)
    clean_close();

inused:
  unlock();

cantlock:
  return err;
}

static
const char * get_tag(const sc68_cinfo_t * const cinfo, const char * const key)
{
  int i;
  for (i=0; i<cinfo->tags; ++i)
    if (!strcmp68(cinfo->tag[i].key, key))
      return cinfo->tag[i].val;
  return 0;
}

static void xfinfo(char *title, int *msptr, sc68_t *sc68, sc68_disk_t disk)
{
  const int max = GETFILEINFO_TITLE_LENGTH;
  sc68_music_info_t tmpmi, * const mi = &tmpmi;

  if (sc68_music_info(sc68, mi,
                      sc68 ? SC68_CUR_TRACK : SC68_DEF_TRACK, disk))
    return;

  if (title) {
    const char * artist = get_tag(&mi->dsk, "aka");
    if (!artist)
      artist = mi->artist;

    if (mi->tracks == 1 /* || !strcmp68(mi->title, mi->album) */)
      snprintf(title, max, "%s - %s",artist, mi->album);
    else
      snprintf(title, max, "%s - %s [%d tracks]",
               artist, mi->album, mi->tracks);
  }
  if (msptr)
    *msptr = mi->dsk.time_ms;
}


static
/*****************************************************************************
 * GET FILE INFO
 ****************************************************************************/
/*
 * this is an odd function. It is used to get the title and/or length
 * of a track.  If filename is either NULL or of length 0, it means
 * you should return the info of lastfn. Otherwise, return the
 * information for the file in filename.  if title is NULL, no title
 * is copied into it.  if msptr is NULL, no length is copied
 * into it.
 ****************************************************************************/
void getfileinfo(const in_char * uri, in_char * title, int * msptr)
{
  if (title)
    *title = 0;
  if (msptr)
    *msptr = 0;

  if (!uri || !*uri) {
    /* current disk */
    if (lock()) {
      if (g_sc68)
        xfinfo(title, msptr, g_sc68, 0);
      unlock();
    }
  } else {
    /* some other disk */
    sc68_disk_t disk;
    if (disk = wasc68_cache_get(uri), disk) {
      xfinfo(title, msptr, 0, disk);
      wasc68_cache_release(disk, 0);
    }
  }
}

static
/*****************************************************************************
 * LOOP
 ****************************************************************************/
DWORD WINAPI playloop(LPVOID cookie)
{
  for (;;) {
    int settrack, n = 0, canwrite;

    if (atomic_get(&g_stopreq)) {
      DBG("stop request detected\n");
      break;
    }
    settrack = atomic_set(&g_settrack,0);
    canwrite = g_mod.outMod->CanWrite();

    if (settrack) {
      DBG("change track has been requested -- %02d\n", settrack);
      n = 0;
      sc68_play(g_sc68, settrack, SC68_DEF_LOOP);
      g_code = sc68_process(g_sc68, 0, 0);
      DBG("code=%x\n",g_code);
    } else if (canwrite >= (576 << (2+!!g_mod.dsp_isactive()))) {
      n = 576;
      g_code = sc68_process(g_sc68, g_spl, &n);
    } else {
      Sleep(20);                        /* wait a while */
      continue;
    }

    /* Exit on error or legit end */
    if (g_code & SC68_END) {
      DBG("SC68_END detected -- %x\n", g_code);
      break;
    }
    /* Change track detected */
    if (g_code & SC68_CHANGE) {
      DBG("SC68_CHANGE detected -- %x\n", g_code);
      if (!g_allin1) {
        DBG("Not all in 1, exit\n");
        g_code |= SC68_END;
        break;
      }
      g_track = sc68_cntl(g_sc68,SC68_GET_TRACK);
      DBG("change track -- %02d\n", g_track);
    }
    if (settrack) {
      g_trackpos = sc68_cntl(g_sc68, SC68_GET_ORG);
      DBG("flushing audio track pos: -- %d \n", g_trackpos);
      g_mod.outMod->Flush(0);
    }

    /* if (g_code & SC68_CHANGE) { */
    /*   PostMessage(g_mod.hMainWindow,WM_WA_IPC,0,IPC_UPDTITLE); */
    /* } */

    /* Send audio data to output mod with optionnal DSP processing if
     * it is requested. */
    if (n > 0) {
      int l;
      int vispos = g_trackpos + g_mod.outMod->GetOutputTime();

      /* Give the samples to the vis subsystems */
      g_mod.SAAddPCMData (g_spl, 2, 16, vispos);
      g_mod.VSAAddPCMData(g_spl, 2, 16, vispos);

      /* If we have a DSP plug-in, then call it on our samples */
      l = (
        g_mod.dsp_isactive()
        ? g_mod.dsp_dosamples((short *)g_spl, n, 16, 2, g_spr)
        : n ) << 2;

      /* Write the pcm data to the output system */
      g_mod.outMod->Write((char*)g_spl, l);
    }
  }
  atomic_set(&g_playing,0);

  /* Wait buffered output to be processed */
  while (!atomic_get(&g_stopreq)) {
    g_mod.outMod->CanWrite();           /* needed by some out mod */
    if (!g_mod.outMod->IsPlaying()) {
      /* Done playing: tell Winamp and quit the thread */
      PostMessage(g_mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
      break;
    } else {
      Sleep(15);              // give a little CPU time back to the system.
    }
  }

  DBG("exit with code -- %x\n", g_code);
  return 0;
}

static int onchange_ufi(const option68_t * opt, value68_t * val)
{
  g_useufi = !!val->num;
  return 0;
}

static int onchange_hook(const option68_t * opt, value68_t * val)
{
  g_usehook = !!val->num;
  return 0;
}

static
/*****************************************************************************
 * PLUGIN INIT
 ****************************************************************************/
void init()
{
  /* sc68 init */

  static option68_t opts[] = {
    OPT68_BOOL(0,"ufi", "winamp","Unified file info dialog",1,onchange_ufi),
    OPT68_BOOL(0,"hook","winamp","Hook prev/next track",    1,onchange_hook)
  };

  sc68_init_t init68;
#ifndef NDEBUG
  const int debug =
#ifdef DEBUG
    1
#else
    0
#endif
    ;
#endif

  memset(&init68,0,sizeof(init68));
  init68.argv = argv;
  init68.argc = sizeof(argv) / sizeof(*argv);
#ifndef NDEBUG
  wasc68_cat = msg68_cat("winamp", "winamp input plugin", debug);
  init68.debug_set_mask = debug << wasc68_cat;
  init68.debug_clr_mask = 0;
  init68.msg_handler = (sc68_msg_t) msgfct;
#else
  wasc68_cat = msg68_NEVER;
  init68.debug_clr_mask = -1;
#endif
  init68.flags.no_load_config = 1;      /* disable config load */
  sc68_init(&init68);

  /* sc68 winamp init */
  option68_append(opts, sizeof(opts)/sizeof(*opts));
  option68_iset(option68_get("ufi",opt68_ALWAYS),0,opt68_ALWAYS,opt68_CFG);
  option68_iset(option68_get("hook",opt68_ALWAYS),1,opt68_ALWAYS,opt68_CFG);
  sc68_cntl(0,SC68_CONFIG_LOAD);

  /* clear and init private */
#ifdef USE_LOCK
  g_lock = CreateMutex(NULL, FALSE, NULL);
#endif

  /* clear and init cacke */
  wasc68_cache_init();

  /* Hook messages */
  /* mw_hook(); */

#ifdef WITH_API_SERVICE
  /* Get WASABI service */
  if (!g_service) {
    g_service = (api_service *)
      SendMessage(g_mod.hMainWindow,WM_WA_IPC,0,IPC_GET_API_SERVICE);
    if (g_service == (api_service *)1)
      g_service = 0;
  }

  static int fcc[] = {
    WaSvc::NONE,
    WaSvc::UNIQUE,
    WaSvc::OBJECT,
    WaSvc::CONTEXTCMD,
    WaSvc::DEVICE,
    WaSvc::FILEREADER,
    WaSvc::FILESELECTOR,
    WaSvc::STORAGEVOLENUM,
    WaSvc::IMAGEGENERATOR,
    WaSvc::IMAGELOADER,
    WaSvc::IMAGEWRITER,
    WaSvc::ITEMMANAGER,
    WaSvc::PLAYLISTREADER,
    WaSvc::PLAYLISTWRITER,
    WaSvc::MEDIACONVERTER,
    WaSvc::MEDIACORE,
    WaSvc::MEDIARECORDER,
    WaSvc::SCRIPTOBJECT,
    WaSvc::WINDOWCREATE,
    WaSvc::XMLPROVIDER,
    WaSvc::DB,
    WaSvc::SKINFILTER,
    WaSvc::METADATA,
    WaSvc::METATAG,
    WaSvc::EVALUATOR,
    WaSvc::MINIBROWSER,
    WaSvc::TOOLTIPSRENDERER,
    WaSvc::XUIOBJECT,
    WaSvc::STRINGCONVERTER,
    WaSvc::ACTION,
    WaSvc::COREADMIN,
    WaSvc::DROPTARGET,
    WaSvc::OBJECTDIR,
    WaSvc::TEXTFEED,
    WaSvc::ACCESSIBILITY,
    WaSvc::ACCESSIBILITYROLESERVER,
    WaSvc::EXPORTER,
    WaSvc::COLLECTION,
    WaSvc::REDIRECT,
    WaSvc::FONTRENDER,
    WaSvc::SRCCLASSFACTORY,
    WaSvc::SRCEDITOR,
    WaSvc::MP4AUDIODECODER,
    WaSvc::PLAYLISTREADER_WA5,
    WaSvc::PLAYLISTWRITER_WA5,
    WaSvc::PLAYLISTHANDLER,
    WaSvc::TAGPROVIDER,
    WaSvc::NSVFACTORY,
    -1
  };

  if (g_service) {
    int i, n, j;
    waServiceFactory * s;
    DBG("SVC api <%p>\n", g_service);

    for (j=0; fcc[j] != -1; ++j) {
      char cc[5];
      cc[0] = fcc[j]>>24; cc[1] = fcc[j]>>16;
      cc[2] = fcc[j]>> 8; cc[3] = fcc[j]; cc[4] = 0;

      n = g_service->service_getNumServices(fcc[j]);
      DBG("[%s] got %d service(s)\n", cc, n);
      for (s = g_service->service_enumService(fcc[j], i=0);
           s;
           s = g_service->service_enumService(fcc[j], ++i)) {
        DBG("[%s] #%02d SVC factory  '%s'\n",
            cc, i, s ? s->getServiceName() : "(nil)");
      }
    }

    s = g_service->service_getServiceByGuid(languageApiGUID);
    if (s) {
      DBG("service factory lang %p '%s'\n",
          s, s?s->getServiceName():"(nil)");
    } else {
      DBG("don't have service factory lang\n");
    }

    s = g_service->service_getServiceByGuid(memMgrApiServiceGuid);
    if (s) {
      DBG("service factory memman %p '%s'\n",
          s, s?s->getServiceName():"(nil)");
    } else {
      DBG("don't have service memman lang\n");
    }
    //if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

  }
#endif

  DBG("init completed\n");
}

static
/*****************************************************************************
 * PLUGIN SHUTDOWN
 ****************************************************************************/
void quit()
{
  DBG("\n");
  mw_unhook();

  lock();
  sc68_cntl(0,SC68_CONFIG_SAVE);
  unlock();
#ifdef USE_LOCK
  CloseHandle(g_lock);
  g_lock = 0;
#endif
  wasc68_cache_kill();
  msg68_cat_free(wasc68_cat);
  wasc68_cat = msg68_NEVER;
  sc68_shutdown();
}

static int xinfo(const char *data, char *dest, size_t destlen,
                 sc68_t * sc68, sc68_disk_t disk, int track)
{
  sc68_music_info_t tmpmi, * const mi = &tmpmi;
  const char * value = 0;

  if (!strcasecmp(data, "type")) {
    /* "TYPE" is an important value as it tells Winamp if this is an
     * audio or video format */
    value = "0";
  }
  else if (!strcasecmp(data,"family")) {
    value = "sc68 audio files";
  /* Get the info for the default track only. */
  }
  else if (sc68_music_info(sc68, mi,
                           track>0 ? track : SC68_DEF_TRACK, disk)) {
  }
  else if (!strcasecmp(data,"album")) { /* Album name */
    value = mi->album;
  }
  else if (!strcasecmp(data,"title")) { /* Song title */
    value = !track ? mi->album : mi->title;

    /* if (!track) { */
    /*   if (mi->tracks > 1) { */
    /*     snprintf(dest, destlen, "%s [%02d]",mi->album, mi->tracks); */
    /*     value = dest; */
    /*   } else { */
    /*     value = mi->album; */
    /*   } */
    /* } else if (!strcmp68(mi->album,mi->title)) { */
    /*   snprintf(dest, destlen, "%s #%02", mi->album, mi->trk.track); */
    /*   value = dest; */
    /* } else { */
    /*   value = mi->title; */
    /* } */
  }
  else if (!strcasecmp(data,"artist")) { /* Song artist */
    value = get_tag(&mi->trk,"aka");
    if (!value)
      value = mi->artist;
  }
  else if (!strcasecmp(data,"track")) {
    if (track == mi->trk.track) {
      snprintf(dest, destlen, "%02d", track);
      value = dest;
    }
  }
  /* else if (!strcasecmp(data,"disc")) { */
  /*   snprintf(dest, destlen, "%02d", mi->tracks); */
  /*   value = dest; */
  /* } */
  else if (!strcasecmp(data,"albumartist")) {
    value = get_tag(&mi->dsk,"aka");
    if (!value) value = get_tag(&mi->dsk,"artist");
    if (!value) value = mi->artist;
  }
  else if (!strcasecmp(data,"composer")) {
    value = get_tag(&mi->trk,"original");
    if (!value) value = get_tag(&mi->trk,"composer");
    if (!value) value = get_tag(&mi->dsk,"original");
    if (!value) value = get_tag(&mi->dsk,"composer");
  } else if (!strcasecmp(data,"genre")) {
    value = mi->genre;
  }
  else if (!strcasecmp(data,"length")) {
    /* length in ms */
    /* $$$ Right now disk length but that might change in the future */
    snprintf(dest, destlen, "%u", mi->dsk.time_ms);
    value = dest;
  }
  else if (!strcasecmp(data,"year")) {
    value = get_tag(&mi->dsk,"year");
    if (!value)
      value = get_tag(&mi->trk,"year");
  }
  else if (!strcasecmp(data,"publisher")) {
    value = get_tag(&mi->trk,"converter");
    if (!value) value = get_tag(&mi->dsk,"converter");
    value = get_tag(&mi->trk,"converter");
  }
  else if (!strcasecmp(data,"streamtype")) {
  }
  else if (!strcasecmp(data, "lossless")) {
    value = "1";
  }
  else if (!strcasecmp(data, "comment")) {
    value = get_tag(&mi->trk,"comment");
    if(!value)
    value = get_tag(&mi->dsk,"comment");
  }
  else if (!strcasecmp(data,"")) {
    DBG("unhandled TAG '%s'\n", data);
  }

  if (!value)
    return 0;

  if (value != dest)
    strncpy(dest, value, destlen);
  dest[destlen-1] = 0;
  return 1;
}


/**
 * Provides the extended meta tag support of winamp.
 *
 * @param  uri   URI or file to get info from (0 or empty for playing)
 * @param  data  Tag name
 * @param  dest  Buffer for tag value
 * @param  max   Size of dest buffer
 *
 * @retval 1 tag handled
 * @retval 0 unsupported tag
 */
EXPORT
int winampGetExtendedFileInfo(const char *uri, const char *data,
                              char *dest, size_t max)
{
  sc68_disk_t disk;
  int res = 0;

  if (data && *data && dest && max > 2) {
    if (!uri || !*uri) {
      if (lock()) {
        res = xinfo(data, dest, max, g_sc68, 0, g_track);
        unlock();
      }
    } else {
      int settrack = track_from_uri(&uri);
      if (disk = wasc68_cache_get(uri), disk) {
        res = xinfo(data, dest, max, 0, disk, settrack);
        wasc68_cache_release(disk, 0);
      }
    }
  }
  return res;
}

/**
 * Provides fake support for writing tag to prevent winamp unified
 * file info to complain.
 */
EXPORT
int winampSetExtendedFileInfo(const char *fn, const char *data, char *val)
{
  DBG("\"%s\" [%s] = \"%s\"\n", fn, data, val);
  return 1;
}

/**
 * Provides writing tags to prevent winamp unified file info to
 * complain.
 */
EXPORT
int winampWriteExtendedFileInfo()
{
  DBG("fake success\n");
  return 1;
}

int set_asid(int asid)
{
  int res = -1;
  if (lock()) {
    res = sc68_cntl(g_sc68,SC68_SET_ASID,asid);
    if (g_sc68)
      res = sc68_cntl(0,SC68_SET_ASID,asid);
    unlock();
  }
  DBG("set asid mode -- %d -> %d\n", asid, res);
  return res;
}

#if 0
//Not used anymore config is saved by the dial68 handler
int save_config(void)
{
  int res;
  res = sc68_cntl(0,SC68_CONFIG_SAVE);
  if (!res) {
    if (lock()) {
      if (g_sc68)
        res = sc68_cntl(g_sc68,SC68_CONFIG_LOAD);
      unlock();
    }
  }
  DBG("save config -> %d\n", res);
  return res;
}
#endif