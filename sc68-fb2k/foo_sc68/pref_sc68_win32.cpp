#include "stdafx.h"
#include "resource.h"
#include <helpers/atl-misc.h>
#include <helpers/DarkMode.h>
#include <string>
#include "GUID.h"
#include "global_config.h"
#include "sc68/file68_opt.h"

class Csc68Preferences : public CDialogImpl<Csc68Preferences>, public preferences_page_instance {
public:
	preferences_page_callback::ptr m_callback;
	fb2k::CDarkModeHooks m_dark;
    int m_set_spr;
    int m_set_deftime;
    int m_set_asid;
    int m_set_ym_model;
    int m_set_ym_filter;
    int m_set_ym_volmodel;
    int m_set_amiga_blend;
    bool m_set_amiga_filter;
public:
	Csc68Preferences(preferences_page_callback::ptr callback) : m_callback(callback) {};
	enum {IDD=IDD_SC68_PREFERENCES};
	t_uint32 get_state() {
		t_uint32 state = preferences_state::dark_mode_supported | preferences_state::resettable;
		if (HasChanged()) state |= preferences_state::changed;
		return state;
	}
    
    void apply() {
        cfg_spr = m_set_spr;
        cfg_deftime = m_set_deftime;
        cfg_asid = m_set_asid;
        cfg_ym_model = m_set_ym_model;
        cfg_ym_filter = m_set_ym_filter;
        cfg_ym_volmodel = m_set_ym_volmodel;
        cfg_amiga_blend = m_set_amiga_blend;
        cfg_amiga_filter = m_set_amiga_filter;

        sc68_cntl(0, SC68_SET_OPT_INT, "sampling-rate", (int)cfg_spr);
        sc68_cntl(0, SC68_SET_OPT_INT, "default-time", (int)cfg_deftime);
        sc68_cntl(0, SC68_SET_ASID, (int)cfg_asid);
        sc68_cntl(0, SC68_SET_OPT_INT, "ym-engine", (int)cfg_ym_model);
        sc68_cntl(0, SC68_SET_OPT_INT, "ym-filter", (int)cfg_ym_filter);
        sc68_cntl(0, SC68_SET_OPT_INT, "ym-volmodel", (int)cfg_ym_volmodel);
        sc68_cntl(0, SC68_SET_OPT_INT, "amiga-blend", (int)cfg_amiga_blend);
        sc68_cntl(0, SC68_SET_OPT_INT, "amiga-filter", (bool)cfg_amiga_filter);
        OnChanged();
    }
    void reset() {
        m_set_spr = default_spr;
        m_set_deftime = default_time;
        m_set_asid = default_asid;
        m_set_ym_model = default_ym_model;
        m_set_ym_filter = default_ym_filter;
        m_set_ym_volmodel = default_ym_volmodel;
        m_set_amiga_blend = default_amiga_blend;
        m_set_amiga_filter = default_amiga_filter;
        wchar_t cust_sample[32], deftime[32];
        swprintf_s(cust_sample, L"%d", 48000);
        swprintf_s(deftime, L"%d", (int)default_time);
        SetDlgItemTextW(IDC_SC68_USRSPR, cust_sample);
        SetDlgItemTextW(IDC_SC68_DEFTIME, deftime);
        CComboBox asid_combo(GetDlgItem(IDC_SC68_ASID));
        asid_combo.SetCurSel((int)cfg_asid);
        CComboBox ym_model_combo(GetDlgItem(IDC_SC68_ENGINE));
        ym_model_combo.SetCurSel((int)cfg_ym_model);
        CComboBox ym_filter_combo(GetDlgItem(IDC_SC68_FILTER));
        ym_filter_combo.SetCurSel((int)cfg_ym_filter);
        CComboBox ym_volmodel_combo(GetDlgItem(IDC_SC68_VOLMODEL));
        ym_volmodel_combo.SetCurSel((int)cfg_ym_volmodel);
        SendDlgItemMessage(IDC_SC68_SPRSPIN, UDM_SETPOS32, 0, (int)default_spr);
        SendDlgItemMessage(IDC_SC68_TIMESPIN, UDM_SETPOS32, 0, (int)default_time);
        SendDlgItemMessage(IDC_SC68_BLEND, TBM_SETPOS, TRUE, static_cast<LPARAM>((int)default_amiga_blend));
        CheckDlgButton(IDC_SC68_AGAFILTER, BST_UNCHECKED);
        OnChanged();
    }
    void OnChanged() {
        m_callback->on_state_changed();
    }
    bool HasChanged() {
        console::printf("m_set_spr=%d", m_set_spr);
        return m_set_spr != cfg_spr || m_set_deftime != cfg_deftime || m_set_asid != cfg_asid
            || m_set_ym_model != cfg_ym_model || m_set_ym_filter != cfg_ym_filter || m_set_ym_volmodel != cfg_ym_volmodel
            || m_set_amiga_blend != cfg_amiga_blend || m_set_amiga_filter != cfg_amiga_filter;
    }
    static void construct_combo_from_opt(char* opt_key, CComboBox& combo) {
        option68_t* opt = option68_get(opt_key, opt68_ALWAYS);
        for (unsigned i = 0; i < opt->sets; i++) {
            if (!strcmp68(opt_key, "ym-engine") && !strcmp68(((char**)opt->set)[i], "dump")) continue;
            combo.AddString(pfc::stringcvt::string_wide_from_ansi(((char**)opt->set)[i]).get_ptr());
        }
    }
    BEGIN_MSG_MAP_EX(Csc68Preferences)
        MSG_WM_INITDIALOG(OnInitDialog)
        COMMAND_HANDLER_EX(IDC_SC68_USRSPR,EN_CHANGE,OnSampleChange)
        COMMAND_HANDLER_EX(IDC_SC68_DEFTIME,EN_CHANGE,OnDeftimeChange)
        COMMAND_HANDLER_EX(IDC_SC68_ASID,CBN_SELCHANGE,OnASIDChange)
        COMMAND_HANDLER_EX(IDC_SC68_ENGINE,CBN_SELCHANGE,OnYMModelChange)
        COMMAND_HANDLER_EX(IDC_SC68_FILTER,CBN_SELCHANGE,OnYMFilterChange)
        COMMAND_HANDLER_EX(IDC_SC68_VOLMODEL,CBN_SELCHANGE,OnYMVolmodelChange)
        MESSAGE_HANDLER(WM_HSCROLL,OnBlendChange)
        COMMAND_HANDLER_EX(IDC_SC68_AGAFILTER,BN_CLICKED,IsFiltered)

