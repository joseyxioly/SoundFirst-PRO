# SoundFirst PRO - User Manual
*The intuitive guide to mastering your hardware.*

## What is this?
SoundFirst PRO is a **custom driver** that takes over your Komplete Kontrol keyboard and turns it into a deep production tool for Reaper.

Unlike standard MIDI mappings, this driver "talks" to Reaper directly. It knows which track is selected, which plugin is open, and instantly changes the behavior of every knob and button to match your current task.

---

## The Concept: 4 Modes
Think of your keyboard as having 4 distinct "personalities" or Modes. You switch between them depending on what you want to do.

1.  **Mixer Mode (Default):** For balancing levels, panning, and basic transport.
2.  **Plugin Mode (formerly FX Mode):** For tweaking plugins, synthesizers, and effects.
3.  **Audio Mode:** For editing waveforms, splicing, and fading audio items.
4.  **MIDI Mode:** For composing and editing notes in the MIDI editor.

Let's explore them one by one.

---

## 1. Mixer Mode (TRACK Button)
This is the home base. Press the **TRACK** button to enter this mode.
Use this mode when you are balancing your mix or recording.

### The Knobs
*   **Touch a Knob:** The system instantly **Solos** that track (if "Auto-Solo" is enabled). Perfect for checking a specific instrument in the mix without clicking anything.
*   **Turn Knobs 1-8:** Controls the **Volume** of the selected bank of 8 tracks.
*   **Hold SHIFT + Turn:** Controls the **Pan** (Left/Right) of the tracks.

### The Encoder (4D Joystick)
*   **Rotate:** Moves the playhead cursor.
*   **Up/Down:** Selects the Next/Previous track. *Note: As you move, the knobs automatically update to control the new bank of tracks.*
*   **Left/Right:** Jumps to the Previous/Next Marker.
*   **Click:** Inserts a new Marker at the current position.

### Vital Buttons
*   **PLAY / REC / STOP:** Work exactly as you expect.
*   **LOOP:** Toggles the loop (repeat) mode.
*   **METRO:** Turns the metronome click on/off.
*   **TEMPO:** Tap it rhythmically to set the project BPM.
*   **UNDO / REDO (Shift):** Fix mistakes instantly.
*   **MUTE / SOLO:** Mute or Solo the *currently selected* track.
*   **QUANTIZE:** Special function -> Solos the Reference Track (Track 1) for quick A/B comparison.

---

## 2. Plugin Mode (PLUGIN Button)
This is where the magic happens. Press the **PLUGIN** button to enter.
Use this mode when you want to tweak an EQ, Compressor, or Synth.

**What is Plugin Mode?**
This mode allows you to control any VST/AU/CLAP plugin that has been mapped. Mappings can be created by the developer (included), the community, or by yourself using our easy tools.

> **Important:** This mode focuses on the *currently selected track*.

### The Knobs
*   **Turn:** Controls a specific parameter of the open plugin (e.g., Cutoff, Threshold, Gain).
*   **Touch:** If the plugin supports it (like Pro-Q3), touching the knob can isolate (solo) that specific band!

### The Encoder (4D Joystick)
*   **Left/Right:** Navigates between different selected plugins in the chain (e.g., move from EQ to Compressor).
*   **Click:** Opens or Closes the floating window of the current plugin.

### Page Navigation (PRESET Buttons)
A plugin might have more than 8 parameters.
*   **PRESET UP:** Go to the Next Page of 8 parameters.
*   **PRESET DOWN:** Go to the Previous Page.

*Tip: Use our **SoundFirst PRO Mapper** tool to custom-map exactly which parameter goes to which knob.*
👉 **[Open the Easy Mapping Guide (MAPPING_GUIDE.md)](MAPPING_GUIDE.md)**

---

## 3. Audio Mode (Browser -> Audio)
Press **BROWSER** and select "AUDIO" (or toggle) to enter.
Use this mode when editing audio clips in the timeline (Arrange view).

### The Knobs
When in Audio Mode, the knobs become precision editing tools for the *selected audio item*:
1.  **Knob 1:** Move Cursor (Fine precision).
2.  **Knob 2:** Nudge Item (Move the clip slightly).
3.  **Knob 3:** Trim Start (Cut the beginning).
4.  **Knob 4:** Trim End (Cut the end).
5.  **Knob 5:** Item Volume / Gain.
6.  **Knob 6:** Fade In length.
7.  **Knob 7:** Fade Out length.
8.  **Knob 8:** Horizontal Zoom.

### Editing Buttons
*   **LOOP:** Cuts (Splits) the item at the cursor.
*   **QUANTIZE:** -
*   **Shift + QUANTIZE:** Reverses the audio item.

---

## 4. MIDI Mode (Browser -> MIDI)
Press **BROWSER** and select "MIDI" to enter.
Use this mode when the MIDI Editor (Piano Roll) is open.

### The Knobs
1.  **Knob 1:** Navigate between notes in a chord.
2.  **Knob 2:** Pitch (Move note Up/Down).
3.  **Knob 3:** Move Note (Left/Right in time).
4.  **Knob 4:** Note Length (Shorten/Lengthen).
5.  **Knob 5:** Transpose (Semitones). *Shift: Octaves.*
6.  **Knob 6:** Velocity (Loudness).
7.  **Knob 7:** Grid Size.
8.  **Knob 8:** Zoom.

---

## Global Tools
These work in all modes.

*   **SCALE:** Saves the Project (Ctrl+S).
*   **ARP:** Spoken Feedback -> Reads current Track Peak Level (dB).
*   **IDEAS:** Spoken Feedback -> Reads Gain Reduction (GR) on supported compressor mappings.

### Getting Support
Need help? Verify you have installed the required SWS Extensions and OSARA.
Check the [Release Page](https://github.com/joseyxioly/SoundFirst-PRO/releases) for updates.

👉 **[Contact me on Telegram](https://t.me/JoseyXioly)**
👉 **[Support Development / Buy a Coffee](https://buymeacoffee.com/soundfirstpro)**

*Power to the Creators.*
