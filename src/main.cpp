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

extern "C" {

void RawLog(const char* txt) {
    char* appData = getenv("APPDATA");
    if (appData) {
        char path[512]; sprintf(path, "%s\\REAPER\\UserPlugins\\SoundFirst_Logs.txt", appData);
        FILE* f = fopen(path, "a");
        if (f) { fprintf(f, "[ENTRY] %s\n", txt); fclose(f); }
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
        
        return 1;
    } else {
        // UNLOAD
        if (g_surface) {
            delete g_surface;
            g_surface = nullptr;
        }
        return 0;
    }
}

} // extern C