    END_MSG_MAP()
    BOOL OnInitDialog(CWindow, LPARAM) {
        m_dark.AddDialogWithControls(*this);
        m_set_spr = cfg_spr;
        m_set_deftime = cfg_deftime;
        m_set_asid = cfg_asid;
        m_set_ym_model = cfg_ym_model;
        m_set_ym_filter = cfg_ym_filter;
        m_set_ym_volmodel = cfg_ym_volmodel;
        m_set_amiga_blend = cfg_amiga_blend;
        m_set_amiga_filter = cfg_amiga_filter;
        wchar_t cust_sample[32], deftime[32];
        swprintf_s(cust_sample, L"%d", (int)cfg_spr);
        swprintf_s(deftime, L"%d", (int)cfg_deftime);
        SetDlgItemTextW(IDC_SC68_USRSPR, cust_sample);
        SetDlgItemTextW(IDC_SC68_DEFTIME, deftime);
        SendDlgItemMessage(IDC_SC68_SPRSPIN, UDM_SETBUDDY, reinterpret_cast<WPARAM>(GetDlgItem(IDC_SC68_USRSPR).m_hWnd), 0);
        SendDlgItemMessage(IDC_SC68_TIMESPIN, UDM_SETBUDDY, reinterpret_cast<WPARAM>(GetDlgItem(IDC_SC68_DEFTIME).m_hWnd), 0);
        SendDlgItemMessage(IDC_SC68_SPRSPIN, UDM_SETRANGE32, 4000, 96000);
        SendDlgItemMessage(IDC_SC68_TIMESPIN, UDM_SETRANGE32, 0, 86399);
        UDACCEL SprStep = { 0,1 }, TimeStep = { 0,1 };
        SendDlgItemMessage(IDC_SC68_SPRSPIN, UDM_SETACCEL, 1, reinterpret_cast<LPARAM>(&SprStep));
        SendDlgItemMessage(IDC_SC68_TIMESPIN, UDM_SETACCEL, 1, reinterpret_cast<LPARAM>(&TimeStep));
        SendDlgItemMessage(IDC_SC68_SPRSPIN, UDM_SETPOS32, 0, (int)cfg_spr);
        SendDlgItemMessage(IDC_SC68_TIMESPIN, UDM_SETPOS32, 0, (int)cfg_deftime);
        CComboBox asid_combo(GetDlgItem(IDC_SC68_ASID));
        asid_combo.AddString(L"Off");
        asid_combo.AddString(L"On");
        asid_combo.AddString(L"Force");
        asid_combo.SetCurSel((int)cfg_asid);
        CComboBox ym_model_combo(GetDlgItem(IDC_SC68_ENGINE));
        CComboBox ym_filter_combo(GetDlgItem(IDC_SC68_FILTER));
        CComboBox ym_volmodel_combo(GetDlgItem(IDC_SC68_VOLMODEL));
        construct_combo_from_opt("ym-engine", ym_model_combo);
        ym_model_combo.SetCurSel((int)cfg_ym_model);
        construct_combo_from_opt("ym-filter", ym_filter_combo);
        ym_filter_combo.SetCurSel((int)cfg_ym_filter);
        construct_combo_from_opt("ym-volmodel", ym_volmodel_combo);
        ym_volmodel_combo.SetCurSel((int)cfg_ym_volmodel);
        SendDlgItemMessage(IDC_SC68_BLEND, TBM_SETRANGE, TRUE, MAKELPARAM(0, 255));
        SendDlgItemMessage(IDC_SC68_BLEND, TBM_SETPOS, TRUE, static_cast<LPARAM>((int)cfg_amiga_blend));
        CheckDlgButton(IDC_SC68_AGAFILTER, (bool)cfg_amiga_filter ? BST_CHECKED : BST_UNCHECKED);
        return FALSE;
    }
    void OnSampleChange(UINT, int, CWindow) {
        CString sample_rate;
        GetDlgItemTextW(IDC_SC68_USRSPR, sample_rate);
        try {
            int sample_rate_i = std::stoi(std::wstring(sample_rate.GetString()));
            if (sample_rate_i < 4000 || sample_rate_i > 96000) return;
            m_set_spr = sample_rate_i;
        }
        catch (...) { return; }
        OnChanged();
    }
    void OnDeftimeChange(UINT, int, CWindow) {
        CString default_time;
        GetDlgItemTextW(IDC_SC68_DEFTIME, default_time);
        try {
            int default_time_i = std::stoi(std::wstring(default_time.GetString()));
            if (default_time_i < 0 || default_time_i > 86399) return;
            m_set_deftime = default_time_i;
        }
        catch (...) { return; }
        OnChanged();
    }

