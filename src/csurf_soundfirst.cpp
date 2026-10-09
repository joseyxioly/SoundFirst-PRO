/*
** SoundFirst PRO V1.0
** Copyright (c) 2026 José Pérez
** License: GPL v3 (See LICENSE file)
*/

#include "csurf_soundfirst.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cctype>
#include "fx_mappings.h" // V3 Pro FX Mapping Engine

#ifdef _WIN32
#include <windows.h>
#include <ole2.h>
#include <sapi.h>
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

// --- GLOBALS/HELPERS ---

std::map<std::string, std::map<int, int>> g_plugin_mappings;
std::map<std::string, std::map<int, std::string>> g_page_names;

// Global REAPER Plugin Info
extern "C" void RawLog(const char* txt);
extern reaper_plugin_info_t* g_rec;

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void AutoDetectSoundFirstPorts(int& inDev, int& outDev) {
    inDev = -1; outDev = -1;
    int num_inputs = GetNumMIDIInputs();
    for (int i = 0; i < num_inputs; i++) {
        char name[256];
        if (GetMIDIInputName(i, name, sizeof(name))) {
            if (strstr(name, "Komplete") && strstr(name, "DAW")) { inDev = i; break; }
        }
    }
    int num_outputs = GetNumMIDIOutputs();
    for (int i = 0; i < num_outputs; i++) {
        char name[256];
        if (GetMIDIOutputName(i, name, sizeof(name))) {
            if (strstr(name, "Komplete") && strstr(name, "DAW")) { outDev = i; break; }
        }
    }
}

// --- CLASS IMPLEMENTATION ---

CSurf_SoundFirst::CSurf_SoundFirst(int inDev, int outDev, int outDev2) {
    ::CoInitialize(NULL);
    LogDebug("--- CONSTRUCTOR INICIADO (SoundFirst PRO) ---");
    
    m_midi_in_dev = inDev;
    m_midi_out_dev = outDev;
    
    // Config path setup
    const char* rPath = GetResourcePath();
    if (rPath) {
        char iniPath[1024]; sprintf(iniPath, "%s\\UserPlugins\\SoundFirst_PRO.ini", rPath);
        m_config_path = iniPath;
        LogDebug("Detectando archivo INI...");
    }

    LogDebug("Buscando puertos MIDI Komplete DAW...");
    int detIn = -1, detOut = -1;
    AutoDetectSoundFirstPorts(detIn, detOut);
    if (detIn != -1) { m_midi_in_dev = detIn; LogDebug("MIDI In detectado automáticamente."); }
    else LogDebug("MIDI In NO encontrado - Usando puerto por defecto.");

    if (detOut != -1) { m_midi_out_dev = detOut; LogDebug("MIDI Out detectado automáticamente."); }
    else LogDebug("MIDI Out NO encontrado - Usando puerto por defecto.");



    // Session opens after application state has been initialized.
    m_current_bank = 0;
    m_current_mode = MODE_MIXER;
    m_fx_page = 0;
    m_selected_fx_index = 0;
    m_knob_sensitivity = 0.002;
    m_knob_sensitivity_shift = 0.0005;
    m_last_button_time = 0;
    m_shift_pressed = false;
    m_startup_announced = false;
    m_last_vol_track = nullptr;
    m_gr_peak_hold = 0.0;
    m_last_gr_peak_time = 0;
    m_loop_button_state = 0;
    m_grid_index = 4;
    
    // Display V1.0 Init
    m_screensaver_active = false;
    m_last_input_time = 0; // Will be set in Run()
    m_gr_meter_mode = false; // Default to Peak
    
    // Accessibility defaults
    m_announce_buttons = true;
    m_announce_knobs = false;
    m_announce_encoder = true;
    m_announce_actions = false;
    m_touch_auto_solo = false;
    
    // Initialize touch state arrays
    for (int i = 0; i < 8; i++) {
        m_knob_touch_state[i] = false;
        m_previous_solo_state[i] = 0;
    }
    
    // Mode system defaults
    // MIXER y FX tienen botones dedicados (TRACK / PLUGIN).
    // BROWSER rota solo entre MIDI y AUDIO.
    m_available_modes = {"MIDI", "AUDIO", "Instrument"};
    m_current_mode_idx = 0;
    m_browser_mode_selection = false;
    
    // FX page system defaults
    m_current_fx_page = 0;
    m_current_fx_plugin = "";
    
    LoadMappingConfig();
    
    // FORCE MODE RESET (V1.0 Standard)
    // Ignore whatever the INI file said.
    m_available_modes = {"MIDI", "AUDIO", "Instrument"};
    m_current_mode_idx = 0;
    
    memset(m_last_hid_report, 0, sizeof(m_last_hid_report));
    std::fill(m_last_knob_values, m_last_knob_values + 8, -1);
    OpenNiSession();
    m_hid_handle = INVALID_HANDLE_VALUE;
    m_hid_running = true;
    m_hid_thread = std::thread(&CSurf_SoundFirst::HidThreadLoop, this);
    LogDebug("Hilo de escucha HID de baja latencia arrancado.");

    // Hot-reload setup
    if (!m_config_path.empty()) {
        WIN32_FILE_ATTRIBUTE_DATA data;
        if (GetFileAttributesExA(m_config_path.c_str(), GetFileExInfoStandard, &data)) {
            m_last_config_time = ((WDL_UINT64)data.ftLastWriteTime.dwHighDateTime << 32) | data.ftLastWriteTime.dwLowDateTime;
        }
    }
    LogDebug("--- CONSTRUCTOR FINALIZADO CON ÉXITO ---");
}

CSurf_SoundFirst::~CSurf_SoundFirst() {
    ReleaseTouchSolos();
    m_hid_running = false;
    if (m_hid_thread.joinable()) {
        m_hid_thread.join();
    }
    if (m_hid_handle != INVALID_HANDLE_VALUE) CloseHandle(m_hid_handle);
    CloseNiSession();
    ::CoUninitialize();
}

void CSurf_SoundFirst::CloseNoReset() { CloseNiSession(); }

const char* CSurf_SoundFirst::GetConfigString() {
    static char cfg[512]; sprintf(cfg, "%d %d", m_midi_in_dev, m_midi_out_dev);
    return cfg;
}


int CSurf_SoundFirst::Extended(int call, void *parm1, void *parm2, void *parm3) {
    if (call == CSURF_EXT_SETFXCHANGE) RefreshNiFocus(true);
    UpdateBankFromSelectedTrack();
    m_ni_feedback_dirty = true;
    return 0;
}

void CSurf_SoundFirst::OnMIDIEvent(MIDI_event_t* evt) {
    if (!evt || evt->size < 1) return;
    ++m_ni_received;
    soundfirst_ni::Command command;
    const bool decoded=soundfirst_ni::decodeCc(evt->midi_message,evt->size,command);
    if (decoded) ++m_ni_cc_received; else ++m_ni_rejected;
    // Short startup diagnostic window, bounded packets and bytes. This logs
    // the DAW protocol only, not the ordinary keybed music input.
    if (time_precise()<=m_ni_diagnostic_until && m_ni_diagnostic_packets<24) {
        ++m_ni_diagnostic_packets;
        std::ostringstream raw;
        raw << "[NI_MIDI_RX] size=" << evt->size << " decoded=" << decoded << " bytes=";
        char octet[4];
        for (int i=0;i<std::min(evt->size,24);++i) {
            snprintf(octet,sizeof(octet),"%02X",evt->midi_message[i]); raw<<octet<<' ';
        }
        LogDebug(raw.str().c_str());
    }
    if (decoded) HandleNiCommand(command.number,command.value);
}

void CSurf_SoundFirst::Run() {
    CheckConfigHotReload();
    PumpNiInput();
    ProcessHidQueue();
    if (time_precise() - m_ni_last_feedback > 0.1) {
        RefreshNiFocus();
        MaybeRecoverNiHello();
        ReportNiConnect();
        m_ni_last_feedback = time_precise();
    }
    if (m_current_mode == MODE_INSTRUMENT) return;
    
    // --- DISPLAY LOOP ---
    UpdateDisplay(); // Handles Screensaver logic & Text Sync
    
    // --- VU METER ENGINE (0x49) ---
    // Only update if screen is active (save USB bandwidth)
    if (!m_screensaver_active) {
        MediaTrack* tr = GetSelectedTrack(0, 0);
        if (tr) {
            int meterVal = 0;
            if (m_gr_meter_mode) {
                 // TODO: GR Phase 2. For now, show inverted peak or 0
                 // Ideal: Read specific GR param if mapped.
                 meterVal = 0; 
            } else {
                 // PEAK MODE
                 double peakL = Track_GetPeakInfo(tr, 0);
                 double peakR = Track_GetPeakInfo(tr, 1);
                 double maxPeak = (peakL > peakR) ? peakL : peakR;
                 
                 // Logarithmic Scaling for Meter
                 if (maxPeak > 0.0001) {
                     double db = 20.0 * log10(maxPeak);
                     // Map -60dB (0) to +6dB (127)
                     // Scale: 66dB range
                     double norm = (db + 60.0);
                     if (norm < 0) meterVal = 0;
                     else {
                         meterVal = (int)(norm * (127.0 / 66.0));
                         if (meterVal > 127) meterVal = 127;
                     }
                 }
            }
            SendLCDValue(0x49, meterVal);
        } else {
            SendLCDValue(0x49, 0); // No track = 0
        }
    }
}

#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>
#include <hidsdi.h>

// Helper to find the dynamic HID path for A61 (VID 17CC, PID 1620, Interface 3)
// Helper to find the dynamic HID path (VID 17CC, PID 1750/1620)
std::string GetA61HidPath() {
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE) return "";

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    std::string bestMatch = "";
    int ni_count = 0;

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &hidGuid, i, &deviceInterfaceData); i++) {
        DWORD detailSize = 0;
        SetupDiGetDeviceInterfaceDetailA(hDevInfo, &deviceInterfaceData, NULL, 0, &detailSize, NULL);
        if (detailSize == 0) continue;
        
        PSP_DEVICE_INTERFACE_DETAIL_DATA_A pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_A)malloc(detailSize);
        pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

        if (SetupDiGetDeviceInterfaceDetailA(hDevInfo, &deviceInterfaceData, pDetail, detailSize, NULL, NULL)) {
            std::string path = pDetail->DevicePath;
            std::string matchPath = path;
            std::transform(matchPath.begin(), matchPath.end(), matchPath.begin(), ::tolower);
            
            if (matchPath.find("vid_17cc") != std::string::npos) {
                ni_count++;
                
                // Log all NI devices for diagnostics
                char lMsg[512]; sprintf(lMsg, "[HID_SCAN] Found NI Device: %s", path.c_str());
                RawLog(lMsg);
                
                // Prioritize user's actual hardware: PID 1730 (A25), 1740 (A49), 1750 (A61), 1860 (M32), 1620 (Legacy?)
                // Both use the same protocol (MI 02 for DAW control usually)
                if (matchPath.find("pid_1730") != std::string::npos ||
                    matchPath.find("pid_1750") != std::string::npos ||
                    matchPath.find("pid_1860") != std::string::npos || 
                    matchPath.find("pid_1740") != std::string::npos || 
                    matchPath.find("pid_1620") != std::string::npos) {
                    // Check for Interface 2 (DAW) or Interface 1/3 depending on model. 
                    // We look for "mi_02" or just accept the first valid PID match if strict MI check fails.
                    if (matchPath.find("mi_02") != std::string::npos) {
                        bestMatch = path;
                        free(pDetail);
                        // Stop at first good match to avoid ambiguity
                        break; 
                    }
                    // Fallback: If we see the PID but not "mi_02" (maybe mi_01?), keep it as candidate
                    if (bestMatch.empty()) bestMatch = path;
                }
            }
        }
        free(pDetail);
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return bestMatch;
}

