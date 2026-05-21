#include "scanner.h"
#include "../log.h"
#include "wifi_conf.h"

Scanner Scan;

static Scanner*      _scanInst = nullptr;
static volatile bool _cbDone   = false;

static rtw_result_t _onScanResult(rtw_scan_handler_result_t* item) {
    if (item->scan_complete) {
        _cbDone = true;
        return (rtw_result_t)RTW_SUCCESS;
    }
    if (_scanInst) _scanInst->_addScanResult(&item->ap_details);
    return (rtw_result_t)RTW_SUCCESS;
}

void Scanner::_addScanResult(const void* ap_ptr) {
    if (_count >= MAX_NETWORKS) return;
    const rtw_scan_result_t& ap = *(const rtw_scan_result_t*)ap_ptr;
    Network& net = _nets[_count++];
    memset(&net, 0, sizeof(net));

    int ssidlen = ap.SSID.len < 32 ? ap.SSID.len : 32;
    memcpy(net.ssid, ap.SSID.val, ssidlen);
    net.ssid[ssidlen] = 0;

    memcpy(net.bssid, ap.BSSID.octet, 6);
    net.rssi    = (int8_t)ap.signal_strength;
    net.channel = (uint8_t)ap.channel;
    net.freq    = (ap.band == RTW_802_11_BAND_5GHZ) ? 1 : 0;
    net.hidden  = (ap.SSID.len == 0);

    uint32_t sec = (uint32_t)ap.security;
    if      (sec & WEP_ENABLED)   net.encryption = 5;
    else if (sec & WPA2_SECURITY) net.encryption = 4;
    else if (sec & WPA_SECURITY)  net.encryption = 2;

    net.pmf = (sec & AES_CMAC_ENABLED) != 0;
    net.wps = (ap.wps_type != RTW_WPS_TYPE_NONE);

    net.rssi_history[0] = net.rssi;
    net.rssi_idx = 1;
}

void Scanner::init() {
    _count    = 0;
    _scanning = false;
}

bool Scanner::scan() {
    _scanning = true;
    Log.log(LOG_INFO, "Scan started");

    // Preserve sniffer-populated client counts across scan refresh
    uint8_t prev_bssids[MAX_NETWORKS][6];
    uint8_t prev_clients[MAX_NETWORKS];
    int prev_count = _count;
    for (int i = 0; i < prev_count; i++) {
        memcpy(prev_bssids[i], _nets[i].bssid, 6);
        prev_clients[i] = _nets[i].client_count;
    }

    _count    = 0;
    _cbDone   = false;
    _scanInst = this;
    wifi_scan_networks(_onScanResult, nullptr);

    uint32_t deadline = millis() + 8000;
    while (!_cbDone && millis() < deadline) delay(50);
    _scanInst = nullptr;
    _scanning = false;

    if (!_cbDone) {
        Log.log(LOG_ERROR, "Scan timeout");
        return false;
    }

    for (int i = 0; i < _count; i++) {
        for (int j = 0; j < prev_count; j++) {
            if (memcmp(prev_bssids[j], _nets[i].bssid, 6) == 0) {
                _nets[i].client_count = prev_clients[j];
                break;
            }
        }
    }

    _lastScan = millis();
    _changes++;
    Log.log(LOG_INFO, "Scan done: %d networks", _count);
    return true;
}

bool Scanner::isScanning() const { return _scanning; }
int  Scanner::networkCount() const { return _count; }

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
