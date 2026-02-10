#pragma once
#include <string>
#include <vector>
#include <map>

struct FXKnobMap {
    int param_id = -1;       // The parameter to control
    int shift_param_id = -1; // Parameter when Shift is held
    int touch_param_id = -1; // Parameter to toggle on Touch (e.g. Solo)
    bool is_toggle = false;  // If true, treats as switch
};

struct FXPage {
    std::string name;
    FXKnobMap knobs[8]; // 8 Knobs
};

struct FXMapping {
    std::string plugin_name_signature; // e.g. "Lindell 50"
    std::vector<FXPage> pages;
};

// Global Registry
class FXMappingRegistry {
public:
    static FXMapping* GetMapping(const char* fx_name) {
        static std::vector<FXMapping> mappings;
        if (mappings.empty()) InitializeMappings(mappings);
        
        std::string name = fx_name;
        for (auto& m : mappings) {
            if (name.find(m.plugin_name_signature) != std::string::npos) {
                return &m;
            }
        }
        return nullptr;
    }

private:
    static void InitializeMappings(std::vector<FXMapping>& m) {
        
        // =========================================================
        // 1. CONSOLE STRIPS (SSL, Lindell, API)
        // Std: P1=EQ, P2=DYN, P3=FILTERS/GLOBAL
        // =========================================================
        
        // --- LINDELL 50 ---
        {
            FXMapping map; map.plugin_name_signature = "Lindell 50";
            FXPage p1; p1.name = "EQ Main"; // Fixed Freqs
            p1.knobs[0] = { 26, 24 }; // 125Hz (S: 31Hz)
            p1.knobs[1] = { 27, 25 }; // 250Hz (S: 63Hz)
            p1.knobs[2] = { 28 };     // 500Hz
            p1.knobs[3] = { 29 };     // 1kHz
            p1.knobs[4] = { 30 };     // 2kHz
            p1.knobs[5] = { 31 };     // 4kHz
            p1.knobs[6] = { 32 };     // 8kHz
            p1.knobs[7] = { 33 };     // 16kHz
            map.pages.push_back(p1);
            
            FXPage p2; p2.name = "Dynamics"; // VCA (Main)
            p2.knobs[0] = { 46 }; // Threshold (K1)
            p2.knobs[1] = { 54 }; // Makeup (K2)
            p2.knobs[2] = { 47 }; // Attack
            p2.knobs[3] = { 48 }; // Release
            p2.knobs[4] = { 49 }; // Ratio
            p2.knobs[5] = { 56 }; // Gate Thresh
            p2.knobs[6] = { 57 }; // Gate Range
            p2.knobs[7] = { 0 };  // Pre Gain
            map.pages.push_back(p2);
            m.push_back(map);
        }

        // --- SSL EV2 (Waves) ---
        {
            FXMapping map; map.plugin_name_signature = "SSL EV2";
            FXPage p1; p1.name = "EQ"; 
            // LF, LMF, HMF, HF
            p1.knobs[0] = { 16, 17 }; // LF Gain (S: Freq)
            p1.knobs[1] = { 18, 19 }; // LMF Gain (S: Freq)
            p1.knobs[2] = { 21, 22 }; // HMF Gain (S: Freq)
            p1.knobs[3] = { 26, 27 }; // HF Gain (S: Freq)
            p1.knobs[4] = { 20, 15 }; // LMF Q (S: LF Bell)
            p1.knobs[5] = { 23, 25 }; // HMF Q (S: HF Bell)
            p1.knobs[6] = { 13 };     // EQ Type (Brown/Black)
            p1.knobs[7] = { 12 };     // EQ Bypass
            map.pages.push_back(p1);
            
            FXPage p2; p2.name = "Dynamics";
            p2.knobs[0] = { 30 }; // Comp Thresh (K1)
            p2.knobs[1] = { 39 }; // Output/Makeup (K2)
            p2.knobs[2] = { 32 }; // Attack
            p2.knobs[3] = { 33 }; // Release
            p2.knobs[4] = { 31 }; // Ratio
            p2.knobs[5] = { 35 }; // Gate Thresh
            p2.knobs[6] = { 36 }; // Gate Range
            p2.knobs[7] = { 34 }; // Gate/Exp Mode
            map.pages.push_back(p2);
            
            FXPage p3; p3.name = "Global";
            p3.knobs[0] = { 2 };  // Line Input
            p3.knobs[1] = { 3 };  // Mic Input
            p3.knobs[2] = { 7 };  // HPF
            p3.knobs[3] = { 8 };  // LPF
            p3.knobs[4] = { 40 }; // Width
            p3.knobs[5] = { 4 };  // Analog
            p3.knobs[6] = { 6 };  // Phase
            p3.knobs[7] = { 11 }; // Split
            map.pages.push_back(p3);
            m.push_back(map);
        }

        // --- BX_CONSOLE SSL 9000 J ---
        {
            FXMapping map; map.plugin_name_signature = "SSL 9000 J";
            FXPage p1; p1.name = "EQ";
            p1.knobs[0] = { 47, 48 }; // Low Gain (S: Freq)
            p1.knobs[1] = { 43, 44 }; // Low Mid Gain (S: Freq)
            p1.knobs[2] = { 39, 40 }; // High Mid Gain (S: Freq)
            p1.knobs[3] = { 36, 37 }; // High Gain (S: Freq)
            p1.knobs[4] = { 45, 49 }; // LM Q (S: Low Bell)
            p1.knobs[5] = { 41, 38 }; // HM Q (S: High Bell)
            p1.knobs[6] = { 51 };     // Type (E/G)
            p1.knobs[7] = { 50 };     // EQ On/Off
            map.pages.push_back(p1);

            FXPage p2; p2.name = "Dynamics";
            p2.knobs[0] = { 21 }; // Comp Thresh (K1)
            p2.knobs[1] = { 7 };  // Out Gain (K2)
            p2.knobs[2] = { 23 }; // Attack
            p2.knobs[3] = { 22 }; // Release
            p2.knobs[4] = { 20 }; // Ratio
            p2.knobs[5] = { 31 }; // Gate Thresh
            p2.knobs[6] = { 30 }; // Gate Range
            p2.knobs[7] = { 25 }; // Mix
            map.pages.push_back(p2);
            
            FXPage p3; p3.name = "Filters/Global";
            p3.knobs[0] = { 15, 14 }; // HPF Freq (S: On)
            p3.knobs[1] = { 18, 17 }; // LPF Freq (S: On)
            p3.knobs[2] = { 3 };      // In Gain
            p3.knobs[3] = { 4 };      // Virtual Gain (TMT)
            p3.knobs[4] = { 5 };      // THD
            m.push_back(map);
        }

        // =========================================================
        // 2. COMPRESSORS (1176, LA2A, SSL Bus, API 2500)
        // Std: P1 = Main Controls. P2 = Advanced.
        // =========================================================

        // --- CLA-76 (Moved to bottom) ---

        // --- CLA-2A (LA-2A Opto) ---
        {
            FXMapping map; map.plugin_name_signature = "CLA-2A";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 3, 10 }; // Peak Red (S: Auto Makeup)
            p1.knobs[1] = { 2 };     // Gain
            p1.knobs[2] = { 5 };     // Hi Freq (Sidechain)
            p1.knobs[3] = { 4 };     // Comp/Limit
            p1.knobs[4] = { 8 };     // Mix
            p1.knobs[5] = { 6 };     // Analog
            p1.knobs[6] = { 9 };     // Trim
            p1.knobs[7] = { 7 };     // Meter
            map.pages.push_back(p1);
            m.push_back(map);
        }

