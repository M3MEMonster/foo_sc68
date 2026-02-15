/*
 * @file    pref_sc68.cpp
 * @brief   sc68 foobar2000 - preferences page
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (C) 2013-2015 Benjamin Gerard
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
#include "pref_resource.h"
#include "sc68/file68_opt.h"

#include <commctrl.h>

/* -----------------------------------------------------------------------
   Configuration variable GUIDs
   ----------------------------------------------------------------------- */

// GUID: SC68 preferences page
static const GUID guid_sc68_pref_page =
  { 0xa8b3c4d5, 0xe6f7, 0x4802, { 0x92, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xe2 } };

// GUIDs for persistent cfg_int variables
static const GUID guid_cfg_spr =
  { 0xa8b3c4d5, 0xe6f7, 0x4803, { 0x92, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xe3 } };
static const GUID guid_cfg_deftime =
  { 0xa8b3c4d5, 0xe6f7, 0x4804, { 0x92, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xe4 } };
static const GUID guid_cfg_asid =
  { 0xa8b3c4d5, 0xe6f7, 0x4805, { 0x92, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xe5 } };
static const GUID guid_cfg_engine =
  { 0xa8b3c4d5, 0xe6f7, 0x4806, { 0x92, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xe6 } };
static const GUID guid_cfg_filter =
  { 0xa8b3c4d5, 0xe6f7, 0x4807, { 0x92, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xe7 } };
static const GUID guid_cfg_volmodel =
  { 0xa8b3c4d5, 0xe6f7, 0x4808, { 0x92, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xe8 } };
static const GUID guid_cfg_blend =
  { 0xa8b3c4d5, 0xe6f7, 0x4809, { 0x92, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xe9 } };
static const GUID guid_cfg_amiga_filter =
  { 0xa8b3c4d5, 0xe6f7, 0x480a, { 0x92, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xea } };


/* -----------------------------------------------------------------------
   Default values
   ----------------------------------------------------------------------- */

enum {
  default_sampling_rate = 48000,
  default_time          = 180,
  default_asid          = 0,    /* off     */
  default_engine        = 0,    /* blep    */
  default_filter        = 0,    /* 2-poles */
  default_volmodel      = 0,    /* atari   */
  default_blend         = 128,  /* center  */
  default_amiga_filter  = 0,    /* off     */
};


/* -----------------------------------------------------------------------
   Persistent configuration variables (foobar2000 cfg_var system)
   ----------------------------------------------------------------------- */

static cfg_int cfg_spr         (guid_cfg_spr,          default_sampling_rate);
static cfg_int cfg_deftime     (guid_cfg_deftime,       default_time);
static cfg_int cfg_asid        (guid_cfg_asid,          default_asid);
static cfg_int cfg_engine      (guid_cfg_engine,        default_engine);
static cfg_int cfg_filter      (guid_cfg_filter,        default_filter);
static cfg_int cfg_volmodel    (guid_cfg_volmodel,      default_volmodel);
static cfg_int cfg_blend       (guid_cfg_blend,         default_blend);
static cfg_int cfg_amiga_filter(guid_cfg_amiga_filter,  default_amiga_filter);


/* -----------------------------------------------------------------------
   Sampling rate helpers.
   The combo has "< custom >" at index 0, then the predefined rates.
   ----------------------------------------------------------------------- */

static const int   spr_values[] = { 11025, 22050, 44100, 48000, 96000 };
static const char *spr_labels[] = { "11 kHz", "22 kHz", "44.1 kHz", "48 kHz", "96 kHz" };
static const int   spr_count    = sizeof(spr_values) / sizeof(*spr_values);

/* Return the combo index (1-based for presets, 0 = custom). */
static int spr_to_combo_index(int spr) {
  for (int i = 0; i < spr_count; i++)
    if (spr_values[i] == spr)
      return i + 1;           /* +1 because index 0 is "< custom >" */
  return 0;                   /* custom */
}

