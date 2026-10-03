#pragma once
#include <cstdint>

namespace voice_ui {
constexpr int Width = 240;
constexpr int Height = 135;
enum class State { Disconnected, Ready, Speaking, WaitingForAudio, Released, Timeout, Error };
struct View {
    bool codex = false;
    State state = State::Disconnected;
    uint32_t ms = 0;
    uint32_t elapsed_ms = 0;
    uint32_t peak = 0;
    bool usb_connected = true;
};

// Same renderer runs on the ESP32 and in the headless preview executable.
// No heap allocations, font parsing, USB access or audio processing per frame.
class Renderer {
public:
    void render(const View& view);
    const uint16_t* pixels() const { return pixels_; }
private:
    uint16_t pixels_[Width * Height]{};
    float levels_[16]{};
    float envelope_ = 0;
    State previous_ = State::Disconnected;
    void pixel(int x, int y, uint32_t color, uint8_t alpha = 255);
    void box(float x, float y, float w, float h, float radius, uint32_t color);
    void ellipse(float cx, float cy, float rx, float ry, uint32_t color);
    void mask(int x, int y, int w, int h, const uint8_t* data, uint32_t color);
    void label(int id, int x, int y, uint32_t color);
    void mascot(const View& view, int x, int y);
};
}
