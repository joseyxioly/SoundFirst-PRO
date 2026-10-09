// SoundFirst NI bridge. Independently written from a behavioral specification.
// Copyright (c) 2026 Jose Perez. License: GPL v3 (see documents/LICENSE).
#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

namespace soundfirst_ni {
using Packet = std::vector<unsigned char>;
inline Packet hello() { return {0xBF, 0x01, 0x04}; }
inline Packet goodbye() { return {0xBF, 0x02, 0x00}; }
inline int relative(unsigned char value) {
    return value < 64 ? int(value) : int(value) - 128;
}
inline bool validInstance(const std::string& id) {
    if (id.size() < 6 || id[0] != 'N' || id[1] != 'I') return false;
    std::size_t p = 2;
    while (p < id.size() && ((id[p] >= 'A' && id[p] <= 'Z') ||
                           (id[p] >= 'a' && id[p] <= 'z'))) ++p;
    if (p < 4 || id.size() - p < 2) return false;
    for (; p < id.size(); ++p) if (id[p] < '0' || id[p] > '9') return false;
    return true;
}
inline Packet sysex(unsigned char command, unsigned char value,
                    unsigned char slot, const std::string& text = {}) {
    Packet result{0xF0, 0x00, 0x21, 0x09, 0x00, 0x00, 0x44, 0x43, 0x01, 0x00,
                  static_cast<unsigned char>(command & 127),
                  static_cast<unsigned char>(value & 127),
                  static_cast<unsigned char>(slot & 127)};
    for (unsigned char c : text) result.push_back(c < 128 ? c : '?');
    result.push_back(0xF7);
    return result;
}
inline Packet focus(const std::string& id) {
    return sysex(0x41, 0, 0, validInstance(id) ? id : std::string());
}
inline std::string selectedFocus(bool instrument, const std::string& detected) {
    return instrument && validInstance(detected) ? detected : std::string();
}
inline bool risingBit(unsigned char current, unsigned char previous, unsigned char mask) {
    return (current & mask) != 0 && (previous & mask) == 0;
}
class DoublePress {
    bool armed = false;
    double first = 0;
public:
    void reset() { armed = false; }
    bool press(double seconds, double window) {
        if (armed && seconds >= first && seconds - first <= window) {
            armed = false; return true;
        }
        first = seconds; armed = true; return false;
    }
};
class HelloRecovery {
    int attempts_ = 0;
    double last_ = -1;
public:
    void reset() { attempts_=0; last_=-1; }
    void sent(double now) { ++attempts_; last_=now; }
    int attempts() const { return attempts_; }
    bool due(double now, bool instrument, bool hasId, bool ack, bool portsOpen) const {
        return portsOpen && !ack && instrument && hasId && attempts_>0 &&
               attempts_<3 && now>=last_ && now-last_>=3.0;
    }
};
inline std::string connectionPhase(bool portsOpen, bool ack, bool timedOut,
                                   int attempts, bool instrument, bool hasId) {
    if (ack) return "confirmed";
    if (!portsOpen) return "ports unavailable";
    if (!timedOut) return "awaiting ACK";
    if (attempts>=3) return "attempts exhausted";
    if (!instrument) return "recovery paused: custom mode";
    if (!hasId) return "recovery paused: no NI ID";
    return "recovery eligible";
}
struct Command { unsigned char number = 0; unsigned char value = 0; };
inline bool decodeCc(const unsigned char* bytes, std::size_t count, Command& out) {
    if (!bytes || count < 3 || bytes[0] != 0xBF || bytes[1] > 127 || bytes[2] > 127)
        return false;
    out.number = bytes[1]; out.value = bytes[2]; return true;
}
inline bool validSysex(const unsigned char* bytes, std::size_t count) {
    const Packet prefix{0xF0, 0x00, 0x21, 0x09, 0x00, 0x00, 0x44, 0x43, 0x01, 0x00};
    if (!bytes || count < prefix.size() + 4 || bytes[count-1] != 0xF7 ||
        !std::equal(prefix.begin(), prefix.end(), bytes)) return false;
    for (std::size_t p = prefix.size(); p + 1 < count; ++p)
        if (bytes[p] > 127) return false;
    return true;
}
// MIDI is for NI session/focus only; HID owns custom-mode actions.
inline bool allowDawAction(bool instrument, bool connected, unsigned char command) {
    (void)instrument; (void)connected; (void)command;
    return false;
}
inline bool allowOutput(bool instrument, const unsigned char* bytes, std::size_t count) {
    if (!instrument) return true;
    if (!bytes) return false;
    if (count == 3 && bytes[0] == 0xBF && bytes[2] < 128)
        return bytes[1] == 0x01 || bytes[1] == 0x02; // session HELLO/GOODBYE only
    if (!validSysex(bytes,count)) return false;
    // Instance focus and handshake surface configuration are not display writes.
    return bytes[10] == 0x41 || bytes[10] == 0x03;
}
struct DisplayCache {
    char line1[64] = {};
    char line2_name[64] = {};
    char line2_value[64] = {};
    bool screensaver = false;
    bool selector = false;
    int track_available = -1;
    int bank_mapping = -1;
    void invalidate() { *this = DisplayCache{}; }
};
inline bool shiftTrack(unsigned char current, unsigned char previous, unsigned char modifiers) {
    return (modifiers & 1) != 0 && risingBit(current,previous,4);
}
inline bool heldEnough(double start, double release, double seconds) {
    return start >= 0 && release >= start && release - start >= seconds;
}
inline bool nativeNavigation(unsigned char command) {
    return command == 0x30 || command == 0x31 || command == 0x32 ||
           command == 0x33 || command == 0x34 || command == 0x35 || command == 0x36;
}
inline bool useCustomHid(bool instrument) { return !instrument; }
inline int volumeFeedback(double amplitude) {
    // SoundFirst's own monotonic display scale; verify feel/display on hardware.
    if (amplitude <= 0) return 0;
    double db = 20 * std::log10(amplitude);
    return int(std::clamp((db + 60) * 127 / 66, 0.0, 127.0) + 0.5);
}
inline int panFeedback(double pan) {
    return int(std::clamp((pan + 1) * 63.5, 0.0, 127.0) + 0.5);
}
}
