#include <M5Unified.h>
#include <atomic>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_log.h"
#include "soc/rtc_cntl_reg.h"
#include "tusb.h"
#include "usb_device_uac.h"
#include "voice_ui.h"

static std::atomic<bool> pressed{false};
static std::atomic<uint32_t> audio_callbacks{0};
static std::atomic<uint32_t> audio_peak{0};
static std::atomic<bool> audio_error{false};
static bool codex_mode = false;
static voice_ui::Renderer display_ui;

static uint32_t show_screen(const voice_ui::View& view) {
    const int64_t start = esp_timer_get_time();
    display_ui.render(view);
    M5.Display.pushImage(0, 0, voice_ui::Width, voice_ui::Height, display_ui.pixels());
    return static_cast<uint32_t>(esp_timer_get_time() - start);
}

static void disable_amplifier() {
    auto &power = M5.Power.M5pm1;
    power.setGPIOOutput(m5::M5PM1_Class::gpio3, false);
    power.setGPIODrive(m5::M5PM1_Class::gpio3, m5::M5PM1_Class::push_pull);
    power.setGPIOMode(m5::M5PM1_Class::gpio3, m5::M5PM1_Class::output);
    // GPIO3's mux occupies bits 6:7, not bit 3 of PM1 register 0x16.
    power.setGPIOFunction(m5::M5PM1_Class::gpio3, m5::M5PM1_Class::gpio);
    // The codec keeps DAC state across ESP resets; silence its output too.
    M5.In_I2C.writeRegister8(0x18, 0x32, 0x00, 100000); // DAC volume = mute.
    M5.In_I2C.writeRegister8(0x18, 0x31, 0x60, 100000); // DAC soft mute.
    M5.In_I2C.writeRegister8(0x18, 0x12, 0x02, 100000); // Power down DAC only.
    M5.In_I2C.writeRegister8(0x18, 0x13, 0x00, 100000); // Disable headphone drive.
}