/* Minimum / maximum for the custom spin. */
enum { SPR_MIN = 4000, SPR_MAX = 96000 };


/* -----------------------------------------------------------------------
   Apply preferences to sc68 library and global variables.
   Called from on_init (startup) and from the preferences Apply button.
   ----------------------------------------------------------------------- */

void apply_sc68_preferences()
{
  g_sampling_rate = (int) cfg_spr;
  g_ym_asid       = (int) cfg_asid;
  g_ym_engine     = (int) cfg_engine;
  g_ym_filter     = (int) cfg_filter;

  sc68_cntl(0, SC68_SET_OPT_INT, "sampling-rate", (int) cfg_spr);
  sc68_cntl(0, SC68_SET_OPT_INT, "default-time",  (int) cfg_deftime);
  sc68_cntl(0, SC68_SET_ASID,    (int) cfg_asid);
  sc68_cntl(0, SC68_SET_OPT_INT, "ym-engine",     (int) cfg_engine);
  sc68_cntl(0, SC68_SET_OPT_INT, "ym-filter",     (int) cfg_filter);
  sc68_cntl(0, SC68_SET_OPT_INT, "ym-volmodel",   (int) cfg_volmodel);
  sc68_cntl(0, SC68_SET_OPT_INT, "amiga-blend",   (int) cfg_blend);
  sc68_cntl(0, SC68_SET_OPT_INT, "amiga-filter",  (int) cfg_amiga_filter);
}


/* -----------------------------------------------------------------------
   Populate a combo box from an option68 enum option.
   ----------------------------------------------------------------------- */

static void populate_combo_from_opt(HWND hdlg, int idc, const char *optkey)
{
  HWND hcb = GetDlgItem(hdlg, idc);
  if (!hcb) return;

  option68_t *opt = option68_get(optkey, opt68_ALWAYS);
  SendMessageA(hcb, CB_RESETCONTENT, 0, 0);

  if (opt && opt->type == opt68_ENU) {
    const char **names = (const char **) opt->set;
    for (unsigned i = 0; i < opt->sets; i++) {
      SendMessageA(hcb, CB_ADDSTRING, 0,
                   (LPARAM)(names[i] ? names[i] : "<unknown>"));
    }
  }
}


/* -----------------------------------------------------------------------
   Populate the sampling rate combo: "< custom >", then presets.
   ----------------------------------------------------------------------- */

static void populate_spr_combo(HWND hdlg)
{
  HWND hcb = GetDlgItem(hdlg, IDC_SC68_SPR);
  if (!hcb) return;

  SendMessageA(hcb, CB_RESETCONTENT, 0, 0);
  SendMessageA(hcb, CB_ADDSTRING, 0, (LPARAM) "< custom >");
  for (int i = 0; i < spr_count; i++)
    SendMessageA(hcb, CB_ADDSTRING, 0, (LPARAM)spr_labels[i]);
}


/* -----------------------------------------------------------------------
   Populate the aSIDifier combo manually.
   ----------------------------------------------------------------------- */

static void populate_asid_combo(HWND hdlg)
{
  static const char *labels[] = { "Off", "On", "Force" };
  HWND hcb = GetDlgItem(hdlg, IDC_SC68_ASID);
  if (!hcb) return;

  SendMessageA(hcb, CB_RESETCONTENT, 0, 0);
  for (int i = 0; i < 3; i++)
    SendMessageA(hcb, CB_ADDSTRING, 0, (LPARAM)labels[i]);
}


/* -----------------------------------------------------------------------
   Enable / disable the custom sampling-rate edit + spin.
   They are enabled only when "< custom >" (index 0) is selected.
   ----------------------------------------------------------------------- */

static void update_spr_enable(HWND hdlg)
{
  int idx = (int) SendDlgItemMessageA(hdlg, IDC_SC68_SPR,
                                      CB_GETCURSEL, 0, 0);
  BOOL custom = (idx == 0);
  EnableWindow(GetDlgItem(hdlg, IDC_SC68_USRSPR),  custom);
  EnableWindow(GetDlgItem(hdlg, IDC_SC68_SPRSPIN),  custom);
}


