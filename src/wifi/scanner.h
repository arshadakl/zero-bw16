#pragma once
#include <Arduino.h>
#include "../config.h"

struct Network {
    char ssid[33];
    uint8_t bssid[6];
    int8_t rssi;
    uint8_t channel;
    uint8_t encryption;   // WiFi encryptionType value
    uint8_t freq;         // 0=2.4GHz 1=5GHz
    bool wps;
    bool pmf;
    bool hidden;
    int8_t rssi_history[10];
    uint8_t rssi_idx;
    uint8_t client_count; // filled by sniffer
};

class Scanner {
public:
    void init();
    bool scan();           // blocking scan, returns true on success
    bool isScanning() const;

    int networkCount() const;
    const Network* network(int i) const;
    Network* networkByBSSID(const uint8_t* bssid);

    void updateClientCount(const uint8_t* bssid, int delta);
    uint32_t lastScanTime() const;
    uint32_t changeCounter() const;

private:
    Network _nets[MAX_NETWORKS];
    int _count = 0;
    bool _scanning = false;
    uint32_t _lastScan = 0;
    uint32_t _changes = 0;

    void _parseIE(const uint8_t* ie, int ie_len, bool& wps, bool& pmf);
    bool _isBand5(int16_t freq_mhz);
    void _bssidStr(const uint8_t* b, char* out);
};

extern Scanner Scan;
