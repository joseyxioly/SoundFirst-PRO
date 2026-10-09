# r12 release documentation verification, 2026-10-09

README and RELEASE_NOTES describe the owner-tested A61 workflow and make the
A49/M32 limits explicit. README, release notes and changelog changed; production
code and build files are byte-identical to r11. Existing Debug/Release CTest
3/3 passed again and source guards passed. No prebuilt DLL or full MSVC link/load
is produced by this documentation revision.

José subsequently confirmed Instrument entry, double Browser exit, MIDI editing
and switching selected/armed Komplete Kontrol tracks on his A61. He clarified
that his earlier cold-start connection concern involved the USB cable being
disconnected. Earlier entries below are historical automated-test records; their
open hardware/cold-start questions are not current claims that his workflow is
still failing. NI physical context and untested hardware remain limits.

# r11 name-only verification

Debug/Release CTest3/3, existing source guards and Windows C++17 syntax passed.
Production source unchanged from r10c. Changed CMake/build naming and FX Mapper
text only, plus installation docs/manifest. No actual MSVC DLL build/link/load.

# r10c voice and quick start, 2026-10-09

Use Browser, choose Instrument with encoder/click, then Plugin followed by
Browser for native NKS control. Press Browser twice to leave Instrument.
English voice and README/quick-start docs use this exact flow. Outside Instrument
selector remains Browser+encoder. No routing/protocol/retry change from r10b.

# r10 verification, 2026-10-08

Base SoundFirst commit: 75669c67ad7de22efebeb39bada1cddd965e1d82.
ReaKontrol reference commit: 4e84390f7ceb4567cd1587f1963eb1f934759160.
Both matched live GitHub HEAD during the code audit.

- Debug CMake/CTest: 3/3 passed.
- Release CMake/CTest: 3/3 passed; assertions explicitly retained under NDEBUG.
- Exhaustive DAW action matrix: 128 commands, both modes/ACK states, no action.
- Output matrix: every CC/SysEx command; Instrument permits only session CC01/02
  and config/focus SysEx03/41, blocks custom screen/mixer/meter output.
- Mocked production branches: actual DAW handler tested across command/value
  combinations; actual Instrument HID branch tested all bits/30 positions,
  baseline updates, legacy double TRACK04 and new double selector01.
- Selector tests: repeats do not exit, two released presses within400ms exit,
  mixed01/04 do not combine, disabling01 works. Earlier capture pair at
  7790.34/7790.50 exits explicitly under r7; this was not the r6 behavior.
- Actual focus branch mocked across Mixer/FX/MIDI/Audio/Instrument, tracks,
  startup ACK, no ACK and exit. Custom modes never send valid NI IDs; entering
  Instrument sends selected ID and exit clears it. Mode/track unchanged still
  responds to the routing transition. Detected ID retained without ACK; r9 transmits mode-selected focus
  before ACK without enabling actions. Recovery actual function tested: spacing,
  max3 requests per session,20 rapid ID changes do not spam, ACK cancels and
  custom modes do not retry. Protocol helper also tests noID/noports/noinitial
  request and NIKB00/01/02 all treated as valid, without track-index dependence.
- Source guards verify both routing and focus mock text matches production, no action sink in the DAW
  handler, output guards before sends, per-instance cache and explicit redraw.
- Windows-target C++17 syntax check: main.cpp/csurf_soundfirst.cpp with MinGW10,
  Windows headers, NOMINMAX, WIN32_LEAN_AND_MEAN and _WIN32_WINNT=0x0601: passed.
- Source ZIP and manifest checked; Drive round-trip hash verification at delivery.

Not run: full Windows/MSVC link, ABI/load test, physical A61, native NI UI/display,
A49/M32 or real MIDI-port ownership. Mocked branches are not REAPER runtime tests.
No pixels to verify for this source-only change. Hardware screen rendering is
explicitly unverified; no new designed screen deliverable is included.

Known limits: physical TRACK may not arrive; r7 adds a separately selected01 gesture,
not a guessed replacement bit. Native NI side effects from double Browser01
remain unverified. The owner reported r6 exit success but its exact physical
route is not established by the earlier capture; do not claim r6 listened01.
ACK0 cause in the owner's last log remains unknown without session-opening data.
No forced Browser command or exclusive ownership of the NI keyboard is claimed.

r9 is a startup-sequence/recovery/diagnostic improvement. It is not a confirmed
fix for missing ACK, the owner's track1 correlation or native NI encoder activity
outside Instrument. No privileged instance ID, fake ACK or protocol downgrade.

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
Kontrol. Double-press Browser to return to SoundFirst." Exit describes the actual
custom selector: Browser, turn encoder, press encoder. This does not pretend
Plugin+Browser changes SoundFirst's mode; it activates native NI context.
Pending ACK stays in the log, not voice. Explicit Report writes technical status
to the log and speaks only that it was written. Unavailable ports/final exhausted
ACK timeout still speak a real failure. Routing/protocol/retries unchanged.