void CSurf_SoundFirst::HidThreadLoop() {
    LogDebug("Hilo HID iniciado.");
    int attempts = 0;
    while (m_hid_running) {
        if (m_hid_handle == INVALID_HANDLE_VALUE) {
            std::string path = GetA61HidPath();
            if (!path.empty()) {
                LogDebug((std::string("Ruta HID encontrada: ") + path).c_str());
                m_hid_handle = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
                if (m_hid_handle != INVALID_HANDLE_VALUE) {
                    LogDebug("Conexión HID establecida con éxito.");
                    std::string lowerPath = path;
                    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
                    m_encoder_byte = (lowerPath.find("pid_1860") != std::string::npos) ? 24 : 28;
                    LogDebug(m_encoder_byte == 24 ? "[HID_LAYOUT] M32: encoder at byte 24" : "[HID_LAYOUT] A-series: encoder at byte 28");
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));
                    // REMOVED UNSAFE SpeakText FROM BACKGROUND THREAD
                    // SpeakText("Conectado"); 
                    attempts = 0;
                } else {
                    LogDebug("ERROR: No se pudo abrir el handle HID (CreateFileA falló).");
                }
            } else {
                if (attempts % 10 == 0) LogDebug("Buscando dispositivo HID...");
            }
            
            if (m_hid_handle == INVALID_HANDLE_VALUE) {
                attempts++;
                if (attempts == 3) LogDebug("Searching for Keyboard");
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }
        }
        unsigned char buf[64] = {0}; 
        DWORD read = 0;
        // Request 64 bytes (Maximum Report Size). A-Series sends 30, M32 sends ~38. 
        // ReadFile will return what is available if we request enough.
        OVERLAPPED operation = {};
        operation.hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
        if (!operation.hEvent) {
            CloseHandle(m_hid_handle); m_hid_handle = INVALID_HANDLE_VALUE;
            continue;
        }
        BOOL complete = ReadFile(m_hid_handle, buf, 64, &read, &operation);
        if (!complete && GetLastError() == ERROR_IO_PENDING) {
            while (m_hid_running && WaitForSingleObject(operation.hEvent, 200) == WAIT_TIMEOUT) {}
            if (!m_hid_running) {
                CancelIoEx(m_hid_handle, &operation);
                WaitForSingleObject(operation.hEvent, INFINITE);
            }
            complete = GetOverlappedResult(m_hid_handle, &operation, &read, FALSE);
        }
        CloseHandle(operation.hEvent);
        if (complete && read > 0) {
            std::lock_guard<std::mutex> lock(m_hid_mutex);
            // Ensure we don't overflow internal fixed buffer (64)
            if (read > 64) read = 64; 
            
            HidEvent ev; 
            memset(ev.data, 0, 64);
            memcpy(ev.data, buf, read);
            ev.size = read;
            ev.observed = GetTickCount64() / 1000.0;
            m_hid_queue.push(ev);
        } else {
            CloseHandle(m_hid_handle); m_hid_handle = INVALID_HANDLE_VALUE;
            std::lock_guard<std::mutex> lock(m_hid_mutex);
            m_hid_queue.push(HidEvent{});
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void CSurf_SoundFirst::ProcessHidQueue() {
    std::lock_guard<std::mutex> lock(m_hid_mutex);
    while (!m_hid_queue.empty()) {
        const HidEvent& ev = m_hid_queue.front();
        if (ev.size == 0) { ReleaseTouchSolos(); m_hid_baseline = false; m_track_press.reset(); m_mode_press.reset(); m_undo_pressed_at=-1; }
        if (m_hid_capture_until > 0 && GetTickCount64()/1000.0 <= m_hid_capture_until && ev.size > 0) {
            std::ostringstream report;
            report << "[NI_CAPTURE] packet=" << ++m_hid_capture_packets
                   << " time=" << ev.observed << " size=" << ev.size << " bytes=";
            char octet[4];
            for (DWORD byte=0; byte<ev.size; ++byte) {
                snprintf(octet,sizeof(octet),"%02X",ev.data[byte]);
                report << octet << ' ';
            }
            LogDebug(report.str().c_str());
            if (m_hid_capture_packets >= 3000) {
                m_hid_capture_until = 0; LogDebug("[NI_CAPTURE] packet limit reached");
            }
        } else if (m_hid_capture_until > 0 && GetTickCount64()/1000.0 > m_hid_capture_until) {
            m_hid_capture_until=0; LogDebug("[NI_CAPTURE] 60 second capture finished");
        }
        if (ev.size >= 30) {
            m_hid_observed = ev.observed;
            HandleHidReport(const_cast<unsigned char*>(ev.data));
        }
        m_hid_queue.pop();
    }
}

bool CSurf_SoundFirst::RunNhlAction(const char* key) {
    LogDebug((std::string("Action Hit: ") + key).c_str());
    
    // Announce button press if enabled
    if (m_announce_buttons) SpeakText(key);

    // 1. PRIORIDAD: Mapeos específicos de plugin si estamos en modo FX
    if (m_current_mode == MODE_FX) {
        MediaTrack* track = GetSelectedTrack(NULL, 0);
        if (track && m_selected_fx_index >= 0) {
            char fxName[256]; TrackFX_GetFXName(track, m_selected_fx_index, fxName, 256);
            
            // Checking dynamic mapping registry (User File or Factory)
            FXMapping* map = FXMappingRegistry::GetMapping(fxName);
            if (map) {
                 int btn_id = -1;
                 if (std::string(key) == "LOOP") btn_id = BTN_LOOP;
                 else if (std::string(key) == "METRO") btn_id = BTN_METRO;
                 else if (std::string(key) == "TEMPO") btn_id = BTN_TEMPO;
                 else if (std::string(key) == "IDEAS") btn_id = BTN_IDEAS;
                 else if (std::string(key) == "QUANTIZE") btn_id = BTN_QUANTIZE;
                 else if (std::string(key) == "AUTO") btn_id = BTN_AUTO;
                 else if (std::string(key) == "MUTE") btn_id = BTN_MUTE;
                 else if (std::string(key) == "SOLO") btn_id = BTN_SOLO;
                 // UNDO Removed by user request (Always Hardcoded Undo)

                 if (btn_id != -1 && map->buttons.count(btn_id)) {
                     std::string actionStr = map->buttons[btn_id];
                     
                     // 1. Is it a special action?
                     if (actionStr.find('@') == 0) {
                         // Recursively call self? No, that would loop for buttons.
                         // But for special actions like @REPORT_PEAK or @REPORT_GR, we can call recursively
                         // because the recursive call will hit the "Read from INI/Global" block? 
                         // Wait, RunNhlAction handles @ strings at end.
                         // Let's jump to the end logic by setting 'val' but we are inside a specific block.
                         // Better: Execute logic here or refactor.
                         
                         // Re-use logic:
                         if (actionStr.find("@REPORT_GR") == 0) {
                             // Dynamic GR Report
                             // If actionStr is just "@REPORT_GR", use current FX
                             std::string cmd = actionStr;
                             if (cmd == "@REPORT_GR") cmd = ":"+std::to_string(m_selected_fx_index);
                             HandleReportGR(cmd);
                             return true;
                         }
                         else if (actionStr == "@REPORT_PEAK") {
                             ReportTrackPeak();
                             return true;
                         }
                         // Extend for other actions as needed
                     }
                     
                     // 2. Is it a Parameter ID?
                     try {
                         int p_idx = std::stoi(actionStr);
                         if (p_idx >= 0) {
                             double v = TrackFX_GetParam(track, m_selected_fx_index, p_idx, NULL, NULL);
                             TrackFX_SetParam(track, m_selected_fx_index, p_idx, (v > 0.5) ? 0.0 : 1.0);
                             
                             // Announce change
                             if (m_announce_buttons) {
                                char buf[256];
                                TrackFX_GetFormattedParamValue(track, m_selected_fx_index, p_idx, buf, 256);
                                SpeakText(buf);
                             }
                             return true; // Override successful
                         }
                     } catch (...) {}
                 }
            }
        }
    }

    // 2. Read from INI config (mode-specific or global)
    std::string val = "";
    if (m_shift_pressed) {
        std::string sKey = std::string("SHIFT_") + key;
        if (m_mode_actions[m_current_mode].count(sKey)) val = m_mode_actions[m_current_mode][sKey];
        else if (m_nhl_actions.count(sKey)) val = m_nhl_actions[sKey];
    }
    
    if (val.empty()) {
        if (m_mode_actions[m_current_mode].count(key)) val = m_mode_actions[m_current_mode][key];
        else if (m_nhl_actions.count(key)) val = m_nhl_actions[key];
    }

    // If no config found, return false (not handled)
    if (val.empty()) return false;

    // Announce action if enabled
    if (m_announce_actions) {
        std::string action_msg = "Action " + val;
        SpeakText(action_msg.c_str());
    }

    if (val[0] == '@') {
        // Bank navigation
        if (val == "@BANK_UP") { m_current_bank++; char m[64]; sprintf(m, "Bank %d", m_current_bank+1); SpeakText(m); }
        else if (val == "@BANK_DOWN" && m_current_bank > 0) { m_current_bank--; char m[64]; sprintf(m, "Bank %d", m_current_bank+1); SpeakText(m); }
        
        // FX Page navigation
        else if (val == "@PAGE_UP") { 
            if (m_fx_page > 0) m_fx_page--; 
            char m[64]; sprintf(m, "Page %d", m_fx_page+1); 
            SpeakText(m); 
        }
        else if (val == "@PAGE_DOWN") { 
            m_fx_page++; 
            char m[64]; sprintf(m, "Page %d", m_fx_page+1); 
            SpeakText(m); 
        }
        
        // FX Bypass toggle
        else if (val == "@FX_BYPASS_TOGGLE") {
            MediaTrack* track = GetLastTouchedTrack();
            if (track) {
                int fx_count = TrackFX_GetCount(track);
                if (fx_count > 0) {
                    // Toggle first FX (can be extended to focused FX)
                    bool enabled = TrackFX_GetEnabled(track, 0);
                    TrackFX_SetEnabled(track, 0, !enabled);
                    SpeakText(enabled ? "FX Bypass" : "FX Active");
                }
            }
        }
        
        // FX Parameter Inc/Dec/Set
        else if (val.find("@FX_PARAM_INC:") == 0) {
            HandleFxParamChange(val, 0.01);  // Increment 1%
        }
        else if (val.find("@FX_PARAM_DEC:") == 0) {
            HandleFxParamChange(val, -0.01); // Decrement 1%
        }
        else if (val.find("@FX_PARAM_SET:") == 0) {
            HandleFxParamSet(val);
        }
        
        // Reporting (mapeable per-FX)
        else if (val.find("@REPORT_GR:") == 0) {
            HandleReportGR(val);  // Format: @REPORT_GR:fx_idx
        }
        else if (val == "@REPORT_PEAK") {
            ReportTrackPeak();
        }
        else if (val == "@DUMP_FX") DumpCurrentFX();
        else if (val == "@REPORT_STOP") { /* Stop reporting - handled by touch OFF */ }
        
        return true;
    }

    int cmdId = 0;
    if (val[0] == '_') cmdId = NamedCommandLookup(val.c_str());
    else { try { cmdId = std::stoi(val); } catch (...) {} }
    if (cmdId > 0) {
        Main_OnCommand(cmdId, 0);
        
        // Auto-update bank only after track navigation commands (MIXER mode)
        // 40285=Track Down, 40286=Track Up
        if (cmdId == 40285 || cmdId == 40286) {
            UpdateBankFromSelectedTrack();
        }
        return true;
    }
    return false;
}

// FX Parameter Manipulation Functions
void CSurf_SoundFirst::HandleFxParamChange(const std::string& cmd, double delta) {
    // Parse command: @FX_PARAM_INC:fx_idx:param_idx or @FX_PARAM_DEC:fx_idx:param_idx
    size_t pos1 = cmd.find(':');
    if (pos1 == std::string::npos) return;
    
    size_t pos2 = cmd.find(':', pos1 + 1);
    if (pos2 == std::string::npos) return;
    
    try {
        int fx_idx = std::stoi(cmd.substr(pos1 + 1, pos2 - pos1 - 1));
        int param_idx = std::stoi(cmd.substr(pos2 + 1));
        
        MediaTrack* track = GetLastTouchedTrack();
        if (!track) return;
        
        // Get current value
        double min_val, max_val;
        double current = TrackFX_GetParam(track, fx_idx, param_idx, &min_val, &max_val);
        
        // Calculate new value
        double new_val = current + delta;
        if (new_val < 0.0) new_val = 0.0;
        if (new_val > 1.0) new_val = 1.0;
        
        // Set new value
        TrackFX_SetParam(track, fx_idx, param_idx, new_val);
        
        // Announce if knobs are announced
        if (m_announce_knobs) {
            char buf[256];
            TrackFX_GetFormattedParamValue(track, fx_idx, param_idx, buf, 256);
            SpeakText(buf);
        }
    } catch (...) {
        LogDebug("Error parsing FX param change command");
    }
}

void CSurf_SoundFirst::HandleFxParamSet(const std::string& cmd) {
    // Parse command: @FX_PARAM_SET:fx_idx:param_idx:value
    size_t pos1 = cmd.find(':');
    if (pos1 == std::string::npos) return;
    
    size_t pos2 = cmd.find(':', pos1 + 1);
    if (pos2 == std::string::npos) return;
    
    size_t pos3 = cmd.find(':', pos2 + 1);
    if (pos3 == std::string::npos) return;
    
    try {
        int fx_idx = std::stoi(cmd.substr(pos1 + 1, pos2 - pos1 - 1));
        int param_idx = std::stoi(cmd.substr(pos2 + 1, pos3 - pos2 - 1));
        double value = std::stod(cmd.substr(pos3 + 1));
        
        MediaTrack* track = GetLastTouchedTrack();
        if (!track) return;
        
        // Set parameter value (0.0 to 1.0)
        TrackFX_SetParam(track, fx_idx, param_idx, value);
        
        // Announce if knobs are announced
        if (m_announce_knobs) {
            char buf[256];
            TrackFX_GetFormattedParamValue(track, fx_idx, param_idx, buf, 256);
            SpeakText(buf);
        }
    } catch (...) {
        LogDebug("Error parsing FX param set command");
    }
}

// Gain Reduction Reporting (mapeable per-FX)
void CSurf_SoundFirst::HandleReportGR(const std::string& cmd) {
    // Parse command: @REPORT_GR:fx_idx OR :fx_idx
    int fx_idx = -1;
    size_t pos1 = cmd.find(':');
    if (pos1 != std::string::npos) {
        try { fx_idx = std::stoi(cmd.substr(pos1 + 1)); } catch(...) {}
    } else {
        try { fx_idx = std::stoi(cmd); } catch(...) {}
    }
    
    if (fx_idx < 0) return;

    // Use Selected Track (as we are in FX Mode covering Selected Track)
    MediaTrack* track = GetSelectedTrack(NULL, 0);
    if (!track) { SpeakText("No track selected"); return; }
    
    // Read GainReduction_dB from FX
    char buf[256] = {0};
    // Try "GainReduction_dB" (VST3/CLAP standard)
    bool success = TrackFX_GetNamedConfigParm(track, fx_idx, "GainReduction_dB", buf, 256);
    
    if (success && buf[0] != '\0') {
        try {
            double gr_db = std::stod(buf);
            if (gr_db == 0.0) {
                 SpeakText("No reduction");
            } else {
                 // GR is usually negative? Or positive reduction?
                 // Some plugins report -3.0. A61 usually displays +3.0 reduction.
                 // We'll read absolute value.
                 int gr_int = (int)round(fabs(gr_db));
                 char msg[64]; sprintf(msg, "Reduction %d dB", gr_int);
                 SpeakText(msg);
            }
        } catch (...) {
            SpeakText("GR Value Error");
        }
    } else {
        // Fallback: Check if it's a Waves plugin? They obscure GR.
        SpeakText("Plugin reports no GR");
    }
}

// Track Peak Reporting
void CSurf_SoundFirst::ReportTrackPeak() {
    MediaTrack* track = GetLastTouchedTrack();
    if (!track) return;
    
    // Read peak values using Track_GetPeakInfo (More reliable for surface feedback)
    double peak_l = Track_GetPeakInfo(track, 0);
    double peak_r = Track_GetPeakInfo(track, 1);
    
    // Get the highest peak
    double peak_max = (peak_l > peak_r) ? peak_l : peak_r;
    
    if (peak_max > 0.00000001) {
        // Convert to dB
        double peak_db = 20.0 * log10(peak_max);
        int peak_int = (int)round(peak_db);
        
        char msg[64];
        if (peak_int >= 0) {
            sprintf(msg, "Peak +%d dB", peak_int);
        } else {
            sprintf(msg, "Peak %d dB", peak_int);
        }
        
        SpeakText(msg);
        LogDebug(msg);
    } else {
        SpeakText("No signal");
    }
}

// Auto-update bank when selected track changes (from v1.2 - event-driven, not polling)
void CSurf_SoundFirst::UpdateBankFromSelectedTrack() {
    MediaTrack* track = GetSelectedTrack(NULL, 0);
    if (!track) return;
    
    int trackNumber = (int)GetMediaTrackInfo_Value(track, "IP_TRACKNUMBER");
    int trackIndex = trackNumber - 1;
    if (trackIndex < 0) return; // Master track or error

    int new_bank = trackIndex / 8;
    
    if (new_bank != m_current_bank) {
        m_current_bank = new_bank;
        
        char msg[128];
        sprintf(msg, "Bank %d (tracks %d-%d)", 
                m_current_bank + 1,
                m_current_bank * 8 + 1,
                m_current_bank * 8 + 8);
        
        LogDebug(msg);
        SpeakText(msg);
    }
}

void CSurf_SoundFirst::HandleHidReport(unsigned char* data) {
    ResetIdleTimer(); // Activity detected
    unsigned char* old = m_last_hid_report;
    if (!m_hid_baseline) {
        // The keyboard only sends a report when something changes, so the
        // first report after connecting is usually a button press. Take knob
        // and encoder positions from it, but compare buttons (bytes 1-5)
        // against "nothing held" so that first press is not swallowed.
        AdoptHidBaseline(data);
        memset(m_last_hid_report + 1, 0, 5);
    }
    if (m_current_mode == MODE_INSTRUMENT) {
        if (m_shift_track_exit && soundfirst_ni::shiftTrack(data[3],old[3],data[1])) {
            LogDebug("[NI_EXIT_SHIFT_TRACK] Shift+TRACK observed; exit requested; NI can also switch instance");
            ExitInstrumentMode(); AdoptHidBaseline(data); return;
        }
        if (m_hold_undo_exit) {
            if (soundfirst_ni::risingBit(data[1],old[1],0x08)) {
                m_undo_pressed_at=m_hid_observed;
                LogDebug("[NI_EXIT_UNDO] press observed; hold >=1 second and release");
            }
            if (!(data[1]&0x08) && (old[1]&0x08) && m_undo_pressed_at >= 0) {
                bool held=soundfirst_ni::heldEnough(m_undo_pressed_at,m_hid_observed,1.0);
                m_undo_pressed_at=-1;
                if (held) {
                    LogDebug("[NI_EXIT_UNDO] held and released; exit requested");
                    ExitInstrumentMode(); AdoptHidBaseline(data); return;
                }
                LogDebug("[NI_EXIT_UNDO] release before 1 second; no exit");
            }
        }
        if (data[3] != old[3]) {
            char diagnostic[96];
            snprintf(diagnostic,sizeof(diagnostic),"[NI_HID_MODE] byte3 old=%02X new=%02X (Browser01 Plugin02 Track04)",old[3],data[3]);
            LogDebug(diagnostic);
        }
        // The mode selector is the original Browser bit01, distinct from
        // the Track-to-Mixer bit04. Each gesture has an independent timer.
        // Both presses still reach native NI Browser; they are not consumed.
        if (soundfirst_ni::risingBit(data[3],old[3],0x01)) {
            if (!m_double_mode_exit) {
                LogDebug("[NI_EXIT_MODE_SELECTOR] byte3 mask01 press; double exit disabled");
            } else if (m_mode_press.press(m_hid_observed,m_double_mode_window)) {
                LogDebug("[NI_EXIT_MODE_SELECTOR] byte3 mask01 second press in window; exit requested");
                ExitInstrumentMode(); AdoptHidBaseline(data); return;
            } else {
                LogDebug("[NI_EXIT_MODE_SELECTOR] byte3 mask01 first press or window expired; waiting for second");
            }
        }
        // Legacy double TRACK remains independent, with its original bit04.
        // NI also receives those presses and may change native context.
        if (soundfirst_ni::risingBit(data[3], old[3], 0x04)) {
            if (!m_double_track_exit) {
                LogDebug("[NI_EXIT_TRACK] byte3 mask04 TRACK-to-Mixer press; double exit disabled in INI");
            } else if (m_track_press.press(m_hid_observed, m_double_track_window)) {
                LogDebug("[NI_EXIT_TRACK] byte3 mask04 second press in window; exit requested");
                ExitInstrumentMode();
                AdoptHidBaseline(data);
                return;
            } else {
                LogDebug("[NI_EXIT_TRACK] byte3 mask04 first press or previous window expired; waiting for second");
            }
        }
        AdoptHidBaseline(data);
        return;
    }
    
    // --- VERIFIED A61 (PID 1750) GROUND TRUTH ---
    bool isShift = (data[1] & 0x01); m_shift_pressed = isShift;

    if (m_hid_capture_until>0 && memcmp(data, old, 64) != 0) {
        char hex[128]; char* p = hex;
        for (int i = 0; i < 8; i++) p += sprintf(p, "%02X ", data[i]);
        char fullMsg[256]; sprintf(fullMsg, "[A61_TRUTH] %s", hex);
        RawLog(fullMsg);
    }
    
    // START CONTEXT FIX
    HWND midi_editor = NULL;
    if (m_current_mode == MODE_MIDI) midi_editor = MIDIEditor_GetActive();

    // Byte 1: SYSTEM (0x02) -> SCALE (Save Project)
    // Confirmed: 0x02 is Scale. Ideas is 0x20.
    if ((data[1] & 0x02) && !(old[1] & 0x02)) { 
        Main_OnCommand(40026, 0); // Save project
        SpeakText("Project Saved");
    }
    
    // ARP -> Report Peak
    if ((data[1] & 0x04) && !(old[1] & 0x04)) ReportTrackPeak();
    
    // UNDO / REDO
    if ((data[1] & 0x08) && !(old[1] & 0x08)) {
        if (isShift) Main_OnCommand(40030, 0); // REDO
        else Main_OnCommand(40029, 0);         // UNDO
    }
    
    // IDEAS (0x20) -> GR Toggle (Shift) / GR Report / Auto-Solo
    if ((data[1] & 0x20) && !(old[1] & 0x20)) {
         if (RunNhlAction("IDEAS")) { /* Overridden */ }
         else if (isShift) {
             // Shift: Toggle Meter Mode (Peak <-> GR)
             m_gr_meter_mode = !m_gr_meter_mode;
             if (m_gr_meter_mode) SpeakText("Meter: G R");
             else SpeakText("Meter: Peak");
             UpdateDisplay();
         } else {
             // Contextual Action
             if (m_current_mode == MODE_FX) {
                 // FX Mode: Report Gain Reduction
                 if (m_selected_fx_index >= 0) {
                     char cmd[32]; sprintf(cmd, ":%d", m_selected_fx_index);
                     HandleReportGR(std::string(cmd));
                 } else SpeakText("No FX Selected");
             } else {
                 // Global: Toggle Auto-Solo
                 m_touch_auto_solo = !m_touch_auto_solo;
                 if (m_touch_auto_solo) SpeakText("Auto-Solo On");
                 else SpeakText("Auto-Solo Off");
             }
         }
    }
    
    // QUANTIZE -> Quantize Events (Mixer) / Quantize Events (MIDI) / Reverse (Audio)
    if ((data[1] & 0x10) && !(old[1] & 0x10)) {
        if (RunNhlAction("QUANTIZE")) { /* Overridden */ }
        else if (m_current_mode == MODE_MIDI) {
             if (isShift) { /* Unmapped */ }
             else {
                 // FIX: Send to Editor if available
                 if (midi_editor) MIDIEditor_OnCommand(midi_editor, 40469);
                 else Main_OnCommand(40469, 0); 
             }
        } else if (m_current_mode == MODE_AUDIO) {
             if (isShift) Main_OnCommand(41051, 0); // Reverse Item
             else Main_OnCommand(40326, 0);         // Quantize (Main)
        } else {
             // MIXER MODE: QUANTIZE BUTTON -> REFERENCE SOLO (TRACK 1)
             // Logic: Solo Track 1 exclusively. Second press = UNDO (Restore previous state).
             static bool ref_solo_active = false;
             if (!ref_solo_active) {
                 Undo_BeginBlock2(0);
                 Main_OnCommand(40340, 0); // Unsolo all tracks
                 MediaTrack* t1 = GetTrack(0, 0);
                 if (t1) {
                     SetMediaTrackInfo_Value(t1, "I_SOLO", 1); // Solo Track 1
                     Undo_EndBlock2(0, "Solo Reference", -1);
                     ref_solo_active = true;
                     SpeakText("Reference Solo");
                 } else {
                     SpeakText("Track 1 not found");
                 }
             } else {
                 Main_OnCommand(40029, 0); // Undo (Restores previous solo state)
                 ref_solo_active = false;
                 SpeakText("Restored");
             }
        }
    }
    
    // LOOP -> Loop Toggle (Mixer) / CUT (MIDI/AUDIO)
    if ((data[1] & 0x40) && !(old[1] & 0x40)) { 
        if (RunNhlAction("LOOP")) { /* Overridden */ }
        else if (m_current_mode == MODE_MIDI) {
             // User Request: Force 40012 (Cut)
             if (midi_editor) MIDIEditor_OnCommand(midi_editor, 40012);
             else Main_OnCommand(40012, 0); 
             SpeakText("Cut Events");
        } else if (m_current_mode == MODE_AUDIO) {
            Main_OnCommand(40059, 0); // CUT Items
            SpeakText("Cut Items");
        } else {
            // MIXER MODE: LOOP BUTTON -> TOGGLE REPEAT
            // User requested V1.2 behavior (Simple Toggle)
            Main_OnCommand(1068, 0); // Transport: Toggle loop
            // Feedback state?
            int state = GetToggleCommandState(1068);
            if (state == 1) SpeakText("Loop Active");
            else SpeakText("Loop Inactive");
        }
    }
    
    // METRO -> Metro Toggle (Mixer) / COPY (MIDI/AUDIO)
    if ((data[1] & 0x80) && !(old[1] & 0x80)) {
        if (RunNhlAction("METRO")) { /* Overridden */ }
        else if (m_current_mode == MODE_MIDI) {
             // User Request: Force 40010 (Copy)
             if (midi_editor) MIDIEditor_OnCommand(midi_editor, 40010);
             else Main_OnCommand(40010, 0);
             SpeakText("Copy Events");
        } else if (m_current_mode == MODE_AUDIO) {
            Main_OnCommand(40057, 0); // COPY Items
            SpeakText("Copy Items");
        } else {
            Main_OnCommand(40364, 0); // METRO
        }
    }

    // Byte 2: TRANSPORT
    // TEMPO -> Tap Tempo (Mixer) / PASTE (MIDI/AUDIO)
    if ((data[2] & 0x01) && !(old[2] & 0x01)) {
        if (RunNhlAction("TEMPO")) { /* Overridden */ }
        else if (m_current_mode == MODE_MIDI) {
             // User Request: Force 40011 (Paste)
             if (midi_editor) MIDIEditor_OnCommand(midi_editor, 40011);
             else Main_OnCommand(40011, 0);
             SpeakText("Paste Events");
        } else if (m_current_mode == MODE_AUDIO) {
            Main_OnCommand(42398, 0); // PASTE Items
            SpeakText("Paste Items");
        } else {
            Main_OnCommand(1134, 0); // TEMPO -> Tap Tempo
        }
    }

    if ((data[2] & 0x02) && !(old[2] & 0x02)) { // PLAY
         if (isShift) {
             // Consistency: Always Global Start (40042)
             Main_OnCommand(40042, 0); 
         }
         else Main_OnCommand(40073, 0);         // Play/Pause
    }
    
    // REC: Record (Normal) / Cycle Automation (Shift)
    if ((data[2] & 0x04) && !(old[2] & 0x04)) {
        if (isShift) {
             if (!RunNhlAction("AUTO")) Main_OnCommand(40406, 0); // Cycle Automation Mode
        }
        else Main_OnCommand(1013, 0);          // Transport: Record
    }

    if ((data[2] & 0x08) && !(old[2] & 0x08)) Main_OnCommand(1016, 0); // STOP
    
    // TRACK MANAGEMENT / EDIT ACTIONS
    // PRESET UP (0x10) -> FX Page Up / Arm Auto / Split
    if ((data[2] & 0x10) && !(old[2] & 0x10)) {
        if (m_current_mode == MODE_FX) {
            // FX MODE: NEXT PAGE
            MediaTrack* t = GetSelectedTrack(NULL, 0);
            char fxN[256]; TrackFX_GetFXName(t, m_selected_fx_index, fxN, 256);
            FXMapping* map = FXMappingRegistry::GetMapping(fxN);
            
            if (map && m_fx_page < map->pages.size() - 1) {
                m_fx_page++;
                char m[64]; sprintf(m, "FX Page %d: %s", m_fx_page+1, map->pages[m_fx_page].name.c_str());
                SpeakText(m);
            } else if (!map) {
                 // Auto-Map Unlimited Pages (Groups of 8)
                 m_fx_page++;
                 char m[64]; sprintf(m, "Auto Page %d", m_fx_page+1);
                 SpeakText(m);
            } else {
                 SpeakText("Last Page");
            }
        } 
        else if (m_current_mode == MODE_AUDIO) {
            Main_OnCommand(40012, 0); // Split Items
            SpeakText("Split Item");
        } else if (m_current_mode == MODE_MIDI) {
             HWND midi_editor = MIDIEditor_GetActive();
             if (!midi_editor) { Main_OnCommand(40012, 0); SpeakText("Split Item"); } 
             else Main_OnCommand(40294, 0); 
        } else {
            Main_OnCommand(40294, 0); // PRESET UP -> Arm Auto
        }
    }

    // MUTE (0x40)
    if ((data[2] & 0x40) && !(old[2] & 0x40)) {
        if (RunNhlAction("MUTE")) { /* Overridden */ }
        else {
             if (m_current_mode == MODE_FX) {
                 // FX Mode: Toggle Plugin Bypass
                 MediaTrack* t = GetSelectedTrack(NULL, 0);
                 if (t && m_selected_fx_index >= 0) {
                     bool en = TrackFX_GetEnabled(t, m_selected_fx_index);
                     TrackFX_SetEnabled(t, m_selected_fx_index, !en);
                     SpeakText(en ? "Bypass On" : "Bypass Off"); 
                 }
             } else {
                 // Default Mute Toggle (Track)
                 Main_OnCommand(6, 0); // Track: Toggle Mute for selected tracks
             }
        }
    }

    // SOLO (0x80)
    if ((data[2] & 0x80) && !(old[2] & 0x80)) {
        if (RunNhlAction("SOLO")) { /* Overridden */ }
        else {
             // Default Solo Toggle (Track)
             Main_OnCommand(7, 0); // Track: Toggle Solo for selected tracks
        }
    }

    // PRESET DOWN (0x20) -> FX Page Down / Bypass / Delete
    if ((data[2] & 0x20) && !(old[2] & 0x20)) {
        if (m_current_mode == MODE_FX) {
             // FX MODE: PREV PAGE
             if (m_fx_page > 0) {
                 m_fx_page--;
                 // Get page name
                 MediaTrack* t = GetSelectedTrack(NULL, 0);
                 char fxN[256]; TrackFX_GetFXName(t, m_selected_fx_index, fxN, 256);
                 FXMapping* map = FXMappingRegistry::GetMapping(fxN);
                 std::string pName = (map && m_fx_page < map->pages.size()) ? map->pages[m_fx_page].name : "Auto";
                 
                 char m[64]; sprintf(m, "FX Page %d: %s", m_fx_page+1, pName.c_str());
                 SpeakText(m);
             } else {
                 SpeakText("Page 1");
             }
        }
        else if (m_current_mode == MODE_MIDI) {
             HWND midi_editor = MIDIEditor_GetActive();
             if (midi_editor) { MIDIEditor_OnCommand(midi_editor, 40667); }
             else { Main_OnCommand(NamedCommandLookup("_OSARA_REMOVE"), 0); SpeakText("Delete Item"); }
        } else if (m_current_mode == MODE_AUDIO) {
            Main_OnCommand(NamedCommandLookup("_OSARA_REMOVE"), 0); // Remove items/tracks
            SpeakText("Delete");
        } else {
            Main_OnCommand(40298, 0); // Bypass FX Chain
        }
    }

    // Duplicate PRESET block removed.

    // Byte 3: 4D NAV / MODES
    
    // DIAGNOSTIC LOGGING FOR BUTTONS (Byte 3)
    if (m_hid_capture_until>0 && data[3] != old[3]) {
        char diag[64]; sprintf(diag, "Byte 3 Change: %02X (Was %02X)", data[3], old[3]);
        RawLog(diag);
    }
    
    // SHIFT+BROWSER: Mode Selection. Plain Browser belongs to Komplete
    // Kontrol: it opens NI's browser, which then takes the encoder too.
    // NI ignores Shift+Browser, so the encoder only moves SoundFirst's menu.
    if (isShift && (data[3] & 0x01) && !(old[3] & 0x01)) {
        m_browser_mode_selection = true;
        SpeakText(("Mode " + m_available_modes[m_current_mode_idx]).c_str());
    }
    
    // PLUGIN: FX Mode Directly
    if ((data[3] & 0x02) && !(old[3] & 0x02)) {
        m_browser_mode_selection = false;
        m_current_mode = MODE_FX;
        m_selected_fx_index = 0; 
        m_fx_page = 0; // RESET PAGE on Entry
        SpeakText("FX Mode: Plugin Active");
        
        // Announce current plugin name
        MediaTrack* t = GetSelectedTrack(NULL, 0);
        if (t) {
             char fxN[256]; TrackFX_GetFXName(t, m_selected_fx_index, fxN, 256);
             SpeakText(fxN);
        }
    }
    
    // TRACK: Mixer Mode Directly
    if ((data[3] & 0x04) && !(old[3] & 0x04)) {
        m_browser_mode_selection = false;
        m_current_mode = MODE_MIXER;
        SpeakText("MIXER Mode");
    }
    
    // ENCODER NAVIGATION (Hardcoded)
    // ENCODER NAVIGATION (Hardcoded)
    if ((data[3] & 0x20) && !(old[3] & 0x20)) {
        Main_OnCommand(40286, 0); // ENC_UP -> Prev Track
        UpdateBankFromSelectedTrack();
    }
    if ((data[3] & 0x01) == 0 && (data[4] & 0x01) && !(old[4] & 0x01)) {
        Main_OnCommand(40285, 0); // ENC_DOWN -> Next Track
        UpdateBankFromSelectedTrack();
    }

    if ((data[3] & 0x40) && !(old[3] & 0x40)) { // ENC_LEFT
         if (m_current_mode == MODE_FX) {
             // FX: Prev Plugin
             if (m_selected_fx_index > 0) {
                 m_selected_fx_index--;
                 m_fx_page = 0; // RESET PAGE on Change
             }
             
             MediaTrack* t = GetSelectedTrack(NULL, 0);
             if (t) {
                 // SYNC UI: Select FX in Chain (Simulate Arrow Up behavior)
                 TrackFX_Show(t, m_selected_fx_index, 1); 
                 
                 char fxN[256]; TrackFX_GetFXName(t, m_selected_fx_index, fxN, 256);
                 SpeakText(fxN);
             }
         } else if (m_current_mode == MODE_AUDIO || m_current_mode == MODE_MIDI) {
             Main_OnCommand(40416, 0); // Select prev item
         } else {
             // Global: Prev Marker
             Main_OnCommand(40172, 0); 
             SpeakText("Prev Marker");
         }
    }
    
    if ((data[3] & 0x80) && !(old[3] & 0x80)) { // ENC_RIGHT
         if (m_current_mode == MODE_FX) {
             // FX: Next Plugin
             MediaTrack* t = GetSelectedTrack(NULL, 0);
             if (t) {
                 int count = TrackFX_GetCount(t);
                 if (m_selected_fx_index < count - 1) {
                     m_selected_fx_index++;
                     m_fx_page = 0; // RESET PAGE on Change
                 }
                 
                 // SYNC UI: Select FX in Chain (Simulate Arrow Down behavior)
                 TrackFX_Show(t, m_selected_fx_index, 1);
                 
                 char fxN[256]; TrackFX_GetFXName(t, m_selected_fx_index, fxN, 256);
                 SpeakText(fxN);
             }
         } else if (m_current_mode == MODE_AUDIO || m_current_mode == MODE_MIDI) {
             Main_OnCommand(40417, 0); // Select next item
         } else {
             // Global: Next Marker
             Main_OnCommand(40173, 0); 
             SpeakText("Next Marker");
         }
    }

    // Encoder Click
    if ((data[5] & 0x02) && !(old[5] & 0x02)) {
        if (m_browser_mode_selection) {
            // Confirm Mode
            m_browser_mode_selection = false;
            // Mode Select Logic
            if (m_available_modes[m_current_mode_idx] == "MIXER") m_current_mode = MODE_MIXER;
            else if (m_available_modes[m_current_mode_idx] == "FX") m_current_mode = MODE_FX;
            else if (m_available_modes[m_current_mode_idx] == "MIDI") m_current_mode = MODE_MIDI;
            else if (m_available_modes[m_current_mode_idx] == "AUDIO") m_current_mode = MODE_AUDIO;
            else if (m_available_modes[m_current_mode_idx] == "Instrument") {
                ToggleInstrumentMode();
                AdoptHidBaseline(data);
                return;
            }
             SpeakText(("Mode confirmed: " + m_available_modes[m_current_mode_idx]).c_str());
        } else {
            // Normal Click
            if (m_current_mode == MODE_FX) {
                // FX: Open/Close Window
                MediaTrack* t = GetSelectedTrack(NULL, 0);
                if (t && m_selected_fx_index >= 0) {
                    if (isShift) {
                        // SHIFT+CLICK: Cerrar TODAS las cadenas FX (SWS) o Nativo Safe
                        // 1. Try SWS Action _S&M_WNCLS4
                        int closeAllCmd = NamedCommandLookup("_S&M_WNCLS4");
                        if (closeAllCmd > 0) {
                            Main_OnCommand(closeAllCmd, 0);
                            // SpeakText("Cerrar Ventanas (SWS)");
                        } else {
                            // 2. Fallback: Close Chain for Selected Track
                            // Avoid ESC simulation to prevent clearing Loop Selection in Arrange View
                            TrackFX_Show(t, 0, 0); // Hide Chain
                            // SpeakText("Cerrar Cadena");
                        }
                    } else {
                        // NORMAL CLICK: Open FX Chain (40291)
                        Main_OnCommand(40291, 0);
                        // SpeakText("Cadena FX");
                    }
                }
            } else if (m_current_mode == MODE_MIDI || m_current_mode == MODE_AUDIO) {
                 if (isShift) {
                     // Shift+Click: Close Window (Global or Context)
                     if (m_current_mode == MODE_MIDI) {
                         HWND midi_editor = MIDIEditor_GetActive();
                         if (midi_editor) MIDIEditor_OnCommand(midi_editor, 2); // Close window
                         else Main_OnCommand(2, 0);
                     } else {
                         Main_OnCommand(2, 0);
                     }
                 }
                 else Main_OnCommand(40153, 0);     // Open MIDI Editor (Global)
            } else {
                 Main_OnCommand(40157, 0); // Insert Marker
                 SpeakText("Marker Inserted");
            }
        }
    }
    
    // Touch detection - Byte 4 (K1-K7) with Auto-Solo and ON/OFF actions
    // (Preserve Mixer Auto-Solo Logic)
    for (int k = 0; k < 7; k++) {
        bool current_touch = (data[4] & (1 << (k+1))) != 0;
        bool previous_touch = (old[4] & (1 << (k+1))) != 0;
        
        if (current_touch != previous_touch) {
             // In Mixer Mode: Auto Solo Logic
             if (m_current_mode == MODE_MIXER && m_touch_auto_solo) {
                 int track_idx = (m_current_bank * 8) + k;
                 MediaTrack* track = GetTrack(NULL, track_idx);
                 if (track || (!current_touch && m_knob_touch_state[k])) {
                     if (current_touch) {
                         // Save state and set Solo
                         m_touch_track[k] = track; m_knob_touch_state[k] = true;
                         m_previous_solo_state[k] = *(int*)GetSetMediaTrackInfo(track, "I_SOLO", NULL);
                         int s = 1; GetSetMediaTrackInfo(track, "I_SOLO", &s);
                     } else {
                         // Restore state
                         if (m_touch_track[k] && ValidatePtr2(nullptr,m_touch_track[k],"MediaTrack*"))
                             GetSetMediaTrackInfo(m_touch_track[k], "I_SOLO", &m_previous_solo_state[k]);
                         m_touch_track[k] = nullptr; m_knob_touch_state[k] = false;
                     }
                 }
             }
             // In FX Mode: Pass to VST Logic (TouchK_ON/OFF)
             else if (m_current_mode == MODE_FX) {
                  HandleFxTouch(k, current_touch); // V1.0: Touch Solo Logic
             }
        }
    }
    
    // Touch K8 (separate logic in packet)
    {
        bool current_touch = (data[5] & 0x01) != 0;
        bool previous_touch = (old[5] & 0x01) != 0;
        if (current_touch != previous_touch) {
             if (m_current_mode == MODE_MIXER && m_touch_auto_solo) {
                 int track_idx = (m_current_bank * 8) + 7;
                 MediaTrack* track = GetTrack(NULL, track_idx);
                 if (track || (!current_touch && m_knob_touch_state[7])) {
                     if (current_touch) {
                         m_touch_track[7] = track; m_knob_touch_state[7] = true;
                         m_previous_solo_state[7] = *(int*)GetSetMediaTrackInfo(track, "I_SOLO", NULL);
                         int s = 1; GetSetMediaTrackInfo(track, "I_SOLO", &s);
                     } else {
                         if (m_touch_track[7] && ValidatePtr2(nullptr,m_touch_track[7],"MediaTrack*"))
                             GetSetMediaTrackInfo(m_touch_track[7], "I_SOLO", &m_previous_solo_state[7]);
                         m_touch_track[7] = nullptr; m_knob_touch_state[7] = false;
                     }
                 }
             } else if (m_current_mode == MODE_FX) {
                  HandleFxTouch(7, current_touch); // Knob 8 (Index 7)
             }
        }
    }

    // Encoder Turn (Continuous Counter): byte 28 on A-series, 24 on M32
    const int enc = m_encoder_byte;
    if (data[enc] != old[enc]) {
        int delta = soundfirst_ni::encoderDelta(data[enc], old[enc]);
        if (delta != 0) HandleEncoderRotation(delta);
    }

    // Bytes 6-21: Knobs
    for (int i = 0; i < 8; i++) {
        int b = 6 + (i * 2);
        int val = data[b] | (data[b+1] << 8);
        if (m_last_knob_values[i] != -1 && val != m_last_knob_values[i]) {
            int d = val - m_last_knob_values[i];
            if (d > 512) d -= 1024; else if (d < -512) d += 1024;
            if (abs(d) < 200) HandleKnobRotation(i, d);
        }
        m_last_knob_values[i] = val;
    }

    memcpy(m_last_hid_report, data, 64);
}


void CSurf_SoundFirst::HandleEncoderRotation(int delta) {
    if (delta == 0) return;
    ResetIdleTimer();
    
    // Check if we're in browser mode selection
    if (m_browser_mode_selection) {
        // BROWSER MODE: Navigate through available modes
        if (delta > 0) {
            m_current_mode_idx = (m_current_mode_idx + 1) % m_available_modes.size();
        } else {
            m_current_mode_idx--;
            if (m_current_mode_idx < 0) m_current_mode_idx = (int)m_available_modes.size() - 1;
        }
        
        // Announce mode
        SpeakText(("Mode " + m_available_modes[m_current_mode_idx]).c_str());
    } else {
        // NORMAL MODE: Hardcoded Actions
        if (m_shift_pressed) {
            
            // --- MIXER MODE SHIFT LOGIC (User Request) ---
            if (m_current_mode == MODE_MIXER) {
                 // Shift + Rotate = Move Loop Points (40632/40633)
                 if (delta > 0) Main_OnCommand(40632, 0); // Loop Right
                 else Main_OnCommand(40633, 0);           // Loop Left
                 return;
            }
            
            // SHIFT: Transient Navigation (OSARA)
            if (delta > 0) Main_OnCommand(NamedCommandLookup("_OSARA_NEXTTRANSIENT"), 0);
            else Main_OnCommand(NamedCommandLookup("_OSARA_PREVTRANSIENT"), 0);
        } else {
            // NORMAL: Move Edit Cursor (One Beat)
            if (delta > 0) Main_OnCommand(41044, 0); // Move edit cursor forward one beat
            else Main_OnCommand(41045, 0);           // Move edit cursor back one beat
        }
    }
}

// Dispatcher with Adaptive Acceleration
void CSurf_SoundFirst::HandleKnobRotation(int idx, int d) {
    if (d == 0) return;
    ResetIdleTimer();

    // ADAPTIVE ACCELERATION (V1.0)
    // Formula: d_final = d * (1 + (abs(d) / 4))
    // Small turns (d=1,2,3) -> Stay 1, 2, 3 (Precision)
    // Medium turns (d=4,5) -> Becomes 8, 11 (Boost)
    // Fast turns (d=10) -> Becomes 35 (Turbo)
    
    int ad = abs(d);
    int accel_d = d;
    
    // Split Acceleration Profiles
    if (m_current_mode == MODE_FX) {
        // GENTLER curve for FX (Parameters 0.0-1.0 are sensitive)
        if (ad > 4) {
             accel_d = d * (1 + (ad / 8)); 
        }
    } else {
        // AGGRESSIVE curve for Mixer/MIDI (Faders need travel)
        if (ad > 3) {
             accel_d = d * (1 + (ad / 4));
        }
    }
    
    if (m_current_mode == MODE_MIXER) HandleKnob_Mixer(idx, accel_d);
    else if (m_current_mode == MODE_MIDI) HandleKnob_MIDI(idx, accel_d);
    else if (m_current_mode == MODE_AUDIO) HandleKnob_Audio(idx, accel_d);
    else if (m_current_mode == MODE_FX) HandleKnob_FX(idx, accel_d);
}


void CSurf_SoundFirst::HandleKnob_Mixer(int idx, int d) {
    bool sh = m_shift_pressed;
    MediaTrack* t = GetTrack(NULL, (m_current_bank * 8) + idx);
    if (!t) return;

    if (sh) {
        // PAN
        double vol = GetMediaTrackInfo_Value(t, "D_PAN");
        vol += (d * m_knob_sensitivity);
        if (vol < -1.0) vol = -1.0; else if (vol > 1.0) vol = 1.0;
        SetMediaTrackInfo_Value(t, "D_PAN", vol);
    } else {
        // VOL
        double vol = GetMediaTrackInfo_Value(t, "D_VOL");
        vol *= pow(10.0, (d * m_knob_sensitivity * 2.0) / 20.0);
        if (vol < 0.0000001) vol = 0.0; else if (vol > 3.98) vol = 3.98; // +12dB
        SetMediaTrackInfo_Value(t, "D_VOL", vol);
    }
}

void CSurf_SoundFirst::HandleKnob_MIDI(int idx, int d) {
    // V1.0 MIDI Logic: Restored V1.2 Context Logic
    HWND midi_editor = MIDIEditor_GetActive();
    bool sh = m_shift_pressed;

    if (midi_editor) {
        // EDITOR OPEN: Use Targeted API (Matches V1.2 behavior)
        if (idx == 0) { // K1: Nav Time
            if (sh) {
                 int cmd = NamedCommandLookup("_OSARA_NEXTCHORDKEEPSEL");
                 if (d > 0) MIDIEditor_OnCommand(midi_editor, cmd ? cmd : 0); 
                 else { cmd = NamedCommandLookup("_OSARA_PREVCHORDKEEPSEL"); MIDIEditor_OnCommand(midi_editor, cmd ? cmd : 0); }
            } else {
                 int cmd = NamedCommandLookup("_OSARA_NEXTCHORD");
                 if (d > 0) MIDIEditor_OnCommand(midi_editor, cmd ? cmd : 0);
                 else { cmd = NamedCommandLookup("_OSARA_PREVCHORD"); MIDIEditor_OnCommand(midi_editor, cmd ? cmd : 0); }
            }
        } 
        else if (idx == 1) { // K2: Nav Pitch
            if (d > 0) MIDIEditor_OnCommand(midi_editor, NamedCommandLookup("_OSARA_HIGHERNOTEINCHORD"));
            else MIDIEditor_OnCommand(midi_editor, NamedCommandLookup("_OSARA_LOWERNOTEINCHORD"));
        }
        else if (idx == 2) { // K3: Grid/Pixel Move
             // Note: V1.2 used Main IDs here? If user provided Main IDs (4xxxx), try Editor first.
             // If these fail, we might need a lookup map. But keeping user IDs for now.
             if (sh) { if (d > 0) MIDIEditor_OnCommand(midi_editor, 40182); else MIDIEditor_OnCommand(midi_editor, 40181); } 
             else { if (d > 0) MIDIEditor_OnCommand(midi_editor, 40184); else MIDIEditor_OnCommand(midi_editor, 40183); }
        }
        else if (idx == 3) { // K4: Len Grid/Pixel
             if (sh) { if (d > 0) MIDIEditor_OnCommand(midi_editor, 40444); else MIDIEditor_OnCommand(midi_editor, 40445); } 
             else { if (d > 0) MIDIEditor_OnCommand(midi_editor, 40446); else MIDIEditor_OnCommand(midi_editor, 40447); }
        }
        else if (idx == 4) { // K5: Transpose
             if (sh) { if (d > 0) MIDIEditor_OnCommand(midi_editor, 40179); else MIDIEditor_OnCommand(midi_editor, 40180); } 
             else { if (d > 0) MIDIEditor_OnCommand(midi_editor, 40177); else MIDIEditor_OnCommand(midi_editor, 40178); }
        }
        else if (idx == 5) { // K6: Velocity
             if (sh) { if (d > 0) MIDIEditor_OnCommand(midi_editor, 40463); else MIDIEditor_OnCommand(midi_editor, 40465); } 
             else { if (d > 0) MIDIEditor_OnCommand(midi_editor, 40462); else MIDIEditor_OnCommand(midi_editor, 40464); }
        }
        else if (idx == 6) { // K7: Grid Cycle (10 Steps)
             // FIX: Inverted Logic per User Request (Right = Decrease Grid? Or User meant inverted direction?)
             // User said: "girar a la derecha hace lo que deberia hacer a la izquierda".
             // Currently: d > 0 (Right) -> index++ (Finer? Or Coarser?).
             // Array: {1/48, ... 1}. Index 0 = Smallest (1/48). Index 9 = Largest (1).
             // Originally: Right -> ++ -> Larger (1/1).
             // User wants Invert. So Right (d > 0) -> -- (Smaller/Finer).
             static const int grid_actions[10] = { 40189, 40190, 40191, 40192, 40193, 40197, 40199, 40201, 40203, 40204 };
             if (d > 0) { if (m_grid_index > 0) m_grid_index--; } 
             else { if (m_grid_index < 9) m_grid_index++; }
             
             // Route to Editor
             if (midi_editor) MIDIEditor_OnCommand(midi_editor, grid_actions[m_grid_index]);
             else Main_OnCommand(grid_actions[m_grid_index], 0); // Fallback for consistency
        }
        else if (idx == 7) { // K8: Zoom
             if (d > 0) Main_OnCommand(1012, 0); else Main_OnCommand(1011, 0); // Zoom is Global
        }
    } else {
        // FALLBACK: MIDI ITEM OBJECT EDITING (Arrange View)
        // Checks if MIDI Editor is closed. If so, we treat the MIDI Clip as an Object (Item).
        // This allows Moving, Resizing, and Trimming the MIDI Item itself.
        // (Logic is identical to Audio Item handling, but applied to MIDI Clips).
        HandleKnob_Audio(idx, d);
    }
}

void CSurf_SoundFirst::HandleKnob_Audio(int idx, int d) {
    bool sh = m_shift_pressed;
    if (idx == 0) { // Legacy Cursor
        if (d > 0) Main_OnCommand(40105, 0); else Main_OnCommand(40104, 0);
    }
    else if (idx == 1) { // Move Item
         if (d > 0) Main_OnCommand(40119, 0); else Main_OnCommand(40120, 0);
    }
    else if (idx == 2) { // Trim Start
         if (d > 0) Main_OnCommand(40226, 0); else Main_OnCommand(40225, 0);
    }
    else if (idx == 3) { // Trim End
         if (d > 0) Main_OnCommand(40228, 0); else Main_OnCommand(40227, 0);
    }
    else if (idx == 4) { // Vol
         if (d > 0) Main_OnCommand(NamedCommandLookup("_XENAKIOS_NUDGETAKEVOLUP"), 0); 
         else Main_OnCommand(NamedCommandLookup("_XENAKIOS_NUDGETAKEVOLDOWN"), 0);
    }
    else if (idx == 5) { // Fade In (Native - No SWS Required)
         MediaTrack* t = GetSelectedTrack(NULL, 0); // Or use generic item selection?
         // Audio Mode usually targets *Selected Items*.
         // We iterate over selected items to apply fade.
         int num_sel = CountSelectedMediaItems(NULL);
         for (int i=0; i<num_sel; ++i) {
             MediaItem* item = GetSelectedMediaItem(NULL, i);
             if (item) {
                  double len = GetMediaItemInfo_Value(item, "D_FADEINLEN");
                  len += (d * 0.01); // Sensitivity
                  if (len < 0.0) len = 0.0;
                  SetMediaItemInfo_Value(item, "D_FADEINLEN", len);
             }
         }
         if (num_sel == 0) SpeakText("Select an Item");
         else UpdateTimeline(); // Redraw
    }
    else if (idx == 6) { // Fade Out (Native - No SWS Required)
         int num_sel = CountSelectedMediaItems(NULL);
         for (int i=0; i<num_sel; ++i) {
             MediaItem* item = GetSelectedMediaItem(NULL, i);
             if (item) {
                  double len = GetMediaItemInfo_Value(item, "D_FADEOUTLEN");
                  len += (d * 0.01); 
                  if (len < 0.0) len = 0.0;
                  SetMediaItemInfo_Value(item, "D_FADEOUTLEN", len);
             }
         }
         if (num_sel == 0) SpeakText("Select an Item");
         else UpdateTimeline();
    }
    else if (idx == 7) { // Zoom
         if (d > 0) Main_OnCommand(1012, 0); else Main_OnCommand(1011, 0);
    }
}

// Helper for Touch Solo
void CSurf_SoundFirst::HandleFxTouch(int knob_idx, bool touched) {
    if (m_current_mode != MODE_FX) return;
    
    // Respect Global Auto-Solo Toggle (Ideas/Scale Button)
    if (!m_touch_auto_solo) return; 

    MediaTrack* t = GetSelectedTrack(NULL, 0);
    if (!t || m_selected_fx_index < 0) return;

    char fxN[256]; TrackFX_GetFXName(t, m_selected_fx_index, fxN, 256);
    FXMapping* map = FXMappingRegistry::GetMapping(fxN);
    
    if (map && m_fx_page < map->pages.size()) {
         // Check if this knob has a Touch Param (e.g. Solo)
         int touch_p = map->pages[m_fx_page].knobs[knob_idx].touch_param_id;
         if (touch_p != -1) {
             TrackFX_SetParam(t, m_selected_fx_index, touch_p, touched ? 1.0 : 0.0);
             // Feedback only on touch to avoid spam
             if (touched) SpeakText("Solo");
         }
    }
}

void CSurf_SoundFirst::HandleKnob_FX(int idx, int d) {
    bool sh = m_shift_pressed;
    MediaTrack* t = GetSelectedTrack(NULL, 0);
    if (!t || m_selected_fx_index < 0) return;

    char fxN[256]; TrackFX_GetFXName(t, m_selected_fx_index, fxN, 256);
    
    // 1. Try V3 Pro Mapping Registry
    FXMapping* map = FXMappingRegistry::GetMapping(fxN);
    int p = -1;

    if (map) {
        if (m_fx_page < map->pages.size()) {
            const FXKnobMap& km = map->pages[m_fx_page].knobs[idx];
            if (sh && km.shift_param_id != -1) p = km.shift_param_id;
            else p = km.param_id;
            
            // Handle Toggle type (if used for "Band Used" logic)
            if (km.is_toggle && p != -1) {
                // If it's a toggle, we might want to just flip it on rotation?
                // Or standard increment? Pro-Q "Band Used" is usually 0.0 or 1.0?
                // For now, treat as standard param, but maybe stricter steps.
            }
        }
    }

    // 2. Fallback: Auto-Map (Linear)
    if (p == -1) {
        // Standard Auto-Map
        int auto_p = idx + (m_fx_page * 8);
        if (sh) auto_p += 1000; // Shift page offset
        int param_count = TrackFX_GetNumParams(t, m_selected_fx_index);
        if (auto_p < param_count) p = auto_p;
    }

    if (p != -1) {
        double current = TrackFX_GetParam(t, m_selected_fx_index, p, NULL, NULL);
        double step = (sh ? m_knob_sensitivity_shift : m_knob_sensitivity) * d;
        TrackFX_SetParam(t, m_selected_fx_index, p, current + step);
    }
}

void CSurf_SoundFirst::HandleSpecificKnobTouch(int i, bool o, bool f) {}

void CSurf_SoundFirst::DumpCurrentFX() {
    MediaTrack* t = GetSelectedTrack(NULL, 0);
    if (!t) return;
    char fxN[256]; TrackFX_GetFXName(t, m_selected_fx_index, fxN, 256);
    const char* rp = GetResourcePath();
    if (rp) {
        char path[1024]; sprintf(path, "%s\\UserPlugins\\FX_Dump.txt", rp);
        std::ofstream f(path);
        if (f.is_open()) {
            std::stringstream ss; ss << "[" << fxN << "]\n";
            int count = TrackFX_GetNumParams(t, m_selected_fx_index);
            for (int i = 0; i < count; i++) {
                char pN[256]; TrackFX_GetParamName(t, m_selected_fx_index, i, pN, 256);
                ss << "; " << i << " = " << pN << "\n";
            }
            f << ss.str(); f.close();
            if (OpenClipboard(NULL)) {
                EmptyClipboard();
                HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, ss.str().size() + 1);
                if (h) { memcpy(GlobalLock(h), ss.str().c_str(), ss.str().size() + 1); GlobalUnlock(h); SetClipboardData(CF_TEXT, h); }
                CloseClipboard();
            }
            SpeakText("Dump copied");
        }
    }
}

