void CSurf_SoundFirst::RefreshNiFocus(bool force) {
    MediaTrack* track = GetSelectedTrack(nullptr, 0);
    std::string id;
    if (track) {
        for (int fx = 0; fx < TrackFX_GetCount(track) && id.empty(); ++fx) {
            const int count = TrackFX_GetNumParams(track, fx);
            for (int parameter : {0, 128, 2048, 4096}) {
                if (parameter >= count) continue;
                char name[128] = {};
                if (TrackFX_GetParamName(track, fx, parameter, name, sizeof(name)) &&
                    soundfirst_ni::validInstance(name)) { id = name; break; }
            }
        }
    }
    const bool instrument = m_current_mode == MODE_INSTRUMENT;
    const std::string requested = soundfirst_ni::selectedFocus(instrument,id);
    const std::string scan="[NI_FOCUS_SCAN] detected="+(id.empty()?std::string("NONE"):id)+
        " routing="+(instrument?"Instrument":"custom");
    if (scan!=m_ni_last_scan_log) { LogDebug(scan.c_str()); m_ni_last_scan_log=scan; }
    if (m_midi_out && (!m_ni_focus_sent || requested != m_ni_sent_focus ||
                      instrument != m_ni_focus_instrument)) {
        SendNiPacket(soundfirst_ni::focus(requested));
        m_ni_sent_focus=requested; m_ni_focus_sent=true;
        LogDebug(("[NI_FOCUS_SENT] "+(requested.empty()?std::string("EMPTY"):requested)+
                  " ACK="+std::to_string(m_ni_version)).c_str());
    } else if (!m_midi_out && m_ni_last_unavailable_log!=scan) {
        LogDebug("[NI_FOCUS] output unavailable; scan only"); m_ni_last_unavailable_log=scan;
    }
    m_ni_focus_track=track; m_ni_last_focus=id;
    m_ni_focus_instrument=instrument;

}

void CSurf_SoundFirst::MaybeRecoverNiHello() {
    const double now=time_precise();
    if (!m_ni_recovery.due(now,m_current_mode==MODE_INSTRUMENT,
                          !m_ni_last_focus.empty(),m_ni_version!=0,
                          m_ni_input && m_midi_out)) return;
    SendNiPacket(soundfirst_ni::hello());
    m_ni_recovery.sent(now);
    m_ni_started=now; m_ni_timeout_announced=false;
    m_ni_session_state="awaiting ACK (bounded retry)";
    m_ni_diagnostic_until=now+4; // packet budget is NOT reset by retry
    LogDebug(("[NI_HELLO_RETRY] BF0104 attempt="+std::to_string(m_ni_recovery.attempts())+
        "/3; Instrument detected="+m_ni_last_focus+"; ports retained").c_str());
}

