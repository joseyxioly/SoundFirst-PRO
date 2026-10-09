# Changelog: Instrument mode and SoundFirst PRO naming

## V2.0.2 change: Shift+Browser opens the mode menu (2026-10-09)

**Problem.** Plain Browser opened SoundFirst's mode menu, but Browser is also
Komplete Kontrol's own button. Each press opened NI's browser as well, and
NI's browser then took the encoder. Turning the encoder to choose a
SoundFirst mode also scrolled through Komplete Kontrol's browser. SoundFirst
cannot stop NI from receiving a button, so this happened on every model.

**Cause.** A shared button. Outside its browser, NI does not use the encoder
itself; it only forwards turns to the DAW port (CC 0x34), which SoundFirst
ignores. So the clash came from Browser, not from the encoder.

**Change.** The mode menu now opens with Shift+Browser. Komplete Kontrol
ignores that combination (checked on an M32 with NI's speech), so NI's
browser stays closed and the encoder only moves SoundFirst's menu. Plain
Browser is left to Komplete Kontrol. Instrument mode is unchanged: Plugin
then Browser still hands control to NI, and Browser twice still leaves it.
The Mixer voice guidance now says "press Shift and Browser".

**Also: plain Browser enters Instrument mode.** Since plain Browser now
belongs to Komplete Kontrol, pressing it on a selected track that has an NI
instance switches SoundFirst into Instrument mode in the same press:
SoundFirst pauses its controls and focuses that instance while NI opens its
browser. Voice: "Instrument mode. Press Browser twice to leave." On a track
without an NI instance SoundFirst says "No Komplete Kontrol on this track"
and stays in its current mode. The instance scan moved into
`FindNiInstance`, shared with focus updates; the focus test covers it.

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

## V2.0.1 bug fix: encoder ignored on M32 (2026-10-09)

**Symptom.** On the Komplete Kontrol M32, turning the 4D encoder did nothing
in SoundFirst. After pressing Browser, the encoder could not choose MIDI,
Audio or Instrument, so Instrument mode could not be reached from the
keyboard. Only the NI software reacted to the encoder.

**Cause.** SoundFirst read the encoder counter from byte 28 of the HID
report, which is where the A-series keeps it. The M32 sends a longer report
with the counter at byte 24, and byte 28 is always zero. Confirmed with the
HID capture action on an M32: three clicks right counted 1 to 4 at byte 24,
three clicks left counted back to 1. Knob values and knob touch were already
read from the right positions on the M32.

**Fix.** When the HID connection opens, SoundFirst checks the keyboard model.
M32 (PID 1860) reads the encoder from byte 24; A-series keyboards keep byte
28. The log records which layout was chosen (`[HID_LAYOUT]`).

## V2.0.1 bug fix: encoder jumped the wrong way (2026-10-09)

**Symptom.** Roughly once every 16 encoder clicks, a single click was read
as a large step in the opposite direction.

**Cause.** The encoder counter is 4 bits: it counts 0 to 15 and then wraps
back to 0. SoundFirst only handled wrapping for an 8-bit counter, so one
click across the wrap (15 to 0) looked like 15 steps backwards. The 4-bit
counter is documented for the A25 by the komplementary-kontrol project, which
reads the same report; the M32 capture shows the same small counter.

**Fix.** The encoder step now takes the shortest way round a 4-bit counter
(and still handles an 8-bit counter). The calculation lives in
`soundfirst_ni::encoderDelta` and is covered by the protocol tests.

## V2.0.1 addition: Komplete Kontrol A25 detection (2026-10-09)

**Problem.** The A25 (USB product ID 1730) was not in SoundFirst's device
list, so SoundFirst never connected to its buttons, knobs or encoder.

**Fix.** PID 1730 is now detected. The A25 sends the same 30-byte report as
the A49/A61 (button bits, knob touch, encoder at byte 28), confirmed against
the komplementary-kontrol A25 source. Not yet tested on an A25.

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
