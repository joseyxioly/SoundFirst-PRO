# r10c: control Komplete Kontrol

1. Press Shift+Browser (V2.0.2; plain Browser before).
2. Turn the encoder to choose Instrument mode, then press the encoder.
3. Press Plugin, followed by Browser.

You can now use Komplete Kontrol's native NKS instrument controls.
To leave Instrument mode, press Browser twice.
Outside Instrument, choose MIDI/Audio/Instrument with Shift+Browser, turn the encoder,
then press the encoder. Double Browser is an exit from Instrument, not an
outside-Instrument mode-cycle gesture. SoundFirst mode and native NI context
are separate. Both physical presses remain visible to NI.

# Instrument mode r10: native NI handoff

Base: SoundFirst-PRO 75669c67ad7de22efebeb39bada1cddd965e1d82.
Target: Windows, A61. A49/M32 still need external testers.
Experimental source package, not a tested REAPER DLL.

## Routing

Mixer, FX, MIDI and Audio keep the existing SoundFirst HID controls and voice
feedback. Instrument stops every custom HID action except an enabled exit
recognizer, and stops every MIDI DAW action: no transport, undo, navigation,
volume, pan, mute, solo or FX change from that path. Native NI may still act on
those physical controls. SoundFirst cannot consume or suppress NI's input.

The HID reader stays open with shared access, continuously adopting baseline
and knob values. This allows exit observation and prevents replaying changes
made while Instrument was active. Custom touch solos are released before entry.

Keep the NI DAW session and instance focus. HELLO BF 01 04 requests protocol
version 4; nonzero BF 01 V confirms it. Normal close sends BF 02 00. During
Instrument, only these session packets, configuration SysEx03 and instance
focus SysEx41 may leave the MIDI output. No custom screen, mixer, meter or
screensaver writes. NI's own native display is unaffected.

Outside Instrument, preserve the existing display work rather than redesign
it. Correct its CC channel B0 to BF; CC05 selects track/FX bank mapping, not
native Browser ownership. Display cache is per surface and invalidated at
entry/exit. Exit restores Mixer routing and redraws its existing display.
The actual hardware rendering remains unverified.

## Focus and session status

Scan selected-track top-level FX parameters 0,128,2048,4096 for an NI instance
ID: NI, at least two letters, at least two digits. First match wins. No nested,
input or take FX support. Scan and transmit the mode-selected focus even without ACK, matching the
pre-ACK selection path in ReaKontrol. This never enables DAW actions.
Detected ID and last transmitted focus are separate. Only Instrument may
transmit a valid NI instance ID. Mixer/FX/MIDI/Audio request empty focus, including
startup selection/ACK. Exit explicitly sends empty focus once to withdraw this surface's
link. Custom-mode track changes cannot reactivate the detected NI instance.
Mode changes are considered even when selected track/ID did not change.
Thus "no ID", "custom routing disabled" and "ACK unconfirmed" are separate facts.
Empty focus is NOT a guarantee of exclusive physical control: NI software still
receives native HID controls and may remain in its own Browser context. Opening logs endpoint
names, match counts, selected pair, failures and timeout. Recovery has at most
3 HELLO requests per opened session (initial plus2), at least3 seconds apart.
Retries only while Instrument, a valid detected ID exists, ports remain open
and ACK is absent. No new budget for each track/instance; stop after ACK.
Manual Reconnect opens a new session and resets its bounded budget.
Explicit Reconnect and Report actions remain. No port reconnect on every mode toggle. HELLO recovery is separate.

## Entry and exit

Enter through the existing mode selector or REAPER toggle action. Native NI
Browser/Plugin behavior still belongs to NI; there is no forced Browser command.
Assign the REAPER Exit action a shortcut before testing.

[INSTRUMENT] defaults:
- DoubleModeExit=1; DoubleModeWindowMs=400. Mode selector/Browser, byte3 mask01.
- DoubleTrackExit=1; DoubleTrackWindowMs=400. Legacy TRACK-to-Mixer, mask04.
  Existing explicit 0 overrides only its respective option.
- ShiftTrackExit=0; requires Shift already held on TRACK rising edge.
- HoldUndoExit=0; hold UNDO at least one second then release.

