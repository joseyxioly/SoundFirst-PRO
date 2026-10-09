#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../src/ni_bridge_protocol.h"
#include <cassert>
#include <cstring>
struct MediaTrack { const char* id; };
MediaTrack first{"NIKK12"}, second{"NIKK34"};
double clockSeconds=0;
double time_precise(){return clockSeconds;}
MediaTrack* selected=&first;
MediaTrack* GetSelectedTrack(void*,int){return selected;}
int TrackFX_GetCount(MediaTrack*){return 1;}
int TrackFX_GetNumParams(MediaTrack*,int){return 1;}
bool TrackFX_GetParamName(MediaTrack* t,int,int,char* name,int len){std::strncpy(name,t->id,len);return true;}
class CSurf_SoundFirst {
public:
 enum {MODE_MIXER,MODE_FX,MODE_MIDI,MODE_AUDIO,MODE_INSTRUMENT};
 int m_current_mode=MODE_MIXER;
 unsigned char m_ni_version=3;
 bool m_midi_out=true,m_ni_input=true;
 soundfirst_ni::HelloRecovery m_ni_recovery;
 double m_ni_started=0,m_ni_diagnostic_until=0;
 bool m_ni_timeout_announced=false;
 std::string m_ni_session_state;
 void MaybeRecoverNiHello();
 MediaTrack* m_ni_focus_track=nullptr;
 std::string m_ni_last_focus,m_ni_sent_focus,m_ni_last_scan_log,m_ni_last_unavailable_log;
 bool m_ni_focus_sent=false,m_ni_focus_instrument=false;
 std::vector<soundfirst_ni::Packet> sent;
 void RefreshNiFocus(bool force=false);
 std::string FindNiInstance(MediaTrack* track);
 void LogDebug(const char*){}
 void SendNiPacket(const soundfirst_ni::Packet& p){sent.push_back(p);}
};
#include "focus_source.inc"
int main(){
 CSurf_SoundFirst s;
 for(int mode:{s.MODE_MIXER,s.MODE_FX,s.MODE_MIDI,s.MODE_AUDIO}){
  s.m_current_mode=mode;selected=&first;s.RefreshNiFocus(true);
  assert(s.sent.back()==soundfirst_ni::focus(""));
  selected=&second;s.RefreshNiFocus();
  assert(s.m_ni_last_focus=="NIKK34" && s.m_ni_sent_focus.empty());
  for(auto& p:s.sent)assert(p==soundfirst_ni::focus(""));
 }
 s.m_current_mode=s.MODE_INSTRUMENT;s.RefreshNiFocus();
 assert(s.sent.back()==soundfirst_ni::focus("NIKK34"));
 selected=&first;s.RefreshNiFocus();assert(s.sent.back()==soundfirst_ni::focus("NIKK12"));
 s.m_current_mode=s.MODE_MIXER;s.RefreshNiFocus(true);
 assert(s.sent.back()==soundfirst_ni::focus(""));
 auto count=s.sent.size();s.RefreshNiFocus(true);s.RefreshNiFocus(true);assert(s.sent.size()==count);
 selected=&second;s.RefreshNiFocus();
 assert(s.sent.size()==count && s.m_ni_sent_focus.empty());
 s.m_ni_version=0;s.m_ni_focus_sent=false;s.sent.clear();s.RefreshNiFocus(true);
 assert(s.sent.back()==soundfirst_ni::focus("") && s.m_ni_last_focus=="NIKK34");
 s.m_current_mode=s.MODE_INSTRUMENT;s.RefreshNiFocus();
 assert(s.sent.back()==soundfirst_ni::focus("NIKK34"));
 s.m_current_mode=s.MODE_MIXER;s.RefreshNiFocus();
 assert(s.sent.back()==soundfirst_ni::focus(""));
 s.m_ni_version=3;s.m_ni_focus_sent=false;s.RefreshNiFocus(true); // ACK in Mixer
 assert(s.sent.back()==soundfirst_ni::focus(""));
 CSurf_SoundFirst retry;retry.m_ni_version=0;retry.m_current_mode=retry.MODE_INSTRUMENT;
 retry.m_ni_last_focus="NIKB01";retry.m_ni_recovery.sent(0);
 clockSeconds=2.9;retry.MaybeRecoverNiHello();assert(retry.sent.empty());
 clockSeconds=3;retry.MaybeRecoverNiHello();assert(retry.sent.size()==1 && retry.sent.back()==soundfirst_ni::hello());
 for(int i=0;i<20;++i){retry.m_ni_last_focus=i%2 ? "NIKB01":"NIKB02";retry.MaybeRecoverNiHello();}
 assert(retry.sent.size()==1);
 clockSeconds=6;retry.MaybeRecoverNiHello();assert(retry.sent.size()==2);
 clockSeconds=100;retry.MaybeRecoverNiHello();assert(retry.sent.size()==2);
 retry.m_ni_recovery.reset();retry.m_ni_recovery.sent(0);retry.m_ni_version=3;
 retry.MaybeRecoverNiHello();assert(retry.sent.size()==2);
 retry.m_ni_version=0;retry.m_current_mode=retry.MODE_MIXER;
 retry.MaybeRecoverNiHello();assert(retry.sent.size()==2);
}
