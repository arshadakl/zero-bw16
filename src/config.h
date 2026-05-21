#pragma once

#define FW_VERSION        "1.0.0"
#define PROJECT_NAME      "zero-bw16"

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
#define LED_PIN    PA27   // BW16 built-in LED (active low)
#define BUTTON_PIN PA12   // optional hardware button

// --- Web server ---
#define WEB_PORT   80
#define SSE_INTERVAL_MS 1000

// --- Limits ---
#define MAX_NETWORKS      30
#define MAX_PROBE_ENTRIES 50
#define MAX_CLIENT_ENTRIES 100
#define MAX_PMKID_ENTRIES  20
#define MAX_LOG_ENTRIES   200
#define MAX_BEACON_SSIDS   50

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
