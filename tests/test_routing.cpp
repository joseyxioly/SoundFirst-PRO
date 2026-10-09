#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../src/ni_bridge_protocol.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
class CSurf_SoundFirst {
public:
 enum { MODE_MIXER, MODE_INSTRUMENT };
 int m_current_mode=MODE_INSTRUMENT;
 unsigned char m_ni_version=0;
 std::string m_ni_session_state;
 soundfirst_ni::DisplayCache m_display_cache;
 bool m_ni_final_timeout_spoken=false;
 bool m_native_notice=false,m_ni_focus_sent=false;
 bool m_ni_feedback_dirty=false, m_hid_baseline=true;
 bool m_double_mode_exit=true;
 double m_double_mode_window=.4;
 soundfirst_ni::DoublePress m_mode_press;
 bool m_shift_track_exit=false, m_hold_undo_exit=false, m_double_track_exit=true;
 double m_hid_observed=0, m_undo_pressed_at=-1, m_double_track_window=.4;
 soundfirst_ni::DoublePress m_track_press;
 unsigned char m_last_hid_report[64]={};
 int exits=0, focus=0, custom_actions=0;
 std::vector<soundfirst_ni::Packet> sent;
 void HandleNiCommand(unsigned char,unsigned char);
 void HandleHidReport(unsigned char*);
 void ResetIdleTimer() {}
 void LogDebug(const char*) {}
 void RefreshNiFocus(bool) { ++focus; }
 void SendNiPacket(const soundfirst_ni::Packet& p) {sent.push_back(p);}
 void AdoptHidBaseline(const unsigned char* data) { memcpy(m_last_hid_report,data,64); m_hid_baseline=true; }
 void ExitInstrumentMode() {++exits; m_track_press.reset(); m_mode_press.reset(); m_current_mode=MODE_MIXER; m_display_cache.invalidate();}
};
#include "routing_source.inc"
int main() {
 CSurf_SoundFirst surface;
 for (int mode : {CSurf_SoundFirst::MODE_MIXER,CSurf_SoundFirst::MODE_INSTRUMENT})
  for (unsigned char version : {0,3}) {
   surface.m_current_mode=mode; surface.m_ni_version=version;
   for(int command=0;command<128;++command) if(command!=1)
    for(int value : {0,1,63,64,127}) surface.HandleNiCommand(command,value);
   assert(surface.sent.empty() && surface.focus==0 && surface.custom_actions==0);
  }
 surface.m_current_mode=CSurf_SoundFirst::MODE_INSTRUMENT;
 surface.HandleNiCommand(1,0); assert(surface.sent.empty());
 surface.HandleNiCommand(1,3);
 assert(surface.m_ni_version==3 && surface.m_ni_session_state=="confirmed");
 assert(surface.focus==1 && surface.sent.size()==1 && surface.sent[0][10]==3);
 // Actual Instrument HID branch: unrelated bytes, encoder, knob and button
 // changes only update baseline. All masks and positions are exercised.
 unsigned char report[64]={};
 for (int byte=0;byte<30;++byte) for(int bit=0;bit<8;++bit) {
  memset(report,0,64); surface.HandleHidReport(report);
  report[byte]=1<<bit; surface.m_hid_observed+=1;
  surface.HandleHidReport(report);
  assert(surface.exits==0 && surface.m_current_mode==CSurf_SoundFirst::MODE_INSTRUMENT);
 }
 memset(report,0,64);surface.HandleHidReport(report);
 report[3]=4;surface.m_hid_observed=100;surface.HandleHidReport(report);
 report[3]=0;surface.HandleHidReport(report);
 report[3]=4;surface.m_hid_observed=100.2;surface.HandleHidReport(report);
 assert(surface.exits==1 && surface.m_current_mode==CSurf_SoundFirst::MODE_MIXER);
 assert(surface.m_last_hid_report[3]==4);
 // Mode selector01 now exits on two actual presses, not repeated reports.
 CSurf_SoundFirst selector;
 memset(report,0,64);selector.HandleHidReport(report);
 report[3]=1;selector.m_hid_observed=200;selector.HandleHidReport(report);
 selector.HandleHidReport(report);assert(selector.exits==0);
 report[3]=0;selector.HandleHidReport(report);
 report[3]=1;selector.m_hid_observed=200.2;selector.HandleHidReport(report);
 assert(selector.exits==1 && selector.m_last_hid_report[3]==1);
 // Different buttons cannot combine to make a double click.
 CSurf_SoundFirst mixed;
 memset(report,0,64);mixed.HandleHidReport(report);
 report[3]=1;mixed.m_hid_observed=300;mixed.HandleHidReport(report);
 report[3]=0;mixed.HandleHidReport(report);
 report[3]=4;mixed.m_hid_observed=300.1;mixed.HandleHidReport(report);
 assert(mixed.exits==0);
 // Disabling the mode exit does not alter legacy TRACK settings.
 CSurf_SoundFirst disabled;disabled.m_double_mode_exit=false;
 for(int i=0;i<7;++i) {
  memset(report,0,64);disabled.HandleHidReport(report);
  report[3]=1;disabled.m_hid_observed=i*.1;disabled.HandleHidReport(report);
 }
 assert(disabled.exits==0);
 // The seven01 capture's first pair is within400ms: r7 exits there.
 CSurf_SoundFirst capture;
 memset(report,0,64);capture.HandleHidReport(report);
 report[3]=1;capture.m_hid_observed=7790.34;capture.HandleHidReport(report);
 report[3]=0;capture.m_hid_observed=7790.44;capture.HandleHidReport(report);
 report[3]=1;capture.m_hid_observed=7790.50;capture.HandleHidReport(report);
 assert(capture.exits==1);
}