void CSurf_SoundFirst::CheckConfigHotReload() {
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (GetFileAttributesExA(m_config_path.c_str(), GetFileExInfoStandard, &d)) {
        WDL_UINT64 now = ((WDL_UINT64)d.ftLastWriteTime.dwHighDateTime << 32) | d.ftLastWriteTime.dwLowDateTime;
        if (m_last_config_time != 0 && now > m_last_config_time) { m_last_config_time = now; LoadMappingConfig(); }
    }
}

void CSurf_SoundFirst::LoadMappingConfig() {
    g_plugin_mappings.clear(); m_nhl_actions.clear(); m_mode_actions.clear();
    
    // Keep current accessibility and modes as defaults (already initialized in constructor)
    // They will be overridden if found in INI
    
    // Default Actions (Sane fallbacks)
    m_nhl_actions["PLAY"] = "40044";
    m_nhl_actions["STOP"] = "1016";
    m_nhl_actions["REC"] = "1013";
    m_nhl_actions["LOOP"] = "1068";
    m_nhl_actions["BROWSER"] = "40271"; // Media Explorer
    m_nhl_actions["UNDO"] = "40029";
    m_nhl_actions["QUANTIZE"] = "40469";
    m_nhl_actions["PRESET_UP"] = "@BANK_UP";
    m_nhl_actions["PRESET_DOWN"] = "@BANK_DOWN";
    m_nhl_actions["ENC_PRESS"] = "40294";

    std::ifstream f(m_config_path);
    if (!f.is_open()) { LogDebug("[NI_CONFIG] INI not found; DoubleModeExit and legacy DoubleTrackExit default ON"); return; }
    
    std::string line, sec;
    std::vector<std::string> loaded_modes; // Temporary storage for modes
    
    while (std::getline(f, line)) {
        line = trim(line); if (line.empty() || line[0] == ';') continue;
        if (line[0] == '[' && line.back() == ']') { sec = trim(line.substr(1, line.size() - 2)); continue; }
        size_t eq = line.find('='); if (eq == std::string::npos) continue;
        std::string k = trim(line.substr(0, eq)), v = trim(line.substr(eq+1));
        
        // Section: ACCESSIBILITY
        if (sec == "ACCESSIBILITY") {
            if (k == "AnnounceButtons") m_announce_buttons = (std::stoi(v) != 0);
            else if (k == "AnnounceKnobs") m_announce_knobs = (std::stoi(v) != 0);
            else if (k == "AnnounceEncoder") m_announce_encoder = (std::stoi(v) != 0);
            else if (k == "AnnounceActions") m_announce_actions = (std::stoi(v) != 0);
            else if (k == "TouchAutoSolo") m_touch_auto_solo = (std::stoi(v) != 0);
        }

        
        // Optional Instrument exit gesture. Fail closed on invalid config.
        else if (sec == "INSTRUMENT") {
            try {
                if (k == "DoubleModeExit") {
                    m_double_mode_exit=std::stoi(v)!=0;
                    LogDebug(m_double_mode_exit ? "[NI_CONFIG] DoubleModeExit ON (selector byte3 mask01)" : "[NI_CONFIG] DoubleModeExit OFF");
                }
                else if (k == "DoubleModeWindowMs")
                    m_double_mode_window=std::clamp(std::stoi(v),150,1000)/1000.0;
                else if (k == "DoubleTrackExit") {
                    m_double_track_exit = std::stoi(v) != 0;
                    LogDebug(m_double_track_exit ? "[NI_CONFIG] DoubleTrackExit ON" : "[NI_CONFIG] DoubleTrackExit OFF");
                }
                else if (k == "ShiftTrackExit") {
                    m_shift_track_exit=std::stoi(v)!=0;
                    LogDebug(m_shift_track_exit ? "[NI_CONFIG] ShiftTrackExit ON" : "[NI_CONFIG] ShiftTrackExit OFF");
                }
                else if (k == "HoldUndoExit") {
                    m_hold_undo_exit=std::stoi(v)!=0;
                    LogDebug(m_hold_undo_exit ? "[NI_CONFIG] HoldUndoExit ON" : "[NI_CONFIG] HoldUndoExit OFF");
                }
                else if (k == "DoubleTrackWindowMs")
                    m_double_track_window = std::clamp(std::stoi(v), 150, 1000) / 1000.0;
            } catch (...) { LogDebug("Invalid Instrument config ignored"); }
            m_track_press.reset(); m_mode_press.reset();
        }
        // Section: MODES
        else if (sec == "MODES") {
            if (k.find("Mode") == 0) {
                // Rename legacy "EDIT" mode to "AUDIO" and "FX" to "PLUGIN"
                std::string mode_val = (v == "EDIT") ? "AUDIO" : (v == "FX" ? "PLUGIN" : v);
                loaded_modes.push_back(mode_val);
            }
        }
        
        // Section: NHL_ACTIONS
        else if (sec == "NHL_ACTIONS") m_nhl_actions[k] = v;
        
        // Section: MODE-specific actions (e.g. MODE_MIXER, MODE_FX)
        else if (sec.find("MODE_") == 0) {
            // Determine which mode this is for
            std::string mode_name = sec.substr(5); // Remove "MODE_" prefix
            ControlMode mode = MODE_MIXER; // default
            if (mode_name == "MIXER") mode = MODE_MIXER;
            else if (mode_name == "FX" || mode_name == "PLUGIN") mode = MODE_FX;
            else if (mode_name == "MIDI") mode = MODE_MIDI;
            else if (mode_name == "EDIT" || mode_name == "AUDIO") mode = MODE_AUDIO;
            
            m_mode_actions[mode][k] = v;
        }
        
        // Legacy NHL mode sections
        else if (sec == "NHL_ACTIONS_MIXER") m_mode_actions[MODE_MIXER][k] = v;
        else if (sec == "NHL_ACTIONS_FX") m_mode_actions[MODE_FX][k] = v;
        
        // Plugin mappings
        else {
            try {
                if (k.find("ShiftKnob") == 0) g_plugin_mappings[sec][1000 + (std::stoi(k.substr(9)) - 1)] = std::stoi(v);
                else if (k.find("Knob") == 0) g_plugin_mappings[sec][std::stoi(k.substr(4)) - 1] = std::stoi(v);
                else if (k.find("Touch") == 0) g_plugin_mappings[sec][2000 + (std::stoi(k.substr(5)) - 1)] = std::stoi(v);
                
                else if (k == "IDEAS") g_plugin_mappings[sec][3000] = std::stoi(v);
                else if (k == "QUANTIZE") g_plugin_mappings[sec][3001] = std::stoi(v);
                else if (k == "UNDO") g_plugin_mappings[sec][3002] = std::stoi(v);
                else if (k == "METRO") g_plugin_mappings[sec][3003] = std::stoi(v);
                else if (k == "LOOP") g_plugin_mappings[sec][3004] = std::stoi(v);
                else if (k == "MUTE") g_plugin_mappings[sec][3005] = std::stoi(v);
                else if (k == "SOLO") g_plugin_mappings[sec][3006] = std::stoi(v);
                else if (k == "PRESET_UP") g_plugin_mappings[sec][3007] = std::stoi(v);
                else if (k == "PRESET_DOWN") g_plugin_mappings[sec][3008] = std::stoi(v);
                else if (k == "PLAY") g_plugin_mappings[sec][3009] = std::stoi(v);
                else if (k == "STOP") g_plugin_mappings[sec][3010] = std::stoi(v);
                else if (k == "REC") g_plugin_mappings[sec][3011] = std::stoi(v);
                else if (k == "ENC_PRESS") g_plugin_mappings[sec][2017] = std::stoi(v);
                else if (k == "REC") g_plugin_mappings[sec][2018] = std::stoi(v);
            } catch (...) {}
        }
    }
    
    // Apply loaded modes if any were found
    if (!loaded_modes.empty()) {
        m_available_modes = loaded_modes;
        // Clamp current index if necessary
        if (m_current_mode_idx >= (int)m_available_modes.size()) {
            m_current_mode_idx = 0;
        }
    }
}

