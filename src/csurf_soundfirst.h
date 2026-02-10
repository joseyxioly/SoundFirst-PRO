/*
** SoundFirst PRO V1.0
** Copyright (c) 2026 José Pérez
** License: GPL v3 (See LICENSE file)
*/

#ifndef CSURF_SOUNDFIRST_H
#define CSURF_SOUNDFIRST_H

#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <map>
#include <string>

// WDL types (Required by REAPER headers)
#ifndef WDL_INT64
#ifdef _MSC_VER
typedef __int64 WDL_INT64;
typedef unsigned __int64 WDL_UINT64;
#else
typedef long long WDL_INT64;
typedef unsigned long long WDL_UINT64;
#endif
#endif

// Forward declarations
class ReaProject;
typedef class ReaProject* Reaproject;

// REAPER API requirements
#define REAPERAPI_WANT_ShowConsoleMsg
#define REAPERAPI_WANT_CreateMIDIInput
#define REAPERAPI_WANT_CreateMIDIOutput
#define REAPERAPI_WANT_GetTrack
#define REAPERAPI_WANT_GetMasterTrack
#define REAPERAPI_WANT_GetSelectedTrack
#define REAPERAPI_WANT_GetMediaTrackInfo_Value
#define REAPERAPI_WANT_SetMediaTrackInfo_Value
#define REAPERAPI_WANT_GetSetMediaTrackInfo
#define REAPERAPI_WANT_Track_GetPeakInfo
#define REAPERAPI_WANT_CSurf_TrackFromID
#define REAPERAPI_WANT_Main_OnCommand
#define REAPERAPI_WANT_NamedCommandLookup
#define REAPERAPI_WANT_TrackFX_GetCount
#define REAPERAPI_WANT_TrackFX_GetNumParams
#define REAPERAPI_WANT_TrackFX_GetParam
#define REAPERAPI_WANT_TrackFX_SetParam
#define REAPERAPI_WANT_TrackFX_GetFXName
#define REAPERAPI_WANT_TrackFX_GetFormattedParamValue
#define REAPERAPI_WANT_TrackFX_GetParamName
#define REAPERAPI_WANT_TrackFX_GetNamedConfigParm
#define REAPERAPI_WANT_TrackFX_GetOpen
#define REAPERAPI_WANT_TrackFX_SetOpen
#define REAPERAPI_WANT_MIDIEditor_GetActive
#define REAPERAPI_WANT_MIDIEditor_OnCommand
#define REAPERAPI_WANT_GetResourcePath

#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"

// Helper Macros
#ifndef MKVOL2DB
#include <cmath>
#define MKVOL2DB(x) ((x) <= 0.0000000298023223876953125 ? -150.0 : (20.0 * log10(x)))
#endif

class CSurf_SoundFirst : public IReaperControlSurface {
public:
    CSurf_SoundFirst(int inDev, int outDev, int outDev2);
    virtual ~CSurf_SoundFirst();

    virtual const char* GetTypeString() { return "SOUNDFIRST_A61"; }
    virtual const char* GetDescString() { return "SoundFirst PRO (Native)"; }
    virtual const char* GetConfigString();
    virtual int GetConfig(void* cfg, int cfg_sz) { return 0; }
    virtual void CloseNoReset();
    virtual int Extended(int call, void *parm1, void *parm2, void *parm3);

    virtual void OnMIDIEvent(MIDI_event_t* evt);
    virtual void SendMidiCC(int cc, int val);
    virtual void SetSurfaceVolume(MediaTrack* trackid, double volume);
    virtual void SetSurfacePan(MediaTrack* trackid, double pan);
    virtual void SetSurfaceMute(MediaTrack* trackid, bool mute);
    virtual void SetSurfaceSolo(MediaTrack* trackid, bool solo);
    virtual void SetSurfaceRecArm(MediaTrack* trackid, bool recarm);
    virtual void SetPlayState(bool play, bool pause, bool rec);
    virtual void SetRepeatState(bool rep);
    virtual void Run();

    enum ControlMode { MODE_MIXER = 0, MODE_FX, MODE_MIDI, MODE_EDIT, MODE_AUDIO };
    enum SelectionMode { MODE_NORMAL, MODE_CHOOSING };

private:
    int m_midi_in_dev, m_midi_out_dev;

