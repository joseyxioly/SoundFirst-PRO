# SoundFirst PRO V2.0.2: Shift+Browser opens the mode menu

## What changed

To choose a SoundFirst mode (MIDI, Audio or Instrument), press **Shift +
Browser**, turn the encoder, then click the encoder. Plain Browser no longer
opens SoundFirst's mode menu.

## Why

Browser is also Komplete Kontrol's own button. Pressing it opened Komplete
Kontrol's browser at the same time as SoundFirst's menu, so turning the
encoder moved through both at once. Choosing a SoundFirst mode also scrolled
through Komplete Kontrol's sounds.

Komplete Kontrol doesn't react to Shift + Browser, so now only SoundFirst's
menu opens and the encoder only moves through SoundFirst's modes.

## Instrument mode

1. Select and arm the track with Komplete Kontrol.
2. Press **Shift + Browser**, turn the encoder to **Instrument**, and click
   the encoder.
3. Press **Plugin**, then **Browser**, to control Komplete Kontrol.
4. Press **Browser twice** to leave Instrument mode.

Steps 3 and 4 are unchanged. There, Browser is meant to reach Komplete
Kontrol.

## Upgrade

Close REAPER and replace `reaper_soundfirst_pro.dll` in `UserPlugins` with
the one attached to this release.

---

# SoundFirst PRO V2.0.1: bug fixes and A25 support

This release fixes three bugs and adds the Komplete Kontrol A25. Nothing
else changed from V2.0.

## Bug 1: the first button press was ignored

After starting REAPER, the first button you pressed on the keyboard was
ignored. Most people noticed it on Browser: you had to press it twice before
SoundFirst said "Mode MIDI". Everything after that first press worked. It
came back whenever the keyboard was disconnected and reconnected.

**Why it happened.** The keyboard only sends data when something changes.
SoundFirst treated the very first data it received as the "nothing pressed"
starting point and did not act on it. Since that first data was usually your
first button press, the press was used as the starting point instead of
being handled.

**The fix.** SoundFirst still takes knob and encoder positions from that
first data, so knobs don't jump. Buttons are now compared against "nothing
pressed", so your first press works like every other one.

## Bug 2: the encoder did nothing on the M32

On the Komplete Kontrol M32, turning the encoder did not reach SoundFirst.
After pressing Browser you could not choose MIDI, Audio or Instrument, so
Instrument mode could not be reached from the keyboard. Only the Komplete
Kontrol software reacted to the encoder.

**Why it happened.** Each keyboard model arranges its data a little
differently. SoundFirst looked for the encoder where the A-series keeps it.
The M32 keeps it in a different place, so SoundFirst never saw it move.

**The fix.** SoundFirst now checks which keyboard is connected and reads the
encoder from the right place. A-series keyboards work exactly as before.

## Bug 3: the encoder sometimes jumped the wrong way

About once every 16 clicks, one click of the encoder could move a long way
in the wrong direction.

**Why it happened.** The keyboard counts encoder clicks from 0 to 15 and
then starts again at 0. SoundFirst didn't expect the count to restart that
soon, so going from 15 to 0 looked like 15 clicks backwards.

**The fix.** SoundFirst now understands the count restarting, so each click
moves exactly one step. This affects every supported keyboard.

## New: Komplete Kontrol A25

The A25 wasn't in SoundFirst's list of keyboards, so it was never detected.
It's now included. It works the same way as the A49 and A61, but nobody has
tested it on an A25 yet. If you own one, please tell us how it goes.

## Upgrade

Close REAPER and replace `reaper_soundfirst_pro.dll` in `UserPlugins` with
the one attached to this release.

---

# SoundFirst PRO: native NKS instruments in the workflow

Instrument mode brings Komplete Kontrol's native NKS controls into SoundFirst
PRO. Select and arm a track, choose Instrument, press Plugin then Browser, and
play or change your instrument. Press Browser twice to return to SoundFirst.

You can move between a piano on one Komplete Kontrol track and a guitar on
another, then switch to MIDI editing for what you recorded. José tested that
workflow on his A61. A49/M32 Instrument testing remains open.

## The workflow

1. Select and arm the track containing Komplete Kontrol.
2. Press Browser, choose Instrument with the encoder, then press the encoder.
3. Press Plugin, followed by Browser, for native NI/NKS controls.
4. Play, browse presets or change the instrument.
5. Press Browser twice to leave Instrument. Disarm that track.
6. Select and arm the next track and repeat, or use Browser+encoder to choose
   Audio or MIDI editing outside Instrument.

The armed-track workflow is José's tested setup. The integration actually finds
the first NI instance on the selected track, so keep selection and arming aligned.
It does not select automatically among multiple armed tracks.

## Upgrade

Close REAPER and move `reaper_kompletereaper.dll` out of `UserPlugins`. Install
`reaper_soundfirst_pro.dll` instead. Do not load both extensions.
This package is source only; compilation and installation are described in
[README](README.md). No prebuilt binary is included.

## What changed

- Instrument pauses SoundFirst custom HID/MIDI actions and custom screen writes.
- NI instance routing is limited by SoundFirst mode; native physical NI context
  still needs Plugin followed by Browser.
- Double Browser exits Instrument, with an independent legacy double Track exit.
- Session recovery is bounded and diagnostics are compact, without timestamps.
- Pending ACK stays in the log; voice guides the next action. Real port/final
  timeout failures still receive a spoken warning.
- Product and Windows DLL names now match SoundFirst PRO.

## Limits

Native NI entry/exit and HID behavior depend on NI software and keyboard state.
SoundFirst cannot consume native button events or guarantee that NI ignores the
encoder in other contexts. Selected-track top-level NI instances are supported;
nested, input and take FX are not covered by this version.

Automated tests and Windows syntax checks passed. No full MSVC link/load test,
prebuilt DLL or hardware testing on A49/M32 is claimed by this source package.
For connection trouble, check USB, use Reconnect NI DAW, then Report NI connection
and focus. The `[NI_CONNECT]` line gives a compact connection summary.
