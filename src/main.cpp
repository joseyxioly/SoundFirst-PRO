// Windows and Standard includes
#include <windows.h>
#include <stdio.h>

// IMPLEMENT REAPER API HERE - must be before including csurf header
#define REAPERAPI_IMPLEMENT

#include "csurf_soundfirst.h"

// Global surface pointer (reaKontrol pattern)
IReaperControlSurface* g_surface = nullptr;
reaper_plugin_info_t* g_rec = nullptr; 
void* (*g_GetFunc)(const char* name) = nullptr; // Global pointer (1-arg version)

extern "C" void RawLog(const char*);
static int instrumentAction = 0;
static int reconnectAction = 0;
static int diagnosticAction = 0;
static int exitAction = 0;
static int captureAction = 0;
static bool SoundFirstCommand(KbdSectionInfo*, int command, int, int, int, HWND) {
    auto* surface = static_cast<CSurf_SoundFirst*>(g_surface);
    if (!surface) return false;
    if (command == instrumentAction) { RawLog("[NI_ACTION] Toggle received by REAPER"); surface->ToggleInstrumentMode(); return true; }
    if (command == captureAction) { surface->CaptureInstrumentHid(); return true; }
    if (command == exitAction) { RawLog("[NI_ACTION] Exit received by REAPER"); surface->ExitInstrumentMode(); return true; }
    if (command == diagnosticAction) { surface->ReportNiState(); return true; }
    if (command == reconnectAction) { surface->ReconnectNi(); return true; }
    return false;
}

extern "C" {

void RawLog(const char* txt) {
    char* appData = getenv("APPDATA");
    if (appData) {
        char path[512]; sprintf(path, "%s\\REAPER\\UserPlugins\\SoundFirst_Logs.txt", appData);
        FILE* f = fopen(path, "a");
        if (f) { fprintf(f, "%s\n", txt); fclose(f); }
    }
}

REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(
    REAPER_PLUGIN_HINSTANCE hInstance,
    reaper_plugin_info_t *rec)
{
    RawLog("DLL Entry Point hit.");
    g_rec = rec;
    
    if (rec) {
        g_GetFunc = rec->GetFunc; // Store GetFunc globally
        // LOAD
        if (rec->caller_version != REAPER_PLUGIN_VERSION || !g_GetFunc) {
            return 0;
        }
        
        // Load API
        int err = REAPERAPI_LoadAPI(rec->GetFunc);
        if (err != 0) {
            return 0;
        }

        // Create surface directly (reaKontrol pattern - NOT a factory)
        g_surface = new CSurf_SoundFirst(0, 0, 0); // Default ports for now
        
        // Register the surface instance directly
        rec->Register("csurf_inst", (void*)g_surface);
        custom_action_register_t toggle = {0, "SOUNDFIRST_INSTRUMENT", "SoundFirst: Toggle Instrument mode"};
        custom_action_register_t reconnect = {0, "SOUNDFIRST_NI_RECONNECT", "SoundFirst: Reconnect NI DAW"};
        instrumentAction = rec->Register("custom_action", &toggle);
        reconnectAction = rec->Register("custom_action", &reconnect);
        custom_action_register_t diagnostic = {0,"SOUNDFIRST_NI_STATUS","SoundFirst: Report NI connection and focus"};
        diagnosticAction = rec->Register("custom_action", &diagnostic);
        custom_action_register_t exit = {0,"SOUNDFIRST_EXIT_INSTRUMENT","SoundFirst: Exit Instrument mode"};
        exitAction = rec->Register("custom_action", &exit);
        custom_action_register_t capture = {0,"SOUNDFIRST_CAPTURE_NI_HID","SoundFirst: Capture Instrument HID for 60 seconds"};
        captureAction = rec->Register("custom_action", &capture);
        rec->Register("hookcommand2", (void*)SoundFirstCommand);
        
        return 1;
    } else {
        // UNLOAD
        if (g_surface) {
            // REAPER no longer schedules callbacks after unregistering the instance.
            // g_rec is null on unload, so use the API entry captured at load.
            plugin_register("-csurf_inst", (void*)g_surface);
            plugin_register("-hookcommand2", (void*)SoundFirstCommand);
            delete g_surface;
            g_surface = nullptr;
        }
        return 0;
    }
}

} // extern C
