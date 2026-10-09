# Changelog: Instrument mode and SoundFirst PRO naming

## V2.0.1 bug fix: first button press ignored (2026-10-09)

**Symptom.** After REAPER started, the first button press on the keyboard did
nothing. For example, Browser had to be pressed twice before SoundFirst said
"Mode MIDI". Every press after that worked. It happened again after the
keyboard disconnected and reconnected.

**Cause.** The keyboard only sends a HID report when a button, knob or the
encoder changes. SoundFirst used the first report it received as its
"nothing pressed" reference and returned without processing it. That first
report was normally the user's first button press, so the press became the
reference instead of an action. Found with the HID capture action on an M32.

**Fix.** The first report still sets the knob and encoder reference, so
knobs do not jump. Buttons (bytes 1-5) are compared against "nothing held",
so a press in the first report is handled like any other press. This applies
in every mode, including Instrument. A regression test covers it.

## Release preparation (r12, 2026-10-09)

README and release notes explain native NKS control, the selected/armed-track
workflow, installation, compilation, requirements and troubleshooting. Tested
scope and remaining limits are explicit. No code change from r11.

## User-facing changes

- New Instrument mode alongside existing Mixer, Plugin, Audio and MIDI controls.
- Native Komplete Kontrol/NKS browsing and instrument control after choosing
  Instrument and pressing Plugin then Browser. SoundFirst does not implement
  an NKS library engine.
- Double Browser exits Instrument by default; legacy double Track is separate.
  Browser+encoder remains the selector outside Instrument.
- Voice gives the physical workflow. Pending connection status stays in the log.
- Windows extension renamed `reaper_soundfirst_pro.dll`, build/project and
  FX Mapper branding changed to SoundFirst PRO.

## Integration and reliability

- Custom HID/DAW actions and screen/mixer writes paused in Instrument, apart from
  enabled exit recognition. HID baseline stays current for return to custom modes.
- Detected NI ID and transmitted focus kept separate. Valid instance focus only
  in Instrument; custom modes announce empty focus, including exit.
- Focus can be sent before ACK. CC dispatch uses channel16/BF with direct Send;
  padded CC events accepted after checking their first three bytes.
- HELLO recovery: initial request plus at most two retries per open session,
  at least three seconds apart, only with Instrument/ID/open ports/no ACK.
- Compact logs without timestamps, bounded raw DAW diagnostics, no empty endpoint
  lists or identical focus-state repeats. `[NI_CONNECT]` records connection state.
- Per-surface display cache, explicit restoration on return to Mixer and
  idempotent REAPER Exit action.

## Verification and known limits

Protocol/routing/focus tests pass in Debug and Release; source checks and Windows
C++17 syntax checks pass. José tested his A61 workflow and switching instances.
A49/M32 Instrument use remains unverified by him. No prebuilt binary or full
MSVC ABI/load validation is included in this source package. Native NI context
and physical events are not exclusive to SoundFirst.
