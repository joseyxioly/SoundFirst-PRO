# r11 build name: SoundFirst PRO

The Windows extension is now reaper_soundfirst_pro.dll. Build script/project and
FX Mapper title use SoundFirst PRO. No Instrument/routing/protocol changes.

When upgrading from an earlier experimental build, quit REAPER and move the old
reaper_kompletereaper.dll out of UserPlugins before installing the renamed DLL.
Do not load both: two extensions could claim the same keyboard/DAW ports.
The build script does not delete the old DLL automatically.

# r10c: control Komplete Kontrol

1. Press Browser.
2. Turn the encoder to choose Instrument mode, then press the encoder.
3. Press Plugin, followed by Browser.

You can now use Komplete Kontrol's native NKS instrument controls.
To leave Instrument mode, press Browser twice.
Outside Instrument, choose MIDI/Audio/Instrument with Browser, turn the encoder,
then press the encoder. Double Browser is an exit from Instrument, not an
outside-Instrument mode-cycle gesture. SoundFirst mode and native NI context
are separate. Both physical presses remain visible to NI.

# r10 focused A61 validation

No need to repeat a large diagnostic sequence before checking the main behavior.
Build using the existing Windows/MSVC build or CMake. This ZIP has source, not
an installable DLL. Do not run ReaKontrol or another surface on the same DAW pair.
Keep ordinary music MIDI separate from NI DAW ports.

1. Assign "SoundFirst: Exit Instrument mode" a REAPER shortcut first.
2. First check startup Mixer with Komplete Kontrol present. Report should show
   detected ID but custom NI routing disabled and transmitted focus empty.
   Changing REAPER tracks must not announce a valid NI focus ID in Mixer.
   This tests this surface's focus routing, not native NI input isolation.
   Select a track containing Komplete Kontrol. Enter Instrument using the
   existing selector or toggle action. Read Report NI connection and focus.
   A found ID with ACK0 may now be sent before ACK. It does not mean session
   acknowledgement succeeded or enable any SoundFirst DAW actions.
3. Use native Plugin/Browser and the encoder/knobs. SoundFirst must not move
   the REAPER cursor, volume, pan, mute, solo, transport or undo from those
   incoming control messages. NI itself may perform native actions.
4. Exit with the REAPER action or the double selector01 gesture.
   [NI_FOCUS_SENT] must show EMPTY, then Mixer returns. NI may still process
   native Browser events independently; this build does not block them. Existing Mixer encoder, knobs and voice should
   resume without replaying movements made in Instrument. Existing Mixer
   display is restored, not redesigned in r9.

Default exit: press the SAME selector used to choose MIDI/Audio/Instrument
 twice within 400 ms, releasing between presses. This is Browser/selector01,
not TRACK04. One press, release alone or duplicate reports must not exit.
On the second press, SoundFirst returns to Mixer. NI receives both presses too:
watch whether its native Browser/selector opens/closes or changes state. That
side effect is not suppressed or verified here.

Legacy double TRACK04 remains ON independently. Disable with DoubleTrackExit=0
if not wanted. DoubleModeExit=0 disables only the new selector gesture.
DoubleModeWindowMs and DoubleTrackWindowMs independently allow 150-1000 ms.
No INI edit is needed for default selector exit. Computer-keyboard REAPER Exit
remains the fallback. No octave mapping or exit is added.

Capture is optional and manual: "SoundFirst: Capture Instrument HID for 60
seconds". If needed, label physical controls and times, avoid unrelated keybed
activity, review before sharing. Endpoint logs include MIDI device names.

If ACK is unconfirmed, the new [NI_ENDPOINT], [NI_SESSION], [NI_ACK] and
[NI_STATUS] lines distinguish missing names, ambiguous pair, open failure and
no reply. Reconnect explicitly resets the attempt budget. Instrument with a detected
ID also permits at most2 HELLO retries,3 seconds apart, without reopening ports. The code does not establish which other
program owns a port when opening fails.

A49/M32, Windows linking, REAPER load, native display and physical TRACK remain
unverified in this workspace. No claim that SoundFirst can block NI events.

No need for another large button capture. For startup/recovery, select the track
with KK (any track/ID), enter Instrument and wait up to9 seconds for the bounded
attempts and final timeout. Report gives counters. [NI_HELLO_RETRY], [NI_MIDI_RX],
[NI_ACK] and [NI_STATUS] now show what happened. Raw DAW diagnostics are bounded;
review them before sharing. Do not infer that track1 or NIKB00 is required.

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
