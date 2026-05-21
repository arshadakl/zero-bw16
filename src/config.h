#pragma once

// Realtek SDK defines FW_VERSION as 0x0100 — use APP_VERSION instead
#ifdef FW_VERSION
#undef FW_VERSION
#endif
#define APP_VERSION       "1.0.0"
#define PROJECT_NAME      "zero-bw16"

// ---- Realtek AmebaD compat ------------------------------------------------
// min/max not in global namespace on this toolchain
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

// strlcpy missing from Realtek toolchain
#include <string.h>
static inline size_t _zbw_strlcpy(char* d, const char* s, size_t n) {
    size_t l = strlen(s);
    if (n) { size_t c = (l < n-1) ? l : n-1; memcpy(d, s, c); d[c] = '\0'; }
    return l;
}
#define strlcpy _zbw_strlcpy

// strncasecmp missing from Realtek toolchain
#include <ctype.h>
static inline int _zbw_strncasecmp(const char* a, const char* b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        int d = tolower((uint8_t)a[i]) - tolower((uint8_t)b[i]);
        if (d || !a[i]) return d;
    }
    return 0;
}
#define strncasecmp _zbw_strncasecmp

// Serial.printf not available on Realtek AmebaD
#define SERIAL_PRINTF(fmt, ...) do { \
    char _sp[160]; snprintf(_sp, sizeof(_sp), fmt, ##__VA_ARGS__); \
    Serial.print(_sp); } while(0)
// ---------------------------------------------------------------------------

// --- AP defaults ---
#define DEFAULT_AP_SSID   "zero-bw16"
#define DEFAULT_AP_PASS   "zerobw16!"
#define DEFAULT_AP_CHANNEL 6
#define AP_IP             "192.168.1.1"

// --- Attack defaults ---
#define DEFAULT_DEAUTH_FRAMES  5
#define DEFAULT_SEND_DELAY     5
#define DEFAULT_DEAUTH_REASON  2

// --- GPIO ---
#define LED_PIN        PA27  // BW16 user LED (active low)
// Set LED_PIN_GREEN to a real pin number if your board has a second green LED.
// -1 = not present; attack state then uses a 2 Hz blink on the primary LED instead.
#define LED_PIN_GREEN  -1
#define BUTTON_PIN PA12   // optional hardware button

// --- Web server ---
#define WEB_PORT   80
#define SSE_INTERVAL_MS 1000

// --- Limits (tuned for BD_RAM_NS on RTL8720DN) ---
#define MAX_NETWORKS       20
#define MAX_PROBE_ENTRIES  20
#define MAX_CLIENT_ENTRIES 20
#define MAX_PMKID_ENTRIES  10
#define MAX_LOG_ENTRIES    40
#define MAX_BEACON_SSIDS   20

// --- NVS ---
#define NVS_EEPROM_SIZE   256
#define NVS_MAGIC         "ZBW"

// --- OTA ---
#define OTA_CHUNK_SIZE    1024

// --- Channels ---
#define CHANNEL_MIN_24   1
#define CHANNEL_MAX_24   13

// 5GHz UNII band channels (RTL8720DN supports UNII-1/2/3)
#define CHANNEL_MIN_5    36
#define CHANNEL_MAX_5    165
// Helper: channel > 14 means 5GHz
#define IS_5GHZ_CH(ch)   ((ch) >= 36)

// Common 5GHz channels (UNII-1: 36-48, UNII-2A: 52-64, UNII-3: 149-165)
static const uint8_t CHANNELS_5GHZ[] = {
    36,  40,  44,  48,          // UNII-1
    52,  56,  60,  64,          // UNII-2A
    100, 104, 108, 112, 116,    // UNII-2C lower
    132, 136, 140, 144,         // UNII-2C upper
    149, 153, 157, 161, 165     // UNII-3
};
