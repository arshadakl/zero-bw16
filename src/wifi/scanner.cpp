#include "scanner.h"
#include <WiFi.h>
#include "../log.h"

Scanner Scan;

void Scanner::init() {
    _count = 0;
    _scanning = false;
}

bool Scanner::scan() {
    _scanning = true;
    Log.log(LOG_INFO, "Scan started");
    int n = WiFi.scanNetworks();
    _scanning = false;
    if (n < 0) {
        Log.log(LOG_ERROR, "Scan failed: %d", n);
        return false;
    }
    int prev_count = _count;
    _count = 0;
    for (int i = 0; i < n && _count < MAX_NETWORKS; i++) {
        Network& net = _nets[_count];
        String ssid = WiFi.SSID(i);
        strlcpy(net.ssid, ssid.c_str(), sizeof(net.ssid));
        memcpy(net.bssid, WiFi.BSSID(i), 6);
        net.rssi = WiFi.RSSI(i);
        net.channel = WiFi.channel(i);
        net.encryption = WiFi.encryptionType(i);
        net.freq = 0; // BW16 scan API may not expose freq band directly
        net.hidden = (ssid.length() == 0);
        net.wps = false;
        net.pmf = false;
        // Preserve client_count from previous scan
        Network* prev = (prev_count > 0) ? networkByBSSID(net.bssid) : nullptr;
        net.client_count = prev ? prev->client_count : 0;
        // Rolling RSSI history
        net.rssi_history[net.rssi_idx % 10] = net.rssi;
        net.rssi_idx = (net.rssi_idx + 1) % 10;
        _count++;
    }
    _lastScan = millis();
    _changes++;
    Log.log(LOG_INFO, "Scan done: %d networks", _count);
    return true;
}

bool Scanner::isScanning() const { return _scanning; }
int Scanner::networkCount() const { return _count; }
const Network* Scanner::network(int i) const {
    if (i < 0 || i >= _count) return nullptr;
    return &_nets[i];
}

Network* Scanner::networkByBSSID(const uint8_t* bssid) {
    for (int i = 0; i < _count; i++) {
        if (memcmp(_nets[i].bssid, bssid, 6) == 0) return &_nets[i];
    }
    return nullptr;
}

void Scanner::updateClientCount(const uint8_t* bssid, int delta) {
    Network* n = networkByBSSID(bssid);
    if (n) {
        n->client_count = max(0, (int)n->client_count + delta);
        _changes++;
    }
}

uint32_t Scanner::lastScanTime() const { return _lastScan; }
uint32_t Scanner::changeCounter() const { return _changes; }

void Scanner::_bssidStr(const uint8_t* b, char* out) {
    snprintf(out, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             b[0],b[1],b[2],b[3],b[4],b[5]);
}
