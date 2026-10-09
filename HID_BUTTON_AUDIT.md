# r7 selector exit update

Mode selector for MIDI/Audio/Instrument: byte3 mask01 (original Browser label).
Double selector01 now exits Instrument by default; separate from legacy
TRACK-to-Mixer mask04. Logs name both masks explicitly. Outside Instrument the
original mode-selector behavior is unchanged. NI also receives both01 presses.

# r4 default: double TRACK enabled

Double TRACK exit is ON by default with a 400 ms window. No INI edit is needed
for a new/default configuration. An explicit existing DoubleTrackExit=0 in the
INI still disables it. Set it to 1 or remove that override to use the default.
DoubleTrackWindowMs can adjust 150-1000 ms. Both presses can still affect NI.
REAPER Toggle and Exit actions remain the fallback; assign your own shortcut.

# HID press/release audit

Reviewed original GitHub commit 75669c67ad7de22efebeb39bada1cddd965e1d82.
Line references below refer to that original, not this modified copy:
https://github.com/joseyxioly/SoundFirst-PRO/blob/main/src/csurf_soundfirst.cpp

## Reports are not actions

HandleHidReport compares data with m_last_hid_report (line 703). A normal button
runs its action only when its bit is now 1 and was previously 0. A release packet
(bit now 0) updates the saved report but does not run that action. A repeated
packet with bit still 1 also does not run it. At line 1190 the whole new report
is saved for the next comparison.

The repo does not include the user's separate HID capture script. Its printed
"command" labels cannot prove whether raw press/release bytes differ. If both
raw packets really contain the identical pressed bit, with no release bit seen,
this state parser cannot distinguish a second press. We need raw captures to
verify that situation, not a guessed alternate event-counting algorithm.

## Buttons in the original

All rows except Shift and knob touch execute on press edge only; release does
not repeat the action. Report/log activity may still occur on release.

| Button/control | Byte / mask | Original lines | Press behavior |
| --- | --- | --- | --- |
| Shift | 1 / 01 | 706 | Updates modifier state on every report; no standalone action |
| Scale | 1 / 02 | 721-724 | Save project and announce |
| Arp | 1 / 04 | 727 | Report track peak |
| Undo | 1 / 08 | 730-733 | Undo; Shift means redo |
| Quantize | 1 / 10 | 762-796 | Run mapping first; otherwise MIDI quantize, Audio quantize/reverse with Shift, or reference solo toggle in Mixer |
| Ideas | 1 / 20 | 736-759 | Mapping first; otherwise Shift toggles peak/GR, FX reports GR, other modes toggle touch auto-solo |
| Loop | 1 / 40 | 799-818 | Mapping first; fallback cut MIDI/Audio or toggle repeat |
| Metro | 1 / 80 | 821-834 | Mapping first; fallback copy MIDI/Audio or metronome |
| Tempo | 2 / 01 | 838-851 | Mapping first; fallback paste MIDI/Audio or tap tempo |
| Play | 2 / 02 | 853-859 | Play/pause; Shift executes 40042 |
| Record | 2 / 04 | 862-867 | Record; Shift tries AUTO mapping then automation-cycle |
| Stop | 2 / 08 | 869 | Stop |
| Preset up | 2 / 10 | 873-903 | Next FX page, Audio split, or the stated 40294 action in MIDI/Mixer context |
| Preset down | 2 / 20 | 934-961 | Previous FX page, MIDI/Audio removal, or FX-chain bypass |
| Mute | 2 / 40 | 906-922 | Mapping first; fallback plugin bypass in FX, track mute otherwise |
| Solo | 2 / 80 | 925-931 | Mapping first; fallback selected-track solo |
| Browser | 3 / 01 | 974-977 | Opens SoundFirst mode selection, announces current choice |
| Plugin | 3 / 02 | 980-993 | Enters FX, resets FX/page to 0, announces plugin |
| TRACK | 3 / 04 | 996-1000 | Cancels mode selector, enters Mixer, announces MIXER Mode |
| Encoder up | 3 / 20 | 1004-1007 | Previous track and bank sync |
| Encoder down | 4 / 01 | 1008-1011 | Next track and bank sync, only when Browser bit is not set |
| Encoder left | 3 / 40 | 1013-1036 | Previous FX with UI sync, previous item in MIDI/Audio, or previous marker |
| Encoder right | 3 / 80 | 1038-1062 | Next FX with UI sync, next item in MIDI/Audio, or next marker |
| Encoder click | 5 / 02 | 1065-1117 | Confirms mode selector; otherwise opens/closes editors/FX according to mode and Shift, or inserts marker |

Mapping priority matters. RunNhlAction (roughly lines 360-535) can override
fallbacks through FX maps, mode actions, Shift actions and global INI settings.
LoadMappingConfig also supplies defaults, so a comment describing the fallback
is not proof that this user's INI executes it. The actual user's INI is not in
this source package and has not been inspected.

## Not ordinary buttons

- Knob touch K1-K7: byte 4 bits 1-7, lines 1121-1145. K8: byte 5 bit 0,
  lines 1149-1167. Both touch and release are intentionally processed. In Mixer,
  touching stores solo then sets it; release restores solo. In FX the handler
  gets true on touch and false on release. These are NOT duplicate commands.
- Encoder rotation: byte 28 counter difference, lines 1171-1175.
- Knob rotation: bytes 6-21 counters, lines 1179-1187. Differences are wrapped
  and large jumps filtered; these are not press/release commands.

## Instrument double TRACK

Only in Instrument mode and enabled by default in r4, use the same 0->1 predicate as
original TRACK. Count two press edges within 400 ms, never release events.
The timestamp is recorded when HID is received, not when the queue is drained.
Outside Instrument the original TRACK condition/action is unchanged.

Verified helper test sequences:
- 0, pressed, released: no double exit.
- 0, pressed, pressed duplicate, released, released duplicate: no double exit.
- 0, pressed, released, pressed, released within window: one double exit.
- 0, pressed, pressed, pressed (identical raw packets): no double exit.
- Two press edges outside window: no double exit.

The first experimental build unnecessarily reset the HID baseline on the toggle
REAPER action. If the next received report was the first TRACK press, it could
be treated as baseline and missed. Revision r2 preserves the continuously
observed baseline on mode toggle. Initial connection/reconnection still needs a
baseline packet. If the keyboard's first ever report is already a held button,
that held state is adopted safely, not fired as a new action.

## Changes outside Instrument that must be tested, not hidden

The button action bodies/conditions above were retained, except adding Instrument
as a mode choice. There ARE surrounding changes: initialization/baseline on HID
connection, minimum report length, overlapped HID reading and shutdown, logging
instead of calling speech on the worker's search retry, DAW session and feedback,
and safer Mixer touch restoration using the track that was actually touched.
These can affect runtime. Passing parser tests is not proof of no regression.
Test the existing modes and bindings on A61 before relying on the new build.

NI receives both TRACK presses too. No native action is intercepted or undone.
Do not assume that Track remains in the same NI context between presses. Use the
REAPER toggle shortcut if HID reports or NI interactions make the gesture fail.
