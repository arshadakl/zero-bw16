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
