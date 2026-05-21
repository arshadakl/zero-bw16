#include "nvs.h"
#include <Preferences.h>
#include "../log.h"

NVS Nvs;

static Preferences _prefs;
static const char* NVS_NS = "zbw";
static const char* NVS_KEY = "cfg";

void NVS::_defaults() {
    memset(&_cfg, 0, sizeof(_cfg));
    memcpy(_cfg.magic, NVS_MAGIC, 4);
    _cfg.version = 1;
    strlcpy(_cfg.ap_ssid, DEFAULT_AP_SSID, sizeof(_cfg.ap_ssid));
    strlcpy(_cfg.ap_pass, DEFAULT_AP_PASS, sizeof(_cfg.ap_pass));
    _cfg.ap_channel    = DEFAULT_AP_CHANNEL;
    _cfg.deauth_frames = DEFAULT_DEAUTH_FRAMES;
    _cfg.send_delay_ms = DEFAULT_SEND_DELAY;
    _cfg.auto_scan     = 0;
    _cfg.auto_attack   = 0;
    _cfg.led_enabled   = 1;
    _cfg.language      = 0;
    _cfg.deauth_reason = DEFAULT_DEAUTH_REASON;
    strlcpy(_cfg.profile_name, "default", sizeof(_cfg.profile_name));
}

void NVS::init() {
    load();
}

void NVS::load() {
    _prefs.begin(NVS_NS, true);
    size_t len = _prefs.getBytesLength(NVS_KEY);
    if (len == sizeof(_cfg)) {
        _prefs.getBytes(NVS_KEY, &_cfg, sizeof(_cfg));
    }
    _prefs.end();
    if (memcmp(_cfg.magic, NVS_MAGIC, 4) != 0 || _cfg.version != 1) {
        Log.log(LOG_WARN, "NVS invalid, loading defaults");
        _defaults();
        save();
    }
}

void NVS::save() {
    _prefs.begin(NVS_NS, false);
    _prefs.putBytes(NVS_KEY, &_cfg, sizeof(_cfg));
    _prefs.end();
}

void NVS::reset() {
    _defaults();
    save();
    Log.log(LOG_INFO, "NVS reset to defaults");
}

Settings& NVS::cfg() { return _cfg; }
