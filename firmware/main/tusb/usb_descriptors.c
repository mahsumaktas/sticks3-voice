#include <string.h>
#include <stdio.h>
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tusb.h"
#include "uac_descriptors.h"

enum { AUDIO_CONTROL, AUDIO_MIC, KEYBOARD, CDC_CONTROL, CDC_DATA, INTERFACE_COUNT };
static const tusb_desc_device_t device = {
    .bLength = sizeof(tusb_desc_device_t), .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200, .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON, .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A, .idProduct = 0x8000, .bcdDevice = 0x0100,
    .iManufacturer = 1, .iProduct = 2, .iSerialNumber = 3, .bNumConfigurations = 1,
};
static const uint8_t keyboard_report[] = { TUD_HID_REPORT_DESC_KEYBOARD() };
#define TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_AUDIO_DEVICE_DESC_LEN + TUD_HID_DESC_LEN + TUD_CDC_DESC_LEN)
static const uint8_t configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, INTERFACE_COUNT, 0, TOTAL_LEN, 0, 250),
    TUD_AUDIO_DESCRIPTOR(AUDIO_CONTROL, 4, 0x01, 0x82, 0x81),
    TUD_HID_DESCRIPTOR(KEYBOARD, 6, HID_ITF_PROTOCOL_KEYBOARD, sizeof(keyboard_report), 0x83, 8, 5),
    TUD_CDC_DESCRIPTOR(CDC_CONTROL, 7, 0x84, 8, 0x01, 0x81, 64),
};
_Static_assert(sizeof(configuration) == TOTAL_LEN, "USB descriptor length mismatch");
const uint8_t *tud_descriptor_device_cb(void) { return (const uint8_t *)&device; }
const uint8_t *tud_descriptor_configuration_cb(uint8_t index) { (void)index; return configuration; }
const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance) { (void)instance; return keyboard_report; }
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t type, uint8_t *buffer, uint16_t length) {
    (void)instance; (void)report_id; (void)type; (void)buffer; (void)length; return 0;
}
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t type, const uint8_t *buffer, uint16_t length) {
    (void)instance; (void)report_id; (void)type; (void)buffer; (void)length;
}
const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t result[48];
    // Derive each board's USB identity at runtime. No developer device ID is
    // embedded in source or release binaries; this value stays on the USB link.
    static char serial[21];
    if (!serial[0]) {
        uint8_t mac[6];
        if (esp_efuse_mac_get_default(mac) != ESP_OK) return NULL;
        snprintf(serial, sizeof(serial), "StickS3-%02x%02x%02x%02x%02x%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    const char *strings[] = { "", "StickS3 Voice", "StickS3 Voice", serial, "StickS3 Audio", "StickS3 Microphone", "Push To Talk", "Diagnostics" };
    if (index == 0) { result[0] = (TUSB_DESC_STRING << 8) | 4; result[1] = 0x0409; return result; }
    if (index >= sizeof(strings)/sizeof(strings[0])) return NULL;
    size_t length = strlen(strings[index]);
    if (length > 47) length = 47;
    for (size_t i = 0; i < length; ++i) result[i+1] = strings[index][i];
    result[0] = (TUSB_DESC_STRING << 8) | (2 * length + 2);
    return result;
}
