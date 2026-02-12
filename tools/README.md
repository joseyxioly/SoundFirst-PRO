# FX Mapper - Standalone Executable Guide

## Building the Executable

To create `FX_Mapper.exe`, simply run:

```cmd
build_mapper.bat
```

**What it does:**
1. Checks if PyInstaller is installed (installs it if missing)
2. Compiles `fx_mapper_gen.py` into a standalone `.exe`
3. Creates `dist\FX_Mapper.exe` (ready to distribute)

**Requirements:**
- Python must be in your PATH
- wxPython must be installed (`pip install wxPython`)

## Using the Executable

**For End Users:**
1. Double-click `FX_Mapper.exe`
2. Choose "Create New Mapping"
3. Paste REAPER FX dump
4. Map your controls
5. Save (creates `%APPDATA%\REAPER\UserPlugins\SoundFirst_Maps\` automatically)

**No installation needed!** The `.exe` is fully portable.

## Distribution

You can share `dist\FX_Mapper.exe` directly with users. It includes all dependencies (wxPython, Python runtime) embedded.

**File size:** ~50-100 MB (due to wxPython bundle)

## Notes

- The build process creates a `build/` and `dist/` folder
- You only need to distribute `dist\FX_Mapper.exe`
- Users do NOT need Python installed to run the `.exe`