The selector that opens MIDI/Audio/Instrument uses byte3 mask01 in the original
source. r7 adds double press of that same selector to exit. Legacy TRACK remains
byte3 mask04. They have independent timers: selector then TRACK is not a double.
Both timers reset on entry, exit, config change and HID disconnect. Native NI
also receives both selector presses; these may affect its Browser/selector state.
SoundFirst cannot consume them or guarantee the exact NI side effect. Native NI
may also respond to either TRACK press or Shift+TRACK. UNDO may undo natively.
No octave exit or inferred alternate bit. These gestures depend on HID reports
actually arriving; r7 does not claim to fix missing physical TRACK reports. It implements the
owner-selected mode-selector exit explicitly.
The REAPER Exit action is the known fallback from the owner's tests.

## Protocol provenance

Interoperability behavior was studied from public ReaKontrol source; no GPLv2
implementation is incorporated. This is independently written, not a formal
clean-room process or legal opinion. SoundFirst retains its existing license.
ReaKontrol does not implement this application mode or a HID TRACK exit.

## r9 startup diagnosis boundaries

Focus before ACK is a sequence change, not proof that it will cause an ACK.
HELLO BF0104 still addresses the DAW service and carries no track or instance
number. NIKB00/01/02 are instance IDs, not track numbers or privileged IDs.
No special rule for track1. ReaKontrol itself sends HELLO once at construction
and uses manual Reconnect; these bounded retries are SoundFirst's addition.

For at most24 received DAW messages in startup/retry diagnostic windows, log
size/decode result and first24 bytes. Ordinary music MIDI input is not opened by
this diagnostic. Counters persist through retries and reset on port reconnect.
The status separates received MIDI, decoded CC, rejected messages, HELLO attempts,
ACK, detected ID and last transmitted focus. No CC substitutes for an ACK.

Empty custom-mode focus does not stop native NI HID/IPC reactions. Missing ACK
cause and owner's track1 correlation remain unproven. Source tests cannot
promise exclusive physical control, successful ACK or native screen behavior.

## r9b compact log and wire alignment

Keep the same SoundFirst_Logs.txt destination. No wall-clock time or ENTRY prefix
on new lines. Old appended lines are not rewritten. Endpoint logs only matching
present A/M DAW devices, not128 empty slots. Repeated identical focus scans and
transmissions suppressed; explicit Report still prints current status. Raw HID
truth output only on manual capture. First ignored DAW command per Instrument
entry only; bounded raw MIDI diagnostics/counters remain.

CC uses Send(BF,command,value,-1), SysEx uses SendMsg(...,-1), like ReaKontrol.
CC decoder accepts padded size>=3 while validating first3 bytes. No ACK is
assumed. HANDSHAKE_COMPARISON.md lists exact differences and uncertainty.

## r10 connection status and voice

No protocol/retry/focus change from r9b. Keep initial+2 HELLO requests per open
session, at least3 seconds apart, only Instrument+ID+ports+missing ACK. No delayed
focus resend. Pending attempts now say "Waiting for NI DAW acknowledgement".
Only final timeout after3 attempts recommends Reconnect; port failures and a
missing instance ID have their own descriptions. Entry voice before a pending
ACK is not proof that all attempts failed.

[NI_CONNECT] reports state, ACK, open input/output, detected ID, hello attempt/3
and received/decoded/rejected counters. Automatic output occurs only when state
changes, not for every incoming message. Report forces one current snapshot.
No timestamp added. Restart creates a new session and resets its attempt budget.

Native PLUG-IN then BROWSER enables native control on reentry, as the owner has
reported. SoundFirst mode and NI physical context are separate. ACK failure on
cold start remains unproven without the final connection summary. This revision
corrects misleading pending-state voice, not a guaranteed handshake fix.

## r10b voice guidance (2026-10-09)

Entry says: "Instrument mode. Press Plugin, then Browser to control Komplete
Kontrol. Press Browser twice to leave Instrument mode." Exit describes the full Browser/encoder/Plugin/Browser entry sequence. This does not pretend
Plugin+Browser changes SoundFirst's mode; it activates native NI context.
Pending ACK stays in the log, not voice. Explicit Report writes technical status
to the log and speaks only that it was written. Unavailable ports/final exhausted
ACK timeout still speak a real failure. Routing/protocol/retries unchanged.