void CSurf_SoundFirst::SwitchMode(ControlMode m) { m_current_mode = m; }
void CSurf_SoundFirst::LogDebug(const char* text) {
    if (!text) return;
    char* appData=getenv("APPDATA");
    if (!appData) return;
    char path[512]; sprintf(path,"%s\\REAPER\\UserPlugins\\SoundFirst_Logs.txt",appData);
    FILE* f=fopen(path,"a");
    if (f) { fprintf(f,"%s\n",text); fclose(f); }
}

// Header for string safety
#include <string>

// Header for string safety
#include <string>

void CSurf_SoundFirst::SpeakText(const char* text) {
    if (!text) return;

    // if (ShowConsoleMsg) ShowConsoleMsg((std::string("[A61] ") + text + "\n").c_str());

    // OSARA via g_rec (Legacy/Direct Access as requested)
    static void (*osara_outputMessage)(const char*) = NULL;
    
    if (!osara_outputMessage && g_rec) {
        osara_outputMessage = (void (*)(const char*))g_rec->GetFunc("osara_outputMessage");
    }

    if (osara_outputMessage) {
        osara_outputMessage(text);
    }
}

void CSurf_SoundFirst::SendMidi(unsigned char s, unsigned char d1, unsigned char d2) { 
    const unsigned char bytes[] = {s,d1,d2};
    if (!soundfirst_ni::allowOutput(m_current_mode == MODE_INSTRUMENT,bytes,3)) return;
    if (m_midi_out) m_midi_out->Send(s,d1,d2,-1);
}


