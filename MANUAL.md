# SoundFirst PRO V1.0 - User Manual 🎹🚀

This manual explains how to use the SoundFirst PRO controller efficiently in **Reaper 7.x**, focusing on the logic of the different modes and workflow.

> 🇪🇸 **Español:** ¿Prefieres leer esto en español? [Haz click aquí para ver el Manual en Español](MANUAL_ES.md)

---

## 📑 Table of Contents
- [Global Navigation](#global-navigation-joystick--browser)
- [Mixer Mode (TRACK Button)](#mixer-mode-track-button)
- [FX Mode (PLUGIN Button)](#fx-mode-plugin-button)
- [MIDI Mode (Browser -> MIDI)](#midi-mode-browser---midi)
- [Audio Mode (Browser -> AUDIO)](#audio-mode-browser---audio)
- [Other Global Buttons](#other-global-buttons)
- [Download & Support](#download--support)

---

## 🕹️ Global Navigation (Joystick & Browser)

### Browser Button
- Press **BROWSER** to switch between **MIDI** and **AUDIO** modes.
- Confirm selection by clicking the **Encoder**.

### Joystick (4D Encoder)
- Functions change slightly depending on the current mode but follow consistent logic:

| Action | Mixer Mode | FX Mode | MIDI / Audio Mode |
| :--- | :--- | :--- | :--- |
| Rotate | Move cursor (1 beat) | Move cursor | Move cursor |
| Rotate + SHIFT | Move loop | Navigate transients | Navigate transients |
| Up | Previous track | Previous track | Previous track |
| Down | Next track | Next track | Next track |
| Left | Previous marker | Previous plugin | Previous item |
| Right | Next marker | Next plugin | Next item |
| Click | Insert marker | Open/close FX window | Open MIDI editor |
| Click + SHIFT | - | Close floating windows | Close editor/windows |

---

## 🎛️ Mixer Mode (TRACK Button)
- Default mode for mixing and transport control.  
- Activated by pressing the **TRACK** button.

### Knobs
| Knob | Normal | SHIFT | Touch |
| :--- | :--- | :--- | :--- |
| 1-8 | Track volume | Pan | Auto-Solo (if enabled via IDEAS button) |

### Transport & Editing Buttons
| Button | Normal | SHIFT |
| :--- | :--- | :--- |
| PLAY | Play / Pause | Global Start (Go to start) |
| REC | Record | Cycle Automation (Touch/Latch/Read) |
| STOP | Stop | - |
| LOOP | Toggle loop | - |
| METRO | Toggle metronome | - |
| TEMPO | Tap tempo | - |
| UNDO | Undo | Redo |
| QUANTIZE | Reference Solo (Track 1) | - |
| MUTE | Mute selected track | - |
| SOLO | Solo selected track | - |

---

## 🎹 FX Mode (PLUGIN Button)
- Activated by pressing the **PLUGIN** button.  
- Allows deep control of effects on the selected track.  
> 📘 **Workflow Guide:** Learn the [Unified VST Workflow here](VST_WORKFLOW.md).

### Knobs
- Control mapped plugin parameters (native or Auto-Map).  
- **Touch**: If the plugin supports it (e.g., FabFilter, Waves), touching a knob triggers **Touch Solo** for that parameter.

### Parameter Navigation
| Button | Function |
| :--- | :--- |
| PRESET UP | Next page (8 parameters per bank) |
| PRESET DOWN | Previous page |
| MUTE | Bypass current plugin |
| IDEAS | Show gain reduction (dB) |

---

## 🎼 MIDI Mode (Browser -> MIDI)
- Edit notes and MIDI events with the MIDI editor open.

### Knobs
| Knob | Normal | SHIFT |
| :--- | :--- | :--- |
| 1 | Navigate chord | Maintain selection |
| 2 | Pitch (note up/down) | - |
| 3 | Move event (pixel) | Move event (grid) |
| 4 | Note length (pixel) | Note length (grid) |
| 5 | Transpose (semitone) | Octave |
| 6 | Velocity (±1) | Velocity (±10) |
| 7 | Grid size | - |
| 8 | Horizontal zoom | - |

### Editing Buttons
| Button | Action |
| :--- | :--- |
| QUANTIZE | Quantize events |
| LOOP | Cut |
| METRO | Copy |
| TEMPO | Paste |
| PRESET UP | Split |
| PRESET DOWN | Delete |

---

## ✂️ Audio Mode (Browser -> AUDIO)
- Edit audio items in the Arrange view.

### Knobs
| Knob | Function |
| :--- | :--- |
| 1 | Move cursor (fine) |
| 2 | Move item (nudge by grid) |
| 3 | Trim start |
| 4 | Trim end |
| 5 | Item volume |
| 6 | Fade In |
| 7 | Fade Out |
| 8 | Horizontal zoom |

### Editing Buttons
| Button | Normal | SHIFT |
| :--- | :--- | :--- |
| QUANTIZE | Quantize items | Reverse |
| LOOP | Cut | - |
| METRO | Copy | - |
| TEMPO | Paste | - |
| PRESET UP | Split item | - |
| PRESET DOWN | Delete / Remove | - |

---

## ⚙️ Other Global Buttons
- **SCALE:** Save project  
- **IDEAS:**  
  - Shift + IDEAS: Toggle meter between **PEAK** and **Gain Reduction (GR)**  
  - Normal: Toggle Global Auto-Solo

---

## 📥 Download & Support
- **Releases / Download v1.0:** [Click here](https://github.com/joseyxioly/SoundFirst-PRO/releases)  
- **Support the project:** [Buy Me a Coffee](https://www.buymeacoffee.com/soundfirstpro)

---

**© 2026 SoundFirst PRO**