/* -----------------------------------------------------------------------
   Preferences page instance (raw Win32, no ATL dependency)
   ----------------------------------------------------------------------- */

class sc68_pref_instance : public preferences_page_instance {
  HWND m_hwnd;
  preferences_page_callback::ptr m_callback;

  /* ---- Dialog initialization ---- */
  void on_init_dialog()
  {
    /* ---- Sampling rate combo + custom spin ---- */
    populate_spr_combo(m_hwnd);
    int spr_idx = spr_to_combo_index((int) cfg_spr);
    SendDlgItemMessageA(m_hwnd, IDC_SC68_SPR, CB_SETCURSEL, spr_idx, 0);

    SendDlgItemMessage(m_hwnd, IDC_SC68_SPRSPIN, UDM_SETRANGE32,
                       SPR_MIN, SPR_MAX);
    SendDlgItemMessage(m_hwnd, IDC_SC68_SPRSPIN, UDM_SETPOS32,
                       0, (int) cfg_spr);
    update_spr_enable(m_hwnd);

    /* ---- Default time (spin: 0..86399 seconds) ---- */
    SendDlgItemMessage(m_hwnd, IDC_SC68_TIMESPIN, UDM_SETRANGE32, 0, 86399);
    SendDlgItemMessage(m_hwnd, IDC_SC68_TIMESPIN, UDM_SETPOS32,
                       0, (int) cfg_deftime);

    /* ---- aSIDifier ---- */
    populate_asid_combo(m_hwnd);
    SendDlgItemMessageA(m_hwnd, IDC_SC68_ASID, CB_SETCURSEL,
                        (int) cfg_asid, 0);

    /* ---- YM engine / filter / volmodel (from option68 enums) ---- */
    populate_combo_from_opt(m_hwnd, IDC_SC68_ENGINE,   "ym-engine");
    populate_combo_from_opt(m_hwnd, IDC_SC68_FILTER,   "ym-filter");
    populate_combo_from_opt(m_hwnd, IDC_SC68_VOLMODEL, "ym-volmodel");

    SendDlgItemMessageA(m_hwnd, IDC_SC68_ENGINE,   CB_SETCURSEL,
                        (int) cfg_engine, 0);
    SendDlgItemMessageA(m_hwnd, IDC_SC68_FILTER,   CB_SETCURSEL,
                        (int) cfg_filter, 0);
    SendDlgItemMessageA(m_hwnd, IDC_SC68_VOLMODEL, CB_SETCURSEL,
                        (int) cfg_volmodel, 0);

    /* ---- L/R Blend slider (0..255, ticks at quarters) ---- */
    SendDlgItemMessage(m_hwnd, IDC_SC68_BLEND, TBM_SETRANGE,
                       FALSE, MAKELPARAM(0, 255));
    SendDlgItemMessage(m_hwnd, IDC_SC68_BLEND, TBM_CLEARTICS, FALSE, 0);
    SendDlgItemMessage(m_hwnd, IDC_SC68_BLEND, TBM_SETTIC, 0, 64);
    SendDlgItemMessage(m_hwnd, IDC_SC68_BLEND, TBM_SETTIC, 0, 128);
    SendDlgItemMessage(m_hwnd, IDC_SC68_BLEND, TBM_SETTIC, 0, 192);
    SendDlgItemMessage(m_hwnd, IDC_SC68_BLEND, TBM_SETPOS,
                       TRUE, (int) cfg_blend);

    /* ---- Amiga filter checkbox ---- */
    SendDlgItemMessage(m_hwnd, IDC_SC68_AGAFILTER, BM_SETCHECK,
                       cfg_amiga_filter ? BST_CHECKED : BST_UNCHECKED, 0);
  }


  /* ---- Read current values from the dialog controls ---- */