void CSurf_SoundFirst::SendSysex(unsigned char* d, int l) {
    if (l < 0 || !soundfirst_ni::allowOutput(m_current_mode == MODE_INSTRUMENT,d,std::size_t(l))) return;
    if (m_midi_out) { 
        // Safer vector-based allocation
        std::vector<char> evt(sizeof(MIDI_event_t) + l);
        MIDI_event_t* e = (MIDI_event_t*)evt.data();
        e->frame_offset = 0; 
        e->size = l; 
        memcpy(e->midi_message, d, l); 
        m_midi_out->SendMsg(e, -1); 
    }
}



void CSurf_SoundFirst::SetSurfaceVolume(MediaTrack* t, double v) { m_ni_feedback_dirty = true; }
void CSurf_SoundFirst::SetSurfacePan(MediaTrack* t, double p) { m_ni_feedback_dirty = true; }
void CSurf_SoundFirst::SetSurfaceMute(MediaTrack* t, bool m) { m_ni_feedback_dirty = true; }
void CSurf_SoundFirst::SetSurfaceSolo(MediaTrack* t, bool s) { m_ni_feedback_dirty = true; }
void CSurf_SoundFirst::SetSurfaceRecArm(MediaTrack* t, bool r) { m_ni_feedback_dirty = true; }
void CSurf_SoundFirst::SetPlayState(bool p, bool pa, bool r) { m_ni_feedback_dirty = true; }
void CSurf_SoundFirst::SetRepeatState(bool r) { m_ni_feedback_dirty = true; }

