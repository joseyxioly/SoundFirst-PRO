# SoundFirst PRO

## NKS instruments join the SoundFirst workflow

SoundFirst PRO now works with NKS instruments through Komplete Kontrol's native
controls. Play, browse and change an instrument, then return to Mixer, Audio or
MIDI editing without leaving your keyboard.

Instrument mode is the new handoff: SoundFirst steps aside while Native
Instruments handles its Browser, parameter pages and NKS mappings. SoundFirst
does not read an NKS library or replace Komplete Kontrol.

Designed for sound-first work in REAPER, with spoken feedback through OSARA.
The instructions below use button names and ordered steps rather than pictures.

## Quick start: play and change an NKS instrument

1. Load Komplete Kontrol on a REAPER track. Select and arm that track.
2. Press Browser on the keyboard.
3. Turn the encoder to choose Instrument mode, then press the encoder.
4. Press Plugin, followed by Browser. This activates native Komplete Kontrol
   control after choosing SoundFirst's Instrument mode.
5. Play, browse presets or change the instrument with Komplete Kontrol's native
   NKS controls.
6. Press Browser twice, with a release between presses, to leave Instrument.
   SoundFirst returns to Mixer. The default double-press window is 400 ms.
7. Disarm the track when finished. Select and arm the next track, then repeat.

### Multiple Komplete Kontrol instances

José tested this workflow with a piano on one track and a guitar on another:
select and arm the track to use, enter Instrument, then press Plugin and Browser.
After playing, leave Instrument, disarm it and select/arm the next track.
In that workflow, the armed track takes control of Komplete Kontrol.

Selection matters too: SoundFirst sends the NI instance ID found on the selected
track. It does not automatically choose between several armed tracks. For a
predictable result, keep the intended track selected and only that track armed.
The first matching top-level NI instance on the selected track is used; nested,
input and take FX are not covered by this version.

### Change to Audio or MIDI editing

Outside Instrument, press Browser, turn the encoder to choose Audio or MIDI,
then press the encoder. Double Browser is the exit from Instrument, not a gesture
that cycles through modes outside it. Track selects Mixer; Plugin selects the
custom plugin-control mode when SoundFirst is handling the controls.

SoundFirst's mode and NI's physical keyboard context are separate. Pressing
Plugin then Browser is still needed to activate native controls when returning
to Komplete Kontrol. Both Browser presses also reach NI; they are not consumed.

## Work modes

- Mixer: existing track, mix, transport and encoder controls.
- Plugin: SoundFirst's custom plugin mappings and parameter pages.
- Audio: existing audio-item editing controls.
- MIDI: existing note-editing controls.
- Instrument: native Komplete Kontrol/NKS control, with SoundFirst custom actions
  and display writes paused until exit.

Optional touch-to-solo and spoken-feedback settings remain configurable. A
computer-keyboard shortcut for "SoundFirst: Exit Instrument mode" is a useful
fallback before testing. Legacy double Track exit remains available separately;
see [Instrument settings](INSTRUMENT_MODE_SPEC.md).

## Requirements and tested scope

- Windows, REAPER and the Native Instruments software/drivers needed by your
  keyboard and Komplete Kontrol installation.
- Komplete Kontrol with an NKS instrument or preset you own and have installed.
- OSARA for SoundFirst's spoken messages. Native NI accessibility is handled by
  NI's own software and keyboard context.
- A working USB connection and an available A/M DAW MIDI input/output pair.

José tested the Instrument workflow, exit and switching between instances on
his A61. A49/M32 code paths exist, but this release's Instrument workflow has not
been tested by him on those keyboards. Other users should treat those models
as unverified. Native NI entry/exit and physical button behavior still depend on
NI software, drivers and keyboard state.

This source package contains no prebuilt DLL. Automated code tests and Windows
syntax checks passed; that is not a substitute for a full Windows build or a
physical test of every supported model.

## Install or upgrade

