from pathlib import Path
r = Path(__file__).resolve().parents[1]
s = (r/'src/csurf_soundfirst.cpp').read_text()
h = (r/'src/csurf_soundfirst.h').read_text()
m = (r/'src/main.cpp').read_text()
assert 'MODE_INSTRUMENT' in h
assert 'SOUNDFIRST_INSTRUMENT' in m and 'SOUNDFIRST_NI_RECONNECT' in m
assert 'if (m_current_mode == MODE_INSTRUMENT) return;' in s
block = s.split('if (m_current_mode == MODE_INSTRUMENT) {')[1].split('return;')[0]
assert 'AdoptHidBaseline(data)' in block
assert 'Main_OnCommand' not in block and 'HandleKnobRotation' not in block
assert 'allowDawAction(m_current_mode == MODE_INSTRUMENT' in s
assert 'FILE_FLAG_OVERLAPPED' in s and 'CancelIoEx' in s
assert 'inputFamily != outputFamily' in s
assert 'if (m_current_mode == MODE_INSTRUMENT) return;' in s.split('SendMidiCC(int cc, int val)')[1].split('UpdateDisplay')[0]
assert 'RefreshNiFeedback' not in s and 'RefreshNiFeedback' not in h
assert 'case 0x30:' not in s
assert 'SOUNDFIRST_EXIT_INSTRUMENT' in m
assert '[NI_ACTION] Exit received by REAPER' in m
assert 'void CSurf_SoundFirst::ExitInstrumentMode()' in s
assert '[NI_EXIT_TRACK]' in s
assert 'bool m_double_track_exit = true;' in h
assert 'SOUNDFIRST_CAPTURE_NI_HID' in m
assert 'bool m_hold_undo_exit = false;' in h
assert '[NI_CAPTURE]' in s
print('Source integration guards passed (not runtime/hardware validation)')

# Source reachability checks supplement the exhaustive protocol routing matrix.
handler = s.split('void CSurf_SoundFirst::HandleNiCommand')[1].split('void CSurf_SoundFirst::AdoptHidBaseline')[0]
for forbidden in ['Main_OnCommand','CSurf_OnPlay','CSurf_OnRecord','CSurf_OnStop','CSurf_OnVolumeChange','CSurf_OnPanChange','CSurf_OnMuteChange','CSurf_OnSoloChange','TrackFX_SetParam']:
    assert forbidden not in handler
assert 'static char last_line' not in s
assert 'soundfirst_ni::DisplayCache m_display_cache' in h
exit_block = s.split('void CSurf_SoundFirst::ExitInstrumentMode()')[1].split('void CSurf_SoundFirst::CaptureInstrumentHid')[0]
assert 'm_display_cache.invalidate()' in exit_block and 'UpdateDisplay()' in exit_block
assert exit_block.index('m_current_mode = MODE_MIXER') < exit_block.index('UpdateDisplay()')
assert 'm_midi_out->Send(0xB0' not in s
assert 'm_midi_out->Send(0xBF' in s
for signature in ['SendMidi(unsigned char', 'SendSysex(unsigned char']:
    block = s.split('void CSurf_SoundFirst::'+signature)[1].split('\n}')[0]
    assert 'allowOutput(' in block and block.index('allowOutput(') < min(block.find('SendMsg(') if 'SendMsg(' in block else len(block),block.find('m_midi_out->Send(') if 'm_midi_out->Send(' in block else len(block))
focus = s.split('void CSurf_SoundFirst::RefreshNiFocus')[1].split('void CSurf_SoundFirst::SetSurfaceSelected')[0]
assert 'if (!m_ni_version) return;' not in focus
assert 'selectedFocus(instrument,id)' in focus and 'SendNiPacket(soundfirst_ni::focus(requested))' in focus
print('r6 pure-handoff, output firewall, focus scan and redraw source checks passed')

handler = s.split('void CSurf_SoundFirst::HandleNiCommand')[1].split('void CSurf_SoundFirst::AdoptHidBaseline')[0]
hid = s.split('void CSurf_SoundFirst::HandleHidReport')[1].split('    // --- VERIFIED A61')[0]
assert (r/'tests/routing_source.inc').read_text() == 'void CSurf_SoundFirst::HandleNiCommand'+handler+'\nvoid CSurf_SoundFirst::HandleHidReport'+hid+'\n}\n'
print('Mocked routing harness uses exact production DAW handler and Instrument HID branch')

assert 'bool m_double_mode_exit = true;' in h
assert 'risingBit(data[3],old[3],0x01)' in s
assert 'DoubleModeExit' in s and 'DoubleModeWindowMs' in s
assert '[NI_EXIT_MODE_SELECTOR]' in s
assert 'm_track_press.reset(); m_mode_press.reset();' in s
print('r7 mode-selector01 and legacy TRACK04 independent exit configuration verified')

assert 'RefreshNiFocus(true)' in exit_block
assert 'm_ni_sent_focus' in h and 'm_ni_focus_sent' in h
assert 'NI_FOCUS_SCAN' in s and 'NI_FOCUS_SENT' in s
print('r8 detected/transmitted focus separated; exit withdraws link')
f=s.split('void CSurf_SoundFirst::RefreshNiFocus')[1].split('void CSurf_SoundFirst::ReportNiConnect')[0]
assert (r/'tests/focus_source.inc').read_text()=='void CSurf_SoundFirst::RefreshNiFocus'+f
print('r8 mocked focus test runs exact production scan/transmission branch')

assert 'MaybeRecoverNiHello();' in s
assert 'NI DAW not connected' not in s
assert 'NI_MIDI_RX' in s and 'm_ni_diagnostic_packets<24' in s
assert 'if (m_midi_out && (!m_ni_focus_sent' in focus
print('r9 pre-ACK focus, bounded recovery and bounded DAW diagnostics present')

assert 'GetLocalTime' not in s
assert 'if (present && suitable(name)) LogDebug' in s
assert 'm_midi_out->Send(s,d1,d2,-1)' in s
assert 'if (packet.size()==3 && packet[0]==0xBF)' in s
print('r9b compact untimestamped logging and direct CC Send verified')

assert '[NI_CONNECT]' in s and 'ReportNiConnect(true)' in s
assert 'm_ni_recovery.attempts()>=3' in s
assert 'Press Browser twice to leave Instrument mode.' in s and 'SpeakText(status.c_str())' not in s
assert 'NI handshake not confirmed. Use reconnect.' not in s
print('r10 pending vs final-timeout voice and compact connection summary verified')

assert 'Press Plugin, then Browser to control Komplete Kontrol.' in s
assert 'press Shift and Browser, choose Instrument with the encoder and press it. Then press Plugin, followed by Browser.' in s
print('r10b action guidance voice, technical status logged, actual selector flow retained')