        // --- SSL COMP (Bus Compressor) ---
        {
            FXMapping map; map.plugin_name_signature = "SSLComp";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 2 };     // Thresh (K1)
            p1.knobs[1] = { 3, 10 }; // Makeup (K2)
            p1.knobs[2] = { 4 };     // Attack
            p1.knobs[3] = { 5 };     // Release
            p1.knobs[4] = { 6 };     // Ratio
            p1.knobs[5] = { 11 };    // Mix
            p1.knobs[6] = { 7 };     // Rate-S (Fade Rate)
            p1.knobs[7] = { 9 };     // Analog
            map.pages.push_back(p1);
            m.push_back(map);
        }

        // --- API-2500 ---
        {
            FXMapping map; map.plugin_name_signature = "API-2500";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 2 };     // Thresh (K1)
            p1.knobs[1] = { 15 };    // Makeup (K2)
            p1.knobs[2] = { 3 };     // Attack
            p1.knobs[3] = { 5 };     // Release
            p1.knobs[4] = { 4 };     // Ratio
            p1.knobs[5] = { 16 };    // Mix
            p1.knobs[6] = { 7, 8 };  // Thrust (S: Type New/Old)
            p1.knobs[7] = { 6, 9 };  // Knee (S: Link)
            map.pages.push_back(p1);
            m.push_back(map);
        }

        // --- BETTERMAKER BUS COMP ---
        {
            FXMapping map; map.plugin_name_signature = "Bettermaker";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 2 };     // Threshold
            p1.knobs[1] = { 8 };     // Output (K2)
            p1.knobs[2] = { 4 };     // Attack
            p1.knobs[3] = { 5 };     // Release
            p1.knobs[4] = { 3 };     // Ratio
            p1.knobs[5] = { 7 };     // Mix
            p1.knobs[6] = { 10, 9 }; // THD Amount (S: THD Enable)
            p1.knobs[7] = { 13, 12 };// SC HPF Freq (S: Engage)
            map.pages.push_back(p1);
            m.push_back(map);
        }
        
        // --- RVOX ---
        {
            FXMapping map; map.plugin_name_signature = "RVox";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 2 }; // Comp (K1)
            p1.knobs[1] = { 4 }; // Gain (K2)
            p1.knobs[2] = { 3 }; // Gate
            map.pages.push_back(p1);
            m.push_back(map);
        }

        // =========================================================
        // 3. SPECIAL EQS (Parametric, Graphic, Passive)
        // =========================================================

        // --- WAVES F6 (Parametric Std) ---
        {
            FXMapping map; map.plugin_name_signature = "F6 Stereo";
            FXPage p1; p1.name = "Filters";
            p1.knobs[0] = { 5, 3 };  // HPF Freq
            p1.knobs[1] = { 4 };     // HPF Q
            p1.knobs[2] = { 10, 8 }; // LPF Freq
            p1.knobs[3] = { 9 };     // LPF Q
            p1.knobs[4] = { 2 };     // HPF On
            p1.knobs[5] = { 7 };     // LPF On
            map.pages.push_back(p1);
            
            FXPage p2; p2.name = "Bands 1-4";
            p2.knobs[0] = { 17, 16 };     // Gain/Q (NO SOLO)
            p2.knobs[1] = { 15, 14, 24 }; // Freq/Type/Solo1
            p2.knobs[2] = { 30, 29 };     // Gain B2 (NO SOLO)
            p2.knobs[3] = { 28, 27, 37 }; // Freq B2/Solo2
            p2.knobs[4] = { 43, 42 };     // Gain B3 (NO SOLO)
            p2.knobs[5] = { 41, 40, 50 }; // Freq B3/Solo3
            p2.knobs[6] = { 56, 55 };     // Gain B4 (NO SOLO)
            p2.knobs[7] = { 54, 53, 63 }; // Freq B4/Solo4
            map.pages.push_back(p2);
            m.push_back(map);
        }

        // --- PRO-Q 3 (Custom) ---
        {
            FXMapping map; map.plugin_name_signature = "Pro-Q";
            FXPage p1; p1.name = "Bands Use";
            for(int i=0;i<8;i++) p1.knobs[i] = { i*23 }; // Used
            map.pages.push_back(p1);
            
            FXPage p2; p2.name = "Bands 1-4";
            p2.knobs[0]={3,4};    p2.knobs[1]={2,5,22};   // B1 (Gain No Solo)
            p2.knobs[2]={26,27};  p2.knobs[3]={25,28,45}; // B2
            p2.knobs[4]={49,50};  p2.knobs[5]={48,51,68}; // B3
            p2.knobs[6]={72,73};  p2.knobs[7]={71,74,91}; // B4
            map.pages.push_back(p2);

            FXPage p3; p3.name = "Bands 5-8";
            // Stride = +23 per Band
            // B5 (Start~94): Gain(95), Q(96), Freq(94), Type(97), Solo(114)
            p3.knobs[0]={95,96};   p3.knobs[1]={94,97,114};   // B5
            // B6 (Start~117): Gain(118), Q(119), Freq(117), Type(120), Solo(137)
            p3.knobs[2]={118,119}; p3.knobs[3]={117,120,137}; // B6
            // B7 (Start~140): Gain(141), Q(142), Freq(140), Type(143), Solo(160)
            p3.knobs[4]={141,142}; p3.knobs[5]={140,143,160}; // B7
            // B8 (Start~163): Gain(164), Q(165), Freq(163), Type(166), Solo(183)
            p3.knobs[6]={164,165}; p3.knobs[7]={163,166,183}; // B8
            map.pages.push_back(p3);
            m.push_back(map);
        }

        // --- API-560 (Graphic EQ) ---
        // Layout EXACTLY like Lindell 50:
        // K1=125Hz (S:31Hz), K2=250Hz (S:63Hz), K3=500, K4=1k, K5=2k, K6=4k, K7=8k, K8=16k
        {
            FXMapping map; map.plugin_name_signature = "API-560";
            FXPage p1; p1.name = "EQ Bands";
            p1.knobs[0] = { 11, 13 }; // 125Hz (S: 31Hz)
            p1.knobs[1] = { 10, 12 }; // 250Hz (S: 63Hz)
            p1.knobs[2] = { 9, 15 };  // 500Hz (S: Analog)
            p1.knobs[3] = { 8, 2 };   // 1kHz (S: Output)
            p1.knobs[4] = { 7 };      // 2kHz
            p1.knobs[5] = { 6 };      // 4kHz
            p1.knobs[6] = { 5 };      // 8kHz
            p1.knobs[7] = { 4 };      // 16kHz
            map.pages.push_back(p1);
            m.push_back(map);
        }

        // --- CLA-76 (1176) ---
        // VST3 IDs verified by USER DUMP:
        // Philosophy: K1=Input, K2=Output
        {
            FXMapping map; map.plugin_name_signature = "CLA-76";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 2, 13 }; // Input (S: Auto Makeup)
            p1.knobs[1] = { 3, 12 }; // Output (S: Trim)
            p1.knobs[2] = { 4 };     // Attack
            p1.knobs[3] = { 5 };     // Release
            p1.knobs[4] = { 6, 8 };  // Ratio (S: Revision)
            p1.knobs[5] = { 11 };    // Mix
            p1.knobs[6] = { 7 };     // Analog
            p1.knobs[7] = { 9 };     // Meter
            map.pages.push_back(p1);
            m.push_back(map);
        }

        // --- PUIGTEC EQP1A (Passive) ---
        {
            FXMapping map; map.plugin_name_signature = "PuigTec";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 2 }; // Low Boost
            p1.knobs[1] = { 3 }; // Low Atten
            p1.knobs[2] = { 4 }; // Low Freq
            p1.knobs[3] = { 6 }; // Bandwidth
            p1.knobs[4] = { 5 }; // Hi Boost
            p1.knobs[5] = { 7 }; // Hi Freq
            p1.knobs[6] = { 8 }; // Hi Atten
            p1.knobs[7] = { 9 }; // Atten Sel
            map.pages.push_back(p1);
            m.push_back(map);
        }

        // --- BX_DIGITAL V3 (M/S EQ) ---
        {
            FXMapping map; map.plugin_name_signature = "bx_digital V3";
            FXPage p1; p1.name = "Mono/Mid";
            p1.knobs[0] = { 32, 34 }; // Gain/Freq LF1
            p1.knobs[1] = { 37, 39 }; // Gain/Freq LMF1
            p1.knobs[2] = { 42, 44 }; // Gain/Freq MF1
            p1.knobs[3] = { 47, 49 }; // Gain/Freq HMF1
            p1.knobs[4] = { 52, 54 }; // Gain/Freq HF1
            p1.knobs[5] = { 2 };      // Input Gain
            p1.knobs[6] = { 17 };     // Mono Maker
            p1.knobs[7] = { 19 };     // Solo 1 (Mid)
            map.pages.push_back(p1);
            
            FXPage p2; p2.name = "Side";
            p2.knobs[0] = { 57, 59 }; // Gain/Freq LF2
            p2.knobs[1] = { 62, 64 };
            p2.knobs[2] = { 67, 69 };
            p2.knobs[3] = { 72, 74 };
            p2.knobs[4] = { 77, 79 };
            p2.knobs[5] = { 4 };      // Stereo Width
            p2.knobs[6] = { 20 };     // Solo 2 (Side)
            map.pages.push_back(p2);
            m.push_back(map);
        }
        
        // =========================================================
        // 4. OTHER / CREATIVE
        // =========================================================

        // --- BLACK BOX HG-2 ---
        {
            FXMapping map; map.plugin_name_signature = "Black Box";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 4, 3 };  // Saturation (S: In)
            p1.knobs[1] = { 1, 2 };  // Pentode / Triode
            p1.knobs[2] = { 5 };     // Sat Freq
            p1.knobs[3] = { 8, 7 };  // Air (S: In)
            p1.knobs[4] = { 11 };    // Input
            p1.knobs[5] = { 12 };    // Output
            p1.knobs[6] = { 13 };    // Mix
            p1.knobs[7] = { 6 };     // Alt Tube
            map.pages.push_back(p1);
            m.push_back(map);
        }

        // --- H-DELAY ---
        {
            FXMapping map; map.plugin_name_signature = "H-Delay";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 2, 3 }; // Delay Time (S: BPM)
            p1.knobs[1] = { 4 };    // Feedback
            p1.knobs[2] = { 13 };   // HP
            p1.knobs[3] = { 14 };   // LP
            p1.knobs[4] = { 7 };    // Mix
            p1.knobs[5] = { 8 };    // Output
            p1.knobs[6] = { 5, 18 };// PingPong (S: Dir)
            p1.knobs[7] = { 10 };   // LoFi
            map.pages.push_back(p1);
            m.push_back(map);
        }

         // --- H-REVERB ---
        {
            FXMapping map; map.plugin_name_signature = "H-Reverb";
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 7 };  // Time
            p1.knobs[1] = { 6 };  // Size
            p1.knobs[2] = { 2 };  // PreDelay
            p1.knobs[3] = { 48 }; // Damping HF
            p1.knobs[4] = { 10 }; // Dry/Wet
            p1.knobs[5] = { 11 }; // Output
            p1.knobs[6] = { 8 };  // ER Model
            p1.knobs[7] = { 65 }; // Tempo
            map.pages.push_back(p1);
            m.push_back(map);
        }
        
        // --- L3 LIMITER (Ultra / LL) ---
        {
            FXMapping map; map.plugin_name_signature = "L3"; // Matches L3-LL and L3 Ultra
            FXPage p1; p1.name = "Main";
            p1.knobs[0] = { 4 /*Ultra*/ }; // Threshold usually ~4. In LL it is 7?
            // "L3 Ultra": Thresh=4, Ceiling=3
            // "L3-LL": Thresh=7, Ceiling=6
            // Hard to map both if Params diverge. L3 Ultra is most common.
            // Let's assume L3 Ultra or map individually if needed.
            // Using logic for L3 Ultra:
            p1.knobs[0] = { 4 }; // Threshold
            p1.knobs[1] = { 3 }; // Ceiling
            p1.knobs[2] = { 2 }; // Release
            p1.knobs[3] = { 8 }; // Profile
            p1.knobs[4] = { 5 }; // Quantize
            map.pages.push_back(p1);
            m.push_back(map);
        }
    }
};
