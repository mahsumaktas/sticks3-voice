#include "voice_ui.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>

using namespace voice_ui;

static std::array<uint16_t, Width*Height> snapshot(const Renderer& renderer) {
    std::array<uint16_t, Width*Height> result{};
    std::copy(renderer.pixels(), renderer.pixels()+result.size(), result.begin());
    return result;
}

int main() {
    struct Guarded { uint64_t before=0x123456789abcdef0ULL; Renderer renderer;
                     uint64_t after=0xfedcba9876543210ULL; } guarded;
    const State states[] = {State::Disconnected, State::Ready, State::Speaking,
        State::WaitingForAudio, State::Released, State::Timeout, State::Error};
    for (bool codex : {false,true}) for (State state : states)
        for (uint32_t peak : {0U,1U,24000U,32768U,std::numeric_limits<uint32_t>::max()})
            for (uint32_t ms : {0U,4050U,4200U,std::numeric_limits<uint32_t>::max()}) {
                View view{codex,state,ms,ms,peak};
                guarded.renderer.render(view);
                assert(guarded.before==0x123456789abcdef0ULL);
                assert(guarded.after==0xfedcba9876543210ULL);
            }
    for (bool codex : {false,true}) {
        View silence{codex,State::Speaking,1200,12000,0};
        Renderer quiet, loud;
        for(int i=0;i<20;++i) { quiet.render(silence); auto v=silence;v.peak=24000;loud.render(v); }
        assert(snapshot(quiet)!=snapshot(loud));
        // Zero input must never animate a fictional waveform.
        const auto stable=snapshot(quiet);
        for(int i=0;i<20;++i) quiet.render(silence);
        assert(stable==snapshot(quiet));
        // Leaving capture resets peak history.
        View ready{codex,State::Ready,1200,0,0};
        quiet.render(ready); loud.render(ready);
        assert(snapshot(quiet)==snapshot(loud));
        // Input peaks saturate without overflowing.
        Renderer clipped, maximum;
        auto v=silence;v.peak=24000;clipped.render(v);
        v.peak=std::numeric_limits<uint32_t>::max();maximum.render(v);
        assert(snapshot(clipped)==snapshot(maximum));
        // Timer saturates at the capture cap.
        Renderer cap, beyond;v.elapsed_ms=90000;cap.render(v);
        v.elapsed_ms=std::numeric_limits<uint32_t>::max();beyond.render(v);
        assert(snapshot(cap)==snapshot(beyond));
        Renderer connected, disconnected;connected.render(ready);
        ready.usb_connected=false;disconnected.render(ready);
        assert(snapshot(connected)!=snapshot(disconnected));
    }
}
