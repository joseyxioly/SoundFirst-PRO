#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../src/ni_bridge_protocol.h"
#include <cassert>
#include <iostream>
using namespace soundfirst_ni;
int main() {
    assert(hello() == Packet({0xBF, 1, 4}));
    assert(goodbye() == Packet({0xBF, 2, 0}));
    assert(relative(0) == 0 && relative(63) == 63);
    assert(relative(64) == -64 && relative(127) == -1);
    assert(validInstance("NIKK1234") && validInstance("NIKontakt42"));
    assert(!validInstance("NIK1") && !validInstance("NIKK") && !validInstance("Kontakt42"));
    assert(!validInstance("NIKK12x") && !validInstance("NIKK1"));
    Packet expected{0xF0,0,0x21,9,0,0,0x44,0x43,1,0,0x41,0,0,'N','I','K','K','1','2',0xF7};
    assert(focus("NIKK12") == expected);
    assert(focus("bad") == sysex(0x41,0,0));
    assert(selectedFocus(true,"NIKK12")=="NIKK12");
    assert(selectedFocus(false,"NIKK12").empty());
    assert(selectedFocus(true,"bad").empty());
    assert(validSysex(expected.data(), expected.size()));
    assert(!validSysex(expected.data(), 3));
    auto bad = expected; bad[2] = 0x22; assert(!validSysex(bad.data(),bad.size()));
    bad = expected; bad.back() = 0; assert(!validSysex(bad.data(),bad.size()));
    bad = expected; bad[12] = 0xFF; assert(!validSysex(bad.data(),bad.size()));
    Command c; auto h = hello(); assert(decodeCc(h.data(),h.size(),c));
    assert(c.number == 1 && c.value == 4);
    assert(!decodeCc(nullptr,0,c) && !decodeCc(h.data(),2,c));
    auto padded=h;padded.push_back(0);assert(decodeCc(padded.data(),padded.size(),c));
    bad = h; bad[0] = 0xB0; assert(!decodeCc(bad.data(),bad.size(),c));
    bad = h; bad[2] = 255; assert(!decodeCc(bad.data(),bad.size(),c));
    assert(!useCustomHid(true) && useCustomHid(false));
    // Exhaustive DAW command/mode/connection ownership matrix.
    for (bool instrument : {false,true}) for (bool connected : {false,true})
        for (int command=0; command<128; ++command)
            assert(!allowDawAction(instrument,connected,command));
    // Every output CC is suppressed in Instrument except session negotiation.
    for (int command=0; command<128; ++command) {
        Packet cc{0xBF,(unsigned char)command,1};
        assert(allowOutput(false,cc.data(),cc.size()));
        assert(allowOutput(true,cc.data(),cc.size()) == (command==1 || command==2));
        Packet sx=sysex(command,0,0);
        assert(allowOutput(false,sx.data(),sx.size()));
        assert(allowOutput(true,sx.data(),sx.size()) == (command==3 || command==0x41));
    }
    assert(allowOutput(true,hello().data(),3));
    auto focusPacket=focus("NIKK12");
    assert(allowOutput(true,focusPacket.data(),focusPacket.size()));
    Packet custom{0xB0,5,1};
    assert(!allowOutput(true,custom.data(),custom.size()));
    assert(!allowOutput(true,nullptr,0));
    DisplayCache cache;
    cache.line1[0]='x'; cache.line2_name[0]='y'; cache.line2_value[0]='z';
    cache.screensaver=true; cache.selector=true; cache.track_available=1; cache.bank_mapping=0;
    cache.invalidate();
    assert(!cache.line1[0] && !cache.line2_name[0] && !cache.line2_value[0]);
    assert(!cache.screensaver && !cache.selector);
    assert(cache.track_available==-1 && cache.bank_mapping==-1);
    assert(volumeFeedback(0) == 0 && volumeFeedback(100) == 127);
    assert(panFeedback(-1) == 0 && panFeedback(1) == 127);
    assert(connectionPhase(true,true,false,1,true,true)=="confirmed");
    assert(connectionPhase(false,false,false,0,true,true)=="ports unavailable");
    assert(connectionPhase(true,false,false,1,true,true)=="awaiting ACK");
    assert(connectionPhase(true,false,false,3,true,true)=="awaiting ACK");
    assert(connectionPhase(true,false,true,3,true,true)=="attempts exhausted");
    assert(connectionPhase(true,false,true,1,true,false)=="recovery paused: no NI ID");
    assert(connectionPhase(true,false,true,1,false,true)=="recovery paused: custom mode");
    assert(connectionPhase(true,false,true,2,true,true)=="recovery eligible");
    HelloRecovery recovery;
    assert(!recovery.due(100,true,true,false,true)); // no initial request
    recovery.sent(10);
    assert(!recovery.due(12.99,true,true,false,true));
    assert(!recovery.due(13,false,true,false,true));
    assert(!recovery.due(13,true,false,false,true));
    assert(!recovery.due(13,true,true,true,true));
    assert(!recovery.due(13,true,true,false,false));
    assert(recovery.due(13,true,true,false,true)); recovery.sent(13);
    for(int i=0;i<20;++i) assert(!recovery.due(13.5,true,true,false,true));
    assert(recovery.due(16,true,true,false,true)); recovery.sent(16);
    assert(recovery.attempts()==3 && !recovery.due(999,true,true,false,true));
    recovery.reset(); assert(recovery.attempts()==0);
    for(const std::string id : {"NIKB00","NIKB01","NIKB02"}) {
        assert(validInstance(id) && selectedFocus(true,id)==id);
        assert(selectedFocus(false,id).empty());
    }
    DoublePress doubleTrack;
    assert(!doubleTrack.press(1.0,0.4));
    assert(doubleTrack.press(1.3,0.4));
    assert(!doubleTrack.press(1.4,0.4));
    assert(!doubleTrack.press(2.0,0.4));
    doubleTrack.reset();
    assert(!doubleTrack.press(2.1,0.4));
    assert(doubleTrack.press(2.3,0.4));
    assert(!doubleTrack.press(3.0,0.4));
    assert(!doubleTrack.press(2.0,0.4)); // clock rollback is not a double press
    // Exercise the actual edge predicate shared with the Instrument HID handler.
    auto exits = [](const std::vector<unsigned char>& reports) {
        DoublePress gesture;
        unsigned char previous = 0;
        int count = 0;
        double time = 10;
        for (auto current : reports) {
            if (risingBit(current, previous, 4) && gesture.press(time,0.4)) ++count;
            previous = current;
            time += 0.05;
        }
        return count;
    };
    assert(exits({0,4,0}) == 0); // one press and release, not a double
    assert(exits({0,4,4,0,0}) == 0); // duplicate reports do not add presses
    assert(exits({0,4,0,4,0}) == 1); // two real press edges
    assert(exits({0,4,4,4,4}) == 0); // indistinguishable press/release reports
    assert(exits({0,4,0,0,0,0,0,0,0,0,0,4,0}) == 0); // outside window
    for (unsigned char mask : {1,2,4,8,16,32,64,128}) {
        assert(risingBit(mask,0,mask));
        assert(!risingBit(0,mask,mask));
        assert(!risingBit(mask,mask,mask));
    }
    for (unsigned char command : {0x30,0x31,0x32,0x33,0x34,0x35,0x36})
        assert(nativeNavigation(command));
    assert(!nativeNavigation(0x10) && !nativeNavigation(0x12));
    assert(heldEnough(1.0,2.1,1.0));
    assert(!heldEnough(1.0,1.9,1.0));
    assert(!heldEnough(-1,5.0,1.0));
    assert(!heldEnough(5.0,1.0,1.0));
    assert(shiftTrack(4,0,1));
    assert(!shiftTrack(4,0,0));
    assert(!shiftTrack(0,4,1));
    assert(!shiftTrack(4,4,1));
    // 4-bit encoder counter: steps and 15 <-> 0 wrap (A25 source, M32 capture).
    assert(encoderDelta(2,1)==1 && encoderDelta(1,2)==-1);
    assert(encoderDelta(0,15)==1 && encoderDelta(15,0)==-1);
    assert(encoderDelta(4,1)==3 && encoderDelta(1,4)==-3);
    assert(encoderDelta(0,255)==1 && encoderDelta(255,0)==-1);
    std::cout << "NI protocol and action ownership tests passed\n";
}