1. Close REAPER.
2. Back up your installed extension and configuration.
3. Move the older `reaper_kompletereaper.dll` out of REAPER's `UserPlugins` folder.
   Do not load it alongside the new extension: both can claim the same ports.
4. Build this source, or use a separately provided binary you trust. Install
   `reaper_soundfirst_pro.dll` in REAPER's `UserPlugins` folder.
5. Connect the keyboard by USB, then reopen REAPER.

Use REAPER's resource-path action to find `UserPlugins`. The existing build
script copies the DLL to `%APPDATA%\REAPER\UserPlugins`; a portable installation
may need a manual copy. The script does not remove your old DLL automatically.
Do not run another control-surface extension on the same NI DAW port pair.
Keep the ordinary music MIDI input separate from the NI DAW integration ports.

## Build from source

Use a Windows C++17 compiler compatible with REAPER's extension ABI, CMake and
NMake. `build.bat` looks for the Visual Studio paths listed in the script, builds
Release and attempts to copy `reaper_soundfirst_pro.dll` to `UserPlugins`.
If your installation path differs, use a Visual Studio developer command prompt:

```bat
cmake -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Close REAPER before copying the DLL. See [verification](VERIFICATION.md) for
what was tested and what remains unverified.

## Troubleshooting

### The keyboard is missing or ACK is not confirmed

First check the USB cable and that the keyboard is connected. A disconnected
cable caused one apparent cold-start connection failure during testing.

Run "SoundFirst: Reconnect NI DAW" to reopen the pair. If it still fails, run
"SoundFirst: Report NI connection and focus" and read the `[NI_CONNECT]` line in
`%APPDATA%\REAPER\UserPlugins\SoundFirst_Logs.txt`. It shows the state, ACK,
open ports, detected ID, HELLO attempts and incoming MIDI counters.

Recovery is bounded: one initial HELLO and at most two retries, at least three
seconds apart, when Instrument is active, an NI ID is detected and ports are
open. A pending ACK is not a final failure. Reconnect resets that budget.

Check that another extension has not claimed the same DAW ports. Renamed MIDI
port aliases can prevent device-name matching. The log distinguishes unavailable
ports from a session that opened but did not receive an ACK.

### Instrument is selected, but the encoder does not control KK

Press Plugin, then Browser. Selecting Instrument in SoundFirst does not itself
press NI's physical mode buttons. Check that the intended Komplete Kontrol track
is selected and armed. If needed, use the Report action to check the detected ID.

### Double Browser does not leave Instrument

Press twice within the configured window, releasing between presses. Use the
"SoundFirst: Exit Instrument mode" REAPER action if the keyboard reports do not
arrive. No octave-button exit is added.

### Komplete Kontrol still reacts outside Instrument

SoundFirst withdraws its NI instance link in custom modes, but it cannot block
native NI HID/IPC input. NI can still react to physical buttons in its own
context. Empty focus is not exclusive ownership of the keyboard.

### Share a short log

New log lines have no wall-clock timestamps. Empty endpoint slots and identical
focus states are not repeated. Raw DAW startup diagnostics are bounded; full HID
capture runs only through the manual capture action. Old lines already appended
to the log are not rewritten. Review logs before sharing them.

## License and project

SoundFirst PRO retains its GNU GPL v3 license; see [LICENSE](documents/LICENSE).
NI software, NKS content and instruments have their own licenses.

Protocol study was informed by the public ReaKontrol project. The NI bridge here
is independently written; no ReaKontrol GPLv2 implementation is included. Thanks
to the people who documented the integration behavior and to users testing it.
This is not a formal clean-room or legal compatibility claim.

[Release notes](RELEASE_NOTES.md) | [Changelog](CHANGELOG_INSTRUMENT.md) |
[Technical specification](INSTRUMENT_MODE_SPEC.md) |
[A61 validation](TEST_A61.md)

Community: [Telegram](https://t.me/SoundFirstPRO)
Support the project: [Buy Me a Coffee](https://buymeacoffee.com/soundfirstpro)

Copyright 2026 José Pérez.