    void OnASIDChange(UINT, int ctl, CWindow) {
        CComboBox combo(GetDlgItem(ctl));
        m_set_asid = combo.GetCurSel();
        OnChanged();
    }

    void OnYMModelChange(UINT, int ctl, CWindow) {
        CComboBox combo(GetDlgItem(ctl));
        m_set_ym_model = combo.GetCurSel();
        OnChanged();
    }

    void OnYMFilterChange(UINT, int ctl, CWindow) {
        CComboBox combo(GetDlgItem(ctl));
        m_set_ym_filter = combo.GetCurSel();
        OnChanged();
    }

    void OnYMVolmodelChange(UINT, int ctl, CWindow) {
        CComboBox combo(GetDlgItem(ctl));
        m_set_ym_volmodel = combo.GetCurSel();
        OnChanged();
    }

    LRESULT OnBlendChange(UINT, WPARAM, LPARAM lparam, BOOL& handled) {
        if (reinterpret_cast<HWND>(lparam) != GetDlgItem(IDC_SC68_BLEND)) {
            handled = FALSE;
            return 0;
        }
        const int pos = static_cast<int>(SendDlgItemMessage(IDC_SC68_BLEND, TBM_GETPOS));
        m_set_amiga_blend = pos;
        OnChanged();
        return 0;
    }

    void IsFiltered(UINT, int, CWindow) {
        m_set_amiga_filter = IsDlgButtonChecked(IDC_SC68_AGAFILTER);
        OnChanged();
    }

};

class preferences_page_sc68_input_impl : public preferences_page_impl<Csc68Preferences> {
public:
    const char* get_name() { return "SC68 Input"; }
    GUID get_guid() { return guid_sc68_pref_page; }
    GUID get_parent_guid() { return preferences_page::guid_input; }
};

static preferences_page_factory_t<preferences_page_sc68_input_impl> g_preferences_page_sc68_input_impl_factory;
