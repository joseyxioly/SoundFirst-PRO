# SoundFirst PRO - Easy Mapping Guide
*Map any plugin in seconds, no coding required.*

## The Concept
SoundFirst PRO allows you to control any VST/AU/CLAP plugin with your Komplete Kontrol hardware.
To do this, we create simple `.ini` files that tell the driver: *"Knob 1 controls Cutoff"*.

We have built a special tool called **SoundFirst PRO Mapper** to make this instantaneous.

### Where to Find the Tool
*   **Executable (.exe):** Download the latest `SoundFirst_PRO_Mapper.exe` from the **[Releases Section](https://github.com/tu_usuario/tu_repo/releases)** of this repository.
*   **Source Code:** The full Python source code is available in the `tools/` folder of this project.

### Community & Support
Need help mapping a plugin? Or want to share your maps?
👉 **[Contact me on Telegram](https://t.me/JoseyXioly)**
👉 **[Support Development / Buy a Coffee](https://buymeacoffee.com/soundfirstpro)**

---

---

## Step 1: Getting the Parameters
First, we need to know what parameters your plugin has.

1.  Open Reaper and add the plugin you want to map (e.g., *Serum* or *Pro-Q3*) to a track.
2.  Make sure the plugin window is focused (click on it).
3.  Open the Action List (`?`) and run **"Script: dump_fx_params.lua"**.
    *   *Tip: Assign a shortcut like `Ctrl+Shift+D` to this script!*
4.  A message will confirm: **"Parameters copied to clipboard!"**.

---

## Step 2: The Mapper Tool
Now, open the **SoundFirst PRO Mapper** tool included in the download folder (`run_fx_mapper.cmd`).

1.  **Select Action:** Choose **"Create New Mapping"**.
2.  **Paste:** Click "Next". The tool will automatically read the clipboard from Step 1.
3.  **Wizard:** You are now in the graphical editor.

### How to Map
The interface is designed for speed:

*   **Left Column (Controls):** Select the Knob or Button on your hardware.
    *   *Example: Select "Knob 1".*
*   **Center Column (Parameters):** Search for the parameter you want.
    *   *Example: Type "Cutoff".*
*   **Assign:** Press **ENTER**.
    *   Done! Knob 1 is now linked to Cutoff.
    *   The selection automatically jumps to "Knob 2" so you can keep mapping without touching the mouse.

### Advanced Features
*   **Pages:** You can add multiple pages of controls (Page 1, Page 2...). Use the "Add Page" button.
*   **Custom Labels:** You can rename parameters to appear nicely on the screen (e.g., "FiltFreq" instead of "Filter Frequency A").
*   **Mode Specifics:**
    *   **Strip (Touch):** If you map a parameter to "Touch", touching the knob will trigger the Auto-Solo for that band.

---

## Step 3: Save and Play
1.  Click **"Save Mapping"**.
2.  The tool will suggest a filename (e.g., `Serum.ini`). **Do not change this name** unless necessary, as the driver needs it to recognize the plugin.
3.  Save it in your `UserPlugins\FX_Mappings` folder.
4.  **Restart Reaper** (or reload the driver).

Next time you open that plugin, SoundFirst PRO will automatically switch to **Plugin Mode** and your knobs will work instantly.

---

## Where do I put the files?
*   **Windows:** `%APPDATA%\REAPER\UserPlugins\FX_Mappings\`
*   **Mac:** `~/Library/Application Support/REAPER/UserPlugins/FX_Mappings/`

*Happy Mapping!*
