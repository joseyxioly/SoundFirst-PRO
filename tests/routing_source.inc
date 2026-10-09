void CSurf_SoundFirst::HandleNiCommand(unsigned char command, unsigned char value) {
    if (command == 1) {
        if (!value) { LogDebug("[NI_ACK] zero version ignored"); return; }
        m_ni_version = value;
        m_ni_final_timeout_spoken=false;
        m_ni_focus_sent=false; // Re-announce desired focus after negotiation.
        m_ni_session_state = "confirmed";
        if (m_current_mode != MODE_INSTRUMENT) m_display_cache.invalidate();
        LogDebug(("[NI_ACK] version=" + std::to_string(value)).c_str());
        SendNiPacket(soundfirst_ni::sysex(3,1,0,"track_orientation"));
        RefreshNiFocus(true); m_ni_feedback_dirty = true;
        return;
    }
    // No DAW command mutates REAPER in Instrument. Outside Instrument the
    // existing HID handlers own actions, so MIDI cannot duplicate them either.
    if (soundfirst_ni::allowDawAction(m_current_mode == MODE_INSTRUMENT,m_ni_version != 0,command)) {
        // The current routing policy deliberately never reaches this branch.
        return;
    }
    if (m_current_mode == MODE_INSTRUMENT && !m_native_notice) {
        m_native_notice=true;
        char diagnostic[96];
        snprintf(diagnostic,sizeof(diagnostic),"[NI_DAW_IGNORED] command=%02X value=%02X; no SoundFirst action",command,value);
        LogDebug(diagnostic);
    }
}



void CSurf_SoundFirst::HandleHidReport(unsigned char* data) {
    ResetIdleTimer(); // Activity detected
    unsigned char* old = m_last_hid_report;
    if (!m_hid_baseline) {
        // The keyboard only sends a report when something changes, so the
        // first report after connecting is usually a button press. Take knob
        // and encoder positions from it, but compare buttons (bytes 1-5)
        // against "nothing held" so that first press is not swallowed.
        AdoptHidBaseline(data);
        memset(m_last_hid_report + 1, 0, 5);
    }
    if (m_current_mode == MODE_INSTRUMENT) {
        if (m_shift_track_exit && soundfirst_ni::shiftTrack(data[3],old[3],data[1])) {
            LogDebug("[NI_EXIT_SHIFT_TRACK] Shift+TRACK observed; exit requested; NI can also switch instance");
            ExitInstrumentMode(); AdoptHidBaseline(data); return;
        }
        if (m_hold_undo_exit) {
            if (soundfirst_ni::risingBit(data[1],old[1],0x08)) {
                m_undo_pressed_at=m_hid_observed;
                LogDebug("[NI_EXIT_UNDO] press observed; hold >=1 second and release");
            }
            if (!(data[1]&0x08) && (old[1]&0x08) && m_undo_pressed_at >= 0) {
                bool held=soundfirst_ni::heldEnough(m_undo_pressed_at,m_hid_observed,1.0);
                m_undo_pressed_at=-1;
                if (held) {
                    LogDebug("[NI_EXIT_UNDO] held and released; exit requested");
                    ExitInstrumentMode(); AdoptHidBaseline(data); return;
                }
                LogDebug("[NI_EXIT_UNDO] release before 1 second; no exit");
            }
        }
        if (data[3] != old[3]) {
            char diagnostic[96];
            snprintf(diagnostic,sizeof(diagnostic),"[NI_HID_MODE] byte3 old=%02X new=%02X (Browser01 Plugin02 Track04)",old[3],data[3]);
            LogDebug(diagnostic);
        }
        // The mode selector is the original Browser bit01, distinct from
        // the Track-to-Mixer bit04. Each gesture has an independent timer.
        // Both presses still reach native NI Browser; they are not consumed.
        if (soundfirst_ni::risingBit(data[3],old[3],0x01)) {
            if (!m_double_mode_exit) {
                LogDebug("[NI_EXIT_MODE_SELECTOR] byte3 mask01 press; double exit disabled");
            } else if (m_mode_press.press(m_hid_observed,m_double_mode_window)) {
                LogDebug("[NI_EXIT_MODE_SELECTOR] byte3 mask01 second press in window; exit requested");
                ExitInstrumentMode(); AdoptHidBaseline(data); return;
            } else {
                LogDebug("[NI_EXIT_MODE_SELECTOR] byte3 mask01 first press or window expired; waiting for second");
            }
        }
        // Legacy double TRACK remains independent, with its original bit04.
        // NI also receives those presses and may change native context.
        if (soundfirst_ni::risingBit(data[3], old[3], 0x04)) {
            if (!m_double_track_exit) {
                LogDebug("[NI_EXIT_TRACK] byte3 mask04 TRACK-to-Mixer press; double exit disabled in INI");
            } else if (m_track_press.press(m_hid_observed, m_double_track_window)) {
                LogDebug("[NI_EXIT_TRACK] byte3 mask04 second press in window; exit requested");
                ExitInstrumentMode();
                AdoptHidBaseline(data);
                return;
            } else {
                LogDebug("[NI_EXIT_TRACK] byte3 mask04 first press or previous window expired; waiting for second");
            }
        }
        AdoptHidBaseline(data);
        return;
    }
    

}