    midi_Output* m_midi_out;
    bool m_handshake_sent;
    int m_current_bank;
    ControlMode m_current_mode;
    SelectionMode m_selection_mode;
    int m_temp_mode_idx;
    int m_fx_page;
    int m_selected_fx_index;
    double m_knob_sensitivity, m_knob_sensitivity_shift;
    MediaTrack* m_last_vol_track;
    bool m_shift_pressed;
    double m_gr_peak_hold;
    WDL_UINT64 m_last_gr_peak_time;
    int m_loop_button_state;
    bool m_startup_announced;
    int m_grid_index;

    // Accessibility Configuration
    bool m_announce_buttons;
    bool m_announce_knobs;
    bool m_announce_encoder;
    bool m_announce_actions;
    bool m_touch_auto_solo; // Solo track when knob is touched

    // Touch Auto-Solo State
    bool m_knob_touch_state[8];      // Current touch state for each knob
    int m_previous_solo_state[8];    // Solo state before touch (for restoration)

    // Dynamic Mode System
    std::vector<std::string> m_available_modes;
    int m_current_mode_idx;
    bool m_browser_mode_selection; // True when BROWSER pressed, waiting for mode selection

    // FX Page System
    int m_current_fx_page;          // Current page for FX mode
    std::string m_current_fx_plugin; // Name of currently focused plugin

    // HID / NHL
    std::thread m_hid_thread;
    std::atomic<bool> m_hid_running;
    HANDLE m_hid_handle;
    struct HidEvent { unsigned char data[30]; };
    std::queue<HidEvent> m_hid_queue;
    std::mutex m_hid_mutex;
    unsigned char m_last_hid_report[30];
    int m_last_knob_values[8];

    // NHL Actions
    std::map<std::string, std::string> m_nhl_actions;
    std::map<int, std::map<std::string, std::string>> m_mode_actions;
    WDL_UINT64 m_last_button_time;

    // Hot-Reload
    std::string m_config_path;
    WDL_UINT64 m_last_config_time;

    // Internal Methods
    void HidThreadLoop();
    void ProcessHidQueue();
    void HandleHidReport(unsigned char* data);
    void HandleEncoderRotation(int delta);
    void HandleKnobRotation(int knob_idx, int delta);
    void HandleSpecificKnobTouch(int knob_idx, bool touch_on, bool touch_off);
    
    // V3 Architecture: Modular Handlers
    void HandleKnob_Mixer(int idx, int d);
    void HandleKnob_MIDI(int idx, int d);
    void HandleKnob_Audio(int idx, int d);
    void HandleFxTouch(int knob_idx, bool touched);
    void HandleKnob_FX(int idx, int d);
    
    void HandleButton_Transport(int byte1, int byte2);
    void HandleButton_Track(int byte2, int byte3);
    void RunNhlAction(const char* key);
    void HandleReportGR(const std::string& cmd);
    void ReportTrackPeak();
    void CheckConfigHotReload();
    void LoadMappingConfig();
    void SwitchMode(ControlMode mode);
    void UpdateBankFromSelectedTrack();
    void DumpCurrentFX();
    void ReportFXMetrics(MediaTrack* track, int fx_idx);
    void HandleFxParamChange(const std::string& cmd, double delta);
    void HandleFxParamSet(const std::string& cmd);
    void LogDebug(const char* text);
    void SpeakText(const char* text);

    void SendMidi(unsigned char status, unsigned char d1, unsigned char d2);
    void SendSysex(unsigned char* data, int length);

    // Mode Dispatchers
    void OnMidiCC_Mixer(int cc, int val);
    void OnMidiCC_FX(int cc, int val);
    void OnMidiCC_MIDI(int cc, int val);
    void OnMidiCC_Edit(int cc, int val);

    // Display & Screensaver
    void UpdateDisplay();
    void SendLCDMessage(unsigned char cmd, const char* text);
    void SendLCDValue(unsigned char cmd, int val); // For VuMeter
    void ResetIdleTimer();

    bool m_screensaver_active;
    double m_last_input_time;
    bool m_gr_meter_mode; // true = GR, false = Peak

    // SysEx Constants
    // Header: F0 00 21 09 00 00 44 43 01 00
    const unsigned char SYSEX_HEADER[10] = { 0xF0, 0x00, 0x21, 0x09, 0x00, 0x00, 0x44, 0x43, 0x01, 0x00 };

};

#endif
