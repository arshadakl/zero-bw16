#pragma once
#include <Arduino.h>
#include "../config.h"

struct Settings {
    char magic[4];          // "ZBW\0"
    uint8_t version;
    char ap_ssid[32];
    char ap_pass[32];
    uint8_t ap_channel;
    uint8_t deauth_frames;
    uint8_t send_delay_ms;
    uint8_t auto_scan;
    uint8_t auto_attack;
    uint8_t led_enabled;
    uint8_t language;       // 0=EN 1=ID 2=VI 3=TH 4=JP
    uint8_t deauth_reason;
    char profile_name[16];
    uint8_t _pad[8];
};

class NVS {
public:
    void init();
    void load();
    void save();
    void reset();
    Settings& cfg();
private:
    Settings _cfg;
    bool _valid = false;
    void _defaults();
};

extern NVS Nvs;
