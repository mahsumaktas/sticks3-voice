#include "voice_ui.h"
#include "assets/ui_assets.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iterator>

namespace voice_ui {
namespace {
using assets::Label;
struct Palette { uint32_t bg, ink, muted, line, surface, accent, pill, green; };
constexpr Palette Warm{0xFAF4EA,0x382B28,0x7B6154,0xE9DFD1,0xF0E4D5,0x97482B,0xF4DFC9,0x356751};
constexpr Palette Cool{0x112329,0xE9F5EF,0xA4BFB7,0x294047,0x193139,0xB1E9D0,0x24443F,0xA3E8BC};
uint16_t pack(uint32_t c) { return ((c >> 8) & 0xF800) | ((c >> 5) & 0x7E0) | ((c >> 3) & 31); }
uint32_t unpack(uint16_t c) {
    const uint32_t r = (c >> 11) & 31, g = (c >> 5) & 63, b = c & 31;
    return (((r << 3) | (r >> 2)) << 16) | (((g << 2) | (g >> 4)) << 8) | (b << 3) | (b >> 2);
}
int index(Label value) { return static_cast<int>(value); }
}

void Renderer::pixel(int x, int y, uint32_t color, uint8_t alpha) {
    if (x < 0 || x >= Width || y < 0 || y >= Height || alpha == 0) return;
    auto& dst = pixels_[y * Width + x];
    if (alpha != 255) {
        const uint32_t old = unpack(dst);
        const uint32_t r = (((color >> 16) & 255) * alpha + ((old >> 16) & 255) * (255-alpha) + 127) / 255;
        const uint32_t g = (((color >> 8) & 255) * alpha + ((old >> 8) & 255) * (255-alpha) + 127) / 255;
        const uint32_t b = ((color & 255) * alpha + (old & 255) * (255-alpha) + 127) / 255;
        color = (r << 16) | (g << 8) | b;
    }
    dst = pack(color);
}

void Renderer::box(float x, float y, float w, float h, float r, uint32_t color) {
    for (int py = std::max(0, static_cast<int>(y)); py < std::min(Height, static_cast<int>(std::ceil(y+h))); ++py) {
        for (int px = std::max(0, static_cast<int>(x)); px < std::min(Width, static_cast<int>(std::ceil(x+w))); ++px) {
            const float dx = std::max(std::abs(px+.5f-x-w*.5f) - (w*.5f-r), 0.f);
            const float dy = std::max(std::abs(py+.5f-y-h*.5f) - (h*.5f-r), 0.f);
            const float coverage = std::clamp(r+.5f-std::sqrt(dx*dx+dy*dy), 0.f, 1.f);
            pixel(px, py, color, static_cast<uint8_t>(coverage * 255));
        }
    }
}

void Renderer::ellipse(float cx, float cy, float rx, float ry, uint32_t color) {
    for (int y = std::max(0, static_cast<int>(cy-ry-1)); y <= std::min(Height-1, static_cast<int>(cy+ry+1)); ++y)
        for (int x = std::max(0, static_cast<int>(cx-rx-1)); x <= std::min(Width-1, static_cast<int>(cx+rx+1)); ++x) {
            const float dx=(x+.5f-cx)/rx, dy=(y+.5f-cy)/ry;
            const float a=std::clamp((1.f-std::sqrt(dx*dx+dy*dy))*std::min(rx,ry)+.5f,0.f,1.f);
            pixel(x,y,color,static_cast<uint8_t>(a*255));
        }
}

void Renderer::mask(int x, int y, int w, int h, const uint8_t* data, uint32_t color) {
    for (int row=0; row<h; ++row)
        for (int col=0; col<w; ++col) pixel(x+col,y+row,color,data[row*w+col]);
}

void Renderer::label(int id, int x, int y, uint32_t color) {
    const auto& m=assets::labels[id];
    mask(x,y,m.width,m.height,m.data,color);
}

void Renderer::mascot(const View& view, int x, int y) {
    // Original geometric characters, deliberately independent of vendor mascots.
    const bool blink = view.state != State::Speaking && view.ms % 4300 >= 4050;
    const bool talking = view.state == State::Speaking;
    if (!view.codex) {
        // Sprig: a seed with two leaves, little shoes, and a curved seed-shell seam.
        box(x+29,y+7,3,15,1.5f,0x55735A);
        ellipse(x+23,y+10,10,5,0x729571);
        ellipse(x+38,y+5,8,5,0x55735A);
        ellipse(x+29,y+62,8,4,0x80503B);
        ellipse(x+49,y+62,8,4,0x80503B);
        ellipse(x+39,y+39,26,25,0xCC8153);
        ellipse(x+35,y+35,21,21,0xE9AE70);
        ellipse(x+20,y+42,6,3,0xDA8B63);
        ellipse(x+55,y+42,6,3,0xDA8B63);
        box(x+27,y+29,4,blink?1.5f:7,2,0x473126);
        box(x+44,y+29,4,blink?1.5f:7,2,0x473126);
        if (talking) ellipse(x+38,y+43,3,4,0x80503B);
        else { ellipse(x+38,y+41,5,4,0x80503B); ellipse(x+38,y+39,5,3,0xE9AE70); }
    } else {
        // Orbit: a tilted moon-ring above a soft capsule with three small thrusters.
        ellipse(x+39,y+9,20,5,0x6BADA7);
        ellipse(x+39,y+8,15,2.5f,0x193139);
        ellipse(x+19,y+35,8,13,0x6BADA7);
        ellipse(x+59,y+35,8,13,0x6BADA7);
        box(x+20,y+17,39,42,17,0xB1E9D0);
        box(x+25,y+25,29,22,10,0x24443F);
        box(x+31,y+32,3,blink?1.5f:6,1.5f,0xE9F5EF);
        box(x+45,y+32,3,blink?1.5f:6,1.5f,0xE9F5EF);
        if (talking) ellipse(x+39.5f,y+42,2,2,0xE9F5EF);
        box(x+28,y+61,4,6,2,0x6BADA7);
        box(x+37,y+61,4,9,2,0xB1E9D0);
        box(x+46,y+61,4,6,2,0x6BADA7);
        ellipse(x+61,y+12,3,3,0xD2C8A7);
    }
}

void Renderer::render(const View& view) {
    const auto& p=view.codex?Cool:Warm;
    const bool speaking=view.state==State::Speaking;
    const bool held=speaking || view.state==State::WaitingForAudio || view.state==State::Timeout;
    const bool connected=view.usb_connected && view.state!=State::Disconnected;
    const bool error=view.state==State::Error;
    const uint32_t warning=view.codex?0xFFC29E:0xA63B30;
    if (!speaking || previous_!=State::Speaking) {
        std::fill(std::begin(levels_),std::end(levels_),0.f);
        envelope_=0;
    }
    if (speaking) {
        const float sample=std::sqrt(std::min<uint32_t>(view.peak,24000)/24000.f);
        envelope_=std::max(sample,envelope_*.76f);
        for (int i=0;i<15;++i) levels_[i]=levels_[i+1];
        levels_[15]=envelope_;
    }
    previous_=view.state;
    std::fill(std::begin(pixels_),std::end(pixels_),pack(p.bg));
    auto text=[&](Label id,int x,int y,uint32_t c){label(index(id),x,y,c);};

    // Header: mode identity and physical USB status, not an inferred AI status.
    text(view.codex?Label::Codex:Label::Claude,12,10,p.ink);
    const int pillX=connected?176:151;
    box(pillX,10,228-pillX,18,9,p.surface);
    ellipse(pillX+10,19,2.5f,2.5f,connected?p.green:p.muted);
    text(connected?Label::Usb:Label::NoUsb,pillX+18,15,connected?p.green:p.muted);
    box(12,33,216,1,0.5f,p.line);

    // Float by at most two pixels in a slow four-second cycle.
    const float phase=(view.ms%4200)/4200.f*6.2831853f;
    const int bob=error?0:static_cast<int>(std::round(std::sin(phase)*1.5f));
    ellipse(48,75,35,31,p.surface);
    ellipse(49,106,21+std::sin(phase),2,p.line);
    mascot(view,10,35+bob);
    if (speaking && envelope_>.05f) {
        const float r=2.f+envelope_*2.5f;
        ellipse(88,53,r,r,p.accent);
        ellipse(7,79,1.5f,1.5f,p.accent);
    } else if (view.state==State::Ready) {
        box(83,45,1,5,.5f,p.accent);box(81,47,5,1,.5f,p.accent);
    }

    Label title=Label::Hello, hint=Label::HoldHint;
    switch(view.state) {
        case State::Disconnected: title=Label::Connect;hint=Label::ConnectHint;break;
        case State::Speaking: title=Label::Talk;hint=Label::OpenHint;break;
        case State::WaitingForAudio: title=Label::Wait;hint=Label::WaitHint;break;
        case State::Released: title=Label::Done;hint=Label::ClosedHint;break;
        case State::Timeout: title=Label::Timeout;hint=Label::TimeoutHint;break;
        case State::Error: title=Label::Error;hint=Label::ErrorHint;break;
        case State::Ready: break;
    }
    text(title,101,44,error?warning:p.ink);
    text(hint,102,71,p.muted);
    if (speaking) {
        for(int i=0;i<16;++i) {
            const float h=2.f+levels_[i]*16.f;
            box(102+i*5.5f,97-h*.5f,3,h,1.5f,p.accent);
        }
        const unsigned seconds=std::min<uint32_t>(view.elapsed_ms/1000,90);
        char timer[6];std::snprintf(timer,sizeof(timer),"%u:%02u",seconds/60,seconds%60);
        for(int i=0;timer[i];++i) {
            const int n=timer[i]==':'?10:timer[i]-'0';
            mask(201+i*7,91,7,12,assets::digits+n*7*12,p.muted);
        }
    } else if (view.state==State::Ready || view.state==State::Released) {
        const Label tag=view.codex?Label::Voice:Label::Draft;
        const int w=assets::labels[index(tag)].width+16;
        box(101,90,w,16,8,p.pill);
        text(tag,109,94,p.accent);
    } else {
        // Quiet connection / warning indicator; never a fake audio waveform.
        for(int i=0;i<3;++i) ellipse(106+i*8,98,1.5f,1.5f,error?warning:p.muted);
    }

    box(0,113,240,22,0.5f,p.surface);
    box(12,119,11,11,3,held?p.accent:p.muted);
    ellipse(17.5f,124.5f,2,2,p.bg);
    const Label action=error?Label::Reconnect:(view.state==State::Timeout?Label::Retry:(held?Label::Release:Label::Hold));
    text(action,30,120,p.ink);
    const Label next=view.codex?Label::NextClaude:Label::NextCodex;
    const int nextWidth=assets::labels[index(next)].width;
    const uint32_t nextColor=(held || !connected || error)?p.muted:p.accent;
    text(next,216-nextWidth,120,nextColor);
    // Right-side button pictogram.
    box(223,119,5,11,2,nextColor);
    box(230,122,2,5,1,nextColor);
}
}
