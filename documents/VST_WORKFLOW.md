# SoundFirst PRO - Unified VST Workflow 🎛️🧠

## The Philosophy: "Muscle Memory Mixing"
SoundFirst PRO is not just a driver; it is a **workflow**. 
We believe that to mix professionally, you shouldn't have to look at the screen to find the "Threshold" knob every time you open a new compressor.

**The Solution:** A Unified Workflow.
*   **Knob 1 (K1)** is *always* the **Driver** (Threshold, Input Gain, or primary Boost).
*   **Knob 2 (K2)** is *always* the **Level** (Makeup, Output, or Volume).
*   **Shift + Knob** accesses secondary but related parameters.
*   **Touch Knob 🖐️** often solos the parameter or band (e.g. in Pro-Q3).

This means you can close your eyes, load *any* supported compressor, and reach for Knob 1 to compress and Knob 2 to level match. Your hands learn the mixing, not your eyes.

> **Note:** This is a professional starting point. You can customize ANY mapping using our included tools (`tools/` folder) to fit your personal needs. But we encourage you to try this unified approach—it changes everything.

---

## Included Mappings (Native)

### 1️⃣ Channel Strips
*Emulations of classic consoles. The layout is standardized across brands.*

**Lindell 50 / SSL EV2 / SSL 9000 J**
*   **Page 1 (EQ):**
    *   **K1-K4:** Low, Low-Mid, High-Mid, High Gain.
    *   **Shift + K1-K4:** Frequencies for the above bands.
    *   **K5-K6:** Q (Bandwidth).
    *   **K7-K8:** Types (Bell/Shelf) or Filters.
*   **Page 2 (Dynamics):**
    *   **K1:** Threshold (The Driver).
    *   **K2:** Output / Makeup (The Level).
    *   **K3-K5:** Attack, Release, Ratio.
    *   **K6-K7:** Gate Controls.

### 2️⃣ Compressors
*The "Knob 1 = Threshold, Knob 2 = Makeup" rule applies everywhere.*

**CLA-76 (1176 Style)**
*   **K1:** Input (Driver). *(Shift: Auto-Makeup)*
*   **K2:** Output (Level). *(Shift: Trim)*
*   **K3-K4:** Attack, Release.
*   **K5:** Ratio. *(Shift: Revision/Color)*
*   **K6:** Mix (Parallel).
*   **K7:** Analog Noise.

**CLA-2A (LA-2A Style)**
*   **K1:** Peak Reduction (Driver).
*   **K2:** Gain (Level).
*   **K3:** Hi-Freq (Sidechain).
*   **K4:** Compress/Limit Mode.

**SSL Bus Comp / API-2500 / Bettermaker / L3 Limiter**
*   **K1:** Threshold.
*   **K2:** Makeup / Ceiling.
*   **K3-K5:** Attack, Release, Ratio.
*   **K7-K8:** Special features (Thrust, Knee, THD, Profile).

**RVox (Waves)**
*   **K1:** Compression.
*   **K2:** Gain.
*   **K3:** Gate.

### 3️⃣ Equalizers
*From surgical digital EQs to vintage passive units.*

**FabFilter Pro-Q 3**
*   **Page 1:** "Bands Use" (Overview).
*   **Page 2:** Bands 1-4.
*   **Page 3:** Bands 5-8.
    *   **Knob Pair (e.g., K1+K2):** Gain + Frequency.
    *   **Shift:** Q + Type.
    *   **Touch:** Solo Band (Listen).

**API-560 (Graphic EQ)**
*   **K1-K8:** Control 8 fixed frequency bands per page.
    *   **Shift:** Access intermediate frequencies or Output Gain.

**PuigTec EQP1A (Passive)**
*   **K1:** Low Boost.
*   **K2:** Low Atten.
*   **K3:** Low Freq.
*   **K5:** Hi Boost.
*   **K6:** Hi Freq.

**bx_digital V3 (M/S Mastering EQ)**
*   **Page 1:** Mid (Mono) Channel.
*   **Page 2:** Side Channel.
*   **K6 (Page 1):** Mono-Maker.
*   **K6 (Page 2):** Stereo Width.

### 4️⃣ Creative & FX
**H-Delay**
*   **K1:** Delay Time. *(Shift: Sync/BPM)*
*   **K2:** Feedback.
*   **K3-K4:** HP/LP Filters.
*   **K5:** Mix.
*   **K7:** Ping-Pong On/Off.

**H-Reverb**
*   **K1:** Time (Decay).
*   **K2:** Size.
*   **K3:** Pre-Delay.
*   **K5:** Dry/Wet.

**Black Box HG-2**
*   **K1:** Saturation (Pentode).
*   **K2:** Triode.
*   **K5:** Input.
*   **K6:** Output.

---

## Customizing Your Workflow
This list is just the beginning. 
Using the **SoundFirst PRO Mapper** included in the `tools/` folder, you can:
1.  Map *any* other plugin you own.
2.  Override these defaults if you prefer a different layout.
3.  Add "Smart Actions" (like `@REPORT_GR`) to your own mappings.

**Join the Community:** We are building a standard. Share your `.ini` maps with other users to expand this library!

👉 **[Contact me on Telegram](https://t.me/JoseyXioly)**
👉 **[Support Development / Buy a Coffee](https://buymeacoffee.com/soundfirstpro)**