// Standard 1200-baud USB touch returns to ROM download mode for recovery.
extern "C" void tud_cdc_line_coding_cb(uint8_t, cdc_line_coding_t const *coding) {
    if (coding->bit_rate == 1200) {
        REG_SET_BIT(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
        esp_restart();
    }
}

static esp_err_t microphone(uint8_t *buffer, size_t length, size_t *read, void *) {
    *read = length;
    audio_callbacks++;
    if (!pressed.load()) {
        memset(buffer, 0, length);
        return ESP_OK;
    }
    if (!M5.Mic.record(reinterpret_cast<int16_t *>(buffer), length/2, 48000, false)) {
        memset(buffer, 0, length);
        audio_error = true;
        return ESP_OK;
    }
    const int64_t deadline = esp_timer_get_time() + 100000;
    while (M5.Mic.isRecording()) {
        if (esp_timer_get_time() > deadline) {
            // Keep the buffer owned until the DMA task has been stopped.
            M5.Mic.end();
            memset(buffer, 0, length);
            audio_error = true;
            return ESP_OK;
        }
        vTaskDelay(1);
    }
    if (!pressed.load()) memset(buffer, 0, length);
    auto samples = reinterpret_cast<int16_t *>(buffer);
    uint32_t peak = 0;
    for (size_t i=0; i<length/2; ++i) {
        const int32_t n = samples[i];
        const uint32_t amplitude = n < 0 ? -n : n;
        if (amplitude > peak) peak = amplitude;
    }
    audio_peak.store(peak);
    return ESP_OK;
}

extern "C" void app_main(void) {
    ESP_LOGI("Voice", "Starting StickS3 native USB audio");
    auto config = M5.config();
    config.fallback_board = m5::board_t::board_M5StickS3;
    config.external_display_value = 0;
    config.output_power = false;
    config.internal_spk = false;
    config.internal_mic = true;
    config.internal_imu = false;
    config.internal_rtc = false;
    M5.begin(config);
    M5.Power.setExtOutput(false);
    ESP_LOGI("Voice", "M5 ready board=%d display=%dx%d", (int)M5.getBoard(), (int)M5.Display.width(), (int)M5.Display.height());
    M5.Speaker.setVolume(0);
    M5.Speaker.end();
    disable_amplifier();
    M5.Display.setRotation(1);
    M5.Display.setBrightness(80);
    M5.Display.setSwapBytes(true); // Portable renderer stores native-endian RGB565.
    M5.BtnA.setDebounceThresh(25);
    M5.BtnB.setDebounceThresh(40);
    auto mic_config = M5.Mic.config();
    mic_config.sample_rate = 48000;
    mic_config.magnification = 2;
    M5.Mic.config(mic_config);
    if (!M5.Mic.begin()) {
        ESP_LOGE("Voice", "Microphone initialization failed");
        show_screen({false, voice_ui::State::Error, 0, 0, 0, false});
        return;
    }
    disable_amplifier();
    uac_device_config_t audio = {};
    audio.input_cb = microphone;
    audio.spk_itf_num = -1;
    audio.mic_itf_num = 1;
    ESP_LOGI("Voice", "Starting USB stack");
    ESP_ERROR_CHECK(uac_device_init(&audio));
    uint32_t frame_us = show_screen({});
    bool last_pressed = false;
    bool last_mount = false;
    bool sent_pressed = false;
    bool timed_out = false;
    int64_t started = 0;
    int64_t last_log = 0;
    int f8_phase = 0;
    int64_t f8_release_at = 0;
    int64_t next_frame = 0;
    int64_t released_until = 0;
    int64_t last_audio_packet = 0;
    uint32_t last_audio_count = 0;
    voice_ui::State ui_state = voice_ui::State::Disconnected;
    // Read-only framebuffer capture: 'S' on CDC returns a header and RGB565 LE.
    // Transfer is bounded and nonblocking, so USB audio/HID continue normally.
    int capture_offset = -1;
    int64_t capture_started = 0;
    while (true) {
        M5.update();
        const bool mounted = tud_mounted();
        const bool physical = M5.BtnA.isPressed();
        const int64_t now = esp_timer_get_time();
        bool mode_changed = false;
        if (mounted && M5.BtnB.wasPressed() && !physical && !sent_pressed && f8_phase == 0) {
            codex_mode = !codex_mode;
            mode_changed = true;
            // Default binding tested with Codex CLI 0.159.2; configurable in menuconfig.
            // Change mode while the Codex terminal is focused.
            f8_phase = 1;
        }
        if (f8_phase != 0 && tud_hid_ready()) {
            uint8_t keys[6] = {0};
            if (f8_phase == 1) {
                keys[0] = CONFIG_STICKS3_CODEX_KEY;
                if (tud_hid_keyboard_report(0, 0, keys)) {
                    f8_phase = 2;
                    f8_release_at = now + 50000;
                }
            } else if (now >= f8_release_at && tud_hid_keyboard_report(0, 0, keys)) {
                f8_phase = 0;
            }
        }
        if (!physical) timed_out = false;
        if (physical && !last_pressed && !timed_out) started = now;
        if (physical && now - started > int64_t(CONFIG_STICKS3_MAX_HOLD_SECONDS) * 1000000) timed_out = true;
        const bool active = mounted && physical && f8_phase == 0 && !timed_out && !audio_error.load();
        pressed.store(active);
        const bool space_down = active && !codex_mode;
        if (f8_phase == 0 && space_down != sent_pressed && tud_hid_ready()) {
            uint8_t keys[6] = {0};
            if (space_down) keys[0] = CONFIG_STICKS3_CLAUDE_KEY;
            if (tud_hid_keyboard_report(0, 0, keys)) sent_pressed = space_down;
        }
        if (!mounted) { sent_pressed = false; f8_phase = 0; }
        const uint32_t audio_count = audio_callbacks.load();
        if (audio_count != last_audio_count) { last_audio_count = audio_count; last_audio_packet = now; }
        if (active && !last_pressed) audio_peak.store(0);
        if (last_pressed && !active && !physical && mounted) released_until = now + 650000;
        if (mode_changed) released_until = 0;
        const bool streaming = last_audio_packet != 0 && now - last_audio_packet < 250000;
        const voice_ui::State state = audio_error.load() ? voice_ui::State::Error
            : !mounted ? voice_ui::State::Disconnected
            : timed_out ? voice_ui::State::Timeout
            : active ? (streaming ? voice_ui::State::Speaking : voice_ui::State::WaitingForAudio)
            : now < released_until ? voice_ui::State::Released : voice_ui::State::Ready;
        const bool changed = state != ui_state || mode_changed || mounted != last_mount;
        if (capture_offset < 0 && (changed || now >= next_frame)) {
            frame_us = show_screen({codex_mode, state, static_cast<uint32_t>(now / 1000),
                static_cast<uint32_t>(active ? (now - started) / 1000 : 0), active && streaming ? audio_peak.load() : 0, mounted});
            ui_state = state;
            next_frame = now + 67000; // At most 15 fps; button polling remains independent.
        }
        last_pressed = active;
        last_mount = mounted;
        if (tud_cdc_connected() && tud_cdc_available()) {
            uint8_t commands[32];
            const uint32_t size = tud_cdc_read(commands, sizeof(commands));
            for (uint32_t i = 0; i < size; ++i) {
                if (commands[i] == 'S' && capture_offset < 0 && tud_cdc_write_available() >= 32) {
                    tud_cdc_write_str("UIFRAME 240 135 64800 RGB565LE\n");
                    capture_offset = 0;
                    capture_started = now;
                }
            }
        }
        if (capture_offset >= 0) {
            if (!tud_cdc_connected() || now - capture_started > 2000000) capture_offset = -1;
            else {
                constexpr int bytes = voice_ui::Width * voice_ui::Height * sizeof(uint16_t);
                const uint32_t available = tud_cdc_write_available();
                const uint32_t remaining = bytes - capture_offset;
                const uint32_t count = available < remaining ? available : remaining;
                const auto* pixels = reinterpret_cast<const uint8_t*>(display_ui.pixels());
                capture_offset += tud_cdc_write(pixels + capture_offset, count);
                tud_cdc_write_flush();
                if (capture_offset == bytes) capture_offset = -1;
            }
        }
        if (capture_offset < 0 && tud_cdc_connected() && now-last_log > 1000000) {
            uint8_t pm1[8] = {};
            const bool pm1_ok = M5.In_I2C.readRegister(0x6e, 0x10, pm1, sizeof(pm1), 100000);
            char message[384];
            snprintf(message, sizeof(message), "{\"mode\":\"%s\",\"pressed\":%s,\"audio_callbacks\":%lu,\"peak\":%lu,\"error\":%s,\"pm1_ok\":%s,\"pm1\":[%u,%u,%u,%u,%u,%u,%u,%u],\"boost\":%s,\"ui\":\"sticks3-voice\",\"ui_state\":%d,\"frame_us\":%lu}\n",
                codex_mode?"codex":"claude", active?"true":"false", (unsigned long)audio_callbacks.load(), (unsigned long)audio_peak.load(), audio_error?"true":"false",
                pm1_ok?"true":"false", pm1[0],pm1[1],pm1[2],pm1[3],pm1[4],pm1[5],pm1[6],pm1[7], M5.Power.M5pm1.getExtOutput()?"true":"false",
                static_cast<int>(ui_state), static_cast<unsigned long>(frame_us));
            tud_cdc_write_str(message);
            tud_cdc_write_flush();
            last_log = now;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
