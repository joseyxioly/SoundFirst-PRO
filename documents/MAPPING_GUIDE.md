# SoundFirst PRO - Easy Mapping Guide
*Map any plugin in seconds, no coding required.*

---

## The Concept
SoundFirst PRO turns your Komplete Kontrol into a **universal controller for any VST/AU/CLAP plugin**.
We use simple `.ini` files that tell the driver: *"Knob 1 controls Cutoff"*.

The **SoundFirst PRO Mapper** tool makes creating these maps instant and visual.

---

## Where to Find the Tool
* **Executable (.exe):** Download the mapper from the **[Releases Section](https://github.com/joseyxioly/SoundFirst-PRO/releases)**.
* **Source Code:** The Python code is in the `tools/` folder of the project.

---

## Community & Support
Need help or want to share your maps?
📢 [Join the discussion group on Telegram](https://t.me/SoundFirstPRO)
☕ [Support development / Buy me a Coffee](https://buymeacoffee.com/soundfirstpro)

---

## Step 1: Get Plugin Parameters
1. Open REAPER and add your plugin to a track (e.g. *Serum* or *Pro-Q3*).
2. Click on the plugin window to focus it.
3. Open the **Action List** (`?`) and run **“Script: dump_fx_params.lua”**.
   *Tip: assign a shortcut, like `Ctrl+Shift+D`.*
4. Copy the result to the clipboard (`Ctrl+C`).

---

## Step 2: Open SoundFirst PRO Mapper
1. Run `SoundFirst_PRO_Mapper.exe`.
2. Click on **“Create New Mapping”**.
3. Paste the dump copied from Step 1 into the box and click **“Next”**.
4. You are now in the graphical editor.

### How to Map
The editor has 3 columns:

1.  **Left Column (Controls):** Select the knob or button on your hardware.
    *   *Optional:* Check **Shift** for the secondary layer or **Touch** for touch action (e.g. Auto-Solo).
2.  **Center Column (Parameters):** Search for the parameter you want to control.
    *   *Example: type “Cutoff” in the search box.*
3.  **Assign:** Press **“Assign (Enter)”** or just hit **ENTER**.
    *   *The selection automatically jumps to the next control for fast mapping.*

### Advanced Functions
- **Pages:** Use the right column to **Add Page** if you need more than 8 knobs.
- **Right Click** on a page to rename or reorder it.

---

## Step 3: Save and Use
1. Click on **“Save Mapping”**.
2. The tool will suggest a filename (e.g. `Serum.ini`). **Do not change it** unless necessary.
3. Save the file in the default folder (`.../UserPlugins/SoundFirst_Maps`).

> The next time you open that plugin, SoundFirst PRO will automatically switch to **Plugin Mode** and your knobs will work instantly.
> ⚠️ **Note:** You do not need to restart Reaper; the driver reads maps dynamically.

---

*Happy mapping and produce like a pro!*