// --- DISPLAY ENGINE ---

void CSurf_SoundFirst::ResetIdleTimer() {
    m_last_input_time = time_precise();
    if (m_screensaver_active) {
        m_screensaver_active = false;
        m_startup_announced = false; // Reset simple flag to force redraw if needed
        UpdateDisplay(); 
    }
}

void CSurf_SoundFirst::SendLCDMessage(unsigned char cmd, const char* text) {
    if (!m_midi_out || !text) return;
    
    // Header (10) + Cmd (1) + Val (1) + Idx (1) + Text (N) + F7 (1)
    std::vector<unsigned char> sysex;
    sysex.reserve(128);
    
    // F0 00 21 09 00 00 44 43 01 00
    sysex.push_back(0xF0); sysex.push_back(0x00); sysex.push_back(0x21); sysex.push_back(0x09);
    sysex.push_back(0x00); sysex.push_back(0x00); sysex.push_back(0x44); sysex.push_back(0x43);
    sysex.push_back(0x01); sysex.push_back(0x00);
    
    sysex.push_back(cmd);
    sysex.push_back(0); // Value
    sysex.push_back(0); // Index
    
    // Clean string from non-ASCII
    for (int i=0; text[i] != 0 && i < 64; i++) {
        if (text[i] >= 32 && text[i] < 127) sysex.push_back((unsigned char)text[i]);
    }
    
    sysex.push_back(0xF7);
    SendSysex(sysex.data(), (int)sysex.size());
}

