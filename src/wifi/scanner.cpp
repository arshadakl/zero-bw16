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
    // Save previous BSSID→client_count mapping before overwriting _nets
    uint8_t prev_bssids[MAX_NETWORKS][6];
    uint8_t prev_clients[MAX_NETWORKS];
    int prev_count = _count;
    for (int i = 0; i < prev_count; i++) {
        memcpy(prev_bssids[i], _nets[i].bssid, 6);
        prev_clients[i] = _nets[i].client_count;
    }

    _count = 0;
    for (int i = 0; i < n && _count < MAX_NETWORKS; i++) {
        Network& net = _nets[_count];
        memset(&net, 0, sizeof(net)); // clear stale data from previous scan slot
        String ssid = WiFi.SSID(i);
        strncpy(net.ssid, ssid.c_str(), sizeof(net.ssid) - 1);
        net.ssid[sizeof(net.ssid) - 1] = '\0';
        WiFi.BSSID(net.bssid);   // fills buffer for last-indexed result
        net.rssi = WiFi.RSSI(i);
        net.channel = 0;          // WiFi.channel(i) not available on AmebaD
        net.encryption = WiFi.encryptionType(i);
        net.freq = IS_5GHZ_CH(net.channel) ? 1 : 0;
        net.hidden = (ssid.length() == 0);
        net.wps = false;
        net.pmf = false;
        // Restore client_count from previous scan using saved mapping
        net.client_count = 0;
        for (int j = 0; j < prev_count; j++) {
            if (memcmp(prev_bssids[j], net.bssid, 6) == 0) {
                net.client_count = prev_clients[j];
                break;
            }
        }
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
        int nc = (int)n->client_count + delta;
        n->client_count = nc < 0 ? 0 : (uint8_t)nc;
        _changes++;
    }
}

uint32_t Scanner::lastScanTime() const { return _lastScan; }
uint32_t Scanner::changeCounter() const { return _changes; }

void Scanner::_bssidStr(const uint8_t* b, char* out) {
    snprintf(out, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             b[0],b[1],b[2],b[3],b[4],b[5]);
}
