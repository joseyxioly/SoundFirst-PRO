# ReaKontrol vs SoundFirst handshake, r9b

Reference ReaKontrol HEAD 4e84390f7ceb4567cd1587f1963eb1f934759160.
Source: https://github.com/jcsteh/reaKontrol/blob/master/src/main.cpp
Source: https://github.com/jcsteh/reaKontrol/blob/master/src/niMidi.cpp

| Step | ReaKontrol | SoundFirst through r8 | r9b |
|---|---|---|---|
| Endpoint | First present suffix match; A/M plus S variants | Exactly one A/M substring match each direction, same family | Same safe A/M match, no guessing fallback |
| Open | Create input, create output, input start | Same order | Same |
| HELLO | BF 01 04 via Send(...,-1) | Identical bytes via SendMsg(frame0) | Send(BF,01,04,-1), same API as ReaKontrol |
| ACK | Checks status BF, then reads command/value | Requires size exactly3, BF and7bit bytes | Size at least3; BF and valid first3 bytes, padded event accepted |
| Focus | Selection callback sends SysEx41 before ACK too | All focus gated on ACK | Mode-selected focus before ACK: Instrument ID, custom empty |
| SysEx dispatch | SendMsg(event,-1) | SendMsg(event,0) | SendMsg(event,-1) |
| HELLO retries | None, manual reconnect | None, manual reconnect | Bounded initial+2, >=3s apart, Instrument+ID+ports/noACK |
| Channel pressure | NI handler ignores nonBF except SysEx | Same | Same |

ReaKontrol's opening does not inspect a plugin or require track1 before HELLO.
NIKB00/01/02 are instance IDs, not track numbers. ReaKontrol's protocol version
starts at0, but selection focus is not gated on it. Its one-shot timer resolves
OSARA only; it does not retry handshake. SoundFirst retains safe A/M-only endpoint
validation rather than expanding hardware scope or choosing the first ambiguous
endpoint. The owner's log already shows input1/output2 opened, so discovery
failure is not the demonstrated cause for that session.

Concrete robustness differences corrected: pre-ACK focus, CC dispatch API/frame,
SysEx frame, padded CC event acceptance. We do NOT know which (if any) caused the
owner's missing ACK. Receiving CC34 proves incoming DAW traffic, not HELLO ACK.
Raw startup diagnostics record event size and bytes to distinguish no response
from rejected/padded response. Never infer ACK from other CCs or plugin ID.

Native NI Browser/knobs work through NI software's own route. Empty focus is not
a physical encoder lock. r9b is not a confirmed fix for track1 correlation or
exclusive custom-mode ownership. No protocol downgrade or privileged ID.