void CSurf_SoundFirst::SendLCDValue(unsigned char cmd, int val) {
    if (!m_midi_out) return;
    
    std::vector<unsigned char> sysex;
    sysex.reserve(16);
    
    sysex.push_back(0xF0); sysex.push_back(0x00); sysex.push_back(0x21); sysex.push_back(0x09);
    sysex.push_back(0x00); sysex.push_back(0x00); sysex.push_back(0x44); sysex.push_back(0x43);
    sysex.push_back(0x01); sysex.push_back(0x00);
    
    sysex.push_back(cmd);
    sysex.push_back((unsigned char)(val & 0x7F)); // Clamp 0-127
    sysex.push_back(0); // Index
    
    sysex.push_back(0xF7);
    SendSysex(sysex.data(), (int)sysex.size());
}


// Helper to pad string for display safety (prevents garbage)
static void PadString(char* dest, const char* src, int length) {
    int i = 0;
    while (src[i] != 0 && i < length - 1) {
        dest[i] = src[i];
        i++;
    }
    while (i < length - 1) {
        dest[i] = ' '; // Pad with spaces
        i++;
    }
    dest[length - 1] = 0;
}



// Send MIDI CC Helper (Direct to MIDI Out)
void CSurf_SoundFirst::SendMidiCC(int cc, int val) {
    // Custom display/bank mapping belongs to SoundFirst only outside Instrument.
    if (m_current_mode == MODE_INSTRUMENT) return;
    if (m_midi_out) m_midi_out->Send(0xBF, cc, val, -1);
}

void CSurf_SoundFirst::UpdateDisplay() {
    if (m_current_mode == MODE_INSTRUMENT) return;
    auto& last_line1 = m_display_cache.line1;
    auto& last_line2_name = m_display_cache.line2_name;
    auto& last_line2_val = m_display_cache.line2_value;
    auto& last_screensaver = m_display_cache.screensaver;
    auto& last_browser_mode = m_display_cache.selector;
    auto& last_track_available = m_display_cache.track_available;
    auto& last_mode_force = m_display_cache.bank_mapping;

    // Force Mode refresh periodically? No, just on change.
    
    // 1. Screensaver Priority
    if (time_precise() - m_last_input_time > 10.0) { // 10s Timeout
        if (!m_screensaver_active) {
            m_screensaver_active = true;
        }
    }

    if (m_screensaver_active) {
        if (!last_screensaver) {
            m_screensaver_active = true;
            last_screensaver = true;
            // FX bank mapping for custom text; this is not native Browser ownership.
            if (last_mode_force != 1) { SendMidiCC(5, 1); last_mode_force = 1; }
            
            SendLCDMessage(0x48, "SoundFirst PRO                  "); 
            SendLCDMessage(0x72, "                                ");               
            SendLCDMessage(0x73, "          Idle Mode             "); 
            SendLCDValue(0x49, 0); 
            SendLCDValue(0x40, 0); // Track Empty (maybe helps clear?)
        }
        return; 
    }

    // 2. Wake Logic
    if (last_screensaver) {
        last_screensaver = false;
        last_line1[0] = 0; 
        last_line2_name[0] = 0; 
        last_line2_val[0] = 0;
        last_track_available = -1; // Force status resend
    }
    
    // 3. Browser Mode Priority (User Request: "No cambia modo")
    if (m_browser_mode_selection) {
        // Force refresh if entering this mode
        if (!last_browser_mode) {
             last_line1[0] = 0; last_line2_name[0] = 0; last_line2_val[0] = 0;
             last_browser_mode = true;
             // FX bank mapping for selector text, not a native Plugin/Browser switch.
             if (last_mode_force != 1) { SendMidiCC(5, 1); last_mode_force = 1; }
        }
        
        SendLCDMessage(0x48, "SELECT MODE                     ");
        char modeBuf[64]; sprintf(modeBuf, "Current: %-23s", m_available_modes[m_current_mode_idx].c_str());
        SendLCDMessage(0x72, modeBuf); // Line 2 Name
        SendLCDMessage(0x73, "                                "); // Line 2 Val (Clear)
        // Force Track Available so screen stays on?
        SendLCDValue(0x40, 1); 
        return; // Important: Don't run standard logic
    }
    
    if (last_browser_mode) {
        last_browser_mode = false;
        // Force refresh on exit
        last_line1[0] = 0; last_line2_name[0] = 0; last_line2_val[0] = 0;
        last_track_available = -1;
    }

    // 4. Active Display Logic
    char line1[64];
    char line2_name[64];
    char line2_val[64];
    int track_available = 0; // 0=Empty, 1=Audio
    
    // Line 1: Track Context
    MediaTrack* tr = GetSelectedTrack(0, 0);
    if (tr) {
        track_available = 1;
        char name[256]; name[0] = 0;
        // FIX: GetSetMediaTrackInfo with a pointer as 3rd arg SETS the value.
        // We must pass NULL to GET the pointer.
        char* pName = (char*)GetSetMediaTrackInfo(tr, "P_NAME", NULL);
        if (pName) {
            strncpy(name, pName, 255);
            name[255] = 0;
        }

        if (strlen(name) == 0) sprintf(line1, "Track %d                        ", (int)GetMediaTrackInfo_Value(tr, "IP_TRACKNUMBER"));
        else sprintf(line1, "%d. %-.24s", (int)GetMediaTrackInfo_Value(tr, "IP_TRACKNUMBER"), name); // Limit to 24 chars safely
    } else {
        track_available = 0; // Host tells Controller: EMPTY
        sprintf(line1, "No Track Selected               ");
    }

    // Determine Mode for Display
    int desired_mode = 0; // Track bank mapping
    
    // Line 2: Mode Context
    if (m_current_mode == MODE_FX) {
        desired_mode = 1; // FX bank mapping
        if (tr) {
             char fxName[256];
             if (TrackFX_GetFXName(tr, m_selected_fx_index, fxName, 256)) {
                 sprintf(line2_name, "FX: %-24s", fxName);
             } else {
                 sprintf(line2_name, "FX: (Empty)                     ");
             }
             sprintf(line2_val, "Page: %d                       ", m_fx_page + 1);
        } else {
            sprintf(line2_name, "FX Mode                         ");
            sprintf(line2_val, "No Track                        ");
        }
    } else {
        // Mixer Mode (Volume/Pan). Keep Native Mixer?
        // Actually, if we want custom strings to work, we might need Plugin Mode even for Mixer?
        // But let's try Native Mixer since user liked the "Vol" bar layout.
        desired_mode = 0; 
        if (tr) {
            double vol = GetMediaTrackInfo_Value(tr, "D_VOL");
            double volIdx = MKVOL2DB(vol);
            
            // For Mixer Mode, we populate the strings, but we will send them to DIFFERENT addresses
            sprintf(line2_name, "Volume                          "); // Just label (or Pan?)
            
            if (volIdx < -140.0) sprintf(line2_val, "-inf dB                         ");
            else sprintf(line2_val, "%.1f dB                         ", volIdx);
        } else {
             sprintf(line2_name, "Mixer Mode                      ");
             sprintf(line2_val, "                                ");
        }
    }
    
    // 5. SEND UPDATES
    
    // Bank mapping (CC 05 on channel 16): tracks=0, FX mapping=1.
    if (desired_mode != last_mode_force) {
        SendMidiCC(5, desired_mode); 
        last_mode_force = desired_mode;
    }
    
    // Track Available Status (Critical for Display State)
    if (track_available != last_track_available) {
        // 0x40 = Track Type (0=Empty, 1=Audio, 6=Master)
        // If track is MASTER, maybe send 6? For now use 1 for active.
        SendLCDValue(0x40, track_available); 
        // Also send Selected state? (0x42)
        SendLCDValue(0x42, track_available); 
        last_track_available = track_available;
    }
    
    // Text Updates
    if (strncmp(line1, last_line1, 63) != 0) {
        SendLCDMessage(0x48, line1); // Top Line (Track Name)
        strcpy(last_line1, line1);
    }

    // Bottom Line Logic: Mixer vs FX
    if (m_current_mode == MODE_FX || m_browser_mode_selection) {
        // Use PARAM DISPLAY (0x72/0x73)
        if (strncmp(line2_name, last_line2_name, 63) != 0) {
            SendLCDMessage(0x72, line2_name);
            strcpy(last_line2_name, line2_name);
        }
        if (strncmp(line2_val, last_line2_val, 63) != 0) {
            SendLCDMessage(0x73, line2_val);
            strcpy(last_line2_val, line2_val);
        }
    } else {
        // MODE_MIXER: Use VOLUME TEXT (0x46) and maybe PAN TEXT (0x47)
        // The User reported "Vol" stuck at bottom.
        // We will send our Volume string to 0x46.
        if (strncmp(line2_val, last_line2_val, 63) != 0) { // Using line2_val ("-inf dB")
             SendLCDMessage(0x46, line2_val); // 0x46 = Volume Text
             strcpy(last_line2_val, line2_val);
             
             // Also Clear 0x72/0x73 to prevent overlap?
             // SendLCDMessage(0x72, "                                ");
        }
        // Pan?
        // SendLCDMessage(0x47, "Pan Value...");
    }
}


// Instrument mode bridge: independent implementation of INSTRUMENT_MODE_SPEC.md.
void CSurf_SoundFirst::SendNiPacket(const soundfirst_ni::Packet& packet) {
    if (packet.size()==3 && packet[0]==0xBF) {
        SendMidi(packet[0],packet[1],packet[2]);
    } else if (!packet.empty()) {
        SendSysex(const_cast<unsigned char*>(packet.data()),int(packet.size()));
    }
}

void CSurf_SoundFirst::OpenNiSession() {
    m_ni_version = 0;
    m_ni_recovery.reset();
    m_ni_final_timeout_spoken=false; m_ni_last_connect_log.clear();
    m_ni_received=0; m_ni_cc_received=0; m_ni_rejected=0;
    m_ni_diagnostic_packets=0; m_ni_diagnostic_until=time_precise()+12;
    m_ni_session_state = "discovering";
    LogDebug("[NI_SESSION] scanning A/M DAW endpoints");
    int input = -1, output = -1, inputCount = 0, outputCount = 0;
    std::string inputFamily, outputFamily;
    auto suitable = [](const char* name) {
        const std::string n(name);
        return n.find("Komplete Kontrol A DAW") != std::string::npos ||
               n.find("Komplete Kontrol M DAW") != std::string::npos;
    };
    for (int i = 0; i < GetNumMIDIInputs(); ++i) {
        char name[256] = {};
        const bool present = GetMIDIInputName(i, name, sizeof(name));
        if (present && suitable(name)) LogDebug(("[NI_ENDPOINT] input=" + std::to_string(i) + " present=" + std::to_string(present) + " name=" + name).c_str());
        if (present && suitable(name)) {
            input = i; ++inputCount;
            inputFamily = std::string(name).find("Kontrol A DAW") != std::string::npos ? "A" : "M";
        }
    }
    for (int i = 0; i < GetNumMIDIOutputs(); ++i) {
        char name[256] = {};
        const bool present = GetMIDIOutputName(i, name, sizeof(name));
        if (present && suitable(name)) LogDebug(("[NI_ENDPOINT] output=" + std::to_string(i) + " present=" + std::to_string(present) + " name=" + name).c_str());
        if (present && suitable(name)) {
            output = i; ++outputCount;
            outputFamily = std::string(name).find("Kontrol A DAW") != std::string::npos ? "A" : "M";
        }
    }
    if (inputCount != 1 || outputCount != 1 || inputFamily != outputFamily) {
        m_ni_session_state = "missing or ambiguous DAW endpoints";
        LogDebug(("[NI_SESSION] missing/ambiguous pair; input matches=" + std::to_string(inputCount) + " output matches=" + std::to_string(outputCount) + " families=" + inputFamily + "/" + outputFamily + "; no fallback").c_str());
        return;
    }
    m_midi_in_dev = input; m_midi_out_dev = output;
    LogDebug(("[NI_SESSION] opening input=" + std::to_string(input) + " output=" + std::to_string(output)).c_str());
    m_ni_input = CreateMIDIInput(input);
    if (!m_ni_input) { m_ni_session_state = "DAW input open failed"; LogDebug("[NI_SESSION] cannot open DAW input; ownership/availability not determined"); return; }
    m_midi_out = CreateMIDIOutput(output, false, nullptr);
    if (!m_midi_out) {
        delete m_ni_input; m_ni_input = nullptr;
        m_ni_session_state = "DAW output open failed";
        LogDebug("[NI_SESSION] cannot open DAW output; ownership/availability not determined"); return;
    }
    m_ni_input->start();
    m_ni_started = time_precise(); m_ni_timeout_announced = false;
    m_ni_version = 0;
    m_ni_session_state = "awaiting ACK";
    SendNiPacket(soundfirst_ni::hello());
    m_ni_recovery.sent(time_precise());
    LogDebug("[NI_SESSION] DAW pair opened; HELLO BF0104 attempt=1; awaiting ACK.");
}

void CSurf_SoundFirst::CloseNiSession() {
    if (m_midi_out) SendNiPacket(soundfirst_ni::goodbye());
    if (m_ni_input) { m_ni_input->stop(); delete m_ni_input; m_ni_input = nullptr; }
    if (m_midi_out) { delete m_midi_out; m_midi_out = nullptr; }
    m_ni_version = 0;
    m_ni_session_state = "closed";
    m_ni_recovery.reset();
    m_ni_focus_track = nullptr; m_ni_last_focus.clear();
    m_ni_sent_focus.clear(); m_ni_focus_sent=false; m_ni_focus_instrument=false;
    m_ni_last_scan_log.clear(); m_ni_last_unavailable_log.clear();
}