  int get_ui_spr() const {
    int idx = (int) SendDlgItemMessageA(m_hwnd, IDC_SC68_SPR,
                                        CB_GETCURSEL, 0, 0);
    if (idx > 0 && idx <= spr_count)
      return spr_values[idx - 1];
    /* Custom: read from the edit / spin */
    BOOL ok = FALSE;
    int v = (int) GetDlgItemInt(m_hwnd, IDC_SC68_USRSPR, &ok, FALSE);
    if (ok && v >= SPR_MIN && v <= SPR_MAX)
      return v;
    return default_sampling_rate;
  }

  int get_ui_deftime() const {
    BOOL ok = FALSE;
    int v = (int) GetDlgItemInt(m_hwnd, IDC_SC68_DEFTIME, &ok, FALSE);
    return ok ? v : default_time;
  }

  int get_ui_combo(int idc, int def) const {
    int v = (int) SendDlgItemMessageA(m_hwnd, idc, CB_GETCURSEL, 0, 0);
    return (v >= 0) ? v : def;
  }

  int get_ui_blend() const {
    return (int) SendDlgItemMessage(m_hwnd, IDC_SC68_BLEND,
                                    TBM_GETPOS, 0, 0);
  }

  int get_ui_amiga_filter() const {
    return (SendDlgItemMessage(m_hwnd, IDC_SC68_AGAFILTER,
                               BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
  }


  /* ---- Check whether any setting has changed ---- */

  bool has_changed() const {
    if (get_ui_spr()                                    != (int) cfg_spr)          return true;
    if (get_ui_deftime()                                != (int) cfg_deftime)      return true;
    if (get_ui_combo(IDC_SC68_ASID,     default_asid)   != (int) cfg_asid)         return true;
    if (get_ui_combo(IDC_SC68_ENGINE,   default_engine)  != (int) cfg_engine)       return true;
    if (get_ui_combo(IDC_SC68_FILTER,   default_filter)  != (int) cfg_filter)       return true;
    if (get_ui_combo(IDC_SC68_VOLMODEL, default_volmodel)!= (int) cfg_volmodel)     return true;
    if (get_ui_blend()                                  != (int) cfg_blend)        return true;
    if (get_ui_amiga_filter()                           != (int) cfg_amiga_filter) return true;
    return false;
  }


  /* ---- Notify host that something changed ---- */

  void on_changed() {
    m_callback->on_state_changed();
  }


  /* ---- Win32 dialog procedure ---- */

  static INT_PTR CALLBACK DlgProc(HWND hdlg, UINT msg,
                                   WPARAM wparam, LPARAM lparam)
  {
    sc68_pref_instance *self;

    if (msg == WM_INITDIALOG) {
      self = (sc68_pref_instance *) lparam;
      SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR) self);
      self->m_hwnd = hdlg;
      self->on_init_dialog();
      return TRUE;
    }

    self = (sc68_pref_instance *) GetWindowLongPtr(hdlg, GWLP_USERDATA);
    if (!self) return FALSE;

    switch (msg) {
    case WM_COMMAND:
      switch (LOWORD(wparam)) {
      case IDC_SC68_SPR:
        if (HIWORD(wparam) == CBN_SELCHANGE) {
          update_spr_enable(hdlg);
          self->on_changed();
        }
        return TRUE;

      case IDC_SC68_ASID:
      case IDC_SC68_ENGINE:
      case IDC_SC68_FILTER:
      case IDC_SC68_VOLMODEL:
        if (HIWORD(wparam) == CBN_SELCHANGE)
          self->on_changed();
        return TRUE;

      case IDC_SC68_DEFTIME:
      case IDC_SC68_USRSPR:
        if (HIWORD(wparam) == EN_CHANGE)
          self->on_changed();
        return TRUE;

      case IDC_SC68_AGAFILTER:
        self->on_changed();
        return TRUE;
      }
      break;

    case WM_HSCROLL:
      if ((HWND) lparam == GetDlgItem(hdlg, IDC_SC68_BLEND)) {
        self->on_changed();
        return TRUE;
      }
      break;
    }
    return FALSE;
  }


public:

  /* ---- Constructor: create the child dialog ---- */

  sc68_pref_instance(HWND parent, preferences_page_callback::ptr callback)
    : m_hwnd(NULL), m_callback(callback)
  {
    m_hwnd = CreateDialogParamA(
      (HINSTANCE) core_api::get_my_instance(),
      MAKEINTRESOURCEA(IDD_SC68_PREFERENCES),
      parent,
      DlgProc,
      (LPARAM) this);
  }

  /* ---- Destructor ---- */
  ~sc68_pref_instance() {
    if (m_hwnd && IsWindow(m_hwnd))
      DestroyWindow(m_hwnd);
  }


  /* -- preferences_page_instance interface -- */

  t_uint32 get_state() {
    t_uint32 state = preferences_state::resettable;
    if (has_changed())
      state |= preferences_state::changed;
    return state;
  }

  HWND get_wnd() { return m_hwnd; }

  void apply() {
    cfg_spr          = get_ui_spr();
    cfg_deftime      = get_ui_deftime();
    cfg_asid         = get_ui_combo(IDC_SC68_ASID,     default_asid);
    cfg_engine       = get_ui_combo(IDC_SC68_ENGINE,    default_engine);
    cfg_filter       = get_ui_combo(IDC_SC68_FILTER,    default_filter);
    cfg_volmodel     = get_ui_combo(IDC_SC68_VOLMODEL,  default_volmodel);
    cfg_blend        = get_ui_blend();
    cfg_amiga_filter = get_ui_amiga_filter();

    /* Push to sc68 library and global variables */
    apply_sc68_preferences();

    /* Tell host the apply button can be disabled now */
    on_changed();
  }

  void reset() {
    /* Reset all controls to factory defaults */
    int spr_idx = spr_to_combo_index(default_sampling_rate);
    SendDlgItemMessageA(m_hwnd, IDC_SC68_SPR, CB_SETCURSEL, spr_idx, 0);
    SendDlgItemMessage(m_hwnd, IDC_SC68_SPRSPIN, UDM_SETPOS32,
                       0, default_sampling_rate);
    update_spr_enable(m_hwnd);

    SendDlgItemMessage(m_hwnd, IDC_SC68_TIMESPIN, UDM_SETPOS32,
                       0, default_time);
    SendDlgItemMessageA(m_hwnd, IDC_SC68_ASID, CB_SETCURSEL,
                        default_asid, 0);
    SendDlgItemMessageA(m_hwnd, IDC_SC68_ENGINE, CB_SETCURSEL,
                        default_engine, 0);
    SendDlgItemMessageA(m_hwnd, IDC_SC68_FILTER, CB_SETCURSEL,
                        default_filter, 0);
    SendDlgItemMessageA(m_hwnd, IDC_SC68_VOLMODEL, CB_SETCURSEL,
                        default_volmodel, 0);
    SendDlgItemMessage(m_hwnd, IDC_SC68_BLEND, TBM_SETPOS,
                       TRUE, default_blend);
    SendDlgItemMessage(m_hwnd, IDC_SC68_AGAFILTER, BM_SETCHECK,
                       default_amiga_filter ? BST_CHECKED : BST_UNCHECKED, 0);
    on_changed();
  }
};


/* -----------------------------------------------------------------------
   Preferences page factory (registered with foobar2000)
   ----------------------------------------------------------------------- */

class sc68_pref_page : public preferences_page_v3 {
public:
  preferences_page_instance::ptr instantiate(HWND parent,
                                             preferences_page_callback::ptr callback)
  {
    return new service_impl_t<sc68_pref_instance>(parent, callback);
  }

  const char * get_name() { return "SC68"; }

  GUID get_guid() { return guid_sc68_pref_page; }

  // Register under the existing Decoding branch (guid_input)
  GUID get_parent_guid() { return preferences_page::guid_input; }
};

static preferences_page_factory_t<sc68_pref_page> g_sc68_pref_factory;