void CSurf_SoundFirst::ReconnectNi() {
    CloseNiSession(); OpenNiSession();
    m_display_cache.invalidate();
    SpeakText("Reconnecting NI DAW session");
}

void CSurf_SoundFirst::PumpNiInput() {
    if (!m_ni_input) return;
    m_ni_input->SwapBufs(timeGetTime());
    MIDI_eventlist* events = m_ni_input->GetReadBuf();
    int position = 0;
    while (MIDI_event_t* event = events->EnumItems(&position)) OnMIDIEvent(event);
    if (!m_ni_version && !m_ni_timeout_announced && time_precise() - m_ni_started > 3) {
        m_ni_timeout_announced = true;
        m_ni_session_state = "ACK timeout";
        LogDebug(("[NI_SESSION] ACK timeout after HELLO attempt="+std::to_string(m_ni_recovery.attempts())+
            " received="+std::to_string(m_ni_received)+" decodedCC="+std::to_string(m_ni_cc_received)+
            " rejected="+std::to_string(m_ni_rejected)+"; recovery requires Instrument+NI ID; max3 attempts/session").c_str());
        if (m_current_mode==MODE_INSTRUMENT && m_ni_recovery.attempts()>=3 &&
            !m_ni_final_timeout_spoken) {
            m_ni_final_timeout_spoken=true;
            SpeakText("NI DAW acknowledgement not confirmed after three attempts. Use reconnect.");
        }
    }
}

void CSurf_SoundFirst::RefreshNiFocus(bool force) {
    MediaTrack* track = GetSelectedTrack(nullptr, 0);
    std::string id;
    if (track) {
        for (int fx = 0; fx < TrackFX_GetCount(track) && id.empty(); ++fx) {
            const int count = TrackFX_GetNumParams(track, fx);
            for (int parameter : {0, 128, 2048, 4096}) {
                if (parameter >= count) continue;
                char name[128] = {};
                if (TrackFX_GetParamName(track, fx, parameter, name, sizeof(name)) &&
                    soundfirst_ni::validInstance(name)) { id = name; break; }
            }
        }
    }
    const bool instrument = m_current_mode == MODE_INSTRUMENT;
    const std::string requested = soundfirst_ni::selectedFocus(instrument,id);
    const std::string scan="[NI_FOCUS_SCAN] detected="+(id.empty()?std::string("NONE"):id)+
        " routing="+(instrument?"Instrument":"custom");
    if (scan!=m_ni_last_scan_log) { LogDebug(scan.c_str()); m_ni_last_scan_log=scan; }
    if (m_midi_out && (!m_ni_focus_sent || requested != m_ni_sent_focus ||
                      instrument != m_ni_focus_instrument)) {
        SendNiPacket(soundfirst_ni::focus(requested));
        m_ni_sent_focus=requested; m_ni_focus_sent=true;
        LogDebug(("[NI_FOCUS_SENT] "+(requested.empty()?std::string("EMPTY"):requested)+
                  " ACK="+std::to_string(m_ni_version)).c_str());
    } else if (!m_midi_out && m_ni_last_unavailable_log!=scan) {
        LogDebug("[NI_FOCUS] output unavailable; scan only"); m_ni_last_unavailable_log=scan;
    }
    m_ni_focus_track=track; m_ni_last_focus=id;
    m_ni_focus_instrument=instrument;

}

void CSurf_SoundFirst::MaybeRecoverNiHello() {
    const double now=time_precise();
    if (!m_ni_recovery.due(now,m_current_mode==MODE_INSTRUMENT,
                          !m_ni_last_focus.empty(),m_ni_version!=0,
                          m_ni_input && m_midi_out)) return;
    SendNiPacket(soundfirst_ni::hello());
    m_ni_recovery.sent(now);
    m_ni_started=now; m_ni_timeout_announced=false;
    m_ni_session_state="awaiting ACK (bounded retry)";
    m_ni_diagnostic_until=now+4; // packet budget is NOT reset by retry
    LogDebug(("[NI_HELLO_RETRY] BF0104 attempt="+std::to_string(m_ni_recovery.attempts())+
        "/3; Instrument detected="+m_ni_last_focus+"; ports retained").c_str());
}

void CSurf_SoundFirst::ReportNiConnect(bool force) {
    const bool input=m_ni_input!=nullptr, output=m_midi_out!=nullptr;
    const std::string phase=soundfirst_ni::connectionPhase(input && output,m_ni_version!=0,
        m_ni_timeout_announced,m_ni_recovery.attempts(),m_current_mode==MODE_INSTRUMENT,
        !m_ni_last_focus.empty());
    // RX counters belong in this compact snapshot, but do not generate a new
    // line for every packet. Automatic output changes only on connection state.
    const std::string state="state="+phase+" ACK="+std::to_string(m_ni_version)+
        " in="+std::to_string(input)+" out="+std::to_string(output)+
        " ID="+(m_ni_last_focus.empty()?std::string("NONE"):m_ni_last_focus)+
        " hello="+std::to_string(m_ni_recovery.attempts())+"/3";
    if (force || state!=m_ni_last_connect_log) {
        LogDebug(("[NI_CONNECT] "+state+" rx="+std::to_string(m_ni_received)+
            " cc="+std::to_string(m_ni_cc_received)+" rejected="+std::to_string(m_ni_rejected)).c_str());
        m_ni_last_connect_log=state;
    }
}

void CSurf_SoundFirst::SetSurfaceSelected(MediaTrack* track, bool selected) {
    if (!selected) return;
    UpdateBankFromSelectedTrack();
    RefreshNiFocus(true); m_ni_feedback_dirty = true;
}
void CSurf_SoundFirst::SetTrackListChange() {
    while (m_current_bank > 0 && !GetTrack(nullptr,m_current_bank*8)) --m_current_bank;
    m_ni_feedback_dirty = true; RefreshNiFocus(true);
}

void CSurf_SoundFirst::HandleNiCommand(unsigned char command, unsigned char value) {
    if (command == 1) {
        if (!value) { LogDebug("[NI_ACK] zero version ignored"); return; }
        m_ni_version = value;
        m_ni_final_timeout_spoken=false;
        m_ni_focus_sent=false; // Re-announce desired focus after negotiation.
        m_ni_session_state = "confirmed";
        if (m_current_mode != MODE_INSTRUMENT) m_display_cache.invalidate();
        LogDebug(("[NI_ACK] version=" + std::to_string(value)).c_str());
        SendNiPacket(soundfirst_ni::sysex(3,1,0,"track_orientation"));
        RefreshNiFocus(true); m_ni_feedback_dirty = true;
        return;
    }
    // No DAW command mutates REAPER in Instrument. Outside Instrument the
    // existing HID handlers own actions, so MIDI cannot duplicate them either.
    if (soundfirst_ni::allowDawAction(m_current_mode == MODE_INSTRUMENT,m_ni_version != 0,command)) {
        // The current routing policy deliberately never reaches this branch.
        return;
    }
    if (m_current_mode == MODE_INSTRUMENT && !m_native_notice) {
        m_native_notice=true;
        char diagnostic[96];
        snprintf(diagnostic,sizeof(diagnostic),"[NI_DAW_IGNORED] command=%02X value=%02X; no SoundFirst action",command,value);
        LogDebug(diagnostic);
    }
}


void CSurf_SoundFirst::AdoptHidBaseline(const unsigned char* data) {
    memcpy(m_last_hid_report,data,64);
    m_shift_pressed = (data[1] & 1) != 0;
    for (int i = 0; i < 8; ++i) m_last_knob_values[i] = data[6+i*2] | (data[7+i*2] << 8);
    m_hid_baseline = true;
}

void CSurf_SoundFirst::ReleaseTouchSolos() {
    // Restore only touch states owned by this surface, never an unrelated solo.
    for (int k = 0; k < 8; ++k) {
        if (!m_knob_touch_state[k]) continue;
        MediaTrack* t = m_touch_track[k];
        if (t && ValidatePtr2(nullptr,t,"MediaTrack*"))
            GetSetMediaTrackInfo(t,"I_SOLO",&m_previous_solo_state[k]);
        m_knob_touch_state[k] = false; m_touch_track[k] = nullptr;
    }
    if (m_current_mode == MODE_FX && m_touch_auto_solo) {
        for (int k = 0; k < 8; ++k) {
            const bool touched = k == 7 ? (m_last_hid_report[5]&1) != 0 :
                                          (m_last_hid_report[4]&(1<<(k+1))) != 0;
            if (touched) HandleFxTouch(k,false);
        }
    }
}

void CSurf_SoundFirst::ToggleInstrumentMode() {
    if (m_current_mode == MODE_INSTRUMENT) {
        ExitInstrumentMode();
        return;
    } else {
        PumpNiInput();
        ReleaseTouchSolos();
        m_display_cache.invalidate();
        m_current_mode = MODE_INSTRUMENT;
        m_browser_mode_selection = false;
        m_ni_feedback_dirty = true;
        RefreshNiFocus(true);
        MaybeRecoverNiHello();
        SpeakText("Instrument mode. Press Plugin, then Browser to control Komplete Kontrol. Press Browser twice to leave Instrument mode.");
        if (!m_ni_input || !m_midi_out)
            SpeakText(("NI DAW ports unavailable. " + m_ni_session_state + ". Use reconnect.").c_str());
        else if (m_ni_timeout_announced && m_ni_recovery.attempts()>=3)
            SpeakText("NI DAW acknowledgement not confirmed after three attempts. Use reconnect.");
        ReportNiConnect();
    }
    // Keep the last received baseline: otherwise the first exit-button press after
    // an action-triggered mode entry could be swallowed as initialization.
    m_track_press.reset(); m_mode_press.reset();
    m_undo_pressed_at=-1;
    m_native_notice = false;
    LogDebug((std::string("[NI_MODE] ") + (m_current_mode == MODE_INSTRUMENT ? "Instrument" : "Mixer") +
              " ACK=" + std::to_string(m_ni_version) + " detected=" +
              (m_ni_last_focus.empty() ? "NONE" : m_ni_last_focus)).c_str());
    if (m_current_mode == MODE_INSTRUMENT && m_ni_version) {
        if (m_ni_last_focus.empty()) SpeakText("No NI instance ID found on the selected track");
    }
    ResetIdleTimer();
}


void CSurf_SoundFirst::ReportNiState() {
    PumpNiInput();
    RefreshNiFocus(true);
    ReportNiConnect(true);
    const std::string status = std::string("NI DAW version ") + std::to_string(m_ni_version) +
        ". Session " + m_ni_session_state +
        ". " + (m_ni_last_focus.empty() ? "No instance ID found on selected track" : "Instance ID " + m_ni_last_focus) +
        ". NI instance routing " + (m_current_mode == MODE_INSTRUMENT ? "Instrument enabled" : "custom disabled") +
        ". Last transmitted focus " + (!m_ni_focus_sent ? "not sent; output unavailable" : m_ni_sent_focus.empty() ? "empty" : m_ni_sent_focus) +
        ". HELLO attempts " + std::to_string(m_ni_recovery.attempts()) +
        ". MIDI received " + std::to_string(m_ni_received) +
        ", decoded CC " + std::to_string(m_ni_cc_received) +
        ", rejected " + std::to_string(m_ni_rejected) +
        ". Mode " + std::string(m_current_mode == MODE_INSTRUMENT ? "Instrument" : "Mixer or custom") +
        ". Double mode selector exit (Browser bit01) " + (m_double_mode_exit ? "on" : "off") +
        ". Legacy double Track exit (bit04) " + (m_double_track_exit ? "on" : "off") +
        ". HID " + (m_hid_baseline ? "reports received" : "baseline not received");
    SpeakText("NI connection status written to the log."); LogDebug(("[NI_STATUS] " + status).c_str());
    MediaTrack* track = GetSelectedTrack(nullptr,0);
    if (!track) { LogDebug("[NI_SCAN] no selected track"); return; }
    for (int fx = 0; fx < TrackFX_GetCount(track); ++fx) {
        char name[256] = {};
        TrackFX_GetFXName(track,fx,name,sizeof(name));
        const int count = TrackFX_GetNumParams(track,fx);
        LogDebug(("[NI_SCAN] FX=" + std::to_string(fx) + " name=" + name + " params=" + std::to_string(count)).c_str());
        for (int param : {0,128,2048,4096}) {
            if (param >= count) continue;
            char parameter[128] = {};
            TrackFX_GetParamName(track,fx,param,parameter,sizeof(parameter));
            LogDebug(("[NI_SCAN] candidate=" + std::to_string(param) + " name=" + parameter).c_str());
        }
    }
}


void CSurf_SoundFirst::ExitInstrumentMode() {
    const bool wasInstrument = m_current_mode == MODE_INSTRUMENT;
    m_current_mode = MODE_MIXER;
    m_browser_mode_selection = false;
    RefreshNiFocus(true); // Withdraw only the NI instance link announced by this surface.
    m_track_press.reset(); m_mode_press.reset();
    m_undo_pressed_at=-1;
    m_native_notice = false;
    m_display_cache.invalidate();
    m_screensaver_active = false;
    LogDebug(wasInstrument ? "[NI_EXIT] Instrument -> Mixer; custom HID routing restored" :
                            "[NI_EXIT] already outside Instrument; Mixer selected");
    // This changes SoundFirst only. It does not revoke native NI ownership.
    ResetIdleTimer();
    UpdateDisplay(); // Restore complete Mixer state even when values match pre-entry.
    SpeakText("Mixer mode. To control Komplete Kontrol, press Shift and Browser, choose Instrument with the encoder and press it. Then press Plugin, followed by Browser.");
}


void CSurf_SoundFirst::CaptureInstrumentHid() {
    m_hid_capture_packets=0;
    m_hid_capture_until=GetTickCount64()/1000.0+60;
    LogDebug("[NI_CAPTURE] start 60 seconds; all HID reports including duplicates/short reports; no interpretation");
    SpeakText("HID capture started for sixty seconds. Press only the controls you want to test");
}
